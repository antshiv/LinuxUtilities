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
./cursor_spotlight --radius 180 --dim 0.68 --fps 50
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
- `Shortcut Cheat Sheet` (open complete keyboard/mouse/shell mapping)

Gromit shape tools in draw mode (profile-based):
- hold `Shift`: marker
- hold `Ctrl`: straight line
- hold `Ctrl+Shift`: arrow line
- hold `Alt`: rectangle
- hold `Alt+Shift`: circle
- hold `Alt+Ctrl`: filled circle

Presenter dash helper script:
- `presenter_dash.sh` supports: `anchor`, `dash`, `dot`, `solid`, `arrow`, `clear`, `undo`, `redo`, `reset`

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
./linux_control_center
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
./gtk_dsl_demo dsl/workbench.gdsl
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
