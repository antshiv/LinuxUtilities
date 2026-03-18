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
local presenter = require("linuxutils.presenter")
local launchers = require("linuxutils.launchers")
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
        calendar_popup = {
            month = function()
                return {
                    attach = function()
                    end,
                }
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
    imagebox = {},
}, {
    __call = function(_, spec)
        return widget_methods(spec or {})
    end,
})
wibox.container = {
    margin = {},
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

local widgets_controller = widgets.new({
    awful = awful,
    gears = gears,
    wibox = wibox,
    shorten_label = function(value, max_len)
        return (value or ""):sub(1, max_len)
    end,
    audio = audio_controller,
    launchers = launchers_controller,
})

assert(type(brightness_controller.up) == "function")
assert(type(brightness_controller.down) == "function")

assert(type(audio_controller.refresh) == "function")
assert(type(audio_controller.volume_up) == "function")
assert(type(audio_controller.media_play_pause) == "function")
assert(type(audio_controller.tooltip_text()) == "string")

assert(type(presenter_controller.toggle_cursor_spotlight) == "function")
assert(type(presenter_controller.adjust_cursor_spotlight) == "function")
assert(type(presenter_controller.presenter_dash_dash) == "function")

assert(type(launchers_controller.open_flameshot) == "function")
assert(type(launchers_controller.launch_program_palette) == "function")
assert(type(launchers_controller.open_linux_control_center_for_client) == "function")

assert(type(widgets_controller.refresh_system_statuses) == "function")
assert(type(widgets_controller.create_audio_widget()) == "table")
assert(type(widgets_controller.create_battery_widget()) == "table")
assert(type(widgets_controller.create_network_widget()) == "table")
assert(type(widgets_controller.create_bluetooth_widget()) == "table")
assert(type(widgets_controller.create_thunderbird_widget()) == "table")
assert(type(widgets_controller.create_clock_widget()) == "table")
