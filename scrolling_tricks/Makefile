ASM=vasmm68k_mot
ASM_FLAGS = -Fhunk -devpac
.SUFFIXES = .o .asm .c

CC=vc +aos68k
CFLAGS=-c99 -I$(NDK_INC) -DDEBUG

all: xunlimited xlimited

clean:
	rm -f xunlimited xlimited hardware.o cop.o main.o

cop.o: cop.asm
	$(ASM) $(ASM_FLAGS) -o $@ $<

.c.o:
	$(CC) $(CFLAGS)  -c -o $@ $^

xunlimited: hardware.o cop.o common.o xunlimited.o
	$(CC) -o $@ $^ -lamiga -lauto


xlimited: hardware.o cop.o common.o xlimited.o
	$(CC) -o $@ $^ -lamiga -lauto
