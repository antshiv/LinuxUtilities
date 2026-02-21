# Linux Utilities

![Linux Terminal](assets/linuxterminal.png)

This directory contains a collection of utility scripts designed to enhance my workflow on Linux, particularly with AwesomeWM and various AI CLI tools. The primary objective is to streamline daily tasks, automate repetitive actions, and facilitate a more efficient and seamless integration of these technologies.

## My Setup

![My awesomeWM setup](assets/awesomeWMTerminal.png)

My current setup is simple and focused on efficiency:

*   **OS:** Linux Mint
*   **Window Manager:** awesomeWM
*   **Terminal:** Terminator
*   **Tooling:** AI CLI tools

## Scripts:

- `import_screenshots.sh`: A utility to manage screenshots and phone photos, moving them to designated directories and generating prompts for AI CLI tools.
- `rc.lua`: (AwesomeWM configuration file) - This file likely contains custom configurations for the Awesome Window Manager, tailoring its behavior and appearance to my specific needs.
- `redshift.sh`: A script likely related to Redshift, a program that adjusts the color temperature of the screen according to your surroundings, reducing eye strain.
- `remove_duplicate_assets.sh`: A script to identify and remove duplicate files, helping to keep the system clean and organized.
- `rescue-windows.sh`: A script potentially used for recovery or specific interactions with a Windows environment, possibly in a dual-boot or virtualized setup.
- `cursor_spotlight.c`: Lightweight X11 cursor spotlight overlay utility.
- `build_cursor_spotlight.sh`: Build helper for `cursor_spotlight`.

## Cursor Spotlight Utility

Build/run:

```bash
sudo apt install pkg-config libx11-dev libxext-dev libxfixes-dev libxrender-dev libcairo2-dev
./build_cursor_spotlight.sh
./build/bin/cursor_spotlight --radius 180 --dim 0.68 --fps 50
# optional install target for PATH-based launches
./build_cursor_spotlight.sh --install
# then run as:
~/Programs/bin/cursor_spotlight --radius 180 --dim 0.68 --fps 50
```

Controls:

- Press `Esc` to exit spotlight.
- AwesomeWM hotkeys in `rc.lua`:
  - `F7`: toggle spotlight
  - `F9` / `F10`: less/more dim
  - `Shift+F9` / `Shift+F10`: smaller/larger radius
  - `Mod4+g` and `Ctrl+Alt+g` fallback toggles

The `Utilities` tab now includes:
- `Cursor Spotlight` (toggle)
- `Build Spotlight` (compile helper)
- `Presenter Canvas` (live whiteboard with shape/icon tools + JSON save/load)
- `Storyboard DSL` (timeline parser/player for scripted animations)
- `Teleprompter` (local script reader window)

## Presenter Drawing (Epic Pen Style)

Install:

```bash
sudo apt install gromit-mpx xdotool
./install_gromit_profile.sh
```

AwesomeWM hotkeys in `rc.lua`:

- `F6`: toggle drawing mode
- `Shift+F6`: clear all strokes
- `Ctrl+F6`: undo stroke
- `Ctrl+Shift+F6`: redo stroke
- `Alt+F6`: toggle overlay visibility
- `Ctrl+Alt+F6`: quit overlay
- `Alt+F11`: set presenter-dash anchor at current cursor
- `F11`: draw animated dashed segment from anchor to cursor (anchor advances)
- `Shift+F11`: draw animated dotted segment
- `Ctrl+F11`: draw animated solid segment
- `Mod4+F11`: draw animated arrow segment
- `Ctrl+Alt+F11`: reset presenter-dash anchor

`Utilities` tab helpers:
- `Gromit Draw` (toggle draw mode)
- `Gromit Clear` (clear strokes)
- `Dash Anchor`, `Dash Segment`, `Dot Segment`, `Arrow Segment` (real-time flow overlays)
- `Install Gromit Profile` (installs `config/gromit-mpx.cfg` to `~/.config/gromit-mpx.cfg`)
- `Presenter Canvas` (opens `presenter_canvas.html` via `launch_presenter_canvas.sh`)
- `Storyboard DSL` (opens `presenter_storyboard.html` via `launch_presenter_storyboard.sh`)
- `Teleprompter` (opens `teleprompter.html` via `launch_teleprompter.sh`)
- `Shortcut Cheat Sheet` (open complete keyboard/mouse/shell mapping)

Gromit draw tools in draw mode (compat profile, default):
- hold `Shift`: marker
- hold `Ctrl`: arrow pen
- hold `Alt`: red pen
- hold middle button (`Button2`): fine pen
- hold right button (`Button3`): eraser

Advanced shape profile (rect/circle/smooth/orthogonal) is available for newer
Gromit builds:

```bash
GROMIT_PROFILE_MODE=advanced ./install_gromit_profile.sh
```

Presenter dash helper script:
- `presenter_dash.sh` supports: `anchor`, `dash`, `dot`, `solid`, `arrow`, `clear`, `undo`, `redo`, `reset`

## Teleprompter (ATEM Friendly)

Launch:

```bash
./launch_teleprompter.sh
```

Core controls inside the teleprompter:
- `Space`: play/pause scroll
- `R`: reset to top
- `F`: focus mode
- `M`: mirror mode (for glass teleprompter rigs)
- `Up` / `Down`: speed adjust
- `[` / `]`: font size adjust

ATEM dual-display setup:
- Keep teleprompter on the laptop panel.
- Put browser/terminal windows on the HDMI output that ATEM captures.
- Use extended display mode, not mirrored display mode.

## Presenter Canvas (Live Whiteboard)

Launch:

```bash
./launch_presenter_canvas.sh
```

Core controls inside the canvas:
- `1..9` and `0`: switch tools (select, pen, line, arrow, rect, ellipse, text, icon, eraser, pan)
- `Ctrl+S` / `Ctrl+O`: save/load canvas JSON
- `Ctrl+Shift+S`: export DSL starter JSON for storyboard timeline player
- `Ctrl+Z` / `Ctrl+Y`: undo/redo
- `Delete`: delete selected shape
- `F` / `H`: focus mode and show/hide controls
- Mouse wheel: zoom at cursor
- Middle mouse drag or `0` pan tool: move camera

For Wacom on X11, map stylus to your recording output (example HDMI):

```bash
xsetwacom set "Wacom Intuos S 2 Pen stylus" MapToOutput HDMI-1
```

## Storyboard DSL Player (Timeline + Parser)

Launch:

```bash
./launch_presenter_storyboard.sh
```

Workflow:
1. Draw scene in Presenter Canvas.
2. Export with `Ctrl+Shift+S` (or `Export DSL Starter` button).
3. Open Storyboard DSL Player and load/edit the generated DSL.
4. Press play and record voiceover.

Built-in presets for your core content:
- `CKE Train Preset`: data -> tokenizer/batch -> forward -> loss -> backprop -> optimizer -> checkpoint
- `CKE Infer Preset`: prompt -> embed/KV -> CKE kernels -> sampler -> next-token loop

Core controls inside the storyboard player:
- `Space`: play/pause
- `R`: reset timeline to 0s
- `Ctrl+Enter`: validate DSL + play
- `Ctrl+S`: save DSL JSON
- `F` / `H`: focus mode / toggle side panel
- `.` / `,`: playback speed up/down

Supported timeline actions:
- `show`, `hide`, `draw`, `move`, `pulse`, `zoom`, `pan`
- targets can be exact id, `*`, or selectors like `type:edge`, `type:icon`
- plus `morph` for scene transition (`from` + `to` ids)

## Shortcut Cheat Sheet

For the full list of shortcuts (AwesomeWM keys, gromit/presenter controls, mouse side buttons, widget clicks, shell navigation, and interactive browse controls), use:

- File: `SHORTCUTS_CHEATSHEET.md`
- Utility tab button: `Shortcut Cheat Sheet`

## Desktop GUI Control Center

- `linux_control_center.c`: GTK desktop app with:
  - Night Light tab (manual/auto Redshift controls)
  - Shortcuts tab (presenter workflow + grouped key/mouse/shell cheat sheet)
  - Screenshots tab (thumbnail browser, path/prompt copy, open folder)
- `build_linux_control_center.sh`: Build helper for the GTK app.

Build/run:

```bash
sudo apt install libgtk-3-dev pkg-config
./build_linux_control_center.sh
./build/bin/linux_control_center
# optional install target for AwesomeWM/global launchers
./build_linux_control_center.sh --install
~/Programs/bin/linux_control_center
```

## GTK DSL Starter (Box/Grid Mapping)

- `gtk_dsl_demo.c`: A small declarative GTK runtime that reads a `.gdsl` file and maps:
  - `type=box` -> `GtkBox`
  - `type=grid` -> `GtkGrid`
  - `type=label|button|switch|scale|separator` -> matching GTK widgets
- `dsl/workbench.gdsl`: Example layout file you can edit for your own launcher/panels.
- `build_gtk_dsl_demo.sh`: Build helper.

Build/run:

```bash
./build_gtk_dsl_demo.sh
./build/bin/gtk_dsl_demo dsl/workbench.gdsl
# optional install target
./build_gtk_dsl_demo.sh --install
~/Programs/bin/gtk_dsl_demo dsl/workbench.gdsl
```

DSL quick notes:

- Parenting/layout:
  - `parent=<id>` to attach under another widget.
  - Inside `box`: use `expand`, `fill`, `padding`.
  - Inside `grid`: use `left`, `top`, `width`, `height`.
- Styling and sizing:
  - `class=<css_class>`
  - `margin`, `margin_top`, `margin_bottom`, `margin_start`, `margin_end`
  - `hexpand`, `vexpand`, `halign`, `valign`
- Actions:
  - For buttons: `on_click=<shell command>`
  - For switches: `on_toggle_on=<cmd>`, `on_toggle_off=<cmd>`
