#!/usr/bin/env python3
"""Read a complete text file through CKE's checked Kokoro planner and player.

The newline-delimited JSON control protocol accepts speak_file, replace_file,
stop, pause, resume, status, and quit. CKE owns all text preparation, neural
execution, chunk order, and audio validation. This process owns request
lifecycle and prevents a cancelled request from reaching the next one.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import threading
import time
from typing import Callable


class ReaderError(RuntimeError):
    pass


class _Cancelled(Exception):
    pass


def _sha256(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def _checked_plan(plan: dict, text: str) -> int:
    source = text.strip()
    if (plan.get("schema") != "cke.kokoro.accessibility-text-plan.v1" or
            plan.get("source_text") != source or
            not isinstance(plan.get("chunks"), list) or not plan["chunks"]):
        raise ReaderError("CKE text plan has an invalid source or schema")
    position = 0
    for index, chunk in enumerate(plan["chunks"]):
        start, end = chunk.get("source_start"), chunk.get("source_end")
        ids = chunk.get("phoneme_ids")
        if (chunk.get("index") != index or type(start) is not int or
                type(end) is not int or start != position or
                not start < end <= len(source) or
                not isinstance(ids, list) or not ids or
                type(chunk.get("valid_token_count")) is not int or
                chunk["valid_token_count"] != len(ids) or
                chunk.get("source_text") != source[start:end].strip()):
            raise ReaderError(f"CKE text plan lost source coverage at chunk {index}")
        position = end
    if position != len(source):
        raise ReaderError("CKE text plan did not cover the complete source")
    return len(plan["chunks"])


class KokoroReader:
    def __init__(self, *, cke_root: Path, config: Path, bundle: Path,
                 player: Path | None = None, python: Path = Path(sys.executable),
                 emit: Callable[[dict], None] | None = None):
        self.cke_root = cke_root.resolve()
        self.config = config.resolve()
        self.bundle = bundle.resolve()
        self.player = player.resolve() if player is not None else None
        self.python = python.resolve()
        self.planner = self.cke_root / "version/v8/tts/prepare_kokoro_accessibility_text.py"
        self.playback = self.cke_root / "version/v8/tts/ck_kokoro_play.py"
        for path in (self.planner, self.playback, self.config, self.python):
            if not path.is_file():
                raise ReaderError(f"required CKE speech path is unavailable: {path}")
        if self.player is not None and (not self.player.is_file() or
                                        not os.access(self.player, os.X_OK)):
            raise ReaderError("selected audio player is not executable")
        self.emit = emit or (lambda event: None)
        self.lock = threading.RLock()
        self.request: dict | None = None
        self.next_id = 1

    def _event(self, event: str, request: dict, **fields):
        self.emit({"event": event, "request_id": request["id"], **fields})

    def _run_stage(self, request: dict, command: list[str], stage: str) -> None:
        with self.lock:
            if request["cancelled"]:
                raise _Cancelled
            child = subprocess.Popen(command, stdin=subprocess.DEVNULL,
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                     start_new_session=True, text=True)
            request["child"] = child
            request["stage"] = stage
        try:
            stdout, stderr = child.communicate()
        finally:
            with self.lock:
                if request["child"] is child:
                    request["child"] = None
        if request["cancelled"]:
            raise _Cancelled
        if child.returncode:
            detail = (stderr or stdout or "").strip()[-600:]
            raise ReaderError(f"CKE {stage} failed ({child.returncode}): {detail}")

    def _work(self, request: dict, text: str, mode: str) -> None:
        try:
            with tempfile.TemporaryDirectory(prefix="linuxutils-kokoro-") as folder:
                root = Path(folder)
                source = root / "source.txt"
                planned = root / "plan.json"
                report = root / "playback.json"
                source.write_text(text, encoding="utf-8")
                self._run_stage(request, [str(self.python), str(self.planner),
                    "--text-file", str(source), "--config", str(self.config),
                    "--bundle-dir", str(self.bundle), "--technical-mode", mode,
                    "--output", str(planned)], "planning")
                try:
                    plan = json.loads(planned.read_text(encoding="utf-8"))
                except (OSError, ValueError) as error:
                    raise ReaderError("CKE did not publish a readable text plan") from error
                count = _checked_plan(plan, text)
                self._event("planned", request, source_sha256=request["source_sha256"],
                            chunks=count, bundle_identity_sha256=plan.get("bundle_identity_sha256"),
                            source_spans=[[row["source_start"], row["source_end"]]
                                          for row in plan["chunks"]])
                if request["preflight_only"]:
                    self._event("preflight_complete", request, chunks=count)
                    return
                if self.player is None:
                    raise ReaderError("no audio player was selected")
                playback_started = time.monotonic()
                self._run_stage(request, [str(self.python), str(self.playback),
                    "--incremental", "--text-file", str(source),
                    "--config", str(self.config), "--bundle-dir", str(self.bundle),
                    "--player", str(self.player), "--technical-mode", mode,
                    "--report", str(report)], "synthesis_and_playback")
                try:
                    result = json.loads(report.read_text(encoding="utf-8"))
                except (OSError, ValueError) as error:
                    raise ReaderError("CKE did not publish a readable playback report") from error
                if (result.get("status") != "execution_pass" or
                        result.get("source_sha256") != request["source_sha256"] or
                        len(result.get("chunks", [])) != count):
                    raise ReaderError("CKE playback did not match the checked text plan")
                first_launch = result.get("first_playback_start_seconds")
                self._event("completed", request, chunks=count,
                            sample_count=result.get("sample_count"),
                            audio_seconds=result.get("audio_seconds"),
                            planner_seconds=playback_started - request["started"],
                            first_player_launch_seconds=(
                                playback_started - request["started"] + first_launch
                                if isinstance(first_launch, (int, float)) else None),
                            playback_gap_seconds=result.get("playback_gap_total_seconds"),
                            synthesis_seconds_total=sum(
                                row.get("synthesis_seconds", 0)
                                for row in result["chunks"]),
                            native_model_loads=result.get("native_model_loads"),
                            total_seconds=time.monotonic() - request["started"],
                            known_connected_waveform_parity=result.get(
                                "known_connected_waveform_parity"))
        except _Cancelled:
            self._event("cancelled", request)
        except Exception as error:
            if request["cancelled"]:
                self._event("cancelled", request)
            else:
                self._event("failed", request, detail=str(error))
        finally:
            with self.lock:
                request["stage"] = "finished"

    def speak_file(self, path: Path, *, mode: str = "natural",
                   expected_sha256: str | None = None,
                   preflight_only: bool = False) -> int:
        if mode not in ("natural", "exact"):
            raise ReaderError("technical mode must be natural or exact")
        if not preflight_only and self.player is None:
            raise ReaderError("no audio player was selected")
        raw = path.read_bytes()
        text = raw.decode("utf-8")
        if not text.strip() or "\x00" in text:
            raise ReaderError("speech text is empty or invalid")
        source_hash = _sha256(text.strip())
        file_hash = hashlib.sha256(raw).hexdigest()
        if expected_sha256 is not None and file_hash != expected_sha256:
            raise ReaderError("speech source SHA-256 differs from requested summary")
        with self.lock:
            if self.request and self.request["thread"].is_alive():
                raise ReaderError("speech is active; use replace_file or stop")
            request = {"id": self.next_id, "started": time.monotonic(),
                       "source_sha256": source_hash, "file_sha256": file_hash,
                       "preflight_only": preflight_only, "cancelled": False,
                       "paused": False, "child": None, "stage": "starting"}
            self.next_id += 1
            request["thread"] = threading.Thread(
                target=self._work, args=(request, text, mode), daemon=True)
            self.request = request
            self._event("started", request, source_sha256=request["source_sha256"],
                        file_sha256=file_hash)
            request["thread"].start()
        return request["id"]

    def stop(self) -> float:
        started = time.monotonic()
        with self.lock:
            request = self.request
            if request is None or not request["thread"].is_alive():
                return 0.0
            request["cancelled"] = True
            child = request["child"]
            if child is not None and child.poll() is None:
                try:
                    if request["paused"]:
                        os.killpg(child.pid, signal.SIGCONT)
                    os.killpg(child.pid, signal.SIGTERM)
                except ProcessLookupError:
                    pass
        request["thread"].join(timeout=3)
        if request["thread"].is_alive():
            with self.lock:
                child = request["child"]
                if child is not None and child.poll() is None:
                    try:
                        os.killpg(child.pid, signal.SIGKILL)
                    except ProcessLookupError:
                        pass
            request["thread"].join(timeout=3)
        if request["thread"].is_alive():
            raise ReaderError("cancelled CKE request did not release its worker")
        elapsed = time.monotonic() - started
        self._event("stopped", request, native_release_seconds=elapsed)
        return elapsed

    def replace_file(self, path: Path, *, mode: str = "natural",
                     expected_sha256: str | None = None) -> int:
        self.stop()
        return self.speak_file(path, mode=mode, expected_sha256=expected_sha256)

    def pause(self) -> None:
        with self.lock:
            request = self.request
            if not request or request["stage"] != "synthesis_and_playback" or request["paused"]:
                raise ReaderError("no active Kokoro playback to pause")
            child = request["child"]
            if child is None or child.poll() is not None:
                raise ReaderError("no active Kokoro playback to pause")
            os.killpg(child.pid, signal.SIGSTOP)
            request["paused"] = True
        self._event("paused", request)

    def resume(self) -> None:
        with self.lock:
            request = self.request
            if not request or not request["paused"]:
                raise ReaderError("no paused Kokoro playback to resume")
            child = request["child"]
            if child is None or child.poll() is not None:
                raise ReaderError("paused Kokoro playback already ended")
            os.killpg(child.pid, signal.SIGCONT)
            request["paused"] = False
        self._event("resumed", request)

    def status(self) -> dict:
        with self.lock:
            request = self.request
            return {"event": "status", "request_id": request["id"] if request else None,
                    "stage": request["stage"] if request else "idle",
                    "paused": request["paused"] if request else False}

    def wait(self) -> None:
        with self.lock:
            request = self.request
        if request:
            request["thread"].join()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cke-root", type=Path, required=True)
    parser.add_argument("--config", type=Path, required=True)
    parser.add_argument("--bundle-dir", type=Path, required=True)
    parser.add_argument("--player", type=Path,
                        help="explicit player executable required for audible speech")
    parser.add_argument("--python", type=Path, default=Path(sys.executable),
                        help="Python environment with CKE's pinned Kokoro frontend")
    parser.add_argument("--text-file", type=Path,
                        help="start by reading a complete UTF-8 summary")
    parser.add_argument("--technical-mode", choices=("natural", "exact"),
                        default="natural")
    parser.add_argument("--expected-sha256",
                        help="require this SHA-256 of the exact input file bytes")
    parser.add_argument("--preflight-only", action="store_true",
                        help="check complete CKE source-span coverage without playback")
    args = parser.parse_args()
    output_lock = threading.Lock()

    def emit(event: dict) -> None:
        with output_lock:
            print(json.dumps(event, ensure_ascii=False), flush=True)

    reader = None
    try:
        reader = KokoroReader(cke_root=args.cke_root, config=args.config,
                              bundle=args.bundle_dir, player=args.player,
                              python=args.python, emit=emit)
        if args.text_file:
            reader.speak_file(args.text_file, mode=args.technical_mode,
                              expected_sha256=args.expected_sha256,
                              preflight_only=args.preflight_only)
        for line in sys.stdin:
            try:
                command = json.loads(line)
                action = command.get("action")
                if action == "speak_file":
                    reader.speak_file(Path(command["path"]),
                                      mode=command.get("technical_mode", "natural"),
                                      expected_sha256=command.get("expected_sha256"))
                elif action == "preflight_file":
                    reader.speak_file(Path(command["path"]),
                                      mode=command.get("technical_mode", "natural"),
                                      expected_sha256=command.get("expected_sha256"),
                                      preflight_only=True)
                elif action == "replace_file":
                    reader.replace_file(Path(command["path"]),
                                        mode=command.get("technical_mode", "natural"),
                                        expected_sha256=command.get("expected_sha256"))
                elif action == "stop":
                    reader.stop()
                elif action == "pause":
                    reader.pause()
                elif action == "resume":
                    reader.resume()
                elif action == "status":
                    emit(reader.status())
                elif action == "quit":
                    reader.stop()
                    return 0
                else:
                    raise ReaderError("unknown Kokoro control action")
            except (ReaderError, OSError, ValueError, KeyError, TypeError) as error:
                emit({"event": "command_error", "detail": str(error)})
        reader.wait()
        return 0
    except KeyboardInterrupt:
        if reader is not None:
            reader.stop()
        return 130
    except (ReaderError, OSError) as error:
        emit({"event": "fatal", "detail": str(error)})
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
