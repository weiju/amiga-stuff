#include <clib/exec_protos.h>
#include <hardware/custom.h>
#include <stdio.h>
#include "common.h"

/*
 * A simple setup to display a sprite.
 */
extern struct Custom custom;

static UWORD __chip coplist_pal[] = {
    COP_MOVE(SPR0PTH, 0x0000),
    COP_MOVE(SPR0PTL, 0x0000),
    COP_WAIT_END,
    COP_WAIT_END
};
static UWORD __chip coplist_ntsc[] = {
    COP_MOVE(SPR0PTL, 0x0000),
    COP_MOVE(SPR0PTH, 0x0000),
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
    coplist_ntsc[1] = ((ULONG) spdat0) & 0xffff;
    coplist_ntsc[3] = (((ULONG) spdat0) >> 16) & 0xffff;
    coplist_pal[1] = ((ULONG) spdat0) & 0xffff;
    coplist_pal[3] = (((ULONG) spdat0) >> 16) & 0xffff;

    custom.cop1lc = is_pal ? (ULONG) coplist_pal : (ULONG) coplist_ntsc;

    waitmouse();

    reset_display();
    return 0;
}
