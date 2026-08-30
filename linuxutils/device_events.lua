local M = {}

function M.new(opts)
    local gears = assert(opts and opts.gears, "device_events.new requires gears")
    local dbus_api = opts.dbus_api or rawget(_G, "dbus")
    local callbacks = opts.callbacks or {}
    local pending = {}
    local controller = {}

    local function dispatch(domain)
        pending[domain] = nil
        if callbacks[domain] then
            callbacks[domain]()
        elseif callbacks.all then
            callbacks.all()
        end
        return false
    end

    local function schedule(domain)
        domain = tostring(domain or "all")
        if pending[domain] then
            return
        end
        pending[domain] = true
        gears.timer.start_new(0.2, function()
            return dispatch(domain)
        end)
    end

    function controller.start()
        if not dbus_api or not dbus_api.add_match or not dbus_api.connect_signal then
            return false
        end
        dbus_api.add_match("session", "interface='com.antshiv.LinuxUtilities'")
        dbus_api.connect_signal("com.antshiv.LinuxUtilities", function(_, domain)
            schedule(domain)
        end)
        return true
    end

    function controller.dispatch(domain)
        schedule(domain)
    end

    return controller
end

return M
