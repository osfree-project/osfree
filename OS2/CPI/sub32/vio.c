#define INCL_DOSMODULEMGR     /* Module Manager values */
#define INCL_DOSERRORS        /* Error values */
#include <os2.h>

APIRET unimplemented(char *func);
APIRET APIENTRY BVSMain(ULONG fn);

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
#define FN_REGISTER        41
#define FN_DEREGISTER      42
#define FN_GLOBALREG       43
#define FN_CREATECA        44
#define FN_GETCASTATE      45
#define FN_SETCASTATE      46
#define FN_DESTROYCA       47
#define FN_CHECKCHARTYPE   48
#define FN_SAVE            65
#define FN_RESTORE         66
#define FN_FREE            67
#define FN_SHELLINIT       68

// Call VIO function
APIRET APIENTRY VioCall(ULONG fn)
{
  return BVSMain(fn);
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
   } else {
      //printf("Module %s loaded.\n", ModuleName);
   } /* endif */
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
   rc = DosFreeModule(ModuleHandle);
   return NO_ERROR;
}

/*
32-bit versions
USHORT VioCheckCharType (PUSHORT pType, USHORT usRow, USHORT usColumn,
    HVIO hvio);
USHORT VioDeRegister (VOID);
USHORT VioGlobalReg (PCSZ pszModName, PCSZ pszEntryName, ULONG ulFunMask1,
    ULONG ulFunMask2, USHORT usReturn);
USHORT VioPopUp (PUSHORT pfWait, HVIO hvio);
USHORT VioRegister (PCSZ pszModName, PCSZ pszEntryName, ULONG ulFunMask1,
    ULONG ulFunMask2);
 */

USHORT Vio32Route(USHORT func_no, USHORT, USHORT, USHORT, USHORT, USHORT, USHORT, USHORT, USHORT)
{
  return 0;
}
 
USHORT Vio32GetPhysBuf(PVIOPHYSBUF pvioPhysBuf, USHORT usReserved)
{
  return Vio32Route(FN_GETPHYSBUF, (HVIO) NULL, NULL);
}

USHORT Vio32GetBuf(PULONG pLVB, PUSHORT pcbLVB, HVIO hvio)
{
  return Vio32Route(FN_GETBUF, hvio, NULL);
}

USHORT Vio32ShowBuf(USHORT offLVB, USHORT cb, HVIO hvio)
{
  return Vio32Route(FN_SHOWBUF, hvio, NULL);
}

USHORT Vio32GetCurPos(PUSHORT pusRow, PUSHORT pusColumn, HVIO hvio)
{
  return Vio32Route(FN_GETCURPOS, hvio, NULL);
}

USHORT Vio32GetCurType(PVIOCURSORINFO pvioCursorInfo, HVIO hvio)
{
  return Vio32Route(FN_GETCURTYPE, hvio, NULL);
}

USHORT Vio32GetMode(PVIOMODEINFO pvioModeInfo, HVIO hvio)
{
  return Vio32Route(FN_GETMODE, hvio, NULL);
}

USHORT Vio32SetCurType (PVIOCURSORINFO pvioCursorInfo, HVIO hvio)
{
  return Vio32Route(FN_SETCURTYPE, hvio, NULL);
}

USHORT Vio32SetCurPos (USHORT usRow, USHORT usColumn, HVIO hvio)
{
  return Vio32Route(FN_SETCURPOS, hvio, NULL);
}

USHORT Vio32SetMode (PVIOMODEINFO pvioModeInfo, HVIO hvio)
{
  return Vio32Route(FN_SETMODE, hvio, NULL);
}

USHORT Vio32SetCp (USHORT usReserved, USHORT usCodePage, HVIO hvio)
{
  return Vio32Route(FN_SETCP, hvio, NULL);
}

USHORT Vio32ReadCharStr (PCH pch, PUSHORT pcb, USHORT usRow, USHORT usColumn,
    HVIO hvio)
{
  return Vio32Route(FN_READCHARSTR, hvio, NULL);
}

USHORT Vio32ReadCellStr (PCH pchCellStr, PUSHORT pcb, USHORT usRow,
    USHORT usColumn, HVIO hvio)
{
  return Vio32Route(FN_READCELLSTR, NULL);
}

USHORT Vio32WrtNChar (PCCH pch, USHORT cb, USHORT usRow, USHORT usColumn,
    HVIO hvio)
{
  return Vio32Route(FN_WRTNCHAR, NULL);
}

USHORT Vio32WrtNAttr (__const__ BYTE *pAttr, USHORT cb, USHORT usRow,
    USHORT usColumn, HVIO hvio)
{
  return Vio32Route(FN_WRTNATTR, NULL);
}

USHORT Vio32WrtNCell (__const__ BYTE *pCell, USHORT cb, USHORT usRow,
    USHORT usColumn, HVIO hvio)
{
  return Vio32Route(FN_WRTNCELL, NULL);
}

USHORT Vio32WrtCharStr (PCCH pch, USHORT cb, USHORT usRow, USHORT usColumn,
    HVIO hvio)
{
  return Vio32Route(FN_WRTCHARSTR, NULL);
}

USHORT Vio32WrtCharStrAtt (PCCH pch, USHORT cb, USHORT usRow, USHORT usColumn,
    PBYTE pAttr, HVIO hvio)
{
  return Vio32Route(FN_WRTCHARSTRATT, NULL);
}

USHORT Vio32WrtCellStr (PCCH pchCellStr, USHORT cb, USHORT usRow,
    USHORT usColumn, HVIO hvio)
{
  return Vio32Route(FN_WRTCELLSTR, NULL);
}

USHORT Vio32WrtTTY (PCCH pch, USHORT cb, HVIO hvio)
{
  return Vio32Route(FN_WRTTTY, NULL);
}

USHORT Vio32ScrollUp (USHORT usTopRow, USHORT usLeftCol, USHORT usBotRow,
    USHORT usRightCol, USHORT cbLines, PBYTE pCell, HVIO hvio)
{
  return Vio32Route(FN_SCROLLUP, NULL);
}

USHORT Vio32ScrollDn (USHORT usTopRow, USHORT usLeftCol, USHORT usBotRow,
    USHORT usRightCol, USHORT cbLines, PBYTE pCell, HVIO hvio)
{
  return Vio32Route(FN_SCROLLDN, NULL);
}

USHORT Vio32ScrollLf (USHORT usTopRow, USHORT usLeftCol, USHORT usBotRow,
    USHORT usRightCol, USHORT cbCol, PBYTE pCell, HVIO hvio)
{
  return Vio32Route(FN_SCROLLLF, NULL);
}

USHORT Vio32ScrollRt (USHORT usTopRow, USHORT usLeftCol, USHORT usBotRow,
    USHORT usRightCol, USHORT cbCol, PBYTE pCell, HVIO hvio)
{
  return Vio32Route(FN_SCROLLRT, NULL);
}

USHORT Vio32SetAnsi (USHORT fAnsi, HVIO hvio)
{
  return Vio32Route(FN_SETANSI, NULL);
}

USHORT Vio32GetAnsi (PUSHORT pfAnsi, HVIO hvio)
{
  return Vio32Route(FN_GETANSI, NULL);
}

USHORT Vio32PrtSc (HVIO hvio)
{
  return Vio32Route(FN_PRTSC, NULL);
}

USHORT Vio32ScrLock (USHORT fWait, PUCHAR pfNotLocked, HVIO hvio)
{
  return Vio32Route(FN_SCRLOCK, NULL);
}

USHORT Vio32ScrUnLock (HVIO hvio)
{
  return Vio32Route(FN_SCRUNLOCK, NULL);
}

USHORT Vio32SavRedrawWait (USHORT usRedrawInd, PUSHORT pusNotifyType,
    USHORT usReserved)
{
  return Vio32Route(FN_SAVREDRAWWAIT, NULL);
}

USHORT Vio32SavRedrawUndo (USHORT usOwnerInd, USHORT usKillInd,
    USHORT usReserved)
{
  return Vio32Route(FN_SAVREDRAWUNDO, NULL);
}

USHORT Vio32PrtScToggle (HVIO hvio)
{
  return Vio32Route(FN_PRTSCTOGGLE, NULL);
}

USHORT Vio32ModeWait (USHORT usReqType, PUSHORT pNotifyType, USHORT usReserved)
{
  return Vio32Route(FN_MODEWAIT, NULL);
}

USHORT Vio32ModeUndo (USHORT usOwnerInd, USHORT usKillInd, USHORT usReserved)
{
  return Vio32Route(FN_MODEUNDO, NULL);
}

USHORT Vio32GetFont (PVIOFONTINFO pviofi, HVIO hvio)
{
  return Vio32Route(FN_GETFONT, NULL);
}

USHORT Vio32GetConfig (USHORT usConfigId, PVIOCONFIGINFO pvioin, HVIO hvio)
{
  return Vio32Route(FN_GETCONFIG, NULL);
}

USHORT Vio32GetCp (USHORT usReserved, PUSHORT pusCodePage, HVIO hvio)
{
  return Vio32Route(FN_GETCP, NULL);
}

USHORT Vio32SetFont (PVIOFONTINFO pviofi, HVIO hvio)
{
  return Vio32Route(FN_SETFONT, NULL);
}

USHORT Vio32GetState (PVOID pState, HVIO hvio)
{
  return Vio32Route(FN_GETSTATE, NULL);
}

USHORT Vio32SetState (CPVOID pState, HVIO hvio)
{
  return Vio32Route(FN_SETSTATE, NULL);
}
