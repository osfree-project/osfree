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


APIRET APIENTRY BVS32Main(ULONG fn)
{
  APIRET rc=0;

      switch (fn)  
      {  
        case FN_GETPHYSBUF:
          break;
        case FN_GETBUF:
          break;
        case FN_SHOWBUF:
          break;
        case FN_GETCURPOS:
          break;
        case FN_GETCURTYPE:
          break;
        case FN_GETMODE:
          break;
        case FN_SETCURPOS:
          break;
        case FN_SETCURTYPE:
          break;
        case FN_SETMODE:
          break;
        case FN_READCHARSTR:
          break;
        case FN_READCELLSTR:
          break;
        case FN_WRTNCHAR:
          break;
        case FN_WRTNATTR:
          break;
        case FN_WRTNCELL:
          break;
        case FN_WRTCHARSTR:
          break;
        case FN_WRTCHARSTRATT:
          break;
        case FN_WRTCELLSTR:
          break;
        case FN_WRTTTY:
          break;
        case FN_SCROLLUP:
          break;
        case FN_SCROLLDN:
          break;
        case FN_SCROLLLF:
          break;
        case FN_SCROLLRT:
          break;
        case FN_SETANSI:
          break;
        case FN_GETANSI:
          break;
        case FN_PRTSC:
          break;
        case FN_SCRLOCK:
          break;
        case FN_SCRUNLOCK:
          break;
        case FN_SAVREDRAWWAIT:
          break;
        case FN_SAVREDRAWUNDO:
          break;
        case FN_POPUP:
          break;
        case FN_ENDPOPUP:
          break;
        case FN_PRTSCTOGGLE:
          break;
        case FN_MODEWAIT:
          break;
        case FN_MODEUNDO:
          break;
        case FN_GETFONT:
          break;
        case FN_GETCONFIG:
          break;
        case FN_SETCP:
          break;
        case FN_GETCP:
          break;
        case FN_SETFONT:
          break;
        case FN_GETSTATE:
          break;
        case FN_SETSTATE:
          break;
        case FN_REGISTER:
          break;
        case FN_DEREGISTER:
          break;
        case FN_GLOBALREG:
          break;
        case FN_CREATECA:
          break;
        case FN_GETCASTATE:
          break;
        case FN_SETCASTATE:
          break;
        case FN_DESTROYCA:
          break;
        case FN_CHECKCHARTYPE:
          break;
        case FN_SAVE:
          break;
        case FN_RESTORE:
          break;
        case FN_FREE:
          break;
        case FN_SHELLINIT:
          break;
//        default: 
//          BVSError();
      }  
  return rc;
}

USHORT APIENTRY BvsGetPhysBuf (PVIOPHYSBUF pvioPhysBuf,
                              USHORT usReserved)
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
                                 PUSHORT pNotifyType,
                                 USHORT usReserved)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsSavRedrawUndo (USHORT usOwnerInd,
                                 USHORT usKillInd,
                                 USHORT usReserved)
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
                            PUSHORT pNotifyType,
                            USHORT usReserved)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsModeUndo (USHORT usOwnerInd,
                              USHORT usKillInd,
                              USHORT usReserved)
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

USHORT APIENTRY BvsSetCp(const USHORT Reserved, const USHORT IdCodePage, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY BvsGetCp(const USHORT Reserved, USHORT * IdCodePage, const HVIO Handle)
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
