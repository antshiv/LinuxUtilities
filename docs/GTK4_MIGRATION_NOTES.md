# GTK4 Migration Notes

## Current State (2026-02-23)

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
- Annotation canvas (Select, Arrow, Rect)
- Event-controller input system (click, motion, scroll, key)

## Next Parity Work

1. Re-introduce legacy utility panels (Night Light, shortcuts, audio) as GTK4 pages.
2. Add save/export for annotations.
3. Replace `GtkFlowBox` browser with model-driven `GtkGridView` for very large screenshot sets.
4. Add undo/redo stack for annotation edits.
