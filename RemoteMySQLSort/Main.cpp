/*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mysql/mysql.h"

//-----------------------------------------------------------------------------
int main(int argc, char** argv) {
	MYSQL m_dbCon;

	if(mysql_init(&m_dbCon) == NULL) {
		fprintf(stderr, "ERROR: \"%s\"\n", mysql_error(&m_dbCon));
		system("pause");
		return -1;
	}

	if(mysql_real_connect(&m_dbCon, "23.88.121.38", "root", "root_password", "employees", 0, NULL, 0) == NULL) {
		fprintf(stderr, "ERROR: \"%s\"\n", mysql_error(&m_dbCon));
		mysql_close(&m_dbCon);
		system("pause");
		return -1;
	}

	if(mysql_query(&m_dbCon, "SELECT * FROM employees;")) {
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

	mysql_close(&m_dbCon);

	system("pause");
	return 0;
}
