all: mysort coach quicksort heapsort

mysort: mysort.c
	gcc -o mysort mysort.c

coach: coach.c
	gcc -o coach coach.c


quicksort: sorter.c quicksort.o comparators.o
	gcc -o quicksort sorter.c quicksort.o comparators.o

quicksort.o: quicksort.c quicksort.h
	gcc -g -c quicksort.c

heapsort: sorter.c heapsort.o comparators.o
	gcc -D__HEAP_SORT__ -o heapsort sorter.c heapsort.o comparators.o

heapsort.o: heapsort.c
	gcc -g -c heapsort.c

comparators.o: comparators.c comparators.h
	gcc -g -c comparators.c


clean: 
	rm -f ./mysort ./coach ./quicksort ./heapsort quicksort.o comparators.o heapsort.o