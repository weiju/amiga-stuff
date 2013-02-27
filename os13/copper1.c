/*
 * User-Copper-Lists Demo Program.
 * ROM Kernel Reference Manual, Libraries and Devices
 */
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfxmacros.h>
#include <graphics/copper.h>
#include <intuition/intuition.h>
#include <hardware/custom.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <stdlib.h>

#define WINDOWGADGETS (WINDOWSIZING|WINDOWDRAG|WINDOWDEPTH|WINDOWCLOSE)
#define WWIDTH 120
#define WHEIGHT 90

#ifndef MAXINT
#define MAXINT 0xffffffff
#endif

extern struct Window *OpenWindow();
extern struct Screen *OpenScreen();

struct Library *IntuitionBase = 0;
struct Library *GfxBase = 0;

struct TextAttr TestFont = {
  "topaz.font", 8, 0, 0
};

struct NewScreen ns = {
  0, 0, 320, 200, 4, 0, 1, 0, CUSTOMSCREEN, &TestFont, "Test Screen", NULL
};

extern struct Custom custom;

int main()
{
  struct Window *w;
  struct RastPort *rp;
  struct ViewPort *vp;
  struct UCopList *cl;
  struct Screen *screen;

  GfxBase = OpenLibrary("graphics.library", 0);
  if (GfxBase == NULL) exit(1000);
  IntuitionBase = OpenLibrary("intuition.library", 0);
  if (IntuitionBase == NULL) {
    CloseLibrary(GfxBase);
    exit(2000);
  }
  screen = OpenScreen(&ns);
  if (!screen) {
    goto cleanup;
  } else {
    vp = &screen->ViewPort;
    rp = &screen->RastPort;
  }
  /* init copper list in V1.1 we need to do the following, for >= 1.2, we can use CINIT */
  cl = (struct UCopList *) AllocMem(sizeof(struct UCopList), MEMF_PUBLIC|MEMF_CLEAR);
  CWAIT(cl, 100, 0);
  CMOVE(cl, custom.color[0], 0xfff);
  CEND(cl);

  vp->UCopIns = cl;
  Delay(50);
  RethinkDisplay();
  Delay(100);
  CloseScreen(screen);

 cleanup:
  CloseLibrary(IntuitionBase);
  CloseLibrary(GfxBase);
}

