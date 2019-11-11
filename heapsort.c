#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comparators.h"
#include "heapsort.h"
#include "Record.h"

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
    heap_sort(records, record_count -1, comparator[column]);


    printf("----------------------------\n");

    for(i=0;i<record_count;i++){
        printf("%ld %s %s  %s %d %s %s %-9.2f\n", \
		records[i]->id,records[i]->name ,records[i]->surname , \
		records[i]->home_address, records[i]->home_number, records[i]->city, records[i]->mail_sector, \
		records[i]->salary);
    }

    return 0;

}

void heap_sort(Record** array, int end,int (*comparator)(Record*, Record*)){
    heapify(array, end, comparator);

    for(int i = end;i>0;i--){
        swap(&array[0], &array[i]);
        end--;
        heap_fixup(array, end, 0 ,comparator);
    }
}

void heapify(Record** array, int end,int (*comparator)(Record*, Record*)){

    for(int i = end/2; i>=0;i--){
        heap_fixup(array, end, i, comparator);
    }
}


void heap_fixup(Record** array,int size, int i, int (*comparator)(Record*, Record*)){
    int left = left_child(i);
    int right = right_child(i);
    int max;
    if(left <= size &&(*comparator)(array[left], array[i]) > 0){
        max = left;
    } else max = i;

    if(right <= size &&(*comparator)(array[right], array[max]) > 0){
        max = right;
    }

    if(max != i){
        swap(&array[i], &array[max]);
        heap_fixup(array, size, max, comparator);
    }
}

int parent(int i){
    return (i-1)/2;
}

int left_child(int i){
   return 2*i + 1;
}

int right_child(int i){
    return 2*i + 2;
 
}


