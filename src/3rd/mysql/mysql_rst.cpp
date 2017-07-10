#include "mysql_rst.h"
static const char* INDEX_ERROR = "Index Error!";
using namespace gnet;

mysql_rst::mysql_rst(MYSQL_RES* res, uint32 FieldCount, uint32 RowCount) : m_nFieldCount(FieldCount), m_nRowCount(RowCount), m_pResult( res )
{
	if( res == NULL || FieldCount == 0 )
	{
		m_pCurrentRow	= NULL;
		m_pFields		= NULL;
		m_nRowCount		= 0;
		//m_fieldName.clear();
	}else{
		m_pCurrentRow	= new mysql_field[FieldCount];
		m_pFields		= mysql_fetch_fields(res);
		/*for ( uint32 iField = 0; iField < m_nFieldCount; iField ++ )
		{
			m_fieldName[ m_pFields[iField].name] = iField;
		}*/
	}
}

mysql_rst::~mysql_rst()
{
	//m_fieldName.clear();
	if(m_pResult) mysql_free_result(m_pResult);
	delete [] m_pCurrentRow;
}

bool mysql_rst::next_row()
{
	if( m_nRowCount <= 0 )
		return false;

	MYSQL_ROW row = mysql_fetch_row(m_pResult);
	if(row == NULL)
		return false;

	for(uint32 i = 0; i < m_nFieldCount; ++i)
		m_pCurrentRow[i].set_value(row[i]);

	return true;
}


std::string mysql_rst::get_field_name(uint32 iCol)
{
	if ( iCol < 0 || iCol >= m_nFieldCount)
	{
		return INDEX_ERROR;
	}
	return m_pFields[iCol].name;
}

const mysql_field& mysql_rst::operator[](uint32 iCol) const
{
	assert(m_pCurrentRow&& iCol<= m_nFieldCount);
	return m_pCurrentRow[iCol];
}
