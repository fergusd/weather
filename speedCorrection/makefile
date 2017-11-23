CC=g++
CFLAGS=-I.

unitTest: interpolator.c interpolator.h unitTest.c
	$(CC) -o unitTest unitTest.c interpolator.c -I .

clean:
	rm -f *.o unitTest
