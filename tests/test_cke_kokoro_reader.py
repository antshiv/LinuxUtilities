"""The LinuxUtilities adapter keeps CKE as the only speech planner/runtime."""

import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from cke_kokoro_reader import KokoroReader, ReaderError, _checked_plan  # noqa: E402


def wait_for(predicate, timeout=5):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.01)
    raise AssertionError("timed out waiting for speech event")


class KokoroReaderTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.cke = self.root / "cke"
        tts = self.cke / "version/v8/tts"
        tts.mkdir(parents=True)
        self.bundle = self.root / "bundle"
        self.bundle.mkdir()
        self.config = self.root / "config.json"
        self.config.write_text("{}")
        self.player = self.root / "player"
        self.player.write_text("#!/bin/sh\nexit 0\n")
        self.player.chmod(0o700)
        self.started = self.root / "started.txt"
        self.completed = self.root / "completed.txt"
        (tts / "prepare_kokoro_accessibility_text.py").write_text(
            "import argparse,json,sys\nfrom pathlib import Path\n"
            "p=argparse.ArgumentParser()\n"
            "for n in ('text-file','config','bundle-dir','technical-mode','output'):p.add_argument('--'+n)\n"
            "a=p.parse_args();text=Path(a.text_file).read_text().strip()\n"
            "if 'UNSUPPORTED' in text:print('unsupported source span',file=sys.stderr);sys.exit(2)\n"
            "plan={'schema':'cke.kokoro.accessibility-text-plan.v1','source_text':text,"
            "'bundle_identity_sha256':'synthetic','chunks':[{'index':0,'source_start':0,"
            "'source_end':len(text),'source_text':text,'phoneme_ids':[0,1,0],"
            "'valid_token_count':3}]}\n"
            "Path(a.output).write_text(json.dumps(plan))\n")
        (tts / "ck_kokoro_play.py").write_text(
            "import argparse,hashlib,json,subprocess,sys,time\nfrom pathlib import Path\n"
            "p=argparse.ArgumentParser()\n"
            "p.add_argument('--incremental',action='store_true')\n"
            "for n in ('text-file','config','bundle-dir','player','technical-mode','report'):p.add_argument('--'+n)\n"
            "a=p.parse_args();text=Path(a.text_file).read_text().strip()\n"
            f"Path({str(self.started)!r}).write_text(text)\n"
            "if 'slow' in text:subprocess.run([sys.executable,'-c','import time;time.sleep(30)'])\n"
            "else:time.sleep(0.03)\n"
            f"with Path({str(self.completed)!r}).open('a') as f:f.write(json.dumps(text)+'\\n')\n"
            "result={'status':'execution_pass','source_sha256':hashlib.sha256(text.encode()).hexdigest(),"
            "'chunks':[{'index':0}],'sample_count':1200,'first_playback_start_seconds':0.05,"
            "'playback_gap_total_seconds':0.0,'known_connected_waveform_parity':'fail_unresolved'}\n"
            "Path(a.report).write_text(json.dumps(result))\n")
        self.events = []
        self.reader = KokoroReader(cke_root=self.cke, config=self.config,
                                   bundle=self.bundle, player=self.player,
                                   emit=self.events.append)
        self.addCleanup(self.reader.stop)

    def source(self, name, text):
        path = self.root / name
        path.write_text(text)
        return path

    def completed_texts(self):
        return [json.loads(line) for line in self.completed.read_text().splitlines()]

    def test_complete_text_is_planned_then_played_with_identity(self):
        source = self.source("summary.txt", "CKE PR 862 and issue 833.\nRead everything.")
        request_id = self.reader.speak_file(source)
        self.reader.wait()
        self.assertEqual(self.completed_texts(), [source.read_text().strip()])
        kinds = [event["event"] for event in self.events]
        self.assertIn("planned", kinds)
        self.assertIn("completed", kinds)
        planned = next(event for event in self.events if event["event"] == "planned")
        self.assertEqual(planned["request_id"], request_id)
        self.assertEqual(planned["source_spans"], [[0, len(source.read_text().strip())]])
        completed = next(event for event in self.events if event["event"] == "completed")
        self.assertGreaterEqual(completed["first_player_launch_seconds"], 0.05)
        self.assertEqual(completed["known_connected_waveform_parity"], "fail_unresolved")
        self.assertEqual(self.reader.status()["stage"], "finished")

    def test_stop_replace_and_recovery_cannot_publish_old_audio(self):
        slow = self.source("slow.txt", "slow first summary")
        fresh = self.source("fresh.txt", "second summary")
        third = self.source("third.txt", "third summary")
        old_id = self.reader.speak_file(slow)
        wait_for(lambda: self.started.exists() and self.started.read_text() == "slow first summary")
        elapsed = self.reader.stop()
        self.assertLess(elapsed, 3)
        new_id = self.reader.replace_file(fresh)
        self.reader.wait()
        self.assertGreater(new_id, old_id)
        self.assertEqual(self.completed_texts(), ["second summary"])
        self.reader.speak_file(third)
        self.reader.wait()
        self.assertEqual(self.completed_texts(),
                         ["second summary", "third summary"])
        self.assertTrue(any(row["event"] == "cancelled" and row["request_id"] == old_id
                            for row in self.events))

    def test_pause_resume_then_stop_releases_playback_group(self):
        self.reader.speak_file(self.source("slow.txt", "slow paused summary"))
        wait_for(lambda: self.started.exists())
        self.reader.pause()
        self.assertTrue(self.reader.status()["paused"])
        self.reader.resume()
        self.assertFalse(self.reader.status()["paused"])
        self.reader.pause()
        self.assertLess(self.reader.stop(), 3)
        self.assertFalse(self.completed.exists())
        self.assertEqual([row["event"] for row in self.events
                          if row["event"] in ("paused", "resumed")],
                         ["paused", "resumed", "paused"])

    def test_unsupported_text_fails_before_playback(self):
        self.reader.speak_file(self.source("bad.txt", "UNSUPPORTED token"))
        self.reader.wait()
        self.assertFalse(self.started.exists())
        failure = next(row for row in self.events if row["event"] == "failed")
        self.assertIn("unsupported source span", failure["detail"])
        self.reader.speak_file(self.source("good.txt", "good next request"))
        self.reader.wait()
        self.assertEqual(self.completed_texts(), ["good next request"])

    def test_preflight_covers_complete_file_without_launching_playback(self):
        source = self.source("planned.txt", "Read the whole summary, including PR 862.")
        self.reader.speak_file(source, preflight_only=True)
        self.reader.wait()
        self.assertFalse(self.started.exists())
        self.assertTrue(any(row["event"] == "preflight_complete" for row in self.events))

    def test_one_shot_cli_emits_complete_report_with_file_identity(self):
        source = self.source("summary.txt", "Read PR 862 and issue 833.")
        digest = hashlib.sha256(source.read_bytes()).hexdigest()
        script = Path(__file__).resolve().parents[1] / "scripts/cke_kokoro_reader.py"
        result = subprocess.run([sys.executable, str(script),
            "--cke-root", str(self.cke), "--config", str(self.config),
            "--bundle-dir", str(self.bundle), "--player", str(self.player),
            "--text-file", str(source), "--expected-sha256", digest],
            input="", text=True, capture_output=True, timeout=5)
        self.assertEqual(result.returncode, 0, result.stderr)
        events = [json.loads(line) for line in result.stdout.splitlines()]
        self.assertEqual([row["event"] for row in events],
                         ["started", "planned", "completed"])
        self.assertEqual(events[0]["file_sha256"], digest)
        self.assertEqual(events[-1]["sample_count"], 1200)

    def test_rejects_missing_plan_spans_and_invalid_modes(self):
        with self.assertRaisesRegex(ReaderError, "cover the complete source"):
            _checked_plan({"schema": "cke.kokoro.accessibility-text-plan.v1",
                           "source_text": "one two", "chunks": [{"index": 0,
                            "source_start": 0, "source_end": 3,
                            "source_text": "one", "phoneme_ids": [0, 1, 0],
                            "valid_token_count": 3}]}, "one two")
        with self.assertRaisesRegex(ReaderError, "technical mode"):
            self.reader.speak_file(self.source("good.txt", "hello"), mode="guess")
        with self.assertRaisesRegex(ReaderError, "SHA-256"):
            self.reader.speak_file(self.source("hash.txt", "changed summary"),
                                   expected_sha256="0" * 64)
        self.assertFalse(self.started.exists())

    def test_document_preflight_and_durable_resume_use_cke_interfaces(self):
        tts = self.cke / "version/v8/tts"
        (tts / "prepare_kokoro_document.py").write_text(
            "import argparse,json\nfrom pathlib import Path\n"
            "p=argparse.ArgumentParser()\n"
            "for n in ('text-file','config','bundle-dir','technical-mode','output'):p.add_argument('--'+n)\n"
            "a=p.parse_args();text=Path(a.text_file).read_text()\n"
            "cut=text.index('.')+1\n"
            "chunks=[{'index':i,'source_start':start,'source_end':end,"
            "'source_text':text[start:end].strip(),'phoneme_ids':[0,1,0],"
            "'valid_token_count':3} for i,(start,end) in "
            "enumerate(((0,cut),(cut,len(text))))]\n"
            "plan={'schema':'cke.kokoro.document-text-plan.v1',"
            "'source_text':text,'chunks':chunks,'bundle_identity_sha256':'bundle'}\n"
            "Path(a.output).write_text(json.dumps(plan))\n")
        (tts / "ck_kokoro_play.py").write_text(
            "import argparse,hashlib,json\nfrom pathlib import Path\n"
            "p=argparse.ArgumentParser()\n"
            "p.add_argument('--incremental',action='store_true')\n"
            "p.add_argument('--document',action='store_true')\n"
            "p.add_argument('--resume',action='store_true')\n"
            "for n in ('text-file','config','bundle-dir','player','technical-mode',"
            "'report','progress-file','session-id'):p.add_argument('--'+n)\n"
            "a=p.parse_args();assert a.incremental and a.document\n"
            "text=Path(a.text_file).read_text();progress=Path(a.progress_file)\n"
            "if a.resume: assert progress.read_text()=='0';first=1\n"
            "else: assert not progress.exists();progress.write_text('0');first=0\n"
            "if first==0: raise SystemExit(2)\n"
            "progress.write_text('1')\n"
            "result={'status':'execution_pass','source_sha256':hashlib.sha256(text.encode()).hexdigest(),"
            "'resume_from_chunk':first,'chunks':[{'index':1}],"
            "'sample_count':1200,'first_playback_start_seconds':0.05,"
            "'known_connected_waveform_parity':'fail_unresolved'}\n"
            "Path(a.report).write_text(json.dumps(result))\n")
        progress_dir = self.root / "progress"
        reader = KokoroReader(cke_root=self.cke, config=self.config,
                              bundle=self.bundle, player=self.player,
                              progress_dir=progress_dir, emit=self.events.append)
        self.addCleanup(reader.stop)
        source = self.source("document.txt", "First sentence. Second sentence.\n")
        reader.speak_file(source, document=True, preflight_only=True)
        reader.wait()
        planned = next(row for row in self.events if row["event"] == "planned")
        self.assertEqual(planned["chunks"], 2)
        self.assertEqual(planned["source_spans"],
                         [[0, 15], [15, len(source.read_text())]])
        self.assertFalse(progress_dir.exists())
        reader.speak_file(source, document=True)
        reader.wait()
        self.assertTrue(any(row["event"] == "failed" for row in self.events))
        checkpoint = next(progress_dir.glob("*.json"))
        self.assertEqual(checkpoint.read_text(), "0")
        reader.speak_file(source, document=True, resume_existing=True)
        reader.wait()
        completed = next(row for row in self.events if row["event"] == "completed")
        self.assertEqual(completed["resume_from_chunk"], 1)
        self.assertEqual(completed["chunks"], 2)
        self.assertEqual(checkpoint.read_text(), "1")
        with self.assertRaisesRegex(ReaderError, "checkpoint exists"):
            reader.speak_file(source, document=True)


if __name__ == "__main__":
    unittest.main()
