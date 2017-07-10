#pragma once
#include "../commom/pre_header.h"
#include "mysql_rst.h"
namespace gnet {
	class mysql_connection
	{
		MYSQL* m_connection;
	public:
		mysql_connection(void);
		virtual ~mysql_connection(void);

		virtual bool connect(const char* Hostname, unsigned int port,
			const char* Username, const char* Password, const char* DatabaseName);
		virtual bool select_database(const char* pszDatabaseName);
		virtual void close();
		virtual bool check_valid();

		virtual std::string escape_string(std::string Escape);

		virtual mysql_rst_ptr query_format(const char* QueryString, ...);
		virtual int execute_format(const char* QueryString, ...);

		virtual mysql_rst_ptr query(const char* QueryString);
		virtual int execute(const char* QueryString);
		virtual uint32 get_insert_id();

		virtual void transaction_begin();
		virtual void commit();
		virtual void rollback();

		virtual bool lock(const char* pTableName);
		virtual bool unlock();

		const char* get_last_error();
	protected:
		bool	_query(const char* strSql);
	};

}



