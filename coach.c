#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <signal.h>
#include <fcntl.h>
#include "Record.h"
#include "comparators.h"
#include "coach.h"
#include "coach_statistics.h"

int signal_counter = 0;

int main(int argc, char const *argv[]){

    FILE* file, *out_file;
    int id = 0;
    char column[] = "1";
    char sort = 'q';
    int sorters;
    int** pipes;
    int my_pipe;
    int record_count = -1;
    int node_size;
    int fd = -1;
    int error;
    coach_statistics stats;
    stats.max_time = 9999.0;
    stats.min_time = -1.0;
    stats.avg_time = 0.0;

    double t1, t2, real_time;
    double ticspersec;
    struct tms tb1, tb2;
    char pipe_file_d[5];
    char sorter_id[2];
    char* filename;
    char* new_filename;
    char* start_off;
    char end_off[15];
    int bytes_read;
    pid_t pid;

    sorter_records* temp_records;
    Record** records;

    
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
        } else if(strcmp(*argv, "-h") == 0){
            sort = 'h';
            strcpy(column, *(argv + 1));
        } else if(strcmp(*argv, "-q") == 0){
            sort = 'q';
            strcpy(column, *(argv + 1));
        } else if(strcmp(*argv, "-size") == 0){
            record_count = atoi(*(argv + 1));
        } else if(strcmp(*argv, "-id") == 0){
            id = atoi(*(argv + 1));
        } else if(strcmp(*argv, "-p") == 0){
            my_pipe = atoi(*(argv + 1));
        }
        argv++;
    }

    sorters = 1 << id; 

    pipes = malloc(sorters * sizeof(int*));

    new_filename = malloc(strlen(filename) + 3);
    strcpy(new_filename, filename);
    strcat(new_filename, ".");
    strcat(new_filename, column);
    out_file = fopen(new_filename, "w");
    
    ticspersec = (double) sysconf(_SC_CLK_TCK);
    t1 = (double) times(&tb1);

    if(fd != -1 && record_count == -1){
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
            if(sort == 'h'){
                execlp("./heapsort", "heapsort", "-f", filename, "-id", sorter_id, "-p", pipe_file_d,"-c", column, "-s", start_off, end_off, (char  *) NULL);
            } else {
                execlp("./quicksort", "quicksort", "-f", filename,"-id", sorter_id, "-p", pipe_file_d, "-c", column, "-s", start_off, end_off, (char  *) NULL);
            }
        } else {
            close(pipes[i][1]);
        }
    }
    for(i = 0;i<sorters;i++){
        int j = 0;
        bytes_read = read(pipes[i][0], &temp_records[i].exec_time,sizeof(double));
      do{
          bytes_read = read(pipes[i][0], temp_records[i].records[j],sizeof(Record));
          j++;
      }while(bytes_read > 0 && j < temp_records[i].size);
    }

 
    for(i=0;i<sorters;i++){
        wait(NULL);
    }

    merge(temp_records, sorters, records, record_count, comparator[atoi(column) - 1]);


    for(i=0;i<sorters;i++){
        if(temp_records[i].exec_time < stats.min_time){
            stats.min_time = temp_records[i].exec_time;
        }

        if(temp_records[i].exec_time > stats.max_time){
            stats.max_time = temp_records[i].exec_time;
        }

        stats.avg_time += temp_records[i].exec_time;
    }

    stats.avg_time = stats.avg_time/sorters;


    for(i=0;i<record_count;i++){
        fprintf(out_file, "%ld %s %s  %s %d %s %s %-9.2f\n", \
        records[i]->id,records[i]->name ,records[i]->surname , \
        records[i]->home_address, records[i]->home_number, records[i]->city, records[i]->mail_sector, \
        records[i]->salary);
    }
    fclose(out_file);
    for(i=0;i<sorters;i++){
       free(temp_records[i].records);
       free(pipes[i]);
    }
    free(temp_records);
    free(pipes);

    for(i=0;i<record_count;i++){
        free(records[i]);
    }

    free(records);
    free(start_off);
    free(filename);
    free(new_filename);

    stats.signals = signal_counter;
    t2 = (double) times(&tb2);
    stats.exec_time = (t2-t1)/ticspersec;
    
    return 0;
}


void sig_handler(int sig){
    signal(SIGUSR2, sig_handler);
    signal_counter++;
}

void merge(sorter_records* temp_records, int sorters, Record** records, int record_count, int (*comparator)(Record*, Record*)){
    int i, j, k, s;
    int j_size;
    int size;
    Record** temp = malloc(record_count*sizeof(Record*));
   
    if(sorters == 1){
        for(i = 0;i<record_count;i++){
            records[i] =  temp_records->records[i];
        }
        
    } else {
        j = k =0;
        for(i=0;i<2*temp_records[0].size;i++){
            if(j < temp_records[0].size  && k < temp_records[1].size){
                if((*comparator)(temp_records[0].records[j],temp_records[1].records[k]) < 0){
                    temp[i] = temp_records[0].records[j];
                    j++;
                } else {
                    temp[i] = temp_records[1].records[k];
                    k++;
                }
            } else {
                if(j < temp_records[0].size){
                    temp[i] = temp_records[0].records[j];
                    j++;
                }
                if(k < temp_records[1].size){
                    temp[i] = temp_records[1].records[k];
                    k++;
                }
            }
        }

        j_size = i;

        if(sorters > 2){
            for(s=2;s<sorters;s++){
                j = k = 0;
                if(s == sorters - 1){
                    size = record_count;
                }else {
                    size = j_size + temp_records[s].size;
                }
                for(i=0;i<size;i++){
                    if(j < j_size && k < temp_records[s].size){
                        if((*comparator)(temp[j],temp_records[s].records[k]) < 0){
                            records[i] = temp[j];
                            j++;
                        } else {
                            records[i] = temp_records[s].records[k];
                            k++;
                        }
                    } else {
                        if(j < j_size){
                            records[i] = temp[j];
                            j++;
                        }
                        if(k < temp_records[s].size){
                            records[i] = temp_records[s].records[k];
                            k++;
                        }
                    }
                }

                j_size = j + k;

                for(i=0;i<size;i++){
                   temp[i] = records[i];
                }

            }
        } else {
            for(i=0;i<2*temp_records[0].size;i++){
                records[i] = temp[i];
            }
        }
    }

    free(temp);
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
        
        temp_records[0].records = malloc(node_size*sizeof(Record*));
        temp_records[1].records = malloc(node_size*sizeof(Record*));
        for(i=0;i<node_size;i++){
            temp_records[0].records[i] = malloc(sizeof(Record));
            temp_records[1].records[i] = malloc(sizeof(Record));
        }
        break;

    case 2:
        node_size = (1.0/8.0)*record_count;
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
