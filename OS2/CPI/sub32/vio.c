#define INCL_VIO
#define INCL_PMAVIO
#include <os2.h>

APIRET unimplemented(char *func);
//APIRET APIENTRY BVSMain(ULONG fn, ARGS args);

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
			HVIO Handle;
		} GetCurPos;
struct	{
			VIOCURSORINFO * CursorInfo;
			HVIO Handle;
		} GetCurType;
struct	{
			VIOMODEINFO * ModeInfo;
			HVIO Handle;
		} GetMode;
struct	{
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} SetCurPos;
struct	{
			PVIOCURSORINFO CursorInfo;
			HVIO Handle;
		} SetCurType;
struct	{
			PVIOMODEINFO ModeInfo;
			HVIO hvio;
		} SetMode;
struct	{
			CHAR * CellStr;
			USHORT * Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} ReadCharStr;
struct	{
			CHAR * CellStr;
			USHORT * Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} ReadCellStr;
struct	{
			PCHAR Char;
			USHORT Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} WrtNChar;
struct	{
			PBYTE Attr;
			USHORT Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} WrtNAttr;
struct	{
			PBYTE Cell;
			USHORT Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} WrtNCell;
struct	{
			PCHAR Str;
			USHORT Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} WrtCharStr;
struct	{ 
			PCHAR Str;
			USHORT Count;
			USHORT Row;
			USHORT Column;
			PBYTE pAttr;
			HVIO Handle;
		} WrtCharStrAtt;
struct	{
			PCHAR CellStr;
			USHORT Count;
			USHORT Row;
			USHORT Column;
			HVIO Handle;
		} WrtCellStr;
struct	{ 
			USHORT TopRow;
			USHORT LeftCol;
			USHORT BotRow;
			USHORT RightCol;
			USHORT Lines;
			PBYTE Cell;
			HVIO Handle;
		} ScrollUp;
struct	{
			USHORT TopRow;
			USHORT LeftCol;
			USHORT BotRow;
			USHORT RightCol;
			USHORT Lines;
			PBYTE Cell;
			HVIO Handle;
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
			USHORT Ansi;
			HVIO Handle;
		} SetAnsi;
struct	{ 
			USHORT * Ansi;
			HVIO Handle;
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
			USHORT ConfigId;
			VIOCONFIGINFO * vioin;
			HVIO hvio;
		} GetConfig;
struct	{ 
			USHORT IdCodePage;
			HVIO Handle;
		} SetCp;
struct	{ 
			USHORT * IdCodePage;
			HVIO Handle;
		} GetCp;
struct	{ 
			PVIOFONTINFO pviofi;
			HVIO hvio;
		} SetFont;
struct	{ 
			PVOID pState;
			HVIO Handle;
		} GetState;
struct	{ 
			PVOID pState;
			HVIO Handle;
		} SetState;
struct	{ 
			PCHAR Str;
			USHORT Count;
			HVIO Handle;
		} WrtTTY;
} ARGS, * PARGS;

// Call VIO function
APIRET APIENTRY VioCall(ULONG fn, PARGS pargs)
{
  return 0; //BVSMain(fn);
}

USHORT APIENTRY VioEndPopUp (HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY VioInit(VOID) 
{
 PSZ      ModuleName     = "BVSCALLS.DLL";  /* Name of module   */
 UCHAR    LoadError[256] = "";          /* Area for Load failure information */
 HMODULE  ModuleHandle   = NULLHANDLE;  /* Module handle                     */
 PFN      ModuleAddr     = 0;           /* Pointer to a system function      */
 APIRET   rc             = NO_ERROR;    /* Return code                       */
   rc = DosLoadModule(LoadError,               /* Failure information buffer */
                      sizeof(LoadError),       /* Size of buffer             */
                      ModuleName,              /* Module to load             */
                      &ModuleHandle);          /* Module handle returned     */
   if (rc != NO_ERROR) {
      //printf("DosLoadModule error: return code = %u\n", rc);
      return 1;
   }
   
   // Find 32-bit version
   rc = DosQueryProcAddr(ModuleHandle,         /* Handle to module           */
                         0,                    /* ProcName specified      */
                         "BVS32MAIN",            /* ProcName (not specified)   */
                         &ModuleAddr);         /* Address returned           */
						 
   if (rc == ERROR_INVALID_NAME) 
   { // if no 32bit, try 16-bit
     rc = DosQueryProcAddr(ModuleHandle,         /* Handle to module           */
                           0,                    /* ProcName specified      */
                           "BVSMAIN",            /* ProcName (not specified)   */
                           &ModuleAddr);         /* Address returned           */
	 
   };
   
   if (!rc) rc = DosFreeModule(ModuleHandle);
   
   return NO_ERROR;
}

/*
32-bit versions
USHORT VioCheckCharType (PUSHORT pType, USHORT usRow, USHORT usColumn,
    HVIO hvio);
USHORT VioDeRegister (VOID);
USHORT VioGetAnsi (PUSHORT pfAnsi, HVIO hvio);
USHORT VioGetBuf (PULONG pLVB, PUSHORT pcbLVB, HVIO hvio);
USHORT VioGetConfig (USHORT usConfigId, PVIOCONFIGINFO pvioin, HVIO hvio);
USHORT VioGetCp (USHORT usReserved, PUSHORT pusCodePage, HVIO hvio);
USHORT VioGetCurPos (PUSHORT pusRow, PUSHORT pusColumn, HVIO hvio);
USHORT VioGetCurType (PVIOCURSORINFO pvioCursorInfo, HVIO hvio);
USHORT VioGetFont (PVIOFONTINFO pviofi, HVIO hvio);
USHORT VioGetMode (PVIOMODEINFO pvioModeInfo, HVIO hvio);
USHORT VioGetPhysBuf (PVIOPHYSBUF pvioPhysBuf, USHORT usReserved);
USHORT VioGetState (PVOID pState, HVIO hvio);
USHORT VioGlobalReg (PCSZ pszModName, PCSZ pszEntryName, ULONG ulFunMask1,
    ULONG ulFunMask2, USHORT usReturn);
USHORT VioModeUndo (USHORT usOwnerInd, USHORT usKillInd, USHORT usReserved);
USHORT VioModeWait (USHORT usReqType, PUSHORT pNotifyType, USHORT usReserved);
USHORT VioPopUp (PUSHORT pfWait, HVIO hvio);
USHORT VioPrtSc (HVIO hvio);
USHORT VioPrtScToggle (HVIO hvio);
USHORT VioReadCellStr (PCH pchCellStr, PUSHORT pcb, USHORT usRow,
    USHORT usColumn, HVIO hvio);
USHORT VioReadCharStr (PCH pch, PUSHORT pcb, USHORT usRow, USHORT usColumn,
    HVIO hvio);
USHORT VioRegister (PCSZ pszModName, PCSZ pszEntryName, ULONG ulFunMask1,
    ULONG ulFunMask2);
USHORT VioSavRedrawUndo (USHORT usOwnerInd, USHORT usKillInd,
    USHORT usReserved);
USHORT VioSavRedrawWait (USHORT usRedrawInd, PUSHORT pusNotifyType,
    USHORT usReserved);
USHORT VioScrLock (USHORT fWait, PUCHAR pfNotLocked, HVIO hvio);
USHORT VioScrollDn (USHORT usTopRow, USHORT usLeftCol, USHORT usBotRow,
    USHORT usRightCol, USHORT cbLines, PBYTE pCell, HVIO hvio);
USHORT VioScrollLf (USHORT usTopRow, USHORT usLeftCol, USHORT usBotRow,
    USHORT usRightCol, USHORT cbCol, PBYTE pCell, HVIO hvio);
USHORT VioScrollRt (USHORT usTopRow, USHORT usLeftCol, USHORT usBotRow,
    USHORT usRightCol, USHORT cbCol, PBYTE pCell, HVIO hvio);
USHORT VioScrollUp (USHORT usTopRow, USHORT usLeftCol, USHORT usBotRow,
    USHORT usRightCol, USHORT cbLines, PBYTE pCell, HVIO hvio);
USHORT VioScrUnLock (HVIO hvio);
USHORT VioSetAnsi (USHORT fAnsi, HVIO hvio);
USHORT VioSetCp (USHORT usReserved, USHORT usCodePage, HVIO hvio);
USHORT VioSetCurPos (USHORT usRow, USHORT usColumn, HVIO hvio);
USHORT VioSetCurType (PVIOCURSORINFO pvioCursorInfo, HVIO hvio);
USHORT VioSetFont (PVIOFONTINFO pviofi, HVIO hvio);
USHORT VioSetMode (PVIOMODEINFO pvioModeInfo, HVIO hvio);
USHORT VioSetState (CPVOID pState, HVIO hvio);
USHORT VioShowBuf (USHORT offLVB, USHORT cb, HVIO hvio);
USHORT VioWrtCellStr (PCCH pchCellStr, USHORT cb, USHORT usRow,
    USHORT usColumn, HVIO hvio);
USHORT VioWrtCharStr (PCCH pch, USHORT cb, USHORT usRow, USHORT usColumn,
    HVIO hvio);
USHORT VioWrtCharStrAtt (PCCH pch, USHORT cb, USHORT usRow, USHORT usColumn,
    PBYTE pAttr, HVIO hvio);
USHORT VioWrtNAttr (__const__ BYTE *pAttr, USHORT cb, USHORT usRow,
    USHORT usColumn, HVIO hvio);
USHORT VioWrtNCell (__const__ BYTE *pCell, USHORT cb, USHORT usRow,
    USHORT usColumn, HVIO hvio);
USHORT VioWrtNChar (PCCH pch, USHORT cb, USHORT usRow, USHORT usColumn,
    HVIO hvio);
USHORT VioWrtTTY (PCCH pch, USHORT cb, HVIO hvio);
 */


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
	ARGS args;
	
	args.GetAnsi.Ansi=Ansi;
	args.GetAnsi.Handle=Handle;
	
	return VioCall(FN_SETANSI, &args);
}


USHORT APIENTRY VioGetCp(const USHORT Reserved, USHORT * IdCodePage, const HVIO Handle)
{
	ARGS args;
	
	args.GetCp.IdCodePage=IdCodePage;
	args.GetCp.Handle=Handle;
	
	return VioCall(FN_GETCP, &args);
}


USHORT APIENTRY VioGetCurPos(USHORT * Row, USHORT * Column, const HVIO Handle)
{
	ARGS args;
	
	args.GetCurPos.Row=Row;
	args.GetCurPos.Column=Column;
	args.GetCurPos.Handle=Handle;
  
	return VioCall(FN_GETCURPOS, &args);
}


USHORT APIENTRY VioGetCurType(VIOCURSORINFO * CursorInfo, const HVIO Handle)
{
	ARGS args;
	
	args.GetCurType.CursorInfo=CursorInfo;
	args.GetCurType.Handle=Handle;
	
	return VioCall(FN_GETCURTYPE, &args);
}


USHORT APIENTRY VioGetMode(VIOMODEINFO * ModeInfo, const HVIO Handle)
{
	ARGS args;
	
	args.GetMode.ModeInfo=ModeInfo;
	args.GetMode.Handle=Handle;
	
	return VioCall(FN_GETMODE, &args);
}


USHORT APIENTRY VioReadCellStr(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
	ARGS args;
	
	args.ReadCellStr.CellStr=CellStr;
	args.ReadCellStr.Count=Count;
	args.ReadCellStr.Row=Row;
	args.ReadCellStr.Column=Column;
	args.ReadCellStr.Handle=Handle;
	
	return VioCall(FN_READCELLSTR, &args);
}


USHORT APIENTRY VioReadCharStr(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
	ARGS args;
	
	args.ReadCharStr.CellStr=CellStr;
	args.ReadCharStr.Count=Count;
	args.ReadCharStr.Row=Row;
	args.ReadCharStr.Column=Column;
	args.ReadCharStr.Handle=Handle;
	
	return VioCall(FN_READCHARSTR, &args);
}


USHORT APIENTRY VioScrollDn(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
	ARGS args;
	
	args.ScrollDn.TopRow=TopRow;
	args.ScrollDn.LeftCol=LeftCol;
	args.ScrollDn.BotRow=BotRow;
	args.ScrollDn.RightCol=RightCol;
	args.ScrollDn.Lines=Lines;
	args.ScrollDn.Cell=Cell;
	args.ScrollDn.Handle=Handle;
	
	return VioCall(FN_SCROLLDN, &args);
}


USHORT APIENTRY VioScrollLf(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle)
{
	ARGS args;


	args.ScrollLf.usTopRow=TopRow;
	args.ScrollLf.usLeftCol=LeftCol;
	args.ScrollLf.usBotRow=BotRow;
	args.ScrollLf.usRightCol=RightCol;
	args.ScrollLf.cbCol=Columns;
	args.ScrollLf.pCell=Cell;
	args.ScrollLf.hvio=Handle;
	
	return VioCall(FN_SCROLLLF, &args);
}


USHORT APIENTRY VioScrollUp(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
	ARGS args;
	
	args.ScrollUp.TopRow=TopRow;
	args.ScrollUp.LeftCol=LeftCol;
	args.ScrollUp.BotRow=BotRow;
	args.ScrollUp.RightCol=RightCol;
	args.ScrollUp.Lines=Lines;
	args.ScrollUp.Cell=Cell;
	args.ScrollUp.Handle=Handle;
	
	return VioCall(FN_SCROLLUP, &args);
}


USHORT APIENTRY VioSetAnsi(const USHORT Ansi, const HVIO Handle)
{
	ARGS args;
	
	args.SetAnsi.Ansi=Ansi;
	args.SetAnsi.Handle=Handle;
	
	return VioCall(FN_SETANSI, &args);
}


USHORT APIENTRY VioSetCp(const USHORT Reserved, const USHORT IdCodePage, const HVIO Handle)
{
	ARGS args;
	
	args.SetCp.IdCodePage=IdCodePage;
	args.SetCp.Handle=Handle;
  
	return VioCall(FN_SETCP, &args);
}


USHORT APIENTRY VioSetCurPos(const USHORT Row, const USHORT Column, const HVIO Handle)
{
	ARGS args;
	
	args.SetCurPos.Row=Row;
	args.SetCurPos.Column=Column;
	args.SetCurPos.Handle=Handle;
	
	return VioCall(FN_SETCURPOS, &args);
}


USHORT APIENTRY VioSetCurType(const PVIOCURSORINFO CursorInfo, const HVIO Handle)
{
	ARGS args;
	
	args.SetCurType.CursorInfo=CursorInfo;
	args.SetCurType.Handle=Handle;
	
	return VioCall(FN_SETCURTYPE, &args);
}


USHORT APIENTRY VioSetMode(const PVIOMODEINFO ModeInfo, const HVIO hvio)
{
	ARGS args;
	
	args.SetMode.ModeInfo=ModeInfo;
	args.SetMode.hvio=hvio;
	
	return VioCall(FN_SETMODE, &args);
}


USHORT APIENTRY VioWrtCellStr(const PCHAR CellStr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
	ARGS args;
	
	args.WrtCellStr.CellStr=CellStr;
	args.WrtCellStr.Count=Count;
	args.WrtCellStr.Row=Row;
	args.WrtCellStr.Column=Column;
	args.WrtCellStr.Handle=Handle;
	
	return VioCall(FN_WRTCELLSTR, &args);
}


USHORT APIENTRY VioWrtCharStr(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
	ARGS args;
	
	args.WrtCharStr.Str=Str;
	args.WrtCharStr.Count=Count;
	args.WrtCharStr.Row=Row;
	args.WrtCharStr.Column=Column;
	args.WrtCharStr.Handle=Handle;
	
	return VioCall(FN_WRTCHARSTR, &args);
}


USHORT APIENTRY VioWrtCharStrAtt(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const PBYTE pAttr, const HVIO Handle)
{
	ARGS args;
	
	args.WrtCharStrAtt.Str=Str;
	args.WrtCharStrAtt.Count=Count;
	args.WrtCharStrAtt.Row=Row;
	args.WrtCharStrAtt.Column=Column;
	args.WrtCharStrAtt.pAttr=pAttr;
	args.WrtCharStrAtt.Handle=Handle;
  
	return VioCall(FN_WRTCHARSTRATT, &args);
}


USHORT APIENTRY VioWrtNAttr(const PBYTE Attr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
	ARGS args;
	
	args.WrtNAttr.Attr=Attr;
	args.WrtNAttr.Count=Count;
	args.WrtNAttr.Row=Row;
	args.WrtNAttr.Column=Column;
	args.WrtNAttr.Handle=Handle;
	
	return VioCall(FN_WRTNATTR, &args);
}


USHORT APIENTRY VioWrtNCell(const PBYTE Cell, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
	ARGS args;
	
	args.WrtNCell.Cell=Cell;
	args.WrtNCell.Count=Count;
	args.WrtNCell.Row=Row;
	args.WrtNCell.Column=Column;
	args.WrtNCell.Handle=Handle;
	
	return VioCall(FN_WRTNCELL, &args);
}


USHORT APIENTRY VioWrtNChar(const PCHAR Char, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
	ARGS args;
	
	args.WrtNChar.Char=Char;
	args.WrtNChar.Count=Count;
	args.WrtNChar.Row=Row;
	args.WrtNChar.Column=Column;
	args.WrtNChar.Handle=Handle;
	
	return VioCall(FN_WRTNCHAR, &args);
}


//USHORT APIENTRY VioWrtTTY(const PCHAR Str, const USHORT Count, const HVIO Handle)


USHORT APIENTRY VioGetState(const PVOID pState, const HVIO Handle)
{
	ARGS args;
	
	args.GetState.pState=pState;
	args.GetState.Handle=Handle;
	
	return VioCall(FN_GETSTATE, &args);
}


USHORT APIENTRY VioSetState(const PVOID pState, const HVIO Handle)
{
	ARGS args;
	
	args.SetState.pState=pState;
	args.SetState.Handle=Handle;
	
	return VioCall(FN_SETSTATE, &args);
}


USHORT APIENTRY VioGetConfig(const USHORT ConfigId, VIOCONFIGINFO * vioin, const HVIO hvio)
{
	ARGS args;
	
	args.GetConfig.ConfigId=ConfigId;
	args.GetConfig.vioin=vioin;
	args.GetConfig.hvio=hvio;
	
	return VioCall(FN_GETCONFIG, &args);
}


USHORT APIENTRY VioPopUp (PUSHORT pfWait,
                         HVIO hvio)
{
	ARGS args;
	
	args.PopUp.pfWait=pfWait;
	args.PopUp.hvio=hvio;
	
	return VioCall(FN_POPUP, &args);
}


USHORT APIENTRY VioGetPhysBuf (PVIOPHYSBUF pvioPhysBuf,
                              USHORT usReserved)
{
	ARGS args;
	
	args.GetPhysBuf.pvioPhysBuf=pvioPhysBuf;
	
	return VioCall(FN_GETPHYSBUF, &args);
}


USHORT APIENTRY VioPrtSc (HVIO hvio)
{
	ARGS args;
	
	args.PrtSc.hvio=hvio;
	
	return VioCall(FN_PRTSC, &args);
}


USHORT APIENTRY VioScrollRt (USHORT usTopRow,
                            USHORT usLeftCol,
                            USHORT usBotRow,
                            USHORT usRightCol,
                            USHORT cbCol,
                            PBYTE pCell,
                            HVIO hvio)
{
	ARGS args;
	
	args.ScrollRt.usTopRow=usTopRow;
	args.ScrollRt.usLeftCol=usLeftCol;
	args.ScrollRt.usBotRow=usBotRow;
	args.ScrollRt.usRightCol=usRightCol;
	args.ScrollRt.cbCol=cbCol;
	args.ScrollRt.pCell=pCell;
	args.ScrollRt.hvio=hvio;
	
	return VioCall(FN_SCROLLRT, &args);
}


USHORT APIENTRY VioScrLock (USHORT fWait,
                           PUCHAR pfNotLocked,
                           HVIO hvio)
{
	ARGS args;
	
	args.ScrLock.fWait=fWait;
	args.ScrLock.pfNotLocked=pfNotLocked;
	args.ScrLock.hvio=hvio;
	
	return VioCall(FN_SCRLOCK, &args);
}


USHORT APIENTRY VioScrUnLock (HVIO hvio)
{
	ARGS args;
	
	args.ScrUnLock.hvio=hvio;
	
	return VioCall(FN_SCRUNLOCK, &args);
}


USHORT APIENTRY VioSavRedrawWait (USHORT usRedrawInd,
                                 PUSHORT pNotifyType,
                                 USHORT usReserved)
{
	ARGS args;
	
	args.SavRedrawWait.usRedrawInd=usRedrawInd;
	args.SavRedrawWait.pNotifyType=pNotifyType;

	return VioCall(FN_SAVREDRAWWAIT, &args);
}


USHORT APIENTRY VioSavRedrawUndo (USHORT usOwnerInd,
                                 USHORT usKillInd,
                                 USHORT usReserved)
{
	ARGS args;
	
	args.SavRedrawUndo.usOwnerInd=usOwnerInd;
	args.SavRedrawUndo.usKillInd=usKillInd;
	
	return VioCall(FN_SAVREDRAWUNDO, &args);
}


USHORT APIENTRY VioGetFont (PVIOFONTINFO pviofi,
                           HVIO hvio)
{
	ARGS args;
	
	args.GetFont.pviofi=pviofi;
	args.GetFont.hvio=hvio;
	
	return VioCall(FN_GETFONT, &args);
}


USHORT APIENTRY VioSetFont (PVIOFONTINFO pviofi,
                           HVIO hvio)
{
	ARGS args;
	
	args.SetFont.pviofi=pviofi;
	args.SetFont.hvio=hvio;
	
	return VioCall(FN_SETFONT, &args);
}


USHORT APIENTRY VioGetBuf (PULONG pLVB,
                          PUSHORT pcbLVB,
                          HVIO hvio)
{
	ARGS args;
	
	args.GetBuf.pLVB=pLVB;
	args.GetBuf.pcbLVB=pcbLVB;
	args.GetBuf.hvio=hvio;
	
	return VioCall(FN_GETBUF, &args);
}


USHORT APIENTRY VioModeWait (USHORT usReqType,
                            PUSHORT pNotifyType,
                            USHORT usReserved)
{
	ARGS args;
	
	args.ModeWait.usReqType=usReqType;
	args.ModeWait.pNotifyType=pNotifyType;
	
	return VioCall(FN_MODEWAIT, &args);
}


USHORT APIENTRY VioModeUndo (USHORT usOwnerInd,
                              USHORT usKillInd,
                              USHORT usReserved)
{
	ARGS args;
	
	args.ModeUndo.usOwnerInd=usOwnerInd;
	args.ModeUndo.usKillInd=usKillInd;
	
	return VioCall(FN_MODEUNDO, &args);
}


USHORT APIENTRY VioShowBuf (USHORT offLVB,
                             USHORT cb,
                             HVIO hvio)
{
	ARGS args;
	
	args.ShowBuf.offLVB=offLVB;
	args.ShowBuf.cb=cb;
	args.ShowBuf.hvio=hvio;
	
	return VioCall(FN_SHOWBUF, &args);
}


USHORT APIENTRY VioPrtScToggle (HVIO hvio)
{
	ARGS args;
	
	args.PrtScToggle.hvio=hvio;
	
	return VioCall(FN_PRTSCTOGGLE, &args);
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
