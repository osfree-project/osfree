// 16-bit api
#include <kal.h>

// 16-bit to 32-bit wrappers. 32-bit VIO subsystem lives in SUB32.DLL

APIRET unimplemented(char *func);

// 32-bit api
typedef USHORT HVPS;
typedef HVPS * PHVPS;

USHORT APIENTRY Vio32DeRegister(VOID);
USHORT APIENTRY Vio32Register(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFun1, const ULONG flFun2);
USHORT APIENTRY Vio32GlobalReg(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFun1, const ULONG flFun2, const USHORT usReturn);
USHORT APIENTRY Vio32GetAnsi(USHORT * Ansi, const HVIO Handle);
USHORT APIENTRY Vio32GetCp(const USHORT Reserved, USHORT * IdCodePage, const HVIO Handle);
USHORT APIENTRY Vio32GetCurPos(USHORT * Row, USHORT * Column, const HVIO Handle);
USHORT APIENTRY Vio32GetCurType(VIOCURSORINFO * CursorInfo, const HVIO Handle);
USHORT APIENTRY Vio32GetMode(VIOMODEINFO * ModeInfo, const HVIO Handle);
USHORT APIENTRY Vio32ReadCellStr(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY Vio32ReadCharStr(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY Vio32ScrollDown(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY Vio32ScrollDn(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY Vio32ScrollLeft(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY Vio32ScrollLf(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY Vio32ScrollRight(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY Vio32ScrollRt(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY Vio32ScrollUp(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle);
USHORT APIENTRY Vio32SetAnsi(const USHORT Ansi, const HVIO Handle);
USHORT APIENTRY Vio32SetCp(const USHORT Reserved, const USHORT IdCodePage, const HVIO Handle);
USHORT APIENTRY Vio32SetCurPos(const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY Vio32SetCurType(const PVIOCURSORINFO CursorInfo, const HVIO Handle);
USHORT APIENTRY Vio32SetMode(const PVIOMODEINFO ModeInfo, const HVIO hvio);
USHORT APIENTRY Vio32WrtCellStr(const PCHAR CellStr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY Vio32WrtCharStr(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY Vio32WrtCharStrAttr(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const PBYTE pAttr, const HVIO Handle);
USHORT APIENTRY Vio32WrtCharStrAtt(const PCH pch, const USHORT cb, const USHORT usRow, const USHORT usColumn, const PBYTE pAttr, const HVIO hvio);
USHORT APIENTRY Vio32WrtNAttr(const PBYTE Attr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY Vio32WrtNCell(const PBYTE Cell, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY Vio32WrtNChar(const PCHAR Char, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle);
USHORT APIENTRY Vio32WrtTTY(const PCHAR Str, const USHORT Count, const HVIO Handle);
USHORT APIENTRY Vio32GetState(const PVOID pState, const HVIO Handle);
USHORT APIENTRY Vio32SetState(const PVOID pState, const HVIO Handle);
USHORT APIENTRY Vio32GetConfig(const USHORT ConfigId, VIOCONFIGINFO * vioin, const HVIO hvio);
USHORT APIENTRY Vio32PopUp(USHORT * Options, const HVIO VioHandle);
USHORT APIENTRY Vio32EndPopUp(const HVIO VioHandle);
USHORT APIENTRY Vio32GetPhysBuf(VIOPHYSBUF * pvioPhysBuf, const USHORT usReserved);
USHORT APIENTRY Vio32PrtSc(const HVIO hvio);
USHORT APIENTRY Vio32PrtScToggle(const HVIO hvio);
USHORT APIENTRY Vio32ShowBuf(const USHORT offLVB, const USHORT cb, const HVIO hvio);
USHORT APIENTRY Vio32ScrLock(const USHORT fWait, UCHAR * pfNotLocked, const HVIO hvio);
USHORT APIENTRY Vio32ScrUnLock(const HVIO hvio);
USHORT APIENTRY Vio32SavRedrawWait(const USHORT usRedrawInd, USHORT * pNotifyType, const USHORT usReserved);
USHORT APIENTRY Vio32SavRedrawUndo(const USHORT usOwnerInd, const USHORT usKillInd, const USHORT usReserved);
USHORT APIENTRY Vio32ModeWait(const USHORT usReqType, USHORT * pNotifyType, const USHORT usReserved);
USHORT APIENTRY Vio32ModeUndo(const USHORT usOwnerInd, const USHORT usKillInd, const USHORT usReserved);
USHORT APIENTRY Vio32GetFont(VIOFONTINFO * pviofi, const HVIO hvio);
USHORT APIENTRY Vio32SetFont(VIOFONTINFO * pviofi, const HVIO hvio);
USHORT APIENTRY Vio32GetBuf(ULONG * pLVB, USHORT * pcbLVB, const HVIO hvio);
USHORT APIENTRY Vio32CheckCharType(USHORT * pType, const USHORT usRow, const USHORT usColumn, const HVIO hvio);
USHORT APIENTRY Vio32Associate(const HDC hdc, const HVPS hvps);
USHORT APIENTRY Vio32CreateLogFont(const PFATTRS pfatattrs, const LONG llcid, const PSTR8 pName, const HVPS hvps);
USHORT APIENTRY Vio32CreatePS(HVPS * phvps, const SHORT sdepth, const SHORT swidth, const SHORT sFormat, const SHORT sAttrs, const HVPS hvpsReserved);
USHORT APIENTRY Vio32DeleteSetId(const LONG llcid, const HVPS hvps);
USHORT APIENTRY Vio32DestroyPS(const HVPS hvps);
USHORT APIENTRY Vio32GetDeviceCellSize(SHORT * psHeight, SHORT * psWidth, const HVPS hvps);
USHORT APIENTRY Vio32GetOrg(SHORT * psRow, SHORT * psColumn, const HVPS hvps);
USHORT APIENTRY Vio32QueryFonts(LONG * plRemfonts, FONTMETRICS * afmMetrics, const LONG lMetricsLength, LONG * plFonts, const PSZ pszFacename, const ULONG flOptions, const HVPS hvps);
USHORT APIENTRY Vio32QuerySetIds(LONG * allcids, STR8 * pNames, LONG * alTypes, const LONG lcount, const HVPS hvps);
USHORT APIENTRY Vio32SetDeviceCellSize(const SHORT sHeight, const SHORT sWidth, const HVPS hvps);
USHORT APIENTRY Vio32SetOrg(const SHORT sRow, const SHORT sColumn, const HVPS hvps);
USHORT APIENTRY Vio32ShowPS(const SHORT sDepth, const SHORT sWidth, const SHORT soffCell, const HVPS hvps);
//MRESULT APIENTRY WinDefAVioWindowProc(const HWND hwnd, const USHORT msg, const ULONG mp1, const ULONG mp2);


USHORT APIENTRY16 VIOENDPOPUP(HVIO hvio)
{
  return Vio32EndPopUp(hvio);
}


USHORT APIENTRY16 VIOGETPHYSBUF(PVIOPHYSBUF pvioPhysBuf,
                              USHORT usReserved)
{
  return Vio32GetPhysBuf(pvioPhysBuf, usReserved);
}

/*

USHORT APIENTRY16 VIOGETANSI(USHORT * Ansi, const HVIO Handle)
{
  return Vio32GetAnsi(Ansi, Handle);
}

USHORT APIENTRY16 VIOSETANSI(const USHORT Ansi, const HVIO Handle)
{
  return Vio32SetAnsi(Ansi, Handle);
}

USHORT APIENTRY16 VIODEREGISTER(VOID)
{
  return Vio32DeRegister();
}

USHORT APIENTRY16 VIOSCROLLUP(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
  return Vio32ScrollUp(TopRow, LeftCol, BotRow, RightCol, Lines, Cell, Handle);
}

USHORT APIENTRY16 VIOPRTSC(HVIO hvio)
{
  return Vio32PrtSc(hvio);
}

USHORT APIENTRY16 VIOGETCURPOS(USHORT * Row, USHORT * Column, const HVIO Handle)
{
  return Vio32GetCurPos(Row, Column, Handle);
}

USHORT APIENTRY16 VIOWRTCELLSTR(const PCHAR CellStr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return Vio32WrtCellStr(CellStr, Count, Row, Column, Handle);
}

USHORT APIENTRY16 VIOPOPUP (PUSHORT pfWait,
                         HVIO hvio)
{
  return Vio32PopUp(pfWait, hvio);
}

USHORT APIENTRY16 VIOSCROLLRT(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle)
{
  return Vio32ScrollRt(TopRow, LeftCol, BotRow, RightCol, Columns, Cell, Handle);
}

USHORT APIENTRY16 VIOWRTCHARSTR(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return Vio32WrtCharStr(Str, Count, Row, Column, Handle);
}

USHORT APIENTRY16 VIOSETCURPOS(const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return Vio32SetCurPos(Row, Column, Handle);
}

USHORT APIENTRY16 VIOSCRUNLOCK(HVIO hvio)
{
  return Vio32ScrUnLock(hvio);
}

USHORT APIENTRY16 VIOGETMODE(VIOMODEINFO * ModeInfo, const HVIO Handle)
{
  return Vio32GetMode(ModeInfo, Handle);
}

USHORT APIENTRY16 VIOSETMODE(const PVIOMODEINFO ModeInfo, const HVIO hvio)
{
  return Vio32SetMode(ModeInfo, hvio);
}

USHORT APIENTRY16 VIOSCRLOCK(USHORT fWait,
                           PUCHAR pfNotLocked,
                           HVIO hvio)
{
  return Vio32ScrLock(fWait, pfNotLocked, hvio);
}

USHORT APIENTRY16 VIOREADCELLSTR(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return Vio32ReadCellStr(CellStr, Count, Row, Column, Handle);
}

USHORT APIENTRY16 VIOSAVREDRAWWAIT(USHORT usRedrawInd,
                                 PUSHORT pNotifyType,
                                 USHORT usReserved)
{
  return Vio32SavRedrawWait(usRedrawInd, pNotifyType, usReserved);
}

USHORT APIENTRY16 VIOWRTNATTR(const PBYTE Attr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return Vio32WrtNAttr(Attr, Count, Row, Column, Handle);
}

USHORT APIENTRY16 VIOGETCURTYPE(VIOCURSORINFO * CursorInfo, const HVIO Handle)
{
  return Vio32GetCurType(CursorInfo, Handle);
}

USHORT APIENTRY16 VIOSAVREDRAWUNDO(USHORT usOwnerInd,
                                 USHORT usKillInd,
                                 USHORT usReserved)
{
  return Vio32SavRedrawUndo(usOwnerInd, usKillInd, usReserved);
}

USHORT APIENTRY16 VIOGETFONT(PVIOFONTINFO pviofi,
                           HVIO hvio)
{
  return Vio32GetFont(pviofi, hvio);
}

USHORT APIENTRY16 VIOREADCHARSTR(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return Vio32ReadCharStr(CellStr, Count, Row, Column, Handle);
}

USHORT APIENTRY16 VIOGETBUF(PULONG pLVB,
                          PUSHORT pcbLVB,
                          HVIO hvio)
{
  return Vio32GetBuf(pLVB, pcbLVB, hvio);
}

USHORT APIENTRY16 VIOSETCURTYPE(const PVIOCURSORINFO CursorInfo, const HVIO Handle)
{
  return Vio32SetCurType(CursorInfo, Handle);
}

USHORT APIENTRY16 VIOSETFONT(PVIOFONTINFO pviofi,
                           HVIO hvio)
{
  return Vio32SetFont(pviofi, hvio);
}

USHORT APIENTRY16 VIOMODEUNDO (USHORT usOwnerInd,
                              USHORT usKillInd,
                              USHORT usReserved)
{
  return Vio32ModeUndo(usOwnerInd, usKillInd, usReserved);
}

USHORT APIENTRY16 VIOMODEWAIT (USHORT usReqType,
                            PUSHORT pNotifyType,
                            USHORT usReserved)
{
  return Vio32ModeWait(usReqType, pNotifyType, usReserved);
}

USHORT APIENTRY16 VIOGETCP(const USHORT Reserved, USHORT * IdCodePage, const HVIO Handle)
{
  return Vio32GetCp(Reserved, IdCodePage, Handle);
}

USHORT APIENTRY16 VIOSETCP(const USHORT Reserved, const USHORT IdCodePage, const HVIO Handle)
{
  return Vio32SetCp(Reserved, IdCodePage, Handle);
}

USHORT APIENTRY16 VIOSHOWBUF (USHORT offLVB,
                             USHORT cb,
                             HVIO hvio)
{
  return Vio32ShowBuf(offLVB, cb, hvio);
}

USHORT APIENTRY16 VIOSCROLLLF(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle)
{
  return Vio32ScrollLf(TopRow, LeftCol, BotRow, RightCol, Columns, Cell, Handle);
}

USHORT APIENTRY16 VIOREGISTER(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFun1, const ULONG flFun2)
{
  return Vio32Register(pszModName, pszEntryName, flFun1, flFun2);
}

USHORT APIENTRY16 VIOGETCONFIG(const USHORT ConfigId, VIOCONFIGINFO * vioin, const HVIO hvio)
{
  return Vio32GetConfig(ConfigId, vioin, hvio);
}

USHORT APIENTRY16 VIOSCROLLDN(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
  return Vio32ScrollDn(TopRow, LeftCol, BotRow, RightCol, Lines, Cell, Handle);
}

//USHORT APIENTRY16 VIOWRTCHARSTRATT(const PCCH pch, const USHORT cb, const USHORT usRow, const USHORT usColumn, const PBYTE pAttr, const HVIO hvio)
USHORT APIENTRY16 VIOWRTCHARSTRATT(void *pch, const USHORT cb, const USHORT usRow, const USHORT usColumn, const PBYTE pAttr, const HVIO hvio)
{
  return Vio32WrtCharStrAtt(pch, cb, usRow, usColumn, pAttr, hvio);
}

USHORT APIENTRY16 VIOGETSTATE(const PVOID pState, const HVIO Handle)
{
  return Vio32GetState(pState, Handle);
}

USHORT APIENTRY16 VIOPRTSCTOGGLE (HVIO hvio)
{
  return Vio32PrtScToggle(hvio);
}

USHORT APIENTRY16 VIOSETSTATE(const PVOID pState, const HVIO Handle)
{
  return Vio32SetState(pState, Handle);
}

USHORT APIENTRY16 VIOWRTNCELL(const PBYTE Cell, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return Vio32WrtNCell(Cell, Count, Row, Column, Handle);
}

USHORT APIENTRY16 VIOWRTNCHAR(const PCHAR Char, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return Vio32WrtNChar(Char, Count, Row, Column, Handle);
}

USHORT APIENTRY16 VIOASSOCIATE(HDC hdc,
                             HVPS hvps)
{
  return Vio32Associate(hdc, hvps);
}

USHORT APIENTRY16 VIOCREATEPS(PHVPS phvps,
                            SHORT sdepth,
                            SHORT swidth,
                            SHORT sFormat,
                            SHORT sAttrs,
                            HVPS hvpsReserved)
{
  return Vio32CreatePS(phvps, sdepth, swidth, sFormat, sAttrs, hvpsReserved);
}

USHORT APIENTRY16 VIODELETESETID(LONG llcid,
                               HVPS hvps)
{
  return Vio32DeleteSetId(llcid, hvps);
}

USHORT APIENTRY16 VIOGETDEVICECELLSIZE(PSHORT psHeight,
                                     PSHORT psWidth,
                                     HVPS hvps)
{
  return Vio32GetDeviceCellSize(psHeight, psWidth, hvps);
}

USHORT APIENTRY16 VIOGETORG(PSHORT psRow,
                          PSHORT psColumn,
                          HVPS hvps)
{
  return Vio32GetOrg(psRow, psColumn, hvps);
}

USHORT APIENTRY16 VIOCREATELOGFONT(PFATTRS pfatattrs,
                                 LONG llcid,
                                 PSTR8 pName,
                                 HVPS hvps)
{
  return Vio32CreateLogFont(pfatattrs, llcid, pName, hvps);
}

USHORT APIENTRY16 VIODESTROYPS(HVPS hvps)
{
  return Vio32DestroyPS(hvps);
}

USHORT APIENTRY16 VIOQUERYSETIDS(PLONG allcids,
                               PSTR8 pNames,
                               PLONG alTypes,
                               LONG lcount,
                               HVPS hvps)
{
  return Vio32QuerySetIds(allcids, pNames, alTypes, lcount, hvps);
}

USHORT APIENTRY16 VIOSETORG(SHORT sRow,
                          SHORT sColumn,
                          HVPS hvps)
{
  return Vio32SetOrg(sRow, sColumn, hvps);
}

USHORT APIENTRY16 VIOQUERYFONTS(PLONG plRemfonts,
                              PFONTMETRICS afmMetrics,
                              LONG lMetricsLength,
                              PLONG plFonts,
                              PSZ pszFacename,
                              ULONG flOptions,
                              HVPS hvps)
{
  return Vio32QueryFonts(plRemfonts, afmMetrics, lMetricsLength, plFonts, pszFacename, flOptions, hvps);
}

USHORT APIENTRY16 VIOSETDEVICECELLSIZE(SHORT sHeight,
                                     SHORT sWidth,
                                     HVPS hvps)
{
  return Vio32SetDeviceCellSize(sHeight, sWidth, hvps);
}

USHORT APIENTRY16 VIOSHOWPS(SHORT sDepth,
                          SHORT sWidth,
                          SHORT soffCell,
                          HVPS hvps)
{
  return Vio32ShowPS(sDepth, sWidth, soffCell, hvps);
}

USHORT APIENTRY16 VIOGLOBALREG(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFun1, const ULONG flFun2, const USHORT usReturn)
{
  return Vio32GlobalReg(pszModName, pszEntryName, flFun1, flFun2, usReturn);
}

USHORT APIENTRY16 VIOCHECKCHARTYPE (PUSHORT pType,
                                 USHORT usRow,
                                 USHORT usColumn,
                                 HVIO hvio)
{
  return Vio32CheckCharType(pType, usRow, usColumn, hvio);
}

*/
USHORT APIENTRY16 VIOWRTTTY(const PCHAR Str, const USHORT Count, const HVIO Handle)
{
  return Vio32WrtTTY(Str, Count, Handle);
}

/*
USHORT APIENTRY16 VIOFREE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 AVS_PRTSC(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 AVS_PRTSCTOGGLE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         VIOSRFBLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         VIOSRFUNBLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         VIOSAVE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         VIORESTORE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         VIOHETINIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         VIOSSWSWITCH(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         VIOSHELLINIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         VIOGETPSADDRESS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         VIOQUERYCONSOLE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         VIOREDRAWSIZE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         XVIOSETCASTATE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         XVIOCHECKCHARTYPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         XVIODESTROYCA(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         XVIOCREATECA(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         XVIOGETCASTATE(void)
{
  return unimplemented(__FUNCTION__);
}

*/

/*
  WinDefAVioWindowProc        .30, &
*/
