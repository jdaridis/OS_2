
typedef struct Record Record;
typedef struct sorter_records{
    int start_off;
    int size;
    Record** records;
    float fraction;
    double exec_time;
} sorter_records;
void initialize_temp_records(sorter_records* temp_records,int record_count, int id);
void sig_handler(int sig);
void merge(sorter_records* temp_records, int sorters, Record** records, int record_count, int (*comparator)(Record*, Record*));
