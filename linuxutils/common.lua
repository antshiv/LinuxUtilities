local M = {}

function M.get_short_hostname()
    local hostname_pipe = io.popen("hostname -s 2>/dev/null")
    local host = nil
    if hostname_pipe then
        host = hostname_pipe:read("*l")
        hostname_pipe:close()
    end
    if not host or host == "" then
        host = os.getenv("HOSTNAME") or "linux"
    end
    return host
end

function M.compact_sink_name(sink)
    if not sink or sink == "" then
        return "unknown"
    end

    local compact = sink
    compact = compact:gsub("^alsa_output%.", "")
    compact = compact:gsub("^bluez_output%.", "bt:")
    if #compact > 18 then
        compact = compact:sub(1, 18) .. "..."
    end
    return compact
end

function M.shorten_label(label, max_len)
    if not label or label == "" then
        return "unknown"
    end
    if #label <= max_len then
        return label
    end
    return label:sub(1, max_len) .. "..."
end

function M.shell_quote(value)
    if not value or value == "" then
        return "''"
    end
    return "'" .. tostring(value):gsub("'", "'\\''") .. "'"
end

function M.clamp_number(value, lo, hi)
    if value < lo then
        return lo
    end
    if value > hi then
        return hi
    end
    return value
end

function M.command_exists(cmd)
    local ok, _, code = os.execute("command -v " .. cmd .. " >/dev/null 2>&1")
    return ok == true or code == 0
end

return M
