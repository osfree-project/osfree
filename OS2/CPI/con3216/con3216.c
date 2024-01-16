/*
  32TEXT to EMXWRAP interface converter
*/

#include "emxwrap.h"
#include "conbsesub.h"


APIRET APIENTRY  KbdCharIn(PKBDKEYINFO CharData, ULONG Wait, HKBD hkbd)
{
  KBDKEYINFO16 kbdki;
  APIRET16 rc;
  
  rc=EMXKbdCharIn(&kbdki, Wait, hkbd);

  CharData->chChar=kbdki.chChar;
  CharData->chScan=kbdki.chScan;
  CharData->fbStatus=kbdki.fbStatus;
  CharData->bNlsShift=kbdki.bNlsShift;
  CharData->fsState=kbdki.fsState;
  CharData->time=kbdki.time;

  return rc;
}

APIRET APIENTRY  KbdGetConsole(PVOID Data, PULONG Kind, ULONG Flags,
                 HKBD hkbd)
{
  return 0;
}

APIRET APIENTRY  KbdFlushBuffer(HKBD hkbd)
{
  return EMXKbdFlushBuffer(hkbd);
}

APIRET APIENTRY  KbdGetCp(ULONG ulReserved, PUSHORT pidCP, HKBD hkbd)
{
  return EMXKbdGetCp(ulReserved, pidCP, hkbd);
}

APIRET APIENTRY  KbdGetHWID(PKBDHWID pkbdhwid, HKBD hkbd)
{
  KBDHWID16 hwid;
  APIRET16 rc;

  rc=EMXKbdGetHWID(&hwid, hkbd);

  pkbdhwid->cb=sizeof(KBDHWID);
  pkbdhwid->idKbd=hwid.idKbd;
  pkbdhwid->idSecond=hwid.usReserved1;

  return rc;
}

APIRET APIENTRY  KbdGetLayout(PSZ name, HKBD hkbd)
{
  return 0;
}

APIRET APIENTRY  KbdGetLayoutUni(USHORT * name, HKBD hkbd)
{
  return 0;
}

APIRET APIENTRY  KbdGetStatus (PKBDINFO pkbdinfo, HKBD hkbd)
{
  return EMXKbdGetStatus(pkbdinfo, hkbd);
}

APIRET APIENTRY  KbdPeek(PKBDKEYINFO pkbci, HKBD hkbd)
{
  KBDKEYINFO16 ki;
  APIRET16 rc;

  rc=EMXKbdPeek(&ki, hkbd);
  pkbci->chChar=ki.chChar;
  pkbci->chScan=ki.chScan;
  pkbci->fbStatus=ki.fbStatus;
  pkbci->bNlsShift=ki.bNlsShift;
  pkbci->fsState=ki.fsState;
  pkbci->time=ki.time;
  pkbci->ucUniChar=ki.chChar;
  pkbci->VKey=0;
  pkbci->resv=0;

  return rc;
}

APIRET APIENTRY  KbdSetCp(ULONG ulReserved, USHORT pidCP, HKBD hkbd)
{
  return EMXKbdSetCp(ulReserved, pidCP, hkbd);
}

APIRET APIENTRY  KbdSetLayout(PSZ name, HKBD hkbd)
{
  return 0;
}

APIRET APIENTRY  KbdSetLayoutUni(USHORT * name, HKBD hkbd)
{
  return 0;
}

APIRET APIENTRY  KbdSetRate (ULONG rate, ULONG delay, HKBD hkbd)
{
  return 0;
}

APIRET APIENTRY  KbdSetStatus (PKBDINFO pkbdinfo, HKBD hkbd)
{
  return EMXKbdSetStatus(pkbdinfo, hkbd);
}

APIRET APIENTRY  KbdStringIn (PCH pch, PSTRINGINBUF pchIn,
                              ULONG Flag, HKBD hkbd)
{
  return 0;
}

APIRET APIENTRY  KbdStringInUni (USHORT * pch, PSTRINGINBUF pchIn,
                                 ULONG Flag, HKBD hkbd)
{
  return 0;
}

APIRET APIENTRY  KbdXlate (PKBDKEYINFO pkbdtrans, HKBD hkbd)
{
  return 0;
}


APIRET APIENTRY  VioAssociate(ULONG hdc, HVIO hvps)
{
  return 0;
}

APIRET APIENTRY  VioCharWidthUni(USHORT uch)
{
  return 0;
}

APIRET APIENTRY  VioCheckCharType (PULONG pType, ULONG ulRow,
                                   ULONG ulColumn, HVIO hvio)
{
  return 0;
}

//APIRET APIENTRY  VioCreateLogFont(PFATTRS pfat, ULONG lcid, PSTR8 pstr8Name,
//                                 HVIO hvps)
//{
//  return 0;
//}

//APIRET APIENTRY  VioCreatePS(PHVIO phvps, ULONG Rows, ULONG Columns,
//                            ULONG Format, ULONG AttrBytes, HVIO hvps)
//{
//  return 0;
//}

APIRET APIENTRY  VioDeleteSetId(ULONG lcid, HVIO hvps)
{
  return 0;
}

APIRET APIENTRY  VioDestroyPS(HVIO hvps)
{
  return 0;
}

APIRET APIENTRY  VioEndPopUp (HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioGetAnsi (PULONG pfAnsi, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioGetBuf  (PULONG pLVB, PULONG pcbLVB, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioGetConfig (ULONG ulConfigId, PVIOCONFIGINFO pvioin,
                                   HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioGetCp (ULONG ulReserved, PUSHORT pIdCodePage, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioGetCurPos (PULONG pusRow, PULONG pusColumn, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioGetCurType (PVIOCURSORINFO pvioCursorInfo, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioGetDeviceCellSize(PULONG Height, PULONG Width,
                                     HVIO hvps)
{
  return 0;
}

APIRET APIENTRY  VioGetMode (PVIOMODEINFO pvioModeInfo, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioGetOrigin(PULONG Row, PULONG Column, HVIO hvps)
{
  return 0;
}

APIRET APIENTRY  VioGetState (PVOID pState, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioModeUndo (ULONG ulOwnerInd, ULONG ulKillInd,
                                  ULONG ulReserved)
{
  return 0;
}

APIRET APIENTRY  VioModeWait (ULONG ulReqType, PULONG pNotifyType,
                                  ULONG ulReserved)
{
  return 0;
}

APIRET APIENTRY  VioPopUp (PULONG pfWait, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioPrtSc (HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioPrtScToggle (HVIO hvio)
{
  return 0;
}

//APIRET APIENTRY  VioQueryFonts(PULONG Remfonts, PFONTMETRICS Metrics,
//                              ULONG MetricsLength, PULONG Fonts,
//                              PSZ Facename, ULONG Options, HVIO hvps)
//{
//  return 0;
//}

//APIRET APIENTRY  VioQueryFontsUni(PULONG Remfonts, PFONTMETRICS Metrics,
//                              ULONG MetricsLength, PULONG Fonts,
//                              USHORT * Facename, ULONG Options, HVIO hvps)
//{
//  return 0;
//}

APIRET APIENTRY  VioQuerySetIds(PULONG lcids, PSTR8 Names,
                               PULONG Types, ULONG count, HVIO hvps)
{
  return 0;
}

APIRET APIENTRY  VioReadCellStr (PCH pchCellStr, PULONG pcb, ULONG ulRow,
                                 ULONG ulColumn, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioReadCharStr (PCH pchCellStr, PULONG pcb, ULONG ulRow,
                                 ULONG ulColumn, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioSaveRedrawUndo (ULONG ulOwnerInd, ULONG ulKillInd,
                                       ULONG ulReserved)
{
  return 0;
}

APIRET APIENTRY  VioSaveRedrawWait (ULONG ulRedrawInd, PULONG pNotifyType,
                                       ULONG ulReserved)
{
  return 0;
}

APIRET APIENTRY  VioScrLock (ULONG fWait, PUCHAR pfNotLocked, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioScrollDown (ULONG ulTopRow, ULONG ulLeftCol,
                              ULONG ulBotRow, ULONG ulRightCol,
                              ULONG cbLines,  PBYTE pCell, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioScrollLeft (ULONG ulTopRow, ULONG ulLeftCol,
                              ULONG ulBotRow, ULONG ulRightCol,
                              ULONG cbCol, PBYTE pCell, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioScrollRight (ULONG ulTopRow, ULONG ulLeftCol,
                              ULONG ulBotRow, ULONG ulRightCol,
                              ULONG cbCol, PBYTE pCell, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioScrollUp (ULONG ulTopRow, ULONG ulLeftCol,
                              ULONG ulBotRow, ULONG ulRightCol,
                              ULONG cbLines, PBYTE pCell, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioScrUnLock (HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioSetAnsi (ULONG fAnsi, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioSetCp (ULONG ulReserved, USHORT idCodePage, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioSetCurPos (ULONG  ulRow,  ULONG  ulColumn,  HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioSetCurType (PVIOCURSORINFO pvioCursorInfo, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioSetDeviceCellSize(ULONG Height, ULONG Width, HVIO hvps)
{
  return 0;
}

APIRET APIENTRY  VioSetMode (PVIOMODEINFO pvioModeInfo, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioSetOrigin(ULONG Row, ULONG Column, HVIO hvps)
{
  return 0;
}

APIRET APIENTRY  VioSetState (PVOID pState, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioShowBuf (ULONG offLVB, ULONG cb, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioShowPS(ULONG Depth, ULONG Width, ULONG Cell, HVIO hvps)
{
  return 0;
}

APIRET APIENTRY  VioWrtCellStr (PCH pchCellStr, ULONG cb, ULONG ulRow,
                                ULONG ulColumn, HVIO hvio)
{
  return 0;
}

APIRET  APIENTRY VioWrtCharStr (PCH pchStr, ULONG cb, ULONG ulRow,
                                ULONG ulColumn, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioWrtCharStrAttr (PCH pch, ULONG cb, ULONG ulRow,
                                   ULONG ulColumn, PBYTE pAttr, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioWrtNAttr (PBYTE pAttr, ULONG cb, ULONG ulRow,
                              ULONG ulColumn, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioWrtNCell (PBYTE pCell, ULONG cb, ULONG ulRow,
                              ULONG ulColumn, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioWrtNChar (PCH pchChar, ULONG cb, ULONG ulRow,
                              ULONG ulColumn, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioWrtTTY   (PCH pch, ULONG cb, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  VioWrtTTYUni (USHORT * pch, ULONG cb, HVIO hvio)
{
  return 0;
}

APIRET APIENTRY  MouFlushQue (HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouDrawPtr (HMOU hmou)
{
  return 0;
}


APIRET APIENTRY  MouGetDevStatus (PULONG  DevStat, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouGetEventMask (PULONG  EventMask, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouGetNumButtons (PULONG  Buttons, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouGetNumMickeys (PULONG  Mickeys, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouGetNumQueEl (PMOUQUEINFO QElements, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouGetPtrPos (PPTRLOC PtrLoc, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouGetPtrShape (PVOID PtrBuf, PPTRSHAPE PtrInfo, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouGetScaleFact (PSCALEFACT MouScale, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouReadEventQue (PMOUEVENTINFO EventMask, PULONG Wait,
                                  HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouRemovePtr (PNOPTRRECT MouseRect, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouSetDevStatus (PULONG  DevStat, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouSetEventMask (PULONG  EventMask, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouSetPtrPos (PPTRLOC PtrLoc, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouSetPtrShape (PBYTE PtrBuf, PPTRSHAPE PtrInfo, HMOU hmou)
{
  return 0;
}

APIRET APIENTRY  MouSetScaleFact (PSCALEFACT Scale, HMOU hmou)
{
  return 0;
}

