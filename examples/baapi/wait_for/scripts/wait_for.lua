local lamx = require 'lamx'

lamx.auto_connect()

local el = lamx.eventloop.new()


local objects_available = function()
    print("Wait done - object " .. arg[1] .. " is available");
    el:stop()
end

print("Wait until object " .. arg[1] .. " is available");
lamx.bus.wait_for(arg[1], objects_available)

el:start()

lamx.disconnect_all()
