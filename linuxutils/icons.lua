local M = {}

M.palette = {
    text = "#d8dee9",
    muted = "#7f8c9f",
    wifi = "#7dcfff",
    bluetooth = "#6ea8ff",
    ethernet = "#8bd49c",
    notes = "#9ad8ff",
    apps = "#d7ba7d",
    offline = "#96a0b5",
}

M.glyphs = {
    bluetooth_on = "◆",
    bluetooth_off = "◇",
    wifi = "◉",
    ethernet = "▣",
    online = "◎",
    offline = "○",
    system = "▤",
    folder = "▥",
    apps = "▦",
}

local function escape_markup(text)
    return tostring(text or ""):gsub("&", "&amp;"):gsub("<", "&lt;"):gsub(">", "&gt;")
end

local function span(text, color, extra_attrs)
    local attrs = ""
    if color and color ~= "" then
        attrs = attrs .. string.format(' foreground="%s"', color)
    end
    if extra_attrs and extra_attrs ~= "" then
        attrs = attrs .. " " .. extra_attrs
    end
    return string.format("<span%s>%s</span>", attrs, escape_markup(text))
end

function M.decorate(icon, icon_color, label, label_color)
    return table.concat({
        " ",
        span(icon, icon_color, 'weight="bold"'),
        span(" " .. tostring(label or ""), label_color or M.palette.text),
        " ",
    })
end

function M.bluetooth(powered, connected)
    local device_count = tonumber(connected) or 0
    if powered == "yes" then
        local label = "BT"
        if device_count > 0 then
            label = label .. " " .. tostring(device_count)
        end
        return M.decorate(M.glyphs.bluetooth_on, M.palette.bluetooth, label, M.palette.text)
    end
    return M.decorate(M.glyphs.bluetooth_off, M.palette.muted, "BT", M.palette.muted)
end

function M.network(ntype, label)
    if ntype == "wifi" then
        return M.decorate(M.glyphs.wifi, M.palette.wifi, label, M.palette.text)
    end
    if ntype == "ethernet" then
        return M.decorate(M.glyphs.ethernet, M.palette.ethernet, label, M.palette.text)
    end
    if ntype == "connected" then
        return M.decorate(M.glyphs.online, M.palette.wifi, label, M.palette.text)
    end
    return M.decorate(M.glyphs.offline, M.palette.offline, label or "offline", M.palette.muted)
end

function M.system_usage(cpu_percent, mem_percent, compact)
    local cpu_value = tonumber(cpu_percent)
    local mem_value = tonumber(mem_percent)
    local cpu_label
    local mem_label

    if compact then
        cpu_label = cpu_value and string.format("C%d", cpu_value) or "C--"
        mem_label = mem_value and string.format("R%d", mem_value) or "R--"
    else
        cpu_label = cpu_value and string.format("CPU %d%%", cpu_value) or "CPU --"
        mem_label = mem_value and string.format("RAM %d%%", mem_value) or "RAM --"
    end
    return M.decorate(M.glyphs.system, M.palette.notes, cpu_label .. " " .. mem_label, M.palette.text)
end

function M.folder(label)
    return M.decorate(M.glyphs.folder, M.palette.wifi, label or "Files", M.palette.text)
end

function M.applications(label)
    return M.decorate(M.glyphs.apps, M.palette.apps, label or "Apps", M.palette.text)
end

return M
