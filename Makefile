all: mysort quicksort heapsort

mysort: mysort.o
	gcc -o mysort mysort.o

mysort.o: mysort.c
	gcc -g -c mysort.c

quicksort: quicksort.o comparators.o
	gcc -o quicksort quicksort.o comparators.o

quicksort.o: quicksort.c quicksort.h
	gcc -g -c quicksort.c

heapsort: heapsort.o comparators.o
	gcc -o heapsort heapsort.o comparators.o

heapsort.o: heapsort.c
	gcc -g -c heapsort.c

comparators.o: comparators.c comparators.h
	gcc -g -c comparators.c


clean: 
	rm -f ./mysort ./quicksort ./heapsort mysort.o quicksort.o comparators.o heapsort.o