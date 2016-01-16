#ifdef __AMIGAOS4__
#define __USE_INLINE__
#include <proto/exec.h>
#include <proto/icon.h>
#else
#include <clib/exec_protos.h>
#include <clib/icon_protos.h>
#endif

#include <exec/libraries.h>
#include <workbench/startup.h>

static struct DiskObject *dobj = NULL;
static int num_ttypes = 0;
static STRPTR *ttypes = NULL;

STRPTR *ArgArrayInit2(int argc, char **argv)
{
  if (argc == 0)
  {
    /* Run from workbench */
    struct WBStartup *wb_msg = (struct WBStartup *) argv;
    dobj = GetDiskObject(wb_msg->sm_ArgList[0].wa_Name);
    if (dobj)
    {
      /* extract tool types  */
      return dobj->do_ToolTypes;
    }
  } else if (argc > 1)
  {
    /* from CLI, copy arguments */
    int i;
    ttypes = AllocVec(sizeof(STRPTR) * argc - 1, MEMF_ANY);
    num_ttypes = argc - 1;
    for (i = 1; i < argc; i++) ttypes[i - 1] = argv[i];
    return ttypes;
  }
  return NULL;
}

void ArgArrayDone2(void)
{
  if (dobj) FreeDiskObject(dobj);
  else if (ttypes) FreeVec(ttypes);
}

STRPTR ArgString2(UBYTE **tt, STRPTR entry, STRPTR defaultstr)
{
  if (tt)
  {
    STRPTR result = FindToolType(tt, entry);
    return result ? result : defaultstr;
  }
  return defaultstr;
}

LONG ArgInt2(UBYTE **tt, STRPTR entry, LONG defaultval)
{
  if (tt)
  {
    STRPTR result = FindToolType(tt, entry);
    return result ? atoi(result) : defaultval;
  }
  return defaultval;
}
