#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quicksort.h"
#include "Record.h"
#include "comparators.h"



int main(int argc, char const *argv[]){

     FILE* file;

    Record temp_rec;
    Record** records;

    int record_count = 0;
    int column = 0;
    int start;
    float frac;
    int size;
    while(--argc){
        if(strcmp(*argv, "-f") == 0){
            file = fopen(*(argv + 1), "r");
            
        } else if(strcmp(*argv, "-c") == 0){
            column = atoi(*(argv + 1)) - 1;
        } else if(strcmp(*argv, "-s") == 0){
            start = atoi(*(argv + 1));
            frac = atof(*(argv + 2));
        } else if(strcmp(*argv, "-size") == 0){
            size = atoi(*(argv + 1));
        }
        argv++;
    }
    
    record_count = size*frac;
   /*  while(!feof(file) && (float)record_count < size*frac){
        
        fread(&temp_rec, sizeof(Record), 1, file);
        if(!feof(file)){
            record_count++;
        }
        
    } */

    fseek(file, start*sizeof(Record), SEEK_SET);

    printf("File size %d\n", record_count);

    records = malloc(record_count*sizeof(Record*));
    int i = 0;
    for(i=0;i<record_count;i++){
        records[i] = malloc(sizeof(Record));
    }


    i = 0;
     while(!feof(file) && (float)i < record_count){
        if(!feof(file)){
            fread(records[i], sizeof(Record), 1, file);
            i++;
        }
        
    }
    quick_sort(records, 0, record_count - 1, comparator[column]);


    printf("----------------------------\n");

    for(i=0;i<record_count;i++){
        printf("%ld %s %s  %s %d %s %s %-9.2f\n", \
		records[i]->id,records[i]->name ,records[i]->surname , \
		records[i]->home_address, records[i]->home_number, records[i]->city, records[i]->mail_sector, \
		records[i]->salary);
    }

    return 0;

}



void quick_sort(Record **array, int low, int high, int (*comparator)(Record* s1, Record* s2)){
    int p;
    if(low < high){
        p = partition(array, low, high, comparator);
        quick_sort(array, low, p-1, comparator);
        quick_sort(array, p+1, high, comparator);
    }
}

int partition(Record **array, int low, int high, int (*comparator)(Record* s1, Record* s2)){
    Record* pivot = array[high];

    int i = low;

    for(int j = low;j<high;j++){
        if((*comparator)(array[j], pivot) < 0){
            swap(&array[i], &array[j]);
            i++;
        }
        
    }
    swap(&array[i], &array[high]);
    return i;
    
}











