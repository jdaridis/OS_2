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
    while(--argc){
        if(strcmp(*argv, "-f") == 0){
            file = fopen(*(argv + 1), "r");
            column = atoi(*(argv + 2)) - 1;
        }
        argv++;
    }


    while(!feof(file)){
        
        fread(&temp_rec, sizeof(Record), 1, file);
        if(!feof(file)){
            record_count++;
        }
        
    }

    fseek(file, 0, SEEK_SET);

    printf("File size %d\n", record_count);

    records = malloc(record_count*sizeof(Record*));
    int i = 0;
    for(i=0;i<record_count;i++){
        records[i] = malloc(sizeof(Record));
    }


    i = 0;
     while(!feof(file)){
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

void heap_sort(Record** array, int size,int (*comparator)(Record*, Record*)){
    heapify(array, size, comparator);

    for(int i = size;i>0;i--){
        swap(&array[0], &array[i]);
        size--;
        heap_fixup(array, size, 0 ,comparator);
    }
}

void heapify(Record** array, int size,int (*comparator)(Record*, Record*)){

    for(int i = size/2; i>=0;i--){
        heap_fixup(array, size, i, comparator);
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


