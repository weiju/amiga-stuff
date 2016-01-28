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
 * This is essentially the code for startup.asm from
 * "Amiga Demo Coders Reference Manual" translated to C.
 *
 * This allows a program which operates directly on the hardware
 * to play nice with the operating system and ensure a clean exit
 * to Workbench.
 * A great starting point to use as a template for demos and games.
 */
extern struct Custom custom;
extern struct CIA ciaa, ciab;
extern struct Library *GfxBase;

// VBCC Inline assembly
void waitmouse() = "waitmouse:\n\tbtst\t#6,$bfe001\n\tbne\twaitmouse";

#define EXEC_BASE (4L)
#define TASK_PRIORITY 127
#define VFREQ_PAL 50
#define WB_SCREEN_NAME "Workbench"

static UWORD __chip coplist_pal[] = {
    0x100, 0x200,        // otherwise no display!
    0x180, 0x00,
    0x8107, 0xfffe,      // wait for $8107,$fffe
    0x180,
    0xf0f,               // background red
    0xd607, 0xfffe,      // wait for $d607,$fffe
    0x180, 0xff0,        // background yellow
    0xffff, 0xfffe,
    0xffff, 0xfffe
};
static UWORD __chip coplist_ntsc[] = {
    0x100, 0x0200,      // otherwise no display!
    0x180, 0x00,
    0x6e07, 0xfffe,     // wait for $6e07,$fffe
    0x180, 0xf00,       // background red
    0xb007, 0xfffe,     // wait for $b007,$fffe
    0x180, 0xff0,       // background yellow
    0xffff, 0xfffe,
    0xffff, 0xfffe
};

static struct Screen *wbscreen;
static ULONG oldresolution;

static void ApplySpriteFix()
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
static void UnapplySpriteFix()
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

int main(int argc, char **argv)
{
    // translated startup.asm
    struct Task *current_task = FindTask(NULL);
    BYTE old_prio = SetTaskPri(current_task, TASK_PRIORITY);
    struct View *current_view = ((struct GfxBase *) GfxBase)->ActiView;
    UWORD lib_version = ((struct Library *) GfxBase)->lib_Version;
    BOOL is_pal;

    LoadView(NULL);
    WaitTOF();
    WaitTOF();

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

    waitmouse();
    if (lib_version >= 39) UnapplySpriteFix();
    LoadView(current_view);
    WaitTOF();
    WaitTOF();
    custom.cop1lc = (ULONG) ((struct GfxBase *) GfxBase)->copinit;
    RethinkDisplay();
    return 0;
}
