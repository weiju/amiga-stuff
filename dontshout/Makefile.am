if VBCC
AM_CFLAGS = -I$(NDK_INC)
AM_LDFLAGS = -lamiga -lauto
endif
if MORPHOS
AM_CFLAGS =
AM_LDFLAGS = -noixemul
endif
if AMIGAOS4
AM_CFLAGS =
AM_LDFLAGS = -lauto
endif


bin_PROGRAMS = dontshout

dontshout_SOURCES = dontshout.c args.c

include_HEADERS = args.h
