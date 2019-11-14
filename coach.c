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
#include "comparators.h"

typedef struct Record Record;

typedef struct sorter_records{
    float fraction;
    int start_off;
    int size;
    Record** records;
} sorter_records;

sorter_records* temp_records;
Record** records;

int signal_counter = 0;

void initialize_temp_records(sorter_records* temp_records,int record_count, int id);
void sig_handler(int sig);
void merge(sorter_records* temp_records, int sorters, Record** records, int record_count, int (*comparator)(Record*, Record*));
int main(int argc, char const *argv[]){


    FILE* file;
    int id = 0;
    char column[] = "1";
    char sort = 'q';
    int sorters;
    int** pipes;
    int record_count;
    int node_size;
    int fd = -1;
    int error;
    char pipe_file_d[5];
    char sorter_id[2];
    char* filename;
    char* start_off;
    char end_off[15];
    int bytes_read;
    int total_bytes;
    int total_bytes_read;
    Record pipe_buffer;
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
    pipes = malloc(sorters * sizeof(int*));

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
        records = malloc(record_count*sizeof(Record*));
    }

    int i = 0;
    for(i=0;i<record_count;i++){
        records[i] = malloc(sizeof(Record));
    }

    temp_records = malloc(sorters*sizeof(sorter_records));
    
    initialize_temp_records(temp_records, record_count, id);
    signal(SIGUSR2, sig_handler);

    for(i = 0;i<sorters;i++){
       pipes[i] = malloc(2*sizeof(int));
       
       pipe(pipes[i]);
       
    }

    
    for(i = 0;i<sorters;i++){
        pid = fork();
        if(pid == 0){
            close(pipes[i][0]);
            sprintf(start_off, "%d", temp_records[i].start_off);
            sprintf(end_off, "%d", temp_records[i].start_off + temp_records[i].size);
            sprintf(pipe_file_d, "%d", pipes[i][1]);
            sprintf(sorter_id, "%d", i);
            printf("Start %s end %s\n", start_off, end_off);
            if(sort == 'h'){
                printf("Excecuting heap sort\n");
                execlp("./heapsort", "heapsort", "-f", filename, "-id", sorter_id, "-p", pipe_file_d,"-c", column, "-s", start_off, end_off, (char  *) NULL);
            } else {
                printf("Excecuting quick sort\n");
                execlp("./quicksort", "quicksort", "-f", filename,"-id", sorter_id, "-p", pipe_file_d, "-c", column, "-s", start_off, end_off, (char  *) NULL);
                printf("Error\n");           
            }
        } else {
            close(pipes[i][1]);
        }
    }
    for(i = 0;i<sorters;i++){
        close(pipes[i][1]);
        int j = 0;
      do{
        //   printf("Before read\n");
          bytes_read = read(pipes[i][0], temp_records[i].records[j],sizeof(Record));
        //   printf("After read\n");
        //   memcpy(temp_records[i].records[j], &pipe_buffer, sizeof(Record));
        //   if(bytes_read >0  && j < temp_records[i].size)
        //   printf("Just Read %ld %s %s  %s %d %s %s %-9.2f\n", \
        //     temp_records[i].records[j]->id,temp_records[i].records[j]->name ,temp_records[i].records[j]->surname , \
        //     temp_records[i].records[j]->home_address, temp_records[i].records[j]->home_number, temp_records[i].records[j]->city, temp_records[i].records[j]->mail_sector, \
        //     temp_records[i].records[j]->salary);
            
          j++;
        //   printf("Sorter %d j =  %d\n", i, j);
      }while(bytes_read > 0 && j < temp_records[i].size);
    }

 /* int j = 0;
    do{
        for(i=0;i<sorters;i++){
            if(j < temp_records[i].size){
                bytes_read = 0;
                total_bytes = 0;
                do{
                    printf("reading form pipe %d\n", pipes[i][0]);
                    bytes_read = read(pipes[i][0], temp_records[i].records[j] + bytes_read,sizeof(Record) - bytes_read);
                    printf("read %d bytes\n", bytes_read);
                    if(bytes_read <= 0){
                        break;
                    }
                    total_bytes += bytes_read;
                } while(total_bytes != sizeof(Record));
                printf("%ld %s %s  %s %d %s %s %-9.2f\n", \
            temp_records[i].records[j]->id,temp_records[i].records[j]->name ,temp_records[i].records[j]->surname , \
            temp_records[i].records[j]->home_address, temp_records[i].records[j]->home_number, temp_records[i].records[j]->city, temp_records[i].records[j]->mail_sector, \
            temp_records[i].records[j]->salary);
            }
        }
        j++;
    } while(bytes_read > 0 && j < record_count); */
    /* for(i=0;i<record_count;i++){
            printf("%ld %s %s  %s %d %s %s %-9.2f\n", \
            temp_records[0].records[i]->id,temp_records[0].records[i]->name ,temp_records[0].records[i]->surname , \
            temp_records[0].records[i]->home_address, temp_records[0].records[i]->home_number, temp_records[0].records[i]->city, temp_records[0].records[i]->mail_sector, \
            temp_records[0].records[i]->salary);
        } */
    // printf("Before merge\n");
    merge(temp_records, sorters, records, record_count, comparator[atoi(column) - 1]);

    for(i=0;i<record_count;i++){
        printf("%ld %s %s  %s %d %s %s %-9.2f\n", \
        records[i]->id,records[i]->name ,records[i]->surname , \
        records[i]->home_address, records[i]->home_number, records[i]->city, records[i]->mail_sector, \
        records[i]->salary);
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

void merge(sorter_records* temp_records, int sorters, Record** records, int record_count, int (*comparator)(Record*, Record*)){
    int i, j, k, s;
    int size;
    Record** temp = malloc(record_count*sizeof(Record*));
    for(i=0;i<record_count;i++){
        temp[i]=malloc(sizeof(Record));
    }
    if(sorters == 1){
        for(i = 0;i<record_count;i++){
            records[i] =  temp_records->records[i];
        }
        
    } else {
        printf("Else here temp_records[0].size = %d \n", temp_records[0].size);
        j = k =0;
        for(i=0;i<2*temp_records[0].size;i++){
            if(j < temp_records[0].size 
            && k < temp_records[1].size 
            && (*comparator)(temp_records[0].records[j],temp_records[1].records[k]) < 0){
                temp[i] = temp_records[0].records[j];
                /* printf("%ld %s %s  %s %d %s %s %-9.2f\n", \
        records[i]->id,records[i]->name ,records[i]->surname , \
        records[i]->home_address, records[i]->home_number, records[i]->city, records[i]->mail_sector, \
        records[i]->salary); */
                j++;
            } else if(k < temp_records[1].size){
                temp[i] = temp_records[1].records[k];
               /*  printf("%ld %s %s  %s %d %s %s %-9.2f\n", \
        records[i]->id,records[i]->name ,records[i]->surname , \
        records[i]->home_address, records[i]->home_number, records[i]->city, records[i]->mail_sector, \
        records[i]->salary); */
                k++;
            } else {
                while(j < temp_records[0].size){
                    temp[i] = temp_records[0].records[j];
                    j++;
                }
                while(k < temp_records[1].size){
                    temp[i] = temp_records[1].records[k];
                    k++;
                }
            }
        }

        

        if(sorters > 2){
            for(s=2;s<sorters;s++){
                j = k = 0;
               
                if(s == sorters - 1){
                    size = record_count;
                } else {
                    size = 2*temp_records[s].size;
                }
                 printf("size %d\n", size);
                for(i=0;i<size;i++){
                    if(j < size/2 && k < temp_records[s].size && (*comparator)(temp[j],temp_records[s].records[k]) < 0){
                        records[i] = temp[j];
                        j++;
                    } else if(k < temp_records[s].size) {
                        records[i] = temp_records[s].records[k];
                        k++;
                    } else {
                        while(j < size/2){
                            records[i] = temp[j];
                            j++;
                        }
                        while(k < temp_records[s].size){
                            records[i] = temp_records[s].records[k];
                            k++;
                        }
                    }
                }

                for(i=0;i<=size;i++){
                   temp[i] = records[i];
                }


            }
        } else {
            for(i=0;i<2*temp_records[0].size;i++){
                records[i] = temp[i];
            }
        }
    }
}




void initialize_temp_records(sorter_records* temp_records,int record_count, int id){
    int node_size;
    int i;
    switch (id){
    case 0:
        temp_records->records = malloc(record_count*sizeof(Record*));
        temp_records->start_off = 0;
        temp_records->fraction = 1.0;
        temp_records->size = record_count;
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
        temp_records[0].size = node_size;
        temp_records[1].size = node_size;
        
        printf("node size %d\n", node_size);
        temp_records[0].records = malloc(node_size*sizeof(Record*));
        temp_records[1].records = malloc(node_size*sizeof(Record*));
        for(i=0;i<node_size;i++){
            temp_records[0].records[i] = malloc(sizeof(Record));
            temp_records[1].records[i] = malloc(sizeof(Record));
        }
        break;

    case 2:
        node_size = (1.0/8.0)*record_count;
        printf("node size %d\n", node_size);
        temp_records[0].records = malloc(node_size*sizeof(Record*));
        temp_records[0].fraction = (1.0/8.0);
        temp_records[0].start_off = 0;
        temp_records[0].size = node_size;

        temp_records[1].records = malloc(node_size*sizeof(Record*));
        temp_records[1].fraction = (1.0/8.0);
        temp_records[1].start_off = (1.0/8.0)*record_count;
        temp_records[1].size = node_size;

        for(i=0;i<node_size;i++){
            temp_records[0].records[i] = malloc(sizeof(Record));
            temp_records[1].records[i] = malloc(sizeof(Record));
        }
        node_size = (1.0/4.0)*record_count;
        temp_records[2].records = malloc(node_size*sizeof(Record*));
        temp_records[2].fraction = (1.0/4.0);
        temp_records[2].start_off = temp_records[1].start_off + temp_records[1].fraction *record_count;
        temp_records[2].size = node_size;

         for(i=0;i<node_size;i++){
            temp_records[2].records[i] = malloc(sizeof(Record));
        }
        node_size = (2.0/4.0)*record_count;
       
        temp_records[3].fraction = (2.0/4.0);
        temp_records[3].start_off = temp_records[2].start_off + temp_records[2].fraction*record_count;
        temp_records[3].size = record_count - temp_records[3].start_off;
        temp_records[3].records = malloc(temp_records[3].size*sizeof(Record*));

         for(i=0;i<temp_records[3].size;i++){
            temp_records[3].records[i] = malloc(sizeof(Record));
        }
        break;
    case 3:
        node_size = (1.0/16.0)*record_count;
        temp_records[0].records = malloc(node_size*sizeof(Record*));
        temp_records[0].fraction = (1.0/16.0);
        temp_records[0].start_off = 0;
        temp_records[0].size = node_size;

        temp_records[1].records = malloc(node_size*sizeof(Record*));
        temp_records[1].fraction = (1.0/16.0);
        temp_records[1].start_off = (1.0/16.0)*record_count;
        temp_records[1].size = node_size;


        temp_records[2].records = malloc(node_size*sizeof(Record*));
        temp_records[2].fraction =  (1.0/16.0);
        temp_records[2].start_off = temp_records[1].start_off +  temp_records[1].fraction*record_count;
        temp_records[2].size = node_size;

        temp_records[3].records = malloc(node_size*sizeof(Record*));
        temp_records[3].fraction = (1.0/16.0);
        temp_records[3].start_off = temp_records[2].start_off + temp_records[2].fraction*record_count;
        temp_records[3].size = node_size;
        
        for(i=0;i<node_size;i++){
            temp_records[0].records[i] = malloc(sizeof(Record));
            temp_records[1].records[i] = malloc(sizeof(Record));
            temp_records[2].records[i] = malloc(sizeof(Record));
            temp_records[3].records[i] = malloc(sizeof(Record));
        }
        node_size = (2.0/16.0)*record_count;
        temp_records[4].records = malloc(node_size*sizeof(Record*));
        temp_records[4].fraction = (2.0/16.0);
        temp_records[4].start_off =  temp_records[3].start_off + temp_records[3].fraction*record_count;
        temp_records[4].size = node_size;

        temp_records[5].records = malloc(node_size*sizeof(Record*));
        temp_records[5].fraction = (2.0/16.0);
        temp_records[5].start_off = temp_records[4].start_off + temp_records[4].fraction*record_count;
        temp_records[5].size = node_size;
        
        for(i=0;i<node_size;i++){
            temp_records[4].records[i] = malloc(sizeof(Record));
            temp_records[5].records[i] = malloc(sizeof(Record));
        }
        node_size = (4.0/16.0)*record_count;
        temp_records[6].records = malloc(node_size*sizeof(Record*));
        temp_records[6].fraction = (4.0/16.0);
        temp_records[6].start_off = temp_records[5].start_off + temp_records[5].fraction*record_count;
        temp_records[6].size = node_size;
        
        temp_records[7].fraction = (4.0/16.0);
        temp_records[7].start_off = temp_records[6].start_off + temp_records[6].fraction*record_count;
        temp_records[7].size = record_count - temp_records[7].start_off;
        temp_records[7].records = malloc(temp_records[7].size*sizeof(Record*));

        for(i=0;i<node_size;i++){
            temp_records[6].records[i] = malloc(sizeof(Record));
        }
        for(i=0;i<temp_records[7].size;i++){
            temp_records[7].records[i] = malloc(sizeof(Record));
        }
        break;
    default:
        break;
    }

}
