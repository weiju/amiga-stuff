#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__

#include <exec/types.h>

struct PrgOptions {
    BOOL ntsc, how, speed, sky;
    WORD fetchmode;
};

struct FetchInfo
{
	WORD	ddfstart;
	WORD	ddfstop;
	WORD	modulooffset;
	WORD	bitmapoffset;
	WORD	scrollpixels;
};

extern ULONG get_mode_id(BOOL option_how, BOOL option_ntsc);
extern BOOL get_arguments(struct PrgOptions *options, char *s);

#endif /* __COMMON_H__ */
