#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heapsort.h"

void swap(Record** r1, Record** r2);

// Generic heapsort of Record array.
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


