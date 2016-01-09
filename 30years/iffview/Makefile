#CC=vc +aos68k
CC=vc +kick13
CFLAGS=-c99 -I$(NDK_INC)

all: iffview

clean:
	rm -f *.o iffview ilbm

iffview: iffview.o ilbm.o filereq.o
	$(CC) $(CFLAGS) $? -lamiga -lauto -o $@

# Mostly for testing
ilbm: ilbm.c
	gcc -o $@ $< -DSTANDALONE -DDEBUG
