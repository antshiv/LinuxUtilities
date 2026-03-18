local M = {}

function M.new(opts)
    local awful = assert(opts and opts.awful, "launchers.new requires awful")
    local gears = assert(opts and opts.gears, "launchers.new requires gears")
    local naughty = assert(opts and opts.naughty, "launchers.new requires naughty")
    local command_exists = assert(opts and opts.command_exists, "launchers.new requires command_exists")
    local shell_quote = assert(opts and opts.shell_quote, "launchers.new requires shell_quote")

    local flameshot_lock = false
    local controller = {}

    local function build_flameshot_target_file()
        local home = os.getenv("HOME") or ""
        local shots_dir = home ~= "" and (home .. "/Screenshots") or "Screenshots"
        local stamp = os.date("%Y-%m-%d_%H-%M-%S")
        return shots_dir, shots_dir .. "/Screenshot-" .. stamp .. ".png"
    end

    function controller.open_flameshot()
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

    function controller.launch_program_palette()
        local home = os.getenv("HOME") or ""
        local workspace_dir = os.getenv("LINUXUTILITIES_DIR")
        local launcher_script = nil
        local rofi_theme = nil

        if not workspace_dir or workspace_dir == "" then
            workspace_dir = home ~= "" and (home .. "/Workspace/LinuxUtilities") or "."
        end
        launcher_script = workspace_dir .. "/scripts/awesome_program_launcher.sh"
        rofi_theme = workspace_dir .. "/config/rofi_linuxutilities.rasi"

        if command_exists("rofi") and gears.filesystem.file_readable(launcher_script) then
            local theme_arg = ""
            if gears.filesystem.file_readable(rofi_theme) then
                theme_arg = " -theme " .. shell_quote(rofi_theme)
            end
            local cmd = string.format(
                "rofi -show-icons -i -matching fuzzy%s -show combi -combi-modi \"lcu,drun,run,window\" -modi \"lcu:%s,drun,run,window\"",
                theme_arg,
                shell_quote(launcher_script)
            )
            awful.spawn.with_shell(cmd)
            return
        end

        if command_exists("rofi") then
            awful.spawn.with_shell("rofi -show drun -show-icons -i")
            return
        end

        awful.screen.focused().mypromptbox:run()
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

        cleaned = cleaned:gsub("%s+%- Terminator$", "")
        cleaned = cleaned:gsub("%s+%- GNU/Linux$", "")

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

    function controller.open_linux_control_center(target_dir)
        local arg = ""
        if target_dir and target_dir ~= "" then
            arg = " " .. shell_quote(target_dir)
        end

        awful.spawn.easy_async_with_shell(string.format([[
            app=""
            for candidate in \
                "$HOME/Workspace/LinuxUtilities/build/bin/linux_control_center" \
                "$HOME/Programs/bin/linux_control_center"; do
                if [ -x "$candidate" ]; then
                    app="$candidate"
                    break
                fi
            done
            if [ -n "$app" ]; then
                "$app"%s >/dev/null 2>&1 &
            else
                exit 1
            fi
        ]], arg), function(_, _, _, exit_code)
            if exit_code ~= 0 then
                naughty.notify({
                    preset = naughty.config.presets.warn,
                    title = "Control Center Missing",
                    text = "Build LinuxUtilities/build/bin/linux_control_center or install it to ~/Programs/bin.",
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

    function controller.open_linux_control_center_for_client(c)
        local title_dir = extract_dir_from_client_title(c)

        if title_dir then
            notify_linux_control_center_pwd("PWD (title): " .. title_dir)
            controller.open_linux_control_center(title_dir)
            return
        end

        if not c or not c.pid then
            notify_linux_control_center_pwd("PWD: fallback to LinuxUtilities (no client PID)")
            controller.open_linux_control_center()
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
                controller.open_linux_control_center(cwd)
            else
                notify_linux_control_center_pwd("PWD: fallback to LinuxUtilities (could not resolve /proc/" .. c.pid .. "/cwd)")
                controller.open_linux_control_center()
            end
        end)
    end

    function controller.open_network_manager()
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

    function controller.open_bluetooth_manager()
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

    function controller.open_power_manager()
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

    function controller.open_calendar_app()
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

    function controller.open_time_preferences()
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

    function controller.open_thunderbird()
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

    function controller.open_thunderbird_calendar()
        awful.spawn({ "thunderbird", "-calendar" }, false)
    end

    return controller
end

return M
