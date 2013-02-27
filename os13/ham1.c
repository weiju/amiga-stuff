/*********************************************************************
 * HAM Mode Demo
 * ROM Kernel Reference Manual - Libraries and Devices
 **********************************************************************/
#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <stdio.h>
#include <stdlib.h>

#define XSIZE 11
#define YSIZE 6

struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;

struct RastPort *rp;
struct ViewPort *vp;

struct TextAttr TestFont = {
  "topaz.font", 8, 0, 0
};

struct Window *w;
struct Screen *screen;
struct IntuiMessage *message;

struct NewScreen ns = {
  0, 0, 320, 200, 6, 0, 1, HAM, CUSTOMSCREEN, &TestFont,
  " 256 different out of 4096", NULL
};

struct NewWindow nw = {
  0, 11, 320, 186, -1, -1, MOUSEBUTTONS | CLOSEWINDOW,
  ACTIVATE | WINDOWCLOSE,
  NULL, NULL,
  "colors at any given moment",
  NULL, NULL, 0, 0, 320, 186, CUSTOMSCREEN
};

LONG squarecolor[16 * 16], freecolors[4096 - (16 * 16)];
SHORT squares[16 * 16];
SHORT xpos[16], ypos[16];

char *number[] = {
  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
  "A", "B", "C", "D", "E", "F"
};

SHORT sStop, cStop, sequence;
BOOL textneeded;

int prompt();
int hamBox(LONG color, LONG x, LONG y);
int colorWheel();
int colorFull();

int main(int argc, char **argv)
{
  ULONG class;
  USHORT code, i;
  BOOL wheelmode;

  for (i = 0; i < 16; i++) {
    xpos[i] = (XSIZE + 4) * i + 20;
    ypos[i] = (YSIZE + 3) * i + 21;
  }
  GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 0);
  if (GfxBase == NULL) exit(100);
  IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 0);
  if (IntuitionBase == NULL) {
    CloseLibrary((struct Library *) GfxBase);
    exit(200);
  }

  screen = (struct Screen *) OpenScreen(&ns);
  if (screen == NULL) {
    CloseLibrary((struct Library *) IntuitionBase);
    CloseLibrary((struct Library *) GfxBase);
    exit(300);
  }
  nw.Screen = screen;
  w = (struct Window *) OpenWindow(&nw);
  if (w == NULL) {
    CloseScreen(screen);
    CloseLibrary((struct Library *) IntuitionBase);
    CloseLibrary((struct Library *) GfxBase);
    exit(400);
  }

  vp = &screen->ViewPort;
  rp = w->RPort;

  SetRGB4(vp, 0,  0,  0,  0);
  SetRGB4(vp, 1, 15,  0,  0);
  SetRGB4(vp, 2,  0, 15,  0);
  SetRGB4(vp, 3,  0,  0, 15);
  SetRGB4(vp, 4, 15, 15, 15);

  SetBPen(rp, 0);
  textneeded = TRUE;
  wheelmode = TRUE;

  for (;;) {
    Wait(1 << w->UserPort->mp_SigBit);
    while ((message = (struct IntuiMessage *)GetMsg(w->UserPort)) != NULL) {
      class = message->Class;
      code = message->Code;
      ReplyMsg((struct Message *) message);

      if (class == CLOSEWINDOW) {
        CloseWindow(w);
        CloseScreen(screen);
        CloseLibrary((struct Library *) IntuitionBase);
        CloseLibrary((struct Library *) GfxBase);
        exit(0);
      }
      if (class == MOUSEBUTTONS && code == SELECTDOWN) {
        wheelmode = NOT wheelmode;
        SetAPen(rp, 0);
        SetDrMd(rp, JAM1);
        RectFill(rp, 3, 12, 318, 183);
        textneeded = TRUE;
      }
    }
    if (wheelmode) colorWheel(); else colorFull();
  }
  return 0;
}

int colorFull()
{
  SHORT sChoice, cChoice, usesquare;
  LONG usecolor;

  if (textneeded) {
    prompt();
    sStop = 255;
    cStop = 4095 - 256;

    for (usecolor = 0; usecolor < 256; usecolor++) {
      usesquare = usecolor;
      squares[usesquare] = usesquare;
      squarecolor[usesquare] = usecolor;
      hamBox(usecolor, xpos[usesquare % 16], ypos[usesquare / 16]);
    }
    for (usecolor = 256; usecolor < 4095; usecolor++) {
      freecolors[usecolor - 256] = usecolor;
    }
  }

  /* randomly choose next square */
  sChoice = RangeRand(sStop + 1);
  usesquare = squares[sChoice];
  squares[sChoice] = squares[sStop];
  squares[sStop] = usesquare;

  if (NOT sStop--) sStop = 255;

  /* randomly choose new color */
  cChoice = RangeRand(cStop + 1);
  usecolor = freecolors[cChoice];
  freecolors[cChoice] = freecolors[cStop];
  freecolors[cStop] = squarecolor[usesquare];
  squarecolor[usesquare] = usecolor;

  if (NOT cStop--) cStop = 4095 - 256;

  hamBox(usecolor, xpos[usesquare % 16], ypos[usesquare / 16]);
  return 0;
}

int colorWheel()
{
  SHORT i, j;
  if (textneeded) {
    prompt();
    SetAPen(rp, 2);  /* green pen for green color numbers */
    Move(rp, 260, ypos[15] + 17);
    Text(rp, "Green", 5);

    for (i = 0; i < 16; i++) {
      Move(rp, xpos[i] + 3, ypos[15] + 17);
      Text(rp, number[i], 1);
    }

    SetAPen(rp, 3);  /* blue pen for blue color numbers */
    Move(rp, 4, 18);
    Text(rp, "Blue", 4);
    for (i = 0; i < 16; i++) {
      Move(rp, 7, ypos[i] + 6);
      Text(rp, number[i], 1);
    }

    SetAPen(rp, 1);  /* red pen for red color numbers */
    Move(rp, 271, 100);
    Text(rp, "Red", 3);

    sequence = 0;
  }

  SetAPen(rp, 1);  /* Identify the red color in use */
  SetDrMd(rp, JAM2);
  Move(rp, 280, 115);
  Text(rp, number[sequence], 1);

  for (j = 0; j < 16; j++)
    for (i = 0; i < 16; i++)
      hamBox((sequence << 8 | i << 4 | j), xpos[i], ypos[j]);

  if (++sequence == 16) sequence = 0;
  return 0;
}

int prompt()
{
  SetDrMd(rp, JAM2);
  SetAPen(rp, 4);
  Move(rp, 23, 183);
  Text(rp, "[left mouse button = new mode]", 30);
  textneeded = FALSE;
  return 0;
}

int hamBox(LONG color, LONG x, LONG y)
{
  SHORT c;
  SetDrMd(rp, JAM1);
  c = ((color & 0xf00) >> 8);  /* extract red color component */
  SetAPen(rp, c + 0x20); /* Hold G, B from previous pixel. Set R = n */
  Move(rp, x, y);
  Draw(rp, x, y + YSIZE);

  x++;
  c = ((color & 0x0f0) >> 4);  /* extract green color component */
  SetAPen(rp, c + 0x30); /* Hold R, B from previous pixel. Set G = n */
  Move(rp, x, y);
  Draw(rp, x, y + YSIZE);

  x++;
  c = color & 0x0f;  /* extract blue color component */
  SetAPen(rp, c + 0x10);  /* Hold R, G from previous pixel. Set B = n */
  RectFill(rp, x, y, x + XSIZE - 2, y + YSIZE);
  return 0;
}
