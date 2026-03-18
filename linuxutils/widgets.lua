local icons = require("linuxutils.icons")

local M = {}

local function set_widget_text(widget_list, text)
    for _, widget in ipairs(widget_list) do
        widget:set_text(text)
    end
end

local function set_widget_markup(widget_list, markup)
    for _, widget in ipairs(widget_list) do
        if widget.set_markup then
            widget:set_markup(markup)
        else
            widget:set_text(markup)
        end
    end
end

function M.new(opts)
    local awful = assert(opts and opts.awful, "widgets.new requires awful")
    local gears = assert(opts and opts.gears, "widgets.new requires gears")
    local wibox = assert(opts and opts.wibox, "widgets.new requires wibox")
    local shorten_label = assert(opts and opts.shorten_label, "widgets.new requires shorten_label")
    local audio = assert(opts and opts.audio, "widgets.new requires audio controller")
    local launchers = assert(opts and opts.launchers, "widgets.new requires launchers controller")
    local calendar = assert(opts and opts.calendar, "widgets.new requires calendar controller")

    local battery_state = { status = "unknown", percent = "?", time = "" }
    local network_state = { state = "unknown", ntype = "unknown", label = "offline" }
    local bluetooth_state = { powered = "unknown", connected = "0" }
    local battery_status_widgets = {}
    local network_status_widgets = {}
    local bluetooth_status_widgets = {}
    local controller = {}

    local function set_battery_widget_text(text)
        set_widget_text(battery_status_widgets, text)
    end

    local function set_network_widget_markup(markup)
        set_widget_markup(network_status_widgets, markup)
    end

    local function set_bluetooth_widget_markup(markup)
        set_widget_markup(bluetooth_status_widgets, markup)
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

            local label_short = shorten_label(network_state.label, 12)
            set_network_widget_markup(icons.network(network_state.ntype, label_short))
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

            set_bluetooth_widget_markup(icons.bluetooth(bluetooth_state.powered, bluetooth_state.connected))
        end)
    end

    function controller.refresh_system_statuses()
        refresh_battery_state()
        refresh_network_state()
        refresh_bluetooth_state()
    end

    function controller.create_thunderbird_widget()
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
            awful.button({ }, 1, launchers.open_thunderbird),
            awful.button({ }, 3, launchers.open_thunderbird_calendar)
        ))

        awful.tooltip({
            objects = { widget },
            text = "Thunderbird\nLeft click: mail\nRight click: calendar",
        })

        return widget
    end

    function controller.create_audio_widget()
        local widget = wibox.widget.textbox()
        widget:set_text(" 🔊 -- ")
        widget:buttons(gears.table.join(
            awful.button({ }, 1, audio.open_pavucontrol),
            awful.button({ }, 3, audio.volume_toggle_mute),
            awful.button({ }, 4, audio.volume_up),
            awful.button({ }, 5, audio.volume_down)
        ))
        awful.tooltip({
            objects = { widget },
            timer_function = function()
                return audio.tooltip_text()
            end,
        })
        audio.attach_widget(widget)
        audio.refresh(false)
        return widget
    end

    function controller.create_battery_widget()
        local widget = wibox.widget.textbox()
        widget:set_text(" 🔋 -- ")
        widget:buttons(gears.table.join(
            awful.button({ }, 1, launchers.open_power_manager),
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

    function controller.create_network_widget()
        local widget = wibox.widget.textbox()
        widget:set_markup(icons.network("unknown", "--"))
        widget:buttons(gears.table.join(
            awful.button({ }, 1, launchers.open_network_manager),
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

    function controller.create_bluetooth_widget()
        local widget = wibox.widget.textbox()
        widget:set_markup(icons.bluetooth("unknown", "0"))
        widget:buttons(gears.table.join(
            awful.button({ }, 1, launchers.open_bluetooth_manager),
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

    function controller.create_clock_widget()
        local widget = wibox.widget.textclock(" 🕒 %a %d %b %H:%M ")
        widget:buttons(gears.table.join(
            awful.button({ }, 1, function()
                calendar.toggle_popup()
            end),
            awful.button({ }, 2, function()
                launchers.open_calendar_app()
            end),
            awful.button({ }, 3, function()
                launchers.open_time_preferences()
            end),
            awful.button({ }, 4, function()
                calendar.previous_month()
            end),
            awful.button({ }, 5, function()
                calendar.next_month()
            end)
        ))
        widget._calendar_controller = calendar
        awful.tooltip({
            objects = { widget },
            timer_function = calendar.clock_tooltip_text,
        })
        return widget
    end

    return controller
end

return M
