local M = {}

function M.new(opts)
    local awful = assert(opts and opts.awful, "presenter.new requires awful")
    local gears = assert(opts and opts.gears, "presenter.new requires gears")
    local naughty = assert(opts and opts.naughty, "presenter.new requires naughty")
    local shell_quote = assert(opts and opts.shell_quote, "presenter.new requires shell_quote")
    local clamp_number = assert(opts and opts.clamp_number, "presenter.new requires clamp_number")

    local spotlight_toggle_lock = false
    local spotlight_radius = opts.spotlight_radius or 180
    local spotlight_dim = opts.spotlight_dim or 0.68
    local spotlight_fps = opts.spotlight_fps or 50
    local gromit_action_lock = false
    local controller = {}

    local function release_spotlight_toggle_lock()
        gears.timer.start_new(0.2, function()
            spotlight_toggle_lock = false
            return false
        end)
    end

    local function notify_spotlight_settings(prefix)
        naughty.notify({
            title = "Cursor Spotlight",
            text = string.format("%s Radius: %d  Dim: %.2f", prefix or "Settings", spotlight_radius, spotlight_dim),
            timeout = 1.8,
        })
    end

    local function handle_spotlight_start_result(stdout, exit_code)
        local result = (stdout or ""):gsub("%s+$", "")
        local failed_reason = nil

        if result:sub(1, 7) == "failed:" then
            failed_reason = result:sub(8)
            result = "failed"
        end

        if result == "missing" then
            naughty.notify({
                preset = naughty.config.presets.warn,
                title = "Cursor Spotlight Missing",
                text = "Build LinuxUtilities/build/bin/cursor_spotlight or install it to ~/Programs/bin.",
            })
        elseif exit_code ~= 0 or result == "failed" then
            naughty.notify({
                preset = naughty.config.presets.warn,
                title = "Cursor Spotlight Failed",
                text = failed_reason or "Start a compositor (for example: picom) and try again.",
            })
        end
    end

    local function start_cursor_spotlight(restart_existing, notify_on_start)
        local restart_cmd = ""
        if restart_existing then
            restart_cmd = "pkill -u \"$USER\" -f \"(^|/)cursor_spotlight( |$)\" >/dev/null 2>&1 || true\nsleep 0.1\n"
        end

        awful.spawn.easy_async_with_shell(string.format([[
            spot_pat='(^|/)cursor_spotlight( |$)'
            bin=""
            for candidate in \
                "$HOME/Programs/bin/cursor_spotlight" \
                "$HOME/Workspace/LinuxUtilities/build/bin/cursor_spotlight"; do
                if [ -x "$candidate" ]; then
                    bin="$candidate"
                    break
                fi
            done
            errfile="${XDG_RUNTIME_DIR:-/tmp}/cursor_spotlight.err"
            if [ -z "$bin" ]; then
                echo missing
                exit 2
            fi
            if ! pgrep -u "$USER" -x picom >/dev/null 2>&1; then
                if command -v picom >/dev/null 2>&1; then
                    picom --config /dev/null >/dev/null 2>&1 &
                    sleep 0.4
                fi
            fi
            %s: >"$errfile"
            "$bin" --radius %d --dim %.2f --fps %d 2>"$errfile" >/dev/null &
            sleep 0.2
            if pgrep -u "$USER" -f "$spot_pat" >/dev/null 2>&1; then
                echo started
            else
                reason="$(head -n1 "$errfile" 2>/dev/null || true)"
                if [ -n "$reason" ]; then
                    printf 'failed:%%s\n' "$reason"
                else
                    echo failed
                fi
                exit 3
            fi
        ]], restart_cmd, spotlight_radius, spotlight_dim, spotlight_fps), function(stdout, _, _, exit_code)
            local result = (stdout or ""):gsub("%s+$", "")
            if notify_on_start and exit_code == 0 and result == "started" then
                naughty.notify({
                    title = "Cursor Spotlight",
                    text = "Enabled.",
                    timeout = 1.2,
                })
            end
            handle_spotlight_start_result(stdout, exit_code)
            release_spotlight_toggle_lock()
        end)
    end

    function controller.toggle_cursor_spotlight()
        if spotlight_toggle_lock then
            return
        end
        spotlight_toggle_lock = true

        awful.spawn.easy_async_with_shell([[
            if pgrep -u "$USER" -f '(^|/)cursor_spotlight( |$)' >/dev/null 2>&1; then
                echo running
            else
                echo stopped
            fi
        ]], function(stdout)
            local status = (stdout or ""):gsub("%s+$", "")
            if status == "running" then
                awful.spawn.easy_async_with_shell("pkill -u \"$USER\" -f '(^|/)cursor_spotlight( |$)' >/dev/null 2>&1 || true", function()
                    naughty.notify({
                        title = "Cursor Spotlight",
                        text = "Disabled.",
                        timeout = 1.2,
                    })
                    release_spotlight_toggle_lock()
                end)
            else
                start_cursor_spotlight(false, true)
            end
        end)
    end

    function controller.adjust_cursor_spotlight(radius_delta, dim_delta)
        spotlight_radius = math.floor(clamp_number(spotlight_radius + (radius_delta or 0), 80, 520))
        spotlight_dim = clamp_number(spotlight_dim + (dim_delta or 0), 0.10, 0.90)
        spotlight_dim = math.floor(spotlight_dim * 100 + 0.5) / 100
        notify_spotlight_settings("Updated.")

        awful.spawn.easy_async_with_shell([[
            if pgrep -u "$USER" -f '(^|/)cursor_spotlight( |$)' >/dev/null 2>&1; then
                echo running
            else
                echo stopped
            fi
        ]], function(stdout)
            local status = (stdout or ""):gsub("%s+$", "")
            if status ~= "running" or spotlight_toggle_lock then
                return
            end
            spotlight_toggle_lock = true
            start_cursor_spotlight(true, false)
        end)
    end

    local function release_gromit_action_lock()
        gears.timer.start_new(0.15, function()
            gromit_action_lock = false
            return false
        end)
    end

    local function notify_gromit_result(text, is_warn)
        naughty.notify({
            preset = is_warn and naughty.config.presets.warn or nil,
            title = "Gromit Presenter",
            text = text,
            timeout = 1.4,
        })
    end

    local function control_gromit_mpx(control_flag, success_text)
        if gromit_action_lock then
            return
        end
        gromit_action_lock = true

        awful.spawn.easy_async_with_shell(string.format([[
            if ! command -v gromit-mpx >/dev/null 2>&1; then
                echo missing
                exit 2
            fi
            if ! pgrep -u "$USER" -x gromit-mpx >/dev/null 2>&1; then
                gromit-mpx --key F6 --undo-key F5 >/dev/null 2>&1 &
                sleep 0.25
            fi
            if ! pgrep -u "$USER" -x gromit-mpx >/dev/null 2>&1; then
                echo failed
                exit 3
            fi
            gromit-mpx %s >/dev/null 2>&1 || exit 4
            echo ok
        ]], control_flag), function(stdout, _, _, exit_code)
            local result = (stdout or ""):gsub("%s+$", "")
            if result == "missing" then
                notify_gromit_result("Install gromit-mpx first.", true)
            elseif exit_code ~= 0 or result ~= "ok" then
                notify_gromit_result("Gromit command failed.", true)
            elseif success_text and success_text ~= "" then
                notify_gromit_result(success_text, false)
            end
            release_gromit_action_lock()
        end)
    end

    function controller.toggle_gromit_draw()
        control_gromit_mpx("-t", "Draw mode toggled.")
    end

    function controller.clear_gromit_draw()
        control_gromit_mpx("-c", "Drawings cleared.")
    end

    function controller.undo_gromit_draw()
        control_gromit_mpx("-z", "Undid last stroke.")
    end

    function controller.redo_gromit_draw()
        control_gromit_mpx("-y", "Redid stroke.")
    end

    function controller.toggle_gromit_visibility()
        control_gromit_mpx("-v", "Overlay visibility toggled.")
    end

    function controller.quit_gromit()
        control_gromit_mpx("-q", "Gromit overlay stopped.")
    end

    local function run_presenter_dash(mode)
        awful.spawn.easy_async_with_shell(string.format([[
            script="$HOME/Workspace/LinuxUtilities/presenter_dash.sh"
            if [ ! -x "$script" ]; then
                echo missing
                exit 2
            fi
            "$script" %s >/dev/null 2>&1
        ]], shell_quote(mode or "dash")), function(stdout, _, _, exit_code)
            local result = (stdout or ""):gsub("%s+$", "")
            if result == "missing" or exit_code ~= 0 then
                naughty.notify({
                    preset = naughty.config.presets.warn,
                    title = "Presenter Dash",
                    text = "Install xdotool and keep LinuxUtilities/presenter_dash.sh executable.",
                })
            end
        end)
    end

    function controller.presenter_dash_anchor()
        run_presenter_dash("anchor")
    end

    function controller.presenter_dash_dash()
        run_presenter_dash("dash")
    end

    function controller.presenter_dash_dot()
        run_presenter_dash("dot")
    end

    function controller.presenter_dash_solid()
        run_presenter_dash("solid")
    end

    function controller.presenter_dash_arrow()
        run_presenter_dash("arrow")
    end

    function controller.presenter_dash_reset()
        run_presenter_dash("reset")
    end

    return controller
end

return M
