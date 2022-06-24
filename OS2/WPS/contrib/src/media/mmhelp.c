
/*
 *@@sourcefile mmhelp.c:
 *      this has generic multimedia helpers which offer
 *      frequently used functions in a more convinent
 *      interface than the ugly mciSendCommand, where
 *      type-safety is somewhat difficult.
 *
 *      This is all new with V0.9.3. Most of this code used
 *      to be in the "Speedy" ("Quick") thread in filesys\xthreads.c
 *      previously.
 *
 *      With V0.9.3, SOUND.DLL is gone also. That DLL was introduced
 *      because if we linked XFLDR.DLL against mmpm2.lib,
 *      XFolder/XWorkplace refused to install on systems without
 *      OS/2 multimedia installed because the DLL imports failed.
 *      Achim HasenmÅller pointed out to me that I could also
 *      dynamically import the MMPM/2 routines into the XFLDR.DLL
 *      itself, so that's what we do here. You'll find function
 *      prototypes for for MMPM/2 functions below, which are
 *      resolved by xmmInit.
 *
 *      Note: Those G_mmio* and G_mci* identifiers are global
 *      variables containing MMPM/2 API entries. Those are
 *      resolved by xmmInit (mmthread.c) and must only be used
 *      after checking xmmQueryStatus.
 *
 *@@added V0.9.3 (2000-04-25) [umoeller]
 *@@header "media\media.h"
 */

/*
 *      Copyright (C) 1997-2003 Ulrich Mîller.
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
 *  7)  headers in filesys\ (as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINPOINTERS

#define INCL_GPI                // required for INCL_MMIO_CODEC
#define INCL_GPIBITMAPS         // required for INCL_MMIO_CODEC
#include <os2.h>

// multimedia includes
#define INCL_MMIOOS2
#define INCL_MCIOS2
#define INCL_MMIOOS2
#define INCL_MMIO_CODEC
#include <os2me.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <io.h>                 // access etc.

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\mmpmh.h"              // MMPM/2 helpers
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\syssound.h"           // system sound helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file

#include "media\media.h"                // XWorkplace multimedia support

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// list of all open MMPM/2 devices: this list
// holds the plain USHORT device ID's
static HMTX        G_hmtxOpenDevices = NULLHANDLE;
static LINKLIST    G_lstOpenDevices;

extern HWND G_hwndMediaObject;      // in mmthread.c

/* ******************************************************************
 *
 *   Device manager
 *
 ********************************************************************/

/*
 *@@ xmmLockDevicesList:
 *      internal helper to make the device functions
 *      thread safe. This requests a mutex semaphore.
 *      xmmUnlockDevicesList must be called to release it.
 *
 *@@added V0.9.3 (2000-04-29) [umoeller]
 */

BOOL xmmLockDevicesList(VOID)
{
    if (G_hmtxOpenDevices)
        return !DosRequestMutexSem(G_hmtxOpenDevices, SEM_INDEFINITE_WAIT);

    // first call:
    if (!DosCreateMutexSem(NULL,
                           &G_hmtxOpenDevices,
                           0,
                           TRUE))
    {
        lstInit(&G_lstOpenDevices,
                FALSE);     // we have USHORT's on the list, so no free

        return TRUE;
    }

    return FALSE;
}

/*
 *@@ xmmUnlockDevicesList:
 *      releases the mutex semaphore requested
 *      by xmmLockDevicesList.
 *
 *@@added V0.9.3 (2000-04-29) [umoeller]
 */

VOID xmmUnlockDevicesList(VOID)
{
    DosReleaseMutexSem(G_hmtxOpenDevices);
}

/*
 *@@ xmmOpenDevice:
 *      opens any multimedia device.
 *
 *      If *pusDeviceID is != 0, this simply
 *      returns TRUE. Otherwise, the device is
 *      opened from the specified parameters
 *      and *pusDeviceID receives the device ID.
 *      As a result, if TRUE is returned, you can
 *      always be sure that the device is open.
 *
 *      If hwndNotify is specified, the device is
 *      opened in "shared" mode, and hwndNotify will
 *      receive MM_MCIPASSDEVICE messages.
 *
 *      Use xmmCloseDevice to close the device again.
 *
 *      This keeps a list of all open devices so that
 *      xmmCleanup can clean up on shutdown.
 *
 *@@added V0.9.3 (2000-04-29) [umoeller]
 */

ULONG xmmOpenDevice(HWND hwndNotify,
                    USHORT usDeviceTypeID,   // in: MCI_DEVTYPE_* ID
                    USHORT usDeviceIndex,    // in: device index (0 for default)
                    PUSHORT pusDeviceID)   // in/out: MMPM/2 device ID
{
    ULONG ulrc = -1;
    if (*pusDeviceID == 0)
    {
        // device not opened yet:
        MCI_OPEN_PARMS  mop;
        memset(&mop, 0, sizeof(mop));

        mop.hwndCallback = hwndNotify;

        // set device type (MCI_OPEN_TYPE_ID):
        // low word is standard device ID,
        // high word is index; if 0, default device will be opened.
        mop.pszDeviceType = (PSZ)(MPFROM2SHORT(usDeviceTypeID,
                                               usDeviceIndex));

        ulrc = G_mciSendCommand(0,  // device ID, ignored on MCI_OPEN
                                MCI_OPEN,
                                MCI_WAIT
                                    | MCI_OPEN_TYPE_ID // pszDeviceType is valid
                                    | MCI_OPEN_SHAREABLE,
                                &mop,
                                0);                              // No user parm

        if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
        {
            // successful: go on
            *pusDeviceID = mop.usDeviceID;     // remember device ID

            if (xmmLockDevicesList())
            {
                lstAppendItem(&G_lstOpenDevices,
                              (PVOID)mop.usDeviceID);
                xmmUnlockDevicesList();
            }
        }
        else
        {
            // no success:
            // we need to close the device again!
            // Even if MCI_OPEN fails, MMPM/2 has allocated resources.
            MCI_GENERIC_PARMS mgp = {0};
            memset(&mgp, 0, sizeof(mgp));
            G_mciSendCommand(mop.usDeviceID,
                             MCI_CLOSE,
                             MCI_WAIT,
                             &mgp,
                             0);
        }
    }
    // else: device already open:

    return ulrc;
}

/*
 *@@ xmmCloseDevice:
 *      closes a device opened by xmmOpenDevice and sets
 *      *pusDeviceID to null.
 *
 *@@added V0.9.3 (2000-04-29) [umoeller]
 */

ULONG xmmCloseDevice(PUSHORT pusDeviceID)
{
    ULONG ulrc;
    MCI_GENERIC_PARMS mgp = {0};
    ulrc = G_mciSendCommand(*pusDeviceID,
                            MCI_CLOSE,
                            MCI_WAIT,
                            &mgp,
                            0);
    if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
    {
        if (xmmLockDevicesList())
        {
            lstRemoveItem(&G_lstOpenDevices,
                          (PVOID)*pusDeviceID);
            xmmUnlockDevicesList();
        }

        *pusDeviceID = 0;
    }

    return ulrc;
}

/*
 *@@ xmmCleanup:
 *      closes all leftover open devices. This is
 *      used during XShutdown because otherwise
 *      the WPS hangs after the Desktop restart. So
 *      much for exit-list cleanup, IBM.
 *
 *@@added V0.9.3 (2000-04-29) [umoeller]
 *@@changed V0.9.12 (2001-05-27) [umoeller]: fixed crashes on exit
 */

VOID xmmCleanup(VOID)
{
    if (xmmLockDevicesList())
    {
        // always delete the first node, because
        // xmmCloseDevice modifies the list
        PLISTNODE pNode = lstQueryFirstNode(&G_lstOpenDevices);
        while (pNode)
        {
            PLISTNODE pNext = pNode->pNext;
            USHORT usDeviceID = (USHORT)(pNode->pItemData);
            #ifdef __DEBUG__
                DosBeep(1000, 100);
            #endif
            xmmCloseDevice(&usDeviceID);
            // lstRemoveNode(&G_lstOpenDevices, pNode);
                // V0.9.7 (2000-12-21) [umoeller]
                // V0.9.12 (2001-05-27) [umoeller]: bull, this was removed already

            pNode = pNext;
        }

        xmmUnlockDevicesList();
    }
}

/* ******************************************************************
 *
 *   Sound helpers
 *
 ********************************************************************/

/*
 *@@ xmmOpenWaveDevice:
 *      this is called by the Media thread (xmm_fnwpMediaObject)
 *      when it receives QM_PLAYSOUND to start playing a sound
 *      and the waveform device is not yet open.
 *
 *      Note: The device is opened "shared" and will only play
 *      anything after hwndObject receives MM_MCIPASSDEVICE.
 *
 *@@changed V0.9.3 (2000-04-26) [umoeller]: this was in SOUND.DLL previously
 *@@changed V0.9.12 (2001-05-27) [umoeller]: mostly rewritten to allow device reuse
 */

ULONG xmmOpenWaveDevice(HWND hwndObject,       // in: Media thread object wnd
                        PUSHORT pusDeviceID)   // out: waveform device ID
{
    ULONG               ulrc = 0;
    MCI_GENERIC_PARMS   mgp;

    if (0 == *pusDeviceID)
    {
        // no device yet:
        // open one then

        PMPF_SOUNDS(("Opening new device"));

        // now we open the default sound device;
        // note, we DO NOT CLOSE the device any more,
        // this is too slow. Instead, we keep loading
        // files into the default sound device.
        // V0.9.12 (2001-05-27) [umoeller]
        ulrc = xmmOpenDevice(hwndObject,
                             MCI_DEVTYPE_WAVEFORM_AUDIO,   // in: MCI_DEVTYPE_* ID
                             0,             // in: device index (0 for default)
                             pusDeviceID);   // in/out: MMPM/2 device ID
        // _PmpfF(("xmmOpenDevice returned 0x%lX", ulrc));
    }
    else
    {
        // go acquire the device
        mgp.hwndCallback = hwndObject;
        ulrc = G_mciSendCommand(*pusDeviceID,
                                MCI_ACQUIREDEVICE,
                                MCI_WAIT,
                                &mgp,
                                0);

        // --  if the device ID was != 0, we could already
        //     be playing something... so send MCI_STOP first
        memset(&mgp, 0, sizeof(mgp));
        ulrc = G_mciSendCommand(*pusDeviceID,
                                MCI_STOP,
                                MCI_WAIT,
                                &mgp,
                                0);
        PMPF_SOUNDS(("MCI_STOP returned 0x%lX", ulrc));
    }

    return ulrc;
}

/*
 *@@ xmmPlaySound:
 *      this is called by the Media thread (xmm_fnwpMediaObject)
 *      when it receives MM_MCIPASSDEVICE with MCI_GAINING_USE
 *      set, i.e. the device is ready to play. So playing
 *      is what we'll do here.
 *
 *@@changed V0.9.3 (2000-04-26) [umoeller]: this was in SOUND.DLL previously
 */

ULONG xmmPlaySound(HWND hwndObject,     // in: Media thread object wnd
                   PUSHORT pusDeviceID, // in: "device" ID (= sound file)
                   const char *pcszFile,
                   ULONG ulVolume)      // in: volume for sound (0-100)
{
    ULONG ulrc;
    MCI_PLAY_PARMS  mpp;
    MCI_SET_PARMS   msp;

    // load the new sound file
    MCI_LOAD_PARMS mlp;
    memset(&mlp, 0, sizeof(mlp));
    mlp.pszElementName = (PSZ)pcszFile;
    ulrc = G_mciSendCommand(*pusDeviceID,
                            MCI_LOAD,
                            MCI_WAIT | MCI_OPEN_ELEMENT | MCI_READONLY,
                            &mlp,
                            0);
    // _PmpfF(("MCI_LOAD returned 0X%LX", ulrc));

    if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
    {
        // set the volume for this sound
        msp.ulLevel = ulVolume;
        msp.ulAudio = MCI_SET_AUDIO_ALL;
        ulrc = G_mciSendCommand(*pusDeviceID,
                                MCI_SET,
                                MCI_WAIT | MCI_SET_AUDIO | MCI_SET_VOLUME,
                                &msp, 0);
        // _PmpfF(("MCI_SET returned 0x%lX", ulrc));

        if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
        {
            // play and request MM_MCINOTIFY msg to
            // the Media thread object window
            mpp.hwndCallback = (HWND)hwndObject;
            mpp.ulFrom = 0;
            ulrc = G_mciSendCommand(*pusDeviceID,
                                    MCI_PLAY,
                                    MCI_FROM | MCI_NOTIFY,
                                    (PVOID)&mpp,
                                    0);
            // _PmpfF(("MCI_PLAY returned 0x%lX", ulrc));
        }
    }

    return ulrc;
}

/*
 *@@ xmmStopSound:
 *      this is called by the Media thread (xmm_fnwpMediaObject)
 *      in two situations:
 *
 *      1)  MMPM/2 is done playing our sound, i.e.
 *          upon receiving MM_MCINOTIFY;
 *
 *      2)  another application requests the waveform
 *          device for playing, i.e. upon receiving
 *          MM_MCIPASSDEVICE with MCI_LOSING_USE set.
 *
 *      In both situations, we need to stop playing.
 *
 *@@changed V0.9.3 (2000-04-26) [umoeller]: this was in SOUND.DLL previously
 *@@changed V0.9.12 (2001-05-27) [umoeller]: no longer closing device, release it instead
 */

ULONG xmmStopSound(PUSHORT pusDeviceID)
{
    ULONG ulrc;
    MCI_GENERIC_PARMS mgp;
    // stop playing the sound (this will probably do
    // nothing if the sound is already done with)
    memset(&mgp, 0, sizeof(mgp));
    ulrc = G_mciSendCommand(*pusDeviceID,
                            MCI_STOP,
                            MCI_WAIT,
                            &mgp,
                            0);
    // _PmpfF(("MCI_STOP returned 0x%lX", ulrc));

    // go release the device
    ulrc = G_mciSendCommand(*pusDeviceID,
                            MCI_RELEASEDEVICE,
                            MCI_RETURN_RESOURCE, // MCI_WAIT,
                            &mgp,
                            0);
    // _PmpfF(("MCI_RELEASEDEVICE returned 0x%lX", ulrc));

    return ulrc;
}

/* ******************************************************************
 *
 *   CD player helpers
 *
 ********************************************************************/

/*
 *@@ fnwpCDPlayerObject:
 *      private object window for position advise.
 *
 *@@added V0.9.12 (2001-05-27) [umoeller]
 */

MRESULT EXPENTRY fnwpCDPlayerObject(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        /*
         * MM_MCIPOSITIONCHANGE:
         *
         */

        case MM_MCIPOSITIONCHANGE:
        {
            if (xmmLockDevicesList())
            {
                PXMMCDPLAYER pPlayer = (PXMMCDPLAYER)WinQueryWindowPtr(hwnd, QWL_USER);
                if (pPlayer)
                {
                    USHORT  usDeviceID = SHORT2FROMMP(mp1);

                    if (    (usDeviceID == pPlayer->usDeviceID)
                         && (pPlayer->hwndNotify)
                       )
                    {
                        ULONG   ulMMTime = (ULONG)mp2;

                        ULONG   ulSeconds;
                        ULONG   ulTrack = xmmCDCalcTrack(pPlayer,
                                                         ulMMTime,
                                                         &ulSeconds);
                        MPARAM  mp1Post = (MPARAM)-1,
                                mp2Post = (MPARAM)-1;
                        if (ulTrack != pPlayer->ulTrack)
                        {
                            pPlayer->ulTrack = ulTrack;
                            mp1Post = (MPARAM)ulTrack;
                        }
                        if (ulSeconds != pPlayer->ulSecondsInTrack)
                        {
                            pPlayer->ulSecondsInTrack = ulSeconds;
                            mp2Post = (MPARAM)ulSeconds;
                        }

                        WinPostMsg(pPlayer->hwndNotify,
                                   pPlayer->ulNotifyMsg,
                                   mp1Post,
                                   mp2Post);
                    }
                }
                xmmUnlockDevicesList();
            } // end if (xmmLockDevicesList())
        }
        break;

        default:
            mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ xmmCDOpenDevice:
 *      opens the CD device by calling xmmOpenDevice
 *      and prepares it for playing.
 *
 *      This creates a new XMMCDPLAYER structure in
 *      *ppPlayer which should be closed using
 *      xmmCDCloseDevice when done.
 *
 *@@added V0.9.3 (2000-04-25) [umoeller]
 *@@changed V0.9.12 (2001-05-27) [umoeller]: now returning MMPM2 error code
 *@@changed V0.9.12 (2001-05-27) [umoeller]: fixed too many things to count
 */

ULONG xmmCDOpenDevice(PXMMCDPLAYER *ppPlayer,
                      ULONG ulDeviceIndex)       // 0 for default
{
    ULONG ulrc = 0;
    if (ppPlayer)
    {
        PXMMCDPLAYER pPlayer = malloc(sizeof(*pPlayer));
        if (pPlayer)
        {
            memset(pPlayer, 0, sizeof(*pPlayer));

            ulrc = xmmOpenDevice(NULLHANDLE,        // no notify
                                 MCI_DEVTYPE_CD_AUDIO,
                                 ulDeviceIndex,
                                 &pPlayer->usDeviceID);
            _PmpfF(("xmmOpenDevice returned 0x%lX, ID is 0x%lX",
                                    ulrc,
                                    pPlayer->usDeviceID));

            if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
            {
                // create our private object window
                pPlayer->hwndObject = winhCreateObjectWindow(WC_STATIC,
                                                             NULL);
                WinSetWindowPtr(pPlayer->hwndObject,
                                QWL_USER,
                                pPlayer);
                WinSubclassWindow(pPlayer->hwndObject,
                                  fnwpCDPlayerObject);

                // give caller the new player
                *ppPlayer = pPlayer;
            }
            else
                free(pPlayer);
        }
    }

    return ulrc;
}

/*
 *@@ xmmCDCloseDevice:
 *      closes the CD player again.
 *
 *@@added V0.9.7 (2000-12-21) [umoeller]
 *@@changed V0.9.12 (2001-05-27) [umoeller]: now returning MMPM2 error code
 *@@changed V0.9.12 (2001-05-27) [umoeller]: fixed too many things to count
 */

ULONG xmmCDCloseDevice(PXMMCDPLAYER *ppPlayer)
{
    ULONG ulrc = -1;
    if (ppPlayer)
    {
        PXMMCDPLAYER pPlayer;
        if (pPlayer = *ppPlayer)
        {
            ulrc = xmmCloseDevice(&pPlayer->usDeviceID);

            _PmpfF(("xmmCloseDevice returned 0x%lX", ulrc));

            if (pPlayer->hwndNotify && pPlayer->ulNotifyMsg)
                WinPostMsg(pPlayer->hwndNotify,
                           pPlayer->ulNotifyMsg,
                           0,
                           0);

            if (pPlayer->aTocEntries)
                free(pPlayer->aTocEntries);
            pPlayer->aTocEntries = NULL;
            pPlayer->cTocEntries = 0;

            winhDestroyWindow(&pPlayer->hwndObject);

            free(pPlayer);
            *ppPlayer = NULL;
        }
    }

    return ulrc;
}

/*
 *@@ xmmCDGetTOC:
 *      retrieves the CD's table of contents.
 *      This operates synchronously.
 *
 *      This sets cTracks, aTocEntries and
 *      cTocEntries in the XMMCDPLAYER.
 *
 *      The MCI_TOC_REC structure is defined
 *      as follows:
 *
 +      typedef struct _MCI_TOC_REC {
 +          BYTE       TrackNum;     //  Track number.
 +          ULONG      ulStartAddr;  //  Starting address in MMTIME.
 +          ULONG      ulEndAddr;    //  Ending address in MMTIME.
 +          BYTE       Control;      //  Track control info.
 +          USHORT     usCountry;    //  Country.
 +          ULONG      ulOwner;      //  Owner.
 +          ULONG      ulSerialNum;  //  Serial number.
 +      } MCI_TOC_REC;
 +      typedef MCI_TOC_REC *PTOCREC;
 *
 *@@added V0.9.7 (2000-12-20) [umoeller]
 *@@changed V0.9.12 (2001-05-27) [umoeller]: now returning MMPM2 error code
 *@@changed V0.9.12 (2001-05-27) [umoeller]: fixed too many things to count
 */

ULONG xmmCDGetTOC(PXMMCDPLAYER pPlayer)
{
    ULONG ulrc = -1;

    if (xmmLockDevicesList())
    {
        if ((pPlayer) && (pPlayer->usDeviceID))
        {
            MCI_TOC_PARMS mtp = {0};
            MCI_STATUS_PARMS msp = {0};

            // clear old toc
            if (pPlayer->aTocEntries)
                free(pPlayer->aTocEntries);
            pPlayer->aTocEntries = NULL;
            pPlayer->cTocEntries = 0;

            // get no. of tracks
            msp.ulItem = MCI_STATUS_NUMBER_OF_TRACKS;

            ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                    MCI_STATUS,
                                    MCI_WAIT
                                        | MCI_STATUS_ITEM, // msp.ulItem is valid
                                    &msp,
                                    0);                              // No user parm

            _PmpfF(("MCI_STATUS returned 0x%lX", ulrc));

            if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
            {
                pPlayer->cTracks = LOUSHORT(msp.ulReturn);

                // now get the toc
                ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                        MCI_GETTOC,
                                        MCI_WAIT,
                                        &mtp,
                                        0);                              // No user parm
                if (mtp.ulBufSize)
                {
                    mtp.pBuf = malloc(mtp.ulBufSize);
                    ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                            MCI_GETTOC,
                                            MCI_WAIT,
                                            &mtp,
                                            0);                              // No user parm
                    if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
                    {
                        pPlayer->aTocEntries = mtp.pBuf;
                        pPlayer->cTocEntries = mtp.ulBufSize / sizeof(MCI_TOC_REC);
                    }
                    else
                    {
                        CHAR szError[1000];
                        G_mciGetErrorString(ulrc, szError, sizeof(szError));
                        _PmpfF(("DeviceID %d has error %d (\"%s\")",
                               pPlayer->usDeviceID, LOUSHORT(ulrc), szError));

                        free(mtp.pBuf);
                    }
                }
            }
            else
            {
                CHAR szError[1000];
                G_mciGetErrorString(ulrc, szError, sizeof(szError));
                _PmpfF(("DeviceID %d has error %d (\"%s\")",
                       pPlayer->usDeviceID, LOUSHORT(ulrc), szError));
            }
        }
        xmmUnlockDevicesList();
    }

    return ulrc;
}

/*
 *@@ xmmCDQueryStatus:
 *      returns the status of a CD device opened
 *      with xmmCDOpenDevice.
 *
 *      This operates synchronously.
 *
 *      Returns one of the following:
 *      -- 0: invalid device.
 *      -- MCI_MODE_NOT_READY
 *      -- MCI_MODE_PAUSE
 *      -- MCI_MODE_PLAY
 *      -- MCI_MODE_STOP
 *
 *@@added V0.9.3 (2000-04-25) [umoeller]
 */

ULONG xmmCDQueryStatus(USHORT usDeviceID)
{
    ULONG ulReturn = 0,
          ulrc;

    if (xmmLockDevicesList())
    {
        if (usDeviceID)
        {
            MCI_STATUS_PARMS msp = {0};

            msp.ulItem = MCI_STATUS_MODE;

            ulrc = G_mciSendCommand(usDeviceID,
                                    MCI_STATUS,
                                    MCI_WAIT
                                        | MCI_STATUS_ITEM, // msp.ulItem is valid
                                    &msp,
                                    0);                              // No user parm

            _PmpfF(("MCI_STATUS returned 0x%lX", ulrc));

            if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
                ulReturn = LOUSHORT(msp.ulReturn);
            else
            {
                CHAR szError[1000];
                G_mciGetErrorString(ulrc, szError, sizeof(szError));
                _PmpfF(("DeviceID %d has error %d (\"%s\")",
                       usDeviceID, LOUSHORT(ulrc), szError));
            }
        }
        xmmUnlockDevicesList();
    }

    return ulReturn;
}

/*
 *@@ xmmCDQueryCurrentTrack:
 *      returns the current track of the CD,
 *      no matter whether the CD is playing
 *      or not.
 *
 *      Returns 0 on errors.
 *
 *@@added V0.9.3 (2000-04-25) [umoeller]
 *@@changed V0.9.12 (2001-05-27) [umoeller]: now returning MMPM2 error code
 *@@changed V0.9.13 (2001-06-14) [umoeller]: now using position advise, prototype changed
 */

ULONG xmmCDQueryCurrentTrack(PXMMCDPLAYER pPlayer)
{
    ULONG ulReturn = 0,
          ulrc;

    if (xmmLockDevicesList())
    {
        if (pPlayer->usDeviceID)
        {
            if (    (    (pPlayer->ulStatus == MCI_MODE_PAUSE)
                      || (pPlayer->ulStatus == MCI_MODE_PLAY)
                    )
                 && (pPlayer->fPositionAdvising)
               )
            {
                // currently playing, and position advise is on:
                // use current track then
                ulReturn = pPlayer->ulTrack;
            }
            else
            {
                MCI_STATUS_PARMS msp = {0};

                msp.ulItem = MCI_STATUS_CURRENT_TRACK;

                ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                        MCI_STATUS,
                                        MCI_WAIT
                                            | MCI_STATUS_ITEM, // msp.ulItem is valid
                                        &msp,
                                        0);                              // No user parm

                _PmpfF(("MCI_STATUS returned 0x%lX", ulrc));

                if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
                    ulReturn = LOUSHORT(msp.ulReturn);
                else
                {
                    CHAR szError[1000];
                    G_mciGetErrorString(ulrc, szError, sizeof(szError));
                    _PmpfF(("DeviceID %d has error %d (\"%s\")",
                           pPlayer->usDeviceID, LOUSHORT(ulrc), szError));
                }
            }
        } // end if (pPlayer->usDeviceID)
        // else: not even a device ID, return 0

        xmmUnlockDevicesList();
    }

    return ulReturn;
}

/*
 *@@ xmmCDPlay:
 *      starts playing if the CD is currently
 *      stopped or paused. Otherwise, this does
 *      nothing and returns FALSE.
 *
 *@@added V0.9.3 (2000-04-27) [umoeller]
 *@@changed V0.9.12 (2001-05-27) [umoeller]: now returning MMPM2 error code
 *@@changed V0.9.12 (2001-05-27) [umoeller]: fixed too many things to count
 */

ULONG xmmCDPlay(PXMMCDPLAYER pPlayer,
                BOOL fShowWaitPointer)
{
    ULONG       ulrc = -1;
    HPOINTER    hptrOld = NULLHANDLE;
    if (fShowWaitPointer)
        hptrOld = winhSetWaitPointer();

    if (xmmLockDevicesList())
    {
        if ((pPlayer) && (pPlayer->usDeviceID))
        {
            // ULONG ulNewTrack = 0;

            if (    (pPlayer->ulStatus != MCI_MODE_PAUSE)
                 && (pPlayer->ulStatus != MCI_MODE_PLAY)
               )
            {
                // device stopped:
                // start at beginning
                ulrc = xmmCDPlayTrack(pPlayer,
                                      1,
                                      fShowWaitPointer);
                                   // this also gets the TOC

                _PmpfF(("xmmCDPlayTrack returned 0x%lX", ulrc));

                /* MCI_PLAY_PARMS mpp = {0};
                ULONG fl = 0;
                ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                        MCI_PLAY,
                                        MCI_WAIT,
                                        &mpp,      // from and to are ignored,
                                                   // since we've not set the flags
                                        0);
                ulNewStatus = MCI_MODE_PLAY;
                ulNewTrack = 1; */
            }
            else if (pPlayer->ulStatus == MCI_MODE_PAUSE)
            {
                // device paused:
                // resume
                MCI_GENERIC_PARMS mgp = {0};
                mgp.hwndCallback = pPlayer->hwndObject;
                ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                        MCI_RESUME,
                                        MCI_WAIT,
                                        &mgp,
                                        0);

                _PmpfF(("MCI_RESUME returned 0x%lX", ulrc));

                if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
                {
                    pPlayer->ulStatus = MCI_MODE_PLAY;
                }
                else
                {
                    CHAR szError[1000];
                    G_mciGetErrorString(ulrc, szError, sizeof(szError));
                    _PmpfF(("DeviceID %d has error %d (\"%s\")",
                           pPlayer->usDeviceID, LOUSHORT(ulrc), szError));
                }
            }
        }
        xmmUnlockDevicesList();
    }

    if (hptrOld)
        WinSetPointer(HWND_DESKTOP, hptrOld);

    return ulrc;
}

/*
 *@@ xmmCDPlayTrack:
 *      starts playing at the specified track.
 *
 *@@added V0.9.3 (2000-04-25) [umoeller]
 *@@changed V0.9.12 (2001-05-27) [umoeller]: now returning MMPM2 error code
 *@@changed V0.9.12 (2001-05-27) [umoeller]: fixed too many things to count
 */

ULONG xmmCDPlayTrack(PXMMCDPLAYER pPlayer,
                     USHORT usTrack,
                     BOOL fShowWaitPointer)
{
    ULONG       ulrc = -1;
    HPOINTER    hptrOld = NULLHANDLE;
    if (fShowWaitPointer)
        hptrOld = winhSetWaitPointer();

    if (xmmLockDevicesList())
    {
        if ((pPlayer) && (pPlayer->usDeviceID))
        {
            if (pPlayer->aTocEntries == 0)
            {
                // ain't got no toc yet:
                // go get it
                ulrc = xmmCDGetTOC(pPlayer);
                _PmpfF(("xmmCDGetTOC returned 0x%lX", ulrc));
            }

            if (pPlayer->aTocEntries)
            {
                // switch time format to tracks
                MCI_SET_PARMS msetp = {0};
                msetp.ulTimeFormat = MCI_FORMAT_TMSF;
                ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                        MCI_SET,
                                        MCI_WAIT
                                            | MCI_SET_TIME_FORMAT,
                                        &msetp,
                                        0);

                _PmpfF(("MCI_SET returned 0x%lX", ulrc));

                if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
                {
                    MCI_PLAY_PARMS mpp = {0};

                    if (usTrack < 0)
                        usTrack = 1;
                    if (usTrack > pPlayer->cTracks)
                        return FALSE;

                    TMSF_TRACK(mpp.ulFrom) = usTrack;
                    TMSF_MINUTE(mpp.ulFrom) = 0;
                    TMSF_SECOND(mpp.ulFrom) = 0;
                    TMSF_FRAME(mpp.ulFrom) = 0;
                    ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                            MCI_PLAY,
                                            MCI_FROM,
                                                // do not MCI_WAIT here,
                                                // this will never return
                                            &mpp,
                                            0);

                    _PmpfF(("MCI_PLAY from returned 0x%lX", ulrc));

                    if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
                    {
                        pPlayer->ulStatus = MCI_MODE_PLAY;
                    }
                    else
                    {
                        CHAR szError[1000];
                        G_mciGetErrorString(ulrc, szError, sizeof(szError));
                        _PmpfF(("DeviceID %d has error %d (\"%s\")",
                               pPlayer->usDeviceID, LOUSHORT(ulrc), szError));
                    }
                }
                else
                {
                    CHAR szError[1000];
                    G_mciGetErrorString(ulrc, szError, sizeof(szError));
                    _PmpfF(("DeviceID %d has error %d (\"%s\")",
                           pPlayer->usDeviceID, LOUSHORT(ulrc), szError));
                }
            }
        }
        xmmUnlockDevicesList();
    }

    if (hptrOld)
        WinSetPointer(HWND_DESKTOP, hptrOld);

    return ulrc;
}

/*
 *@@ xmmCDCalcTrack:
 *      calculates the track and the currently elapsed
 *      seconds in the track from the given ulMMTime.
 *
 *      The track (1-99) is returned.
 *
 *      Returns 0 if the track could not be found.
 *
 *      The caller must lock pPlayer first.
 */

ULONG xmmCDCalcTrack(PXMMCDPLAYER pPlayer,
                     ULONG ulMMTime,
                     PULONG pulSecondsInTrack)
{
    ULONG           ul;
    ULONG           ulTrack = 0,
                    ulMSInTrack = 0;
    MCI_TOC_REC     *pTocEntryFound = NULL;

    for (ul = 0;
         ul < pPlayer->cTocEntries;
         ul++)
    {
        MCI_TOC_REC     *pTocEntryThis = &pPlayer->aTocEntries[ul];
        if (    (pTocEntryThis->ulStartAddr < ulMMTime)
             && (pTocEntryThis->ulEndAddr > ulMMTime)
           )
        {
            pTocEntryFound = pTocEntryThis;
            break;
        }
    }

    if (pTocEntryFound)
    {
        ulTrack = pTocEntryFound->TrackNum;
        ulMSInTrack = MSECFROMMM(ulMMTime - pTocEntryFound->ulStartAddr);
    }

    *pulSecondsInTrack = ulMSInTrack / 1000;

    return ulTrack;
}

/*
 *@@ xmmCDPositionAdvise:
 *      intializes position advise messages on the media
 *      thread. Gets called automatically from xmmCDPlayTrack
 *      and xmmCDPlay.
 *
 *      If hwndNotify is NULLHANDLE, notifiations are stopped.
 *
 *@@added V0.9.7 (2000-12-20) [umoeller]
 *@@changed V0.9.12 (2001-05-27) [umoeller]: now returning MMPM2 error code
 *@@changed V0.9.12 (2001-05-27) [umoeller]: added private object window for notify
 */

ULONG xmmCDPositionAdvise(PXMMCDPLAYER pPlayer,
                          HWND hwndNotify,
                          ULONG ulNotifyMsg)
{
    ULONG ulrc = -1,
          fl = MCI_WAIT;

    if (xmmLockDevicesList())
    {
        if ((pPlayer) && (pPlayer->usDeviceID))
        {
            MCI_POSITION_PARMS mpp = {0};
            MCI_SET_PARMS msetp = {0};

            if (!hwndNotify)
                pPlayer->hwndNotify = NULLHANDLE;

            // switch time format to milliseconds
            msetp.ulTimeFormat = MCI_FORMAT_MMTIME;
            ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                    MCI_SET,
                                    MCI_WAIT
                                        | MCI_SET_TIME_FORMAT,
                                    &msetp,
                                    0);

            mpp.hwndCallback = pPlayer->hwndObject;
            mpp.ulUnits = MSECTOMM(200);

            if (hwndNotify)
                fl |= MCI_SET_POSITION_ADVISE_ON;
            else
                fl |= MCI_SET_POSITION_ADVISE_OFF;

            ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                    MCI_SET_POSITION_ADVISE,
                                    fl,
                                    &mpp,
                                    0);

            if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
            {
                pPlayer->fPositionAdvising = (hwndNotify != NULLHANDLE);
                pPlayer->hwndNotify = hwndNotify;
                pPlayer->ulNotifyMsg = ulNotifyMsg;
            }
            else
            {
                CHAR szError[1000];
                G_mciGetErrorString(ulrc, szError, sizeof(szError));
                _PmpfF(("DeviceID %d has error %d (\"%s\")",
                       pPlayer->usDeviceID, LOUSHORT(ulrc), szError));
                pPlayer->hwndNotify = hwndNotify;
            }
        }
        xmmUnlockDevicesList();
    }

    return ulrc;
}

/*
 *@@ xmmCDPause:
 *      pauses the CD, which should be currently
 *      playing.
 *
 *@@added V0.9.3 (2000-04-27) [umoeller]
 *@@changed V0.9.12 (2001-05-27) [umoeller]: now returning MMPM2 error code
 *@@changed V0.9.12 (2001-05-27) [umoeller]: status update was missing, fixed
 */

ULONG xmmCDPause(PXMMCDPLAYER pPlayer)
{
    ULONG ulrc = -1;
    if (xmmLockDevicesList())
    {
        if ((pPlayer) && (pPlayer->usDeviceID))
        {
            MCI_GENERIC_PARMS mgp = {0};
            mgp.hwndCallback = pPlayer->hwndObject;
            ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                    MCI_PAUSE,
                                    MCI_WAIT,
                                    &mgp,
                                    0);
            _PmpfF(("MCI_PAUSE returned 0x%lX", ulrc));

            if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
                pPlayer->ulStatus = MCI_MODE_PAUSE;
        }
        xmmUnlockDevicesList();
    }

    return ulrc;
}

/*
 *@@ xmmCDStop:
 *      stops the CD playing and closes the
 *      device by calling xmmCDCloseDevice.
 *
 *@@added V0.9.7 (2000-12-20) [umoeller]
 *@@changed V0.9.12 (2001-05-27) [umoeller]: now returning MMPM2 error code
 *@@changed V0.9.12 (2001-05-27) [umoeller]: now setting ppPlayer to NULL, changed prototype
 */

ULONG xmmCDStop(PXMMCDPLAYER *ppPlayer)
{
    ULONG ulrc = -1;
    if (xmmLockDevicesList())
    {
        PXMMCDPLAYER pPlayer;
        if (pPlayer = *ppPlayer)
        {
            if (pPlayer->usDeviceID)
            {
                if (xmmCDQueryStatus(pPlayer->usDeviceID) == MCI_MODE_PLAY)
                {
                    MCI_GENERIC_PARMS mgp = {0};
                    mgp.hwndCallback = pPlayer->hwndObject;
                    ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                            MCI_STOP,
                                            MCI_WAIT,
                                            &mgp,
                                            0);
                    if (LOUSHORT(ulrc) != MCIERR_SUCCESS)
                    {
                        CHAR szError[1000];
                        G_mciGetErrorString(ulrc, szError, sizeof(szError));
                        _PmpfF(("DeviceID %d has error %d (\"%s\")",
                               pPlayer->usDeviceID, LOUSHORT(ulrc), szError));
                    }
                }
            }

            xmmCDCloseDevice(ppPlayer);     // sets *ppPlayer to NULL
        }
        xmmUnlockDevicesList();
    }

    return ulrc;
}

/*
 *@@ xmmCDEject:
 *      ejects the CD and closes the device
 *      by calling xmmCDCloseDevice.
 *
 *@@added V0.9.7 (2000-12-20) [umoeller]
 *@@changed V0.9.12 (2001-05-27) [umoeller]: now returning MMPM2 error code
 *@@changed V0.9.12 (2001-05-27) [umoeller]: now setting ppPlayer to NULL, changed prototype
 */

ULONG xmmCDEject(PXMMCDPLAYER *ppPlayer)
{
    ULONG ulrc = -1;
    if (xmmLockDevicesList())
    {
        PXMMCDPLAYER pPlayer;
        if (pPlayer = *ppPlayer)
        {
            if (pPlayer->usDeviceID)
            {
                MCI_SET_PARMS msetp = {0};

                ulrc = G_mciSendCommand(pPlayer->usDeviceID,
                                        MCI_SET,
                                        MCI_SET_DOOR_OPEN,
                                        &msetp,
                                        0);
                if (LOUSHORT(ulrc) != MCIERR_SUCCESS)
                {
                    CHAR szError[1000];
                    G_mciGetErrorString(ulrc, szError, sizeof(szError));
                    _PmpfF(("DeviceID %d has error %d (\"%s\")",
                           pPlayer->usDeviceID, LOUSHORT(ulrc), szError));
                }
            }

            xmmCDCloseDevice(ppPlayer); // sets *ppPlayer to NULL
        }
        xmmUnlockDevicesList();
    }

    return ulrc;
}

/* ******************************************************************
 *
 *   Master volume helpers
 *
 ********************************************************************/

/*
 *@@ xmmQueryMasterVolume:
 *      returns the current master volume setting,
 *      which is a percentage in the range of 0 to 100.
 *
 *@@added V0.9.12 (2001-05-27) [umoeller]
 */

ULONG xmmQueryMasterVolume(PULONG pulVolume)
{
    ULONG ulrc = -1;

    MCI_MASTERAUDIO_PARMS mvp;
    memset(&mvp, 0, sizeof(mvp));

    ulrc = G_mciSendCommand(0,              // device ID for master audio!
                            MCI_MASTERAUDIO,
                            MCI_MASTERVOL | MCI_QUERYCURRENTSETTING | MCI_WAIT,
                            &mvp,
                            0);
    _PmpfF(("MCI_MASTERAUDIO returned 0x%lX", ulrc));

    if (LOUSHORT(ulrc) == MCIERR_SUCCESS)
        *pulVolume = mvp.ulReturn;

    return ulrc;
}

/*
 *@@ xmmSetMasterVolume:
 *      sets a new master volume value, which must
 *      be in the range of 0 to 100.
 *
 *@@added V0.9.12 (2001-05-27) [umoeller]
 */

ULONG xmmSetMasterVolume(ULONG ulVolume)
{
    ULONG ulrc = -1;

    MCI_MASTERAUDIO_PARMS mvp;
    memset(&mvp, 0, sizeof(mvp));

    mvp.ulMasterVolume = ulVolume;

    ulrc = G_mciSendCommand(0,              // device ID for master audio!
                            MCI_MASTERAUDIO,
                            MCI_MASTERVOL | MCI_WAIT,
                            &mvp,
                            0);

    return ulrc;
}

/* ******************************************************************
 *
 *   MMPM/2 configuration queries
 *
 ********************************************************************/

/* typedef struct _DEVICETYPE
{
    ULONG   ulDeviceTypeID;
    PSZ     pszDeviceType;
} DEVICETYPE, *PDEVICETYPE; */

/* DEVICETYPE aDeviceTypes[] =
        {
            MCI_DEVTYPE_VIDEOTAPE, "Video tape",
            MCI_DEVTYPE_VIDEODISC, "Video disc",
            MCI_DEVTYPE_CD_AUDIO, "CD Audio",
            MCI_DEVTYPE_DAT, "DAT",
            MCI_DEVTYPE_AUDIO_TAPE, "Audio tape",
            MCI_DEVTYPE_OTHER, "Other",
            MCI_DEVTYPE_WAVEFORM_AUDIO, "Wave audio",
            MCI_DEVTYPE_SEQUENCER, "Sequencer",
            MCI_DEVTYPE_AUDIO_AMPMIX, "Ampmix",
            MCI_DEVTYPE_OVERLAY, "Overlay",
            MCI_DEVTYPE_ANIMATION, "Animation",
            MCI_DEVTYPE_DIGITAL_VIDEO, "Digital video",
            MCI_DEVTYPE_SPEAKER, "Speaker",
            MCI_DEVTYPE_HEADPHONE, "Headphone",
            MCI_DEVTYPE_MICROPHONE, "Microphone",
            MCI_DEVTYPE_MONITOR, "Monitor",
            MCI_DEVTYPE_CDXA, "CDXA",
            MCI_DEVTYPE_FILTER, "Filter",       // Warp 4 only
            MCI_DEVTYPE_TTS, "Text-to-speech"
        }; */

/*
 *@@ GetDeviceTypeName:
 *      returns an NLS string describing the
 *      input MCI_DEVTYPE_* device type.
 *
 *@@added V0.9.7 (2000-11-30) [umoeller]
 */

STATIC const char* GetDeviceTypeName(ULONG ulDeviceType)
{
    ULONG   ulStringID = 0;
    // const char *prc = "Unknown";

    switch (ulDeviceType)
    {
        case MCI_DEVTYPE_VIDEOTAPE:
            ulStringID = ID_MMSI_DEVTYPE_VIDEOTAPE;  // pszDevTypeVideotape
        break;

        case MCI_DEVTYPE_VIDEODISC:
            ulStringID = ID_MMSI_DEVTYPE_VIDEODISC;  // pszDevTypeVideodisc
        break;

        case MCI_DEVTYPE_CD_AUDIO:
            ulStringID = ID_MMSI_DEVTYPE_CD_AUDIO;  // pszDevTypeCDAudio
        break;

        case MCI_DEVTYPE_DAT:
            ulStringID = ID_MMSI_DEVTYPE_DAT;  // pszDevTypeDAT
        break;

        case MCI_DEVTYPE_AUDIO_TAPE:
            ulStringID = ID_MMSI_DEVTYPE_AUDIO_TAPE;  // pszDevTypeAudioTape
        break;

        case MCI_DEVTYPE_OTHER:
            ulStringID = ID_MMSI_DEVTYPE_OTHER;  // pszDevTypeOther
        break;

        case MCI_DEVTYPE_WAVEFORM_AUDIO:
            ulStringID = ID_MMSI_DEVTYPE_WAVEFORM_AUDIO;  // pszDevTypeWaveformAudio
        break;

        case MCI_DEVTYPE_SEQUENCER:
            ulStringID = ID_MMSI_DEVTYPE_SEQUENCER;  // pszDevTypeSequencer
        break;

        case MCI_DEVTYPE_AUDIO_AMPMIX:
            ulStringID = ID_MMSI_DEVTYPE_AUDIO_AMPMIX;  // pszDevTypeAudioAmpmix
        break;

        case MCI_DEVTYPE_OVERLAY:
            ulStringID = ID_MMSI_DEVTYPE_OVERLAY;  // pszDevTypeOverlay
        break;

        case MCI_DEVTYPE_ANIMATION:
            ulStringID = ID_MMSI_DEVTYPE_ANIMATION;  // pszDevTypeAnimation
        break;

        case MCI_DEVTYPE_DIGITAL_VIDEO:
            ulStringID = ID_MMSI_DEVTYPE_DIGITAL_VIDEO;  // pszDevTypeDigitalVideo
        break;

        case MCI_DEVTYPE_SPEAKER:
            ulStringID = ID_MMSI_DEVTYPE_SPEAKER;  // pszDevTypeSpeaker
        break;

        case MCI_DEVTYPE_HEADPHONE:
            ulStringID = ID_MMSI_DEVTYPE_HEADPHONE;  // pszDevTypeHeadphone
        break;

        case MCI_DEVTYPE_MICROPHONE:
            ulStringID = ID_MMSI_DEVTYPE_MICROPHONE;  // pszDevTypeMicrophone
        break;

        case MCI_DEVTYPE_MONITOR:
            ulStringID = ID_MMSI_DEVTYPE_MONITOR;  // pszDevTypeMonitor
        break;

        case MCI_DEVTYPE_CDXA:
            ulStringID = ID_MMSI_DEVTYPE_CDXA;  // pszDevTypeCDXA
        break;

        case MCI_DEVTYPE_FILTER:
            ulStringID = ID_MMSI_DEVTYPE_FILTER;  // pszDevTypeFilter
        break;

        case MCI_DEVTYPE_TTS:
            ulStringID = ID_MMSI_DEVTYPE_TTS;  // pszDevTypeTTS
        break;
    }

    if (ulStringID)
        return cmnGetString(ulStringID);

    return "Unknown";
}

/*
 *@@ GetAllDeviceNames:
 *      writes a string array with all current MMPM/2
 *      device names into *ppszNames. The last entry
 *      is marked with two null bytes.
 *
 *      Each device string is of the "nameXX" form,
 *      with XX being the device index (e.g.
 *      "CDaudio01".
 *
 *      Caller must issue DosFreeMem on the string
 *      returned.
 *
 *      Returns an MMPM/2 error code.
 *
 *@@added V0.9.13 (2001-06-14) [umoeller]
 */

STATIC ULONG GetAllDeviceNames(PULONG pcDevices,       // out: device count
                               PSZ *ppszNames)         // out: device names
{
    ULONG               cDevices = 0,
                        ulrc = 0;

    MCI_SYSINFO_PARMS   sip;
    CHAR                szDevicesCount[10];

    ZERO(&sip);
    sip.pszReturn = szDevicesCount;
    sip.ulRetSize = sizeof(szDevicesCount);

    // step 1: get no. of logical devices;
    // this is done by issuing
    // MCI_SYSINFO :: MCI_SYSINFO_QUANTITY
    // against the special device ID 0xFFFF
    ulrc = G_mciSendCommand(MCI_ALL_DEVICE_ID,    // all devices (0xFFFF)
                            MCI_SYSINFO,
                            MCI_SYSINFO_QUANTITY, // get no. of logical devices
                            &sip,
                            0);
    if (    (!ulrc)
         // number of devices is returned as a STRING
         // (whoever came up with this sick interface...):
         // get device count from string
         && (cDevices = atoi(sip.pszReturn))
       )
    {
        // OK, allocate memory for the device names:
        ULONG cbDevices = cDevices * MAX_DEVICE_NAME;
        if (!(ulrc = DosAllocMem ((PVOID*)&sip.pszReturn,
                                  cbDevices + 1,
                                  PAG_READ | PAG_WRITE | PAG_COMMIT)))
        {
            sip.ulRetSize = (ULONG)cbDevices;
            // now get the device names:
            // this is done by issuing
            // MCI_SYSINFO :: MCI_SYSINFO_NAME
            // against the special device ID 0xFFFF
            ulrc = G_mciSendCommand(MCI_ALL_DEVICE_ID,
                                    MCI_SYSINFO,
                                    MCI_SYSINFO_NAME,   // get device names
                                    &sip,
                                    0);
            if (!ulrc)
            {
                // now we got all the device names, separated
                // by spaces... make this a string array
                PSZ pDevices = sip.pszReturn;
                PSZ pszName;
                pDevices[strlen(pDevices)+1] = 0;       // double null

                for (pszName = strchr(pDevices, ' ');
                     (pszName) && (*pszName);
                     pszName = strchr(pszName + 1, ' '))
                {
                    if (*pszName == ' ')
                        *pszName = 0;
                }

                *pcDevices = cDevices;
                *ppszNames = pDevices;
            }
        }
    }

    return ulrc;
}

/*
 *@@ GetDeviceInfo:
 *      retrieves detailed info for the specified
 *      device name and writes it into the specified
 *      XMMDEVICE instance.
 *
 *@@added V0.9.13 (2001-06-14) [umoeller]
 *@@changed V0.9.14 (2001-08-01) [umoeller]: fixed memory leak
 */

STATIC ULONG GetDeviceInfo(PXMMDEVICE pDevice,         // out: device info
                           const char *pcszName)       // in: device name (e.g. "CDaudio01")
{
    ULONG rc;

    // Alright, here comes the sick stuff with MCI_SYSINFO.

    // To get the details for a device, the following
    // steps are necessary:

    // --  Issue MCI_SYSINFO :: MCI_SYSINFO_ITEM
    //     against device ID 0 (!).
    // --  This expects an MCI_SYSINFO_PARMS structure.
    // --  In _that_ structure, ulItem specifies what
    //     to retrieve. The pSysInfoParm must point
    //     to another structure, depending on what
    //     was set with ulItem.

    // Totally sick interface, but appears to work.

    // sub-structures pointed to by sip below
    MCI_SYSINFO_QUERY_NAME  siNames;
    MCI_SYSINFO_LOGDEVICE   siLogDevice;
    MCI_SYSINFO_DEVPARAMS   siDevParams;

    // main structure
    MCI_SYSINFO_PARMS       sip;

    // 1) So to get the detailed device names, we
    //    use MCI_SYSINFO_QUERY_NAMES, which expects
    //    an MCI_SYSINFO_QUERY_NAME structure.
    //    (Good naming, too. Note the single "S" which
    //    differentiates the flag from the structure. Geese.)

    ZERO(&siNames);
    strlcpy(siNames.szLogicalName, pcszName, sizeof(siNames.szLogicalName));

    ZERO(&sip);
    sip.ulItem = MCI_SYSINFO_QUERY_NAMES;
    sip.pSysInfoParm = &siNames;

    if (!(rc = G_mciSendCommand(0,                  // device ID
                                MCI_SYSINFO,
                                MCI_SYSINFO_ITEM,   // get sub-info
                                &sip,
                                0)))
    {
        /*
         typedef struct _MCI_SYSINFO_QUERY_NAME {
           CHAR       szInstallName[MAX_DEVICE_NAME];  //  Device install name.
           CHAR       szLogicalName[MAX_DEVICE_NAME];  //  Logical device name.
           CHAR       szAliasName[MAX_ALIAS_NAME];     //  Alias name.
           USHORT     usDeviceType;                    //  Device type number.
           USHORT     usDeviceOrd;                     //  Device type ordinal.
         } MCI_SYSINFO_QUERY_NAME;
        */

        pDevice->ulDeviceType = siNames.usDeviceType;
        pDevice->pcszDeviceType = GetDeviceTypeName(pDevice->ulDeviceType);
        pDevice->ulDeviceIndex = siNames.usDeviceOrd;

        strhncpy0(pDevice->szInstallName,
                  siNames.szInstallName,
                  sizeof(pDevice->szInstallName));
        strhncpy0(pDevice->szLogicalName,
                  siNames.szLogicalName,
                  sizeof(pDevice->szLogicalName));
        strhncpy0(pDevice->szAliasName,
                  siNames.szAliasName,
                  sizeof(pDevice->szAliasName));

        // 2) To now get the detailed driver info, we
        //    use MCI_SYSINFO_QUERY_DRIVER, which expects
        //    an MCI_SYSINFO_LOGDEVICE structure.
        //    This time, input is the install name, which
        //    is one of the strings like "IBMCDAUDIO01"
        //    as in MMPM2.INI. This was returned above.
        ZERO(&siLogDevice);
        strlcpy(siLogDevice.szInstallName, siNames.szInstallName, sizeof(siLogDevice.szInstallName));

        ZERO(&sip);
        sip.ulItem = MCI_SYSINFO_QUERY_DRIVER;
        sip.pSysInfoParm = &siLogDevice;

        if (!(rc = G_mciSendCommand(0,         // device ID
                                    MCI_SYSINFO,
                                    MCI_SYSINFO_ITEM,       // get sub-info
                                    &sip,
                                    0)))
        {
            /*
             typedef struct _MCI_SYSINFO_LOGDEVICE {
               CHAR       szInstallName[MAX_DEVICE_NAME];                //  Device install name.
               USHORT     usDeviceType;                                  //  Device type name.
               ULONG      ulDeviceFlag;
                  // Flag indicating whether device is controllable or not:
                        -- MCI_SYSINFO_DEVICESETTINGS: Indicates the MCD has custom device
                           settings pages.
                        -- MCI_SYSINFO_DEV_CONTROLLABLE: If a device is controllable, it
                           usually accepts a PLAY command.
                        -- MCI_SYSINFO_DEV_NONCONTROLLABLE: Examples of non-controllable
                           devices are speakers, headphones,  microphone, and amp-mixer devices.

               CHAR       szVersionNumber[MAX_VERSION_NUMBER];           //  INI file version number.
               CHAR       szProductInfo[MAX_PRODINFO];                   //  Textual product description.
               CHAR       szMCDDriver[MAX_DEVICE_NAME];                  //  Driver DLL name.
               CHAR       szVSDDriver[MAX_DEVICE_NAME];                  //  VSD DLL name.
               CHAR       szPDDName[MAX_PDD_NAME];
                            // Device PDD name. The device driver name must not be more than eight
                            // characters (excluding the file extension).

               CHAR       szMCDTable[MAX_DEVICE_NAME];                   //  Device type command table.
               CHAR       szVSDTable[MAX_DEVICE_NAME];                   //  Device specific command table.
               USHORT     usShareType;                                   //  Device sharing mode.
               CHAR       szResourceName[MAX_DEVICE_NAME;                //  Resource name.
               USHORT     usResourceUnits;                               //  Resource units available.
               USHORT     usResourceClasses;                             //  Number of resource classes.
               USHORT     ausClassArray[MAX_CLASSES];                    //  Maximum resource units per class.
               USHORT     ausValidClassArray[MAX_CLASSES][MAX_CLASSES];  //  Valid class combinations.
             } MCI_SYSINFO_LOGDEVICE;
            */

            XSTRING strTemp;
            BOOL    fHasMCD = (strlen(siLogDevice.szMCDDriver) != 0),
                    fHasVSD = (strlen(siLogDevice.szVSDDriver) != 0),
                    fHasPDD = (strlen(siLogDevice.szPDDName) != 0);

            xstrInit(&strTemp, 100);

            if (fHasMCD)
            {
                xstrcpy(&strTemp, cmnGetString(ID_MMSI_MCD), 0);
                xstrcat(&strTemp, ": ", 2);
                xstrcat(&strTemp, siLogDevice.szMCDDriver, 0);
            }

            if (fHasVSD)
            {
                if (strTemp.ulLength)
                    xstrcatc(&strTemp, '\n');
                xstrcat(&strTemp, cmnGetString(ID_MMSI_VSD), 0);
                xstrcat(&strTemp, ": ", 2);
                xstrcat(&strTemp, siLogDevice.szVSDDriver, 0);
            }
            if (fHasPDD)
            {
                if (strTemp.ulLength)
                    xstrcatc(&strTemp, '\n');
                xstrcat(&strTemp, cmnGetString(ID_MMSI_PDD), 0);
                xstrcat(&strTemp, ": ", 2);
                xstrcat(&strTemp, siLogDevice.szPDDName, 0);
            }

            // 3) To now get the "device parameters", we
            //    use MCI_SYSINFO_QUERY_DRIVER, which expects
            //    an MCI_SYSINFO_DEVPARAMS structure.
            //    Again, input is the install name, which
            //    is one of the strings like "IBMCDAUDIO01"
            //    as in MMPM2.INI. This was returned above.

            ZERO(&siDevParams);
            strlcpy(siDevParams.szInstallName, siNames.szInstallName, sizeof(siDevParams.szInstallName));

            ZERO(&sip);
            sip.ulItem = MCI_SYSINFO_QUERY_PARAMS;
            sip.pSysInfoParm = &siDevParams;

            if (!(rc = G_mciSendCommand(0,         // device ID
                                        MCI_SYSINFO,
                                        MCI_SYSINFO_ITEM,       // get sub-info
                                        &sip,
                                        0)))
            {
                ULONG ulParamsLength = strlen(siDevParams.szDevParams);
                if (ulParamsLength)
                {
                    if (strTemp.ulLength)
                        xstrcatc(&strTemp, '\n');
                    xstrcat(&strTemp, cmnGetString(ID_MMSI_PARAMS), 0);
                    xstrcat(&strTemp, ": \"", 3);
                    xstrcat(&strTemp, siDevParams.szDevParams, ulParamsLength);
                    xstrcatc(&strTemp, '\"');
                }
            }

            if (strTemp.ulLength)
                pDevice->pszInfo = strTemp.psz;
                        // do not free strTemp here
            else
                xstrClear(&strTemp);        // V0.9.14 (2001-08-01) [umoeller]
        }
    }

    return rc;
}

/*
 *@@ xmmQueryDevices:
 *      returns an array of XMMDEVICE structures describing
 *      all available MMPM/2 devices on your system.
 *
 *      *pcDevices receives the no. of items in the array
 *      (not the array size!). Use xmmFreeDevices to clean up.
 *
 *@@added V0.9.3 (2000-04-29) [umoeller]
 *@@changed V0.9.7 (2000-11-30) [umoeller]: added NLS for device types
 *@@changed V0.9.13 (2001-06-14) [umoeller]: completely rewritten, now using MCI_SYSINFO to no longer open devices
 */

PXMMDEVICE xmmQueryDevices(PULONG pcDevices)
{
    ULONG   ulrc;
    ULONG   cDevices = 0;
    PSZ     pszDeviceStrings = NULL;
    PXMMDEVICE paDevices = 0;

    if (    (!(ulrc = GetAllDeviceNames(&cDevices,
                                        &pszDeviceStrings)))
         && (cDevices)
         && (pszDeviceStrings)
       )
    {
        // OK, now we got a string array in pszDeviceStrings
        ULONG   cb = (cDevices + 1) * sizeof(XMMDEVICE);
        // allocate XMMDEVICE array for output to caller;
        // note, we allocate one extra array item as
        // a null terminator (which is completely zeroed)
        if (paDevices = (PXMMDEVICE)malloc(cb))
        {
            ULONG   ul;
            PSZ     pszNameThis;
            PXMMDEVICE pDeviceThis = paDevices;
            memset(paDevices, 0, cb);

            // run thru the device names array produced above
            for (ul = 0, pszNameThis = pszDeviceStrings;
                 (ul < cDevices) && (pszNameThis) && (*pszNameThis);
                 ul++, pDeviceThis++, pszNameThis += strlen(pszNameThis) + 1)
            {
                // now get all the details for this device
                if (GetDeviceInfo(pDeviceThis,
                                  pszNameThis))
                    // error:
                    break;
            }
        }

        DosFreeMem(pszDeviceStrings);
    }

    *pcDevices = cDevices;

    return paDevices;
}

/*
 *@@ xmmFreeDevices:
 *      frees resources allocated by xmmQueryDevices.
 *
 *@@added V0.9.3 (2000-04-29) [umoeller]
 *@@changed V0.9.13 (2001-06-14) [umoeller]: rewritten
 */

BOOL xmmFreeDevices(PXMMDEVICE paDevices)
{
    PXMMDEVICE pThis;

    if (pThis = paDevices)
    {
        // run thru the array and free strings;
        // xmmQueryDevices has allocated one extra
        // array item where ulDeviceIndex is 0,
        // so we just loop until we find that
        while (pThis->ulDeviceIndex)
        {
            if (pThis->pszInfo)
            {
                free(pThis->pszInfo);
                pThis->pszInfo = NULL;
            }

            pThis++;
        }

        // now free the entire array
        free(paDevices);

        return TRUE;
    }

    return FALSE;
}


