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
#include "coach_statistics.h"

typedef struct Record Record;

typedef struct coach{
    char type;
    char column[1];
} coach;

int main(int argc, char const *argv[]){
    
    FILE* file;
    int fd;
    char* filename;
    int coach_count = 0;;
    coach coaches[4];
    struct stat stat;
    int record_count;

    Record temp_rec;

    int record_count = 0;
    while(--argc){
        if(strcmp(*argv, "-f") == 0){
            fd = open(*(argv + 1), O_RDONLY);
            fstat(fd, &stat);
            filename = malloc(strlen(*(argv + 1)) + 1);
            strcpy(filename, *(argv + 1));
        } else if(strcmp(*argv, "-q") == 0){
            if(coach_count < 4){
                coaches[coach_count].type = 'q';
                strcpy(coaches[coach_count].column, *(argv + 1));
                coach_count++;
            }
            
        }  else if(strcmp(*argv, "-h") == 0){
            if(coach_count < 4){
                coaches[coach_count].type = 'h';
                strcpy(coaches[coach_count].column, *(argv + 1));
                coach_count++;
            }
        }
        argv++;
    }

    record_count = stat.st_size/sizeof(Record);

   



    return 0;
}
