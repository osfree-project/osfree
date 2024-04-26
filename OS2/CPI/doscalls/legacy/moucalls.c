// 16-bit api
#include <kal.h>

// 16-bit to 32-bit wrappers. 32-bit MOU subsystem lives in SUB32.DLL

// 32-bit API
USHORT APIENTRY Mou32DeRegister(VOID);
USHORT APIENTRY Mou32FlushQue(const HMOU hmou);
USHORT APIENTRY Mou32GetPtrPos(PTRLOC * pmouLoc, const HMOU hmou);
USHORT APIENTRY Mou32SetPtrPos(const PPTRLOC pmouLoc, const HMOU hmou);
USHORT APIENTRY Mou32SetPtrShape(const PBYTE pBuf, const PPTRSHAPE pmoupsInfo, const HMOU hmou);
USHORT APIENTRY Mou32GetPtrShape(BYTE * pBuf, PTRSHAPE * pmoupsInfo, const HMOU hmou);
USHORT APIENTRY Mou32GetDevStatus(USHORT * pfsDevStatus, const HMOU hmou);
USHORT APIENTRY Mou32GetNumButtons(USHORT * pcButtons, const HMOU hmou);
USHORT APIENTRY Mou32GetNumMickeys(USHORT * pcMickeys, const HMOU hmou);
USHORT APIENTRY Mou32ReadEventQue(MOUEVENTINFO * pmouevEvent, const PUSHORT pfWait, const HMOU hmou);
USHORT APIENTRY Mou32GetNumQueEl(MOUQUEINFO * qmouqi, const HMOU hmou);
USHORT APIENTRY Mou32GetEventMask(USHORT * pfsEvents, const HMOU hmou);
USHORT APIENTRY Mou32SetEventMask(const PUSHORT pfsEvents, const HMOU hmou);
USHORT APIENTRY Mou32GetScaleFact(SCALEFACT * pmouscFactors, const HMOU hmou);
USHORT APIENTRY Mou32SetScaleFact(const PSCALEFACT pmouscFactors, const HMOU hmou);
USHORT APIENTRY Mou32Open(const PSZ pszDvrName, HMOU * phmou);
USHORT APIENTRY Mou32Close(const HMOU hmou);
USHORT APIENTRY Mou32RemovePtr(const PNOPTRRECT pmourtRect, const HMOU hmou);
USHORT APIENTRY Mou32DrawPtr(const HMOU hmou);
USHORT APIENTRY Mou32SetDevStatus(const PUSHORT pfsDevStatus, const HMOU hmou);
USHORT APIENTRY Mou32InitReal(const PSZ str);
USHORT APIENTRY Mou32Synch(const USHORT pszDvrName);
USHORT APIENTRY Mou32GetThreshold(VOID * pthreshold, const HMOU hmou);
USHORT APIENTRY Mou32SetThreshold(const VOID * pthreshold, const HMOU hmou);
USHORT APIENTRY Mou32Register(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFuns);

// 16->32 wrappers

APIRET16 APIENTRY16 MOUCLOSE(HMOU hmou)
{
	return Mou32Close(hmou);
}

APIRET16 APIENTRY16 MOUDEREGISTER(VOID)
{
	return Mou32DeRegister();
}

APIRET16 APIENTRY16 MOUDRAWPTR(HMOU hmou)
{
	return Mou32DrawPtr(hmou);
}

APIRET16 APIENTRY16 MOUFLUSHQUE(HMOU hmou)
{
	return Mou32FlushQue(hmou);
}

APIRET16 APIENTRY16 MOUGETDEVSTATUS(PUSHORT push,HMOU hmou)
{
	return Mou32GetDevStatus(push, hmou);
}

APIRET16 APIENTRY16 MOUGETEVENTMASK(PUSHORT push, HMOU hmou)
{
	return Mou32GetEventMask(push, hmou);
}

APIRET16 APIENTRY16 MOUGETNUMBUTTONS(PUSHORT push, HMOU hmou)
{
	return Mou32GetNumButtons(push, hmou);
}

APIRET16 APIENTRY16 MOUGETNUMMICKEYS(PUSHORT push,HMOU hmou)
{
	return Mou32GetNumMickeys(push, hmou);
}

APIRET16 APIENTRY16 MOUGETNUMQUEEL(PMOUQUEINFO qmouqi,HMOU hmou)
{
	return Mou32GetNumQueEl(qmouqi, hmou);
}

APIRET16 APIENTRY16 MOUGETPTRPOS(PPTRLOC pmouLoc,HMOU hmou)
{
	return Mou32GetPtrPos(pmouLoc, hmou);
}

APIRET16 APIENTRY16 MOUGETPTRSHAPE(PBYTE pbyte,PPTRSHAPE pmoupsInfo,HMOU hmou)
{
	return Mou32GetPtrShape(pbyte, pmoupsInfo, hmou);
}

APIRET16 APIENTRY16 MOUGETSCALEFACT(PSCALEFACT pmouscFactors,HMOU hmou)
{
	return Mou32GetScaleFact(pmouscFactors, hmou);
}

APIRET16 APIENTRY16 MOUINITREAL(PSZ psz)
{
	return Mou32InitReal(psz);
}

APIRET16 APIENTRY16 MOUOPEN(PSZ psz,PHMOU phmou)
{
	return Mou32Open(psz, phmou);
}

APIRET16 APIENTRY16 MOUREADEVENTQUE(PMOUEVENTINFO ei,PUSHORT push,HMOU hmou)
{
	return Mou32ReadEventQue(ei, push, hmou);
}

APIRET16 APIENTRY16 MOUREGISTER(PSZ a,PSZ b,ULONG c)
{
	return Mou32Register(a,b,c);
}

APIRET16 APIENTRY16 MOUREMOVEPTR(PNOPTRRECT ptr,HMOU hmou)
{
	return Mou32RemovePtr(ptr, hmou);
}

APIRET16 APIENTRY16 MOUSETDEVSTATUS(PUSHORT a,HMOU b)
{
	return Mou32SetDevStatus(a, b);
}

APIRET16 APIENTRY16 MOUSETEVENTMASK(PUSHORT a,HMOU b)
{
	return Mou32SetEventMask(a, b);
}

APIRET16 APIENTRY16 MOUSETPTRPOS(PPTRLOC a,HMOU b)
{
	return Mou32SetPtrPos(a, b);
}

APIRET16 APIENTRY16 MOUSETPTRSHAPE(PBYTE a,PPTRSHAPE b,HMOU c)
{
	return Mou32SetPtrShape(a,b,c);
}

APIRET16 APIENTRY16 MOUSETSCALEFACT(PSCALEFACT a,HMOU b)
{
	return Mou32SetScaleFact(a,b);
}

APIRET16 APIENTRY16 MOUSYNCH(USHORT a)
{
	return Mou32Synch(a);
}

//USHORT APIENTRY16 MOUGETTHRESHOLD(THRESHOLD * _Seg16 pthreshold, const HMOU hmou)
APIRET16 APIENTRY16 MOUGETTHRESHOLD(void * _Seg16 pthreshold, const HMOU hmou)
{
  return Mou32GetThreshold(pthreshold, hmou);
}

//USHORT APIENTRY16 MOUSETTHRESHOLD(const THRESHOLD * _Seg16 pthreshold, const HMOU hmou)
APIRET16 APIENTRY16 MOUSETTHRESHOLD(void * _Seg16 pthreshold, const HMOU hmou)
{
  return Mou32SetThreshold(pthreshold, hmou);
}

// fix prototype !!! (undoc)
APIRET16 APIENTRY16 MOUSHELLINIT(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!! (undoc)
APIRET16 APIENTRY16 MOUGETHOTKEY(void)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!! (undoc)
APIRET16 APIENTRY16 MOUSETHOTKEY(void)
{
  return unimplemented(__FUNCTION__);
}
