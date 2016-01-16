#ifdef __AMIGAOS4__
#define __USE_INLINE__
#include <proto/exec.h>
#include <proto/commodities.h>
#include <proto/utility.h>
#else
#include <clib/exec_protos.h>
#include <clib/commodities_protos.h>
#include <clib/utility_protos.h>
#endif

#include <exec/libraries.h>
#include <libraries/commodities.h>
#include <dos/dos.h>
#include <clib/alib_protos.h>

#ifdef __MORPHOS__
#include <emul/emulinterface.h>
#endif

#ifdef __VBCC__
#include <clib/alib_stdio_protos.h>
#endif

#include "args.h"

struct MsgPort *broker_mp;
ULONG cxsigflag;
UWORD qualifier_bit;

struct NewBroker newbroker = {
  NB_VERSION, "DontShout", "Don't Shout",
  "Remap caps/lock key",
  NBU_UNIQUE, 0, 0, 0, 0
};

static void ProcessMsg(CxObj *broker)
{
  CxMsg *msg;
  ULONG sigrcvd, msgid, msgtype;
  LONG retval = 1L;
  while (retval)
  {
    sigrcvd = Wait(SIGBREAKF_CTRL_C | cxsigflag);

    while (msg = (CxMsg *) GetMsg(broker_mp))
    {
      msgid = CxMsgID(msg);
      msgtype = CxMsgType(msg);
      ReplyMsg((struct Message *) msg);
      switch (msgtype)
      {
      case CXM_COMMAND:
        switch (msgid)
        {
        case CXCMD_DISABLE:
          ActivateCxObj(broker, 0L);
          break;
        case CXCMD_ENABLE:
          ActivateCxObj(broker, 1L);
          break;
        case CXCMD_KILL:
          retval = 0L;
          break;
        case CXCMD_UNIQUE:
          retval = 0L;
          break;
        default:
          break;
        }
        break;
      default:
        break;
      }
    }
    /* Ctrl-C break */
    if (sigrcvd & SIGBREAKF_CTRL_C) retval = 0L;
  }
}

BOOL Init(void)
{
  if (!(broker_mp = CreateMsgPort())) return FALSE;
  return TRUE;
}

void Cleanup(void)
{
  if (broker_mp) DeletePort(broker_mp);
}

#ifdef __MORPHOS__
void translate(void)
{
  struct InputEvent *ie = (struct InputEvent *) CxMsgData((APTR) REG_A0);
  /* struct CxObj *co = (APTR) REG_A1; */
#else
void translate(register CxMsg *cxm, CxObj *co)
{
  struct InputEvent *ie = (struct InputEvent *) CxMsgData(cxm);
#endif
  if (ie->ie_Qualifier & IEQUALIFIER_CAPSLOCK) {
    ie->ie_Qualifier &= ~IEQUALIFIER_CAPSLOCK;
    ie->ie_Qualifier |= qualifier_bit;
  }
}

#ifdef __MORPHOS__
struct EmulLibEntry translate_gate =
{
  TRAP_LIB, 0, (void *) translate
};
#endif

BOOL AttachCxObjects(CxObj *broker)
{
  CxObj *custom;
#ifdef __MORPHOS__
  custom = CxCustom((void *) &translate_gate, 0L);
#else
  custom = CxCustom(translate, 0L);
#endif
  if (custom) {
    AttachCxObj(broker, custom);
    return TRUE;
  }
  return FALSE;
}

UWORD get_qualifier(STRPTR qualifier)
{
  if (!Stricmp(qualifier, "CONTROL"))
    return IEQUALIFIER_CONTROL;
  if (!Stricmp(qualifier, "LSHIFT"))
    return IEQUALIFIER_LSHIFT;
  if (!Stricmp(qualifier, "RSHIFT"))
    return IEQUALIFIER_RSHIFT;
  if (!Stricmp(qualifier, "LALT"))
    return IEQUALIFIER_LALT;
  if (!Stricmp(qualifier, "RALT"))
    return IEQUALIFIER_RALT;
  if (!Stricmp(qualifier, "LAMIGA"))
    return IEQUALIFIER_LCOMMAND;
  if (!Stricmp(qualifier, "RAMIGA"))
    return IEQUALIFIER_RCOMMAND;
  return IEQUALIFIER_CONTROL;
}

int main(int argc, char **argv)
{
  STRPTR *ttypes, qualifier;
  CxMsg *msg;
  CxObj *broker;

  if (Init())
  {
    newbroker.nb_Port = broker_mp;
    cxsigflag = 1L << broker_mp->mp_SigBit;
    ttypes = ArgArrayInit2(argc, argv);
    newbroker.nb_Pri = (BYTE) ArgInt2(ttypes, "CX_PRIORITY", 0);
    qualifier = ArgString2(ttypes, "QUALIFIER", "CONTROL");
    qualifier_bit = get_qualifier(qualifier);

    if (broker = CxBroker(&newbroker, NULL))
    {
      if (AttachCxObjects(broker))
      {
        ActivateCxObj(broker, 1L);
        ProcessMsg(broker);
      }
      DeleteCxObjAll(broker);
      while (msg = (CxMsg *) GetMsg(broker_mp)) ReplyMsg((struct Message *) msg);
    }
    ArgArrayDone2();
  }
  Cleanup();
  return 0;
}
