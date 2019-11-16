all: mysort coach quicksort heapsort

mysort: mysort.c
	gcc -g -o mysort mysort.c -lm

coach: coach.c coach.h comparators.o
	gcc -g -o coach coach.c comparators.o -lm


quicksort: sorter.c quicksort.o comparators.o
	gcc -g -o quicksort sorter.c quicksort.o comparators.o

quicksort.o: quicksort.c quicksort.h
	gcc -g -c quicksort.c

heapsort: sorter.c heapsort.o comparators.o
	gcc -D__HEAP_SORT__ -g -o heapsort sorter.c heapsort.o comparators.o

heapsort.o: heapsort.c
	gcc -g -c heapsort.c

comparators.o: comparators.c comparators.h
	gcc -g -c comparators.c


clean: 
	rm -f ./mysort ./coach ./quicksort ./heapsort quicksort.o comparators.o heapsort.o