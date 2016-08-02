#CC=vc +aos68k
CC=vc +kick13
CFLAGS=-c99 -I$(NDK_INC) -DDEBUG -O2

all: main

clean:
	rm -f *.o main dos_compat_test file_list_test

main: main.o filereq.o dos13.o file_list.o dos_compat.o
	$(CC) $(CFLAGS) $^ -lamiga -lauto -o $@


check: file_list_test dos_compat_test

file_list_test: file_list.c file_list_test.c ../chibi_test/chibi.c
	gcc $^ -o $@
	./file_list_test


dos_compat_test: dos_compat.c dos_compat_test.c ../chibi_test/chibi.c
	gcc $^ -o $@
	./dos_compat_test
