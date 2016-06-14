ASM=vasmm68k_mot
ASM_FLAGS = -Fhunk -devpac
.SUFFIXES = .o .asm .c

CC=vc +aos68k
CFLAGS=-c99 -I$(NDK_INC) -DDEBUG

all: main

clean:
	rm -f main hardware.o cop.o main.o

cop.o: cop.asm
	$(ASM) $(ASM_FLAGS) -o $@ $<

.c.o:
	$(CC) $(CFLAGS)  -c -o $@ $^

main: hardware.o cop.o main.o
	$(CC) -o $@ $^ -lamiga -lauto
