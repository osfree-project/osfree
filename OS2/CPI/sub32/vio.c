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
