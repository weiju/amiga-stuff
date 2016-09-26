#include <clib/exec_protos.h>
#include <hardware/custom.h>
#include <stdio.h>
#include "common.h"

/*
 * A simple setup to display a sprite.
 */
extern struct Custom custom;
char VERSION_STRING[] = "\0$VER: sprites1 1.0 (21.09.2016)\0";

#define NUM_IMG_WORDS (20 * NUM_RASTER_LINES)
#define NUM_SPRITES   (8)
#define COPLIST_SPRPTR_OFFSET 4

static UWORD __chip imdata[NUM_IMG_WORDS];
static UWORD __chip coplist[] = {
    COP_MOVE(BPL1PTH, 0),
    COP_MOVE(BPL1PTL, 0),
    COP_MOVE(SPR0PTH, 0),
    COP_MOVE(SPR0PTL, 0),
    COP_MOVE(SPR1PTH, 0),
    COP_MOVE(SPR1PTL, 0),
    COP_MOVE(SPR2PTH, 0),
    COP_MOVE(SPR2PTL, 0),
    COP_MOVE(SPR3PTH, 0),
    COP_MOVE(SPR3PTL, 0),
    COP_MOVE(SPR4PTH, 0),
    COP_MOVE(SPR4PTL, 0),
    COP_MOVE(SPR5PTH, 0),
    COP_MOVE(SPR5PTL, 0),
    COP_MOVE(SPR6PTH, 0),
    COP_MOVE(SPR6PTL, 0),
    COP_MOVE(SPR7PTH, 0),
    COP_MOVE(SPR7PTL, 0),
    COP_WAIT_END,
    COP_WAIT_END
};

// space ship data from the Hardware Reference Manual
static UWORD __chip sprite_data[] = {
    0x6d60, 0x7200,  // VSTART+HSTART, VSTOP
    // data here
    0x0990, 0x07e0,
    0x13c8, 0x0ff0,
    0x23c4, 0x1ff8,
    0x13c8, 0x0ff0,
    0x0990, 0x07e0,
    0x0000, 0x0000
};

volatile UBYTE *ciaa_pra = (volatile UBYTE *) 0xbfe001;
volatile UBYTE *custom_vhposr= (volatile UBYTE *) 0xdff006;
#define PRA_FIR0_BIT (1 << 6)
#define DELAY 1

int main(int argc, char **argv)
{
    struct Task *current_task = FindTask(NULL);
    BYTE old_prio = SetTaskPri(current_task, TASK_PRIORITY);
    BOOL is_pal = init_display();

    custom.bplcon0 = 0x1200;  // use bitplane 1-5 = BPU 101, composite color enable
    custom.bpl1mod = 0;       // modulo = 0 for odd bitplanes
    custom.bplcon1 = 0;       // horizontal scroll value = 0 for all playfields
    custom.bplcon2 = 0x24;    // sprites have priority over playfields
    custom.ddfstrt = DDFSTRT_VALUE;
    custom.ddfstop = DDFSTOP_VALUE;
    custom.diwstrt = DIWSTRT_VALUE;
    custom.diwstop = DIWSTOP_VALUE;

    custom.color[0]  = 0x008;
    custom.color[1]  = 0x000;
    custom.color[17] = 0xff0;  // color 1 for sprite 1
    custom.color[18] = 0x0ff;  // color 2 for sprite 1
    custom.color[19] = 0xf0f;  // color 3 for sprite 1

    // fill the whole image data array, effectively setting
    // the background to color 1
    for (int i = 0; i < NUM_IMG_WORDS; i++) imdata[i] = 0xffff;

    // set bitmap 0 pointer to initialized data
    ULONG addr = (ULONG) imdata;
    coplist[1] = (addr >> 16) & 0xffff;
    coplist[3] = addr & 0xffff;

    // point sprites 0-7 to the same data structure
    for (int i = 0; i < NUM_SPRITES; i++) {
        coplist[COPLIST_SPRPTR_OFFSET + i * 4 + 1] = (((ULONG) sprite_data) >> 16) & 0xffff;
        coplist[COPLIST_SPRPTR_OFFSET + i * 4 + 3] = ((ULONG) sprite_data) & 0xffff;
    }

    custom.cop1lc  = (ULONG) coplist;
    custom.dmacon  = 0x83a0;  // Bitplane + Copper + Sprite DMA activate
    custom.copjmp1 = 1;

    // wait for mouse button
    UBYTE hstart = 0x60;
    UBYTE vstart = 0x6d;
    UBYTE vstop = 0x72;
    int incx = 1, minx = 0x60, maxx = 0x90;
    int incy = 1, miny = 0x6d, maxy = 0x90;
    int delay = DELAY;

    while ((*ciaa_pra & PRA_FIR0_BIT) != 0) {
        // PAL vblank test
        while (*custom_vhposr != 0x00) ;
        delay--;
        if (delay == 0) {
            delay = DELAY;
            hstart += incx;
            vstart += incy;
            if (hstart >= maxx || hstart <= minx) incx = -incx;
            if (vstart >= maxy || vstart <= miny) incy = -incy;
        }
        vstop = vstart + 5;
        sprite_data[0] = (vstart << 8) | hstart;
        sprite_data[1] = vstop << 8;
    }

    reset_display();
    return 0;
}
