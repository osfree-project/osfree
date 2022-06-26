
/*
 *@@sourcefile drivemonitor.c:
 *
 *@@added V0.9.0 [umoeller]
 *@@header "hook\xwpdaemn.h"
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

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINSTDCNR
#include <os2.h>

#include <stdio.h>
#include <time.h>
#include <setjmp.h>

#define DONT_REPLACE_FOR_DBCS
#define DONT_REPLACE_MALLOC         // in case mem debug is enabled
#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\standards.h"
#include "helpers\threads.h"

#include "xwpapi.h"                     // public XWorkplace definitions

#include "filesys\disk.h"               // XFldDisk implementation

#include "hook\xwphook.h"               // hook and daemon definitions
#include "hook\xwpdaemn.h"              // XPager and daemon declarations

#include "bldlevel.h"

/* ******************************************************************
 *
 *   Private declarations
 *
 ********************************************************************/

/* #define IOCTL_CDROMDISK             0x80
#define CDROMDISK_DEVICESTATUS      0x60
#define CDROMDISK_GETUPC            0x79 */

/*
 *@@ DISKWATCHITEM:
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

typedef struct _DISKWATCHITEM
{
    ULONG       ulLogicalDrive;
    HWND        hwndNotify;
    ULONG       ulMessage;

    double      dFreeLast;
    APIRET      arcLast;

} DISKWATCHITEM, *PDISKWATCHITEM;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

static LINKLIST    G_llDrives;         // linked list of DISKWATCHITEM structs,
                                // auto-free

static HMTX        G_hmtxDrivesList = NULLHANDLE;
                                // mutex protecting that list

/* ******************************************************************
 *
 *   Drive monitoring
 *
 ********************************************************************/

/* int CheckRemoveableDrive(void)
{
    APIRET  arc;
    HFILE   hFile;
    ULONG   ulTemp;

    arc = DosOpen("X:",   // <--- CD drive
                  &hFile,
                  &ulTemp,
                  0,
                  FILE_NORMAL,
                  OPEN_ACTION_FAIL_IF_NEW
                         | OPEN_ACTION_OPEN_IF_EXISTS,
                  OPEN_FLAGS_DASD
                         | OPEN_FLAGS_FAIL_ON_ERROR
                         | OPEN_ACCESS_READONLY
                         | OPEN_SHARE_DENYNONE,
                  NULL);
    if (arc == NO_ERROR)
    {
        UCHAR   achUnit[4] = { 'C', 'D', '0', '1' };
        struct
        {
            UCHAR   uchCtlAdr;
            UCHAR   achUPC[7];
            UCHAR   uchReserved;
            UCHAR   uchFrame;
        } Result;

        ULONG ulDeviceStatus = 0;

        ulTemp = 0;

        arc = DosDevIOCtl(hFile,
                          IOCTL_CDROMDISK,
                          CDROMDISK_GETUPC,
                          achUnit, sizeof(achUnit), NULL,
                          &Result, sizeof(Result), &ulTemp );
        if (arc == NO_ERROR)
        {
            CHAR szUPC[100];
            sprintf(szUPC,
                    "UPC: %02x%02x%02x%02x%02x%02x%02x",
                    Result.achUPC[0], Result.achUPC[1], Result.achUPC[2],
                    Result.achUPC[3], Result.achUPC[4], Result.achUPC[5], Result.achUPC[6]);
            _Pmpf(("New ADR: 0x%lX", Result.uchCtlAdr));
            _Pmpf(("New UPC: %s", szUPC));
        }
        else
            _Pmpf(("DosDevIOCtl rc: %d", arc));

        arc = DosDevIOCtl(hFile,
                          IOCTL_CDROMDISK,
                          CDROMDISK_DEVICESTATUS,
                          achUnit, sizeof(achUnit), NULL,
                          &ulDeviceStatus, sizeof(ulDeviceStatus), &ulTemp);
        if (arc == NO_ERROR)
        {
            _Pmpf(("ulStatus: 0x%lX", ulDeviceStatus));
            _Pmpf(("  CDDA support:  %s", (ulDeviceStatus & (1<<30)) ? "yes" : "no"));
            _Pmpf(("  Playing audio: %s", (ulDeviceStatus & (1<<12)) ? "yes" : "no"));
            _Pmpf(("  Disk present:  %s", (ulDeviceStatus & (1<<11)) ? "yes" : "no"));
            _Pmpf(("  Door open:     %s", (ulDeviceStatus & (1    )) ? "yes" : "no"));
        }
        else
            _Pmpf(("DosDevIOCtl rc: %d", arc));

        DosClose(hFile);
    }
    else
        _Pmpf(("Open failed !\n"));

    return 0;
} */

/*
 *@@ LockDrivesList:
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

BOOL LockDrivesList(VOID)
{
    if (G_hmtxDrivesList)
        return !DosRequestMutexSem(G_hmtxDrivesList, SEM_INDEFINITE_WAIT);

    // first call:
    lstInit(&G_llDrives, TRUE);         // auto-free
    return !DosCreateMutexSem(NULL,
                              &G_hmtxDrivesList,
                              0,
                              TRUE);      // request!
}

/*
 *@@ UnlockDrivesList:
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

VOID UnlockDrivesList(VOID)
{
    DosReleaseMutexSem(G_hmtxDrivesList);
}

/*
 *@@ dmnAddDiskfreeMonitor:
 *      adds or removes a disk watch for a logical drive.
 *
 *      If all parameters are specified (!= -1), a disk
 *      watch is added with the given window to be notified.
 *
 *      If (ulMessage == -1), a disk watch is removed.
 *      Only in that case, if (ulLogicalDrive == -1) also,
 *      all disk watches for the given window are removed.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

BOOL dmnAddDiskfreeMonitor(ULONG ulLogicalDrive,    // in: disk to be watched or -1
                           HWND hwndNotify,         // in: window to be notified
                           ULONG ulMessage)         // in: message to be posted on change;
                                                    // if -1, notification is removed
{
    BOOL brc = FALSE,
         fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = LockDrivesList())
        {
            if (ulMessage != -1)
            {
                // add:
                PDISKWATCHITEM pNew;
                if (pNew = NEW(DISKWATCHITEM))
                {
                    ZERO(pNew);

                    pNew->ulLogicalDrive = ulLogicalDrive;
                    pNew->hwndNotify = hwndNotify;
                    pNew->ulMessage = ulMessage;

                    pNew->dFreeLast = 0;

                    // _Pmpf(("Added diskwatch for drive %d, hwnd 0x%lX",
                       //      ulLogicalDrive,
                          //   hwndNotify));

                    brc = (lstAppendItem(&G_llDrives,
                                         pNew) != 0);
                }
            }
            else
            {
                // remove:
                PLISTNODE pNode = lstQueryFirstNode(&G_llDrives);
                while (pNode)
                {
                    PLISTNODE pNext = pNode->pNext;
                    PDISKWATCHITEM pThis = (PDISKWATCHITEM)pNode->pItemData;

                    if (    (pThis->hwndNotify == hwndNotify)
                         && (    (ulLogicalDrive == -1)
                              || (pThis->ulLogicalDrive == ulLogicalDrive)
                            )
                       )
                    {
                        lstRemoveNode(&G_llDrives,
                                      pNode);

                        // _Pmpf(("Removed diskwatch for drive %d, hwnd 0x%lX",
                           //      ulLogicalDrive,
                              //   hwndNotify));

                        if (ulLogicalDrive != -1)
                            // not remove all drives: stop here
                            break;
                    }
                    pNode = pNext;
                }
            }
        }
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    if (fLocked)
        UnlockDrivesList();

    return brc;
}

/*
 *@@ dmnQueryDisks:
 *      implementation for XDM_QUERYDISKS in
 *      fnwpDaemonObject.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

BOOL dmnQueryDisks(ULONG ulLogicalDrive,
                   MPARAM mpDiskInfos)
{
    BOOL brc = TRUE;
    TRY_LOUD(excpt1)
    {
        PXDISKINFO  paDiskInfos = (PXDISKINFO)mpDiskInfos;

        // _PmpfF(("paDiskInfos is 0x%lX", paDiskInfos));

        if (ulLogicalDrive == -1)
        {
            ULONG ul;

            // all disks:
            // _PmpfF(("ulLogicalDrive == -1, getting all disks"));
            for (ul = 0; ul < 26; ul++)
            {
                doshGetDriveInfo(ul + 1,
                                 0,
                                 &paDiskInfos[ul]);
            }
        }
        else
            // just one:
            doshGetDriveInfo(ulLogicalDrive,
                             0,
                             paDiskInfos);
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    return brc;
}

/*
 *@@ fntDiskWatch:
 *      drive monitor thread.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

void _Optlink fntDiskWatch(PTHREADINFO ptiMyself)
{
    BOOL fLocked = FALSE;

    if (LockDrivesList())
    {
        // tell main() we're done
        DosPostEventSem(ptiMyself->hevRunning);

        UnlockDrivesList();
    }

    TRY_LOUD(excpt1)
    {
        // run forever unless exit
        while (!ptiMyself->fExit)
        {
            // go sleep a while
            DosSleep(500);

            if (fLocked = LockDrivesList())
            {
                PLISTNODE pNode = lstQueryFirstNode(&G_llDrives);
                while (pNode)
                {
                    PLISTNODE       pNext = pNode->pNext;

                    PDISKWATCHITEM  pWatch = (PDISKWATCHITEM)pNode->pItemData;

                    // _Pmpf(("processing watch for drive %d", pWatch->ulLogicalDrive));

                    if (!WinIsWindow(ptiMyself->hab,
                                     pWatch->hwndNotify))
                    {
                        // notify window no longer exists:
                        dmnAddDiskfreeMonitor(pWatch->ulLogicalDrive,
                                              pWatch->hwndNotify,
                                              -1);      // remove!
                    }
                    else
                    {
                        APIRET          arc;
                        double          dFree;
                        LONG            lKB = -1;
                        if (!(arc = doshQueryDiskFree(pWatch->ulLogicalDrive,
                                                      &dFree)))
                        {
                            if (dFree != pWatch->dFreeLast)
                            {
                                // free space changed:
                                // _PmpfF(("drive %d changed, old: %f, new: %f",
                                //         pWatch->ulLogicalDrive,
                                   //      pWatch->dFreeLast,
                                      //   dFree));
                                lKB = dFree / 1024L;
                                pWatch->dFreeLast = dFree;
                                pWatch->arcLast = NO_ERROR;
                            }
                        }
                        else
                        {
                            if (arc != pWatch->arcLast)
                            {
                                // error code changed:
                                lKB = -(LONG)arc;
                                pWatch->dFreeLast = 0;
                                pWatch->arcLast = arc;
                            }
                        }

                        if (lKB != -1)
                            // we got something to report:
                            // well, report then
                            WinPostMsg(pWatch->hwndNotify,
                                       // post the msg that client wants from us
                                       pWatch->ulMessage,
                                       (MPARAM)pWatch->ulLogicalDrive,
                                       (MPARAM)lKB);
                    }

                    pNode = pNext;
                }

                UnlockDrivesList();
                fLocked = FALSE;
            }
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    if (fLocked)
        UnlockDrivesList();
}
