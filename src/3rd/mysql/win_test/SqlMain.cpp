#include <stdio.h>
#include "../../../3rd_lib/MySql/MySqlDatabase.h"

int Sql_main()
{
	CMySqlDatabase sDB;
	sDB.Connect("10.1.1.159",3306,"mmorpg_editor","9fang","mmorpg_db");

	CMySqlConnection *conn = sDB.GetConnection();
	if (conn == NULL) {
		printf("load table cache data, but connect database failed.");
		return false;
	}

	CMysqlRstPtr rst = conn->QueryFormat("SELECT * FROM %s", "gma_list_txt");
	if (!rst) {
		printf("load table cache data, but query database failed.");
		sDB.PutConnection(conn);
		return false;
	}

	if (rst->GetRowCount() <= 0) {
		printf("load table cache data, but there is not data.");
		sDB.PutConnection(conn);
		return false;
	}

	const size_t number = 9;
	if (rst->GetFieldCount() != number) {
		printf("load table cache data, but field number is not match.");
	}

	int i = 0;
	do {
		i ++;
		Field *field = rst->Fetch();
		for (size_t index = 0; index < number; ++index, ++field)
			printf("gma_list_txt: %d Row %d Colomn %s \n",i,number,field->GetString());
		//field->GetUInt8() field->GetUInt16() ...
	
	} while (rst->NextRow());

	sDB.PutConnection(conn);

	printf("sqlTest");
	getchar();
	return 0;
}

//////////////////////////////////////////
// 将二进制文件存入数据库 读出数据库///////
/////////////////////////////////////////

// #include "stdafx.h"
// #include <WinSock.h>
// #include "../src/mysql.h"
// #include <fstream>
// #include <string>
// using namespace std;
// 
// int _tmain(int argc, _TCHAR* argv[])
// {
// 	MYSQL* mysqlConn;
// 	my_bool my_true = true;
// 	mysqlConn = mysql_init( NULL );
// 	if( mysqlConn == NULL )
// 	{
// 		return 0;
// 	}
// 	if(mysql_options(mysqlConn, MYSQL_SET_CHARSET_NAME, "utf8"))
// 	{
// 		printf("MySQLDatabase: Could not set utf8 character set.");
// 	}
// 	if (mysql_options(mysqlConn, MYSQL_OPT_RECONNECT, &my_true))
// 	{
// 		printf("MySQLDatabase: MYSQL_OPT_RECONNECT could not be set, connection drops may occur but will be counteracted.");
// 	}
// 
// 	MYSQL* connection = mysql_real_connect( mysqlConn, "localhost", "root", "286339554", "db_test", 3306, NULL, CLIENT_FOUND_ROWS );
// 	if( connection == NULL )
// 	{
// 		printf("MySQLDatabase: Connection failed due to: `%s`", mysql_error( mysqlConn ) );
// 		mysql_close(mysqlConn);
// 		return false;
// 	}
// 
// 	do 
// 	{
// 		ofstream o_file("testOutget.pvr",ios_base::binary);
// 		ifstream pic("test.pvr",ios_base::binary);
// 
// 		pic.seekg(0, ios::end);
// 		int len = pic.tellg();
// 		char *pFile = new char[len*2+1000];
// 		memset(pFile,0,len+10);
// 		char *pSql = new char[len*2+1000];
// 		memset(pSql,0,len+100);
// 		pic.seekg(0,ios::beg);
// 		pic.read(pFile, len);
// 		char _bite[1];
// 		pic.seekg(0,ios::beg);
// 
// 		unsigned long index = 0;
// 		while(!pic.eof())
// 		{
// 			pic.get(_bite,1);
// 			o_file.write(_bite,1);
// 			pic.seekg(++index,ios::beg);
// 		}
// 		_bite[0]= -1;
// 		o_file.write(_bite,1);
// 
// 
// 		pic.close();
// 		o_file.close();
// 		std::string Escape;
// 		std::string ret;
// 
// 		ret.resize(len * 2 + 1);
// 		size_t length = 0;
// 		if (connection != NULL)
// 		{
// 			length = mysql_real_escape_string(connection, (char*)ret.data(), pFile, len);
// 			ret.resize(length);
// 		}
// 		sprintf(pSql,"INSERT INTO `picture` value(NULL,'%s');",ret.c_str());
// 		mysql_query( connection,pSql);
// 
// 		sprintf(pSql,"SELECT * FROM `picture` WHERE ID=5;");
// 		mysql_query( connection,pSql);
// 
// 		MYSQL_RES * pRes = mysql_store_result( connection );
// 
// 		MYSQL_ROW row;
// 		unsigned int num_fields;
// 		unsigned int i;
// 		num_fields = mysql_num_fields(pRes);
// 		while ((row = mysql_fetch_row(pRes)))
// 		{	
// 			for(i = 0; i < num_fields; i++)
// 			{
// 				MYSQL_FIELD *field; 
// 				field = mysql_fetch_field(pRes);
// 				printf("field name %s\n", field->name);
// 
// 				unsigned long *lengths;
// 				lengths = mysql_fetch_lengths(pRes);
// 				printf("lenth:%ld\n",lengths[i]);
// 				printf("[%d %s] ", (int) lengths[i], row[i] ? row[i] : "NULL");
// 
// 				if (lengths[i] > 1000)
// 				{
// 					ofstream o_file22("testOut22.pvr",ios_base::binary);
// 					o_file22.write(row[i],lengths[i]);
// 					ret.clear();
// 					ret.append(row[i],lengths[i]);
// 					printf("ret.lenth:%ld\n",ret.length());
// 				}
// 
// 			}
// 			printf("\n");
// 		}
// 
// 		mysql_free_result(pRes);
// 		delete pFile;
// 		delete pSql;
// 	} while (0);
// 
// 	getchar();
// 
// 	mysql_close(connection);
// 	return 0;
// }



// {
// 	MYSQL* mysqlConn;
// 	my_bool my_true = true;
// 	mysqlConn = mysql_init( NULL );
// 	if( mysqlConn == NULL )
// 	{
// 		return;
// 	}
// 	if(mysql_options(mysqlConn, MYSQL_SET_CHARSET_NAME, "utf8"))
// 	{
// 		printf("MySQLDatabase: Could not set utf8 character set.");
// 	}
// 	if (mysql_options(mysqlConn, MYSQL_OPT_RECONNECT, &my_true))
// 	{
// 		printf("MySQLDatabase: MYSQL_OPT_RECONNECT could not be set, connection drops may occur but will be counteracted.");
// 	}
// 
// 	MYSQL* connection = mysql_real_connect( mysqlConn, "10.1.1.159", "mmorpg_editor", "9fang", "mmorpg_db", 3306, NULL, CLIENT_FOUND_ROWS );
// 	if( connection == NULL )
// 	{
// 		printf("MySQLDatabase: Connection failed due to: `%s`", mysql_error( mysqlConn ) );
// 		mysql_close(mysqlConn);
// 		return ;
// 	}
// 
// 	char pSql[51240];
// 	char dataa[51240];
// 	CTableCache<gma_list_txt>* pTable = sDBMgr->GetTable<gma_list_txt>();
// 	CTableCache<gma_list_txt>::iterator iter = pTable->Begin();
// 	for (;iter != pTable->End(); iter++)
// 	{
// 		mysql_real_escape_string(connection,dataa, iter->second.stringContent[0].c_str(), iter->second.stringContent[0].size());
// 
// 		sprintf(pSql,"UPDATE `gma_list_txt` SET stringContentCN = '%s' WHERE stringID = %d",dataa,iter->second.stringID);
// 		int ss = mysql_query( connection,pSql);
// 		if (ss == -1)
// 		{
// 			ASSERT(0);
// 		}
// 
// 	}
// 
// 
// 	mysql_close(connection);
// }

