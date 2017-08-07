package.path = package.path ..';..\\src\\extend\\lua_service\\lua\\?.lua';
--package.cpath 是 dll文件的目录

-- 可重新加载一些lua文件
--	package.loaded["data/zhajinhua_data"] = nil 
--	require "data/zhajinhua_data"

require "msg_util"
local data_packet = require "data_packet"

function update(ms_delay)	
end


function binary_service_msg_handler(source_service_id,msg)

end

function lua_service_msg_handler(source_service_id,msg)
	print(msg.name)
	print(msg.id)
	print(msg.content)
end

local data = data_packet.new()

data:start_write()
data:set_op(1000)
data:put_bool(true)
data:put_bool(false)
data:put_bool(nil)
data:put_string("put_string")
data:end_write()

send_binary_msg("lua_service_01", data)



