CC	= vc +aos68k
CFLAGS   = -O2 -+ -I$(NDK_INC) -DVBCC -c
LDFLAGS  = -nostdlib -lamiga
LDLIBS	=

OBJS = startup.o libinit.o libfuncs.o

all: example.library

clean:
	rm -f *.o example.library

example.library: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

startup.o: startup.c
	$(CC) $(CFLAGS) $^

libinit.o: libinit.c
	$(CC) $(CFLAGS) $^

libfuncs.o: libfuncs.c
	$(CC) $(CFLAGS) $^
