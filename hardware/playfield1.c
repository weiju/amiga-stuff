#include <hardware/custom.h>
#include <hardware/cia.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>
#include <stdio.h>

#include "test-320x200.h"

/*
 * A simple setup to display a playfield with a depth of 1 bit.
 */
extern struct Custom custom;
extern struct CIA ciaa, ciab;
extern struct Library *GfxBase;

// VBCC Inline assembly
void waitmouse(void) = "waitmouse:\n\tbtst\t#6,$bfe001\n\tbne\twaitmouse";

#define EXEC_BASE (4L)
#define TASK_PRIORITY 127
#define VFREQ_PAL 50
#define WB_SCREEN_NAME "Workbench"

#define BPLCON0       0x100
#define COLOR00       0x180
#define SPR0PTH       0x120
#define SPR0PTL       0x122
#define BPL1PTH       0x0e0
#define BPL1PTL       0x0e2


#define BPLCON0_COLOR (1 << 9)

#define COLOR0        0x00a
#define COLOR1        0xfff

#define COP_MOVE(addr, data) addr, data
#define COP_WAIT_END  0xffff, 0xfffe

static UWORD __chip coplist[] = {
    COP_MOVE(BPL1PTH, 0),
    COP_MOVE(BPL1PTL, 0),
    COP_WAIT_END,
    COP_WAIT_END
};

static struct Screen *wbscreen;
static ULONG oldresolution;

static void ApplySpriteFix(void)
{
    if (wbscreen = LockPubScreen(WB_SCREEN_NAME)) {
        struct TagItem video_control_tags[] = {
            {VTAG_SPRITERESN_GET, SPRITERESN_ECS},
            {TAG_DONE, 0}
        };
        struct TagItem video_control_tags2[] = {
            {VTAG_SPRITERESN_SET, SPRITERESN_140NS},
            {TAG_DONE, 0}
        };
        VideoControl(wbscreen->ViewPort.ColorMap, video_control_tags);
        oldresolution = video_control_tags[0].ti_Data;
        VideoControl(wbscreen->ViewPort.ColorMap, video_control_tags2);
        MakeScreen(wbscreen);
        RethinkDisplay();
    }
}

static void UnapplySpriteFix(void)
{
    if (wbscreen) {
        struct TagItem video_control_tags[] = {
            {VTAG_SPRITERESN_SET, oldresolution},
            {TAG_DONE, 0}
        };
        VideoControl(wbscreen->ViewPort.ColorMap, video_control_tags);
        MakeScreen(wbscreen);
        UnlockPubScreen(NULL, wbscreen);
    }
}

static BOOL init_display(UWORD lib_version)
{
    BOOL is_pal;

    LoadView(NULL);  // clear display, reset hardware registers
    WaitTOF();       // 2 WaitTOFs to wait for 1. long frame and
    WaitTOF();       // 2. short frame copper lists to finish (if interlaced)

    // Kickstart > 3.0: fix sprite bug
    if (lib_version >= 39) {
        ApplySpriteFix();
        is_pal = (((struct GfxBase *) GfxBase)->DisplayFlags & PAL) == PAL;
    } else {
        // Note: FS-UAE reports 0 this, so essentially, there is no information
        // for 1.x
        printf("PAL/NTSC: %d\n", (int) ((struct ExecBase *) EXEC_BASE)->VBlankFrequency);
        is_pal = ((struct ExecBase *) EXEC_BASE)->VBlankFrequency == VFREQ_PAL;
    }
    return is_pal;
}

static void reset_display(struct View *current_view, UWORD lib_version)
{
    if (lib_version >= 39) UnapplySpriteFix();
    LoadView(current_view);
    WaitTOF();
    WaitTOF();
    custom.cop1lc = (ULONG) ((struct GfxBase *) GfxBase)->copinit;
    RethinkDisplay();
}

int main(int argc, char **argv)
{
    // translated startup.asm
    struct Task *current_task = FindTask(NULL);
    BYTE old_prio = SetTaskPri(current_task, TASK_PRIORITY);
    struct View *current_view = ((struct GfxBase *) GfxBase)->ActiView;
    UWORD lib_version = ((struct Library *) GfxBase)->lib_Version;

    BOOL is_pal = init_display(lib_version);

    custom.bplcon0 = 0x1200;  // use bitplane 1 = BPU 001, composite color enable
    custom.bplcon1 = 0;  // horizontal scroll value = 0 for all playfields
    custom.bpl1mod = 0;  // modulo = 0 for odd bitplanes
    custom.ddfstrt = 0x0038;
    custom.ddfstop = 0x00d0;
    custom.diwstrt = 0x2c81;
    custom.diwstop = 0xf4c1;
    custom.color[0] = COLOR0;  // background red
    custom.color[1] = COLOR1;  // color 1 is yellow

    // Initialize copper list with image data address
    coplist[1] = (((ULONG) imdata) >> 16) & 0xffff;
    coplist[3] = ((ULONG) imdata) & 0xffff;

    // and point to the copper list
    custom.cop1lc = (ULONG) coplist;

    waitmouse();

    reset_display(current_view, lib_version);
    return 0;
}
