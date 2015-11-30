CC=vc +kick13
CFLAGS=-c99 -I$(NDK_INC)

all: intuitoy1

clean:
	rm -f *.o intuitoy1

intuitoy1: intuitoy1.c
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@
