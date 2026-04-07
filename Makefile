SHELL := /bin/bash
.RECIPEPREFIX := >

AWESOME_SYSTEM_DIR ?= /etc/xdg/awesome
AWESOME_SYSTEM_RC ?= $(AWESOME_SYSTEM_DIR)/rc.lua
AWESOME_USER_DIR ?= $(HOME)/.config/awesome
AWESOME_USER_RC ?= $(AWESOME_USER_DIR)/rc.lua
AWESOME_ACTIVE_RC ?= $(if $(wildcard $(AWESOME_USER_RC)),$(AWESOME_USER_RC),$(AWESOME_SYSTEM_RC))
AWESOME_MODULE_DIR ?= linuxutils
AWESOME_MODULE_FILES ?= $(sort $(wildcard $(AWESOME_MODULE_DIR)/*.lua))
AWESOME_ICON_DIR ?= icons
LOCAL_RC ?= rc.lua
LOCAL_RC_BACKUP ?= rc.dupe.lua
LOCAL_AWESOME_MODULE_BACKUP ?= $(AWESOME_MODULE_DIR).dupe
LOCAL_AWESOME_ICON_BACKUP ?= $(AWESOME_ICON_DIR).dupe
BACKUP_TAG := $(shell date +%Y%m%d-%H%M%S)
USER_RC_BACKUP ?= $(AWESOME_USER_RC).bak.$(BACKUP_TAG)
USER_MODULE_BACKUP ?= $(AWESOME_USER_DIR)/$(AWESOME_MODULE_DIR).bak.$(BACKUP_TAG)
USER_ICON_BACKUP ?= $(AWESOME_USER_DIR)/$(AWESOME_ICON_DIR).bak.$(BACKUP_TAG)
SYSTEM_RC_BACKUP ?= $(AWESOME_SYSTEM_RC).bak.$(BACKUP_TAG)
SYSTEM_MODULE_BACKUP ?= $(AWESOME_SYSTEM_DIR)/$(AWESOME_MODULE_DIR).bak.$(BACKUP_TAG)
SYSTEM_ICON_BACKUP ?= $(AWESOME_SYSTEM_DIR)/$(AWESOME_ICON_DIR).bak.$(BACKUP_TAG)
HOST_SHORT ?= $(shell hostname -s 2>/dev/null)

BUILD_BIN_DIR ?= build/bin
PROGRAMS_BIN ?= $(HOME)/Programs/bin
DOCS_PORT ?= 8080
PRESENTER_CANVAS_PORT ?= 38947
WACOM_OUTPUT ?= HDMI-1
WACOM_STATE_FILE ?= $(HOME)/.cache/linuxutilities/wacom_output_last
REVEAL_URL ?= http://127.0.0.1:8000
CODE_URL ?=
PRESENT_WACOM_MODE ?= none
PRESENT_LIVE_DELAY_SEC ?= 0.35
SHORTS_AUDIO ?=
SHORTS_VIDEO ?=
SHORTS_TRANSCRIPT ?= /tmp/shorts_transcript.json
SHORTS_STYLE ?= config/shorts_style_default.json
SHORTS_OUTPUT ?= /tmp/shorts_output.mp4
SHORTS_MODEL ?= small
SHORTS_LANGUAGE ?= en
SHORTS_RECORD_OUTPUT ?= /tmp/shorts_record.wav
SHORTS_RECORD_DURATION ?= 60
SHORTS_START ?=
SHORTS_DURATION ?=
MANIM_DIR ?= $(HOME)/Workspace/manim
MANIM_VENV ?= $(MANIM_DIR)/.venv/bin/activate
MANIM_FILE ?= smoke.py
MANIM_SCENE ?= Smoke
MANIM_QUALITY ?= ql
SMB_530_HOST ?= 10.0.0.119
SMB_530_SHARE ?= shared
SMB_530_MOUNT ?= /mnt/w530
SMB_530_CREDENTIALS ?= $(HOME)/.smbcredentials-530
AUDIO_OUTPUT_SINK ?=
BT_CARD ?=
BT_DEVICE ?=

-include config/local.mk
ifneq ($(HOST_SHORT),)
-include config/host/$(HOST_SHORT).mk
endif

.PHONY: help rc-backup awesome-backup awesome-update awesome-user-backup awesome-user-update awesome-system-backup awesome-system-update awesome-test test-fast apt-check apt-update deps-check-build deps-check-runtime deps-check build-all build-all-install linuxutils linuxutils-install docs docs-serve present-live present-profile present-profile-live present-profile-list audio-help audio-status audio-bt-mic audio-bt-music audio-bt-recover shorts-help shorts-record shorts-transcribe shorts-render manim-help manim-version manim-smoke manim-scene manim-shell wacom-help wacom wacom-list-outputs wacom-list-devices wacom-status wacom-set-screen wacom-switch wacom-hdmi wacom-external samba-530-probe mount-530 umount-530 desktop-install

help:
>@echo "Targets:"
>@echo "  make rc-backup           Copy active Awesome rc.lua -> $(LOCAL_RC_BACKUP)"
>@echo "  make awesome-backup      Copy $(AWESOME_SYSTEM_RC) -> $(SYSTEM_RC_BACKUP) (uses sudo)"
>@echo "  make awesome-update      Backup + install $(LOCAL_RC) + $(AWESOME_MODULE_DIR)/*.lua + $(AWESOME_ICON_DIR)/ -> $(AWESOME_SYSTEM_RC) (uses sudo)"
>@echo "  make awesome-user-backup Copy user rc.lua -> $(USER_RC_BACKUP)"
>@echo "  make awesome-user-update Backup + install $(LOCAL_RC) + $(AWESOME_MODULE_DIR)/*.lua + $(AWESOME_ICON_DIR)/ -> $(AWESOME_USER_RC)"
>@echo "  make awesome-system-backup Alias for awesome-backup"
>@echo "  make awesome-system-update Alias for awesome-update"
>@echo "  make awesome-test        Run AwesomeWM config validation tests"
>@echo "  make test-fast           Run fast repo checks used by pre-commit"
>@echo "  local overrides: config/local.mk and config/host/$(HOST_SHORT).mk"
>@echo "  make apt-check         Warn if apt metadata looks stale"
>@echo "  make apt-update        Run sudo apt update"
>@echo "  make deps-check-build  Check compile-time deps (fails if missing)"
>@echo "  make deps-check-runtime Check runtime/tool deps (warn-only)"
>@echo "  make deps-check        Run apt-check + build/runtime dependency checks"
>@echo "  make build-all         Build cursor_spotlight, linux_control_center, gtk_dsl_demo"
>@echo "  make build-all-install Build and install binaries to $(PROGRAMS_BIN)"
>@echo "  make linuxutils        Check deps, then build all"
>@echo "  make linuxutils-install Check deps, build all, install to $(PROGRAMS_BIN)"
>@echo "  make docs                Build docs into docs/build"
>@echo "  make docs-serve          Build docs and serve locally on http://localhost:$(DOCS_PORT)"
>@echo "  make present-live        Launch reveal.js + Presenter Canvas (+ optional code tab)"
>@echo "  make present-profile     One-click prep profile (audio + bluetooth + wacom + support apps)"
>@echo "  make present-profile-live Prep profile + launch present-live workflow"
>@echo "  make present-profile-list List profile names from config/presentation_profiles.json"
>@echo "  make audio-help          Show Bluetooth microphone profile usage"
>@echo "  make audio-status        Show Bluetooth card/profile + sink/source defaults"
>@echo "  make audio-bt-mic        Switch BT headset to mic mode (HFP/HSP)"
>@echo "  make audio-bt-music      Switch BT headset back to music mode (A2DP)"
>@echo "  make audio-bt-recover    Recover silent BT headset playback after PipeWire/Bluetooth glitches"
>@echo "  make shorts-help         Show transcript-driven shorts pipeline commands"
>@echo "  make manim-help          Show Manim helper commands (uses $(MANIM_DIR))"
>@echo "  make wacom-help          Show quick Wacom mapping cheatsheet"
>@echo "  make wacom               Map tablet to default HDMI output ($(WACOM_OUTPUT))"
>@echo "  make wacom-list-outputs  List connected display outputs (xrandr)"
>@echo "  make wacom-list-devices  List detected tablet devices (xsetwacom)"
>@echo "  make wacom-status        Show outputs/devices and last mapped output"
>@echo "  make wacom-set-screen OUTPUT=<display>  Map tablet area to one screen"
>@echo "  make wacom-switch        Cycle tablet mapping to the next connected output"
>@echo "  make wacom-hdmi          Shortcut for OUTPUT=HDMI-1"
>@echo "  make wacom-external      Auto-detect first external display and map tablet"
>@echo "  make samba-530-probe     Check host/share reachability and list SMB shares"
>@echo "  make mount-530           Mount //$(SMB_530_HOST)/$(SMB_530_SHARE) -> $(SMB_530_MOUNT)"
>@echo "  make umount-530          Unmount $(SMB_530_MOUNT)"
>@echo "  make desktop-install     Install LinuxUtilities .desktop launchers"

rc-backup:
>@src="$(AWESOME_ACTIVE_RC)"; \
>test -f "$$src" || { echo "Missing Awesome rc.lua: $$src"; exit 1; }; \
>cp "$$src" "$(LOCAL_RC_BACKUP)"; \
>echo "Saved local backup from $$src: $(LOCAL_RC_BACKUP)"; \
>module_src="$$(dirname "$$src")/$(AWESOME_MODULE_DIR)"; \
>if [ -d "$$module_src" ]; then \
>  mkdir -p "$(LOCAL_AWESOME_MODULE_BACKUP)"; \
>  cp -R "$$module_src"/. "$(LOCAL_AWESOME_MODULE_BACKUP)/"; \
>  echo "Saved local module backup from $$module_src: $(LOCAL_AWESOME_MODULE_BACKUP)"; \
>fi; \
>icon_src="$$(dirname "$$src")/$(AWESOME_ICON_DIR)"; \
>if [ -d "$$icon_src" ]; then \
>  mkdir -p "$(LOCAL_AWESOME_ICON_BACKUP)"; \
>  cp -R "$$icon_src"/. "$(LOCAL_AWESOME_ICON_BACKUP)/"; \
>  echo "Saved local icon backup from $$icon_src: $(LOCAL_AWESOME_ICON_BACKUP)"; \
>fi

awesome-backup:
>@test -f "$(AWESOME_SYSTEM_RC)" || { echo "Missing system rc.lua: $(AWESOME_SYSTEM_RC)"; exit 1; }
>@sudo cp "$(AWESOME_SYSTEM_RC)" "$(SYSTEM_RC_BACKUP)"
>@echo "Saved system backup: $(SYSTEM_RC_BACKUP)"
>@if [ -d "$(AWESOME_SYSTEM_DIR)/$(AWESOME_MODULE_DIR)" ]; then \
>  sudo cp -R "$(AWESOME_SYSTEM_DIR)/$(AWESOME_MODULE_DIR)" "$(SYSTEM_MODULE_BACKUP)"; \
>  echo "Saved system module backup: $(SYSTEM_MODULE_BACKUP)"; \
>fi
>@if [ -d "$(AWESOME_SYSTEM_DIR)/$(AWESOME_ICON_DIR)" ]; then \
>  sudo cp -R "$(AWESOME_SYSTEM_DIR)/$(AWESOME_ICON_DIR)" "$(SYSTEM_ICON_BACKUP)"; \
>  echo "Saved system icon backup: $(SYSTEM_ICON_BACKUP)"; \
>fi

awesome-update: rc-backup awesome-backup
>@test -f "$(LOCAL_RC)" || { echo "Missing local rc.lua: $(LOCAL_RC)"; exit 1; }
>@sudo install -m 0644 "$(LOCAL_RC)" "$(AWESOME_SYSTEM_RC)"
>@echo "Installed $(LOCAL_RC) -> $(AWESOME_SYSTEM_RC)"
>@if [ -n "$(strip $(AWESOME_MODULE_FILES))" ]; then \
>  sudo install -d "$(AWESOME_SYSTEM_DIR)/$(AWESOME_MODULE_DIR)"; \
>  sudo install -m 0644 $(AWESOME_MODULE_FILES) "$(AWESOME_SYSTEM_DIR)/$(AWESOME_MODULE_DIR)"; \
>  echo "Installed $(AWESOME_MODULE_DIR)/*.lua -> $(AWESOME_SYSTEM_DIR)/$(AWESOME_MODULE_DIR)"; \
>fi
>@if [ -d "$(AWESOME_ICON_DIR)" ]; then \
>  sudo install -d "$(AWESOME_SYSTEM_DIR)/$(AWESOME_ICON_DIR)"; \
>  sudo cp -R "$(AWESOME_ICON_DIR)"/. "$(AWESOME_SYSTEM_DIR)/$(AWESOME_ICON_DIR)/"; \
>  if command -v gtk-update-icon-cache >/dev/null 2>&1 && [ -d "$(AWESOME_SYSTEM_DIR)/$(AWESOME_ICON_DIR)/LinuxUtilitiesStatus" ]; then \
>    sudo gtk-update-icon-cache -f -t "$(AWESOME_SYSTEM_DIR)/$(AWESOME_ICON_DIR)/LinuxUtilitiesStatus" >/dev/null 2>&1 || true; \
>  fi; \
>  echo "Installed $(AWESOME_ICON_DIR)/ -> $(AWESOME_SYSTEM_DIR)/$(AWESOME_ICON_DIR)"; \
>fi

awesome-user-backup:
>@mkdir -p "$(AWESOME_USER_DIR)"
>@if [ -f "$(AWESOME_USER_RC)" ]; then \
>  cp "$(AWESOME_USER_RC)" "$(USER_RC_BACKUP)"; \
>  echo "Saved user backup: $(USER_RC_BACKUP)"; \
>elif [ -f "$(AWESOME_SYSTEM_RC)" ]; then \
>  cp "$(AWESOME_SYSTEM_RC)" "$(USER_RC_BACKUP)"; \
>  echo "Saved initial backup from system rc: $(USER_RC_BACKUP)"; \
>else \
>  echo "Missing Awesome rc.lua: $(AWESOME_SYSTEM_RC)"; \
>  exit 1; \
>fi
>@if [ -d "$(AWESOME_USER_DIR)/$(AWESOME_MODULE_DIR)" ]; then \
>  cp -R "$(AWESOME_USER_DIR)/$(AWESOME_MODULE_DIR)" "$(USER_MODULE_BACKUP)"; \
>  echo "Saved user module backup: $(USER_MODULE_BACKUP)"; \
>elif [ -d "$(AWESOME_SYSTEM_DIR)/$(AWESOME_MODULE_DIR)" ]; then \
>  cp -R "$(AWESOME_SYSTEM_DIR)/$(AWESOME_MODULE_DIR)" "$(USER_MODULE_BACKUP)"; \
>  echo "Saved initial module backup from system tree: $(USER_MODULE_BACKUP)"; \
>fi
>@if [ -d "$(AWESOME_USER_DIR)/$(AWESOME_ICON_DIR)" ]; then \
>  cp -R "$(AWESOME_USER_DIR)/$(AWESOME_ICON_DIR)" "$(USER_ICON_BACKUP)"; \
>  echo "Saved user icon backup: $(USER_ICON_BACKUP)"; \
>elif [ -d "$(AWESOME_SYSTEM_DIR)/$(AWESOME_ICON_DIR)" ]; then \
>  cp -R "$(AWESOME_SYSTEM_DIR)/$(AWESOME_ICON_DIR)" "$(USER_ICON_BACKUP)"; \
>  echo "Saved initial icon backup from system tree: $(USER_ICON_BACKUP)"; \
>fi

awesome-user-update: rc-backup awesome-user-backup
>@test -f "$(LOCAL_RC)" || { echo "Missing local rc.lua: $(LOCAL_RC)"; exit 1; }
>@mkdir -p "$(AWESOME_USER_DIR)"
>@install -m 0644 "$(LOCAL_RC)" "$(AWESOME_USER_RC)"
>@echo "Installed $(LOCAL_RC) -> $(AWESOME_USER_RC)"
>@if [ -n "$(strip $(AWESOME_MODULE_FILES))" ]; then \
>  install -d "$(AWESOME_USER_DIR)/$(AWESOME_MODULE_DIR)"; \
>  install -m 0644 $(AWESOME_MODULE_FILES) "$(AWESOME_USER_DIR)/$(AWESOME_MODULE_DIR)"; \
>  echo "Installed $(AWESOME_MODULE_DIR)/*.lua -> $(AWESOME_USER_DIR)/$(AWESOME_MODULE_DIR)"; \
>fi
>@if [ -d "$(AWESOME_ICON_DIR)" ]; then \
>  install -d "$(AWESOME_USER_DIR)/$(AWESOME_ICON_DIR)"; \
>  cp -R "$(AWESOME_ICON_DIR)"/. "$(AWESOME_USER_DIR)/$(AWESOME_ICON_DIR)/"; \
>  if command -v gtk-update-icon-cache >/dev/null 2>&1 && [ -d "$(AWESOME_USER_DIR)/$(AWESOME_ICON_DIR)/LinuxUtilitiesStatus" ]; then \
>    gtk-update-icon-cache -f -t "$(AWESOME_USER_DIR)/$(AWESOME_ICON_DIR)/LinuxUtilitiesStatus" >/dev/null 2>&1 || true; \
>  fi; \
>  echo "Installed $(AWESOME_ICON_DIR)/ -> $(AWESOME_USER_DIR)/$(AWESOME_ICON_DIR)"; \
>fi

awesome-system-backup: awesome-backup
>@:

awesome-system-update: awesome-update
>@:

awesome-test:
>@bash tests/awesomewm_config_test.sh

test-fast:
>@bash tests/run_fast.sh

apt-check:
>@set -euo pipefail; \
>stamp="/var/lib/apt/periodic/update-success-stamp"; \
>if [ -f "$$stamp" ]; then \
>  now=$$(date +%s); \
>  updated=$$(stat -c %Y "$$stamp" 2>/dev/null || echo 0); \
>  age_days=$$(( (now - updated) / 86400 )); \
>  if [ "$$age_days" -gt 7 ]; then \
>    echo "WARN: apt metadata is $$age_days day(s) old. Run: make apt-update"; \
>  else \
>    echo "APT metadata age: $$age_days day(s)."; \
>  fi; \
>else \
>  echo "WARN: apt update timestamp not found. Run: make apt-update"; \
>fi

apt-update:
>@sudo apt update

deps-check-build:
>@set -euo pipefail; \
>missing=0; \
>check_cmd() { \
>  local cmd="$$1"; local hint_pkg="$$2"; \
>  if command -v "$$cmd" >/dev/null 2>&1; then \
>    echo "OK   command: $$cmd"; \
>  else \
>    echo "MISS command: $$cmd (install package: $$hint_pkg)"; \
>    missing=1; \
>  fi; \
>}; \
>check_pkg() { \
>  local spec="$$1"; local hint_pkgs="$$2"; \
>  if pkg-config --exists $$spec; then \
>    echo "OK   pkg-config: $$spec"; \
>  else \
>    echo "MISS pkg-config: $$spec (install packages: $$hint_pkgs)"; \
>    missing=1; \
>  fi; \
>}; \
>check_cmd gcc build-essential; \
>check_cmd pkg-config pkg-config; \
>if command -v pkg-config >/dev/null 2>&1; then \
>  check_pkg "x11 xext xfixes xrender cairo" "libx11-dev libxext-dev libxfixes-dev libxrender-dev libcairo2-dev"; \
>  check_pkg "gtk4" "libgtk-4-dev"; \
>fi; \
>if [ "$$missing" -ne 0 ]; then \
>  echo ""; \
>  echo "Build dependencies are missing. Install and retry."; \
>  echo "sudo apt install build-essential pkg-config libx11-dev libxext-dev libxfixes-dev libxrender-dev libcairo2-dev libgtk-4-dev"; \
>  exit 1; \
>fi; \
>echo "Build dependencies look good."

deps-check-runtime:
>@set -euo pipefail; \
>warn=0; \
>checks=( \
>  "gromit-mpx|Presenter drawing overlay (F6)|gromit-mpx" \
>  "xdotool|Presenter dash segment helper|xdotool" \
>  "flameshot|Screenshot utility button|flameshot" \
>  "xrandr|Wacom output detection/mapping|x11-xserver-utils" \
>  "xsetwacom|Wacom tablet mapping|xserver-xorg-input-wacom" \
>  "playerctl|Media play/pause/next/prev keys in AwesomeWM|playerctl" \
>  "rofi|Mod4+r program launcher palette|rofi" \
>  "redshift|Night Light tab|redshift" \
>  "pavucontrol|Audio mixer button|pavucontrol" \
>  "nm-connection-editor|Network button|network-manager-gnome" \
>  "blueman-manager|Bluetooth button|blueman" \
>  "picom|Compositor auto-start for spotlight|picom" \
>  "xvfb-run|Headless GUI smoke test|xvfb" \
>); \
>for item in "$${checks[@]}"; do \
>  IFS='|' read -r cmd feature pkg <<< "$$item"; \
>  if command -v "$$cmd" >/dev/null 2>&1; then \
>    echo "OK   $$cmd ($$feature)"; \
>  else \
>    echo "WARN $$cmd missing -> $$feature may not work (install package: $$pkg)"; \
>    warn=1; \
>  fi; \
>done; \
>if [ "$$warn" -ne 0 ]; then \
>  echo ""; \
>  echo "Runtime checks completed with warnings."; \
>  echo "Build can still proceed, but related features will be unavailable."; \
>else \
>  echo "Runtime checks look good."; \
>fi

deps-check: apt-check deps-check-build deps-check-runtime

build-all:
>@./build_cursor_spotlight.sh
>@./build_linux_control_center.sh
>@./build_gtk_dsl_demo.sh
>@echo "Build complete. Binaries in $(BUILD_BIN_DIR)"

build-all-install:
>@./build_cursor_spotlight.sh --install
>@./build_linux_control_center.sh --install
>@./build_gtk_dsl_demo.sh --install
>@echo "Build + install complete. Binaries installed to $(PROGRAMS_BIN)"

linuxutils: deps-check build-all
>@echo "LinuxUtilities build complete."
>@echo "Tip: run 'make build-all-install' if you want binaries in $(PROGRAMS_BIN)"

linuxutils-install: deps-check build-all-install
>@echo "LinuxUtilities build/install complete."

docs:
>@bash docs/build.sh

docs-serve: docs
>@echo "Serving docs on http://localhost:$(DOCS_PORT)"
>@cd docs/build && python3 -m http.server $(DOCS_PORT)

present-live:
>@REVEAL_URL="$(REVEAL_URL)" \
>CODE_URL="$(CODE_URL)" \
>PRESENT_WACOM_MODE="$(PRESENT_WACOM_MODE)" \
>PRESENT_LIVE_DELAY_SEC="$(PRESENT_LIVE_DELAY_SEC)" \
>PRESENTER_CANVAS_PORT="$(PRESENTER_CANVAS_PORT)" \
>./launch_present_live.sh

present-profile:
>@PRESENT_WACOM_MODE="$(PRESENT_WACOM_MODE)" \
>REVEAL_URL="$(REVEAL_URL)" \
>CODE_URL="$(CODE_URL)" \
>PRESENT_LIVE_DELAY_SEC="$(PRESENT_LIVE_DELAY_SEC)" \
>./scripts/presentation_mode.sh prep

present-profile-live:
>@PRESENT_WACOM_MODE="$(PRESENT_WACOM_MODE)" \
>REVEAL_URL="$(REVEAL_URL)" \
>CODE_URL="$(CODE_URL)" \
>PRESENT_LIVE_DELAY_SEC="$(PRESENT_LIVE_DELAY_SEC)" \
>./scripts/presentation_mode.sh live

present-profile-list:
>@PRESENT_PROFILE_FILE="$(PWD)/config/presentation_profiles.json" ./scripts/presentation_mode.sh list

audio-help:
>@./scripts/audio_bt_profile.sh help

audio-status:
>@./scripts/audio_bt_profile.sh status

audio-bt-mic:
>@AUDIO_OUTPUT_SINK="$(AUDIO_OUTPUT_SINK)" BT_CARD="$(BT_CARD)" ./scripts/audio_bt_profile.sh mic-mode

audio-bt-music:
>@BT_CARD="$(BT_CARD)" ./scripts/audio_bt_profile.sh music-mode

audio-bt-recover:
>@BT_CARD="$(BT_CARD)" BT_DEVICE="$(BT_DEVICE)" ./scripts/audio_bt_profile.sh recover

shorts-help:
>@echo "Shorts pipeline:"
>@echo "  make shorts-record SHORTS_RECORD_OUTPUT=/tmp/voice.wav SHORTS_RECORD_DURATION=60"
>@echo "  make shorts-transcribe SHORTS_AUDIO=/tmp/voice.wav SHORTS_TRANSCRIPT=/tmp/voice.json"
>@echo "  make shorts-render SHORTS_VIDEO=input.mp4 SHORTS_TRANSCRIPT=/tmp/voice.json SHORTS_STYLE=$(SHORTS_STYLE) SHORTS_OUTPUT=/tmp/short.mp4"
>@echo "Options:"
>@echo "  SHORTS_MODEL=$(SHORTS_MODEL) SHORTS_LANGUAGE=$(SHORTS_LANGUAGE)"
>@echo "  SHORTS_START=<seconds> SHORTS_DURATION=<seconds> (for clip window override)"

shorts-record:
>@if [ -z "$(SHORTS_RECORD_OUTPUT)" ]; then echo "Set SHORTS_RECORD_OUTPUT=<path.wav>"; exit 1; fi
>@if ! command -v ffmpeg >/dev/null 2>&1; then echo "Missing ffmpeg"; exit 1; fi
>@echo "Recording microphone to $(SHORTS_RECORD_OUTPUT) for $(SHORTS_RECORD_DURATION)s..."
>@ffmpeg -y -f pulse -i default -t "$(SHORTS_RECORD_DURATION)" -ac 1 -ar 16000 "$(SHORTS_RECORD_OUTPUT)"

shorts-transcribe:
>@if [ -z "$(SHORTS_AUDIO)" ]; then echo "Set SHORTS_AUDIO=<path audio/video>"; exit 1; fi
>@python3 scripts/shorts_studio.py transcribe \
>  --audio "$(SHORTS_AUDIO)" \
>  --output "$(SHORTS_TRANSCRIPT)" \
>  --model "$(SHORTS_MODEL)" \
>  --language "$(SHORTS_LANGUAGE)"

shorts-render:
>@if [ -z "$(SHORTS_VIDEO)" ]; then echo "Set SHORTS_VIDEO=<input.mp4>"; exit 1; fi
>@if [ -z "$(SHORTS_TRANSCRIPT)" ]; then echo "Set SHORTS_TRANSCRIPT=<transcript.json>"; exit 1; fi
>@if [ -z "$(SHORTS_STYLE)" ]; then echo "Set SHORTS_STYLE=<style.json>"; exit 1; fi
>@if [ -z "$(SHORTS_OUTPUT)" ]; then echo "Set SHORTS_OUTPUT=<output.mp4>"; exit 1; fi
>@set -euo pipefail; \
>args=(); \
>if [ -n "$(SHORTS_START)" ]; then args+=(--start "$(SHORTS_START)"); fi; \
>if [ -n "$(SHORTS_DURATION)" ]; then args+=(--duration "$(SHORTS_DURATION)"); fi; \
>python3 scripts/shorts_studio.py render \
>  --video "$(SHORTS_VIDEO)" \
>  --transcript "$(SHORTS_TRANSCRIPT)" \
>  --style "$(SHORTS_STYLE)" \
>  --output "$(SHORTS_OUTPUT)" \
>  "$${args[@]}"

manim-help:
>@echo "Manim helpers (workspace: $(MANIM_DIR))"
>@echo "  make manim-version"
>@echo "  make manim-smoke"
>@echo "  make manim-scene MANIM_FILE=smoke.py MANIM_SCENE=Smoke MANIM_QUALITY=ql"
>@echo "  make manim-shell"
>@echo "Env overrides:"
>@echo "  MANIM_DIR, MANIM_VENV, MANIM_FILE, MANIM_SCENE, MANIM_QUALITY"

manim-version:
>@MANIM_DIR="$(MANIM_DIR)" MANIM_VENV="$(MANIM_VENV)" ./manim_tools.sh version

manim-smoke:
>@MANIM_DIR="$(MANIM_DIR)" \
>  MANIM_VENV="$(MANIM_VENV)" \
>  MANIM_FILE=smoke.py \
>  MANIM_SCENE=Smoke \
>  MANIM_QUALITY="$(MANIM_QUALITY)" \
>  ./manim_tools.sh smoke

manim-scene:
>@MANIM_DIR="$(MANIM_DIR)" \
>  MANIM_VENV="$(MANIM_VENV)" \
>  MANIM_FILE="$(MANIM_FILE)" \
>  MANIM_SCENE="$(MANIM_SCENE)" \
>  MANIM_QUALITY="$(MANIM_QUALITY)" \
>  ./manim_tools.sh scene

manim-shell:
>@MANIM_DIR="$(MANIM_DIR)" MANIM_VENV="$(MANIM_VENV)" ./manim_tools.sh term-shell

wacom-help:
>@echo "Wacom mapping quick commands:"
>@echo "  make wacom                          (default: $(WACOM_OUTPUT))"
>@echo "  make wacom-list-outputs"
>@echo "  make wacom-list-devices"
>@echo "  make wacom-status"
>@echo "  make wacom-set-screen OUTPUT=HDMI-1"
>@echo "Shortcuts:"
>@echo "  make wacom-switch"
>@echo "  make wacom-hdmi"
>@echo "  make wacom-external"

wacom:
>@$(MAKE) --no-print-directory wacom-set-screen OUTPUT="$(WACOM_OUTPUT)"

wacom-list-outputs:
>@if ! command -v xrandr >/dev/null 2>&1; then echo "Missing xrandr (install package: x11-xserver-utils)"; exit 1; fi
>@xrandr --query | awk '/ connected/{print $$1}'

wacom-list-devices:
>@if ! command -v xsetwacom >/dev/null 2>&1; then echo "Missing xsetwacom (install package: xserver-xorg-input-wacom)"; exit 1; fi
>@xsetwacom --list devices

wacom-status:
>@echo "Wacom status:"
>@echo "  default output: $(WACOM_OUTPUT)"
>@echo "  state file: $(WACOM_STATE_FILE)"
>@if [ -f "$(WACOM_STATE_FILE)" ]; then \
>  last=$$(head -n 1 "$(WACOM_STATE_FILE)" | awk 'NF {print $$1; exit}'); \
>  echo "  last mapped output: $${last:-unknown}"; \
>else \
>  echo "  last mapped output: (none saved yet)"; \
>fi
>@echo "Connected outputs:"
>@if ! command -v xrandr >/dev/null 2>&1; then \
>  echo "  xrandr missing (install package: x11-xserver-utils)"; \
>elif xrandr --query >/dev/null 2>&1; then \
>  xrandr --query | awk '/ connected/{print "  - " $$1}'; \
>else \
>  echo "  xrandr query failed (run in active X11 desktop session)."; \
>fi
>@echo "Tablet devices:"
>@if ! command -v xsetwacom >/dev/null 2>&1; then \
>  echo "  xsetwacom missing (install package: xserver-xorg-input-wacom)"; \
>elif xsetwacom --list devices >/dev/null 2>&1; then \
>  xsetwacom --list devices | sed 's/^/  - /'; \
>else \
>  echo "  xsetwacom query failed (check device/driver/session)."; \
>fi

wacom-set-screen:
>@if [ -z "$(OUTPUT)" ]; then \
>  echo "Usage: make wacom-set-screen OUTPUT=<display>"; \
>  echo "Example: make wacom-set-screen OUTPUT=HDMI-1"; \
>  echo "Tip: use 'make wacom-list-outputs' to find output names."; \
>  exit 1; \
>fi
>@if [ -z "$$DISPLAY" ]; then echo "DISPLAY is not set. Run this in your desktop X11 session."; exit 1; fi
>@if ! command -v xsetwacom >/dev/null 2>&1; then echo "Missing xsetwacom (install package: xserver-xorg-input-wacom)"; exit 1; fi
>@if ! command -v xrandr >/dev/null 2>&1; then echo "Missing xrandr (install package: x11-xserver-utils)"; exit 1; fi
>@if ! xrandr --query | awk '/ connected/{print $$1}' | grep -Fx "$(OUTPUT)" >/dev/null 2>&1; then \
>  echo "Output not found: $(OUTPUT)"; \
>  echo "Available outputs:"; \
>  xrandr --query | awk '/ connected/{print "  - " $$1}'; \
>  exit 1; \
>fi
>@set -euo pipefail; \
>mapped=0; \
>while IFS= read -r device; do \
>  [ -z "$$device" ] && continue; \
>  echo "Mapping $$device -> $(OUTPUT)"; \
>  xsetwacom --set "$$device" MapToOutput "$(OUTPUT)"; \
>  mapped=$$((mapped + 1)); \
>done < <(xsetwacom --list devices | sed -n 's/^\(.*[^[:space:]]\)[[:space:]]\+id:[[:space:]]*[0-9][0-9]*[[:space:]]\+type:[[:space:]]*\(STYLUS\|ERASER\|TOUCH\)[[:space:]]*$$/\1/p'); \
>if [ "$$mapped" -eq 0 ]; then \
>  echo "No Wacom STYLUS/ERASER/TOUCH devices found."; \
>  echo "Check with: make wacom-list-devices"; \
>  exit 1; \
>fi; \
>state_file="$(WACOM_STATE_FILE)"; \
>mkdir -p "$$(dirname "$$state_file")" 2>/dev/null || true; \
>printf '%s\n' "$(OUTPUT)" > "$$state_file" 2>/dev/null || true; \
>echo "Mapped $$mapped Wacom device(s) to $(OUTPUT)."; \
>echo "Tip: run 'make wacom-help' for shortcuts."

wacom-switch:
>@if [ -z "$$DISPLAY" ]; then echo "DISPLAY is not set. Run this in your desktop X11 session."; exit 1; fi
>@if ! command -v xsetwacom >/dev/null 2>&1; then echo "Missing xsetwacom (install package: xserver-xorg-input-wacom)"; exit 1; fi
>@if ! command -v xrandr >/dev/null 2>&1; then echo "Missing xrandr (install package: x11-xserver-utils)"; exit 1; fi
>@set -euo pipefail; \
>mapfile -t outputs < <(xrandr --query | awk '/ connected/{print $$1}'); \
>if [ "$${#outputs[@]}" -eq 0 ]; then \
>  echo "No connected displays found."; \
>  exit 1; \
>fi; \
>if [ "$${#outputs[@]}" -eq 1 ]; then \
>  echo "Only one connected output ($${outputs[0]}). Nothing to switch."; \
>  $(MAKE) --no-print-directory wacom-set-screen OUTPUT="$${outputs[0]}"; \
>  exit 0; \
>fi; \
>mapfile -t devices < <(xsetwacom --list devices | sed -n 's/^\(.*[^[:space:]]\)[[:space:]]\+id:[[:space:]]*[0-9][0-9]*[[:space:]]\+type:[[:space:]]*\(STYLUS\|ERASER\|TOUCH\)[[:space:]]*$$/\1/p'); \
>if [ "$${#devices[@]}" -eq 0 ]; then \
>  echo "No Wacom STYLUS/ERASER/TOUCH devices found."; \
>  echo "Check with: make wacom-list-devices"; \
>  exit 1; \
>fi; \
>state_file="$(WACOM_STATE_FILE)"; \
>current=$$(xsetwacom --get "$${devices[0]}" MapToOutput 2>/dev/null | awk 'NF {print $$1; exit}' || true); \
>if [ -z "$$current" ] && [ -f "$$state_file" ]; then \
>  current=$$(head -n 1 "$$state_file" | awk 'NF {print $$1; exit}' || true); \
>fi; \
>target=""; \
>if [ -n "$$current" ]; then \
>  for i in "$${!outputs[@]}"; do \
>    if [ "$${outputs[$$i]}" = "$$current" ]; then \
>      target="$${outputs[$$(( ($$i + 1) % $${#outputs[@]} ))]}"; \
>      break; \
>    fi; \
>  done; \
>fi; \
>if [ -z "$$target" ]; then \
>  if printf '%s\n' "$${outputs[@]}" | grep -Fx "$(WACOM_OUTPUT)" >/dev/null 2>&1; then \
>    target="$(WACOM_OUTPUT)"; \
>  else \
>    target="$${outputs[0]}"; \
>  fi; \
>fi; \
>echo "Switching Wacom mapping: $${current:-unknown} -> $$target"; \
>$(MAKE) --no-print-directory wacom-set-screen OUTPUT="$$target"

wacom-hdmi: wacom

wacom-external:
>@if ! command -v xrandr >/dev/null 2>&1; then echo "Missing xrandr (install package: x11-xserver-utils)"; exit 1; fi
>@if [ -z "$$DISPLAY" ]; then echo "DISPLAY is not set. Run this in your desktop X11 session."; exit 1; fi
>@set -euo pipefail; \
>outputs=$$(xrandr --query | awk '/ connected/{print $$1}'); \
>output=$$(printf '%s\n' "$$outputs" | awk '/^HDMI/{print; exit}'); \
>if [ -z "$$output" ]; then \
>  output=$$(printf '%s\n' "$$outputs" | awk '!/^(eDP|LVDS|DSI)-/{print; exit}'); \
>fi; \
>if [ -z "$$output" ]; then \
>  echo "No external display detected. Use: make wacom-set-screen OUTPUT=<display>"; \
>  echo "Available outputs:"; \
>  printf '%s\n' "$$outputs" | awk 'NF {print "  - " $$1}'; \
>  exit 1; \
>fi; \
>echo "Detected external output: $$output"; \
>$(MAKE) --no-print-directory wacom-set-screen OUTPUT="$$output"

samba-530-probe:
>@SMB_530_HOST="$(SMB_530_HOST)" \
>SMB_530_SHARE="$(SMB_530_SHARE)" \
>SMB_530_MOUNT="$(SMB_530_MOUNT)" \
>SMB_530_CREDENTIALS="$(SMB_530_CREDENTIALS)" \
>./mount_530.sh --probe

mount-530:
>@SMB_530_HOST="$(SMB_530_HOST)" \
>SMB_530_SHARE="$(SMB_530_SHARE)" \
>SMB_530_MOUNT="$(SMB_530_MOUNT)" \
>SMB_530_CREDENTIALS="$(SMB_530_CREDENTIALS)" \
>./mount_530.sh

umount-530:
>@SMB_530_MOUNT="$(SMB_530_MOUNT)" ./umount_530.sh

desktop-install:
>@./scripts/install_desktop_entries.sh
