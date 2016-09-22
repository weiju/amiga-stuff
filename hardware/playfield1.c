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
extern struct Library *GfxBase;

// VBCC Inline assembly
void waitmouse(void) = "waitmouse:\n\tbtst\t#6,$bfe001\n\tbne\twaitmouse";

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
        //printf("PAL/NTSC: %d\n", (int) ((struct ExecBase *) EXEC_BASE)->VBlankFrequency);
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

    ULONG pl1data = (ULONG) image_data;
    ULONG pl2data = ((ULONG) &image_data[20 * NUM_RASTER_LINES]);

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

    reset_display(current_view, lib_version);
    return 0;
}
