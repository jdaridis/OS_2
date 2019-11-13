#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include "Record.h"

typedef struct Record Record;

typedef struct sorter_records{
    float fraction;
    int start_off;
    Record** records;
} sorter_records;

sorter_records* temp_records;
Record** records;

int signal_counter = 0;

void initialize_temp_records(sorter_records* temp_records,int record_count, int id);
void sig_handler(int sig);

int main(int argc, char const *argv[]){


    FILE* file;
    int id = 0;
    char column[] = "1";
    char sort = 'q';
    int sorters;
    int record_count;
    int node_size;
    int fd = -1;
    int error;
    char file_d[5];
    char* filename;
    char* start_off;
    char end_off[15];

    pid_t pid;

    
    
    Record temp_rec;
    struct stat stat;

    while(--argc){
        if(strcmp(*argv, "-f") == 0){
            file = fopen(*(argv + 1), "r");
            filename = malloc(strlen(*(argv + 1)) + 1);
            strcpy(filename, *(argv + 1));
        } else if(strcmp(*argv, "-fd") == 0){
            fd = open(*(argv + 1), O_RDONLY);
            fstat(fd, &stat);
            filename = malloc(strlen(*(argv + 1)) + 1);
            strcpy(filename, *(argv + 1));
            // printf("Fd is %d, size is %d, number of records is %d\n", fd, stat.st_size, stat.st_size/sizeof(Record));
            // exit(0);
        } else if(strcmp(*argv, "-h") == 0){
            sort = 'h';
            strcpy(column, *(argv + 1));
        } else if(strcmp(*argv, "-q") == 0){
            sort = 'q';
            strcpy(column, *(argv + 1));
        } else if(strcmp(*argv, "-id") == 0){
            id = atoi(*(argv + 1));
        }
        argv++;
    }

    sorters = 1 << id;

    if(fd != -1){
        record_count = stat.st_size/sizeof(Record);
        records = malloc(record_count*sizeof(Record));
        start_off = malloc(record_count);
        
    } else {
          while(!feof(file)){
        
            fread(&temp_rec, sizeof(Record), 1, file);
            if(!feof(file)){
                record_count++;
            }
            
        }
        records = malloc(record_count*sizeof(Record));
    }

    int i = 0;
    for(i=0;i<record_count;i++){
        records[i] = malloc(sizeof(Record));
    }

    temp_records = malloc(sorters*sizeof(sorter_records));
    
    initialize_temp_records(temp_records, record_count, id);
    signal(SIGUSR2, sig_handler);
    for(i = 0;i<sorters;i++){
        pid = fork();
        if(pid == 0){
            sprintf(start_off, "%d", temp_records[i].start_off);
            sprintf(end_off, "%d", (int)(temp_records[i].start_off + temp_records[i].fraction*record_count));
           printf("Start %s end %s\n", start_off, end_off);
           if(sort == 'h'){
                printf("Excecuting heap sort\n");
                execlp("./heapsort", "heapsort", "-f", filename, "-c", column, "-s", start_off, end_off, (char  *) NULL);
           } else {
                printf("Excecuting quick sort\n");
                execlp("./quicksort", "quicksort", "-f", filename, "-c", column, "-s", start_off, end_off, (char  *) NULL);
                printf("Error\n");           
           }
        }
    }

    

    while (wait(NULL) > 0);
    // wait(NULL);

    printf("SIgnals %d\n", signal_counter);
    
    return 0;
}


void sig_handler(int sig){
    signal(SIGUSR2, sig_handler);
    signal_counter++;
    printf("Child finished\n");
}




void initialize_temp_records(sorter_records* temp_records,int record_count, int id){
    int node_size;
    int i;
    switch (id){
    case 0:
        temp_records->records = malloc(record_count*sizeof(Record));
        temp_records->start_off = 0;
        temp_records->fraction = 1.0;
        for(i=0;i<record_count;i++){
            temp_records->records[i] = malloc(sizeof(Record));
        }
        break;
    case 1:
        temp_records[0].fraction = 0.5;
        temp_records[0].start_off = 0;

        temp_records[1].fraction = 0.5;
        temp_records[1].start_off = 0.5*record_count;

        node_size = 0.5*record_count;
        printf("node size %d\n", node_size);
        temp_records[0].records = malloc(node_size*sizeof(Record));
        temp_records[1].records = malloc(node_size*sizeof(Record));
        for(i=0;i<node_size;i++){
            temp_records[0].records[i] = malloc(sizeof(Record));
            temp_records[0].records[i] = malloc(sizeof(Record));
        }
        break;

    case 2:
        node_size = (1.0/8.0)*record_count;
        printf("node size %d\n", node_size);
        temp_records[0].records = malloc(node_size*sizeof(Record));
        temp_records[0].fraction = (1.0/8.0);
        temp_records[0].start_off = 0;

        temp_records[1].records = malloc(node_size*sizeof(Record));
        temp_records[1].fraction = (1.0/8.0);
        temp_records[1].start_off = (1.0/8.0)*record_count;

        for(i=0;i<node_size;i++){
            temp_records[0].records[i] = malloc(sizeof(Record));
            temp_records[1].records[i] = malloc(sizeof(Record));
        }
        node_size = (1.0/4.0)*record_count;
        temp_records[2].records = malloc(node_size*sizeof(Record));
        temp_records[2].fraction = (1.0/4.0);
        temp_records[2].start_off = temp_records[1].start_off + temp_records[1].fraction *record_count;

         for(i=0;i<node_size;i++){
            temp_records[2].records[i] = malloc(sizeof(Record));
        }
        node_size = (2.0/4.0)*record_count;
        temp_records[3].records = malloc(node_size*sizeof(Record));
        temp_records[3].fraction = (2.0/4.0);
        temp_records[3].start_off = temp_records[2].start_off + temp_records[2].fraction*record_count;
         for(i=0;i<node_size;i++){
            temp_records[3].records[i] = malloc(sizeof(Record));
        }
        break;
    case 3:
        node_size = (1.0/16.0)*record_count;
        temp_records[0].records = malloc(node_size*sizeof(Record));
        temp_records[0].fraction = (1.0/16.0);
        temp_records[0].start_off = 0;

        temp_records[1].records = malloc(node_size*sizeof(Record));
        temp_records[1].fraction = (1.0/16.0);
        temp_records[1].start_off = (1.0/16.0)*record_count;

        temp_records[2].records = malloc(node_size*sizeof(Record));
        temp_records[2].fraction =  (1.0/16.0);
        temp_records[2].start_off = temp_records[1].start_off +  temp_records[1].fraction*record_count;

        temp_records[3].records = malloc(node_size*sizeof(Record));
        temp_records[3].fraction = (1.0/16.0);
        temp_records[3].start_off = temp_records[2].start_off + temp_records[2].fraction*record_count;
        for(i=0;i<node_size;i++){
            temp_records[0].records[i] = malloc(sizeof(Record));
            temp_records[1].records[i] = malloc(sizeof(Record));
            temp_records[2].records[i] = malloc(sizeof(Record));
            temp_records[3].records[i] = malloc(sizeof(Record));
        }
        node_size = (2.0/16.0)*record_count;
        temp_records[4].records = malloc(node_size*sizeof(Record));
        temp_records[4].fraction = (2.0/16.0);
        temp_records[4].start_off =  temp_records[3].start_off + temp_records[3].fraction*record_count;

        temp_records[5].records = malloc(node_size*sizeof(Record));
        temp_records[5].fraction = (2.0/16.0);
        temp_records[5].start_off = temp_records[4].start_off + temp_records[4].fraction*record_count;
        for(i=0;i<node_size;i++){
            temp_records[4].records[i] = malloc(sizeof(Record));
            temp_records[5].records[i] = malloc(sizeof(Record));
        }
        node_size = (4.0/16.0)*record_count;
        temp_records[6].records = malloc(node_size*sizeof(Record));
        temp_records[6].fraction = (4.0/16.0);
        temp_records[6].start_off = temp_records[5].start_off + temp_records[5].fraction*record_count;

        temp_records[7].records = malloc(node_size*sizeof(Record));
        temp_records[7].fraction = (4.0/16.0);
        temp_records[7].start_off = temp_records[6].start_off + temp_records[6].fraction*record_count;
        for(i=0;i<node_size;i++){
            temp_records[6].records[i] = malloc(sizeof(Record));
            temp_records[7].records[i] = malloc(sizeof(Record));
        }
        break;
    default:
        break;
    }

}
