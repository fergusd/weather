CC=g++
CFLAGS=-I.

unitTest: interpolator.c interpolator.h unitTest.c
	$(CC) -o unitTest unitTest.c interpolator.c $(CFLAGS)

clean:
	rm -f *.o unitTest
