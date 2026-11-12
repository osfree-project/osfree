/*!
 * @file win_wire.h
 * @brief Wire codes + argument structures for F_CMD_CLASS_WIN.
 *
 * Covers every 32-bit (non-RESIDENT) Win* export from PMWIN.DLL.
 * RESIDENT entries are 16-bit twins; 16-bit thunks, when added, map
 * into the same 32-bit entry points.
 *
 * Handles live in the *_Args structure, not in a separate command
 * handle (unlike GPI, where every Gpi* function has hps as first arg
 * and F_CMD_GPI(fn) carries it via PmcPostCommand's data field).
 *
 * Return value: via FPM_RESP_HEADER.ulResult.
 * Out params:   via pcbPayload (in-place), pattern is cbXxx + abXxx[1]
 *               for variable-length data, or an explicit struct pointer.
 */
#ifndef WIN_WIRE_H
#define WIN_WIRE_H

#include "F_platform.h"

/*!
 * @def NWIN_SYNC_FLAG
 * @brief High bit of a WIN fnCode: marks a sync command (SendCommand).
 *
 * Set in the command code definition (e.g. NWinInitialize). The server
 * strips it before dispatch; the client does not use it directly.
 */
#define NWIN_SYNC_FLAG  0x8000UL

/* ========================================================================== */
/*  Domain ranges                                                             */
/* ========================================================================== */
/*  0x0001-0x000F  HAB lifecycle                                             */
/*  0x0010-0x001F  Message queue                                             */
/*  0x0020-0x003F  Window lifecycle                                          */
/*  0x0040-0x005F  Window queries                                            */
/*  0x0060-0x007F  Paint / PS / DC                                           */
/*  0x0080-0x009F  Class / Subclass / Thunk                                  */
/*  0x00A0-0x00BF  Dialogs                                                   */
/*  0x00C0-0x00CF  Menus / Timers / Pointer / Cursor                         */
/*  0x00D0-0x00DF  Clipboard                                                 */
/*  0x00E0-0x00EF  Atoms                                                     */
/*  0x00F0-0x00FF  Strings / Codepages / Accel / Hooks / Resource            */
/*  0x0100-0x010F  Errors / Version                                          */
/*  0x0110-0x011F  Sys / Desktop                                             */
/*  0x0120-0x013F  Rect helpers                                              */
/*  0x0140-0x014F  Input / Keyboard                                          */
/*  0x0150-0x015F  Draw helpers                                              */
/*  0x0160-0x016F  Frame / calc                                              */
/*  0x0170-0x017F  Sync primitives                                           */
/*  0x0180-0x018F  Seamless / Msp / Pen / Dax                                */
/*  0x0190-0x019F  Win32-specific                                            */
/*  0x01A0-0x01FF  reserved                                                  */

/* ========================================================================== */
/*  HAB lifecycle                                                             */
/* ========================================================================== */

#define NWinInitialize                (0x0001UL | NWIN_SYNC_FLAG)
#define NWinTerminate                 (0x0002UL | NWIN_SYNC_FLAG)
#define NWinGetLastError              (0x0003UL | NWIN_SYNC_FLAG)
#define NWinQueryAnchorBlock          0x0004UL
#define NWinQueryVersion              0x0005UL

typedef struct _WinQueryAnchorBlock_Args {
    HWND hwnd;
} WinQueryAnchorBlock_Args, *PWinQueryAnchorBlock_Args;

typedef struct _WinQueryVersion_Args {
    ULONG iHab;
} WinQueryVersion_Args, *PWinQueryVersion_Args;

/* ========================================================================== */
/*  Message queue                                                             */
/* ========================================================================== */

#define NWinCreateMsgQueue            (0x0010UL | NWIN_SYNC_FLAG)
#define NWinDestroyMsgQueue           (0x0011UL | NWIN_SYNC_FLAG)
#define NWinGetMsg                    0x0012L
#define NWinPeekMsg                   0x0013L
#define NWinWaitMsg                   0x0014L
#define NWinDispatchMsg               0x0015L
#define NWinPostMsg                   0x0016L
#define NWinPostQueueMsg              0x0017L
#define NWinSendMsg                   0x0018L
#define NWinSendDlgItemMsg            0x0019L
#define NWinBroadcastMsg              0x001AL
#define NWinInSendMsg                 0x001BL
#define NWinQuerySendMsg              0x001CL
#define NWinReplyMsg                  0x001DL
#define NWinWakeThread                0x001EL
#define NWinQueryQueueStatus          0x001FL

/* BOOL WinGetMsg(HAB, PQMSG, HWND filter, ULONG first, ULONG last) */
typedef struct _WinGetMsg_Args {
    ULONG iHab; HWND hwndFilter; ULONG msgFirst; ULONG msgLast;
    QMSG  qmsg;   /* out: extracted message (WinGetMsg) */
} WinGetMsg_Args, *PWinGetMsg_Args;
/* BOOL WinPeekMsg(HAB, PQMSG, HWND, ULONG, ULONG, ULONG fl) */
typedef struct _WinPeekMsg_Args {
    ULONG iHab; HWND hwndFilter; ULONG msgFirst; ULONG msgLast; ULONG fl;
    QMSG  qmsg;   /* out: peeked message (WinPeekMsg) */
} WinPeekMsg_Args, *PWinPeekMsg_Args;
/* BOOL WinWaitMsg(HAB, ULONG first, ULONG last) */
typedef struct _WinWaitMsg_Args { ULONG iHab; ULONG msgFirst; ULONG msgLast; } WinWaitMsg_Args, *PWinWaitMsg_Args;
/* MRESULT WinDispatchMsg(HAB, PQMSG) */
typedef struct _WinDispatchMsg_Args {
    ULONG iHab; QMSG qmsg;   /* in: message to dispatch */
} WinDispatchMsg_Args, *PWinDispatchMsg_Args;
/* BOOL WinPostMsg(HWND, ULONG msg, MPARAM mp1, MPARAM mp2) */
typedef struct _WinPostMsg_Args { HWND hwnd; ULONG msg; MPARAM mp1; MPARAM mp2; } WinPostMsg_Args, *PWinPostMsg_Args;
/* BOOL WinPostQueueMsg(HMQ, ULONG msg, MPARAM mp1, MPARAM mp2) */
typedef struct _WinPostQueueMsg_Args { ULONG hmq; ULONG msg; MPARAM mp1; MPARAM mp2; } WinPostQueueMsg_Args, *PWinPostQueueMsg_Args;
/* MRESULT WinSendMsg(HWND, ULONG msg, MPARAM mp1, MPARAM mp2) */
typedef struct _WinSendMsg_Args { HWND hwnd; ULONG msg; MPARAM mp1; MPARAM mp2; } WinSendMsg_Args, *PWinSendMsg_Args;
/* MRESULT WinSendDlgItemMsg(HWND dlg, ULONG id, ULONG msg, MPARAM mp1, MPARAM mp2) */
typedef struct _WinSendDlgItemMsg_Args { HWND hwndDlg; ULONG idItem; ULONG msg; MPARAM mp1; MPARAM mp2; } WinSendDlgItemMsg_Args, *PWinSendDlgItemMsg_Args;
/* BOOL WinBroadcastMsg(HWND, ULONG msg, MPARAM mp1, MPARAM mp2, ULONG fl) */
typedef struct _WinBroadcastMsg_Args { HWND hwnd; ULONG msg; MPARAM mp1; MPARAM mp2; ULONG fl; } WinBroadcastMsg_Args, *PWinBroadcastMsg_Args;
/* BOOL WinInSendMsg(HAB) */
typedef struct _WinInSendMsg_Args { ULONG iHab; } WinInSendMsg_Args, *PWinInSendMsg_Args;
/* BOOL WinQuerySendMsg(HAB) */
typedef struct _WinQuerySendMsg_Args { ULONG iHab; } WinQuerySendMsg_Args, *PWinQuerySendMsg_Args;
/* BOOL WinReplyMsg(HMQ, ULONG msg, MRESULT mres, MPARAM mp1, MPARAM mp2) */
typedef struct _WinReplyMsg_Args { ULONG hmq; ULONG msg; MRESULT mres; MPARAM mp1; MPARAM mp2; } WinReplyMsg_Args, *PWinReplyMsg_Args;
/* BOOL WinWakeThread(HMQ) */
typedef struct _WinWakeThread_Args { ULONG hmq; } WinWakeThread_Args, *PWinWakeThread_Args;
/* ULONG WinQueryQueueStatus(HAB) */
typedef struct _WinQueryQueueStatus_Args { ULONG iHab; } WinQueryQueueStatus_Args, *PWinQueryQueueStatus_Args;

/* ========================================================================== */
/*  Window lifecycle                                                          */
/* ========================================================================== */

#define NWinCreateWindow              0x0020L
#define NWinCreateStdWindow           0x0021L
#define NWinDestroyWindow             (0x0022UL | NWIN_SYNC_FLAG)
#define NWinSetWindowPos              0x0023L
#define NWinSetMultWindowPos          0x0024L
#define NWinShowWindow                0x0025L
#define NWinEnableWindow              0x0026L
#define NWinEnableWindowUpdate        0x0027L
#define NWinIsWindow                  0x0028L
#define NWinIsWindowVisible           0x0029L
#define NWinIsWindowEnabled           0x002AL
#define NWinIsWindowShowing           0x002BL
#define NWinIsWindowOfClass           0x002CL
#define NWinIsChild                   0x002DL
#define NWinIsThreadActive            0x002EL
#define NWinSetParent                 0x002FL
#define NWinSetOwner                  0x0030L
#define NWinSetCapture                0x0031L
#define NWinSetFocus                  0x0032L
#define NWinSetActiveWindow           0x0033L
#define NWinSetSysModalWindow         0x0034L
#define NWinBeginEnumWindows          0x0035L
#define NWinEndEnumWindows            0x0036L
#define NWinGetNextWindow             0x0037L
#define NWinFlashWindow               0x0038L
#define NWinFocusChange               0x0039L
#define NWinSaveWindowPos             0x003AL
#define NWinScrollWindow              0x003BL
#define NWinSetWindowBits             0x003CL
#define NWinSetWindowPtr              0x003DL
#define NWinSetWindowULong            0x003EL
#define NWinSetWindowUShort           0x003FL

/* HWND WinCreateWindow(HWND parent, PCSZ class, PCSZ name, ULONG style,
                        LONG x, y, cx, cy, HWND owner, HWND behind, ULONG id,
                        PVOID pCtlData, PVOID pPresParams) */

/*! @brief Wire args for NWinCreateWindow (hwndParent travels in `data`).
 *
 * Followed by (contiguous, no NUL terminators — lengths are explicit):
 *   char szClass[cbClass];
 *   char szName[cbName];
 *   if (flPresent & WCRF_HAS_CTLDATA):
 *     BYTE abCtlData[];     (first 2 bytes = USHORT cbCtlData)
 *   if (flPresent & WCRF_HAS_PRESPARAMS):
 *     BYTE abPresParams[];  (first 4 bytes = ULONG  cbPresParams)
 */
#define WCRF_HAS_CTLDATA     0x01
#define WCRF_HAS_PRESPARAMS  0x02

typedef struct _WinCreateWindow_Args {
    ULONG cbClass;
    ULONG cbName;
    ULONG flStyle;
    LONG  x, y, cx, cy;
    HWND  hwndOwner, hwndInsertBehind;
    ULONG id;
    BYTE  flPresent;
} WinCreateWindow_Args, *PWinCreateWindow_Args;

/* HWND WinCreateStdWindow(HWND parent, ULONG style, PULONG pflCreateFlags,
    PCSZ clientClass, PCSZ title, ULONG clientStyle, HMODULE hmod,
    ULONG idResources, PHWND phwndClient) */
typedef struct _WinCreateStdWindow_Args {
    HWND hwndParent; ULONG flStyle; ULONG flCreateFlags;   /* in/out */
    ULONG cbClass; char szClientClass[1];
    ULONG cbTitle; char szTitle[1];
    ULONG styleClient; HMODULE hmod; ULONG idResources;
    HWND hwndClient; /* out */
} WinCreateStdWindow_Args, *PWinCreateStdWindow_Args;
/* BOOL WinDestroyWindow(HWND) */
typedef struct _WinDestroyWindow_Args { HWND hwnd; } WinDestroyWindow_Args, *PWinDestroyWindow_Args;
/* BOOL WinSetWindowPos(HWND, HWND behind, LONG x,y,cx,cy, ULONG fl) */
typedef struct _WinSetWindowPos_Args { HWND hwnd; HWND hwndInsertBehind; LONG x, y, cx, cy; ULONG fl; } WinSetWindowPos_Args, *PWinSetWindowPos_Args;
/* BOOL WinSetMultWindowPos(HAB, PSWP pswp, ULONG cswp) */
typedef struct _WinSetMultWindowPos_Args {
    ULONG iHab; ULONG cswp;
    /* followed by cswp x SWP { HWND hwnd, hwndInsertBehind; LONG x,y,cx,cy; ULONG fl; } */
} WinSetMultWindowPos_Args, *PWinSetMultWindowPos_Args;
/* BOOL WinShowWindow(HWND, BOOL) */
typedef struct _WinShowWindow_Args { HWND hwnd; BOOL fShow; } WinShowWindow_Args, *PWinShowWindow_Args;
/* BOOL WinEnableWindow(HWND, BOOL) */
typedef struct _WinEnableWindow_Args { HWND hwnd; BOOL fEnable; } WinEnableWindow_Args, *PWinEnableWindow_Args;
/* BOOL WinEnableWindowUpdate(HWND, BOOL) */
typedef struct _WinEnableWindowUpdate_Args { HWND hwnd; BOOL fEnable; } WinEnableWindowUpdate_Args, *PWinEnableWindowUpdate_Args;
/* BOOL WinIsWindow(HAB, HWND) */
typedef struct _WinIsWindow_Args { ULONG iHab; HWND hwnd; } WinIsWindow_Args, *PWinIsWindow_Args;
/* BOOL WinIsWindowVisible(HWND) */
typedef struct _WinIsWindowVisible_Args { HWND hwnd; } WinIsWindowVisible_Args, *PWinIsWindowVisible_Args;
/* BOOL WinIsWindowEnabled(HWND) */
typedef struct _WinIsWindowEnabled_Args { HWND hwnd; } WinIsWindowEnabled_Args, *PWinIsWindowEnabled_Args;
/* BOOL WinIsWindowShowing(HWND) */
typedef struct _WinIsWindowShowing_Args { HWND hwnd; } WinIsWindowShowing_Args, *PWinIsWindowShowing_Args;
/* BOOL WinIsWindowOfClass(HWND, PCSZ) */
typedef struct _WinIsWindowOfClass_Args { HWND hwnd; ULONG cbClass; char szClass[1]; } WinIsWindowOfClass_Args, *PWinIsWindowOfClass_Args;
/* BOOL WinIsChild(HWND, HWND parent) */
typedef struct _WinIsChild_Args { HWND hwnd; HWND hwndParent; } WinIsChild_Args, *PWinIsChild_Args;
/* BOOL WinIsThreadActive(HAB) */
typedef struct _WinIsThreadActive_Args { ULONG iHab; } WinIsThreadActive_Args, *PWinIsThreadActive_Args;
/* BOOL WinSetParent(HWND, HWND newParent, BOOL fRedraw) */
typedef struct _WinSetParent_Args { HWND hwnd; HWND hwndNewParent; BOOL fRedraw; } WinSetParent_Args, *PWinSetParent_Args;
/* BOOL WinSetOwner(HWND, HWND owner) */
typedef struct _WinSetOwner_Args { HWND hwnd; HWND hwndOwner; } WinSetOwner_Args, *PWinSetOwner_Args;
/* HWND WinSetCapture(HWND desktop, HWND) */
typedef struct _WinSetCapture_Args { HWND hwndDesktop; HWND hwnd; } WinSetCapture_Args, *PWinSetCapture_Args;
/* HWND WinSetFocus(HWND desktop, HWND) */
typedef struct _WinSetFocus_Args { HWND hwndDesktop; HWND hwnd; } WinSetFocus_Args, *PWinSetFocus_Args;
/* HWND WinSetActiveWindow(HWND desktop, HWND) */
typedef struct _WinSetActiveWindow_Args { HWND hwndDesktop; HWND hwnd; } WinSetActiveWindow_Args, *PWinSetActiveWindow_Args;
/* HWND WinSetSysModalWindow(HWND desktop, HWND) */
typedef struct _WinSetSysModalWindow_Args { HWND hwndDesktop; HWND hwnd; } WinSetSysModalWindow_Args, *PWinSetSysModalWindow_Args;
/* HENUM WinBeginEnumWindows(HWND) — HENUM as ULONG */
typedef struct _WinBeginEnumWindows_Args { HWND hwnd; } WinBeginEnumWindows_Args, *PWinBeginEnumWindows_Args;
/* BOOL WinEndEnumWindows(HENUM) */
typedef struct _WinEndEnumWindows_Args { ULONG henum; } WinEndEnumWindows_Args, *PWinEndEnumWindows_Args;
/* HWND WinGetNextWindow(HENUM) */
typedef struct _WinGetNextWindow_Args { ULONG henum; } WinGetNextWindow_Args, *PWinGetNextWindow_Args;
/* BOOL WinFlashWindow(HWND, BOOL) */
typedef struct _WinFlashWindow_Args { HWND hwnd; BOOL fFlash; } WinFlashWindow_Args, *PWinFlashWindow_Args;
/* HWND WinFocusChange(HWND desktop, HWND, ULONG fl) */
typedef struct _WinFocusChange_Args { HWND hwndDesktop; HWND hwnd; ULONG fl; } WinFocusChange_Args, *PWinFocusChange_Args;
/* BOOL WinSaveWindowPos(HSAVEWP, PSWP pswp, ULONG cswp) */
typedef struct _WinSaveWindowPos_Args {
    ULONG hswp; ULONG cswp;   /* followed by cswp SWP */
} WinSaveWindowPos_Args, *PWinSaveWindowPos_Args;
/* BOOL WinScrollWindow(HWND, LONG dx, LONG dy, PRECTL prclScroll,
    PRECTL prclClip, HRGN hrgnUpdate, PRECTL prclUpdate, ULONG fl) */
typedef struct _WinScrollWindow_Args {
    HWND hwnd; LONG dx, dy;
    RECTL rclScroll; ULONG fScrollSet;
    RECTL rclClip;   ULONG fClipSet;
    ULONG hrgnUpdate;
    RECTL rclUpdate; ULONG fUpdateSet;
    ULONG fl;
    /* out: rclUpdate updated, pcbPayload = sizeof(WinScrollWindow_Args) */
} WinScrollWindow_Args, *PWinScrollWindow_Args;
/* BOOL WinSetWindowBits(HWND, LONG index, ULONG flData, ULONG flMask) */
typedef struct _WinSetWindowBits_Args { HWND hwnd; LONG index; ULONG flData; ULONG flMask; } WinSetWindowBits_Args, *PWinSetWindowBits_Args;
/* BOOL WinSetWindowPtr(HWND, LONG index, PVOID) */
typedef struct _WinSetWindowPtr_Args { HWND hwnd; LONG index; ULONG p; } WinSetWindowPtr_Args, *PWinSetWindowPtr_Args;
/* BOOL WinSetWindowULong(HWND, LONG index, ULONG) */
typedef struct _WinSetWindowULong_Args { HWND hwnd; LONG index; ULONG ul; } WinSetWindowULong_Args, *PWinSetWindowULong_Args;
/* BOOL WinSetWindowUShort(HWND, LONG index, USHORT) */
typedef struct _WinSetWindowUShort_Args { HWND hwnd; LONG index; USHORT us; } WinSetWindowUShort_Args, *PWinSetWindowUShort_Args;

/* ========================================================================== */
/*  Window queries                                                            */
/* ========================================================================== */

#define NWinQueryWindow               0x0040L
#define NWinQueryWindowPos            0x0041L
#define NWinQueryWindowRect           0x0042L
#define NWinQueryWindowText           0x0043L
#define NWinQueryWindowTextLength     0x0044L
#define NWinSetWindowText             0x0045L
#define NWinQueryWindowPtr            0x0046L
#define NWinQueryWindowULong          0x0047L
#define NWinQueryWindowUShort         0x0048L
#define NWinQueryWindowProcess        0x0049L
#define NWinQueryWindowModel          0x004AL
#define NWinQueryWindowDC             0x004BL
#define NWinWindowFromPoint           0x004CL
#define NWinWindowFromID              0x004DL
#define NWinWindowFromDC              0x004EL
#define NWinMultWindowFromIDs         0x004FL
#define NWinMapWindowPoints           0x0050L
#define NWinQueryDesktopWindow        0x0051L
#define NWinQueryObjectWindow         0x0052L
#define NWinQueryCapture              0x0053L
#define NWinQueryFocus                0x0054L
#define NWinQueryActiveWindow         0x0055L
#define NWinQuerySysModalWindow       0x0056L
#define NWinQueryClassName            0x0057L
#define NWinQueryClassInfo            0x0058L

/* HWND WinQueryWindow(HWND, LONG lCode) */
typedef struct _WinQueryWindow_Args { HWND hwnd; LONG lCode; } WinQueryWindow_Args, *PWinQueryWindow_Args;
/* BOOL WinQueryWindowPos(HWND, PSWP) — SWP inline: HWND behind; LONG x,y,cx,cy; ULONG fl */
typedef struct _WinQueryWindowPos_Args {
    HWND hwnd;
    /* out */ HWND hwndInsertBehind; LONG x, y, cx, cy; ULONG fl;
} WinQueryWindowPos_Args, *PWinQueryWindowPos_Args;
/* BOOL WinQueryWindowRect(HWND, PRECTL) */
typedef struct _WinQueryWindowRect_Args { HWND hwnd; /* out */ RECTL rcl; } WinQueryWindowRect_Args, *PWinQueryWindowRect_Args;
/* LONG WinQueryWindowText(HWND, LONG cchMax, PSZ) */
typedef struct _WinQueryWindowText_Args {
    HWND hwnd; LONG cchMax;
    /* out: cbText bytes, pcbPayload = cbText */
} WinQueryWindowText_Args, *PWinQueryWindowText_Args;
/* LONG WinQueryWindowTextLength(HWND) */
typedef struct _WinQueryWindowTextLength_Args { HWND hwnd; } WinQueryWindowTextLength_Args, *PWinQueryWindowTextLength_Args;
/* BOOL WinSetWindowText(HWND, PCSZ) */
typedef struct _WinSetWindowText_Args { HWND hwnd; ULONG cbText; char szText[1]; } WinSetWindowText_Args, *PWinSetWindowText_Args;
/* PVOID WinQueryWindowPtr(HWND, LONG) */
typedef struct _WinQueryWindowPtr_Args { HWND hwnd; LONG index; } WinQueryWindowPtr_Args, *PWinQueryWindowPtr_Args;
/* ULONG WinQueryWindowULong(HWND, LONG) */
typedef struct _WinQueryWindowULong_Args { HWND hwnd; LONG index; } WinQueryWindowULong_Args, *PWinQueryWindowULong_Args;
/* USHORT WinQueryWindowUShort(HWND, LONG) */
typedef struct _WinQueryWindowUShort_Args { HWND hwnd; LONG index; } WinQueryWindowUShort_Args, *PWinQueryWindowUShort_Args;
/* BOOL WinQueryWindowProcess(HWND, PPID, PTID) — PID/TID as ULONG */
typedef struct _WinQueryWindowProcess_Args { HWND hwnd; /* out */ ULONG pid, tid; } WinQueryWindowProcess_Args, *PWinQueryWindowProcess_Args;
/* ULONG WinQueryWindowModel(HWND) */
typedef struct _WinQueryWindowModel_Args { HWND hwnd; } WinQueryWindowModel_Args, *PWinQueryWindowModel_Args;
/* HWND WinWindowFromPoint(HWND parent, PPOINTL, BOOL fEnumChildren) */
typedef struct _WinWindowFromPoint_Args { HWND hwndParent; POINTL ptl; BOOL fEnumChildren; } WinWindowFromPoint_Args, *PWinWindowFromPoint_Args;
/* HWND WinWindowFromID(HWND parent, ULONG id) */
typedef struct _WinWindowFromID_Args { HWND hwndParent; ULONG id; } WinWindowFromID_Args, *PWinWindowFromID_Args;
/* HWND WinWindowFromDC(HDC) */
typedef struct _WinWindowFromDC_Args { HDC hdc; } WinWindowFromDC_Args, *PWinWindowFromDC_Args;
/* BOOL WinMultWindowFromIDs(HWND parent, PHWND ahwnd, ULONG cNum) */
typedef struct _WinMultWindowFromIDs_Args { HWND hwndParent; ULONG cNum; /* out: cNum HWNDs, pcbPayload = cNum*4 */ } WinMultWindowFromIDs_Args, *PWinMultWindowFromIDs_Args;
/* BOOL WinMapWindowPoints(HWND from, HWND to, PPOINTL prgptl, LONG cwpt) */
typedef struct _WinMapWindowPoints_Args {
    HWND hwndFrom; HWND hwndTo; LONG cwpt;
    /* in/out: cwpt POINTLs; pcbPayload = cwpt*sizeof(POINTL) */
} WinMapWindowPoints_Args, *PWinMapWindowPoints_Args;
/* HWND WinQueryDesktopWindow(HAB, HDC) */
typedef struct _WinQueryDesktopWindow_Args { ULONG iHab; HDC hdc; } WinQueryDesktopWindow_Args, *PWinQueryDesktopWindow_Args;
/* HWND WinQueryObjectWindow(HWND desktop) */
typedef struct _WinQueryObjectWindow_Args { HWND hwndDesktop; } WinQueryObjectWindow_Args, *PWinQueryObjectWindow_Args;
/* HWND WinQueryCapture(HWND desktop) */
typedef struct _WinQueryCapture_Args { HWND hwndDesktop; } WinQueryCapture_Args, *PWinQueryCapture_Args;
/* HWND WinQueryFocus(HWND desktop) */
typedef struct _WinQueryFocus_Args { HWND hwndDesktop; } WinQueryFocus_Args, *PWinQueryFocus_Args;
/* HWND WinQueryActiveWindow(HWND desktop) */
typedef struct _WinQueryActiveWindow_Args { HWND hwndDesktop; } WinQueryActiveWindow_Args, *PWinQueryActiveWindow_Args;
/* HWND WinQuerySysModalWindow(HWND desktop) */
typedef struct _WinQuerySysModalWindow_Args { HWND hwndDesktop; } WinQuerySysModalWindow_Args, *PWinQuerySysModalWindow_Args;
/* LONG WinQueryClassName(HWND, LONG cchMax, PCH) */
typedef struct _WinQueryClassName_Args { HWND hwnd; LONG cchMax; /* out: cchMax chars, pcbPayload = cbText */ } WinQueryClassName_Args, *PWinQueryClassName_Args;
/* BOOL WinQueryClassInfo(HAB, PCSZ, PCLASSINFO) */
typedef struct _WinQueryClassInfo_Args {
    ULONG iHab; ULONG cbClass; char szClass[1];
    /* out: CLASSINFO inline (flClassStyle, pfWndProc, cbWindowData) */
    /* TBD: classinfo layout */
} WinQueryClassInfo_Args, *PWinQueryClassInfo_Args;

/* ========================================================================== */
/*  Paint / PS / DC                                                           */
/* ========================================================================== */

#define NWinGetPS                     0x0060L
#define NWinReleasePS                 0x0061L
#define NWinBeginPaint                0x0062L
#define NWinEndPaint                  0x0063L
#define NWinGetScreenPS               0x0064L
#define NWinGetClipPS                 0x0065L
#define NWinGetErasePS                0x0066L
#define NWinReleaseErasePS            0x0067L
#define NWinOpenWindowDC              (0x0068UL | NWIN_SYNC_FLAG)
#define NWinInvalidateRect            0x0069L
#define NWinInvalidateRegion          0x006AL
#define NWinValidateRect              0x006BL
#define NWinValidateRegion            0x006CL
#define NWinUpdateWindow              0x006DL
#define NWinQueryUpdateRect           0x006EL
#define NWinQueryUpdateRegion         0x006FL
#define NWinExcludeUpdateRegion       0x0070L
#define NWinSetClipRegion             0x0071L
#define NWinQueryClipRegion           0x0072L
#define NWinSetWindowDC               0x0073L

/* HPS WinGetPS(HWND) */
typedef struct _WinGetPS_Args { HWND hwnd; } WinGetPS_Args, *PWinGetPS_Args;
/* BOOL WinReleasePS(HPS) */
typedef struct _WinReleasePS_Args { HPS hps; } WinReleasePS_Args, *PWinReleasePS_Args;
/* HPS WinBeginPaint(HWND, HPS, PRECTL) */
typedef struct _WinBeginPaint_Args { HWND hwnd; HPS hps; /* out */ RECTL rcl; } WinBeginPaint_Args, *PWinBeginPaint_Args;
/* BOOL WinEndPaint(HPS) */
typedef struct _WinEndPaint_Args { HPS hps; } WinEndPaint_Args, *PWinEndPaint_Args;
/* HPS WinGetScreenPS(HWND desktop) */
typedef struct _WinGetScreenPS_Args { HWND hwndDesktop; } WinGetScreenPS_Args, *PWinGetScreenPS_Args;
/* HPS WinGetClipPS(HWND, HWND clip, ULONG fl) */
typedef struct _WinGetClipPS_Args { HWND hwnd; HWND hwndClip; ULONG fl; } WinGetClipPS_Args, *PWinGetClipPS_Args;
/* HPS WinGetErasePS(HWND) */
typedef struct _WinGetErasePS_Args { HWND hwnd; } WinGetErasePS_Args, *PWinGetErasePS_Args;
/* BOOL WinReleaseErasePS(HPS) */
typedef struct _WinReleaseErasePS_Args { HPS hps; } WinReleaseErasePS_Args, *PWinReleaseErasePS_Args;
/* BOOL WinInvalidateRect(HWND, PRECTL, BOOL) */
typedef struct _WinInvalidateRect_Args { HWND hwnd; RECTL rcl; BOOL fIncludeChildren; BOOL fNull; } WinInvalidateRect_Args, *PWinInvalidateRect_Args;
/* BOOL WinInvalidateRegion(HWND, HRGN, BOOL) */
typedef struct _WinInvalidateRegion_Args { HWND hwnd; ULONG hrgn; BOOL fIncludeChildren; } WinInvalidateRegion_Args, *PWinInvalidateRegion_Args;
/* BOOL WinValidateRect(HWND, PRECTL, BOOL) */
typedef struct _WinValidateRect_Args { HWND hwnd; RECTL rcl; BOOL fIncludeChildren; BOOL fNull; } WinValidateRect_Args, *PWinValidateRect_Args;
/* BOOL WinValidateRegion(HWND, HRGN, BOOL) */
typedef struct _WinValidateRegion_Args { HWND hwnd; ULONG hrgn; BOOL fIncludeChildren; } WinValidateRegion_Args, *PWinValidateRegion_Args;
/* BOOL WinUpdateWindow(HWND) */
typedef struct _WinUpdateWindow_Args { HWND hwnd; } WinUpdateWindow_Args, *PWinUpdateWindow_Args;
/* BOOL WinQueryUpdateRect(HWND, PRECTL) */
typedef struct _WinQueryUpdateRect_Args { HWND hwnd; /* out */ RECTL rcl; } WinQueryUpdateRect_Args, *PWinQueryUpdateRect_Args;
/* LONG WinQueryUpdateRegion(HWND, HRGN) */
typedef struct _WinQueryUpdateRegion_Args { HWND hwnd; ULONG hrgn; } WinQueryUpdateRegion_Args, *PWinQueryUpdateRegion_Args;
/* LONG WinExcludeUpdateRegion(HPS, HWND) */
typedef struct _WinExcludeUpdateRegion_Args { HPS hps; HWND hwnd; } WinExcludeUpdateRegion_Args, *PWinExcludeUpdateRegion_Args;
/* BOOL WinSetClipRegion(HPS, HRGN) */
typedef struct _WinSetClipRegion_Args { HPS hps; ULONG hrgn; } WinSetClipRegion_Args, *PWinSetClipRegion_Args;
/* HRGN WinQueryClipRegion(HPS) */
typedef struct _WinQueryClipRegion_Args { HPS hps; } WinQueryClipRegion_Args, *PWinQueryClipRegion_Args;
/* BOOL WinSetWindowDC(HWND, HDC) */
typedef struct _WinSetWindowDC_Args { HWND hwnd; HDC hdc; } WinSetWindowDC_Args, *PWinSetWindowDC_Args;

/* ========================================================================== */
/*  Class / Subclass / Thunk                                                  */
/* ========================================================================== */

#define NWinRegisterClass             0x0080L
#define NWinSubclassWindow            0x0081L
#define NWinSetClassThunkProc         0x0082L
#define NWinQueryClassThunkProc       0x0083L
#define NWinSetWindowThunkProc        0x0084L
#define NWinQueryWindowThunkProc      0x0085L
#define NWinDefWindowProc             0x0086L

/* BOOL WinRegisterClass(HAB, PCSZ class, PFNWP, ULONG style, ULONG cbWindowData) */
typedef struct _WinRegisterClass_Args {
    ULONG iHab; ULONG cbClass; char szClass[1];
    ULONG pfnWndProc; ULONG flStyle; ULONG cbWindowData;
} WinRegisterClass_Args, *PWinRegisterClass_Args;
/* PFNWP WinSubclassWindow(HWND, PFNWP) */
typedef struct _WinSubclassWindow_Args { HWND hwnd; ULONG pfnWndProc; /* out */ ULONG pfnOldProc; } WinSubclassWindow_Args, *PWinSubclassWindow_Args;
/* BOOL WinSetClassThunkProc(PCSZ, PVOID) */
typedef struct _WinSetClassThunkProc_Args { ULONG cbClass; char szClass[1]; ULONG pfnThunkProc; } WinSetClassThunkProc_Args, *PWinSetClassThunkProc_Args;
/* PVOID WinQueryClassThunkProc(PCSZ) */
typedef struct _WinQueryClassThunkProc_Args { ULONG cbClass; char szClass[1]; } WinQueryClassThunkProc_Args, *PWinQueryClassThunkProc_Args;
/* PVOID WinSetWindowThunkProc(HWND, PVOID) */
typedef struct _WinSetWindowThunkProc_Args { HWND hwnd; ULONG pfnThunkProc; /* out */ ULONG pfnOldThunkProc; } WinSetWindowThunkProc_Args, *PWinSetWindowThunkProc_Args;
/* PVOID WinQueryWindowThunkProc(HWND) */
typedef struct _WinQueryWindowThunkProc_Args { HWND hwnd; } WinQueryWindowThunkProc_Args, *PWinQueryWindowThunkProc_Args;
/* MRESULT WinDefWindowProc(HWND, ULONG msg, MPARAM mp1, MPARAM mp2) */
typedef struct _WinDefWindowProc_Args { HWND hwnd; ULONG msg; MPARAM mp1; MPARAM mp2; } WinDefWindowProc_Args, *PWinDefWindowProc_Args;

/* ========================================================================== */
/*  Dialogs                                                                   */
/* ========================================================================== */

#define NWinLoadDlg                   0x00A0L
#define NWinCreateDlg                 0x00A1L
#define NWinDlgBox                    0x00A2L
#define NWinProcessDlg                0x00A3L
#define NWinDismissDlg                0x00A4L
#define NWinDefDlgProc                0x00A5L
#define NWinEnumDlgItem               0x00A6L
#define NWinGetDlgMsg                 0x00A7L
#define NWinSetDlgItemShort           0x00A8L
#define NWinQueryDlgItemShort         0x00A9L
#define NWinSetDlgItemText            0x00AAL
#define NWinQueryDlgItemText          0x00ABL
#define NWinQueryDlgItemTextLength    0x00ACL
#define NWinMessageBox                0x00ADL
#define NWinMessageBox2               0x00AEL
#define NWinMapDlgPoints              0x00AFL

/* HWND WinLoadDlg(HWND parent, HWND owner, PFNWP, HMODULE, ULONG idDlg, PVOID createParams) */
typedef struct _WinLoadDlg_Args { HWND hwndParent; HWND hwndOwner; ULONG pfnDlgProc; HMODULE hmod; ULONG idDlg; ULONG pCreateParams; } WinLoadDlg_Args, *PWinLoadDlg_Args;
/* HWND WinCreateDlg(HWND parent, HWND owner, PFNWP, PDLGTEMPLATE, PVOID) */
typedef struct _WinCreateDlg_Args {
    HWND hwndParent; HWND hwndOwner; ULONG pfnDlgProc;
    ULONG cbDlgTemplate; char abDlgTemplate[1];   /* variable */
    ULONG pCreateParams;
} WinCreateDlg_Args, *PWinCreateDlg_Args;
/* ULONG WinDlgBox(HWND parent, HWND owner, PFNWP, HMODULE, ULONG idDlg, PVOID) */
typedef struct _WinDlgBox_Args { HWND hwndParent; HWND hwndOwner; ULONG pfnDlgProc; HMODULE hmod; ULONG idDlg; ULONG pCreateParams; } WinDlgBox_Args, *PWinDlgBox_Args;
/* ULONG WinProcessDlg(HWND dlg) */
typedef struct _WinProcessDlg_Args { HWND hwndDlg; } WinProcessDlg_Args, *PWinProcessDlg_Args;
/* BOOL WinDismissDlg(HWND dlg, ULONG ulResult) */
typedef struct _WinDismissDlg_Args { HWND hwndDlg; ULONG ulResult; } WinDismissDlg_Args, *PWinDismissDlg_Args;
/* MRESULT WinDefDlgProc(HWND, ULONG msg, MPARAM mp1, MPARAM mp2) */
typedef struct _WinDefDlgProc_Args { HWND hwndDlg; ULONG msg; MPARAM mp1; MPARAM mp2; } WinDefDlgProc_Args, *PWinDefDlgProc_Args;
/* HWND WinEnumDlgItem(HWND dlg, HWND hwnd, ULONG code) */
typedef struct _WinEnumDlgItem_Args { HWND hwndDlg; HWND hwnd; ULONG code; } WinEnumDlgItem_Args, *PWinEnumDlgItem_Args;
/* BOOL WinGetDlgMsg(HWND dlg, PQMSG) */
typedef struct _WinGetDlgMsg_Args { HWND hwndDlg; /* out: QMSG, pcbPayload = sizeof(QMSG) */ } WinGetDlgMsg_Args, *PWinGetDlgMsg_Args;
/* BOOL WinSetDlgItemShort(HWND, ULONG id, USHORT, BOOL fSigned) */
typedef struct _WinSetDlgItemShort_Args { HWND hwndDlg; ULONG idItem; USHORT us; BOOL fSigned; } WinSetDlgItemShort_Args, *PWinSetDlgItemShort_Args;
/* USHORT WinQueryDlgItemShort(HWND, ULONG id, PBOOL pSuccess, BOOL fSigned) */
typedef struct _WinQueryDlgItemShort_Args { HWND hwndDlg; ULONG idItem; BOOL fSigned; /* out */ BOOL fSuccess; } WinQueryDlgItemShort_Args, *PWinQueryDlgItemShort_Args;
/* BOOL WinSetDlgItemText(HWND, ULONG id, PCSZ) */
typedef struct _WinSetDlgItemText_Args { HWND hwndDlg; ULONG idItem; ULONG cbText; char szText[1]; } WinSetDlgItemText_Args, *PWinSetDlgItemText_Args;
/* LONG WinQueryDlgItemText(HWND, ULONG id, LONG cchMax, PSZ) */
typedef struct _WinQueryDlgItemText_Args { HWND hwndDlg; ULONG idItem; LONG cchMax; /* out: cchMax chars, pcbPayload = cbText */ } WinQueryDlgItemText_Args, *PWinQueryDlgItemText_Args;
/* LONG WinQueryDlgItemTextLength(HWND, ULONG id) */
typedef struct _WinQueryDlgItemTextLength_Args { HWND hwndDlg; ULONG idItem; } WinQueryDlgItemTextLength_Args, *PWinQueryDlgItemTextLength_Args;
/* ULONG WinMessageBox(HWND owner, HWND parent, PCSZ text, PCSZ title, ULONG idBox, ULONG style) */
typedef struct _WinMessageBox_Args {
    HWND hwndOwner; HWND hwndParent;
    ULONG cbText;   char szText[1];    /* variable */
    ULONG cbTitle;  char szTitle[1];   /* variable */
    ULONG idMsgBox; ULONG flStyle;
} WinMessageBox_Args, *PWinMessageBox_Args;
/* ULONG WinMessageBox2(HWND, HWND, PCSZ, PCSZ, ULONG, PMB2INFO) — MB2INFO inline, TBD */
typedef struct _WinMessageBox2_Args {
    HWND hwndOwner; HWND hwndParent;
    ULONG cbText;  char szText[1];
    ULONG cbTitle; char szTitle[1];
    ULONG idMsgBox;
    ULONG cbMB2INFO; char abMB2INFO[1];
} WinMessageBox2_Args, *PWinMessageBox2_Args;
/* BOOL WinMapDlgPoints(HWND dlg, PPOINTL, ULONG cwpt, BOOL fToDlg) */
typedef struct _WinMapDlgPoints_Args { HWND hwndDlg; ULONG cwpt; BOOL fToDlg; /* in/out: cwpt POINTLs, pcbPayload = cwpt*sizeof(POINTL) */ } WinMapDlgPoints_Args, *PWinMapDlgPoints_Args;

/* ========================================================================== */
/*  Menus / Timers / Pointer / Cursor                                         */
/* ========================================================================== */

#define NWinCreateMenu                0x00C0L
#define NWinLoadMenu                  0x00C1L
#define NWinPopupMenu                 0x00C2L
#define NWinStartTimer                0x00C3L
#define NWinStopTimer                 0x00C4L
#define NWinGetCurrentTime            0x00C5L
#define NWinLoadPointer               0x00C6L
#define NWinCreatePointer             0x00C7L
#define NWinCreatePointerIndirect     0x00C8L
#define NWinDestroyPointer            0x00C9L
#define NWinQueryPointer              0x00CAL
#define NWinQueryPointerInfo          0x00CBL
#define NWinQueryPointerPos           0x00CCL
#define NWinQuerySysPointer           0x00CDL
#define NWinSetPointer                0x00CEL
#define NWinSetPointerPos             0x00CFL
#define NWinShowPointer               0x00D0L
#define NWinShowCursor                0x00D1L
#define NWinQueryCursorInfo           0x00D2L
#define NWinCreateCursor              0x00D3L
#define NWinDestroyCursor             0x00D4L
#define NWinMoveCursor                0x00D5L
#define NWinStretchPointer            0x00D6L
#define NWinSetPointerOwner           0x00D7L
#define NWinQueryPointerClipRect      0x00D8L
#define NWinSetPointerClipRect        0x00D9L
#define NWinLockPointerUpdate         0x00DAL
#define NWinGetSysBitmap              0x00DBL

/* HWND WinCreateMenu(HWND parent, PVOID pMenuTemplate) */
typedef struct _WinCreateMenu_Args { HWND hwndParent; ULONG cbTemplate; char abTemplate[1]; } WinCreateMenu_Args, *PWinCreateMenu_Args;
/* HWND WinLoadMenu(HWND parent, HMODULE hmod, ULONG idMenu) */
typedef struct _WinLoadMenu_Args { HWND hwndParent; HMODULE hmod; ULONG idMenu; } WinLoadMenu_Args, *PWinLoadMenu_Args;
/* BOOL WinPopupMenu(HWND parent, HWND owner, HWND menu, LONG x, LONG y, ULONG idItem, ULONG style) */
typedef struct _WinPopupMenu_Args { HWND hwndParent; HWND hwndOwner; HWND hwndMenu; LONG x, y; ULONG idItem; ULONG flStyle; } WinPopupMenu_Args, *PWinPopupMenu_Args;
/* ULONG WinStartTimer(HAB, HWND, ULONG idTimer, ULONG dtTimeout) */
typedef struct _WinStartTimer_Args { ULONG iHab; HWND hwnd; ULONG idTimer; ULONG dtTimeout; } WinStartTimer_Args, *PWinStartTimer_Args;
/* BOOL WinStopTimer(HAB, HWND, ULONG idTimer) */
typedef struct _WinStopTimer_Args { ULONG iHab; HWND hwnd; ULONG idTimer; } WinStopTimer_Args, *PWinStopTimer_Args;
/* ULONG WinGetCurrentTime(HAB) */
typedef struct _WinGetCurrentTime_Args { ULONG iHab; } WinGetCurrentTime_Args, *PWinGetCurrentTime_Args;
/* HPOINTER WinLoadPointer(HWND desktop, HMODULE hmod, ULONG idPointer) */
typedef struct _WinLoadPointer_Args { HWND hwndDesktop; HMODULE hmod; ULONG idPointer; } WinLoadPointer_Args, *PWinLoadPointer_Args;
/* HPOINTER WinCreatePointer(HWND desktop, HBITMAP hbm, BOOL fSysPointer, LONG xHotspot, LONG yHotspot) */
typedef struct _WinCreatePointer_Args { HWND hwndDesktop; HBITMAP hbm; BOOL fSysPointer; LONG xHotspot, yHotspot; } WinCreatePointer_Args, *PWinCreatePointer_Args;
/* HPOINTER WinCreatePointerIndirect(HWND desktop, PPOINTERINFO) — POINTERINFO inline */
typedef struct _WinCreatePointerIndirect_Args {
    HWND hwndDesktop;
    ULONG fPointer; LONG xHotspot, yHotspot; HBITMAP hbm;
    /* TBD: POINTERINFO layout */
} WinCreatePointerIndirect_Args, *PWinCreatePointerIndirect_Args;
/* BOOL WinDestroyPointer(HPOINTER) */
typedef struct _WinDestroyPointer_Args { ULONG hptr; } WinDestroyPointer_Args, *PWinDestroyPointer_Args;
/* HPOINTER WinQueryPointer(HWND desktop) */
typedef struct _WinQueryPointer_Args { HWND hwndDesktop; } WinQueryPointer_Args, *PWinQueryPointer_Args;
/* BOOL WinQueryPointerInfo(HPOINTER, PPOINTERINFO) — TBD layout */
typedef struct _WinQueryPointerInfo_Args { ULONG hptr; /* out: POINTERINFO, pcbPayload = sizeof */ } WinQueryPointerInfo_Args, *PWinQueryPointerInfo_Args;
/* BOOL WinQueryPointerPos(HWND desktop, PPOINTL) */
typedef struct _WinQueryPointerPos_Args { HWND hwndDesktop; /* out */ POINTL ptl; } WinQueryPointerPos_Args, *PWinQueryPointerPos_Args;
/* HPOINTER WinQuerySysPointer(HWND desktop, LONG id, BOOL fLoad) */
typedef struct _WinQuerySysPointer_Args { HWND hwndDesktop; LONG id; BOOL fLoad; } WinQuerySysPointer_Args, *PWinQuerySysPointer_Args;
/* BOOL WinSetPointer(HWND desktop, HPOINTER hptrNew) */
typedef struct _WinSetPointer_Args { HWND hwndDesktop; ULONG hptrNew; } WinSetPointer_Args, *PWinSetPointer_Args;
/* BOOL WinSetPointerPos(HWND desktop, LONG x, LONG y) */
typedef struct _WinSetPointerPos_Args { HWND hwndDesktop; LONG x, y; } WinSetPointerPos_Args, *PWinSetPointerPos_Args;
/* BOOL WinShowPointer(HWND desktop, BOOL fShow) */
typedef struct _WinShowPointer_Args { HWND hwndDesktop; BOOL fShow; } WinShowPointer_Args, *PWinShowPointer_Args;
/* BOOL WinShowCursor(HWND, BOOL) */
typedef struct _WinShowCursor_Args { HWND hwnd; BOOL fShow; } WinShowCursor_Args, *PWinShowCursor_Args;
/* BOOL WinQueryCursorInfo(HWND desktop, PCURSORINFO) — TBD */
typedef struct _WinQueryCursorInfo_Args { HWND hwndDesktop; /* out: CURSORINFO */ } WinQueryCursorInfo_Args, *PWinQueryCursorInfo_Args;
/* HPOINTER WinCreateCursor(HWND, LONG x,y,cx,cy, ULONG attr, PRECTL clip) */
typedef struct _WinCreateCursor_Args { HWND hwnd; LONG x, y, cx, cy; ULONG flAttr; RECTL rclClip; BOOL fClipSet; } WinCreateCursor_Args, *PWinCreateCursor_Args;
/* BOOL WinDestroyCursor(HWND) */
typedef struct _WinDestroyCursor_Args { HWND hwnd; } WinDestroyCursor_Args, *PWinDestroyCursor_Args;
/* BOOL WinMoveCursor(HWND, LONG x, LONG y) */
typedef struct _WinMoveCursor_Args { HWND hwnd; LONG x, y; } WinMoveCursor_Args, *PWinMoveCursor_Args;
/* BOOL WinStretchPointer(HWND, LONG x,y,cx,cy, HPOINTER, ULONG fl) */
typedef struct _WinStretchPointer_Args { HWND hwnd; LONG x, y, cx, cy; ULONG hptr; ULONG fl; } WinStretchPointer_Args, *PWinStretchPointer_Args;
/* BOOL WinSetPointerOwner(HPOINTER, PID pid, BOOL fDestroy) — PID as ULONG */
typedef struct _WinSetPointerOwner_Args { ULONG hptr; ULONG pid; BOOL fDestroy; } WinSetPointerOwner_Args, *PWinSetPointerOwner_Args;
/* BOOL WinQueryPointerClipRect(HWND desktop, PRECTL) */
typedef struct _WinQueryPointerClipRect_Args { HWND hwndDesktop; /* out */ RECTL rcl; } WinQueryPointerClipRect_Args, *PWinQueryPointerClipRect_Args;
/* BOOL WinSetPointerClipRect(HWND desktop, PRECTL) */
typedef struct _WinSetPointerClipRect_Args { HWND hwndDesktop; RECTL rcl; BOOL fNull; } WinSetPointerClipRect_Args, *PWinSetPointerClipRect_Args;
/* BOOL WinLockPointerUpdate(HWND, BOOL) */
typedef struct _WinLockPointerUpdate_Args { HWND hwnd; BOOL fLock; } WinLockPointerUpdate_Args, *PWinLockPointerUpdate_Args;
/* HBITMAP WinGetSysBitmap(HWND desktop, ULONG id) */
typedef struct _WinGetSysBitmap_Args { HWND hwndDesktop; ULONG id; } WinGetSysBitmap_Args, *PWinGetSysBitmap_Args;

/* ========================================================================== */
/*  Clipboard                                                                 */
/* ========================================================================== */

#define NWinOpenClipbrd               0x00E0L
#define NWinCloseClipbrd              0x00E1L
#define NWinEmptyClipbrd              0x00E2L
#define NWinSetClipbrdOwner           0x00E3L
#define NWinQueryClipbrdOwner         0x00E4L
#define NWinSetClipbrdData            0x00E5L
#define NWinQueryClipbrdData          0x00E6L
#define NWinEnumClipbrdFmts           0x00E7L
#define NWinQueryClipbrdFmtInfo       0x00E8L
#define NWinSetClipbrdViewer          0x00E9L
#define NWinQueryClipbrdViewer        0x00EAL
#define NWinSetSysClipbrdViewer       0x00EBL

/* BOOL WinOpenClipbrd(HAB) */
typedef struct _WinOpenClipbrd_Args { ULONG iHab; } WinOpenClipbrd_Args, *PWinOpenClipbrd_Args;
/* BOOL WinCloseClipbrd(HAB) */
typedef struct _WinCloseClipbrd_Args { ULONG iHab; } WinCloseClipbrd_Args, *PWinCloseClipbrd_Args;
/* BOOL WinEmptyClipbrd(HAB) */
typedef struct _WinEmptyClipbrd_Args { ULONG iHab; } WinEmptyClipbrd_Args, *PWinEmptyClipbrd_Args;
/* BOOL WinSetClipbrdOwner(HAB, HWND) */
typedef struct _WinSetClipbrdOwner_Args { ULONG iHab; HWND hwnd; } WinSetClipbrdOwner_Args, *PWinSetClipbrdOwner_Args;
/* HWND WinQueryClipbrdOwner(HAB) */
typedef struct _WinQueryClipbrdOwner_Args { ULONG iHab; } WinQueryClipbrdOwner_Args, *PWinQueryClipbrdOwner_Args;
/* BOOL WinSetClipbrdData(HAB, ULONG ulData, ULONG fmt, ULONG fl) */
typedef struct _WinSetClipbrdData_Args { ULONG iHab; ULONG ulData; ULONG fmt; ULONG fl; } WinSetClipbrdData_Args, *PWinSetClipbrdData_Args;
/* ULONG WinQueryClipbrdData(HAB, ULONG fmt) */
typedef struct _WinQueryClipbrdData_Args { ULONG iHab; ULONG fmt; } WinQueryClipbrdData_Args, *PWinQueryClipbrdData_Args;
/* ULONG WinEnumClipbrdFmts(HAB, ULONG fmt) */
typedef struct _WinEnumClipbrdFmts_Args { ULONG iHab; ULONG fmt; } WinEnumClipbrdFmts_Args, *PWinEnumClipbrdFmts_Args;
/* BOOL WinQueryClipbrdFmtInfo(HAB, ULONG fmt, PFORMATINFO) — FORMATINFO inline */
typedef struct _WinQueryClipbrdFmtInfo_Args { ULONG iHab; ULONG fmt; /* out */ ULONG fFmtInfo; ULONG cbFmtInfo; } WinQueryClipbrdFmtInfo_Args, *PWinQueryClipbrdFmtInfo_Args;
/* HWND WinSetClipbrdViewer(HAB, HWND hwndNewViewer) */
typedef struct _WinSetClipbrdViewer_Args { ULONG iHab; HWND hwndNewViewer; /* out */ HWND hwndOldViewer; } WinSetClipbrdViewer_Args, *PWinSetClipbrdViewer_Args;
/* HWND WinQueryClipbrdViewer(HAB) */
typedef struct _WinQueryClipbrdViewer_Args { ULONG iHab; } WinQueryClipbrdViewer_Args, *PWinQueryClipbrdViewer_Args;
/* BOOL WinSetSysClipbrdViewer(HAB, HWND) */
typedef struct _WinSetSysClipbrdViewer_Args { ULONG iHab; HWND hwnd; } WinSetSysClipbrdViewer_Args, *PWinSetSysClipbrdViewer_Args;

/* ========================================================================== */
/*  Atoms                                                                     */
/* ========================================================================== */

#define NWinAddAtom                   0x00F0L
#define NWinFindAtom                  0x00F1L
#define NWinDeleteAtom                0x00F2L
#define NWinQueryAtomLength           0x00F3L
#define NWinQueryAtomName             0x00F4L
#define NWinQueryAtomUsage            0x00F5L
#define NWinQuerySystemAtomTable      0x00F6L
#define NWinCreateAtomTable           0x00F7L
#define NWinDestroyAtomTable          0x00F8L
#define NWinSetAtomTableOwner         0x00F9L

/* ATOM WinAddAtom(HATOMTBL, PCSZ) — ATOM as ULONG */
typedef struct _WinAddAtom_Args { ULONG hatomtbl; ULONG cbName; char szName[1]; } WinAddAtom_Args, *PWinAddAtom_Args;
/* ATOM WinFindAtom(HATOMTBL, PCSZ) */
typedef struct _WinFindAtom_Args { ULONG hatomtbl; ULONG cbName; char szName[1]; } WinFindAtom_Args, *PWinFindAtom_Args;
/* ATOM WinDeleteAtom(HATOMTBL, ATOM) */
typedef struct _WinDeleteAtom_Args { ULONG hatomtbl; ULONG atom; } WinDeleteAtom_Args, *PWinDeleteAtom_Args;
/* ULONG WinQueryAtomLength(HATOMTBL, ATOM) */
typedef struct _WinQueryAtomLength_Args { ULONG hatomtbl; ULONG atom; } WinQueryAtomLength_Args, *PWinQueryAtomLength_Args;
/* ULONG WinQueryAtomName(HATOMTBL, ATOM, PSZ, ULONG cbBuf) */
typedef struct _WinQueryAtomName_Args { ULONG hatomtbl; ULONG atom; ULONG cbBuffer; /* out: cbBuffer chars, pcbPayload = cb */ } WinQueryAtomName_Args, *PWinQueryAtomName_Args;
/* ULONG WinQueryAtomUsage(HATOMTBL, ATOM) */
typedef struct _WinQueryAtomUsage_Args { ULONG hatomtbl; ULONG atom; } WinQueryAtomUsage_Args, *PWinQueryAtomUsage_Args;
/* HATOMTBL WinQuerySystemAtomTable(VOID) */
typedef struct _WinQuerySystemAtomTable_Args { ULONG dummy; } WinQuerySystemAtomTable_Args, *PWinQuerySystemAtomTable_Args;
/* HATOMTBL WinCreateAtomTable(ULONG initial, ULONG buckets) */
typedef struct _WinCreateAtomTable_Args { ULONG initial; ULONG buckets; } WinCreateAtomTable_Args, *PWinCreateAtomTable_Args;
/* BOOL WinDestroyAtomTable(HATOMTBL) */
typedef struct _WinDestroyAtomTable_Args { ULONG hatomtbl; } WinDestroyAtomTable_Args, *PWinDestroyAtomTable_Args;
/* BOOL WinSetAtomTableOwner(HATOMTBL, PID) */
typedef struct _WinSetAtomTableOwner_Args { ULONG hatomtbl; ULONG pid; } WinSetAtomTableOwner_Args, *PWinSetAtomTableOwner_Args;

/* ========================================================================== */
/*  Strings / Codepages / Accel / Hooks / Resource                            */
/* ========================================================================== */

#define NWinLoadString                0x0100L
#define NWinLoadMessage               0x0101L
#define NWinLoadLibrary               0x0102L
#define NWinLoadProcedure             0x0103L
#define NWinDeleteLibrary             0x0104L
#define NWinDeleteProcedure           0x0105L
#define NWinQueryCp                   0x0106L
#define NWinQueryCpList               0x0107L
#define NWinSetCp                     0x0108L
#define NWinCpTranslateString         0x0109L
#define NWinCpTranslateChar           0x010AL
#define NWinCompareStrings            0x010BL
#define NWinUpper                     0x010CL
#define NWinUpperChar                 0x010DL
#define NWinNextChar                  0x010EL
#define NWinPrevChar                  0x010FL
#define NWinSubstituteStrings         0x0110L
#define NWinLoadAccelTable            0x0111L
#define NWinCreateAccelTable          0x0112L
#define NWinDestroyAccelTable         0x0113L
#define NWinTranslateAccel            0x0114L
#define NWinSetAccelTable             0x0115L
#define NWinQueryAccelTable           0x0116L
#define NWinCopyAccelTable            0x0117L
#define NWinSetAccelTableValue        0x0118L
#define NWinQueryAccelTableValue      0x0119L
#define NWinSetHook                   0x011AL
#define NWinReleaseHook               0x011BL
#define NWinCallMsgFilter             0x011CL
#define NWinSetMsgInterest            0x011DL
#define NWinSetClassMsgInterest       0x011EL
#define NWinRegisterUserMsg           0x011FL
#define NWinRegisterUserDatatype      0x0120L

/* LONG WinLoadString(HAB, HMODULE, ULONG id, LONG cchMax, PSZ) */
typedef struct _WinLoadString_Args { ULONG iHab; HMODULE hmod; ULONG id; LONG cchMax; /* out: cchMax chars, pcbPayload = cb */ } WinLoadString_Args, *PWinLoadString_Args;
/* LONG WinLoadMessage(HAB, HMODULE, ULONG id, LONG cchMax, PSZ) */
typedef struct _WinLoadMessage_Args { ULONG iHab; HMODULE hmod; ULONG id; LONG cchMax; /* out: cchMax chars, pcbPayload = cb */ } WinLoadMessage_Args, *PWinLoadMessage_Args;
/* HMODULE WinLoadLibrary(HAB, PCSZ) */
typedef struct _WinLoadLibrary_Args { ULONG iHab; ULONG cbLib; char szLib[1]; } WinLoadLibrary_Args, *PWinLoadLibrary_Args;
/* PFN WinLoadProcedure(HMODULE, PCSZ) */
typedef struct _WinLoadProcedure_Args { HMODULE hmod; ULONG cbProc; char szProc[1]; } WinLoadProcedure_Args, *PWinLoadProcedure_Args;
/* BOOL WinDeleteLibrary(HAB, HMODULE) */
typedef struct _WinDeleteLibrary_Args { ULONG iHab; HMODULE hmod; } WinDeleteLibrary_Args, *PWinDeleteLibrary_Args;
/* BOOL WinDeleteProcedure(HAB, PFN) */
typedef struct _WinDeleteProcedure_Args { ULONG iHab; ULONG pfn; } WinDeleteProcedure_Args, *PWinDeleteProcedure_Args;
/* ULONG WinQueryCp(HMQ) */
typedef struct _WinQueryCp_Args { ULONG hmq; } WinQueryCp_Args, *PWinQueryCp_Args;
/* ULONG WinQueryCpList(HAB, ULONG ulcount, PULONG aulCP) */
typedef struct _WinQueryCpList_Args { ULONG iHab; ULONG ulcount; /* out: ulcount ULONGs, pcbPayload = ulcount*4 */ } WinQueryCpList_Args, *PWinQueryCpList_Args;
/* BOOL WinSetCp(HMQ, ULONG cp) */
typedef struct _WinSetCp_Args { ULONG hmq; ULONG cp; } WinSetCp_Args, *PWinSetCp_Args;
/* LONG WinCpTranslateString(HAB, ULONG srcCp, PCSZ src, ULONG dstCp, LONG cchDst, PSZ dst) */
typedef struct _WinCpTranslateString_Args {
    ULONG iHab; ULONG ulSrcCp; ULONG cbSrc; char szSrc[1];
    ULONG ulDstCp; LONG cchDst;
    /* out: cchDst chars, pcbPayload = cb */
} WinCpTranslateString_Args, *PWinCpTranslateString_Args;
/* BOOL WinCpTranslateChar(HAB, ULONG srcCp, ULONG chSrc, ULONG dstCp, PCH pchDst) */
typedef struct _WinCpTranslateChar_Args { ULONG iHab; ULONG ulSrcCp; ULONG chSrc; ULONG ulDstCp; /* out */ CHAR chDst; } WinCpTranslateChar_Args, *PWinCpTranslateChar_Args;
/* LONG WinCompareStrings(HAB, ULONG codepage, ULONG country, PCSZ key1, PCSZ key2, ULONG res) */
typedef struct _WinCompareStrings_Args { ULONG iHab; ULONG cp; ULONG country; ULONG cbKey1; char szKey1[1]; ULONG cbKey2; char szKey2[1]; ULONG res; } WinCompareStrings_Args, *PWinCompareStrings_Args;
/* PSZ WinUpper(HAB, ULONG cp, ULONG country, PCSZ src, PSZ dst) */
typedef struct _WinUpper_Args { ULONG iHab; ULONG cp; ULONG country; ULONG cbSrc; char szSrc[1]; /* out: same length, pcbPayload = cbSrc */ } WinUpper_Args, *PWinUpper_Args;
/* ULONG WinUpperChar(HAB, ULONG cp, ULONG country, ULONG chSrc) */
typedef struct _WinUpperChar_Args { ULONG iHab; ULONG cp; ULONG country; ULONG chSrc; } WinUpperChar_Args, *PWinUpperChar_Args;
/* ULONG WinNextChar(HAB, ULONG cp, ULONG country, PCSZ current) — returns byte offset */
typedef struct _WinNextChar_Args { ULONG iHab; ULONG cp; ULONG country; ULONG offCur; } WinNextChar_Args, *PWinNextChar_Args;
/* ULONG WinPrevChar(HAB, ULONG cp, ULONG country, PCSZ start, PCSZ current) — returns byte offset */
typedef struct _WinPrevChar_Args { ULONG iHab; ULONG cp; ULONG country; ULONG offStart; ULONG offCur; } WinPrevChar_Args, *PWinPrevChar_Args;
/* LONG WinSubstituteStrings(HWND, PCSZ src, LONG lMaxLen, PSZ dst) */
typedef struct _WinSubstituteStrings_Args { HWND hwnd; ULONG cbSrc; char szSrc[1]; LONG cchMax; /* out: cchMax chars */ } WinSubstituteStrings_Args, *PWinSubstituteStrings_Args;
/* HACCEL WinLoadAccelTable(HAB, HMODULE, ULONG idTable) */
typedef struct _WinLoadAccelTable_Args { ULONG iHab; HMODULE hmod; ULONG idTable; } WinLoadAccelTable_Args, *PWinLoadAccelTable_Args;
/* HACCEL WinCreateAccelTable(HAB, PACCELTABLE) — TBD layout */
typedef struct _WinCreateAccelTable_Args { ULONG iHab; ULONG cbAccelTable; char abAccelTable[1]; } WinCreateAccelTable_Args, *PWinCreateAccelTable_Args;
/* BOOL WinDestroyAccelTable(HACCEL) */
typedef struct _WinDestroyAccelTable_Args { ULONG haccel; } WinDestroyAccelTable_Args, *PWinDestroyAccelTable_Args;
/* BOOL WinTranslateAccel(HAB, HWND, HACCEL, PQMSG) */
typedef struct _WinTranslateAccel_Args { ULONG iHab; HWND hwnd; ULONG haccel; /* in/out QMSG */ HWND qmsgHwnd; ULONG qmsgMsg; MPARAM qmsgMp1, qmsgMp2; ULONG qmsgTime; POINTL qmsgPtl; ULONG qmsgReserved; } WinTranslateAccel_Args, *PWinTranslateAccel_Args;
/* BOOL WinSetAccelTable(HAB, HACCEL, HWND) */
typedef struct _WinSetAccelTable_Args { ULONG iHab; ULONG haccel; HWND hwnd; } WinSetAccelTable_Args, *PWinSetAccelTable_Args;
/* HACCEL WinQueryAccelTable(HAB, HWND) */
typedef struct _WinQueryAccelTable_Args { ULONG iHab; HWND hwnd; } WinQueryAccelTable_Args, *PWinQueryAccelTable_Args;
/* ULONG WinCopyAccelTable(HACCEL, PACCELTABLE, ULONG cbCopy) */
typedef struct _WinCopyAccelTable_Args { ULONG haccel; ULONG cbCopy; /* out: cbCopy bytes */ } WinCopyAccelTable_Args, *PWinCopyAccelTable_Args;
/* BOOL WinSetAccelTableValue(HACCEL, PACCELTABLEENTRY) — TBD */
typedef struct _WinSetAccelTableValue_Args { ULONG haccel; ULONG cbEntry; char abEntry[1]; } WinSetAccelTableValue_Args, *PWinSetAccelTableValue_Args;
/* BOOL WinQueryAccelTableValue(HACCEL, PACCELTABLEENTRY) — TBD */
typedef struct _WinQueryAccelTableValue_Args { ULONG haccel; ULONG cbEntry; char abEntry[1]; } WinQueryAccelTableValue_Args, *PWinQueryAccelTableValue_Args;
/* BOOL WinSetHook(HAB, HMQ, LONG idHook, PFNHOOK, HMODULE) */
typedef struct _WinSetHook_Args { ULONG iHab; ULONG hmq; LONG idHook; ULONG pfnHook; HMODULE hmod; } WinSetHook_Args, *PWinSetHook_Args;
/* BOOL WinReleaseHook(HAB, HMQ, LONG idHook, PFNHOOK, HMODULE) */
typedef struct _WinReleaseHook_Args { ULONG iHab; ULONG hmq; LONG idHook; ULONG pfnHook; HMODULE hmod; } WinReleaseHook_Args, *PWinReleaseHook_Args;
/* BOOL WinCallMsgFilter(HAB, PQMSG, LONG idFilter) */
typedef struct _WinCallMsgFilter_Args { ULONG iHab; HWND hwnd; ULONG msg; MPARAM mp1; MPARAM mp2; ULONG time; POINTL ptl; ULONG ulReserved; LONG idFilter; } WinCallMsgFilter_Args, *PWinCallMsgFilter_Args;
typedef struct _WinSetMsgInterest_Args { ULONG iHab; HWND hwnd; ULONG ulClass; ULONG flInterest; } WinSetMsgInterest_Args, *PWinSetMsgInterest_Args;
/* BOOL WinSetClassMsgInterest(HAB, PCSZ class, ULONG ulClass, ULONG flInterest) */
typedef struct _WinSetClassMsgInterest_Args { ULONG iHab; ULONG cbClass; char szClass[1]; ULONG ulClass; ULONG flInterest; } WinSetClassMsgInterest_Args, *PWinSetClassMsgInterest_Args;
/* BOOL WinRegisterUserMsg(HAB, ULONG id, ULONG type, LONG size, ULONG fl) */
typedef struct _WinRegisterUserMsg_Args { ULONG iHab; ULONG ulMsgId; ULONG ulMsgType; LONG lSize; ULONG ulFlags; } WinRegisterUserMsg_Args, *PWinRegisterUserMsg_Args;
/* BOOL WinRegisterUserDatatype(HAB, ULONG id, PCSZ) */
typedef struct _WinRegisterUserDatatype_Args { ULONG iHab; ULONG ulMsgId; ULONG cbName; char szName[1]; } WinRegisterUserDatatype_Args, *PWinRegisterUserDatatype_Args;

/* ========================================================================== */
/*  Errors / Version                                                          */
/* ========================================================================== */

#define NWinGetErrorInfo              0x0130L
#define NWinFreeErrorInfo             0x0131L
#define NWinSetErrorInfo              0x0132L

/* PERRINFO WinGetErrorInfo(HAB) — PERRINFO as ULONG (opaque, out via cbPayload) */
typedef struct _WinGetErrorInfo_Args { ULONG iHab; /* out: ERRINFO bytes, pcbPayload = sizeof */ } WinGetErrorInfo_Args, *PWinGetErrorInfo_Args;
/* BOOL WinFreeErrorInfo(PERRINFO) */
typedef struct _WinFreeErrorInfo_Args { ULONG perri; } WinFreeErrorInfo_Args, *PWinFreeErrorInfo_Args;
/* ERRORID WinSetErrorInfo(ERRORID) */
typedef struct _WinSetErrorInfo_Args { ULONG errid; } WinSetErrorInfo_Args, *PWinSetErrorInfo_Args;

/* ========================================================================== */
/*  Sys / Desktop                                                             */
/* ========================================================================== */

#define NWinSetSysColors              0x0140L
#define NWinQuerySysColor             0x0141L
#define NWinSetSysValue               0x0142L
#define NWinQuerySysValue             0x0143L
#define NWinRealizePalette            0x0144L
#define NWinSetSysFont                0x0145L
#define NWinSetDesktopBkgnd           0x0146L
#define NWinQueryDesktopBkgnd         0x0147L
#define NWinSetDesktopWorkArea        0x0148L
#define NWinQueryDesktopWorkArea      0x0149L
#define NWinQueryVisibleRegion        0x014AL
#define NWinSetVisibleRegionNotify    0x014BL
#define NWinSetSysPointerData         0x014CL
#define NWinQuerySysPointerData       0x014DL
#define NWinQueryQueueInfo            0x014EL

/* BOOL WinSetSysColors(LONG format, LONG first, LONG count, PLONG alColors) */
typedef struct _WinSetSysColors_Args { LONG lFormat; LONG lFirst; LONG lCount; /* in: lCount LONGs; pcbPayload */ } WinSetSysColors_Args, *PWinSetSysColors_Args;
/* ULONG WinQuerySysColor(HWND desktop, LONG index, LONG res) */
typedef struct _WinQuerySysColor_Args { HWND hwndDesktop; LONG lColorIndex; LONG lReserved; } WinQuerySysColor_Args, *PWinQuerySysColor_Args;
/* BOOL WinSetSysValue(HWND desktop, LONG index, ULONG value) */
typedef struct _WinSetSysValue_Args { HWND hwndDesktop; LONG lIndex; ULONG ulValue; } WinSetSysValue_Args, *PWinSetSysValue_Args;
/* ULONG WinQuerySysValue(HWND desktop, LONG index) */
typedef struct _WinQuerySysValue_Args { HWND hwndDesktop; LONG lIndex; } WinQuerySysValue_Args, *PWinQuerySysValue_Args;
/* ULONG WinRealizePalette(HWND, HPS, PULONG pulcChg) */
typedef struct _WinRealizePalette_Args { HWND hwnd; HPS hps; /* out */ ULONG ulcChg; } WinRealizePalette_Args, *PWinRealizePalette_Args;
/* BOOL WinSetSysFont(HWND desktop, ULONG flag, PCSZ fontName) */
typedef struct _WinSetSysFont_Args { HWND hwndDesktop; ULONG ulFlag; ULONG cbFontName; char szFontName[1]; } WinSetSysFont_Args, *PWinSetSysFont_Args;
/* BOOL WinSetDesktopBkgnd(HWND desktop, PDESKTOP) — TBD */
typedef struct _WinSetDesktopBkgnd_Args { HWND hwndDesktop; ULONG cbDesktop; char abDesktop[1]; } WinSetDesktopBkgnd_Args, *PWinSetDesktopBkgnd_Args;
/* BOOL WinQueryDesktopBkgnd(HWND desktop, PDESKTOP) — TBD */
typedef struct _WinQueryDesktopBkgnd_Args { HWND hwndDesktop; /* out: cbDesktop bytes, pcbPayload */ } WinQueryDesktopBkgnd_Args, *PWinQueryDesktopBkgnd_Args;
/* BOOL WinSetDesktopWorkArea(HWND desktop, PDESKTOP) — TBD */
typedef struct _WinSetDesktopWorkArea_Args { HWND hwndDesktop; ULONG cbDesktop; char abDesktop[1]; } WinSetDesktopWorkArea_Args, *PWinSetDesktopWorkArea_Args;
/* BOOL WinQueryDesktopWorkArea(HWND desktop, PDESKTOP) — TBD */
typedef struct _WinQueryDesktopWorkArea_Args { HWND hwndDesktop; /* out: cbDesktop bytes, pcbPayload */ } WinQueryDesktopWorkArea_Args, *PWinQueryDesktopWorkArea_Args;
/* ULONG WinQueryVisibleRegion(HWND, HRGN) */
typedef struct _WinQueryVisibleRegion_Args { HWND hwnd; ULONG hrgn; } WinQueryVisibleRegion_Args, *PWinQueryVisibleRegion_Args;
/* BOOL WinSetVisibleRegionNotify(HWND, BOOL) */
typedef struct _WinSetVisibleRegionNotify_Args { HWND hwnd; BOOL fNotify; } WinSetVisibleRegionNotify_Args, *PWinSetVisibleRegionNotify_Args;
/* BOOL WinSetSysPointerData(HWND desktop, LONG id, PPOINTERINFO) — TBD */
typedef struct _WinSetSysPointerData_Args { HWND hwndDesktop; LONG id; ULONG cbPointerInfo; char abPointerInfo[1]; } WinSetSysPointerData_Args, *PWinSetSysPointerData_Args;
/* BOOL WinQuerySysPointerData(HWND desktop, LONG id, PPOINTERINFO) — TBD */
typedef struct _WinQuerySysPointerData_Args { HWND hwndDesktop; LONG id; /* out: POINTERINFO */ } WinQuerySysPointerData_Args, *PWinQuerySysPointerData_Args;
/* BOOL WinQueryQueueInfo(HMQ, PMQINFO, ULONG cbCopied) — TBD layout */
typedef struct _WinQueryQueueInfo_Args { ULONG hmq; ULONG cbCopy; /* out: MQINFO bytes */ } WinQueryQueueInfo_Args, *PWinQueryQueueInfo_Args;

/* ========================================================================== */
/*  Rect helpers                                                              */
/* ========================================================================== */

#define NWinSetRect                   0x0120L
#define NWinSetRectEmpty              0x0121L
#define NWinCopyRect                  0x0122L
#define NWinEqualRect                 0x0123L
#define NWinPtInRect                  0x0124L
#define NWinIsRectEmpty               0x0125L
#define NWinOffsetRect                0x0126L
#define NWinInflateRect               0x0127L
#define NWinIntersectRect             0x0128L
#define NWinUnionRect                 0x0129L
#define NWinSubtractRect              0x012AL
#define NWinMakeRect                  0x012BL
#define NWinMakePoints                0x012CL
#define NWinFillRect                  0x012DL
#define NWinInvertRect                0x012EL

/* BOOL WinSetRect(HAB, PRECTL, LONG l, LONG b, LONG r, LONG t) */
typedef struct _WinSetRect_Args { ULONG iHab; /* in/out */ RECTL rcl; LONG lLeft; LONG lBottom; LONG lRight; LONG lTop; } WinSetRect_Args, *PWinSetRect_Args;
/* BOOL WinSetRectEmpty(HAB, PRECTL) */
typedef struct _WinSetRectEmpty_Args { ULONG iHab; /* out */ RECTL rcl; } WinSetRectEmpty_Args, *PWinSetRectEmpty_Args;
/* BOOL WinCopyRect(HAB, PRECTL dst, PRECTL src) */
typedef struct _WinCopyRect_Args { ULONG iHab; RECTL rclSrc; /* out */ RECTL rclDst; } WinCopyRect_Args, *PWinCopyRect_Args;
/* BOOL WinEqualRect(HAB, PRECTL, PRECTL) */
typedef struct _WinEqualRect_Args { ULONG iHab; RECTL rcl1; RECTL rcl2; } WinEqualRect_Args, *PWinEqualRect_Args;
/* BOOL WinPtInRect(HAB, PRECTL, PPOINTL) */
typedef struct _WinPtInRect_Args { ULONG iHab; RECTL rcl; POINTL ptl; } WinPtInRect_Args, *PWinPtInRect_Args;
/* BOOL WinIsRectEmpty(HAB, PRECTL) */
typedef struct _WinIsRectEmpty_Args { ULONG iHab; RECTL rcl; } WinIsRectEmpty_Args, *PWinIsRectEmpty_Args;
/* BOOL WinOffsetRect(HAB, PRECTL, LONG dx, LONG dy) */
typedef struct _WinOffsetRect_Args { ULONG iHab; /* in/out */ RECTL rcl; LONG dx; LONG dy; } WinOffsetRect_Args, *PWinOffsetRect_Args;
/* BOOL WinInflateRect(HAB, PRECTL, LONG dx, LONG dy) */
typedef struct _WinInflateRect_Args { ULONG iHab; /* in/out */ RECTL rcl; LONG dx; LONG dy; } WinInflateRect_Args, *PWinInflateRect_Args;
/* BOOL WinIntersectRect(HAB, PRECTL dst, PRECTL src1, PRECTL src2) */
typedef struct _WinIntersectRect_Args { ULONG iHab; RECTL rclSrc1; RECTL rclSrc2; /* out */ RECTL rclDst; } WinIntersectRect_Args, *PWinIntersectRect_Args;
/* BOOL WinUnionRect(HAB, PRECTL dst, PRECTL src1, PRECTL src2) */
typedef struct _WinUnionRect_Args { ULONG iHab; RECTL rclSrc1; RECTL rclSrc2; /* out */ RECTL rclDst; } WinUnionRect_Args, *PWinUnionRect_Args;
/* BOOL WinSubtractRect(HAB, PRECTL dst, PRECTL src1, PRECTL src2) — TBD layout (dst may be array) */
typedef struct _WinSubtractRect_Args { ULONG iHab; RECTL rclSrc1; RECTL rclSrc2; ULONG cRects; /* out: cRects RECTLs */ } WinSubtractRect_Args, *PWinSubtractRect_Args;
/* BOOL WinMakeRect(HAB, PRECTL, PPOINTL, PPOINTL) */
typedef struct _WinMakeRect_Args { ULONG iHab; POINTL ptl1; POINTL ptl2; /* out */ RECTL rcl; } WinMakeRect_Args, *PWinMakeRect_Args;
/* BOOL WinMakePoints(HAB, PPOINTL, ULONG, PRECTL) */
typedef struct _WinMakePoints_Args { ULONG iHab; ULONG cNum; RECTL rcl; /* out: cNum POINTLs */ } WinMakePoints_Args, *PWinMakePoints_Args;
/* BOOL WinFillRect(HPS, PRECTL, LONG color) */
typedef struct _WinFillRect_Args { HPS hps; RECTL rcl; LONG lColor; } WinFillRect_Args, *PWinFillRect_Args;
/* BOOL WinInvertRect(HPS, PRECTL) */
typedef struct _WinInvertRect_Args { HPS hps; RECTL rcl; } WinInvertRect_Args, *PWinInvertRect_Args;

/* ========================================================================== */
/*  Input / Keyboard                                                          */
/* ========================================================================== */

#define NWinGetKeyState               0x0140L
#define NWinGetPhysKeyState           0x0141L
#define NWinSetKeyboardStateTable     0x0142L
#define NWinEnablePhysInput           0x0143L
#define NWinIsPhysInputEnabled        0x0144L
#define NWinLockInput                 0x0145L
#define NWinCheckInput                0x0146L
#define NWinAlarm                     0x0147L

/* LONG WinGetKeyState(HWND desktop, LONG vkey) */
typedef struct _WinGetKeyState_Args { HWND hwndDesktop; LONG vkey; } WinGetKeyState_Args, *PWinGetKeyState_Args;
/* LONG WinGetPhysKeyState(HWND desktop, LONG vkey) */
typedef struct _WinGetPhysKeyState_Args { HWND hwndDesktop; LONG vkey; } WinGetPhysKeyState_Args, *PWinGetPhysKeyState_Args;
/* BOOL WinSetKeyboardStateTable(HWND desktop, PBYTE, BOOL fSet) */
typedef struct _WinSetKeyboardStateTable_Args { HWND hwndDesktop; BYTE abKeyState[256]; BOOL fSet; } WinSetKeyboardStateTable_Args, *PWinSetKeyboardStateTable_Args;
/* BOOL WinEnablePhysInput(HWND desktop, BOOL) */
typedef struct _WinEnablePhysInput_Args { HWND hwndDesktop; BOOL fEnable; } WinEnablePhysInput_Args, *PWinEnablePhysInput_Args;
/* BOOL WinIsPhysInputEnabled(HWND desktop) */
typedef struct _WinIsPhysInputEnabled_Args { HWND hwndDesktop; } WinIsPhysInputEnabled_Args, *PWinIsPhysInputEnabled_Args;
/* BOOL WinLockInput(HAB, HWND desktop) */
typedef struct _WinLockInput_Args { ULONG iHab; HWND hwndDesktop; } WinLockInput_Args, *PWinLockInput_Args;
/* BOOL WinCheckInput(VOID) */
typedef struct _WinCheckInput_Args { ULONG dummy; } WinCheckInput_Args, *PWinCheckInput_Args;
/* BOOL WinAlarm(HWND desktop, ULONG type) */
typedef struct _WinAlarm_Args { HWND hwndDesktop; ULONG ulType; } WinAlarm_Args, *PWinAlarm_Args;

/* ========================================================================== */
/*  Draw helpers                                                              */
/* ========================================================================== */

#define NWinDrawBitmap                0x0150L
#define NWinDrawBorder                0x0151L
#define NWinDrawPointer               0x0152L
#define NWinDrawText                  0x0153L
#define NWinDrawTabbedText            0x0154L
#define NWinShowTrackRect             0x0155L
#define NWinTrackRect                 0x0156L

/* BOOL WinDrawBitmap(HPS, HBITMAP, PRECTL src, PPOINTL dst, LONG fg, LONG bg, ULONG fl) */
typedef struct _WinDrawBitmap_Args { HPS hps; HBITMAP hbm; RECTL rclSrc; POINTL ptlDst; LONG lColorBack; LONG lColorFore; ULONG fl; } WinDrawBitmap_Args, *PWinDrawBitmap_Args;
/* BOOL WinDrawBorder(HPS, PRECTL, LONG thick, LONG thick2, LONG fg, LONG bg, ULONG fl) */
typedef struct _WinDrawBorder_Args { HPS hps; RECTL rcl; LONG lThick; LONG lThick2; LONG lColorBack; LONG lColorFore; ULONG fl; } WinDrawBorder_Args, *PWinDrawBorder_Args;
/* BOOL WinDrawPointer(HPS, LONG x, LONG y, HPOINTER, LONG color) */
typedef struct _WinDrawPointer_Args { HPS hps; LONG x, y; ULONG hptr; LONG lColor; } WinDrawPointer_Args, *PWinDrawPointer_Args;
/* LONG WinDrawText(HPS, LONG cch, PCH, PRECTL, LONG fg, LONG bg, ULONG fl) */
typedef struct _WinDrawText_Args { HPS hps; LONG cch; char szText[1]; RECTL rcl; LONG lColorFore; LONG lColorBack; ULONG fl; } WinDrawText_Args, *PWinDrawText_Args;
/* LONG WinDrawTabbedText(HPS, LONG cch, PCH, PRECTL, PLONG pAdx, LONG fg, LONG bg, ULONG fl) */
typedef struct _WinDrawTabbedText_Args { HPS hps; LONG cch; char szText[1]; RECTL rcl; LONG lColorFore; LONG lColorBack; ULONG fl; } WinDrawTabbedText_Args, *PWinDrawTabbedText_Args;
/* BOOL WinShowTrackRect(HWND, BOOL) */
typedef struct _WinShowTrackRect_Args { HWND hwnd; BOOL fShow; } WinShowTrackRect_Args, *PWinShowTrackRect_Args;
/* BOOL WinTrackRect(HWND, HPS, PTRACKINFO) — TRACKINFO inline TBD */
typedef struct _WinTrackRect_Args { HWND hwnd; HPS hps; ULONG cbTrackInfo; char abTrackInfo[1]; } WinTrackRect_Args, *PWinTrackRect_Args;

/* ========================================================================== */
/*  Frame / calc                                                              */
/* ========================================================================== */

#define NWinCalcFrameRect             0x0160L
#define NWinCreateFrameControls       0x0161L
#define NWinGetMaxPosition            0x0162L
#define NWinGetMinPosition            0x0163L
#define NWinGetFrameTreePPSize        0x0164L
#define NWinGetFrameTreePPs           0x0165L
#define NWinSetFrameTreePPs           0x0166L
#define NWinQueryProcessCP            0x0167L
#define NWinCancelShutdown            0x0168L
#define NWinSystemShutdown            0x0169L

/* BOOL WinCalcFrameRect(HWND frame, PRECTL, BOOL fClient) */
typedef struct _WinCalcFrameRect_Args { HWND hwndFrame; /* in/out */ RECTL rcl; BOOL fClient; } WinCalcFrameRect_Args, *PWinCalcFrameRect_Args;
/* BOOL WinCreateFrameControls(HWND, PFRAMECDATA, PCSZ title) */
typedef struct _WinCreateFrameControls_Args { HWND hwndFrame; ULONG cbFrameCD; char abFrameCD[1]; ULONG cbTitle; char szTitle[1]; } WinCreateFrameControls_Args, *PWinCreateFrameControls_Args;
/* BOOL WinGetMaxPosition(HWND, PSWP) */
typedef struct _WinGetMaxPosition_Args { HWND hwnd; /* out */ HWND hwndInsertBehind; LONG x, y, cx, cy; ULONG fl; } WinGetMaxPosition_Args, *PWinGetMaxPosition_Args;
/* BOOL WinGetMinPosition(HWND, PSWP, PPOINTL) */
typedef struct _WinGetMinPosition_Args { HWND hwnd; /* out */ HWND hwndInsertBehind; LONG x, y, cx, cy; ULONG fl; POINTL ptl; } WinGetMinPosition_Args, *PWinGetMinPosition_Args;
/* BOOL WinGetFrameTreePPSize(HWND frame, PULONG pSize) */
typedef struct _WinGetFrameTreePPSize_Args { HWND hwndFrame; /* out */ ULONG size; } WinGetFrameTreePPSize_Args, *PWinGetFrameTreePPSize_Args;
/* BOOL WinGetFrameTreePPs(HWND, PULONG, ULONG size) */
typedef struct _WinGetFrameTreePPs_Args { HWND hwndFrame; ULONG size; /* out: size bytes */ } WinGetFrameTreePPs_Args, *PWinGetFrameTreePPs_Args;
/* BOOL WinSetFrameTreePPs(HWND, PULONG, ULONG size) */
typedef struct _WinSetFrameTreePPs_Args { HWND hwndFrame; ULONG size; char abData[1]; } WinSetFrameTreePPs_Args, *PWinSetFrameTreePPs_Args;
/* ULONG WinQueryProcessCP(HAB) */
typedef struct _WinQueryProcessCP_Args { ULONG iHab; } WinQueryProcessCP_Args, *PWinQueryProcessCP_Args;
/* BOOL WinCancelShutdown(HAB, BOOL) */
typedef struct _WinCancelShutdown_Args { ULONG iHab; BOOL fCancel; } WinCancelShutdown_Args, *PWinCancelShutdown_Args;
/* BOOL WinSystemShutdown(VOID) */
typedef struct _WinSystemShutdown_Args { ULONG dummy; } WinSystemShutdown_Args, *PWinSystemShutdown_Args;

/* ========================================================================== */
/*  Sync primitives (semaphores)                                              */
/* ========================================================================== */

#define NWinMsgSemWait                0x0170L
#define NWinMsgMuxSemWait             0x0171L
#define NWinSetMsgMode                0x0172L
#define NWinSetSynchroMode            0x0173L
#define NWinSemQuery                  0x0174L
#define NWinSemEnter                  0x0175L
#define NWinSemLeave                  0x0176L
#define NWinSemEnterTimeout           0x0177L
#define NWinWaitEventSem              0x0178L
#define NWinRequestMutexSem           0x0179L
#define NWinWaitMuxWaitSem            0x017AL
#define NWinQueueFromID               0x017BL
#define NWinThreadAssocQueue          0x017CL
#define NWinBuildPtrHandle            0x017DL

/* BOOL WinMsgSemWait(PSEMRECORD) — TBD layout */
typedef struct _WinMsgSemWait_Args { ULONG cbSemRecord; char abSemRecord[1]; } WinMsgSemWait_Args, *PWinMsgSemWait_Args;
/* BOOL WinMsgMuxSemWait(PMUXWAIT) — TBD layout */
typedef struct _WinMsgMuxSemWait_Args { ULONG cbMuxWait; char abMuxWait[1]; } WinMsgMuxSemWait_Args, *PWinMsgMuxSemWait_Args;
/* BOOL WinSetMsgMode(HAB, PCSZ class, LONG mode) */
typedef struct _WinSetMsgMode_Args { ULONG iHab; ULONG cbClass; char szClass[1]; LONG mode; } WinSetMsgMode_Args, *PWinSetMsgMode_Args;
/* BOOL WinSetSynchroMode(HAB, PCSZ class, LONG mode) */
typedef struct _WinSetSynchroMode_Args { ULONG iHab; ULONG cbClass; char szClass[1]; LONG mode; } WinSetSynchroMode_Args, *PWinSetSynchroMode_Args;
/* BOOL WinSemQuery(HSEM, PULONG count, PULONG flags) */
typedef struct _WinSemQuery_Args { ULONG hsem; /* out */ ULONG count; ULONG flags; } WinSemQuery_Args, *PWinSemQuery_Args;
/* BOOL WinSemEnter(HSEM) */
typedef struct _WinSemEnter_Args { ULONG hsem; } WinSemEnter_Args, *PWinSemEnter_Args;
/* BOOL WinSemLeave(HSEM) */
typedef struct _WinSemLeave_Args { ULONG hsem; } WinSemLeave_Args, *PWinSemLeave_Args;
/* BOOL WinSemEnterTimeout(HSEM, ULONG timeout) */
typedef struct _WinSemEnterTimeout_Args { ULONG hsem; ULONG ulTimeout; } WinSemEnterTimeout_Args, *PWinSemEnterTimeout_Args;
/* BOOL WinWaitEventSem(HEV, ULONG timeout) */
typedef struct _WinWaitEventSem_Args { ULONG hev; ULONG ulTimeout; } WinWaitEventSem_Args, *PWinWaitEventSem_Args;
/* BOOL WinRequestMutexSem(HMTX, ULONG timeout) */
typedef struct _WinRequestMutexSem_Args { HMTX hmtx; ULONG ulTimeout; } WinRequestMutexSem_Args, *PWinRequestMutexSem_Args;
/* BOOL WinWaitMuxWaitSem(HMUX, PULONG user, ULONG timeout) */
typedef struct _WinWaitMuxWaitSem_Args { ULONG hmux; ULONG ulTimeout; /* out */ ULONG ulUser; } WinWaitMuxWaitSem_Args, *PWinWaitMuxWaitSem_Args;
/* HMQ WinQueueFromID(HAB, PID, TID) */
typedef struct _WinQueueFromID_Args { ULONG iHab; ULONG pid; ULONG tid; } WinQueueFromID_Args, *PWinQueueFromID_Args;
/* BOOL WinThreadAssocQueue(HAB, TID) */
typedef struct _WinThreadAssocQueue_Args { ULONG iHab; ULONG tid; } WinThreadAssocQueue_Args, *PWinThreadAssocQueue_Args;
/* PVOID WinBuildPtrHandle(PVOID) */
typedef struct _WinBuildPtrHandle_Args { ULONG p; } WinBuildPtrHandle_Args, *PWinBuildPtrHandle_Args;

/* ========================================================================== */
/*  Seamless / Msp / Pen / Dax                                                */
/* ========================================================================== */

#define NWinVisRegionNotify           0x0180L
#define NWinSeamlessBox               0x0181L
#define NWinJiggleMouse               0x0182L
#define NWinTrackSeamless             0x0183L
#define NWinRegisterSeamlessProc      0x0184L
#define NWinMspInit                   0x0185L
#define NWinMspAlloc                  0x0186L
#define NWinMspFree                   0x0187L
#define NWinExplodeWindow             0x0188L
#define NWinTrackWindow               0x0189L
#define NWinRegisterPen               0x018AL
#define NWinSetDaxData                0x018BL
#define NWinQueryDaxData              0x018CL

/* BOOL WinVisRegionNotify(HWND, HRGN) — TBD */
typedef struct _WinVisRegionNotify_Args { HWND hwnd; ULONG hrgn; } WinVisRegionNotify_Args, *PWinVisRegionNotify_Args;
/* HWND WinSeamlessBox(...) — TBD */
typedef struct _WinSeamlessBox_Args { ULONG op; LONG x, y, cx, cy; ULONG hwndParent; } WinSeamlessBox_Args, *PWinSeamlessBox_Args;
/* BOOL WinJiggleMouse(HWND, LONG, LONG) — TBD */
typedef struct _WinJiggleMouse_Args { HWND hwnd; LONG x, y; } WinJiggleMouse_Args, *PWinJiggleMouse_Args;
/* BOOL WinTrackSeamless(...) — TBD */
typedef struct _WinTrackSeamless_Args { HWND hwnd; ULONG cbTrackInfo; char abTrackInfo[1]; } WinTrackSeamless_Args, *PWinTrackSeamless_Args;
/* BOOL WinRegisterSeamlessProc(...) — TBD */
typedef struct _WinRegisterSeamlessProc_Args { ULONG pfn; ULONG fl; } WinRegisterSeamlessProc_Args, *PWinRegisterSeamlessProc_Args;
/* BOOL WinMspInit(HAB) */
typedef struct _WinMspInit_Args { ULONG iHab; } WinMspInit_Args, *PWinMspInit_Args;
/* PVOID WinMspAlloc(HAB, ULONG cb) */
typedef struct _WinMspAlloc_Args { ULONG iHab; ULONG cb; } WinMspAlloc_Args, *PWinMspAlloc_Args;
/* BOOL WinMspFree(HAB, PVOID) */
typedef struct _WinMspFree_Args { ULONG iHab; ULONG p; } WinMspFree_Args, *PWinMspFree_Args;
/* BOOL WinExplodeWindow(HWND) */
typedef struct _WinExplodeWindow_Args { HWND hwnd; } WinExplodeWindow_Args, *PWinExplodeWindow_Args;
/* BOOL WinTrackWindow(HWND, PTRACKINFO) — TBD */
typedef struct _WinTrackWindow_Args { HWND hwnd; ULONG cbTrackInfo; char abTrackInfo[1]; } WinTrackWindow_Args, *PWinTrackWindow_Args;
/* BOOL WinRegisterPen(...) — TBD */
typedef struct _WinRegisterPen_Args { ULONG dummy; } WinRegisterPen_Args, *PWinRegisterPen_Args;
/* BOOL WinSetDaxData(HWND, PCSZ, PCSZ) — TBD */
typedef struct _WinSetDaxData_Args { HWND hwnd; ULONG cbKey; char szKey[1]; ULONG cbData; char szData[1]; } WinSetDaxData_Args, *PWinSetDaxData_Args;
/* BOOL WinQueryDaxData(HWND, PCSZ, PCSZ, ULONG cbBuf, PVOID) — TBD */
typedef struct _WinQueryDaxData_Args { HWND hwnd; ULONG cbKey; char szKey[1]; ULONG cbValueName; char szValueName[1]; ULONG cbBuf; /* out: cbBuf bytes */ } WinQueryDaxData_Args, *PWinQueryDaxData_Args;

/* ========================================================================== */
/*  Win32-specific                                                            */
/* ========================================================================== */

#define NWin32DebugBreak              0x0190L
#define NWin32AddClipbrdViewer        0x0191L
#define NWin32RemoveClipbrdViewer     0x0192L
#define NWin32QueryClipbrdViewerChain 0x0193L
#define NWin32IncrementAtomUsage      0x0194L
#define NWin32SetKbdLayout            0x0195L
#define NWin32GetAFCValue             0x0196L
#define NWin32SetAFCValue             0x0197L
#define NWin32TranslateChar2          0x0198L
#define NWin32WaitForIdleThread       0x0199L
#define NWin32QueryActiveOtherWindow  0x019AL
#define NWin32EnumVkeyScanIDs         0x019BL

/* void Win32DebugBreak(VOID) */
typedef struct _Win32DebugBreak_Args { ULONG dummy; } Win32DebugBreak_Args, *PWin32DebugBreak_Args;
/* BOOL Win32AddClipbrdViewer(HWND) */
typedef struct _Win32AddClipbrdViewer_Args { HWND hwnd; } Win32AddClipbrdViewer_Args, *PWin32AddClipbrdViewer_Args;
/* BOOL Win32RemoveClipbrdViewer(HWND) */
typedef struct _Win32RemoveClipbrdViewer_Args { HWND hwnd; } Win32RemoveClipbrdViewer_Args, *PWin32RemoveClipbrdViewer_Args;
/* ULONG Win32QueryClipbrdViewerChain(PHWND, ULONG cMax) */
typedef struct _Win32QueryClipbrdViewerChain_Args { ULONG cMax; /* out: cMax HWNDs, pcbPayload */ } Win32QueryClipbrdViewerChain_Args, *PWin32QueryClipbrdViewerChain_Args;
/* ATOM Win32IncrementAtomUsage(HATOMTBL, ATOM, ULONG) */
typedef struct _Win32IncrementAtomUsage_Args { ULONG hatomtbl; ULONG atom; ULONG inc; } Win32IncrementAtomUsage_Args, *PWin32IncrementAtomUsage_Args;
/* BOOL Win32SetKbdLayout(HAB, ULONG) */
typedef struct _Win32SetKbdLayout_Args { ULONG iHab; ULONG layout; } Win32SetKbdLayout_Args, *PWin32SetKbdLayout_Args;
/* BOOL Win32GetAFCValue(HAB, ULONG) */
typedef struct _Win32GetAFCValue_Args { ULONG iHab; ULONG flag; } Win32GetAFCValue_Args, *PWin32GetAFCValue_Args;
/* BOOL Win32SetAFCValue(HAB, ULONG) */
typedef struct _Win32SetAFCValue_Args { ULONG iHab; ULONG value; } Win32SetAFCValue_Args, *PWin32SetAFCValue_Args;
/* LONG Win32TranslateChar2(HAB, ULONG cp, ULONG ch, PCH pchOut) — TBD */
typedef struct _Win32TranslateChar2_Args { ULONG iHab; ULONG cp; ULONG ch; /* out */ CHAR chOut; } Win32TranslateChar2_Args, *PWin32TranslateChar2_Args;
/* BOOL Win32WaitForIdleThread(VOID) */
typedef struct _Win32WaitForIdleThread_Args { ULONG dummy; } Win32WaitForIdleThread_Args, *PWin32WaitForIdleThread_Args;
/* HWND Win32QueryActiveOtherWindow(HWND) */
typedef struct _Win32QueryActiveOtherWindow_Args { HWND hwndDesktop; } Win32QueryActiveOtherWindow_Args, *PWin32QueryActiveOtherWindow_Args;
/* ULONG Win32EnumVkeyScanIDs(...) — TBD */
typedef struct _Win32EnumVkeyScanIDs_Args { ULONG cMax; /* out: array, pcbPayload */ } Win32EnumVkeyScanIDs_Args, *PWin32EnumVkeyScanIDs_Args;

/* ========================================================================== */
/*  Misc                                                                      */
/* ========================================================================== */

#define NWinXXCreateHeap2             0x01A0L
#define NWinQuerySharedHeapData       0x01A1L
#define NWinSetCharXlateTbl           0x01A2L
#define NWinLoadVkeyGlyphXlatetbl     0x01A3L
#define NWinSetVKeyGlyphXlatetbl      0x01A4L
#define NWinQueryVKeyGlyphXlatetbl    0x01A5L
#define NWinWaitMsgTimeout            0x01A6L

typedef struct _WinXXCreateHeap2_Args { ULONG cbHeap; ULONG flOptions; ULONG flag; } WinXXCreateHeap2_Args, *PWinXXCreateHeap2_Args;
typedef struct _WinQuerySharedHeapData_Args { ULONG hheap; /* out */ ULONG cbHeap; ULONG cbUsed; } WinQuerySharedHeapData_Args, *PWinQuerySharedHeapData_Args;
typedef struct _WinSetCharXlateTbl_Args { ULONG pTable; } WinSetCharXlateTbl_Args, *PWinSetCharXlateTbl_Args;
typedef struct _WinLoadVkeyGlyphXlatetbl_Args { ULONG iHab; ULONG cb; char ab[1]; } WinLoadVkeyGlyphXlatetbl_Args, *PWinLoadVkeyGlyphXlatetbl_Args;
typedef struct _WinSetVKeyGlyphXlatetbl_Args { ULONG pTable; } WinSetVKeyGlyphXlatetbl_Args, *PWinSetVKeyGlyphXlatetbl_Args;
typedef struct _WinQueryVKeyGlyphXlatetbl_Args { ULONG dummy; } WinQueryVKeyGlyphXlatetbl_Args, *PWinQueryVKeyGlyphXlatetbl_Args;
typedef struct _WinWaitMsgTimeout_Args { ULONG iHab; HWND hwndFilter; ULONG msgFirst; ULONG msgLast; ULONG timeout; QMSG qmsg; /* out: */ } WinWaitMsgTimeout_Args, *PWinWaitMsgTimeout_Args;

/* ========================================================================== */
/*  Dax* — Direct Access 32-bit PMWIN exports (not Win*-prefixed)             */
/*  Domain: 0x01B0-0x01BF                                                     */
/* ========================================================================== */

#define NDaxCreateCaret               0x01B0L
#define NDaxDestroyCaret              0x01B1L
#define NDaxGetCaretBlinkTime         0x01B2L
#define NDaxHideCaret                 0x01B3L
#define NDaxSetCaretBlinkTime         0x01B4L
#define NDaxShowCaret                 0x01B5L
#define NDaxShowCursor                0x01B6L
#define NDaxSwapMouseButton           0x01B7L
#define NDaxOpenClipbrd               0x01B8L   /* TBD: maybe internal */

/*!
 * @struct DaxCreateCaret_Args
 * @brief BOOL DaxCreateCaret(HWND hwnd, HBITMAP hbm, LONG cx, LONG cy)
 *        cx/cy = 0 -> default caret size; hbm = NULLHANDLE -> solid caret.
 */
typedef struct _DaxCreateCaret_Args {
    HWND hwnd; HBITMAP hbm; LONG cx; LONG cy;
} DaxCreateCaret_Args, *PDaxCreateCaret_Args;

/* BOOL DaxDestroyCaret(HWND hwnd) */
typedef struct _DaxDestroyCaret_Args {
    HWND hwnd;
} DaxDestroyCaret_Args, *PDaxDestroyCaret_Args;

/* LONG DaxGetCaretBlinkTime(HWND hwnd) — returns blink interval in msec */
typedef struct _DaxGetCaretBlinkTime_Args {
    HWND hwnd;
} DaxGetCaretBlinkTime_Args, *PDaxGetCaretBlinkTime_Args;

/* BOOL DaxHideCaret(HWND hwnd) */
typedef struct _DaxHideCaret_Args {
    HWND hwnd;
} DaxHideCaret_Args, *PDaxHideCaret_Args;

/* BOOL DaxSetCaretBlinkTime(HWND hwnd, LONG msec) — msec <= 0: restore default */
typedef struct _DaxSetCaretBlinkTime_Args {
    HWND hwnd; LONG msec;
} DaxSetCaretBlinkTime_Args, *PDaxSetCaretBlinkTime_Args;

/* BOOL DaxShowCaret(HWND hwnd) */
typedef struct _DaxShowCaret_Args {
    HWND hwnd;
} DaxShowCaret_Args, *PDaxShowCaret_Args;

/* BOOL DaxShowCursor(HWND hwnd, BOOL fShow) — desktop only */
typedef struct _DaxShowCursor_Args {
    HWND hwnd; BOOL fShow;
} DaxShowCursor_Args, *PDaxShowCursor_Args;

/* BOOL DaxSwapMouseButton(HWND hwnd, BOOL fSwap) — desktop only */
typedef struct _DaxSwapMouseButton_Args {
    HWND hwnd; BOOL fSwap;
} DaxSwapMouseButton_Args, *PDaxSwapMouseButton_Args;

/*!
 * @struct DaxOpenClipbrd_Args
 * @brief TBD: signature uncertain — HWND-based, unlike WinOpenClipbrd(HAB).
 *        Possibly an internal direct-access entry point used by WinOpenClipbrd
 *        itself; may not need a wire code at all once verified.
 */
typedef struct _DaxOpenClipbrd_Args {
    HWND hwnd;
} DaxOpenClipbrd_Args, *PDaxOpenClipbrd_Args;

/* ========================================================================== */
/*  LookupAtom — atom helper exported from PMWIN.DLL                          */
/* ========================================================================== */
/* Ordinal .10020. Not Win/Dax-prefixed, not RESIDENT. Likely an internal
 * helper for the atom table, possibly pure client-side (no server round-trip
 * needed). Code assigned in the Dax range to keep it out of the Win* block.
 * Verify before implementing; may not need to exist in the wire protocol. */

#define NLookupAtom                   0x01BFL

typedef struct _LookupAtom_Args {
    ULONG hatomtbl;
    ULONG cbName;
    char  szName[1];
} LookupAtom_Args, *PLookupAtom_Args;

#endif /* WIN_WIRE_H */
