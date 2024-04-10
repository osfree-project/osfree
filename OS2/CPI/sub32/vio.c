#define INCL_VIO
#define INCL_PMAVIO
#include <os2.h>

APIRET unimplemented(char *func);
//APIRET APIENTRY BVSMain(ULONG fn);

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

// Call VIO function
APIRET APIENTRY VioCall(ULONG fn)
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
