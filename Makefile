CC= g++
CFLAGS= -Wall -std=c++11

all: uthreads.o
	ar rcs libuthreads.a uthreads.o

uthreads.o: uthreads.cpp uthreads.h
	$(CC) $(CFLAGS) -c uthreads.cpp

tar:
	tar cvf ex2.tar uthreads.cpp Makefile README

clean:
	rm -f *.o libuthreads.a ex2.tar


