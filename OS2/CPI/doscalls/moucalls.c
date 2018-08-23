/* MouCalls */

#define  INCL_MOU
#include <os2.h>


USHORT __pascal MOUREGISTER(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFuns)
{
  return MouRegister(pszModName, pszEntryName, flFuns);
}


USHORT __pascal MOUDEREGISTER(VOID)
{
  return MouDeRegister();
}


USHORT __pascal MOUFLUSHQUE(const HMOU hmou)
{
  return MouFlushQue(hmou);
}


USHORT __pascal MOUGETPTRPOS(PTRLOC * pmouLoc, const HMOU hmou)
{
  return MouGetPtrPos(pmouLoc, hmou);
}


USHORT __pascal MOUSETPTRPOS(const PPTRLOC pmouLoc, const HMOU hmou)
{
  return MouSetPtrPos(pmouLoc, hmou);
}


USHORT __pascal MOUSETPTRSHAPE(const PBYTE pBuf, const PPTRSHAPE pmoupsInfo, const HMOU hmou)
{
  return MouSetPtrShape(pBuf, pmoupsInfo, hmou);
}


USHORT __pascal MOUGETPTRSHAPE(BYTE * pBuf, PTRSHAPE * pmoupsInfo, const HMOU hmou)
{
  return MouGetPtrShape(pBuf, pmoupsInfo, hmou);
}


USHORT __pascal MOUGETDEVSTATUS(USHORT * pfsDevStatus, const HMOU hmou)
{
  return MouGetDevStatus(pfsDevStatus, hmou);
}


USHORT __pascal MOUGETNUMBUTTONS(USHORT * pcButtons, const HMOU hmou)
{
  return MouGetNumButtons(pcButtons, hmou);
}


USHORT __pascal MOUGETNUMMICKEYS(USHORT * pcMickeys, const HMOU hmou)
{
  return MouGetNumMickeys(pcMickeys, hmou);
}


USHORT __pascal MOUREADEVENTQUE(MOUEVENTINFO * pmouevEvent, const PUSHORT pfWait, const HMOU hmou)
{
  return MouReadEventQue(pmouevEvent, pfWait, hmou);
}


USHORT __pascal MOUGETNUMQUEEL(MOUQUEINFO * qmouqi, const HMOU hmou)
{
  return MouGetNumQueEl(qmouqi, hmou);
}


USHORT __pascal MOUGETEVENTMASK(USHORT * pfsEvents, const HMOU hmou)
{
  return MouGetEventMask(pfsEvents, hmou);
}


USHORT __pascal MOUSETEVENTMASK(const PUSHORT pfsEvents, const HMOU hmou)
{
  return MouSetEventMask(pfsEvents, hmou);
}


USHORT __pascal MOUGETSCALEFACT(SCALEFACT * pmouscFactors, const HMOU hmou)
{
  return MouGetScaleFact(pmouscFactors, hmou);
}


USHORT __pascal MOUSETSCALEFACT(const PSCALEFACT pmouscFactors, const HMOU hmou)
{
  return MouSetScaleFact(pmouscFactors, hmou);
}


USHORT __pascal MOUOPEN(const PSZ pszDvrName, HMOU * phmou)
{
  return MouOpen(pszDvrName, phmou);
}


USHORT __pascal MOUCLOSE(const HMOU hmou)
{
  return MouClose(hmou);
}


USHORT __pascal MOUREMOVEPTR(const PNOPTRRECT pmourtRect, const HMOU hmou)
{
  return MouRemovePtr(pmourtRect, hmou);
}


USHORT __pascal MOUDRAWPTR(const HMOU hmou)
{
  return MouDrawPtr(hmou);
}


USHORT __pascal MOUSETDEVSTATUS(const PUSHORT pfsDevStatus, const HMOU hmou)
{
  return MouSetDevStatus(pfsDevStatus, hmou);
}


USHORT __pascal MOUINITREAL(const PSZ str)
{
  return MouInitReal(str);
}


USHORT __pascal MOUSYNCH(const USHORT pszDvrName)
{
  return MouSynch(pszDvrName);
}


USHORT __pascal MOUGETTHRESHOLD(THRESHOLD * pthreshold, const HMOU hmou)
{
  return MouGetThreshold(pthreshold, hmou);
}


USHORT __pascal MOUSETTHRESHOLD(const PTHRESHOLD pthreshold, const HMOU hmou)
{
  return MouSetThreshold(pthreshold, hmou);
}
