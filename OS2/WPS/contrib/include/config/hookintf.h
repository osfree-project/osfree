
/*
 *@@sourcefile hookintf.h:
 *      header file for hookintf.c (daemon/hook interface).
 *
 *@@include #include <os2.h>
 *@@include #include "shared\notebook.h"
 *@@include #include "hook\xwphook.h"
 *@@include #include "setup\hookintf.h"
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef HOOKINTF_HEADER_INCLUDED
    #define HOOKINTF_HEADER_INCLUDED

    /********************************************************************
     *
     *   Declarations
     *
     ********************************************************************/

    #ifdef XWPHOOK_HEADER_INCLUDED
        /*
         *@@ HOTKEYRECORD:
         *      extended record core used for
         *      hotkey definitions in "Hotkeys"
         *      settings page of XWPKeyboard.
         */

        typedef struct _HOTKEYRECORD
        {
            RECORDCORE  recc;

            ULONG       ulIndex;

            // object handle
            PSZ         pszHandle;          // points to szHandle
            CHAR        szHandle[10];       // string representation of GLOBALHOTKEY handle member

            // folder of object
            PSZ         pszFolderPath;      // points to szFolderPath
            CHAR        szFolderPath[CCHMAXPATH];

            // hotkey description
            PSZ         pszHotkey;          // points to szHotkey
            CHAR        szHotkey[200];

            // hotkey data; this contains the object handle
            GLOBALHOTKEY Hotkey;

            // object ptr
            WPObject    *pObject;
        } HOTKEYRECORD, *PHOTKEYRECORD;

        /*
         *@@ FUNCTIONKEYRECORD:
         *      extended record core used for
         *      function key definitions in
         *      "Function keys" settings page
         *      of XWPKeyboard.
         *
         *@@added V0.9.3 (2000-04-18) [umoeller]
         */

        typedef struct _FUNCTIONKEYRECORD
        {
            RECORDCORE  recc;

            ULONG       ulIndex;

            FUNCTIONKEY FuncKey;            // function key definition (xwphook.h)

            PSZ         pszDescription;     // points to FUNCTIONKEY.szDescription

            PSZ         pszScanCode;        // points to szScanCode
            CHAR        szScanCode[40];

            PSZ         pszModifier;        // points to static "x" string if key is a modifier string

        } FUNCTIONKEYRECORD, *PFUNCTIONKEYRECORD;

    #endif

    /********************************************************************
     *
     *   XWorkplace daemon/hook interface
     *
     ********************************************************************/

    BOOL hifEnableHook(BOOL fEnable);

    BOOL hifXWPHookReady(VOID);

#ifndef __NOPAGER__
    BOOL hifEnableXPager(BOOL fEnable);
#endif

    BOOL hifHookConfigChanged(PVOID pvdc);

    /* ******************************************************************
     *
     *   Object hotkeys interface
     *
     ********************************************************************/

#ifndef __ALWAYSOBJHOTKEYS__
    BOOL hifObjectHotkeysEnabled(VOID);

    VOID hifEnableObjectHotkeys(BOOL fEnable);
#endif

    PVOID hifQueryObjectHotkeys(PULONG pcHotkeys);

    VOID hifFreeObjectHotkeys(PVOID pvHotkeys);

    BOOL hifSetObjectHotkeys(PVOID pvHotkeys,
                             ULONG cHotkeys);

    /* ******************************************************************
     *
     *   Function keys interface
     *
     ********************************************************************/

    #ifdef XWPHOOK_HEADER_INCLUDED
        PFUNCTIONKEY hifQueryFunctionKeys(PULONG pcFunctionKeys);

        BOOL hifFreeFunctionKeys(PFUNCTIONKEY paFunctionKeys);

        BOOL hifSetFunctionKeys(PFUNCTIONKEY paFunctionKeys,
                                ULONG cFunctionKeys);

        BOOL hifAppendFunctionKey(PFUNCTIONKEY pNewKey);

        PFUNCTIONKEY hifFindFunctionKey(PFUNCTIONKEY paFunctionKeys,
                                        ULONG cFunctionKeys,
                                        UCHAR ucScanCode);

        BOOL hifDeleteFunctionKey(PFUNCTIONKEY paFunctionKeys,
                                  PULONG pcFunctionKeys,
                                  ULONG ulDelete);
    #endif

    /* ******************************************************************
     *
     *   Screenborder objects interface
     *
     ********************************************************************/

    BOOL hifSetScreenBorderObjectUnique(ULONG ulBorder,
                                        HOBJECT hobj);

    /* ******************************************************************
     *
     *   XWPKeyboard notebook callbacks (notebook.c)
     *
     ********************************************************************/

    VOID hifCollectHotkeys(MPARAM mp1,
                           MPARAM mp2);

    #ifdef NOTEBOOK_HEADER_INCLUDED

        VOID XWPENTRY hifKeybdHotkeysInitPage(PNOTEBOOKPAGE pnbp,
                                              ULONG flFlags);

        MRESULT XWPENTRY hifKeybdHotkeysItemChanged(PNOTEBOOKPAGE pnbp,
                                           ULONG ulItemID,
                                           USHORT usNotifyCode,
                                           ULONG ulExtra);

        VOID XWPENTRY hifKeybdFunctionKeysInitPage(PNOTEBOOKPAGE pnbp,
                                                   ULONG flFlags);

        MRESULT XWPENTRY hifKeybdFunctionKeysItemChanged(PNOTEBOOKPAGE pnbp,
                                                ULONG ulItemID, USHORT usNotifyCode,
                                                ULONG ulExtra);

        /* ******************************************************************
         *
         *   XWPMouse notebook callbacks (notebook.c)
         *
         ********************************************************************/

        VOID XWPENTRY hifMouseMappings2InitPage(PNOTEBOOKPAGE pnbp,
                                                ULONG flFlags);

        MRESULT XWPENTRY hifMouseMappings2ItemChanged(PNOTEBOOKPAGE pnbp,
                                             ULONG ulItemID, USHORT usNotifyCode,
                                             ULONG ulExtra);

        VOID XWPENTRY hifMouseCornersInitPage(PNOTEBOOKPAGE pnbp,
                                              ULONG flFlags);

        MRESULT XWPENTRY hifMouseCornersItemChanged(PNOTEBOOKPAGE pnbp,
                                           ULONG ulItemID, USHORT usNotifyCode,
                                           ULONG ulExtra);

        VOID XWPENTRY hifMouseMovementInitPage(PNOTEBOOKPAGE pnbp,
                                               ULONG flFlags);

        MRESULT XWPENTRY hifMouseMovementItemChanged(PNOTEBOOKPAGE pnbp,
                                            ULONG ulItemID, USHORT usNotifyCode,
                                            ULONG ulExtra);

        // the following two funcs added V0.9.14 (2001-08-02) [lafaix]
        VOID XWPENTRY hifMouseMovement2InitPage(PNOTEBOOKPAGE pnbp,
                                               ULONG flFlags);

        MRESULT XWPENTRY hifMouseMovement2ItemChanged(PNOTEBOOKPAGE pnbp,
                                            ULONG ulItemID, USHORT usNotifyCode,
                                            ULONG ulExtra);
    #endif

#endif

