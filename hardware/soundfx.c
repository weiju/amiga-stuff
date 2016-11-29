#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <clib/exec_protos.h>
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
char VERSION_STRING[] = "\0$VER: startup 1.0 (21.09.2016)\0";

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

// Assuming a 14k sample rate
#define SOUND_DATA_SIZE (25074)
// 0-64
#define MAX_VOLUME (64)
#define SAMPLE_PERIOD (256)

static UBYTE __chip sound_data[SOUND_DATA_SIZE];

void start_channel(int channel)
{
    int channel_bits = 0;
    switch (channel) {
    case 0:
        channel_bits = DMAF_AUD0;
        break;
    case 1:
        channel_bits = DMAF_AUD1;
        break;
    case 2:
        channel_bits = DMAF_AUD2;
        break;
    case 3:
        channel_bits = DMAF_AUD3;
        break;
    default:
        break;
    }
    custom.dmacon = (DMAF_SETCLR | channel_bits | DMAF_MASTER);
}

void stop_channel(int channel)
{
    int channel_bits = 0;
    switch (channel) {
    case 0:
        channel_bits = DMAF_AUD0;
        break;
    case 1:
        channel_bits = DMAF_AUD1;
        break;
    case 2:
        channel_bits = DMAF_AUD2;
        break;
    case 3:
        channel_bits = DMAF_AUD3;
        break;
    default:
        break;
    }
    custom.dmacon = channel_bits;
}

int main(int argc, char **argv)
{
    struct Task *current_task = FindTask(NULL);
    BYTE old_prio = SetTaskPri(current_task, TASK_PRIORITY);
    BOOL is_pal = init_display();
    FILE *fp = fopen("laser.raw8", "rb");
    int bytes_read = fread(sound_data, sizeof(UBYTE), SOUND_DATA_SIZE, fp);
    custom.aud[0].ac_ptr = (UWORD *) sound_data;
    custom.aud[0].ac_len = SOUND_DATA_SIZE / 2;
    custom.aud[0].ac_vol = MAX_VOLUME;
    custom.aud[0].ac_per = SAMPLE_PERIOD;
    start_channel(0);


    custom.cop1lc = is_pal ? (ULONG) coplist_pal : (ULONG) coplist_ntsc;

    // strobe the COPJMP1 register to make sure the system is using
    // copper list 1 (I found out that leaving this out can lead to
    // strange effects on an emulated 4000 system)
    custom.copjmp1 = 1;

    waitmouse();
    stop_channel(0);

    reset_display();
    return 0;
}
