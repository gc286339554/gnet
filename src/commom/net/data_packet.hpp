#pragma once
#include "pre_header.h"

namespace gnet {
#define DATA_HEAD_LEN (sizeof(uint32))
#define DATA_OP_LEN (sizeof(uint32))	
#define DATA_MAX_LEN (5*1024*1024)

class data_packet
{
	private:
		uint8* m_buff;
		uint32 m_pos = 0;
		uint32 m_buff_len;
		uint32 m_data_len;
		uint32 m_id;
		uint32 m_op = 0;
		bool   m_using = true;
		std::map<std::string, std::string> m_extend_data;
	public:
		data_packet(uint32 size = 1024)
		{
			m_buff_len = size;
			m_buff = new uint8[size];
			memset(m_buff, 0, size);
			static uint32 id = 0;
			m_id = id++;
		}
		~data_packet(){	SAFE_DEL(m_buff);}
	public:
		const uint8* get_buff() { return m_buff; }
		uint32 get_buff_len() { return m_buff_len; }
		void set_using(bool t_using) { m_using = t_using; }
		bool get_using() { return m_using; }
		uint32 get_data_len() { return m_data_len; }
		uint32 get_data_pos() {	return m_pos; }
		uint32 get_op() { return m_op; }
		data_packet& set_op(uint32 op) { m_op = op; return *this;}
		
		data_packet& calculate_data_len_when_read()
		{
			m_data_len = *((uint32*)m_buff);
			m_op = *((uint32*)(m_buff + DATA_HEAD_LEN));
			re_malloc(m_data_len);
			return *this;
		}
		data_packet& reset()
		{
			m_pos = 0;
			m_extend_data.clear();
			return *this;
		}
		data_packet& start_write()
		{
			assert(m_pos == 0);
			put_uint32(m_data_len);
			put_uint32(m_op);
			return *this;
		}
		data_packet& end_write()
		{
			assert(m_pos != 0);
			m_data_len = m_pos;
			m_pos = 0;
			memcpy(m_buff, &m_data_len, DATA_HEAD_LEN);
			memcpy(m_buff + DATA_HEAD_LEN, &m_op, DATA_OP_LEN);
			return *this;
		}
		data_packet& start_read()
		{
			m_pos = 0;
			m_data_len = get_uint32();
			m_op = get_uint32();
			return *this;
		}

		data_packet& put_int_reserved(int val, int pos_reserved)
		{
			memcpy(m_buff + pos_reserved, &val, sizeof(int));
			return *this;
		}
		void assert_using(){ assert(m_using);}
		void copy_from(data_packet* t_packet)
		{
			re_malloc(t_packet->get_data_len());
			memcpy(m_buff, t_packet->get_buff(), t_packet->get_data_len());

			m_pos = t_packet->m_pos;
			m_data_len = t_packet->m_data_len;
			m_op = t_packet->m_op;
			for (auto it : t_packet->m_extend_data)
			{
				m_extend_data.insert(it);
			}
		}
		data_packet& append(const uint8* data, uint32 count)
		{
			while ((m_buff_len - m_pos) < count)
			{
				re_malloc();
			}
			memcpy(m_buff + m_pos, data, count);
			m_pos += count;
			return *this;
		}
		//仅供同一进程间服务使用
		std::string& get_extend_data(const char* key)
		{
			return m_extend_data[key];
		}
		data_packet& put_extend_data(const char* key, const char* value)
		{
			m_extend_data[key] = value;
			return *this;
		}
	private:
		void re_malloc(uint32 new_size = 0)
		{
			new_size = new_size == 0 ? m_buff_len * 2 : new_size;
			if (new_size <= m_buff_len){ return;}
			uint8* temp = m_buff;
			m_buff = new uint8[new_size];
			if (m_buff == NULL){ throw("memory Less Exception");}
			m_buff_len = new_size;
			memset(m_buff, 0, m_buff_len);
			memcpy(m_buff, temp, m_pos);
			delete[] temp;
		}
	public:
		template <typename T> data_packet& put(T value)
		{
			assert_using();
			while ((m_buff_len - m_pos) < sizeof(T))
			{
				re_malloc();
			}
			memcpy(m_buff + m_pos, &value, sizeof(T));
			m_pos += sizeof(T);
			return *this;
		}

		template <> data_packet& put(std::string& value)
		{
			assert_using();
			uint32 str_len = value.size();//可传输二进制
			while ((m_buff_len - m_pos) < (str_len + sizeof(uint32)))
			{
				re_malloc();
			}
			memcpy(m_buff + m_pos, &str_len, sizeof(uint32));
			m_pos += sizeof(uint32);

			memcpy(m_buff + m_pos, value.c_str(), str_len);
			m_pos += str_len;
			return *this;
		}
		template <> data_packet& put(const char* value)
		{
			assert_using();
			uint32 str_len = strlen(value);
			while ((m_buff_len - m_pos) < (str_len + sizeof(uint32)))
			{
				re_malloc();
			}
			memcpy(m_buff + m_pos, &str_len, sizeof(uint32));
			m_pos += sizeof(uint32);

			memcpy(m_buff + m_pos, value, str_len);
			m_pos += str_len;
			return *this;
		}
		/*-------------------------------------------------------------------*/
		template <typename T> T get()
		{
			assert_using();
			if ((m_buff_len - m_pos) < sizeof(T)) throw("buf Less Exception");
			T ret;
			memcpy(&ret, m_buff + m_pos, sizeof(T));
			m_pos += sizeof(T);
			return ret;
		}
		template <> std::string get()
		{
			assert_using();
			if ((m_buff_len - m_pos) < sizeof(uint32)) throw("buf Less Exception");
			uint32 str_len;
			std::string str;
			memcpy(&str_len, m_buff + m_pos, sizeof(uint32));
			m_pos += sizeof(int);

			str.resize(str_len, 0);
			memcpy((void*)str.c_str(), m_buff + m_pos, str_len);
			m_pos += str_len;

			return str;
		}
		/*-------------------------------------------------------------------*/
		data_packet& put_uint8(uint8 val) { return put(val); }
		data_packet& put_int8(int8 val) { return put(val); }
		data_packet& put_uint16(uint16 val) { return put(val); }
		data_packet& put_int16(uint16 val) { return put(val); }
		data_packet& put_uint32(uint32 val){ return put(val); }
		data_packet& put_int32(int32 val) { return put(val); }
		data_packet& put_uint64(uint64 val) { return put(val); }
		data_packet& put_int64(int64 val) { return put(val); }
		data_packet& put_f32(f32 val) { return put(val); }
		data_packet& put_f64(f64 val) { return put(val); }
		data_packet& put_bool(bool val) { return put(val); }
		data_packet& put_string(std::string& val) { return put(val); }
		data_packet& put_string(const char* val) { return put(val); }
			
		uint8 get_uint8() { return get<uint8>(); }
		int8 get_int8() { return get<int8>(); }
		uint16 get_uint16() { return get<uint16>(); }
		int16 get_int16() { return get<int16>(); }
		uint32 get_uint32(){ return get<uint32>(); }
		int32 get_int32() { return get<int32>(); }
		int64 get_uint64() { return get<uint64>(); }
		int64 get_int64() { return get<int64>(); }
		f32 get_f32() { return get<f32>(); }
		f64 get_f64() { return get<f64>(); }
		bool get_bool() { return get<bool>(); }
		std::string get_string() { return get<std::string>(); }
};

};

