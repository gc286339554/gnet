package.path = package.path ..';..\\src\\extend\\lua_service\\lua\\?.lua';
require "msg_util"
function update(ms_delay)	
end

function binary_service_msg_handler(source_service_id,msg)
	print(msg:get_op())
	print(msg:get_bool())
	print(msg:get_bool())
	print(msg:get_bool())
	print(msg:get_string())
end

function lua_service_msg_handler(source_service_id,msg)

end

send_lua_msg("lua_service",{name = "msg_test",id = 0,content = "test"})


