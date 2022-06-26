
/*
 *@@sourcefile classlst.c:
 *      implementation of the "WPS Class List" object, whose
 *      SOM code is in src\classes\xclslist.c.
 *
 *      Function prefix for this file:
 *      --  cll* (changed V0.9.1 (99-12-10))
 *
 *      This file implements the actual "WPS Class List"
 *      object (an instance of XWPClassList), which uses the
 *      parse-SOM-classes code in shared\classes.c intensively.
 *
 *      The code in this file consists of window procedures mainly
 *      which are specified when XWPClassList::wpOpen opens
 *      a new class list view. This creates a fairly complex
 *      hierarchy of split windows. See fnwpClassListClient for
 *      details.
 *
 *@@header "config\classlst.h"
 */

/*
 *      Copyright (C) 1997-2006 Ulrich M”ller.
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

/*
 *@@todo:
 *  somIsObj (somapi.h):
 *      Test whether <obj> is a valid SOM object. This test is based solely on
 *      the fact that (on this architecture) the first word of a SOM object is a
 *      pointer to its method table. The test is therefore most correctly understood
 *      as returning true if and only if <obj> is a pointer to a pointer to a
 *      valid SOM method table. If so, then methods can be invoked on <obj>.
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
#define INCL_WINSTDCNR
#define INCL_WINMLE
#define INCL_WINSTDFILE
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles
#include "helpers\standards.h"          // some standard macros
#include "helpers\threads.h"            // thread helpers
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
// #include "xfobj.ih"
#include "xclslist.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\classes.h"             // WPS class list helper functions
#include "shared\cnrsort.h"             // container sort comparison functions
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#include "config\classlst.h"            // SOM logic for "WPS Classes" page

#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Declarations
 *
 ********************************************************************/

// client window class name
#define WC_CLASSLISTCLIENT      "XWPClassListWindow"

#define ID_CLASSFRAME           3000

#define ID_SPLITMAIN            3001
#define ID_SPLITRIGHT           3002

/*
 *@@ METHODRECORD:
 *      extended record structure for
 *      "method info" cnr window.
 *
 *@@changed V0.9.6 (2000-10-16) [umoeller]: added token
 */

typedef struct _METHODRECORD
{
    RECORDCORE          recc;
    // the method name is in RECORDCODE.pszIcon
    ULONG               ulMethodIndex;          // unique method index
    PSZ                 pszType;                // "S"=static, "D"=dynamic, "N"=nonstatic.
                                                // This points to a static string and must not be freed.
    PSZ                 pszIntroducedBy;        // name of class which introduced the method.
                                                // This is the return value of _somGetname and must not be freed.
    ULONG               ulIntroducedBy;         // inheritance distance (see SOMMETHOD.ulIntroducedBy, classlst.h)
    PSZ                 pszOverriddenBy2;       // string of classes which override this method.
                                                // This is composed of the linked list in SOMMETHOD.
    ULONG               ulOverriddenBy;         // inheritance distance (see SOMMETHOD.ulOverriddenBy, classlst.h)

    PSZ                 pszMethodProc;          // method address; always points to szMethodProc buffer below
    CHAR                szMethodProc[30];

    PSZ                 pszToken;               // method token
    CHAR                szToken[20];
} METHODRECORD, *PMETHODRECORD;

#pragma pack(1)

/*
 *@@ CLIENTCTLDATA:
 *      client control data passed with WM_CREATE
 *      to fnwpClassListClient.
 */

typedef struct _CLIENTCTLDATA
{
    USHORT          cb;
    XWPClassList*   somSelf;
    ULONG           ulView;
} CLIENTCTLDATA, *PCLIENTCTLDATA;

/*
 *@@ CLASSLISTVIEWITEM:
 *
 *@@added V0.9.2 (2000-03-08) [umoeller]
 */

typedef struct _CLASSLISTVIEWITEM
{
    USEITEM             UseItem;            // use item; immediately followed by view item
    VIEWITEM            ViewItem;           // view item
} CLASSLISTVIEWITEM, *PCLASSLISTVIEWITEM;

#pragma pack()

/*
 *@@ CLASSLISTCLIENTDATA:
 *      this holds data for the class list client
 *      window (fnwpClassListClient, stored in its
 *      QWL_USER).
 *
 *      A pointer to this data is also stored in
 *      the various other structured for the subwindows.
 */

typedef struct _CLASSLISTCLIENTDATA
{
    XWPClassList        *somSelf;           // pointer to class list instance
    XWPClassListData    *somThis;           // instance data with more settings

    CLASSLISTVIEWITEM   clvi;               // use item and view item (packed)

    HWND                hwndClient,
                        hwndSplitMain,      // "split windows" (comctl.c)
                        hwndSplitRight;

    // class list container
    HWND                hwndClassCnrDlg;    // left child of hwndSplitMain
    PSELECTCLASSDATA    pscd;               // WPS class data (classlist.h)
    ULONG               ulUpdateTimerID;    // timer for delayed window updated

    // class info dlg
    HWND                hwndClassInfoDlg;   // top right child of hwndSplitRight

    // method info dlg
    HWND                hwndMethodInfoDlg;  // bottom right child of hwndSplitRight
    LINKLIST            llCnrStrings;       // linked list of container strings which must be free()'d
    PMETHODINFO         pMethodInfo;        // method info for currently selected class (classlist.h)
    THREADINFO          tiMethodCollectThread; // temporary thread for creating method info

    // popup menus
    HWND                hmenuClassPopup,    // popup menu on class item
                        hmenuMethodsWhitespacePopup;
                                            // popup menu on methods whitespace
} CLASSLISTCLIENTDATA, *PCLASSLISTCLIENTDATA;

/*
 *@@ METHODTHREADINFO:
 *      user data structure used with
 *      fntMethodCollectThread.
 *
 *@@added V0.9.1 (99-12-20) [umoeller]
 */

typedef struct _METHODTHREADINFO
{
    SOMClass            *pClassObject;
    BOOL                fClassMethods;
    HWND                hwndMethodInfoDlg;
} METHODTHREADINFO, *PMETHODTHREADINFO;

/*
 *@@ CLASSLISTTREECNRDATA:
 *      this holds data for the class list container
 *      subwindow (fnwpClassTreeCnrDlg, stored in its QWL_USER).
 */

typedef struct _CLASSLISTTREECNRDATA
{
    PCLASSLISTCLIENTDATA pClientData;
    XADJUSTCTRLS        xacClassCnr;        // for winhAdjustControls

} CLASSLISTTREECNRDATA, *PCLASSLISTTREECNRDATA;

/*
 *@@ CLASSLISTINFODATA:
 *      this holds data for the class info subwindow
 *      (fnwpClassInfoDlg, stored in its QWL_USER).
 */

typedef struct _CLASSLISTINFODATA
{
    PCLASSLISTCLIENTDATA pClientData;
    XADJUSTCTRLS        xacClassInfo;       // for winhAdjustControls
} CLASSLISTINFODATA, *PCLASSLISTINFODATA;

/*
 *@@ CLASSLISTMETHODDATA:
 *      this holds data for the method info subwindow
 *      (fnwpMethodInfoDlg, stored in its QWL_USER).
 */

typedef struct _CLASSLISTMETHODDATA
{
    PCLASSLISTCLIENTDATA pClientData;
    PMETHODRECORD       pMethodReccSource;  // current source record for context menu or NULL
    XADJUSTCTRLS        xacMethodInfo;      // for winhAdjustControls
} CLASSLISTMETHODDATA, *PCLASSLISTMETHODDATA;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

PSZ pszClassInfo = NULL;

/*
 *@@ ampClassCnrCtls:
 *      static array used with winhAdjustControls
 *      for the class list container subwindow
 *      (fnwpClassTreeCnrDlg).
 */

static MPARAM ampClassCnrCtls[] =
    {
        MPFROM2SHORT(ID_XLDI_CNR, XAC_SIZEX | XAC_SIZEY)
    };

/*
 *@@ ampClassInfoCtls:
 *      static array used with winhAdjustControls
 *      for the class info subwindow
 *      (fnwpClassInfoDlg).
 */

static MPARAM ampClassInfoCtls[] =
    {
        MPFROM2SHORT(ID_XLDI_CLASSNAMETXT, XAC_MOVEY),
        MPFROM2SHORT(ID_XLDI_CLASSNAME, XAC_MOVEY | XAC_SIZEX),
        MPFROM2SHORT(ID_XLDI_REPLACEDBYTXT, XAC_MOVEY),
        MPFROM2SHORT(ID_XLDI_REPLACEDBY, XAC_MOVEY | XAC_SIZEX),
        MPFROM2SHORT(ID_XLDI_CLASSTITLETXT, XAC_MOVEY),
        MPFROM2SHORT(ID_XLDI_CLASSTITLE, XAC_MOVEY | XAC_SIZEX),
        MPFROM2SHORT(ID_XLDI_CLASSMODULETXT, XAC_MOVEY),
        MPFROM2SHORT(ID_XLDI_CLASSMODULE, XAC_MOVEY | XAC_SIZEX),
        MPFROM2SHORT(ID_XLDI_ICONTXT, XAC_MOVEY),
        MPFROM2SHORT(ID_XLDI_ICON, XAC_MOVEY),
        MPFROM2SHORT(ID_XLDI_BYTESPERINSTANCETXT, XAC_MOVEY),
        MPFROM2SHORT(ID_XLDI_BYTESPERINSTANCE, XAC_MOVEY | XAC_SIZEX),
        MPFROM2SHORT(ID_XLDI_TEXT2, XAC_SIZEX | XAC_SIZEY)
    };

/*
 *@@ ampMethodInfoCtls:
 *      static array used with winhAdjustControls
 *      for the method info subwindow.
 *      (fnwpMethodInfoDlg).
 */

static MPARAM ampMethodInfoCtls[] =
    {
        MPFROM2SHORT(ID_XLDI_CNR, XAC_SIZEX | XAC_SIZEY),
        MPFROM2SHORT(ID_XLDI_RADIO_INSTANCEMETHODS, XAC_MOVEY),
        MPFROM2SHORT(ID_XLDI_RADIO_CLASSMETHODS, XAC_MOVEY)
    };

/* ******************************************************************
 *
 *   "Register new class" dlg
 *
 ********************************************************************/

/*
 * fnwpOpenFilter:
 *      just a dummy.
 */

STATIC MRESULT EXPENTRY fnwpOpenFilter(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   return WinDefFileDlgProc(hwnd, msg, mp1, mp2);
}

/*
 *@@ fnwpRegisterClass:
 *      dlg func for "Register Class" dialog; use with WinLoadDlg().
 */

STATIC MRESULT EXPENTRY fnwpRegisterClass(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc;
    PREGISTERCLASSDATA prcd = (PREGISTERCLASSDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);

    switch(msg)
    {
        /*
         * WM_INITDLG:
         *      set up the container data and colors
         */

        case WM_INITDLG:
            // CNRINFO CnrInfo;
            WinSetWindowULong(hwndDlg, QWL_USER, (ULONG)mp2);
            prcd = (PREGISTERCLASSDATA)mp2;
            if (prcd->ulHelpPanel == 0)
                winhShowDlgItem(hwndDlg, DID_HELP, FALSE);
            WinSendDlgItemMsg(hwndDlg, ID_XLDI_CLASSNAME, EM_SETTEXTLIMIT,
                              (MPARAM)(255-1), MPNULL);
            WinSendDlgItemMsg(hwndDlg, ID_XLDI_CLASSMODULE, EM_SETTEXTLIMIT,
                              (MPARAM)(255-1), MPNULL);
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;

        case WM_CONTROL:
            if ( (     (SHORT1FROMMP(mp1) == ID_XLDI_CLASSNAME)
                    || (SHORT1FROMMP(mp1) == ID_XLDI_CLASSMODULE)
                 )
                 && (SHORT2FROMMP(mp1) == EN_CHANGE)
               )
            {
                BOOL fEnable = FALSE;
                WinQueryDlgItemText(hwndDlg, ID_XLDI_CLASSNAME,
                                    sizeof(prcd->szClassName), prcd->szClassName);
                if (strlen(prcd->szClassName))
                {
                    WinQueryDlgItemText(hwndDlg, ID_XLDI_CLASSMODULE,
                                        sizeof(prcd->szModName), prcd->szModName);
                    if (strlen(prcd->szClassName))
                        fEnable = TRUE;
                }
                WinEnableControl(hwndDlg, DID_OK, fEnable);
            }
        break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case DID_OK:
                    if (prcd)
                    {
                        WinQueryDlgItemText(hwndDlg, ID_XLDI_CLASSNAME,
                                            sizeof(prcd->szClassName), prcd->szClassName);
                        WinQueryDlgItemText(hwndDlg, ID_XLDI_CLASSMODULE,
                                            sizeof(prcd->szModName), prcd->szModName);
                    }
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                break;

                case ID_XLDI_BROWSE:
                {
                    CHAR szFile[CCHMAXPATH] = "*.DLL";

                    if (cmnFileDlg(hwndDlg,     // V0.9.16 (2001-10-19) [umoeller]
                                   szFile,
                                   WINH_FOD_INILOADDIR | WINH_FOD_INISAVEDIR,
                                   HINI_USER,
                                   INIAPP_XWORKPLACE,
                                   "RegisterClassLastDir"))
                    {
                        WinSetDlgItemText(hwndDlg,
                                          ID_XLDI_CLASSMODULE,
                                          szFile);
                    }
                }
                break;

                default:
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            }
        break;

        case WM_HELP:
            if (prcd->ulHelpPanel)
                _wpDisplayHelp(cmnQueryActiveDesktop(),
                               prcd->ulHelpPanel,
                               (PSZ)prcd->pszHelpLibrary);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }
    return mrc;
}

/* ******************************************************************
 *
 *   XWPClassList Helper functions
 *
 ********************************************************************/

/*
 *@@ ParseDescription:
 *      searches pszBuf for pszSrch0; if found, the line containing
 *      pszSrch is parsed as follows:
 +          #pszSrch0#  #ulFlags# #RestOfLine#
 +             PSZ       hex str     PSZ
 *
 *      Returns length of what was copied to pszDescription or
 *      zero on errors.
 *
 *      This is used for explaining a certain WPS class when it is
 *      being selected in the "WPS classes" notebook page.
 *      Returns TRUE if pszSrch0 was found.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfsys.c.
 *@@changed V0.9.0 [umoeller]: fixed small memory leak
 *@@changed V0.9.6 (2000-10-25) [umoeller]: this didn't find words correctly, fixed
 *@@changed V1.0.1 (2003-01-30) [umoeller]: optimized, fixed buffer overflows
 */

STATIC BOOL ParseDescription(PSZ pszBuf,           // in: complete descriptions text file
                             PSZ pszSrch0,         // in: class name (token) to search for
                             PULONG pulFlags,      // out: if found, flags for the class
                             PSZ pszDescription,   // out: if found, class's description
                             ULONG cbDescription)  // in: size of pszDescription buffer
{
    BOOL    brc = FALSE;

    // create search string for beginning of line
    PSZ     pszSrch2;
    ULONG   len = strlen(pszSrch0);
    if (pszSrch2 = malloc(len + 4))
    {
        PSZ     p1, p2;
        BOOL    fFound = FALSE;

        pszSrch2[0] = '\r';
        pszSrch2[1] = '\n';
        memcpy(pszSrch2 + 2,
               pszSrch0,
               len + 1);

        // find word V0.9.6 (2000-10-25) [umoeller]
        p1 = pszBuf;
        len += 2;
        while (p1)
        {
            if (p1 = strstr(p1, pszSrch2))
                // item found:
                if (p1[len] == ' ')
                {
                    fFound = TRUE;
                    break;
                }
                else
                    // not a word: search on
                    p1++;
        }

        if (fFound)
        {
            // found:
            p1 += 2; // skip \r\n
            if (p1 = strchr(p1, ' '))
            {
                // p1 now has ulFlags
                // sscanf(p1, "%lX ", pulFlags);
                *pulFlags = atoi(p1);
                if (p1 = strchr(p1 + 1, ' '))
                {
                    brc = TRUE;
                    // p1 now has beginning of description
                    if (p2 = strstr(p1, "\r\n"))
                    {
                        // p2 now has end of description
                        strlcpy(pszDescription,
                                p1 + 1,
                                min((p2 - p1), cbDescription));
                    }
                    else
                        strlcat(pszDescription,
                                p1 + 1,
                                cbDescription);
                }
            }
        }

        free(pszSrch2);
    }

    return brc;
}

/*
 *@@ RelinkWindows:
 *      this gets called to link the various split windows
 *      properly, depending on _fShowMethods.
 */

STATIC VOID RelinkWindows(PCLASSLISTCLIENTDATA pClientData,
                          BOOL fReformat)
{
    XWPClassListData *somThis = pClientData->somThis;

    // a) main split window:
    WinSendMsg(pClientData->hwndSplitMain,
               SPLM_SETLINKS,
               // left window: class list container dlg
               (MPARAM)pClientData->hwndClassCnrDlg,
               // right window:
               // depending on whether method info is to be shown:
               (MPARAM)( (_fShowMethods)
                         // right split window
                         ? pClientData->hwndSplitRight
                         // or class info
                         : pClientData->hwndClassInfoDlg
                       ));
    if (fReformat)
        ctlUpdateSplitWindow(pClientData->hwndSplitMain);

    if (_fShowMethods)
    {
        // b) right split window
        WinSendMsg(pClientData->hwndSplitRight,
                   SPLM_SETLINKS,
                   // bottom window: method info dlg
                   (MPARAM)pClientData->hwndMethodInfoDlg,
                   // top window: class info dlg
                   (MPARAM)pClientData->hwndClassInfoDlg);
        if (fReformat)
            ctlUpdateSplitWindow(pClientData->hwndSplitRight);
    }

    WinShowWindow(pClientData->hwndSplitRight, _fShowMethods);
    WinShowWindow(pClientData->hwndMethodInfoDlg, _fShowMethods);
}

/*
 *@@ CleanupMethodsInfo:
 *      cleans up the methods container.
 */

STATIC VOID CleanupMethodsInfo(PCLASSLISTCLIENTDATA pClientData)
{
    // clear methods container first; the container
    // uses the strings from METHODINFO, so
    // we better do this first
    WinSendDlgItemMsg(pClientData->hwndMethodInfoDlg, ID_XLDI_CNR,
                      CM_REMOVERECORD,
                      NULL,
                      MPFROM2SHORT(0, CMA_FREE | CMA_INVALIDATE));

    // did we allocate a method info before?
    if (pClientData->pMethodInfo)
    {
        // yes: clean up
        clsFreeMethodInfo(&pClientData->pMethodInfo);
    }

    // now clean up container strings
    lstClear(&pClientData->llCnrStrings);
}

/*
 *@@ fntMethodCollectThread:
 *
 *      This gets a METHODTHREADINFO in the user
 *      parameter.
 *
 *      If THREADINFO.fExit gets set to TRUE, the
 *      thread terminates itself.
 *
 *@@added V0.9.1 (99-12-20) [umoeller]
 */

STATIC void _Optlink fntMethodCollectThread(PTHREADINFO pti)
{
    PMETHODTHREADINFO pmti = (PMETHODTHREADINFO)(pti->ulData);
    // now update method info
    PMETHODINFO pMethodInfo = clsQueryMethodInfo(pmti->pClassObject,
                                                 // return-class-methods flag
                                                 pmti->fClassMethods,
                                                 &pti->fExit); // exit flag
    // notify method info dlg;
    // WinPostMsg works even though we don't
    // have a message queue
    WinPostMsg(pmti->hwndMethodInfoDlg,
               WM_FILLCNR,
               (MPARAM)pMethodInfo,
               0);

    free(pmti);
}

/*
 *@@ NewClassSelected:
 *      this is called every time a new class gets selected
 *      in the "WPS classes" page (CN_EMPHASIS in
 *      fnwpSettingsWpsClasses), after the update timer
 *      has elapsed in fnwpClassTreeCnrDlg.
 *
 *      We will then update the class info display
 *      and the method information.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfsys.c; updated for new window hierarchy
 *@@changed V0.9.0 [umoeller]: added method information
 *@@changed V0.9.0 [umoeller]: now using a delay timer
 *@@changed V0.9.1 (99-12-20) [umoeller]: now using fntMethodCollectThread for method infos
 *@@changed V0.9.12 (2001-05-17) [pr]: beautify class title
 *@@changed V0.9.20 (2002-08-04) [umoeller]: added instance size
 *@@changed V1.0.0 (2002-08-12) [umoeller]: fixed instance size for null objects
 */

STATIC VOID NewClassSelected(PCLASSLISTCLIENTDATA pClientData)
{
    PWPSLISTITEM    pwps = pClientData->pscd->preccSelection->pwps;
    CHAR            szInfo[1000] = "",
                    szInfo2[256] = "";
    // PNLSSTRINGS     pNLSStrings = cmnQueryNLSStrings();

    CleanupMethodsInfo(pClientData);
        // @@todo this still causes memory leaks when the methods thread is running

    if ((pwps) && (pClientData->hwndClassInfoDlg))
    {
        // pwps != NULL: valid class selected (and not the "Orphans" item)

        HPOINTER hClassIcon = NULLHANDLE;

        // first check if we have a WPS class,
        // because otherwise we better not invoke
        // WPS methods on it
        BOOL    fIsWPSClass = FALSE;
        if (pwps->pClassObject)
            fIsWPSClass = _somDescendedFrom(pwps->pClassObject, _WPObject);

        /*
         * class info:
         *
         */

        // dll name
        WinSetDlgItemText(pClientData->hwndClassInfoDlg, ID_XLDI_CLASSMODULE, pwps->pszModName);

        // class name
        strlcpy(szInfo, pwps->pszClassName, sizeof(szInfo));
        WinSetDlgItemText(pClientData->hwndClassInfoDlg, ID_XLDI_CLASSNAME, szInfo);

        // "replaced with"
        WinSetDlgItemText(pClientData->hwndClassInfoDlg, ID_XLDI_REPLACEDBY,
                (pwps->pszReplacedWithClasses)
                    ? pwps->pszReplacedWithClasses
                    : "");

        // class icon
        if (pwps->pClassObject)
            if (fIsWPSClass)
                hClassIcon = _wpclsQueryIcon(pwps->pClassObject);

        WinSendDlgItemMsg(pClientData->hwndClassInfoDlg, ID_XLDI_ICON,
                          SM_SETHANDLE,
                          (MPARAM)hClassIcon,  // NULLHANDLE if error -> hide
                          MPNULL);

        // class title
        if (pwps->pClassObject)
        {
            PSZ pszClassTitle = NULL;
            if (fIsWPSClass)
                pszClassTitle = strdup(_wpclsQueryTitle(pwps->pClassObject));
            if (pszClassTitle)
            {
                strhBeautifyTitle(pszClassTitle);
                sprintf(szInfo2,
                        "\"%s\"",
                        pszClassTitle);
                free(pszClassTitle);
            }
            else
            {
                szInfo2[0] = '?';
                szInfo2[1] = '\0';
            }
        }
        else
            cmnGetString2(szInfo2,
                          ID_XSSI_WPSCLASSLOADINGFAILED,
                          sizeof(szInfo2));

        WinSetDlgItemText(pClientData->hwndClassInfoDlg,
                          ID_XLDI_CLASSTITLE,
                          szInfo2);

        // instance size V0.9.20 (2002-08-04) [umoeller]
        // check class object first V1.0.0 (2002-08-12) [umoeller]
        if (pwps->pClassObject)
            sprintf(szInfo2,
                    "%d / %d",
                    _somGetInstanceSize(pwps->pClassObject),
                    _somGetInstancePartSize(pwps->pClassObject));
        else
            szInfo2[0] = '\0';

        WinSetDlgItemText(pClientData->hwndClassInfoDlg,
                          ID_XLDI_BYTESPERINSTANCE,
                          szInfo2);

        // class information
        if (pszClassInfo)
        {
            ULONG ulFlags;
            if (!ParseDescription(pszClassInfo,
                                  pwps->pszClassName,
                                  &ulFlags,
                                  szInfo,
                                  sizeof(szInfo)))
            {
                // not found: search for "UnknownClass"
                ParseDescription(pszClassInfo,
                                 "UnknownClass",
                                 &ulFlags,
                                 szInfo,
                                 sizeof(szInfo));
            }
        }

        /*
         * method info:
         *
         */

        if (pwps->pClassObject)
        {
            PMETHODTHREADINFO pmti = (PMETHODTHREADINFO)malloc(sizeof(METHODTHREADINFO));
            pmti->pClassObject = pwps->pClassObject;
            pmti->fClassMethods = (pClientData->somThis->ulMethodsRadioB
                                                   == ID_XLDI_RADIO_CLASSMETHODS);
            pmti->hwndMethodInfoDlg = pClientData->hwndMethodInfoDlg;

            // class object exists:
            // start thread for collecting method info
            thrCreate(&pClientData->tiMethodCollectThread,
                      fntMethodCollectThread,
                      NULL, // running flag
                      "CollectMethods",
                      0,    // no msgq
                      (ULONG)pmti);

        } // end if (pwps->pClassObject)
    } // end if (pwps)
    else
    {
        // if (pwps == NULL), the "Orphans" item has been
        // selected: give info for this
        cmnGetString2(szInfo,
                      ID_XSSI_WPSCLASSORPHANSINFO,
                      sizeof(szInfo));
        WinSetDlgItemText(pClientData->hwndClassInfoDlg, ID_XLDI_CLASSMODULE, "");
        WinSetDlgItemText(pClientData->hwndClassInfoDlg, ID_XLDI_CLASSNAME, "");
        WinSetDlgItemText(pClientData->hwndClassInfoDlg, ID_XLDI_REPLACEDBY, "");
        WinSetDlgItemText(pClientData->hwndClassInfoDlg, ID_XLDI_CLASSTITLE, "");

        WinSendDlgItemMsg(pClientData->hwndClassInfoDlg, ID_XLDI_ICON,
                          SM_SETHANDLE,
                          (MPARAM)NULLHANDLE,  // hide icon
                          MPNULL);
    }

    // give MLE new text
    WinSetDlgItemText(pClientData->hwndClassInfoDlg, ID_XLDI_TEXT2, szInfo);
    // scroll MLE to top
    WinSendDlgItemMsg(pClientData->hwndClassInfoDlg, ID_XLDI_TEXT2,
                      MLM_SETFIRSTCHAR,
                      (MPARAM)0,
                      MPNULL);
}

/*
 *@@ StartMethodsUpdateTimer:
 *      this starts a timer upon the class cnr dlg
 *      with ID 2. When WM_TIMER comes into that window,
 *      the methods info will be re-retrieved.
 */

STATIC VOID StartMethodsUpdateTimer(PCLASSLISTCLIENTDATA pClientData)
{
    HAB     habDlg = WinQueryAnchorBlock(pClientData->hwndClassCnrDlg);
    // start one-shot timer to update other
    // dlgs delayed
    if (pClientData->ulUpdateTimerID != 0)
    {
        // timer already running:
        // restart
        WinStopTimer(habDlg, pClientData->hwndClassCnrDlg,
                     2);
    }

    // (re)start timer
    pClientData->ulUpdateTimerID
            = WinStartTimer(habDlg,
                            pClientData->hwndClassCnrDlg,
                            2,          // timer ID
                            100);       // ms
}

/*
 *@@ fncbReturnWPSClassAttr:
 *      this callback function is called from clsWpsClasses2Cnr
 *      for every single record core which represents a WPS class;
 *      we need to return the record core attributes.
 *
 *      For classes which have been replaced, we set the CRA_DISABLED
 *      attribute. This is now finally working with 0.9.15.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfsys.c.
 *@@changed V0.9.0 [umoeller]: added CRA_DISABLED
 *@@changed V0.9.0 [umoeller]: changed initially expanded classes
 *@@changed V0.9.15 (2001-09-14) [umoeller]: now expanding replaced classes too
 */

STATIC MRESULT EXPENTRY fncbReturnWPSClassAttr(HWND hwndCnr,
                                               ULONG ulscd,   // SELECTCLASSDATA struct
                                               MPARAM mpwps,  // current WPSLISTITEM struct
                                               MPARAM mpreccParent) // parent record core
{
    PWPSLISTITEM        pwps;

    USHORT              usAttr = CRA_RECORDREADONLY;
    BOOL                fExpand = FALSE;

    if (pwps = (PWPSLISTITEM)mpwps)
    {
        // if the class is one of the following,
        // expand all the parent records of the new record
        // so that these classes are initially visible
        if (    (!strcmp(pwps->pszClassName, "WPAbstract"))
             || (!strcmp(pwps->pszClassName, G_pcszWPDataFile)) // V0.9.0

             || (!strcmp(pwps->pszClassName, G_pcszWPFolder))   // V0.9.0
           )
        {
            cnrhExpandFromRoot(hwndCnr, (PRECORDCORE)mpreccParent);
        }
        else
            // if this class replaces a parent class,
            // expand all records up to that parent class only
            if (pwps->pszReplacesClass)
            {
                PCLASSRECORDCORE preccParent = (PCLASSRECORDCORE)mpreccParent;
                while (preccParent)
                {
                    WinSendMsg(hwndCnr, CM_EXPANDTREE, (MPARAM)preccParent, MPNULL);
                    if (!strcmp(preccParent->pwps->pszClassName,
                                pwps->pszReplacesClass))
                        // reached the replacement class: stop
                        break;

                    preccParent = (PCLASSRECORDCORE)WinSendMsg(hwndCnr,
                                                          CM_QUERYRECORD,
                                                          (MPARAM)preccParent,
                                                          MPFROM2SHORT(CMA_PARENT,
                                                                       CMA_ITEMORDER));

                    if (    (!preccParent)
                         || (preccParent == (PCLASSRECORDCORE)-1)
                       )
                        break;
                }
            }

        if (pwps->pszReplacedWithClasses)
        {
            // if the class itself is replaced,
            // disable it (cnr owner draw will paint it gray)
            PMPF_CLASSLIST(("class %s is replaced with %s",
                            pwps->pszClassName,
                            pwps->pszReplacedWithClasses));

            usAttr |= CRA_DISABLED;
        }
    }

    if (fExpand)        // V0.9.15 (2001-09-14) [umoeller]
        usAttr |= CRA_EXPANDED;
    else
        usAttr |= CRA_COLLAPSED;

    return (MPARAM)usAttr;
}

/*
 *@@ fncbReplaceClassSelected:
 *      callback func for class selected in the "Replace
 *      with subclass" dlg;
 *      mphwndInfo has been set to the static control hwnd.
 *      Returns TRUE if the selection is valid; the dlg func
 *      will then enable the OK button.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfsys.c.
 */

STATIC MRESULT EXPENTRY fncbReplaceClassSelected(HWND hwndCnr,
                                                 ULONG ulpsbsc,
                                                 MPARAM mpwps,
                                                 MPARAM mphwndInfo)
{
    PWPSLISTITEM pwps = (PWPSLISTITEM)mpwps;
    // PSZ pszClassTitle;
    CHAR szInfo[2000] = "";

    if (pwps->pClassObject)
    {
        if (pszClassInfo)
        {
            ULONG ulFlags;
            if (!ParseDescription(pszClassInfo,
                                  pwps->pszClassName,
                                  &ulFlags,
                                  szInfo,
                                  sizeof(szInfo)))
            {
                // not found: search for "UnknownClass"
                ParseDescription(pszClassInfo,
                                 "UnknownClass",
                                 &ulFlags,
                                 szInfo,
                                 sizeof(szInfo));
            }
        }
    }

    WinSetWindowText((HWND)mphwndInfo, szInfo);

    return (MRESULT)TRUE;
}

/*
 *@@ fnCompareMethodIndex:
 *      container sort function for sorting by
 *      method index.
 *
 *      Note: the information in the PM reference is flat out wrong.
 *      Container sort functions need to return the following:
 +          0   pmrc1 == pmrc2
 +         -1   pmrc1 <  pmrc2
 +         +1   pmrc1 >  pmrc2
 */

STATIC SHORT EXPENTRY fnCompareMethodIndex(PMETHODRECORD precc1,
                                           PMETHODRECORD precc2,
                                           PVOID pStorage)
{
    SHORT src = 0;
    if ((precc1) && (precc2))
    {
        if (precc1->ulMethodIndex < precc2->ulMethodIndex)
            src = -1;
        else if (precc1->ulMethodIndex > precc2->ulMethodIndex)
            src = 1;
        // else equal: return 0
    }
    return src;
}

/*
 *@@ fnCompareMethodName:
 *      container sort function for sorting by
 *      method name.
 */

STATIC SHORT EXPENTRY fnCompareMethodName(PMETHODRECORD precc1,
                                          PMETHODRECORD precc2,
                                          PVOID pStorage)
{
    SHORT src = 0;
    if ((precc1) && (precc2))
    {
        int i  = strcmp(precc1->recc.pszIcon, precc2->recc.pszIcon);
        if (i < 0)
            src = -1;
        else if (i > 0)
            src = 1;
        // else equal: return 0
    }
    return src;
}

/*
 *@@ fnCompareMethodIntro:
 *      container sort function for sorting by
 *      the class which introduced a method.
 */

STATIC SHORT EXPENTRY fnCompareMethodIntro(PMETHODRECORD precc1,
                                           PMETHODRECORD precc2,
                                           PVOID pStorage)
{
    SHORT src = 0;
    if ((precc1) && (precc2))
    {
        if (precc1->ulIntroducedBy < precc2->ulIntroducedBy)
            src = -1;            // put newest methods on top (reverse)
        else if (precc1->ulIntroducedBy > precc2->ulIntroducedBy)
            src = 1;
        // else equal: return 0
    }
    return src;
}

/*
 *@@ fnCompareMethodOverride:
 *      container sort function for sorting by
 *      the class which overrode a method.
 */

STATIC SHORT EXPENTRY fnCompareMethodOverride(PMETHODRECORD precc1,
                                              PMETHODRECORD precc2,
                                              PVOID pStorage)
{
    SHORT src = 0;
    if ((precc1) && (precc2))
    {
        if (precc1->ulOverriddenBy < precc2->ulOverriddenBy)
            src = -1;            // put newest methods on top (reverse)
        else if (precc1->ulOverriddenBy > precc2->ulOverriddenBy)
            src = 1;
        else
            // equal: compare method intro
            src = fnCompareMethodIntro(precc1, precc2, pStorage);
    }
    return src;
}

/*
 *@@ QueryMethodsSortFunc:
 *      returns the method container's sort function
 *      according to the current instance data.
 */

STATIC PFNCNRSORT QueryMethodsSortFunc(PCLASSLISTCLIENTDATA pClientData)
{
    PFNCNRSORT  pfnCnrSort = NULL;

    switch (pClientData->somThis->ulSortID)
    {
        // "Sort by" commands
        case ID_XLMI_METHOD_SORT_INDEX:
            pfnCnrSort = (PFNCNRSORT)fnCompareMethodIndex;
        break;

        case ID_XLMI_METHOD_SORT_NAME:
            pfnCnrSort = (PFNCNRSORT)fnCompareMethodName;
        break;

        case ID_XLMI_METHOD_SORT_INTRO:
            pfnCnrSort = (PFNCNRSORT)fnCompareMethodIntro;
        break;

        case ID_XLMI_METHOD_SORT_OVERRIDE:
            pfnCnrSort = (PFNCNRSORT)fnCompareMethodOverride;
        break;
    }

    return pfnCnrSort;
}

/* ******************************************************************
 *
 *   Class list window procedures
 *
 ********************************************************************/

MRESULT EXPENTRY fnwpClassTreeCnrDlg(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY fnwpClassInfoDlg(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY fnwpMethodInfoDlg(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2);

/*
 *  Here come the window procedures for the various
 *  class list windows: the client, and the two
 *  subdialogs.
 */

/*
 *@@ fnwpClassListClient:
 *      this is the window proc for the client window of
 *      a "class list" view of the XWPClassList object.
 *
 *      Upon WM_CREATE, this function creates all the
 *      class list subwindows, as listed below. Since
 *      the frame is created in cllCreateClassListView,
 *      that function also adds the frame to the class
 *      list object's in-use list and registers the view.
 *      Apparently, this cannot be done during WM_CREATE
 *      because the WPS loses track of the open windows then.
 *      The view is removed in this function upon WM_CLOSE.
 *
 *      The main client window (with this window procedure)
 *      is created from cllCreateClassListView and gets passed
 *      a CLIENTCTLDATA structure with WM_CREATE, upon which
 *      we'll create all the subwindows here.
 *
 *      The window hierarchy is as follows (lines signify
 *      parentship):
 *
 +      ID_CLASSFRAME (standard WC_FRAME window)
 +        |
 +        +-- ... default frame controls
 +        |
 +        +-- FID_CLIENT   (this window proc)
 +              |
 +              +-- ID_SPLITMAIN (vertical split, ctl_fnwpSplitWindow, comctl.c)
 +                    |
 +                    +-- ID_XLD_CLASSLIST  (fnwpClassTreeCnrDlg, loaded from resources)
 +                    |      |
 +                    |      +-- container with actual class list
 +                    |
 +                    +-- ID_SPLITBAR     (ctl_fnwpSplitBar, comctl.c)
 +                    |
 +                    +-- ID_SPLITRIGHT (horizontal split on the right, ctl_fnwpSplitWindow)
 +                           |
 +                           +-- ID_XLD_CLASSINFO (fnwpClassInfoDlg, loaded from resources)
 +                           |      |
 +                           |      +-- ... all the class info subwindows
 +                           |
 +                           +-- ID_SPLITBAR     (ctl_fnwpSplitBar, comctl.c)
 +                           |
 +                           +-- ID_XLD_METHODINFO (fnwpMethodInfoDlg, loaded from resources)
 +                                  |
 +                                  +-- ... all the method info subwindows
 *
 *      Visually, this appears like this:
 *
 +      +-------------------------+-----------------------------+
 +      |                         |                             |
 +      |                         |  ID_XLD_CLASSINFO           |
 +      |                         |  (fnwpClassInfoDlg,         |
 +      |                         |   loaded from NLS resrcs)   |
 +      |                         |                             |
 +      |  ID_XLD_CLASSLIST       +-----------------------------+
 +      |  (fnwpClassTreeCnrDlg,  |                             |
 +      |   loaded from NLS       |  ID_XLD_METHODINFO          |
 +      |   resources)            |  (fnwpMethodInfoDlg,        |
 +      |                         |   loaded from NLS rescrs)   |
 +      |                         |                             |
 +      +-------------------------+-----------------------------+
 *
 *      Note that to keep all the different data apart, we create a
 *      slightly complex system of data structures with each of the
 *      frame windows which are created here:
 *
 *      --  The client window (fnwpClassListClient) creates a
 *          CLASSLISTCLIENTDATA structure in its QWL_USER
 *          window word.
 *
 *      --  The class list container dialog (fnwpClassTreeCnrDlg)
 *          gets the client's CLASSLISTCLIENTDATA with WM_INITDLG
 *          and creates a CLASSLISTTREECNRDATA in its own QWL_USER
 *          window word.
 *
 *      --  The class info dialog (fnwpClassInfoDlg)
 *          gets the client's CLASSLISTCLIENTDATA with WM_INITDLG
 *          and creates a CLASSLISTINFODATA in its own QWL_USER
 *          window word.
 *
 *      --  The method info dialog (fnwpMethodInfoDlg)
 *          gets the client's CLASSLISTCLIENTDATA with WM_INITDLG
 *          and creates a CLASSLISTMETHODDATA in its own QWL_USER
 *          window word.
 *
 *      When the main window (the frame of the view) is resized,
 *      fnwpClassListClient gets a WM_WINDOWPOSCHANGED message,
 *      as usual. We then call WinSetWindowPos on the main split
 *      window (ID_SPLITMAIN), whose window proc (ctl_fnwpSplitWindow,
 *      comctl.c) will then automatically resize all the subwindows.
 *
 *      When any of the three subdialogs receive WM_WINDOWPOSCHANGED
 *      in turn, they'll use winhAdjustControls to update their
 *      controls' positions and sizes.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.3 (2000-04-02) [umoeller]: moved wpRegisterView etc. to cllCreateClassListView
 *@@changed V0.9.6 (2000-10-16) [umoeller]: fixed excessive menu creation
 *@@changed V1.0.1 (2003-01-25) [umoeller]: beautified split view a little
 *@@changed V1.0.6 (2006-10-16) [pr]: fixed show/maximize from minimize @@fixes 865
 */

STATIC MRESULT EXPENTRY fnwpClassListClient(HWND hwndClient, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = (MRESULT)0;
    PCLASSLISTCLIENTDATA pClientData
            = (PCLASSLISTCLIENTDATA)WinQueryWindowPtr(hwndClient, QWL_USER);

    switch(msg)
    {
        /*
         * WM_CREATE:
         *
         */

        case WM_CREATE:
        {
            // frame window successfully created:
            SPLITBARCDATA   sbcd;
            PCLIENTCTLDATA  pCData = (PCLIENTCTLDATA)mp1;
            HAB             hab = WinQueryAnchorBlock(hwndClient);

            // now add the view to the object's use list;
            // this use list is used by wpViewObject and
            // wpClose to check for existing open views.
            // get storage for and initialize a use list item
            pClientData = (PCLASSLISTCLIENTDATA)_wpAllocMem(pCData->somSelf,
                                                            sizeof(CLASSLISTCLIENTDATA),
                                                            NULL);
            memset((PVOID)pClientData, 0, sizeof(CLASSLISTCLIENTDATA));
            pClientData->somSelf         = pCData->somSelf;

            // initialize list of cnr items to be freed later
            lstInit(&pClientData->llCnrStrings, TRUE);

            // save the pointer to the use item in the window words so that
            // the window procedure can remove it from the list when the window
            // is closed
            WinSetWindowPtr(hwndClient, QWL_USER, pClientData);

            // store instance data
            pClientData->somThis = XWPClassListGetData(pCData->somSelf);

            // store client HWND in window data too
            pClientData->hwndClient = hwndClient;

            /*
             * "Class info" dlg subwindow (top right):
             *
             */

            // this will be linked to the right split window later,
            // which will in turn be linked to the main split window.
            pClientData->hwndClassInfoDlg = WinLoadDlg(hwndClient, hwndClient,
                                                         // parent and owner;
                                                         // the parent will be changed
                                                         // by ctlCreateSplitWindow
                                                       fnwpClassInfoDlg,
                                                       cmnQueryNLSModuleHandle(FALSE),
                                                       ID_XLD_CLASSINFO,
                                                       pClientData);     // create param
            WinSetWindowBits(pClientData->hwndClassInfoDlg,
                             QWL_STYLE,
                             WS_CLIPCHILDREN,         // set bit
                             WS_CLIPCHILDREN);

            // preload popup menu
            pClientData->hmenuClassPopup
                = WinLoadMenu(HWND_OBJECT,
                              cmnQueryNLSModuleHandle(FALSE),
                              ID_XLM_CLASS_SEL);

            /*
             * "Method info" dlg subwindow (bottom right):
             *
             */

            pClientData->hwndMethodInfoDlg = WinLoadDlg(hwndClient, hwndClient,
                                                            // parent and owner;
                                                            // the parent will be changed
                                                            // by ctlCreateSplitWindow
                                                        fnwpMethodInfoDlg,
                                                        cmnQueryNLSModuleHandle(FALSE), ID_XLD_METHODINFO,
                                                        pClientData);     // create param
            WinSetWindowBits(pClientData->hwndMethodInfoDlg,
                             QWL_STYLE,
                             WS_CLIPCHILDREN,         // set bit
                             WS_CLIPCHILDREN);

            // preload popup menu
            pClientData->hmenuMethodsWhitespacePopup
                = WinLoadMenu(HWND_OBJECT,
                              cmnQueryNLSModuleHandle(FALSE),
                              ID_XLM_METHOD_NOSEL);


            /*
             * container dlg subwindow (left half of window);:
             *
             */

            // this will be linked to the split window later.
            // This is a sizeable dialog defined in the NLS resources.
            pClientData->hwndClassCnrDlg = WinLoadDlg(hwndClient, hwndClient,
                                                            // parent and owner;
                                                            // the parent will be changed
                                                            // by ctlCreateSplitWindow
                                                      fnwpClassTreeCnrDlg,
                                                      cmnQueryNLSModuleHandle(FALSE), ID_XLD_CLASSLIST,
                                                      pClientData);     // create param
            WinSetWindowBits(pClientData->hwndClassCnrDlg,
                             QWL_STYLE,
                             WS_CLIPCHILDREN,         // set bit
                             WS_CLIPCHILDREN);

            // create main split window (vertical split bar)
            memset(&sbcd, 0, sizeof(SPLITBARCDATA));
            sbcd.ulCreateFlags = SBCF_VERTICAL          // vertical split bar
                                    | SBCF_PERCENTAGE   // lPos has split bar pos
                                                        // in percent of the client
                                    | SBCF_3DEXPLORERSTYLE // V1.0.1 (2003-01-25) [umoeller]
                                    // SBCF_3DSUNK       // draw 3D "sunk" frame
                                    | SBCF_MOVEABLE ;    // moveable split bar
            sbcd.lPos = 50;
            sbcd.hwndParentAndOwner = hwndClient;
            sbcd.ulLeftOrBottomLimit = 100;
            sbcd.ulRightOrTopLimit = 100;
            sbcd.ulSplitWindowID = ID_SPLITMAIN;
            pClientData->hwndSplitMain = ctlCreateSplitWindow(hab,
                                                              &sbcd);

            // create right split window (horizontal split bar)
            memset(&sbcd, 0, sizeof(SPLITBARCDATA));
            sbcd.ulCreateFlags = SBCF_HORIZONTAL        // horizontal split bar
                                    | SBCF_PERCENTAGE   // lPos has split bar pos
                                                        // in percent of the client
                                    // | SBCF_3DSUNK       // we already have one
                                    | SBCF_MOVEABLE;    // moveable split bar
            sbcd.lPos = 50;
            sbcd.hwndParentAndOwner = pClientData->hwndSplitMain;
            sbcd.ulLeftOrBottomLimit = 100;
            sbcd.ulRightOrTopLimit = 100;
            sbcd.ulSplitWindowID = ID_SPLITRIGHT;
            pClientData->hwndSplitRight = ctlCreateSplitWindow(hab,
                                                               &sbcd);



            // now set the "split links"
            RelinkWindows(pClientData, FALSE);

            // and fill the container with the classes;
            // this is handled in fnwpClassTreeCnrDlg
            WinPostMsg(pClientData->hwndClassCnrDlg, WM_FILLCNR, MPNULL, MPNULL);

            // show help panel if opened for the first time
            /* if ((cmnQuerySetting(sulIntroHelpShown) & HLPS_CLASSLIST) == 0)
            {
                WinPostMsg(hwndClient, WM_HELP, 0, 0);
                cmnQuerySetting(sulIntroHelpShown) |= HLPS_CLASSLIST;
                cmnStoreGlobalSettings();
            } */

            mrc = (MPARAM)FALSE;
        }
        break;

        /*
         * WM_WINDOWPOSCHANGED:
         *
         */

        case WM_WINDOWPOSCHANGED:
            if (pClientData)
            {
                // this msg is passed two SWP structs:
                // one for the old, one for the new data
                // (from PM docs)
                PSWP pswpNew = PVOIDFROMMP(mp1);

                // resizing?
                if (pswpNew->fl & SWP_SIZE)
                {

                    // adjust size of "split window",
                    // which will rearrange all the linked
                    // windows (comctl.c)
                    WinSetWindowPos(pClientData->hwndSplitMain, HWND_TOP,
                                    0, 0,
                                    pswpNew->cx, pswpNew->cy, // sCXNew, sCYNew,
                                    SWP_SIZE);
                }
            }
        break;

        /*
         * WM_MINMAXFRAME:
         *      when minimizing, we hide the "split window",
         *      because otherwise the child dialogs will
         *      display garbage
         */

        case WM_MINMAXFRAME:
        {
            PSWP pswp = (PSWP)mp1;
            if (pswp->fl & SWP_MINIMIZE)
                WinShowWindow(pClientData->hwndSplitMain, FALSE);
            else if (pswp->fl & (SWP_MAXIMIZE | SWP_RESTORE))  // V1.0.6 (2006-10-16) [pr]: @@fixes 865
                WinShowWindow(pClientData->hwndSplitMain, TRUE);
        }
        break;

        /*
         * WM_CLOSE:
         *      window list is being closed:
         *      store the window position
         */

        case WM_CLOSE:
        {
            HWND hwndFrame = WinQueryWindow(hwndClient, QW_PARENT);
            // get the object pointer and the use list item from the window

            // save window position
            winhSaveWindowPos(hwndFrame,
                              HINI_USER,
                              INIAPP_XWORKPLACE,
                              INIKEY_WNDPOSCLASSINFO);
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
                // wait for method thread to terminate
                thrWait(&pClientData->tiMethodCollectThread);

                // remove this window from the object's use list
                _wpDeleteFromObjUseList(pClientData->somSelf,
                                        &pClientData->clvi.UseItem);

                pClientData->somThis->hwndOpenView = NULLHANDLE;

                // destroy popups
                WinDestroyWindow(pClientData->hmenuClassPopup);
                WinDestroyWindow(pClientData->hmenuMethodsWhitespacePopup);

                // free the use list item
                _wpFreeMem(pClientData->somSelf, (PBYTE)pClientData);
            }

            // return default NULL
        break;

        /*
         * WM_HELP:
         *
         */

        case WM_HELP:
            cmnDisplayHelp(pClientData->somSelf,
                           ID_XSH_SETTINGS_WPSCLASSES);
        break;

        default:
           mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ ShowClassContextMenu:
 *      this gets called from fnwpClassTreeCnrDlg
 *      when a context menu is requested for a
 *      class in the class tree. This builds,
 *      sets up, and shows that class's context menu.
 *
 *@@added V0.9.1 (99-12-28) [umoeller]
 *@@changed V0.9.5 (2000-08-26) [umoeller]: orphaned classes had "Deregister" disabled; fixed
 *@@changed V0.9.6 (2000-10-16) [umoeller]: orphaned classes had "Create object" enabled; fixed
 *@@changed V0.9.6 (2000-10-16) [umoeller]: fixed excessive menu creation
 */

STATIC VOID ShowClassContextMenu(HWND hwndDlg,
                                 PSELECTCLASSDATA pscd,
                                 HWND hPopupMenu)      // in: "class" popup menu
{
    LONG lrc2;
    /* HWND hPopupMenu = WinLoadMenu(hwndDlg,
                                  cmnQueryNLSModuleHandle(FALSE),
                                  ID_XLM_CLASS_SEL); */
    BOOL        fAllowDeregister = FALSE,
                fAllowReplace = FALSE,
                fAllowUnreplace = FALSE,
                fAllowCreate = TRUE;

    if (pscd->preccSource->pwps)
    {
        BOOL fIsWPSClass = TRUE;        // V0.9.5 (2000-08-26) [umoeller]
                            // make TRUE the default to allow deregistering
                            // orphaned WPS classes

        if (pscd->preccSource->pwps->pClassObject)
            fIsWPSClass = _somDescendedFrom(pscd->preccSource->pwps->pClassObject, _WPObject);

        fAllowDeregister = TRUE;
        fAllowReplace = TRUE;
        fAllowUnreplace = TRUE;
        fAllowCreate = TRUE;

        // allow deregistering?
        if (   (!pscd->preccSource->pwps->pszModName)
                       // DLL == NULL: not in WPS class list,
                       // so we better not allow touching this
            || (pscd->preccSource->pwps->pszReplacesClass)
            || (pscd->preccSource->pwps->pszReplacedWithClasses)
                       // or replacement class;
                       // the user should undo the replacement first
            || (!fIsWPSClass)
                       // no WPS class:
           )
        {
            fAllowDeregister = FALSE;
        }

        if (pszClassInfo)
        {
            CHAR szDummy[2000];

            ULONG flSelected = 0;
            // parse class info text whether this
            // class may be deregistered
            if (ParseDescription(pszClassInfo,
                                 pscd->preccSource->pwps->pszClassName,
                                 &flSelected,
                                 szDummy,
                                 sizeof(szDummy)))
            {
                PMPF_CLASSLIST(("flSelected: 0x%lX", flSelected));

                // bit 0 signifies whether this class may
                // be deregistered
                if (!(flSelected & 1))
                    fAllowDeregister = FALSE;

                // bit 2 signifies whether this class
                // can have instances created from it
                if (flSelected & 4)
                                // bit set
                    fAllowCreate = FALSE;

                PMPF_CLASSLIST(("1: fAllowCreate == %d", fAllowCreate));
            }
        }

        if (   (!pscd->preccSource->pwps->pszModName)
            || (pscd->preccSource->pwps->pszReplacesClass)
            || (!pscd->preccSource->pwps->pClassObject) // class object invalid?
            || (pscd->preccSource->pwps->pszReplacesClass)
                       // or replacement class;
                       // we'll only allow creation
                       // of objects of the _replaced_
                       // class
            || (!fIsWPSClass)
                       // no WPS class:
           )
        {
            fAllowCreate = FALSE;
            PMPF_CLASSLIST(("2: fAllowCreate == %d", fAllowCreate));
            PMPF_CLASSLIST(("    pszModName: %s", STRINGORNULL(pscd->preccSource->pwps->pszModName)));
            PMPF_CLASSLIST(("    pszReplacesClass: %s", STRINGORNULL(pscd->preccSource->pwps->pszReplacesClass)));
            PMPF_CLASSLIST(("    pClassObject: 0x%lX", pscd->preccSource->pwps->pClassObject));
            PMPF_CLASSLIST(("    pszReplacesClass: %s", STRINGORNULL(pscd->preccSource->pwps->pszReplacesClass)));
            PMPF_CLASSLIST(("    fIsWPSClass: %d", fIsWPSClass));
        }

        // allow replacements only if the
        // class has subclasses
        lrc2 = (LONG)WinSendMsg(pscd->hwndCnr,
                                CM_QUERYRECORD,
                                (MPARAM)pscd->preccSource,
                                MPFROM2SHORT(CMA_FIRSTCHILD,
                                             CMA_ITEMORDER));
        if (    (lrc2 == 0)
             || (lrc2 == -1)
                // disallow if the class replaces something itself
             || (pscd->preccSource->pwps->pszReplacesClass)
             || (!fIsWPSClass)
           )
            fAllowReplace = FALSE;

        // allow un-replacement only if the class
        // replaces another class
        if (    (pscd->preccSource->pwps->pszReplacesClass == NULL)
             || (!fIsWPSClass)
           )
            fAllowUnreplace = FALSE;

    } // end if (pscd->preccSource->pwps)

    PMPF_CLASSLIST(("3: fAllowCreate == %d", fAllowCreate));

    WinEnableMenuItem(hPopupMenu, ID_XLMI_DEREGISTER, fAllowDeregister);
    WinEnableMenuItem(hPopupMenu, ID_XLMI_REPLACE, fAllowReplace);
    WinEnableMenuItem(hPopupMenu, ID_XLMI_UNREPLACE, fAllowUnreplace);
    WinEnableMenuItem(hPopupMenu, ID_XLMI_CREATEOBJECT, fAllowCreate); // V0.9.6 (2000-10-16) [umoeller]

    cnrhShowContextMenu(pscd->hwndCnr,
                        (PRECORDCORE)(pscd->preccSource),
                        hPopupMenu,
                        hwndDlg);
}

BOOL fFillingCnr = FALSE;

/*
 *@@ fnwpClassTreeCnrDlg:
 *      this is the window proc for the dialog window
 *      with the actual WPS class list container (in Tree view).
 *      This dlg is a child of the abstract "split window"
 *      and has in turn one child, the container window.
 *      See fnwpClassListClient for a window hierarchy.
 *
 *      This calls in clsWpsClasses2Cnr in classes.c to
 *      have the WPS class tree inserted, which in turn
 *      calls the fncbReturnWPSClassAttr and fncbReplaceClassSelected
 *      callbacks above for configuration.
 *
 *      This is a sizeable dialog. Since dialogs do not
 *      receive WM_SIZE, we need to handle WM_WINDOWPOSCHANGED
 *      instead. This gets sent to us when the "split window"
 *      calls WinSetWindowPos when the frame window has been
 *      resized.
 *
 *      Also, we support container owner-draw here for the
 *      "disabled" (i.e. replaced) classes.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.5 (2000-08-26) [umoeller]: fixed WM_SYSCOMMAND handling
 *@@changed V0.9.14 (2001-07-31) [umoeller]: extra confirmation for XWP* classes was missing, fixed
 *@@changed V0.9.16 (2001-10-23) [umoeller]: another confirmation was missing, fixed
 */

STATIC MRESULT EXPENTRY fnwpClassTreeCnrDlg(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = MPNULL;

    PCLASSLISTTREECNRDATA pClassTreeCnrData = (PCLASSLISTTREECNRDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);

    TRY_LOUD(excpt1)
    {
        switch(msg)
        {

            /*
             * WM_INITDLG:
             *      set up the container data and colors.
             *      mp2 (pCreateParam) points to the frame's
             *      CLASSLISTCLIENTDATA structure.
             */

            case WM_INITDLG:
            {
                // create a structure for the functions in
                // classlst.c, which we'll store in the client's
                // window words
                PSELECTCLASSDATA pscd = (PSELECTCLASSDATA)malloc(sizeof(SELECTCLASSDATA));
                memset(pscd, 0, sizeof(SELECTCLASSDATA));

                // create our own structure for QWL_USER
                pClassTreeCnrData = malloc(sizeof(CLASSLISTTREECNRDATA));
                memset(pClassTreeCnrData, 0, sizeof(CLASSLISTTREECNRDATA));
                WinSetWindowULong(hwndDlg, QWL_USER, (ULONG)pClassTreeCnrData);

                // store the client data (create param from mp2)
                pClassTreeCnrData->pClientData = (PCLASSLISTCLIENTDATA)mp2;

                pClassTreeCnrData->pClientData->pscd = pscd;

                // setup container child
                pscd->hwndCnr = WinWindowFromID(hwndDlg, ID_XLDI_CNR);
                BEGIN_CNRINFO()
                {
                    cnrhSetView(CV_TREE | CA_TREELINE | CV_TEXT
                                    | CA_OWNERDRAW);
                    cnrhSetTreeIndent(30);
                    cnrhSetSortFunc(fnCompareName);
                } END_CNRINFO(pscd->hwndCnr);

                // fill container later
                fFillingCnr = FALSE;

                // initialize XADJUSTCTRLS structure
                winhAdjustControls(hwndDlg,             // dialog
                                   ampClassCnrCtls,    // MPARAMs array
                                   sizeof(ampClassCnrCtls) / sizeof(MPARAM), // items count
                                   NULL,                // pswpNew == NULL: initialize
                                   &pClassTreeCnrData->xacClassCnr);  // storage area

                mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            }
            break;

            /*
             * WM_DRAWITEM:
             *      container control owner draw: the
             *      cnr only allows the same color for
             *      all text items, so we need to draw
             *      the text ourselves
             */

            case WM_DRAWITEM:
                if ((USHORT)mp1 == ID_XLDI_CNR)
                    mrc = cnrhOwnerDrawRecord(mp2,
                                              CODFL_DISABLEDTEXT);
                else
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            break;

            /*
             * WM_WINDOWPOSCHANGED:
             *      posted _after_ the window has been moved
             *      or resized.
             *      Since we have a sizeable dlg, we need to
             *      update the controls' sizes also, or PM
             *      will display garbage. This is the trick
             *      how to use sizeable dlgs, because these do
             *      _not_ get sent WM_SIZE messages.
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
                    if (pClassTreeCnrData)
                    {
                        winhAdjustControls(hwndDlg,             // dialog
                                           ampClassCnrCtls,    // MPARAMs array
                                           sizeof(ampClassCnrCtls) / sizeof(MPARAM), // items count
                                           pswpNew,             // mp1
                                           &pClassTreeCnrData->xacClassCnr);  // storage area
                    }
                }
                mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            }
            break;

            /*
             * WM_FILLCNR:
             *      this fills the cnr with all the WPS classes.
             *      This uses the generic (and complex, I admit)
             *      functions in classlst.c, which can take
             *      any container window and fill it up with
             *      WPS classes.
             */

            case WM_FILLCNR:
            {
                CHAR szClassInfoFile[CCHMAXPATH];

                HPOINTER hptrOld = winhSetWaitPointer();

                // pscd is the SELECTCLASSDATA structure in our window words;
                // this is used for communicating with the classlist functions
                PSELECTCLASSDATA pscd = pClassTreeCnrData->pClientData->pscd;

                fFillingCnr = TRUE;

                // class to start with: either WPObject or SOMObject,
                // depending on instance data
                pscd->pcszRootClass =
                    (pClassTreeCnrData->pClientData->somThis->fShowSOMObject)
                        ? "SOMObject"
                        : G_pcszWPObject;
                // class to select: the same
                strlcpy(pscd->szClassSelected,
                        pscd->pcszRootClass,
                        sizeof(pscd->szClassSelected));

                pscd->pfnwpReturnAttrForClass = fncbReturnWPSClassAttr;
                    // callback for cnr recc attributes
                pscd->pfnwpClassSelected = NULL;
                    // we don't need no callback for class selections

                // prepare class info text file path
                if (cmnQueryXWPBasePath(szClassInfoFile))
                {
                    sprintf(szClassInfoFile+strlen(szClassInfoFile),
                            "\\help\\xfcls%s.txt",
                            cmnQueryLanguageCode());
                    doshLoadTextFile(szClassInfoFile,
                                     &pszClassInfo,
                                     NULL);
                }
                else
                    return NULL;

                // add orphans; this is done by setting the title
                // for the "Orphans" recc tree
                pscd->pcszOrphans = cmnGetString(ID_XSSI_WPSCLASSORPHANS);  // pszWpsClassOrphans

                // finally, fill container with WPS data (classlst.c)
                pscd->pwpsc = clsWpsClasses2Cnr(pscd->hwndCnr,
                                                pscd->pcszRootClass,
                                                pscd);  // also contains callback

                fFillingCnr = FALSE;
                WinSetPointer(HWND_DESKTOP, hptrOld);
            }
            break;

            /*
             * WM_CONTROL:
             *      capture cnr notifications
             */

            case WM_CONTROL:

                if (SHORT1FROMMP(mp1) == ID_XLDI_CNR)
                {

                    // pscd is the SELECTCLASSDATA structure in our window words;
                    // this is used for communicating with the classlist functions
                    PSELECTCLASSDATA pscd = pClassTreeCnrData->pClientData->pscd;

                    switch (SHORT2FROMMP(mp1)) // notify code
                    {
                        /*
                         * CN_EXPANDTREE:
                         *      tree view has been expanded:
                         *      do cnr auto-scroll if we're
                         *      not initially filling the cnr
                         */

                        case CN_EXPANDTREE:
                        {
                            if (!fFillingCnr)
                            {
                                if (cmnQuerySetting(sfTreeViewAutoScroll))
                                {
                                    pscd->preccExpanded = (PRECORDCORE)mp2;
                                    WinStartTimer(WinQueryAnchorBlock(hwndDlg),
                                                  hwndDlg,
                                                  1,
                                                  100);
                                }
                            }
                            mrc = MPNULL;
                        }
                        break;

                        /*
                         * CN_CONTEXTMENU:
                         *      context menu requested
                         */

                        case CN_CONTEXTMENU:
                        {
                            pscd->preccSource = (PCLASSRECORDCORE)mp2;
                            if (pscd->preccSource)
                            {
                                // we have a selection:
                                ShowClassContextMenu(hwndDlg,
                                                     pscd,
                                                     pClassTreeCnrData->pClientData->hmenuClassPopup);
                            }
                            else
                            {
                                // no selection: use default
                                // "open view" context menu
                                POINTL ptl = { 0, 0 };
                                XWPClassListData *somThis
                                    = XWPClassListGetData(pClassTreeCnrData->pClientData->somSelf);
                                // set flag for wpModifyPoupMenu
                                _fMenuCnrWhitespace = TRUE;

                                WinQueryPointerPos(HWND_DESKTOP,
                                                   &ptl);
                                // convert from screen to cnr coords
                                WinMapWindowPoints(HWND_DESKTOP,
                                                   pscd->hwndCnr,
                                                   &ptl,
                                                   2);
                                _wpDisplayMenu(pClassTreeCnrData->pClientData->somSelf,
                                               pClassTreeCnrData->pClientData->clvi.ViewItem.handle,
                                                        // owner: the frame
                                               pscd->hwndCnr,
                                                        // client: the tree cnr
                                               &ptl,
                                               MENU_OPENVIEWPOPUP,
                                               0);
                            }
                        }
                        break;

                        /*
                         * CN_EMPHASIS:
                         *      selection changed: call NewClassSelected
                         */

                        case CN_EMPHASIS:
                        {
                            // get cnr notification struct
                            PNOTIFYRECORDEMPHASIS pnre = (PNOTIFYRECORDEMPHASIS)mp2;

                            if (pnre)
                                if (    (pnre->fEmphasisMask & CRA_SELECTED)
                                     && (pnre->pRecord)
                                   )
                                {
                                    if (     (PCLASSRECORDCORE)(pnre->pRecord)
                                          != pscd->preccSelection
                                       )
                                    {
                                        // store currently selected class record
                                        // in SELECTCLASSDATA
                                        pscd->preccSelection = (PCLASSRECORDCORE)(pnre->pRecord);

                                        StartMethodsUpdateTimer(pClassTreeCnrData->pClientData);
                                    }
                                }
                        }
                        break;

                        /*
                         * CN_HELP:
                         *
                         */

                        case CN_HELP:
                            WinPostMsg(pClassTreeCnrData->pClientData->hwndClient,
                                       WM_HELP, 0, 0);
                        break;
                    }
                } // end if (SHORT1FROMMP(mp1) == ID_XLDI_CNR) {
            break;

            /*
             * WM_TIMER:
             *      we have two timers here.
             *      -- ID 1: timer for tree view auto-scroll (we don't
             *               use the Worker thread here).
             *      -- ID 2: timer for delayed window updates when a new
             *               class record gets selected. With a delay
             *               of 100 ms, we update the other windows.
             */

            case WM_TIMER:
                // stop timer (it's just for one shot)
                WinStopTimer(WinQueryAnchorBlock(hwndDlg), hwndDlg,
                             (ULONG)mp1);       // timer ID

                switch ((ULONG)mp1)        // timer ID
                {
                    /*
                     * Timer 1:
                     *      container auto-scroll timer.
                     */

                    case 1:
                    {
                        // pscd is the SELECTCLASSDATA structure in our window words;
                        // this is used for communicating with the classlist functions
                        PSELECTCLASSDATA pscd = pClassTreeCnrData->pClientData->pscd;

                        if ( ( pscd->preccExpanded->flRecordAttr & CRA_EXPANDED) != 0 )
                        {
                            PRECORDCORE     preccLastChild;
                            // scroll the tree view properly
                            preccLastChild = WinSendMsg(pscd->hwndCnr,
                                                        CM_QUERYRECORD,
                                                        pscd->preccExpanded,
                                                                // expanded PRECORDCORE from CN_EXPANDTREE
                                                        MPFROM2SHORT(CMA_LASTCHILD,
                                                                     CMA_ITEMORDER));
                            if (preccLastChild)
                            {
                                // ULONG ulrc;
                                cnrhScrollToRecord(pscd->hwndCnr,
                                        (PRECORDCORE)preccLastChild,
                                        CMA_TEXT,   // record text rectangle only
                                        TRUE);      // keep parent visible
                            }
                        }
                    }
                    break;

                    /*
                     * Timer 2:
                     *      update methods info timer.
                     */

                    case 2:
                        pClassTreeCnrData->pClientData->ulUpdateTimerID = 0;
                        // update other windows with class info
                        NewClassSelected(pClassTreeCnrData->pClientData);
                    break;
                }
            break;

            /*
             * WM_COMMAND:
             *      this handles the commands from the context menu.
             */

            case WM_COMMAND:
            {
                // pscd is the SELECTCLASSDATA structure in our window words;
                // this is used for communicating with the classlist functions
                PSELECTCLASSDATA pscd = pClassTreeCnrData->pClientData->pscd;

                switch (SHORT1FROMMP(mp1))  // menu command
                {
                    // "Help" menu command
                    case ID_XFMI_HELP:
                        WinPostMsg(pClassTreeCnrData->pClientData->hwndClient,
                                   WM_HELP, 0, 0);
                    break;

                    // "Deregister" menu command:
                    case ID_XLMI_DEREGISTER:
                    {
                        if (pscd->preccSource)
                            if (pscd->preccSource->pwps)
                            {
                                BOOL fAllow = FALSE;
                                CHAR szTemp[CCHMAXPATH];
                                PCSZ pTable[2];
                                pTable[0] = szTemp;
                                pTable[1] = pscd->preccSource->pwps->pszReplacedWithClasses;

                                strlcpy(szTemp,
                                        pscd->preccSource->pwps->pszClassName,
                                        sizeof(szTemp));
                                    // save for later

                                // do not allow deregistering if the class is currently
                                // replaced by another class
                                if (pscd->preccSource->pwps->pszReplacedWithClasses)
                                {
                                    // show warning
                                    cmnMessageBoxExt(hwndDlg,
                                                116,
                                                pTable, 2, 139,
                                                MB_OK);
                                    // and stop
                                    break;
                                }

                                if (    (!strncmp(pscd->preccSource->pwps->pszClassName,
                                                 "XFld", 4))
                                     || (!strncmp(pscd->preccSource->pwps->pszClassName,
                                                 "XWP", 4))
                                        // V0.9.14 (2001-07-31) [umoeller]
                                   )
                                {
                                    // XFolder class
                                    if (cmnMessageBoxExt(hwndDlg,
                                                116,
                                                pTable, 1, 120,
                                                MB_YESNO | MB_DEFBUTTON2)
                                            == MBID_YES)
                                        fAllow = TRUE;
                                } else
                                    if (cmnMessageBoxExt(hwndDlg,
                                                116,
                                                pTable, 1, 118,
                                                MB_YESNO | MB_DEFBUTTON2)
                                            == MBID_YES)
                                        fAllow = TRUE;

                                if (fAllow)
                                {
                                    if (WinDeregisterObjectClass(pscd->preccSource->pwps->pszClassName))
                                    {
                                        // success
                                        WinSendMsg(pscd->hwndCnr,
                                                   CM_REMOVERECORD,
                                                   (MPARAM)&(pscd->preccSource),
                                                   MPFROM2SHORT(1, // remove one record
                                                           CMA_FREE | CMA_INVALIDATE));

                                        lstRemoveItem(pscd->pwpsc->pllClassList,
                                                      pscd->preccSource->pwps);
                                                        // remove item from list

                                        // free(pscd->pRecordSelected->pwps);

                                        cmnMessageBoxExt(hwndDlg,
                                                121,
                                                pTable, 1, 122,
                                                MB_OK);
                                    } else
                                        // error
                                        cmnMessageBoxExt(hwndDlg,
                                                104,
                                                pTable, 1, 119,
                                                MB_OK);
                                }
                            }
                    }
                    break;

                    // "Replace class" menu command:
                    // show yet another WPS classes dlg
                    case ID_XLMI_REPLACE:
                    {
                        if (pscd->preccSource)
                        {
                            SELECTCLASSDATA         scd;
                            PCSZ                     pszClassName =
                                        pscd->preccSource->pwps->pszClassName;

                            XSTRING strTitle,
                                    strIntroText;
                            xstrInit(&strTitle, 0);
                            xstrInit(&strIntroText, 0);

                            cmnGetMessage(NULL, 0,
                                          &strTitle,
                                          112);
                            // replace "%1" with class name which is to be
                            // replaced
                            cmnGetMessage(&pszClassName, 1,
                                          &strIntroText, 123);
                            scd.pszDlgTitle = strTitle.psz;
                            scd.pszIntroText = strIntroText.psz;
                            scd.pcszRootClass = pszClassName;
                            scd.pcszOrphans = NULL;
                            strlcpy(scd.szClassSelected,
                                    scd.pcszRootClass,
                                    sizeof(scd.szClassSelected));

                            scd.pfnwpReturnAttrForClass = NULL; // fncbStatusBarReturnClassAttr;
                            scd.pfnwpClassSelected = fncbReplaceClassSelected;
                            scd.ulUserClassSelected = 0; //(ULONG)&sbsc;

                            scd.pszHelpLibrary = cmnQueryHelpLibrary();
                            scd.ulHelpPanel = 0;

                            if (clsSelectWpsClassDlg(hwndDlg,
                                                     cmnQueryNLSModuleHandle(FALSE),
                                                     ID_XLD_SELECTCLASS,
                                                     &scd) == DID_OK)
                            {
                                PCSZ pTable[2];
                                pTable[0] = pscd->preccSource->pwps->pszClassName;
                                pTable[1] = scd.szClassSelected;
                                if (cmnMessageBoxExt(hwndDlg,
                                            116,
                                            pTable, 2, 124,
                                            MB_YESNO | MB_DEFBUTTON2)
                                        == MBID_YES)
                                    if (WinReplaceObjectClass(
                                            pscd->preccSource->pwps->pszClassName,
                                            scd.szClassSelected,
                                            TRUE))
                                        // success
                                        cmnMessageBoxExt(hwndDlg,
                                                121,
                                                pTable, 2, 129,
                                                MB_OK);
                                    else
                                        // error
                                        cmnMessageBoxExt(hwndDlg,
                                                104,
                                                pTable, 2, 130,
                                                MB_OK);
                            }

                            xstrClear(&strTitle);
                            xstrClear(&strIntroText);
                        }
                    }
                    break;

                    // "Unreplace class" menu command
                    case ID_XLMI_UNREPLACE:
                    {
                        if (pscd->preccSource)
                            if (pscd->preccSource->pwps)
                            {
                                BOOL fAllow = FALSE;
                                PCSZ pTable[2];
                                pTable[0] = pscd->preccSource->pwps->pszReplacesClass;
                                pTable[1] = pscd->preccSource->pwps->pszClassName;

                                if (    (!strncmp(pscd->preccSource->pwps->pszClassName,
                                                "XFld", 4))
                                     || (!strncmp(pscd->preccSource->pwps->pszClassName,
                                                 "XWP", 4))
                                            // was missing V0.9.16 (2001-10-23) [umoeller]
                                   )
                                {
                                    // some XFolder class
                                    if (cmnMessageBoxExt(hwndDlg,
                                                116,
                                                pTable, 2, 125,
                                                MB_YESNO | MB_DEFBUTTON2)
                                            == MBID_YES)
                                        fAllow = TRUE;
                                }
                                else
                                    if (cmnMessageBoxExt(hwndDlg,
                                                116,
                                                pTable, 2, 126,
                                                MB_YESNO | MB_DEFBUTTON2)
                                            == MBID_YES)
                                        fAllow = TRUE;

                                if (fAllow)
                                {
                                    if (WinReplaceObjectClass(
                                            pscd->preccSource->pwps->pszReplacesClass,
                                            pscd->preccSource->pwps->pszClassName,
                                            FALSE))
                                        // success
                                        cmnMessageBoxExt(hwndDlg,
                                                121,
                                                pTable, 2, 127,
                                                MB_OK);
                                    else
                                        // error
                                        cmnMessageBoxExt(hwndDlg,
                                                104,
                                                pTable, 2, 128,
                                                MB_OK);
                                }
                            }
                    }
                    break;

                    // "Create object" menu command
                    // (new with V0.9.0)
                    case ID_XLMI_CREATEOBJECT:
                    {
                        if (pscd->preccSource)
                            if (pscd->preccSource->pwps)
                            {
                                // BOOL fAllow = FALSE;
                                PCSZ pTable[2];
                                pTable[0] = pscd->preccSource->pwps->pszClassName;
                                pTable[1] = _wpclsQueryTitle(pscd->preccSource->pwps->pClassObject);

                                if (cmnMessageBoxExt(hwndDlg,
                                            116,
                                            pTable, 2, 141,
                                            MB_YESNO | MB_DEFBUTTON2)
                                        == MBID_YES)
                                {
                                    WinCreateObject(pscd->preccSource->pwps->pszClassName,
                                                    _wpclsQueryTitle(pscd->preccSource->pwps->pClassObject),
                                                    "",        // setup string
                                                    (PSZ)WPOBJID_DESKTOP, // "<WP_DESKTOP>", // location
                                                    CO_FAILIFEXISTS);
                                }

                            }
                    }
                    break;

                } // end switch (SHORT1FROMMP(mp1))
                pscd->preccSource = NULL;
            }
            break;

            /*
             * WM_MENUEND:
             *      if the context menu is dismissed, we'll need
             *      to remove the cnr source emphasis which was
             *      set above when showing the context menu.
             */

            case WM_MENUEND:
            {
                // pscd is the SELECTCLASSDATA structure in our window words;
                // this is used for communicating with the classlist functions
                PSELECTCLASSDATA pscd = pClassTreeCnrData->pClientData->pscd;
                cnrhSetSourceEmphasis(pscd->hwndCnr,
                                      pscd->preccSource,
                                      FALSE);
            }
            break;

            /*
             * WM_SYSCOMMAND:
             *      pass on to frame
             */

            case WM_SYSCOMMAND:
                WinPostMsg(WinQueryWindow(pClassTreeCnrData->pClientData->hwndClient,
                                          QW_PARENT),
                           msg, mp1, mp2);
            break;

            /*
             * WM_HELP:
             *      pass on to client
             */

            case WM_HELP:
                WinPostMsg(pClassTreeCnrData->pClientData->hwndClient, msg, mp1, mp2);
            break;

            /*
             * WM_DESTROY:
             *      clean up big time
             */

            case WM_DESTROY:
            {
                HPOINTER hptrOld = winhSetWaitPointer();
                // free cnr records
                WinSendMsg(WinWindowFromID(hwndDlg, ID_XLDI_CNR),
                           CM_REMOVERECORD,
                           (MPARAM)NULL,
                           MPFROM2SHORT(0, // remove all records
                                   CMA_FREE | CMA_INVALIDATE));

                // cleanup allocated WPS data (classlst.c)
                clsCleanupWpsClasses(pClassTreeCnrData->pClientData->pscd->pwpsc);
                free(pClassTreeCnrData->pClientData->pscd);

                free(pszClassInfo);
                pszClassInfo = NULL;

                // clean up window positions
                winhAdjustControls(hwndDlg,             // dialog
                                   NULL,                // clean up
                                   0,                   // items count
                                   NULL,                // clean up
                                   &pClassTreeCnrData->xacClassCnr); // storage area

                free(pClassTreeCnrData);

                mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                WinSetPointer(HWND_DESKTOP, hptrOld);
            }
            break;

            default:
                mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        }
    }
    CATCH(excpt1)
    {
        mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    } END_CATCH();

    return mrc;
}

/*
 *@@ fnwpClassInfoDlg:
 *      this is the window proc for the dialog window
 *      with all the different class info.
 *      This dlg is the second child of the abstract "split window"
 *      and has many more subcontrols for the class info.
 *      See fnwpClassListClient for a window hierarchy.
 *
 *      This is a sizeable dialog. Since dialogs do not
 *      receive WM_SIZE, we need to handle WM_WINDOWPOSCHANGED
 *      instead. This gets sent to us when the "split window"
 *      calls WinSetWindowPos when the frame window has been
 *      resized.
 *
 *      This function does not update the class info controls.
 *      This is done in NewClassSelected only, which gets called
 *      from fnwpClassListDlg.
 *      We only need this function to be able to resize that
 *      dialog window and reposition the subcontrols.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.5 (2000-08-26) [umoeller]: fixed WM_SYSCOMMAND handling
 */

STATIC MRESULT EXPENTRY fnwpClassInfoDlg(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    PCLASSLISTINFODATA pClassInfoData
            = (PCLASSLISTINFODATA)WinQueryWindowPtr(hwndDlg, QWL_USER);

    switch (msg)
    {

        /*
         * WM_INITDLG:
         *      mp2 (pCreateParam) points to the frame's
         *      CLASSLISTCLIENTDATA structure.
         */

        case WM_INITDLG:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            ctlPrepareStaticIcon(WinWindowFromID(hwndDlg, ID_XLDI_ICON), 1);
                // even though we have no animation, we use
                // the animation funcs to subclass the icon control;
                // otherwise we'll get garbage with transparent
                // class icons

            // create our own structure for QWL_USER
            pClassInfoData = malloc(sizeof(CLASSLISTINFODATA));
            memset(pClassInfoData, 0, sizeof(CLASSLISTINFODATA));
            WinSetWindowULong(hwndDlg, QWL_USER, (ULONG)pClassInfoData);

            // store the client data (create param from mp2)
            pClassInfoData->pClientData = (PCLASSLISTCLIENTDATA)mp2;

            // initialize XADJUSTCTRLS structure
            winhAdjustControls(hwndDlg,             // dialog
                               ampClassInfoCtls,    // MPARAMs array
                               sizeof(ampClassInfoCtls) / sizeof(MPARAM), // items count
                               NULL,                // pswpNew == NULL: initialize
                               &pClassInfoData->xacClassInfo);  // storage area
        break;

        /*
         * WM_WINDOWPOSCHANGED:
         *      posted _after_ the window has been moved
         *      or resized.
         *      Since we have a sizeable dlg, we need to
         *      update the controls' sizes also, or PM
         *      will display garbage. This is the trick
         *      how to use sizeable dlgs, because these do
         *      _not_ get sent WM_SIZE messages.
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
                if (pClassInfoData)
                {
                    winhAdjustControls(hwndDlg,             // dialog
                                       ampClassInfoCtls,    // MPARAMs array
                                       sizeof(ampClassInfoCtls) / sizeof(MPARAM), // items count
                                       pswpNew,             // mp1
                                       &pClassInfoData->xacClassInfo);  // storage area
                }
            }
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        }
        break;

        /*
         * WM_SYSCOMMAND:
         *      pass on to frame
         */

        case WM_SYSCOMMAND:
            WinPostMsg(WinQueryWindow(pClassInfoData->pClientData->hwndClient,
                                      QW_PARENT),
                       msg, mp1, mp2);
        break;

        /*
         * WM_HELP:
         *      pass on to client
         */

        case WM_HELP:
            WinPostMsg(pClassInfoData->pClientData->hwndClient, msg, mp1, mp2);
        break;

        /*
         * WM_DESTROY:
         *      clean up.
         */

        case WM_DESTROY:
            // clean up window positions
            winhAdjustControls(hwndDlg,             // dialog
                               NULL,                // clean up
                               0,                   // items count
                               NULL,                // clean up
                               &pClassInfoData->xacClassInfo); // storage area
            free(pClassInfoData);
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;
    }

    return mrc;
}

/*
 *@@ fnwpMethodInfoDlg:
 *
 *@@changed V0.9.1 (99-12-20) [umoeller]: now using fntMethodCollectThread for method infos
 *@@changed V0.9.5 (2000-08-26) [umoeller]: fixed WM_SYSCOMMAND handling
 *@@changed V0.9.6 (2000-10-16) [umoeller]: added token, added static/dynamic
 */

STATIC MRESULT EXPENTRY fnwpMethodInfoDlg(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = MPNULL;

    PCLASSLISTMETHODDATA pMethodInfoData = (PCLASSLISTMETHODDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);

    switch(msg)
    {

        /*
         * WM_INITDLG:
         *      set up the container data and colors.
         *      mp2 (pCreateParam) points to the frame's
         *      CLASSLISTWINDOWDATA structure.
         */

        case WM_INITDLG:
        {
            // PNLSSTRINGS     pNLSStrings = cmnQueryNLSStrings();
            XFIELDINFO      xfi[7];
            PFIELDINFO      pfi = NULL;
            HWND            hwndCnr = WinWindowFromID(hwndDlg, ID_XLDI_CNR);
            int             i = 0;
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);

            // create our own structure for QWL_USER
            pMethodInfoData = (PCLASSLISTMETHODDATA)malloc(sizeof(CLASSLISTMETHODDATA));
            memset(pMethodInfoData, 0, sizeof(CLASSLISTMETHODDATA));
            WinSetWindowULong(hwndDlg, QWL_USER, (ULONG)pMethodInfoData);

            // store the client data (create param from mp2)
            pMethodInfoData->pClientData = (PCLASSLISTCLIENTDATA)mp2;

            // initialize XADJUSTCTRLS structure
            winhAdjustControls(hwndDlg,             // dialog
                               ampMethodInfoCtls,    // MPARAMs array
                               sizeof(ampMethodInfoCtls) / sizeof(MPARAM), // items count
                               NULL,                // pswpNew == NULL: initialize
                               &pMethodInfoData->xacMethodInfo);  // storage area

            // set up cnr details view
            xfi[i].ulFieldOffset = FIELDOFFSET(METHODRECORD, ulMethodIndex);
            xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_CLSLIST_INDEX);  // pszClsListIndex
            xfi[i].ulDataType = CFA_ULONG;
            xfi[i++].ulOrientation = CFA_RIGHT;

            xfi[i].ulFieldOffset = FIELDOFFSET(RECORDCORE, pszIcon);
            xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_CLSLIST_METHOD);  // pszClsListMethod
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT;

            xfi[i].ulFieldOffset = FIELDOFFSET(METHODRECORD, pszToken);
            xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_CLSLISTTOKEN);  // pszClsListToken
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT;

            xfi[i].ulFieldOffset = FIELDOFFSET(METHODRECORD, pszMethodProc);
            xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_CLSLIST_ADDRESS);  // pszClsListAddress
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT;

            xfi[i].ulFieldOffset = FIELDOFFSET(METHODRECORD, pszIntroducedBy);
            xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_CLSLIST_CLASS);  // pszClsListClass
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT;

            xfi[i].ulFieldOffset = FIELDOFFSET(METHODRECORD, pszOverriddenBy2);
            xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_CLSLIST_OVERRIDDENBY);  // pszClsListOverriddenBy
            xfi[i].ulDataType = CFA_STRING;
            xfi[i++].ulOrientation = CFA_LEFT;

            pfi = cnrhSetFieldInfos(hwndCnr,
                                    &xfi[0],
                                    i,             // array item count
                                    TRUE,          // no draw lines
                                    3);            // return second column

            BEGIN_CNRINFO()
            {
                cnrhSetView(CV_DETAIL | CA_DETAILSVIEWTITLES);
                cnrhSetSplitBarAfter(pfi);
                cnrhSetSplitBarPos(200);
                cnrhSetSortFunc(QueryMethodsSortFunc(pMethodInfoData->pClientData));
            } END_CNRINFO(hwndCnr);

            // check "instance methods"
            winhSetDlgItemChecked(hwndDlg,
                                  pMethodInfoData->pClientData->somThis->ulMethodsRadioB,
                                  TRUE);
        }
        break;

        /*
         * WM_WINDOWPOSCHANGED:
         *      posted _after_ the window has been moved
         *      or resized.
         *      Since we have a sizeable dlg, we need to
         *      update the controls' sizes also, or PM
         *      will display garbage. This is the trick
         *      how to use sizeable dlgs, because these do
         *      _not_ get sent WM_SIZE messages.
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
                if (pMethodInfoData)
                {
                    winhAdjustControls(hwndDlg,             // dialog
                                       ampMethodInfoCtls,    // MPARAMs array
                                       sizeof(ampMethodInfoCtls) / sizeof(MPARAM), // items count
                                       pswpNew,             // mp1
                                       &pMethodInfoData->xacMethodInfo);  // storage area
                }
            }
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        }
        break;

        /*
         * WM_FILLCNR:
         *      this gets posted from fntMethodCollectThread
         *      after the thread is done collecting the method
         *      info.
         *
         *      This has a newly allocated PMETHODINFO in mp1.
         */

        case WM_FILLCNR:
        {
            PMETHODINFO pMethodInfo = (PMETHODINFO)mp1;
            if (pMethodInfo)
            {
                HWND hwndMethodsCnr = WinWindowFromID(hwndDlg,
                                                      ID_XLDI_CNR);

                // allocate as many records as methods exist;
                // we get a linked list now
                PMETHODRECORD precc = (PMETHODRECORD)cnrhAllocRecords(
                                                    hwndMethodsCnr,
                                                    sizeof(METHODRECORD),
                                                    pMethodInfo->ulMethodCount),
                              preccThis = precc;
                PLISTNODE     pNode = lstQueryFirstNode(&pMethodInfo->llMethods);
                ULONG         ulIndex = 0;

                // now set up the METHODRECORD according
                // to the method data returned by clsQueryMethodInfo
                while ((preccThis) && (pNode))
                {
                    PSOMMETHOD  psm = (PSOMMETHOD)pNode->pItemData;
                    PLISTNODE   pnodeOverride;
                    XSTRING     strOverriddenBy;
                    xstrInit(&strOverriddenBy, 200);

                    // set up information
                    preccThis->ulMethodIndex = ulIndex;
                    preccThis->recc.pszIcon = psm->pszMethodName;

                    switch(psm->ulType)  // 0=static, 1=dynamic, 2=nonstatic
                    {
                        case 0: preccThis->pszType = "S"; break;
                        case 1: preccThis->pszType = "D"; break;
                        case 2: preccThis->pszType = "N"; break;
                        default: preccThis->pszType = "?"; break;
                    }

                    if (psm->pIntroducedBy)
                        preccThis->pszIntroducedBy = _somGetName(psm->pIntroducedBy);
                    preccThis->ulIntroducedBy = psm->ulIntroducedBy;

                    // go thru list of class objects which overrode this method
                    // and create a string from the class names
                    pnodeOverride = lstQueryFirstNode(&psm->llOverriddenBy);
                    while (pnodeOverride)
                    {
                        SOMClass *pClassObject = (SOMClass*)pnodeOverride->pItemData;

                        if (pClassObject)
                        {
                            if (strOverriddenBy.ulLength)
                                // not first class: append comma
                                xstrcat(&strOverriddenBy, ", ", 2);

                            xstrcat(&strOverriddenBy,
                                    _somGetName(pClassObject),
                                    0);
                        }

                        pnodeOverride = pnodeOverride->pNext;
                    }

                    if (strOverriddenBy.ulLength)
                        // we have overrides:
                        preccThis->pszOverriddenBy2 = strOverriddenBy.psz;

                    if (strOverriddenBy.psz)
                        // store this string in list of items to be freed later
                        lstAppendItem(&pMethodInfoData->pClientData->llCnrStrings,
                                      strOverriddenBy.psz);

                    preccThis->ulOverriddenBy = psm->ulOverriddenBy;

                    // add token (this is a PVOID really...)
                    sprintf(preccThis->szToken, "0x%lX",
                            psm->tok);
                    // and point PSZ to that buffer
                    preccThis->pszToken = preccThis->szToken;

                    // write method address to recc's string buffer
                    sprintf(preccThis->szMethodProc, "0x%lX (%s)",
                            psm->pMethodProc,
                            preccThis->pszType); // V1.0.4 (2005-04-08) [pr]
                    // and point PSZ to that buffer
                    preccThis->pszMethodProc = preccThis->szMethodProc;

                    // go for next
                    preccThis = (PMETHODRECORD)preccThis->recc.preccNextRecord;
                    pNode = pNode->pNext;
                    ulIndex++;
                }

                cnrhInsertRecords(hwndMethodsCnr,
                                  NULL,         // no parent record
                                  (PRECORDCORE)precc,        // first record
                                  TRUE, // invalidate
                                  NULL,         // no text
                                  CRA_RECORDREADONLY,
                                  pMethodInfo->ulMethodCount);
                                                // record count
                // store in main window data
                // so we can release that later
                pMethodInfoData->pClientData->pMethodInfo = pMethodInfo;
            } // end if (pMethodInfoData->pMethodInfo)
        }
        break;   // WM_FILLCNR

        /*
         * WM_CONTROL:
         *
         */

        case WM_CONTROL:
            switch (SHORT1FROMMP(mp1))  // usID
            {
                case ID_XLDI_RADIO_INSTANCEMETHODS:
                case ID_XLDI_RADIO_CLASSMETHODS:
                    if (    (SHORT2FROMMP(mp1) == BN_CLICKED)
                         || (SHORT2FROMMP(mp1) == BN_DBLCLICKED)
                       )
                        if (pMethodInfoData->pClientData->somThis->ulMethodsRadioB != SHORT1FROMMP(mp1))
                        {
                            // radio button selection changed:
                            pMethodInfoData->pClientData->somThis->ulMethodsRadioB = SHORT1FROMMP(mp1);
                            _wpSaveDeferred(pMethodInfoData->pClientData->somSelf);
                            // update method info
                            StartMethodsUpdateTimer(pMethodInfoData->pClientData);
                        }
                break;

                case ID_XLDI_CNR:
                    switch (SHORT2FROMMP(mp1)) // notify code
                    {
                        case CN_HELP:
                            WinPostMsg(pMethodInfoData->pClientData->hwndClient,
                                       WM_HELP, 0, 0);
                        break;

                        case CN_CONTEXTMENU:
                        {
                            HWND            hPopupMenu = 0;
                            pMethodInfoData->pMethodReccSource = (PMETHODRECORD)mp2;
                            if (pMethodInfoData->pMethodReccSource)
                            {
                                // we have a selection
                            }
                            else
                            {
                                static s_last_sort_id = 0;
                                // whitespace:
                                hPopupMenu = pMethodInfoData->pClientData->hmenuMethodsWhitespacePopup;

                                if (s_last_sort_id)
                                    WinCheckMenuItem(hPopupMenu,
                                                     s_last_sort_id,
                                                     FALSE);
                                // check current sort item
                                WinCheckMenuItem(hPopupMenu,
                                                 pMethodInfoData->pClientData->somThis->ulSortID,
                                                 TRUE);
                                s_last_sort_id = pMethodInfoData->pClientData->somThis->ulSortID;
                            }
                            if (hPopupMenu)
                                cnrhShowContextMenu(WinWindowFromID(hwndDlg, ID_XLDI_CNR),
                                                    (PRECORDCORE)pMethodInfoData->pMethodReccSource,
                                                    hPopupMenu,
                                                    hwndDlg);
                        }
                        break;
                    }
                break; // case ID_XLDI_CNR
            }
        break;

        /*
         * WM_COMMAND:
         *      this handles the commands from the context menu.
         */

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))  // menu command
            {
                // "Help" menu command
                case ID_XFMI_HELP:
                    WinPostMsg(pMethodInfoData->pClientData->hwndClient,
                               WM_HELP, 0, 0);
                break;

                // "Sort by" commands
                case ID_XLMI_METHOD_SORT_INDEX:
                case ID_XLMI_METHOD_SORT_NAME:
                case ID_XLMI_METHOD_SORT_INTRO:
                case ID_XLMI_METHOD_SORT_OVERRIDE:
                    // store sort item in window data for
                    // menu item checks above
                    pMethodInfoData->pClientData->somThis->ulSortID = SHORT1FROMMP(mp1);
                    _wpSaveDeferred(pMethodInfoData->pClientData->somSelf);
                    // update container's sort func
                    BEGIN_CNRINFO()
                    {
                        cnrhSetSortFunc(QueryMethodsSortFunc(pMethodInfoData->pClientData));
                    } END_CNRINFO(WinWindowFromID(hwndDlg, ID_XLDI_CNR));
                break;
            }
        break;

        /*
         * WM_MENUEND:
         *      if the context menu is dismissed, we'll need
         *      to remove the cnr source emphasis which was
         *      set above when showing the context menu.
         */

        case WM_MENUEND:
            cnrhSetSourceEmphasis(WinWindowFromID(hwndDlg, ID_XLDI_CNR),
                                  pMethodInfoData->pMethodReccSource,
                                  FALSE);
        break;

        /*
         * WM_SYSCOMMAND:
         *      pass on to frame
         */

        case WM_SYSCOMMAND:
            WinPostMsg(WinQueryWindow(pMethodInfoData->pClientData->hwndClient,
                                      QW_PARENT),
                       msg, mp1, mp2);
        break;

        /*
         * WM_HELP:
         *      pass on to client
         */

        case WM_HELP:
            WinPostMsg(pMethodInfoData->pClientData->hwndClient, msg, mp1, mp2);
        break;

        case WM_DESTROY:
            // clean up window positions
            winhAdjustControls(hwndDlg,             // dialog
                               NULL,                // clean up
                               0,                   // items count
                               NULL,                // clean up
                               &pMethodInfoData->xacMethodInfo); // storage area
            CleanupMethodsInfo(pMethodInfoData->pClientData);
            free(pMethodInfoData);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        break;
    }

    return mrc;
}

/* ******************************************************************
 *
 *   XWPClassList notebook callbacks (notebook.c)
 *
 ********************************************************************/

/*
 *@@ cllClassListInitPage:
 *      "Class list" page notebook callback function (notebook.c).
 *      Sets the controls on the page according to the instance
 *      settings.
 *
 */

VOID cllClassListInitPage(PNOTEBOOKPAGE pnbp,  // notebook info struct
                           ULONG flFlags)              // CBI_* flags (notebook.h)
{
    XWPClassListData *somThis = XWPClassListGetData(pnbp->inbp.somSelf);

    if (flFlags & CBI_INIT)
    {
        // first call: backup instance data for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = malloc(sizeof(XWPClassListData));
        memcpy(pnbp->pUser, somThis, sizeof(XWPClassListData));
    }

    if (flFlags & CBI_SET)
    {
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XLDI_SHOWSOMOBJECT,
                              _fShowSOMObject);
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XLDI_SHOWMETHODS,
                              _fShowMethods);
    }

    if (flFlags & CBI_ENABLE)
    {
    }
}

/*
 * cllClassListItemChanged:
 *      "XFolder" page notebook callback function (notebook.c).
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 */

MRESULT cllClassListItemChanged(PNOTEBOOKPAGE pnbp,
                                ULONG ulItemID,
                                USHORT usNotifyCode,
                                ULONG ulExtra)      // for checkboxes: contains new state
{
    XWPClassListData *somThis = XWPClassListGetData(pnbp->inbp.somSelf);
    BOOL    fUpdate = TRUE,
            fOldShowMethods = _fShowMethods;

    switch (ulItemID)
    {
        case ID_XLDI_SHOWSOMOBJECT:
            _fShowSOMObject = ulExtra;
        break;

        case ID_XLDI_SHOWMETHODS:
            _fShowMethods = ulExtra;
        break;

        case DID_UNDO:
            if (pnbp->pUser)
            {
                XWPClassListData *Backup = (pnbp->pUser);
                // "Undo" button: restore backed up instance data
                _fShowSOMObject = Backup->fShowSOMObject;
                _fShowMethods = Backup->fShowMethods;
                // have the page updated by calling the callback above
                cllClassListInitPage(pnbp, CBI_SHOW | CBI_ENABLE);
            }
        break;

        case DID_DEFAULT:
            // "Default" button:
            _fShowSOMObject = 0;
            _fShowMethods = 1;
            // have the page updated by calling the callback above
            cllClassListInitPage(pnbp, CBI_SHOW | CBI_ENABLE);
        break;

        default:
            fUpdate = FALSE;
        break;
    }

    if (fUpdate)
        _wpSaveDeferred(pnbp->inbp.somSelf);

    if (fOldShowMethods != _fShowMethods)
        if (_hwndOpenView)
        {
            // reformat open view:
            HWND hwndClient = WinWindowFromID(_hwndOpenView, FID_CLIENT);
            PCLASSLISTCLIENTDATA pClientData = WinQueryWindowPtr(hwndClient, QWL_USER);
            RelinkWindows(pClientData, TRUE);
        }

    return (MPARAM)0;
}

/*
 *@@ cllModifyPopupMenu:
 *      implementation for XWPClassList::wpModifyPopupMenu.
 *
 *      This gets called after a successful call to the
 *      WPAbstract parent method.
 *
 *@@added V0.9.1 (99-12-28) [umoeller]
 *@@changed V0.9.3 (2000-04-28) [umoeller]: "register class" was missing frequently; fixed
 */

BOOL cllModifyPopupMenu(XWPClassList *somSelf,
                        HWND hwndMenu,
                        HWND hwndCnr,
                        ULONG iPosition)
{
    XWPClassListData *somThis = XWPClassListGetData(somSelf);
    MENUITEM mi;
    // get handle to the "Open" submenu in the
    // the popup menu
    if (winhQueryMenuItem(hwndMenu,
                          WPMENUID_OPEN,
                          TRUE,
                          &mi))
    {
        // mi.hwndSubMenu now contains "Open" submenu handle,
        // which we add items to now
        winhInsertMenuItem(mi.hwndSubMenu, MIT_END,
                           *G_pulVarMenuOfs + ID_XFMI_OFS_XWPVIEW,
                           cmnGetString(ID_XFSI_OPENCLASSLIST),  // pszOpenClassList
                           MIS_TEXT, 0);
        // insert "register class" only if this is
        // for an open class list view
        if (_fMenuCnrWhitespace)
        {
            cmnInsertSeparator(hwndMenu, MIT_END);

            winhInsertMenuItem(hwndMenu, MIT_END,
                               ID_XLMI_REGISTER, // is above WPMENUID_USER
                               cmnGetString(ID_XFSI_REGISTERCLASS),  // pszRegisterClass
                               MIS_TEXT, 0);
            // "Refresh" V0.9.6 (2000-11-12) [umoeller]
            winhInsertMenuItem(hwndMenu, MIT_END,
                               ID_XLMI_REFRESH_VIEW,
                               cmnGetString(ID_XSSI_REFRESHNOW),  // pszRefreshNow
                               MIS_TEXT, 0);
            // "Find" V1.0.1 (2002-11-26) [jsmall]
            winhInsertMenuItem(hwndMenu, MIT_END,
                               ID_XLMI_FIND_CLASS,
                               cmnGetString(ID_XSDI_MENU_FIND),  // pszFind
                               MIS_TEXT, 0);
            _fMenuCnrWhitespace = FALSE;
        }

        return TRUE;
    }
    else
        return FALSE;
}

/*
 *@@ cllMenuItemSelected:
 *      implementation for XWPClassList::wpMenuItemSelected.
 *      If this returns FALSE, the parent gets called.
 *
 *@@added V0.9.1 (99-12-28) [umoeller]
 *@@changed V0.9.3 (2000-04-28) [umoeller]: fixed hangs with register class
 *@@changed V0.9.6 (2000-11-12) [jsmall]: added "Refresh"
 */

BOOL cllMenuItemSelected(XWPClassList *somSelf,
                         HWND hwndFrame,
                         ULONG ulMenuId)
{
    BOOL brc = FALSE;

    if (ulMenuId == *G_pulVarMenuOfs + ID_XFMI_OFS_XWPVIEW)
    {
        // "Open" --> "Class list":
        // wpViewObject will call wpOpen if a new view is necessary
        _wpViewObject(somSelf,
                      NULLHANDLE,   // hwndCnr; "WPS-internal use only", IBM says
                      ulMenuId,     // ulView; must be the same as menu item
                      0);           // parameter passed to wpOpen
        _xwpHandleSelfClose(somSelf, hwndFrame, ulMenuId); // V1.0.1 (2002-12-08) [umoeller]
        brc = TRUE;
    }
    else switch (ulMenuId)
    {
        case ID_XLMI_REGISTER:
        {
            // "Register" menu command:
            REGISTERCLASSDATA rcd;

            // get class tree cnr dlg
            HWND hwndClient = WinWindowFromID(hwndFrame, FID_CLIENT);
            HWND hwndSplitMain = WinWindowFromID(hwndClient, ID_SPLITMAIN);
            HWND hwndDlg = WinWindowFromID(hwndSplitMain, ID_XLD_CLASSLIST);
            PCLASSLISTTREECNRDATA pClassTreeCnrData
                = (PCLASSLISTTREECNRDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);
            if (pClassTreeCnrData)
            {
                rcd.pszHelpLibrary = cmnQueryHelpLibrary();
                rcd.ulHelpPanel = ID_XFH_REGISTERCLASS;
                if (WinDlgBox(HWND_DESKTOP,
                              hwndDlg,
                              fnwpRegisterClass,
                              cmnQueryNLSModuleHandle(FALSE),
                              ID_XLD_REGISTERCLASS,
                              &rcd)
                        == DID_OK)
                {
                    HPOINTER hptrOld = winhSetWaitPointer();
                    CHAR    szErrorCode[10] = "?",
                            szModuleError[500] = "none";
                    PCSZ pTable[3] = {
                                rcd.szClassName,
                                szErrorCode,
                                szModuleError
                        };
                    APIRET arc;

                    /* WinSendMsg(pClassTreeCnrData->pClientData->pscd->hwndCnr,
                               CM_REMOVERECORD,
                               (MPARAM)NULL,
                               MPFROM2SHORT(0, // remove all records
                                            CMA_FREE | CMA_INVALIDATE));
                    clsCleanupWpsClasses(pClassTreeCnrData->pClientData->pscd->pwpsc);
                    WinSetPointer(HWND_DESKTOP, hptrOld);
                    free(pszClassInfo);
                    pszClassInfo = NULL; */

                    arc = winhRegisterClass(rcd.szClassName,
                                            rcd.szModName,
                                            szModuleError,
                                            sizeof(szModuleError));
                    WinSetPointer(HWND_DESKTOP, hptrOld);

                    if (arc == NO_ERROR)
                        // success
                        cmnMessageBoxExt(hwndDlg,
                                            121,
                                            pTable, 1, 131,
                                            MB_OK);
                    else
                    {
                        // error:
                        sprintf(szErrorCode, "%d", arc);
                        cmnMessageBoxExt(hwndDlg,
                                            104,
                                            pTable, 3, 132,
                                            MB_OK);
                    }

                    // fill cnr again
                    // WinPostMsg(hwndDlg, WM_FILLCNR, MPNULL, MPNULL);
                }
                brc = TRUE;
            }
        } // end else if (ulMenuId == ID_XLMI_REGISTER)
        break;

        case ID_XLMI_REFRESH_VIEW:
        {
            // "Refresh View" menu command:

            // get class tree cnr dlg
            HWND hwndClient = WinWindowFromID(hwndFrame, FID_CLIENT);
            HWND hwndSplitMain = WinWindowFromID(hwndClient, ID_SPLITMAIN);
            HWND hwndDlg = WinWindowFromID(hwndSplitMain, ID_XLD_CLASSLIST);
            PCLASSLISTTREECNRDATA pClassTreeCnrData
                = (PCLASSLISTTREECNRDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);
            if (pClassTreeCnrData)
            {
                HPOINTER hptrOld = winhSetWaitPointer();

                WinSendMsg(pClassTreeCnrData->pClientData->pscd->hwndCnr,
                           CM_REMOVERECORD,
                           (MPARAM)NULL,
                           MPFROM2SHORT(0, // remove all records
                                        CMA_FREE | CMA_INVALIDATE));
                clsCleanupWpsClasses(pClassTreeCnrData->pClientData->pscd->pwpsc);
                WinSetPointer(HWND_DESKTOP, hptrOld);
                free(pszClassInfo);
                pszClassInfo = NULL;

                // fill cnr again
                WinPostMsg(hwndDlg, WM_FILLCNR, MPNULL, MPNULL);
            }
        }
        break;

        case ID_XLMI_FIND_CLASS:
        {
            // get class tree cnr dlg
            HWND hwndClient = WinWindowFromID(hwndFrame, FID_CLIENT);
            HWND hwndSplitMain = WinWindowFromID(hwndClient, ID_SPLITMAIN);
            HWND hwndDlg = WinWindowFromID(hwndSplitMain, ID_XLD_CLASSLIST);
            PCLASSLISTTREECNRDATA pClassTreeCnrData;

            if (pClassTreeCnrData = (PCLASSLISTTREECNRDATA)WinQueryWindowPtr(hwndDlg, QWL_USER))
            {
                PRECORDCORE prec = NULL;
                XSTRING title, text;
                PSZ searchClass;
                PLINKLIST pll = pClassTreeCnrData->pClientData->pscd->pwpsc->pllClassList;
                PLISTNODE pNode;
                xstrInit(&title, 0);
                xstrInit(&text, 0);
                cmnGetMessage(NULL, 0, &title, 256);
                cmnGetMessage(NULL, 0, &text, 257);
                searchClass = cmnTextEntryBox(hwndFrame,
                      (PCSZ)title.psz,        // "Search for Class",
                      (PCSZ)text.psz,       //  "Enter the name of the class:",
                      (PCSZ)NULL,
                      255,
                      0);
                xstrClear(&title);
                xstrClear(&text);
                if (searchClass)
                {
                    pNode = pll->pFirst;
                    while (pNode)
                    {
                        if (!stricmp(((PWPSLISTITEM)pNode->pItemData)->pszClassName, searchClass))
                        {
                            prec = ((PWPSLISTITEM)pNode->pItemData)->pRecord;
                            pNode = NULL;
                        }
                        else
                            pNode = pNode->pNext;
                    } /* endfor */

                    if (prec)
                    {
                        HWND hwndCnr = pClassTreeCnrData->pClientData->pscd->hwndCnr;
                        cnrhExpandFromRoot(hwndCnr, prec);
                        cnrhScrollToRecord(hwndCnr, prec, CMA_TEXT, FALSE);
                        WinSendMsg(hwndCnr,
                                   CM_SETRECORDEMPHASIS,
                                   (MPARAM)prec,
                                   MPFROM2SHORT(TRUE, CRA_CURSORED | CRA_SELECTED));
                    }
                    else
                    {
                        PCSZ psz = searchClass;
                        PCSZ apsz[] = { psz };
                        cmnMessageBoxExt(hwndFrame,
                                     256,      // XWP search for class
                                     apsz,
                                     1,
                                     233,
                                     MB_CANCEL);
                    } /* endif */
                }
            }
        }
        break;
    }

    return brc;
}

/*
 *@@ cllCreateClassListView:
 *      this gets called from XWPClassList::wpOpen to
 *      create a new "class list" view. The parameters
 *      are just passed on from wpOpen.
 *
 *      The class list window is a regular standard PM
 *      frame, using fnwpClassListClient as its client
 *      window procedure. In that procedure, upon WM_CREATE,
 *      the subwindows are created and linked.
 *
 *@@changed V0.9.3 (2000-04-02) [umoeller]: moved wpRegisterView etc. here
 *@@changed V0.9.5 (2000-09-18) [umoeller]: fixed view title
 */

HWND cllCreateClassListView(WPObject *somSelf,
                            HWND hwndCnr,
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
                         WC_CLASSLISTCLIENT,
                         fnwpClassListClient,
                         CS_SIZEREDRAW | CS_SYNCPAINT,
                         sizeof(PCLASSLISTCLIENTDATA)); // additional bytes to reserve

        // create the frame window
        flCreate = FCF_SYSMENU
                    | FCF_SIZEBORDER
                    | FCF_TITLEBAR
                    // | FCF_MINMAX
                    | FCF_NOBYTEALIGN
                    | cmnQueryFCF(somSelf); // V1.0.1 (2002-12-08) [umoeller]

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

        if (hwndFrame = winhCreateStdWindow(HWND_DESKTOP,           // frame parent
                                            &swpFrame,
                                            flCreate,
                                            WS_ANIMATE,
                                            _wpQueryTitle(somSelf), // title bar
                                            0,                      // res IDs
                                            WC_CLASSLISTCLIENT,     // client class
                                            0L,                     // client wnd style
                                            ID_CLASSFRAME,          // ID
                                            &ClientCData,
                                            &hwndClient))
        {
            // get client data window pointer; this has been allocated
            // by WM_CREATE in fnwpClassListClient
            PCLASSLISTCLIENTDATA pClientData
                    = (PCLASSLISTCLIENTDATA)WinQueryWindowPtr(hwndClient, QWL_USER);

            // now position the frame and the client:
            // 1) frame
            if (!winhRestoreWindowPos(hwndFrame,
                                      HINI_USER,
                                      INIAPP_XWORKPLACE,
                                      INIKEY_WNDPOSCLASSINFO,
                                      SWP_MOVE | SWP_SIZE))
                // INI data not found:
                WinSetWindowPos(hwndFrame,
                                HWND_TOP,
                                100, 100,
                                500, 500,
                                SWP_MOVE | SWP_SIZE);

            // finally, show window
            WinShowWindow(hwndFrame, TRUE);

            // add use list item and register view
            cmnRegisterView(somSelf,
                            &pClientData->clvi.UseItem,
                            ulView,
                            hwndFrame,
                            cmnGetString(ID_XFSI_OPENCLASSLIST));

        }
    }
    CATCH(excpt1) { } END_CATCH();

    return hwndFrame;
}

