
/*
 *@@sourcefile shutdown.h:
 *      header file for shutdown.c.
 *
 *@@include #define INCL_DOSPROCESS
 *@@include #define INCL_DOSSEMAPHORES
 *@@include #define INCL_WINWINDOWMGR
 *@@include #define INCL_WINPOINTERS
 *@@include #define INCL_WINSWITCHLIST
 *@@include #include <os2.h>
 *@@include #include "helpers\linklist.h"
 *@@include #include <wpobject.h>
 *@@include #include "startshut\shutdown.h"
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

#ifndef XSHUTDWN_HEADER_INCLUDED
    #define XSHUTDWN_HEADER_INCLUDED

    /********************************************************************
     *
     *   Declarations
     *
     ********************************************************************/

    /*
     *@@ SHUTDOWNPARAMS:
     *      shutdown structure for external calls.
     *      This is also used internally in shutdown.c.
     *
     *@@changed V0.9.5 (2000-08-10) [umoeller]: changed ulRestartWPS
     */

    typedef struct _SHUTDOWNPARAMS
    {
        BOOL        optReboot,
                    optConfirm,
                    optDebug;
        ULONG       ulCloseMode;
            // changed V0.9.19 (2002-04-02) [umoeller]
            // one of the following:
              #define SHUT_SHUTDOWN               1
              #define SHUT_RESTARTWPS             2
              #define SHUT_LOGOFF                 3

        // ULONG       ulRestartWPS;
            // changed V0.9.5 (2000-08-10) [umoeller]:
            // restart Desktop flag, meaning:
            // -- 0: no, do shutdown
            // -- 1: yes, restart Desktop
            // -- 2: yes, restart Desktop, but logoff also
            //          (only if XWPSHELL is running)

        BOOL        optWPSCloseWindows,
                    optAutoCloseVIO,
                    optLog,
                    // optAnimate,          // removed V0.9.12 (2001-05-12) [umoeller]
                    optPowerOff,  // V1.0.5 (2006-06-26) [pr]
                    optDelay,     // V1.0.5 (2006-06-26) [pr]
                    optACPIOff,   // V1.0.5 (2006-06-26) [pr]
#ifndef __NOXSHUTDOWN__
                    optWPSProcessShutdown,
                            // run shutdown folder? (SHUT_RESTARTWPS and SHUT_LOGOFF only;
                            // ignored with SHUT_SHUTDOWN, which always runs it)
                            // added V1.0.1 (2003-01-29) [umoeller]
                    optWPSReuseStartupFolder,
#endif
                    optEmptyTrashCan,
                    optWarpCenterFirst;
        CHAR        szRebootCommand[CCHMAXPATH];
    } SHUTDOWNPARAMS, *PSHUTDOWNPARAMS;

    // traffic light animation
    #define XSD_ANIM_COUNT 8            // animation steps

    typedef struct _SHUTDOWNANIM
    {
        HPOINTER    ahptr[XSD_ANIM_COUNT];
    } SHUTDOWNANIM, *PSHUTDOWNANIM;

    extern SHUTDOWNANIM     G_sdAnim;       // V1.0.0 (2002-09-13) [umoeller]

    /*
     *@@ AUTOCLOSELISTITEM:
     *      this is the user-defined list of auto-close items
     */

    typedef struct _AUTOCLOSELISTITEM
    {
        CHAR        szItemName[100];
        USHORT      usAction;
    } AUTOCLOSELISTITEM, *PAUTOCLOSELISTITEM;

    // auto-close actions
    // #define ACL_CTRL_C              1            removed V0.9.19 (2002-05-23) [umoeller]
    #define ACL_WMCLOSE             2
    #define ACL_KILLSESSION         3
    #define ACL_SKIP                4

    #ifdef SOM_WPObject_h
    #ifdef LINKLIST_HEADER_INCLUDED

        typedef struct SHUTLIST_STRUCT
        {
            SWCNTRL                 swctl;          // system tasklist structure (see PM ref.)
            WPObject                *pObject;       // NULL for non-WPS windows
            CHAR                    szClass[100];   // window class of the task's main window
            LONG                    lSpecial;       // XSD_* flags; > 0 if Desktop or WarpCenter
        } SHUTLISTITEM, *PSHUTLISTITEM;

        /*
         *@@ AUTOCLOSEWINDATA:
         *      data structure used in the "Auto close"
         *      dialog (stored in QWL_USER).
         */

        typedef struct _AUTOCLOSEWINDATA
        {
            PLINKLIST           pllAutoClose;       // changed (V0.9.0)
            SHORT               sSelected;
            PAUTOCLOSELISTITEM  pliSelected;
            USHORT              usItemCount;
            /* LINKLIST           SHUTLISTITEM llShut
                                psliLast; */
        } AUTOCLOSEWINDATA, *PAUTOCLOSEWINDATA;

        /*
         * REBOOTLISTITEM:
         *      data structure for user reboot items.
         */

        typedef struct _REBOOTLISTITEM
        {
            CHAR                szItemName[100];
            CHAR                szCommand[CCHMAXPATH];
        } REBOOTLISTITEM, *PREBOOTLISTITEM;

        #ifdef DOSH_HEADER_INCLUDED
        /*
         *@@ REBOOTWINDATA:
         *      data structure used in the "Reboot items"
         *      dialog (stored in QWL_USER).
         */

        typedef struct _REBOOTWINDATA
        {
            PLINKLIST       pllReboot;              // changed (V0.9.0)
            SHORT           sSelected;
            PREBOOTLISTITEM pliSelected;
            USHORT          usItemCount;

            /* PPARTITIONINFO  ppi;                   // partitions list (V0.9.0)
            USHORT          usPartitions; */

            PPARTITIONSLIST pPartitionsList;        // new partitions list
                                                    // V0.9.9 (2001-04-07) [umoeller]
            BOOL            fPartitionsLoaded;      // TRUE after first attempt,
                                                    // even if failed
        } REBOOTWINDATA, *PREBOOTWINDATA;
        #endif

    #endif // LINKLIST_HEADER_INCLUDED
    #endif // SOM_WPObject_h

    /* ******************************************************************
     *
     *   Shutdown interface
     *
     ********************************************************************/

    VOID xsdQueryShutdownSettings(PSHUTDOWNPARAMS psdp);

    // current shutdown status
    #define XSD_IDLE                0       // not started yet
    #define XSD_CONFIRMING          1       // confirmation window is open
    #define XSD_INITIALIZING        3       // shutdown thread is init'ing
    #define XSD_CLOSINGWINDOWS      4       // currently closing windows
    #define XSD_CANCELLED           5       // user pressed cancel
                                            // (only in a short time frame before
                                            // the shutdown thread has cleaned
                                            // up; we then reset to XSD_IDLE)
    #define XSD_ALLCLOSED_SAVING    6       // all windows closed, saving WPS
    #define XSD_SAVEDONE_FLUSHING   7       // WPS saved, preparing file-system
                                            // flush or WPS restart, and then
                                            // while system is halted

    ULONG xsdQueryShutdownState(VOID);

    BOOL xsdInitiateShutdown(VOID);

    BOOL xsdInitiateRestartWPS(BOOL fLogoff);

    BOOL xsdInitiateShutdownExt(PSHUTDOWNPARAMS psdp);

    /* ******************************************************************
     *
     *   Shutdown settings pages
     *
     ********************************************************************/

    #ifdef NOTEBOOK_HEADER_INCLUDED
        VOID XWPENTRY xsdShutdownInitPage(PNOTEBOOKPAGE pnbp,
                                          ULONG flFlags);

        MRESULT XWPENTRY xsdShutdownItemChanged(PNOTEBOOKPAGE pnbp,
                                       ULONG ulItemID,
                                       USHORT usNotifyCode,
                                       ULONG ulExtra);
    #endif

    /* ******************************************************************
     *
     *   Shutdown helper functions
     *
     ********************************************************************/

    #ifdef LINKLIST_HEADER_INCLUDED
        USHORT xsdLoadAutoCloseItems(PLINKLIST pllItems,
                                     HWND hwndListbox);

        USHORT xsdWriteAutoCloseItems(PLINKLIST pllItems);

    #endif

    VOID xsdLoadAnimation(PSHUTDOWNANIM psda);

    VOID xsdFreeAnimation(PSHUTDOWNANIM psda);

    VOID xsdRestartWPS(HAB hab,
                       BOOL fLogoff);

    APIRET xsdFlushWPS2INI(VOID);

    #ifdef INCL_WINSWITCHLIST
    #ifdef KERNEL_HEADER_INCLUDED
    #ifdef SOM_WPObject_h

        #define XSD_SYSTEM              -1
        #define XSD_INVISIBLE           -2
        #define XSD_DEBUGNEED           -3

        #define XSD_DESKTOP             -4
        #define XSD_WARPCENTER          -5

        #define XSD_WPSOBJECT_CLOSE     0       // close in all modes

        #define XSD_OTHER_OWNED         1       // close with shutdown and logoff,
                                                // but do not close in restart WPS mode

        #define XSD_OTHER_FOREIGN       2       // object is not owned by current user,
                                                // do not close in logoff mode

        typedef struct _SHUTDOWNCONSTS
        {
            PCKERNELGLOBALS pKernelGlobals;
            SOMClass        *pWPDesktop;
            WPObject        *pActiveDesktop;
            HWND            hwndActiveDesktop;
            HWND            hwndOpenWarpCenter; // OPEN_RUNNING view of open WarpCenter

            PID             pidWPS,     // WinQueryWindowProcess(G_hwndMain, &G_pidWPS, NULL);
                            pidPM;      // WinQueryWindowProcess(HWND_DESKTOP, &G_pidPM, NULL);

            HWND            hwndMain,
                                        // dlg with listbox (visible only in debug mode)
                            hwndShutdownStatus;
                                        // status window (always visible)
            HWND            hwndVioDlg;
                                        // if != NULLHANDLE, currently open VIO confirmation
                                        // window

            ULONG           uid;        // user ID if XWPShell is running
                                        // V0.9.19 (2002-04-02) [umoeller]
                                        // -1 otherwise

        } SHUTDOWNCONSTS, *PSHUTDOWNCONSTS;

        VOID xsdGetShutdownConsts(PSHUTDOWNCONSTS pConsts);

        #ifdef XWPSECTY_HEADER_INCLUDED
        LONG xsdIsClosable(HAB hab,
                           PSHUTDOWNCONSTS pConsts,
                           SWENTRY *pSwEntry,
                           WPObject **ppObject,
                           XWPSECID *puidOwner);

        #endif
    #endif
    #endif
    #endif

    /* ******************************************************************
     *
     *   XShutdown dialogs
     *
     ********************************************************************/

    ULONG xsdConfirmShutdown(PSHUTDOWNPARAMS psdParms);

    ULONG xsdConfirmRestartWPS(PSHUTDOWNPARAMS psdParms);

    VOID xsdShowAutoCloseDetails(HWND hwndOwner);

    // MRESULT EXPENTRY fnwpUserRebootOptions(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2);

    VOID xsdShowRebootActions(HWND hwndOwner);

    /* ******************************************************************
     *
     *   Shutdown thread
     *
     ********************************************************************/

    #ifdef THREADS_HEADER_INCLUDED
    void _Optlink fntShutdownThread(PTHREADINFO pti);
    #endif

    /* ******************************************************************
     *
     *   Window list debugging (winlist.c)
     *
     ********************************************************************/

    HWND winlCreateWinListWindow(VOID);

#endif
