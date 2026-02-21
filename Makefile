SHELL := /bin/bash
.RECIPEPREFIX := >

AWESOME_DIR ?= /etc/xdg/awesome
AWESOME_RC ?= $(AWESOME_DIR)/rc.lua
LOCAL_RC ?= rc.lua
LOCAL_RC_BACKUP ?= rc.dupe.lua
BACKUP_TAG := $(shell date +%Y%m%d-%H%M%S)
SYSTEM_RC_BACKUP ?= $(AWESOME_RC).bak.$(BACKUP_TAG)

BUILD_BIN_DIR ?= build/bin
PROGRAMS_BIN ?= $(HOME)/Programs/bin
DOCS_PORT ?= 8080
WACOM_OUTPUT ?= HDMI-1
WACOM_STATE_FILE ?= $(HOME)/.cache/linuxutilities/wacom_output_last

.PHONY: help rc-backup awesome-backup awesome-update apt-check apt-update deps-check-build deps-check-runtime deps-check build-all build-all-install linuxutils linuxutils-install docs docs-serve wacom-help wacom wacom-list-outputs wacom-list-devices wacom-status wacom-set-screen wacom-switch wacom-hdmi wacom-external

help:
>@echo "Targets:"
>@echo "  make rc-backup         Copy $(AWESOME_RC) -> $(LOCAL_RC_BACKUP)"
>@echo "  make awesome-backup    Copy $(AWESOME_RC) -> $(SYSTEM_RC_BACKUP) (uses sudo)"
>@echo "  make awesome-update    Backup + install $(LOCAL_RC) -> $(AWESOME_RC) (uses sudo)"
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
>@echo "  make wacom-help          Show quick Wacom mapping cheatsheet"
>@echo "  make wacom               Map tablet to default HDMI output ($(WACOM_OUTPUT))"
>@echo "  make wacom-list-outputs  List connected display outputs (xrandr)"
>@echo "  make wacom-list-devices  List detected tablet devices (xsetwacom)"
>@echo "  make wacom-status        Show outputs/devices and last mapped output"
>@echo "  make wacom-set-screen OUTPUT=<display>  Map tablet area to one screen"
>@echo "  make wacom-switch        Cycle tablet mapping to the next connected output"
>@echo "  make wacom-hdmi          Shortcut for OUTPUT=HDMI-1"
>@echo "  make wacom-external      Auto-detect first external display and map tablet"

rc-backup:
>@test -f "$(AWESOME_RC)" || { echo "Missing system rc.lua: $(AWESOME_RC)"; exit 1; }
>@cp "$(AWESOME_RC)" "$(LOCAL_RC_BACKUP)"
>@echo "Saved local backup: $(LOCAL_RC_BACKUP)"

awesome-backup:
>@test -f "$(AWESOME_RC)" || { echo "Missing system rc.lua: $(AWESOME_RC)"; exit 1; }
>@sudo cp "$(AWESOME_RC)" "$(SYSTEM_RC_BACKUP)"
>@echo "Saved system backup: $(SYSTEM_RC_BACKUP)"

awesome-update: rc-backup awesome-backup
>@test -f "$(LOCAL_RC)" || { echo "Missing local rc.lua: $(LOCAL_RC)"; exit 1; }
>@sudo install -m 0644 "$(LOCAL_RC)" "$(AWESOME_RC)"
>@echo "Installed $(LOCAL_RC) -> $(AWESOME_RC)"

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
>  check_pkg "gtk+-3.0" "libgtk-3-dev"; \
>fi; \
>if [ "$$missing" -ne 0 ]; then \
>  echo ""; \
>  echo "Build dependencies are missing. Install and retry."; \
>  echo "sudo apt install build-essential pkg-config libx11-dev libxext-dev libxfixes-dev libxrender-dev libcairo2-dev libgtk-3-dev"; \
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
