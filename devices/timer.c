/*
 * timer.c - Small example program to see how the timer.device
 * works
 */
#include <stdio.h>
#include <stdlib.h>

#include <exec/types.h>
#include <exec/ports.h>
#include <devices/timer.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

struct Library *TimerBase = NULL;

int main(int argc, char **argv)
{
  struct MsgPort *timer_port = NULL;
  struct timerequest *timer_io = NULL;
  BYTE error = 0;

  timer_port = CreatePort(NULL, 0);
  if (!timer_port) {
    printf("could not create message port\n");
    return 0;
  }
  timer_io = (struct timerequest *) CreateExtIO(timer_port,
                                                sizeof(struct timerequest));
  if (!timer_io) {
    printf("could not create create timerequest object\n");
    DeletePort(timer_port);
    return 0;
  }
  error = OpenDevice(TIMERNAME, UNIT_MICROHZ,
                     (struct IORequest *) timer_io, 0);
  if (error) {
    printf("could not open timer.device\n");
  } else {
    ULONG secs, mins, hours, days;

    TimerBase = (struct Library *) timer_io->tr_node.io_Device;
    timer_io->tr_node.io_Command = TR_GETSYSTIME;
    DoIO((struct IORequest *) timer_io);
    secs = timer_io->tr_time.tv_secs;

    mins = secs / 60;
    hours = mins / 60;
    days = hours / 24;
    secs %= 60;
    mins %= 60;
    hours %= 24;

    printf("%lu days, %02lu:%02lu:%02lu\n",
           days, hours, mins, secs);

    /*
     * clean close of device, note that this sequence will
     * either crash or hang indefinitely if no DoIO() was
     * sent to the device
     */
    if (!CheckIO((struct IORequest *) timer_io)) {
      AbortIO((struct IORequest *) timer_io);
    }
    WaitIO((struct IORequest *) timer_io);
    TimerBase = NULL;
  }
  /* cleanup */
  CloseDevice((struct IORequest *) timer_io);
  if (timer_io) DeleteExtIO((struct IORequest *) timer_io);
  if (timer_port) DeletePort(timer_port);
  return 0;
}
