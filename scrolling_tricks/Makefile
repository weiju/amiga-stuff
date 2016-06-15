ASM=vasmm68k_mot
ASM_FLAGS = -Fhunk -devpac
.SUFFIXES = .o .asm .c

CC=vc +aos68k
CFLAGS=-c99 -I$(NDK_INC) -DDEBUG

all: xunlimited xlimited xylimited

clean:
	rm -f xunlimited xlimited *.o

%.o: %.c
	$(CC) $(CFLAGS)  -c -o $@ $^

%.o: %.asm
	$(ASM) $(ASM_FLAGS) -o $@ $<

xunlimited: hardware.o cop_x.o common.o xunlimited.o
	$(CC) -o $@ $^ -lamiga -lauto


xlimited: hardware.o cop_x.o common.o xlimited.o
	$(CC) -o $@ $^ -lamiga -lauto

xylimited: hardware.o cop_xy.o common.o xylimited.o
	$(CC) -o $@ $^ -lamiga -lauto
