#!/usr/bin/env python3
"""Transcript-driven shorts pipeline: transcribe audio and render styled captions."""

from __future__ import annotations

import argparse
import json
import math
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any


ROOT_DIR = Path(__file__).resolve().parents[1]
DEFAULT_STYLE_PATH = ROOT_DIR / "config" / "shorts_style_default.json"

ALIGNMENT_MAP = {
    "bottom_left": 1,
    "bottom_center": 2,
    "bottom_right": 3,
    "middle_left": 4,
    "middle_center": 5,
    "middle_right": 6,
    "top_left": 7,
    "top_center": 8,
    "top_right": 9,
}


def fail(message: str) -> int:
    print(f"ERROR: {message}", file=sys.stderr)
    return 1


def ensure_command_exists(name: str) -> None:
    check = subprocess.run(["bash", "-lc", f"command -v {shlex.quote(name)} >/dev/null 2>&1"])
    if check.returncode != 0:
        raise RuntimeError(f"Missing required command: {name}")


def run_checked(cmd: list[str]) -> None:
    print("+", " ".join(shlex.quote(part) for part in cmd))
    result = subprocess.run(cmd)
    if result.returncode != 0:
        raise RuntimeError(f"Command failed with exit code {result.returncode}")


def run_capture(cmd: list[str]) -> str:
    result = subprocess.run(cmd, check=True, text=True, capture_output=True)
    return result.stdout.strip()


def load_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as fh:
        return json.load(fh)


def write_json(path: Path, payload: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as fh:
        json.dump(payload, fh, indent=2, ensure_ascii=False)
        fh.write("\n")


def parse_hex_argb(value: str, fallback: str) -> tuple[str, str, str, str]:
    raw = (value or fallback).strip()
    if raw.startswith("#"):
        raw = raw[1:]
    if len(raw) == 6:
        alpha = "00"
        red, green, blue = raw[0:2], raw[2:4], raw[4:6]
    elif len(raw) == 8:
        alpha = raw[0:2]
        red, green, blue = raw[2:4], raw[4:6], raw[6:8]
    else:
        raw = fallback.strip().lstrip("#")
        if len(raw) == 6:
            alpha = "00"
            red, green, blue = raw[0:2], raw[2:4], raw[4:6]
        else:
            alpha = "88"
            red, green, blue = "00", "00", "00"
    return alpha.upper(), red.upper(), green.upper(), blue.upper()


def to_ass_color(value: str, fallback: str) -> str:
    alpha, red, green, blue = parse_hex_argb(value, fallback)
    return f"&H{alpha}{blue}{green}{red}"


def ass_timestamp(seconds: float) -> str:
    centis = max(0, int(round(seconds * 100)))
    hours = centis // 360000
    minutes = (centis % 360000) // 6000
    secs = (centis % 6000) // 100
    cs = centis % 100
    return f"{hours}:{minutes:02d}:{secs:02d}.{cs:02d}"


def ass_escape_text(text: str) -> str:
    escaped = str(text).replace("\\", r"\\").replace("{", r"\{").replace("}", r"\}")
    return escaped.replace("\n", r"\N")


def normalize_segments(payload: Any) -> list[dict[str, Any]]:
    if isinstance(payload, dict):
        raw_segments = payload.get("segments", [])
    elif isinstance(payload, list):
        raw_segments = payload
    else:
        raw_segments = []

    normalized: list[dict[str, Any]] = []
    for item in raw_segments:
        if not isinstance(item, dict):
            continue
        text = str(item.get("text", "")).strip()
        if not text:
            continue
        try:
            start = float(item.get("start"))
            end = float(item.get("end"))
        except (TypeError, ValueError):
            continue
        if end <= start:
            continue
        normalized.append({"start": start, "end": end, "text": text})

    normalized.sort(key=lambda seg: (seg["start"], seg["end"]))
    return normalized


def transform_text(text: str, style: dict[str, Any]) -> str:
    result = str(text)
    replace_map = style.get("replace", {})
    if isinstance(replace_map, dict):
        for src, dst in replace_map.items():
            result = result.replace(str(src), str(dst))
    if style.get("uppercase"):
        result = result.upper()
    return " ".join(result.split())


def chunk_caption(start: float, end: float, text: str, max_words: int) -> list[dict[str, Any]]:
    words = text.split()
    if not words:
        return []
    if max_words <= 0 or len(words) <= max_words:
        return [{"start": start, "end": end, "text": text}]

    chunk_count = int(math.ceil(len(words) / max_words))
    total = max(0.001, end - start)
    step = total / chunk_count
    chunks: list[dict[str, Any]] = []
    for idx in range(chunk_count):
        chunk_words = words[idx * max_words : (idx + 1) * max_words]
        if not chunk_words:
            continue
        c_start = start + idx * step
        c_end = end if idx == chunk_count - 1 else start + (idx + 1) * step
        if c_end - c_start < 0.05:
            c_end = c_start + 0.05
        chunks.append({"start": c_start, "end": c_end, "text": " ".join(chunk_words)})
    return chunks


def probe_resolution(video_path: Path) -> tuple[int, int]:
    ensure_command_exists("ffprobe")
    out = run_capture(
        [
            "ffprobe",
            "-v",
            "error",
            "-select_streams",
            "v:0",
            "-show_entries",
            "stream=width,height",
            "-of",
            "csv=p=0:s=x",
            str(video_path),
        ]
    )
    try:
        width_str, height_str = out.split("x", 1)
        return int(width_str), int(height_str)
    except Exception as exc:  # noqa: BLE001
        raise RuntimeError(f"Unable to read video resolution from ffprobe output: {out}") from exc


def build_ass_file(
    ass_path: Path,
    captions: list[dict[str, Any]],
    style: dict[str, Any],
    play_res_x: int,
    play_res_y: int,
) -> None:
    font_family = str(style.get("font_family", "DejaVu Sans"))
    font_size = int(style.get("font_size", 64))
    bold = -1 if bool(style.get("bold", True)) else 0
    italic = -1 if bool(style.get("italic", False)) else 0
    alignment = ALIGNMENT_MAP.get(str(style.get("alignment", "bottom_center")), 2)
    margin_h = int(style.get("margin_h", 80))
    margin_v = int(style.get("margin_v", 120))

    primary = to_ass_color(str(style.get("text_color", "#FFFFFF")), "#FFFFFF")
    secondary = to_ass_color(str(style.get("secondary_color", "#FFFFFF")), "#FFFFFF")
    outline_color = to_ass_color(str(style.get("outline_color", "#000000")), "#000000")
    background_raw = str(style.get("background_color", "#88000000"))
    back = to_ass_color(background_raw, "#88000000")
    bg_alpha, _, _, _ = parse_hex_argb(background_raw, "#88000000")

    border_style = 3 if bg_alpha != "FF" else 1
    outline_size = float(style.get("outline_size", 0 if border_style == 3 else 2))
    shadow = float(style.get("shadow_size", 0))

    lines: list[str] = [
        "[Script Info]",
        "ScriptType: v4.00+",
        f"PlayResX: {play_res_x}",
        f"PlayResY: {play_res_y}",
        "ScaledBorderAndShadow: yes",
        "WrapStyle: 2",
        "",
        "[V4+ Styles]",
        "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding",
        (
            "Style: Default,"
            f"{font_family},{font_size},{primary},{secondary},{outline_color},{back},"
            f"{bold},{italic},0,0,100,100,0,0,{border_style},{outline_size},{shadow},"
            f"{alignment},{margin_h},{margin_h},{margin_v},1"
        ),
        "",
        "[Events]",
        "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text",
    ]

    for caption in captions:
        start = ass_timestamp(float(caption["start"]))
        end = ass_timestamp(float(caption["end"]))
        text = ass_escape_text(str(caption["text"]))
        lines.append(f"Dialogue: 0,{start},{end},Default,,0,0,0,,{text}")

    ass_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def maybe_float(value: Any) -> float | None:
    if value is None or value == "":
        return None
    try:
        return float(value)
    except (TypeError, ValueError):
        return None


def do_transcribe(args: argparse.Namespace) -> int:
    audio_path = Path(args.audio).expanduser().resolve()
    if not audio_path.exists():
        return fail(f"Audio/video file not found: {audio_path}")

    out_path = Path(args.output).expanduser().resolve()
    language = None if args.language == "auto" else args.language
    segments_out: list[dict[str, Any]] = []
    detected_language = args.language
    backend = ""

    try:
        from faster_whisper import WhisperModel  # type: ignore

        backend = "faster-whisper"
        model = WhisperModel(args.model, device="auto", compute_type=args.compute_type)
        segments, info = model.transcribe(str(audio_path), language=language, vad_filter=True)
        for seg in segments:
            text = str(seg.text).strip()
            if not text:
                continue
            start = float(seg.start)
            end = float(seg.end)
            if end <= start:
                continue
            segments_out.append({"start": start, "end": end, "text": text})
        detected_language = getattr(info, "language", args.language) or args.language
    except ModuleNotFoundError:
        try:
            import whisper  # type: ignore

            backend = "openai-whisper"
            model = whisper.load_model(args.model)
            result = model.transcribe(str(audio_path), language=language)
            detected_language = str(result.get("language", args.language))
            for seg in result.get("segments", []):
                text = str(seg.get("text", "")).strip()
                if not text:
                    continue
                start = float(seg.get("start", 0.0))
                end = float(seg.get("end", start))
                if end <= start:
                    continue
                segments_out.append({"start": start, "end": end, "text": text})
        except ModuleNotFoundError:
            return fail(
                "No transcription backend found. Install one:\n"
                "  pip install faster-whisper\n"
                "or\n"
                "  pip install openai-whisper"
            )

    if not segments_out:
        return fail("Transcription produced no segments.")

    payload = {
        "version": 1,
        "backend": backend,
        "source_audio": str(audio_path),
        "language": detected_language,
        "segments": segments_out,
    }
    write_json(out_path, payload)
    print(f"Wrote transcript: {out_path} ({len(segments_out)} segments)")
    return 0


def do_render(args: argparse.Namespace) -> int:
    video_path = Path(args.video).expanduser().resolve()
    transcript_path = Path(args.transcript).expanduser().resolve()
    style_path = Path(args.style).expanduser().resolve()
    output_path = Path(args.output).expanduser().resolve()

    if not video_path.exists():
        return fail(f"Video file not found: {video_path}")
    if not transcript_path.exists():
        return fail(f"Transcript file not found: {transcript_path}")
    if not style_path.exists():
        return fail(f"Style file not found: {style_path}")

    try:
        ensure_command_exists("ffmpeg")
        ensure_command_exists("ffprobe")
    except RuntimeError as exc:
        return fail(str(exc))

    transcript_payload = load_json(transcript_path)
    style = load_json(style_path)
    if not isinstance(style, dict):
        return fail("Style JSON must be an object.")

    segments = normalize_segments(transcript_payload)
    if not segments:
        return fail("Transcript contains no valid segments.")

    clip_start = maybe_float(args.start)
    if clip_start is None:
        clip_start = maybe_float(style.get("clip_start_sec")) or 0.0
    clip_duration = maybe_float(args.duration)
    if clip_duration is None:
        clip_duration = maybe_float(style.get("clip_duration_sec"))
    clip_end = clip_start + clip_duration if clip_duration is not None else float("inf")

    max_words = int(style.get("max_words_per_caption", 6))
    captions: list[dict[str, Any]] = []
    for seg in segments:
        if seg["end"] <= clip_start:
            continue
        if seg["start"] >= clip_end:
            continue
        start_rel = max(seg["start"], clip_start) - clip_start
        end_rel = min(seg["end"], clip_end) - clip_start
        if end_rel <= start_rel:
            continue
        text = transform_text(seg["text"], style)
        if not text:
            continue
        captions.extend(chunk_caption(start_rel, end_rel, text, max_words))

    if not captions:
        return fail("No captions remained after clip window/filtering.")

    source_w, source_h = probe_resolution(video_path)
    target_w = int(style.get("target_width", source_w))
    target_h = int(style.get("target_height", source_h))

    with tempfile.TemporaryDirectory(prefix="shorts-studio-") as tmpdir:
        ass_path = Path(tmpdir) / "captions.ass"
        build_ass_file(ass_path, captions, style, target_w, target_h)

        vf_chain: list[str] = []
        if target_w != source_w or target_h != source_h:
            vf_chain.append(
                f"scale={target_w}:{target_h}:force_original_aspect_ratio=increase,crop={target_w}:{target_h}"
            )
        vf_chain.append(f"ass={ass_path}")
        vf = ",".join(vf_chain)

        cmd = ["ffmpeg", "-y", "-i", str(video_path)]
        if clip_start > 0:
            cmd.extend(["-ss", f"{clip_start:.3f}"])
        if clip_duration is not None:
            cmd.extend(["-t", f"{clip_duration:.3f}"])
        cmd.extend(
            [
                "-map",
                "0:v:0",
                "-map",
                "0:a?",
                "-vf",
                vf,
                "-c:v",
                "libx264",
                "-preset",
                "medium",
                "-crf",
                str(style.get("crf", 18)),
                "-c:a",
                "aac",
                "-b:a",
                str(style.get("audio_bitrate", "192k")),
                "-movflags",
                "+faststart",
                str(output_path),
            ]
        )
        try:
            run_checked(cmd)
        except RuntimeError as exc:
            return fail(str(exc))

    print(f"Wrote video: {output_path}")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Transcript-driven shorts: transcribe media and render styled captioned clips."
    )
    sub = parser.add_subparsers(dest="command", required=True)

    p_trans = sub.add_parser("transcribe", help="Transcribe audio/video into timestamped JSON.")
    p_trans.add_argument("--audio", required=True, help="Input audio/video file path.")
    p_trans.add_argument("--output", required=True, help="Output transcript JSON path.")
    p_trans.add_argument("--model", default="small", help="Whisper model name (default: small).")
    p_trans.add_argument(
        "--language",
        default="en",
        help="Language code (e.g. en) or 'auto'. Default: en.",
    )
    p_trans.add_argument(
        "--compute-type",
        default="int8",
        help="faster-whisper compute type (default: int8).",
    )
    p_trans.set_defaults(func=do_transcribe)

    p_render = sub.add_parser("render", help="Render transcript captions onto video and export MP4.")
    p_render.add_argument("--video", required=True, help="Input video file path.")
    p_render.add_argument("--transcript", required=True, help="Transcript JSON path.")
    p_render.add_argument(
        "--style",
        default=str(DEFAULT_STYLE_PATH),
        help=f"Caption/style JSON path (default: {DEFAULT_STYLE_PATH}).",
    )
    p_render.add_argument("--output", required=True, help="Output MP4 path.")
    p_render.add_argument("--start", help="Optional clip start (seconds). Overrides style.")
    p_render.add_argument("--duration", help="Optional clip duration (seconds). Overrides style.")
    p_render.set_defaults(func=do_render)

    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    return int(args.func(args))


if __name__ == "__main__":
    raise SystemExit(main())
