
typedef struct Record Record;

int partition(Record **array, int low, int high, int (*comparator)(Record* s1, Record* s2));
void quick_sort(Record **array, int low, int high, int (*comparator)(Record* s1, Record* s2));


