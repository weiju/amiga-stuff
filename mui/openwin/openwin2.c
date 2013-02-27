/*
 openwin2.c - Version of openwin.c that uses the macros in mui.h
 This should be compilable both for SAS/C and VBCC
 */
#include <stdio.h>

#ifdef __SASC_60

#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/exec.h>
#include <libraries/mui.h>
#include <proto/muimaster.h>

#else /* VBCC */

#include <intuition/classes.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <libraries/mui.h>
#include <inline/muimaster_protos.h>

#endif

#include "mui_util.h"

/*
  Make sure IntuitionBase is set to a valid value. The BOOPSI and MUI calls
  expect it.
 */
struct Library *IntuitionBase = NULL, *MUIMasterBase = NULL;

BOOL Init()
{
  if (!(IntuitionBase = OpenLibrary("intuition.library", 39))) return 0;
  if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, 19))) {
    CloseLibrary((struct Library *)IntuitionBase);
    return 0;
  }
  return 1;
}

void Cleanup(void)
{
  if (IntuitionBase) CloseLibrary(IntuitionBase);
  if (MUIMasterBase) CloseLibrary(MUIMasterBase);
}

int main(int argc,char *argv[])
{
  APTR app, window;

  if (!Init()) {
    printf("Initialization failed\n");
    return 0;
  }

  app = ApplicationObject,
    MUIA_Application_Title  , "Mini MUI App 2",
    MUIA_Application_Version , "1.0",
    MUIA_Application_Copyright , "©2013, Wei-ju Wu",
    MUIA_Application_Author  , "Wei-ju Wu",
    MUIA_Application_Description, "Description",
    MUIA_Application_Base  , "Application Base",

    SubWindow, window = WindowObject,
      MUIA_Window_Title, "2nd MUI Window",
      MUIA_Window_ID , MAKE_ID('A','P','P','W'),
      WindowContents, VGroup,
        Child, TextObject,
          TextFrame,
          MUIA_Background, MUII_TextBack,
          MUIA_Text_Contents, "\33c This is the centered first line.\nThis is the second line.",
          End,
        End,
      End,
    End;

  if (!app) {
    printf("Could not create application object.\n");
    return 0;
  }

  DoMethod(window, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
           app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
  set(window, MUIA_Window_Open, TRUE);

  {
    ULONG sigs = 0;

    while (DoMethod(app, MUIM_Application_NewInput, &sigs) != MUIV_Application_ReturnID_Quit) {
      if (sigs) {
        sigs = Wait(sigs | SIGBREAKF_CTRL_C);
        if (sigs & SIGBREAKF_CTRL_C) break;
      }
    }
  }
  set(window, MUIA_Window_Open, FALSE);
  MUI_DisposeObject(app);
  Cleanup();
}
