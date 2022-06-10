
/*
 *@@ ImplCallFileDlg:
 *      this code contacts XFLDR.DLL to show the
 *      XWorkplace file dialog. This does not
 *      return until either an error occurred
 *      or XFLDR.DLL has dismissed the dialog,
 *      either because the user pressed "OK"
 *      or "Cancel".
 *
 *      In summary, this can almost be called
 *      instead of WinFileDlg.
 *
 *      If this fails because FILEDLG contains
 *      options that are presently not supported
 *      by XWP's dialog replacement, we set
 *      *pfCallDefault to TRUE and return NULLHANDLE.
 *      As a result, you should check that flag
 *      on return and call WinFileDlg instead
 *      if the flag is TRUE.
 *
 *@@added V0.9.19 (2002-04-24) [umoeller]
 *@@changed V1.0.2 (2003-03-07) [umoeller]: HWND_DESKTOP owner causes focus problems, fixed
 */

HWND APIENTRY ImplCallFileDlg(HWND hwndOwner,       // in: owner for dialog
                              PFILEDLG pfd,         // in/out: as with WinFileDlg
                              PBOOL pfCallDefault)  // out: set to TRUE if replacement failed
{
    HWND        hwndReturn = NULLHANDLE;

    *pfCallDefault = TRUE;

    // first, some compatibility checks...
    if (    (pfd)
         && (   (!(pfd->fl & (    FDS_CUSTOM
                                | FDS_MODELESS     // we can't handle non-modal dialogs
                                | FDS_MULTIPLESEL   // we can't handle multiple selections for now
                             )
            )))
       )
    {
        // OK:

        // check if XWP is running; if so, a block of
        // named shared memory must exist
        APIRET arc;
        PXWPGLOBALSHARED pXwpGlobalShared = 0;
        if (!(arc = DosGetNamedSharedMem((PVOID*)&pXwpGlobalShared,
                                         SHMEM_XWPGLOBAL, // "\\SHAREMEM\\XWORKPLC\\DMNSHARE.DAT",
                                         PAG_READ | PAG_WRITE)))
        {
            // we can get the shared memory --> XWP running:
            PID     pidWPS = 0;
            TID     tidWPS = 0;
            CHAR    szEmptyTitle[1];
            HWND    hwndNotify;
            HAB     hab;

            szEmptyTitle[0] = 0;

            // create temporary object window for notifications from XFLDR.DLL;
            // this handle is passed to XFLDR.DLL in the shared memory block
            // so it can post WM_USER back to this window, upon which we
            // terminate our modal loop here
            if (    (hwndNotify = WinCreateWindow(HWND_OBJECT,
                                                  WC_STATIC,
                                                  szEmptyTitle,
                                                  0,
                                                  0,0,0,0,
                                                  0,
                                                  HWND_BOTTOM,
                                                  0,
                                                  NULL,
                                                  NULL))
                    // and we need the anchor block for WinGetMsg:
                 && (hab = WinQueryAnchorBlock(hwndNotify))
                    // is thread-1 object window in XFLDR.DLL running?
                 && (pXwpGlobalShared->hwndThread1Object)
                 && (WinIsWindow(hab,
                                 pXwpGlobalShared->hwndThread1Object))
                    // get WPS PID from thread-1 object window
                 && (WinQueryWindowProcess(pXwpGlobalShared->hwndThread1Object,
                                           &pidWPS,     // on stack
                                           &tidWPS))    // on stack
               )
            {
                // yes:
                PXWPFILEDLG pfdShared = NULL;

                // sum up how much shared memory we need:
                // this is the sum of FILEDLG plus buffers for
                // all the strings the stupid caller gave to WinFileDlg
                // (we're cross-process here)

                // 1) at least the size of FILEDLG
                ULONG       cbShared = sizeof(XWPFILEDLG),
                            cTypes = 0;     // count of types in papszITypes

                // 2) add memory for extra fields
                if (pfd->pszTitle)
                    cbShared += strlen(pfd->pszTitle) + 1;
                if (pfd->pszOKButton)
                    cbShared += strlen(pfd->pszOKButton) + 1;

                // 3) type
                if (pfd->pszIType)
                    cbShared += strlen(pfd->pszIType) + 1;
                // and types array: this is especially sick...
                // this is a pointer to an array of PSZ's, so
                // we need:
                // a) 4 bytes for each PSZ in the PSZ's array
                // b) string length + 1 for each string pointed
                //    to from the array
                // c) another 4 bytes for the NULL array terminator
                if (pfd->papszITypeList)
                {
                    PSZ *ppszThis = pfd->papszITypeList[0];
                    while (*ppszThis)
                    {
                        cbShared +=   sizeof(PSZ)           // for the PSZ array item
                                    + strlen(*ppszThis) // string length
                                    + 1;                    // null terminator
                        ppszThis++;
                        cTypes++;           // count the types so we can align
                                            // properly below
                    }
                    cbShared += 4;      // array is terminated with a NULL psz,
                                        // which we must allocate too
                }

                // 4) drives array... ignored for now
                /* if (pfd->pszIType)
                    cbShared += strlen(pfd->pszIDrive) + 1;
                if (pfd->papszIDriveList)
                {
                    PSZ *ppszThis = pfd->papszIDriveList[0];
                    while (*ppszThis)
                    {
                        cbShared += strlen(*ppszThis) + 1;
                        ppszThis++;
                    }
                } */

                // OK, now we know how much memory we need...
                // allocate a block of shared memory with this size
                if (    (!(arc = DosAllocSharedMem((PVOID*)&pfdShared,     // on stack
                                                   NULL,       // unnamed
                                                   cbShared,
                                                   PAG_COMMIT | OBJ_GIVEABLE | OBJ_TILE
                                                       | PAG_READ | PAG_WRITE)))
                     && (pfdShared)
                   )
                {
                    // OK, we got shared memory:
                    PPIB    ppib = NULL;
                    PTIB    ptib = NULL;
                    ULONG   ulCurDisk = 0;
                    ULONG   ulMap = 0;
                    ULONG   cbBuf = CCHMAXPATH;

                    ULONG   cbThis;
                    PBYTE   pb = (PBYTE)pfdShared + sizeof(XWPFILEDLG);
                                // current offset where to copy to

                    // ZERO the structure
                    memset((PBYTE)pfdShared, 0, cbShared);

                    // fix HWND_DESKTOP owner V1.0.2 (2003-03-07) [umoeller]
                    if (hwndOwner == HWND_DESKTOP)
                        hwndOwner = NULLHANDLE;

                    // copy owner window
                    pfdShared->hwndOwner = hwndOwner;

                    // store PID and TID of caller
                    DosGetInfoBlocks(&ptib, &ppib);
                    pfdShared->pidCaller = ppib->pib_ulpid;
                    pfdShared->tidCaller = ptib->tib_ptib2->tib2_ultid;

                    // get the process's current directory so
                    // file dialog can base on that
                    if (    (arc = DosQueryCurrentDisk(&ulCurDisk, &ulMap))
                         || (ulCurDisk == 0)
                       )
                        DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                                        &ulCurDisk,
                                        sizeof(ulCurDisk));

                    pfdShared->szCurrentDir[0] = 'A' + ulCurDisk - 1;
                    pfdShared->szCurrentDir[1] = ':';
                    pfdShared->szCurrentDir[2] = '\\';
                    DosQueryCurrentDir(0,
                                       &pfdShared->szCurrentDir[3],
                                       &cbBuf);

                    // copy FILEDLG
                    memcpy((PBYTE)&pfdShared->fd,
                           (PBYTE)pfd,
                           _min(pfd->cbSize, sizeof(FILEDLG)));

                    // now pack the various fields into the
                    // shared mem AFTER the XWPFILEDLG
                    if (pfd->pszTitle)
                    {
                        cbThis = strlen(pfd->pszTitle) + 1;
                        memcpy(pb, pfd->pszTitle, cbThis);
                        pfdShared->fd.pszTitle = pb;
                        pb += cbThis;
                    }
                    if (pfd->pszOKButton)
                    {
                        cbThis = strlen(pfd->pszOKButton) + 1;
                        memcpy(pb, pfd->pszOKButton, cbThis);
                        pfdShared->fd.pszOKButton = pb;
                        pb += cbThis;
                    }

                    // types array
                    if (pfd->pszIType)
                    {
                        cbThis = strlen(pfd->pszIType) + 1;
                        memcpy(pb, pfd->pszIType, cbThis);
                        pfdShared->fd.pszIType = pb;
                        pb += cbThis;
                    }

                    if (cTypes)     // we counted types in the array above:
                    {
                        ULONG ul;

                        // 1) reserve room for the array of PSZ's;
                        //    that's (cTypes + 1) * sizeof(PSZ) (null-terminator!)

                        PAPSZ papszTarget = (PAPSZ)pb;

                        PSZ *ppszSourceThis = pfd->papszITypeList[0],
                            *ppszTargetThis = papszTarget[0];

                        pb += ((cTypes + 1) * sizeof(PSZ));

                        // 2) pb points to the room for the first string now...
                        //    (after the PSZ's array)

                        for (ul = 0;
                             ul < cTypes;
                             ul++)
                        {
                            // copy this string from buffer to buffer
                            cbThis = strlen(*ppszSourceThis) + 1;
                            memcpy(pb, *ppszSourceThis, cbThis);

                            // set target PSZ array item
                            *ppszTargetThis = pb;
                            // advance pointers
                            pb += cbThis;
                            ppszSourceThis++;
                            ppszTargetThis++;
                        }

                        // set null terminator in target array... christ
                        *ppszTargetThis = 0;

                        pfdShared->fd.papszITypeList = papszTarget;
                    }

                    // drives array
                    /* if (pfd->pszIDrive)
                    {
                        cbThis = strlen(pfd->pszIDrive) + 1;
                        memcpy(pb, pfd->pszIDrive, cbThis);
                        pfdShared->fd.pszIDrive = pb;
                        pb += cbThis;
                    }
                    if (pfd->papszIDriveList)
                    {
                        PSZ *ppszThis = pfd->papszIDriveList[0];
                        PBYTE pbFirst = pb;
                        while (*ppszThis)
                        {
                            cbThis = strlen(*ppszThis) + 1;
                            memcpy(pb, *ppszThis, cbThis);
                            pb += cbThis;

                            ppszThis++;
                        }

                        pfdShared->fd.papszIDriveList = (PVOID)pbFirst;
                    } */

                    // OK, now we got everything in shared memory...
                    // give the WPS access to it
                    if (!(arc = DosGiveSharedMem(pfdShared,
                                                 pidWPS,
                                                 PAG_READ | PAG_WRITE)))
                    {
                        // send this block to XFLDR.DLL;
                        // we can't use WinSendMsg because this would
                        // block the msg queue for the calling thread.
                        // So instead we use WinPostMsg and wait for
                        // XFLDR.DLL to post something back to our
                        // temporary object window here...

                        pfdShared->hwndNotify = hwndNotify;

                        if (WinPostMsg(pXwpGlobalShared->hwndAPIObject,
                                       APIM_FILEDLG,
                                       (MPARAM)pfdShared,
                                       0))
                        {
                            // the file dialog should be showing up now
                            // in the WPS process...

                            QMSG    qmsg;
                            BOOL    fQuit = FALSE;
                            USHORT  fsFrameFlags = 0;

                            // OK, if we got this far:

                            // 1) do not show the default dialog
                            *pfCallDefault = FALSE;

                            // 2) DISABLE the owner window... we need to
                            // simulate a modal dialog here, but this doesn't
                            // really work because the dialog is in the WPS
                            // process, so a couple hacks are needed

                            if (hwndOwner)      // WinEnableWindow
                                WinEnableWindow(hwndOwner, FALSE);

                            // keep processing our message queue until
                            // XFLDR.DLL posts back WM_USER to the
                            // object window
                            while (WinGetMsg(hab,
                                             &qmsg,        // on stack
                                             0,
                                             0,
                                             0))
                            {
                                // current message for our object window?
                                if (    (qmsg.hwnd == hwndNotify)
                                     && (qmsg.msg == WM_USER)
                                   )
                                    // yes: this means the file dlg has been
                                    // dismissed, and we can get outta here
                                    fQuit = TRUE;

                                WinDispatchMsg(hab,
                                               &qmsg);      // on stack

                                if (fQuit)
                                    break;
                            }

                            // return hwndReturn from XFLDR.DLL;
                            // this is either TRUE or FALSE (stupid cast)
                            hwndReturn = pfdShared->hwndReturn;

                            // copy stuff back from shared block (filled
                            // by WPS code) into caller's FILEDLG
                            pfd->lReturn = pfdShared->fd.lReturn;
                            pfd->lSRC = pfdShared->fd.lSRC;
                            memcpy(pfd->szFullFile,
                                   pfdShared->fd.szFullFile,
                                   sizeof(pfd->szFullFile));

                            pfd->ulFQFCount = pfdShared->fd.ulFQFCount;
                            pfd->sEAType = pfdShared->fd.sEAType;
                            // @@todo: papszFQFilename for multiple selections

                            // re-enable the owner and bring it back to top
                            if (hwndOwner)
                            {
                                WinEnableWindow(hwndOwner, TRUE);
                                WinSetActiveWindow(HWND_DESKTOP, hwndOwner);
                            }
                        }
                    } // end DosGiveSharedMem(pfdShared,

                    DosFreeMem(pfdShared);

                } // end DosAllocSharedMem

            } // end if hwndNotify and stuff

            if (hwndNotify)
                WinDestroyWindow(hwndNotify);

        } // end if !arc = DosGetNamedSharedMem((PVOID*)&pXwpGlobalShared,
    } // end if if (    (pfd)

    return hwndReturn;
}

