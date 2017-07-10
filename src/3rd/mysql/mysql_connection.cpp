#include "mysql_connection.h"

using namespace gnet;
#define SQL_BUFFER_MAX (65536)

mysql_connection::mysql_connection(void)
{
	m_connection = NULL;
}

mysql_connection::~mysql_connection(void)
{
	close();
}

bool mysql_connection::connect(const char* Hostname, unsigned int port,
							 const char* Username, const char* Password, const char* DatabaseName)
{
	my_bool my_true = true;

	MYSQL* mysqlConn;
	mysqlConn = mysql_init( NULL );
	if( mysqlConn == NULL )
		return false;

	if(mysql_options(mysqlConn, MYSQL_SET_CHARSET_NAME, "utf8"))
		printf("MySQLDatabase: Could not set utf8 character set.");

	if (mysql_options(mysqlConn, MYSQL_OPT_RECONNECT, &my_true))
		printf("MySQLDatabase: MYSQL_OPT_RECONNECT could not be set, connection drops may occur but will be counteracted.");

	m_connection = mysql_real_connect( mysqlConn, Hostname, Username, Password, DatabaseName, port, NULL, CLIENT_MULTI_STATEMENTS );
	if( m_connection == NULL )
	{
		printf("MySQLDatabase: Connection failed due to: `%s`", mysql_error( mysqlConn ) );
		mysql_close(mysqlConn);
		return false;
	}
	return true;
}

void mysql_connection::close()
{
	if (m_connection)
		mysql_close(m_connection);
	m_connection = NULL;
}

bool mysql_connection::check_valid()
{
	return m_connection && (mysql_ping(m_connection) == 0);
}

bool mysql_connection::select_database(const char* pszDatabaseName)
{
	int nResult = m_connection ? mysql_select_db(m_connection, pszDatabaseName) : -1;
	return (nResult == 0);
}

std::string mysql_connection::escape_string(std::string Escape)
{
	std::string ret;
	ret.resize(Escape.size() * 2 + 1);
	size_t length = 0;
	if (m_connection != NULL)
	{
		length = mysql_real_escape_string(m_connection, (char*)ret.data(), Escape.c_str(), Escape.size());
	}
	else
	{
		length = mysql_escape_string((char*)ret.data(), Escape.c_str(), Escape.size());
	}
	ret.resize(length);
	return ret;
}

bool mysql_connection::_query(const char* strSql)
{	
	if ( !m_connection )
		return false;
	int result = mysql_query( m_connection,strSql);
	if(result > 0)
	{
		const char* pError = get_last_error() ;
		printf("%s/n",pError?pError:"Unknown Error!");
	}
	return (result == 0 ? true : false);
}

mysql_rst_ptr mysql_connection::query_format(const char* szQueryString, ...)
{
	char sql[SQL_BUFFER_MAX];
	va_list vlist;
	va_start(vlist, szQueryString);
	vsnprintf(sql, SQL_BUFFER_MAX, szQueryString, vlist);
	va_end(vlist);

	return query(sql);
}

int mysql_connection::execute_format(const char* szQueryString, ...)
{
	char sql[SQL_BUFFER_MAX];
	va_list vlist;
	va_start(vlist, szQueryString);
	vsnprintf(sql, SQL_BUFFER_MAX, szQueryString, vlist);
	va_end(vlist);

	return execute(sql);
}

int mysql_connection::execute(const char* szQueryString)
{
	bool isSuc = _query(szQueryString);
	if ( !isSuc )
	{
		return -1;
	}
	return (int)mysql_affected_rows(m_connection);
}

mysql_rst_ptr mysql_connection::query(const char* szQueryString)
{
	bool isSuc = _query( szQueryString);
	if(!isSuc)
	{
		return NULL;
	}

	MYSQL_RES * pRes = mysql_store_result( m_connection );
	uint32 uRows = (uint32)mysql_affected_rows( m_connection );
	uint32 uFields = (uint32)mysql_field_count( m_connection );

	mysql_rst_ptr pRst(new mysql_rst( pRes, uFields, uRows ));
	pRst->next_row();
	return pRst;
}

uint32 mysql_connection::get_insert_id()
{
	return (uint32)mysql_insert_id(m_connection);
}

 void mysql_connection::transaction_begin()
{
	_query("START TRANSACTION");
}
 void mysql_connection::commit() 
{
	_query("COMMIT");
}
 void mysql_connection::rollback()
 {
	 _query("ROLLBACK");
 }

 bool mysql_connection::lock( const char* pTableName )
 {
	 char buff[1024];
	 sprintf(buff,"LOCK TABLES %s WRITE",pTableName);
	 _query( buff );
	 return true;
 }

 bool mysql_connection::unlock( )
 {
	 _query( "UNLOCK TABLES " );
	 return true;
 }

const char* mysql_connection::get_last_error()
{
	if ( !m_connection )
		return "Invalid Connection";
	if(mysql_errno(m_connection))
	{
		return mysql_error(m_connection);
	}else{
		return NULL;
	}
}
