/*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mysql/mysql.h"

//-----------------------------------------------------------------------------
void insert_sort(MYSQL_ROW* rows, int n,
	int (*cmp)(const MYSQL_ROW* a, const MYSQL_ROW* b)
) {

}

//-----------------------------------------------------------------------------
#define H(key) hash_table[hash_multiplicative(key)]

#define HASH_SIZE 1000
int hash_table[HASH_SIZE];

unsigned int hash_multiplicative(const char* key) {
	unsigned int hash = 0;

	int len = strlen(key);
	for(int i=0; i<len; ++i) {
		hash = 31*hash + key[i];
	}

	return (hash % HASH_SIZE);
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

	if(mysql_query(&db_con, "SELECT * FROM employees LIMIT 5;")) {
		fprintf(stderr, "ERROR: \"%s\"\n", mysql_error(&db_con));
		system("pause");
		return -1;
	}

	MYSQL_RES* result = mysql_store_result(&db_con);
	if(result == NULL) {
		fprintf(stderr, "ERROR: \"%s\"\n", mysql_error(&db_con));
		system("pause");
		return -1;
	}

	unsigned int num_fields = mysql_num_fields(result);
	MYSQL_FIELD** fields = (MYSQL_FIELD**)malloc(num_fields*sizeof(MYSQL_FIELD*));

	MYSQL_FIELD* field;
	for(int f=0; (field=mysql_fetch_field(result)); ++f) {
		fields[f] = field, H(field->name) = f;
	}

	unsigned long long num_rows = mysql_num_rows(result);
	MYSQL_ROW* rows = (MYSQL_ROW*)malloc((size_t)num_rows*sizeof(MYSQL_ROW));

	unsigned long** lengths = (unsigned long**)malloc((size_t)num_rows*sizeof(unsigned long*));

	MYSQL_ROW row;
	for(int r=0; (row = mysql_fetch_row(result)); ++r) {
		rows[r] = row, lengths[r] = mysql_fetch_lengths(result);
	}

	// TESTING

	// NOTE: the index into the array to be sorted is "i" and the value is one of the fields
	for(int i=0; i<num_rows; ++i) {
		/*
		printf("emp_no:     %s\n", rows[i][H("emp_no")]);
		printf("birth_date: %s\n", rows[i][H("birth_date")]);
		printf("first_name: %s\n", rows[i][H("first_name")]);
		printf("last_name:  %s\n", rows[i][H("last_name")]);
		printf("gender:     %s\n", rows[i][H("gender")]);
		printf("hire_date:  %s\n", rows[i][H("hire_date")]);
		printf("\n");
		*/

		int b_year, b_month, b_day;
		sscanf(rows[i][H("hire_date")], "%d-%d-%d", &b_year, &b_month, &b_day);

		char tmp[9] = "";
		sprintf(tmp, "%04d%02d%02d", b_year, b_month, b_day);
		int key = atoi(tmp);

		printf("%d\n", key);

		
	}

	// TESTING

	free(lengths);
	free(rows);
	free(fields);
	mysql_free_result(result);

	mysql_close(&db_con);

	system("pause");
	return 0;
}
