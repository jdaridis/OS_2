#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIELD_NUM 8

typedef struct Record Record;

int id_cmp(Record* r1, Record* r2);
int name_cmp(Record* r1, Record* r2);
int surname_cmp(Record* r1, Record* r2);
int home_addr_cmp(Record* r1, Record* r2);
int home_num_cmp(Record* r1, Record* r2);
int city_cmp(Record* r1, Record* r2);
int mail_cmp(Record* r1, Record* r2);
int salary_cmp(Record* r1, Record* r2);


/* Available comparators:
ID: 0
Name; 1
Surname: 2 
Home address: 3
Home number: 4
City: 5
Mail: 6
Salary: 7
 */
int (*comparator[FIELD_NUM])(Record*, Record*);