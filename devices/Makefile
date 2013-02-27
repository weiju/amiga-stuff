CC=vc +aos68k
CC13=vc +kick13

# Note: VBCC comes with linker library replacements, that should be used.
# If not, printf() stops working in the expected way, because it is part
# of amiga.lib
CFLAGS=-c99 -I$(NDK_INC)

all: timer

clean:
	rm -f timer

timer: timer.c
	$(CC13) $(CFLAGS) timer.c -o timer -lamiga
