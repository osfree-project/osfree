
/*
 *@@sourcefile cctl_combo.c:
 *      super combo box control, which is, essentially, a
 *      subclassed entry field with a list box attached to
 *      it.
 *
 *      Compared to the standard PM combo box, this one has a
 *      number of advantages:
 *
 *      --  It doesn't require the parent window to have the
 *          WS_CLIPCHILDREN style bit cleared.
 *
 *      --  It has proper window positioning; the size of the
 *          enhanced combo is the size of the entry field, while
 *          the PM combo wants to have the size of the expanded
 *          list box too, which isn't easy to handle.
 *
 *      See ctlComboFromEntryField for details.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\comctl.h"
 *@@added V0.9.16 (2002-01-01) [umoeller]
 */

/*
 *      Copyright (C) 2001-2002 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WININPUT
#define INCL_WINPOINTERS

#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINLISTBOXES

#define INCL_GPILOGCOLORTABLE
#define INCL_GPIBITMAPS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

#include "setup.h"                      // code generation and debugging options

#include "helpers\except.h"             // exception handling
#include "helpers\winh.h"

#include "helpers\comctl.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Window classes\Super combo box
 *      See cctl_combo.c.
 */

/* ******************************************************************
 *
 *   Super Combination Box control
 *
 ********************************************************************/

#define COMBO_BUTTON_WIDTH      20

#define ID_COMBO_BUTTON         1001
#define ID_COMBO_LISTBOX        1002

/*
 *@@ COMBODATA:
 *
 *@@added V0.9.9 (2001-03-17) [umoeller]
 */

typedef struct _COMBODATA
{
    PFNWP       pfnwpOrigEntryField,
                pfnwpOrigButton;
    ULONG       flStyle;

    // position of entire combo
    LONG        x,
                y,
                cx,
                cy;

    HWND        hwndButton,
                hwndListbox;

    HBITMAP     hbmButton;
    SIZEL       szlButton;          // bitmap dimensions

} COMBODATA, *PCOMBODATA;

/*
 *@@ PaintButtonBitmap:
 *
 *@@added V0.9.9 (2001-03-17) [umoeller]
 */

STATIC VOID PaintButtonBitmap(HWND hwnd,
                              PCOMBODATA pcd)
{
    HPS hps;
    RECTL rcl;
    POINTL ptlDest;

    hps = WinGetPS(hwnd);
    WinQueryWindowRect(hwnd, &rcl);

    ptlDest.x = (rcl.xRight - pcd->szlButton.cx) / 2;
    ptlDest.y = (rcl.yTop - pcd->szlButton.cy) / 2;
    WinDrawBitmap(hps,
                  pcd->hbmButton,
                  NULL,
                  &ptlDest,
                  0, 0,
                  DBM_NORMAL);

    WinReleasePS(hps);
}

/*
 *@@ fnwpSubclassedComboButton:
 *      window proc the combobox's button is subclassed with.
 *      This is only for WM_PAINT because BN_PAINT is really
 *      not that great for painting a button that looks like
 *      a standard button.
 *
 *@@added V0.9.9 (2001-03-17) [umoeller]
 */

STATIC MRESULT EXPENTRY fnwpSubclassedComboButton(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    PCOMBODATA pcd;

    switch (msg)
    {
        case WM_PAINT:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                mrc = pcd->pfnwpOrigButton(hwnd, msg, mp1, mp2);

                PaintButtonBitmap(hwnd, pcd);
            }
        break;

        /*
         * default:
         *
         */

        default:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
                mrc = pcd->pfnwpOrigButton(hwnd, msg, mp1, mp2);
        break;
    }

    return mrc;
}

/*
 *@@ ShowListbox:
 *
 *@@added V0.9.9 (2001-03-17) [umoeller]
 */

STATIC VOID ShowListbox(HWND hwnd,      // in: subclassed entry field
                        PCOMBODATA pcd,
                        BOOL fShow)    // in: TRUE == show, FALSE == hide
{
    BOOL fHilite = FALSE;

    if (fShow)
    {
        // list box is invisible:
        SWP swp;
        POINTL ptl;
        WinQueryWindowPos(hwnd, &swp);

        _Pmpf(("showing lb"));

        // convert to desktop
        ptl.x = swp.x;
        ptl.y = swp.y;
        WinMapWindowPoints(WinQueryWindow(hwnd, QW_PARENT), // from
                           HWND_DESKTOP,    // to
                           &ptl,
                                // SWP.y comes before SWP.x
                           1);

        WinSetWindowPos(pcd->hwndListbox,
                        HWND_TOP,
                        ptl.x + COMBO_BUTTON_WIDTH,
                        ptl.y - 100,
                        swp.cx,
                        100,
                        SWP_MOVE | SWP_SIZE | SWP_ZORDER | SWP_NOREDRAW);
        WinSetParent(pcd->hwndListbox,
                     HWND_DESKTOP,
                     TRUE);        // redraw

        // set focus to subclassed entry field in any case;
        // we never let the listbox get the focus
        WinSetFocus(HWND_DESKTOP, hwnd);

        fHilite = TRUE;
    }
    else
    {
        // list box is showing:
        HWND hwndFocus = WinQueryFocus(HWND_DESKTOP);
        _Pmpf(("hiding listbox"));

        WinSetParent(pcd->hwndListbox,
                     HWND_OBJECT,
                     TRUE);         // redraw now
        // give focus back to entry field
        if (hwndFocus == pcd->hwndListbox)
            WinSetFocus(HWND_DESKTOP, hwnd);
    }

    WinSendMsg(pcd->hwndButton,
               BM_SETHILITE,
               (MPARAM)fHilite,
               0);
    PaintButtonBitmap(pcd->hwndButton, pcd);
}

/*
 *@@ fnwpComboSubclass:
 *
 *@@added V0.9.9 (2001-03-17) [umoeller]
 */

STATIC MRESULT EXPENTRY fnwpComboSubclass(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT     mrc = 0;
    PCOMBODATA  pcd;

    switch (msg)
    {
        /*
         * WM_ADJUSTWINDOWPOS:
         *
         */

        case WM_ADJUSTWINDOWPOS:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                PSWP pswp = (PSWP)mp1;

                if (pswp->fl & SWP_SIZE)
                    // if we're being sized, make us smaller so that
                    // there's room for the button
                    pswp->cx -= COMBO_BUTTON_WIDTH;

                mrc = pcd->pfnwpOrigEntryField(hwnd, msg, mp1, mp2);
            }
        break;

        /*
         * WM_WINDOWPOSCHANGED:
         *
         */

        case WM_WINDOWPOSCHANGED:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                PSWP pswpNew = (PSWP)mp1;

                if (pswpNew->fl & (SWP_SIZE | SWP_MOVE))
                {
                    // moved or sized:
                    SWP swp;
                    WinQueryWindowPos(hwnd, &swp);
                    WinSetWindowPos(pcd->hwndButton,
                                    0,
                                    pswpNew->x + pswpNew->cx, // has already been truncated!
                                    pswpNew->y,
                                    COMBO_BUTTON_WIDTH,
                                    pswpNew->cy,
                                    SWP_MOVE | SWP_SIZE);
                }

                mrc = pcd->pfnwpOrigEntryField(hwnd, msg, mp1, mp2);
            }
        break;

        /*
         * WM_SETFOCUS:
         *      hide listbox if focus is going away from us
         */

        case WM_SETFOCUS:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                if (!mp2)
                    // we're losing focus:
                    // is listbox currently showing?
                    ShowListbox(hwnd,
                                pcd,
                                FALSE);

                mrc = pcd->pfnwpOrigEntryField(hwnd, msg, mp1, mp2);
            }
        break;

        /*
         * WM_COMMAND:
         *      show/hide listbox if the button gets pressed.
         */

        case WM_COMMAND:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                if (SHORT1FROMMP(mp1) == ID_COMBO_BUTTON)
                {
                    // button clicked:
                    ShowListbox(hwnd,
                                pcd,
                                // check state of list box
                                (WinQueryWindow(pcd->hwndListbox, QW_PARENT)
                                 == WinQueryObjectWindow(HWND_DESKTOP)));

                    // do not call parent
                    break;

                } // end if ((SHORT)mp1 == ID_COMBO_BUTTON)

                mrc = pcd->pfnwpOrigEntryField(hwnd, msg, mp1, mp2);
            }
        break;

        /*
         * WM_CONTROL:
         *      handle notifications from listbox.
         */

        case WM_CONTROL:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                USHORT usid = SHORT1FROMMP(mp1),
                       uscode = SHORT2FROMMP(mp1);
                if (usid == ID_COMBO_LISTBOX)
                {
                    switch (uscode)
                    {
                        case LN_ENTER:
                        break;

                        case LN_SELECT:
                        {
                            SHORT sSelected = winhQueryLboxSelectedItem(pcd->hwndListbox,
                                                                        LIT_FIRST);
                            PSZ psz = NULL;
                            if (sSelected != LIT_NONE)
                            {
                                psz = winhQueryLboxItemText(pcd->hwndListbox,
                                                            sSelected);
                            }
                            WinSetWindowText(hwnd, psz);
                            if (psz)
                            {
                                WinPostMsg(hwnd,
                                           EM_SETSEL,
                                           MPFROM2SHORT(0, strlen(psz)),
                                           0);
                                free(psz);
                            }
                        break; }

                        case LN_SETFOCUS:
                            // when the list box gets the focus, always
                            // set focus to ourselves
                            WinSetFocus(HWND_DESKTOP, hwnd);
                        break;
                    }

                    // forward list box notifications to
                    // our own owner, but replace the id
                    // with the combo box id
                    ctlPostWmControl(hwnd,
                                     uscode,
                                     mp2);

                    // do not call parent
                    break;

                } // end if (usid == ID_COMBO_LISTBOX)

                mrc = pcd->pfnwpOrigEntryField(hwnd, msg, mp1, mp2);
            }
        break;

        /*
         * WM_CHAR:
         *
         */

        case WM_CHAR:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                USHORT usFlags    = SHORT1FROMMP(mp1);
                // USHORT usch       = SHORT1FROMMP(mp2);
                USHORT usvk       = SHORT2FROMMP(mp2);

                if ((usFlags & KC_KEYUP) == 0)
                {
                    if (usFlags & KC_VIRTUALKEY)
                    {
                        switch (usvk)
                        {
                            case VK_DOWN:
                            case VK_UP:
                                // if alt is pressed with these, show/hide listbox
                                if (usFlags & KC_ALT)
                                    WinPostMsg(hwnd,
                                               CBM_SHOWLIST,
                                               (MPARAM)(WinQueryWindow(pcd->hwndListbox, QW_PARENT)
                                                        == WinQueryObjectWindow(HWND_DESKTOP)),
                                               0);
                                else
                                {
                                    // just up or down, no alt:
                                    // select next or previous item in list box
                                    SHORT sSelected = winhQueryLboxSelectedItem(pcd->hwndListbox,
                                                                                LIT_FIRST),
                                          sNew = 0;

                                    if (usvk == VK_DOWN)
                                    {
                                        if (sSelected != LIT_NONE)
                                        {
                                            if (sSelected < WinQueryLboxCount(pcd->hwndListbox))
                                                sNew = sSelected + 1;
                                        }
                                        // else: sNew still 0
                                    }
                                    else
                                    {
                                        // up:
                                        if (    (sSelected != LIT_NONE)
                                             && (sSelected > 0)
                                           )
                                            sNew = sSelected - 1;
                                    }

                                    winhSetLboxSelectedItem(pcd->hwndListbox,
                                                            sNew,
                                                            TRUE);
                                }
                            break;
                        }
                    }
                }

                // call parent only if this is not a drop-down list
                if ((pcd->flStyle & CBS_DROPDOWNLIST) == 0)
                    mrc = pcd->pfnwpOrigEntryField(hwnd, msg, mp1, mp2);
                else
                    // forward to owner
                    WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),
                               msg,
                               mp1,
                               mp2);
            }
        break;

        /*
         * CBM_ISLISTSHOWING:
         *      implementation of the original combobox msg.
         */

        case CBM_ISLISTSHOWING:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                mrc = (MPARAM)(WinQueryWindow(pcd->hwndListbox, QW_PARENT)
                                     == WinQueryObjectWindow(HWND_DESKTOP));
            }
        break;

        /*
         * CBM_SHOWLIST:
         *      implementation of the original combobox msg.
         */

        case CBM_SHOWLIST:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                ShowListbox(hwnd,
                            pcd,
                            (BOOL)mp1);
            }
        break;

        /*
         * list box messages:
         *      forward all these to the listbox and
         *      return the listbox return value.
         */

        case LM_INSERTITEM:
        case LM_SETTOPINDEX:
        case LM_QUERYTOPINDEX:
        case LM_DELETEITEM:
        case LM_SELECTITEM:
        case LM_QUERYSELECTION:
        case LM_SETITEMTEXT:
        case LM_QUERYITEMTEXT:
        case LM_SEARCHSTRING:
        case LM_DELETEALL:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
                mrc = WinSendMsg(pcd->hwndListbox, msg, mp1, mp2);
        break;

        /*
         * WM_DESTROY:
         *
         */

        case WM_DESTROY:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                WinDestroyWindow(pcd->hwndButton);
                WinDestroyWindow(pcd->hwndListbox);

                mrc = pcd->pfnwpOrigEntryField(hwnd, msg, mp1, mp2);

                free(pcd);
            }
        break;

        /*
         * default:
         *
         */

        default:
            if (pcd = (PCOMBODATA)WinQueryWindowPtr(hwnd, QWL_USER))
                mrc = pcd->pfnwpOrigEntryField(hwnd, msg, mp1, mp2);
        break;
    }

    return mrc;
}

/*
 *@@ ctlComboFromEntryField:
 *      turns a standard entry field control into an
 *      XComboBox.
 *
 *      The XComboBox is intended to work like a standard
 *      combobox, but it doesn't have the silly limitation
 *      that the size of the combobox is assumed to be
 *      the size of the dropped-down combobox. This limitation
 *      makes it impossible to use standard comboboxes in
 *      windows which have the WS_CLIPCHILDREN style because
 *      the entire combo area will always be clipped out.
 *
 *      This is not a full reimplementation. Only drop-down
 *      and drop-down list comboboxes are supported. Besides,
 *      the XComboBox is essentially a subclassed entryfield,
 *      so there might be limitations.
 *
 *      On input to this function, with flStyle, specify
 *      either CBS_DROPDOWN or CBS_DROPDOWNLIST. CBS_SIMPLE
 *      is not supported.
 *
 *      Supported messages to the XComboBox after this funcion
 *      has been called:
 *
 *      -- CBM_ISLISTSHOWING
 *
 *      -- CBM_SHOWLIST
 *
 *      -- LM_QUERYITEMCOUNT
 *
 *      -- LM_INSERTITEM
 *
 *      -- LM_SETTOPINDEX
 *
 *      -- LM_QUERYTOPINDEX
 *
 *      -- LM_DELETEITEM
 *
 *      -- LM_SELECTITEM
 *
 *      -- LM_QUERYSELECTION
 *
 *      -- LM_SETITEMTEXT
 *
 *      -- LM_QUERYITEMTEXT
 *
 *      -- LM_SEARCHSTRING
 *
 *      -- LM_DELETEALL
 *
 *      NOTE: This occupies QWL_USER of the entryfield.
 *
 *@@added V0.9.9 (2001-03-17) [umoeller]
 */

BOOL ctlComboFromEntryField(HWND hwnd,          // in: entry field to be converted
                            ULONG flStyle)      // in: combo box styles
{
    BOOL brc = FALSE;
    PFNWP pfnwpOrig;
    if (pfnwpOrig = WinSubclassWindow(hwnd,
                                      fnwpComboSubclass))
    {
        PCOMBODATA pcd;
        if (pcd = (PCOMBODATA)malloc(sizeof(*pcd)))
        {
            SWP swp;
            BITMAPINFOHEADER2 bmih2;

            memset(pcd, 0, sizeof(*pcd));
            pcd->pfnwpOrigEntryField = pfnwpOrig;
            pcd->flStyle = flStyle;

            WinSetWindowPtr(hwnd, QWL_USER, pcd);

            WinQueryWindowPos(hwnd, &swp);
            pcd->x = swp.x;
            pcd->y = swp.y;
            pcd->cx = swp.cx;
            pcd->cy = swp.cy;

            swp.cx -= COMBO_BUTTON_WIDTH;
            WinSetWindowPos(hwnd,
                            0,
                            0, 0,
                            swp.cx, swp.cy,
                            SWP_SIZE | SWP_NOADJUST);       // circumvent subclassing

            pcd->hbmButton = WinGetSysBitmap(HWND_DESKTOP,
                                             SBMP_COMBODOWN);
            bmih2.cbFix = sizeof(bmih2);
            GpiQueryBitmapInfoHeader(pcd->hbmButton,
                                     &bmih2);
            pcd->szlButton.cx = bmih2.cx;
            pcd->szlButton.cy = bmih2.cy;

            pcd->hwndButton = WinCreateWindow(WinQueryWindow(hwnd, QW_PARENT),
                                              WC_BUTTON,
                                              "",
                                              WS_VISIBLE
                                                | BS_PUSHBUTTON | BS_NOPOINTERFOCUS,
                                              swp.x + swp.cx - COMBO_BUTTON_WIDTH,
                                              swp.y,
                                              COMBO_BUTTON_WIDTH,
                                              swp.cy,
                                              hwnd,     // owner == entry field!
                                              hwnd,     // insert behind entry field
                                              ID_COMBO_BUTTON,
                                              NULL,
                                              NULL);
            WinSetWindowPtr(pcd->hwndButton, QWL_USER, pcd);
            pcd->pfnwpOrigButton = WinSubclassWindow(pcd->hwndButton,
                                                     fnwpSubclassedComboButton);

            pcd->hwndListbox = WinCreateWindow(HWND_OBJECT,      // parent, for now
                                               WC_LISTBOX,
                                               "?",
                                               WS_VISIBLE | WS_SAVEBITS | WS_CLIPSIBLINGS
                                                 | LS_NOADJUSTPOS,
                                               0,
                                               0,
                                               0,
                                               0,
                                               hwnd,     // owner == entry field!
                                               HWND_TOP,     // insert behind entry field
                                               ID_COMBO_LISTBOX,
                                               NULL,
                                               NULL);

            // finally, set style of entry field... we force
            // these flags no matter what the original style
            // was
            /* WinSetWindowBits(hwnd,
                             QWL_STYLE,
                             // bits to set:
                            (flStyle & CBS_DROPDOWNLIST)
                                ? ES_READONLY
                                : 0,
                             // mask:
                             ES_READONLY); */
        }
    }

    return brc;
}


