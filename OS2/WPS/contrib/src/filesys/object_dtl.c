
/*
 *@@sourcefile object_dtl.c:
 *      implementation code for the "Details" dialog
 *      (Icon page).
 *
 *      This code has been moved from object.c with V1.0.0.
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 *@@header "filesys\object.h"
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

#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSEMAPHORES
#define INCL_DOSRESOURCES
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WININPUT
#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINPOINTERS
#define INCL_WINSYS
#define INCL_WINTIMER
#define INCL_WINMENUS
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINMLE
#define INCL_WINSTDCNR
#define INCL_WINSTDBOOK
#define INCL_WINPROGRAMLIST     // needed for wppgm.h
#define INCL_WINSWITCHLIST
#define INCL_WINSHELLDATA
#define INCL_WINCLIPBOARD
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\apps.h"               // application helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\exeh.h"               // executable helpers
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\tree.h"               // red-black binary trees
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"
#include "xfobj.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\classtest.h"           // some cheap funcs for WPS class checks
#include "shared\cnrsort.h"             // container sort comparison functions
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\notebook.h"            // generic XWorkplace notebook handling
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

// headers in /hook
#include "hook\xwphook.h"

#include "filesys\fdrmenus.h"           // shared folder menu logic
#include "filesys\filesys.h"            // various file-system object implementation code
#include "filesys\folder.h"             // XFolder implementation
#include "filesys\icons.h"              // icons handling
#include "filesys\object.h"             // XFldObject implementation
#include "filesys\program.h"            // program implementation; WARNING: this redefines macros
#include "filesys\xthreads.h"           // extra XWorkplace threads

#include "config\hookintf.h"            // daemon/hook interface

#pragma hdrstop

/* ******************************************************************
 *
 *   Object details dialog
 *
 ********************************************************************/

/*
 *@@ OBJECTUSAGERECORD:
 *      extended RECORDCORE structure for
 *      "Object" settings page.
 *      All inserted records are of this type.
 */

typedef struct _OBJECTUSAGERECORD
{
    RECORDCORE     recc;
    CHAR           szText[1000];        // should suffice even for setup strings
} OBJECTUSAGERECORD, *POBJECTUSAGERECORD;

/*
 *@@ AddObjectUsage2Cnr:
 *      shortcut for the "object usage" functions below
 *      to add one cnr record core.
 */

STATIC POBJECTUSAGERECORD AddObjectUsage2Cnr(HWND hwndCnr,     // in: container on "Object" page
                                             POBJECTUSAGERECORD preccParent, // in: parent record or NULL for root
                                             PCSZ pcszTitle,     // in: text to appear in cnr
                                             ULONG flAttrs)    // in: CRA_* flags for record
{
    POBJECTUSAGERECORD preccNew;

    if (preccNew = (POBJECTUSAGERECORD)cnrhAllocRecords(hwndCnr,
                                                        sizeof(OBJECTUSAGERECORD),
                                                        1))
    {
        strhncpy0(preccNew->szText, pcszTitle, sizeof(preccNew->szText));
        cnrhInsertRecords(hwndCnr,
                          (PRECORDCORE)preccParent,       // parent
                          (PRECORDCORE)preccNew,          // new record
                          FALSE, // invalidate
                          preccNew->szText,
                          flAttrs,
                          1);
    }

    return preccNew;
}

/*
 *@@ AddFolderView2Cnr:
 *
 *@@added V0.9.1 (2000-01-17) [umoeller]
 *@@changed V0.9.19 (2002-05-23) [umoeller]: now using string ID
 */

STATIC VOID AddFolderView2Cnr(HWND hwndCnr,
                              POBJECTUSAGERECORD preccLevel2,
                              WPObject *pObject,
                              ULONG ulView,
                              ULONG idStringView)
{
    XSTRING strTemp;
    ULONG   ulViewAttrs = _wpQueryFldrAttr(pObject, ulView);
    CHAR    szView[200];
    ULONG   lenView;

    lenView = cmnGetString2(szView,
                            idStringView,
                            sizeof(szView));

    strhKillChar(szView,
                 '~',
                 &lenView);

    xstrInit(&strTemp, lenView + 30);

    xstrcpy(&strTemp, szView, lenView);
    xstrcat(&strTemp, ": ", 2);

    if (ulViewAttrs & CV_ICON)
        xstrcat(&strTemp, "CV_ICON ", 0);
    if (ulViewAttrs & CV_NAME)
        xstrcat(&strTemp, "CV_NAME ", 0);
    if (ulViewAttrs & CV_TEXT)
        xstrcat(&strTemp, "CV_TEXT ", 0);
    if (ulViewAttrs & CV_TREE)
        xstrcat(&strTemp, "CV_TREE ", 0);
    if (ulViewAttrs & CV_DETAIL)
        xstrcat(&strTemp, "CV_DETAIL ", 0);
    if (ulViewAttrs & CA_DETAILSVIEWTITLES)
        xstrcat(&strTemp, "CA_DETAILSVIEWTITLES ", 0);

    if (ulViewAttrs & CV_MINI)
        xstrcat(&strTemp, "CV_MINI ", 0);
    if (ulViewAttrs & CV_FLOW)
        xstrcat(&strTemp, "CV_FLOW ", 0);
    if (ulViewAttrs & CA_DRAWICON)
        xstrcat(&strTemp, "CA_DRAWICON ", 0);
    if (ulViewAttrs & CA_DRAWBITMAP)
        xstrcat(&strTemp, "CA_DRAWBITMAP ", 0);
    if (ulViewAttrs & CA_TREELINE)
        xstrcat(&strTemp, "CA_TREELINE ", 0);

    // owner...

    AddObjectUsage2Cnr(hwndCnr,
                       preccLevel2,
                       strTemp.psz,
                       CRA_RECORDREADONLY);

    xstrClear(&strTemp);
}

/*
 *@@ AddColor:
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 */

VOID AddColor(HWND hwndCnr,
              POBJECTUSAGERECORD preccLevel2,
              PCSZ pcszDescription,
              LONG lColor)
{
    #define GET_BLUE(lcol)  *( ((PBYTE)(&(lcol))) )
    #define GET_GREEN(lcol) *( ((PBYTE)(&(lcol))) + 1 )
    #define GET_RED(lcol)   *( ((PBYTE)(&(lcol))) + 2 )

    CHAR szTemp[100];
    sprintf(szTemp,
            "%s: 0x%lX (%d %d %d)",
            pcszDescription,
            lColor,
            GET_RED(lColor),
            GET_GREEN(lColor),
            GET_BLUE(lColor));

    AddObjectUsage2Cnr(hwndCnr,
                       preccLevel2,
                       szTemp,
                       CRA_RECORDREADONLY);
}

/*
 *@@ AddFolderItems:
 *
 *@@added V1.0.0 (2002-08-24) [umoeller]
 */

VOID AddFolderItems(WPFolder *pObject,
                    HWND hwndCnr,
                    POBJECTUSAGERECORD preccRoot)
{
    XFolderData     *somThis = XFolderGetData(pObject);
    PIBMFOLDERDATA  pFdrData;
    POBJECTUSAGERECORD
                    preccLevel2,
                    preccLevel3;
    ULONG           ul;

    preccLevel2 = AddObjectUsage2Cnr(hwndCnr,
                                     preccRoot,
                                     cmnGetString(ID_XSSI_OBJDET_FOLDERFLAGS), // "Folder flags",
                                     CRA_RECORDREADONLY);
    if (ul = _wpQueryFldrFlags(pObject))
    {
        static const struct
        {
            ULONG   fl;
            PCSZ    pcsz;
        } Flags[] =
        {
            #define FLAGENTRY(s) s, #s
            FLAGENTRY(FOI_POPULATEDWITHALL),
            FLAGENTRY(FOI_POPULATEDWITHFOLDERS),
            FLAGENTRY(FOI_FIRSTPOPULATE),
            FLAGENTRY(FOI_WORKAREA),
            FLAGENTRY(FOI_CHANGEFONT),
            FLAGENTRY(FOI_NOREFRESHVIEWS),
            FLAGENTRY(FOI_ASYNCREFRESHONOPEN),
            FLAGENTRY(FOI_REFRESHINPROGRESS),
            FLAGENTRY(FOI_WAMCRINPROGRESS),
            FLAGENTRY(FOI_CNRBKGNDOLDFORMAT),
            FLAGENTRY(FOI_DELETEINPROGRESS),
        };

        ULONG i;
        for (i = 0;
             i < ARRAYITEMCOUNT(Flags);
             ++i)
        {
            if (ul & Flags[i].fl)
                AddObjectUsage2Cnr(hwndCnr,
                                   preccLevel2,
                                   Flags[i].pcsz,
                                   CRA_RECORDREADONLY);
        }
    }

    // folder views:
    preccLevel2 = AddObjectUsage2Cnr(hwndCnr,
                                     preccRoot,
                                     cmnGetString(ID_XSSI_OBJDET_FOLDERVIEWFLAGS),
                                            // "Folder view flags",
                                     CRA_RECORDREADONLY);
    AddFolderView2Cnr(hwndCnr,
                      preccLevel2,
                      pObject,
                      OPEN_CONTENTS,
                      ID_XSDI_MENU_ICONVIEW);
    AddFolderView2Cnr(hwndCnr,
                      preccLevel2,
                      pObject,
                      OPEN_DETAILS,
                      ID_XSDI_MENU_DETAILSVIEW);
    AddFolderView2Cnr(hwndCnr,
                      preccLevel2,
                      pObject,
                      OPEN_TREE,
                      ID_XSDI_MENU_TREEVIEW);



    // folder colors: V1.0.0 (2002-08-24) [umoeller]

    if (pFdrData = (PIBMFOLDERDATA)_pvWPFolderData)
    {
        preccLevel2 = AddObjectUsage2Cnr(hwndCnr,
                                         preccRoot,
                                         "Folder colors",
                                         CRA_RECORDREADONLY);

        #define ADDCOLOR(x) AddColor(hwndCnr, preccLevel2, # x, pFdrData->LongArray.x)

        ADDCOLOR(rgbTextBkgndCol);
        ADDCOLOR(rgbIconViewTextColNoIcons);
        ADDCOLOR(rgbIconViewTextColColumns);
        ADDCOLOR(rgbIconViewTextColAsPlaced);
        ADDCOLOR(rgbTreeViewTextColTextOnly);
        ADDCOLOR(rgbTreeViewTextColIcons);
        ADDCOLOR(rgbDetlViewTextCol);
        ADDCOLOR(rgbIconViewShadowCol);
        ADDCOLOR(rgbTreeViewShadowCol);
        ADDCOLOR(rgbDetlViewShadowCol);
    }

    // Desktop: add WPS data
    // we use a conditional compile flag here because
    // _heap_walk adds additional overhead to malloc()
    #ifdef DEBUG_MEMORY
        if (pObject == cmnQueryActiveDesktop())
        {
            preccLevel2 = AddObjectUsage2Cnr(hwndCnr, preccRoot,
                                             "Workplace Shell status",
                                             CRA_RECORDREADONLY);

            lObjectCount = 0;
            lTotalObjectSize = 0;
            lFreedObjectCount = 0;
            lHeapStatus = _HEAPOK;

            // get heap info using the callback above
            _heap_walk(fncbHeapWalk);

            strths(szTemp1, lTotalObjectSize, ',');
            sprintf(szText, "XWorkplace memory consumption: %s bytes\n"
                    "(%d objects used, %d objects freed)",
                    szTemp1,
                    lObjectCount,
                    lFreedObjectCount);
            AddObjectUsage2Cnr(hwndCnr, preccLevel2, szText,
                               CRA_RECORDREADONLY);

            sprintf(szText, "XWorkplace memory heap status: %s",
                    (lHeapStatus == _HEAPOK) ? "OK"
                    : (lHeapStatus == _HEAPBADBEGIN) ? "Invalid heap (_HEAPBADBEGIN)"
                    : (lHeapStatus == _HEAPBADNODE) ? "Damaged memory node"
                    : (lHeapStatus == _HEAPEMPTY) ? "Heap not initialized"
                    : "unknown error"
                    );
            AddObjectUsage2Cnr(hwndCnr, preccLevel2, szText,
                               CRA_RECORDREADONLY);
        }
    #endif // DEBUG_MEMORY

    #ifdef __DEBUG__
    {
        PFDRCONTENTITEM pThis;
        BOOL fLocked = FALSE;

        // dump the file-system objects
        TRY_LOUD(excpt1)
        {
            if (fLocked = !_wpRequestFolderMutexSem(pObject, SEM_INDEFINITE_WAIT))
            {
                CHAR    szTemp1[100];
                sprintf(szTemp1,
                        "Folder contents (%d fs objects)",
                        _cFileSystems);

                preccLevel2 = AddObjectUsage2Cnr(hwndCnr,
                                                 preccRoot,
                                                 szTemp1,
                                                 CRA_RECORDREADONLY);

                for (pThis = (PFDRCONTENTITEM)treeFirst(
                                _FileSystemsTreeRoot);
                     pThis;
                     pThis = (PFDRCONTENTITEM)treeNext((TREE*)pThis))
                {
                    AddObjectUsage2Cnr(hwndCnr,
                                       preccLevel2,
                                       (PCSZ)pThis->Tree.ulKey,
                                       CRA_RECORDREADONLY);
                }

                sprintf(szTemp1,
                        "Folder contents (%d abstract objects)",
                        _cAbstracts);

                preccLevel2 = AddObjectUsage2Cnr(hwndCnr,
                                                 preccRoot,
                                                 szTemp1,
                                                 CRA_RECORDREADONLY);

                for (pThis = (PFDRCONTENTITEM)treeFirst(
                                _AbstractsTreeRoot);
                     pThis;
                     pThis = (PFDRCONTENTITEM)treeNext((TREE*)pThis))
                {
                    AddObjectUsage2Cnr(hwndCnr,
                                       preccLevel2,
                                       _wpQueryTitle(pThis->pobj),
                                       CRA_RECORDREADONLY);
                }
            }
        }
        CATCH(excpt1) {} END_CATCH();

        if (fLocked)
            _wpReleaseFolderMutexSem(pObject);
    }
    #endif
}

/*
 *@@ FillCnrWithObjectUsage:
 *      adds all the object details into a given
 *      container window.
 *
 *@@changed V0.9.1 (2000-01-16) [umoeller]: added object setup string
 *@@changed V0.9.6 (2000-10-16) [umoeller]: added program data
 *@@changed V0.9.16 (2001-10-06): fixed memory leak with program objects
 *@@changed V0.9.19 (2002-05-23) [umoeller]: added obj default view
 *@@changed V0.9.19 (2002-05-23) [umoeller]: finally localized strings in cnr
 */

STATIC VOID FillCnrWithObjectUsage(HWND hwndCnr,       // in: cnr to insert into
                                   WPObject *pObject)  // in: object for which to insert data
{
    POBJECTUSAGERECORD
                    preccRoot, preccLevel2, preccLevel3;
    CHAR            szTemp1[100], szText[500];
    PUSEITEM        pUseItem;

    CHAR            szObjectHandle[20];
    PSZ             pszObjectID;
    ULONG           ul;

    if (pObject)
    {
        APIRET      arc = NO_ERROR;
        HOBJECT     hObject = NULLHANDLE;
        ULONG       ulDefaultView;

        sprintf(szText, "%s (%s: %s)",
                _wpQueryTitle(pObject),
                cmnGetString(ID_XSSI_CLSLIST_CLASS),        // "Class"
                _somGetClassName(pObject));
        preccRoot = AddObjectUsage2Cnr(hwndCnr, NULL, szText,
                                       CRA_RECORDREADONLY | CRA_EXPANDED);

        // object ID
        preccLevel2 = AddObjectUsage2Cnr(hwndCnr, preccRoot,
                                         cmnGetString(ID_XSSI_OBJDET_OBJECTID), // "Object ID",
                                         CRA_RECORDREADONLY | CRA_EXPANDED);
        if (pszObjectID = _wpQueryObjectID(pObject))
            AddObjectUsage2Cnr(hwndCnr, preccLevel2,
                               pszObjectID,
                               (strcmp(pszObjectID, "<WP_DESKTOP>") != 0
                                    ? 0 // editable!
                                    : CRA_RECORDREADONLY)); // for the Desktop
        else
            AddObjectUsage2Cnr(hwndCnr, preccLevel2,
                               cmnGetString(ID_XSSI_OBJDET_NONESET), // "none set",
                               0); // editable!

        // original object ID   V0.9.16 (2001-12-06) [umoeller]
        preccLevel2 = AddObjectUsage2Cnr(hwndCnr, preccRoot,
                                         cmnGetString(ID_XSSI_OBJDET_OBJECTID_ORIG), // "Original object ID",
                                         CRA_RECORDREADONLY | CRA_EXPANDED);
        if (pszObjectID = _xwpQueryOriginalObjectID(pObject))
            AddObjectUsage2Cnr(hwndCnr, preccLevel2,
                               pszObjectID,
                               CRA_RECORDREADONLY);
        else
            AddObjectUsage2Cnr(hwndCnr, preccLevel2,
                               cmnGetString(ID_XSSI_OBJDET_NONESET), // "none set",
                               CRA_RECORDREADONLY);

        // object default view V0.9.19 (2002-05-23) [umoeller]
        ulDefaultView = _wpQueryDefaultView(pObject);
        sprintf(szText,
                "%s: 0x%lX",
                cmnGetString(ID_XSSI_OBJDET_DEFAULTVIEW),
                ulDefaultView);

        switch (ulDefaultView)
        {
            #define CASE_VIEW(v) case v: strcat(szText, " (" #v ")"); break
            CASE_VIEW(OPEN_UNKNOWN);
            CASE_VIEW(OPEN_DEFAULT);
            CASE_VIEW(OPEN_CONTENTS);
            CASE_VIEW(OPEN_SETTINGS);
            CASE_VIEW(OPEN_HELP);
            CASE_VIEW(OPEN_RUNNING);
            CASE_VIEW(OPEN_PROMPTDLG);
            CASE_VIEW(OPEN_PALETTE);
        }

        preccLevel2 = AddObjectUsage2Cnr(hwndCnr, preccRoot,
                                         szText,
                                         CRA_RECORDREADONLY | CRA_EXPANDED);

        // object handle
        preccLevel2 = AddObjectUsage2Cnr(hwndCnr, preccRoot,
                                         cmnGetString(ID_XSSI_OBJDET_HANDLE), // "Object handle",
                                         CRA_RECORDREADONLY | CRA_EXPANDED);
        if (_somIsA(pObject, _WPFileSystem))
        {
            // for file system objects:
            // do not call wpQueryHandle, because
            // this always _creates_ a handle!
            // So instead, we search OS2SYS.INI directly.
            CHAR    szPath[CCHMAXPATH];
            _wpQueryFilename(pObject, szPath,
                             TRUE);      // fully qualified
            arc = wphQueryHandleFromPath(HINI_USER,
                                         HINI_SYSTEM,
                                         szPath,
                                         &hObject);
        }
        else // if (_somIsA(pObject, _WPAbstract))
            // not file system: that's safe
            hObject = _wpQueryHandle(pObject);

        if ((LONG)hObject > 0)
            sprintf(szObjectHandle, "0x%lX", hObject);
        else
            // hObject is 0:
            if ((arc) && (arc != ERROR_FILE_NOT_FOUND))
                sprintf(szObjectHandle,
                        "Error %d",
                        arc);
            else
                cmnGetString2(szObjectHandle,
                              ID_XSSI_OBJDET_NONESET,
                              sizeof(szObjectHandle));

        AddObjectUsage2Cnr(hwndCnr, preccLevel2,
                           szObjectHandle,
                           CRA_RECORDREADONLY);

        // object style
        preccLevel2 = AddObjectUsage2Cnr(hwndCnr, preccRoot,
                                         cmnGetString(ID_XSSI_OBJDET_STYLEGROUP), // "Object style",
                                         CRA_RECORDREADONLY);
        if (ul = _wpQueryStyle(pObject))
        {
            static const struct
            {
                ULONG   fl;
                ULONG   idString;
            } Flags[] =
            {
                #define STYLEENTRY(s) OBJSTYLE_ ## s, ID_XSSI_OBJDET_ ## s
                STYLEENTRY(CUSTOMICON),
                STYLEENTRY(NOTDEFAULTICON),
                STYLEENTRY(NOCOPY),
                STYLEENTRY(NODELETE),
                STYLEENTRY(NODRAG),
                STYLEENTRY(NODROPON),
                STYLEENTRY(NOLINK),
                STYLEENTRY(NOMOVE),
                STYLEENTRY(NOPRINT),
                STYLEENTRY(NORENAME),
                STYLEENTRY(NOSETTINGS),
                STYLEENTRY(NOTVISIBLE),
                STYLEENTRY(TEMPLATE),
                STYLEENTRY(LOCKEDINPLACE)
            };

            ULONG i;
            for (i = 0;
                 i < ARRAYITEMCOUNT(Flags);
                 ++i)
            {
                if (ul & Flags[i].fl)
                    AddObjectUsage2Cnr(hwndCnr,
                                       preccLevel2,
                                       cmnGetString(Flags[i].idString),
                                       CRA_RECORDREADONLY);
            }
        }

        /*
         * program data:
         *
         */

        if (progIsProgramOrProgramFile(pObject))
        {
            ULONG   ulSize = 0;
            PPROGDETAILS    pDetails;
            if ((pDetails = progQueryDetails(pObject)))
            {
                // OK, now we got the program object data....

                PCSZ pcszTemp;

                /*
                typedef struct _PROGDETAILS {
                  ULONG        Length;          //  Length of structure.
                  PROGTYPE     progt;           //  Program type.
                  PSZ          pszTitle;        //  Title.
                  PSZ          pszExecutable;   //  Executable file name (program name).
                  PSZ          pszParameters;   //  Parameter string.
                  PSZ          pszStartupDir;   //  Start-up directory.
                  PSZ          pszIcon;         //  Icon-file name.
                  PSZ          pszEnvironment;  //  Environment string.
                  SWP          swpInitial;      //  Initial window position and size.
                } PROGDETAILS; */

                preccLevel2 = AddObjectUsage2Cnr(hwndCnr, preccRoot,
                                                 cmnGetString(ID_XSSI_OBJDET_PROGRAMDATA),
                                                 CRA_RECORDREADONLY);

                // program type
                // (moved code to helpers V0.9.16 (2001-10-06))
                if (!(pcszTemp = exehDescribeProgType(pDetails->progt.progc)))
                    pcszTemp = cmnGetString(ID_SDDI_APMVERSION); // "unknown";

                sprintf(szTemp1,
                        "%s: %s (0x%lX)",
                        cmnGetString(ID_XSSI_OBJDET_PROGRAMTYPE),
                        pcszTemp,
                        pDetails->progt.progc);
                AddObjectUsage2Cnr(hwndCnr, preccLevel2, szTemp1,
                                   CRA_RECORDREADONLY);

                // executable
                sprintf(szTemp1, "%s: %s",
                        cmnGetString(ID_XSSI_SBMNC_500), // "Executable program file"
                        (pDetails->pszExecutable)
                            ? pDetails->pszExecutable
                            : "NULL");
                AddObjectUsage2Cnr(hwndCnr, preccLevel2, szTemp1,
                                   CRA_RECORDREADONLY);

                // parameters
                sprintf(szTemp1, "%s: %s",
                        cmnGetString(ID_XSSI_SBMNC_510), // "Parameter list"
                        (pDetails->pszParameters)
                            ? pDetails->pszParameters
                            : "NULL");
                AddObjectUsage2Cnr(hwndCnr, preccLevel2, szTemp1,
                                   CRA_RECORDREADONLY);

                // startup dir
                sprintf(szTemp1, "%s: %s",
                        cmnGetString(ID_XSSI_SBMNC_520), // "Working directory"
                        (pDetails->pszStartupDir)
                            ? pDetails->pszStartupDir
                            : "NULL");
                AddObjectUsage2Cnr(hwndCnr, preccLevel2, szTemp1,
                                   CRA_RECORDREADONLY);

                // environment
                preccLevel3 = AddObjectUsage2Cnr(hwndCnr, preccLevel2,
                                                 cmnGetString(ID_XSSI_OBJDET_ENVIRONMENT), // "Environment",
                                                 CRA_RECORDREADONLY);
                // if (pProgDetails->pszEnvironment)
                {
                    DOSENVIRONMENT Env = {0};
                    if (    (pDetails->pszEnvironment == 0)
                         || (appParseEnvironment(pDetails->pszEnvironment,
                                                 &Env)
                               != NO_ERROR)
                       )
                    {
                        // parse error:
                        // just add it...
                        AddObjectUsage2Cnr(hwndCnr, preccLevel3,
                                           (pDetails->pszEnvironment)
                                                ? pDetails->pszEnvironment
                                                : "NULL",
                                           CRA_RECORDREADONLY);
                    }
                    else
                    {
                        if (Env.papszVars)
                        {
                            PSZ *ppszThis = Env.papszVars;
                            for (ul = 0;
                                 ul < Env.cVars;
                                 ul++)
                            {
                                PSZ pszThis = *ppszThis;
                                // pszThis now has something like PATH=C:\TEMP
                                AddObjectUsage2Cnr(hwndCnr, preccLevel3,
                                                   pszThis,
                                                   CRA_RECORDREADONLY);
                                // next environment string
                                ppszThis++;
                            }
                        }
                        appFreeEnvironment(&Env);
                    }
                }

                free(pDetails);     // was missing V0.9.16 (2001-10-06)
            }
        }

        /*
         * folder data:
         *
         */

        else if (_somIsA(pObject, _WPFolder))
        {
            AddFolderItems(pObject,
                           hwndCnr,
                           preccRoot);
        } // end WPFolder

        // object usage:
        preccLevel2 = AddObjectUsage2Cnr(hwndCnr,
                                         preccRoot,
                                         cmnGetString(ID_XSSI_OBJDET_OBJUSAGE), // "Object usage",
                                         CRA_RECORDREADONLY);

        // 1) open views
        preccLevel3 = NULL;
        for (pUseItem = _wpFindUseItem(pObject, USAGE_OPENVIEW, NULL);
             pUseItem;
             pUseItem = _wpFindUseItem(pObject, USAGE_OPENVIEW, pUseItem))
        {
            PVIEWITEM   pViewItem = (PVIEWITEM)(pUseItem+1);

            // V1.0.0 (2002-08-31) [umoeller]
            PCSZ        pcszViewName = cmnIdentifyView(pViewItem->view);

            if (pViewItem->view != OPEN_RUNNING)
            {
                PID pid;
                TID tid;
                WinQueryWindowProcess(pViewItem->handle, &pid, &tid);
                sprintf(szText,
                        "0x%lX (%s, HWND 0x%lX, TID 0x%lX)",
                        pViewItem->view,
                        pcszViewName,
                        pViewItem->handle,
                        tid);
            }
            else
            {
                sprintf(szText, "0x%lX (%s, HAPP 0x%lX)",
                        pViewItem->view,
                        pcszViewName,
                        pViewItem->handle);
            }

            if (!preccLevel3)
                preccLevel3 = AddObjectUsage2Cnr(hwndCnr, preccLevel2,
                                                 cmnGetString(ID_XSSI_OBJDET_OPENVIEWS),
                                                    // "Currently open views",
                                                 CRA_RECORDREADONLY | CRA_EXPANDED);
            AddObjectUsage2Cnr(hwndCnr, preccLevel3, szText, CRA_RECORDREADONLY);
        }

        // 2) allocated memory
        preccLevel3 = NULL;
        for (pUseItem = _wpFindUseItem(pObject, USAGE_MEMORY, NULL);
             pUseItem;
             pUseItem = _wpFindUseItem(pObject, USAGE_MEMORY, pUseItem))
        {
            PMEMORYITEM pMemoryItem = (PMEMORYITEM)(pUseItem+1);
            sprintf(szText, "Size: %d", pMemoryItem->cbBuffer);
            if (!preccLevel3)
                preccLevel3 = AddObjectUsage2Cnr(hwndCnr, preccLevel2,
                                                 cmnGetString(ID_XSSI_OBJDET_ALLOCMEM),
                                                    // "Allocated memory",
                                                 CRA_RECORDREADONLY);
            AddObjectUsage2Cnr(hwndCnr, preccLevel3, szText, CRA_RECORDREADONLY);
        }

        // 3) awake shadows
        preccLevel3 = NULL;
        for (pUseItem = _wpFindUseItem(pObject, USAGE_LINK, NULL);
             pUseItem;
             pUseItem = _wpFindUseItem(pObject, USAGE_LINK, pUseItem))
        {
            PLINKITEM pLinkItem = (PLINKITEM)(pUseItem+1);
            CHAR      szShadowPath[CCHMAXPATH];
            if (pLinkItem->LinkObj)
            {
                _wpQueryFilename(_wpQueryFolder(pLinkItem->LinkObj),
                                 szShadowPath,
                                 TRUE);     // fully qualified
                sprintf(szText, "%s\n(%s)",
                        _wpQueryTitle(pLinkItem->LinkObj),
                        szShadowPath);
            }
            else
                // error: shouldn't happen, because pObject
                // itself is obviously valid
                memcpy(szText, "broken", 7);

            if (!preccLevel3)
                preccLevel3 = AddObjectUsage2Cnr(hwndCnr, preccLevel2,
                                                 cmnGetString(ID_XSSI_OBJDET_AWAKESHADOWS),
                                                    // "Awake shadows of this object",
                                                 CRA_RECORDREADONLY | CRA_EXPANDED);
            AddObjectUsage2Cnr(hwndCnr, preccLevel3,
                               szText,
                               CRA_RECORDREADONLY);
        }

        // 4) containers into which object has been inserted
        preccLevel3 = NULL;
        for (pUseItem = _wpFindUseItem(pObject, USAGE_RECORD, NULL);
            pUseItem;
            pUseItem = _wpFindUseItem(pObject, USAGE_RECORD, pUseItem))
        {
            PRECORDITEM pRecordItem = (PRECORDITEM)(pUseItem+1);
            CHAR szFolderTitle[256];
            WinQueryWindowText(WinQueryWindow(pRecordItem->hwndCnr, QW_PARENT),
                               sizeof(szFolderTitle)-1,
                               szFolderTitle);
            sprintf(szText,
                    "%s: 0x%lX\n(\"%s\")",
                    cmnGetString(ID_XSSI_OBJDET_CNRHWND),
                    pRecordItem->hwndCnr,
                    szFolderTitle);
            if (!preccLevel3)
                preccLevel3 = AddObjectUsage2Cnr(hwndCnr, preccLevel2,
                                                 cmnGetString(ID_XSSI_OBJDET_FOLDERWINDOWS),
                                                     // "Folder windows containing this object",
                                                 CRA_RECORDREADONLY | CRA_EXPANDED);
            AddObjectUsage2Cnr(hwndCnr, preccLevel3, szText, CRA_RECORDREADONLY);
        }

        // 5) applications (associations)
        preccLevel3 = NULL;
        for (pUseItem = _wpFindUseItem(pObject, USAGE_OPENFILE, NULL);
            pUseItem;
            pUseItem = _wpFindUseItem(pObject, USAGE_OPENFILE, pUseItem))
        {
            PVIEWFILE pViewFile = (PVIEWFILE)(pUseItem+1);
            if (!preccLevel3)
                preccLevel3 = AddObjectUsage2Cnr(hwndCnr, preccLevel2,
                                                 cmnGetString(ID_XSSI_OBJDET_APPSOPEN),
                                                     // "Applications which opened this object",
                                                 CRA_RECORDREADONLY | CRA_EXPANDED);

            sprintf(szText,
                    "%s: 0x%lX",
                    cmnGetString(ID_XSSI_OBJDET_APPHANDLE),
                        // Open handle (probably HAPP)
                    pViewFile->handle);
            AddObjectUsage2Cnr(hwndCnr, preccLevel3,
                               szText,  // open handle
                               CRA_RECORDREADONLY);

            sprintf(szText,
                    "Menu ID: 0x%lX",
                    cmnGetString(ID_XSSI_OBJDET_MENUID),
                        // Menu ID
                    pViewFile->ulMenuId);
            AddObjectUsage2Cnr(hwndCnr, preccLevel3,
                               szText,  // open handle
                               CRA_RECORDREADONLY);
        }

        preccLevel3 = NULL;
        for (ul = 0; ul < 100; ul++)
            if (    (ul != USAGE_OPENVIEW)
                 && (ul != USAGE_MEMORY)
                 && (ul != USAGE_LINK)
                 && (ul != USAGE_RECORD)
                 && (ul != USAGE_OPENFILE)
               )
            {
                for (pUseItem = _wpFindUseItem(pObject, ul, NULL);
                    pUseItem;
                    pUseItem = _wpFindUseItem(pObject, ul, pUseItem))
                {
                    sprintf(szText, "Type: 0x%lX", pUseItem->type);
                    if (!preccLevel3)
                        preccLevel3 = AddObjectUsage2Cnr(hwndCnr, preccLevel2,
                                                         "Undocumented usage types",
                                                         CRA_RECORDREADONLY | CRA_EXPANDED);
                    AddObjectUsage2Cnr(hwndCnr, preccLevel3,
                                       szText, // "undocumented:"
                                       CRA_RECORDREADONLY);
                }
            }

        // 6) icon server and clients, in debug mode V0.9.20 (2002-07-25) [umoeller]

        #ifdef __DEBUG__
        {
            XFldObjectData *somThis = XFldObjectGetData(pObject);
            BOOL fLocked = FALSE;

            TRY_LOUD(excpt1)
            {
                if (fLocked = icomLockIconShares())
                {
                    sprintf(szTemp1,
                            "Icon server: %s",
                            _pobjIconServer ? _wpQueryTitle(_pobjIconServer) : "none");

                    preccLevel2 = AddObjectUsage2Cnr(hwndCnr,
                                                     preccRoot,
                                                     szTemp1,
                                                     CRA_RECORDREADONLY);

                    sprintf(szTemp1,
                            "Icon clients: %d",
                            _cIconClients);

                    preccLevel2 = AddObjectUsage2Cnr(hwndCnr,
                                                     preccRoot,
                                                     szTemp1,
                                                     CRA_RECORDREADONLY);

                    if (_cIconClients)
                    {
                        WPObject *pobjClient = _pFirstIconClient;
                        PCSZ pcszTitle;

                        while (pobjClient)
                        {
                            XFldObjectData *somThat = XFldObjectGetData(pobjClient);

                            WPFolder *pFolder;
                            szText[0] = '\0';
                            if (pFolder = _wpQueryFolder(pobjClient))
                                _wpQueryFilename(pFolder, szText, TRUE);

                            sprintf(szTemp1,
                                    "%s {%s}",
                                    (pcszTitle = _wpQueryTitle(pobjClient))
                                                    ? pcszTitle
                                                    : "??? no title",
                                    szText);

                            AddObjectUsage2Cnr(hwndCnr,
                                               preccLevel2,
                                               szTemp1,
                                               CRA_RECORDREADONLY);
                            pobjClient = somThat->pNextClient;
                        }
                    }
                }
            }
            CATCH(excpt1) {} END_CATCH();

            if (fLocked)
                icomUnlockIconShares();
        }
        #endif
    } // end if (pObject)

    cnrhInvalidateAll(hwndCnr); // V0.9.16 (2001-10-25) [umoeller]
}

/*
 * XFOBJWINDATA:
 *      structure used with "Object" page
 *      (obj_fnwpSettingsObjDetails) for data
 *      exchange with XFldObject instance data.
 *      Created in WM_INITDLG.
 */

typedef struct _XFOBJWINDATA
{
    WPObject        *somSelf;
    CHAR            szOldID[CCHMAXPATH];
    HWND            hwndCnr;
    XSTRING         strOldObjectID;         // V0.9.19 (2002-04-02) [umoeller]
    BOOL            fEscPressed;
    PRECORDCORE     preccExpanded;
} XFOBJWINDATA, *PXFOBJWINDATA;

/*
 *@@ fnwpObjectDetails:
 *      dialog proc for object details dlg.
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 *@@changed V0.9.16 (2001-12-06) [umoeller]: fixed crash if "No" was selected on object ID change confirmation
 *@@changed V0.9.19 (2002-04-02) [umoeller]: now handling WM_TEXTEDIT for keyboard support
 *@@changed V1.0.3 (2004-06-19) [pr]: allow null object ID @@fixes 580
 */

STATIC MRESULT EXPENTRY fnwpObjectDetails(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    PXFOBJWINDATA   pWinData = (PXFOBJWINDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);

    switch (msg)
    {

        /*
         * WM_TIMER:
         *      timer for tree view auto-scroll
         */

        case WM_TIMER:
        {
            if (pWinData->preccExpanded->flRecordAttr & CRA_EXPANDED)
            {
                PRECORDCORE     preccLastChild;
                WinStopTimer(WinQueryAnchorBlock(hwndDlg),
                        hwndDlg,
                        1);
                // scroll the tree view properly
                preccLastChild = WinSendMsg(pWinData->hwndCnr,
                                            CM_QUERYRECORD,
                                            pWinData->preccExpanded,
                                               // expanded PRECORDCORE from CN_EXPANDTREE
                                            MPFROM2SHORT(CMA_LASTCHILD,
                                                         CMA_ITEMORDER));
                if ((preccLastChild) && (preccLastChild != (PRECORDCORE)-1))
                {
                    // ULONG ulrc;
                    cnrhScrollToRecord(pWinData->hwndCnr,
                                       (PRECORDCORE)preccLastChild,
                                       CMA_TEXT,   // record text rectangle only
                                       TRUE);      // keep parent visible
                }
            }
        }
        break;

        /*
         * WM_CONTROL:
         *
         */

        case WM_CONTROL:
        {
            USHORT usID = SHORT1FROMMP(mp1),
                   usNotifyCode = SHORT2FROMMP(mp1);

            switch (usID)
            {
                /*
                 * ID_XSDI_DETAILS_CONTAINER:
                 *      "Internals" container
                 */

                case ID_XSDI_DETAILS_CONTAINER:
                    switch (usNotifyCode)
                    {
                        /*
                         * CN_EXPANDTREE:
                         *      do tree-view auto scroll
                         */

                        case CN_EXPANDTREE:
                            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                            if (cmnQuerySetting(sfTreeViewAutoScroll))
                            {
                                pWinData->preccExpanded = (PRECORDCORE)mp2;
                                WinStartTimer(WinQueryAnchorBlock(hwndDlg),
                                        hwndDlg,
                                        1,
                                        100);
                            }
                        break;

                        /*
                         * CN_BEGINEDIT:
                         *      when user alt-clicked on a recc
                         */

                        case CN_BEGINEDIT:
                            pWinData->fEscPressed = TRUE;
                            mrc = (MPARAM)0;
                        break;

                        /*
                         * CN_REALLOCPSZ:
                         *      just before the edit MLE is closed
                         */

                        case CN_REALLOCPSZ:
                        {
                            PCNREDITDATA pced = (PCNREDITDATA)mp2;
                            PSZ pszChanging = *(pced->ppszText);
                            xstrcpy(&pWinData->strOldObjectID, pszChanging, 0);
                            pWinData->fEscPressed = FALSE;
                            mrc = (MPARAM)TRUE;
                        }
                        break;

                        /*
                         * CN_ENDEDIT:
                         *      recc text changed: update our data
                         */

                        case CN_ENDEDIT:
                            if (!pWinData->fEscPressed)
                            {
                                PCNREDITDATA pced = (PCNREDITDATA)mp2;
                                PSZ pszNew = *(pced->ppszText);
                                BOOL fChange = FALSE;
                                // has the object ID changed?
                                if (strhcmp(pWinData->strOldObjectID.psz, pszNew) != 0)
                                {
                                    USHORT usLength = strlen(pszNew);
                                    // is this a valid object ID?
                                    // V1.0.3 (2004-06-19) [pr]: @@fixes 580
                                    if (   (usLength == 0)
                                        || (
                                               (usLength > 2)
                                            && (pszNew[0] == '<')
                                            && (pszNew[usLength-1] == '>')
                                           )
                                       )
                                    {
                                        // valid: confirm change
                                        if (cmnMessageBoxExt(hwndDlg,
                                                             107,
                                                             NULL, 0,
                                                             109,
                                                             MB_YESNO)
                                                      == MBID_YES)
                                            fChange = TRUE;
                                    }
                                    else
                                        cmnMessageBoxExt(hwndDlg,
                                                         104,
                                                         NULL, 0,
                                                         108,
                                                         MB_OK);
                                            // fixed (V0.85)

                                    if (fChange)
                                    {
                                        _wpSetObjectID(pWinData->somSelf,
                                                       usLength ? pszNew : NULL);
                                        if (usLength == 0)
                                        {
                                            strcpy(((POBJECTUSAGERECORD)(pced->pRecord))->szText,
                                                   cmnGetString(ID_XSSI_OBJDET_NONESET));
                                            WinSendMsg(pWinData->hwndCnr,
                                                       CM_INVALIDATERECORD,
                                                       (MPARAM)&pced->pRecord,
                                                       MPFROM2SHORT(1,
                                                                    CMA_TEXTCHANGED));
                                        }
                                    }
                                    else
                                    {
                                        // change aborted: restore old recc text
                                        strlcpy(((POBJECTUSAGERECORD)pced->pRecord)->szText,
                                                pWinData->strOldObjectID.psz,
                                                sizeof(((POBJECTUSAGERECORD)pced->pRecord)->szText));
                                        WinSendMsg(pWinData->hwndCnr,
                                                   CM_INVALIDATERECORD,
                                                   (MPARAM)&pced->pRecord,
                                                        // fixed crash V0.9.16 (2001-12-06) [umoeller]
                                                   MPFROM2SHORT(1,
                                                                CMA_TEXTCHANGED));
                                    }
                                }
                            }
                            mrc = (MPARAM)0;
                        break;

                        /*
                         * CN_HELP:
                         *      V0.9.4 (2000-07-11) [umoeller]
                         */

                        case CN_HELP:
                            // always display help for the whole page, not for single items
                            cmnDisplayHelp(pWinData->somSelf,
                                           ID_XSH_SETTINGS_OBJINTERNALS);
                        break;

                        default:
                            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                    } // end switch
                break; // ID_XSDI_DTL_CNR

                default:
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);

            } // end switch (usID)
        }
        break;  // WM_CONTROL

        case WM_TEXTEDIT:
            // V0.9.19 (2002-04-02) [umoeller]
            cnrhOpenEdit(pWinData->hwndCnr);
        break;

        case WM_HELP:
            // always display help for the whole page, not for single items
            cmnDisplayHelp(pWinData->somSelf,
                           ID_XSH_SETTINGS_OBJINTERNALS);
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

#define DETAILS_WIDTH       200

static const CONTROLDEF
    DetailsGroup = LOADDEF_GROUP(ID_XSDI_DETAILS_GROUP, SZL_AUTOSIZE),
    DetailsCnr =
        {
            WC_CONTAINER,
            NULL,
            WS_VISIBLE | /* CCS_READONLY | */ CCS_EXTENDSEL,
            ID_XSDI_DETAILS_CONTAINER,
            CTL_COMMON_FONT,
            {DETAILS_WIDTH, 120},
            COMMON_SPACING
        },
    SetupStringGroup = LOADDEF_GROUP(ID_XSDI_DETAILS_SETUPSTR_GROUP, SZL_AUTOSIZE),
    SetupStringEF = CONTROLDEF_ENTRYFIELD_RO(
                            NULL,
                            ID_XSDI_DETAILS_SETUPSTR_EF,
                            DETAILS_WIDTH,
                            -1),
    DtlCloseButton = LOADDEF_DEFPUSHBUTTON(DID_CLOSE),
    DtlHelpButton = LOADDEF_HELPPUSHBUTTON(DID_HELP);

static const DLGHITEM dlgObjDetails[] =
    {
        START_TABLE,            // root table, required
            START_ROW(ROW_VALIGN_TOP),       // row 1 in the root table, required
                START_GROUP_TABLE(&DetailsGroup),
                    START_ROW(0),
                        CONTROL_DEF(&DetailsCnr),
                END_TABLE,
            START_ROW(ROW_VALIGN_TOP),       // row 1 in the root table, required
                START_GROUP_TABLE(&SetupStringGroup),
                    START_ROW(0),
                        CONTROL_DEF(&SetupStringEF),
                END_TABLE,
            START_ROW(ROW_VALIGN_TOP),
                CONTROL_DEF(&DtlCloseButton),
                CONTROL_DEF(&DtlHelpButton),
        END_TABLE
    };

/*
 *@@ objShowObjectDetails:
 *      displays the "object details" dialog for
 *      the specified object.
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 */

VOID objShowObjectDetails(HWND hwndOwner,
                          WPObject *pobj)
{
    HWND hwndDlg;

    HPOINTER hptrOld = winhSetWaitPointer();

    if (!dlghCreateDlg(&hwndDlg,
                       hwndOwner,
                       FCF_FIXED_DLG,
                       fnwpObjectDetails,
                       cmnGetString(ID_XSDI_DETAILS_DIALOG),
                       dlgObjDetails,
                       ARRAYITEMCOUNT(dlgObjDetails),
                       NULL,
                       cmnQueryDefaultFont()))
    {
        PXFOBJWINDATA pWinData = NEW(XFOBJWINDATA);
        ZERO(pWinData);
        xstrInit(&pWinData->strOldObjectID, 0);
        pWinData->somSelf = pobj;
        pWinData->hwndCnr = WinWindowFromID(hwndDlg, ID_XSDI_DETAILS_CONTAINER);
        WinSetWindowPtr(hwndDlg, QWL_USER, pWinData);

        TRY_LOUD(excpt1)
        {
            PSZ pszSetupString;
            ULONG ulLength;

            winhCenterWindow(hwndDlg);

            BEGIN_CNRINFO()
            {
                cnrhSetView(CV_TREE | CV_TEXT | CA_TREELINE);
                cnrhSetTreeIndent(30);
                cnrhSetSortFunc(fnCompareName);
            } END_CNRINFO(pWinData->hwndCnr);

            FillCnrWithObjectUsage(pWinData->hwndCnr,
                                   pobj);

            if (    (pszSetupString = _xwpQuerySetup(pobj, &ulLength))
                 && (ulLength)
               )
            {
                HWND hwndEF = WinWindowFromID(hwndDlg, ID_XSDI_DETAILS_SETUPSTR_EF);
                winhSetEntryFieldLimit(hwndEF, ulLength + 1);
                WinSetWindowText(hwndEF, pszSetupString);
                _xwpFreeSetupBuffer(pobj, pszSetupString);
            }

            WinSetPointer(HWND_DESKTOP, hptrOld);
            hptrOld = NULLHANDLE;

            WinProcessDlg(hwndDlg);
        }
        CATCH(excpt1) {} END_CATCH();

        winhDestroyWindow(&hwndDlg);
        xstrClear(&pWinData->strOldObjectID);
        free(pWinData);
    }

    if (hptrOld)
        WinSetPointer(HWND_DESKTOP, hptrOld);
}


