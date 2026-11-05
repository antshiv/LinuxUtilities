# CKE Kokoro summary reader

`scripts/cke_kokoro_reader.py` is the LinuxUtilities control boundary for CKE
Kokoro. It calls CKE's checked full-context text planner and incremental player.
LinuxUtilities does not phonemize, replace words, split sentences, or synthesize
audio. All chunks must cover the original source in order before playback starts.

## Read the current summary

The `server_summary.txt` observed on 2026-11-05 had raw-file SHA-256
`3fa845c01db57312158592a46dd0e7a19f4e8462096eea10b5f52103e1f66bdb`.
It was 1,776 bytes. The hash check rejects a changed file rather than reading
an unintended revision. Set these paths for the actual machine:

```sh
CKE_READER_ROOT=/home/antshiv/Workspace/C-Kernel-Engine
CKE_READER_CONFIG=/tmp/cke-kokoro-model-pinned/config.json
CKE_READER_BUNDLE=/tmp/cke-kokoro-p3-tile32-w8-bundle-20261104
CKE_READER_PYTHON=/path/to/pinned-kokoro-frontend/bin/python
CKE_READER_SUMMARY="$CKE_READER_ROOT/server_summary.txt"
CKE_READER_HASH=3fa845c01db57312158592a46dd0e7a19f4e8462096eea10b5f52103e1f66bdb
```

First check every source span without launching audio:

```sh
python3 scripts/cke_kokoro_reader.py \
  --cke-root "$CKE_READER_ROOT" --python "$CKE_READER_PYTHON" \
  --config "$CKE_READER_CONFIG" --bundle-dir "$CKE_READER_BUNDLE" \
  --text-file "$CKE_READER_SUMMARY" --expected-sha256 "$CKE_READER_HASH" \
  --preflight-only
```

The terminal must report `planned` with complete source spans followed by
`preflight_complete`. An unknown word or unsupported phoneme produces `failed`
with CKE's error, and playback never starts.

For audible reading, select the Sony output device in PipeWire and supply an
explicit executable player. For example, when `pw-play` is installed:

```sh
python3 scripts/cke_kokoro_reader.py \
  --cke-root "$CKE_READER_ROOT" --python "$CKE_READER_PYTHON" \
  --config "$CKE_READER_CONFIG" --bundle-dir "$CKE_READER_BUNDLE" \
  --player "$(command -v pw-play)" \
  --text-file "$CKE_READER_SUMMARY" --expected-sha256 "$CKE_READER_HASH"
```

Each line of standard input is a JSON control command. These also work when
another local application owns the controller process and sends commands over
its stdin:

```json
{"action":"stop"}
{"action":"replace_file","path":"/absolute/path/new-summary.txt"}
{"action":"pause"}
{"action":"resume"}
{"action":"status"}
{"action":"quit"}
```

`preflight_file` and `speak_file` accept `path`, optional `technical_mode`
(`natural` or `exact`), and optional `expected_sha256`. The controller emits
newline-delimited JSON with request IDs, source hashes, checked spans, failures,
completion metrics, and native-release time after stop. The reported first
player-launch time includes planning; it is not a measurement of the first
audible sample at the headset. `replace_file` waits for
the old process group to exit before launching the replacement, preventing old
audio from returning. The exact mode spells technical paths according to CKE's
planner; LinuxUtilities never rewrites them itself.

## Resume a long document

With CKE PR #1037 or newer, pass `--document` for the checked document planner
and `--progress-dir` for durable playback checkpoints. Keep the checkpoint
directory private; its filename contains the source-file hash, while the
checkpoint records model/frontend/voice identity and the last completed chunk.
The planner checks the entire source before audio starts and rejects unsupported
text at a named source span. The document remains unchanged.

```sh
python3 scripts/cke_kokoro_reader.py \
  --cke-root "$CKE_READER_ROOT" --python "$CKE_READER_PYTHON" \
  --config "$CKE_READER_CONFIG" --bundle-dir "$CKE_READER_BUNDLE" \
  --player "$(command -v pw-play)" --document \
  --progress-dir "$HOME/.local/state/cke-kokoro-documents" \
  --text-file /absolute/path/document.txt
```

After a stop or process failure, rerun the same command with `--resume-file`.
In an interactive controller session, send
`{"action":"resume_file","path":"/absolute/path/document.txt"}`. This
resumes after the last player-completed chunk. An interrupted sentence may
replay. The in-process `resume` command only unpauses current playback; it does
not load a saved checkpoint. For a new reading of the same document, move or
delete its checkpoint explicitly, or use a separate progress directory.

The real P3-to-Sony route still requires an explicit transport if synthesis
runs on P3 and the headset is on a laptop. This adapter currently invokes a
local CKE player and does not certify remote audio transport or uninterrupted
10,000-word playback. Its bounded spool avoids holding the whole document WAV
in RAM; if synthesis remains slower than playback, gaps will occur.

## Evidence boundaries

The asset-free controller tests cover source-span integrity, full-file handoff,
unsupported text, stop during native work, replacement, and a later successful
request. The real pinned frontend, generated bundle, Sony routing, audible gaps,
and human listening require the asset-backed headset trial. The current machine
used for this change lacks the pinned Kokoro frontend packages and an installed
audio player, so the complete summary has **not** been heard or certified here.

CKE's incremental player keeps its native model loaded across the chunks of one
request. It starts a new native worker for a subsequent request. Its report
distinguishes execution from the still unresolved connected waveform numerical
parity. Check first-audio time, total time, gaps, and headset intelligibility
separately; buffering alone cannot make synthesis faster than playback.
