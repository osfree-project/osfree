/*!
 * @file dispatch_win_stubs.cpp
 * @brief Empty placeholders for Win* domains not yet implemented.
 *
 * As each domain reaches its phase (4, 5, 6, 7), the corresponding
 * function moves out of this file into dispatch_win_<domain>.cpp and
 * gains its own registry arguments in dispatch_win.hpp.
 */
#include "dispatch_win.hpp"

#define WIN_DOMAIN_STUB(name) \
    ULONG name(ULONG fnCode, ULONG data, \
               PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload) \
    { \
        (void)fnCode; (void)data; (void)pArgs; (void)cbArgs; \
        if (pcbPayload) *pcbPayload = 0; \
        return 0; \
    }

WIN_DOMAIN_STUB(DispatchWinMq)
WIN_DOMAIN_STUB(DispatchWinWnd)
WIN_DOMAIN_STUB(DispatchWinPs)
WIN_DOMAIN_STUB(DispatchWinCls)
WIN_DOMAIN_STUB(DispatchWinDlg)
WIN_DOMAIN_STUB(DispatchWinPtr)
WIN_DOMAIN_STUB(DispatchWinClip)
WIN_DOMAIN_STUB(DispatchWinAtom)
WIN_DOMAIN_STUB(DispatchWinStr)
WIN_DOMAIN_STUB(DispatchWinErr)
WIN_DOMAIN_STUB(DispatchWinSys)
WIN_DOMAIN_STUB(DispatchWinRect)
WIN_DOMAIN_STUB(DispatchWinInp)
WIN_DOMAIN_STUB(DispatchWinDraw)
WIN_DOMAIN_STUB(DispatchWinFrame)
WIN_DOMAIN_STUB(DispatchWinSync)
WIN_DOMAIN_STUB(DispatchWinMisc)
WIN_DOMAIN_STUB(DispatchWin32)
WIN_DOMAIN_STUB(DispatchWinDax)

#undef WIN_DOMAIN_STUB
