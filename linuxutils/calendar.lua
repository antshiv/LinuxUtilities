local icons = require("linuxutils.icons")

local M = {}

local function expand_tilde(path)
    if not path or path == "" then
        return path
    end
    if path == "~" then
        return os.getenv("HOME") or path
    end
    if path:sub(1, 2) == "~/" then
        return (os.getenv("HOME") or "") .. path:sub(2)
    end
    return path
end

local function clamp(value, min_value, max_value)
    if value < min_value then
        return min_value
    end
    if value > max_value then
        return max_value
    end
    return value
end

local function shifted_month_date(offset)
    local now = os.date("*t")
    local month_index = (now.month - 1) + (offset or 0)
    local year = now.year + math.floor(month_index / 12)
    local month = (month_index % 12) + 1
    return {
        year = year,
        month = month,
        day = now.day,
    }
end

local function format_month_title(date_table)
    return os.date("%B %Y", os.time({
        year = date_table.year,
        month = date_table.month,
        day = 1,
        hour = 12,
    }))
end

local function read_note_preview(path)
    local file = io.open(path, "r")
    if not file then
        return "No daily note yet.\nUse Daily Note to create one."
    end

    local lines = {}
    for _ = 1, 8 do
        local line = file:read("*line")
        if not line then
            break
        end
        if line ~= "" then
            table.insert(lines, line)
        end
    end
    file:close()

    if #lines == 0 then
        return "Today's note exists but is empty."
    end

    return table.concat(lines, "\n")
end

local function markup(text, color, extra_attrs)
    local safe_text = tostring(text or "")
        :gsub("&", "&amp;")
        :gsub("<", "&lt;")
        :gsub(">", "&gt;")
    local attrs = ""
    if color and color ~= "" then
        attrs = attrs .. string.format(' foreground="%s"', color)
    end
    if extra_attrs and extra_attrs ~= "" then
        attrs = attrs .. " " .. extra_attrs
    end
    return string.format("<span%s>%s</span>", attrs, safe_text)
end

function M.new(opts)
    local awful = assert(opts and opts.awful, "calendar.new requires awful")
    local gears = assert(opts and opts.gears, "calendar.new requires gears")
    local wibox = assert(opts and opts.wibox, "calendar.new requires wibox")
    local launchers = assert(opts and opts.launchers, "calendar.new requires launchers controller")
    local controller = {}

    local font_size = tonumber(os.getenv("LINUXUTILS_CALENDAR_FONT_SIZE")) or 15
    font_size = clamp(font_size, 10, 24)

    local state = {
        month_offset = 0,
        popup = nil,
        month_title = nil,
        month_subtitle = nil,
        calendar_widget = nil,
        note_path = nil,
        note_preview = nil,
        note_hint = nil,
    }

    local function resolve_screen(candidate)
        if candidate ~= nil then
            local ok, workarea = pcall(function()
                return candidate.workarea
            end)
            if ok and type(workarea) == "table" then
                return candidate
            end
        end

        if mouse and mouse.screen then
            return mouse.screen
        end
        if awful.screen and awful.screen.focused then
            return awful.screen.focused()
        end
        return nil
    end

    local function calendar_font()
        return string.format("Monospace %d", font_size)
    end

    local function is_today(date_table)
        local now = os.date("*t")
        return type(date_table) == "table"
            and date_table.year == now.year
            and date_table.month == now.month
            and date_table.day == now.day
    end

    local function decorate_calendar_cell(widget, flag, date_table)
        local text_color = icons.palette.text
        local background = "#172033"
        local border_width = 0
        local border_color = icons.palette.bluetooth
        local shape = gears.shape and gears.shape.rounded_rect or nil
        local highlight_today = (flag == "focus")
            or (
                is_today(date_table)
                and flag ~= "monthheader"
                and flag ~= "header"
                and flag ~= "weekday"
                and flag ~= "weeknumber"
                and flag ~= "month"
                and flag ~= "year"
                and flag ~= "yearheader"
            )

        if widget and widget.get_text and widget.set_markup then
            local text = widget:get_text()
            local attrs = ""
            if flag == "monthheader" or flag == "header" then
                text_color = icons.palette.wifi
                attrs = 'weight="bold"'
            elseif flag == "weekday" then
                text_color = icons.palette.muted
                attrs = 'weight="bold"'
            elseif flag == "weeknumber" then
                text_color = icons.palette.offline
            elseif highlight_today then
                text_color = icons.palette.text
                background = "#15304b"
                border_width = 2
                border_color = icons.palette.notes
                attrs = 'weight="bold"'
            end
            widget:set_markup(markup(text, text_color, attrs))
        end

        if flag == "month" then
            background = "#0f1727"
        elseif highlight_today then
            background = "#15304b"
            border_width = 2
            border_color = icons.palette.notes
        elseif flag == "monthheader" or flag == "header" then
            background = "#111b2f"
        elseif flag == "weekday" then
            background = "#111827"
        elseif flag == "weeknumber" then
            background = "#101521"
        end

        return wibox.widget {
            {
                widget,
                margins = 4,
                widget = wibox.container.margin,
            },
            fg = text_color,
            bg = background,
            shape = shape,
            shape_border_width = border_width,
            shape_border_color = border_color,
            widget = wibox.container.background,
        }
    end

    local function today_note_path()
        if type(launchers.daily_note_path) == "function" then
            return launchers.daily_note_path(os.date("*t"))
        end
        return expand_tilde("~/Workspace/ShivasNotes/daily/" .. os.date("%Y-%m-%d") .. ".md")
    end

    local function refresh_popup()
        if not state.calendar_widget then
            return
        end

        local date_table = shifted_month_date(state.month_offset)
        state.calendar_widget:set_date(date_table)
        if state.calendar_widget.set_font then
            state.calendar_widget:set_font(calendar_font())
        else
            state.calendar_widget.font = calendar_font()
        end

        if state.month_title then
            state.month_title:set_markup(markup(format_month_title(date_table), icons.palette.text, 'weight="bold"'))
        end

        if state.month_subtitle then
            state.month_subtitle:set_markup(table.concat({
                markup(os.date("%A, %d %B %Y"), icons.palette.wifi, 'weight="bold"'),
                markup("  •  ", icons.palette.muted),
                markup("Mouse wheel: browse months • Today is highlighted", icons.palette.muted),
            }))
        end

        state.note_path = today_note_path()
        if state.note_hint then
            state.note_hint:set_markup(table.concat({
                markup("Today note", icons.palette.notes or icons.palette.wifi, 'weight="bold"'),
                markup(": ", icons.palette.muted),
                markup(state.note_path, icons.palette.muted),
            }))
        end
        if state.note_preview then
            state.note_preview:set_text(read_note_preview(state.note_path))
        end
    end

    local function refresh_note_preview_later()
        if gears.timer and gears.timer.start_new then
            gears.timer.start_new(0.8, function()
                refresh_popup()
                return false
            end)
        else
            refresh_popup()
        end
    end

    local function make_text_action(label, callback, color)
        local widget = wibox.widget.textbox()
        widget:set_markup(table.concat({
            markup("[", icons.palette.muted),
            markup(label, color or icons.palette.wifi, 'weight="bold"'),
            markup("]", icons.palette.muted),
        }))
        widget:buttons(gears.table.join(
            awful.button({ }, 1, callback)
        ))
        return widget
    end

    local function ensure_popup()
        if state.popup then
            return state.popup
        end

        state.month_title = wibox.widget.textbox()
        state.month_subtitle = wibox.widget.textbox()
        state.note_hint = wibox.widget.textbox()
        state.note_preview = wibox.widget.textbox()
        if state.note_preview.set_valign then
            state.note_preview:set_valign("top")
        else
            state.note_preview.valign = "top"
        end
        state.note_preview.wrap = "word_char"

        state.calendar_widget = wibox.widget {
            date = shifted_month_date(0),
            font = calendar_font(),
            week_numbers = true,
            long_weekdays = true,
            fn_embed = decorate_calendar_cell,
            widget = wibox.widget.calendar.month,
        }

        local month_actions = wibox.widget {
            make_text_action("Prev", controller.previous_month, icons.palette.wifi),
            make_text_action("Today", controller.reset_month, icons.palette.bluetooth),
            make_text_action("Next", controller.next_month, icons.palette.wifi),
            make_text_action("A-", controller.decrease_font, icons.palette.offline),
            make_text_action("A+", controller.increase_font, icons.palette.notes or icons.palette.ethernet),
            layout = wibox.layout.fixed.horizontal,
            spacing = 10,
        }

        local note_actions = wibox.widget {
            make_text_action("Daily Note", function()
                launchers.open_daily_note()
                refresh_note_preview_later()
            end, icons.palette.notes or icons.palette.wifi),
            make_text_action("Notes App", launchers.open_notes_app, icons.palette.notes or icons.palette.bluetooth),
            make_text_action("Calendar App", launchers.open_calendar_app, icons.palette.wifi),
            make_text_action("Close", controller.hide_popup, icons.palette.offline),
            layout = wibox.layout.fixed.horizontal,
            spacing = 10,
        }

        state.popup = awful.popup({
            ontop = true,
            visible = false,
            border_width = 2,
            border_color = icons.palette.bluetooth,
            minimum_width = 700,
            minimum_height = 560,
            maximum_width = 900,
            maximum_height = 760,
            placement = awful.placement and awful.placement.centered or nil,
            bg = "#111827",
            fg = icons.palette.text,
            widget = wibox.widget {
                {
                    state.month_title,
                    state.month_subtitle,
                    month_actions,
                    state.calendar_widget,
                    state.note_hint,
                    state.note_preview,
                    note_actions,
                    layout = wibox.layout.fixed.vertical,
                    spacing = 14,
                },
                margins = 20,
                widget = wibox.container.margin,
            },
        })

        refresh_popup()
        return state.popup
    end

    function controller.notes_root()
        if type(launchers.notes_root) == "function" then
            return launchers.notes_root()
        end
        return expand_tilde("~/Workspace/ShivasNotes")
    end

    function controller.daily_note_path(date_table)
        if type(launchers.daily_note_path) == "function" then
            return launchers.daily_note_path(date_table)
        end
        local stamp = os.date("%Y-%m-%d", os.time(date_table or os.date("*t")))
        return controller.notes_root() .. "/daily/" .. stamp .. ".md"
    end

    function controller.toggle_popup(screen)
        local popup = ensure_popup()
        if popup.visible then
            popup.visible = false
            return
        end

        popup.screen = resolve_screen(screen)
        refresh_popup()
        popup.visible = true
        if awful.placement and awful.placement.centered then
            awful.placement.centered(popup, { honor_workarea = true, parent = popup.screen })
        end
    end

    function controller.hide_popup()
        if state.popup then
            state.popup.visible = false
        end
    end

    function controller.previous_month()
        state.month_offset = state.month_offset - 1
        ensure_popup().visible = true
        refresh_popup()
    end

    function controller.next_month()
        state.month_offset = state.month_offset + 1
        ensure_popup().visible = true
        refresh_popup()
    end

    function controller.reset_month()
        state.month_offset = 0
        ensure_popup().visible = true
        refresh_popup()
    end

    function controller.increase_font()
        font_size = clamp(font_size + 1, 10, 24)
        ensure_popup().visible = true
        refresh_popup()
    end

    function controller.decrease_font()
        font_size = clamp(font_size - 1, 10, 24)
        ensure_popup().visible = true
        refresh_popup()
    end

    function controller.clock_tooltip_text()
        return os.date("%A, %d %B %Y\n%H:%M:%S")
            .. "\nLeft click: toggle calendar agenda"
            .. "\nPopup button: Close"
            .. "\nMiddle click: calendar app"
            .. "\nRight click: date/time settings"
            .. "\nWheel: browse months"
    end

    function controller.popup_state()
        return {
            visible = state.popup and state.popup.visible or false,
            note_path = state.note_path,
            font_size = font_size,
            month_offset = state.month_offset,
        }
    end

    return controller
end

return M
