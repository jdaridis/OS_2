#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quicksort.h"
#include "Record.h"


int main(int argc, char const *argv[]){

    FILE* file;

    Record temp_rec;
    Record** records;

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
            // record_count++;
            fread(records[i], sizeof(Record), 1, file);
            i++;
            // printf("Id: %ld, name: %s\n", temp_rec.id, temp_rec.name);
        }
        
    }

    quick_sort(records, 0, record_count - 1, name_cmp);

    printf("----------------------------\n");

    for(i=0;i<record_count;i++){
        printf("Id: %ld, name: %s\n", records[i]->id, records[i]->name);
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


void swap(Record** r1, Record** r2){
    Record* temp = *r1;
    *r1 = *r2;
    *r2 = temp;
}


int int_cmp(void* i1, void* i2){
    return (*(int*)i1 - *(int*) i2);
}

int long_cmp(void* l1, void* l2){
    return (*(long*)l1 - *(long*) l2);
}

int float_cmp(void* f1, void* f2){
    return (*(float*)f1- *(float*) f2);
}

int str_cmp(const void* str1, const void* str2){
	return strcmp(*(char**)str1, *(char**)str2);
}

int id_cmp(Record* r1, Record* r2){
    return ( r1->id - r2->id );
}

int name_cmp(Record* r1, Record* r2){
    return strcmp(r1->name, r2->name);
}

int surname_cmp(Record* r1, Record* r2){
    return strcmp(r1->surname, r2->surname);
}

int home_addr_cmp(Record* r1, Record* r2){
    return strcmp(r1->home_address, r2->home_address);
}

int home_num_cmp(Record* r1, Record* r2){
    return ( r1->home_number - r2->home_number );
}

int city_cmp(Record* r1, Record* r2){
    return strcmp(r1->city, r2->city);
}

int mail_cmp(Record* r1, Record* r2){
    return strcmp(r1->mail_sector, r2->mail_sector);
}


int salary_cmp(Record* r1, Record* r2){
    return ( r1->salary - r2->salary );
}









