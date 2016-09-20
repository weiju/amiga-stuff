CC=vc +kick13
CFLAGS=-c99 -I$(NDK_INC) -DDEBUG -O2

all: startup sprites

clean:
	rm -f *.o startup sprites

startup: startup.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@

sprites: sprites.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@
