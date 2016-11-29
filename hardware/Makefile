CC=vc +kick13
CFLAGS=-c99 -I$(NDK_INC) -DDEBUG -O2

all: startup sprites playfield1 scoopex1 soundfx

clean:
	rm -f *.o startup sprites playfield1 scoopex2 soundfx

startup: startup.o common.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@

sprites: sprites.o common.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@

playfield1: playfield1.o common.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@

scoopex1: scoopex1.c
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@

soundfx: soundfx.o common.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@

