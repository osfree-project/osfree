/*!
 * @file dispatch_win.hpp
 * @brief Per-domain dispatchers for the Win* class.
 *
 * Each Win* function carries a "primary handle" in the command's data
 * field (like hps for GPI), and any remaining arguments in pArgs.
 * The primary handle is:
 *   - HAB    for WinInitialize / WinTerminate / WinGetLastError
 *   - HWND   for window functions (Phase 4+)
 *   - HMQ    for queue functions (Phase 4+)
 *   - HPS    for paint/PS functions (Phase 4+)
 *
 * Dispatchers are split by domain so that each can be implemented and
 * unit-tested independently; the top-level HandleWinCall only routes.
 *
 * Shared parameters for every dispatcher:
 *   fnCode     NWin* / NDax* code (already extracted from the wire)
 *   data       primary handle or ULONG argument (function-specific)
 *   pArgs      in/out payload (variable arguments)
 *   cbArgs     size of pArgs
 *   pcbPayload in/out payload size on reply (0 = no reply payload)
 *
 * Return value goes through FPM_RESP_HEADER.ulResult.
 */
#ifndef DISPATCH_WIN_HPP
#define DISPATCH_WIN_HPP

#define INCL_PM
#include "F_platform.h"
#include "Fs_hab.hpp"
#include "win_wire.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 0x0001-0x000F: HAB lifecycle (Phase 2) ---------------------------- */
ULONG DispatchWinHab(Fs_HabList *pHabs, ULONG idClient,
                     ULONG fnCode, ULONG data,
                     PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x0010-0x001F: Message queue (Phase 4) ---------------------------- */
ULONG DispatchWinMq(ULONG fnCode, ULONG data,
                    PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x0020-0x005F: Window lifecycle + queries (Phase 4) --------------- */
ULONG DispatchWinWnd(ULONG fnCode, ULONG data,
                     PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x0060-0x007F: Paint / PS / DC (Phase 4 / K) ---------------------- */
ULONG DispatchWinPs(ULONG fnCode, ULONG data,
                    PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x0080-0x009F: Class / Subclass / Thunk (Phase 4-5) --------------- */
ULONG DispatchWinCls(ULONG fnCode, ULONG data,
                     PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x00A0-0x00BF: Dialogs (Phase 5) ---------------------------------- */
ULONG DispatchWinDlg(ULONG fnCode, ULONG data,
                     PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x00C0-0x00DF: Menus / Timers / Pointer / Cursor (Phase 5) -------- */
ULONG DispatchWinPtr(ULONG fnCode, ULONG data,
                     PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x00E0-0x00EF: Clipboard / Atoms (Phase 5) ------------------------ */
ULONG DispatchWinClip(ULONG fnCode, ULONG data,
                      PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
ULONG DispatchWinAtom(ULONG fnCode, ULONG data,
                      PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x00F0-0x010F: Strings / CP / Accel / Hooks / Errors (Phase 5-6) -- */
ULONG DispatchWinStr(ULONG fnCode, ULONG data,
                     PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
ULONG DispatchWinErr(ULONG fnCode, ULONG data,
                     PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x0110-0x011F: Sys / Desktop (Phase 5-6) -------------------------- */
ULONG DispatchWinSys(ULONG fnCode, ULONG data,
                     PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x0120-0x013F: Rect helpers (Phase 5-6) --------------------------- */
ULONG DispatchWinRect(ULONG fnCode, ULONG data,
                      PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x0140-0x015F: Input / Draw helpers (Phase 6) --------------------- */
ULONG DispatchWinInp(ULONG fnCode, ULONG data,
                     PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
ULONG DispatchWinDraw(ULONG fnCode, ULONG data,
                      PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x0160-0x017F: Frame / sync primitives (Phase 6) ------------------ */
ULONG DispatchWinFrame(ULONG fnCode, ULONG data,
                       PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
ULONG DispatchWinSync(ULONG fnCode, ULONG data,
                      PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x0180-0x01AF: Seamless / Msp / Pen / Win32 / Misc (Phase 7) ------ */
ULONG DispatchWinMisc(ULONG fnCode, ULONG data,
                      PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);
ULONG DispatchWin32(ULONG fnCode, ULONG data,
                    PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

/* ---- 0x01B0-0x01BF: Dax* (Phase 4-5) ----------------------------------- */
ULONG DispatchWinDax(ULONG fnCode, ULONG data,
                     PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload);

#ifdef __cplusplus
}
#endif
#endif /* DISPATCH_WIN_HPP */
