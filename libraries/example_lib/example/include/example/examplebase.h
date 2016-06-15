#ifndef EXAMPLE_EXAMPLEBASE_H
#define EXAMPLE_EXAMPLEBASE_H

#ifndef  EXEC_LIBRARIES
#include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */

struct ExampleBase
{
    struct Library         exb_LibNode;
    SEGLISTPTR             exb_SegList;
    struct ExecBase       *exb_SysBase;
    struct IntuitionBase  *exb_IntuitionBase;
    struct GfxBase        *exb_GfxBase;
};

#endif /* EXAMPLE_EXAMPLEBASE_H */
