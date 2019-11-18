#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quicksort.h"

void swap(Record** r1, Record** r2);

// Generic quicksort of Record array.
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
