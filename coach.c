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
#include <math.h>
#include "Record.h"
#include "comparators.h"
#include "coach.h"
#include "coach_statistics.h"

int signal_counter = 0;

int main(int argc, char const *argv[]){

    FILE* out_file;
    int id = 0;
    char column[] = "1";
    char sort = 'q';
    int sorters;
    int** pipes;
    int my_pipe = -1;
    int record_count = -1;
    int fd = -1;
    int error;
    coach_statistics stats;
    stats.max_time = -1.0;
    stats.min_time = 9999.0;
    stats.avg_time = 0.0;
    stats.max_time_cpu = -1.0;
    stats.min_time_cpu = 9999.0;
    stats.avg_time_cpu = 0.0;

    double t1, t2, real_time, cpu_time;
    double ticspersec;
    struct tms tb1, tb2;
    char pipe_file_d[5];
    char* filename;
    char* new_filename;
    char* start_off;
    char* end_off;
    int bytes_read;
    pid_t pid;

    sorter_records* temp_records;
    Record** records;
    struct stat stat;

    while(--argc){
        if(strcmp(*argv, "-f") == 0){
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

    //Shifting left by id means 2^id number of sorters.
    sorters = 1 << id; 

    pipes = malloc(sorters * sizeof(int*));

    // Concatinating the old filename with the column number.
    new_filename = malloc(strlen(filename) + 3);
    strcpy(new_filename, filename);
    strtok(new_filename, ".");
    strcat(new_filename, ".");
    strcat(new_filename, column);
    printf("%s\n", new_filename);
    out_file = fopen(new_filename, "w");
    
    // Start the clock.
    ticspersec = (double) sysconf(_SC_CLK_TCK);
    t1 = (double) times(&tb1);

    // In case this program is run separately find the file size.
    if(fd != -1 && record_count == -1){
        record_count = stat.st_size/sizeof(Record);
    } else if(fd == -1 && record_count == -1){
        fd = open(filename, O_RDONLY);
        fstat(fd, &stat);
        record_count = stat.st_size/sizeof(Record);
    }

    records = malloc(record_count*sizeof(Record));

    // Allocate enough memory to hold the number of digits in start and end offset.
    start_off = malloc((int)log10(record_count) + 2);
    end_off = malloc((int)log10(record_count) + 2);

    int i = 0;
   
    temp_records = malloc(sorters*sizeof(sorter_records));
    
    initialize_temp_records(temp_records, record_count, id);
    signal(SIGUSR2, sig_handler); // Register handler for SIGUSR2 signal.

    // For each sorter construct a pipe to communicate.
    for(i = 0;i<sorters;i++){
       pipes[i] = malloc(2*sizeof(int));
       pipe(pipes[i]);
    }
    
    // Spawn the new procceses and execute the sorters.
    for(i = 0;i<sorters;i++){
        pid = fork();
        if(pid == 0){
            close(pipes[i][0]);
            sprintf(start_off, "%d", temp_records[i].start_off);
            sprintf(end_off, "%d", temp_records[i].start_off + temp_records[i].size);
            sprintf(pipe_file_d, "%d", pipes[i][1]);
            if(sort == 'h'){
                execlp("./heapsort", "heapsort", "-f", filename, "-p", pipe_file_d,"-c", column, "-s", start_off, end_off, (char  *) NULL);
            } else {
                execlp("./quicksort", "quicksort", "-f", filename, "-p", pipe_file_d, "-c", column, "-s", start_off, end_off, (char  *) NULL);
            }
        } else {
            close(pipes[i][1]);
        }
    }

    // For each sorter read its pipe.
    for(i = 0;i<sorters;i++){
        int j = 0;
        // First read the stats and then the sorted Records.
        bytes_read = read(pipes[i][0], &temp_records[i].exec_time,sizeof(double));
        bytes_read = read(pipes[i][0], &temp_records[i].exec_time_cpu, sizeof(double));
      do{
          bytes_read = read(pipes[i][0], temp_records[i].records[j],sizeof(Record));
          j++;
      }while(bytes_read > 0 && j < temp_records[i].size);
    }

    // Wait until all the children have finished, meaning wait returns -1;
    while(wait(NULL) > 0);

    merge(temp_records, sorters, records, record_count, comparator[atoi(column) - 1]);

    // Keep the statistics.
    for(i=0;i<sorters;i++){
        if(temp_records[i].exec_time < stats.min_time){
            stats.min_time = temp_records[i].exec_time;
        }

        if(temp_records[i].exec_time_cpu < stats.min_time_cpu){
            stats.min_time_cpu = temp_records[i].exec_time_cpu;
        }

        if(temp_records[i].exec_time > stats.max_time){
            stats.max_time = temp_records[i].exec_time;
        }

        if(temp_records[i].exec_time_cpu > stats.max_time_cpu){
            stats.max_time_cpu = temp_records[i].exec_time_cpu;
        }

        stats.avg_time += temp_records[i].exec_time;
        stats.avg_time_cpu += temp_records[i].exec_time_cpu;
    }

    stats.avg_time = stats.avg_time/sorters;
    stats.avg_time_cpu = stats.avg_time_cpu/sorters;

    // Print the sorted Records to the file.
    for(i=0;i<record_count;i++){
        fprintf(out_file, "%ld %s %s  %s %d %s %s %-9.2f\n", \
        records[i]->id,records[i]->name ,records[i]->surname , \
        records[i]->home_address, records[i]->home_number, records[i]->city, records[i]->mail_sector, \
        records[i]->salary);
    }
    fclose(out_file);

    // Free all the memory allocated.
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
    free(end_off);
    free(filename);
    free(new_filename);

    stats.signals = signal_counter;

    // Stop the clock and find the real time and cpu time elapsed.
    t2 = (double) times(&tb2);
    stats.exec_time = (t2-t1)/ticspersec;
    stats.exec_time_cpu = (double) ((tb2.tms_utime + tb2.tms_stime) -
    (tb1.tms_utime + tb1.tms_stime)) / ticspersec;

    // Pass the stats back to the parent.
    if(my_pipe != -1){
        write(my_pipe, &stats, sizeof(coach_statistics));
    }
    
    return 0;
}

// Handler for the SIGUSR2 signal.
void sig_handler(int sig){
    signal(SIGUSR2, sig_handler);
    signal_counter++;
}

// Function to merge all the sorted sub-arrays back to 1 array.
void merge(sorter_records* temp_records, int sorters, Record** records, int record_count, int (*comparator)(Record*, Record*)){
    int i, j, k, s;
    int j_size;
    int size;
    Record** temp = malloc(record_count*sizeof(Record*));
    
    // If the sorter is 1 trivial case.
    if(sorters == 1){
        for(i = 0;i<record_count;i++){
            records[i] =  temp_records->records[i];
        }
    } else {
        j = k =0;
        // Merge the 2 first sub-arrays.
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

        // If there are more than 2 sub-arrays merge them iteratively.
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

// Initialise the structure to hold the Records coming from the sorters.
// Holding the starting offset, the fraction and the size of the array.
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
