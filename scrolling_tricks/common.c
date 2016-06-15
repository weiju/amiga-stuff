#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
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
