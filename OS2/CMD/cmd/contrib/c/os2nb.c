// OS2NB.C - Tab dialog support routines
// Copyright 1995, JP Software Inc., All Rights Reserved

// Originally based on NBBASE.C from Rick Fishman / CodeBlazers
// (CIS 72251,750).  Used with permission.

#include "product.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "4all.h"

static BOOL TurnToFirstPage(HWND);
static void SetNotebookSize(PNBHDR);
static BOOL GetDialogDimensions(ULONG, PLONG, PLONG, PULONG);
static BOOL CreateNotebook(PNBHDR);
static BOOL SetUpPage(PNBHDR, INT);
static BOOL SetTabDimensions(PNBHDR);
static void SetNBPage(HWND, PPAGESELECTNOTIFY);

#define HWNDERR( hwnd ) (ERRORIDERROR( WinGetLastError( WinQueryAnchorBlock( hwnd ) ) ))


// Top-level notebook dialog procedure -- for WM_INITDLG, expects mp2 to be a
// pointer to a notebook header structure which defines the notebook
MRESULT EXPENTRY NotebookDlgProc(HWND hDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    int nPage;
    PNBHDR pNotebookHdr;

    if (msg != WM_INITDLG)
        pNotebookHdr = (PNBHDR)WinQueryWindowULong(hDlg, QWL_USER);

    switch(msg) {
    case WM_INITDLG:

        pNotebookHdr = (PNBHDR)(PVOIDFROMMP(mp2));
        pNotebookHdr->hwndNB = WinWindowFromID(hDlg, pNotebookHdr->ulNotebookID);

        // Save upper-level dialog handle for use by page dialog code
        pNotebookHdr->hwndNBDlg = hDlg;

        // Save header pointer in window data so WM_SIZE code can use it
        WinSetWindowULong(hDlg, QWL_USER, (ULONG)pNotebookHdr);

        // Create the notebook
        if (!CreateNotebook(pNotebookHdr)) {
            WinDismissDlg(hDlg, 0);
            return 0;
        }

        // Post a message to open the notebook after initialization is complete
        WinPostMsg(hDlg, WM_OPEN_NOTEBOOK, MP0, MP0);
        break;

    case WM_OPEN_NOTEBOOK:

        // Select the first page, and set the notebook size
        TurnToFirstPage(pNotebookHdr->hwndNB);
        SetNotebookSize(pNotebookHdr);
        break;

    case WM_CONTROL:
        // If user selected a new page, switch to it
        if (((ULONG)pNotebookHdr != 0L) && (SHORT1FROMMP(mp1) == pNotebookHdr->ulNotebookID) && (SHORT2FROMMP(mp1) == BKN_PAGESELECTED))
            SetNBPage(hDlg, (PPAGESELECTNOTIFY)mp2);
        break;

    case WM_CLOSE_NOTEBOOK:
        // clear the notebook page array dialog handles to show the pages are no longer in use
        for (nPage = 0; (nPage < pNotebookHdr->nPages); nPage++)
            pNotebookHdr->PageArray[nPage].hwndPageDlg = (HWND)0L;
        WinDismissDlg(hDlg, 0);
        break;

    default:
        return WinDefDlgProc(hDlg, msg, mp1, mp2);
    }

    return 0;

}


// TURN TO THE FIRST PAGE IN THE NOTEBOOK
BOOL TurnToFirstPage(HWND hwndNB)
{
    ULONG ulFirstPage;

    ulFirstPage = (ULONG) WinSendMsg(hwndNB, BKM_QUERYPAGEID, NULL, MPFROM2SHORT(BKA_FIRST, BKA_MAJOR));

    return (ulFirstPage ? ((BOOL) WinSendMsg(hwndNB, BKM_TURNTOPAGE, MPFROMLONG(ulFirstPage), NULL)) : FALSE);
}


// Set the notebook and top-level dialog sizes based on the page sizes
void SetNotebookSize(PNBHDR pNotebookHdr)
{
    int i;
    RECTL rcl;
    SWP swp;
    HWND hwndNB = pNotebookHdr->hwndNB;
    LONG cx, cy, px, py;

    (void) memset(&rcl, 0, sizeof(RECTL));

    // Get the size of a notebook page by finding the dimensions of the largest
    // dialog.

    for (i = 0; i < pNotebookHdr->nPages; i++) {

        // Get the height and width of the dialog. Also fill in the id of the
        // item that is to get focus when the dialog is brought up.
        if (pNotebookHdr->PageArray[i].idDlg) {
            GetDialogDimensions(pNotebookHdr->PageArray[i].idDlg, &cx, &cy, &(pNotebookHdr->PageArray[i].idFocus));
            if (cx > rcl.xRight)
                rcl.xRight = cx;
            if (cy > rcl.yTop)
                rcl.yTop = cy;
        }
    }

    // Adjust the notebook so it is big enough for the pages
    WinMapDlgPoints(HWND_DESKTOP, (PPOINTL) &rcl, 2, TRUE);
    WinSendMsg(hwndNB, BKM_CALCPAGERECT, MPFROMP(&rcl), MPFROMLONG(FALSE));
    WinSetWindowPos(hwndNB, NULLHANDLE, 0, 0, rcl.xRight-rcl.xLeft, rcl.yTop-rcl.yBottom, SWP_SIZE);

    // Calculate the size required for the dialog window
    WinQueryWindowPos(hwndNB, &swp);
    rcl.xLeft = 0;
    rcl.yBottom = 0;
    rcl.xRight = swp.x + swp.cx;
    rcl.yTop = swp.y + swp.cy;
    WinCalcFrameRect(pNotebookHdr->hwndNBDlg, &rcl, FALSE );

    // Find the centered position and size, and adjust the dialog window so
    // it is centered, and big enough for the notebook
    cx = rcl.xRight - rcl.xLeft;
    cy = rcl.yTop - rcl.yBottom;
    px = (WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN) - cx) / 2;
    py = (WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN) - cy) / 2;
    WinSetWindowPos(pNotebookHdr->hwndNBDlg, NULLHANDLE, px, py, cx, cy, SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_ACTIVATE);
}


//  RETURN THE WIDTH AND HEIGHT OF A DIALOG BOX.
BOOL GetDialogDimensions(ULONG idDlg, PLONG plCx, PLONG plCy, PULONG pidFocus)
{
    APIRET rc;
    PDLGTEMPLATE pDlgTemplate = NULL;

    rc = DosGetResource(0, RT_DIALOG, idDlg, (PPVOID) &pDlgTemplate);

    if (!rc) {
        PDLGTITEM pDlgItem;

        // Get offset to the item table
        pDlgItem = (PDLGTITEM)((PBYTE) pDlgTemplate + pDlgTemplate->offadlgti);

        *plCx = (LONG)pDlgItem->cx;
        *plCy = (LONG)pDlgItem->cy;

        // Currently the focus item doesn't seem to be set properly so it will
        // be hard-coded in the constant array.
        //*pidFocus = (ULONG) (pDlgItem + pDlgTemplate->iItemFocus)->id;
        *pidFocus = *pidFocus;
        return TRUE;
    }

    return FALSE;
}


//  CREATE THE NOTEBOOK WINDOW
BOOL CreateNotebook(PNBHDR pNotebookHdr)
{
    int i;

    // Set the page background and tab background colors to grey so they are the same as a dlg box.
    WinSendMsg(pNotebookHdr->hwndNB, BKM_SETNOTEBOOKCOLORS, MPFROMLONG(SYSCLR_FIELDBACKGROUND), MPFROMSHORT(BKA_BACKGROUNDPAGECOLORINDEX));
    WinSendMsg(pNotebookHdr->hwndNB, BKM_SETNOTEBOOKCOLORS, MPFROMLONG(SYSCLR_FIELDBACKGROUND), MPFROMSHORT(BKA_BACKGROUNDMAJORCOLORINDEX));
    WinSendMsg(pNotebookHdr->hwndNB, BKM_SETNOTEBOOKCOLORS, MPFROMLONG(SYSCLR_FIELDBACKGROUND), MPFROMSHORT(BKA_BACKGROUNDMINORCOLORINDEX));

    if (!SetTabDimensions(pNotebookHdr))
        return FALSE;

    // Insert all the pages into the notebook and configure them. The dialog
    // boxes are not going to be loaded and associated with those pages yet.
    for ( i = 0; ( i < pNotebookHdr->nPages ); i++ ) {
        if ( !SetUpPage( pNotebookHdr, i ))
            return FALSE;
    }

    return TRUE;
}


//  SET UP A NOTEBOOK PAGE.
BOOL SetUpPage(PNBHDR pNotebookHdr, INT iPage)
{
    ULONG ulPageId;
    HWND hwndNB = pNotebookHdr->hwndNB;

    // Insert a page into the notebook. Specify that it is to have status text
    // and the window associated with each page will be automatically sized by
    // the notebook according to the size of the page.

    if (!(ulPageId = (ULONG) WinSendMsg(hwndNB, BKM_INSERTPAGE, NULL, MPFROM2SHORT(pNotebookHdr->PageArray[iPage].usTabType | BKA_STATUSTEXTON | BKA_AUTOPAGESIZE, BKA_LAST))))
        return FALSE;
        
    // Insert a pointer to this page's info into the space available
    // in each page (its PAGE DATA that is available to the application).
    if (!WinSendMsg(hwndNB, BKM_SETPAGEDATA, MPFROMLONG(ulPageId), MPFROMP(&(pNotebookHdr->PageArray[iPage]))))
        return FALSE;

    // Set the text into the status line.
    if (!WinSendMsg(hwndNB, BKM_SETSTATUSLINETEXT, MPFROMP(ulPageId), MPFROMP(pNotebookHdr->PageArray[iPage].pszStatusLineText)))
        return FALSE;

    // Set the text into the tab for this page.
    if (!WinSendMsg(hwndNB, BKM_SETTABTEXT, MPFROMLONG(ulPageId), MPFROMP(pNotebookHdr->PageArray[iPage].pszTabText)))
        return FALSE;

    return TRUE;
}


//  SET THE DIMENSIONS OF THE NOTEBOOK TABS.
BOOL SetTabDimensions(PNBHDR pNotebookHdr)
{
    HWND hwndNB = pNotebookHdr->hwndNB;
    HPS hps = WinGetPS(hwndNB);
    FONTMETRICS fm;
    POINTL aptl[TXTBOX_COUNT];
    INT i, iSize, iLongestMajText = 0, iLongestMinText = 0;

    if (!hps)
        return FALSE;

    memset(&fm, 0, sizeof(FONTMETRICS));

    // Calculate the height of a tab as the height of an average font character
    // plus a margin value.
    if (GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fm))
        fm.lMaxBaselineExt += (TAB_HEIGHT_MARGIN * 2);
    else
        fm.lMaxBaselineExt = DEFAULT_NB_TAB_HEIGHT + (TAB_HEIGHT_MARGIN * 2);

    // Calculate the longest tab text for both the MAJOR and MINOR pages
    for (i = 0; i < pNotebookHdr->nPages; i++) {
    
        if (!GpiQueryTextBox(hps, strlen(pNotebookHdr->PageArray[i].pszTabText), pNotebookHdr->PageArray[i].pszTabText, TXTBOX_COUNT, aptl))
            iSize = 0;
        else
            iSize = aptl[TXTBOX_CONCAT].x;

        if (pNotebookHdr->PageArray[i].usTabType == BKA_MAJOR) {
            if (iSize > iLongestMajText)
                iLongestMajText = iSize;
        } else {
            if (iSize > iLongestMinText)
                iLongestMinText = iSize;
        }
    }

    WinReleasePS(hps);

    // Add a margin amount to the longest tab text

    if (iLongestMajText)
        iLongestMajText += TAB_WIDTH_MARGIN;

    if (iLongestMinText)
        iLongestMinText += TAB_WIDTH_MARGIN;

    // Set the tab dimensions for the MAJOR and MINOR pages. Note that the
    // docs as of this writing say to use BKA_MAJOR and BKA_MINOR in mp2 but
    // you really need BKA_MAJORTAB and BKA_MINORTAB.

    if (iLongestMajText) {
        if (!WinSendMsg(hwndNB, BKM_SETDIMENSIONS, MPFROM2SHORT(iLongestMajText, (SHORT)fm.lMaxBaselineExt), MPFROMSHORT(BKA_MAJORTAB)))
            return FALSE;
    }

    // If no minor tab text, set dimensions to 0
    if (!WinSendMsg(hwndNB, BKM_SETDIMENSIONS, (iLongestMinText ? MPFROM2SHORT(iLongestMinText, (SHORT)fm.lMaxBaselineExt) : MP0), MPFROMSHORT(BKA_MINORTAB)))
        return FALSE;

    return TRUE;
}


//  SET THE PAGE IN THE NOTEBOOK CONTROL.
VOID SetNBPage(HWND hDlg, PPAGESELECTNOTIFY ppsn)
{
    PNBPAGE pnbp;
    HWND hPageDlg;

    // Get a pointer to the page information that is associated with this page.
    // It was stored in the page's PAGE DATA in the SetUpPage function.
    pnbp = (PNBPAGE) WinSendMsg(ppsn->hwndBook, BKM_QUERYPAGEDATA, MPFROMLONG(ppsn->ulPageIdNew), NULL);

    if (!pnbp)
        return;

    // If this is a BKA_MAJOR page and it is what this app terms a 'parent'
    // page, that means when the user selects this page we actually want to go
    // to its first MINOR page. So in effect the MAJOR page is just a dummy page
    // that has a tab that acts as a placeholder for its MINOR pages. If the
    // user is using the left arrow to scroll thru the pages and they hit this
    // dummy MAJOR page, that means they have already been to its MINOR pages in
    // reverse order. They would now expect to see the page before the dummy
    // MAJOR page, so we skip the dummy page. Otherwise the user is going the
    // other way and wants to see the first MINOR page associated with this
    // 'parent' page so we skip the dummy page and show its first MINOR page.

    if (pnbp->fParent) {

        ULONG ulPageFwd, ulPageNew;

        ulPageFwd = (ULONG) WinSendMsg(ppsn->hwndBook, BKM_QUERYPAGEID, MPFROMLONG(ppsn->ulPageIdNew), MPFROM2SHORT(BKA_NEXT, BKA_MINOR));

        // If this is true, the user is going in reverse order
        if (ulPageFwd == ppsn->ulPageIdCur)
            ulPageNew = (ULONG) WinSendMsg(ppsn->hwndBook, BKM_QUERYPAGEID, MPFROMLONG(ppsn->ulPageIdNew), MPFROM2SHORT(BKA_PREV, BKA_MAJOR));
        else
            ulPageNew = ulPageFwd;

        if (ulPageNew)
            WinSendMsg(ppsn->hwndBook, BKM_TURNTOPAGE, MPFROMLONG(ulPageNew), NULL);

    } else {

        hPageDlg = (HWND) WinSendMsg(ppsn->hwndBook, BKM_QUERYPAGEWINDOWHWND, MPFROMLONG(ppsn->ulPageIdNew), NULL);

        if (!hPageDlg) {

            // It is time to load this dialog because the user has flipped pages
            // to a page that hasn't yet had the dialog associated with it.  We pass
            // a pointer to the notebook handle so the dialog procedure can use it
            // if necessary.
            hPageDlg = WinLoadDlg(hDlg, hDlg, pnbp->pfnwpDlg, 0, pnbp->idDlg, &(ppsn->hwndBook));

            if (hPageDlg) {

                // Associate the dialog with the page.
                if (!WinSendMsg(ppsn->hwndBook, BKM_SETPAGEWINDOWHWND, MPFROMP(ppsn->ulPageIdNew), MPFROMLONG(hPageDlg))) {
                    WinDestroyWindow(hPageDlg);
                    hPageDlg = NULLHANDLE;
                }

                // Save the dialog window handle in case the dialog procedure needs it
                pnbp->hwndPageDlg = hPageDlg;
        
            }
        }
    }

    // Set focus to the first control in the dialog. This is not automatically
    // done by the notebook.

    if (hPageDlg && !pnbp->fParent)
        WinSetFocus(HWND_DESKTOP, WinWindowFromID(hPageDlg, pnbp->idFocus));
}

