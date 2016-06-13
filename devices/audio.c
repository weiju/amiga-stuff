/* audio.c
   This is a cleaned up version of the example program in Commodore RKM (Devices)
 */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <graphics/gfxbase.h>
#include <devices/audio.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <stdio.h>

#define CLOCK_NTSC    (3579545)
#define CLOCK_PAL     (3546895)
#define SAMPLE_BYTES  (2)
#define DURATION_SECS (3)
#define FREQUENCY     (440)
#define SAMPLE_CYCLES (1)
#define VOLUME        (64)

extern struct GfxBase *GfxBase;
UBYTE whichannel[] = {1, 2, 4, 8};

int main(int argc, char **argv)
{
    struct IOAudio *AudioIO;
    struct MsgPort *AudioMP;
    struct Message *AudioMSG;
    BYTE           *waveptr;
    ULONG           error;
    LONG            clock;

    clock = (GfxBase->DisplayFlags & PAL) ? CLOCK_PAL : CLOCK_NTSC;

    AudioIO = (struct IOAudio *) AllocMem(sizeof(struct IOAudio), MEMF_PUBLIC | MEMF_CLEAR);
    if (AudioIO) {
        puts("IO block created...");
        AudioMP = CreatePort(0, 0);
        if (AudioMP) {
            puts("Port created...");
            AudioIO->ioa_Request.io_Message.mn_ReplyPort   = AudioMP;
            AudioIO->ioa_Request.io_Message.mn_Node.ln_Pri = 0;
            AudioIO->ioa_Request.io_Command                = ADCMD_ALLOCATE;
            AudioIO->ioa_Request.io_Flags                  = ADIOF_NOWAIT;
            AudioIO->ioa_AllocKey                          = 0;
            AudioIO->ioa_Data                              = whichannel;
            AudioIO->ioa_Length                            = sizeof(whichannel);
            puts("I/O block initialized for channel allocation...");
            error = OpenDevice(AUDIONAME, 0L, (struct IORequest *) AudioIO, 0L);
            if (!error) {
                printf("'%s' opened, channel allocated...\n", AUDIONAME);
                waveptr = (BYTE *) AllocMem(SAMPLE_BYTES, MEMF_CHIP|MEMF_PUBLIC);
                waveptr[0] =  127;
                waveptr[1] = -127;
                puts("Wave data ready...");
                AudioIO->ioa_Request.io_Message.mn_ReplyPort = AudioMP;
                AudioIO->ioa_Request.io_Command              = CMD_WRITE;
                AudioIO->ioa_Request.io_Flags                = ADIOF_PERVOL;
                AudioIO->ioa_Data                            = (BYTE *) waveptr;
                AudioIO->ioa_Length                          = SAMPLE_BYTES;
                AudioIO->ioa_Period = clock * SAMPLE_CYCLES / (SAMPLE_BYTES * FREQUENCY);
                AudioIO->ioa_Volume                          = VOLUME;
                AudioIO->ioa_Cycles = FREQUENCY * DURATION_SECS / SAMPLE_CYCLES;
                puts("I/O block initialized to play tone...");
                puts("Starting tone now...");
                BeginIO((struct IORequest *) AudioIO);
                WaitPort(AudioMP);
                AudioMSG = GetMsg(AudioMP);
                if (waveptr) FreeMem(waveptr, SAMPLE_BYTES);
                CloseDevice((struct IORequest *) AudioIO);
            }
            DeletePort(AudioMP);
        }
        FreeMem(AudioIO, sizeof(struct IOAudio));
    }
    return 0;
}
