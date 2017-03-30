/* VioCalls */

#define  INCL_VIO
//#define  INCL_AVIO
#define  INCL_PMAVIO
#include <os2.h>


USHORT __pascal VIOREGISTER(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFun1, const ULONG flFun2)
{
  return VioRegister(pszModName, pszEntryName, flFun1, flFun2);
}


USHORT __pascal VIODEREGISTER(VOID)
{
  return VioDeRegister();
}


USHORT __pascal VIOGLOBALREG(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFun1, const ULONG flFun2, const USHORT usReturn)
{
  return VioGlobalReg(pszModName, pszEntryName, flFun1, flFun2, usReturn);
}


USHORT __pascal VIOGETANSI(USHORT * Ansi, const HVIO Handle)
{
  return VioGetAnsi(Ansi, Handle);
}


USHORT __pascal VIOGETCP(const USHORT Reserved, USHORT * IdCodePage, const HVIO Handle)
{
  return VioGetCp(Reserved, IdCodePage, Handle);
}


USHORT __pascal VIOGETCURPOS(USHORT * Row, USHORT * Column, const HVIO Handle)
{
  return VioGetCurPos(Row, Column, Handle);
}


USHORT __pascal VIOGETCURTYPE(VIOCURSORINFO * CursorInfo, const HVIO Handle)
{
  return VioGetCurType(CursorInfo, Handle);
}


USHORT __pascal VIOGETMODE(VIOMODEINFO * ModeInfo, const HVIO Handle)
{
  return VioGetMode(ModeInfo, Handle);
}


USHORT __pascal VIOREADCELLSTR(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return VioReadCellStr(CellStr, Count, Row, Column, Handle);
}


USHORT __pascal VIOREADCHARSTR(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return VioReadCharStr(CellStr, Count, Row, Column, Handle);
}


USHORT __pascal VIOSCROLLDN(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
  return VioScrollDn(TopRow, LeftCol, BotRow, RightCol, Lines, Cell, Handle);
}


USHORT __pascal VIOSCROLLLF(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle)
{
  return VioScrollLf(TopRow, LeftCol, BotRow, RightCol, Columns, Cell, Handle);
}


USHORT __pascal VIOSCROLLRT(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle)
{
  return VioScrollRt(TopRow, LeftCol, BotRow, RightCol, Columns, Cell, Handle);
}


USHORT __pascal VIOSCROLLUP(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
  return VioScrollUp(TopRow, LeftCol, BotRow, RightCol, Lines, Cell, Handle);
}


USHORT __pascal VIOSETANSI(const USHORT Ansi, const HVIO Handle)
{
  return VioSetAnsi(Ansi, Handle);
}


USHORT __pascal VIOSETCP(const USHORT Reserved, const USHORT IdCodePage, const HVIO Handle)
{
  return VioSetCp(Reserved, IdCodePage, Handle);
}


USHORT __pascal VIOSETCURPOS(const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return VioSetCurPos(Row, Column, Handle);
}


USHORT __pascal VIOSETCURTYPE(const PVIOCURSORINFO CursorInfo, const HVIO Handle)
{
  return VioSetCurType(CursorInfo, Handle);
}


USHORT __pascal VIOSETMODE(const PVIOMODEINFO ModeInfo, const HVIO hvio)
{
  return VioSetMode(ModeInfo, hvio);
}


USHORT __pascal VIOWRTCELLSTR(const PCHAR CellStr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return VioWrtCellStr(CellStr, Count, Row, Column, Handle);
}


USHORT __pascal VIOWRTCHARSTR(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return VioWrtCharStr(Str, Count, Row, Column, Handle);
}


USHORT __pascal VIOWRTCHARSTRATT(const PCHAR pch, const USHORT cb, const USHORT usRow, const USHORT usColumn, const PBYTE pAttr, const HVIO hvio)
{
  return VioWrtCharStrAtt(pch, cb, usRow, usColumn, pAttr, hvio);
}


USHORT __pascal VIOWRTNATTR(const PBYTE Attr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return VioWrtNAttr(Attr, Count, Row, Column, Handle);
}


USHORT __pascal VIOWRTNCELL(const PBYTE Cell, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return VioWrtNCell(Cell, Count, Row, Column, Handle);
}


USHORT __pascal VIOWRTNCHAR(const PCHAR Char, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return VioWrtNChar(Char, Count, Row, Column, Handle);
}


USHORT __pascal VIOWRTTTY(const PCHAR Str, const USHORT Count, const HVIO Handle)
{
  return VioWrtTTY(Str, Count, Handle);
}

USHORT __pascal VIOGETSTATE(const PVOID pState, const HVIO Handle)
{
  return VioGetState(pState, Handle);
}


USHORT __pascal VIOSETSTATE(const PVOID pState, const HVIO Handle)
{
  return VioSetState(pState, Handle);
}


USHORT __pascal VIOGETCONFIG(const USHORT ConfigId, VIOCONFIGINFO * vioin, const HVIO hvio)
{
  return VioGetConfig(ConfigId, vioin, hvio);
}


USHORT __pascal VIOPOPUP (PUSHORT pfWait,
                         HVIO hvio)
{
  return VioPopUp (pfWait, hvio);
}


USHORT __pascal VIOENDPOPUP (HVIO hvio)
{
  return VioEndPopUp(hvio);
}


USHORT __pascal VIOGETPHYSBUF (PVIOPHYSBUF pvioPhysBuf,
                              USHORT usReserved)
{
  return VioGetPhysBuf(pvioPhysBuf, usReserved);
}


USHORT __pascal VIOPRTSC(HVIO hvio)
{
  return VioPrtSc(hvio);
}


USHORT __pascal VIOSCRLOCK (USHORT fWait,
                           PUCHAR pfNotLocked,
                           HVIO hvio)
{
  return VioScrLock(fWait, pfNotLocked, hvio);
}


USHORT __pascal VIOSCRUNLOCK (HVIO hvio)
{
  return VioScrUnLock(hvio);
}


USHORT __pascal VIOSAVREDRAWWAIT (USHORT usRedrawInd,
                                 PUSHORT pNotifyType,
                                 USHORT usReserved)
{
  return VioSavRedrawWait(usRedrawInd, pNotifyType, usReserved);
}


USHORT __pascal VIOSAVREDRAWUNDO (USHORT usOwnerInd,
                                 USHORT usKillInd,
                                 USHORT usReserved)
{
  return VioSavRedrawUndo(usOwnerInd, usKillInd, usReserved);
}


USHORT __pascal VIOGETFONT (PVIOFONTINFO pviofi,
                           HVIO hvio)
{
  return VioGetFont(pviofi, hvio);
}


USHORT __pascal VIOSETFONT (PVIOFONTINFO pviofi,
                           HVIO hvio)
{
  return VioSetFont(pviofi, hvio);
}


USHORT __pascal VIOGETBUF (PULONG pLVB,
                          PUSHORT pcbLVB,
                          HVIO hvio)
{
  return VioGetBuf(pLVB, pcbLVB, hvio);
}


USHORT __pascal VIOMODEWAIT (USHORT usReqType,
                            PUSHORT pNotifyType,
                            USHORT usReserved)
{
  return VioModeWait(usReqType, pNotifyType, usReserved);
}


USHORT __pascal VIOMODEUNDO (USHORT usOwnerInd,
                              USHORT usKillInd,
                              USHORT usReserved)
{
  return VioModeUndo(usOwnerInd, usKillInd, usReserved);
}


USHORT __pascal VIOSHOWBUF (USHORT offLVB,
                             USHORT cb,
                             HVIO hvio)
{
  return VioShowBuf(offLVB, cb, hvio);
}


USHORT __pascal VIOPRTSCTOGGLE (HVIO hvio)
{
  return VioPrtScToggle(hvio);
}


USHORT __pascal VIOCHECKCHARTYPE (PUSHORT pType,
                                 USHORT usRow,
                                 USHORT usColumn,
                                 HVIO hvio)
{
  return VioCheckCharType(pType, usRow, usColumn, hvio);
}


/* AVIO */


USHORT __pascal VIOASSOCIATE(HDC hdc,
                             HVPS hvps)
{
  return VioAssociate(hdc, hvps);
}


USHORT __pascal VIOCREATEPS(PHVPS phvps,
                            SHORT sdepth,
                            SHORT swidth,
                            SHORT sFormat,
                            SHORT sAttrs,
                            HVPS hvpsReserved)
{
  return VioCreatePS(phvps, sdepth, swidth, sFormat, sAttrs, hvpsReserved);
}


USHORT __pascal VIODESTROYPS(HVPS hvps)
{
  return VioDestroyPS(hvps);
}


USHORT __pascal VIODELETESETID(LONG llcid,
                               HVPS hvps)
{
  return VioDeleteSetId(llcid, hvps);
}


USHORT __pascal VIOGETDEVICECELLSIZE(PSHORT psHeight,
                                     PSHORT psWidth,
                                     HVPS hvps)
{
  return VioGetDeviceCellSize(psHeight, psWidth, hvps);
}


USHORT __pascal VIOGETORG(PSHORT psRow,
                          PSHORT psColumn,
                          HVPS hvps)
{
  return VioGetOrg(psRow, psColumn, hvps);
}


USHORT __pascal VIOCREATELOGFONT(PFATTRS pfatattrs,
                                 LONG llcid,
                                 PSTR8 pName,
                                 HVPS hvps)
{
  return VioCreateLogFont(pfatattrs, llcid, pName, hvps);
}


USHORT __pascal VIOQUERYSETIDS(PLONG allcids,
                               PSTR8 pNames,
                               PLONG alTypes,
                               LONG lcount,
                               HVPS hvps)
{
  return VioQuerySetIds(allcids, pNames, alTypes, lcount, hvps);
}


USHORT __pascal VIOSETORG(SHORT sRow,
                          SHORT sColumn,
                          HVPS hvps)
{
  return VioSetOrg(sRow, sColumn, hvps);
}


USHORT __pascal VIOQUERYFONTS(PLONG plRemfonts,
                              PFONTMETRICS afmMetrics,
                              LONG lMetricsLength,
                              PLONG plFonts,
                              PSZ pszFacename,
                              ULONG flOptions,
                              HVPS hvps)
{
  return VioQueryFonts(plRemfonts, afmMetrics, lMetricsLength,
                       plFonts, pszFacename, flOptions, hvps);
}


USHORT __pascal VIOSETDEVICECELLSIZE(SHORT sHeight,
                                     SHORT sWidth,
                                     HVPS hvps)
{
  return VioSetDeviceCellSize(sHeight, sWidth, hvps);
}


USHORT __pascal VIOSHOWPS(SHORT sDepth,
                          SHORT sWidth,
                          SHORT soffCell,
                          HVPS hvps)
{
  return VioShowPS(sDepth, sWidth, soffCell, hvps);
}
