# Microphone Help (Bluetooth + HDMI)

## Why this feels broken

Bluetooth headsets use two different profiles:

- `A2DP`: high quality playback, **no headset mic**
- `HFP/HSP` (Headset Head Unit): headset mic enabled, lower playback quality

So if you enable Bluetooth mic mode, speaker audio sounding "phone-like" is expected.

## Fast commands

From `~/Workspace/LinuxUtilities`:

```bash
# Show detected Bluetooth card/profile + default sink/source
make audio-status

# Enable headset mic profile (HFP/HSP)
make audio-bt-mic

# Return to high-quality playback profile (A2DP)
make audio-bt-music

# Show usage help
make audio-help
```

## Keep HDMI output while using Bluetooth mic

```bash
# find HDMI sink name
pactl list short sinks

# force output sink while enabling BT mic mode
AUDIO_OUTPUT_SINK=alsa_output.pci-0000_00_1f.3.hdmi-stereo make audio-bt-mic
```

## GUI flow (pavucontrol)

1. Open `pavucontrol`
2. `Configuration` tab:
   - Switch headset to `Headset Head Unit (HSP/HFP)` for mic mode
   - Switch headset to `A2DP` for music mode
3. `Input Devices` tab:
   - Set Bluetooth input as fallback for mic mode
4. `Output Devices` tab:
   - Set HDMI as fallback if you want speakers there

## Notes

- Use `headset-head-unit-msbc` when available for better call quality.
- On some headsets, mic mode still sounds narrowband.
- If no Bluetooth card appears, reconnect headset and run:

```bash
./scripts/bluetooth_refresh.sh refresh
make audio-status
```
