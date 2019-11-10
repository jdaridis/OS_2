typedef struct Record Record;
void heap_sort(Record** array, int size,int (*comparator)(Record*, Record*));
void heapify(Record** array, int size,int (*comparator)(Record*, Record*));
void heap_fixup(Record** array,int size, int i, int (*comparator)(Record*, Record*));
int parent(int i);

int left_child(int i);
int right_child(int i);