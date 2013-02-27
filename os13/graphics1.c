/**********************************************************************
 **** Single Playfield example program from Amiga Rom Kernel Manual
 **** This program temporarily hides the Intuition viewport and draws
 **** to its own viewport.
 **** On exit, it cleans up and returns to Intuition again.
 **********************************************************************/
#include <exec/types.h>
#include <exec/exec.h>
#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <graphics/copper.h>
#include <graphics/view.h>
#include <graphics/gels.h>
#include <graphics/regions.h>
#include <graphics/clip.h>
#include <graphics/text.h>
#include <graphics/gfxbase.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include <stdlib.h>

#define DEPTH 2
#define WIDTH 320
#define HEIGHT 256
#define NOT_ENOUGH_MEMORY -1000

struct View v;
struct ViewPort vp;
struct ColorMap *cm;
struct RasInfo ri;
struct BitMap b;
struct RastPort rp;

LONG i;
SHORT j, k, n;
extern struct ColorMap *GetColorMap();
struct GfxBase *GfxBase;
struct View *oldview;

USHORT colortable[] = { 0x000, 0xf00, 0x0f0, 0x00f };
SHORT boxoffsets[] = { 802, 2010, 3218 };

UBYTE *displaymem;
UWORD *colorpalette;

int DrawFilledBox(SHORT fillcolor, SHORT plane);
int FreeMemory();

int main(int argc, char **argv)
{
  GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 0);
  if (GfxBase == NULL) exit(1);
  oldview = GfxBase->ActiView;

  InitView(&v);
  InitVPort(&vp);
  v.ViewPort = &vp;
  InitBitMap(&b, DEPTH, WIDTH, HEIGHT);
  ri.BitMap = &b;
  ri.RxOffset = 0;
  ri.RyOffset = 0;
  ri.Next = NULL;
  vp.DWidth = WIDTH;
  vp.DHeight = HEIGHT;
  vp.RasInfo = &ri;
  cm = GetColorMap(4);
  colorpalette = (UWORD *) cm->ColorTable;
  for (i = 0; i < 4; i++) {
    *colorpalette++ = colortable[i];
  }
  vp.ColorMap = cm;
  for (i = 0; i < DEPTH; i++) {
    b.Planes[i] = (PLANEPTR) AllocRaster(WIDTH, HEIGHT);
    if (b.Planes[i] == NULL) exit(NOT_ENOUGH_MEMORY);
  }
  MakeVPort(&v, &vp);
  MrgCop(&v);
  for (i = 0; i < 2; i++) {
    displaymem = (UBYTE *) b.Planes[i];
    BltClear(displaymem, RASSIZE(WIDTH, HEIGHT), 0);
  }
  LoadView(&v);
  for (n = 1; n < 4; n++) {
    for (k = 0; k < 2; k++) {
      displaymem = b.Planes[k] + boxoffsets[n - 1];
      DrawFilledBox(n, k);
    }
  }
  Delay(50 * 10);
  LoadView(oldview);
  FreeMemory();
  CloseLibrary((struct Library *) GfxBase);
  return 0;
}

int FreeMemory()
{
  for (i = 0; i < DEPTH; i++) {
    FreeRaster(b.Planes[i], WIDTH, HEIGHT);
  }
  FreeColorMap(cm);
  FreeVPortCopLists(&vp);
  FreeCprList(v.LOFCprList);
  return 0;
}

int DrawFilledBox(SHORT fillcolor, SHORT plane)
{
  UBYTE value;
  for (j = 0; j < 100; j++) {
    if ((fillcolor & (1 << plane)) != 0) value = 0xff;
    else value = 0;
    for (i = 0; i < 20; i++) *displaymem++ = value;
    displaymem += (b.BytesPerRow - 20);
  }
  return 0;
}
