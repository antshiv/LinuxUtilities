-- If LuaRocks is installed, make sure that packages installed through it are
-- found (e.g. lgi). If LuaRocks is not installed, do nothing.
pcall(require, "luarocks.loader")

-- Standard awesome library
local gears = require("gears")
local function resolve_config_dir()
    local source = debug.getinfo(1, "S").source
    if type(source) == "string" and source:sub(1, 1) == "@" then
        local source_dir = source:sub(2):match("^(.*[/\\])")
        if source_dir and source_dir ~= "" then
            return source_dir
        end
    end
    return gears.filesystem.get_configuration_dir()
end

local config_dir = resolve_config_dir()
package.path = table.concat({
    config_dir .. "?.lua",
    config_dir .. "?/init.lua",
    package.path,
}, ";")

local awful = require("awful")
require("awful.autofocus")
-- Widget and layout library
local wibox = require("wibox")
-- Theme handling library
local beautiful = require("beautiful")
-- Notification library
local naughty = require("naughty")
local menubar = require("menubar")
local keyboardlayout_widget = require("awful.widget.keyboardlayout")
local hotkeys_popup = require("awful.hotkeys_popup")
-- Enable hotkeys help widget for VIM and other apps
-- when client with a matching name is opened:
require("awful.hotkeys_popup.keys")

-- Load Debian menu entries
local debian = require("debian.menu")
local has_fdo, freedesktop = pcall(require, "freedesktop")
local common = require("linuxutils.common")
local brightness = require("linuxutils.brightness")
local audio = require("linuxutils.audio")
local presenter = require("linuxutils.presenter")
local launchers = require("linuxutils.launchers")
local calendar = require("linuxutils.calendar")
local widgets = require("linuxutils.widgets")
local bindings = require("linuxutils.bindings")

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

local get_short_hostname = common.get_short_hostname
local compact_sink_name = common.compact_sink_name
local shorten_label = common.shorten_label
local shell_quote = common.shell_quote
local clamp_number = common.clamp_number
local command_exists = common.command_exists

local identity_label = (os.getenv("USER") or "user") .. "@" .. get_short_hostname()

local volume_step = "5%"
local brightness_step_percent = 5
local spotlight_radius = 180
local spotlight_dim = 0.68
local spotlight_fps = 50
-- MX Master 3S side buttons are commonly 8/9 on X11.
-- Confirm on your machine with: xev -event button
local flameshot_mouse_button = 8
local utility_mouse_button = 9
-- Keep the custom AwesomeWM Bluetooth/network widgets as the primary status surface.
-- Set LINUXUTILS_AUTOSTART_STATUS_APPLETS=1 if you explicitly want nm-applet/blueman-applet duplicates in the tray.
local autostart_status_tray_applets = os.getenv("LINUXUTILS_AUTOSTART_STATUS_APPLETS") == "1"

local brightness_controller = brightness.new({
    awful = awful,
    naughty = naughty,
    shell_quote = shell_quote,
    step_percent = brightness_step_percent,
})

local audio_controller = audio.new({
    awful = awful,
    gears = gears,
    naughty = naughty,
    command_exists = command_exists,
    compact_sink_name = compact_sink_name,
    volume_step = volume_step,
})

local presenter_controller = presenter.new({
    awful = awful,
    gears = gears,
    naughty = naughty,
    shell_quote = shell_quote,
    clamp_number = clamp_number,
    spotlight_radius = spotlight_radius,
    spotlight_dim = spotlight_dim,
    spotlight_fps = spotlight_fps,
})

local launchers_controller = launchers.new({
    awful = awful,
    gears = gears,
    naughty = naughty,
    command_exists = command_exists,
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
    shorten_label = shorten_label,
    audio = audio_controller,
    launchers = launchers_controller,
    calendar = calendar_controller,
})

local refresh_system_statuses = widgets_controller.refresh_system_statuses
local create_audio_widget = widgets_controller.create_audio_widget
local create_battery_widget = widgets_controller.create_battery_widget
local create_network_widget = widgets_controller.create_network_widget
local create_bluetooth_widget = widgets_controller.create_bluetooth_widget
local create_system_widget = widgets_controller.create_system_widget
local create_folder_widget = widgets_controller.create_folder_widget
local create_applications_widget = widgets_controller.create_applications_widget
local create_thunderbird_widget = widgets_controller.create_thunderbird_widget
local create_clock_widget = widgets_controller.create_clock_widget

local brightness_up = brightness_controller.up
local brightness_down = brightness_controller.down

local volume_up = audio_controller.volume_up
local volume_down = audio_controller.volume_down
local volume_toggle_mute = audio_controller.volume_toggle_mute
local media_play_pause = audio_controller.media_play_pause
local media_next_track = audio_controller.media_next_track
local media_prev_track = audio_controller.media_prev_track
local enable_audio_auto_switch = audio_controller.enable_auto_switch

local toggle_cursor_spotlight = presenter_controller.toggle_cursor_spotlight
local adjust_cursor_spotlight = presenter_controller.adjust_cursor_spotlight
local toggle_gromit_draw = presenter_controller.toggle_gromit_draw
local clear_gromit_draw = presenter_controller.clear_gromit_draw
local undo_gromit_draw = presenter_controller.undo_gromit_draw
local redo_gromit_draw = presenter_controller.redo_gromit_draw
local toggle_gromit_visibility = presenter_controller.toggle_gromit_visibility
local quit_gromit = presenter_controller.quit_gromit
local presenter_dash_anchor = presenter_controller.presenter_dash_anchor
local presenter_dash_dash = presenter_controller.presenter_dash_dash
local presenter_dash_dot = presenter_controller.presenter_dash_dot
local presenter_dash_solid = presenter_controller.presenter_dash_solid
local presenter_dash_arrow = presenter_controller.presenter_dash_arrow
local presenter_dash_reset = presenter_controller.presenter_dash_reset

local open_flameshot = launchers_controller.open_flameshot
local open_system_monitor = launchers_controller.open_system_monitor
local open_network_manager = launchers_controller.open_network_manager
local open_network_tui = launchers_controller.open_network_tui
local open_network_scan = launchers_controller.open_network_scan
local show_world_clock_popup = launchers_controller.show_world_clock_popup
local set_timezone_mumbai = launchers_controller.set_timezone_mumbai
local set_timezone_vancouver = launchers_controller.set_timezone_vancouver
local launch_program_palette = launchers_controller.launch_program_palette
local open_linux_control_center_for_client = launchers_controller.open_linux_control_center_for_client

local function autostart_desktop_applets()
    if not autostart_status_tray_applets then
        awful.spawn.with_shell([[
            pkill -u "$USER" -x nm-applet >/dev/null 2>&1 || true
            pkill -u "$USER" -x blueman-applet >/dev/null 2>&1 || true
        ]])
        return
    end
    awful.spawn.with_shell(string.format([=[
        theme_data_dir=%s
        export XDG_DATA_DIRS="$theme_data_dir:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"
        export GTK_ICON_THEME=LinuxUtilitiesStatus

        if command -v gsettings >/dev/null 2>&1; then
            gsettings set org.blueman.general symbolic-status-icons false >/dev/null 2>&1 || true
        fi

        pkill -u "$USER" -x nm-applet >/dev/null 2>&1 || true
        pkill -u "$USER" -x blueman-applet >/dev/null 2>&1 || true

        (sleep 0.2; nm-applet >/dev/null 2>&1 &) 
        (sleep 0.4; blueman-applet >/dev/null 2>&1 &)
    ]=], shell_quote(config_dir:gsub("/+$", ""))))
end

local function autostart_compositor()
    awful.spawn.with_shell([[
        if command -v picom >/dev/null 2>&1; then
            pgrep -u "$USER" -x picom >/dev/null 2>&1 || picom --config /dev/null >/dev/null 2>&1 &
        fi
    ]])
end

enable_audio_auto_switch()
autostart_compositor()
autostart_desktop_applets()
gears.timer({
    timeout = 10,
    autostart = true,
    call_now = true,
    callback = function()
        audio_controller.refresh(false)
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

local menu_notes = {
   { "notes app", launchers_controller.open_notes_app },
   { "daily note", function() launchers_controller.open_daily_note(os.date("*t")) end },
   { "tasks note", launchers_controller.open_tasks_note },
   { "notes folder", launchers_controller.open_notes_folder },
}

local menu_files = {
   { "home folder", launchers_controller.open_home_folder },
   { "LinuxUtilities folder", launchers_controller.open_workspace_folder },
   { "screenshots folder", launchers_controller.open_screenshots_folder },
   { "AppImage library", launchers_controller.open_appimage_library },
   { "AppImage launcher", launchers_controller.open_appimage_palette },
}

local menu_timezones = {
   { "show world clock", show_world_clock_popup },
   { "set Mumbai timezone", set_timezone_mumbai },
   { "set Vancouver timezone", set_timezone_vancouver },
   { "time preferences", launchers_controller.open_time_preferences },
}

local menu_network = {
   { "network settings", open_network_manager },
   { "Wi-Fi picker (nmtui)", open_network_tui },
   { "Wi-Fi scan (nmcli)", open_network_scan },
}

local menu_linuxutilities = {
   { "program palette", launch_program_palette },
   { "system monitor", open_system_monitor },
   { "network", menu_network },
   { "notes", menu_notes },
   { "files", menu_files },
   { "timezones", menu_timezones },
   { "calendar app", launchers_controller.open_calendar_app },
   { "Linux Control Center", function() launchers_controller.open_linux_control_center() end },
}

local menu_awesome = { "awesome", myawesomemenu, beautiful.awesome_icon }
local menu_linuxutilities_root = { "LinuxUtilities", menu_linuxutilities }
local menu_terminal = { "open terminal", terminal }

if has_fdo then
    mymainmenu = freedesktop.menu.build({
        before = { menu_awesome, menu_linuxutilities_root },
        after =  { menu_terminal }
    })
else
    mymainmenu = awful.menu({
        items = {
                  menu_awesome,
                  menu_linuxutilities_root,
                  { "Debian", debian.menu.Debian_menu.Debian },
                  menu_terminal,
                }
    })
end

-- Menubar configuration
menubar.utils.terminal = terminal -- Set the terminal for applications that require it
-- }}}

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

local function build_screen_profile(s)
    local width = 1920
    if s and s.geometry and s.geometry.width then
        width = s.geometry.width
    elseif s and s.workarea and s.workarea.width then
        width = s.workarea.width
    end

    if width >= 1800 then
        return {
            host_label = identity_label,
            network_label_len = 12,
            clock_format = " 🕒 %a %d %b %H:%M ",
            system_compact = false,
        }
    end

    if width >= 1440 then
        return {
            host_label = get_short_hostname(),
            network_label_len = 10,
            clock_format = " 🕒 %d %b %H:%M ",
            system_compact = false,
        }
    end

    return {
        host_label = get_short_hostname(),
        network_label_len = 8,
        clock_format = " 🕒 %H:%M ",
        system_compact = true,
    }
end

local function create_launcher_widget()
    return awful.widget.launcher({
        image = beautiful.awesome_icon,
        menu = mymainmenu,
    })
end

local function create_host_widget(profile)
    local label = profile and profile.host_label or identity_label
    local widget = wibox.widget.textbox(" 💻 " .. label .. " ")
    awful.tooltip({
        objects = { widget },
        text = "Current host identity\n" .. identity_label,
    })
    return widget
end

local function create_keyboard_widget()
    local widget = keyboardlayout_widget.new()
    awful.tooltip({
        objects = { widget },
        text = "Keyboard layout\nLeft click: next layout",
    })
    return widget
end

local primary_systray = wibox.widget.systray()
primary_systray:set_screen("primary")

-- Re-set wallpaper when a screen's geometry changes (e.g. different resolution)
screen.connect_signal("property::geometry", set_wallpaper)

awful.screen.connect_for_each_screen(function(s)
    local screen_profile = build_screen_profile(s)

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
    s.mylauncher = create_launcher_widget()
    s.mykeyboardlayout = create_keyboard_widget()
    s.myhostlabel = create_host_widget(screen_profile)
    s.myaudiostatus = create_audio_widget()
    s.mybatterystatus = create_battery_widget()
    s.mynetworkstatus = create_network_widget(screen_profile)
    s.mybluetoothstatus = create_bluetooth_widget()
    s.mysystemstatus = create_system_widget(screen_profile)
    s.myfoldernav = create_folder_widget()
    s.myapplicationsnav = create_applications_widget()
    s.mythunderbirdstatus = create_thunderbird_widget()
    s.myclock = create_clock_widget(screen_profile)
    s.mysystray = (s == screen.primary) and primary_systray or wibox.widget.textbox("")
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
            spacing = 2,
            s.mylauncher,
            s.mytaglist,
            s.mypromptbox,
        },
        s.mytasklist, -- Middle widget
        { -- Right widgets
            layout = wibox.layout.fixed.horizontal,
            spacing = 2,
            s.mybluetoothstatus,
            s.mynetworkstatus,
            s.mysystemstatus,
            s.mybatterystatus,
            s.myaudiostatus,
            s.mykeyboardlayout,
            s.mysystray,
            s.myapplicationsnav,
            s.myfoldernav,
            s.myhostlabel,
            s.mythunderbirdstatus,
            s.myclock,
            s.mylayoutbox,
        },
    }
end)
-- }}}

local binding_tables = bindings.build({
    awful = awful,
    gears = gears,
    hotkeys_popup = hotkeys_popup,
    menubar = menubar,
    awesome = awesome,
    client_global = client,
    mymainmenu = mymainmenu,
    modkey = modkey,
    terminal = terminal,
    flameshot_mouse_button = flameshot_mouse_button,
    utility_mouse_button = utility_mouse_button,
    actions = {
        open_flameshot = open_flameshot,
        open_system_monitor = open_system_monitor,
        show_world_clock_popup = show_world_clock_popup,
        set_timezone_mumbai = set_timezone_mumbai,
        set_timezone_vancouver = set_timezone_vancouver,
        open_linux_control_center_for_client = open_linux_control_center_for_client,
        volume_up = volume_up,
        volume_down = volume_down,
        volume_toggle_mute = volume_toggle_mute,
        brightness_up = brightness_up,
        brightness_down = brightness_down,
        media_play_pause = media_play_pause,
        media_next_track = media_next_track,
        media_prev_track = media_prev_track,
        launch_program_palette = launch_program_palette,
        toggle_gromit_draw = toggle_gromit_draw,
        clear_gromit_draw = clear_gromit_draw,
        undo_gromit_draw = undo_gromit_draw,
        redo_gromit_draw = redo_gromit_draw,
        toggle_gromit_visibility = toggle_gromit_visibility,
        quit_gromit = quit_gromit,
        presenter_dash_anchor = presenter_dash_anchor,
        presenter_dash_dash = presenter_dash_dash,
        presenter_dash_dot = presenter_dash_dot,
        presenter_dash_solid = presenter_dash_solid,
        presenter_dash_arrow = presenter_dash_arrow,
        presenter_dash_reset = presenter_dash_reset,
        toggle_cursor_spotlight = toggle_cursor_spotlight,
        adjust_cursor_spotlight = adjust_cursor_spotlight,
    },
})

root.buttons(binding_tables.root_buttons)

globalkeys = binding_tables.globalkeys
clientkeys = binding_tables.clientkeys
clientbuttons = binding_tables.clientbuttons

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
