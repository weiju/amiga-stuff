/* C version of How To Code 7 http://aminet.net/package/docs/misc/howtocode7 */
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>
#include <hardware/custom.h>

#include <stdio.h>

#include "common.h"

extern struct Custom custom;
extern struct GfxBase *GfxBase;
static struct Screen *wbscreen;
static ULONG oldresolution;

struct ExecBase **exec_base_ptr = (struct ExecBase **) (4L);

static void apply_sprite_fix(void)
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

static void unapply_sprite_fix(void)
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

BOOL init_display(void)
{
    UWORD lib_version = GfxBase->LibNode.lib_Version;
    BOOL is_pal;

    LoadView(NULL);  // clear display, reset hardware registers
    WaitTOF();       // 2 WaitTOFs to wait for 1. long frame and
    WaitTOF();       // 2. short frame copper lists to finish (if interlaced)

    // Kickstart > 3.0: fix sprite bug
    if (lib_version >= 39) {
        apply_sprite_fix();
        is_pal = (((struct GfxBase *) GfxBase)->DisplayFlags & PAL) == PAL;
    } else {
        int vblank_freq = (*exec_base_ptr)->VBlankFrequency;
        printf("Gfx Lib version: %u, Vertical Blank Frequency: %d\n", lib_version, vblank_freq);
        is_pal = vblank_freq == VFREQ_PAL;
    }
    return is_pal;
}

void reset_display(void)
{
    struct View *current_view = ((struct GfxBase *) GfxBase)->ActiView;
    UWORD lib_version = GfxBase->LibNode.lib_Version;
    if (lib_version >= 39) unapply_sprite_fix();
    LoadView(current_view);
    WaitTOF();
    WaitTOF();
    custom.cop1lc = (ULONG) ((struct GfxBase *) GfxBase)->copinit;
    RethinkDisplay();
}

#define PRA_FIR0_BIT (1 << 6)

void waitmouse(void)
{
    volatile UBYTE *ciaa_pra = (volatile UBYTE *) 0xbfe001;
    while ((*ciaa_pra & PRA_FIR0_BIT) != 0) ;
}
