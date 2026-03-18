local M = {}

local function join_with_table(gears, ...)
    return gears.table.join(...)
end

local function build_root_buttons(opts)
    local awful = opts.awful
    local gears = opts.gears
    local client_global = opts.client_global
    local modkey = opts.modkey
    local mymainmenu = opts.mymainmenu
    local flameshot_mouse_button = opts.flameshot_mouse_button
    local utility_mouse_button = opts.utility_mouse_button
    local actions = opts.actions

    return join_with_table(gears,
        awful.button({ }, 3, function()
            mymainmenu:toggle()
        end),
        awful.button({ }, 4, awful.tag.viewnext),
        awful.button({ }, 5, awful.tag.viewprev),
        awful.button({ }, flameshot_mouse_button, actions.open_flameshot),
        awful.button({ }, utility_mouse_button, function()
            actions.open_linux_control_center_for_client(client_global.focus)
        end),
        awful.button({ modkey }, 2, actions.volume_toggle_mute),
        awful.button({ modkey }, 4, actions.volume_up),
        awful.button({ modkey }, 5, actions.volume_down)
    )
end

local function build_client_keys(opts)
    local awful = opts.awful
    local gears = opts.gears
    local modkey = opts.modkey

    return join_with_table(gears,
        awful.key({ modkey }, "f",
            function(c)
                c.fullscreen = not c.fullscreen
                c:raise()
            end,
            { description = "toggle fullscreen", group = "client" }),
        awful.key({ modkey, "Shift" }, "c", function(c)
            c:kill()
        end, { description = "close", group = "client" }),
        awful.key({ modkey, "Control" }, "space", awful.client.floating.toggle,
            { description = "toggle floating", group = "client" }),
        awful.key({ modkey, "Control" }, "Return", function(c)
            c:swap(awful.client.getmaster())
        end, { description = "move to master", group = "client" }),
        awful.key({ modkey }, "o", function(c)
            c:move_to_screen()
        end, { description = "move to screen", group = "client" }),
        awful.key({ modkey }, "t", function(c)
            c.ontop = not c.ontop
        end, { description = "toggle keep on top", group = "client" }),
        awful.key({ modkey }, "n", function(c)
            c.minimized = true
        end, { description = "minimize", group = "client" }),
        awful.key({ modkey }, "m", function(c)
            c.maximized = not c.maximized
            c:raise()
        end, { description = "(un)maximize", group = "client" }),
        awful.key({ modkey, "Control" }, "m", function(c)
            c.maximized_vertical = not c.maximized_vertical
            c:raise()
        end, { description = "(un)maximize vertically", group = "client" }),
        awful.key({ modkey, "Shift" }, "m", function(c)
            c.maximized_horizontal = not c.maximized_horizontal
            c:raise()
        end, { description = "(un)maximize horizontally", group = "client" })
    )
end

local function add_tag_number_keys(globalkeys, opts)
    local awful = opts.awful
    local gears = opts.gears
    local client_global = opts.client_global
    local modkey = opts.modkey

    for i = 1, 9 do
        globalkeys = join_with_table(gears, globalkeys,
            awful.key({ modkey }, "#" .. i + 9,
                function()
                    local screen = awful.screen.focused()
                    local tag = screen.tags[i]
                    if tag then
                        tag:view_only()
                    end
                end,
                { description = "view tag #" .. i, group = "tag" }),
            awful.key({ modkey, "Control" }, "#" .. i + 9,
                function()
                    local screen = awful.screen.focused()
                    local tag = screen.tags[i]
                    if tag then
                        awful.tag.viewtoggle(tag)
                    end
                end,
                { description = "toggle tag #" .. i, group = "tag" }),
            awful.key({ modkey, "Shift" }, "#" .. i + 9,
                function()
                    if client_global.focus then
                        local tag = client_global.focus.screen.tags[i]
                        if tag then
                            client_global.focus:move_to_tag(tag)
                        end
                    end
                end,
                { description = "move focused client to tag #" .. i, group = "tag" }),
            awful.key({ modkey, "Control", "Shift" }, "#" .. i + 9,
                function()
                    if client_global.focus then
                        local tag = client_global.focus.screen.tags[i]
                        if tag then
                            client_global.focus:toggle_tag(tag)
                        end
                    end
                end,
                { description = "toggle focused client on tag #" .. i, group = "tag" })
        )
    end

    return globalkeys
end

local function build_global_keys(opts)
    local awful = opts.awful
    local gears = opts.gears
    local hotkeys_popup = opts.hotkeys_popup
    local menubar = opts.menubar
    local awesome = opts.awesome
    local client_global = opts.client_global
    local modkey = opts.modkey
    local terminal = opts.terminal
    local mymainmenu = opts.mymainmenu
    local actions = opts.actions

    local globalkeys = join_with_table(gears,
        awful.key({ modkey }, "s", hotkeys_popup.show_help,
            { description = "show help", group = "awesome" }),
        awful.key({ modkey }, "Left", awful.tag.viewprev,
            { description = "view previous", group = "tag" }),
        awful.key({ modkey }, "Right", awful.tag.viewnext,
            { description = "view next", group = "tag" }),
        awful.key({ modkey }, "Escape", awful.tag.history.restore,
            { description = "go back", group = "tag" }),

        awful.key({ modkey }, "j", function()
            awful.client.focus.byidx(1)
        end, { description = "focus next by index", group = "client" }),
        awful.key({ modkey }, "k", function()
            awful.client.focus.byidx(-1)
        end, { description = "focus previous by index", group = "client" }),
        awful.key({ modkey }, "w", function()
            mymainmenu:show()
        end, { description = "show main menu", group = "awesome" }),

        awful.key({ modkey, "Shift" }, "j", function()
            awful.client.swap.byidx(1)
        end, { description = "swap with next client by index", group = "client" }),
        awful.key({ modkey, "Shift" }, "k", function()
            awful.client.swap.byidx(-1)
        end, { description = "swap with previous client by index", group = "client" }),
        awful.key({ modkey, "Control" }, "j", function()
            awful.screen.focus_relative(1)
        end, { description = "focus the next screen", group = "screen" }),
        awful.key({ modkey, "Control" }, "k", function()
            awful.screen.focus_relative(-1)
        end, { description = "focus the previous screen", group = "screen" }),
        awful.key({ modkey }, "u", awful.client.urgent.jumpto,
            { description = "jump to urgent client", group = "client" }),
        awful.key({ modkey }, "Tab", function()
            awful.client.focus.history.previous()
            if client_global.focus then
                client_global.focus:raise()
            end
        end, { description = "go back", group = "client" }),

        awful.key({ modkey }, "Return", function()
            awful.spawn(terminal)
        end, { description = "open a terminal", group = "launcher" }),
        awful.key({ }, "F6", actions.toggle_gromit_draw,
            { description = "toggle presenter drawing (gromit)", group = "launcher" }),
        awful.key({ "Shift" }, "F6", actions.clear_gromit_draw,
            { description = "clear presenter drawing (gromit)", group = "launcher" }),
        awful.key({ "Control" }, "F6", actions.undo_gromit_draw,
            { description = "undo presenter stroke (gromit)", group = "launcher" }),
        awful.key({ "Control", "Shift" }, "F6", actions.redo_gromit_draw,
            { description = "redo presenter stroke (gromit)", group = "launcher" }),
        awful.key({ "Mod1" }, "F6", actions.toggle_gromit_visibility,
            { description = "toggle presenter overlay visibility (gromit)", group = "launcher" }),
        awful.key({ "Control", "Mod1" }, "F6", actions.quit_gromit,
            { description = "quit presenter overlay (gromit)", group = "launcher" }),
        awful.key({ "Mod1" }, "F11", actions.presenter_dash_anchor,
            { description = "set presenter dash anchor", group = "launcher" }),
        awful.key({ }, "F11", actions.presenter_dash_dash,
            { description = "draw animated dashed segment", group = "launcher" }),
        awful.key({ "Shift" }, "F11", actions.presenter_dash_dot,
            { description = "draw animated dotted segment", group = "launcher" }),
        awful.key({ "Control" }, "F11", actions.presenter_dash_solid,
            { description = "draw animated solid segment", group = "launcher" }),
        awful.key({ modkey }, "F11", actions.presenter_dash_arrow,
            { description = "draw animated arrow segment", group = "launcher" }),
        awful.key({ "Control", "Mod1" }, "F11", actions.presenter_dash_reset,
            { description = "reset presenter dash anchor", group = "launcher" }),
        awful.key({ }, "F7", actions.toggle_cursor_spotlight,
            { description = "toggle cursor spotlight", group = "launcher" }),
        awful.key({ }, "F9", function()
            actions.adjust_cursor_spotlight(0, -0.05)
        end, { description = "spotlight less dim", group = "launcher" }),
        awful.key({ }, "F10", function()
            actions.adjust_cursor_spotlight(0, 0.05)
        end, { description = "spotlight more dim", group = "launcher" }),
        awful.key({ "Shift" }, "F9", function()
            actions.adjust_cursor_spotlight(-20, 0)
        end, { description = "spotlight smaller radius", group = "launcher" }),
        awful.key({ "Shift" }, "F10", function()
            actions.adjust_cursor_spotlight(20, 0)
        end, { description = "spotlight larger radius", group = "launcher" }),
        awful.key({ modkey }, "g", actions.toggle_cursor_spotlight,
            { description = "toggle cursor spotlight (fallback)", group = "launcher" }),
        awful.key({ modkey }, "bracketleft", function()
            actions.adjust_cursor_spotlight(0, -0.05)
        end, { description = "spotlight less dim (fallback)", group = "launcher" }),
        awful.key({ modkey }, "bracketright", function()
            actions.adjust_cursor_spotlight(0, 0.05)
        end, { description = "spotlight more dim (fallback)", group = "launcher" }),
        awful.key({ modkey }, "minus", function()
            actions.adjust_cursor_spotlight(-20, 0)
        end, { description = "spotlight smaller radius (fallback)", group = "launcher" }),
        awful.key({ modkey }, "equal", function()
            actions.adjust_cursor_spotlight(20, 0)
        end, { description = "spotlight larger radius (fallback)", group = "launcher" }),
        awful.key({ "Control", "Mod1" }, "g", actions.toggle_cursor_spotlight,
            { description = "toggle cursor spotlight (Ctrl+Alt fallback)", group = "launcher" }),
        awful.key({ "Control", "Mod1" }, "bracketleft", function()
            actions.adjust_cursor_spotlight(0, -0.05)
        end, { description = "spotlight less dim (Ctrl+Alt fallback)", group = "launcher" }),
        awful.key({ "Control", "Mod1" }, "bracketright", function()
            actions.adjust_cursor_spotlight(0, 0.05)
        end, { description = "spotlight more dim (Ctrl+Alt fallback)", group = "launcher" }),
        awful.key({ "Control", "Mod1" }, "minus", function()
            actions.adjust_cursor_spotlight(-20, 0)
        end, { description = "spotlight smaller radius (Ctrl+Alt fallback)", group = "launcher" }),
        awful.key({ "Control", "Mod1" }, "equal", function()
            actions.adjust_cursor_spotlight(20, 0)
        end, { description = "spotlight larger radius (Ctrl+Alt fallback)", group = "launcher" }),
        awful.key({ }, "F8", actions.open_flameshot,
            { description = "open flameshot", group = "launcher" }),
        awful.key({ }, "Print", actions.open_flameshot,
            { description = "open flameshot", group = "launcher" }),
        awful.key({ }, "XF86AudioRaiseVolume", actions.volume_up,
            { description = "increase volume", group = "media" }),
        awful.key({ }, "XF86AudioLowerVolume", actions.volume_down,
            { description = "decrease volume", group = "media" }),
        awful.key({ }, "XF86AudioMute", actions.volume_toggle_mute,
            { description = "toggle mute", group = "media" }),
        awful.key({ }, "XF86MonBrightnessUp", actions.brightness_up,
            { description = "increase brightness", group = "media" }),
        awful.key({ }, "XF86MonBrightnessDown", actions.brightness_down,
            { description = "decrease brightness", group = "media" }),
        awful.key({ }, "F12", actions.brightness_up,
            { description = "increase brightness (fallback)", group = "media" }),
        awful.key({ "Shift" }, "F12", actions.brightness_down,
            { description = "decrease brightness (fallback)", group = "media" }),
        awful.key({ }, "XF86AudioPlay", actions.media_play_pause,
            { description = "play/pause active media", group = "media" }),
        awful.key({ }, "XF86AudioPause", actions.media_play_pause,
            { description = "play/pause active media", group = "media" }),
        awful.key({ }, "XF86AudioNext", actions.media_next_track,
            { description = "next track", group = "media" }),
        awful.key({ }, "XF86AudioPrev", actions.media_prev_track,
            { description = "previous track", group = "media" }),
        awful.key({ modkey, "Control" }, "r", awesome.restart,
            { description = "reload awesome", group = "awesome" }),
        awful.key({ modkey, "Shift" }, "q", awesome.quit,
            { description = "quit awesome", group = "awesome" }),

        awful.key({ modkey }, "l", function()
            awful.tag.incmwfact(0.05)
        end, { description = "increase master width factor", group = "layout" }),
        awful.key({ modkey }, "h", function()
            awful.tag.incmwfact(-0.05)
        end, { description = "decrease master width factor", group = "layout" }),
        awful.key({ modkey, "Shift" }, "h", function()
            awful.tag.incnmaster(1, nil, true)
        end, { description = "increase the number of master clients", group = "layout" }),
        awful.key({ modkey, "Shift" }, "l", function()
            awful.tag.incnmaster(-1, nil, true)
        end, { description = "decrease the number of master clients", group = "layout" }),
        awful.key({ modkey, "Control" }, "h", function()
            awful.tag.incncol(1, nil, true)
        end, { description = "increase the number of columns", group = "layout" }),
        awful.key({ modkey, "Control" }, "l", function()
            awful.tag.incncol(-1, nil, true)
        end, { description = "decrease the number of columns", group = "layout" }),
        awful.key({ modkey }, "space", function()
            awful.layout.inc(1)
        end, { description = "select next", group = "layout" }),
        awful.key({ modkey, "Shift" }, "space", function()
            awful.layout.inc(-1)
        end, { description = "select previous", group = "layout" }),

        awful.key({ modkey, "Control" }, "n", function()
            local c = awful.client.restore()
            if c then
                c:emit_signal("request::activate", "key.unminimize", { raise = true })
            end
        end, { description = "restore minimized", group = "client" }),

        awful.key({ modkey }, "r", actions.launch_program_palette,
            { description = "program launcher (rofi + custom commands)", group = "launcher" }),
        awful.key({ modkey, "Shift" }, "r", function()
            awful.screen.focused().mypromptbox:run()
        end, { description = "run prompt (classic)", group = "launcher" }),
        awful.key({ modkey }, "x", function()
            awful.prompt.run({
                prompt = "Run Lua code: ",
                textbox = awful.screen.focused().mypromptbox.widget,
                exe_callback = awful.util.eval,
                history_path = awful.util.get_cache_dir() .. "/history_eval",
            })
        end, { description = "lua execute prompt", group = "awesome" }),
        awful.key({ modkey }, "p", function()
            menubar.show()
        end, { description = "show the menubar", group = "launcher" })
    )

    return add_tag_number_keys(globalkeys, opts)
end

local function build_client_buttons(opts)
    local awful = opts.awful
    local gears = opts.gears
    local modkey = opts.modkey
    local flameshot_mouse_button = opts.flameshot_mouse_button
    local utility_mouse_button = opts.utility_mouse_button
    local actions = opts.actions

    return join_with_table(gears,
        awful.button({ }, 1, function(c)
            c:emit_signal("request::activate", "mouse_click", { raise = true })
        end),
        awful.button({ }, flameshot_mouse_button, function(c)
            c:emit_signal("request::activate", "mouse_click", { raise = false })
            actions.open_flameshot()
        end),
        awful.button({ }, utility_mouse_button, function(c)
            c:emit_signal("request::activate", "mouse_click", { raise = false })
            actions.open_linux_control_center_for_client(c)
        end),
        awful.button({ modkey }, 1, function(c)
            c:emit_signal("request::activate", "mouse_click", { raise = true })
            awful.mouse.client.move(c)
        end),
        awful.button({ modkey }, 3, function(c)
            c:emit_signal("request::activate", "mouse_click", { raise = true })
            awful.mouse.client.resize(c)
        end)
    )
end

function M.build(opts)
    local awful = assert(opts and opts.awful, "bindings.build requires awful")
    local gears = assert(opts and opts.gears, "bindings.build requires gears")
    assert(opts and opts.hotkeys_popup, "bindings.build requires hotkeys_popup")
    assert(opts and opts.menubar, "bindings.build requires menubar")
    assert(opts and opts.awesome, "bindings.build requires awesome")
    assert(opts and opts.client_global, "bindings.build requires client_global")
    assert(opts and opts.mymainmenu, "bindings.build requires mymainmenu")
    assert(opts and opts.modkey, "bindings.build requires modkey")
    assert(opts and opts.terminal, "bindings.build requires terminal")
    assert(opts and opts.actions, "bindings.build requires actions")

    local binding_opts = {
        awful = awful,
        gears = gears,
        hotkeys_popup = opts.hotkeys_popup,
        menubar = opts.menubar,
        awesome = opts.awesome,
        client_global = opts.client_global,
        mymainmenu = opts.mymainmenu,
        modkey = opts.modkey,
        terminal = opts.terminal,
        flameshot_mouse_button = opts.flameshot_mouse_button or 8,
        utility_mouse_button = opts.utility_mouse_button or 9,
        actions = opts.actions,
    }

    return {
        root_buttons = build_root_buttons(binding_opts),
        globalkeys = build_global_keys(binding_opts),
        clientkeys = build_client_keys(binding_opts),
        clientbuttons = build_client_buttons(binding_opts),
    }
end

return M
