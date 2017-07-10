local data_packet = require "data_packet"
function serialize(obj)  
    local lua = ""  
    local t = type(obj)  
    if t == "number" then  
        lua = lua .. obj  
    elseif t == "boolean" then  
        lua = lua .. tostring(obj)  
    elseif t == "string" then  
        lua = lua .. string.format("%q", obj)  
    elseif t == "table" then  
        lua = lua .. "{\n"  
		for k, v in pairs(obj) do  
			lua = lua .. "[" .. serialize(k) .. "]=" .. serialize(v) .. ",\n"  
		end  
		local metatable = getmetatable(obj)  
			if metatable ~= nil and type(metatable.__index) == "table" then  
			for k, v in pairs(metatable.__index) do  
				lua = lua .. "[" .. serialize(k) .. "]=" .. serialize(v) .. ",\n"  
			end  
		end  
        lua = lua .. "}"  
    elseif t == "nil" then  
        return nil  
    else  
        error("can not serialize a " .. t .. " type.")  
    end  
    return lua  
end  
  
function unserialize(lua)  
    local t = type(lua)  
    if t == "nil" or lua == "" then  
        return nil  
    elseif t == "number" or t == "string" or t == "boolean" then  
        lua = tostring(lua)  
    else  
        error("can not unserialize a " .. t .. " type.")  
    end  
    lua = "return " .. lua  
    local func = load(lua)  
    if func == nil then  
        return nil  
    end  
    return func()  
end 

function send_lua_msg(service_name,msg_table) 
	dp_send_lua_msg(service_name,serialize(msg_table),g_service_this)
end
function send_binary_msg(service_name,msg_table) 
	dp_send_binary_msg(service_name,msg_table.data,g_service_this)
end

function dp_binary_service_msg_handler(source_service_id,msg)
	local data_msg = data_packet.new_from_handler(msg)
	binary_service_msg_handler(source_service_id,data_msg)
end

function dp_lua_service_msg_handler(source_service_id,msg)
	local data_msg = data_packet.new_from_handler(msg)
	local str = data_msg:get_string()
	local msg_table = unserialize(str)
	lua_service_msg_handler(source_service_id,msg_table)
end

