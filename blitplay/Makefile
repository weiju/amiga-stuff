CC=vc +aos68k
CFLAGS=-c99 -I$(MUI_INC) -I$(NDK_INC)
OBJECTS=main.o canvas.o
EXE=blitplay

all: $(EXE)

clean:
	rm -f *.o $(EXE)

c.o :
	$(CC) $(CFLAGS) -c $<

$(EXE):$(OBJECTS) mui_util.h canvas.h
	$(CC) $(OBJECTS) -lamiga -o $(EXE)
