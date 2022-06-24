
/*
 *@@sourcefile kernel.h:
 *      header file for kernel.c. See remarks there.
 *
 *@@include #define INCL_DOSDATETIME
 *@@include #include <os2.h>
 *@@include #include "helpers\linklist.h"       // for some features
 *@@include #include "filesys\xthreads.h"
 */

/*
 *      Copyright (C) 1997-2010 Ulrich M”ller.
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

#ifndef XTHREADS_HEADER_INCLUDED
    #define XTHREADS_HEADER_INCLUDED

    // messages WM_USER+50 to WM_USER+99 are used by common.h

    // messages WM_USER+100 to WM_USER+149 are used by folder.h and statbar.h

    /********************************************************************
     *
     *   Worker thread messages (WOM_xxx)
     *
     ********************************************************************/

    // #define WOM_WELCOME                 (WM_USER+150)

    /*
     *@@ WOM_ADDAWAKEOBJECT:
     *      this is posted by XFldObject for each
     *      object that is awaked by the WPS; we
     *      need to maintain a list of these objects
     *      for XShutdown.
     *
     *      Parameters:
     *          WPObject* mp1: somSelf as in XFldObject::wpObjectReady
     */

    // #define WOM_ADDAWAKEOBJECT          (WM_USER+151)
            // removed V0.9.20 (2002-07-25) [umoeller]

    /*
     *@@ WOM_REMOVEAWAKEOBJECT:
     *      this is posted by XFldObject also, but
     *      when an object goes back to sleep.
     *      Be careful: the object pointer in mp1
     *      does not point to a valid SOM object
     *      any more, because the object has
     *      already been freed in memory; so we
     *      must not call any methods here.
     *      We only use the object pointer
     *      for finding the respective object
     *      in the linked list.
     *
     *      Parameters:
     *          WPObject* mp1: somSelf as in XFldObject::wpUnInitData
     */

    // #define WOM_REMOVEAWAKEOBJECT       (WM_USER+152)
            // removed V0.9.20 (2002-07-25) [umoeller]

    #define WOM_SHOWFOLDERDATA          (WM_USER+153)
    #define WOM_REFRESHFOLDERVIEWS      (WM_USER+154)

    // #define WOM_PROCESSORDEREDCONTENT   (WM_USER+155)
    // #define WOM_WAITFORPROCESSNEXT      (WM_USER+156)
        // removed V0.9.12 (2001-04-29) [umoeller]
    // #define WOM_INVALIDATEORDEREDCONTENT WM_USER+120 removed V0.9.0
    #define WOM_DELETEICONPOSEA         (WM_USER+161)
    #define WOM_DELETEFOLDERPOS         (WM_USER+162)

    #define WOM_UPDATEALLSTATUSBARS     (WM_USER+163)
    // #define WOM_QUICKOPEN               (WM_USER+164)
            // removed V0.9.12 (2001-04-29) [umoeller]

    #define WOM_STOREGLOBALSETTINGS     (WM_USER+165)

    /********************************************************************
     *
     *   Speedy thread messages (QM_xxx)
     *
     ********************************************************************/

#ifndef __NOBOOTUPSTATUS__
    #define QM_BOOTUPSTATUS             (WM_USER+180)
#endif

    // #define QM_PLAYSOUND                (WM_USER+181)
    // #define QM_PLAYSYSTEMSOUND          (WM_USER+182)

#ifndef __NOBOOTLOGO__
    #define QM_DESTROYLOGO              (WM_USER+183)
#endif

    #define QM_TREEVIEWAUTOSCROLL       (WM_USER+184)

    /********************************************************************
     *
     *   File thread messages (FIM_xxx)
     *
     ********************************************************************/

    // flags for recreating config folder (FIM_RECREATECONFIGFOLDER)
    // #define RCF_QUERYACTION            0
            // display message box
            // removed V0.9.16 (2001-10-11) [umoeller]
    // #define RCF_EMPTYCONFIGFOLDERONLY  1
            // create empty config folder
            // removed V0.9.16 (2001-10-23) [umoeller]
    #define RCF_DEFAULTCONFIGFOLDER    2
            // create default config folder
    #define RCF_MAININSTALLFOLDER      3
            // create "main" installation folder

    // #define FIM_DESKTOPPOPULATED        (WM_USER+130)

    #define FIM_DESKTOPPOPULATED        (WM_USER+200)
        // changed V0.9.5 (2000-08-26) [umoeller]

    #define FIM_RECREATECONFIGFOLDER    (WM_USER+201)

    // #define FIM_STARTUP                 (WM_USER+202)
    // #define FIM_STARTUPFOLDERDONE       (WM_USER+203)
            // removed V0.9.12 (2001-04-29) [umoeller]

    #define FIM_PROCESSTASKLIST         (WM_USER+204)

    // #define FIM_REFRESH                 (WM_USER+204)
        // removed V0.9.6 (2000-10-16) [umoeller]

    /*
     *@@ FILELISTITEM:
     *      structure used for DOUBLEFILES.pllFiles.
     */

    typedef struct _FILELISTITEM
    {
        CHAR        szFilename[CCHMAXPATH];
        PSZ         pszDirectory;       // directory (points into DOUBLEFILES.pllDirectories)
        FDATE       fDate;
        FTIME       fTime;
        LONGLONG    llSize;
        BOOL        fProcessed;         // anti-recursion flag, ignore
    } FILELISTITEM, *PFILELISTITEM;

    #ifdef LINKLIST_HEADER_INCLUDED
        /*
         *@@ DOUBLEFILES:
         *      structure posted with FIM_DOUBLEFILES.
         */

        typedef struct _DOUBLEFILES
        {
            PLINKLIST   pllDirectories;     // in: linked list of PSZs with directory names
            HWND        hwndNotify;         // in: notify this wnd when done
            ULONG       ulNotifyMsg;        // in: message to use for notification
            PLINKLIST   pllDoubleFiles;     // out: linked list of FILELISTITEM structures
        } DOUBLEFILES, *PDOUBLEFILES;
    #endif

    #define FIM_DOUBLEFILES             (WM_USER+215)

    #define FIM_INSERTHOTKEYS           (WM_USER+216)

    #define FIM_CALCTRASHOBJECTSIZE     (WM_USER+217)

    // messages WM_USER+250 to WM_USER+269 are used by Media thread

    // messages WM_USER+270 to WM_USER+299 are used by thread-1 object window
    // (include\shared\kernel.h)

    // messages WM_USER+300 to WM_USER+310 are used by API object window
    // (include\xwpapi.h)

    /********************************************************************
     *
     *   Prototypes
     *
     ********************************************************************/

    // BOOL xthrLockAwakeObjectsList(VOID);

    // VOID xthrUnlockAwakeObjectsList(VOID);

    // APIRET xthrQueryAwakeObjectsMutexOwner(PPID ppid,
    //                                        PTID ptid,
    //                                        PULONG pulCount);

    // LONG xthrQueryAwakeObjectsCount(VOID);

    VOID xthrResetWorkerThreadPriority(VOID);

    BOOL xthrPostWorkerMsg(ULONG msg, MPARAM mp1, MPARAM mp2);

    BOOL xthrPostFileMsg(ULONG msg, MPARAM mp1, MPARAM mp2);

    ULONG xthrIsFileThreadBusy(VOID);

    BOOL xthrPostBushMsg(ULONG msg, MPARAM mp1, MPARAM mp2);

    BOOL xthrStartThreads(VOID);

#endif
