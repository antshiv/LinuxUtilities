local M = {}

function M.new(opts)
    local awful = assert(opts and opts.awful, "launchers.new requires awful")
    local gears = assert(opts and opts.gears, "launchers.new requires gears")
    local naughty = assert(opts and opts.naughty, "launchers.new requires naughty")
    local command_exists = assert(opts and opts.command_exists, "launchers.new requires command_exists")
    local shell_quote = assert(opts and opts.shell_quote, "launchers.new requires shell_quote")

    local flameshot_lock = false
    local controller = {}

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

    local function notes_root()
        local explicit = expand_tilde_path(os.getenv("LINUXUTILS_NOTES_DIR"))
        if explicit and explicit ~= "" then
            return explicit
        end
        local home = os.getenv("HOME") or ""
        if home ~= "" then
            return home .. "/Workspace/ShivasNotes"
        end
        return "ShivasNotes"
    end

    local function daily_note_path(date_table)
        local ts = os.time(date_table or os.date("*t"))
        return notes_root() .. "/daily/" .. os.date("%Y-%m-%d", ts) .. ".md"
    end

    local function tasks_note_path()
        return notes_root() .. "/tasks.md"
    end

    local function path_basename(path)
        if not path or path == "" then
            return ""
        end
        local trimmed = tostring(path):gsub("/+$", "")
        return trimmed:match("([^/]+)$") or trimmed
    end

    local function uri_encode(text)
        return (tostring(text or ""):gsub("([^%w%-%._~])", function(char)
            return string.format("%%%02X", string.byte(char))
        end))
    end

    local function notes_vault_name()
        return path_basename(notes_root())
    end

    local function obsidian_vault_uri()
        return "obsidian://open?vault=" .. uri_encode(notes_vault_name())
    end

    local function obsidian_note_uri(path)
        return "obsidian://open?path=" .. uri_encode(path or "")
    end

    local function build_flameshot_target_file()
        local home = os.getenv("HOME") or ""
        local shots_dir = home ~= "" and (home .. "/Screenshots") or "Screenshots"
        local stamp = os.date("%Y-%m-%d_%H-%M-%S")
        return shots_dir, shots_dir .. "/Screenshot-" .. stamp .. ".png"
    end

    local function workspace_root()
        local explicit = expand_tilde_path(os.getenv("LINUXUTILITIES_DIR"))
        if explicit and explicit ~= "" then
            return explicit
        end
        local home = os.getenv("HOME") or ""
        if home ~= "" then
            return home .. "/Workspace/LinuxUtilities"
        end
        return "."
    end

    local function directory_exists(path)
        local ok, _, code = os.execute("[ -d " .. shell_quote(path or "") .. " ] >/dev/null 2>&1")
        return ok == true or code == 0
    end

    local function appimage_root()
        local explicit = expand_tilde_path(os.getenv("LINUXUTILS_APPIMAGE_DIR"))
        if explicit and explicit ~= "" then
            return explicit
        end

        local home = os.getenv("HOME") or ""
        if home ~= "" then
            local uppercase = home .. "/Programs/AppImage"
            local lowercase = home .. "/Programs/appimage"
            if directory_exists(uppercase) then
                return uppercase
            end
            return lowercase
        end

        return "AppImage"
    end

    local function launcher_assets()
        local home = os.getenv("HOME") or ""
        local workspace_dir = expand_tilde_path(os.getenv("LINUXUTILITIES_DIR"))
        if not workspace_dir or workspace_dir == "" then
            workspace_dir = home ~= "" and (home .. "/Workspace/LinuxUtilities") or "."
        end

        return workspace_dir, workspace_dir .. "/scripts/awesome_program_launcher.sh", workspace_dir .. "/config/rofi_linuxutilities.rasi"
    end

    local function parse_key_values(stdout)
        local values = {}
        for key, value in tostring(stdout or ""):gmatch("([A-Z_]+)=([^\n]*)") do
            values[key] = value
        end
        return values
    end

    local function world_clock_snapshot(callback)
        awful.spawn.easy_async_with_shell([=[
            local_zone="$(timedatectl show -p Timezone --value 2>/dev/null | head -n1)"
            [ -n "$local_zone" ] || local_zone="$(date +%Z)"
            printf 'LOCAL_ZONE=%s\n' "$local_zone"
            printf 'LOCAL_TIME=%s\n' "$(date '+%a %d %b %H:%M %Z')"
            printf 'MUMBAI_TIME=%s\n' "$(TZ=Asia/Kolkata date '+%a %d %b %H:%M %Z')"
            printf 'VANCOUVER_TIME=%s\n' "$(TZ=America/Vancouver date '+%a %d %b %H:%M %Z')"
        ]=], function(stdout)
            if callback then
                callback(parse_key_values(stdout))
            end
        end)
    end

    local function switch_timezone(tzid, label)
        awful.spawn.easy_async_with_shell(string.format([=[
            tz=%s
            if ! command -v timedatectl >/dev/null 2>&1; then
                exit 2
            fi

            if timedatectl set-timezone "$tz" >/dev/null 2>&1; then
                exit 0
            fi

            if command -v pkexec >/dev/null 2>&1; then
                pkexec timedatectl set-timezone "$tz" >/dev/null 2>&1
                exit $?
            fi

            exit 1
        ]=], shell_quote(tzid)), function(_, _, _, exit_code)
            if exit_code == 0 then
                world_clock_snapshot(function(snapshot)
                    naughty.notify({
                        title = "Timezone Updated",
                        text = table.concat({
                            "System timezone: " .. (snapshot.LOCAL_ZONE or label),
                            "Local: " .. (snapshot.LOCAL_TIME or label),
                            "Mumbai: " .. (snapshot.MUMBAI_TIME or "n/a"),
                            "Vancouver: " .. (snapshot.VANCOUVER_TIME or "n/a"),
                        }, "\n"),
                        timeout = 6,
                    })
                end)
                return
            end

            naughty.notify({
                preset = naughty.config.presets.warn,
                title = "Timezone Change Failed",
                text = "Could not switch to " .. label .. ". Use timedatectl or install pkexec/polkit support.",
            })
        end)
    end

    local function open_directory(path, label)
        awful.spawn.easy_async_with_shell(string.format([=[
            target=%s
            mkdir -p "$target"

            preferred="${LINUXUTILS_FOLDERS_APP:-auto}"
            if [ "$preferred" != "gui" ] && command -v x-terminal-emulator >/dev/null 2>&1; then
                if [ "$preferred" = "yazi" ] && command -v yazi >/dev/null 2>&1; then
                    x-terminal-emulator -e sh -lc 'cd "$1" && exec yazi "$1"' sh "$target" >/dev/null 2>&1 &
                    exit 0
                elif [ "$preferred" = "ranger" ] && command -v ranger >/dev/null 2>&1; then
                    x-terminal-emulator -e sh -lc 'cd "$1" && exec ranger "$1"' sh "$target" >/dev/null 2>&1 &
                    exit 0
                elif [ "$preferred" = "lf" ] && command -v lf >/dev/null 2>&1; then
                    x-terminal-emulator -e sh -lc 'cd "$1" && exec lf "$1"' sh "$target" >/dev/null 2>&1 &
                    exit 0
                elif [ "$preferred" = "auto" ]; then
                    if command -v yazi >/dev/null 2>&1; then
                        x-terminal-emulator -e sh -lc 'cd "$1" && exec yazi "$1"' sh "$target" >/dev/null 2>&1 &
                        exit 0
                    elif command -v ranger >/dev/null 2>&1; then
                        x-terminal-emulator -e sh -lc 'cd "$1" && exec ranger "$1"' sh "$target" >/dev/null 2>&1 &
                        exit 0
                    elif command -v lf >/dev/null 2>&1; then
                        x-terminal-emulator -e sh -lc 'cd "$1" && exec lf "$1"' sh "$target" >/dev/null 2>&1 &
                        exit 0
                    fi
                fi
            fi

            if command -v xdg-open >/dev/null 2>&1; then
                xdg-open "$target" >/dev/null 2>&1 &
            elif command -v gio >/dev/null 2>&1; then
                gio open "$target" >/dev/null 2>&1 &
            elif command -v x-terminal-emulator >/dev/null 2>&1; then
                x-terminal-emulator -e sh -lc 'cd "$1" && exec "${SHELL:-bash}"' sh "$target" >/dev/null 2>&1 &
            else
                exit 1
            fi
        ]=], shell_quote(path)), function(_, _, _, exit_code)
            if exit_code ~= 0 then
                naughty.notify({
                    preset = naughty.config.presets.warn,
                    title = "Folder Opener Missing",
                    text = "Install yazi/ranger/lf, xdg-open, or gio to open " .. label .. ".",
                })
            end
        end)
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

    function controller.open_home_folder()
        open_directory(os.getenv("HOME") or ".", "home folder")
    end

    function controller.open_workspace_folder()
        open_directory(workspace_root(), "LinuxUtilities folder")
    end

    function controller.open_screenshots_folder()
        local screenshots_dir = (os.getenv("HOME") or "") .. "/Screenshots"
        if screenshots_dir == "/Screenshots" then
            screenshots_dir = "Screenshots"
        end
        open_directory(screenshots_dir, "screenshots folder")
    end

    function controller.open_notes_folder()
        open_directory(notes_root(), "notes folder")
    end

    function controller.open_appimage_library()
        open_directory(appimage_root(), "AppImage library")
    end

    function controller.open_system_monitor()
        awful.spawn.easy_async_with_shell([=[
            if command -v x-terminal-emulator >/dev/null 2>&1; then
                if command -v btop >/dev/null 2>&1; then
                    x-terminal-emulator -e btop >/dev/null 2>&1 &
                elif command -v htop >/dev/null 2>&1; then
                    x-terminal-emulator -e htop >/dev/null 2>&1 &
                else
                    x-terminal-emulator -e top >/dev/null 2>&1 &
                fi
            elif command -v btop >/dev/null 2>&1; then
                btop >/dev/null 2>&1 &
            elif command -v htop >/dev/null 2>&1; then
                htop >/dev/null 2>&1 &
            elif command -v top >/dev/null 2>&1; then
                top >/dev/null 2>&1 &
            else
                exit 1
            fi
        ]=], function(_, _, _, exit_code)
            if exit_code ~= 0 then
                naughty.notify({
                    preset = naughty.config.presets.warn,
                    title = "System Monitor Missing",
                    text = "Install btop or htop, or ensure a terminal is available for top.",
                })
            end
        end)
    end

    function controller.show_world_clock_popup()
        world_clock_snapshot(function(snapshot)
            naughty.notify({
                title = "World Clock",
                text = table.concat({
                    "System timezone: " .. (snapshot.LOCAL_ZONE or "unknown"),
                    "Local: " .. (snapshot.LOCAL_TIME or "n/a"),
                    "Mumbai: " .. (snapshot.MUMBAI_TIME or "n/a"),
                    "Vancouver: " .. (snapshot.VANCOUVER_TIME or "n/a"),
                    "",
                    "Mod4+Ctrl+i: set Mumbai",
                    "Mod4+Ctrl+v: set Vancouver",
                }, "\n"),
                timeout = 8,
            })
        end)
    end

    function controller.set_timezone_mumbai()
        switch_timezone("Asia/Kolkata", "Mumbai (Asia/Kolkata)")
    end

    function controller.set_timezone_vancouver()
        switch_timezone("America/Vancouver", "Vancouver (America/Vancouver)")
    end

    function controller.launch_program_palette()
        local _, launcher_script, rofi_theme = launcher_assets()

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

    function controller.open_appimage_palette()
        local _, launcher_script, rofi_theme = launcher_assets()
        local app_dir = appimage_root()

        awful.spawn.easy_async_with_shell(string.format([=[
            target=%s
            mkdir -p "$target"
            if find "$target" -maxdepth 3 -type f \( -iname '*.AppImage' -o -iname '*.appimage' \) | grep -q .; then
                exit 0
            fi
            exit 4
        ]=], shell_quote(app_dir)), function(_, _, _, exit_code)
            if exit_code == 4 then
                naughty.notify({
                    title = "AppImage Library Empty",
                    text = "Drop .AppImage files into " .. app_dir .. " and they will appear here.",
                    timeout = 4,
                })
                controller.open_appimage_library()
                return
            end

            if command_exists("rofi") and gears.filesystem.file_readable(launcher_script) then
                local theme_arg = ""
                if gears.filesystem.file_readable(rofi_theme) then
                    theme_arg = " -theme " .. shell_quote(rofi_theme)
                end
                awful.spawn.with_shell(string.format(
                    "rofi -show-icons -i -matching fuzzy%s -show appimages -modi %s",
                    theme_arg,
                    shell_quote("appimages:" .. launcher_script .. " --appimages")
                ))
                return
            end

            controller.open_appimage_library()
        end)
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
            elif command -v nmtui-connect >/dev/null 2>&1 && command -v x-terminal-emulator >/dev/null 2>&1; then
                x-terminal-emulator -e nmtui-connect >/dev/null 2>&1 &
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

    function controller.open_network_tui()
        awful.spawn.easy_async_with_shell([[
            if command -v nmtui-connect >/dev/null 2>&1 && command -v x-terminal-emulator >/dev/null 2>&1; then
                x-terminal-emulator -e nmtui-connect >/dev/null 2>&1 &
            elif command -v nmtui >/dev/null 2>&1 && command -v x-terminal-emulator >/dev/null 2>&1; then
                x-terminal-emulator -e nmtui >/dev/null 2>&1 &
            elif command -v nmcli >/dev/null 2>&1 && command -v x-terminal-emulator >/dev/null 2>&1; then
                x-terminal-emulator -e sh -lc 'nmcli device wifi list; echo; read -n1 -rsp "Press any key to close..."' >/dev/null 2>&1 &
            else
                exit 1
            fi
        ]], function(_, _, _, exit_code)
            if exit_code ~= 0 then
                naughty.notify({
                    preset = naughty.config.presets.warn,
                    title = "Network TUI Missing",
                    text = "Install NetworkManager tools such as nmtui.",
                })
            end
        end)
    end

    function controller.open_network_scan()
        awful.spawn.easy_async_with_shell([[
            if command -v nmcli >/dev/null 2>&1 && command -v x-terminal-emulator >/dev/null 2>&1; then
                x-terminal-emulator -e sh -lc 'nmcli device wifi list; echo; read -n1 -rsp "Press any key to close..."' >/dev/null 2>&1 &
            elif command -v nmtui-connect >/dev/null 2>&1 && command -v x-terminal-emulator >/dev/null 2>&1; then
                x-terminal-emulator -e nmtui-connect >/dev/null 2>&1 &
            else
                exit 1
            fi
        ]], function(_, _, _, exit_code)
            if exit_code ~= 0 then
                naughty.notify({
                    preset = naughty.config.presets.warn,
                    title = "Wi-Fi Scan Missing",
                    text = "Install NetworkManager tools such as nmcli or nmtui.",
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

    function controller.notes_root()
        return notes_root()
    end

    function controller.appimage_root()
        return appimage_root()
    end

    function controller.daily_note_path(date_table)
        return daily_note_path(date_table)
    end

    function controller.tasks_note_path()
        return tasks_note_path()
    end

    function controller.open_notes_app()
        awful.spawn.easy_async_with_shell(string.format([=[
            notes_dir=%s
            vault_uri=%s
            repo_root=%s
            mkdir -p "$notes_dir"
            today_note="$notes_dir/daily/$(date +%%F).md"
            mkdir -p "$(dirname "$today_note")"
            managed_links="$notes_dir/linked/LinuxUtilities"

            if [ -d "$repo_root" ]; then
                mkdir -p "$managed_links"
                find "$managed_links" -mindepth 1 -delete >/dev/null 2>&1 || true
                find "$repo_root" -type f \( -name '*.md' -o -name '*.markdown' \) | while IFS= read -r src; do
                    rel="${src#$repo_root/}"
                    dst="$managed_links/$rel"
                    mkdir -p "$(dirname "$dst")"
                    ln -s "$src" "$dst"
                done
            fi

            obsidian_open() {
                uri="$1"
                fallback_path="$2"
                if command -v xdg-mime >/dev/null 2>&1 && [ -n "$(xdg-mime query default x-scheme-handler/obsidian 2>/dev/null)" ]; then
                    if command -v xdg-open >/dev/null 2>&1; then
                        xdg-open "$uri" >/dev/null 2>&1 &
                        return 0
                    elif command -v gio >/dev/null 2>&1; then
                        gio open "$uri" >/dev/null 2>&1 &
                        return 0
                    fi
                fi
                if command -v obsidian >/dev/null 2>&1; then
                    obsidian "$fallback_path" >/dev/null 2>&1 &
                    return 0
                fi
                return 1
            }

            preferred_app="${LINUXUTILS_NOTES_APP:-}"
            if [ "$preferred_app" = "obsidian" ] && obsidian_open "$vault_uri" "$notes_dir"; then
                :
            elif [ "$preferred_app" = "joplin" ] && command -v joplin-desktop >/dev/null 2>&1; then
                joplin-desktop >/dev/null 2>&1 &
            elif obsidian_open "$vault_uri" "$notes_dir"; then
                :
            elif command -v joplin-desktop >/dev/null 2>&1; then
                joplin-desktop >/dev/null 2>&1 &
            elif command -v gio >/dev/null 2>&1; then
                gio open "$notes_dir" >/dev/null 2>&1 &
            elif command -v xdg-open >/dev/null 2>&1; then
                xdg-open "$notes_dir" >/dev/null 2>&1 &
            elif command -v x-terminal-emulator >/dev/null 2>&1; then
                x-terminal-emulator -e sh -lc 'cd "$1" && exec "${SHELL:-bash}"' sh "$notes_dir" >/dev/null 2>&1 &
            else
                exit 1
            fi
        ]=], shell_quote(notes_root()), shell_quote(obsidian_vault_uri()), shell_quote(workspace_root())), function(_, _, _, exit_code)
            if exit_code ~= 0 then
                naughty.notify({
                    preset = naughty.config.presets.warn,
                    title = "Notes App Missing",
                    text = "Install Obsidian or ensure x-terminal-emulator is available for vi fallback.",
                })
            end
        end)
    end

    function controller.open_daily_note(date_table)
        local note_path = daily_note_path(date_table)
        local note_title = os.date("%A, %d %B %Y", os.time(date_table or os.date("*t")))
        awful.spawn.easy_async_with_shell(string.format([=[
            note_path=%s
            note_uri=%s
            note_dir="$(dirname "$note_path")"
            mkdir -p "$note_dir"

            obsidian_open() {
                uri="$1"
                fallback_path="$2"
                if command -v xdg-mime >/dev/null 2>&1 && [ -n "$(xdg-mime query default x-scheme-handler/obsidian 2>/dev/null)" ]; then
                    if command -v xdg-open >/dev/null 2>&1; then
                        xdg-open "$uri" >/dev/null 2>&1 &
                        return 0
                    elif command -v gio >/dev/null 2>&1; then
                        gio open "$uri" >/dev/null 2>&1 &
                        return 0
                    fi
                fi
                if command -v obsidian >/dev/null 2>&1; then
                    obsidian "$fallback_path" >/dev/null 2>&1 &
                    return 0
                fi
                return 1
            }

            if [ ! -f "$note_path" ]; then
                cat > "$note_path" <<'EOF'
# %s

## Tasks

- [ ]

## Notes

EOF
            fi

            preferred_app="${LINUXUTILS_NOTES_APP:-}"
            if [ "$preferred_app" = "obsidian" ] && obsidian_open "$note_uri" "$note_path"; then
                :
            elif [ "$preferred_app" = "joplin" ] && command -v joplin-desktop >/dev/null 2>&1; then
                joplin-desktop >/dev/null 2>&1 &
            elif obsidian_open "$note_uri" "$note_path"; then
                :
            elif command -v joplin-desktop >/dev/null 2>&1; then
                joplin-desktop >/dev/null 2>&1 &
            elif command -v x-terminal-emulator >/dev/null 2>&1; then
                x-terminal-emulator -e sh -lc '${EDITOR:-vi} "$1"' sh "$note_path" >/dev/null 2>&1 &
            elif command -v xdg-open >/dev/null 2>&1; then
                xdg-open "$note_path" >/dev/null 2>&1 &
            elif command -v gio >/dev/null 2>&1; then
                gio open "$note_path" >/dev/null 2>&1 &
            else
                exit 1
            fi
        ]=], shell_quote(note_path), shell_quote(obsidian_note_uri(note_path)), note_title), function(_, _, _, exit_code)
            if exit_code ~= 0 then
                naughty.notify({
                    preset = naughty.config.presets.warn,
                    title = "Daily Note Launcher Missing",
                    text = "Install Obsidian or ensure x-terminal-emulator is available for vi fallback.",
                })
            end
        end)
    end

    function controller.open_tasks_note()
        local note_path = tasks_note_path()
        awful.spawn.easy_async_with_shell(string.format([=[
            note_path=%s
            note_uri=%s
            note_dir="$(dirname "$note_path")"
            mkdir -p "$note_dir"

            obsidian_open() {
                uri="$1"
                fallback_path="$2"
                if command -v xdg-mime >/dev/null 2>&1 && [ -n "$(xdg-mime query default x-scheme-handler/obsidian 2>/dev/null)" ]; then
                    if command -v xdg-open >/dev/null 2>&1; then
                        xdg-open "$uri" >/dev/null 2>&1 &
                        return 0
                    elif command -v gio >/dev/null 2>&1; then
                        gio open "$uri" >/dev/null 2>&1 &
                        return 0
                    fi
                fi
                if command -v obsidian >/dev/null 2>&1; then
                    obsidian "$fallback_path" >/dev/null 2>&1 &
                    return 0
                fi
                return 1
            }

            if [ ! -f "$note_path" ]; then
                cat > "$note_path" <<'EOF'
# Tasks

## Inbox

- [ ]

## Next

- [ ]

## Waiting

- [ ]
EOF
            fi

            preferred_app="${LINUXUTILS_NOTES_APP:-}"
            if [ "$preferred_app" = "obsidian" ] && obsidian_open "$note_uri" "$note_path"; then
                :
            elif [ "$preferred_app" = "joplin" ] && command -v joplin-desktop >/dev/null 2>&1; then
                joplin-desktop >/dev/null 2>&1 &
            elif obsidian_open "$note_uri" "$note_path"; then
                :
            elif command -v joplin-desktop >/dev/null 2>&1; then
                joplin-desktop >/dev/null 2>&1 &
            elif command -v x-terminal-emulator >/dev/null 2>&1; then
                x-terminal-emulator -e sh -lc '${EDITOR:-vi} "$1"' sh "$note_path" >/dev/null 2>&1 &
            elif command -v xdg-open >/dev/null 2>&1; then
                xdg-open "$note_path" >/dev/null 2>&1 &
            elif command -v gio >/dev/null 2>&1; then
                gio open "$note_path" >/dev/null 2>&1 &
            else
                exit 1
            fi
        ]=], shell_quote(note_path), shell_quote(obsidian_note_uri(note_path))), function(_, _, _, exit_code)
            if exit_code ~= 0 then
                naughty.notify({
                    preset = naughty.config.presets.warn,
                    title = "Tasks Note Launcher Missing",
                    text = "Install Obsidian or ensure x-terminal-emulator is available for the tasks note.",
                })
            end
        end)
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
