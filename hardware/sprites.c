#include <hardware/custom.h>
#include <hardware/cia.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>
#include <stdio.h>

/*
 * A simple setup to display a sprite.
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

#define BPLCON0_COLOR (1 << 9)

#define COP_MOVE(addr, data) addr, data
#define COP_WAIT_END  0xffff, 0xfffe

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
    custom.cop1lc = is_pal ? (ULONG) coplist_pal : (ULONG) coplist_ntsc;
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

    coplist_ntsc[1] = ((ULONG) spdat0) & 0xffff;
    coplist_ntsc[3] = (((ULONG) spdat0) >> 16) & 0xffff;
    coplist_pal[1] = ((ULONG) spdat0) & 0xffff;
    coplist_pal[3] = (((ULONG) spdat0) >> 16) & 0xffff;

    init_display(lib_version);

    waitmouse();

    reset_display(current_view, lib_version);
    return 0;
}
