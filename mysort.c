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
#include "coach_statistics.h"

typedef struct Record Record;

typedef struct coach{
    char type[3];
    char column[2];
    char id[2];
    int fd[2];
    coach_statistics stats;
} coach;

int main(int argc, char const *argv[]){
    
    int fd;
    char* filename;
    pid_t pid;
    int coach_count = 0;;
    coach coaches[4];
    char pipe_file_d[5];
    struct stat stat;
    int record_count;
    char* file_size;
    int i;
    int bytes_read;
    double t1, t2, real_time;
    double ticspersec;
    struct tms tb1, tb2;
    double max_time = -1.0;
    double min_time = 9999.0;
    double avg_time = 0.0;
    double max_time_cpu = -1.0;
    double min_time_cpu = 9999.0;
    double avg_time_cpu = 0.0;
    
    while(--argc){
        if(strcmp(*argv, "-f") == 0){
            fd = open(*(argv + 1), O_RDONLY);
            fstat(fd, &stat);
            filename = malloc(strlen(*(argv + 1)) + 1);
            strcpy(filename, *(argv + 1));
        } else if(strcmp(*argv, "-q") == 0 || strcmp(*argv, "-h") == 0){
            if(coach_count < 4){
                i =0;
                do{
                    if(strcmp(*(argv + 1), coaches[i].column) != 0){
                        strcpy(coaches[coach_count].type, *argv);
                        strcpy(coaches[coach_count].column, *(argv + 1));
                        sprintf(coaches[coach_count].id, "%d", coach_count);
                        coach_count++;
                        break;
                    }
                    i++;
                }while(i<coach_count);
                
            } else {
                printf("Too many sorters\n");
                exit(EXIT_FAILURE);
            }
            
        } 
        argv++;
    }

    // If the coach flag is missing and 1 coach for quicksort with the 1st column.
    if(coach_count == 0){
        strcpy(coaches[0].type, "-q");
        strcpy(coaches[0].column, "1");
        strcpy(coaches[0].id, "0");
        sprintf(coaches[0].id, "%d", coach_count);
        coach_count++;
        
    }

    // Start the clock.
    ticspersec = (double) sysconf(_SC_CLK_TCK);
    t1 = (double)times(&tb1);

    record_count = stat.st_size/sizeof(Record);

    // Allocate enough memory to hold the number of digits of the filesize.
    file_size = malloc((int)log10(record_count) + 2);

    sprintf(file_size, "%d", record_count);

    // For each coach construct the pipe to communicate.
    for(i = 0;i<coach_count;i++){
        pipe(coaches[i].fd);
    }

    // Spawn the new procceses and execute the coaches.
    for(i = 0;i<coach_count;i++){
        pid = fork();
        if(pid == 0){
            sprintf(pipe_file_d, "%d", coaches[i].fd[1]);
            execlp("./coach", "coach", "-f", filename,"-id", coaches[i].id, coaches[i].type, coaches[i].column, "-size", file_size,"-p", pipe_file_d, (char*)NULL);
            perror("Failed to exec");
        } else {
            close(coaches[i].fd[1]);
        }
    }

    // For each coach read from its pipe.
    for(i = 0;i<coach_count;i++){
        do{
            bytes_read = read(coaches[i].fd[0], &coaches[i].stats, sizeof(coach_statistics));
        } while(bytes_read != 0);
    }

   // Wait until all the children have finished, meaning wait returns -1;
    while(wait(NULL) > 0);

    printf("---------------\n");

    // Print the statistics for each coach and keep the general statistics.
    for(i = 0;i<coach_count;i++){
        printf("For coach %d\n", i);
        printf("Max sorter time: %lfs\n", coaches[i].stats.max_time);
        printf("Max sorter cpu time: %lfs\n", coaches[i].stats.max_time_cpu);
        printf("Min sorter time: %lfs\n",coaches[i].stats.min_time);
        printf("Min sorter cpu time: %lfs\n",coaches[i].stats.min_time_cpu);
        printf("Average sorter time: %lfs\n", coaches[i].stats.avg_time);
        printf("Average sorter cpu time: %lfs\n", coaches[i].stats.avg_time_cpu);
        printf("Signals received: %d\n\n",coaches[i].stats.signals);
        
        if(coaches[i].stats.exec_time > max_time){
            max_time = coaches[i].stats.exec_time;
        }

        if(coaches[i].stats.exec_time_cpu > max_time_cpu){
            max_time_cpu = coaches[i].stats.exec_time_cpu;
        }

        if(coaches[i].stats.exec_time < min_time){
            min_time = coaches[i].stats.exec_time;
        }

        if(coaches[i].stats.exec_time_cpu < min_time_cpu){
            min_time_cpu = coaches[i].stats.exec_time_cpu;
        }

        avg_time += coaches[i].stats.exec_time;

        avg_time_cpu += coaches[i].stats.exec_time_cpu;
    }

    avg_time = avg_time/coach_count;
    avg_time_cpu = avg_time_cpu/coach_count;
    
    // Print the general statistics.
    printf("---------------\n");
    printf("For the coaches\n");
    printf("Max time: %lfs\n", max_time);
    printf("Max time cpu: %lfs\n", max_time_cpu);
    printf("Min time: %lfs\n", min_time);
    printf("Min time cpu: %lfs\n", min_time_cpu);
    printf("Average time: %lfs\n", avg_time);
    printf("Average time cpu: %lfs\n\n", avg_time_cpu);

     // Stop the clock and find the turnaround time.
    t2 = (double)times(&tb2);

    real_time = (t2 - t1)/ticspersec;

    printf("Turnaround time for the whole program: %lfs\n", real_time);

    free(filename);
    free(file_size);

    return 0;
}
