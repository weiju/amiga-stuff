MUI_INC=/home/weiju/Development/mui-3.8-dev/C/Include
CC=vc +aos68k
CFLAGS=-c99 -I$(MUI_INC) -I$(NDK_INC) -DCLIB_MUIMASTER_PROTOS_H

all: openwin openwin2

clean:
	rm -f openwin openwin2 *.o *.i

openwin: openwin.c mui_util.h
	$(CC) $(CFLAGS) openwin.c -lamiga -o openwin

openwin2: openwin2.c mui_util.h
	$(CC) $(CFLAGS) openwin2.c -lamiga -o openwin2
