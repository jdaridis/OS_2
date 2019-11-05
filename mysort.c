#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Record.h"

typedef struct Record Record;

int main(int argc, char const *argv[]){
    
    FILE* file;


    Record temp_rec;

    int record_count = 0;
    while(--argc){
        if(strcmp(*argv, "-f") == 0){
            file = fopen(*(argv + 1), "r");
        }
        argv++;
    }


    while(!feof(file)){
        
        fread(&temp_rec, sizeof(Record), 1, file);
        if(!feof(file)){
            record_count++;
            // printf("Id: %ld, name: %s\n", temp_rec.id, temp_rec.name);
        }
        
    }

    fseek(file, (record_count - 10) * sizeof(Record), SEEK_SET);

    printf("File size %d\n", record_count);

     while(!feof(file)){
        
        fread(&temp_rec, sizeof(Record), 1, file);
        if(!feof(file)){
            // record_count++;
            printf("Id: %ld, name: %s\n", temp_rec.id, temp_rec.name);
        }
        
    }


    return 0;
}
