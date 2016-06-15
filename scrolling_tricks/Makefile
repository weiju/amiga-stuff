ASM=vasmm68k_mot
ASM_FLAGS = -Fhunk -devpac
.SUFFIXES = .o .asm .c

CC=vc +aos68k
CFLAGS=-c99 -I$(NDK_INC) -DDEBUG

all: xunlimited xlimited

clean:
	rm -f xunlimited xlimited *.o

cop_x.o: cop_x.asm
	$(ASM) $(ASM_FLAGS) -o $@ $<

.c.o:
	$(CC) $(CFLAGS)  -c -o $@ $^

xunlimited: hardware.o cop_x.o common.o xunlimited.o
	$(CC) -o $@ $^ -lamiga -lauto


xlimited: hardware.o cop_x.o common.o xlimited.o
	$(CC) -o $@ $^ -lamiga -lauto
