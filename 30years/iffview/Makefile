#CC=vc +aos68k
CC=vc +kick13
CFLAGS=-c99 -I$(NDK_INC) -DDEBUG

all: iffview

clean:
	rm -f *.o iffview ilbm

iffview: iffview.o ilbm.o filereq.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@

# Mostly for testing
# interestingly, when defining std=c99, LITTLE_ENDIAN disappears as a definition
# in gcc, so we define it explicitly
ilbm: ilbm.c
	gcc -o $@ $< -DSTANDALONE -DDEBUG -std=c99 -DLITTLE_ENDIAN
