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


/* VioCalls */

USHORT APIENTRY VioDeRegister(VOID)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioRegister(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFun1, const ULONG flFun2)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioGlobalReg(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFun1, const ULONG flFun2, const USHORT usReturn)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioGetAnsi(USHORT * Ansi, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioGetCp(const USHORT Reserved, USHORT * IdCodePage, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioGetCurPos(USHORT * Row, USHORT * Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioGetCurType(VIOCURSORINFO * CursorInfo, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioGetMode(VIOMODEINFO * ModeInfo, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioReadCellStr(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioReadCharStr(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioScrollDown(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioScrollDn(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioScrollLeft(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioScrollRight(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioScrollUp(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioSetAnsi(const USHORT Ansi, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioSetCp(const USHORT Reserved, const USHORT IdCodePage, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioSetCurPos(const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioSetCurType(const PVIOCURSORINFO CursorInfo, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioSetMode(const PVIOMODEINFO ModeInfo, const HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioWrtCellStr(const PCHAR CellStr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioWrtCharStr(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioWrtCharStrAttr(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const PBYTE pAttr, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioWrtCharStrAtt(const PCCH pch, const USHORT cb, const USHORT usRow, const USHORT usColumn, const PBYTE pAttr, const HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioWrtNAttr(const PBYTE Attr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioWrtNCell(const PBYTE Cell, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioWrtNChar(const PCHAR Char, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


//USHORT APIENTRY VioWrtTTY(const PCHAR Str, const USHORT Count, const HVIO Handle)


USHORT APIENTRY VioGetState(const PVOID pState, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioSetState(const PVOID pState, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioGetConfig(const USHORT ConfigId, VIOCONFIGINFO * vioin, const HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioPopUp (PUSHORT pfWait,
                         HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioEndPopUp (HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioGetPhysBuf (PVIOPHYSBUF pvioPhysBuf,
                              USHORT usReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioPrtSc (HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioScrollRt (USHORT usTopRow,
                            USHORT usLeftCol,
                            USHORT usBotRow,
                            USHORT usRightCol,
                            USHORT cbCol,
                            PBYTE pCell,
                            HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioScrLock (USHORT fWait,
                           PUCHAR pfNotLocked,
                           HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioScrUnLock (HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioSavRedrawWait (USHORT usRedrawInd,
                                 PUSHORT pNotifyType,
                                 USHORT usReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioSavRedrawUndo (USHORT usOwnerInd,
                                 USHORT usKillInd,
                                 USHORT usReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioGetFont (PVIOFONTINFO pviofi,
                           HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioSetFont (PVIOFONTINFO pviofi,
                           HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioGetBuf (PULONG pLVB,
                          PUSHORT pcbLVB,
                          HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioModeWait (USHORT usReqType,
                            PUSHORT pNotifyType,
                            USHORT usReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioModeUndo (USHORT usOwnerInd,
                              USHORT usKillInd,
                              USHORT usReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioShowBuf (USHORT offLVB,
                             USHORT cb,
                             HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioScrollLf (USHORT usTopRow,
                            USHORT usLeftCol,
                            USHORT usBotRow,
                            USHORT usRightCol,
                            USHORT cbCol,
                            PBYTE pCell,
                            HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioPrtScToggle (HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioCheckCharType (PUSHORT pType,
                                 USHORT usRow,
                                 USHORT usColumn,
                                 HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


/* AVIO */

MRESULT APIENTRY WinDefAVioWindowProc(HWND hwnd,
                                        USHORT msg,
                                        ULONG mp1,
                                        ULONG mp2)
{
  return (MRESULT)unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioAssociate(HDC hdc,
                             HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioCreatePS(PHVPS phvps,
                            SHORT sdepth,
                            SHORT swidth,
                            SHORT sFormat,
                            SHORT sAttrs,
                            HVPS hvpsReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioDestroyPS(HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioDeleteSetId(LONG llcid,
                               HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioGetDeviceCellSize(PSHORT psHeight,
                                     PSHORT psWidth,
                                     HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioGetOrg(PSHORT psRow,
                          PSHORT psColumn,
                          HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioCreateLogFont(PFATTRS pfatattrs,
                                 LONG llcid,
                                 PSTR8 pName,
                                 HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioQuerySetIds(PLONG allcids,
                               PSTR8 pNames,
                               PLONG alTypes,
                               LONG lcount,
                               HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioSetOrg(SHORT sRow,
                          SHORT sColumn,
                          HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioQueryFonts(PLONG plRemfonts,
                              PFONTMETRICS afmMetrics,
                              LONG lMetricsLength,
                              PLONG plFonts,
                              PSZ pszFacename,
                              ULONG flOptions,
                              HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioSetDeviceCellSize(SHORT sHeight,
                                     SHORT sWidth,
                                     HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY VioShowPS(SHORT sDepth,
                          SHORT sWidth,
                          SHORT soffCell,
                          HVPS hvps)
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
