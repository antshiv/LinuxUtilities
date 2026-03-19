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

local calendar_theme = {
    panel_bg = "#111317",
    panel_fg = "#d6d9df",
    panel_border = "#626975",
    surface = "#181b20",
    surface_alt = "#14171b",
    header_bg = "#1a1e24",
    header_fg = "#eceff4",
    weekday_fg = "#b9c0cc",
    weeknumber_fg = "#8d95a3",
    muted_fg = "#9aa3b2",
    cell_border = "#3f454f",
    today_bg = "#272d36",
    today_border = "#aab2bf",
    action_bg = "#191d23",
    action_border = "#4f5663",
    note_bg = "#15191e",
}

function M.new(opts)
    local awful = assert(opts and opts.awful, "calendar.new requires awful")
    local gears = assert(opts and opts.gears, "calendar.new requires gears")
    local wibox = assert(opts and opts.wibox, "calendar.new requires wibox")
    local launchers = assert(opts and opts.launchers, "calendar.new requires launchers controller")
    local controller = {}
    local refresh_note_preview_later

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
        selected_date = os.date("*t"),
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

    local function same_day(left, right)
        return type(left) == "table"
            and type(right) == "table"
            and left.year == right.year
            and left.month == right.month
            and left.day == right.day
    end

    local function clamp_day(year, month, day)
        local last_day = tonumber(os.date("%d", os.time({
            year = year,
            month = month + 1,
            day = 0,
            hour = 12,
        }))) or day or 1

        return {
            year = year,
            month = month,
            day = math.max(1, math.min(day or 1, last_day)),
        }
    end

    local function interactive_day_flag(flag)
        return flag ~= "monthheader"
            and flag ~= "header"
            and flag ~= "weekday"
            and flag ~= "weeknumber"
            and flag ~= "month"
            and flag ~= "year"
            and flag ~= "yearheader"
    end

    local function decorate_calendar_cell(widget, flag, date_table)
        local text_color = calendar_theme.panel_fg
        local background = calendar_theme.surface
        local border_width = 1
        local border_color = calendar_theme.cell_border
        local shape = gears.shape and gears.shape.rectangle or nil
        local is_interactive_day = interactive_day_flag(flag)
        local highlight_today = is_interactive_day and is_today(date_table)
        local highlight_selected = is_interactive_day and same_day(state.selected_date, date_table)

        if widget and widget.get_text and widget.set_markup then
            local text = widget:get_text()
            local attrs = ""
            if flag == "monthheader" or flag == "header" then
                text_color = calendar_theme.header_fg
                attrs = 'weight="bold"'
            elseif flag == "weekday" then
                text_color = calendar_theme.weekday_fg
                attrs = 'weight="bold"'
            elseif flag == "weeknumber" then
                text_color = calendar_theme.weeknumber_fg
            elseif highlight_selected then
                text_color = calendar_theme.header_fg
                background = calendar_theme.today_bg
                border_width = 2
                border_color = calendar_theme.today_border
                attrs = 'weight="bold"'
            elseif highlight_today then
                text_color = calendar_theme.panel_fg
                background = "#20242b"
                border_width = 1
                border_color = "#7e8794"
            end
            widget:set_markup(markup(text, text_color, attrs))
        end

        if flag == "month" then
            background = calendar_theme.surface_alt
        elseif highlight_selected then
            background = calendar_theme.today_bg
            border_width = 2
            border_color = calendar_theme.today_border
        elseif highlight_today then
            background = "#20242b"
            border_width = 1
            border_color = "#7e8794"
        elseif flag == "monthheader" or flag == "header" then
            background = calendar_theme.header_bg
        elseif flag == "weekday" then
            background = calendar_theme.surface_alt
        elseif flag == "weeknumber" then
            background = calendar_theme.surface_alt
        end

        local out = wibox.widget {
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

        if is_interactive_day then
            out:buttons(gears.table.join(
                awful.button({ }, 1, function()
                    controller.select_date(date_table)
                end),
                awful.button({ }, 3, function()
                    controller.select_date(date_table)
                    launchers.open_daily_note(date_table)
                    refresh_note_preview_later()
                end)
            ))
        end

        return out
    end

    local function selected_note_path()
        local selected = state.selected_date or os.date("*t")
        if type(launchers.daily_note_path) == "function" then
            return launchers.daily_note_path(selected)
        end
        return expand_tilde("~/Workspace/ShivasNotes/daily/" .. os.date("%Y-%m-%d", os.time(selected)) .. ".md")
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
            state.month_title:set_markup(markup(format_month_title(date_table), calendar_theme.header_fg, 'weight="bold"'))
        end

        if state.month_subtitle then
            state.month_subtitle:set_markup(table.concat({
                markup(os.date("%A, %d %B %Y", os.time(state.selected_date or os.date("*t"))), calendar_theme.weekday_fg, 'weight="bold"'),
                markup("  •  ", calendar_theme.muted_fg),
                markup("Left click a day to select • Right click a day to open its note", calendar_theme.muted_fg),
            }))
        end

        state.note_path = selected_note_path()
        if state.note_hint then
            state.note_hint:set_markup(table.concat({
                markup("Selected note", calendar_theme.header_fg, 'weight="bold"'),
                markup(": ", calendar_theme.muted_fg),
                markup(state.note_path, calendar_theme.muted_fg),
            }))
        end
        if state.note_preview then
            state.note_preview:set_text(read_note_preview(state.note_path))
        end
    end

    refresh_note_preview_later = function()
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
        local text = wibox.widget.textbox()
        text:set_markup(markup(label, color or calendar_theme.header_fg, 'weight="bold"'))

        local widget = wibox.widget {
            {
                text,
                margins = {
                    left = 10,
                    right = 10,
                    top = 6,
                    bottom = 6,
                },
                widget = wibox.container.margin,
            },
            bg = calendar_theme.action_bg,
            fg = color or calendar_theme.header_fg,
            shape = gears.shape and gears.shape.rectangle or nil,
            shape_border_width = 1,
            shape_border_color = calendar_theme.action_border,
            widget = wibox.container.background,
        }

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
        if state.note_preview.set_font then
            state.note_preview:set_font("Monospace 12")
        else
            state.note_preview.font = "Monospace 12"
        end
        state.note_preview.forced_height = 96

        state.calendar_widget = wibox.widget {
            date = shifted_month_date(0),
            font = calendar_font(),
            week_numbers = false,
            long_weekdays = false,
            fn_embed = decorate_calendar_cell,
            widget = wibox.widget.calendar.month,
        }

        local month_actions = wibox.widget {
            make_text_action("Prev", controller.previous_month, calendar_theme.weekday_fg),
            make_text_action("Today", controller.reset_month, calendar_theme.header_fg),
            make_text_action("Next", controller.next_month, calendar_theme.weekday_fg),
            make_text_action("A-", controller.decrease_font, calendar_theme.muted_fg),
            make_text_action("A+", controller.increase_font, calendar_theme.header_fg),
            make_text_action("Close", controller.hide_popup, calendar_theme.muted_fg),
            layout = wibox.layout.fixed.horizontal,
            spacing = 8,
        }

        local note_actions = wibox.widget {
            make_text_action("Daily Note", function()
                launchers.open_daily_note(state.selected_date or os.date("*t"))
                refresh_note_preview_later()
            end, calendar_theme.header_fg),
            make_text_action("Tasks", launchers.open_tasks_note, calendar_theme.weekday_fg),
            make_text_action("Notes App", launchers.open_notes_app, calendar_theme.weekday_fg),
            make_text_action("Calendar App", launchers.open_calendar_app, calendar_theme.weekday_fg),
            layout = wibox.layout.fixed.horizontal,
            spacing = 8,
        }

        local note_panel = wibox.widget {
            {
                {
                    state.note_hint,
                    state.note_preview,
                    layout = wibox.layout.fixed.vertical,
                    spacing = 8,
                },
                margins = 14,
                widget = wibox.container.margin,
            },
            bg = calendar_theme.note_bg,
            fg = calendar_theme.panel_fg,
            shape = gears.shape and gears.shape.rectangle or nil,
            shape_border_width = 1,
            shape_border_color = calendar_theme.cell_border,
            widget = wibox.container.background,
        }

        state.popup = awful.popup({
            ontop = true,
            visible = false,
            border_width = 2,
            border_color = calendar_theme.panel_border,
            minimum_width = 700,
            minimum_height = 500,
            maximum_width = 900,
            maximum_height = 700,
            placement = awful.placement and awful.placement.centered or nil,
            bg = calendar_theme.panel_bg,
            fg = calendar_theme.panel_fg,
            widget = wibox.widget {
                {
                    state.month_title,
                    state.month_subtitle,
                    month_actions,
                    state.calendar_widget,
                    note_actions,
                    note_panel,
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
        state.selected_date = os.date("*t")
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

    function controller.select_date(date_table)
        if type(date_table) ~= "table" then
            return
        end

        state.selected_date = clamp_day(date_table.year, date_table.month, date_table.day)
        ensure_popup().visible = true
        refresh_popup()
    end

    function controller.open_selected_note()
        launchers.open_daily_note(state.selected_date or os.date("*t"))
        refresh_note_preview_later()
    end

    function controller.clock_tooltip_text()
        return os.date("%A, %d %B %Y\n%H:%M:%S")
            .. "\nLeft click: toggle calendar agenda"
            .. "\nPopup button: Close"
            .. "\nMiddle click: calendar app"
            .. "\nRight click: date/time settings"
            .. "\nWheel: browse months"
            .. "\nCalendar days: left click select, right click open note"
    end

    function controller.popup_state()
        return {
            visible = state.popup and state.popup.visible or false,
            note_path = state.note_path,
            font_size = font_size,
            month_offset = state.month_offset,
            selected_date = state.selected_date and os.date("%Y-%m-%d", os.time(state.selected_date)) or nil,
        }
    end

    return controller
end

return M
