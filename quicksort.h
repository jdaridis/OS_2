
typedef struct Record Record;

int partition(Record **array, int low, int high, int (*comparator)(Record* s1, Record* s2));
void quick_sort(Record **array, int low, int high, int (*comparator)(Record* s1, Record* s2));
void swap(Record** r1, Record** r2);
int int_cmp(void* i1, void* i2);
int long_cmp(void* l1, void* l2);
int float_cmp(void* f1, void* f2);
int str_cmp(const void* str1, const void* str2);

int id_cmp(Record* r1, Record* r2);
int name_cmp(Record* r1, Record* r2);
int surname_cmp(Record* r1, Record* r2);
int home_addr_cmp(Record* r1, Record* r2);
int home_num_cmp(Record* r1, Record* r2);
int city_cmp(Record* r1, Record* r2);
int mail_cmp(Record* r1, Record* r2);
int salary_cmp(Record* r1, Record* r2);