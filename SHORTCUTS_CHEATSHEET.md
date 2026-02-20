# Linux Utilities Shortcut Cheat Sheet

Reference for the current setup in this repo (`rc.lua`, `workspace_shortcuts.sh`, `workspace_browse.sh`, `presenter_dash.sh`).

## Key Legend

- `Mod4` = Super/Windows key
- `Mod1` = Alt key
- `Ctrl` = Control key
- `Shift` = Shift key

## Presenter / Recording Shortcuts

### Gromit-MPX (draw over any app)

| Shortcut | Action |
| --- | --- |
| `F6` | Toggle draw mode on/off |
| `Shift+F6` | Clear all strokes |
| `Ctrl+F6` | Undo stroke |
| `Ctrl+Shift+F6` | Redo stroke |
| `Alt+F6` | Toggle overlay visibility |
| `Ctrl+Alt+F6` | Quit gromit overlay |

Tips:
- If mouse is stuck drawing, press `F6` to leave draw mode.
- `Esc` does not quit gromit. Use `Ctrl+Alt+F6` (or `gromit-mpx -q`).

Install profile once (adds shape tools and modifier map):

```bash
./install_gromit_profile.sh
```

Draw-mode shape/modifier map (from `config/gromit-mpx.cfg`):

| Hold while dragging | Tool |
| --- | --- |
| none | Cyan pen |
| `Shift` | Yellow marker |
| `Ctrl` | Straight line |
| `Ctrl+Shift` | Arrow line |
| `Alt` | Rectangle |
| `Alt+Shift` | Circle |
| `Alt+Ctrl` | Filled circle |
| `Mouse Button2` | Smooth path |
| `Mouse Button2 + Shift` | Orthogonal path |
| `Mouse Button3` | Eraser |

### Presenter Dash (animated flow lines/arrows)

| Shortcut | Action |
| --- | --- |
| `Alt+F11` | Set anchor at current cursor |
| `F11` | Draw animated dashed segment (anchor -> cursor) |
| `Shift+F11` | Draw animated dotted segment |
| `Ctrl+F11` | Draw animated solid segment |
| `Mod4+F11` | Draw animated arrow segment |
| `Ctrl+Alt+F11` | Reset anchor |

CLI helper:

```bash
./presenter_dash.sh anchor
./presenter_dash.sh dash
./presenter_dash.sh dot
./presenter_dash.sh solid
./presenter_dash.sh arrow
./presenter_dash.sh clear
./presenter_dash.sh undo
./presenter_dash.sh redo
./presenter_dash.sh reset
```

Optional tuning:
- `PRESENTER_DASH_COLOR` (default `#00d4ff`)
- `PRESENTER_DASH_THICKNESS` (default `6`)
- `PRESENTER_DASH_FRAME_SLEEP` (default `0.012`)

Dependencies:
- `gromit-mpx`
- `xdotool`

### Cursor Spotlight

| Shortcut | Action |
| --- | --- |
| `F7` | Toggle spotlight |
| `F9` | Less dim |
| `F10` | More dim |
| `Shift+F9` | Smaller radius |
| `Shift+F10` | Larger radius |
| `Mod4+g` | Fallback toggle |
| `Mod4+[ / Mod4+]` | Fallback dim down/up |
| `Mod4+- / Mod4+=` | Fallback radius down/up |
| `Ctrl+Alt+g` | Alternate fallback toggle |
| `Ctrl+Alt+[ / Ctrl+Alt+]` | Alternate fallback dim down/up |
| `Ctrl+Alt+- / Ctrl+Alt+=` | Alternate fallback radius down/up |

Spotlight process:
- `Esc` exits spotlight.

### Screenshot Capture

| Shortcut | Action |
| --- | --- |
| `F8` | Open Flameshot capture |
| `Print` | Open Flameshot capture |

## Mouse Shortcuts (AwesomeWM)

### Desktop/root

| Input | Action |
| --- | --- |
| `Right click` | Open main menu |
| `Wheel up/down` | Next/previous tag |
| `Button 8` | Flameshot capture |
| `Button 9` | Open Linux Control Center (client-aware cwd) |
| `Mod4 + Middle click` | Toggle mute |
| `Mod4 + Wheel up/down` | Volume up/down |

### Client window

| Input | Action |
| --- | --- |
| `Left click` | Focus/raise client |
| `Button 8` | Flameshot capture |
| `Button 9` | Open Linux Control Center for that client |
| `Mod4 + Left drag` | Move window |
| `Mod4 + Right drag` | Resize window |

### Wibar widgets

| Widget input | Action |
| --- | --- |
| `Audio left click` | Open `pavucontrol` |
| `Audio right click` | Toggle mute |
| `Audio wheel` | Volume up/down |
| `Battery left click` | Open power manager |
| `Battery right click` | Refresh battery status |
| `Network left click` | Open network manager |
| `Network right click` | Refresh network status |
| `Bluetooth left click` | Open bluetooth manager |
| `Bluetooth right click` | Refresh bluetooth status |
| `Mail left click` | Open Thunderbird |
| `Mail right click` | Open Thunderbird calendar |
| `Clock left click` | Open calendar app |
| `Clock right click` | Open time preferences |

## Core AwesomeWM Keyboard Shortcuts

### General

| Shortcut | Action |
| --- | --- |
| `Mod4+s` | Show hotkeys help |
| `Mod4+Return` | Open terminal |
| `Mod4+w` | Show main menu |
| `Mod4+p` | Show menubar |
| `Mod4+r` | Run prompt |
| `Mod4+x` | Lua execute prompt |
| `Mod4+Ctrl+r` | Reload awesome |
| `Mod4+Shift+q` | Quit awesome |

### Tags/workspaces

| Shortcut | Action |
| --- | --- |
| `Mod4+Left / Mod4+Right` | Previous/next tag |
| `Mod4+Escape` | Restore previous tag |
| `Mod4+1..9` | View tag 1..9 |
| `Mod4+Ctrl+1..9` | Toggle tag display |
| `Mod4+Shift+1..9` | Move focused client to tag |
| `Mod4+Ctrl+Shift+1..9` | Toggle focused client on tag |

### Client focus and movement

| Shortcut | Action |
| --- | --- |
| `Mod4+j / Mod4+k` | Focus next/previous client |
| `Mod4+Shift+j / Mod4+Shift+k` | Swap with next/previous client |
| `Mod4+Ctrl+j / Mod4+Ctrl+k` | Focus next/previous screen |
| `Mod4+u` | Jump to urgent client |
| `Mod4+Tab` | Switch back to previous client |
| `Mod4+Ctrl+n` | Restore minimized client |

### Layout

| Shortcut | Action |
| --- | --- |
| `Mod4+h / Mod4+l` | Decrease/increase master width |
| `Mod4+Shift+h / Mod4+Shift+l` | Increase/decrease master clients |
| `Mod4+Ctrl+h / Mod4+Ctrl+l` | Increase/decrease columns |
| `Mod4+Space / Mod4+Shift+Space` | Next/previous layout |

### Focused client state

| Shortcut | Action |
| --- | --- |
| `Mod4+f` | Toggle fullscreen |
| `Mod4+Shift+c` | Close client |
| `Mod4+Ctrl+Space` | Toggle floating |
| `Mod4+Ctrl+Return` | Move client to master |
| `Mod4+o` | Move client to another screen |
| `Mod4+t` | Toggle keep on top |
| `Mod4+n` | Minimize client |
| `Mod4+m` | Maximize/unmaximize |
| `Mod4+Ctrl+m` | Maximize vertically |
| `Mod4+Shift+m` | Maximize horizontally |

## Media Keys

| Key | Action |
| --- | --- |
| `XF86AudioRaiseVolume` | Volume up |
| `XF86AudioLowerVolume` | Volume down |
| `XF86AudioMute` | Toggle mute |

## Workspace Shell Shortcuts

Install once:

```bash
./install_workspace_shortcuts.sh
source ~/.bashrc
```

Navigation commands:

| Command | Target |
| --- | --- |
| `ws` | `/home/antshiv/Workspace` |
| `aero` | `/home/antshiv/Workspace/AeroDynControlRig` |
| `aml` | `/home/antshiv/Workspace/AttitudeMathLib` |
| `ant` | `/home/antshiv/Workspace/antsand.com` |
| `ck` | `/home/antshiv/Workspace/C-Kernel-Engine` |
| `ct` | `/home/antshiv/Workspace/C-Transformer` |
| `dls` | `/home/antshiv/Workspace/DronelinuxSystem` |
| `dtr` | `/home/antshiv/Workspace/DroneTestRig` |
| `genai` | `/home/antshiv/Workspace/Generative-AI-with-LLMs` |
| `lu` | `/home/antshiv/Workspace/LinuxUtilities` |
| `sn` | `/home/antshiv/Workspace/ShivasNotes` |

Helper commands:
- `ws_help` -> print all shortcuts
- `ws_status` -> status summary
- `browse` -> interactive workspace browser
- `project_find <pattern>` -> quick directory search

`browse` controls:

| Key | Action |
| --- | --- |
| `Up / Down` | Move selection |
| `Enter` | Open directory/item |
| `Left / Backspace` | Go back one level |
| `h` or `?` | Help/details |
| `q` | Quit |

## Linux Control Center Utilities Tab

Buttons available:
- Pavucontrol
- CC Switch
- Flameshot
- Network
- Bluetooth
- Workspace
- Screenshots
- Terminator
- Cursor Spotlight
- Build Spotlight
- Gromit Draw
- Gromit Clear
- Dash Anchor
- Dash Segment
- Dot Segment
- Arrow Segment
- Install Gromit Profile
- Shortcut Cheat Sheet
