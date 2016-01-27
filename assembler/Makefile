ASM_FLAGS = -Fhunk -devpac -I/home/weiju/Development/NDK_3.9/Include/include_i
ASM = vasmm68k_mot
.SUFFIXES : .o .asm
PROGRAMS = test1 hello test2 mouse raster1 raster2

all: $(PROGRAMS)

.asm.o:
	$(ASM) $(ASM_FLAGS) -o $@ $<
clean:
	rm -f $(PROGRAMS) *.o

test1: test1.o
	vlink -bamigahunk -o $@ -s $<

test2: test2.o
	vlink -bamigahunk -o $@ -s $<

hello: hello.o
	vlink -bamigahunk -o $@ -s $<

mouse: mouse.o
	vlink -bamigahunk -o $@ -s $<

raster1: raster1.o
	vlink -bamigahunk -o $@ -s $<

raster2: raster2.o
	vlink -bamigahunk -o $@ -s $<
