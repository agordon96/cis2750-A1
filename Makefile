all: list parser program clean

LinkedListAPI.o: src/LinkedListAPI.c
	gcc -Iinclude -c src/LinkedListAPI.c -o LinkedListAPI.o -Wall -std=c11

CalendarParser.o: src/CalendarParser.c
	gcc -Iinclude -c src/CalendarParser.c -o CalendarParser.o -Wall -std=c11

list: LinkedListAPI.o
	ar cr bin/libllist.a LinkedListAPI.o

parser: CalendarParser.o LinkedListAPI.o
	ar cr bin/libcparse.a CalendarParser.o LinkedListAPI.o

program:
	gcc -Iinclude -o Demo Demo.c bin/libllist.a bin/libcparse.a -Wall -std=c11

clean:
	rm -f *.o *.so
