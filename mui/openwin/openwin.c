/*
  openwin.c - minimal program to test MUI
  This code can be compiled with either VBCC or SAS/C
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <intuition/classes.h>
#include <utility/tagitem.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <libraries/mui.h>

#ifdef __SASC_60

#include <proto/muimaster.h>

#else /* VBCC */

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
    CloseLibrary(IntuitionBase);
    return 0;
  }
  return 1;
}

void Cleanup()
{
  if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
  if (MUIMasterBase) CloseLibrary((struct Library *) MUIMasterBase);
}

int main(int argc, char **argv)
{
  Object *app = NULL, *window = NULL;
  if (!Init())
  {
    printf("Cannot open libs\n");
    return 0;
  }

  app = MUI_NewObject(MUIC_Application,
                      MUIA_Application_Title, "Mini MUI App",
                      MUIA_Application_Version, "1.0",
                      MUIA_Application_Copyright, "©2013, Wei-ju Wu",
                      MUIA_Application_Author, "Wei-ju Wu",
                      MUIA_Application_Description, "Minimal MUI Demo",
                      MUIA_Application_Base, "Application Base",
                      MUIA_Application_Window, window = MUI_NewObject(MUIC_Window,
                        MUIA_Window_Title, "My first MUI window",
                        MUIA_Window_ID, MAKE_ID('A', 'P', 'P', 'W'),
                        MUIA_Window_RootObject, MUI_NewObject(MUIC_Group,
                          MUIA_Group_Child, MUI_NewObject(MUIC_Text,
                          MUIA_Frame, MUIV_Frame_Text,
                          MUIA_Background, MUII_TextBack,
                          MUIA_Text_Contents, "This is the first line.\nAnd this is the second",
                        TAG_DONE),
                      TAG_DONE),
                    TAG_DONE),
                  TAG_DONE);

  if (!app) {
    printf("Cannot create application.\n");
    return 0;
  }

  DoMethod(window, MUIM_Notify, MUIA_Window_CloseRequest,TRUE,
           app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
  SetAttrs(window, MUIA_Window_Open, TRUE, TAG_DONE);
  {
    ULONG sigs = 0;
    while (DoMethod(app, MUIM_Application_NewInput, &sigs) != MUIV_Application_ReturnID_Quit) {
      if (sigs) {
        sigs = Wait(sigs | SIGBREAKF_CTRL_C);
        if (sigs & SIGBREAKF_CTRL_C) break;
      }
    }
  }
  set((APTR) window, MUIA_Window_Open, FALSE);
  MUI_DisposeObject(app);
  Cleanup();
  return 0;
}
