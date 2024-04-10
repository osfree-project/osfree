#define INCL_VIO
#define INCL_PMAVIO
#include <os2.h>

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

// Function numbers for Base Video System main call
#define FN_GETPHYSBUF      0
#define FN_GETBUF          1
#define FN_SHOWBUF         2
#define FN_GETCURPOS       3
#define FN_GETCURTYPE      4
#define FN_GETMODE         5
#define FN_SETCURPOS       6
#define FN_SETCURTYPE      7
#define FN_SETMODE         8
#define FN_READCHARSTR     9
#define FN_READCELLSTR     10
#define FN_WRTNCHAR        11
#define FN_WRTNATTR        12
#define FN_WRTNCELL        13
#define FN_WRTCHARSTR      14
#define FN_WRTCHARSTRATT   15
#define FN_WRTCELLSTR      16
#define FN_WRTTTY          17
#define FN_SCROLLUP        18
#define FN_SCROLLDN        19
#define FN_SCROLLLF        20
#define FN_SCROLLRT        21
#define FN_SETANSI         22
#define FN_GETANSI         23
#define FN_PRTSC           24
#define FN_SCRLOCK         25
#define FN_SCRUNLOCK       26
#define FN_SAVREDRAWWAIT   27
#define FN_SAVREDRAWUNDO   28
#define FN_POPUP           29
#define FN_ENDPOPUP        30
#define FN_PRTSCTOGGLE     31
#define FN_MODEWAIT        32
#define FN_MODEUNDO        33
#define FN_GETFONT         34
#define FN_GETCONFIG       35
#define FN_SETCP           36
#define FN_GETCP           37
#define FN_SETFONT         38
#define FN_GETSTATE        39
#define FN_SETSTATE        40

typedef
union
{
struct 	{
			PVIOPHYSBUF pvioPhysBuf; 
		} GetPhysBuf;
struct 	{
			PULONG pLVB;
			PUSHORT pcbLVB;
			HVIO hvio; 
		} GetBuf;
struct	{
			USHORT offLVB;
			USHORT cb;
			HVIO hvio;
		} ShowBuf;
struct	{
			USHORT * Row;
			USHORT * Column;
			const HVIO Handle;
		} GetCurPos;
struct	{
			VIOCURSORINFO * CursorInfo;
			const HVIO Handle;
		} GetCurType;
struct	{
			VIOMODEINFO * ModeInfo;
			const HVIO Handle;
		} GetMode;
struct	{
			const USHORT Row;
			const USHORT Column;
			const HVIO Handle;
		} SetCurPos;
struct	{
			const PVIOCURSORINFO CursorInfo;
			const HVIO Handle;
		} SetCurType;
struct	{
			const PVIOMODEINFO ModeInfo;
			const HVIO hvio;
		} SetMode;
struct	{
			CHAR * CellStr;
			USHORT * Count;
			const USHORT Row;
			const USHORT Column;
			const HVIO Handle;
		} ReadCharStr;
struct	{
			CHAR * CellStr;
			USHORT * Count;
			const USHORT Row;
			const USHORT Column;
			const HVIO Handle;
		} ReadCellStr;
struct	{
			const PCHAR Char;
			const USHORT Count;
			const USHORT Row;
			const USHORT Column;
			const HVIO Handle;
		} WrtNChar;
struct	{
			const PBYTE Attr;
			const USHORT Count;
			const USHORT Row;
			const USHORT Column;
			const HVIO Handle;
		} WrtNAttr;
struct	{
			const PBYTE Cell;
			const USHORT Count;
			const USHORT Row;
			const USHORT Column;
			const HVIO Handle;
		} WrtNCell;
struct	{
			const PCHAR Str;
			const USHORT Count;
			const USHORT Row;
			const USHORT Column;
			const HVIO Handle;
		} WrtCharStr;
struct	{ 
			const PCHAR Str;
			const USHORT Count;
			const USHORT Row;
			const USHORT Column;
			const PBYTE pAttr;
			const HVIO Handle;
		} WrtCharStrAtt;
struct	{
			const PCHAR CellStr;
			const USHORT Count;
			const USHORT Row;
			const USHORT Column;
			const HVIO Handle;
		} WrtCellStr;
struct	{ 
			const USHORT TopRow;
			const USHORT LeftCol;
			const USHORT BotRow;
			const USHORT RightCol;
			const USHORT Lines;
			const PBYTE Cell;
			const HVIO Handle;
		} ScrollUp;
struct	{
			const USHORT TopRow;
			const USHORT LeftCol;
			const USHORT BotRow;
			const USHORT RightCol;
			const USHORT Lines;
			const PBYTE Cell;
			const HVIO Handle;
		} ScrollDn;
struct	{
			USHORT usTopRow;
			USHORT usLeftCol;
			USHORT usBotRow;
			USHORT usRightCol;
			USHORT cbCol;
			PBYTE pCell;
			HVIO hvio;
		} ScrollLf;
struct	{ 
			USHORT usTopRow;
			USHORT usLeftCol;
			USHORT usBotRow;
			USHORT usRightCol;
			USHORT cbCol;
			PBYTE pCell;
			HVIO hvio;
		} ScrollRt;
struct	{ 
			const USHORT Ansi;
			const HVIO Handle;
		} SetAnsi;
struct	{ 
			USHORT * Ansi;
			const HVIO Handle;
		} GetAnsi;
struct	{ 
			HVIO hvio;
		} PrtSc;
struct	{
			USHORT fWait;
			PUCHAR pfNotLocked;
			HVIO hvio;
		} ScrLock;
struct	{
			HVIO hvio;
		} ScrUnLock;
struct	{
			USHORT usRedrawInd;
			PUSHORT pNotifyType;
		} SavRedrawWait;
struct	{
			USHORT usOwnerInd;
			USHORT usKillInd;
		} SavRedrawUndo;
struct	{ 
			PUSHORT pfWait;
			HVIO hvio;
		} PopUp;
struct	{ 
			HVIO hvio;
		} EndPopUp;
struct	{ 
			HVIO hvio;
		} PrtScToggle;
struct	{
			USHORT usReqType;
			PUSHORT pNotifyType;
		} ModeWait;	
struct	{ 
			USHORT usOwnerInd;
			USHORT usKillInd;
		} ModeUndo;
struct	{
			PVIOFONTINFO pviofi;
			HVIO hvio;
		} GetFont;
struct	{
			const USHORT ConfigId;
			VIOCONFIGINFO * vioin;
			const HVIO hvio;
		} GetConfig;
struct	{ 
			const USHORT IdCodePage;
			const HVIO Handle;
		} SetCp;
struct	{ 
			USHORT * IdCodePage;
			const HVIO Handle;
		} GetCp;
struct	{ 
			PVIOFONTINFO pviofi;
			HVIO hvio;
		} SetFont;
struct	{ 
			const PVOID pState;
			const HVIO Handle;
		} GetState;
struct	{ 
			const PVOID pState;
			const HVIO Handle;
		} SetState;
struct	{ 
			const PCHAR Str;
			const USHORT Count;
			const HVIO Handle;
		} WrtTTY;
} ARGS, * PARGS;

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

