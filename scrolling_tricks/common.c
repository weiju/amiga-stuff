#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <string.h>

#include "global_defs.h"
#include "common.h"

static ULONG get_mode_id_os3(void)
{
    ULONG modeid = INVALID_ID;
	struct DimensionInfo diminfo;
	DisplayInfoHandle	dih;

    if ((dih = FindDisplayInfo(VGAPRODUCT_KEY))) {
        if (GetDisplayInfoData(dih, (APTR) &diminfo, sizeof(diminfo), DTAG_DIMS, 0))	{
            if (diminfo.MaxDepth >= BLOCKSDEPTH) modeid = VGAPRODUCT_KEY;
        }
    }
    return modeid;
}

ULONG get_mode_id(BOOL option_how, BOOL option_ntsc)
{
    ULONG modeid;

	if (option_how)	{
		modeid = get_mode_id_os3();

		if (modeid == INVALID_ID) {
			if (option_ntsc) modeid = NTSC_MONITOR_ID | HIRESLACE_KEY;
			else modeid = PAL_MONITOR_ID | HIRESLACE_KEY;
		}
	} else {
		if (option_ntsc) modeid = NTSC_MONITOR_ID;
		else modeid = PAL_MONITOR_ID;
	}
    return modeid;
}

BOOL get_arguments(struct PrgOptions *options, char *s)
{
	struct RDArgs *myargs;
    LONG	args[NUM_ARGS];

    for (int i = 0; i < NUM_ARGS; i++) args[i] = 0;
	if (!(myargs = ReadArgs(ARG_TEMPLATE, args, 0))) {
		Fault(IoErr(), 0, s, 255);
        return FALSE;
	}

	if (args[ARG_SPEED]) options->speed = TRUE;
	if (args[ARG_NTSC]) options->ntsc = TRUE;
	if (args[ARG_HOW]) {
		options->how = TRUE;
		options->speed = FALSE;
	}
	if (args[ARG_SKY] && (!options->speed)) {
		options->sky = TRUE;
	}
	if (args[ARG_FMODE]) {
		options->fetchmode = *(LONG *) args[ARG_FMODE];
	}
	FreeArgs(myargs);

	if (options->fetchmode < 0 || options->fetchmode > 3) {
        strcpy(s, "Invalid fetch mode. Must be 0 .. 3!");
        return FALSE;
	}
    return TRUE;
}

BOOL read_level_map(const char *path, struct LevelMap *level_map, char *s)
{
	LONG l;
    BPTR fhandle;
    struct RawMap *raw_map;

	if (!(fhandle = Open(path, MODE_OLDFILE)))
	{
		Fault(IoErr(), 0, s, 255);
		return FALSE;
	}

	Seek(fhandle, 0, OFFSET_END);
	l = Seek(fhandle, 0, OFFSET_BEGINNING);

	if (!(raw_map = AllocVec(l, MEMF_PUBLIC))) {
		strcpy(s, "Out of memory!");
        if (fhandle) Close(fhandle);
        return FALSE;
	}

	if (Read(fhandle, raw_map, l) != l)
	{
		Fault(IoErr(), 0, s, 255);
        if (fhandle) Close(fhandle);
        return FALSE;
	}
	Close(fhandle);

	level_map->data = raw_map->data;
	level_map->width = raw_map->mapwidth;
	level_map->height = raw_map->mapheight;
    return TRUE;
}

struct BitMap *read_blocks(UWORD *colors, char *s, int blocks_width, int blocks_height)
{
	LONG l;
    struct BitMap *bitmap;
    BPTR fhandle;

	if (!(bitmap = AllocBitMap(blocks_width,
                               blocks_height,
                               BLOCKSDEPTH,
                               BMF_STANDARD | BMF_INTERLEAVED,
                               0))) {
		strcpy(s, "Can't alloc blocks bitmap!");
        return NULL;
	}

	if (!(fhandle = Open(BLOCKSNAME, MODE_OLDFILE))) {
		Fault(IoErr(), 0, s, 255);
        FreeBitMap(bitmap);
		return NULL;
	}

	if (Read(fhandle, colors, PALSIZE) != PALSIZE) {
		Fault(IoErr(), 0, s, 255);
        FreeBitMap(bitmap);
        Close(fhandle);
        return NULL;
	}

	l = blocks_width * blocks_height * BLOCKSDEPTH / 8;

	if (Read(fhandle, bitmap->Planes[0], l) != l) {
		Fault(IoErr(), 0, s, 255);
        FreeBitMap(bitmap);
        Close(fhandle);
        return NULL;
	}
	Close(fhandle);
    return bitmap;
}
