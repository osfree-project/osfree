/* KbdCalls */

#define INCL_KBD
#include <os2.h>

USHORT __pascal KBDREGISTER(const PSZ pszModName, const PSZ pszEntryPt,
                            const ULONG FunMask)
{
  return KbdRegister(pszModName, pszEntryPt, FunMask);
}


USHORT __pascal KBDDEREGISTER(VOID)
{
  return KbdDeRegister();
}


USHORT __pascal KBDCHARIN(KBDKEYINFO * pkbci, const USHORT fWait, const HKBD hkbd)
{
  return KbdCharIn(pkbci, fWait, hkbd);
}


USHORT __pascal KBDPEEK(KBDKEYINFO * pkbci, const HKBD hkbd)
{
  return KbdPeek(pkbci, hkbd);
}

USHORT __pascal KBDFLUSHBUFFER(const HKBD hkbd)
{
  return KbdFlushBuffer(hkbd);
}


USHORT __pascal KBDSTRINGIN(CHAR * pch, STRINGINBUF * pchIn, const USHORT fsWait, const HKBD hkbd)
{
  return KbdStringIn(pch, pchIn, fsWait, hkbd);
}


USHORT __pascal KBDSETSTATUS(const PKBDINFO pkbdinfo, const HKBD hkbd)
{
  return KbdSetStatus(pkbdinfo, hkbd);
}


USHORT __pascal KBDGETSTATUS(KBDINFO * pkbdinfo, const HKBD hdbd)
{
  return KbdGetStatus(pkbdinfo, hdbd);
}


USHORT __pascal KBDSETCP(const USHORT usReserved, const USHORT pidCP, const HKBD hdbd)
{
  return KbdSetCp(usReserved, pidCP, hdbd);
}


USHORT __pascal KBDGETCP(const ULONG ulReserved, USHORT * pidCP, const HKBD hkbd)
{
  return KbdGetCp(ulReserved, pidCP, hkbd);
}


USHORT __pascal KBDOPEN(PHKBD * hkbd)
{
  return KbdOpen(hkbd);
}


USHORT __pascal KBDCLOSE(const HKBD hkbd)
{
  return KbdClose(hkbd);
}


USHORT __pascal KBDGETFOCUS(const USHORT fWait, const HKBD hkbd)
{
  return KbdGetFocus(fWait, hkbd);
}


USHORT __pascal KBDFREEFOCUS(const HKBD hkbd)
{
  return KbdFreeFocus(hkbd);
}


USHORT __pascal KBDSYNCH(const USHORT fsWait)
{
  return KbdSynch(fsWait);
}


USHORT __pascal KBDSETFGND(VOID)
{
  return KbdSetFgnd();
}


USHORT __pascal KBDGETHWID(PKBDHWID * kbdhwid, const HKBD hkbd)
{
  return KbdGetHWID(kbdhwid, hkbd);
}


USHORT __pascal KBDSETHWID(const PKBDHWID pkbdhwid, const HKBD hkbd)
{
  return KbdSetHWID(pkbdhwid, hkbd);
}


USHORT __pascal KBDXLATE(const PKBDTRANS pkbdtrans, const HKBD hkbd)
{
  return KbdXlate(pkbdtrans, hkbd);
}


USHORT __pascal KBDSETCUSTXT(const PUSHORT usCodePage, const HKBD hkbd)
{
  return KbdSetCustXt(usCodePage, hkbd);
}
