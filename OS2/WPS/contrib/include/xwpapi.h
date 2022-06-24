
/*
 * xwpapi.h:
 *      header for public XWorkplace definitions, to be
 *      referenced from any process.
 */

#ifndef XWPAPI_HEADER_INCLUDED
    #define XWPAPI_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   XWPGLOBALS
     *
     ********************************************************************/

    #define SHMEM_XWPGLOBAL          "\\SHAREMEM\\XWORKPLC\\DMNSHARE.DAT"
            // shared memory name of XWPGLOBALSHARED structure
            // THIS NAME MUST NOT BE CHANGED, fafner relies on this

    // PM window class name for XCenter client (needed by XWPDaemon)
    #define WC_XCENTER_FRAME      "XWPCenterFrame"
    #define WC_XCENTER_CLIENT     "XWPCenterClient"

    /*
     *@@ XWPGLOBALSHARED:
     *      block of shared memory which is used for
     *      communication between the XWorkplace daemon
     *      and XFLDR.DLL (kernel.c).
     *
     *      This is allocated upon initial Desktop startup
     *      by initMain and then requested
     *      by the daemon. See xwpdaemn.c for details.
     *
     *      Since the daemon keeps this block requested,
     *      it can be re-accessed by XFLDR.DLL upon
     *      Desktop restarts and be used for storing data
     *      in between WPS session restarts.
     *
     *      The data in this structure is not stored
     *      anywhere. This structure must only be modified
     *      when either the daemon or the WPS is started.
     *      For hook configuration, HOOKCONFIG is used
     *      instead, since the hook does NOT see this
     *      structure.
     */

    typedef struct _XWPGLOBALSHARED
    {
        HWND        hwndDaemonObject;
                // daemon object window (fnwpDaemonObject, xwpdaemn.c);
                // this is set by the daemon after it has created the object window,
                // so if this is != NULLHANDLE, the daemon is running
        HWND        hwndThread1Object;
                // XFLDR.DLL thread-1 object window (krn_fnwpThread1Object, shared\kernel.c);
                // this is set by initMain before starting the daemon
                // and after the WPS re-initializes
        BOOL        fAllHooksInstalled;
                // TRUE if hook is currently installed;
                // dynamically changed by the daemon upon XDM_HOOKINSTALL
        ULONG       ulWPSStartupCount;
                // Desktop startup count maintained by initMain:
                // 1 at first Desktop startup, 2 at next, ...
        BOOL        fProcessStartupFolder;
                // TRUE if startup folder should be processed;
                // set by initMain and XShutdown (upon Desktop restart)

        HWND        hwndAPIObject;
                // XFLDR.DLL API object window, runs on thread-1 also, used
                // with APIM_* messages for external processes
                // V0.9.9 (2001-03-23) [umoeller]

        CHAR        achNLSStrings[3072+1];
                // Daemon-specific NLS strings temporary storage area.  This
                // area is filled by XFLDR.DLL when the NLS DLL is loaded, and
                // the content is read by the daemon when it receives a
                // XDM_NLSCHANGED message.  Can safely handles up to 12 strings.
                // Size is about 3K so that this structure still fits into
                // a 386 memory page.
                // V1.0.0 (2002-09-15) [lafaix]
    } XWPGLOBALSHARED, *PXWPGLOBALSHARED;

    #define APIM_FILEDLG            (WM_USER + 300)

    #define APIM_SHOWHELPPANEL      (WM_USER + 301)

    #ifdef INCL_WINSTDFILE

        /*
         *@@ XWPFILEDLG:
         *      structure to be posted with APIM_FILEDLG.
         *      This must be in shared memory given to the
         *      WPS process.
         *
         *@@added V0.9.9 (2001-03-23) [umoeller]
         */

        typedef struct _XWPFILEDLG
        {
            HWND        hwndOwner;
                            // owner window for file dialog
                            // (will be a different process)

            HWND        hwndNotify;
                            // notify window to receive WM_USER
                            // when file dialog is done
                            // (created on caller's process)

            PID         pidCaller;
            TID         tidCaller;
                            // PID and TID of thread which called WinFileDlg

            CHAR        szCurrentDir[CCHMAXPATH + 4];
                            // current directory (fully qualified) of calling
                            // process; do not change size, fafner relies on this

            FILEDLG     fd;
                            // regular file-dlg structure; all
                            // member pointers must point to
                            // shared memory too!

            HWND        hwndReturn;
                            // return value of fdlgFileDlg

        } XWPFILEDLG, *PXWPFILEDLG;
    #endif

#endif

