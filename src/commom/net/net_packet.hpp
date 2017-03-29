#pragma once
#include "pre_header.h"

namespace gnet {
#define DATA_HEAD_LEN (sizeof(int))
#define DATA_MAX_LEN (5*1024*1024)

class net_packet
{
	public:
		net_packet(int size = 1024)
		{
			m_pos = 0;
			m_buff_len = size;
			m_buff = new uint8[size];
			memset(m_buff, 0, size);
		}

		~net_packet()
		{
			delete[] m_buff;
		}
	public:
		const uint8* get_buff()
		{
			return m_buff;
		}
		int get_buff_len()
		{
			return m_buff_len;
		}
		int get_data_len()
		{
			return m_data_len;
		}
		int get_data_pos()
		{
			return m_pos;
		}
		void  flip()
		{
			m_data_len = m_pos;
			memcpy(m_buff, &m_data_len, DATA_HEAD_LEN);
			m_pos = 0;
		}
		void reset()
		{
			m_pos = 0;
		}
		void skip_head_for_write()
		{
			if (m_pos != 0)
			{
				throw("incorrect use of this function");
			}
			put_uint32(0);
		}
		void skip_head_for_read()
		{
			if (m_pos != 0)
			{
				throw("incorrect use of this function");
			}
			get_uint32();
		}

		
		net_packet& put_int_reserved(int val, int pos_reserved)
		{
			memcpy(m_buff + pos_reserved, &val, sizeof(int));
			return *this;
		}

	private:
		void re_malloc()
		{
			uint8* temp = m_buff;
			m_buff_len = m_buff_len * 2;
			m_buff = new uint8[m_buff_len];
			if (m_buff == NULL)
			{
				throw("memory Less Exception");
			}
			memset(m_buff, 0, m_buff_len);
			memcpy(m_buff, temp, m_pos);
			delete[] temp;
		}
	public:
		net_packet& put_uint32(uint32 val)
		{
			while ((m_buff_len - m_pos) < sizeof(uint32))
			{
				re_malloc();
			}
			memcpy(m_buff + m_pos, &val, sizeof(uint32));
			m_pos += sizeof(uint32);
			return *this;
		}
		uint32 get_uint32()
		{
			if ((m_buff_len - m_pos) < sizeof(uint32))
			{
				throw("buf Less Exception");
			}
			uint32 ret;
			memcpy(&ret, m_buff + m_pos, sizeof(uint32));
			m_pos += sizeof(uint32);
			return ret;
		}
		net_packet& put_int32(int32 val)
		{
			while ((m_buff_len - m_pos) < sizeof(int32))
			{
				re_malloc();
			}
			memcpy(m_buff + m_pos, &val, sizeof(int32));
			m_pos += sizeof(int32);
			return *this;
		}
		int32 get_int32()
		{
			if ((m_buff_len - m_pos) < sizeof(int32))
			{
				throw("buf Less Exception");
			}
			int32 ret;
			memcpy(&ret, m_buff + m_pos, sizeof(int32));
			m_pos += sizeof(int32);
			return ret;
		}

		net_packet& put_bool(bool val)
		{
			while ((m_buff_len - m_pos) < sizeof(uint8))
			{
				re_malloc();
			}
			memset(m_buff + m_pos, val ? 1 : 0, sizeof(uint8));
			m_pos += sizeof(uint8);
			return *this;
		}
		bool get_bool()
		{
			if ((m_buff_len - m_pos) < sizeof(uint8))
			{
				throw("buf Less Exception");
			}
			bool ret = *(m_buff + m_pos) == 1;
			m_pos += sizeof(uint8);
			return ret;
		}

		net_packet& put_float(f32 val)
		{
			while ((m_buff_len - m_pos) < sizeof(f32))
			{
				re_malloc();
			}
			memcpy(m_buff + m_pos, &val, sizeof(f32));
			m_pos += sizeof(f32);
			return *this;
		}
		f32 get_float()
		{
			if ((m_buff_len - m_pos) < sizeof(f32))
			{
				throw(L"buf Less Exception");
			}
			f32 ret;
			memcpy(&ret, m_buff + m_pos, sizeof(f32));
			m_pos += sizeof(f32);
			return ret;
		}
		net_packet& put_string(std::string& val)
		{
			uint32 str_len = val.size();
			while ((m_buff_len - m_pos) < (str_len + sizeof(uint32)))
			{
				re_malloc();
			}
			memcpy(m_buff + m_pos, &str_len, sizeof(uint32));
			m_pos += sizeof(int);

			memcpy(m_buff + m_pos, val.c_str(), str_len);
			m_pos += str_len;
			return *this;
		}

		net_packet& put_string(const char* val)
		{
			uint32 str_len = strlen(val);
			while ((m_buff_len - m_pos) < (str_len + sizeof(uint32)))
			{
				re_malloc();
			}
			memcpy(m_buff + m_pos, &str_len, sizeof(uint32));
			m_pos += sizeof(uint32);

			memcpy(m_buff + m_pos, val, str_len);
			m_pos += str_len;
			return *this;
		}


		std::string get_string()
		{
			if ((m_buff_len - m_pos) < sizeof(uint32))
			{
				throw("buf Less Exception");
			}
			uint32 str_len;
			std::string str;
			memcpy(&str_len, m_buff + m_pos, sizeof(uint32));
			m_pos += sizeof(int);

			str.resize(str_len + 1, 0);
			memcpy((void*)str.c_str(), m_buff + m_pos, str_len);
			m_pos += str_len;

			return str;
		}

		net_packet& put_buff(uint8* data, uint32 count)
		{
			while ((m_buff_len - m_pos) < count)
			{
				re_malloc();
			}
			memcpy(m_buff + m_pos, data, count);
			m_pos += count;
			return *this;
		}
		net_packet* copy_new()
		{
			net_packet* data = new net_packet(m_buff_len);
			memcpy(const_cast<uint8*>(data->get_buff()), get_buff(), m_buff_len);
			data->m_data_len = m_data_len;
			data->m_pos = m_pos;
			return data;
		}
	private:
		uint8* m_buff;
		uint32 m_pos;
		uint32 m_buff_len;
		uint32 m_data_len;
};

};

