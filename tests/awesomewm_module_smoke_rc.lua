pcall(require, "luarocks.loader")

local source = debug.getinfo(1, "S").source
local this_file = source:sub(2)
local root_dir = this_file:match("^(.*)/tests/[^/]+$")
assert(root_dir, "failed to derive repository root")

package.path = table.concat({
    root_dir .. "/?.lua",
    root_dir .. "/?/init.lua",
    package.path,
}, ";")

local brightness = require("linuxutils.brightness")
local audio = require("linuxutils.audio")
local icons = require("linuxutils.icons")
local presenter = require("linuxutils.presenter")
local launchers = require("linuxutils.launchers")
local calendar = require("linuxutils.calendar")
local widgets = require("linuxutils.widgets")

local notifications = {}

local awful = {
    spawn = {
        easy_async = function(_, callback)
            if callback then
                callback("", "", "", 0)
            end
        end,
        easy_async_with_shell = function(_, callback)
            if callback then
                callback("", "", "", 0)
            end
        end,
        with_shell = function()
        end,
    },
    popup = function(spec)
        return spec or {}
    end,
    placement = {
        centered = function()
        end,
    },
    screen = {
        focused = function()
            return {
                mypromptbox = {
                    run = function()
                    end,
                },
            }
        end,
    },
    widget = {
        calendar = {
            month = function(_)
                return widget_methods({
                    set_date = function(self, date)
                        self.date = date
                    end,
                    set_font = function(self, font)
                        self.font = font
                    end,
                })
            end,
        },
    },
    button = function()
        return {}
    end,
    tooltip = function()
    end,
}

local gears = {
    filesystem = {
        file_readable = function()
            return false
        end,
    },
    table = {
        join = function(...)
            return { ... }
        end,
    },
    timer = {
        start_new = function(_, callback)
            if callback then
                callback()
            end
        end,
    },
}

local naughty = {
    config = {
        presets = {
            warn = {},
            critical = {},
        },
    },
    notify = function(opts)
        table.insert(notifications, opts)
        return { id = #notifications }
    end,
}

local function widget_methods(target)
    target.set_text = function(self, text)
        self.text = text
    end
    target.set_markup = function(self, markup)
        self.markup = markup
    end
    target.buttons = function(self, buttons)
        self._buttons = buttons
    end
    return target
end

local wibox = {}
wibox.widget = setmetatable({
    textbox = function(initial)
        return widget_methods({ text = initial })
    end,
    textclock = function(initial)
        return widget_methods({ text = initial })
    end,
    calendar = {
        month = function(spec)
            local widget = widget_methods({
                date = spec and spec.date or os.date("*t"),
                font = spec and spec.font or "Monospace 12",
            })
            widget.set_date = function(self, date)
                self.date = date
            end
            widget.set_font = function(self, font)
                self.font = font
            end
            return widget
        end,
    },
    imagebox = {},
}, {
    __call = function(_, spec)
        if type(spec) == "table" and spec.widget == wibox.widget.calendar.month then
            return wibox.widget.calendar.month(spec)
        end
        return widget_methods(spec or {})
    end,
})
wibox.container = {
    margin = {},
    background = {},
}
wibox.layout = {
    fixed = {
        horizontal = {},
        vertical = {},
    },
}

local function shell_quote(value)
    return string.format("%q", value or "")
end

local function clamp_number(value, min_value, max_value)
    if value < min_value then
        return min_value
    end
    if value > max_value then
        return max_value
    end
    return value
end

local brightness_controller = brightness.new({
    awful = awful,
    naughty = naughty,
    shell_quote = shell_quote,
    step_percent = 5,
})

local audio_controller = audio.new({
    awful = awful,
    gears = gears,
    naughty = naughty,
    command_exists = function()
        return false
    end,
    compact_sink_name = function(value)
        return value or "unknown"
    end,
    volume_step = "5%",
})

local presenter_controller = presenter.new({
    awful = awful,
    gears = gears,
    naughty = naughty,
    shell_quote = shell_quote,
    clamp_number = clamp_number,
    spotlight_radius = 180,
    spotlight_dim = 0.68,
    spotlight_fps = 50,
})

local launchers_controller = launchers.new({
    awful = awful,
    gears = gears,
    naughty = naughty,
    command_exists = function()
        return false
    end,
    shell_quote = shell_quote,
})

local calendar_controller = calendar.new({
    awful = awful,
    gears = gears,
    wibox = wibox,
    launchers = launchers_controller,
})

local widgets_controller = widgets.new({
    awful = awful,
    gears = gears,
    wibox = wibox,
    shorten_label = function(value, max_len)
        return (value or ""):sub(1, max_len)
    end,
    audio = audio_controller,
    launchers = launchers_controller,
    calendar = calendar_controller,
})

assert(type(brightness_controller.up) == "function")
assert(type(brightness_controller.down) == "function")
assert(type(icons.bluetooth) == "function")
assert(type(icons.network) == "function")

assert(type(audio_controller.refresh) == "function")
assert(type(audio_controller.volume_up) == "function")
assert(type(audio_controller.media_play_pause) == "function")
assert(type(audio_controller.tooltip_text()) == "string")

assert(type(presenter_controller.toggle_cursor_spotlight) == "function")
assert(type(presenter_controller.adjust_cursor_spotlight) == "function")
assert(type(presenter_controller.presenter_dash_dash) == "function")

assert(type(launchers_controller.open_flameshot) == "function")
assert(type(launchers_controller.open_system_monitor) == "function")
assert(type(launchers_controller.open_home_folder) == "function")
assert(type(launchers_controller.open_workspace_folder) == "function")
assert(type(launchers_controller.open_screenshots_folder) == "function")
assert(type(launchers_controller.open_notes_folder) == "function")
assert(type(launchers_controller.open_appimage_library) == "function")
assert(type(launchers_controller.open_appimage_palette) == "function")
assert(type(launchers_controller.open_network_tui) == "function")
assert(type(launchers_controller.open_network_scan) == "function")
assert(type(launchers_controller.launch_program_palette) == "function")
assert(type(launchers_controller.show_world_clock_popup) == "function")
assert(type(launchers_controller.set_timezone_mumbai) == "function")
assert(type(launchers_controller.set_timezone_vancouver) == "function")
assert(type(launchers_controller.open_linux_control_center_for_client) == "function")
assert(type(launchers_controller.open_notes_app) == "function")
assert(type(launchers_controller.open_daily_note) == "function")
assert(type(launchers_controller.open_tasks_note) == "function")
assert(type(launchers_controller.appimage_root()) == "string")

assert(type(calendar_controller.toggle_popup) == "function")
assert(type(calendar_controller.previous_month) == "function")
assert(type(calendar_controller.clock_tooltip_text()) == "string")
calendar_controller.toggle_popup()
calendar_controller.next_month()
calendar_controller.increase_font()
calendar_controller.select_date({ year = 2026, month = 3, day = 20 })
local popup_state = calendar_controller.popup_state()
assert(popup_state.visible == true)
assert(type(popup_state.note_path) == "string")
assert(type(popup_state.selected_date) == "string")
calendar_controller.toggle_popup({ bogus = true })
calendar_controller.toggle_popup()

assert(type(widgets_controller.refresh_system_statuses) == "function")
assert(type(widgets_controller.create_audio_widget()) == "table")
assert(type(widgets_controller.create_battery_widget()) == "table")
local network_widget = widgets_controller.create_network_widget()
local bluetooth_widget = widgets_controller.create_bluetooth_widget()
local system_widget = widgets_controller.create_system_widget()
local folder_widget = widgets_controller.create_folder_widget()
local applications_widget = widgets_controller.create_applications_widget()

assert(type(network_widget) == "table")
assert(type(bluetooth_widget) == "table")
assert(type(system_widget) == "table")
assert(type(folder_widget) == "table")
assert(type(applications_widget) == "table")
assert(type(network_widget.markup) == "string")
assert(type(bluetooth_widget.markup) == "string")
assert(type(system_widget.markup) == "string")
assert(network_widget.markup:match("foreground=") ~= nil)
assert(bluetooth_widget.markup:match("foreground=") ~= nil)
assert(system_widget.markup:match("foreground=") ~= nil)

assert(type(widgets_controller.create_thunderbird_widget()) == "table")
local clock_widget = widgets_controller.create_clock_widget()
assert(type(clock_widget) == "table")
assert(clock_widget._calendar_controller == calendar_controller)

if awesome and awesome.quit then
    awesome.quit()
end
