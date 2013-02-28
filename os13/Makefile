#CC=vc +aos68k
CC=vc +kick13
CFLAGS=-c99 -I$(NDK_INC)

all: intuition1 graphics1 copper1 dualplayfield1 ham1 ostest openwin

clean:
	rm -f intuition1 graphics1 copper1 dualplayfield1 ham1 ostest openwin

ostest: ostest.c
	$(CC) ostest.c -o ostest

intuition1: intuition1.c
	$(CC) $(CFLAGS) intuition1.c -lamiga -o intuition1

openwin: openwin.c
	$(CC) $(CFLAGS) openwin.c -lamiga -o openwin

graphics1: graphics1.c
	$(CC) $(CFLAGS) graphics1.c -lamiga -o graphics1

copper1: copper1.c
	$(CC) $(CFLAGS) copper1.c -lamiga -o copper1

dualplayfield1: dualplayfield1.c
	$(CC) $(CFLAGS) dualplayfield1.c -lamiga -o dualplayfield1

ham1: ham1.c
	$(CC) $(CFLAGS) ham1.c -lamiga -o ham1
