#pragma once
#ifdef _MSC_VER
#ifdef _WIN64
#include <WinSock2.h>
#elif _WIN32
#include <winsock.h>
#endif
#endif
#include "../commom/pre_header.h"
#include "mysql/mysql.h"

#ifdef __GNUC__
	#define stricmp strcasecmp
	#define strnicmp strncasecmp
	#define I64FMT "%016llX"
	#define I64FMTD "%llu"
	#define SI64FMTD "%lld"
#else
	#define I64FMT "%016I64X"
	#define I64FMTD "%I64u"
	#define SI64FMTD "%I64d"
	#define atoll _atoi64
	#define ltoa  _ltoa
#endif
namespace gnet{
	class mysql_field
	{
	public:

		void set_value(char* value) { m_value = value; }

		const char *get_string()const {
			if (m_value == NULL)
				return "";
			return m_value;
		}
		float get_float()const { return m_value ? static_cast<float>(atof(m_value)) : 0; }
		bool get_bool()const { return m_value ? atoi(m_value) > 0 : false; }
		uint8 get_uint8()const { return m_value ? static_cast<uint8>(atoll(m_value)) : 0; }
		int8 get_int8()const { return m_value ? static_cast<int8>(atoll(m_value)) : 0; }
		uint16 get_uint16()const { return m_value ? static_cast<uint16>(atoll(m_value)) : 0; }
		int16 get_int16()const { return m_value ? static_cast<int16>(atoll(m_value)) : 0; }
		uint32 get_uint32()const { return m_value ? static_cast<uint32>(atoll(m_value)) : 0; }
		int32 get_int32() const { return m_value ? static_cast<int32>(atoll(m_value)) : 0; }
		int64 get_int64() const { return m_value ? static_cast<int64>(atoll(m_value)) : 0; }
		uint64 get_uint64()const
		{
			if (m_value)
			{
				uint64 value;
#ifndef _WIN32	// Make GCC happy.
				sscanf(m_value, I64FMTD, (long long unsigned int*)&value);
#else
				sscanf_s(m_value, I64FMTD, &value);
#endif
				return value;
			}
			else
				return 0;
		}

	private:
		char *m_value;
	};


	class mysql_rst
	{
		friend class mysql_rst_ptr;
	public:
		mysql_rst(MYSQL_RES* res, uint32 FieldCount, uint32 RowCount);

		bool next_row();

		mysql_field* fetch() { return m_pCurrentRow; }
		uint32 get_field_count() const { return m_nFieldCount; }
		uint32 get_row_count() const { return m_nRowCount; }


		std::string get_field_name(uint32 iCol);
		const mysql_field& operator[](uint32 iCol) const;

	protected:
		~mysql_rst();
		void destroy() { delete this; };

	protected:
		uint32 m_nFieldCount;
		uint32 m_nRowCount;
		mysql_field * m_pCurrentRow;
		MYSQL_RES* m_pResult;
		MYSQL_FIELD* m_pFields;
	};


	class mysql_rst_ptr
	{
	public:
		mysql_rst_ptr(const mysql_rst_ptr& rst) : m_rst(NULL)
		{
			*this = rst;
		};
		mysql_rst_ptr(mysql_rst* rst = NULL) : m_rst(rst)
		{
		};
		~mysql_rst_ptr()
		{
			destroy();
		};

		mysql_rst_ptr & operator = (const mysql_rst_ptr& arst)
		{
			destroy();
			m_rst = arst.m_rst;
			arst.m_rst = NULL;
			return *this;
		};
		mysql_rst * operator -> ()
		{
			assert(m_rst != NULL);
			return m_rst;
		};
		const mysql_rst * operator -> () const
		{
			assert(m_rst != NULL);
			return m_rst;
		};
		mysql_rst & operator*() const
		{
			assert(m_rst != NULL);
			return *m_rst;
		}
		mysql_rst * Get()
		{
			return m_rst;
		};
		operator bool() const
		{
			return m_rst != NULL;
		};
		bool operator! () const // never throws
		{
			return m_rst == NULL;
		};
	public:
		void destroy()
		{
			if (m_rst)
			{
				m_rst->destroy();
				m_rst = NULL;
			}
		};
	private:
		mutable mysql_rst * m_rst;
	};
}
