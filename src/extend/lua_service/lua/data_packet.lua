local data_packet = {}
data_packet.__index = data_packet
function data_packet.new()
	local o = {}  
	o.data = dp_malloc_data_packet()
    setmetatable(o, data_packet)
    return o 
end
function data_packet.new_from_handler(handler)
	local o = {}  
	o.data = handler
    setmetatable(o, data_packet)
    return o 
end

--free 不能随意调用，只有在lua层分配，不向c++层传递时使用（一般测试接口时会用到）
function data_packet:free()
	dp_free_data_packet(self.data)
end

function data_packet:start_write()
	dp_start_write(self.data)
end
function data_packet:end_write()
	dp_end_write(self.data)
end
function data_packet:put_num(val)
	dp_put_num(self.data,val)
end
function data_packet:get_num()
	return dp_get_num(self.data)
end
function data_packet:put_bool(val)
	dp_put_bool(self.data,val)
end
function data_packet:get_bool()
	return dp_get_bool(self.data)
end
function data_packet:put_string(str)
	dp_put_string(self.data,str)
end
function data_packet:get_string()
	return dp_get_string(self.data,g_service_this)
end
function data_packet:get_op()
	return dp_get_op(self.data)
end
function data_packet:set_op(val)
	dp_set_op(self.data,val)
end


return data_packet