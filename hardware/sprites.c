#include <clib/exec_protos.h>
#include <hardware/custom.h>
#include <stdio.h>
#include "common.h"

#include "kingtut.h"
#define NUM_COLORS 32
#define NUM_BITPLANES 5

/*
 * A simple setup to display a sprite.
 */
extern struct Custom custom;
char VERSION_STRING[] = "\0$VER: sprites 1.0 (21.09.2016)\0";

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
    COP_MOVE(SPR0PTH, 0),
    COP_MOVE(SPR0PTL, 0),
    COP_WAIT_END,
    COP_WAIT_END
};

// space ship data from the Hardware Reference Manual
static UWORD __chip spdat0[] = {
    0x6d60, 0x7200,  // VSTART+HSTART, VSTOP
    // data here
    0x0990, 0x07e0,
    0x13c8, 0x0ff0,
    0x23c4, 0x1ff8,
    0x13c8, 0x0ff0,
    0x0990, 0x07e0,
    0x0000, 0x0000
};

int main(int argc, char **argv)
{
    struct Task *current_task = FindTask(NULL);
    BYTE old_prio = SetTaskPri(current_task, TASK_PRIORITY);
    BOOL is_pal = init_display();

    custom.bplcon0 = 0x5200;  // use bitplane 1-5 = BPU 101, composite color enable
    custom.bplcon1 = 0;       // horizontal scroll value = 0 for all playfields
    custom.bplcon2 = 0x24;    // sprites have priority over playfields
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

    coplist[21] = (((ULONG) spdat0) >> 16) & 0xffff;
    coplist[23] = ((ULONG) spdat0) & 0xffff;

    //custom.dmacon = 0x8020;
    custom.cop1lc = (ULONG) coplist;
    custom.copjmp1 = 1;

    waitmouse();

    reset_display();
    return 0;
}
