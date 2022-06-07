
/*
 *@@sourcefile winh.h:
 *      header file for winh.c (PM helper funcs). See remarks there.
 *
 *      Special #define's which this thing reacts to:
 *
 *      --  If WINH_STANDARDWRAPPERS is defined to anything, this include
 *          file maps a number of freqently API calls (such as WinSendMsg)
 *          to winh* equivalents to reduce the amount of fixup records
 *          in the final executable.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 */

/*      Copyright (C) 1997-2007 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *@@include #define INCL_WINWINDOWMGR
 *@@include #define INCL_WINMESSAGEMGR
 *@@include #define INCL_WINSYS             // for winhStorePresParam
 *@@include #define INCL_WINDIALOGS
 *@@include #define INCL_WINMENUS           // for menu helpers
 *@@include #define INCL_WINENTRYFIELDS     // for entry field helpers
 *@@include #define INCL_WINBUTTONS         // for button/check box helpers
 *@@include #define INCL_WINLISTBOXES       // for list box helpers
 *@@include #define INCL_WINSTDSPIN         // for spin button helpers
 *@@include #define INCL_WINSTDSLIDER       // for slider helpers
 *@@include #define INCL_WININPUT
 *@@include #define INCL_WINSYS
 *@@include #define INCL_WINSHELLDATA
 *@@include #define INCL_WINSWITCHLIST      // for winhQuerySwitchList
 *@@include #define INCL_WINPROGRAMLIST     // for appStartApp
 *@@include #define INCL_WINHELP            // for help manager helpers
 *@@include #include <os2.h>
 *@@include #include "helpers\winh.h"
 */

#if __cplusplus
extern "C" {
#endif

#ifndef WINH_HEADER_INCLUDED
    #define WINH_HEADER_INCLUDED

    #ifndef XWPENTRY
        #error You must define XWPENTRY to contain the standard linkage for the XWPHelpers.
    #endif

    /* ******************************************************************
     *
     *   Declarations
     *
     ********************************************************************/

    #define MPNULL                 (MPFROMP(NULL))
    #define MPZERO                 (MPFROMSHORT(0))
    #define MRTRUE                 (MRFROMSHORT((SHORT) TRUE))
    #define MRFALSE                (MRFROMSHORT((SHORT) FALSE))
    #define BM_UNCHECKED           0   // for checkboxes: disabled
    #define BM_CHECKED             1   // for checkboses: enabled
    #define BM_INDETERMINATE       2   // for tri-state checkboxes: indeterminate

    // these undocumented msgs are posted whenever the mouse
    // enters or leaves a window V1.0.1 (2002-11-30) [umoeller]
    #ifndef WM_MOUSEENTER
        #define WM_MOUSEENTER   0x041E
        #define WM_MOUSELEAVE   0x041F
    #endif

    #ifndef PP_FIELDBACKGROUNDCOLOR
    #define PP_FIELDBACKGROUNDCOLOR                 57L
    #endif

    #ifndef PP_PAGEFOREGROUNDCOLOR
    #define PP_PAGEFOREGROUNDCOLOR                  68L
    #endif

    #ifndef PP_PAGEBACKGROUNDCOLOR
    #define PP_PAGEBACKGROUNDCOLOR                  69L
    #endif

    #ifndef CM_SETGRIDINFO
    #define CM_SETGRIDINFO                0x0354
    #define CM_QUERYGRIDINFO              0x0355
    #define CM_SNAPTOGRID                 0x0356
    #define CRA_LOCKED          0x00000400L /* record is locked.          */
    #endif

    /* ******************************************************************
     *
     *   Wrappers
     *
     ********************************************************************/

    // if WINH_STANDARDWRAPPERS is #define'd before including winh.h,
    // all the following Win* API calls are redirected to the winh*
    // counterparts

    #ifdef WINH_STANDARDWRAPPERS

        MRESULT XWPENTRY winhSendMsg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
        #define WinSendMsg(a,b,c,d) winhSendMsg((a),(b),(c),(d))

        MRESULT XWPENTRY winhSendDlgItemMsg(HWND hwnd, ULONG id, ULONG msg, MPARAM mp1, MPARAM mp2);
        #define WinSendDlgItemMsg(a,b,c,d,e) winhSendDlgItemMsg((a),(b),(c),(d),(e))

        BOOL XWPENTRY winhPostMsg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
        #define WinPostMsg(a,b,c,d) winhPostMsg((a),(b),(c),(d))

        HWND XWPENTRY winhWindowFromID(HWND hwnd, ULONG id);
        #define WinWindowFromID(a,b) winhWindowFromID((a),(b))

        HWND XWPENTRY winhQueryWindow(HWND hwnd, LONG lCode);
        #define WinQueryWindow(a,b) winhQueryWindow((a),(b))

        PVOID XWPENTRY winhQueryWindowPtr(HWND hwnd, LONG index);
        #define WinQueryWindowPtr(a,b) winhQueryWindowPtr((a),(b))

        BOOL XWPENTRY winhSetWindowText2(HWND hwnd, const char *pcsz);
        #define WinSetWindowText(a,b) winhSetWindowText2((a),(b))

        BOOL XWPENTRY winhSetDlgItemText(HWND hwnd, ULONG id, const char *pcsz);
        #define WinSetDlgItemText(a,b,c) winhSetDlgItemText((a),(b),(c))

        // pmwin.h defines the WinEnableControl macro; turn this into our function call
        #ifdef WinEnableControl
            #undef WinEnableControl
        #endif
        #define WinEnableControl(hwndDlg, id, fEnable) winhEnableDlgItem((hwndDlg), (id), (fEnable))

        #ifdef INCL_WINMESSAGEMGR
            APIRET XWPENTRY winhRequestMutexSem(HMTX hmtx, ULONG ulTimeout);
            #define WinRequestMutexSem(a, b) winhRequestMutexSem((a), (b))
        #endif
    #endif

    /* ******************************************************************
     *
     *   Macros
     *
     ********************************************************************/

    /*
     *  Here come some monster macros for
     *  frequently needed functions.
     */

    #define winhDebugBox(hwndOwner, title, text) \
    WinMessageBox(HWND_DESKTOP, hwndOwner, ((PSZ)text), ((PSZ)title), 0, MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE)

    #define winhYesNoBox(title, text) \
    WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, ((PSZ)text), ((PSZ)title), 0, MB_YESNO | MB_ICONQUESTION | MB_MOVEABLE)

    #define winhSetDlgItemChecked(hwnd, id, bCheck) \
            WinSendDlgItemMsg((hwnd), (id), BM_SETCHECK, MPFROMSHORT(bCheck), MPNULL)
    #define winhIsDlgItemChecked(hwnd, id) \
            (SHORT1FROMMR(WinSendDlgItemMsg((hwnd), (id), BM_QUERYCHECK, MPNULL, MPNULL)))

    #define winhSetMenuItemChecked(hwndMenu, usId, bCheck) \
            WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(usId, TRUE), \
                    MPFROM2SHORT(MIA_CHECKED, (((bCheck)) ? MIA_CHECKED : FALSE)))

    #define winhShowDlgItem(hwnd, id, show) \
            WinShowWindow(WinWindowFromID(hwnd, id), show)

    // made these functions V0.9.12 (2001-05-18) [umoeller]
    /* #define winhEnableDlgItem(hwndDlg, ulId, Enable) \
            WinEnableWindow(WinWindowFromID(hwndDlg, ulId), Enable)
    #define winhIsDlgItemEnabled(hwndDlg, ulId) \
            WinIsWindowEnabled(WinWindowFromID(hwndDlg, ulId))
        */

    #define winhSetDlgItemFocus(hwndDlg, ulId) \
            WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwndDlg, ulId))

    /* ******************************************************************
     *
     *   Rectangle helpers
     *
     ********************************************************************/

    VOID XWPENTRY winhOffsetRect(PRECTL prcl, LONG lx, LONG ly);

    /* ******************************************************************
     *
     *   Generics
     *
     ********************************************************************/

    extern LONG G_cxScreen,
                G_cyScreen,
                G_cxIcon,
                G_cyIcon,
                G_lcol3DDark,
                G_lcol3DLight;

    VOID XWPENTRY winhInitGlobals(VOID);

    ULONG XWPENTRY winhQueryWindowStyle(HWND hwnd);

    BOOL XWPENTRY winhEnableDlgItem(HWND hwndDlg, SHORT id, BOOL fEnable);

    BOOL XWPENTRY winhIsDlgItemEnabled(HWND hwndDlg, SHORT id);

    BOOL winhDestroyWindow(HWND *phwnd);

    /* ******************************************************************
     *
     *   Menu helpers
     *
     ********************************************************************/

    // now including all this only with INCL_WINMENUS
    // V1.0.1 (2002-11-30) [umoeller]
    #ifdef INCL_WINMENUS

        #ifndef MM_QUERYITEMBYPOS16
            #define MM_QUERYITEMBYPOS16 0x01f3
            // this undocumented message is sent to retrieve the definition
            // of a menu item by its position.  This message will only query
            // an item in the specified menu.
            //
            // Parameters:
            //     SHORT1FROMMP(mp1) = position of item in the menu
            //     SHORT2FROMMP(mp1) = reserved, set to 0
            //     mp2 = (16 bit pointer) points to a MENUITEM structure
            //           to be filled in.
        #endif
        #ifndef MAKE_16BIT_POINTER
            #define MAKE_16BIT_POINTER(p) \
                    ((PVOID)MAKEULONG(LOUSHORT(p),(HIUSHORT(p) << 3) | 7))
            // converts a flat 32bit pointer to its 16bit offset/selector form
        #endif

        /*
         * winhCreateEmptyMenu:
         *      this macro creates an empty menu, which can
         *      be used with winhInsertMenuItem etc. later.
         *      Useful for creating popup menus on the fly.
         *      Note that even though HWND_DESKTOP is specified
         *      here as both the parent and the owner, the
         *      actual owner and parent are specified later
         *      with WinPopupMenu.
         */

        #define winhCreateEmptyMenu()                                   \
        WinCreateWindow(HWND_DESKTOP, WC_MENU, "", 0, 0, 0, 0, 0,       \
                        HWND_DESKTOP, HWND_TOP, 0, 0, 0)

        BOOL XWPENTRY winhQueryMenuItem(HWND hwndMenu,
                                        USHORT usItemID,
                                        BOOL fSearchSubmenus,
                                        PMENUITEM pmi);

        HWND XWPENTRY winhQuerySubmenu(HWND hMenu, SHORT sID);

        SHORT XWPENTRY winhInsertMenuItem(HWND hwndMenu,
                                          SHORT iPosition,
                                          SHORT sItemId,
                                          const char *pcszItemTitle,
                                          SHORT afStyle,
                                          SHORT afAttr);
        typedef SHORT XWPENTRY WINHINSERTMENUITEM(HWND hwndMenu,
                                                  SHORT iPosition,
                                                  SHORT sItemId,
                                                  const char *pcszItemTitle,
                                                  SHORT afStyle,
                                                  SHORT afAttr);
        typedef WINHINSERTMENUITEM *PWINHINSERTMENUITEM;

        HWND XWPENTRY winhInsertSubmenu(HWND hwndMenu,
                                        ULONG iPosition,
                                        SHORT sMenuId,
                                        const char *pcszSubmenuTitle,
                                        USHORT afMenuStyle,
                                        SHORT sItemId,
                                        const char *pcszItemTitle,
                                        USHORT afItemStyle,
                                        USHORT afAttribute);
        typedef HWND XWPENTRY WINHINSERTSUBMENU(HWND hwndMenu,
                                                ULONG iPosition,
                                                SHORT sMenuId,
                                                const char *pcszSubmenuTitle,
                                                USHORT afMenuStyle,
                                                SHORT sItemId,
                                                const char *pcszItemTitle,
                                                USHORT afItemStyle,
                                                USHORT afAttribute);
        typedef WINHINSERTSUBMENU *PWINHINSERTSUBMENU;

        BOOL XWPENTRY winhSetMenuCondCascade(HWND hwndMenu,
                                             LONG lDefaultItem);

        /*
         *@@ winhRemoveMenuItem:
         *      removes a menu item (SHORT) from the
         *      given menu (HWND). Returns the no. of
         *      remaining menu items (SHORT).
         *
         *      This works for whole submenus too.
         */

        #define winhRemoveMenuItem(hwndMenu, sItemID) \
                SHORT1FROMMR(WinSendMsg(hwndMenu, MM_REMOVEITEM, MPFROM2SHORT(sItemID, FALSE), 0))

        BOOL XWPENTRY winhRemoveMenuItems(HWND hwndMenu,
                                          const SHORT *asItemIDs,
                                          ULONG cItemIDs);

        /*
         *@@ winhDeleteMenuItem:
         *      deleted a menu item (SHORT) from the
         *      given menu (HWND). Returns the no. of
         *      remaining menu items (SHORT).
         *
         *      As opposed to MM_REMOVEITEM, MM_DELETEITEM
         *      frees submenus and bitmaps also.
         *
         *      This works for whole submenus too.
         */

        #define winhDeleteMenuItem(hwndMenu, sItemId) \
                (SHORT)WinSendMsg(hwndMenu, MM_DELETEITEM, MPFROM2SHORT(sItemId, FALSE), 0)

        SHORT XWPENTRY winhInsertMenuSeparator(HWND hMenu,
                                               SHORT iPosition,
                                               SHORT sId);

        #define COPYFL_STRIPTABS            0x0001

        BOOL XWPENTRY winhCopyMenuItem2(HWND hmenuTarget,
                                        HWND hmenuSource,
                                        USHORT usID,
                                        SHORT sTargetPosition,
                                        ULONG fl);

        BOOL XWPENTRY winhCopyMenuItem(HWND hmenuTarget,
                                       HWND hmenuSource,
                                       USHORT usID,
                                       SHORT sTargetPosition);
        typedef BOOL XWPENTRY WINHCOPYMENUITEM(HWND hmenuTarget,
                                       HWND hmenuSource,
                                       USHORT usID,
                                       SHORT sTargetPosition);
        typedef WINHCOPYMENUITEM *PWINHCOPYMENUITEM;

        HWND XWPENTRY winhMergeIntoSubMenu(HWND hmenuTarget,
                                           SHORT sTargetPosition,
                                           const char *pcszTitle,
                                           SHORT sID,
                                           HWND hmenuSource);
        typedef HWND XWPENTRY WINHMERGEINTOSUBMENU(HWND hmenuTarget,
                                           SHORT sTargetPosition,
                                           const char *pcszTitle,
                                           SHORT sID,
                                           HWND hmenuSource);
        typedef WINHMERGEINTOSUBMENU *PWINHMERGEINTOSUBMENU;

        ULONG XWPENTRY winhMergeMenus(HWND hmenuTarget,
                                      SHORT sTargetPosition,
                                      HWND hmenuSource,
                                      ULONG fl);
        typedef ULONG XWPENTRY WINHMERGEMENUS(HWND hmenuTarget,
                                              SHORT sTargetPosition,
                                              HWND hmenuSource,
                                              ULONG fl);
        typedef WINHMERGEMENUS *PWINHMERGEMENUS;

        ULONG XWPENTRY winhClearMenu(HWND hwndMenu);

        PSZ XWPENTRY winhQueryMenuItemText(HWND hwndMenu,
                                           USHORT usItemID);

        BOOL XWPENTRY winhAppend2MenuItemText(HWND hwndMenu,
                                              USHORT usItemID,
                                              const char *pcszAppend,
                                              BOOL fTab);

        VOID XWPENTRY winhMenuRemoveEllipse(HWND hwndMenu,
                                            USHORT usItemId);

        SHORT XWPENTRY winhQueryItemUnderMouse(HWND hwndMenu, POINTL *pptlMouse, RECTL *prtlItem);

    #endif

    /* ******************************************************************
     *
     *   Slider helpers
     *
     ********************************************************************/

    HWND XWPENTRY winhReplaceWithLinearSlider(HWND hwndParent,
                                              HWND hwndOwner,
                                              HWND hwndInsertAfter,
                                              ULONG ulID,
                                              ULONG ulSliderStyle,
                                              ULONG ulTickCount);

    BOOL XWPENTRY winhSetSliderTicks(HWND hwndSlider,
                                     MPARAM mpEveryOther1,
                                     ULONG ulPixels1,
                                     MPARAM mpEveryOther2,
                                     ULONG ulPixels2);
    typedef BOOL XWPENTRY WINHSETSLIDERTICKS(HWND hwndSlider,
                                             MPARAM mpEveryOther1,
                                             ULONG ulPixels1,
                                             MPARAM mpEveryOther2,
                                             ULONG ulPixels2);
    typedef WINHSETSLIDERTICKS *PWINHSETSLIDERTICKS;

    /*
     * winhSetSliderArmPosition:
     *      this moves the slider arm in a given
     *      linear slider.
     *
     *      usMode can be one of the following:
     *      --  SMA_RANGEVALUE: usOffset is in pixels
     *          from the slider's home position.
     *      --  SMA_INCREMENTVALUE: usOffset is in
     *          units of the slider's primary scale.
     */

    #define winhSetSliderArmPosition(hwndSlider, usMode, usOffset)  \
            WinSendMsg(hwndSlider, SLM_SETSLIDERINFO,               \
                   MPFROM2SHORT(SMA_SLIDERARMPOSITION,              \
                     usMode),                                       \
                   MPFROMSHORT(usOffset))

    /*
     * winhQuerySliderArmPosition:
     *      reverse to the previous, this returns a
     *      slider arm position (as a LONG value).
     */

    #define winhQuerySliderArmPosition(hwndSlider, usMode)          \
            (LONG)(WinSendMsg(hwndSlider,                           \
                              SLM_QUERYSLIDERINFO,                  \
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,   \
                                           usMode),                 \
                              0))

    HWND XWPENTRY winhReplaceWithCircularSlider(HWND hwndParent,
                                                HWND hwndOwner,
                                                HWND hwndInsertAfter,
                                                ULONG ulID,
                                                ULONG ulSliderStyle,
                                                SHORT sMin,
                                                SHORT sMax,
                                                USHORT usIncrement,
                                                USHORT usTicksEvery);

    /* ******************************************************************
     *
     *   Spin button helpers
     *
     ********************************************************************/

    VOID XWPENTRY winhSetDlgItemSpinData(HWND hwndDlg,
                                         ULONG idSpinButton,
                                         ULONG min,
                                         ULONG max,
                                         ULONG current);
    typedef VOID XWPENTRY WINHSETDLGITEMSPINDATA(HWND hwndDlg,
                                                 ULONG idSpinButton,
                                                 ULONG min,
                                                 ULONG max,
                                                 ULONG current);
    typedef WINHSETDLGITEMSPINDATA *PWINHSETDLGITEMSPINDATA;

    LONG XWPENTRY winhAdjustDlgItemSpinData(HWND hwndDlg,
                                            USHORT usItemID,
                                            LONG lGrid,
                                            USHORT usNotifyCode);
    typedef LONG XWPENTRY WINHADJUSTDLGITEMSPINDATA(HWND hwndDlg,
                                                    USHORT usItemID,
                                                    LONG lGrid,
                                                    USHORT usNotifyCode);
    typedef WINHADJUSTDLGITEMSPINDATA *PWINHADJUSTDLGITEMSPINDATA;

    /* ******************************************************************
     *
     *   Entry field helpers
     *
     ********************************************************************/

    /*
     * winhSetEntryFieldLimit:
     *      sets the maximum length for the entry field
     *      (EM_SETTEXTLIMIT message).
     *
     *      PMREF doesn't say this, but the limit does
     *      _not_ include the null-terminator. That is,
     *      if you specify "12" characters here, you can
     *      really enter 12 characters.
     *
     *      The default length is 30 characters, I think.
     */

    #define winhSetEntryFieldLimit(hwndEntryField, usLength)        \
            WinSendMsg(hwndEntryField, EM_SETTEXTLIMIT, (MPARAM)(usLength), (MPARAM)0)

    /*
     *@@ winhEntryFieldSelectAll:
     *      this selects the entire text in the entry field.
     *      Useful when the thing gets the focus.
     */

    #define winhEntryFieldSelectAll(hwndEntryField)                 \
            WinSendMsg(hwndEntryField, EM_SETSEL, MPFROM2SHORT(0, 10000), MPNULL)

    /*
     *@@ winhHasEntryFieldChanged:
     *      this queries whether the entry field's contents
     *      have changed (EM_QUERYCHANGED message).
     *
     *      This returns TRUE if changes have occurred since
     *      the last time this message or WM_QUERYWINDOWPARAMS
     *      (WinQueryWindowText) was received.
     */

    #define winhHasEntryFieldChanged(hwndEntryField)                \
            (BOOL)WinSendMsg(hwndEntryField, EM_QUERYCHANGED, (MPARAM)0, (MPARAM)0)

    /* ******************************************************************
     *
     *   List box helpers
     *
     ********************************************************************/

    /*  The following macros are defined in the OS/2 headers for
        list boxes:

        LONG WinQueryLboxCount(HWND hwndLbox);
                    // wrapper around LM_QUERYITEMCOUNT;
                    // list box item count

        SHORT WinQueryLboxItemTextLength(HWND hwndLbox,
                                         SHORT index); // item index, starting from 0
                    // wrapper around LM_QUERYITEMTEXTLENGTH;
                    // returns length of item text, excluding NULL character

        LONG WinQueryLboxItemText(HWND hwndLbox,
                                  SHORT index,      // item index, starting from 0
                                  PSZ psz,          // buffer
                                  PSZ cchMax);      // size of buffer, incl. null
                    // wrapper around LM_QUERYITEMTEXT;
                    // returns length of item text, excluding NULL character

        BOOL WinSetLboxItemText(HWND hwndLbox,
                                LONG index,
                                PSZ psz);
                    // wrapper around LM_SETITEMTEXT

        LONG WinInsertLboxItem(HWND hwndLbox,
                               LONG index,          // new item index, starting from 0
                                                    // or LIT_END
                                                    // or LIT_SORTASCENDING
                                                    // or LIT_SORTDESCENDING
                               PSZ psz)
                    // wrapper around LM_INSERTITEM;
                    // returns LIT_MEMERROR, LIT_ERROR, or zero-based index

        LONG WinDeleteLboxItem(HWND hwndLbox,
                               LONG index);         // item index, starting from 0

        LONG WinQueryLboxSelectedItem(HWND hwndLbox);
                    // wrapper around LM_QUERYSELECTION;
                    // works with single selection only,
                    // use winhQueryLboxSelectedItem instead
    */

    /*
     * winhQueryLboxItemCount:
     *      returns the no. of items in the listbox
     *      as a SHORT.
     *
     *added V0.9.1 (99-12-14) [umoeller]
     */

    // #define winhQueryLboxItemCount(hwndListBox)
    // (SHORT)WinSendMsg(hwndListBox, LM_QUERYITEMCOUNT, 0, 0)

    // removed, use WinQueryLboxCount

    /*
     *@@ winhDeleteAllItems:
     *      deletes all list box items. Returns
     *      a BOOL.
     *
     *@@added V0.9.1 (99-12-14) [umoeller]
     */

    #define winhDeleteAllItems(hwndListBox)                     \
        (BOOL)WinSendMsg(hwndListBox,                           \
                         LM_DELETEALL, 0, 0)
    /*
     *@@ winhQueryLboxSelectedItem:
     *      this queries the next selected list box item.
     *      For the first call, set sItemStart to LIT_FIRST;
     *      then repeat the call with sItemStart set to
     *      the previous return value until this returns
     *      LIT_NONE.
     *
     *      Example:
     +          SHORT sItemStart = LIT_FIRST;
     +          while (TRUE)
     +          {
     +              sItemStart = winhQueryLboxSelectedItem(hwndListBox,
     +                                                     sItemStart)
     +              if (sItemStart == LIT_NONE)
     +                  break;
     +              ...
     +          }
     *
     *      To have the cursored item returned, use LIT_CURSOR.
     *
     *      For single selection, you can also use
     *      WinQueryLboxSelectedItem from the OS/2 PM headers.
     */

    #define winhQueryLboxSelectedItem(hwndListBox, sItemStart) \
            SHORT1FROMMR(WinSendMsg(hwndListBox,                    \
                            LM_QUERYSELECTION,                 \
                            (MPARAM)(sItemStart),              \
                            MPNULL))

    /*
     *@@ winhSetLboxSelectedItem:
     *      selects a list box item.
     *      This works for both single-selection and
     *      multiple-selection listboxes.
     *      In single-selection listboxes, if an item
     *      is selected (fSelect == TRUE), the previous
     *      item is automatically deselected.
     *      Note that (BOOL)fSelect is ignored if
     *      sItemIndex == LIT_NONE.
     */

    #define winhSetLboxSelectedItem(hwndListBox, sItemIndex, fSelect)   \
            (BOOL)(WinSendMsg(hwndListBox,                              \
                            LM_SELECTITEM,                              \
                            (MPARAM)(sItemIndex),                       \
                            (MPARAM)(fSelect)))

    ULONG XWPENTRY winhLboxSelectAll(HWND hwndListBox, BOOL fSelect);

    /*
     * winhSetLboxItemHandle:
     *      sets the "item handle" for the specified sItemIndex.
     *      See LM_SETITEMHANDLE in PMREF for details.
     */

    #define winhSetLboxItemHandle(hwndListBox, sItemIndex, ulHandle)    \
            (BOOL)(WinSendMsg(hwndListBox, LM_SETITEMHANDLE,            \
                              (MPARAM)(sItemIndex),                     \
                              (MPARAM)ulHandle))

    /*
     * winhQueryLboxItemHandle:
     *      the reverse to the previous. Returns a ULONG.
     */

    #define winhQueryLboxItemHandle(hwndListBox, sItemIndex)            \
            (ULONG)WinSendMsg(hwndListBox, LM_QUERYITEMHANDLE,          \
                              MPFROMSHORT(sItemIndex), (MPARAM)NULL)

    PSZ XWPENTRY winhQueryLboxItemText(HWND hwndListbox, SHORT sIndex);

    BOOL XWPENTRY winhMoveLboxItem(HWND hwndSource,
                                   SHORT sSourceIndex,
                                   HWND hwndTarget,
                                   SHORT sTargetIndex,
                                   BOOL fSelectTarget);

    ULONG XWPENTRY winhLboxFindItemFromHandle(HWND hwndListBox,
                                              ULONG ulHandle);

    /* ******************************************************************
     *
     *   Scroll bar helpers
     *
     ********************************************************************/

    BOOL XWPENTRY winhUpdateScrollBar(HWND hwndScrollBar,
                                      ULONG ulWinPels,
                                      ULONG ulViewportPels,
                                      ULONG ulCurUnitOfs,
                                      BOOL fAutoHide);

    LONG XWPENTRY winhHandleScrollMsg(HWND hwndScrollBar,
                                      PLONG plCurPelsOfs,
                                      LONG lWindowPels,
                                      LONG lWorkareaPels,
                                      USHORT usLineStepPels,
                                      ULONG msg,
                                      MPARAM mp2);

    BOOL XWPENTRY winhScrollWindow(HWND hwnd2Scroll,
                                   PRECTL prclClip,
                                   PPOINTL pptlScroll);

    BOOL XWPENTRY winhProcessScrollChars(HWND hwndClient,
                                         HWND hwndVScroll,
                                         HWND hwndHScroll,
                                         MPARAM mp1,
                                         MPARAM mp2,
                                         ULONG ulVertMax,
                                         ULONG ulHorzMax);

    /*
     *@@ SCROLLABLEWINDOW:
     *      scroll data struct for use with winhHandleScrollerMsgs.
     *
     *@@added V1.0.1 (2003-01-25) [umoeller]
     */

    typedef struct _SCROLLABLEWINDOW
    {
        LONG    cxScrollBar,
                cyScrollBar;
        HWND    hwndVScroll,        // vertical scroll bar
                hwndHScroll;        // horizontal scroll bar
        ULONG   idVScroll,
                idHScroll;
        SIZEL   szlWorkarea;        // workarea dimensions (over which window scrolls)
        POINTL  ptlScrollOfs;       // current scroll offset; positive x means right,
                                    // positive y means down
    } SCROLLABLEWINDOW, *PSCROLLABLEWINDOW;

    #define ID_VSCROLL      100
    #define ID_HSCROLL      101

    BOOL XWPENTRY winhCreateScroller(HWND hwndParent,
                                     PSCROLLABLEWINDOW pscrw,
                                     ULONG idVScroll,
                                     ULONG idHScroll);

    MRESULT XWPENTRY winhHandleScrollerMsgs(HWND hwnd2Scroll,
                                            PSCROLLABLEWINDOW pscrw,
                                            PSIZEL pszlWin,
                                            ULONG msg,
                                            MPARAM mp1,
                                            MPARAM mp2);

    /* ******************************************************************
     *
     *   Window positioning helpers
     *
     ********************************************************************/

    BOOL XWPENTRY winhSaveWindowPos(HWND hwnd, HINI hIni, const char *pcszApp, const char *pcszKey);

    BOOL XWPENTRY winhRestoreWindowPos(HWND hwnd, HINI hIni, const char *pcszApp, const char *pcszKey, ULONG fl);

    // V1.0.6 (2006-10-28) [pr]
    // V1.0.7 (2006-12-16) [pr]: Named screen width/height parameters

    /*
     *@@ STOREPOS:
     *
     */

    #pragma pack(2)
    typedef struct _STOREPOS
    {
        USHORT      usMagic;          // Always 0x7B6A (???)
        ULONG       ulFlags;
        USHORT      usXPos;
        USHORT      usYPos;
        USHORT      usWidth;
        USHORT      usHeight;
        ULONG       ulRes1;           // Always 1 (???)
        USHORT      usRestoreXPos;
        USHORT      usRestoreYPos;
        USHORT      usRestoreWidth;
        USHORT      usRestoreHeight;
        ULONG       ulRes2;           // Always 1 (???)
        USHORT      usMinXPos;
        USHORT      usMinYPos;
        ULONG       ulScreenWidth;
        ULONG       ulScreenHeight;
        ULONG       ulRes3;           // Always 0xFFFFFFFF (???)
        ULONG       ulRes4;           // Always 0xFFFFFFFF (???)
        ULONG       ulPPLen;          // Presentation Parameters length
    } STOREPOS, *PSTOREPOS;
    #pragma pack()

    #pragma import(WinGetFrameTreePPSize, , "PMWIN", 972)
    #pragma import(WinGetFrameTreePPs, , "PMWIN", 973)

    ULONG APIENTRY WinGetFrameTreePPSize(HWND hwnd);
    ULONG APIENTRY WinGetFrameTreePPs(HWND hwnd, ULONG cchMax, PCH pch);

    BOOL XWPENTRY winhStoreWindowPos(HWND hwnd, HINI hIni, const char *pcszApp, const char *pcszKey);

    #define XAC_MOVEX       0x0001
    #define XAC_MOVEY       0x0002
    #define XAC_SIZEX       0x0004
    #define XAC_SIZEY       0x0008

    /*
     *@@ XADJUSTCTRLS:
     *
     */

    typedef struct _XADJUSTCTRLS
    {
        BOOL        fInitialized;
        SWP         swpMain;            // SWP for main window
        SWP         *paswp;             // pointer to array of control SWP structs
    } XADJUSTCTRLS, *PXADJUSTCTRLS;

    BOOL XWPENTRY winhAdjustControls(HWND hwndDlg,
                                     const MPARAM *pmpFlags,
                                     ULONG ulCount,
                                     PSWP pswpNew,
                                     PXADJUSTCTRLS pxac);

    void XWPENTRY winhCenterWindow(HWND hwnd);
    typedef void XWPENTRY WINHCENTERWINDOW(HWND hwnd);
    typedef WINHCENTERWINDOW *PWINHCENTERWINDOW;

    #define PLF_SMART               0x0001

    BOOL winhPlaceBesides(HWND hwnd,
                          HWND hwndRelative,
                          ULONG fl);

    HWND XWPENTRY winhFindWindowBelow(HWND hwndFind);

    /* ******************************************************************
     *
     *   Presparams helpers
     *
     ********************************************************************/

    PSZ XWPENTRY winhQueryWindowFont(HWND hwnd);
    typedef PSZ XWPENTRY WINHQUERYWINDOWFONT(HWND hwnd);
    typedef WINHQUERYWINDOWFONT *PWINHQUERYWINDOWFONT;

    PCSZ XWPENTRY winhQueryDefaultFont(VOID);

    PSZ XWPENTRY winhQueryMenuSysFont(VOID);

    BOOL XWPENTRY winhSetWindowFont(HWND hwnd, const char *pcszFont);
    typedef BOOL XWPENTRY WINHSETWINDOWFONT(HWND hwnd, const char *pcszFont);
    typedef WINHSETWINDOWFONT *PWINHSETWINDOWFONT;

    /*
     *@@ winhSetDlgItemFont:
     *      invokes winhSetWindowFont on a dialog
     *      item.
     *
     *      Returns TRUE if successful or FALSE otherwise.
     *
     *@@added V0.9.0
     */

    #define winhSetDlgItemFont(hwnd, usId, pszFont) \
            (winhSetWindowFont(WinWindowFromID(hwnd, usId), pszFont))

    ULONG XWPENTRY winhSetControlsFont(HWND hwndDlg, SHORT usIDMin, SHORT usIDMax, const char *pcszFont);

    #ifdef INCL_WINSYS
        BOOL XWPENTRY winhStorePresParam(PPRESPARAMS *pppp,
                                         ULONG ulAttrType,
                                         ULONG cbData,
                                         PVOID pData);

        PPRESPARAMS XWPENTRY winhCreateDefaultPresparams(VOID);
    #endif

    LONG XWPENTRY winhQueryPresColor(HWND hwnd, ULONG ulPP, BOOL fInherit, LONG lSysColor);
    typedef LONG XWPENTRY WINHQUERYPRESCOLOR(HWND hwnd, ULONG ulPP, BOOL fInherit, LONG lSysColor);
    typedef WINHQUERYPRESCOLOR *PWINHQUERYPRESCOLOR;

    LONG XWPENTRY winhQueryPresColor2(HWND hwnd,
                                      ULONG ulppRGB,
                                      ULONG ulppIndex,
                                      BOOL fInherit,
                                      LONG lSysColor);

    BOOL XWPENTRY winhSetPresColor(HWND hwnd, ULONG ulIndex, LONG lColor);
    // XWP V1.0.8 (2007-05-08) [pr]
    typedef BOOL XWPENTRY WINHSETPRESCOLOR(HWND hwnd, ULONG ulIndex, LONG lColor);
    typedef WINHSETPRESCOLOR *PWINHSETPRESCOLOR;

    /* ******************************************************************
     *
     *   Help instance helpers
     *
     ********************************************************************/

    #ifdef INCL_WINHELP
        HWND XWPENTRY winhCreateHelp(HWND hwndFrame,
                                     const char *pcszFileName,
                                     HMODULE hmod,
                                     PHELPTABLE pHelpTable,
                                     const char *pcszWindowTitle);

        ULONG winhDisplayHelpPanel(HWND hwndHelpInstance,
                                   ULONG ulHelpPanel);

        void XWPENTRY winhDestroyHelp(HWND hwndHelp, HWND hwndFrame);
    #endif

    /* ******************************************************************
     *
     *   Application control
     *
     ********************************************************************/

    BOOL XWPENTRY winhAnotherInstance(const char *pcszSemName, BOOL fSwitch);

    HSWITCH XWPENTRY winhAddToTasklist(HWND hwnd, HPOINTER hIcon);

    BOOL XWPENTRY winhUpdateTasklist(HWND hwnd, PCSZ pcszNewTitle);

    /* ******************************************************************
     *
     *   Miscellaneous
     *
     ********************************************************************/

    VOID XWPENTRY winhFree(PVOID p);
    typedef VOID XWPENTRY WINHFREE(PVOID p);
    typedef WINHFREE *PWINHFREE;

    /*
     *@@ winhMyAnchorBlock:
     *      returns the HAB of the current thread.
     *
     *      Many Win* functions require an HAB to be
     *      passed in. While many of them will work
     *      when passing in NULLHANDLE, some (such as
     *      WinGetMsg) won't. If you don't know the
     *      anchor block of the calling thread, use
     *      this function.
     *
     *      The HAB is simply a LONG whose hiword has
     *      the current PID and the lowword has the
     *      current TID. The previous function that
     *      created a temporary window to figure this
     *      out has been replaced with this macro, since
     *      WinQueryAnchorBlock(HWND_DESKTOP) yields the
     *      same result.
     *
     *@@added V1.0.1 (2002-11-30) [umoeller]
     */

    #define winhMyAnchorBlock() WinQueryAnchorBlock(HWND_DESKTOP)

    /*
    HAB XWPENTRY winhMyAnchorBlock(VOID);
    typedef HAB XWPENTRY WINHMYANCHORBLOCK(VOID);
    typedef WINHMYANCHORBLOCK *PWINHMYANCHORBLOCK;
    */

    VOID XWPENTRY winhSleep(ULONG ulSleep);

    #define WINH_FOD_SAVEDLG        0x0001
    #define WINH_FOD_INILOADDIR     0x0010
    #define WINH_FOD_INISAVEDIR     0x0020

    BOOL XWPENTRY winhFileDlg(HWND hwndOwner,
                              PSZ pszFile,
                              ULONG flFlags,
                              HINI hini,
                              const char *pcszApplication,
                              const char *pcszKey);

    HPOINTER XWPENTRY winhQueryWaitPointer(VOID);

    HPOINTER XWPENTRY winhSetWaitPointer(VOID);

    PSZ XWPENTRY winhQueryWindowText2(HWND hwnd,
                                      PULONG pulExtra);

    PSZ XWPENTRY winhQueryWindowText(HWND hwnd);

    BOOL XWPENTRY winhSetWindowText(HWND hwnd,
                                    const char *pcszFormat,
                                    ...);

    PSZ XWPENTRY winhQueryDlgItemText2(HWND hwnd,
                                       USHORT usItemID,
                                       PULONG pulExtra);

    /*
     *@@ winhQueryDlgItemText:
     *      like winhQueryWindowText, but for the dialog item
     *      in hwnd which has the ID usItemID.
     *
     *@@changed V1.0.1 (2003-01-05) [umoeller]: now using winhQueryDlgItemText2
     */

    #define winhQueryDlgItemText(hwnd, usItemID) winhQueryDlgItemText2(hwnd, usItemID, NULL)

    BOOL XWPENTRY winhAppendWindowEllipseText(HWND hwnd);

    BOOL XWPENTRY winhAppendDlgItemEllipseText(HWND hwnd,
                                               USHORT usItemID);

    BOOL XWPENTRY winhReplaceWindowText(HWND hwnd,
                                        const char *pcszSearch,
                                        const char *pcszReplaceWith);

    ULONG XWPENTRY winhCenteredDlgBox(HWND hwndParent, HWND hwndOwner,
                  PFNWP pfnDlgProc, HMODULE hmod, ULONG idDlg, PVOID pCreateParams);
    typedef ULONG XWPENTRY WINHCENTEREDDLGBOX(HWND hwndParent, HWND hwndOwner,
                  PFNWP pfnDlgProc, HMODULE hmod, ULONG idDlg, PVOID pCreateParams);
    typedef WINHCENTEREDDLGBOX *PWINHCENTEREDDLGBOX;

    ULONG XWPENTRY winhEnableControls(HWND hwndDlg,
                                      USHORT usIDFirst,
                                      USHORT usIDLast,
                                      BOOL fEnable);

    ULONG winhEnableControls2(HWND hwndDlg,
                              const ULONG *paulIDs,
                              ULONG cIDs,
                              BOOL fEnable);

    HWND XWPENTRY winhCreateStdWindow(HWND hwndFrameParent,
                                      PSWP pswpFrame,
                                      ULONG flFrameCreateFlags,
                                      ULONG ulFrameStyle,
                                      const char *pcszFrameTitle,
                                      ULONG ulResourcesID,
                                      const char *pcszClassClient,
                                      ULONG flStyleClient,
                                      ULONG ulID,
                                      PVOID pClientCtlData,
                                      PHWND phwndClient);

    HWND XWPENTRY winhCreateObjectWindow(const char *pcszWindowClass,
                                         PVOID pvCreateParam);

    HWND XWPENTRY winhCreateControl(HWND hwndParent,
                                    HWND hwndOwner,
                                    const char *pcszClass,
                                    const char *pcszText,
                                    ULONG ulStyle,
                                    ULONG ulID);

    BOOL XWPENTRY winhSetParentAndOwner(HWND hwnd,
                                        HWND hwndNewParent,
                                        BOOL fRedraw);

    VOID XWPENTRY winhRepaintWindows(HWND hwndParent);

    HMQ XWPENTRY winhFindMsgQueue(PID pid, TID tid, HAB* phab);

    VOID XWPENTRY winhFindPMErrorWindows(HWND *phwndHardError, HWND *phwndSysError);

    HWND XWPENTRY winhCreateFakeDesktop(HWND hwndSibling);

    // Warp 4 notebook button style
    #ifndef BS_NOTEBOOKBUTTON
        #define BS_NOTEBOOKBUTTON       8L
    #endif

    BOOL XWPENTRY winhAssertWarp4Notebook(HWND hwndDlg,
                                          USHORT usIdThreshold);

    ULONG XWPENTRY winhDrawFormattedText(HPS hps, PRECTL prcl, const char *pcszText, ULONG flCmd);

    #ifdef INCL_WINSWITCHLIST
        PSWBLOCK XWPENTRY winhQuerySwitchList(HAB hab);
        typedef PSWBLOCK XWPENTRY WINHQUERYSWITCHLIST(HAB hab);
        typedef WINHQUERYSWITCHLIST *PWINHQUERYSWITCHLIST;

        HSWITCH XWPENTRY winhHSWITCHfromHAPP(HAPP happ);
    #endif

    HWND XWPENTRY winhQueryTasklistWindow(VOID);
    typedef HWND XWPENTRY WINHQUERYTASKLISTWINDOW(VOID);
    typedef WINHQUERYTASKLISTWINDOW *PWINHQUERYTASKLISTWINDOW;

    VOID XWPENTRY winhKillTasklist(VOID);

    ULONG XWPENTRY winhQueryPendingSpoolJobs(VOID);

    VOID XWPENTRY winhSetNumLock(BOOL fState);

    BOOL XWPENTRY winhSetClipboardText(HAB hab,
                                       PCSZ pcsz,
                                       ULONG cbSize);

    /* ******************************************************************
     *
     *   WPS Class List helpers
     *
     ********************************************************************/

    PBYTE XWPENTRY winhQueryWPSClassList(VOID);

    PBYTE XWPENTRY winhQueryWPSClass(PBYTE pObjClass, const char *pszClass);

    APIRET XWPENTRY winhRegisterClass(const char* pcszClassName,
                                      const char* pcszModule,
                                      PSZ pszBuf,
                                      ULONG cbBuf);

    BOOL XWPENTRY winhIsClassRegistered(const char *pcszClass);

    extern BOOL32 APIENTRY WinRestartWorkplace(void);

    ULONG XWPENTRY winhResetWPS(HAB hab);

#endif

#if __cplusplus
}
#endif

