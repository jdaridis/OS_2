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
    
    FILE* file;
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
                strcpy(coaches[coach_count].type, *argv);
                strcpy(coaches[coach_count].column, *(argv + 1));
                sprintf(coaches[coach_count].id, "%d", coach_count);
                coach_count++;
            }
            
        } 
        argv++;
    }

    record_count = stat.st_size/sizeof(Record);

    file_size = malloc((int)log10(record_count) + 2);

    sprintf(file_size, "%d", record_count);

    printf("Coach count %d, record count %d, file_size %d\n", coach_count, record_count, (int)log10(record_count) + 1);

    for(i = 0;i<coach_count;i++){
        printf("type %s, column %s, id %s\n", coaches[i].type, coaches[i].column, coaches[i].id);
        pipe(coaches[i].fd);
    }



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

   for(i = 0;i<coach_count;i++){
      do{
        bytes_read = read(coaches[i].fd[0], &coaches[i].stats, sizeof(coach_statistics));
      } while(bytes_read != 0);
   }

    for(i = 0;i<coach_count;i++){
        wait(NULL);
    }
    printf("---------------\n");
   for(i = 0;i<coach_count;i++){
        printf("For coach: %d\n", i);
        printf("Max sorter time %lf\n", coaches[i].stats.max_time);
        printf("Max sorter cpu time %lf\n", coaches[i].stats.max_time_cpu);
        printf("Min sorter time: %lf\n",coaches[i].stats.min_time);
        printf("Min sorter cpu time: %lf\n",coaches[i].stats.min_time_cpu);
        printf("Average sorter time: %lf\n", coaches[i].stats.avg_time);
        printf("Average sorter cpu time: %lf\n", coaches[i].stats.avg_time_cpu);
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

    printf("For the coaches\n");
    printf("Max time: %lf\nMin time: %lf\nAverage time: %lf\n", max_time, min_time, avg_time);

    free(filename);
    free(file_size);

    return 0;
}
