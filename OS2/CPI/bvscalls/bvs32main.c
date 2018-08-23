#include <os2.h>

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
