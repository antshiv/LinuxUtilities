Optional host-specific Make overrides live here.

Pattern:
- `config/host/<hostname>.mk`

The main `Makefile` includes:
- `config/local.mk`
- `config/host/$(hostname -s).mk`

Use these files for machine-specific values such as:
- `WACOM_OUTPUT`
- `REVEAL_URL`
- `MANIM_DIR`
- `SMB_530_*`
- `AUDIO_OUTPUT_SINK`
- `BT_CARD`
