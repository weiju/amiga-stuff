ASM_FLAGS = -Fhunk -devpac -I/home/weiju/Development/NDK_3.9/Include/include_i
ASM = vasmm68k_mot

all: test1 hello test2

clean:
	rm -f test1 test2 hello *.o

test1: test1.o
	vlink -bamigahunk -o test1 -s test1.o

test1.o: test1.asm
	$(ASM) $(ASM_FLAGS) -o test1.o test1.asm

test2: test2.o
	vlink -bamigahunk -o test2 -s test2.o

test2.o: test2.asm
	$(ASM) $(ASM_FLAGS) -o test2.o test2.asm

hello: hello.o
	vlink -bamigahunk -o hello -s hello.o

hello.o: hello.asm
	$(ASM) $(ASM_FLAGS) -o hello.o hello.asm
