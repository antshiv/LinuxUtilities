local M = {}

function M.new(opts)
    local awful = assert(opts and opts.awful, "brightness.new requires awful")
    local naughty = assert(opts and opts.naughty, "brightness.new requires naughty")
    local shell_quote = assert(opts and opts.shell_quote, "brightness.new requires shell_quote")
    local step_percent = opts.step_percent or 5

    local notification_id = nil
    local warned = false
    local controller = {}

    local function show_notification(percent, backend)
        local text = "Brightness changed"
        if percent and percent ~= "" and percent ~= "?" then
            text = "Brightness " .. percent .. "%"
        end
        if backend and backend ~= "" then
            text = text .. "\nBackend: " .. backend
        end

        local n = naughty.notify({
            title = "Brightness",
            text = text,
            timeout = 1.4,
            replaces_id = notification_id,
        })
        if n and n.id then
            notification_id = n.id
        end
    end

    function controller.adjust(direction)
        awful.spawn.easy_async_with_shell(string.format([=[
            step=%d
            action=%s

            run_brightnessctl() {
                command -v brightnessctl >/dev/null 2>&1 || return 1
                if [ "$action" = "up" ]; then
                    brightnessctl set "${step}%%+" >/dev/null 2>&1 || return 1
                else
                    brightnessctl set "${step}%%-" >/dev/null 2>&1 || return 1
                fi
                percent=$(brightnessctl -m 2>/dev/null | awk -F, 'NR==1 {gsub("%%","",$4); print $4}')
                [ -z "$percent" ] && percent=$(brightnessctl info 2>/dev/null | awk '/Current brightness/ {for (i = 1; i <= NF; i++) if ($i ~ /\\([0-9]+%%\\)/) {gsub(/[()%%]/, "", $i); print $i; exit}}')
                printf 'BACKEND=brightnessctl\nPERCENT=%%s\n' "${percent:-?}"
                return 0
            }

            run_light() {
                command -v light >/dev/null 2>&1 || return 1
                if [ "$action" = "up" ]; then
                    light -A "$step" >/dev/null 2>&1 || return 1
                else
                    light -U "$step" >/dev/null 2>&1 || return 1
                fi
                percent=$(light -G 2>/dev/null | awk '{printf "%%d", $1 + 0.5}')
                printf 'BACKEND=light\nPERCENT=%%s\n' "${percent:-?}"
                return 0
            }

            run_xbacklight() {
                command -v xbacklight >/dev/null 2>&1 || return 1
                if [ "$action" = "up" ]; then
                    xbacklight -inc "$step" >/dev/null 2>&1 || return 1
                else
                    xbacklight -dec "$step" >/dev/null 2>&1 || return 1
                fi
                percent=$(xbacklight -get 2>/dev/null | awk '{printf "%%d", $1 + 0.5}')
                printf 'BACKEND=xbacklight\nPERCENT=%%s\n' "${percent:-?}"
                return 0
            }

            run_sysfs() {
                for dev in /sys/class/backlight/*; do
                    [ -e "$dev" ] || continue
                    cur=$(cat "$dev/brightness" 2>/dev/null) || continue
                    max=$(cat "$dev/max_brightness" 2>/dev/null) || continue
                    [ -n "$cur" ] || continue
                    [ -n "$max" ] || continue
                    [ "$max" -gt 0 ] || continue
                    [ -w "$dev/brightness" ] || continue
                    step_raw=$(( (max * step + 50) / 100 ))
                    [ "$step_raw" -lt 1 ] && step_raw=1
                    if [ "$action" = "up" ]; then
                        new=$((cur + step_raw))
                    else
                        new=$((cur - step_raw))
                    fi
                    [ "$new" -lt 1 ] && new=1
                    [ "$new" -gt "$max" ] && new="$max"
                    printf '%%s' "$new" > "$dev/brightness" 2>/dev/null || continue
                    percent=$(( (new * 100 + max / 2) / max ))
                    printf 'BACKEND=sysfs:%%s\nPERCENT=%%s\n' "$(basename "$dev")" "$percent"
                    return 0
                done
                return 1
            }

            run_xrandr() {
                command -v xrandr >/dev/null 2>&1 || return 1
                output=$(xrandr --query 2>/dev/null | awk '
                    $2 == "connected" && $0 ~ / primary / { print $1; exit }
                    $2 == "connected" && ($1 ~ /^eDP/ || $1 ~ /^LVDS/) { print $1; exit }
                    $2 == "connected" { print $1; exit }
                ')
                [ -n "$output" ] || return 1
                current=$(xrandr --verbose 2>/dev/null | awk -v out="$output" '
                    $1 == out && $2 == "connected" { in_output = 1; next }
                    in_output && $1 == "Brightness:" { print $2; exit }
                    in_output && /^[^[:space:]]/ { in_output = 0 }
                ')
                [ -n "$current" ] || return 1
                new=$(awk -v cur="$current" -v step="$step" -v action="$action" '
                    BEGIN {
                        delta = step / 100.0
                        value = cur + (action == "up" ? delta : -delta)
                        if (value < 0.10) value = 0.10
                        if (value > 1.00) value = 1.00
                        printf "%%.2f", value
                    }
                ')
                xrandr --output "$output" --brightness "$new" >/dev/null 2>&1 || return 1
                percent=$(awk -v value="$new" 'BEGIN { printf "%%d", value * 100 + 0.5 }')
                printf 'BACKEND=xrandr:%%s\nPERCENT=%%s\n' "$output" "$percent"
                return 0
            }

            run_brightnessctl || run_light || run_xbacklight || run_sysfs || run_xrandr || exit 1
        ]=], step_percent, shell_quote(direction)), function(stdout, _, _, exit_code)
            if exit_code == 0 then
                local percent = stdout:match("PERCENT=([^\n]+)") or "?"
                local backend = stdout:match("BACKEND=([^\n]+)") or "unknown"
                warned = false
                show_notification(percent, backend)
                return
            end

            if not warned then
                naughty.notify({
                    title = "Brightness keys",
                    text = "No usable brightness backend found. Install brightnessctl, or use an output that exposes xrandr brightness/backlight control.",
                    timeout = 2.8
                })
                warned = true
            end
        end)
    end

    function controller.up()
        controller.adjust("up")
    end

    function controller.down()
        controller.adjust("down")
    end

    return controller
end

return M
