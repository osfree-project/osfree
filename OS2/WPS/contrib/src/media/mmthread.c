
/*
 *@@sourcefile mmthread.c:
 *      this has the XWorkplace Party thread, which handles
 *      multimedia tasks (playing sounds etc.) in XWorkplace.
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

#define INCL_WINMESSAGEMGR

#define INCL_GPI                // required for INCL_MMIO_CODEC
#define INCL_GPIBITMAPS         // required for INCL_MMIO_CODEC
#include <os2.h>

// multimedia includes
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
#include "helpers\syssound.h"           // system sound helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines

// XWorkplace implementation headers
#include "shared\init.h"                // XWorkplace initialization
#include "media\media.h"                // XWorkplace multimedia support

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

static HAB         G_habPartyThread = NULLHANDLE;
static HMQ         G_hmqPartyThread = NULLHANDLE;
extern HWND G_hwndPartyObject = NULLHANDLE;

// sound data
static ULONG       G_ulMMPM2Working = MMSTAT_UNKNOWN;

static USHORT      G_usSoundDeviceID = 0;
static ULONG       G_ulVolumeTemp = 0;
static PSZ         G_pszSoundFile = NULL;

static THREADINFO  G_tiPartyThread = {0};

static PCSZ WNDCLASS_MEDIAOBJECT = "XWPPartyThread";

/* ******************************************************************
 *
 *   Party thread
 *
 ********************************************************************/

/*
 *@@ ThreadPlaySystemSound:
 *      implementation for XMM_PLAYSYSTEMSOUND to
 *      reduce stack allocation.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

STATIC VOID ThreadPlaySystemSound(HWND hwndObject,
                                  MPARAM mp1)
{
    CHAR    szDescr[CCHMAXPATH];
    ULONG   ulVolume;
    CHAR    szFile[CCHMAXPATH];

    PMPF_SOUNDS(("XMM_PLAYSYSTEMSOUND index %d", mp1));

    // get system sound from MMPM.INI
    if (sndQuerySystemSound(G_habPartyThread,
                            (USHORT)mp1,
                            szDescr,
                            szFile,
                            &ulVolume))
    {
        // OK, sound file found in MMPM.INI:
        PMPF_SOUNDS(("  posting Sound %d == %s, %s", mp1, szDescr, szFile ));

        // play!
        WinPostMsg(hwndObject,
                   XMM_PLAYSOUND,
                   (MPARAM)strdup(szFile),      // will be free'd in XMM_PLAYSOUND
                   (MPARAM)ulVolume);
    }
}

/*
 *@@ xmm_fnwpPartyObject:
 *      window procedure for the Party thread
 *      (xmm_fntPartyThread) object window.
 *
 *@@added V0.9.3 (2000-04-25) [umoeller]
 *@@changed V0.9.7 (2000-12-20) [umoeller]: removed XMM_CDPLAYER
 *@@changed V0.9.13 (2001-06-19) [umoeller]: fixed Win-OS/2 sound refusal
 *@@changed V0.9.14 (2001-08-01) [umoeller]: fixed memory leak
 */

MRESULT EXPENTRY xmm_fnwpPartyObject(HWND hwndObject, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = NULL;

    switch (msg)
    {
        /*
         *@@ XMM_PLAYSYSTEMSOUND:
         *      plays system sound specified in MMPM.INI.
         *      This is posted by xthrPostPartyMsg.
         *
         *      (USHORT)mp1 must be the MMPM.INI index (see
         *      sndQuerySystemSound for a list).
         */

        case XMM_PLAYSYSTEMSOUND:
            ThreadPlaySystemSound(hwndObject,
                                  mp1);
        break;

        /*
         *@@ XMM_PLAYSOUND:
         *      plays a sound file.
         *
         *      (PSZ)mp1 must specify the full sound file
         *      name. It is assumed to have been allocated
         *      using malloc() and will be freed afterwards.
         *
         *      (ULONG)mp2 must be the volume (0-100).
         *
         *      This message is only posted by
         *      XMM_PLAYSYSTEMSOUND (above) if a system
         *      sound was queried successfully.
         *
         *      Playing sounds is a three-step process:
         *
         *      1)  We call xmmOpenSound first to open a waveform
         *          device for this sound file as a _shareable_
         *          device and then stop for the moment.
         *
         *      2)  If this device is accessible, MMPM/2 then
         *          posts us MM_MCIPASSDEVICE (below) so we can
         *          play the sound.
         *
         *      3)  We close the device if we're either losing
         *          it (because another app needs it -- that's
         *          MM_MCIPASSDEVICE with MCI_LOSING_USE set)
         *          or if MMPM/2 is done with our sound (that's
         *          MM_MCINOTIFY below).
         */

        case XMM_PLAYSOUND:
        {
            if (mp1)
            {
                // check for whether that sound file really exists
                if (access(mp1, 0) == 0)
                {
                    G_ulVolumeTemp = (ULONG)mp2;

                    if (G_usSoundDeviceID)
                    {
                        // we have a device already:
                        xmmPlaySound(hwndObject,
                                     &G_usSoundDeviceID,
                                     (PSZ)mp1,
                                     G_ulVolumeTemp);
                        // free the PSZ passed to us
                        free((PSZ)mp1);
                    }
                    else
                    {
                        ULONG ulrc;
                        if (G_pszSoundFile)
                            free(G_pszSoundFile);
                        G_pszSoundFile = (PSZ)mp1;      // malloc'd
                        ulrc = xmmOpenWaveDevice(hwndObject,
                                                 &G_usSoundDeviceID);
                        // and play when MM_MCIPASSDEVICE comes in

                        PMPF_SOUNDS(("xmmOpenWaveDevice returned 0x%lX", ulrc));
                    }
                }
                else
                    free((PSZ)mp1);     // V0.9.14 (2001-08-01) [umoeller]
            }
        }
        break;

        /*
         * MM_MCIPASSDEVICE:
         *      MMPM/2 posts this msg for shareable devices
         *      to allow multimedia applications to behave
         *      politely when several applications use the
         *      same device. This is posted to us in two cases:
         *
         *      1)  opening the device above was successful
         *          and the device is available (that is, no
         *          other application needs exclusive access
         *          to that device); in this case, mp2 has the
         *          MCI_GAINING_USE flag set, and we can call
         *          xmmPlaySound to actually play the sound.
         *
         *          The device is _not_ available, for example,
         *          if a Win-OS/2 session is running which
         *          uses sounds.
         *
         *      2)  While we are playing, another application
         *          is trying to get access to the device; in
         *          this case, mp2 has the MCI_LOSING_USE flag
         *          set, and we call xmmStopSound to stop
         *          playing our sound.
         */

        #define MM_MCINOTIFY                        0x0500
        #define MM_MCIPASSDEVICE                    0x0501
        #define MCI_LOSING_USE                      0x00000001L
        #define MCI_GAINING_USE                     0x00000002L
        #define MCI_NOTIFY_SUCCESSFUL               0x0000

        case MM_MCIPASSDEVICE:
        {
            BOOL fGainingUse = (SHORT1FROMMP(mp2) == MCI_GAINING_USE);

            PMPF_SOUNDS(("MM_MCIPASSDEVICE: mp1 = 0x%lX, mp2 = 0x%lX", mp1, mp2 ));
            PMPF_SOUNDS(("    --> %s use", (fGainingUse) ? "Gaining" : "Losing" ));

            if (fGainingUse)
            {
                // we're gaining the device (1): play sound
                if (G_pszSoundFile)
                {
                    xmmPlaySound(hwndObject,
                                 &G_usSoundDeviceID,
                                 G_pszSoundFile,
                                 G_ulVolumeTemp);
                    free(G_pszSoundFile);
                    G_pszSoundFile = NULL;
                }
            }
            else
                // we're losing the device (2): stop sound
                xmmStopSound(&G_usSoundDeviceID);
        }
        break;

        /*
         * MM_MCINOTIFY:
         *      this is the general notification msg of MMPM/2.
         *      We need this message to know when MMPM/2 is done
         *      playing our sound; we will then close the device.
         */

        case MM_MCINOTIFY:
        {
            USHORT  usNotifyCode = SHORT1FROMMP(mp1),
                    usDeviceID = SHORT1FROMMP(mp2);
                    // usMessage = SHORT2FROMMP(mp2);

            if (    (G_usSoundDeviceID)
                 && (usDeviceID == G_usSoundDeviceID)
               )
            {
                PMPF_SOUNDS(("MM_MCINOTIFY: usNotifyCode = 0x%lX", usNotifyCode));

                if (usNotifyCode == MCI_NOTIFY_SUCCESSFUL)
                {
                    xmmStopSound(&G_usSoundDeviceID);
                    // now close the device again, or otherwise Win-OS/2
                    // will refuse to work V0.9.13 (2001-06-19) [umoeller]
                    xmmCloseDevice(&G_usSoundDeviceID);
                        // this sets G_usSoundDeviceID to NULLHANDLE again
                }
            }
        }
        break;

        default:
            mrc = WinDefWindowProc(hwndObject, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ xmm_fntPartyThread:
 *      thread func for the Party thread, which creates
 *      an object window (xmm_fnwpPartyObject). This
 *      is responsible for playing sounds and such.
 *
 *@@added V0.9.3 (2000-04-25) [umoeller]
 *@@changed V0.9.18 (2002-02-23) [umoeller]: renamed from "Media" thread
 */

void _Optlink xmm_fntPartyThread(PTHREADINFO pti)
{
    QMSG                  qmsg;
    // PSZ                   pszErrMsg = NULL;
    BOOL                  fTrapped = FALSE;

    TRY_LOUD(excpt1)
    {
        if (G_habPartyThread = WinInitialize(0))
        {
            if (G_hmqPartyThread = WinCreateMsgQueue(G_habPartyThread, 3000))
            {
                WinCancelShutdown(G_hmqPartyThread, TRUE);

                WinRegisterClass(G_habPartyThread,
                                 (PSZ)WNDCLASS_MEDIAOBJECT,    // class name
                                 (PFNWP)xmm_fnwpPartyObject,    // Window procedure
                                 0,                  // class style
                                 0);                 // extra window words

                // set ourselves to higher regular priority
                DosSetPriority(PRTYS_THREAD,
                               PRTYC_REGULAR,
                               +31, // priority delta
                               0);

                // create object window
                G_hwndPartyObject
                    = winhCreateObjectWindow(WNDCLASS_MEDIAOBJECT, NULL);

                if (!G_hwndPartyObject)
                    winhDebugBox(HWND_DESKTOP,
                             "XFolder: Error",
                             "XFolder failed to create the Party thread object window.");

                // now enter the message loop
                while (WinGetMsg(G_habPartyThread, &qmsg, NULLHANDLE, 0, 0))
                    WinDispatchMsg(G_habPartyThread, &qmsg);
                                // loop until WM_QUIT
            }
        }
    }
    CATCH(excpt1)
    {
        // disable sounds
        fTrapped = TRUE;
    } END_CATCH();

    winhDestroyWindow(&G_hwndPartyObject);

    WinDestroyMsgQueue(G_hmqPartyThread);
    G_hmqPartyThread = NULLHANDLE;
    WinTerminate(G_habPartyThread);
    G_habPartyThread = NULLHANDLE;

    if (fTrapped)
        G_ulMMPM2Working = MMSTAT_CRASHED;
    else
        G_ulMMPM2Working = MMSTAT_UNKNOWN;
}

/* ******************************************************************
 *
 *   Party thread interface
 *
 ********************************************************************/

/*
 *@@ xmmInit:
 *      initializes the XWorkplace Party environment
 *      and resolves the MMPM/2 APIs.
 *      Gets called by initMain on
 *      Desktop startup.
 *
 *@@added V0.9.3 (2000-04-25) [umoeller]
 *@@changed V0.9.16 (2001-10-19) [umoeller]: changed to XFILE log file
 */

BOOL xmmInit(VOID)
{
    initLog("Entering " __FUNCTION__ ":");

    G_ulMMPM2Working = MMSTAT_WORKING;

    if (mmhInit())
        G_ulMMPM2Working = MMSTAT_IMPORTSFAILED;

    initLog("  Resolved MMPM/2 imports, new XWP media status: %d",
                      G_ulMMPM2Working);

    if (G_ulMMPM2Working == MMSTAT_WORKING)
    {
        thrCreate(&G_tiPartyThread,
                  xmm_fntPartyThread,
                  NULL, // running flag
                  "Party",
                  0,    // no msgq
                  0);
        initLog("  Started XWP Party thread, TID: %d",
                          G_tiPartyThread.tid);
    }

    return (G_ulMMPM2Working == MMSTAT_WORKING);
}

/*
 *@@ xmmDisable:
 *      disables multimedia completely.
 *      Called from initMain if
 *      the flag in the panic dialog has been set.
 *
 *@@added V0.9.3 (2000-04-30) [umoeller]
 */

VOID xmmDisable(VOID)
{
    G_ulMMPM2Working = MMSTAT_DISABLED;
}

/*
 *@@ xmmQueryStatus:
 *      returns the status of the XWorkplace media
 *      engine, which is one of the following:
 *
 +      --  MMSTAT_UNKNOWN: initial value after startup.
 +      --  MMSTAT_WORKING: media is working.
 +      --  MMSTAT_MMDIRNOTFOUND: MMPM/2 directory not found.
 +      --  MMSTAT_DLLNOTFOUND: MMPM/2 DLLs not found.
 +      --  MMSTAT_IMPORTSFAILED: MMPM/2 imports failed.
 +      --  MMSTAT_CRASHED: Party thread crashed, sounds disabled.
 +      --  MMSTAT_DISABLED: media explicitly disabled in startup panic dlg.
 *
 *      You should check this value when using XWorkplace media
 *      and use the media functions only when MMSTAT_WORKING is
 *      returned.
 *
 *@@added V0.9.3 (2000-04-25) [umoeller]
 */

ULONG xmmQueryStatus(VOID)
{
    return G_ulMMPM2Working;
}

/*
 *@@ xmmPostPartyMsg:
 *      posts a message to xmm_fnwpPartyObject with
 *      error checking.
 *
 *@@added V0.9.3 (2000-04-25) [umoeller]
 */

BOOL xmmPostPartyMsg(ULONG msg, MPARAM mp1, MPARAM mp2)
{
    BOOL rc = FALSE;
    if (thrQueryID(&G_tiPartyThread))
    {
        if (G_hwndPartyObject)
            if (G_ulMMPM2Working == MMSTAT_WORKING)
                rc = WinPostMsg(G_hwndPartyObject, msg, mp1, mp2);
    }
    return rc;
}

/*
 *@@ xmmIsPlayingSystemSound:
 *      returns TRUE if the Party thread is
 *      currently playing a system sound.
 *      This is useful for waiting until it's done.
 */

BOOL xmmIsBusy(VOID)
{
    return (G_usSoundDeviceID != 0);
}


