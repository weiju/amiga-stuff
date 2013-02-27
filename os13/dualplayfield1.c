/**********************************************************************
 * Dual Playfield Demo
 * ROM Kernel Reference Manual - Libraries and Devices
 **********************************************************************/
#include <exec/types.h>
#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <exec/exec.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include <stdlib.h>


#define DEPTH 2
#define WIDTH 320
#define HEIGHT 200
#define NOT_ENOUGH_MEMORY -1000

struct View v;
struct ViewPort vp;
struct ColorMap *cm;
struct RasInfo ri;
struct BitMap b;

struct RasInfo ri2;
struct BitMap b2;

short i, j, k, n;
struct ColorMap *GetColorMap();
struct GfxBase *GfxBase;

USHORT colortable[] = {
  0x000, 0xf00, 0x0f0, 0x00f,
  0, 0, 0, 0,
  0, 0x495, 0x62a, 0xf9c
};

UWORD *colorpalette;
struct RastPort rp, rp2;
struct View *oldview;

int FreeMemory();

int main(int argc, char **argv)
{
  GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 0);
  if (GfxBase == NULL) exit(1);

  InitView(&v);
  v.ViewPort = &vp;
  InitVPort(&vp);

  vp.DWidth = WIDTH;
  vp.DHeight = HEIGHT;
  vp.RasInfo = &ri;
  vp.Modes = DUALPF | PFBA;

  InitBitMap(&b, DEPTH, WIDTH, HEIGHT);
  ri.BitMap = &b;
  ri.RxOffset = 0;
  ri.RyOffset = 0;

  InitBitMap(&b2, DEPTH, WIDTH, HEIGHT);
  ri.Next = &ri2;
  ri2.BitMap = &b2;
  ri2.RxOffset = 0;
  ri2.RyOffset = 0;
  ri2.Next = NULL;

  cm = GetColorMap(12);
  colorpalette = cm->ColorTable;
  for (i = 0; i < 12; i++) *colorpalette++ = colortable[i];
  vp.ColorMap = cm;

  for (i = 0; i < DEPTH; i++) {
    b.Planes[i] = (PLANEPTR) AllocRaster(WIDTH, HEIGHT);
    if (b.Planes[i] == NULL) exit(NOT_ENOUGH_MEMORY);
    b2.Planes[i] = (PLANEPTR) AllocRaster(WIDTH, HEIGHT);
    if (b2.Planes[i] == NULL) exit(NOT_ENOUGH_MEMORY);
  }
  InitRastPort(&rp);
  InitRastPort(&rp2);
  rp.BitMap = &b;
  rp2.BitMap = &b2;
  MakeVPort(&v, &vp);
  MrgCop(&v);

  SetRast(&rp, 0);
  SetRast(&rp2, 0);

  oldview = GfxBase->ActiView;
  LoadView(&v);

  /* draw to first playfield */
  SetAPen(&rp, 1);
  RectFill(&rp, 20, 20, 200, 100);
  SetAPen(&rp, 2);
  RectFill(&rp, 40, 40, 220, 120);
  SetAPen(&rp, 3);
  RectFill(&rp, 60, 60, 240, 140);

  /* draw to second playfield */
  SetAPen(&rp2, 1);
  RectFill(&rp2, 50, 90, 245, 180);
  SetAPen(&rp2, 2);
  RectFill(&rp2, 70, 70, 265, 160);
  SetAPen(&rp2, 3);
  RectFill(&rp2, 90, 10, 285, 148);

  /* Poke some holes in the playfield */
  SetAPen(&rp2, 0);
  RectFill(&rp2, 110, 15, 130, 175);
  RectFill(&rp2, 175, 15, 200, 175);
  Delay(300);
  LoadView(oldview);
  WaitTOF();
  FreeMemory();
  CloseLibrary((struct Library *) GfxBase);

  return 0;
}

int FreeMemory()
{
  for (i = 0; i < DEPTH; i++) {
    FreeRaster(b.Planes[i], WIDTH, HEIGHT);
    FreeRaster(b2.Planes[i], WIDTH, HEIGHT);
  }
  FreeColorMap(cm);
  FreeVPortCopLists(&vp);
  FreeCprList(v.LOFCprList);
  return 0;
}
