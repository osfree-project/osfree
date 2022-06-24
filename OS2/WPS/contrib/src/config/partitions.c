
/*
 *@@sourcefile partitions.c:
 *      implementation of the WPDrives "Partitions" view.
 *
 *      This file is ALL new with V0.9.2.
 *
 *      Function prefix for this file:
 *      --  part*
 *
 *@@header "config\partitions.h"
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
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

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINPOINTERS
#define INCL_WINTIMER
#define INCL_WINMENUS
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINSYS
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines
#include "helpers\threads.h"            // thread helpers

// SOM headers which don't crash with prec. header files
#include "xclslist.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#include "config\partitions.h"          // WPDrives "Partitions" view

#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Declarations
 *
 ********************************************************************/

// client window class name
#define WC_PARTITIONSCLIENT      "XWPPartitionsWindow"

/*
 *@@ CLIENTCTLDATA:
 *      client control data passed with WM_CREATE
 *      to fnwpPartitionsClient.
 */

typedef struct _CLIENTCTLDATA
{
    USHORT          cb;
    WPObject        *somSelf;
    ULONG           ulView;
} CLIENTCTLDATA, *PCLIENTCTLDATA;

/*
 *@@ PARTITIONCLIENTDATA:
 *      window data stored in QWL_USER of
 *      fnwpPartitionsClient.
 */

typedef struct _PARTITIONCLIENTDATA
{
    WPObject        *somSelf;           // pointer to WPDrives instance
    USEITEM         UseItem;            // use item
    VIEWITEM        ViewItem;           // view item

    USHORT          usDiskCount;        // from doshQueryDiskCount
    /* PPARTITIONINFO  ppi;                // from doshGetPartitionsList
    USHORT          usPartitionCount;   // from doshGetPartitionsList
     */

    PPARTITIONSLIST pPartitionsList;

    HWND            hwndDiskChart[16];  // for each disk, a pie chart control
} PARTITIONCLIENTDATA, *PPARTITIONCLIENTDATA;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

/*
 *@@ UpdateChartControlData:
 *
 *@@added V0.9.2 (2000-03-01) [umoeller]
 */

VOID UpdateChartControlData(HWND hwndClient,        // in: partition client window
                            PPARTITIONCLIENTDATA pClientData) // in: client window data
{
    ULONG   ulDiskThis = 0;
    USHORT  usContext = 0;
    APIRET  arc = doshGetPartitionsList(&pClientData->pPartitionsList,
                                        &usContext);
    if (    (pClientData->pPartitionsList)
         && (pClientData->pPartitionsList->cPartitions)
         && (arc == NO_ERROR)
       )
    {
        // array of colors which we'll use
        // in round-robin fashion
        static LONG alRGBColors[] =
            {
                    RGBCOL_RED,
                    RGBCOL_PINK,
                    RGBCOL_BLUE,
                    RGBCOL_CYAN,
                    RGBCOL_GREEN,
                    RGBCOL_YELLOW,
                    RGBCOL_GRAY,
                    RGBCOL_DARKRED,
                    RGBCOL_DARKPINK,
                    RGBCOL_DARKBLUE,
                    RGBCOL_DARKCYAN,
                    RGBCOL_DARKGREEN,
                    RGBCOL_DARKYELLOW,
                    RGBCOL_DARKGRAY
            };
        ULONG   cRGBColors = sizeof(alRGBColors) / sizeof(LONG),
                ulCurrentColor = 0;

        // go thru all disks
        for (ulDiskThis = 0;
             ulDiskThis < pClientData->usDiskCount;
             ulDiskThis++)
        {
            // count partitions on current disk
            ULONG           cPartitionsThisDisk = 0;
            PPARTITIONINFO  ppiThis = pClientData->pPartitionsList->pPartitionInfo;
            while (ppiThis)
            {
                if (ppiThis->bDisk == ulDiskThis + 1)
                    // partition on our disk:
                    cPartitionsThisDisk++;

                ppiThis = ppiThis->pNext;
            }

            // any partitions found:
            if (cPartitionsThisDisk)
            {
                CHARTSTYLE      cs;
                CHARTDATA       cd;

                // define data:
                double          *padData = malloc(sizeof(double) * cPartitionsThisDisk),
                                *pdDataThis = padData;
                // define corresponding colors:
                LONG            *palColors = malloc(sizeof(LONG) * cPartitionsThisDisk),
                                *plColorThis = palColors;
                // define correspdonding descriptions:
                PSZ             *papszDescriptions = malloc(sizeof(PSZ) * cPartitionsThisDisk),
                                *ppszDescriptionThis = papszDescriptions;

                // go thru partitions again and fill data
                ppiThis = pClientData->pPartitionsList->pPartitionInfo;
                while (ppiThis)
                {
                    if (ppiThis->bDisk == ulDiskThis + 1)
                    {
                        CHAR    szTemp[100];
                        // partition on our disk:
                        *pdDataThis = (double)ppiThis->ulSize;
                        *plColorThis = alRGBColors[ulCurrentColor];
                        sprintf(szTemp, "%c:", ppiThis->cLetter);
                        *ppszDescriptionThis = strdup(szTemp);
                        pdDataThis++;
                        plColorThis++;
                        ppszDescriptionThis++;

                        ulCurrentColor++;
                        if (ulCurrentColor >= cRGBColors)
                            // restart with first color
                            ulCurrentColor = 0;
                    }

                    ppiThis = ppiThis->pNext;
                }

                cs.ulStyle = CHS_3D_DARKEN | CHS_DESCRIPTIONS_3D | CHS_SELECTIONS;
                cs.ulThickness = 20;
                cs.dPieSize = .5;
                cs.dDescriptions = 1;
                WinSendMsg(pClientData->hwndDiskChart[ulDiskThis],
                           CHTM_SETCHARTSTYLE,
                           &cs, NULL);

                cd.usStartAngle = 0;       // start at 15ø from right
                cd.usSweepAngle = 270;      // three-quarter pie (for the sum of the
                                            // above values: 100+200+300 = 600)
                cd.cValues = cPartitionsThisDisk;  // array count
                cd.padValues = padData;
                cd.palColors = palColors;
                cd.papszDescriptions = papszDescriptions;
                WinSendMsg(pClientData->hwndDiskChart[ulDiskThis],
                           CHTM_SETCHARTDATA,
                           &cd, NULL);

                free(padData);
                free(palColors);
                free(papszDescriptions);
            } // end if (cPartitionsThisDisk)
        }
    }
}

/*
 *@@ RearrangeCharts:
 *      sets the window positions of the chart
 *      controls according to the client window's
 *      width and height.
 *
 *@@added V0.9.2 (2000-02-29) [umoeller]
 */

VOID RearrangeCharts(HWND hwndClient,           // in: partition client window
                     PPARTITIONCLIENTDATA pClientData, // in: client window data
                     LONG cx,                   // in: client window width
                     LONG cy)                   // in: client window height
{
    ULONG   cxUsable,
            cyUsable,
            cxPerChart,
            ulDisk;
    ULONG   ulRows = (pClientData->usDiskCount + 1) / 2,
            ulRowThis;
    if (ulRows == 0)
        ulRows = 1;

    #define CHART_BORDER 10

    cxUsable = cx - 200;
    cyUsable = (cy / ulRows);

    cxPerChart = (cxUsable / 2);

    ulRowThis = ulRows - 1; // start with top

    // go thru all disks
    for (ulDisk = 0;
         ulDisk < pClientData->usDiskCount;
         ulDisk++)
    {
         ULONG ulColumnThis = (ulDisk % 2);
         _Pmpf(("ulDisk %d ulRow %d ulColumn %d",
                 ulDisk, ulRowThis, ulColumnThis));
         WinSetWindowPos(pClientData->hwndDiskChart[ulDisk],
                         NULLHANDLE,
                         ulColumnThis * cxPerChart + CHART_BORDER,
                         (ulRowThis * cyUsable) + CHART_BORDER,
                         cxPerChart - 2 * CHART_BORDER,
                         cyUsable - 2 * CHART_BORDER,
                         SWP_MOVE | SWP_SIZE);
        if (ulColumnThis == 1)
            ulRowThis--;
    }
}

/*
 *@@ fnwpPartitionsClient:
 *      window procedure for the partition window
 *      client window. This is created with the
 *      standard frame from partCreatePartitionsView.
 *
 *      For each drive, we'll create a pie chart
 *      control (ctl_fnwpChart, comctl.c) as a
 *      child of the client.
 *
 *@@added V0.9.2 (2000-02-29) [umoeller]
 *@@changed V0.9.5 (2000-09-20) [pr]: fixed accelerator char
 */

MRESULT EXPENTRY fnwpPartitionsClient(HWND hwndClient, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    PPARTITIONCLIENTDATA pClientData
            = (PPARTITIONCLIENTDATA)WinQueryWindowPtr(hwndClient, QWL_USER);

    switch (msg)
    {
        /*
         * WM_CREATE:
         *
         */

        case WM_CREATE:
        {
            // frame window successfully created:
            // PNLSSTRINGS     pNLSStrings = cmnQueryNLSStrings();
            PCLIENTCTLDATA  pCData = (PCLIENTCTLDATA)mp1;
            PCREATESTRUCT   pCreateStruct = (PCREATESTRUCT)mp2;
            HWND            hwndFrame = WinQueryWindow(hwndClient, QW_PARENT);
            HAB             hab = WinQueryAnchorBlock(hwndClient);
            ULONG           ul;
            PPRESPARAMS     ppp = NULL;
            CHAR            szFont[] = "9.WarpSans Bold";
            LONG            lBackColor = CLR_WHITE;

            // now add the view to the object's use list;
            // this use list is used by wpViewObject and
            // wpClose to check for existing open views.
            // get storage for and initialize a use list item
            pClientData = (PPARTITIONCLIENTDATA)_wpAllocMem(pCData->somSelf,
                                                            sizeof(PARTITIONCLIENTDATA),
                                                            NULL);
            memset((PVOID)pClientData, 0, sizeof(PARTITIONCLIENTDATA));

            // save the pointer to the use item in the window words so that
            // the window procedure can remove it from the list when the window
            // is closed
            WinSetWindowPtr(hwndClient, QWL_USER, pClientData);

            cmnRegisterView(pCData->somSelf,
                            &pClientData->UseItem,
                            pCData->ulView,
                            hwndFrame,
                            cmnGetString(ID_XSSI_OPENPARTITIONS));

            // get disk count
            pClientData->usDiskCount = doshQueryDiskCount();

            winhStorePresParam(&ppp, PP_FONTNAMESIZE, sizeof(szFont), szFont);
            winhStorePresParam(&ppp, PP_BACKGROUNDCOLOR, sizeof(lBackColor), &lBackColor);

            // create one pie chart for each physical disk
            for (ul = 0;
                 ul < pClientData->usDiskCount;
                 ul++)
            {
                // create a static control
                pClientData->hwndDiskChart[ul]
                    = WinCreateWindow(hwndClient, // parent
                                      WC_STATIC,
                                      "",
                                      WS_VISIBLE | WS_TABSTOP | SS_TEXT,
                                      ul * 100,   // x
                                      10,         // y
                                      90,         // cx
                                      90,         // cy
                                      hwndClient, // owner
                                      HWND_TOP,
                                      ul,
                                      NULL,
                                      ppp);       // preparams
                // and make it a pie chart
                ctlChartFromStatic(pClientData->hwndDiskChart[ul]);
            }

            free(ppp);

            // have client updated
            WinPostMsg(hwndClient,
                       XM_UPDATE,
                       0, 0);
            mrc = (MPARAM)FALSE;
        }
        break;

        /*
         * XM_UPDATE:
         *      posted only once by WM_CREATE. This
         *      sets the data for the chart controls
         *      according to the drives on the system.
         */

        case XM_UPDATE:
            UpdateChartControlData(hwndClient,
                                   pClientData);
        break;

        case WM_ERASEBACKGROUND:
            // don't have frame control erase the client window
            mrc = (MPARAM)FALSE;
        break;

        /*
         * WM_PAINT:
         *      paint background only; the chart controls
         *      paint themselves
         */

        case WM_PAINT:
        {
            RECTL rclPaint;
            HPS hps = WinBeginPaint(hwndClient,
                                    NULLHANDLE,
                                    &rclPaint);
            WinFillRect(hps, &rclPaint, CLR_BLACK);
            WinEndPaint(hps);
        }
        break;

        /*
         * WM_WINDOWPOSCHANGED:
         *      if resized, update the chart's positions
         *      also.
         */

        case WM_WINDOWPOSCHANGED:
        {
            // this msg is passed two SWP structs:
            // one for the old, one for the new data
            // (from PM docs)
            PSWP pswpNew = PVOIDFROMMP(mp1);
            // PSWP pswpOld = pswpNew + 1;

            // resizing?
            if (pswpNew->fl & SWP_SIZE)
            {
                if (pClientData)
                    RearrangeCharts(hwndClient,
                                    pClientData,
                                    pswpNew->cx,
                                    pswpNew->cy);
            }

            // return default NULL
        }
        break;

        /*
         * WM_CLOSE:
         *      window is being closed:
         *      store the window position
         */

        case WM_CLOSE:
        {
            HWND hwndFrame = WinQueryWindow(hwndClient, QW_PARENT);

            // save window position
            winhSaveWindowPos(hwndFrame,
                              HINI_USER,
                              INIAPP_XWORKPLACE,
                              INIKEY_WNDPOSPARTITIONS);
            // destroy the window and return
            WinDestroyWindow(hwndFrame);

            // return default NULL
        }
        break;

        /*
         * WM_DESTROY:
         *      clean up.
         */

        case WM_DESTROY:
            if (pClientData)
            {
                // remove this window from the object's use list
                _wpDeleteFromObjUseList(pClientData->somSelf,
                                        &pClientData->UseItem);

                // free the partition info
                doshFreePartitionsList(pClientData->pPartitionsList);

                // free the use list item
                _wpFreeMem(pClientData->somSelf, (PBYTE)pClientData);
            }

            // return default NULL
        break;

        default:
            mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ partCreatePartitionsView:
 *
 *@@added V0.9.2 (2000-02-29) [umoeller]
 */

HWND partCreatePartitionsView(WPObject *somSelf,     // of WPDrives, really
                              ULONG ulView)
{
    HWND            hwndFrame = 0;

    TRY_LOUD(excpt1)
    {
        HAB         hab = WinQueryAnchorBlock(HWND_DESKTOP);
        HWND        hwndClient;
        ULONG       flCreate;                      // Window creation flags
        SWP         swpFrame;
        CLIENTCTLDATA ClientCData;

        // register the frame class, adding a user word to the window data to
        // anchor the object use list item for this open view
        WinRegisterClass(hab,
                         WC_PARTITIONSCLIENT,
                         fnwpPartitionsClient,
                         CS_SIZEREDRAW | CS_CLIPCHILDREN,
                         sizeof(PVOID)); // additional bytes to reserve

        // create the frame window
        flCreate = FCF_SYSMENU
                    | FCF_SIZEBORDER
                    | FCF_TITLEBAR
                    | FCF_MINMAX
                    // | FCF_TASKLIST
                    | FCF_NOBYTEALIGN;

        swpFrame.x = 100;
        swpFrame.y = 100;
        swpFrame.cx = 500;
        swpFrame.cy = 500;
        swpFrame.hwndInsertBehind = HWND_TOP;
        swpFrame.fl = SWP_MOVE | SWP_SIZE;

        memset(&ClientCData, 0, sizeof(ClientCData));
        ClientCData.cb = sizeof(ClientCData);
        ClientCData.somSelf = somSelf;
        ClientCData.ulView = ulView;
        hwndFrame = winhCreateStdWindow(HWND_DESKTOP,           // frame parent
                                        &swpFrame,
                                        flCreate,
                                        WS_ANIMATE,
                                        _wpQueryTitle(somSelf), // title bar
                                        0,                      // res IDs
                                        WC_PARTITIONSCLIENT,     // client class
                                        0L,                     // client wnd style
                                        2000,          // ID
                                        &ClientCData,
                                        &hwndClient);

        if (hwndFrame)
        {
            // now position the frame and the client:
            // 1) frame
            if (!winhRestoreWindowPos(hwndFrame,
                                      HINI_USER,
                                      INIAPP_XWORKPLACE,
                                      INIKEY_WNDPOSPARTITIONS,
                                      SWP_MOVE | SWP_SIZE))
                // INI data not found:
                WinSetWindowPos(hwndFrame,
                                HWND_TOP,
                                100, 100,
                                500, 500,
                                SWP_MOVE | SWP_SIZE);

            // finally, show window
            WinShowWindow(hwndFrame, TRUE);
        }
    }
    CATCH(excpt1) { } END_CATCH();

    return hwndFrame;

}

