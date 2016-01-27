CC=vc +kick13
CFLAGS=-c99 -I$(NDK_INC) -DDEBUG -O2

all: startup

clean:
	rm -f *.o startup

startup: startup.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@
