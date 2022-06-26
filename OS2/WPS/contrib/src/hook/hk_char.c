
/*
 *@@sourcefile hk_misc.c:
 *
 *@@added V0.9.12 (2001-05-27) [umoeller]
 *@@header "hook\hook_private.h"
 */

/*
 *      Copyright (C) 1999-2003 Ulrich M”ller.
 *      Copyright (C) 1993-1999 Roman Stangl.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINSCROLLBARS
#define INCL_WINSYS
#define INCL_WINTIMER
#define INCL_WINHOOKS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#define INCL_DOSMODULEMGR
#include <os2.h>

#include <stdio.h>

// PMPRINTF in hooks is a tricky issue;
// avoid this unless this is really needed.
// If enabled, NEVER give the PMPRINTF window
// the focus, or your system will hang solidly...
#define DONTDEBUGATALL
#define DONT_REPLACE_FOR_DBCS
#define DONT_REPLACE_MALLOC         // in case mem debug is enabled
#include "setup.h"

#include "hook\xwphook.h"
#include "hook\hook_private.h"          // private hook and daemon definitions

#pragma hdrstop

/******************************************************************
 *
 *  Input hook -- character
 *
 ******************************************************************/

/*
 *@@ WMChar_FunctionKeys:
 *      returns TRUE if the specified key is one of
 *      the user-defined XWP function keys.
 *
 *@@added V0.9.3 (2000-04-20) [umoeller]
 */

BOOL WMChar_FunctionKeys(USHORT usFlags,  // in: SHORT1FROMMP(mp1) from WM_CHAR
                         UCHAR ucScanCode) // in: CHAR4FROMMP(mp1) from WM_CHAR
{
    // set return value:
    // per default, pass message on to next hook or application
    BOOL brc = FALSE;

    // scan code valid?
    if (usFlags & KC_SCANCODE)
    {
        // request access to shared memory
        // with function key definitions:
        PFUNCTIONKEY paFunctionKeysShared = NULL;

        if (!DosGetNamedSharedMem((PVOID*)(&paFunctionKeysShared),
                                           SHMEM_FUNCTIONKEYS,
                                           PAG_READ | PAG_WRITE))
        {
            // search function keys array
            ULONG   ul = 0;
            PFUNCTIONKEY pKeyThis = paFunctionKeysShared;
            for (ul = 0;
                 ul < G_cFunctionKeys;
                 ul++)
            {
                if (pKeyThis->ucScanCode == ucScanCode)
                {
                    // scan codes match:
                    // return "found" flag
                    brc = TRUE;
                    break;  // for
                }

                pKeyThis++;
            }

            DosFreeMem(paFunctionKeysShared);
        } // end if DosGetNamedSharedMem

    } // end if (usFlags & KC_SCANCODE)

    return brc;
}

/*
 *@@ WMChar_Hotkeys:
 *      this gets called from hookPreAccelHook to
 *      process WM_CHAR messages.
 *
 *      As opposed to folder hotkeys (folder.c),
 *      for the global object hotkeys, we need a more
 *      sophisticated processing, because we cannot rely
 *      on the usch field, which is different between
 *      PM and VIO sessions (apparently the translation
 *      is taking place differently for VIO sessions).
 *
 *      As a result, we can only use the scan code to
 *      identify hotkeys. See GLOBALHOTKEY for details.
 *
 *      Since we use a dynamically allocated array of
 *      GLOBALHOTKEY structures, to make this thread-safe,
 *      we have to use shared memory and a global mutex
 *      semaphore. See hookSetGlobalHotkeys. While this
 *      function is being called, the global mutex is
 *      requested, so this is protected.
 *
 *      If this returns TRUE, the msg is swallowed. We
 *      return TRUE if we found a hotkey.
 *
 *      This function gets called for BOTH the key-down and
 *      the key-up message. As usual with PM, applications
 *      should react to key-down messages only. However, since
 *      we swallow the key-down message if a hotkey was found
 *      (and post XDM_HOTKEYPRESSED to the daemon object window),
 *      we should swallow the key-up msg also, because otherwise
 *      the application gets a key-up msg without a previous
 *      key-down event, which might lead to confusion.
 *
 *@@changed V0.9.3 (2000-04-10) [umoeller]: moved debug code to hook
 *@@changed V0.9.3 (2000-04-10) [umoeller]: removed usch and usvk params
 *@@changed V0.9.3 (2000-04-19) [umoeller]: added XWP function keys support
 *@@changed V0.9.9 (2001-03-05) [umoeller]: removed break for multiple hotkey assignments
 */

BOOL WMChar_Hotkeys(USHORT usFlagsOrig,  // in: SHORT1FROMMP(mp1) from WM_CHAR
                    UCHAR ucScanCode) // in: CHAR4FROMMP(mp1) from WM_CHAR
{
    // set return value:
    // per default, pass message on to next hook or application
    BOOL brc = FALSE;

    // request access to shared memory
    // with hotkey definitions:
    PGLOBALHOTKEY pGlobalHotkeysShared = NULL;
    if (!DosGetNamedSharedMem((PVOID*)(&pGlobalHotkeysShared),
                                       SHMEM_HOTKEYS,
                                       PAG_READ | PAG_WRITE))
    {
        // OK, we got the shared hotkeys:
        USHORT  us;
        PGLOBALHOTKEY pKeyThis = pGlobalHotkeysShared;

        // filter out unwanted flags;
        // we used to check for KC_VIRTUALKEY also, but this doesn't
        // work with VIO sessions, where this is never set V0.9.3 (2000-04-10) [umoeller]
        USHORT usFlags = usFlagsOrig & (KC_CTRL | KC_ALT | KC_SHIFT);

        // now go through the shared hotkey list and check
        // if the pressed key was assigned an action to
        us = 0;
        for (us = 0;
             us < G_cGlobalHotkeys;
             us++)
        {
            // when comparing,
            // filter out KC_VIRTUALKEY, because this is never set
            // in VIO sessions... V0.9.3 (2000-04-10) [umoeller]
            if (   ((pKeyThis->usFlags & (KC_CTRL | KC_ALT | KC_SHIFT))
                            == usFlags)
                && (pKeyThis->ucScanCode == ucScanCode)
               )
            {
                // hotkey found:

                // only for the key-down event,
                // notify daemon
                if ((usFlagsOrig & KC_KEYUP) == 0)
                    WinPostMsg(G_HookData.hwndDaemonObject,
                               XDM_HOTKEYPRESSED,
                               (MPARAM)(pKeyThis->ulHandle),
                               (MPARAM)0);

                // reset return code: swallow this message
                // (both key-down and key-up)
                brc = TRUE;
                // get outta here
                // break; // for
                        // removed this V0.9.9 (2001-03-05) [umoeller]
                        // to allow for multiple hotkey assignments...
                        // whoever wants to use this!
            }

            // not found: go for next key to check
            pKeyThis++;
        } // end for

        DosFreeMem(pGlobalHotkeysShared);
    } // end if DosGetNamedSharedMem

    return brc;
}

/*
 *@@ WMChar_Main:
 *      WM_CHAR processing in hookPreAccelHook.
 *      This has been extracted with V0.9.3 (2000-04-20) [umoeller].
 *
 *      This requests the global hotkeys semaphore to make the
 *      whole thing thread-safe.
 *
 *      Then, we call WMChar_FunctionKeys to see if the key is
 *      a function key. If so, the message is swallowed in any
 *      case.
 *
 *      Then, we call WMChar_Hotkeys to check if some global hotkey
 *      has been defined for the key (be it a function key or some
 *      other key combo). If so, the message is swallowed.
 *
 *      Third, we check for the XPager switch-desktop hotkeys.
 *      If one of those is found, we swallow the msg also and
 *      notify XPager.
 *
 *      That is, the msg is swallowed if it's a function key or
 *      a global hotkey or a XPager hotkey.
 *
 *@@added V0.9.3 (2000-04-20) [umoeller]
 *@@changed V0.9.16 (2001-12-08) [umoeller]: added G_HookData.fHotkeysDisabledTemp check
 */

BOOL WMChar_Main(PQMSG pqmsg)       // in/out: from hookPreAccelHook
{
    // set return value:
    // per default, pass message on to next hook or application
    BOOL brc = FALSE;

    USHORT usFlags    = SHORT1FROMMP(pqmsg->mp1);
    // UCHAR  ucRepeat   = CHAR3FROMMP(mp1);
    UCHAR  ucScanCode = CHAR4FROMMP(pqmsg->mp1);
    USHORT usch       = SHORT1FROMMP(pqmsg->mp2);
    // USHORT usvk       = SHORT2FROMMP(pqmsg->mp2);

    APIRET arc;

    // do nothing if hotkeys are temporarily disabled
    // V0.9.16 (2001-12-06) [umoeller]
    if (!G_HookData.fHotkeysDisabledTemp)
    {
        // request access to the hotkeys mutex:
        // first we need to open it, because this
        // code can be running in any PM thread in
        // any process
        if (!(arc = DosOpenMutexSem(NULL,       // unnamed
                                    &G_hmtxGlobalHotkeys)))
        {
            // OK, semaphore opened: request access
            if (!(arc = DosRequestMutexSem(G_hmtxGlobalHotkeys,
                                           TIMEOUT_HMTX_HOTKEYS)))
            {
                // OK, we got the mutex:
                // search the list of function keys
                BOOL    fIsFunctionKey = WMChar_FunctionKeys(usFlags, ucScanCode);
                            // returns TRUE if ucScanCode represents one of the
                            // function keys

                // global hotkeys enabled? This also gets called if XPager hotkeys are on!
#ifndef __ALWAYSOBJHOTKEYS__
                if (G_HookData.HookConfig.__fGlobalHotkeys)
#endif
                {
                    if (    // process only key-down messages
                            // ((usFlags & KC_KEYUP) == 0)
                            // check flags:
                            // do the list search only if the key could be
                            // a valid hotkey, that is:
                            (
                                // 1) it's a function key
                                (fIsFunctionKey)

                                // or 2) it's a typical virtual key or Ctrl/alt/etc combination
                            ||  (     ((usFlags & KC_VIRTUALKEY) != 0)
                                      // Ctrl pressed?
                                   || ((usFlags & KC_CTRL) != 0)
                                      // Alt pressed?
                                   || ((usFlags & KC_ALT) != 0)
                                      // or one of the Win95 keys?
                                   || (   ((usFlags & KC_VIRTUALKEY) == 0)
                                       && (     (usch == 0xEC00)
                                            ||  (usch == 0xED00)
                                            ||  (usch == 0xEE00)
                                          )
                                      )
                                )
                            )
                            // always filter out those ugly composite key (accents etc.),
                            // but make sure the scan code is valid V0.9.3 (2000-04-10) [umoeller]
                       &&   ((usFlags & (KC_DEADKEY
                                         | KC_COMPOSITE
                                         | KC_INVALIDCOMP
                                         | KC_SCANCODE))
                              == KC_SCANCODE)
                       )
                    {
                          /*
                     In PM session:
                                                  usFlags         usvk usch       ucsk
                          Ctrl alone              VK SC CTRL       0a     0        1d
                          Ctrl-A                     SC CTRL        0    61        1e
                          Ctrl-Alt                VK SC CTRL ALT   0b     0        38
                          Ctrl-Alt-A                 SC CTRL ALT    0    61        1e

                          F11 alone               VK SC toggle     2a  8500        57
                          Ctrl alone              VK SC CTRL       0a     0        1d
                          Ctrl-Alt                VK SC CTRL ALT   0b     0        38
                          Ctrl-Alt-F11            VK SC CTRL ALT   2a  8b00        57

                     In VIO session:
                          Ctrl alone                 SC CTRL       07   0          1d
                          Ctrl-A                     SC CTRL        0   1e01       1e
                          Ctrl-Alt                   SC CTRL ALT   07   0          38
                          Ctrl-Alt-A                 SC CTRL ALT   20   1e00       1e

                          Alt-A                      SC      ALT   20   1e00
                          Ctrl-E                     SC CTRL        0   3002

                          F11 alone               ignored...
                          Ctrl alone              VK SC CTRL       07!    0        1d
                          Ctrl-Alt                VK SC CTRL ALT   07!    0        38
                          Ctrl-Alt-F11            !! SC CTRL ALT   20! 8b00        57

                          So apparently, for these keyboard combinations, in VIO
                          sessions, the KC_VIRTUALKEY flag is missing. Strange.

                          */

                        #ifdef _PMPRINTF_
                                CHAR    szFlags[2000] = "";
                                if (usFlags & KC_CHAR)                      // 0x0001
                                    strcat(szFlags, "KC_CHAR ");
                                if (usFlags & KC_VIRTUALKEY)                // 0x0002
                                    strcat(szFlags, "KC_VIRTUALKEY ");
                                if (usFlags & KC_SCANCODE)                  // 0x0004
                                    strcat(szFlags, "KC_SCANCODE ");
                                if (usFlags & KC_SHIFT)                     // 0x0008
                                    strcat(szFlags, "KC_SHIFT ");
                                if (usFlags & KC_CTRL)                      // 0x0010
                                    strcat(szFlags, "KC_CTRL ");
                                if (usFlags & KC_ALT)                       // 0x0020
                                    strcat(szFlags, "KC_ALT ");
                                if (usFlags & KC_KEYUP)                     // 0x0040
                                    strcat(szFlags, "KC_KEYUP ");
                                if (usFlags & KC_PREVDOWN)                  // 0x0080
                                    strcat(szFlags, "KC_PREVDOWN ");
                                if (usFlags & KC_LONEKEY)                   // 0x0100
                                    strcat(szFlags, "KC_LONEKEY ");
                                if (usFlags & KC_DEADKEY)                   // 0x0200
                                    strcat(szFlags, "KC_DEADKEY ");
                                if (usFlags & KC_COMPOSITE)                 // 0x0400
                                    strcat(szFlags, "KC_COMPOSITE ");
                                if (usFlags & KC_INVALIDCOMP)               // 0x0800
                                    strcat(szFlags, "KC_INVALIDCOMP ");
                                if (usFlags & KC_TOGGLE)                    // 0x1000
                                    strcat(szFlags, "KC_TOGGLE ");
                                if (usFlags & KC_INVALIDCHAR)               // 0x2000
                                    strcat(szFlags, "KC_INVALIDCHAR ");
                                if (usFlags & KC_DBCSRSRVD1)                // 0x4000
                                    strcat(szFlags, "KC_DBCSRSRVD1 ");
                                if (usFlags & KC_DBCSRSRVD2)                // 0x8000
                                    strcat(szFlags, "KC_DBCSRSRVD2 ");

                                _Pmpf(("  usFlags: 0x%lX -->", usFlags));
                                _Pmpf(("    %s", szFlags));
                                _Pmpf(("  usvk: 0x%lX", usvk));
                                _Pmpf(("  usch: 0x%lX", usch));
                                _Pmpf(("  ucScanCode: 0x%lX", ucScanCode));
                        #endif

    /* #ifdef __DEBUG__
                        // debug code:
                        // enable Ctrl+Alt+Delete emergency exit
                        if (    (   (usFlags & (KC_CTRL | KC_ALT | KC_KEYUP))
                                     == (KC_CTRL | KC_ALT)
                                )
                              && (ucScanCode == 0x0e)    // delete
                           )
                        {
                            ULONG ul;
                            for (ul = 5000;
                                 ul > 100;
                                 ul -= 200)
                                DosBeep(ul, 20);

                            WinPostMsg(G_HookData.hwndDaemonObject,
                                       WM_QUIT,
                                       0, 0);
                            brc = TRUE;     // swallow
                        }
                        else
    #endif */

                        if (WMChar_Hotkeys(usFlags, ucScanCode))
                            // returns TRUE (== swallow) if hotkey was found
                            brc = TRUE;
                    }
                }

                DosReleaseMutexSem(G_hmtxGlobalHotkeys);

            } // end if !DosRequestMutexSem

            DosCloseMutexSem(G_hmtxGlobalHotkeys);
        } // end if DosOpenMutexSem
    } // end if (!G_HookData.fHotkeysDisabledTemp)

#ifndef __NOPAGER__
    // XPager hotkeys:
    // moved all the following out of the mutex block above
    if (!brc)       // message not swallowed yet:
    {
        if (    (G_HookData.PagerConfig.flPager & PGRFL_HOTKEYS)
                    // arrow hotkeys enabled?
             && (G_HookData.hwndPagerClient)
                    // XPager active?
           )
        {
            // XPager hotkeys enabled:
            // key-up only
            if ((usFlags & KC_KEYUP) == 0)
            {
                // check KC_CTRL etc. flags
                if (   (G_HookData.PagerConfig.flKeyShift | KC_SCANCODE)
                            == (usFlags & (G_HookData.PagerConfig.flKeyShift
                                           | KC_SCANCODE))
                   )
                    // OK: check scan codes
                    if (    (ucScanCode == 0x61)
                         || (ucScanCode == 0x66)
                         || (ucScanCode == 0x63)
                         || (ucScanCode == 0x64)
                       )
                    {
                        // cursor keys:
                        WinPostMsg(G_HookData.hwndPagerClient,
                                   PGRM_PAGERHOTKEY,
                                   (MPARAM)ucScanCode,
                                   0);
                        // swallow
                        brc = TRUE;
                    }

                 if (    !brc
                     &&  ((usFlags & (KC_ALT | KC_CTRL | KC_SHIFT)) == 0)
                     &&  (G_HookData.PagerConfig.flPager & PGRFL_WINDOWS_KEYS)
                     &&  (    (usFlags & KC_VIRTUALKEY) == 0
                         &&  (   (usch == 0xEC00 /* left */)
                              || (usch == 0xED00 /* right */))
                         )
                     )
                     {
                         // cursor keys:
                         WinPostMsg(G_HookData.hwndPagerClient,
                                    PGRM_PAGERHOTKEY,
                                    (MPARAM)(usch == 0xEC00 ? 0x63 : 0x64),
                                    0);
                         // swallow
                         brc = TRUE;
                     }
            }
        }
    } // end if (!brc)       // message not swallowed yet
#endif

    return brc;
}


