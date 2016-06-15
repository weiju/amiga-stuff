/*
**      $VER: examplebase.h 37.30 (7.3.98)
**
**      definition of ExampleBase
**
**      (C) Copyright 1996-98 Andreas R. Kleinert
**      All Rights Reserved.
*/

#ifndef EXAMPLE_EXAMPLEBASE_H
#define EXAMPLE_EXAMPLEBASE_H

#ifdef   __MAXON__
#ifndef  EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif
#else
#ifndef  EXEC_LIBRARIES
#include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */
#endif

struct ExampleBase
{
    struct Library         exb_LibNode;
    SEGLISTPTR             exb_SegList;
    struct ExecBase       *exb_SysBase;
    struct IntuitionBase  *exb_IntuitionBase;
    struct GfxBase        *exb_GfxBase;
};

#endif /* EXAMPLE_EXAMPLEBASE_H */
