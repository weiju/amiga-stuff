CC=vc +kick13
CFLAGS=-c99 -I$(NDK_INC) -DDEBUG -O2

all: startup sprites playfield1

clean:
	rm -f *.o startup sprites playfield1

startup: startup.o common.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@

sprites: sprites.o common.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@

playfield1: playfield1.o common.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@
