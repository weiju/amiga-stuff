CC=vc +kick13

# Note: VBCC comes with linker library replacements, that should be used.
# If not, printf() stops working in the expected way, because it is part
# of amiga.lib
CFLAGS=-c99 -I$(NDK_INC)

all: timer audio

clean:
	rm -f timer audio

timer: timer.c
	$(CC) $(CFLAGS) $^ -o $@ -lamiga

audio: audio.c
	$(CC) $(CFLAGS) $^ -o $@ -lamiga -lauto
