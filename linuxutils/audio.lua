local M = {}

local function set_widget_text(widget_list, text)
    for _, widget in ipairs(widget_list) do
        widget:set_text(text)
    end
end

function M.new(opts)
    local awful = assert(opts and opts.awful, "audio.new requires awful")
    local gears = assert(opts and opts.gears, "audio.new requires gears")
    local naughty = assert(opts and opts.naughty, "audio.new requires naughty")
    local command_exists = assert(opts and opts.command_exists, "audio.new requires command_exists")
    local compact_sink_name = assert(opts and opts.compact_sink_name, "audio.new requires compact_sink_name")
    local volume_step = opts.volume_step or "5%"

    local audio_notification_id = nil
    local audio_state = { volume = "?", mute = "unknown", sink = "unknown" }
    local audio_status_widgets = {}
    local playerctl_available = command_exists("playerctl")
    local playerctl_warned = false
    local controller = {}

    local function set_audio_widget_text(text)
        set_widget_text(audio_status_widgets, text)
    end

    local function show_audio_notification()
        local volume_label
        if audio_state.mute == "yes" then
            volume_label = "Muted (" .. audio_state.volume .. "%)"
        else
            volume_label = "Volume " .. audio_state.volume .. "%"
        end

        local n = naughty.notify({
            title = "Audio",
            text = volume_label .. "\nOutput: " .. audio_state.sink,
            timeout = 1.4,
            replaces_id = audio_notification_id,
        })
        if n and n.id then
            audio_notification_id = n.id
        end
    end

    function controller.refresh(show_notification)
        awful.spawn.easy_async_with_shell([[
            vol=$(pactl get-sink-volume @DEFAULT_SINK@ 2>/dev/null | awk 'NR==1{for(i=1;i<=NF;i++) if($i ~ /%/){gsub("%","",$i); print $i; exit}}')
            mute=$(pactl get-sink-mute @DEFAULT_SINK@ 2>/dev/null | awk '{print $2}')
            sink=$(pactl info 2>/dev/null | sed -n 's/^Default Sink: //p')
            [ -z "$vol" ] && vol="?"
            [ -z "$mute" ] && mute="unknown"
            [ -z "$sink" ] && sink="unknown"
            printf 'VOLUME=%s\nMUTE=%s\nSINK=%s\n' "$vol" "$mute" "$sink"
        ]], function(stdout)
            audio_state.volume = stdout:match("VOLUME=([^\n]+)") or "?"
            audio_state.mute = stdout:match("MUTE=([^\n]+)") or "unknown"
            audio_state.sink = stdout:match("SINK=([^\n]+)") or "unknown"

            if audio_state.mute == "yes" then
                set_audio_widget_text(" 🔇 " .. audio_state.volume .. "% ")
            else
                set_audio_widget_text(" 🔊 " .. audio_state.volume .. "% ")
            end

            if show_notification then
                show_audio_notification()
            end
        end)
    end

    function controller.refresh_later(show_notification)
        gears.timer.start_new(0.15, function()
            controller.refresh(show_notification)
            return false
        end)
    end

    function controller.volume_up()
        awful.spawn({ "pactl", "set-sink-volume", "@DEFAULT_SINK@", "+" .. volume_step }, false)
        controller.refresh_later(true)
    end

    function controller.volume_down()
        awful.spawn({ "pactl", "set-sink-volume", "@DEFAULT_SINK@", "-" .. volume_step }, false)
        controller.refresh_later(true)
    end

    function controller.volume_toggle_mute()
        awful.spawn({ "pactl", "set-sink-mute", "@DEFAULT_SINK@", "toggle" }, false)
        controller.refresh_later(true)
    end

    local function run_playerctl(action)
        if not playerctl_available then
            if not playerctl_warned then
                naughty.notify({
                    title = "Media keys",
                    text = "playerctl not found (install package: playerctl).",
                    timeout = 2.5
                })
                playerctl_warned = true
            end
            return
        end
        awful.spawn({ "playerctl", action }, false)
    end

    function controller.media_play_pause()
        run_playerctl("play-pause")
    end

    function controller.media_next_track()
        run_playerctl("next")
    end

    function controller.media_prev_track()
        run_playerctl("previous")
    end

    function controller.open_pavucontrol()
        awful.spawn({ "pavucontrol" }, false)
    end

    function controller.attach_widget(widget)
        table.insert(audio_status_widgets, widget)
    end

    function controller.tooltip_text()
        local volume_label = audio_state.volume .. "%"
        if audio_state.mute == "yes" then
            volume_label = "muted (" .. volume_label .. ")"
        end
        return "Audio\nVolume: " .. volume_label .. "\nOutput: " .. compact_sink_name(audio_state.sink) .. "\nLeft click: pavucontrol"
    end

    function controller.enable_auto_switch()
        awful.spawn.with_shell([[
            for _ in 1 2 3 4 5; do
                pactl info >/dev/null 2>&1 && break
                sleep 1
            done
            pactl list short modules 2>/dev/null | grep -q "module-switch-on-port-available" \
                || pactl load-module module-switch-on-port-available >/dev/null 2>&1 || true
            pactl list short modules 2>/dev/null | grep -q "module-switch-on-connect" \
                || pactl load-module module-switch-on-connect ignore_virtual=no >/dev/null 2>&1 || true
        ]])
    end

    return controller
end

return M
