#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Record.h"
#include "comparators.h"
#ifdef __HEAP_SORT__ 
#include "heapsort.h"
#else
#include "quicksort.h"
#endif

int main(int argc, char const *argv[]){

     FILE* file;

    Record temp_rec;
    Record** records;

    int record_count = 0;
    int column = 0;
    int start;
    int end;
    float frac;
    int size = -1;
    int fd = -1;
    int bytes_read;
    struct stat stat;
    while(--argc){
        if(strcmp(*argv, "-f") == 0){
            file = fopen(*(argv + 1), "r");
            
        } else if(strcmp(*argv, "-c") == 0){
            column = atoi(*(argv + 1)) - 1;
        } else if(strcmp(*argv, "-s") == 0){
            start = atoi(*(argv + 1));
            end = atoi(*(argv + 2));
        } else if(strcmp(*argv, "-size") == 0){
            size = atoi(*(argv + 1));
        }
        argv++;
    }

    printf("File descreiptor %d, column %d, start %d, end %d\n", fd, column, start, end);

    record_count = end - start;
    
    records = malloc(record_count*sizeof(Record*));
    int i = 0;
    for(i=0;i<record_count;i++){
        records[i] = malloc(sizeof(Record));
    }
    printf("File size %d\n", record_count);

    printf("here\n");
    fseek(file, start*sizeof(Record), SEEK_SET);

    i = 0;
    while(!feof(file) && i < record_count){
        if(!feof(file)){
            fread(records[i], sizeof(Record), 1, file);
            i++;
        }
        
    }

    #ifdef __HEAP_SORT__ 
    heap_sort(records, record_count -1, comparator[column]);
    printf("Using heapsort\n");
    #else
    quick_sort(records, 0, record_count - 1, comparator[column]);
    printf("Using quicksort\n");
    #endif

    printf("----------------------------\n");

    for(i=0;i<record_count;i++){
        printf("%ld %s %s  %s %d %s %s %-9.2f\n", \
		records[i]->id,records[i]->name ,records[i]->surname , \
		records[i]->home_address, records[i]->home_number, records[i]->city, records[i]->mail_sector, \
		records[i]->salary);
    }

    kill(getppid(), SIGUSR2);

    return 0;

}