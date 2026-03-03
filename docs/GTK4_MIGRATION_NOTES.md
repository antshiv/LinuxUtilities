# GTK4 Migration Notes

## Current State (2026-02-24)

- `linux_control_center.c` is now GTK4-only.
- `build_linux_control_center.sh` builds with `pkg-config gtk4`.
- GTK3 fallback code was removed to simplify maintenance.
- `gtk_dsl_demo.c` also builds on GTK4.

## Install (Ubuntu/Mint)

```bash
sudo apt update
sudo apt install -y build-essential pkg-config libgtk-4-dev libadwaita-1-dev
```

Quick check:

```bash
pkg-config --modversion gtk4
pkg-config --modversion libadwaita-1
```

## Control Center Scope After Cutover

Current GTK4 app focuses on screenshot workflow:

- Thumbnail browser with search + multi-select
- Open folder / delete selected
- Annotation canvas tools: Select, Arrow, Line, Rect, Callout, Text, Stamp
- Right dock properties for arrow/callout/rect/text/stamp style controls
- Quick style ribbon backed by SVG assets (`assets/svg/...`) including legacy pointer/link presets
- Editor toggles: `Dock Left/Right`, `Hide Styles`, `Hide Thumbs`, `Fit 100%`
- Save/export: annotated PNG + SVG copy
- Event-controller input system (click, motion, scroll, key)
- Keyboard quick tools: `1` Select, `2` Arrow, `3` Rect, `4` Callout, `5` Line, `6` Text, `7` Stamp
- Step sidebar includes auto-step and link-step modes (linked mode adds connecting arrows between step stamps)
- GTK4-native selection widgets now use `GtkDropDown` (`GtkComboBoxText` deprecations removed)

Legacy reference source preserved:

- `legacy/linux_control_center_gtk3_legacy_snapshot.c`
  - frozen snapshot from before GTK4-only cutover
  - use for feature parity porting (right dock properties, callouts, style presets, etc.)

## Next Parity Work

1. Finish screenshot style parity polish (remaining visual/details parity in quick-style tiles and per-tool property groups).
2. Replace `GtkFlowBox` browser with model-driven `GtkGridView` for very large screenshot sets.
3. Expand undo/redo into full edit-history stack (not just undo-last annotation).
