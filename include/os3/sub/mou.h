/*****************************************************************************
    mou.h
    (C) 2004-2010 osFree project

    WARNING! Automaticaly generated file! Don't edit it manually!
*****************************************************************************/

#ifndef __MOU_H__
#define __MOU_H__

#ifdef __cplusplus
   extern "C" {
#endif

#ifndef NULL
#define NULL  0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef EXPENTRY
#define EXPENTRY  _System
#endif

#ifndef APIENTRY
#define APIENTRY  _System
#endif

#ifndef FAR
#define FAR
#endif

#ifndef NEAR
#define NEAR
#endif

typedef USHORT HMOU;
typedef HMOU * PHMOU;
USHORT APIENTRY MouRegister(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFuns);
#define MR_MOUGETNUMBUTTONS 0x00000001
#define MR_MOUGETNUMMICKEYS 0x00000002
#define MR_MOUGETDEVSTATUS 0x00000004
#define MR_MOUGETNUMQUEEL 0x00000008
#define MR_MOUREADEVENTQUE 0x00000010
#define MR_MOUGETSCALEFACT 0x00000020
#define MR_MOUGETEVENTMASK 0x00000040
#define MR_MOUSETSCALEFACT 0x00000080
#define MR_MOUSETEVENTMASK 0x00000100
#define MR_MOUOPEN 0x00000800
#define MR_MOUCLOSE 0x00001000
#define MR_MOUGETPTRSHAPE 0x00002000
#define MR_MOUSETPTRSHAPE 0x00004000
#define MR_MOUDRAWPTR 0x00008000
#define MR_MOUREMOVEPTR 0x00010000
#define MR_MOUGETPTRPOS 0x00020000
#define MR_MOUSETPTRPOS 0x00040000
#define MR_MOUINITREAL 0x00080000
#define MR_MOUSETDEVSTATUS 0x00100000
USHORT APIENTRY MouDeRegister(VOID);
USHORT APIENTRY MouFlushQue(const HMOU hmou);
#define MHK_BUTTON1 0x0001
#define MHK_BUTTON2 0x0002
#define MHK_BUTTON3 0x0004
typedef struct _PTRLOC {
USHORT row;
USHORT col;
} PTRLOC;
typedef PTRLOC * PPTRLOC;
USHORT APIENTRY MouGetPtrPos(PTRLOC * pmouLoc, const HMOU hmou);
USHORT APIENTRY MouSetPtrPos(const PPTRLOC pmouLoc, const HMOU hmou);
typedef struct _PTRSHAPE {
USHORT cb;
USHORT col;
USHORT row;
USHORT colHot;
USHORT rowHot;
} PTRSHAPE;
typedef PTRSHAPE * PPTRSHAPE;
USHORT APIENTRY MouSetPtrShape(const PBYTE pBuf, const PPTRSHAPE pmoupsInfo, const HMOU hmou);
USHORT APIENTRY MouGetPtrShape(BYTE * pBuf, PTRSHAPE * pmoupsInfo, const HMOU hmou);
#define MOUSE_QUEUEBUSY 0x0001
#define MOUSE_BLOCKREAD 0x0002
#define MOUSE_FLUSH 0x0004
#define MOUSE_UNSUPPORTED_MODE 0x0008
#define MOUSE_DISABLED 0x0100
#define MOUSE_MICKEYS 0x0200
USHORT APIENTRY MouGetDevStatus(USHORT * pfsDevStatus, const HMOU hmou);
USHORT APIENTRY MouGetNumButtons(USHORT * pcButtons, const HMOU hmou);
USHORT APIENTRY MouGetNumMickeys(USHORT * pcMickeys, const HMOU hmou);
#define MOU_NOWAIT 0x0000
#define MOU_WAIT 0x0001
typedef struct _MOUEVENTINFO {
USHORT fs;
ULONG time;
SHORT row;
SHORT col;
} MOUEVENTINFO;
typedef MOUEVENTINFO * PMOUEVENTINFO;
USHORT APIENTRY MouReadEventQue(MOUEVENTINFO * pmouevEvent, const PUSHORT pfWait, const HMOU hmou);
typedef struct _MOUQUEINFO {
USHORT cEvents;
USHORT cmaxEvents;
} MOUQUEINFO;
typedef MOUQUEINFO * PMOUQUEINFO;
USHORT APIENTRY MouGetNumQueEl(MOUQUEINFO * qmouqi, const HMOU hmou);
#define MOUSE_MOTION 0x0001
#define MOUSE_MOTION_WITH_BN1_DOWN 0x0002
#define MOUSE_BN1_DOWN 0x0004
#define MOUSE_MOTION_WITH_BN2_DOWN 0x0008
#define MOUSE_BN2_DOWN 0x0010
#define MOUSE_MOTION_WITH_BN3_DOWN 0x0020
#define MOUSE_BN3_DOWN 0x0040
USHORT APIENTRY MouGetEventMask(USHORT * pfsEvents, const HMOU hmou);
USHORT APIENTRY MouSetEventMask(const PUSHORT pfsEvents, const HMOU hmou);
typedef struct _SCALEFACT {
USHORT rowScale;
USHORT colScale;
} SCALEFACT;
typedef SCALEFACT * PSCALEFACT;
USHORT APIENTRY MouGetScaleFact(SCALEFACT * pmouscFactors, const HMOU hmou);
USHORT APIENTRY MouSetScaleFact(const PSCALEFACT pmouscFactors, const HMOU hmou);
USHORT APIENTRY MouOpen(const PSZ pszDvrName, HMOU * phmou);
USHORT APIENTRY MouClose(const HMOU hmou);
typedef struct _NOPTRRECT {
USHORT row;
USHORT col;
USHORT cRow;
USHORT cCol;
} NOPTRRECT;
typedef NOPTRRECT * PNOPTRRECT;
USHORT APIENTRY MouRemovePtr(const PNOPTRRECT pmourtRect, const HMOU hmou);
USHORT APIENTRY MouDrawPtr(const HMOU hmou);
#define MOU_NODRAW 0x0001
#define MOU_DRAW 0x0000
#define MOU_MICKEYS 0x0002
#define MOU_PELS 0x0000
USHORT APIENTRY MouSetDevStatus(const PUSHORT pfsDevStatus, const HMOU hmou);
USHORT APIENTRY MouInitReal(const PSZ str);
USHORT APIENTRY MouSynch(const USHORT pszDvrName);
typedef struct _THRESHOLD {
USHORT Length;
USHORT Level1;
USHORT Lev1Mult;
USHORT Level2;
USHORT lev2Mult;
} THRESHOLD;
typedef THRESHOLD * PTHRESHOLD;
USHORT APIENTRY MouGetThreshold(THRESHOLD * pthreshold, const HMOU hmou);
USHORT APIENTRY MouSetThreshold(const PTHRESHOLD pthreshold, const HMOU hmou);

#ifdef __cplusplus
   }
#endif

#endif /* __MOU_H__ */
