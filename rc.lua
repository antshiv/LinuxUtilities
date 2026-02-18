-- If LuaRocks is installed, make sure that packages installed through it are
-- found (e.g. lgi). If LuaRocks is not installed, do nothing.
pcall(require, "luarocks.loader")

-- Standard awesome library
local gears = require("gears")
local awful = require("awful")
require("awful.autofocus")
-- Widget and layout library
local wibox = require("wibox")
-- Theme handling library
local beautiful = require("beautiful")
-- Notification library
local naughty = require("naughty")
local menubar = require("menubar")
local hotkeys_popup = require("awful.hotkeys_popup")
-- Enable hotkeys help widget for VIM and other apps
-- when client with a matching name is opened:
require("awful.hotkeys_popup.keys")

-- Load Debian menu entries
local debian = require("debian.menu")
local has_fdo, freedesktop = pcall(require, "freedesktop")

-- {{{ Error handling
-- Check if awesome encountered an error during startup and fell back to
-- another config (This code will only ever execute for the fallback config)
if awesome.startup_errors then
    naughty.notify({ preset = naughty.config.presets.critical,
                     title = "Oops, there were errors during startup!",
                     text = awesome.startup_errors })
end

-- Handle runtime errors after startup
do
    local in_error = false
    awesome.connect_signal("debug::error", function (err)
        -- Make sure we don't go into an endless error loop
        if in_error then return end
        in_error = true

        naughty.notify({ preset = naughty.config.presets.critical,
                         title = "Oops, an error happened!",
                         text = tostring(err) })
        in_error = false
    end)
end
-- }}}

-- {{{ Variable definitions
-- Themes define colours, icons, font and wallpapers.
beautiful.init(gears.filesystem.get_themes_dir() .. "default/theme.lua")

-- This is used later as the default terminal and editor to run.
terminal = "x-terminal-emulator"
editor = os.getenv("EDITOR") or "editor"
editor_cmd = terminal .. " -e " .. editor

-- Default modkey.
-- Usually, Mod4 is the key with a logo between Control and Alt.
-- If you do not like this or do not have such a key,
-- I suggest you to remap Mod4 to another key using xmodmap or other tools.
-- However, you can use another modifier like Mod1, but it may interact with others.
modkey = "Mod4"

local function get_short_hostname()
    local hostname_pipe = io.popen("hostname -s 2>/dev/null")
    local host = nil
    if hostname_pipe then
        host = hostname_pipe:read("*l")
        hostname_pipe:close()
    end
    if not host or host == "" then
        host = os.getenv("HOSTNAME") or "linux"
    end
    return host
end

local identity_label = (os.getenv("USER") or "user") .. "@" .. get_short_hostname()

local volume_step = "5%"
-- MX Master 3S side buttons are commonly 8/9 on X11.
-- Confirm on your machine with: xev -event button
local flameshot_mouse_button = 8
local utility_mouse_button = 9
local flameshot_lock = false
local audio_notification_id = nil
local audio_state = { volume = "?", mute = "unknown", sink = "unknown" }
local battery_state = { status = "unknown", percent = "?", time = "" }
local network_state = { state = "unknown", ntype = "unknown", label = "offline" }
local bluetooth_state = { powered = "unknown", connected = "0" }
local audio_status_widgets = {}
local battery_status_widgets = {}
local network_status_widgets = {}
local bluetooth_status_widgets = {}

local function compact_sink_name(sink)
    if not sink or sink == "" then
        return "unknown"
    end

    local compact = sink
    compact = compact:gsub("^alsa_output%.", "")
    compact = compact:gsub("^bluez_output%.", "bt:")
    if #compact > 18 then
        compact = compact:sub(1, 18) .. "..."
    end
    return compact
end

local function shorten_label(label, max_len)
    if not label or label == "" then
        return "unknown"
    end
    if #label <= max_len then
        return label
    end
    return label:sub(1, max_len) .. "..."
end

local function set_widget_text(widget_list, text)
    for _, widget in ipairs(widget_list) do
        widget:set_text(text)
    end
end

local function set_audio_widget_text(text)
    set_widget_text(audio_status_widgets, text)
end

local function set_battery_widget_text(text)
    set_widget_text(battery_status_widgets, text)
end

local function set_network_widget_text(text)
    set_widget_text(network_status_widgets, text)
end

local function set_bluetooth_widget_text(text)
    set_widget_text(bluetooth_status_widgets, text)
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

local function refresh_audio_state(show_notification)
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

local function refresh_battery_state()
    awful.spawn.easy_async_with_shell([[
        line=$(acpi -b 2>/dev/null | head -n1)
        if [ -z "$line" ]; then
            printf 'STATUS=unknown\nPERCENT=?\nTIME=\n'
            exit 0
        fi
        status=$(printf '%s' "$line" | awk -F', ' '{print $1}' | sed 's/.*: //')
        percent=$(printf '%s' "$line" | awk -F', ' '{print $2}' | tr -d '%')
        timeleft=$(printf '%s' "$line" | awk -F', ' '{print $3}')
        [ -z "$status" ] && status="unknown"
        [ -z "$percent" ] && percent="?"
        printf 'STATUS=%s\nPERCENT=%s\nTIME=%s\n' "$status" "$percent" "$timeleft"
    ]], function(stdout)
        battery_state.status = stdout:match("STATUS=([^\n]+)") or "unknown"
        battery_state.percent = stdout:match("PERCENT=([^\n]+)") or "?"
        battery_state.time = stdout:match("TIME=([^\n]*)") or ""

        local pct = tonumber(battery_state.percent)
        local icon = "🔋"
        if battery_state.status == "Charging" then
            icon = "⚡"
        elseif battery_state.status == "Full" then
            icon = "🔌"
        elseif pct and pct <= 20 then
            icon = "🪫"
        end

        set_battery_widget_text(" " .. icon .. " " .. tostring(battery_state.percent) .. "% ")
    end)
end

local function refresh_network_state()
    awful.spawn.easy_async_with_shell([[
        state=$(nmcli -t -f STATE general 2>/dev/null | head -n1)
        wifi=$(nmcli -t -f active,ssid dev wifi 2>/dev/null | sed -n 's/^yes://p' | head -n1)
        eth=$(nmcli -t -f DEVICE,TYPE,STATE dev status 2>/dev/null | awk -F: '$2=="ethernet" && $3=="connected" {print $1; exit}')

        [ -z "$state" ] && state="unknown"
        ntype="offline"
        label="offline"
        if [ "$state" = "connected" ]; then
            if [ -n "$wifi" ]; then
                ntype="wifi"
                label="$wifi"
            elif [ -n "$eth" ]; then
                ntype="ethernet"
                label="$eth"
            else
                ntype="connected"
                label="online"
            fi
        fi
        printf 'STATE=%s\nTYPE=%s\nLABEL=%s\n' "$state" "$ntype" "$label"
    ]], function(stdout)
        network_state.state = stdout:match("STATE=([^\n]+)") or "unknown"
        network_state.ntype = stdout:match("TYPE=([^\n]+)") or "unknown"
        network_state.label = stdout:match("LABEL=([^\n]+)") or "offline"

        local icon = "⛔"
        if network_state.ntype == "wifi" then
            icon = "📶"
        elseif network_state.ntype == "ethernet" then
            icon = "🖧"
        elseif network_state.ntype == "connected" then
            icon = "🌐"
        end
        local label_short = shorten_label(network_state.label, 12)
        set_network_widget_text(" " .. icon .. " " .. label_short .. " ")
    end)
end

local function refresh_bluetooth_state()
    awful.spawn.easy_async_with_shell([[
        powered=$(bluetoothctl show 2>/dev/null | awk '/Powered:/ {print $2; exit}')
        connected=$(bluetoothctl devices Connected 2>/dev/null | wc -l | tr -d ' ')
        [ -z "$powered" ] && powered="unknown"
        [ -z "$connected" ] && connected="0"
        printf 'POWERED=%s\nCONNECTED=%s\n' "$powered" "$connected"
    ]], function(stdout)
        bluetooth_state.powered = stdout:match("POWERED=([^\n]+)") or "unknown"
        bluetooth_state.connected = stdout:match("CONNECTED=([^\n]+)") or "0"

        local connected = tonumber(bluetooth_state.connected) or 0
        if bluetooth_state.powered == "yes" then
            if connected > 0 then
                set_bluetooth_widget_text(" 🔵 BT " .. tostring(connected) .. " ")
            else
                set_bluetooth_widget_text(" 🔵 BT ")
            end
        else
            set_bluetooth_widget_text(" ⚪ BT ")
        end
    end)
end

local function refresh_system_statuses()
    refresh_battery_state()
    refresh_network_state()
    refresh_bluetooth_state()
end

local function refresh_audio_state_later(show_notification)
    gears.timer.start_new(0.15, function()
        refresh_audio_state(show_notification)
        return false
    end)
end

local function volume_up()
    awful.spawn({ "pactl", "set-sink-volume", "@DEFAULT_SINK@", "+" .. volume_step }, false)
    refresh_audio_state_later(true)
end

local function volume_down()
    awful.spawn({ "pactl", "set-sink-volume", "@DEFAULT_SINK@", "-" .. volume_step }, false)
    refresh_audio_state_later(true)
end

local function volume_toggle_mute()
    awful.spawn({ "pactl", "set-sink-mute", "@DEFAULT_SINK@", "toggle" }, false)
    refresh_audio_state_later(true)
end

local function build_flameshot_target_file()
    local home = os.getenv("HOME") or ""
    local shots_dir = home ~= "" and (home .. "/Screenshots") or "Screenshots"
    local stamp = os.date("%Y-%m-%d_%H-%M-%S")
    return shots_dir, shots_dir .. "/Screenshot-" .. stamp .. ".png"
end

local function open_flameshot()
    if flameshot_lock then
        return
    end
    flameshot_lock = true
    local shots_dir, target_file = build_flameshot_target_file()
    awful.spawn.easy_async({ "mkdir", "-p", shots_dir }, function()
        awful.spawn({ "flameshot", "gui", "--path", target_file }, false)
    end)
    gears.timer.start_new(0.4, function()
        flameshot_lock = false
        return false
    end)
end

local function open_pavucontrol()
    awful.spawn({ "pavucontrol" }, false)
end

local function shell_quote(value)
    if not value or value == "" then
        return "''"
    end
    return "'" .. tostring(value):gsub("'", "'\\''") .. "'"
end

local function expand_tilde_path(path)
    if not path or path == "" then
        return nil
    end
    if path == "~" then
        return os.getenv("HOME")
    end
    if path:sub(1, 2) == "~/" then
        return (os.getenv("HOME") or "") .. path:sub(2)
    end
    return path
end

local function extract_dir_from_client_title(c)
    local title = (c and c.name) or ""
    local candidate = nil
    local cleaned = title

    if title == "" then
        return nil
    end

    -- Common terminal suffixes.
    cleaned = cleaned:gsub("%s+%- Terminator$", "")
    cleaned = cleaned:gsub("%s+%- GNU/Linux$", "")

    -- Typical title format: user@host: ~/Workspace/Project
    candidate = cleaned:match(":%s+(~[^%s]*)$")
    if not candidate then
        candidate = cleaned:match(":%s+(/[^%s]*)$")
    end
    if not candidate then
        return nil
    end

    candidate = expand_tilde_path(candidate)
    if not candidate or candidate == "" then
        return nil
    end

    local probe = io.popen("[ -d " .. shell_quote(candidate) .. " ] && printf ok || true")
    local ok = probe and probe:read("*a") or ""
    if probe then
        probe:close()
    end
    if ok == "ok" then
        return candidate
    end
    return nil
end

local function open_linux_control_center(target_dir)
    local arg = ""
    if target_dir and target_dir ~= "" then
        arg = " " .. shell_quote(target_dir)
    end

    awful.spawn.easy_async_with_shell(string.format([[
        app="$HOME/Workspace/LinuxUtilities/linux_control_center"
        if [ -x "$app" ]; then
            "$app"%s >/dev/null 2>&1 &
        else
            exit 1
        fi
    ]], arg), function(_, _, _, exit_code)
        if exit_code ~= 0 then
            naughty.notify({
                preset = naughty.config.presets.warn,
                title = "Control Center Missing",
                text = "Build LinuxUtilities/linux_control_center first.",
            })
        end
    end)
end

local function notify_linux_control_center_pwd(text)
    naughty.notify({
        title = "Linux Utility Path",
        text = text,
        timeout = 3,
    })
end

local function open_linux_control_center_for_client(c)
    local title_dir = extract_dir_from_client_title(c)

    if title_dir then
        notify_linux_control_center_pwd("PWD (title): " .. title_dir)
        open_linux_control_center(title_dir)
        return
    end

    if not c or not c.pid then
        notify_linux_control_center_pwd("PWD: fallback to LinuxUtilities (no client PID)")
        open_linux_control_center()
        return
    end

    awful.spawn.easy_async_with_shell(string.format([[
        root_pid=%d
        base_cwd="$(readlink -f /proc/$root_pid/cwd 2>/dev/null || true)"
        best_cwd="$base_cwd"
        best_score=0

        score_cwd() {
            local path="$1"
            if [ -z "$path" ] || [ ! -d "$path" ]; then
                echo 0
                return
            fi
            if [ "$path" = "$HOME" ]; then
                echo 1
                return
            fi
            echo "${#path}"
        }

        if [ -n "$base_cwd" ] && [ -d "$base_cwd" ]; then
            best_score="$(score_cwd "$base_cwd")"
        fi

        queue="$root_pid"
        depth=0
        while [ -n "$queue" ] && [ "$depth" -lt 4 ]; do
            next_queue=""
            for pid in $queue; do
                for child in $(pgrep -P "$pid" 2>/dev/null); do
                    next_queue="$next_queue $child"
                    child_cwd="$(readlink -f /proc/$child/cwd 2>/dev/null || true)"
                    score="$(score_cwd "$child_cwd")"
                    if [ "$score" -gt "$best_score" ]; then
                        best_score="$score"
                        best_cwd="$child_cwd"
                    fi
                done
            done
            queue="$next_queue"
            depth=$((depth + 1))
        done

        if [ -n "$best_cwd" ] && [ -d "$best_cwd" ]; then
            printf '%%s' "$best_cwd"
        fi
    ]], c.pid), function(stdout)
        local cwd = ""
        if stdout and stdout ~= "" then
            cwd = stdout:gsub("%s+$", "")
        end
        if cwd ~= "" then
            notify_linux_control_center_pwd("PWD: " .. cwd)
            open_linux_control_center(cwd)
        else
            notify_linux_control_center_pwd("PWD: fallback to LinuxUtilities (could not resolve /proc/" .. c.pid .. "/cwd)")
            open_linux_control_center()
        end
    end)
end

local function open_network_manager()
    awful.spawn.easy_async_with_shell([[
        if command -v nm-connection-editor >/dev/null 2>&1; then
            nm-connection-editor >/dev/null 2>&1 &
        elif command -v nmtui >/dev/null 2>&1; then
            x-terminal-emulator -e nmtui >/dev/null 2>&1 &
        else
            exit 1
        fi
    ]], function(_, _, _, exit_code)
        if exit_code ~= 0 then
            naughty.notify({
                preset = naughty.config.presets.warn,
                title = "Network Utility Missing",
                text = "Install `nm-connection-editor` or `nmtui`.",
            })
        end
    end)
end

local function open_bluetooth_manager()
    awful.spawn.easy_async_with_shell([[
        if command -v blueman-manager >/dev/null 2>&1; then
            blueman-manager >/dev/null 2>&1 &
        elif command -v blueberry >/dev/null 2>&1; then
            blueberry >/dev/null 2>&1 &
        elif command -v bluetoothctl >/dev/null 2>&1; then
            x-terminal-emulator -e bluetoothctl >/dev/null 2>&1 &
        else
            exit 1
        fi
    ]], function(_, _, _, exit_code)
        if exit_code ~= 0 then
            naughty.notify({
                preset = naughty.config.presets.warn,
                title = "Bluetooth Utility Missing",
                text = "Install `blueman-manager` or a Bluetooth utility.",
            })
        end
    end)
end

local function open_power_manager()
    awful.spawn.easy_async_with_shell([[
        if command -v xfce4-power-manager-settings >/dev/null 2>&1; then
            xfce4-power-manager-settings >/dev/null 2>&1 &
        elif command -v gnome-power-statistics >/dev/null 2>&1; then
            gnome-power-statistics >/dev/null 2>&1 &
        elif command -v upower >/dev/null 2>&1; then
            x-terminal-emulator -e sh -lc 'upower -d | less' >/dev/null 2>&1 &
        else
            exit 1
        fi
    ]], function(_, _, _, exit_code)
        if exit_code ~= 0 then
            naughty.notify({
                preset = naughty.config.presets.warn,
                title = "Power Utility Missing",
                text = "Install a power utility (xfce4-power-manager, gnome-power-statistics).",
            })
        end
    end)
end

local function open_calendar_app()
    awful.spawn.easy_async_with_shell([[
        if command -v gnome-calendar >/dev/null 2>&1; then
            gnome-calendar >/dev/null 2>&1 &
        elif command -v thunderbird >/dev/null 2>&1; then
            thunderbird -calendar >/dev/null 2>&1 &
        elif command -v orage >/dev/null 2>&1; then
            orage >/dev/null 2>&1 &
        else
            x-terminal-emulator -e sh -lc 'cal -3; echo; date; echo; read -n1 -rsp "Press any key to close..."' >/dev/null 2>&1 &
        fi
    ]])
end

local function open_time_preferences()
    awful.spawn.easy_async_with_shell([[
        if command -v gnome-control-center >/dev/null 2>&1; then
            gnome-control-center datetime >/dev/null 2>&1 &
        elif command -v xfce4-datetime-settings >/dev/null 2>&1; then
            xfce4-datetime-settings >/dev/null 2>&1 &
        elif command -v timedatectl >/dev/null 2>&1; then
            x-terminal-emulator -e sh -lc 'timedatectl status; echo; read -n1 -rsp "Press any key to close..."' >/dev/null 2>&1 &
        else
            exit 1
        fi
    ]], function(_, _, _, exit_code)
        if exit_code ~= 0 then
            naughty.notify({
                preset = naughty.config.presets.warn,
                title = "Date/Time Utility Missing",
                text = "Install calendar/time tools such as gnome-calendar or xfce4-datetime-settings.",
            })
        end
    end)
end

local function open_thunderbird()
    awful.spawn.easy_async_with_shell([[
        if command -v thunderbird >/dev/null 2>&1; then
            thunderbird >/dev/null 2>&1 &
        else
            exit 1
        fi
    ]], function(_, _, _, exit_code)
        if exit_code ~= 0 then
            naughty.notify({
                preset = naughty.config.presets.warn,
                title = "Thunderbird Missing",
                text = "Install Thunderbird to use this launcher icon.",
            })
        end
    end)
end

local function open_thunderbird_calendar()
    awful.spawn({ "thunderbird", "-calendar" }, false)
end

local function create_thunderbird_widget()
    local icon_candidates = {
        "/usr/share/icons/Mint-X/apps/24/thunderbird.png",
        "/usr/share/icons/hicolor/24x24/apps/thunderbird.png",
        "/usr/share/pixmaps/thunderbird.png",
    }

    local icon_path = nil
    for _, candidate in ipairs(icon_candidates) do
        if gears.filesystem.file_readable(candidate) then
            icon_path = candidate
            break
        end
    end

    local base_widget
    if icon_path then
        base_widget = wibox.widget {
            image = icon_path,
            resize = true,
            forced_width = 16,
            forced_height = 16,
            widget = wibox.widget.imagebox,
        }
    else
        base_widget = wibox.widget.textbox(" 📧 ")
    end

    local widget = wibox.widget {
        base_widget,
        left = 3,
        right = 3,
        widget = wibox.container.margin,
    }

    widget:buttons(gears.table.join(
        awful.button({ }, 1, open_thunderbird),
        awful.button({ }, 3, open_thunderbird_calendar)
    ))

    awful.tooltip({
        objects = { widget },
        text = "Thunderbird\nLeft click: mail\nRight click: calendar",
    })

    return widget
end

local function create_audio_widget()
    local widget = wibox.widget.textbox()
    widget:set_text(" 🔊 -- ")
    widget:buttons(gears.table.join(
        awful.button({ }, 1, open_pavucontrol),
        awful.button({ }, 3, volume_toggle_mute),
        awful.button({ }, 4, volume_up),
        awful.button({ }, 5, volume_down)
    ))
    awful.tooltip({
        objects = { widget },
        timer_function = function()
            local volume_label = audio_state.volume .. "%"
            if audio_state.mute == "yes" then
                volume_label = "muted (" .. volume_label .. ")"
            end
            return "Audio\nVolume: " .. volume_label .. "\nOutput: " .. compact_sink_name(audio_state.sink) .. "\nLeft click: pavucontrol"
        end,
    })
    table.insert(audio_status_widgets, widget)
    refresh_audio_state(false)
    return widget
end

local function create_battery_widget()
    local widget = wibox.widget.textbox()
    widget:set_text(" 🔋 -- ")
    widget:buttons(gears.table.join(
        awful.button({ }, 1, open_power_manager),
        awful.button({ }, 3, refresh_battery_state)
    ))
    awful.tooltip({
        objects = { widget },
        timer_function = function()
            local detail = battery_state.status .. ", " .. battery_state.percent .. "%"
            if battery_state.time and battery_state.time ~= "" then
                detail = detail .. "\nTime: " .. battery_state.time
            end
            return "Battery\n" .. detail .. "\nLeft click: power settings"
        end,
    })
    table.insert(battery_status_widgets, widget)
    refresh_battery_state()
    return widget
end

local function create_network_widget()
    local widget = wibox.widget.textbox()
    widget:set_text(" 🌐 -- ")
    widget:buttons(gears.table.join(
        awful.button({ }, 1, open_network_manager),
        awful.button({ }, 3, refresh_network_state)
    ))
    awful.tooltip({
        objects = { widget },
        timer_function = function()
            return "Network\nState: " .. network_state.state .. "\nType: " .. network_state.ntype .. "\nLink: " .. network_state.label .. "\nLeft click: network settings"
        end,
    })
    table.insert(network_status_widgets, widget)
    refresh_network_state()
    return widget
end

local function create_bluetooth_widget()
    local widget = wibox.widget.textbox()
    widget:set_text(" ⚪ BT ")
    widget:buttons(gears.table.join(
        awful.button({ }, 1, open_bluetooth_manager),
        awful.button({ }, 3, refresh_bluetooth_state)
    ))
    awful.tooltip({
        objects = { widget },
        timer_function = function()
            return "Bluetooth\nPowered: " .. bluetooth_state.powered .. "\nConnected devices: " .. tostring(bluetooth_state.connected) .. "\nLeft click: Bluetooth manager"
        end,
    })
    table.insert(bluetooth_status_widgets, widget)
    refresh_bluetooth_state()
    return widget
end

local function create_clock_widget()
    local widget = wibox.widget.textclock(" 🕒 %a %d %b %H:%M ")
    widget:buttons(gears.table.join(
        awful.button({ }, 1, open_calendar_app),
        awful.button({ }, 3, open_time_preferences)
    ))
    local calendar = awful.widget.calendar_popup.month()
    calendar:attach(widget, "tr", { on_hover = true })
    widget._calendar = calendar
    awful.tooltip({
        objects = { widget },
        timer_function = function()
            return os.date("%A, %d %B %Y\n%H:%M:%S") .. "\nLeft click: calendar app\nRight click: date/time settings"
        end,
    })
    return widget
end

local function enable_audio_auto_switch()
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

local function autostart_desktop_applets()
    awful.spawn.with_shell([[
        pgrep -u "$USER" -x nm-applet >/dev/null 2>&1 || nm-applet --indicator >/dev/null 2>&1 &
        pgrep -u "$USER" -x blueman-applet >/dev/null 2>&1 || blueman-applet >/dev/null 2>&1 &
    ]])
end

enable_audio_auto_switch()
autostart_desktop_applets()
gears.timer({
    timeout = 10,
    autostart = true,
    call_now = true,
    callback = function()
        refresh_audio_state(false)
        refresh_system_statuses()
    end,
})

-- Table of layouts to cover with awful.layout.inc, order matters.
awful.layout.layouts = {
    awful.layout.suit.floating,
    awful.layout.suit.tile,
    awful.layout.suit.tile.left,
    awful.layout.suit.tile.bottom,
    awful.layout.suit.tile.top,
    awful.layout.suit.fair,
    awful.layout.suit.fair.horizontal,
    awful.layout.suit.spiral,
    awful.layout.suit.spiral.dwindle,
    awful.layout.suit.max,
    awful.layout.suit.max.fullscreen,
    awful.layout.suit.magnifier,
    awful.layout.suit.corner.nw,
    -- awful.layout.suit.corner.ne,
    -- awful.layout.suit.corner.sw,
    -- awful.layout.suit.corner.se,
}
-- }}}

-- {{{ Menu
-- Create a launcher widget and a main menu
myawesomemenu = {
   { "hotkeys", function() hotkeys_popup.show_help(nil, awful.screen.focused()) end },
   { "manual", terminal .. " -e man awesome" },
   { "edit config", editor_cmd .. " " .. awesome.conffile },
   { "restart", awesome.restart },
   { "quit", function() awesome.quit() end },
}

local menu_awesome = { "awesome", myawesomemenu, beautiful.awesome_icon }
local menu_terminal = { "open terminal", terminal }

if has_fdo then
    mymainmenu = freedesktop.menu.build({
        before = { menu_awesome },
        after =  { menu_terminal }
    })
else
    mymainmenu = awful.menu({
        items = {
                  menu_awesome,
                  { "Debian", debian.menu.Debian_menu.Debian },
                  menu_terminal,
                }
    })
end


mylauncher = awful.widget.launcher({ image = beautiful.awesome_icon,
                                     menu = mymainmenu })

myhostlabel = wibox.widget.textbox(" 💻 " .. identity_label .. " ")
awful.tooltip({
    objects = { myhostlabel },
    text = "Current host identity",
})

-- Menubar configuration
menubar.utils.terminal = terminal -- Set the terminal for applications that require it
-- }}}

-- Keyboard map indicator and switcher
mykeyboardlayout = awful.widget.keyboardlayout()

-- {{{ Wibar
-- Create a wibox for each screen and add it
local taglist_buttons = gears.table.join(
                    awful.button({ }, 1, function(t) t:view_only() end),
                    awful.button({ modkey }, 1, function(t)
                                              if client.focus then
                                                  client.focus:move_to_tag(t)
                                              end
                                          end),
                    awful.button({ }, 3, awful.tag.viewtoggle),
                    awful.button({ modkey }, 3, function(t)
                                              if client.focus then
                                                  client.focus:toggle_tag(t)
                                              end
                                          end),
                    awful.button({ }, 4, function(t) awful.tag.viewnext(t.screen) end),
                    awful.button({ }, 5, function(t) awful.tag.viewprev(t.screen) end)
                )

local tasklist_buttons = gears.table.join(
                     awful.button({ }, 1, function (c)
                                              if c == client.focus then
                                                  c.minimized = true
                                              else
                                                  c:emit_signal(
                                                      "request::activate",
                                                      "tasklist",
                                                      {raise = true}
                                                  )
                                              end
                                          end),
                     awful.button({ }, 3, function()
                                              awful.menu.client_list({ theme = { width = 250 } })
                                          end),
                     awful.button({ }, 4, function ()
                                              awful.client.focus.byidx(1)
                                          end),
                     awful.button({ }, 5, function ()
                                              awful.client.focus.byidx(-1)
                                          end))

local function set_wallpaper(s)
    -- Wallpaper
    if beautiful.wallpaper then
        local wallpaper = beautiful.wallpaper
        -- If wallpaper is a function, call it with the screen
        if type(wallpaper) == "function" then
            wallpaper = wallpaper(s)
        end
        gears.wallpaper.maximized(wallpaper, s, true)
    end
end

-- Re-set wallpaper when a screen's geometry changes (e.g. different resolution)
screen.connect_signal("property::geometry", set_wallpaper)

awful.screen.connect_for_each_screen(function(s)
    -- Wallpaper
    set_wallpaper(s)

    -- Each screen has its own tag table.
    awful.tag({ "1", "2", "3", "4", "5", "6", "7", "8", "9" }, s, awful.layout.layouts[1])

    -- Create a promptbox for each screen
    s.mypromptbox = awful.widget.prompt()
    -- Create an imagebox widget which will contain an icon indicating which layout we're using.
    -- We need one layoutbox per screen.
    s.mylayoutbox = awful.widget.layoutbox(s)
    s.mylayoutbox:buttons(gears.table.join(
                           awful.button({ }, 1, function () awful.layout.inc( 1) end),
                           awful.button({ }, 3, function () awful.layout.inc(-1) end),
                           awful.button({ }, 4, function () awful.layout.inc( 1) end),
                           awful.button({ }, 5, function () awful.layout.inc(-1) end)))
    s.myaudiostatus = create_audio_widget()
    s.mybatterystatus = create_battery_widget()
    s.mynetworkstatus = create_network_widget()
    s.mybluetoothstatus = create_bluetooth_widget()
    s.mythunderbirdstatus = create_thunderbird_widget()
    s.myclock = create_clock_widget()
    -- Create a taglist widget
    s.mytaglist = awful.widget.taglist {
        screen  = s,
        filter  = awful.widget.taglist.filter.all,
        buttons = taglist_buttons
    }

    -- Create a tasklist widget
    s.mytasklist = awful.widget.tasklist {
        screen  = s,
        filter  = awful.widget.tasklist.filter.currenttags,
        buttons = tasklist_buttons
    }

    -- Create the wibox
    s.mywibox = awful.wibar({ position = "top", screen = s })

    -- Add widgets to the wibox
    s.mywibox:setup {
        layout = wibox.layout.align.horizontal,
        { -- Left widgets
            layout = wibox.layout.fixed.horizontal,
            mylauncher,
            s.mytaglist,
            s.mypromptbox,
        },
        s.mytasklist, -- Middle widget
        { -- Right widgets
            layout = wibox.layout.fixed.horizontal,
            s.mybluetoothstatus,
            s.mynetworkstatus,
            s.mybatterystatus,
            s.myaudiostatus,
            mykeyboardlayout,
            wibox.widget.systray(),
            myhostlabel,
            s.mythunderbirdstatus,
            s.myclock,
            s.mylayoutbox,
        },
    }
end)
-- }}}

-- {{{ Mouse bindings
root.buttons(gears.table.join(
    awful.button({ }, 3, function () mymainmenu:toggle() end),
    awful.button({ }, 4, awful.tag.viewnext),
    awful.button({ }, 5, awful.tag.viewprev),
    awful.button({ }, flameshot_mouse_button, open_flameshot),
    awful.button({ }, utility_mouse_button, function ()
        open_linux_control_center_for_client(client.focus)
    end),
    awful.button({ modkey }, 2, volume_toggle_mute),
    awful.button({ modkey }, 4, volume_up),
    awful.button({ modkey }, 5, volume_down)
))
-- }}}

-- {{{ Key bindings
globalkeys = gears.table.join(
    awful.key({ modkey,           }, "s",      hotkeys_popup.show_help,
              {description="show help", group="awesome"}),
    awful.key({ modkey,           }, "Left",   awful.tag.viewprev,
              {description = "view previous", group = "tag"}),
    awful.key({ modkey,           }, "Right",  awful.tag.viewnext,
              {description = "view next", group = "tag"}),
    awful.key({ modkey,           }, "Escape", awful.tag.history.restore,
              {description = "go back", group = "tag"}),

    awful.key({ modkey,           }, "j",
        function ()
            awful.client.focus.byidx( 1)
        end,
        {description = "focus next by index", group = "client"}
    ),
    awful.key({ modkey,           }, "k",
        function ()
            awful.client.focus.byidx(-1)
        end,
        {description = "focus previous by index", group = "client"}
    ),
    awful.key({ modkey,           }, "w", function () mymainmenu:show() end,
              {description = "show main menu", group = "awesome"}),

    -- Layout manipulation
    awful.key({ modkey, "Shift"   }, "j", function () awful.client.swap.byidx(  1)    end,
              {description = "swap with next client by index", group = "client"}),
    awful.key({ modkey, "Shift"   }, "k", function () awful.client.swap.byidx( -1)    end,
              {description = "swap with previous client by index", group = "client"}),
    awful.key({ modkey, "Control" }, "j", function () awful.screen.focus_relative( 1) end,
              {description = "focus the next screen", group = "screen"}),
    awful.key({ modkey, "Control" }, "k", function () awful.screen.focus_relative(-1) end,
              {description = "focus the previous screen", group = "screen"}),
    awful.key({ modkey,           }, "u", awful.client.urgent.jumpto,
              {description = "jump to urgent client", group = "client"}),
    awful.key({ modkey,           }, "Tab",
        function ()
            awful.client.focus.history.previous()
            if client.focus then
                client.focus:raise()
            end
        end,
        {description = "go back", group = "client"}),

    -- Standard program
    awful.key({ modkey,           }, "Return", function () awful.spawn(terminal) end,
              {description = "open a terminal", group = "launcher"}),
    awful.key({                   }, "F8", open_flameshot,
              {description = "open flameshot", group = "launcher"}),
    awful.key({                   }, "Print", open_flameshot,
              {description = "open flameshot", group = "launcher"}),
    awful.key({                   }, "XF86AudioRaiseVolume", volume_up,
              {description = "increase volume", group = "media"}),
    awful.key({                   }, "XF86AudioLowerVolume", volume_down,
              {description = "decrease volume", group = "media"}),
    awful.key({                   }, "XF86AudioMute", volume_toggle_mute,
              {description = "toggle mute", group = "media"}),
    awful.key({ modkey, "Control" }, "r", awesome.restart,
              {description = "reload awesome", group = "awesome"}),
    awful.key({ modkey, "Shift"   }, "q", awesome.quit,
              {description = "quit awesome", group = "awesome"}),

    awful.key({ modkey,           }, "l",     function () awful.tag.incmwfact( 0.05)          end,
              {description = "increase master width factor", group = "layout"}),
    awful.key({ modkey,           }, "h",     function () awful.tag.incmwfact(-0.05)          end,
              {description = "decrease master width factor", group = "layout"}),
    awful.key({ modkey, "Shift"   }, "h",     function () awful.tag.incnmaster( 1, nil, true) end,
              {description = "increase the number of master clients", group = "layout"}),
    awful.key({ modkey, "Shift"   }, "l",     function () awful.tag.incnmaster(-1, nil, true) end,
              {description = "decrease the number of master clients", group = "layout"}),
    awful.key({ modkey, "Control" }, "h",     function () awful.tag.incncol( 1, nil, true)    end,
              {description = "increase the number of columns", group = "layout"}),
    awful.key({ modkey, "Control" }, "l",     function () awful.tag.incncol(-1, nil, true)    end,
              {description = "decrease the number of columns", group = "layout"}),
    awful.key({ modkey,           }, "space", function () awful.layout.inc( 1)                end,
              {description = "select next", group = "layout"}),
    awful.key({ modkey, "Shift"   }, "space", function () awful.layout.inc(-1)                end,
              {description = "select previous", group = "layout"}),

    awful.key({ modkey, "Control" }, "n",
              function ()
                  local c = awful.client.restore()
                  -- Focus restored client
                  if c then
                    c:emit_signal(
                        "request::activate", "key.unminimize", {raise = true}
                    )
                  end
              end,
              {description = "restore minimized", group = "client"}),

    -- Prompt
    awful.key({ modkey },            "r",     function () awful.screen.focused().mypromptbox:run() end,
              {description = "run prompt", group = "launcher"}),

    awful.key({ modkey }, "x",
              function ()
                  awful.prompt.run {
                    prompt       = "Run Lua code: ",
                    textbox      = awful.screen.focused().mypromptbox.widget,
                    exe_callback = awful.util.eval,
                    history_path = awful.util.get_cache_dir() .. "/history_eval"
                  }
              end,
              {description = "lua execute prompt", group = "awesome"}),
    -- Menubar
    awful.key({ modkey }, "p", function() menubar.show() end,
              {description = "show the menubar", group = "launcher"})
)

clientkeys = gears.table.join(
    awful.key({ modkey,           }, "f",
        function (c)
            c.fullscreen = not c.fullscreen
            c:raise()
        end,
        {description = "toggle fullscreen", group = "client"}),
    awful.key({ modkey, "Shift"   }, "c",      function (c) c:kill()                         end,
              {description = "close", group = "client"}),
    awful.key({ modkey, "Control" }, "space",  awful.client.floating.toggle                     ,
              {description = "toggle floating", group = "client"}),
    awful.key({ modkey, "Control" }, "Return", function (c) c:swap(awful.client.getmaster()) end,
              {description = "move to master", group = "client"}),
    awful.key({ modkey,           }, "o",      function (c) c:move_to_screen()               end,
              {description = "move to screen", group = "client"}),
    awful.key({ modkey,           }, "t",      function (c) c.ontop = not c.ontop            end,
              {description = "toggle keep on top", group = "client"}),
    awful.key({ modkey,           }, "n",
        function (c)
            -- The client currently has the input focus, so it cannot be
            -- minimized, since minimized clients can't have the focus.
            c.minimized = true
        end ,
        {description = "minimize", group = "client"}),
    awful.key({ modkey,           }, "m",
        function (c)
            c.maximized = not c.maximized
            c:raise()
        end ,
        {description = "(un)maximize", group = "client"}),
    awful.key({ modkey, "Control" }, "m",
        function (c)
            c.maximized_vertical = not c.maximized_vertical
            c:raise()
        end ,
        {description = "(un)maximize vertically", group = "client"}),
    awful.key({ modkey, "Shift"   }, "m",
        function (c)
            c.maximized_horizontal = not c.maximized_horizontal
            c:raise()
        end ,
        {description = "(un)maximize horizontally", group = "client"})
)

-- Bind all key numbers to tags.
-- Be careful: we use keycodes to make it work on any keyboard layout.
-- This should map on the top row of your keyboard, usually 1 to 9.
for i = 1, 9 do
    globalkeys = gears.table.join(globalkeys,
        -- View tag only.
        awful.key({ modkey }, "#" .. i + 9,
                  function ()
                        local screen = awful.screen.focused()
                        local tag = screen.tags[i]
                        if tag then
                           tag:view_only()
                        end
                  end,
                  {description = "view tag #"..i, group = "tag"}),
        -- Toggle tag display.
        awful.key({ modkey, "Control" }, "#" .. i + 9,
                  function ()
                      local screen = awful.screen.focused()
                      local tag = screen.tags[i]
                      if tag then
                         awful.tag.viewtoggle(tag)
                      end
                  end,
                  {description = "toggle tag #" .. i, group = "tag"}),
        -- Move client to tag.
        awful.key({ modkey, "Shift" }, "#" .. i + 9,
                  function ()
                      if client.focus then
                          local tag = client.focus.screen.tags[i]
                          if tag then
                              client.focus:move_to_tag(tag)
                          end
                     end
                  end,
                  {description = "move focused client to tag #"..i, group = "tag"}),
        -- Toggle tag on focused client.
        awful.key({ modkey, "Control", "Shift" }, "#" .. i + 9,
                  function ()
                      if client.focus then
                          local tag = client.focus.screen.tags[i]
                          if tag then
                              client.focus:toggle_tag(tag)
                          end
                      end
                  end,
                  {description = "toggle focused client on tag #" .. i, group = "tag"})
    )
end

clientbuttons = gears.table.join(
    awful.button({ }, 1, function (c)
        c:emit_signal("request::activate", "mouse_click", {raise = true})
    end),
    awful.button({ }, flameshot_mouse_button, function (c)
        c:emit_signal("request::activate", "mouse_click", {raise = false})
        open_flameshot()
    end),
    awful.button({ }, utility_mouse_button, function (c)
        c:emit_signal("request::activate", "mouse_click", {raise = false})
        open_linux_control_center_for_client(c)
    end),
    awful.button({ modkey }, 1, function (c)
        c:emit_signal("request::activate", "mouse_click", {raise = true})
        awful.mouse.client.move(c)
    end),
    awful.button({ modkey }, 3, function (c)
        c:emit_signal("request::activate", "mouse_click", {raise = true})
        awful.mouse.client.resize(c)
    end)
)

-- Set keys
root.keys(globalkeys)
-- }}}

-- {{{ Rules
-- Rules to apply to new clients (through the "manage" signal).
awful.rules.rules = {
    -- All clients will match this rule.
    { rule = { },
      properties = { border_width = beautiful.border_width,
                     border_color = beautiful.border_normal,
                     focus = awful.client.focus.filter,
                     raise = true,
                     keys = clientkeys,
                     buttons = clientbuttons,
                     screen = awful.screen.preferred,
                     placement = awful.placement.no_overlap+awful.placement.no_offscreen
     }
    },

    -- Floating clients.
    { rule_any = {
        instance = {
          "DTA",  -- Firefox addon DownThemAll.
          "copyq",  -- Includes session name in class.
          "pinentry",
        },
        class = {
          "Arandr",
          "Blueman-manager",
          "Gpick",
          "Kruler",
          "MessageWin",  -- kalarm.
          "Sxiv",
          "Tor Browser", -- Needs a fixed window size to avoid fingerprinting by screen size.
          "Wpa_gui",
          "veromix",
          "xtightvncviewer"},

        -- Note that the name property shown in xprop might be set slightly after creation of the client
        -- and the name shown there might not match defined rules here.
        name = {
          "Event Tester",  -- xev.
        },
        role = {
          "AlarmWindow",  -- Thunderbird's calendar.
          "ConfigManager",  -- Thunderbird's about:config.
          "pop-up",       -- e.g. Google Chrome's (detached) Developer Tools.
        }
      }, properties = { floating = true }},

    -- Add titlebars to normal clients and dialogs
    { rule_any = {type = { "normal", "dialog" }
      }, properties = { titlebars_enabled = true }
    },

    -- Set Firefox to always map on the tag named "2" on screen 1.
    -- { rule = { class = "Firefox" },
    --   properties = { screen = 1, tag = "2" } },
}
-- }}}

-- {{{ Signals
-- Signal function to execute when a new client appears.
client.connect_signal("manage", function (c)
    -- Set the windows at the slave,
    -- i.e. put it at the end of others instead of setting it master.
    -- if not awesome.startup then awful.client.setslave(c) end

    if awesome.startup
      and not c.size_hints.user_position
      and not c.size_hints.program_position then
        -- Prevent clients from being unreachable after screen count changes.
        awful.placement.no_offscreen(c)
    end
end)

-- Add a titlebar if titlebars_enabled is set to true in the rules.
client.connect_signal("request::titlebars", function(c)
    -- buttons for the titlebar
    local buttons = gears.table.join(
        awful.button({ }, 1, function()
            c:emit_signal("request::activate", "titlebar", {raise = true})
            awful.mouse.client.move(c)
        end),
        awful.button({ }, 3, function()
            c:emit_signal("request::activate", "titlebar", {raise = true})
            awful.mouse.client.resize(c)
        end)
    )

    awful.titlebar(c) : setup {
        { -- Left
            awful.titlebar.widget.iconwidget(c),
            buttons = buttons,
            layout  = wibox.layout.fixed.horizontal
        },
        { -- Middle
            { -- Title
                align  = "center",
                widget = awful.titlebar.widget.titlewidget(c)
            },
            buttons = buttons,
            layout  = wibox.layout.flex.horizontal
        },
        { -- Right
            awful.titlebar.widget.floatingbutton (c),
            awful.titlebar.widget.maximizedbutton(c),
            awful.titlebar.widget.stickybutton   (c),
            awful.titlebar.widget.ontopbutton    (c),
            awful.titlebar.widget.closebutton    (c),
            layout = wibox.layout.fixed.horizontal()
        },
        layout = wibox.layout.align.horizontal
    }
end)

-- Enable sloppy focus, so that focus follows mouse.
client.connect_signal("mouse::enter", function(c)
    c:emit_signal("request::activate", "mouse_enter", {raise = false})
end)

client.connect_signal("focus", function(c) c.border_color = beautiful.border_focus end)
client.connect_signal("unfocus", function(c) c.border_color = beautiful.border_normal end)
-- }}}
