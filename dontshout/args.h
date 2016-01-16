#ifndef __ARGS_H__
#define __ARGS_H__

/*
 * These are replacement functions for ArgArrayInit(), ...
 * which are usually provided in amiga.lib, but can not be expected
 * to be available in AmigaOS 4.x.
 * These parse parameters of the form <SETTING>=<VALUE> regardless
 * whether the tool was started from command line or workbench.
 */
extern STRPTR *ArgArrayInit2(int argc, char **argv);
extern void ArgArrayDone2(void);
extern STRPTR ArgString2(UBYTE **tt, STRPTR entry, STRPTR defaultstr);
extern LONG ArgInt2(UBYTE **tt, STRPTR entry, LONG defaultval);

#endif
