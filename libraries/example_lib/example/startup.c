#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <exec/initializers.h>

#include <proto/exec.h>    // all other compilers
#include "compiler.h"

#include "include/example/examplebase.h"
#include "libfuncs.h"

extern ULONG L_OpenLibs(struct ExampleBase *ExampleBase);
extern void  L_CloseLibs(void);

struct ExampleBase *InitLib(register __a6 struct ExecBase    *sysbase,
                            register __a0 SEGLISTPTR          seglist,
                            register __d0 struct ExampleBase *exb);
struct ExampleBase *OpenLib(register __a6 struct ExampleBase *ExampleBase);
SEGLISTPTR CloseLib(register __a6 struct ExampleBase *ExampleBase);
SEGLISTPTR ExpungeLib(register __a6 struct ExampleBase *exb);
ULONG ExtFuncLib(void);

extern APTR FuncTab[];
extern DataTab;

struct InitTable                       /* do not change */
{
    ULONG              LibBaseSize;
    APTR              *FunctionTable;
    struct MyDataInit *DataTable;
    APTR               InitLibTable;
};

/* ----------------------------------------------------------------------------------------
   ! LibStart:
   !
   ! If someone tries to start a library as an executable, it must return (LONG) -1
   ! as result. That's what we are doing here.
   ---------------------------------------------------------------------------------------- */

LONG LibStart(void) { return -1; }

/* ----------------------------------------------------------------------------------------
   ! Function and Data Tables:
   !
   ! The function and data tables have been placed here for traditional reasons.
   ! Placing the RomTag structure before (-> LibInit.c) would also be a good idea,
   ! but it depends on whether you would like to keep the "version" stuff separately.
   ---------------------------------------------------------------------------------------- */

struct InitTable InitTab = {
    (ULONG)               sizeof(struct ExampleBase),
    (APTR              *) &FuncTab[0],
    (struct MyDataInit *) &DataTab,
    (APTR)                InitLib
};

APTR FuncTab[] = {
    (APTR) OpenLib,
    (APTR) CloseLib,
    (APTR) ExpungeLib,
    (APTR) ExtFuncLib,

    (APTR) EXF_TestRequest,  /* add your own functions here */
    (APTR) ((LONG)-1)
};

extern struct ExampleBase *ExampleBase;

/* ----------------------------------------------------------------------------------------
   ! InitLib:
   !
   ! This one is single-threaded by the Ramlib process. Theoretically you can do, what
   ! you like here, since you have full exclusive control over all the library code and data.
   ! But due to some bugs in Ramlib V37-40, you can easily cause a deadlock when opening
   ! certain libraries here (which open other libraries, that open other libraries, that...)
   !
   ---------------------------------------------------------------------------------------- */

struct ExampleBase *InitLib(register __a6 struct ExecBase      *sysbase,
                            register __a0 SEGLISTPTR            seglist,
                            register __d0 struct ExampleBase   *exb)
{
    ExampleBase = exb;
    ExampleBase->exb_SysBase = sysbase;
    ExampleBase->exb_SegList = seglist;

    if (L_OpenLibs(ExampleBase)) return ExampleBase;

    L_CloseLibs();

    {
        ULONG negsize, possize, fullsize;
        UBYTE *negptr = (UBYTE *) ExampleBase;

        negsize  = ExampleBase->exb_LibNode.lib_NegSize;
        possize  = ExampleBase->exb_LibNode.lib_PosSize;
        fullsize = negsize + possize;
        negptr  -= negsize;

        FreeMem(negptr, fullsize);
    }
    return NULL;
}

/* ----------------------------------------------------------------------------------------
   ! OpenLib:
   !
   ! This one is enclosed within a Forbid/Permit pair by Exec V37-40. Since a Wait() call
   ! would break this Forbid/Permit(), you are not allowed to start any operations that
   ! may cause a Wait() during their processing. It's possible, that future OS versions
   ! won't turn the multi-tasking off, but instead use semaphore protection for this
   ! function.
   !
   ! Currently you only can bypass this restriction by supplying your own semaphore
   ! mechanism.
   ---------------------------------------------------------------------------------------- */

struct ExampleBase *OpenLib(register __a6 struct ExampleBase *ExampleBase)
{
    ExampleBase->exb_LibNode.lib_OpenCnt++;
    ExampleBase->exb_LibNode.lib_Flags &= ~LIBF_DELEXP;
    return ExampleBase;
}

/* ----------------------------------------------------------------------------------------
   ! CloseLib:
   !
   ! This one is enclosed within a Forbid/Permit pair by Exec V37-40. Since a Wait() call
   ! would break this Forbid/Permit(), you are not allowed to start any operations that
   ! may cause a Wait() during their processing. It's possible, that future OS versions
   ! won't turn the multi-tasking off, but instead use semaphore protection for this
   ! function.
   !
   ! Currently you only can bypass this restriction by supplying your own semaphore
   ! mechanism.
   ---------------------------------------------------------------------------------------- */

SEGLISTPTR CloseLib(register __a6 struct ExampleBase *ExampleBase)
{
    ExampleBase->exb_LibNode.lib_OpenCnt--;

    if (!ExampleBase->exb_LibNode.lib_OpenCnt) {
        if (ExampleBase->exb_LibNode.lib_Flags & LIBF_DELEXP) {
            return ExpungeLib(ExampleBase);
        }
    }
    return NULL;
}

/* ----------------------------------------------------------------------------------------
   ! ExpungeLib:
   !
   ! This one is enclosed within a Forbid/Permit pair by Exec V37-40. Since a Wait() call
   ! would break this Forbid/Permit(), you are not allowed to start any operations that
   ! may cause a Wait() during their processing. It's possible, that future OS versions
   ! won't turn the multi-tasking off, but instead use semaphore protection for this
   ! function.
   !
   ! Currently you only could bypass this restriction by supplying your own semaphore
   ! mechanism - but since expunging can't be done twice, one should avoid it here.
   ---------------------------------------------------------------------------------------- */

SEGLISTPTR ExpungeLib(register __a6 struct ExampleBase *exb)
{
    struct ExampleBase *ExampleBase = exb;
    SEGLISTPTR seglist;

    if (!ExampleBase->exb_LibNode.lib_OpenCnt) {
        ULONG negsize, possize, fullsize;
        UBYTE *negptr = (UBYTE *) ExampleBase;

        seglist = ExampleBase->exb_SegList;
        Remove((struct Node *)ExampleBase);
        L_CloseLibs();

        negsize  = ExampleBase->exb_LibNode.lib_NegSize;
        possize  = ExampleBase->exb_LibNode.lib_PosSize;
        fullsize = negsize + possize;
        negptr  -= negsize;

        FreeMem(negptr, fullsize);
        return seglist;
    }

    ExampleBase->exb_LibNode.lib_Flags |= LIBF_DELEXP;
    return NULL;
}

/* ----------------------------------------------------------------------------------------
   ! ExtFunct:
   !
   ! This one is enclosed within a Forbid/Permit pair by Exec V37-40. Since a Wait() call
   ! would break this Forbid/Permit(), you are not allowed to start any operations that
   ! may cause a Wait() during their processing. It's possible, that future OS versions
   ! won't turn the multi-tasking off, but instead use semaphore protection for this
   ! function.
   !
   ! Currently you only can bypass this restriction by supplying your own semaphore
   ! mechanism - but since this function currently is unused, you should not touch
   ! it, either.
   ---------------------------------------------------------------------------------------- */

ULONG ExtFuncLib(void) { return NULL; }

struct ExampleBase *ExampleBase = NULL;
