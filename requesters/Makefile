#CC=vc +aos68k
CC=vc +kick13
CFLAGS=-c99 -I$(NDK_INC) -DDEBUG

all: main

clean:
	rm -f *.o main

main: main.o filereq.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@

