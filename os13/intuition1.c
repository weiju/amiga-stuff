#include <stdio.h>
#include <stdlib.h>
#include <exec/types.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#define INTUITION_REV 33L
#define GRAPHICS_REV 33L

struct TextAttr MyFont = {
  "topaz.font",
  TOPAZ_SIXTY,
  FS_NORMAL,
  FPF_ROMFONT
};

struct Library *IntuitionBase;
struct Library *GfxBase;

struct NewScreen NewScreen = {
  0, 0, 320, 200, 2, 0, 1, 0, CUSTOMSCREEN, &MyFont,
  "My Own Screen", NULL, NULL,
};

int main(int argc, char **argv)
{
  struct Screen *Screen;
  struct NewWindow NewWindow;
  struct Window *Window;
  LONG i;

  IntuitionBase = OpenLibrary("intuition.library", INTUITION_REV);
  if (IntuitionBase == NULL) {
    printf("Intuition could not be opened\n");
    exit(FALSE);
  }
  GfxBase = OpenLibrary("graphics.library", GRAPHICS_REV);
  if (GfxBase == NULL) {
    printf("Graphics could not be opened\n");
    exit(FALSE);
  }
  if ((Screen = (struct Screen *) OpenScreen(&NewScreen)) == NULL) {
    printf("could not open screen\n");
    exit(FALSE);
  }

  NewWindow.LeftEdge = 20;
  NewWindow.TopEdge = 20;
  NewWindow.Width = 300;
  NewWindow.Height = 100;
  NewWindow.DetailPen = 0;
  NewWindow.BlockPen = 1;
  NewWindow.Title = "A Simple Window";
  NewWindow.Flags = WINDOWCLOSE | SMART_REFRESH | ACTIVATE |
    WINDOWSIZING | WINDOWDRAG | WINDOWDEPTH | NOCAREREFRESH;
  NewWindow.IDCMPFlags = CLOSEWINDOW;
  NewWindow.Type = CUSTOMSCREEN;
  NewWindow.FirstGadget = NULL;
  NewWindow.CheckMark = NULL;
  NewWindow.Screen = Screen;
  NewWindow.BitMap = NULL;
  NewWindow.MinWidth = 100;
  NewWindow.MinHeight = 25;
  NewWindow.MaxWidth = 640;
  NewWindow.MaxHeight = 200;

  if ((Window = (struct Window *) OpenWindow(&NewWindow)) == NULL) {
    printf("could not open window");
    exit(FALSE);
  }
  Move(Window->RPort, 20, 20);
  Text(Window->RPort, "Hello World", 11);

  Wait(1 << Window->UserPort->mp_SigBit);
  CloseWindow(Window);
  CloseScreen(Screen);
  CloseLibrary(GfxBase);
  CloseLibrary(IntuitionBase);
  return 0;
}
