#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <exec/initializers.h>

#include <proto/exec.h>
#include "compiler.h"
#include "include/example/examplebase.h"

ULONG L_OpenLibs(struct ExampleBase *ExampleBase);
void  L_CloseLibs(void);

struct ExecBase      *SysBase       = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase       *GfxBase       = NULL;

#define VERSION  37
#define REVISION 32

#define EXLIBNAME "example"
#define EXLIBVER  " 37.32 (2.3.99)"

char ExLibName [] = EXLIBNAME ".library";
char ExLibID   [] = EXLIBNAME EXLIBVER;
char Copyright [] = "(C)opyright 1996-99 by Andreas R. Kleinert. All rights reserved.";
char VERSTRING [] = "\0$VER: " EXLIBNAME EXLIBVER;

/* ----------------------------------------------------------------------------------------
   ! ROMTag and Library initialization structure:
   !
   ! Below you find the ROMTag, which is the most important "magic" part of a library
   ! (as for any other resident module). You should not need to modify any of the
   ! structures directly, since all the data is referenced from constants from somewhere else.
   !
   ! You may place the ROMTag directly after the LibStart (-> StartUp.c) function as well.
   !
   ! Note, that the data initialization structure may be somewhat redundant - it's
   ! for demonstration purposes.
   !
   ! EndResident can be placed somewhere else - but it must follow the ROMTag and
   ! it must not be placed in a different SECTION.
   ---------------------------------------------------------------------------------------- */

extern ULONG InitTab[];
extern APTR EndResident; /* below */

struct Resident ROMTag =     /* do not change */
{
    RTC_MATCHWORD,
    &ROMTag,
    &EndResident,
    RTF_AUTOINIT,
    VERSION,
    NT_LIBRARY,
    0,
    &ExLibName[0],
    &ExLibID[0],
    &InitTab[0]
};

APTR EndResident;

struct MyDataInit                      /* do not change */
{
    UWORD ln_Type_Init;      UWORD ln_Type_Offset;      UWORD ln_Type_Content;
    UBYTE ln_Name_Init;      UBYTE ln_Name_Offset;      ULONG ln_Name_Content;
    UWORD lib_Flags_Init;    UWORD lib_Flags_Offset;    UWORD lib_Flags_Content;
    UWORD lib_Version_Init;  UWORD lib_Version_Offset;  UWORD lib_Version_Content;
    UWORD lib_Revision_Init; UWORD lib_Revision_Offset; UWORD lib_Revision_Content;
    UBYTE lib_IdString_Init; UBYTE lib_IdString_Offset; ULONG lib_IdString_Content;
    ULONG ENDMARK;
} DataTab = {
    0xe000, 8, NT_LIBRARY,
    0x0080, 10, (ULONG) &ExLibName[0],
    0xe000, 14, LIBF_SUMUSED|LIBF_CHANGED,
    0xd000, 20, VERSION,
    0xd000, 22, REVISION,
    0x80,   24, (ULONG) &ExLibID[0],
    (ULONG) 0
};

/* ----------------------------------------------------------------------------------------
   ! L_OpenLibs:
   !
   ! Since this one is called by InitLib, libraries not shareable between Processes or
   ! libraries messing with RamLib (deadlock and crash) may not be opened here.
   !
   ! You may bypass this by calling this function fromout LibOpen, but then you will
   ! have to a) protect it by a semaphore and b) make sure, that libraries are only
   ! opened once (when using global library bases).
   ---------------------------------------------------------------------------------------- */

ULONG L_OpenLibs(struct ExampleBase *ExampleBase)
{
    SysBase = (*((struct ExecBase **) 4));

    IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 37);
    if(!IntuitionBase) return FALSE;

    GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 37);
    if (!GfxBase) return FALSE;

    ExampleBase->exb_IntuitionBase = IntuitionBase;
    ExampleBase->exb_GfxBase       = GfxBase;
    ExampleBase->exb_SysBase       = SysBase;

    return TRUE;
}

/* ----------------------------------------------------------------------------------------
   ! L_CloseLibs:
   !
   ! This one by default is called by ExpungeLib, which only can take place once
   ! and thus per definition is single-threaded.
   !
   ! When calling this fromout LibClose instead, you will have to protect it by a
   ! semaphore, since you don't know whether a given CloseLibrary(foobase) may cause a Wait().
   ! Additionally, there should be protection, that a library won't be closed twice.
   ---------------------------------------------------------------------------------------- */

void L_CloseLibs(void)
{
    if (GfxBase)       CloseLibrary((struct Library *) GfxBase);
    if (IntuitionBase) CloseLibrary((struct Library *) IntuitionBase);
}
