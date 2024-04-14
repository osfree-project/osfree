/*  Unimplemented functions
 *  (dummies)
 *
 */

#include <strnlen.h>

#define INCL_KBD
#define INCL_MOU
#define INCL_VIO
#define INCL_PMAVIO
#define INCL_DOSMONITORS
#include <os2.h>

//.#define  DO_VIO
//#define  DO_KBD
//#define  DO_MOU
//#define  DO_DOSMONITORS
//#include <copied_decl.h>

#include <stdio.h>
#include <stdarg.h>

APIRET APIENTRY DosLogWrite(PSZ s);

void log(const char *fmt, ...)
{
  va_list arg_ptr;
  char buf[1024];

  va_start(arg_ptr, fmt);
  vsprintf(buf, fmt, arg_ptr);
  va_end(arg_ptr);

  DosLogWrite(buf);
}

APIRET unimplemented(char *func)
{
  log("%s is not yet implemented!\n", func);
  return 0;
}


/* KbdCalls */

USHORT APIENTRY KbdRegister(const PSZ pszModName, const PSZ pszEntryPt,
                            const ULONG FunMask)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdDeRegister(VOID)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdCharIn(KBDKEYINFO * pkbci, const USHORT fWait, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdPeek(KBDKEYINFO * pkbci, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY KbdFlushBuffer(const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


//USHORT APIENTRY KbdStringIn(CHAR * pch, STRINGINBUF * pchIn, const USHORT fsWait, const HKBD hkbd);

USHORT APIENTRY KbdSetStatus(const PKBDINFO pkbdinfo, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdGetStatus(KBDINFO * pkbdinfo, const HKBD hdbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdSetCp(const USHORT usReserved, const USHORT pidCP, const HKBD hdbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdGetCp(const ULONG ulReserved, USHORT * pidCP, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdOpen(PHKBD * hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdClose(const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdGetFocus(const USHORT fWait, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdFreeFocus(const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdSynch(const USHORT fsWait)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdSetFgnd(VOID)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdGetHWID(PKBDHWID * kbdhwid, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdSetHWID(const PKBDHWID pkbdhwid, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdXlate(const PKBDTRANS pkbdtrans, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY KbdSetCustXt(const PUSHORT usCodePage, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}

/* MouCalls */

USHORT APIENTRY MouDeRegister(VOID)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouFlushQue(const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouGetPtrPos(PTRLOC * pmouLoc, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouSetPtrPos(const PPTRLOC pmouLoc, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouSetPtrShape(const PBYTE pBuf, const PPTRSHAPE pmoupsInfo, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouGetPtrShape(BYTE * pBuf, PTRSHAPE * pmoupsInfo, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouGetDevStatus(USHORT * pfsDevStatus, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouGetNumButtons(USHORT * pcButtons, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouGetNumMickeys(USHORT * pcMickeys, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouReadEventQue(MOUEVENTINFO * pmouevEvent, const PUSHORT pfWait, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouGetNumQueEl(MOUQUEINFO * qmouqi, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouGetEventMask(USHORT * pfsEvents, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouSetEventMask(const PUSHORT pfsEvents, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouGetScaleFact(SCALEFACT * pmouscFactors, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouSetScaleFact(const PSCALEFACT pmouscFactors, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouOpen(const PSZ pszDvrName, HMOU * phmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouClose(const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouRemovePtr(const PNOPTRRECT pmourtRect, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouDrawPtr(const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouSetDevStatus(const PUSHORT pfsDevStatus, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouInitReal(const PSZ str)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouSynch(const USHORT pszDvrName)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouGetThreshold(THRESHOLD * pthreshold, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouSetThreshold(const PTHRESHOLD pthreshold, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY MouRegister(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFuns)
{
  return unimplemented(__FUNCTION__);
}


/* Moncalls */

USHORT APIENTRY DosMonClose(const HMONITOR hmon)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY DosMonOpen(const PSZ a, HMONITOR * hmon)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY DosMonRead(const PBYTE a, const USHORT b, const PBYTE c, const PUSHORT d)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY DosMonReg(const HMONITOR hmon, const PBYTE b, const PBYTE c, const USHORT d, const USHORT e)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY DosMonWrite(const PBYTE a, const PBYTE b, const USHORT c)
{
  return unimplemented(__FUNCTION__);
}
