CC	= vc +aos68k
CFLAGS   = -Iexample/include -Iinclude -I$(NDK_INC)

all: TestExampleLib

clean:
	rm -f *.o TestExampleLib

TestExampleLib: TestExampleLib.c
	$(CC) $(CFLAGS) -lamiga -o $@ $^
