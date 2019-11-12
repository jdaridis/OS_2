#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Record Record;

int id_cmp(Record* r1, Record* r2);
int name_cmp(Record* r1, Record* r2);
int surname_cmp(Record* r1, Record* r2);
int home_addr_cmp(Record* r1, Record* r2);
int home_num_cmp(Record* r1, Record* r2);
int city_cmp(Record* r1, Record* r2);
int mail_cmp(Record* r1, Record* r2);
int salary_cmp(Record* r1, Record* r2);



int (*comparator[8])(Record*, Record*);