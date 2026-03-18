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

local bindings = require("linuxutils.bindings")

local menu_state = { show_calls = 0, toggle_calls = 0 }
local launcher_calls = {
    flameshot = 0,
    utility = 0,
    volume_up = 0,
    brightness_up = 0,
    launcher = 0,
    spotlight = 0,
}

local function flatten_join(...)
    local joined = {}
    for _, item in ipairs({ ... }) do
        if type(item) == "table" and item.kind == nil then
            for _, nested in ipairs(item) do
                table.insert(joined, nested)
            end
        else
            table.insert(joined, item)
        end
    end
    return joined
end

local focused_tag = {
    viewed = false,
    view_only = function(self)
        self.viewed = true
    end,
}

local promptbox = {
    run_calls = 0,
    widget = {},
    run = function(self)
        self.run_calls = self.run_calls + 1
    end,
}

local focused_screen = {
    tags = {
        focused_tag, focused_tag, focused_tag, focused_tag, focused_tag,
        focused_tag, focused_tag, focused_tag, focused_tag,
    },
    mypromptbox = promptbox,
}

local focused_client = {
    screen = focused_screen,
    raise = function()
    end,
    move_to_tag = function()
    end,
    toggle_tag = function()
    end,
}

local awful = {
    button = function(modifiers, button_id, callback)
        return {
            kind = "button",
            modifiers = modifiers,
            button = button_id,
            callback = callback,
        }
    end,
    client = {
        focus = {
            byidx = function()
            end,
            history = {
                previous = function()
                end,
            },
        },
        swap = {
            byidx = function()
            end,
        },
        urgent = {
            jumpto = function()
            end,
        },
        floating = {
            toggle = function()
            end,
        },
        getmaster = function()
            return {}
        end,
        restore = function()
            return nil
        end,
    },
    key = function(modifiers, key_name, callback, meta)
        return {
            kind = "key",
            modifiers = modifiers,
            key = key_name,
            callback = callback,
            description = meta and meta.description or "",
            group = meta and meta.group or "",
        }
    end,
    layout = {
        inc = function()
        end,
    },
    mouse = {
        client = {
            move = function()
            end,
            resize = function()
            end,
        },
    },
    prompt = {
        run = function()
        end,
    },
    screen = {
        focus_relative = function()
        end,
        focused = function()
            return focused_screen
        end,
    },
    spawn = function()
    end,
    tag = {
        history = {
            restore = function()
            end,
        },
        incmwfact = function()
        end,
        incnmaster = function()
        end,
        incncol = function()
        end,
        viewnext = function()
        end,
        viewprev = function()
        end,
        viewtoggle = function()
        end,
    },
    util = {
        eval = function()
        end,
        get_cache_dir = function()
            return "/tmp"
        end,
    },
}

local gears = {
    table = {
        join = flatten_join,
    },
}

local hotkeys_popup = {
    show_help = function()
    end,
}

local menubar = {
    show = function()
    end,
}

local awesome_api = {
    restart = function()
    end,
    quit = function()
    end,
}

local client_global = {
    focus = focused_client,
}

local mymainmenu = {
    show = function()
        menu_state.show_calls = menu_state.show_calls + 1
    end,
    toggle = function()
        menu_state.toggle_calls = menu_state.toggle_calls + 1
    end,
}

local binding_tables = bindings.build({
    awful = awful,
    gears = gears,
    hotkeys_popup = hotkeys_popup,
    menubar = menubar,
    awesome = awesome_api,
    client_global = client_global,
    mymainmenu = mymainmenu,
    modkey = "Mod4",
    terminal = "x-terminal-emulator",
    flameshot_mouse_button = 8,
    utility_mouse_button = 9,
    actions = {
        open_flameshot = function()
            launcher_calls.flameshot = launcher_calls.flameshot + 1
        end,
        open_linux_control_center_for_client = function()
            launcher_calls.utility = launcher_calls.utility + 1
        end,
        volume_up = function()
            launcher_calls.volume_up = launcher_calls.volume_up + 1
        end,
        volume_down = function()
        end,
        volume_toggle_mute = function()
        end,
        brightness_up = function()
            launcher_calls.brightness_up = launcher_calls.brightness_up + 1
        end,
        brightness_down = function()
        end,
        media_play_pause = function()
        end,
        media_next_track = function()
        end,
        media_prev_track = function()
        end,
        launch_program_palette = function()
            launcher_calls.launcher = launcher_calls.launcher + 1
        end,
        toggle_gromit_draw = function()
        end,
        clear_gromit_draw = function()
        end,
        undo_gromit_draw = function()
        end,
        redo_gromit_draw = function()
        end,
        toggle_gromit_visibility = function()
        end,
        quit_gromit = function()
        end,
        presenter_dash_anchor = function()
        end,
        presenter_dash_dash = function()
        end,
        presenter_dash_dot = function()
        end,
        presenter_dash_solid = function()
        end,
        presenter_dash_arrow = function()
        end,
        presenter_dash_reset = function()
        end,
        toggle_cursor_spotlight = function()
            launcher_calls.spotlight = launcher_calls.spotlight + 1
        end,
        adjust_cursor_spotlight = function()
        end,
    },
})

assert(type(binding_tables.root_buttons) == "table")
assert(type(binding_tables.globalkeys) == "table")
assert(type(binding_tables.clientkeys) == "table")
assert(type(binding_tables.clientbuttons) == "table")

local function find_button(buttons, button_id)
    for _, button in pairs(buttons) do
        if button.kind == "button" and button.button == button_id then
            return button
        end
    end
    return nil
end

local function find_key(keys, key_name, description)
    for _, key in pairs(keys) do
        if key.kind == "key" and key.key == key_name then
            if not description or key.description == description then
                return key
            end
        end
    end
    return nil
end

assert(find_button(binding_tables.root_buttons, 8) ~= nil)
assert(find_button(binding_tables.root_buttons, 9) ~= nil)
assert(find_button(binding_tables.clientbuttons, 8) ~= nil)
assert(find_button(binding_tables.clientbuttons, 9) ~= nil)

assert(find_key(binding_tables.globalkeys, "XF86MonBrightnessUp", "increase brightness") ~= nil)
assert(find_key(binding_tables.globalkeys, "XF86AudioRaiseVolume", "increase volume") ~= nil)
assert(find_key(binding_tables.globalkeys, "F6", "toggle presenter drawing (gromit)") ~= nil)
assert(find_key(binding_tables.globalkeys, "F7", "toggle cursor spotlight") ~= nil)
assert(find_key(binding_tables.globalkeys, "Print", "open flameshot") ~= nil)

find_button(binding_tables.root_buttons, 8).callback()
find_button(binding_tables.root_buttons, 9).callback()
find_key(binding_tables.globalkeys, "XF86MonBrightnessUp", "increase brightness").callback()
find_key(binding_tables.globalkeys, "XF86AudioRaiseVolume", "increase volume").callback()
find_key(binding_tables.globalkeys, "F7", "toggle cursor spotlight").callback()
find_key(binding_tables.globalkeys, "r", "program launcher (rofi + custom commands)").callback()

assert(launcher_calls.flameshot == 1)
assert(launcher_calls.utility == 1)
assert(launcher_calls.brightness_up == 1)
assert(launcher_calls.volume_up == 1)
assert(launcher_calls.spotlight == 1)
assert(launcher_calls.launcher == 1)

if _G.awesome and _G.awesome.quit then
    _G.awesome.quit()
end
