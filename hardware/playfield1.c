#include <hardware/custom.h>
#include <clib/exec_protos.h>
#include <stdio.h>

#include "common.h"

#define NUM_BITPLANES 5

#ifdef USE_PAL

#if NUM_BITPLANES == 1
#include "test320x256.h"
#define NUM_COLORS 2
#elif NUM_BITPLANES == 2
#include "test320x256x2.h"
#define NUM_COLORS 4
#else
#include "kingtut.h"
#define NUM_COLORS 32
#endif

#else
#include "test320x200.h"
#endif

/*
 * A simple setup to display a playfield with a depth of 1 bit.
 */
extern struct Custom custom;

static UWORD __chip coplist[] = {
    COP_MOVE(BPL1PTH, 0),
    COP_MOVE(BPL1PTL, 0),
    COP_MOVE(BPL2PTH, 0),
    COP_MOVE(BPL2PTL, 0),
    COP_MOVE(BPL3PTH, 0),
    COP_MOVE(BPL3PTL, 0),
    COP_MOVE(BPL4PTH, 0),
    COP_MOVE(BPL4PTL, 0),
    COP_MOVE(BPL5PTH, 0),
    COP_MOVE(BPL5PTL, 0),
    COP_WAIT_END,
    COP_WAIT_END
};

int main(int argc, char **argv)
{
    // translated startup.asm
    struct Task *current_task = FindTask(NULL);
    BYTE old_prio = SetTaskPri(current_task, TASK_PRIORITY);
    BOOL is_pal = init_display();

    // hardcoded for UAE, since it seems that the mode returned is always NTSC
    is_pal = USE_PAL;

#if NUM_BITPLANES == 1
    custom.bplcon0 = 0x1200;  // use bitplane 1 = BPU 001, composite color enable
#elif NUM_BITPLANES == 2
    custom.bplcon0 = 0x2200;  // use bitplane 1-2 = BPU 010, composite color enable
#else
    custom.bplcon0 = 0x5200;  // use bitplane 1-5 = BPU 101, composite color enable
#endif
    custom.bplcon1 = 0;  // horizontal scroll value = 0 for all playfields
    custom.bpl1mod = 0;  // modulo = 0 for odd bitplanes
    custom.ddfstrt = DDFSTRT_VALUE;
    custom.ddfstop = DDFSTOP_VALUE;

    custom.diwstrt = DIWSTRT_VALUE;
    custom.diwstop = DIWSTOP_VALUE;
    for (int i = 0; i < NUM_COLORS; i++) {
        custom.color[i] = image_colors[i];
    }

    // Initialize copper list with image data address
    for (int i = 0; i < NUM_BITPLANES; i++) {
        ULONG addr = (ULONG) &(image_data[i * 20 * NUM_RASTER_LINES]);
        coplist[i * 4 + 1] = (addr >> 16) & 0xffff;
        coplist[i * 4 + 3] = addr & 0xffff;
    }

    // and point to the copper list
    custom.cop1lc = (ULONG) coplist;

    waitmouse();

    reset_display();
    return 0;
}
