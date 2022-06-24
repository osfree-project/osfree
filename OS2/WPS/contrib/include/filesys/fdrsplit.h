
/*
 *@@sourcefile fdrsplit.h:
 *      header file for fdrsplit.c.
 *
 *      This file is ALL new with V1.0.0
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\linklist.h"
 *@@include #include "helpers\tree.h"
 *@@include #include <wpfolder.h>
 *@@include #include "filesys\folder.h"
 *@@include #include "filesys\fdrsubclass.h"
 *@@include #include "filesys\fdrviews.h"
 *@@include #include "filesys\fdrsplit.h"
 */

/*
 *      Copyright (C) 2001-2012 Ulrich M”ller.
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

#ifndef FDRSPLIT_HEADER_INCLUDED
    #define FDRSPLIT_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Folder split views (fdrsplit.c)
     *
     ********************************************************************/

    #define ID_TREEFRAME            1001
    #define ID_FILESFRAME           1002

    #define FM_FILLFOLDER           (WM_USER + 1)
        #define FFL_FOLDERSONLY             0x0001
        #define FFL_SCROLLTO                0x0002
        #define FFL_EXPAND                  0x0004
        #define FFL_SETBACKGROUND           0x0008
        #define FFL_UNLOCKOBJECTS           0x1000
        #define FFL_POPULATEFAILED          0x2000

    #define FM_POPULATED_FILLTREE   (WM_USER + 2)
    #define FM_POPULATED_SCROLLTO   (WM_USER + 3)
    #define FM_POPULATED_FILLFILES  (WM_USER + 4)
    #define FM_UPDATEPOINTER        (WM_USER + 5)
    #define FM_DELETINGFDR          (WM_USER + 6)
    #define FM_FDRBACKGROUNDCHANGED (WM_USER + 7)       // msg name changed V1.0.1 (2002-11-30) [umoeller]

    #define FM2_POPULATE            (WM_USER + 10)
    #define FM2_ADDFIRSTCHILD_BEGIN (WM_USER + 11)
    #define FM2_ADDFIRSTCHILD_NEXT  (WM_USER + 12)
    #define FM2_ADDFIRSTCHILD_DONE  (WM_USER + 13)

    #define SPLIT_ANIMATE           0x0001
    #define SPLIT_FDRSTYLES         0x0002
    #define SPLIT_MULTIPLESEL       0x0004
    #define SPLIT_STATUSBAR         0x0008
    #define SPLIT_MENUBAR           0x0010
    #define SPLIT_NOAUTOPOSITION    0x0020
    #define SPLIT_NOAUTOPOPOPULATE  0x0040
    #define SPLIT_TOOLBAR           0x0100      // V1.0.1 (2002-11-30) [umoeller]
    #define SPLIT_NOMINI            0x0200      // V1.0.1 (2002-11-30) [umoeller]
    #define SPLIT_DETAILS           0x0400      // V1.0.1 (2002-11-30) [umoeller]

    // WM_CONTROL notifications sent by the controller
    // to its owner; SHORT1FROMMP(mp1) is always FID_CLIENT

    /*
     *@@ SN_FOLDERCHANGING:
     *      WM_CONTROL notification code sent from the
     *      split view controller (fnwpSplitController)
     *      to its parent, the main frame, which should
     *      be subclassed to intercept this notification.
     *
     *      SHORT1FROMMP(mp1) is always FID_CLIENT.
     *
     *      This one gets sent when a new folder is
     *      populated and the files cnr is to be filled
     *      (that is, FFL_FOLDERSONLY is _not_ set);
     *      mp2 is the WPFolder that is now populating. If
     *      a disk is being populated, this is the root folder.
     *
     *      The return code is ignored.
     *
     *@@added V1.0.0 (2002-09-13) [umoeller]
     */

    #define SN_FOLDERCHANGING       0x1000

    /*
     *@@ SN_FOLDERCHANGED:
     *      WM_CONTROL notification code sent from the
     *      split view controller (fnwpSplitController)
     *      to its parent, the main frame, which should
     *      be subclassed to intercept this notification.
     *
     *      SHORT1FROMMP(mp1) is always FID_CLIENT.
     *
     *      This one gets sent after SN_FOLDERCHANGING,
     *      when populate is done.
     *      mp2 is the same folder as with the previous
     *      SN_FOLDERCHANGING.
     *
     *      The return code is ignored.
     *
     *@@added V1.0.0 (2002-09-13) [umoeller]
     */

    #define SN_FOLDERCHANGED        0x1001

    /*
     *@@ SN_OBJECTSELECTED:
     *      WM_CONTROL notification code sent from the
     *      split view controller (fnwpSplitController)
     *      to its parent, the main frame, which should
     *      be subclassed to intercept this notification.
     *
     *      SHORT1FROMMP(mp1) is always FID_CLIENT.
     *
     *      This one gets sent when an object was selected
     *      in the files cnr. This only comes in if the user
     *      explicitly clicks on an object, not if an object
     *      gets selected by the container automatically.
     *      mp2 is the record of the object, which might be
     *      a shadow or disk.
     *
     *      The return code is ignored.
     *
     *@@added V1.0.0 (2002-09-13) [umoeller]
     */

    #define SN_OBJECTSELECTED       0x1002

    /*
     *@@ SN_OBJECTENTER:
     *      WM_CONTROL notification code sent from the
     *      split view controller (fnwpSplitController)
     *      to its parent, the main frame, which should
     *      be subclassed to intercept this notification.
     *
     *      SHORT1FROMMP(mp1) is always FID_CLIENT.
     *
     *      This one gets sent if an object was double-clicked
     *      upon in the files cnr. This allows the frame
     *      to set up a non-default action when the user
     *      double-clicks on records in the files cnr.
     *      mp2 is the record of the object, which might be
     *      a shadow.
     *
     *      If this returns FALSE (as the default frame and
     *      window procs do), the  split view performs a
     *      default action. If a non-zero value is returned,
     *      the split view does nothing.
     *
     *@@added V1.0.0 (2002-09-13) [umoeller]
     */

    #define SN_OBJECTENTER          0x1003

    /*
     *@@ SN_FRAMECLOSE:
     *      WM_CONTROL notification code posted (!) from the
     *      split view controller (fnwpSplitController)
     *      to its parent, the main frame, which should
     *      be subclassed to intercept this notification.
     *
     *      SHORT1FROMMP(mp1) is always FID_CLIENT.
     *
     *      This one gets posted (!) to the frame when the
     *      client receives WM_CLOSE. The problem
     *      that we have with the "close" processing
     *      is that we want to allow the frame to
     *      handle it, but the frame itself will never
     *      receive WM_CLOSE.
     *
     *      Here's how the PM frame handles WM_SYSCOMMAND
     *      with SC_CLOSE:
     *
     *      --  If the frame has a client, the frame posts
     *          WM_CLOSE to the client (_not_ to itself).
     *
     *      --  If the frame does not have a client,
     *          it posts WM_CLOSE to itself.
     *
     *      As a result, the frame never gets WM_CLOSE
     *      if there's a client, as in our case.
     *
     *      Note that the system switch list also sends
     *      WM_SYSCOMMAND with SC_CLOSE if the user
     *      tells it to stop a session.
     *
     *      The frame processes WM_CLOSE as follows:
     *
     *      --  If there's a client, it is forwarded
     *          (sent) to it.
     *
     *      --  If there's no client, WinDefWindowProc
     *          is called, which, according to PMREF,
     *          posts WM_QUIT.
     *
     *      As a result, we MUST intercept WM_CLOSE
     *      in the controller to avoid a WM_QUIT for
     *      the WPS process. However, we can't just post
     *      WM_CLOSE back to the frame (which would lead
     *      to infinite recursion), so this notification
     *      was introduced.
     *
     *      Again, this gets posted, not sent, so that
     *      the file dialog can intercept it in the
     *      main message loop.
     *
     *      mp2 is always NULL.
     *
     *      This message must always be processed by
     *      the subclassed frame, or the split view
     *      will never close.
     *
     *      The return code is ignored.
     *
     *@@added V1.0.0 (2002-09-13) [umoeller]
     */

    #define SN_FRAMECLOSE           0x1004

    /*
     *@@ SN_VKEY:
     *      WM_CONTROL notification code sent (!) from the
     *      split view controller (fnwpSplitController)
     *      to its parent, the main frame, which should
     *      be subclassed to intercept this notification.
     *
     *      SHORT1FROMMP(mp1) is always FID_CLIENT.
     *
     *      This notification gets sent when the split
     *      view has received a virtual keypress such
     *      as backspace or tab. This only comes in for
     *      the "key down" event; mp2 then has the
     *      SHORT2FROMMP(mp2) from WM_CHAR. The handler
     *      must return TRUE if the message was processed;
     *      otherwise it is passed to the default XFolder
     *      frame proc.
     *
     *@@added V1.0.0 (2002-11-23) [umoeller]
     */

    #define SN_VKEY                 0x1005

    /*
     *@@ SN_UPDATESTATUSBAR:
     *      WM_CONTROL notification code posted from the
     *      XFolder view management to a split view frame,
     *      which should be subclassed to intercept this
     *      notification.
     *
     *      SHORT1FROMMP(mp1) is always FID_CLIENT.
     *
     *      This notification gets sent when status bars
     *      need updating (as with the unreadable code
     *      for standard status bars with regular WPS
     *      folders).
     *
     *      mp2 is one of:
     *
     *      --  1: show/hide status bars according to folder/Global settings;
     *
     *      --  2: reformat status bars (e.g. because fonts have changed)
     *
     *@@added V1.0.1 (2002-11-30) [umoeller]
     */

    #define SN_UPDATESTATUSBAR      0x1006

    #ifdef THREADS_HEADER_INCLUDED
    #ifdef FDRSUBCLASS_HEADER_INCLUDED
    #ifdef FDRVIEWS_HEADER_INCLUDED

        /*
         *@@ SPLITCONTROLLER:
         *      data for the split view controller (the client
         *      of a split view frame, which controls the subframes).
         *
         *      This is shared between a regular folder split
         *      view and a file dialog frame.
         *
         *@@changed V1.0.1 (2002-11-30) [umoeller]: renamed struct
         */

        typedef struct _SPLITCONTROLLER
        {
            USHORT          cbStruct;

            LONG            lSplitBarPos;   // initial split bar position in percent

            WPObject        *pRootObject;   // root object; normally a folder, but
                                            // cann be a disk object to, for which we
                                            // then query the root folder
            WPFolder        *pRootsFolder;  // unless this is a disk, == pRootObject

            ULONG           flSplit;
                                // current split operation mode
                                // (SPLIT_* flags)

            HAB             habGUI;             // anchor block of GUI thread

            // window hierarchy
            HWND            hwndMainFrame,
                            hwndMainControl;    // child of hwndMainFrame

            HWND            hwndSplitWindow;    // child of hwndMainControl

            HWND            hwndTreeFrame;      // child of hwndSplitWindow
            CNRVIEW         cvTree;             // hwndCnr is child of hwndTreeFrame

            HWND            hwndFilesFrame;     // child of hwndSplitWindow
            CNRVIEW         cvFiles;            // hwndCnr is child of hwndFilesFrame

            HWND            hwndFocusCnr;       // container with focus

            XFRAMECONTROLS  xfc;                // extended frame struct (cctl_xframe.c)

            // HWND            hwndStatusBar,      // if present, or NULLHANDLE
            //                 hwndToolBar;        // if present, or NULLHANDLE
            // LONG            lToolBarHeight;
            //          these three are in XFRAMECONTROLS now

            // data for tree view (left)
            PSUBCLFOLDERVIEW psfvTree;
                                // XFolder subclassed view data (needed
                                // for cnr owner subclassing with XFolder's
                                // cooperation);
                                // created in fdlgFileDlg only once

            PMINIRECORDCORE precToPopulate,
                                // gets set to the folder to populate when
                                // the user selects a record in the tree
                                // on the left; we then start a timer to
                                // delay the actual populate in case the
                                // user runs through a lot of folders;
                                // reset to NULL when the timer has elapsed
                            precPopulating,
                                // while populate is running, this holds
                                // the object (folder, disk, shadow) that is
                                // populating to avoid duplicate populate posts
                            precTreeSelected,
                                // record that is currently selected
                                // in the tree on the left
                            precTreeExpanded,
                                // != NULL if the user not only selected
                                // a record, but expanded it as well
                            precFilesShowing;
                                // record whose contents are currently
                                // showing in the files cnr on the right;
                                // this is == precTreeSelected after the
                                // timer has elapsed and populate is done
                                // and _all_ objects are inserted

            WPObject        *pobjUseList;
                                // object whose use list the following was
                                // added to
            USEITEM         uiDisplaying;
            VIEWITEM        viDisplaying;

            // data for files view (right)
            PSUBCLFOLDERVIEW psfvFiles;
                                // XFolder subclassed view data (see above)

            PCSZ            pcszFileMask;
                    // if this is != NULL, it must point to a CHAR
                    // array in the user code, and we will then
                    // populate the files cnr with file system objects
                    // only that match this file mask (even if it is "*").
                    // If it is NULL, this uses the folder filter.

            BOOL            fUnlockOnClear;
                    // set to TRUE by FM_POPULATED_FILLFILES when we
                    // received notification that we did a full populate
                    // and objects therefore should be unlocked when
                    // clearing out the container
                    // V1.0.0 (2002-09-13) [umoeller]

            BOOL            fSplitViewReady;
                    // while this is FALSE (during the initial setup),
                    // the split view refuses to react any changes in
                    // the containers; this must be set to TRUE explicitly
                    // by the user code

            ULONG           cThreadsRunning;
                    // if > 0, STPR_WAIT is used for the pointer

            // populate thread
            THREADINFO      tiSplitPopulate;
            volatile TID    tidSplitPopulate;
            HWND            hwndSplitPopulate;

        } SPLITCONTROLLER, *PSPLITCONTROLLER;

        #define INSERT_UNLOCKFILTERED       0x10000000

        VOID fdrInsertContents(WPFolder *pFolder,
                               HWND hwndCnr,
                               PMINIRECORDCORE precParent,
                               ULONG flInsert,
                               HWND hwndAddFirstChild,
                               PCSZ pcszFileMask);

        HPOINTER fdrSplitQueryPointer(PSPLITCONTROLLER pctl);

        VOID fdrSplitPopulate(PSPLITCONTROLLER pctl,
                              PMINIRECORDCORE prec,
                              ULONG fl);

        VOID fdrPostFillFolder(PSPLITCONTROLLER pctl,
                               PMINIRECORDCORE prec,
                               ULONG fl);

        BOOL fdrSplitCreateFrame(WPObject *pRootObject,
                                 WPFolder *pRootsFolder,
                                 PSPLITCONTROLLER pctl,
                                 ULONG flFrame,
                                 PFNWP pfnwpSubclass,
                                 PCSZ pcszTitle,
                                 ULONG flSplit,
                                 PCSZ pcszFileMask,
                                 LONG lSplitBarPos);

        VOID fdrSplitDestroyFrame(PSPLITCONTROLLER pctl);

    #endif
    #endif
    #endif

    WPFolder* fdrQueryOpenFolders(WPFolder *pFind,
                                  BOOL fLock);

    HWND fdrCreateSplitView(WPObject *pRootObject,
                            WPFolder *pRootsFolder,
                            ULONG ulView);

#endif


