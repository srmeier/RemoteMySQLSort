/*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mysql/mysql.h"

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
int cmp_hire_date(const MYSQL_ROW a, const MYSQL_ROW b) {
	char tmp[9] = "";
	int year, month, day;

	sscanf(a[H("hire_date")], "%d-%d-%d", &year, &month, &day);
	sprintf(tmp, "%04d%02d%02d", year, month, day);
	int a_key = atoi(tmp);

	sscanf(b[H("hire_date")], "%d-%d-%d", &year, &month, &day);
	sprintf(tmp, "%04d%02d%02d", year, month, day);
	int b_key = atoi(tmp);

	return (a_key - b_key);
}

/* INSERTION SORT
- best    O(n)
- average O(n^2)
- worst   O(n^2)
*/
// NOTE: best for arrays that are small or nearly sorted
//-----------------------------------------------------------------------------
void insert_sort(MYSQL_ROW* rows, int n,
	int (*cmp)(const MYSQL_ROW a, const MYSQL_ROW b)
) {
	for(int j=1; j<n; ++j) {
		int i = j-1;

		MYSQL_ROW row = rows[j];
		while(i>=0 && cmp(rows[i], row)>0) {
			rows[i+1] = rows[i];
			i--;
		}

		rows[i+1] = row;
	}
}

// NOTE: linear time partition function returning index "store" such that
// [left, store] is less than or equal to "pivot" and (store, right] is greater
// than "pivot" (no sorting is being done here)
//-----------------------------------------------------------------------------
int _partition(MYSQL_ROW* rows, int(*cmp)(const MYSQL_ROW a, const MYSQL_ROW b),
	int left, int right, int pivot_index
) {
	int idx, store;
	MYSQL_ROW pivot = rows[pivot_index];

	// move pivot to the end of the array
	MYSQL_ROW tmp = rows[right];
	rows[right] = pivot;
	rows[pivot_index] = tmp;

	// all values <= pivot are moved to the front of array and pivot is
	// inserted just after them
	store = left;
	for(idx = left; idx<right; ++idx) {
		if(cmp(rows[idx], pivot) <= 0) {
			tmp = rows[idx];
			rows[idx] = rows[store];
			rows[store] = tmp;
			store++;
		}
	}

	tmp = rows[right];
	rows[right] = rows[store];
	rows[store] = tmp;

	return store;
}

// NOTE: average-time linear (worst-case O(n^2)) function to find the position of
// the kth element in a subarray [left, right]
//-----------------------------------------------------------------------------
int _select_kth(MYSQL_ROW* rows, int(*cmp)(const MYSQL_ROW a, const MYSQL_ROW b),
	int k, int left, int right
) {
	// use a random first guess
	int idx = rand()%((right+1)-left)+left;
	int pivot_index = _partition(rows, cmp_hire_date, left, right, idx);

	if(left+k-1 == pivot_index) { return pivot_index; }

	// continue the loop, narrowing the range as appropriate. if we are within
	// the left-hand side of the pivot then k can stay the same
	if(left+k-1 < pivot_index)
		return _select_kth(rows, cmp, k, left, pivot_index-1);
	else
		return _select_kth(rows, cmp, k-(pivot_index-left+1), pivot_index+1, right);
}

/* MEDIAN SORT
- best    O(n*log(n))
- average O(n*log(n))
- worst   O(n^2)
*/
//-----------------------------------------------------------------------------
void _median_sort(MYSQL_ROW* rows, int(*cmp)(const MYSQL_ROW a, const MYSQL_ROW b),
	int left, int right
) {
	if(right <= left) return;

	int mid = (right-left+1)/2;
	int me  = _select_kth(rows, cmp, mid+1, left, right);

	_median_sort(rows, cmp, left, left+mid-1);
	_median_sort(rows, cmp, left+mid+1, right);
}

void median_sort(MYSQL_ROW* rows, int n,
	int(*cmp)(const MYSQL_ROW a, const MYSQL_ROW b)
) {
	_median_sort(rows, cmp, 0, n-1);
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

	if(mysql_query(&db_con, "SELECT * FROM employees LIMIT 20;")) {
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

	//insert_sort(rows, num_rows, cmp_hire_date);
	median_sort(rows, num_rows, cmp_hire_date);

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

		printf("%s\n", rows[i][H("hire_date")]);
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
