/*
  main.c - MUI application to play with the blitter
  This code was written to be compiled with either VBCC
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <intuition/classes.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/icclass.h>

#include <graphics/gfx.h>

#include <libraries/gadtools.h>
#include <utility/tagitem.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>

#include <libraries/mui.h>
#include <inline/muimaster_protos.h>

#include "mui_util.h"
#include "canvas.h"

/*
 This is one of the magic variables of an Amiga program:
 the address of this structure is set to the base
 address of the custom registers and the structure corresponds
 to the memory layout of the custom registers, so we reads
 and writes are direct accesses to the memory-mapped custom
 chip registers.
*/
extern struct Custom custom;

/*
  Make sure that these pointers to the library bases are set to a valid values.
  The BOOPSI/MUI/Intuition/Graphics calls expect it and will crash the
  application in spectacular ways.
 */
struct Library *IntuitionBase = NULL, *MUIMasterBase = NULL,
  *GfxBase = NULL, *UtilityBase = NULL;

BOOL Init()
{
  if (!(IntuitionBase = OpenLibrary("intuition.library", 39))) return 0;
  if (!(GfxBase = OpenLibrary("graphics.library", 39))) {
    CloseLibrary(IntuitionBase);
    return 0;
  }
  if (!(UtilityBase = OpenLibrary("utility.library", 39))) {
    CloseLibrary(GfxBase);
    CloseLibrary(IntuitionBase);
    return 0;
  }
  if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, 19))) {
    CloseLibrary(UtilityBase);
    CloseLibrary(GfxBase);
    CloseLibrary(IntuitionBase);
    return 0;
  }
  return 1;
}

void Cleanup()
{
  if (MUIMasterBase) CloseLibrary(MUIMasterBase);
  if (UtilityBase) CloseLibrary(UtilityBase);
  if (GfxBase) CloseLibrary(GfxBase);
  if (IntuitionBase) CloseLibrary(IntuitionBase);
}

/* For simplicity, I just have all the controls as a module-global
   variable, so the handlers can access them */
Object *strAddrA, *strModA, *strShiftA, *strDataA, *strAFWM, *strALWM;
Object *strAddrB, *strModB, *strShiftB, *strDataB;
Object *strAddrC, *strModC, *strDataC;
Object *strAddrD, *strModD;
Object *window, *canvas, *strWidth, *strHeight, *strFunction;

LONG selectDMAA = 0, selectDMAB = 0, selectDMAC = 0, selectDMAD = 0;

/* We have a select func per DMA group - it's cleaner */
ULONG SelectDMAChannelAFunc(__reg("a0") struct Hook *h,
                            __reg("a2") Object *app,
                            __reg("a1") Object **pObj)
{
  get(*pObj, MUIA_Selected, &selectDMAA);
  set(strAddrA, MUIA_Disabled, !selectDMAA);
  set(strModA, MUIA_Disabled, !selectDMAA);
  set(strDataA, MUIA_Disabled, !selectDMAA);
  set(strShiftA, MUIA_Disabled, !selectDMAA);
  set(strAFWM, MUIA_Disabled, !selectDMAA);
  set(strALWM, MUIA_Disabled, !selectDMAA);
  return 0;
}
ULONG SelectDMAChannelBFunc(__reg("a0") struct Hook *h,
                            __reg("a2") Object *app,
                            __reg("a1") Object **pObj)
{
  get(*pObj, MUIA_Selected, &selectDMAB);
  set(strAddrB, MUIA_Disabled, !selectDMAB);
  set(strModB, MUIA_Disabled, !selectDMAB);
  set(strDataB, MUIA_Disabled, !selectDMAB);
  set(strShiftB, MUIA_Disabled, !selectDMAB);
  return 0;
}
ULONG SelectDMAChannelCFunc(__reg("a0") struct Hook *h,
                            __reg("a2") Object *app,
                            __reg("a1") Object **pObj)
{
  get(*pObj, MUIA_Selected, &selectDMAC);
  set(strAddrC, MUIA_Disabled, !selectDMAC);
  set(strModC, MUIA_Disabled, !selectDMAC);
  set(strDataC, MUIA_Disabled, !selectDMAC);
  return 0;
}
ULONG SelectDMAChannelDFunc(__reg("a0") struct Hook *h,
                            __reg("a2") Object *app,
                            __reg("a1") Object **pObj)
{
  get(*pObj, MUIA_Selected, &selectDMAD);
  set(strAddrD, MUIA_Disabled, !selectDMAD);
  set(strModD, MUIA_Disabled, !selectDMAD);
  return 0;
}

BOOL IsBlitSafe(Object *app, Object *win)
{
  int flags = 0;
  if (!selectDMAD) return TRUE; /* If destination  is off, why bother ? */
  /* Now we need to check the sizes for validity */
  //MUI_Request(app, win, flags, "Unsafe Blit", "OK",
  //            "Can not perform blit");
  return TRUE;
}

void DoBlit(Object *app)
{
  char *str;
  LONG functionNum, blitWidth, blitHeight, activeChannels = 0;
  LONG addrA, modA, dataA, shiftA, addrB, modB, dataB, shiftB,
    addrC, modC, dataC, addrD, modD;
  PLANEPTR bitplane;
  struct BitMap *bitmap;
  

  get(strAddrA,    MUIA_String_Contents, (LONG *) &str);
  get(canvas,      CANVASA_BITMAP,       (ULONG *) &bitmap);
  get(strFunction, MUIA_String_Integer,  (LONG *) &functionNum);
  get(strWidth,    MUIA_String_Integer,  (LONG *) &blitWidth);
  get(strHeight,   MUIA_String_Integer,  (LONG *) &blitHeight);
  bitplane = bitmap->Planes[0];

  get(strAddrA,  MUIA_String_Integer, (LONG *) &addrA);
  get(strModA,   MUIA_String_Integer, (LONG *) &modA);
  get(strDataA,  MUIA_String_Integer, (LONG *) &dataA);
  get(strShiftA, MUIA_String_Integer, (LONG *) &shiftA);

  get(strAddrB,  MUIA_String_Integer, (LONG *) &addrB);
  get(strModB,   MUIA_String_Integer, (LONG *) &modB);
  get(strDataB,  MUIA_String_Integer, (LONG *) &dataB);
  get(strShiftB, MUIA_String_Integer, (LONG *) &shiftB);

  get(strAddrC,  MUIA_String_Integer, (LONG *) &addrC);
  get(strModC,   MUIA_String_Integer, (LONG *) &modC);
  get(strDataC,  MUIA_String_Integer, (LONG *) &dataC);

  get(strAddrD,  MUIA_String_Integer, (LONG *) &addrD);
  get(strModD,   MUIA_String_Integer, (LONG *) &modD);

  printf("you pushed me ! '%s', function: %d\n", str, (int) functionNum);

  /* do the blit action */
  if (selectDMAA) activeChannels |= BC0F_SRCA;
  if (selectDMAB) activeChannels |= BC0F_SRCB;
  if (selectDMAC) activeChannels |= BC0F_SRCC;
  if (selectDMAD) activeChannels |= BC0F_DEST;

  if (IsBlitSafe(app, window)) {
    OwnBlitter();
    WaitBlit();

    custom.bltcon0 = activeChannels | (functionNum & 0xff);
    custom.bltcon1 = 0x0000;

    custom.bltadat = dataA;
    custom.bltbdat = dataB;
    custom.bltcdat = dataC;
  
    custom.bltamod = modA;
    custom.bltbmod = modB;
    custom.bltcmod = modC;
    custom.bltdmod = modD;

    custom.bltafwm = 0xffff;
    custom.bltalwm = 0xffff;

    custom.bltapt = bitplane + addrA;
    custom.bltbpt = bitplane + addrB;
    custom.bltcpt = bitplane + addrC;
    custom.bltdpt = bitplane + addrD;

    custom.bltsize = (blitHeight << 6) | blitWidth; /* max 20 x 32  (320x32 pixel ) */
    DisownBlitter();
    MUI_Redraw(canvas, MADF_DRAWUPDATE);
  }
}

#define ACCEPT_POSITIVE_NUMS  "0123456789"
#define ACCEPT_BINARY        "01%"
#define ACCEPT_HEX           "0123456789ABCDEFXabcdefx"

/* Menu definition */
/* Menu ids */
enum {
  MENU_NONE, MENU_PROJECT, MENU_ABOUT, MENU_QUIT,
  MENU_EXECUTE, MENU_GO,
  /* AREXX commands */
  ID_PING
};

/* Menu structure */
static struct NewMenu menudata[] = {
  {NM_TITLE, "Project", 0, 0, 0, (APTR) MENU_PROJECT},
  {NM_ITEM, "About", "?", 0, 0, (APTR) MENU_ABOUT},
  {NM_ITEM, NM_BARLABEL, 0, 0, 0, (APTR) MENU_NONE},
  {NM_ITEM, "Quit", "Q", 0, 0, (APTR) MENU_QUIT},

  {NM_TITLE, "Execute", 0, 0, 0, (APTR) MENU_EXECUTE},
  {NM_ITEM, "Go !", "G", 0, 0, (APTR) MENU_GO},

  {NM_END, NULL, 0, 0, 0, NULL }
};

/* send simple commands with 
   address blitplay.1 <command>
*/
static struct MUI_Command arexx_commands[] = {
  {"ping", MC_TEMPLATE_ID, ID_PING, NULL},
  {NULL, NULL, 0, NULL}
};

static char *pageTitles[] = {"Copy/Block Mode", "Line Mode", NULL };

int main(int argc, char **argv)
{
  int plane_size;
  struct MUI_CustomClass *canvasClass;

  Object *app = NULL, *xdisplay = NULL, *ydisplay;
  Object *cmEnableDMA[4];
  struct Hook SelectDMAChannelAHook = { {0, 0}, SelectDMAChannelAFunc, NULL, NULL };
  struct Hook SelectDMAChannelBHook = { {0, 0}, SelectDMAChannelBFunc, NULL, NULL };
  struct Hook SelectDMAChannelCHook = { {0, 0}, SelectDMAChannelCFunc, NULL, NULL };
  struct Hook SelectDMAChannelDHook = { {0, 0}, SelectDMAChannelDFunc, NULL, NULL };

  if (!Init())
  {
    printf("Cannot open libs\n");
    return 0;
  }

  printf("custom base: %04x\n", (int) &custom);

  /* override the BitMap class to support drawing with the mouse */
  if (!(canvasClass = CreateCanvasClass())) {
    Cleanup();
    return 0;
  }
  xdisplay = StringObject,
    MUIA_String_Integer, 0,
    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
    MUIA_String_Format, MUIV_String_Format_Right,
  End;
  ydisplay = StringObject,
    MUIA_String_Integer, 0,
    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
    MUIA_String_Format, MUIV_String_Format_Right,
  End;

  app = ApplicationObject,
          MUIA_Application_Title, "BlitterPlayground",
          MUIA_Application_Version, "1.0",
          MUIA_Application_Copyright, "©2013, Wei-ju Wu",
          MUIA_Application_Author, "Wei-ju Wu",
          MUIA_Application_Description, "A Blitter Toy",
          MUIA_Application_Base, "blitplay",
          MUIA_Application_Commands, arexx_commands,
          SubWindow, window = WindowObject,
            MUIA_Window_Title, "Blitter Playground 1.0",
            MUIA_Window_ID, MAKE_ID('A', 'P', 'P', 'W'),
            MUIA_Window_AppWindow, TRUE,
            MUIA_Background, MUII_WindowBack,
            MUIA_Window_Menustrip, MUI_MakeObject(MUIO_MenustripNM, menudata, 0),
            MUIA_Window_SizeGadget, FALSE,
            WindowContents, VGroup,
              Child, HGroup,
                Child, canvas = NewObject(canvasClass->mcc_Class, NULL,
                                          CANVASA_XDISPLAY, xdisplay,
                                          CANVASA_YDISPLAY, ydisplay,
                                          MUIA_Frame, MUIV_Frame_Text,
                                          TAG_DONE),

                Child, ColGroup(2),
                  Child, Label("Address (M+): "),
                  Child, xdisplay,
                  Child, Label("Shift: "),
                  Child, ydisplay,
                End,

                /* define a frame around the bitmap */
                MUIA_Frame, MUIV_Frame_Group,
              End,
              Child, HGroup,
                Child, Label("Width"),
                Child, strWidth = StringObject,
                  StringFrame,
                  MUIA_CycleChain, 1,
                  MUIA_String_Integer, 0,
                  MUIA_String_MaxLen, 3,
                  MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                  MUIA_String_Format, MUIV_String_Format_Right,
                  MUIA_ShortHelp, "Width of blit in 16-bit words",
                  MUIA_MaxWidth, 60,
                End,
                Child, Label("Height"),
                Child, strHeight = StringObject,
                  StringFrame,
                  MUIA_CycleChain, 1,
                  MUIA_String_Integer, 0,
                  MUIA_String_MaxLen, 3,
                  MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                  MUIA_String_Format, MUIV_String_Format_Right,
                  MUIA_ShortHelp, "Height of blit in pixels",
                  MUIA_MaxWidth, 60,
                End,
                Child, Label("Function"),
                Child, strFunction = StringObject,
                  StringFrame,
                  MUIA_CycleChain, 1,
                  MUIA_String_Integer, 0,
                  MUIA_String_MaxLen, 4,
                  MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                  MUIA_String_Format, MUIV_String_Format_Right,
                  MUIA_ShortHelp, "Function number",
                  MUIA_MaxWidth, 60,
                End,
              End,
              Child, RegisterGroup(pageTitles),
                MUIA_Register_Frame, TRUE,
              /* DMA channel setup */
              Child, VGroup,
                MUIA_Frame, MUIV_Frame_Group,
                MUIA_FrameTitle, "DMA Channels",

                Child, ColGroup(6),
 
                  /* Headers */
                  Child, Label("Ch"),
                  Child, Label("Use"),
                  Child, Label("Address"),
                  Child, Label("Modulo"),
                  Child, Label("Data"),
                  Child, Label("Shift"),

                  /* Channel A inputs */
                  Child, Label("A"),
                  Child, cmEnableDMA[0] = CheckMark(FALSE),
                  Child, strAddrA = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Source address DMA channel A",
                  End,
                  Child, strModA = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Modulo DMA channel A",
                  End,
                  Child, strDataA = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Data register DMA channel A",
                  End,
                  Child, strShiftA = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_MaxLen, 3,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Shift register DMA channel A",
                  End,

                  /* Channel B inputs */
                  Child, Label("B"),
                  Child, cmEnableDMA[1] = CheckMark(FALSE),
                  Child, strAddrB = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Source address DMA channel B",
                  End,
                  Child, strModB = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                  End,
                  Child, strDataB = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Modulo DMA channel B",
                  End,
                  Child, strShiftB = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_MaxLen, 3,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Data register DMA channel B",
                  End,

                  /* Channel C inputs */
                  Child, Label("C"),
                  Child, cmEnableDMA[2] = CheckMark(FALSE),
                  Child, strAddrC = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Source address DMA channel C",
                  End,
                  Child, strModC = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Modulo DMA channel C",
                  End,
                  Child, strDataC = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Data register DMA channel C",
                  End,
                  Child, RectangleObject,
                    MUIA_MaxWidth, 40,
                  End,

                  /* Channel D inputs */
                  Child, Label("D"),
                  Child, cmEnableDMA[3] = CheckMark(FALSE),
                  Child, strAddrD = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Source address DMA channel D",
                  End,
                  Child, strModD = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Integer, 0,
                    MUIA_String_Accept, ACCEPT_POSITIVE_NUMS,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Modulo DMA channel D",
                    MUIA_MaxWidth, 50,
                  End,
                  Child, RectangleObject,
                    MUIA_MaxWidth, 50,
                  End,
                  Child, RectangleObject,
                    MUIA_MaxWidth, 50,
                  End,

                End, /* ColGroup */

                Child, HGroup,
                  Child, Label("Ch. A FWM"),
                  Child, strAFWM = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Contents, "%1111111111111111",
                    MUIA_String_MaxLen, 18,
                    MUIA_String_Accept, ACCEPT_BINARY,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "First word mask channel A",
                  End,

                  Child, Label("Ch. A LWM"),
                  Child, strALWM = StringObject,
                    StringFrame,
                    MUIA_CycleChain, 1,
                    MUIA_String_Contents, "%1111111111111111",
                    MUIA_String_MaxLen, 18,
                    MUIA_String_Accept, ACCEPT_BINARY,
                    MUIA_String_Format, MUIV_String_Format_Right,
                    MUIA_Disabled, TRUE,
                    MUIA_ShortHelp, "Last word mask channel A",
                  End,
                End,

              End, /* DMA channel setup */
    End, /* Register */
            End,
          End,
        End;
  if (!app) {
    printf("Cannot create application.\n");
    return 0;
  }

  DoMethod(window, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
           app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
  DoMethod(cmEnableDMA[0], MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
           app, 3, MUIM_CallHook, &SelectDMAChannelAHook, cmEnableDMA[0]);
  DoMethod(cmEnableDMA[1], MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
           app, 3, MUIM_CallHook, &SelectDMAChannelBHook, cmEnableDMA[1]);
  DoMethod(cmEnableDMA[2], MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
           app, 3, MUIM_CallHook, &SelectDMAChannelCHook, cmEnableDMA[2]);
  DoMethod(cmEnableDMA[3], MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
           app, 3, MUIM_CallHook, &SelectDMAChannelDHook, cmEnableDMA[3]);

  SetAttrs(window, MUIA_Window_Open, TRUE, TAG_DONE);
  {
    BOOL running = TRUE;
    ULONG sigs = 0, id;

    while (running) {
      id = DoMethod(app, MUIM_Application_NewInput, &sigs);
      switch(id)
      {
        case MENU_QUIT:
        case MUIV_Application_ReturnID_Quit:
          running = FALSE;
          break;
        case MENU_ABOUT:
          MUI_Request(app, window, 0, NULL, "Close", "Blitter Playground 1.0");
          break;
        case MENU_GO:
          DoBlit(app);
          break;
        case ID_PING:
          printf("Ping !\n");
          break;
        default:
          break;
      }
      if (running && sigs) {
        sigs = Wait(sigs | SIGBREAKF_CTRL_C);
        if (sigs & SIGBREAKF_CTRL_C) break;
      }
    }
  }
  set((APTR) window, MUIA_Window_Open, FALSE);
  MUI_DisposeObject(app);
  MUI_DeleteCustomClass(canvasClass);

  Cleanup();
  return 0;
}
