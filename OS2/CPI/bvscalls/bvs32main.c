#define INCL_VIO
#define INCL_PMAVIO
#include <os2.h>

#include <stdio.h>
#include <stdarg.h>
#include "bvs.h"
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


USHORT APIENTRY BvsGetPhysBuf (PVIOPHYSBUF pvioPhysBuf)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsGetBuf (PULONG pLVB,
                          PUSHORT pcbLVB,
                          HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsShowBuf (USHORT offLVB,
                             USHORT cb,
                             HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsGetCurPos(USHORT * Row, USHORT * Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsGetCurType(VIOCURSORINFO * CursorInfo, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsGetMode(VIOMODEINFO * ModeInfo, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsSetCurPos(const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsSetCurType(const PVIOCURSORINFO CursorInfo, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsSetMode(const PVIOMODEINFO ModeInfo, const HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsReadCharStr(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsReadCellStr(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsWrtNChar(const PCHAR Char, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsWrtNAttr(const PBYTE Attr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsWrtNCell(const PBYTE Cell, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsWrtCharStr(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsWrtCharStrAtt(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const PBYTE pAttr, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsWrtCellStr(const PCHAR CellStr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsScrollUp(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsScrollDn(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsScrollLf (USHORT usTopRow,
                            USHORT usLeftCol,
                            USHORT usBotRow,
                            USHORT usRightCol,
                            USHORT cbCol,
                            PBYTE pCell,
                            HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsScrollRt (USHORT usTopRow,
                            USHORT usLeftCol,
                            USHORT usBotRow,
                            USHORT usRightCol,
                            USHORT cbCol,
                            PBYTE pCell,
                            HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsSetAnsi(const USHORT Ansi, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsGetAnsi(USHORT * Ansi, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsPrtSc (HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsScrLock (USHORT fWait,
                           PUCHAR pfNotLocked,
                           HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsScrUnLock (HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsSavRedrawWait (USHORT usRedrawInd,
                                 PUSHORT pNotifyType)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsSavRedrawUndo (USHORT usOwnerInd,
                                 USHORT usKillInd)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsPopUp (PUSHORT pfWait,
                         HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsEndPopUp (HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsPrtScToggle (HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsModeWait (USHORT usReqType,
                            PUSHORT pNotifyType)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsModeUndo (USHORT usOwnerInd,
                              USHORT usKillInd)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsGetFont (PVIOFONTINFO pviofi,
                           HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsGetConfig(const USHORT ConfigId, VIOCONFIGINFO * vioin, const HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsSetCp(const USHORT IdCodePage, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsGetCp(USHORT * IdCodePage, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsSetFont (PVIOFONTINFO pviofi,
                           HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsGetState(const PVOID pState, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsSetState(const PVOID pState, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsWrtTTY(const PCHAR Str, const USHORT Count, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY BVS32Main(ULONG fn, PARGS pargs)
{
	APIRET rc=0;

	switch (fn)  
	{  
	case FN_GETPHYSBUF:
		rc=BvsGetPhysBuf(pargs->GetPhysBuf.pvioPhysBuf);
		break;
	case FN_GETBUF:
		rc=BvsGetBuf(pargs->GetBuf.pLVB, pargs->GetBuf.pcbLVB, pargs->GetBuf.hvio);
		break;
	case FN_SHOWBUF:
		rc=BvsShowBuf(pargs->ShowBuf.offLVB, pargs->ShowBuf.cb, pargs->ShowBuf.hvio);
		break;
	case FN_GETCURPOS:
		rc=BvsGetCurPos(pargs->GetCurPos.Row, pargs->GetCurPos.Column, pargs->GetCurPos.Handle);
		break;
	case FN_GETCURTYPE:
		rc=BvsGetCurType(pargs->GetCurType.CursorInfo, pargs->GetCurType.Handle);
		break;
	case FN_GETMODE:
		rc=BvsGetMode(pargs->GetMode.ModeInfo, pargs->GetMode.Handle);
		break;
	case FN_SETCURPOS:
		rc=BvsSetCurPos(pargs->SetCurPos.Row, pargs->SetCurPos.Column, pargs->SetCurPos.Handle);
		break;
	case FN_SETCURTYPE:
		rc=BvsSetCurType(pargs->SetCurType.CursorInfo, pargs->SetCurType.Handle);
		break;
	case FN_SETMODE:
		rc=BvsSetMode(pargs->SetMode.ModeInfo, pargs->SetMode.hvio);
		break;
	case FN_READCHARSTR:
		rc=BvsReadCharStr(pargs->ReadCharStr.CellStr, pargs->ReadCharStr.Count, pargs->ReadCharStr.Row, pargs->ReadCharStr.Column, pargs->ReadCharStr.Handle);
		break;
	case FN_READCELLSTR:
		rc=BvsReadCellStr(pargs->ReadCellStr.CellStr, pargs->ReadCellStr.Count, pargs->ReadCellStr.Row, pargs->ReadCellStr.Column, pargs->ReadCellStr.Handle);
		break;
	case FN_WRTNCHAR:
		rc=BvsWrtNChar(pargs->WrtNChar.Char, pargs->WrtNChar.Count, pargs->WrtNChar.Row, pargs->WrtNChar.Column, pargs->WrtNChar.Handle);
		break;
	case FN_WRTNATTR:
		rc=BvsWrtNAttr(pargs->WrtNAttr.Attr, pargs->WrtNAttr.Count, pargs->WrtNAttr.Row, pargs->WrtNAttr.Column, pargs->WrtNAttr.Handle);
		break;
	case FN_WRTNCELL:
		rc=BvsWrtNCell(pargs->WrtNCell.Cell, pargs->WrtNCell.Count, pargs->WrtNCell.Row, pargs->WrtNCell.Column, pargs->WrtNCell.Handle);
		break;
	case FN_WRTCHARSTR:
		rc=BvsWrtCharStr(pargs->WrtCharStr.Str, pargs->WrtCharStr.Count, pargs->WrtCharStr.Row, pargs->WrtCharStr.Column, pargs->WrtCharStr.Handle);
		break;
	case FN_WRTCHARSTRATT:
		rc=BvsWrtCharStrAtt(pargs->WrtCharStrAtt.Str, pargs->WrtCharStrAtt.Count, pargs->WrtCharStrAtt.Row, pargs->WrtCharStrAtt.Column, pargs->WrtCharStrAtt.pAttr, pargs->WrtCharStrAtt.Handle);
		break;
	case FN_WRTCELLSTR:
		rc=BvsWrtCellStr(pargs->WrtCellStr.CellStr, pargs->WrtCellStr.Count, pargs->WrtCellStr.Row, pargs->WrtCellStr.Column, pargs->WrtCellStr.Handle);
		break;
	case FN_WRTTTY:
		rc=BvsWrtTTY(pargs->WrtTTY.Str, pargs->WrtTTY.Count, pargs->WrtTTY.Handle);
		break;
	case FN_SCROLLUP:
		rc=BvsScrollUp(pargs->ScrollUp.TopRow, pargs->ScrollUp.LeftCol, pargs->ScrollUp.BotRow, pargs->ScrollUp.RightCol, pargs->ScrollUp.Lines, pargs->ScrollUp.Cell, pargs->ScrollUp.Handle);
		break;
	case FN_SCROLLDN:
		rc=BvsScrollDn(pargs->ScrollDn.TopRow, pargs->ScrollDn.LeftCol, pargs->ScrollDn.BotRow, pargs->ScrollDn.RightCol, pargs->ScrollDn.Lines, pargs->ScrollDn.Cell, pargs->ScrollDn.Handle);
		break;
	case FN_SCROLLLF:
		rc=BvsScrollLf(pargs->ScrollLf.usTopRow, pargs->ScrollLf.usLeftCol, pargs->ScrollLf.usBotRow, pargs->ScrollLf.usRightCol, pargs->ScrollLf.cbCol, pargs->ScrollLf.pCell, pargs->ScrollLf.hvio);
		break;
	case FN_SCROLLRT:
		rc=BvsScrollRt(pargs->ScrollRt.usTopRow, pargs->ScrollRt.usLeftCol, pargs->ScrollRt.usBotRow, pargs->ScrollRt.usRightCol, pargs->ScrollRt.cbCol, pargs->ScrollRt.pCell, pargs->ScrollRt.hvio);
		break;
	case FN_SETANSI:
		rc=BvsSetAnsi(pargs->SetAnsi.Ansi, pargs->SetAnsi.Handle);
		break;
	case FN_GETANSI:
		rc=BvsGetAnsi(pargs->GetAnsi.Ansi, pargs->GetAnsi.Handle);
		break;
	case FN_PRTSC:
		rc=BvsPrtSc(pargs->PrtSc.hvio);
		break;
	case FN_SCRLOCK:
		rc=BvsScrLock(pargs->ScrLock.fWait, pargs->ScrLock.pfNotLocked, pargs->ScrLock.hvio);
		break;
	case FN_SCRUNLOCK:
		rc=BvsScrUnLock(pargs->ScrUnLock.hvio);
		break;
	case FN_SAVREDRAWWAIT:
		rc=BvsSavRedrawWait(pargs->SavRedrawWait.usRedrawInd, pargs->SavRedrawWait.pNotifyType);
		break;
	case FN_SAVREDRAWUNDO:
		rc=BvsSavRedrawUndo(pargs->SavRedrawUndo.usOwnerInd, pargs->SavRedrawUndo.usKillInd);
		break;
	case FN_POPUP:
		rc=BvsPopUp(pargs->PopUp.pfWait, pargs->PopUp.hvio);
		break;
	case FN_ENDPOPUP:
		rc=BvsEndPopUp(pargs->EndPopUp.hvio);
		break;
	case FN_PRTSCTOGGLE:
		rc=BvsPrtScToggle(pargs->PrtScToggle.hvio);
		break;
	case FN_MODEWAIT:
		rc=BvsModeWait(pargs->ModeWait.usReqType, pargs->ModeWait.pNotifyType);
		break;
	case FN_MODEUNDO:
		rc=BvsModeUndo(pargs->ModeUndo.usOwnerInd, pargs->ModeUndo.usKillInd);
		break;
	case FN_GETFONT:
		rc=BvsGetFont(pargs->GetFont.pviofi, pargs->GetFont.hvio);
		break;
	case FN_GETCONFIG:
		rc=BvsGetConfig(pargs->GetConfig.ConfigId, pargs->GetConfig.vioin, pargs->GetConfig.hvio);
		break;
	case FN_SETCP:
		rc=BvsSetCp(pargs->SetCp.IdCodePage, pargs->SetCp.Handle);
		break;
	case FN_GETCP:
		rc=BvsGetCp(pargs->GetCp.IdCodePage, pargs->GetCp.Handle);
		break;
	case FN_SETFONT:
		rc=BvsSetFont(pargs->SetFont.pviofi, pargs->SetFont.hvio);
		break;
	case FN_GETSTATE:
		rc=BvsGetState(pargs->GetState.pState, pargs->GetState.Handle);
		break;
	case FN_SETSTATE:
		rc=BvsSetState(pargs->SetState.pState, pargs->SetState.Handle);
		break;
//        default: 
//          BVSError();
      }  
  return rc;
}

