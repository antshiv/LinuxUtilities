# Legacy GTK3 Reference

This folder keeps a frozen pre-migration snapshot for parity porting.

- `linux_control_center_gtk3_legacy_snapshot.c`
  - Restored from commit parent `a3f25ce^`
  - Original GTK3-heavy implementation (tabs, screenshot studio dock, tool presets, property panels)

## Why it exists

The active app is now GTK4, but this snapshot is preserved so features can be ported one-by-one without guessing old behavior.

## Useful diffs

```bash
# Compare current GTK4 file against legacy snapshot
meld linux_control_center.c legacy/linux_control_center_gtk3_legacy_snapshot.c

# Fast search for old screenshot/editor features
rg -n "shots_editor|quick styles|callout|step|text|dock|icon view" legacy/linux_control_center_gtk3_legacy_snapshot.c
```
