/*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mysql/mysql.h"

//-----------------------------------------------------------------------------
class DBQuery {
private:
	MYSQL*       m_pDBCon;
	MYSQL_RES*   m_pResult;
	char*        m_strQuery;
	unsigned int m_uiNumFields;
	MYSQL_FIELD* m_pFields;

protected:
	void _reset(void);

public:
	bool Execute(const char* strQuery);
	const char* GetQuery(void) { return m_strQuery; }

public:
	DBQuery(MYSQL* pDBCon);
	~DBQuery(void);
};

//-----------------------------------------------------------------------------
DBQuery::DBQuery(MYSQL* pDBCon) {
	m_pDBCon      = pDBCon;
	m_pResult     = NULL;
	m_strQuery    = NULL;
	m_uiNumFields = 0;
	m_pFields     = NULL;
}

//-----------------------------------------------------------------------------
DBQuery::~DBQuery(void) {
	_reset();
}

//-----------------------------------------------------------------------------
void DBQuery::_reset(void) {
	if(m_strQuery) free(m_strQuery);
	if(m_pResult) mysql_free_result(m_pResult);
	if(m_pFields) free(m_pFields);
}

//-----------------------------------------------------------------------------
bool DBQuery::Execute(const char* strQuery) {
	if(strQuery == NULL) return false;

	_reset();

	int iStrLen = strlen(strQuery);
	m_strQuery = (char*)calloc((iStrLen+1), sizeof(char));
	memcpy(m_strQuery, strQuery, iStrLen);

	if(mysql_query(m_pDBCon, m_strQuery)) {
		fprintf(stderr, "ERROR: \"%s\"\n", mysql_error(m_pDBCon));
		return false;
	}

	m_pResult = mysql_store_result(m_pDBCon);

	if(m_pResult == NULL) {
		fprintf(stderr, "ERROR: \"%s\"\n", mysql_error(m_pDBCon));
		return false;
	}

	m_uiNumFields = mysql_num_fields(m_pResult);
	m_pFields = (MYSQL_FIELD*)malloc(m_uiNumFields*sizeof(MYSQL_FIELD));

	MYSQL_FIELD* field;
	for(int i=0; (field=mysql_fetch_field(m_pResult)); ++i) {
		m_pFields[i] = *field;
	}

	MYSQL_ROW row;
	for(int i=0; (row = mysql_fetch_row(m_pResult)); ++i) {
		unsigned long* lengths = mysql_fetch_lengths(m_pResult);

		unsigned int j;
		for(j=0; j<m_uiNumFields; ++j) {

			char first_name[15] = "";

			if(!strcmp(m_pFields[j].name, "first_name")) {
				if(m_pFields[j].type & MYSQL_TYPE_STRING) {
					memcpy(first_name, row[j], lengths[j]);
					printf("first_name: %s\n", first_name);
				}
			}

		}
	}
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv) {
	MYSQL db_con;

	if(mysql_init(&db_con) == NULL) {
		fprintf(stderr, "ERROR: \"%s\"\n", mysql_error(&db_con));
		system("pause");
		return -1;
	}

	if(mysql_real_connect(&db_con, "23.88.121.38", "root", "root_password", "employees", 0, NULL, 0) == NULL) {
		fprintf(stderr, "ERROR: \"%s\"\n", mysql_error(&db_con));
		mysql_close(&db_con);
		system("pause");
		return -1;
	}

	DBQuery query(&db_con);
	query.Execute("SELECT * FROM employees LIMIT 10;");

	/*
	if(mysql_query(&m_dbCon, "SELECT * FROM employees LIMIT 10;")) {
		fprintf(stderr, "ERROR: \"%s\"\n", mysql_error(&m_dbCon));
		system("pause");
		return -1;
	}

	MYSQL_RES* result = mysql_store_result(&m_dbCon);
	if(result == NULL) {
		fprintf(stderr, "ERROR: \"%s\"\n", mysql_error(&m_dbCon));
		system("pause");
		return -1;
	}

	unsigned int nFields = mysql_num_fields(result);
	MYSQL_FIELD** ppFields = (MYSQL_FIELD**)malloc(nFields*sizeof(MYSQL_FIELD*));

	MYSQL_FIELD* pField;
	for(int f=0; (pField=mysql_fetch_field(result)); ++f) {
		ppFields[f] = pField;
	}

	MYSQL_ROW row;
	for(int r=0; (row = mysql_fetch_row(result)); ++r) {
		unsigned int i = 0;
		unsigned long* lens = mysql_fetch_lengths(result);

		unsigned int f;
		for(f=0; f<nFields; ++f) {

			char first_name[15] = "";

			if(!strcmp(ppFields[f]->name, "first_name")) {
				if(ppFields[f]->type & MYSQL_TYPE_STRING) {
					memcpy(first_name, row[f], lens[f]);
					printf("first_name: %s\n", first_name);
				}
			}

		}
	}

	free(ppFields);
	mysql_free_result(result);
	*/

	mysql_close(&db_con);

	system("pause");
	return 0;
}
