#include <hardware/custom.h>
#include <hardware/cia.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>
#include <stdio.h>

#include "common.h"

/*
 * This is essentially the code for startup.asm from
 * "Amiga Demo Coders Reference Manual" translated to C.
 *
 * This allows a program which operates directly on the hardware
 * to play nice with the operating system and ensure a clean exit
 * to Workbench.
 * A great starting point to use as a template for demos and games.
 */
extern struct Custom custom;
extern struct Library *GfxBase;

static UWORD __chip coplist_pal[] = {
    COP_MOVE(BPLCON0, BPLCON0_COMPOSITE_COLOR),
    COP_MOVE(COLOR00, 0x000),
    0x8107, 0xfffe,            // wait for $8107,$fffe
    COP_MOVE(COLOR00, 0xf00),
    0xd607, 0xfffe,            // wait for $d607,$fffe
    COP_MOVE(COLOR00, 0xff0),
    COP_WAIT_END,
    COP_WAIT_END
};
static UWORD __chip coplist_ntsc[] = {
    COP_MOVE(BPLCON0, BPLCON0_COMPOSITE_COLOR),
    COP_MOVE(COLOR00, 0x000),
    0x6e07, 0xfffe,           // wait for $6e07,$fffe
    COP_MOVE(COLOR00, 0xf00),
    0xb007, 0xfffe,           // wait for $b007,$fffe
    COP_MOVE(COLOR00, 0xff0),
    COP_WAIT_END,
    COP_WAIT_END
};

int main(int argc, char **argv)
{
    // translated startup.asm
    struct Task *current_task = FindTask(NULL);
    BYTE old_prio = SetTaskPri(current_task, TASK_PRIORITY);
    struct View *current_view = ((struct GfxBase *) GfxBase)->ActiView;
    UWORD lib_version = ((struct Library *) GfxBase)->lib_Version;

    BOOL is_pal = init_display(lib_version);
    custom.cop1lc = is_pal ? (ULONG) coplist_pal : (ULONG) coplist_ntsc;

    waitmouse();

    reset_display(current_view, lib_version);
    return 0;
}
