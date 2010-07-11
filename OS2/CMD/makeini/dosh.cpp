
/*
 *@@sourcefile dosh.c:
 *      dosh.c contains Control Program helper functions.
 *
 *      This file has miscellaneous system functions,
 *      drive helpers, file helpers, and partition functions.
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  dosh*   Dos (Control Program) helper functions
 *
 *      These funcs are forward-declared in dosh.h, which
 *      must be #include'd first.
 *
 *      The resulting dosh.obj object file can be linked
 *      against any application object file. As opposed to
 *      the code in dosh2.c, it does not require any other
 *      code from the helpers.
 *
 *      dosh.obj can also be used with the VAC subsystem
 *      library (/rn compiler option).
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\dosh.h"
 */

/*
 *      This file Copyright (C) 1997-2007 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSESMGR
#define INCL_DOSQUEUES
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS

#define INCL_KBD
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#include "setup.h"                      // code generation and debugging options

#include "dosh.h"
#include "standards.h"

#pragma hdrstop

// static const CHAR  G_acDriveLetters[28] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/*
 *@@category: Helpers\Control program helpers\Wrappers
 */

/* ******************************************************************
 *
 *   Wrappers
 *
 ********************************************************************/

#ifdef DOSH_STANDARDWRAPPERS

    /*
     *@@ doshSleep:
     *
     *@@added V0.9.16 (2002-01-26) [umoeller]
     */

    APIRET doshSleep(ULONG msec)
    {
        // put the call in brackets so the macro won't apply here
        return (DosSleep)(msec);
    }

    /*
     *@@ doshCreateMutexSem:
     *
     *@@added V0.9.16 (2002-01-26) [umoeller]
     */

    APIRET doshCreateMutexSem(PSZ pszName,
                              PHMTX phmtx,
                              ULONG flAttr,
                              BOOL32 fState)
    {
        // put the call in brackets so the macro won't apply here
        return (DosCreateMutexSem)(pszName, phmtx, flAttr, fState);
    }

    /*
     *@@ doshRequestMutexSem:
     *
     *@@added V0.9.16 (2002-01-26) [umoeller]
     */

    APIRET doshRequestMutexSem(HMTX hmtx, ULONG ulTimeout)
    {
        return (DosRequestMutexSem)(hmtx, ulTimeout);
    }

    /*
     *@@ doshReleaseMutexSem:
     *
     *@@added V0.9.16 (2002-01-26) [umoeller]
     */

    APIRET doshReleaseMutexSem(HMTX hmtx)
    {
        return (DosReleaseMutexSem)(hmtx);
    }

    /*
     *@@ doshSetExceptionHandler:
     *
     *@@added V0.9.16 (2002-01-26) [umoeller]
     */

    APIRET doshSetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
    {
        // put the call in brackets so the macro won't apply here
        return (DosSetExceptionHandler)(pERegRec);
    }

    /*
     *@@ doshUnsetExceptionHandler:
     *
     *@@added V0.9.16 (2002-01-26) [umoeller]
     */

    APIRET doshUnsetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
    {
        // put the call in brackets so the macro won't apply here
        return (DosUnsetExceptionHandler)(pERegRec);
    }

#endif

/*
 *@@category: Helpers\Control program helpers\Miscellaneous
 *      Miscellaneous helpers in dosh.c that didn't fit into any other
 *      category.
 */

/* ******************************************************************
 *
 *   Miscellaneous
 *
 ********************************************************************/

/*
 *@@ doshGetChar:
 *      reads a single character from the keyboard.
 *      Useful for VIO sessions, since there's great
 *      confusion between the various C dialects about
 *      how to use getc(), and getc() doesn't work
 *      with the VAC subsystem library.
 *
 *@@added V0.9.4 (2000-07-27) [umoeller]
 */

CHAR doshGetChar(VOID)
{
    // CHAR    c;
    // ULONG   ulRead = 0;

    KBDKEYINFO kki;
    KbdCharIn(&kki,
              0, // wait
              0);

    return kki.chChar;
}

/*
 *@@ doshQueryShiftState:
 *      returns TRUE if any of the SHIFT keys are
 *      currently pressed. Useful for checks during
 *      WM_COMMAND messages from menus.
 *
 *@@changed V0.9.5 (2000-09-27) [umoeller]: added error checking
 */

BOOL doshQueryShiftState(VOID)
{
    BOOL            brc = FALSE;
    APIRET          arc = NO_ERROR;
    HFILE           hfKbd;
    ULONG           ulAction;

    if (!(arc = DosOpen("KBD$", &hfKbd, &ulAction, 0,
                        FILE_NORMAL,
                        FILE_OPEN,
                        OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                        (PEAOP2)NULL)))
    {
        SHIFTSTATE      ShiftState;
        ULONG           cbDataLen = sizeof(ShiftState);

        if (!(arc = DosDevIOCtl(hfKbd, IOCTL_KEYBOARD, KBD_GETSHIFTSTATE,
                                NULL, 0, NULL,      // no parameters
                                &ShiftState, cbDataLen, &cbDataLen)))
            brc = ((ShiftState.fsState & 3) != 0);

        DosClose(hfKbd);
    }

    return brc;
}

/*
 *@@ doshQuerySysErrorMsg:
 *      this retrieves the error message for a system error
 *      (APIRET) from the system error message file (OSO001.MSG).
 *      This file better be on the DPATH (it normally is).
 *
 *      This returns the string in the "SYSxxx: blahblah" style,
 *      which is normally displayed on the command line when
 *      errors occur.
 *
 *      The error message is returned in a newly allocated
 *      buffer, which should be free()'d afterwards.
 *
 *      The returned string is properly null-terminated but
 *      should not end with a line break (\r or \n) if I see
 *      this correctly.
 *
 *      Returns NULL upon errors.
 *
 *@@changed V1.0.0 (2002-09-24) [umoeller]: now supporting NET messages as well
 */

PSZ doshQuerySysErrorMsg(APIRET arc)    // in: DOS error code
{
    PSZ     pszReturn = NULL;
    CHAR    szDosError[1000];
    ULONG   cbDosError = 0;
    PCSZ    pcszMsgFile;

    if (    (arc >= 2100)
         && (arc <= 8000)
       )
        pcszMsgFile = "NET.MSG";
    else
        pcszMsgFile = "OSO001.MSG";        // default OS/2 message file

    if (!DosGetMessage(NULL, 0,       // no string replacements
                       szDosError, sizeof(szDosError),
                       arc,
                       (PSZ)pcszMsgFile,
                       &cbDosError))
    {
        szDosError[cbDosError - 2] = 0;
        pszReturn = strdup(szDosError);
    }

    return pszReturn;
}

/*
 *@@ doshQuerySysUptime:
 *      returns the system uptime in milliseconds.
 *      This can be used for time comparisons.
 *
 *@@added V0.9.12 (2001-05-18) [umoeller]
 */

ULONG doshQuerySysUptime(VOID)
{
    ULONG ulms;
    DosQuerySysInfo(QSV_MS_COUNT,
                    QSV_MS_COUNT,
                    &ulms,
                    sizeof(ulms));
    return ulms;
}

/*
 *@@ doshDevIOCtl:
 *
 *      Works with those IOCtls where the buffer
 *      size parameters are always the same anyway,
 *      which applies to all IOCtls I have seen
 *      so far.
 *
 *@@added V0.9.13 (2001-06-14) [umoeller]
 */

APIRET doshDevIOCtl(HFILE hf,
                    ULONG ulCategory,
                    ULONG ulFunction,
                    PVOID pvParams,
                    ULONG cbParams,
                    PVOID pvData,
                    ULONG cbData)
{
    return DosDevIOCtl(hf,
                       ulCategory,
                       ulFunction,
                       pvParams, cbParams, &cbParams,
                       pvData, cbData, &cbData);
}

/*
 *@@category: Helpers\Control program helpers\Shared memory management
 *      helpers for allocating and requesting shared memory.
 */

/* ******************************************************************
 *
 *   Memory helpers
 *
 ********************************************************************/

/*
 *@@ doshMalloc:
 *      wrapper around malloc() which automatically
 *      sets ERROR_NOT_ENOUGH_MEMORY.
 *
 *@@added V0.9.16 (2001-10-19) [umoeller]
 */

PVOID doshMalloc(ULONG cb,
                 APIRET *parc)
{
    PVOID pv;
    *parc = NO_ERROR;
    if (!(pv = malloc(cb)))
        *parc = ERROR_NOT_ENOUGH_MEMORY;

    return pv;
}

/*
 *@@ doshAllocArray:
 *      allocates c * cbArrayItem bytes.
 *      Similar to calloc(), but returns
 *      error codes:
 *
 *      --  NO_ERROR: *ppv and *pcbAllocated were set.
 *
 *      --  ERROR_NO_DATA: either c or cbArrayItem are
 *          zero.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY: malloc() failed.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

APIRET doshAllocArray(ULONG c,              // in: array item count
                      ULONG cbArrayItem,    // in: size of one array item
                      PBYTE *ppv,           // out: memory ptr if NO_ERROR is returned
                      PULONG pcbAllocated)  // out: # of bytes allocated
{
    if (!c || !cbArrayItem)
        return ERROR_NO_DATA;

    *pcbAllocated = c * cbArrayItem;
    if (!(*ppv = (PBYTE)malloc(*pcbAllocated)))
        return ERROR_NOT_ENOUGH_MEMORY;

    return NO_ERROR;
}

/*
 *@@ doshAllocSharedMem:
 *      wrapper for DosAllocSharedMem which has
 *      a malloc()-like syntax. Just due to my
 *      lazyness.
 *
 *      Note that ulSize is always rounded up to the
 *      next 4KB value, so don't use this hundreds of times.
 *
 *      Returns NULL upon errors. Possible errors include
 *      that a memory block calle pcszName has already been
 *      allocated.
 *
 *      Use DosFreeMem(pvrc) to free the memory. The memory
 *      will only be freed if no other process has requested
 *      access.
 *
 *@@added V0.9.3 (2000-04-18) [umoeller]
 */

PVOID doshAllocSharedMem(ULONG ulSize,      // in: requested mem block size (rounded up to 4KB)
                         const char* pcszName) // in: name of block ("\\SHAREMEM\\xxx") or NULL
{
    PVOID   pvrc = NULL;

    if (!DosAllocSharedMem((PVOID*)&pvrc,
                           (PSZ)pcszName,
                           ulSize,
                           PAG_COMMIT | PAG_READ | PAG_WRITE))
        return pvrc;

    return NULL;
}

/*
 *@@ doshRequestSharedMem:
 *      requests access to a block of named shared memory
 *      allocated by doshAllocSharedMem.
 *
 *      Returns NULL upon errors.
 *
 *      Use DosFreeMem(pvrc) to free the memory. The memory
 *      will only be freed if no other process has requested
 *      access.
 *
 *@@added V0.9.3 (2000-04-19) [umoeller]
 */

PVOID doshRequestSharedMem(PCSZ pcszName)
{
    PVOID pvrc = NULL;

    if (!DosGetNamedSharedMem((PVOID*)pvrc,
                              (PSZ)pcszName,
                              PAG_READ | PAG_WRITE))
        return pvrc;

    return NULL;
}

/*
 *@@category: Helpers\Control program helpers\Drive management
 *      functions for managing drives... enumerating, testing,
 *      querying etc.
 */

/* ******************************************************************
 *
 *   Drive helpers
 *
 ********************************************************************/

/*
 *@@ doshIsFixedDisk:
 *      checks whether a disk is fixed or removeable.
 *      ulLogicalDrive must be 1 for drive A:, 2 for B:, ...
 *      The result is stored in *pfFixed.
 *      Returns DOS error code.
 *
 *      From my testing, this function does _not_ provoke
 *      "drive not ready" popups, even if the disk is not
 *      ready.
 *
 *      Warning: This uses DosDevIOCtl, which has proved
 *      to cause problems with some device drivers for
 *      removeable disks.
 *
 *      Returns:
 *
 *      --  NO_ERROR: *pfFixed was set.
 *
 *      --  ERROR_INVALID_DRIVE: drive letter invalid
 *
 *      --  ERROR_NOT_SUPPORTED (50): for network drives.
 *
 *@@changed V0.9.14 (2001-08-03) [umoeller]: added extra fix for A: and B:
 */

APIRET doshIsFixedDisk(ULONG ulLogicalDrive,   // in: 1 for A:, 2 for B:, 3 for C:, ...
                       PBOOL pfFixed)          // out: TRUE for fixed disks
{
    APIRET arc = ERROR_INVALID_DRIVE;

    if (    (ulLogicalDrive == 1)
         || (ulLogicalDrive == 2)
       )
    {
        // drive A: and B: can never be fixed V0.9.14 (2001-08-03) [umoeller]
        *pfFixed = FALSE;
        return NO_ERROR;
    }

    if (ulLogicalDrive)
    {
        // parameter packet
        #pragma pack(1)
        struct {
            UCHAR   command,
                    drive;
        } parms;
        #pragma pack()

        // data packet
        UCHAR ucNonRemoveable;

        parms.drive = (UCHAR)(ulLogicalDrive - 1);
        if (!(arc = doshDevIOCtl((HFILE)-1,
                                 IOCTL_DISK,                  // 0x08
                                 DSK_BLOCKREMOVABLE,          // 0x20
                                 &parms, sizeof(parms),
                                 &ucNonRemoveable, sizeof(ucNonRemoveable))))
            *pfFixed = (BOOL)ucNonRemoveable;
    }

    return arc;
}

/*
 *@@ doshQueryDiskParams:
 *      this retrieves more information about a given drive,
 *      which is stored in the specified BIOSPARAMETERBLOCK
 *      structure.
 *
 *      BIOSPARAMETERBLOCK is defined in the Toolkit headers,
 *      and from my testing, it's the same with the Toolkits
 *      3 and 4.5.
 *
 *      If NO_ERROR is returned, the bDeviceType field can
 *      be one of the following (according to CPREF):
 *
 *      --  0:  48 TPI low-density diskette drive
 *      --  1:  96 TPI high-density diskette drive
 *      --  2:  3.5-inch 720KB diskette drive
 *      --  3:  8-Inch single-density diskette drive
 *      --  4:  8-Inch double-density diskette drive
 *      --  5:  Fixed disk
 *      --  6:  Tape drive
 *      --  7:  Other (includes 1.44MB 3.5-inch diskette drive)
 *      --  8:  R/W optical disk
 *      --  9:  3.5-inch 4.0MB diskette drive (2.88MB formatted)
 *
 *      From my testing, this function does _not_ provoke
 *      "drive not ready" popups, even if the disk is not
 *      ready.
 *
 *      Warning: This uses DosDevIOCtl, which has proved
 *      to cause problems with some device drivers for
 *      removeable disks.
 *
 *      This returns the DOS error code of DosDevIOCtl.
 *      This will be:
 *
 *      --  NO_ERROR for all local disks;
 *
 *      --  ERROR_NOT_SUPPORTED (50) for network drives.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.13 (2001-06-14) [umoeller]: changed prototype to use BIOSPARAMETERBLOCK directly
 *@@changed V0.9.13 (2001-06-14) [umoeller]: now querying standard media, no redetermine
 */

APIRET doshQueryDiskParams(ULONG ulLogicalDrive,        // in:  1 for A:, 2 for B:, 3 for C:, ...
                           PBIOSPARAMETERBLOCK pdp)     // out: drive parameters
{
    APIRET arc = ERROR_INVALID_DRIVE;

    if (ulLogicalDrive)
    {
        #pragma pack(1)
        // parameter packet
        struct {
            UCHAR   ucCommand,
                    ucDrive;
        } parms;
        #pragma pack()

        parms.ucCommand = 0;    // 0 = return standard media,
                                // 1 = read currently inserted media
                                // (1 doesn't work any more, returns arc 87
                                // V0.9.13 (2001-06-14) [umoeller])
        parms.ucDrive=(UCHAR)(ulLogicalDrive-1);

        // zero the structure V0.9.13 (2001-06-14) [umoeller]
        memset(pdp, 0, sizeof(BIOSPARAMETERBLOCK));

        arc = doshDevIOCtl((HFILE)-1,
                           IOCTL_DISK,                  // 0x08
                           DSK_GETDEVICEPARAMS,         // 0x63
                           &parms, sizeof(parms),
                           pdp,   sizeof(BIOSPARAMETERBLOCK));

        /* if (!arc)
        {
            _Pmpf(("      bDeviceType: %d", pdp->bDeviceType));
            _Pmpf(("      bytes per sector: %d", pdp->usBytesPerSector));
            _Pmpf(("      sectors per track: %d", pdp->usSectorsPerTrack));
        } */
    }

    return arc;
}

/*
 *@@ doshQueryDriveType:
 *      tests the specified BIOSPARAMETERBLOCK
 *      for whether it represents a CD-ROM or
 *      some other removeable drive type.
 *
 *      Returns one of:
 *
 *      --  DRVTYPE_HARDDISK (0)
 *
 *      --  DRVTYPE_PARTITIONABLEREMOVEABLE
 *
 *      --  DRVTYPE_CDROM
 *
 *      --  DRVTYPE_TAPE
 *
 *      --  DRVTYPE_VDISK
 *
 *      --  DRVTYPE_FLOPPY
 *
 *      --  DRVTYPE_UNKNOWN (255)
 *
 *      The BIOSPARAMETERBLOCK must be filled
 *      first using doshQueryDiskParams.
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

BYTE doshQueryDriveType(ULONG ulLogicalDrive,
                        PBIOSPARAMETERBLOCK pdp,
                        BOOL fFixed)
{
    if (pdp)
    {
        if (pdp->fsDeviceAttr & DEVATTR_PARTITIONALREMOVEABLE) // 0x08
            return DRVTYPE_PRT;

        if (fFixed)
            return DRVTYPE_HARDDISK;

        if (    (pdp->bDeviceType == 7)     // "other"
             && (pdp->usBytesPerSector == 2048)
             && (pdp->usSectorsPerTrack == (USHORT)-1)
           )
            return DRVTYPE_CDROM;

        switch (pdp->bDeviceType)
        {
            case DEVTYPE_TAPE: // 6
                return DRVTYPE_TAPE;

            case DEVTYPE_48TPI:     // 0, 360k  5.25" floppy
            case DEVTYPE_96TPI:     // 1, 1.2M  5.25" floppy
            case DEVTYPE_35:        // 2, 720k  3.5" floppy
            case DEVTYPE_OTHER:     // 7, 1.44  3.5" floppy
                                    //    1.84M 3.5" floppy
            case DEVTYPE_35_288MB:
                if (    (ulLogicalDrive == 1)
                     || (ulLogicalDrive == 2)
                   )
                    return DRVTYPE_FLOPPY;

                return DRVTYPE_VDISK;

            case DEVTYPE_RWOPTICAL: // 8, what is this?!?
                return DRVTYPE_FLOPPY;
        }
    }

    return DRVTYPE_UNKNOWN;
}

/*
 *@@ doshQueryCDDrives:
 *      returns the no. of CD-ROM drives on the system
 *      as well as the drive letter of the first
 *      CD-ROM drive.
 *
 *@@added V1.0.0 (2002-08-31) [umoeller]
 */

APIRET doshQueryCDDrives(PBYTE pcCDs,           // out: CD-ROM drives count
                         PCHAR pcFirstCD)       // out: drive letter of first CD
{
    APIRET  arc;
    HFILE   hfCDROM;
    ULONG   ulAction;

    if (!(arc = DosOpen("\\DEV\\CD-ROM2$",
                        &hfCDROM,
                        &ulAction,
                        0,
                        FILE_NORMAL,
                        OPEN_ACTION_OPEN_IF_EXISTS,
                        OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY,
                        NULL)))
    {
        struct
        {
            USHORT cCDs;
            USHORT usFirstCD;       // 0 == A:, 1 == B:, ...
        } cdinfo;

        ULONG cb = sizeof(cdinfo);

        if (!(arc = DosDevIOCtl(hfCDROM,
                                0x82,
                                0x60,
                                NULL,
                                0,
                                NULL,
                                &cdinfo,
                                cb,
                                &cb)))
        {
            *pcCDs = cdinfo.cCDs;
            *pcFirstCD = cdinfo.usFirstCD + 'A';
        }

        DosClose(hfCDROM);
    }

    return arc;
}

/*
 *@@ doshOpenDrive:
 *      opens the given logical drive using
 *      DosOpen with OPEN_FLAGS_DASD. Use
 *      the file handle returned from here
 *      for doshHasAudioCD and doshQueryCDStatus.
 *
 *      If NO_ERROR is returned, use DosClose
 *      to close the device again.
 *
 *@@added V1.0.0 (2002-08-31) [umoeller]
 */

APIRET doshOpenDrive(ULONG ulLogicalDrive,
                     HFILE *phf)            // out: open drive's file handle
{
    ULONG   dummy;

    CHAR    szDrive[] = "C:";
    szDrive[0] = 'A' + ulLogicalDrive - 1;

    return DosOpen(szDrive,   // "C:", "D:", ...
                   phf,
                   &dummy,
                   0,
                   FILE_NORMAL,
                   // OPEN_ACTION_FAIL_IF_NEW
                          OPEN_ACTION_OPEN_IF_EXISTS,
                   OPEN_FLAGS_DASD
                          | OPEN_FLAGS_FAIL_ON_ERROR
                                // ^^^ if this flag is not set, we get the white
                                // hard-error box
                          | OPEN_FLAGS_NOINHERIT     // V0.9.6 (2000-11-25) [pr]
               //            | OPEN_ACCESS_READONLY  // V0.9.13 (2001-06-14) [umoeller]
                          | OPEN_SHARE_DENYNONE,
                   NULL);
}

/*
 *@@ doshHasAudioCD:
 *      sets *pfAudio to whether ulLogicalDrive
 *      currently has an audio CD inserted.
 *
 *      Better call this only if you're sure that
 *      ulLogicalDrive is a CD-ROM drive. Use
 *      doshQueryRemoveableType to check.
 *
 *@@added V0.9.14 (2001-08-01) [umoeller]
 *@@changed V1.0.0 (2002-08-31) [umoeller]: removed ulLogicalDrive which was not needed
 */

APIRET doshHasAudioCD(HFILE hfDrive,            // in: DASD open
                      BOOL fMixedModeCD,
                      PBOOL pfAudio)
{
    APIRET  arc = NO_ERROR;

    ULONG ulAudioTracks = 0,
          ulDataTracks = 0;

    CHAR cds1[4] = { 'C', 'D', '0', '1' };
    CHAR cds2[4];

    *pfAudio = FALSE;

    // check for proper driver signature
    if (!(arc = doshDevIOCtl(hfDrive,
                             IOCTL_CDROMDISK,
                             CDROMDISK_GETDRIVER,
                             &cds1, sizeof(cds1),
                             &cds2, sizeof(cds2))))
    {
        if (memcmp(&cds1, &cds2, 4))
            // this is not a CD-ROM then:
            arc = NO_ERROR;
        else
        {
            #pragma pack(1)         // V1.0.0 (2002-08-31) [umoeller]

            struct {
                UCHAR   ucFirstTrack,
                        ucLastTrack;
                ULONG   ulLeadOut;
            } cdat;

            struct {
                ULONG   ulTrackAddress;
                BYTE    bFlags;
            } trackdata;

            #pragma pack()

            // get track count
            if (!(arc = doshDevIOCtl(hfDrive,
                                     IOCTL_CDROMAUDIO,
                                     CDROMAUDIO_GETAUDIODISK,
                                     &cds1, sizeof(cds1),
                                     &cdat, sizeof(cdat))))
            {
                // still no error: build the audio TOC
                ULONG i;
                for (i = cdat.ucFirstTrack;
                     i <= cdat.ucLastTrack;
                     i++)
                {
                    BYTE cdtp[5] =
                      { 'C', 'D', '0', '1', (UCHAR)i };

                    if (!(arc = doshDevIOCtl(hfDrive,
                                             IOCTL_CDROMAUDIO,
                                             CDROMAUDIO_GETAUDIOTRACK,
                                             &cdtp, sizeof(cdtp),
                                             &trackdata, sizeof(trackdata))))
                    {
                        if (trackdata.bFlags & 64)
                            ulDataTracks++;
                        else
                        {
                            ulAudioTracks++;

                            if (!fMixedModeCD)
                            {
                                // caller doesn't want mixed mode:
                                // stop here
                                ulDataTracks = 0;
                                break;
                            }
                        }
                    }
                }

                // _Pmpf(("   got %d audio, %d data tracks",
                //             ulAudioTracks, ulDataTracks));

                if (!ulDataTracks)
                    *pfAudio = TRUE;
            }
            else
            {
                // not audio disk:
                // go on then
                // _Pmpf(("   CDROMAUDIO_GETAUDIODISK returned %d", arc));
                arc = NO_ERROR;
            }
        }
    }
    else
    {
        // not CD-ROM: go on then
        // _Pmpf(("   CDROMDISK_GETDRIVER returned %d", arc));
        arc = NO_ERROR;
    }

    return arc;
}

/*
 *@@ doshQueryCDStatus:
 *      returns the status bits of a CD-ROM drive.
 *      This calls the CDROMDISK_DEVICESTATUS
 *      ioctl.
 *
 *      If NO_ERROR is returned, *pflStatus has
 *      received the following flags:
 *
 *      --  CDFL_DOOROPEN (bit 0)
 *
 *      --  CDFL_DOORLOCKED (bit 1)
 *
 *      and many more (see dosh.h).
 *
 *      Actually I wrote this function to have a way to
 *      find out whether the drive door is already open.
 *      But thanks to IBM's thoughtful design, this ioctl
 *      is 99% useless for that purpose since it requires
 *      a DASD disk handle to be passed in, which cannot
 *      be obtained if there's no media in the drive.
 *
 *      In other words, it is absolutely impossible to
 *      ever get the CDFL_DOOROPEN flag, because if the
 *      door is open, DosOpen already fails on the drive.
 *      As a consequence, it is seems to be impossible
 *      to find out if the door is open with OS/2.
 *
 *@@added V1.0.0 (2002-08-31) [umoeller]
 */

APIRET doshQueryCDStatus(HFILE hfDrive,            // in: DASD open
                         PULONG pflStatus)         // out: CD-ROM status bits
{
    APIRET  arc;

    CHAR    cds1[4] = { 'C', 'D', '0', '1' };
    ULONG   fl;

    *pflStatus = 0;

    if (!(arc = doshDevIOCtl(hfDrive,
                             IOCTL_CDROMDISK,
                             CDROMDISK_DEVICESTATUS,
                             &cds1, sizeof(cds1),
                             &fl, sizeof(fl))))
    {
        *pflStatus = fl;
    }

    return arc;
}

/*
 *@@ doshEnumDrives:
 *      this function enumerates all valid drive letters on
 *      the system by composing a string of drive letters
 *      in the buffer pointed to by pszBuffer, which should
 *      be 27 characters in size to hold information for
 *      all drives. The buffer will be null-terminated.
 *
 *      If (pcszFileSystem != NULL), only drives matching
 *      the specified file system type (e.g. "HPFS") will
 *      be enumerated. If (pcszFileSystem == NULL), all
 *      drives will be enumerated.
 *
 *      If (fSkipRemovables == TRUE), removeable drives will
 *      be skipped. This applies to floppy, CD-ROM, and
 *      virtual floppy drives. This will start the search
 *      at drive letter C: so that drives A: and B: will
 *      never be checked (to avoid the hardware bumps).
 *
 *      Otherwise, the search starts at drive A:. Still,
 *      removeable drives will only be added if valid media
 *      is inserted.
 *
 *@@changed V0.9.4 (2000-07-03) [umoeller]: this stopped at the first invalid drive letter; fixed
 *@@changed V0.9.4 (2000-07-03) [umoeller]: added fSkipRemoveables
 */

VOID doshEnumDrives(PSZ pszBuffer,      // out: drive letters
                    PCSZ pcszFileSystem,  // in: FS's to match or NULL
                    BOOL fSkipRemoveables) // in: if TRUE, only non-removeable disks will be returned
{
    CHAR    szName[5] = "";
    ULONG   ulLogicalDrive = 1, // start with drive A:
            ulFound = 0;        // found drives count
    APIRET  arc = NO_ERROR; // return code

    if (fSkipRemoveables)
        // start with drive C:
        ulLogicalDrive = 3;

    // go thru the drives, start with C: (== 3), stop after Z: (== 26)
    while (ulLogicalDrive <= 26)
    {
        #pragma pack(1)
        struct
        {
            UCHAR dummy,drive;
        } parms;
        #pragma pack()

        // data packet
        UCHAR nonRemovable=0;

        parms.drive=(UCHAR)(ulLogicalDrive-1);
        arc = doshDevIOCtl((HFILE)-1,
                           IOCTL_DISK,
                           DSK_BLOCKREMOVABLE,
                           &parms, sizeof(parms),
                           &nonRemovable, sizeof(nonRemovable));

        if (    // fixed disk and non-removeable
                ((arc == NO_ERROR) && (nonRemovable))
                // or network drive:
             || (arc == ERROR_NOT_SUPPORTED)
           )
        {
            ULONG  ulOrdinal       = 0;     // ordinal of entry in name list
            BYTE   fsqBuffer[sizeof(FSQBUFFER2) + (3 * CCHMAXPATH)] = {0};
            ULONG  cbBuffer   = sizeof(fsqBuffer);        // Buffer length)
            PFSQBUFFER2 pfsqBuffer = (PFSQBUFFER2)fsqBuffer;

            szName[0] = ulLogicalDrive + 'A' - 1;
            szName[1] = ':';
            szName[2] = '\0';

            arc = DosQueryFSAttach(szName,          // logical drive of attached FS
                                   ulOrdinal,       // ignored for FSAIL_QUERYNAME
                                   FSAIL_QUERYNAME, // return data for a Drive or Device
                                   pfsqBuffer,      // returned data
                                   &cbBuffer);      // returned data length

            if (arc == NO_ERROR)
            {
                // The data for the last three fields in the FSQBUFFER2
                // structure are stored at the offset of fsqBuffer.szName.
                // Each data field following fsqBuffer.szName begins
                // immediately after the previous item.
                CHAR* pszFSDName = (PSZ)&(pfsqBuffer->szName) + (pfsqBuffer->cbName) + 1;
                if (pcszFileSystem == NULL)
                {
                    // enum-all mode: always copy
                    pszBuffer[ulFound] = szName[0]; // drive letter
                    ulFound++;
                }
                else if (strcmp(pszFSDName, pcszFileSystem) == 0)
                {
                    pszBuffer[ulFound] = szName[0]; // drive letter
                    ulFound++;
                }
            }
        }

        ulLogicalDrive++;
    } // end while (G_acDriveLetters[ulLogicalDrive] <= 'Z')

    pszBuffer[ulFound] = '\0';
}

/*
 *@@ doshQueryBootDrive:
 *      returns the letter of the boot drive as a
 *      single (capital) character, which is useful for
 *      constructing file names using sprintf and such.
 *
 *@@changed V0.9.16 (2002-01-13) [umoeller]: optimized
 */

CHAR doshQueryBootDrive(VOID)
{
    // this can never change, so query this only once
    // V0.9.16 (2002-01-13) [umoeller]
    static CHAR     cBootDrive = '\0';

    if (!cBootDrive)
    {
        ULONG ulBootDrive;
        DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                        &ulBootDrive,
                        sizeof(ulBootDrive));
        cBootDrive = (CHAR)ulBootDrive + 'A' - 1;
    }

    return cBootDrive;
}

/*
 *@@ doshQueryMedia:
 *      determines whether the given drive currently
 *      has media inserted.
 *
 *      Call this only for non-fixed (removable) disks.
 *      Use doshIsFixedDisk to find out.
 *
 *      Returns:
 *
 *      --  NO_ERROR: media is present.
 *
 *      --  ERROR_AUDIO_CD_ROM (10000): audio CD-ROM is present.
 *
 *      --  ERROR_NOT_READY (21) or other: drive has no media.
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

APIRET doshQueryMedia(ULONG ulLogicalDrive,    // in: 1 for A:, 2 for B:, 3 for C:, ...
                      BOOL fCDROM,             // in: is drive CD-ROM?
                      ULONG fl)                // in: DRVFL_* flags
{
    APIRET  arc;

    HFILE   hf;

    // exported this code to doshOpenDrive V1.0.0 (2002-08-31) [umoeller]
    arc = doshOpenDrive(ulLogicalDrive,
                        &hf);

    // this still returns NO_ERROR for audio CDs in a
    // CD-ROM drive...
    // however, the WPS then attempts to read in the
    // root directory for audio CDs, which produces
    // a "sector not found" error box...

    if (    (!arc)
         && (hf)
         && (fCDROM)
       )
    {
        BOOL fAudio;
        if (    (!(arc = doshHasAudioCD(hf,
                                        ((fl & DRVFL_MIXEDMODECD) != 0),
                                        &fAudio)))
             && (fAudio)
           )
            arc = ERROR_AUDIO_CD_ROM;       // special private error code (10000)
    }

    if (hf)
        DosClose(hf);

    return arc;
}

/*
 *@@ doshAssertDrive:
 *      this checks for whether the given drive
 *      is currently available without provoking
 *      those ugly white "Drive not ready" popups.
 *
 *      "fl" can specify additional flags for testing
 *      and can be any combination of:
 *
 *      --  DRVFL_MIXEDMODECD: whether to allow
 *          mixed-mode CD-ROMs. See error codes below.
 *
 *      This returns (from my testing):
 *
 *      -- NO_ERROR: drive is available.
 *
 *      -- ERROR_INVALID_DRIVE (15): drive letter does not exist.
 *
 *      -- ERROR_NOT_READY (21): drive exists, but is not ready.
 *                  This is produced by floppies and CD-ROM drives
 *                  which do not have valid media inserted.
 *
 *      -- ERROR_AUDIO_CD_ROM (10000): special error code returned
 *                  only by this function if a CD-ROM drive has audio
 *                  media inserted.
 *
 *                  If DRVFL_MIXEDMODECD was specified, ERROR_AUDIO_CD_ROM
 *                  is returned _only_ if _no_ data tracks are
 *                  present on a CD-ROM. Since OS/2 is not very
 *                  good at handling mixed-mode CDs, this might not
 *                  be desireable.
 *
 *                  If DRVFL_MIXEDMODECD was not set, ERROR_AUDIO_CD_ROM
 *                  will be returned already if _one_ audio track is present.
 *
 *@@changed V0.9.1 (99-12-13) [umoeller]: rewritten, prototype changed. Now using DosOpen on the drive instead of DosError.
 *@@changed V0.9.1 (2000-01-08) [umoeller]: DosClose was called even if DosOpen failed, which messed up OS/2 error handling.
 *@@changed V0.9.1 (2000-02-09) [umoeller]: this didn't work for network drives, including RAMFS; fixed.
 *@@changed V0.9.3 (2000-03-28) [umoeller]: added check for network drives, which weren't working
 *@@changed V0.9.4 (2000-08-03) [umoeller]: more network fixes
 *@@changed V0.9.9 (2001-03-19) [pr]: validate drive number
 *@@changed V0.9.11 (2001-04-23) [umoeller]: added an extra check for floppies
 *@@changed V0.9.13 (2001-06-14) [umoeller]: added "fl" parameter and lots of CD-ROM checks
 */

APIRET doshAssertDrive(ULONG ulLogicalDrive,    // in: 1 for A:, 2 for B:, 3 for C:, ...
                       ULONG fl)                // in: DRVFL_* flags
{
    APIRET  arc = NO_ERROR;
    BOOL    fFixed = FALSE,
            fCDROM = FALSE;

    if ((ulLogicalDrive < 1) || (ulLogicalDrive > 26))
        return(ERROR_PATH_NOT_FOUND);

    arc = doshIsFixedDisk(ulLogicalDrive,
                          &fFixed);    // V0.9.13 (2001-06-14) [umoeller]

    // _Pmpf((__FUNCTION__ ": doshIsFixedDisk returned %d for disk %d", arc, ulLogicalDrive));
    // _Pmpf(("   fFixed is %d", fFixed));

    if (!arc)
        if (!fFixed)
        {
            // removeable disk:
            // check if it's a CD-ROM
            BIOSPARAMETERBLOCK bpb;
            arc = doshQueryDiskParams(ulLogicalDrive,
                                      &bpb);
            // _Pmpf(("   doshQueryDiskParams returned %d", arc));

            if (    (!arc)
                 && (DRVTYPE_CDROM == doshQueryDriveType(ulLogicalDrive,
                                                         &bpb,
                                                         fFixed))
               )
            {
                // _Pmpf(("   --> is CD-ROM"));
                fCDROM = TRUE;
            }
        }

    if (!arc)
        arc = doshQueryMedia(ulLogicalDrive,
                             fCDROM,
                             fl);

    switch (arc)
    {
        case ERROR_NETWORK_ACCESS_DENIED: // 65
            // added V0.9.3 (2000-03-27) [umoeller];
            // according to user reports, this is returned
            // by all network drives, which apparently don't
            // support DASD DosOpen
        case ERROR_ACCESS_DENIED: // 5
            // added V0.9.4 (2000-07-10) [umoeller]
            // LAN drives still didn't work... apparently
            // the above only works for NFS drives
        case ERROR_PATH_NOT_FOUND: // 3
            // added V0.9.4 (2000-08-03) [umoeller]:
            // this is returned by some other network types...
            // sigh...
        case ERROR_NOT_SUPPORTED: // 50
            // this is returned by file systems which don't
            // support DASD DosOpen;
            // use some other method then, this isn't likely
            // to fail -- V0.9.1 (2000-02-09) [umoeller]

            // but don't do this for floppies
            // V0.9.11 (2001-04-23) [umoeller]
            if (ulLogicalDrive > 2)
            {
                FSALLOCATE  fsa;
                arc = DosQueryFSInfo(ulLogicalDrive,
                                     FSIL_ALLOC,
                                     &fsa,
                                     sizeof(fsa));
                // _Pmpf(("   re-checked, DosQueryFSInfo returned %d", arc));
            }
        break;
    }

    return arc;
}

/*
 *@@ doshGetDriveInfo:
 *      fills the given XDISKINFO buffer with
 *      information about the given logical drive.
 *
 *      This function will not provoke "Drive not
 *      ready" popups, hopefully.
 *
 *      fl can be any combination of the following:
 *
 *      --  DRVFL_MIXEDMODECD: see doshAssertDrive.
 *
 *      --  DRVFL_TOUCHFLOPPIES: drive A: and B: should
 *          be touched for media checks (click, click);
 *          otherwise they will be left alone and
 *          default values will be returned.
 *
 *      --  DRVFL_CHECKEAS: drive should always be
 *          checked for EA support. If this is set,
 *          we will call DosFSCtl for the non-well-known
 *          file systems so we will always have a
 *          value for the DFL_SUPPORTS_EAS flags.
 *          The EA support returned by DosFSCtl
 *          might not be correct for remote file
 *          systems since not all of them support
 *          that query.
 *
 *          If not set, we set DFL_SUPPORTS_EAS only
 *          for file systems such as HPFS and JFS
 *          that are known to support EAs.
 *
 *      --  DRVFL_CHECKLONGNAMES: drive should be
 *          tested for longname support. If this is
 *          set, we will try a DosOpen("\\long.name.file")
 *          on the drive to see if it supports long
 *          filenames (unless it's a "well-known"
 *          file-system and we know it does). If enabled,
 *          the DFL_SUPPORTS_LONGNAMES flag is reliable.
 *          Note that this does not check for what special
 *          characters are supported in file names.
 *
 *      This should return only one of the following:
 *
 *      --  NO_ERROR: disk info was filled, but not
 *          necessarily all info was available (e.g.
 *          if no media was present in CD-ROM drive).
 *          See remarks below.
 *
 *      --  ERROR_INVALID_DRIVE 15): ulLogicalDrive
 *          is not used at all (invalid drive letter)
 *
 *      --  ERROR_BAD_UNIT (20): if drive was renamed for
 *          some reason (according to user reports
 *
 *      --  ERROR_NOT_READY (21): for ZIP disks where
 *          no media is inserted, depending on the
 *          driver apparently... normally ZIP drive
 *          letters should disappear when no media
 *          is present
 *
 *      --  ERROR_DRIVE_LOCKED (108)
 *
 *      So in order to check whether a drive is present
 *      and available, use this function as follows:
 *
 *      1)  Call this function and check whether it
 *          returns NO_ERROR for the given drive.
 *          This will rule out invalid drive letters
 *          and drives that are presently locked by
 *          CHKDSK or something.
 *
 *      2)  If so, check whether XDISKINFO.flDevice
 *          has the DFL_MEDIA_PRESENT flag set.
 *          This will rule out removeable drives without
 *          media and unformatted hard disks.
 *
 *      3)  If so, you can test the other fields if
 *          you need more information. For example,
 *          it would not be a good idea to create
 *          a new file if the bType field is
 *          DRVTYPE_CDROM.
 *
 *          If you want to exclude removeable disks,
 *          instead of checking bType, you should
 *          rather check flDevice for the DFL_FIXED
 *          flag, which will be set for ZIP drives also.
 *
 *      Remarks for special drive types:
 *
 *      --  Hard disks always have bType == DRVTYPE_HARDDISK.
 *          For them, we always check the file system.
 *          If this is reported as "UNKNOWN", this means
 *          that the drive is unformatted or formatted
 *          with a file system that OS/2 does not understand
 *          (e.g. NTFS). Only in that case, flDevice
 *          has the DFL_MEDIA_PRESENT bit clear.
 *
 *          DFL_FIXED is always set.
 *
 *      --  Remote (LAN) drives always have bType == DRVTYPE_LAN.
 *          flDevice will always have the DFL_REMOTE and
 *          DFL_MEDIA_PRESENT bits set.
 *
 *      --  ZIP disks will have bType == DRVTYPE_PARTITIONABLEREMOVEABLE.
 *          For them, flDevice will have both the
 *          and DFL_PARTITIONABLEREMOVEABLE and DFL_FIXED
 *          bits set.
 *
 *          ZIP disks are a bit special because they are
 *          dynamically mounted and unmounted when media
 *          is inserted and removed. In other words, if
 *          no media is present, the drive letter becomes
 *          invalid.
 *
 *      --  CD-ROM and DVD drives and CD writers will always
 *          be reported as DRVTYPE_CDROM. The DFL_FIXED bit
 *          will be clear always. For them, always check the
 *          DFL_MEDIA_PRESENT present bit to avoid "Drive not
 *          ready" popups.
 *
 *          As a special goody, we can also determine if the
 *          drive currently has audio media inserted (which
 *          would provoke errors also), by setting the
 *          DFL_AUDIO_CD bit.
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 *@@changed V0.9.19 (2002-04-25) [umoeller]: added CDWFS (RSJ CD-Writer)
 */

APIRET doshGetDriveInfo(ULONG ulLogicalDrive,
                        ULONG fl,               // in: DRVFL_* flags
                        PXDISKINFO pdi)
{
    APIRET  arc = NO_ERROR;

    HFILE   hf;
    ULONG   dummy;
    BOOL    fCheck = TRUE,
            fCheckFS = FALSE,
            fCheckLongnames = FALSE,
            fCheckEAs = FALSE;

    memset(pdi, 0, sizeof(XDISKINFO));

    pdi->cDriveLetter = 'A' + ulLogicalDrive - 1;
    pdi->cLogicalDrive = ulLogicalDrive;

    pdi->bType = DRVTYPE_UNKNOWN;
    pdi->fPresent = TRUE;       // for now

    if (    (ulLogicalDrive == 1)
         || (ulLogicalDrive == 2)
       )
    {
        // drive A: and B: are special cases,
        // we don't even want to touch them (click, click)
        pdi->bType = DRVTYPE_FLOPPY;

        if (0 == (fl & DRVFL_TOUCHFLOPPIES))
        {
            fCheck = FALSE;
            // these support EAs too
            pdi->flDevice  = DFL_MEDIA_PRESENT | DFL_SUPPORTS_EAS;
            strcpy(pdi->szFileSystem, "FAT");
            pdi->lFileSystem = FSYS_FAT;
        }
    }

    if (fCheck)
    {
        // any other drive:
        // check if it's removeable first
        BOOL    fFixed = FALSE;
        arc = doshIsFixedDisk(ulLogicalDrive,
                              &fFixed);

        switch (arc)
        {
            case ERROR_INVALID_DRIVE:
                // drive letter doesn't exist at all:
                pdi->fPresent = FALSE;
                // return this APIRET
            break;

            case ERROR_NOT_SUPPORTED:       // 50 for network drives
                // we get this for remote drives added
                // via "net use", so set these flags
                pdi->bType = DRVTYPE_LAN;
                pdi->lFileSystem = FSYS_REMOTE;
                pdi->flDevice |= DFL_REMOTE | DFL_MEDIA_PRESENT;
                // but still check what file-system we
                // have and whether longnames are supported
                fCheckFS = TRUE;
                fCheckLongnames = TRUE;
                fCheckEAs = TRUE;
            break;

            case NO_ERROR:
            {
                if (fFixed)
                {
                    // fixed drive:
                    pdi->flDevice |= DFL_FIXED | DFL_MEDIA_PRESENT;

                    fCheckFS = TRUE;
                    fCheckLongnames = TRUE;
                    fCheckEAs = TRUE;
                }

                if (!(arc = doshQueryDiskParams(ulLogicalDrive,
                                                &pdi->bpb)))
                {
                    BYTE bTemp = doshQueryDriveType(ulLogicalDrive,
                                                    &pdi->bpb,
                                                    fFixed);
                    if (bTemp != DRVTYPE_UNKNOWN)
                    {
                        // recognized: store it then
                        pdi->bType = bTemp;

                        if (bTemp == DRVTYPE_PRT)
                            pdi->flDevice |=    DFL_FIXED
                                              | DFL_PARTITIONABLEREMOVEABLE;
                    }

                    if (!fFixed)
                    {
                        // removeable:

                        // before checking the drive, try if we have media
                        if (!(arc = doshQueryMedia(ulLogicalDrive,
                                                   (pdi->bType == DRVTYPE_CDROM),
                                                   fl)))
                        {
                            pdi->flDevice |= DFL_MEDIA_PRESENT;
                            fCheckFS = TRUE;
                            fCheckLongnames = TRUE;
                                    // but never EAs
                        }
                        else if (arc == ERROR_AUDIO_CD_ROM)
                        {
                            pdi->flDevice |= DFL_AUDIO_CD;
                            // do not check longnames and file-system
                        }
                        else
                            pdi->arcQueryMedia = arc;

                        arc = NO_ERROR;
                    }
                }
                else
                    pdi->arcQueryDiskParams = arc;
            }
            break;

            default:
                pdi->arcIsFixedDisk = arc;
                // and return this
            break;

        } // end swich arc = doshIsFixedDisk(ulLogicalDrive, &fFixed);
    }

    if (fCheckFS)
    {
        // TRUE only for local fixed disks or
        // remote drives or if media was present above
        if (!(arc = doshQueryDiskFSType(ulLogicalDrive,
                                        pdi->szFileSystem,
                                        sizeof(pdi->szFileSystem))))
        {
            if (!stricmp(pdi->szFileSystem, "UNKNOWN"))
            {
                // this is returned by the stupid DosQueryFSAttach
                // if the file system is not recognized by OS/2,
                // or if the drive is unformatted
                pdi->lFileSystem = FSYS_UNKNOWN;
                pdi->flDevice &= ~DFL_MEDIA_PRESENT;
                fCheckLongnames = FALSE;
                fCheckEAs = FALSE;
                        // should we return ERROR_NOT_DOS_DISK (26)
                        // in this case?
            }
            else if (!stricmp(pdi->szFileSystem, "FAT"))
            {
                pdi->lFileSystem = FSYS_FAT;
                pdi->flDevice |= DFL_SUPPORTS_EAS;
                fCheckLongnames = FALSE;
                fCheckEAs = FALSE;
            }
            else if (    (!stricmp(pdi->szFileSystem, "HPFS"))
                      || (!stricmp(pdi->szFileSystem, "JFS"))
                    )
            {
                pdi->lFileSystem = FSYS_HPFS_JFS;
                pdi->flDevice |= DFL_SUPPORTS_EAS | DFL_SUPPORTS_LONGNAMES;
                fCheckLongnames = FALSE;
                fCheckEAs = FALSE;
            }
            else if (!stricmp(pdi->szFileSystem, "CDFS"))
                pdi->lFileSystem = FSYS_CDFS;
            else if (    (!stricmp(pdi->szFileSystem, "FAT32"))
                      || (!stricmp(pdi->szFileSystem, "ext2"))
                    )
            {
                pdi->lFileSystem = FSYS_FAT32_EXT2;
                fCheckLongnames = TRUE;
                fCheckEAs = TRUE;
            }
            else if (!stricmp(pdi->szFileSystem, "RAMFS"))
            {
                pdi->lFileSystem = FSYS_RAMFS;
                pdi->flDevice |= DFL_SUPPORTS_EAS | DFL_SUPPORTS_LONGNAMES;
                fCheckLongnames = FALSE;
                fCheckEAs = FALSE;
            }
            else if (!stricmp(pdi->szFileSystem, "TVFS"))
            {
                pdi->lFileSystem = FSYS_TVFS;
                fCheckLongnames = TRUE;
                fCheckEAs = TRUE;
            }
            else if (!stricmp(pdi->szFileSystem, "CDWFS"))
                    // V0.9.19 (2002-04-25) [umoeller]
            {
                pdi->lFileSystem = FSYS_CDWFS;
                pdi->flDevice |= DFL_SUPPORTS_LONGNAMES;
                fCheckLongnames = FALSE;
                fCheckEAs = FALSE;
            }
        }
        else
            // store negative error code
            pdi->lFileSystem = -(LONG)arc;
    }

    if (    (!arc)
         && (fCheckLongnames)
         && (fl & DRVFL_CHECKLONGNAMES)
       )
    {
        CHAR szTemp[] = "?:\\long.name.file";
        szTemp[0]  = ulLogicalDrive + 'A' - 1;
        if (!(arc = DosOpen(szTemp,
                            &hf,
                            &dummy,
                            0,
                            0,
                            FILE_READONLY,
                            OPEN_SHARE_DENYNONE | OPEN_FLAGS_NOINHERIT,
                            0)))
        {
            DosClose(hf);
        }

        switch (arc)
        {
            case NO_ERROR:
            case ERROR_OPEN_FAILED:
            case ERROR_FILE_NOT_FOUND:      // returned by TVFS
                pdi->flDevice |= DFL_SUPPORTS_LONGNAMES;
            break;

            // if longnames are not supported,
            // we get ERROR_INVALID_NAME
            default:
                pdi->arcOpenLongnames = arc;
            break;

            // default:
               //  printf("      drive %d returned %d\n", ulLogicalDrive, arc);
        }

        arc = NO_ERROR;
    }

    if (    (!arc)
         && (fCheckEAs)
         && (fl & DRVFL_CHECKEAS)
       )
    {
        EASIZEBUF easb = {0};
        ULONG   cbData = sizeof(easb),
                cbParams = 0;
        CHAR    szDrive[] = "?:\\";
        szDrive[0] = pdi->cDriveLetter;
        if (!(arc = DosFSCtl(&easb,
                             cbData,
                             &cbData,
                             NULL, // params,
                             cbParams,
                             &cbParams,
                             FSCTL_MAX_EASIZE,
                             szDrive,
                             -1,        // HFILE
                             FSCTL_PATHNAME)))
            if (easb.cbMaxEASize != 0)
                        // the other field (cbMaxEAListSize) is 0 always, I think
                pdi->flDevice |= DFL_SUPPORTS_EAS;
    }

    if (doshQueryBootDrive() == pdi->cDriveLetter)
        pdi->flDevice |= DFL_BOOTDRIVE;

    return arc;
}

/*
 *@@ doshSetLogicalMap:
 *      sets the mapping of logical floppy drives onto a single
 *      physical floppy drive.
 *      This means selecting either drive A: or drive B: to refer
 *      to the physical drive.
 *
 *      Paul explained this to me as follows:
 *
 *      "It is really very simple - in a single physical floppy
 *      drive system,  you still have 2 logical floppy drives
 *      A: and B:. This was primarily to  support disk copying
 *      e.g. diskcopy a: b: on a single floppy system without
 *      having to rewrite applications. Whenever the application
 *      accessed the other logical drive, the user would get a
 *      prompt from the OS to swap disks.
 *
 *      "These calls allow applications to bypass the prompt by
 *      doing the mapping themselves. They just get/set which
 *      logical drive is currently mapped to the physical drive.
 *      This concept existed in DOS as well although the specifics
 *      of how it was done escape me now.... actually I just
 *      looked it up - the byte at 0:504h in low memory on
 *      DOS controlled this (it doesn't work in VDMs)."
 *
 *@@added V0.9.6 (2000-11-24) [pr]
 */

APIRET doshSetLogicalMap(ULONG ulLogicalDrive)
{
    CHAR    name[3] = "?:";
    ULONG   fd = 0,
            action = 0;
//             paramsize = 0;
//             datasize = 0;
    APIRET  rc = NO_ERROR;
    USHORT  data,
            param;

    name[0] = doshQueryBootDrive();
    rc = DosOpen(name,
                 &fd,
                 &action,
                 0,
                 0,
                 OPEN_ACTION_FAIL_IF_NEW
                          | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_FLAGS_DASD
                       | OPEN_FLAGS_FAIL_ON_ERROR
                       | OPEN_FLAGS_NOINHERIT
                       | OPEN_ACCESS_READONLY
                       | OPEN_SHARE_DENYNONE,
                 0);

    if (rc == NO_ERROR)
    {
        param = 0;
        data = (USHORT)ulLogicalDrive;
        // paramsize = sizeof(param);
        // datasize = sizeof(data);
        rc = doshDevIOCtl(fd,
                          IOCTL_DISK, DSK_SETLOGICALMAP,
                          &param, sizeof(param),
                          &data, sizeof(data));
        DosClose(fd);
    }

    return(rc);
}

/*
 *@@ doshQueryDiskSize:
 *      returns the size of the specified disk in bytes.
 *
 *      Note: This returns a "double" value, because a ULONG
 *      can only hold values of some 4 billion, which would
 *      lead to funny results for drives > 4 GB.
 *
 *@@added V0.9.11 (2001-04-18) [umoeller]
 */

APIRET doshQueryDiskSize(ULONG ulLogicalDrive, // in: 1 for A:, 2 for B:, 3 for C:, ...
                         double *pdSize)
{
    APIRET      arc = NO_ERROR;
    FSALLOCATE  fsa;
    // double      dbl = -1;

    if (!(arc = DosQueryFSInfo(ulLogicalDrive, FSIL_ALLOC, &fsa, sizeof(fsa))))
        *pdSize = ((double)fsa.cSectorUnit * fsa.cbSector * fsa.cUnit);

    return arc;
}

/*
 *@@ doshQueryDiskFree:
 *      returns the number of bytes remaining on the disk
 *      specified by the given logical drive.
 *
 *      Note: This returns a "double" value, because a ULONG
 *      can only hold values of some 4 billion, which would
 *      lead to funny results for drives > 4 GB.
 *
 *@@changed V0.9.0 [umoeller]: fixed another > 4 GB bug (thanks to Rdiger Ihle)
 *@@changed V0.9.7 (2000-12-01) [umoeller]: changed prototype
 */

APIRET doshQueryDiskFree(ULONG ulLogicalDrive, // in: 1 for A:, 2 for B:, 3 for C:, ...
                         double *pdFree)
{
    APIRET      arc = NO_ERROR;
    FSALLOCATE  fsa;
    // double      dbl = -1;

    if (!(arc = DosQueryFSInfo(ulLogicalDrive, FSIL_ALLOC, &fsa, sizeof(fsa))))
        *pdFree = ((double)fsa.cSectorUnit * fsa.cbSector * fsa.cUnitAvail);
                   // ^ fixed V0.9.0

    return arc;
}

/*
 *@@ doshQueryDiskFSType:
 *       copies the file-system type of the given disk object
 *       (HPFS, FAT, CDFS etc.) to pszBuf.
 *       Returns the DOS error code.
 *
 *@@changed V0.9.1 (99-12-12) [umoeller]: added cbBuf to prototype
 *@@changed V0.9.14 (2001-08-01) [umoeller]: fixed, this never respected cbBuf
 *@@changed V0.9.16 (2001-10-02) [umoeller]: added check for valid logical disk no
 */

APIRET doshQueryDiskFSType(ULONG ulLogicalDrive, // in:  1 for A:, 2 for B:, 3 for C:, ...
                           PSZ pszBuf,           // out: buffer for FS type
                           ULONG cbBuf)          // in: size of that buffer
{
    APIRET arc = NO_ERROR;
    CHAR szName[5];

    BYTE   fsqBuffer[sizeof(FSQBUFFER2) + (3 * CCHMAXPATH)] = {0};
    ULONG  cbBuffer   = sizeof(fsqBuffer);        // Buffer length)
    PFSQBUFFER2 pfsqBuffer = (PFSQBUFFER2)fsqBuffer;

    // compose "D:"-type string from logical drive letter
    if (ulLogicalDrive > 0 && ulLogicalDrive < 27)
    {
        szName[0] = ulLogicalDrive + 'A' - 1;
        szName[1] = ':';
        szName[2] = '\0';

        arc = DosQueryFSAttach(szName,          // logical drive of attached FS ("D:"-style)
                               0,               // ulOrdinal, ignored for FSAIL_QUERYNAME
                               FSAIL_QUERYNAME, // return name for a drive or device
                               pfsqBuffer,      // buffer for returned data
                               &cbBuffer);      // sizeof(*pfsqBuffer)

        if (arc == NO_ERROR)
        {
            if (pszBuf)
            {
                // The data for the last three fields in the FSQBUFFER2
                // structure are stored at the offset of fsqBuffer.szName.
                // Each data field following fsqBuffer.szName begins
                // immediately after the previous item.
                strncpy(pszBuf,
                        (CHAR*)(&pfsqBuffer->szName) + pfsqBuffer->cbName + 1,
                        cbBuf);         // V0.9.14 (2001-08-01) [umoeller]
                *(pszBuf + cbBuf) = '\0';
            }
        }
    }
    else
        arc = ERROR_INVALID_PARAMETER; // V0.9.16 (2001-10-02) [umoeller]

    return arc;
}

/*
 *@@ doshQueryDiskLabel:
 *      this returns the label of a disk into
 *      *pszVolumeLabel, which must be 12 bytes
 *      in size.
 *
 *      This function was added because the Toolkit
 *      information for DosQueryFSInfo is only partly
 *      correct. On OS/2 2.x, that function does not
 *      take an FSINFO structure as input, but a VOLUMELABEL.
 *      On Warp, this does take an FSINFO.
 *
 *      DosSetFSInfo is even worse. See doshSetDiskLabel.
 *
 *      See http://zebra.asta.fh-weingarten.de/os2/Snippets/Bugi6787.HTML
 *      for details.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.11 (2001-04-22) [umoeller]: this copied even with errors, fixed
 */

APIRET doshQueryDiskLabel(ULONG ulLogicalDrive,         // in:  1 for A:, 2 for B:, 3 for C:, ...
                          PSZ pszVolumeLabel)           // out: volume label (must be 12 chars in size)
{
    APIRET      arc;

    #ifdef __OS2V2X__
        VOLUMELABEL FSInfoBuf;
    #else
        FSINFO      FSInfoBuf;
    #endif

    arc = DosQueryFSInfo(ulLogicalDrive,
                         FSIL_VOLSER,
                         &FSInfoBuf,
                         sizeof(FSInfoBuf)); // depends

    if (!arc)       // V0.9.11 (2001-04-22) [umoeller]
    {
        #ifdef __OS2V2X__
            strcpy(pszVolumeLabel, FSInfoBuf.szVolLabel);
        #else
            strcpy(pszVolumeLabel, FSInfoBuf.vol.szVolLabel);
        #endif
    }

    return arc;
}

/*
 *@@ doshSetDiskLabel:
 *      this sets the label of a disk.
 *
 *      This function was added because the Toolkit
 *      information for DosSetFSInfo is flat out wrong.
 *      That function does not take an FSINFO structure
 *      as input, but a VOLUMELABEL. As a result, using
 *      that function with the Toolkit's calling specs
 *      results in ERROR_LABEL_TOO_LONG always.
 *
 *      See http://zebra.asta.fh-weingarten.de/os2/Snippets/Bugi6787.HTML
 *      for details.
 *
 *@@added V0.9.0 [umoeller]
 */

APIRET doshSetDiskLabel(ULONG ulLogicalDrive,        // in:  1 for A:, 2 for B:, 3 for C:, ...
                        PSZ pszNewLabel)
{
    VOLUMELABEL FSInfoBuf;

    // check length; 11 chars plus null byte allowed
    FSInfoBuf.cch = (BYTE)strlen(pszNewLabel);
    if (FSInfoBuf.cch < sizeof(FSInfoBuf.szVolLabel))
    {
        strcpy(FSInfoBuf.szVolLabel, pszNewLabel);

        return DosSetFSInfo(ulLogicalDrive,
                            FSIL_VOLSER,
                            &FSInfoBuf,
                            sizeof(FSInfoBuf));
    }

    return ERROR_LABEL_TOO_LONG;
}

/*
 *@@category: Helpers\Control program helpers\File name parsing
 */

/* ******************************************************************
 *
 *   File name parsing
 *
 ********************************************************************/

/*
 *@@ doshGetDriveSpec:
 *      returns the drive specification in pcszFullFile,
 *      if any is present. This is useful for UNC support.
 *
 *      This returns:
 *
 *      --  NO_ERROR: drive spec was given, and the output
 *          fields have been set.
 *
 *      --  ERROR_INVALID_NAME: incorrect UNC syntax.
 *
 *      --  ERROR_INVALID_DRIVE: second char is ':', but
 *          drive letter is not in the range [A-Z].
 *
 *      --  ERROR_INVALID_PARAMETER: no drive spec given
 *          at all; apparently pcszFullFile is not fully
 *          qualified in the first place, or it is NULL,
 *          or its length is <= 2.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

APIRET doshGetDriveSpec(PCSZ pcszFullFile,      // in: fully q'fied file spec
                        PSZ pszDrive,           // out: drive spec ("C:" or "\\SERVER\RESOURCE"; ptr can be NULL)
                        PULONG pulDriveLen,     // out: length of drive spec (2 if local drive; ptr can be NULL)
                        PBOOL pfIsUNC)          // out: set to TRUE if UNC name, FALSE otherwise (ptr can be NULL)
{
    APIRET  arc = NO_ERROR;
    ULONG   ulFileSpecLength;

    if (    (pcszFullFile)
         && (ulFileSpecLength = strlen(pcszFullFile))
         && (ulFileSpecLength >= 2)
       )
    {
        // upper-case the drive letter
        if (pcszFullFile[1] == ':')
        {
            CHAR cDrive = toupper(*pcszFullFile);
            // local drive specified:
            if (    (cDrive >= 'A')
                 && (cDrive <= 'Z')
               )
            {
                if (pszDrive)
                {
                    pszDrive[0] = cDrive;
                    pszDrive[1] = ':';
                    pszDrive[2] = '\0';
                }

                if (pulDriveLen)
                    *pulDriveLen = 2;
                if (pfIsUNC)
                    *pfIsUNC = FALSE;
            }
            else
                // this is not a valid drive:
                arc = ERROR_INVALID_DRIVE;
        }
        else if (    (pcszFullFile[0] == '\\')
                  && (pcszFullFile[1] == '\\')
                )
        {
            // UNC drive specified:
            // this better be a full \\SERVER\RESOURCE string
            PCSZ pResource;
            if (pResource = strchr(pcszFullFile + 3, '\\'))
            {
                // we got at least \\SERVER\:
                ULONG ulLength;
                PCSZ p;

                // check if more stuff is coming
                if (p = strchr(pResource + 1, '\\'))
                {
                    // yes: copy server and resource excluding that backslash
                    if (p == pResource + 1)
                        // "\\SERVER\\" is invalid
                        arc = ERROR_INVALID_NAME;
                    else
                        // we got "\\SERVER\something\":
                        // drop the last backslash
                        ulLength = p - pcszFullFile;
                }
                else
                    // "\\SERVER\something" only:
                    ulLength = ulFileSpecLength;

                if (!arc)
                {
                    if (pszDrive)
                    {
                        memcpy(pszDrive,
                               pcszFullFile,
                               ulLength);
                        pszDrive[ulLength] = '\0';
                    }

                    if (pulDriveLen)
                        *pulDriveLen = ulLength;
                    if (pfIsUNC)
                        *pfIsUNC = TRUE;
                }
            }
            else
                // invalid UNC name:
                arc = ERROR_INVALID_NAME;
        }
        else
            // neither local, nor UNC:
            arc = ERROR_INVALID_PARAMETER;
    }
    else
        arc = ERROR_INVALID_PARAMETER;

    return arc;
}

/*
 *@@ doshGetExtension:
 *      finds the file name extension of pszFilename,
 *      which can be a file name only or a fully
 *      qualified filename.
 *
 *      This returns a pointer into pszFilename to
 *      the character after the last dot.
 *
 *      Returns NULL if not found (e.g. if the filename
 *      has no dot in it).
 *
 *      In the pathological case of a dot in the path
 *      but not in the filename itself (e.g.
 *      "C:\files.new\readme"), this correctly returns
 *      NULL.
 *
 *@@added V0.9.6 (2000-10-16) [umoeller]
 *@@changed V0.9.7 (2000-12-10) [umoeller]: fixed "F:filename.ext" case
 */

PSZ doshGetExtension(PCSZ pcszFilename)
{
    PSZ pReturn = NULL;

    if (pcszFilename)
    {
        // find filename
        PCSZ    p2,
                pStartOfName = NULL,
                pExtension = NULL;

        if (p2 = strrchr(pcszFilename + 2, '\\'))
                            // works on "C:\blah" or "\\unc\blah"
            pStartOfName = p2 + 1;
        else
        {
            // no backslash found:
            // maybe only a drive letter was specified:
            if (pcszFilename[1] == ':')
                // yes:
                pStartOfName = pcszFilename + 2;
            else
                // then this is not qualified at all...
                // use start of filename
                pStartOfName = (PSZ)pcszFilename;
        }

        // find last dot in filename
        if (pExtension = strrchr(pStartOfName, '.'))
            pReturn = (PSZ)pExtension + 1;
    }

    return pReturn;
}

/*
 *@@category: Helpers\Control program helpers\File management
 */

/* ******************************************************************
 *
 *   File helpers
 *
 ********************************************************************/

/*
 *@@ doshIsFileOnFAT:
 *      returns TRUE if pszFileName resides on
 *      a FAT drive. Note that pszFileName must
 *      be fully qualified (i.e. the drive letter
 *      must be the first character), or this will
 *      return garbage.
 */

BOOL doshIsFileOnFAT(const char* pcszFileName)
{
    BOOL brc = FALSE;
    CHAR szName[5];

    APIRET arc;
    BYTE   fsqBuffer[sizeof(FSQBUFFER2) + (3 * CCHMAXPATH)] = {0};
    ULONG  cbBuffer   = sizeof(fsqBuffer);        // Buffer length)
    PFSQBUFFER2 pfsqBuffer = (PFSQBUFFER2)fsqBuffer;

    szName[0] = pcszFileName[0];    // copy drive letter
    szName[1] = ':';
    szName[2] = '\0';

    if (!(arc = DosQueryFSAttach(szName,          // logical drive of attached FS
                                 0,               // ulOrdinal, ignored for FSAIL_QUERYNAME
                                 FSAIL_QUERYNAME, // return data for a Drive or Device
                                 pfsqBuffer,      // returned data
                                 &cbBuffer)))     // returned data length
    {
        // The data for the last three fields in the FSQBUFFER2
        // structure are stored at the offset of fsqBuffer.szName.
        // Each data field following fsqBuffer.szName begins
        // immediately after the previous item.
        if (!strncmp((PSZ)&(pfsqBuffer->szName) + pfsqBuffer->cbName + 1,
                     "FAT",
                     3))
            brc = TRUE;
    }

    return brc;
}

/*
 *@@ doshQueryFileSize:
 *      returns the size of an already opened file
 *      or 0 upon errors.
 *      Use doshQueryPathSize to query the size of
 *      any file.
 *
 *@@changed V0.9.16 (2001-10-19) [umoeller]: now returning APIRET
 */

APIRET doshQueryFileSize(HFILE hFile,       // in: file handle
                         PULONG pulSize)    // out: file size (ptr can be NULL)
{
    APIRET arc;
    FILESTATUS3 fs3;
    if (!(arc = DosQueryFileInfo(hFile, FIL_STANDARD, &fs3, sizeof(fs3))))
        if (pulSize)
            *pulSize = fs3.cbFile;
    return arc;
}

/*
 *@@ doshQueryPathSize:
 *      returns the size of any file,
 *      or 0 if the file could not be
 *      found.
 *
 *      Use doshQueryFileSize instead to query the
 *      size if you have a HFILE.
 *
 *      Otherwise this returns:
 *
 *      --  ERROR_FILE_NOT_FOUND
 *      --  ERROR_PATH_NOT_FOUND
 *      --  ERROR_SHARING_VIOLATION
 *      --  ERROR_FILENAME_EXCED_RANGE
 *      --  ERROR_INVALID_PARAMETER
 *
 *@@changed V0.9.16 (2001-10-19) [umoeller]: now returning APIRET
 */

APIRET doshQueryPathSize(PCSZ pcszFile,         // in: filename
                         PULONG pulSize)        // out: file size (ptr can be NULL)
{
    APIRET arc;

    if (pcszFile)      // V0.9.16 (2001-12-08) [umoeller]
    {
        FILESTATUS3 fs3;
        if (!(arc = DosQueryPathInfo((PSZ)pcszFile, FIL_STANDARD, &fs3, sizeof(fs3))))
            if (pulSize)
                *pulSize = fs3.cbFile;
    }
    else
        arc = ERROR_INVALID_PARAMETER;

    return arc;
}

/*
 *@@ doshQueryPathAttr:
 *      returns the file attributes of pszFile,
 *      which can be fully qualified. The
 *      attributes will be stored in *pulAttr.
 *      pszFile can also specify a directory,
 *      although not all attributes make sense
 *      for directories.
 *
 *      fAttr can be:
 *      --  FILE_ARCHIVED
 *      --  FILE_READONLY
 *      --  FILE_SYSTEM
 *      --  FILE_HIDDEN
 *
 *      This returns the APIRET of DosQueryPathInfo.
 *      *pulAttr is only valid if NO_ERROR is
 *      returned.
 *
 *      Otherwise this returns:
 *
 *      --  ERROR_FILE_NOT_FOUND
 *      --  ERROR_PATH_NOT_FOUND
 *      --  ERROR_SHARING_VIOLATION
 *      --  ERROR_FILENAME_EXCED_RANGE
 *
 *@@added V0.9.0 [umoeller]
 */

APIRET doshQueryPathAttr(const char* pcszFile,      // in: file or directory name
                         PULONG pulAttr)            // out: attributes (ptr can be NULL)
{
    FILESTATUS3 fs3;
    APIRET arc;

    if (!(arc = DosQueryPathInfo((PSZ)pcszFile,
                                  FIL_STANDARD,
                                  &fs3,
                                  sizeof(fs3))))
    {
        if (pulAttr)
            *pulAttr = fs3.attrFile;
    }

    return arc;
}

/*
 *@@ doshSetPathAttr:
 *      sets the file attributes of pszFile,
 *      which can be fully qualified.
 *      pszFile can also specify a directory,
 *      although not all attributes make sense
 *      for directories.
 *
 *      fAttr can be:
 *      --  FILE_ARCHIVED
 *      --  FILE_READONLY
 *      --  FILE_SYSTEM
 *      --  FILE_HIDDEN
 *
 *      Note that this simply sets all the given
 *      attributes; the existing attributes
 *      are lost.
 *
 *      This returns the APIRET of DosQueryPathInfo.
 */

APIRET doshSetPathAttr(const char* pcszFile,    // in: file or directory name
                       ULONG ulAttr)            // in: new attributes
{
    APIRET arc;

    if (pcszFile)
    {
        FILESTATUS3 fs3;
        if (!(arc = DosQueryPathInfo((PSZ)pcszFile,
                                      FIL_STANDARD,
                                      &fs3,
                                      sizeof(fs3))))
        {
            fs3.attrFile = ulAttr;
            arc = DosSetPathInfo((PSZ)pcszFile,
                                 FIL_STANDARD,
                                 &fs3,
                                 sizeof(fs3),
                                 DSPI_WRTTHRU);
        }
    }
    else
        arc = ERROR_INVALID_PARAMETER;

    return arc;
}

/*
 *@@ FindEAValue:
 *      returns the pointer to the EA value
 *      if the EA with the given name exists
 *      in the given FEA2LIST.
 *
 *      Within the FEA structure
 *
 +          typedef struct _FEA2 {
 +              ULONG      oNextEntryOffset;  // Offset to next entry.
 +              BYTE       fEA;               // Extended attributes flag.
 +              BYTE       cbName;            // Length of szName, not including NULL.
 +              USHORT     cbValue;           // Value length.
 +              CHAR       szName[1];         // Extended attribute name.
 +          } FEA2;
 *
 *      the EA value starts right after szName (plus its null
 *      terminator). The first USHORT of the value should
 *      normally signify the type of the EA, e.g. EAT_ASCII.
 *      This returns a pointer to that type USHORT.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 *@@changed V1.0.1 (2002-12-08) [umoeller]: moved this here from XWorkplace code, renamed from fsysFindEAValue
 */

PBYTE doshFindEAValue(PFEA2LIST pFEA2List2,      // in: file EA list
                      PCSZ pcszEAName,           // in: EA name to search for (e.g. ".LONGNAME")
                      PUSHORT pcbValue)          // out: length of value (ptr can be NULL)
{
    ULONG ulEANameLen;

    /*
    typedef struct _FEA2LIST {
        ULONG     cbList;   // Total bytes of structure including full list.
                            // Apparently, if EAs aren't supported, this
                            // is == sizeof(ULONG).
        FEA2      list[1];  // Variable-length FEA2 structures.
    } FEA2LIST;

    typedef struct _FEA2 {
        ULONG      oNextEntryOffset;  // Offset to next entry.
        BYTE       fEA;               // Extended attributes flag.
        BYTE       cbName;            // Length of szName, not including NULL.
        USHORT     cbValue;           // Value length.
        CHAR       szName[1];         // Extended attribute name.
    } FEA2;
    */

    if (!pFEA2List2)
        return NULL;

    if (    (pFEA2List2->cbList > sizeof(ULONG))
                    // FAT32 and CDFS return 4 for anything here, so
                    // we better not mess with anything else; I assume
                    // any FS which doesn't support EAs will do so then
         && (pcszEAName)
         && (ulEANameLen = strlen(pcszEAName))
       )
    {
        PFEA2 pThis = &pFEA2List2->list[0];
        // maintain a current offset so we will never
        // go beyond the end of the buffer accidentally...
        // who knows what these stupid EA routines return!
        ULONG ulOfsThis = sizeof(ULONG),
              ul = 0;

        while (ulOfsThis < pFEA2List2->cbList)
        {
            if (    (ulEANameLen == pThis->cbName)
                 && (!memcmp(pThis->szName,
                             pcszEAName,
                             ulEANameLen))
               )
            {
                if (pThis->cbValue)
                {
                    PBYTE pbValue =   (PBYTE)pThis
                                    + sizeof(FEA2)
                                    + pThis->cbName;
                    if (pcbValue)
                        *pcbValue = pThis->cbValue;
                    return pbValue;
                }
                else
                    // no value:
                    return NULL;
            }

            if (!pThis->oNextEntryOffset)
                // this was the last entry:
                return NULL;

            ulOfsThis += pThis->oNextEntryOffset;

            pThis = (PFEA2)(((PBYTE)pThis) + pThis->oNextEntryOffset);
            ul++;
        } // end while
    } // end if (    (pFEA2List2->cbList > sizeof(ULONG)) ...

    return NULL;
}

/*
 *@@ doshQueryLongname:
 *      attempts to find the value of the .LONGNAME EA in the
 *      given FEALIST and stores it in the pszLongname buffer,
 *      which must be CCHMAXPATH in size.
 *
 *      Returns TRUE if a .LONGNAME was found and copied.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 *@@changed V1.0.1 (2002-12-08) [umoeller]: moved this here from XWorkplace code, renamed from DecodeLongname
 */

BOOL doshQueryLongname(PFEA2LIST pFEA2List2,
                       PSZ pszLongname,          // out: .LONGNAME if TRUE is returned
                       PULONG pulNameLen)        // out: length of .LONGNAME string
{
    PBYTE pbValue;
    if (pbValue = doshFindEAValue(pFEA2List2,
                                  ".LONGNAME",
                                  NULL))
    {
        PUSHORT pusType = (PUSHORT)pbValue;
        if (*pusType == EAT_ASCII)
        {
            // CPREF: first word after EAT_ASCII specifies length
            PUSHORT pusStringLength = pusType + 1;      // pbValue + 2
            if (*pusStringLength)
            {
                ULONG cb = _min(*pusStringLength, CCHMAXPATH - 1);
                memcpy(pszLongname,
                       pbValue + 4,
                       cb);
                pszLongname[cb] = '\0';
                *pulNameLen = cb;
                return TRUE;
            }
        }
    }

    return FALSE;
}

/*
 *@@category: Helpers\Control program helpers\File management\XFILEs
 */

/* ******************************************************************
 *
 *   XFILEs
 *
 ********************************************************************/

/*
 * doshOpen:
 *      wrapper around DosOpen for simpler opening
 *      of files.
 *
 *      ulOpenMode determines the mode to open the
 *      file in (fptr specifies the position after
 *      the open):
 *
 +      +-------------------------+------+------------+-----------+
 +      |  ulOpenMode             | mode | if exists  | if new    |
 +      +-------------------------+------+------------+-----------+
 +      |  XOPEN_READ_EXISTING    | read | opens      | fails     |
 +      |                         |      | fptr = 0   |           |
 +      +-------------------------+------+------------+-----------+
 +      |  XOPEN_READWRITE_EXISTING r/w  | opens      | fails     |
 +      |                         |      | fptr = 0   |           |
 +      +-------------------------+------+------------+-----------+
 +      |  XOPEN_READWRITE_APPEND | r/w  | opens,     | creates   |
 +      |                         |      | appends    |           |
 +      |                         |      | fptr = end | fptr = 0  |
 +      +-------------------------+------+------------+-----------+
 +      |  XOPEN_READWRITE_NEW    | r/w  | replaces   | creates   |
 +      |                         |      | fptr = 0   | fptr = 0  |
 +      +-------------------------+------+------------+-----------+
 *
 *      In addition, you can OR one of the above values with
 *      the XOPEN_BINARY flag:
 *
 *      --  If XOPEN_BINARY is set, no conversion is performed
 *          on read and write.
 *
 *      --  If XOPEN_BINARY is _not_ set, all \n chars are
 *          converted to \r\n on write.
 *
 *      *ppFile receives a new XFILE structure describing
 *      the open file, if NO_ERROR is returned.
 *
 *      The file pointer is then set to the beginning of the
 *      file _unless_ XOPEN_READWRITE_APPEND was specified;
 *      in that case only, the file pointer is set to the
 *      end of the file so data can be appended (see above).
 *
 *      Otherwise this returns:
 *
 *      --  ERROR_FILE_NOT_FOUND
 *      --  ERROR_PATH_NOT_FOUND
 *      --  ERROR_SHARING_VIOLATION
 *      --  ERROR_FILENAME_EXCED_RANGE
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *      --  ERROR_INVALID_PARAMETER
 *
 *@@added V0.9.16 (2001-10-19) [umoeller]
 *@@changed V0.9.16 (2001-12-18) [umoeller]: fixed error codes
 *@@changed V1.0.1 (2003-01-10) [umoeller]: now allowing read for all modes
 *@@changed V1.0.2 (2003-11-13) [umoeller]: optimized; now calling doshQueryPathSize only on failure
 */

APIRET doshOpen(PCSZ pcszFilename,   // in: filename to open
                ULONG flOpenMode,       // in: XOPEN_* mode
                PULONG pcbFile,         // in: new file size (if new file is created)
                                        // out: file size
                PXFILE *ppFile)
{
    APIRET  arc = NO_ERROR;

    ULONG   fsOpenFlags = 0,
            fsOpenMode =    OPEN_FLAGS_FAIL_ON_ERROR
                          | OPEN_FLAGS_NO_LOCALITY
                          | OPEN_FLAGS_NOINHERIT;

    switch (flOpenMode & XOPEN_ACCESS_MASK)
    {
        case XOPEN_READ_EXISTING:
            fsOpenFlags =   OPEN_ACTION_FAIL_IF_NEW
                          | OPEN_ACTION_OPEN_IF_EXISTS;
            fsOpenMode |=   OPEN_SHARE_DENYWRITE
                          | OPEN_ACCESS_READONLY;

            // run this first, because if the file doesn't
            // exists, DosOpen only returns ERROR_OPEN_FAILED,
            // which isn't that meaningful
            // V0.9.16 (2001-12-08) [umoeller]
            /* arc = doshQueryPathSize(pcszFilename,
                                    pcbFile); */
                // moved this down V1.0.2 (2003-11-13) [umoeller]
        break;

        case XOPEN_READWRITE_EXISTING:
            fsOpenFlags =   OPEN_ACTION_FAIL_IF_NEW
                          | OPEN_ACTION_OPEN_IF_EXISTS;
            fsOpenMode |=   OPEN_SHARE_DENYWRITE
                          | OPEN_ACCESS_READWRITE;

            /* arc = doshQueryPathSize(pcszFilename,
                                    pcbFile); */
                // moved this down V1.0.2 (2003-11-13) [umoeller]
        break;

        case XOPEN_READWRITE_APPEND:
            fsOpenFlags =   OPEN_ACTION_CREATE_IF_NEW
                          | OPEN_ACTION_OPEN_IF_EXISTS;
            fsOpenMode |=   OPEN_SHARE_DENYWRITE // OPEN_SHARE_DENYREADWRITE V1.0.1 (2003-01-10) [umoeller]
                          | OPEN_ACCESS_READWRITE;
            // _Pmpf((__FUNCTION__ ": opening XOPEN_READWRITE_APPEND"));
        break;

        case XOPEN_READWRITE_NEW:
            fsOpenFlags =   OPEN_ACTION_CREATE_IF_NEW
                          | OPEN_ACTION_REPLACE_IF_EXISTS;
            fsOpenMode |=   OPEN_SHARE_DENYWRITE // OPEN_SHARE_DENYREADWRITE V1.0.1 (2003-01-10) [umoeller]
                          | OPEN_ACCESS_READWRITE;
            // _Pmpf((__FUNCTION__ ": opening XOPEN_READWRITE_NEW"));
        break;
    }

    if (    (!arc)
         && fsOpenFlags
         && pcbFile
         && ppFile
       )
    {
        PXFILE pFile;
        if (pFile = NEW(XFILE))
        {
            ULONG ulAction;

            ZERO(pFile);

            // copy open flags
            pFile->flOpenMode = flOpenMode;

            if (!(arc = DosOpen((PSZ)pcszFilename,
                                &pFile->hf,
                                &ulAction,
                                *pcbFile,
                                FILE_ARCHIVED,
                                fsOpenFlags,
                                fsOpenMode,
                                NULL)))       // EAs
            {
                // alright, got the file:

                if (    (ulAction == FILE_EXISTED)
                     && ((flOpenMode & XOPEN_ACCESS_MASK) == XOPEN_READWRITE_APPEND)
                   )
                    // get its size and set ptr to end for append
                    arc = DosSetFilePtr(pFile->hf,
                                        0,
                                        FILE_END,
                                        pcbFile);
                else
                    arc = doshQueryFileSize(pFile->hf,
                                            pcbFile);
                    // file ptr is at beginning

                #ifdef DEBUG_DOSOPEN
                 if (arc)
                    _Pmpf((__FUNCTION__ ": DosSetFilePtr/queryfilesize returned %d for %s",
                                arc, pcszFilename));
                #endif

                // store file size
                pFile->cbInitial
                = pFile->cbCurrent
                = *pcbFile;

                pFile->pszFilename = strdup(pcszFilename);
            }
            else
            {
                #ifdef DEBUG_DOSOPEN
                    _Pmpf((__FUNCTION__ ": DosOpen returned %d for %s",
                             arc, pcszFilename));
                #endif

                // open failed: if the file doesn't exist, DosOpen only
                // returns OPEN_FAILED, while ERROR_FILE_NOT_FOUND would
                // be a bit more informative
                // (this check used to be before DosOpen, but is a bit
                // excessive and should only be run if we really have no open)
                if (arc == ERROR_OPEN_FAILED)
                    arc = doshQueryPathSize(pcszFilename,
                                            pcbFile);
            }

            if (arc)
                doshClose(&pFile);
            else
                *ppFile = pFile;
        }
        else
            arc = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
        if (!arc)       // V0.9.19 (2002-04-02) [umoeller]
            arc = ERROR_INVALID_PARAMETER;

    return arc;
}

/*
 *@@ doshReadAt:
 *      reads cb bytes from the position specified by
 *      lOffset into the buffer pointed to by pbData,
 *      which should be cb bytes in size.
 *
 *      Note that lOffset is always considered to
 *      be from the beginning of the file (FILE_BEGIN
 *      method).
 *
 *      This implements a small cache so that several
 *      calls with a near offset will not touch the
 *      disk always. The cache has been optimized for
 *      the exeh* functions and works quite nicely
 *      there.
 *
 *      Note that the position of the file pointer is
 *      undefined after calling this function because
 *      the data might have been returned from the
 *      cache.
 *
 *      fl may be any combination of the following:
 *
 *      --  DRFL_NOCACHE: do not fill the cache with
 *          new data if the data is not in the cache
 *          currently.
 *
 *      --  DRFL_FAILIFLESS: return ERROR_NO_DATA
 *          if the data returned by DosRead is less
 *          than what was specified. This might
 *          simplify error handling.
 *
 *@@added V0.9.13 (2001-06-14) [umoeller]
 *@@changed V0.9.16 (2001-12-18) [umoeller]: now with XFILE, and always using FILE_BEGIN
 *@@changed V0.9.19 (2002-04-02) [umoeller]: added params checking
 *@@changed V1.0.2 (2003-11-13) [umoeller]: optimized cache (using realloc)
 */

APIRET doshReadAt(PXFILE pFile,
                  ULONG ulOffset,   // in: offset to read from (from beginning of file)
                  PULONG pcb,       // in: bytes to read, out: bytes read (req.)
                  PBYTE pbData,     // out: read buffer (must be cb bytes)
                  ULONG fl)         // in: DRFL_* flags
{
    APIRET arc = NO_ERROR;
    ULONG cb;
    ULONG ulDummy;

    if (!pFile || !pcb)
        // V0.9.19 (2002-04-02) [umoeller]
        return ERROR_INVALID_PARAMETER;

    cb = *pcb;
    *pcb = 0;

    // check if we have the data in the cache already;

    if (    (pFile->pbCache)
            // first byte must be in cache
         && (ulOffset >= pFile->ulReadFrom)
            // last byte must be in cache
         && (    ulOffset + cb
              <= pFile->ulReadFrom + pFile->cbCache
            )
       )
    {
        // alright, return data from cache simply
        ULONG ulOfsInCache = ulOffset - pFile->ulReadFrom;

        memcpy(pbData,
               pFile->pbCache + ulOfsInCache,
               cb);
        *pcb = cb;

        #ifdef DEBUG_DOSOPEN
        _Pmpf((__FUNCTION__ " %s: data is fully in cache",
                    pFile->pszFilename));
        _Pmpf(("  caller wants %d bytes from %d",
                    cb, ulOffset));
        _Pmpf(("  we got %d bytes from %d",
                    pFile->cbCache, pFile->ulReadFrom));
        _Pmpf(("  so copied %d bytes from cache ofs %d",
                    cb, ulOfsInCache));
        #endif
    }
    else
    {
        // data is not in cache:
        // check how much it is... for small amounts,
        // we load the cache first
        if (    (cb <= 4096 - 512)
             && (!(fl & DRFL_NOCACHE))
           )
        {
            #ifdef DEBUG_DOSOPEN
            _Pmpf((__FUNCTION__ " %s: filling cache anew",
                    pFile->pszFilename));
            _Pmpf(("  caller wants %d bytes from %d",
                        cb, ulOffset));
            #endif

            // OK, then fix the offset to read from
            // to a multiple of 512 to get a full sector
            pFile->ulReadFrom = ulOffset / 512L * 512L;
            // and read 4096 bytes always plus the
            // value we cut off above
            pFile->cbCache = 4096;

            #ifdef DEBUG_DOSOPEN
            _Pmpf(("  getting %d bytes from %d",
                        pFile->cbCache, pFile->ulReadFrom));
            #endif

#if 0
            // free old cache
            if (pFile->pbCache)
                free(pFile->pbCache);

            // allocate new cache
            if (!(pFile->pbCache = (PBYTE)malloc(pFile->cbCache)))
#else
            // realloc is better V1.0.2 (2003-11-13) [umoeller]
            if (!(pFile->pbCache = (PBYTE)realloc(pFile->pbCache,
                                                  pFile->cbCache)))
#endif
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                ULONG ulOfsInCache = 0;

                if (!(arc = DosSetFilePtr(pFile->hf,
                                          (LONG)pFile->ulReadFrom,
                                          FILE_BEGIN,
                                          &ulDummy)))
                {
                    if (!(arc = DosRead(pFile->hf,
                                        pFile->pbCache,
                                        pFile->cbCache,
                                        &ulDummy)))
                    {
                        // got data:
                        #ifdef DEBUG_DOSOPEN
                        _Pmpf(("        %d bytes read", ulDummy));
                        #endif

                        pFile->cbCache = ulDummy;

                        // check bounds
                        ulOfsInCache = ulOffset - pFile->ulReadFrom;

                        /*
                        if (ulOfsInCache + cb > pFile->cbCache)
                        {
                            cb = pFile->cbCache - ulOfsInCache;
                            if (fl & DRFL_FAILIFLESS)
                                arc = ERROR_NO_DATA;
                        }
                        */
                    }
                }

                if (!arc)
                {
                    // copy to caller
                    memcpy(pbData,
                           pFile->pbCache + ulOfsInCache,
                           cb);
                    *pcb = cb;

                    #ifdef DEBUG_DOSOPEN
                    _Pmpf(("  so copied %d bytes from cache ofs %d",
                                cb, ulOfsInCache));
                    #endif
                }
                else
                {
                    free(pFile->pbCache);
                    pFile->pbCache = NULL;
                }
            } // end else if (!(pFile->pbCache = (PBYTE)malloc(pFile->cbCache)))
        }
        else
        {
            // read uncached:
            #ifdef DEBUG_DOSOPEN
            _Pmpf(("  " __FUNCTION__ " %s: reading uncached",
                        pFile->pszFilename));
            _Pmpf(("      caller wants %d bytes from %d",
                        cb, ulOffset));
            #endif

            if (!(arc = DosSetFilePtr(pFile->hf,
                                      (LONG)ulOffset,
                                      FILE_BEGIN,
                                      &ulDummy)))
            {
                if (!(arc = DosRead(pFile->hf,
                                    pbData,
                                    cb,
                                    &ulDummy)))
                {
                    if (    (fl & DRFL_FAILIFLESS)
                         && (ulDummy != cb)
                       )
                        arc = ERROR_NO_DATA;
                    else
                        *pcb = ulDummy;     // bytes read
                }
            }
        }
    }

    return arc;
}

/*
 *@@ doshWrite:
 *      writes the specified data to the file.
 *      If (cb == 0), this runs strlen on pcsz
 *      to find out the length.
 *
 *      If the file is not in binary mode, all
 *      \n chars are converted to \r\n before
 *      writing.
 *
 *      Note that this expects that the file
 *      pointer is at the end of the file, or
 *      you will get garbage.
 *
 *@@added V0.9.16 (2001-10-19) [umoeller]
 *@@changed V0.9.16 (2001-12-02) [umoeller]: added XOPEN_BINARY \r\n support
 *@@changed V0.9.16 (2001-12-06) [umoeller]: added check for pFile != NULL
 *@@changed V1.0.1 (2003-01-10) [umoeller]: now forcing DosSetFileSize
 */

APIRET doshWrite(PXFILE pFile,
                 ULONG cb,
                 PCSZ pbData)
{
    APIRET arc = NO_ERROR;
    if ((!pFile) || (!pbData))
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        if (!cb)
            cb = strlen(pbData);

        if (!cb)
            arc = ERROR_INVALID_PARAMETER;
        else
        {
            PSZ pszNew = NULL;

            if (!(pFile->flOpenMode & XOPEN_BINARY))
            {
                // convert all \n to \r\n:
                // V0.9.16 (2001-12-02) [umoeller]

                // count all \n first
                ULONG cNewLines = 0;
                PCSZ pSource = pbData;
                ULONG ul;
                for (ul = 0;
                     ul < cb;
                     ul++)
                {
                    if (*pSource++ == '\n')
                        cNewLines++;
                }

                if (cNewLines)
                {
                    // we have '\n' chars:
                    // then we need just as many \r chars inserted
                    ULONG cbNew = cb + cNewLines;
                    if (!(pszNew = (PSZ)malloc(cbNew)))
                        arc = ERROR_NOT_ENOUGH_MEMORY;
                    else
                    {
                        PSZ pTarget = pszNew;
                        pSource = pbData;
                        for (ul = 0;
                             ul < cb;
                             ul++)
                        {
                            CHAR c = *pSource++;
                            if (c == '\n')
                                *pTarget++ = '\r';
                            *pTarget++ = c;
                        }

                        cb = cbNew;
                    }
                }
            }

            if (!arc)
            {
                ULONG cbWritten;
                if (!(arc = DosWrite(pFile->hf,
                                     (pszNew)
                                            ? pszNew
                                            : (PSZ)pbData,
                                     cb,
                                     &cbWritten)))
                {
                    pFile->cbCurrent += cbWritten;
                    // invalidate the cache
                    FREE(pFile->pbCache);

                    // force a writeout V1.0.1 (2003-01-10) [umoeller]
                    DosSetFileSize(pFile->hf, pFile->cbCurrent);
                }
            }

            if (pszNew)
                free(pszNew);
        }
    }

    return arc;
}

/*
 *@@ doshWriteAt:
 *      writes cb bytes (pointed to by pbData) to the
 *      specified file at the position lOffset (from
 *      the beginning of the file).
 *
 *@@added V0.9.13 (2001-06-14) [umoeller]
 */

APIRET doshWriteAt(PXFILE pFile,
                   ULONG ulOffset,    // in: offset to write at
                   ULONG cb,        // in: bytes to write
                   PCSZ pbData)     // in: ptr to bytes to write (must be cb bytes)
{
    APIRET arc = NO_ERROR;
    ULONG cbWritten;
    if (!(arc = DosSetFilePtr(pFile->hf,
                              (LONG)ulOffset,
                              FILE_BEGIN,
                              &cbWritten)))
    {
        if (!(arc = DosWrite(pFile->hf,
                             (PSZ)pbData,
                             cb,
                             &cbWritten)))
        {
            if (ulOffset + cbWritten > pFile->cbCurrent)
                pFile->cbCurrent = ulOffset + cbWritten;
            // invalidate the cache V0.9.19 (2002-04-02) [umoeller]
            FREE(pFile->pbCache);
        }
    }

    return arc;
}

/*
 *@@ doshWriteLogEntry
 *      writes a log string to an XFILE, adding a
 *      leading timestamp before the line.
 *
 *      The internal string buffer is limited to 2000
 *      characters. Length checking is _not_ performed.
 *
 *@@added V0.9.16 (2001-10-19) [umoeller]
 *@@changed V0.9.16 (2001-12-06) [umoeller]: added check for pFile != NULL
 */

APIRET doshWriteLogEntry(PXFILE pFile,
                         const char* pcszFormat,
                         ...)
{
    APIRET arc = NO_ERROR;

    if ((!pFile) || (!pcszFormat))
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        DATETIME dt;
        CHAR szTemp[2000];
        ULONG   ulLength;

        DosGetDateTime(&dt);
        if (ulLength = sprintf(szTemp,
                               "%04d-%02d-%02d %02d:%02d:%02d:%02d ",
                               dt.year, dt.month, dt.day,
                               dt.hours, dt.minutes, dt.seconds, dt.hundredths))
        {
            if (!(arc = doshWrite(pFile,
                                  ulLength,
                                  szTemp)))
            {
                va_list arg_ptr;
                va_start(arg_ptr, pcszFormat);
                ulLength = vsprintf(szTemp, pcszFormat, arg_ptr);
                va_end(arg_ptr);

                if (pFile->flOpenMode & XOPEN_BINARY)
                    // if we're in binary mode, we need to add \r too
                    szTemp[ulLength++] = '\r';
                szTemp[ulLength++] = '\n';

                arc = doshWrite(pFile,
                                ulLength,
                                szTemp);
            }
        }
    }

    return arc;
}

/*
 * doshClose:
 *      closes an XFILE opened by doshOpen and
 *      sets *ppFile to NULL.
 *
 *@@added V0.9.16 (2001-10-19) [umoeller]
 */

APIRET doshClose(PXFILE *ppFile)
{
    APIRET arc = NO_ERROR;
    PXFILE pFile;

    if (    (ppFile)
         && (pFile = *ppFile)
       )
    {
        // set the ptr to NULL
        *ppFile = NULL;

        FREE(pFile->pbCache);
        FREE(pFile->pszFilename);

        if (pFile->hf)
        {
            DosSetFileSize(pFile->hf, pFile->cbCurrent);
            DosClose(pFile->hf);
            pFile->hf = NULLHANDLE;
        }

        free(pFile);
    }
    else
        arc = ERROR_INVALID_PARAMETER;

    return arc;
}

/*
 *@@ doshReadText:
 *      reads all the contents of the given XFILE into
 *      a newly allocated buffer. Handles Ctrl-Z properly.
 *
 *      Implementation for doshLoadTextFile, but can
 *      be called separately now.
 *
 *@@added V0.9.20 (2002-07-19) [umoeller]
 */

APIRET doshReadText(PXFILE pFile,
                    PSZ* ppszContent,   // out: newly allocated buffer with file's content
                    PULONG pcbRead)     // out: size of that buffer including null byte (ptr can be NULL)
{
    APIRET  arc;
    PSZ     pszContent;

    if (!(pszContent = (PSZ)malloc(pFile->cbCurrent + 1)))
        arc = ERROR_NOT_ENOUGH_MEMORY;
    else
    {
        ULONG cbRead = 0;
        if (!(arc = DosRead(pFile->hf,
                            pszContent,
                            pFile->cbCurrent,
                            &cbRead)))
        {
            if (cbRead != pFile->cbCurrent)
                arc = ERROR_NO_DATA;
            else
            {
                PSZ p;
                pszContent[cbRead] = '\0';

                // check if we have a ctrl-z (EOF) marker
                // this is present, for example, in config.sys
                // after install, and stupid E.EXE writes this
                // all the time when saving a file
                // V0.9.18 (2002-03-08) [umoeller]
                // if (p = strchr(pszContent, '\26')) What the hell is this??? Octal 26 = Decimal 22 != Ctrl-Z
                if (p = strchr(pszContent, '\x1A')) // V1.0.4 (2004-12-18) [pr]
                {
                    *p = '\0';
                    cbRead = p - pszContent;
                }

                *ppszContent = pszContent;
                if (pcbRead)
                    *pcbRead = cbRead + 1;
            }
        }

        if (arc)
            free(pszContent);
    }

    return arc;
}

/*
 *@@ doshLoadTextFile:
 *      reads a text file from disk, allocates memory
 *      via malloc() and sets a pointer to this
 *      buffer (or NULL upon errors).
 *
 *      This allocates one extra byte to make the
 *      buffer null-terminated always. The buffer
 *      is _not_ converted WRT the line format.
 *
 *      If CTRL-Z (ASCII 26) is encountered in the
 *      content, it is set to the null character
 *      instead (V0.9.18).
 *
 *      This returns the APIRET of DosOpen and DosRead.
 *      If any error occurred, no buffer was allocated.
 *      Otherwise, you should free() the buffer when
 *      no longer needed.
 *
 *@@changed V0.9.7 (2001-01-15) [umoeller]: renamed from doshReadTextFile
 *@@changed V0.9.16 (2002-01-05) [umoeller]: added pcbRead
 *@@changed V0.9.16 (2002-01-05) [umoeller]: rewritten using doshOpen
 *@@changed V0.9.18 (2002-03-08) [umoeller]: fixed ctrl-z (EOF) bug
 */

APIRET doshLoadTextFile(PCSZ pcszFile,      // in: file name to read
                        PSZ* ppszContent,   // out: newly allocated buffer with file's content
                        PULONG pcbRead)     // out: size of that buffer including null byte (ptr can be NULL)
{
    APIRET  arc;

    ULONG   cbFile = 0;
    PXFILE  pFile = NULL;

    if (!(arc = doshOpen(pcszFile,
                         XOPEN_READ_EXISTING,
                         &cbFile,
                         &pFile)))
    {
        doshReadText(pFile,
                     ppszContent,
                     pcbRead);
        doshClose(&pFile);
    }

    return arc;
}

/*
 *@@ doshCreateLogFilename:
 *      produces a log filename in pszBuf.
 *      If $(LOGFILES) is set, that directory
 *      is used; otherwise we use the root
 *      directory of the boot drive.
 *
 *@@added V1.0.1 (2003-01-25) [umoeller]
 */

BOOL doshCreateLogFilename(PSZ pszBuf,             // out: fully qualified filename
                           PCSZ pcszFilename,      // in: short log filename
                           BOOL fAllowBootDrive)   // in: allow creating log files on boot drive?
{
    CHAR    szBoot[] = "?:";
//#ifdef __cplusplus             // stupid OS/2 C++ prototypes
//    PCSZ     pszLogDir;
//#else
      PSZ     pszLogDir;
//#endif
    if (DosScanEnv("LOGFILES",      // new eCS 1.1 setting
                   &pszLogDir))
    {
        // variable not set:
        if (!fAllowBootDrive)
            return FALSE;

        szBoot[0] = doshQueryBootDrive();
        pszLogDir = szBoot;
    }

    sprintf(pszBuf,
            "%s\\%s",
            pszLogDir,
            pcszFilename);

    return TRUE;
}

/*
 *@@ doshCreateBackupFileName:
 *      creates a valid backup filename of pszExisting
 *      with a numerical file name extension which does
 *      not exist in the directory where pszExisting
 *      resides.
 *      Returns a PSZ to a new buffer which was allocated
 *      using malloc().
 *
 *      <B>Example:</B> returns "C:\CONFIG.002" for input
 *      "C:\CONFIG.SYS" if "C:\CONFIG.001" already exists.
 *
 *@@changed V0.9.1 (99-12-13) [umoeller]: this crashed if pszExisting had no file extension
 */

PSZ doshCreateBackupFileName(const char* pszExisting)
{
    CHAR    szFilename[CCHMAXPATH];
    PSZ     pszLastDot;
    ULONG   ulCount = 1;
    CHAR    szCount[5];
    ULONG   ulDummy;

    strcpy(szFilename, pszExisting);

    if (!(pszLastDot = strrchr(szFilename, '.')))
        // no dot in filename:
        pszLastDot = szFilename + strlen(szFilename);

    do
    {
        sprintf(szCount, ".%03lu", ulCount);
        strcpy(pszLastDot, szCount);
        ulCount++;
    } while (!doshQueryPathSize(szFilename, &ulDummy));

    return strdup(szFilename);
}

/*
 *@@ doshCreateTempFileName:
 *      produces a file name in the the specified directory
 *      or $(TEMP) which presently doesn't exist. This
 *      checks the directory for existing files, but does
 *      not open the temp file.
 *
 *      If (pcszDir != NULL), we look into that directory.
 *      Otherwise we look into the directory specified
 *      by the $(TEMP) environment variable.
 *      If $(TEMP) is not set, $(TMP) is tried next.
 *
 *      If the directory thus specified does not exist, the
 *      root directory of the boot drive is used instead.
 *      As a result, this function should be fairly bomb-proof.
 *
 *      If (pcszExt != NULL), the temp file receives
 *      that extension, or no extension otherwise.
 *      Do not specify the dot in pcszExt.
 *
 *      pszTempFileName receives the fully qualified
 *      file name of the temp file in that directory
 *      and must point to a buffer CCHMAXPATH in size.
 *      The file name is 8+3 compliant if pcszExt does
 *      not exceed three characters.
 *
 *      If (pcszPrefix != NULL), the temp file name
 *      is prefixed with pcszPrefix. Since the temp
 *      file name must not exceed 8+3 letters, we
 *      can only use ( 8 - strlen(pcszPrefix ) digits
 *      for a random number to make the temp file name
 *      unique. You must therefore use a maximum of
 *      four characters for the prefix. Otherwise
 *      ERROR_INVALID_PARAMETER is returned.
 *
 *      Example: Assuming TEMP is set to C:\TEMP,
 +
 +          doshCreateTempFileName(szBuffer,
 +                                 NULL,             // use $(TEMP)
 +                                 "pre",            // prefix
 +                                 "tmp")            // extension
 +
 *      would produce something like "C:\TEMP\pre07FG2.tmp".
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 */

APIRET doshCreateTempFileName(PSZ pszTempFileName,        // out: fully q'fied temp file name
                              PCSZ pcszDir,        // in: dir or NULL for %TEMP%
                              PCSZ pcszPrefix,     // in: prefix for temp file or NULL
                              PCSZ pcszExt)        // in: extension (without dot) or NULL
{
    APIRET      arc = NO_ERROR;

    ULONG       ulPrefixLen = (pcszPrefix)
                                    ? strlen(pcszPrefix)
                                    : 0;

    if (    (!pszTempFileName)
         || (ulPrefixLen > 4)
       )
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        CHAR        szDir[CCHMAXPATH] = "";
        FILESTATUS3 fs3;

        const char  *pcszTemp = pcszDir;

        if (!pcszTemp)
        {
            if (!(pcszTemp = getenv("TEMP")))
                pcszTemp = getenv("TMP");
        }

        if (pcszTemp)       // either pcszDir or $(TEMP) or $(TMP) now
            if (DosQueryPathInfo((PSZ)pcszTemp,
                                 FIL_STANDARD,
                                 &fs3,
                                 sizeof(fs3)))
                // TEMP doesn't exist:
                pcszTemp = NULL;

        if (!pcszTemp)
            // not set, or doesn't exist:
            // use root directory on boot drive
            sprintf(szDir,
                    "%c:\\",
                    doshQueryBootDrive());
        else
        {
            strcpy(szDir, pcszTemp);
            if (szDir[strlen(szDir) - 1] != '\\')
                strcat(szDir, "\\");
        }

        if (!szDir[0])
            arc = ERROR_PATH_NOT_FOUND;     // shouldn't happen
        else
        {
            ULONG       ulRandom = 0;
            ULONG       cAttempts = 0;

            // produce random number
            DosQuerySysInfo(QSV_MS_COUNT,
                            QSV_MS_COUNT,
                            &ulRandom,
                            sizeof(ulRandom));

            do
            {
                CHAR szFile[20] = "",
                     szFullTryThis[CCHMAXPATH];

                // use the lower eight hex digits of the
                // system uptime as the temp dir name
                sprintf(szFile,
                        "%08lX",
                        ulRandom & 0xFFFFFFFF);

                // if prefix is specified, overwrite the
                // first characters in the random number
                if (pcszPrefix)
                    memcpy(szFile, pcszPrefix, ulPrefixLen);

                if (pcszExt)
                {
                    szFile[8] = '.';
                    strcpy(szFile + 9, pcszExt);
                }

                // now compose full temp file name
                strcpy(szFullTryThis, szDir);
                strcat(szFullTryThis, szFile);
                // now we have: "C:\temp\wpiXXXXX"
                if (DosQueryPathInfo(szFullTryThis,
                                     FIL_STANDARD,
                                     &fs3,
                                     sizeof(fs3))
                        == ERROR_FILE_NOT_FOUND)
                {
                    // file or dir doesn't exist:
                    // cool, we're done
                    strcpy(pszTempFileName, szFullTryThis);
                    return NO_ERROR;
                }

                // if this didn't work, raise ulRandom and try again
                ulRandom += 123;

                // try only 100 times, just to be sure
                cAttempts++;
            } while (cAttempts < 100);

            // 100 loops elapsed:
            arc = ERROR_BAD_FORMAT;
        }
    }

    return arc;
}

/*
 *@@ doshWriteTextFile:
 *      writes a text file to disk; pszFile must contain the
 *      whole path and filename.
 *
 *      An existing file will be backed up if (pszBackup != NULL),
 *      using doshCreateBackupFileName. In that case, pszBackup
 *      receives the name of the backup created, so that buffer
 *      should be CCHMAXPATH in size.
 *
 *      If (pszbackup == NULL), an existing file will be overwritten.
 *
 *      On success (NO_ERROR returned), *pulWritten receives
 *      the no. of bytes written.
 *
 *@@changed V0.9.3 (2000-05-01) [umoeller]: optimized DosOpen; added error checking; changed prototype
 *@@changed V0.9.3 (2000-05-12) [umoeller]: added pszBackup
 */

APIRET doshWriteTextFile(const char* pszFile,        // in: file name
                         const char* pszContent,     // in: text to write
                         PULONG pulWritten,          // out: bytes written (ptr can be NULL)
                         PSZ pszBackup)              // in/out: create-backup?
{
    APIRET  arc = NO_ERROR;
    ULONG   ulWritten = 0;

    if ((!pszFile) || (!pszContent))
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        ULONG   ulAction = 0,
                ulLocal = 0;
        HFILE   hFile = 0;

        ULONG ulSize = strlen(pszContent);      // exclude 0 byte

        if (pszBackup)
        {
            PSZ     pszBackup2 = doshCreateBackupFileName(pszFile);
            DosCopy((PSZ)pszFile,
                    pszBackup2,
                    DCPY_EXISTING); // delete existing
            strcpy(pszBackup, pszBackup2);
            free(pszBackup2);
        }

        if (!(arc = DosOpen((PSZ)pszFile,
                            &hFile,
                            &ulAction,                      // action taken
                            ulSize,                         // primary allocation size
                            FILE_ARCHIVED | FILE_NORMAL,    // file attribute
                            OPEN_ACTION_CREATE_IF_NEW
                               | OPEN_ACTION_REPLACE_IF_EXISTS,  // open flags
                            OPEN_FLAGS_NOINHERIT
                               | OPEN_FLAGS_SEQUENTIAL         // sequential, not random access
                               | OPEN_SHARE_DENYWRITE          // deny write mode
                               | OPEN_ACCESS_WRITEONLY,        // write mode
                            NULL)))                         // no EAs
        {
            if (!(arc = DosSetFilePtr(hFile,
                                      0L,
                                      FILE_BEGIN,
                                      &ulLocal)))
                if (!(arc = DosWrite(hFile,
                                     (PVOID)pszContent,
                                     ulSize,
                                     &ulWritten)))
                    arc = DosSetFileSize(hFile, ulSize);

            DosClose(hFile);
        }
    } // end if ((pszFile) && (pszContent))

    if (pulWritten)
        *pulWritten = ulWritten;

    return arc;
}

/*
 *@@category: Helpers\Control program helpers\Directory management
 *      directory helpers (querying, creating, deleting etc.).
 */

/* ******************************************************************
 *
 *   Directory helpers
 *
 ********************************************************************/

/*
 *@@ doshQueryDirExist:
 *      returns TRUE if the given directory
 *      exists and really is a directory.
 */

BOOL doshQueryDirExist(PCSZ pcszDir)
{
    FILESTATUS3 fs3;
    APIRET arc;

    if (!(arc = DosQueryPathInfo((PSZ)pcszDir,
                                 FIL_STANDARD,
                                 &fs3,
                                 sizeof(fs3))))
        // file found:
        return ((fs3.attrFile & FILE_DIRECTORY) != 0);

    return FALSE;
}

/*
 *@@ doshCreatePath:
 *      this creates the specified directory.
 *      As opposed to DosCreateDir, this
 *      function can create several directories
 *      at the same time, if the parent
 *      directories do not exist yet.
 */

APIRET doshCreatePath(PCSZ pcszPath,
                      BOOL fHidden) // in: if TRUE, the new directories will get FILE_HIDDEN
{
    APIRET  arc0 = NO_ERROR;
    CHAR    path[CCHMAXPATH];
    CHAR    *cp, c;
    ULONG   cbPath;

    strcpy(path, pcszPath);
    cbPath = strlen(path);

    if (path[cbPath] != '\\')
    {
        path[cbPath] = '\\';
        path[cbPath + 1] = 0;
    }

    cp = path;
    // advance past the drive letter only if we have one
    if (*(cp+1) == ':')
        cp += 3;

    // go!
    while (*cp != 0)
    {
        if (*cp == '\\')
        {
            c = *cp;
            *cp = 0;
            if (!doshQueryDirExist(path))
            {
                APIRET arc = DosCreateDir(path,
                                          0);   // no EAs
                if (arc != NO_ERROR)
                {
                    arc0 = arc;
                    break;
                }
                else
                    if (fHidden)
                    {
                        // hide the directory we just created
                        doshSetPathAttr(path, FILE_HIDDEN);
                    }
            }
            *cp = c;
        }
        cp++;
    }

    return arc0;
}

/*
 *@@ doshQueryCurrentDir:
 *      writes the current directory into
 *      the specified buffer, which should be
 *      CCHMAXPATH in size.
 *
 *      As opposed to DosQueryCurrentDir, this
 *      includes the drive letter.
 *
 *@@added V0.9.4 (2000-07-22) [umoeller]
 */

APIRET doshQueryCurrentDir(PSZ pszBuf)
{
    APIRET arc = NO_ERROR;
    ULONG   ulCurDisk = 0;
    ULONG   ulMap = 0;
    if (!(arc = DosQueryCurrentDisk(&ulCurDisk, &ulMap)))
    {
        ULONG   cbBuf = CCHMAXPATH - 3;
        pszBuf[0] = ulCurDisk + 'A' - 1;
        pszBuf[1] = ':';
        pszBuf[2] = '\\';
        pszBuf[3] = '\0';
        arc = DosQueryCurrentDir(0, (unsigned char *)pszBuf + 3, &cbBuf);
    }

    return arc;
}

/*
 *@@ doshDeleteDir:
 *      deletes a directory. As opposed to DosDeleteDir,
 *      this removes empty subdirectories and/or files
 *      as well.
 *
 *      flFlags can be any combination of the following:
 *
 *      -- DOSHDELDIR_RECURSE: recurse into subdirectories.
 *
 *      -- DOSHDELDIR_DELETEFILES: delete all regular files
 *              which are found on the way.
 *
 *              THIS IS NOT IMPLEMENTED YET.
 *
 *      If 0 is specified, this effectively behaves just as
 *      DosDeleteDir.
 *
 *      If you specify DOSHDELDIR_RECURSE only, only empty
 *      subdirectories are deleted as well.
 *
 *      If you specify DOSHDELDIR_RECURSE | DOSHDELDIR_DELETEFILES,
 *      this removes an entire directory tree, including all
 *      subdirectories and files.
 *
 *@@added V0.9.4 (2000-07-01) [umoeller]
 *@@changed WarpIN V1.0.15 (2007-03-27) [pr]: allow NULL pulDirs and pulFiles
 *@@changed WarpIN V1.0.16 (2007-07-28) [pr]: bump directory count on recursive delete failures @@fixes 1001
 */

APIRET doshDeleteDir(PCSZ pcszDir,
                     ULONG flFlags,
                     PULONG pulDirs,        // out: directories found
                     PULONG pulFiles)       // out: files found
{
    APIRET          arc = NO_ERROR,
                    arcReturn = NO_ERROR;

    HDIR            hdirFindHandle = HDIR_CREATE;
    FILEFINDBUF3    ffb3     = {0};      // returned from FindFirst/Next
    ULONG           ulResultBufLen = sizeof(FILEFINDBUF3);
    ULONG           ulFindCount    = 1;        // look for 1 file at a time

    CHAR            szFileMask[2*CCHMAXPATH];
    sprintf(szFileMask, "%s\\*", pcszDir);

    // find files
    arc = DosFindFirst(szFileMask,
                       &hdirFindHandle,     // directory search handle
                       FILE_ARCHIVED | FILE_DIRECTORY | FILE_SYSTEM
                         | FILE_HIDDEN | FILE_READONLY,
                                     // search attributes; include all, even dirs
                       &ffb3,               // result buffer
                       ulResultBufLen,      // result buffer length
                       &ulFindCount,        // number of entries to find
                       FIL_STANDARD);       // return level 1 file info

    if (arc == NO_ERROR)
    {
        // keep finding the next file until there are no more files
        while (arc == NO_ERROR)     // != ERROR_NO_MORE_FILES
        {
            if (ffb3.attrFile & FILE_DIRECTORY)
            {
                // we found a directory:

                // ignore the pseudo-directories
                if (    (strcmp(ffb3.achName, ".") != 0)
                     && (strcmp(ffb3.achName, "..") != 0)
                   )
                {
                    // real directory:
                    if (flFlags & DOSHDELDIR_RECURSE)
                    {
                        // recurse!
                        CHAR szSubDir[2*CCHMAXPATH];
                        sprintf(szSubDir, "%s\\%s", pcszDir, ffb3.achName);
                        arcReturn = doshDeleteDir(szSubDir,
                                                  flFlags,
                                                  pulDirs,
                                                  pulFiles);
                        // this removes ffb3.achName as well
                    }
                    else
                    {
                        // directory, but no recursion:
                        // report "access denied"
                        // (this is what OS/2 reports with DosDeleteDir as well)
                        arcReturn = ERROR_ACCESS_DENIED;  // 5
                    }

                    if (arcReturn != NO_ERROR)  // WarpIN V1.0.16 (2007-07-28) [pr]
                        if (pulDirs)  // WarpIN V1.0.15 (2007-03-27) [pr]
                            (*pulDirs)++;
                }
            }
            else
            {
                // it's a file:
                arcReturn = ERROR_ACCESS_DENIED;
                if (pulFiles)  // WarpIN V1.0.15 (2007-03-27) [pr]
                    (*pulFiles)++;
            }

            if (arc == NO_ERROR)
            {
                ulFindCount = 1;                      // reset find count
                arc = DosFindNext(hdirFindHandle,      // directory handle
                                  &ffb3,         // result buffer
                                  ulResultBufLen,      // result buffer length
                                  &ulFindCount);       // number of entries to find
            }
        } // endwhile

        DosFindClose(hdirFindHandle);    // close our find handle
    }

    if (arcReturn == NO_ERROR)
        // success so far:
        // delete our directory now
        arcReturn = DosDeleteDir((PSZ)pcszDir);

    return arcReturn;
}

/*
 *@@ doshCanonicalize:
 *      simplifies path specifications to remove '.'
 *      and '..' entries and generates a fully
 *      qualified path name where possible.
 *      File specifications are left unchanged.
 *
 *      This returns:
 *
 *      --  NO_ERROR: the buffers were valid.
 *
 *      --  ERROR_INVALID_PARAMETER: the buffers
 *          were invalid.
 *
 *@@added V0.9.19 (2002-04-22) [pr]
 */

APIRET doshCanonicalize(PCSZ pcszFileIn,        // in: path to canonicalize
                        PSZ pszFileOut,         // out: canonicalized path if NO_ERROR
                        ULONG cbFileOut)        // in: size of pszFileOut buffer
{
    APIRET ulrc = NO_ERROR;
    CHAR szFileTemp[CCHMAXPATH];

    if (pcszFileIn && pszFileOut && cbFileOut)
    {
        strncpy(szFileTemp, pcszFileIn, sizeof(szFileTemp) - 1);
        szFileTemp[sizeof(szFileTemp) - 1] = 0;
        if (    strchr(szFileTemp, '\\')
             || strchr(szFileTemp, ':')
           )
        {
            ULONG cbFileTemp = strlen(szFileTemp);

            if (    (cbFileTemp > 3)
                 && (szFileTemp[cbFileTemp - 1] == '\\')
               )
            {
                szFileTemp[cbFileTemp - 1] = 0;
            }

            if (DosQueryPathInfo(szFileTemp,
                                 FIL_QUERYFULLNAME,
                                 pszFileOut,
                                 cbFileOut))
            {
                pszFileOut[0] = 0;
            }
        }
        else
        {
            strncpy(pszFileOut, pcszFileIn, cbFileOut - 1);
            pszFileOut[cbFileOut - 1] = 0;
        }
    }
    else
        ulrc = ERROR_INVALID_PARAMETER;

    return(ulrc);
}

#define FINDBUFSIZE             0x10000     // 64K
#define FINDCOUNT               500

/*
 *@@ doshForAllFiles:
 *      this calles pfncb for all files in a directory matching
 *      the given file mask.
 *
 *      This is to avoid having to recode the typical but
 *      easy-to-get-wrong DosFindFirst/Next loop.
 *
 *      pfncb must be prototyped as follows:
 *
 +          APIRET XWPENTRY fnMyCallback(const FILEFINDBUF3 *pfb3,
 +                                       PVOID pvCallback)
 *
 *      On each iteration, it receives the current file-find
 *      buffer in pfb3. pvCallback is constantly set to what
 *      was passed in to this function.
 *
 *      The callback will get called for every file returned
 *      from the loop. This function will automatically
 *      filter out the stupid "." and ".." directory entries
 *      that DosFindFirst/Next always return, so the callback
 *      will never see those.
 *
 *      If the callback returns any value other than NO_ERROR,
 *      this function aborts and returns that error. The
 *      exception is that if the callback returns
 *      ERROR_NO_MORE_FILES, this function will abort also,
 *      but return NO_ERROR still. This is useful if you are
 *      looking for a specific file and want to cancel the
 *      search early without provoking an error.
 *
 *@@added V1.0.2 (2003-02-03) [umoeller]
 */

APIRET doshForAllFiles(PCSZ pcszSearchMask,         // in: search mask (e.g. "C:\dir\*.txt")
                       ULONG flFile,                // in: any of FILE_ARCHIVED | FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY | FILE_DIRECTORY
                       FNCBFORALLFILES *pfncb,      // in: address of callback function
                       PVOID pvCallback)            // in: parameter passed to callback
{
    APIRET  arc = NO_ERROR;
    HDIR    hdirFindHandle = HDIR_CREATE;
    ULONG   ulFindCount = FINDCOUNT;

    PBYTE   pbFindBuf;

    if (arc = DosAllocMem((PVOID*)&pbFindBuf,
                          FINDBUFSIZE,
                          PAG_COMMIT | PAG_READ | PAG_WRITE | OBJ_TILE))
        return arc;

    arc = DosFindFirst((PSZ)pcszSearchMask,
                       &hdirFindHandle,
                       flFile,
                       pbFindBuf,
                       FINDBUFSIZE,
                       &ulFindCount,
                       FIL_STANDARD);

    while (    (arc == NO_ERROR)
            || (arc == ERROR_BUFFER_OVERFLOW)
          )
    {
        ULONG           ul;
        PFILEFINDBUF3   pfb3 = (PFILEFINDBUF3)pbFindBuf;

        for (ul = 0;
             ul < ulFindCount;
             ul++)
        {
            // filter out the "." and ".." entries
            if (!(    (pfb3->attrFile & FILE_DIRECTORY)
                   && (pfb3->achName[0] == '.')
                   && (    (pfb3->achName[1] == '\0')
                        || (    (pfb3->achName[1] == '.')
                             && (pfb3->achName[2] == '\0')
                           )
                      )
               ))
            {
                // call callback
                if (arc = pfncb(pfb3, pvCallback))
                    // callback returned error:
                    break;
            }

            // next item in buffer
            if (pfb3->oNextEntryOffset)
                pfb3 = (PFILEFINDBUF3)(   (PBYTE)pfb3
                                        + pfb3->oNextEntryOffset
                                      );
        }

        if (!arc)
        {
            ulFindCount = FINDCOUNT;
            arc = DosFindNext(hdirFindHandle,
                              pbFindBuf,
                              FINDBUFSIZE,
                              &ulFindCount);
        }
    }

    // no more files is not an error
    if (arc == ERROR_NO_MORE_FILES)
        arc = NO_ERROR;

    DosFindClose(hdirFindHandle);

    DosFreeMem(pbFindBuf);

    return arc;
}

/*
 *@@category: Helpers\Control program helpers\Module handling
 *      helpers for importing functions from a module (DLL).
 */

/* ******************************************************************
 *
 *   Module handling helpers
 *
 ********************************************************************/

/*
 *@@ doshQueryProcAddr:
 *      attempts to resolve the given procedure from
 *      the given module name. Saves you from querying
 *      the module handle and all that.
 *
 *      This is intended for resolving undocumented
 *      APIs from OS/2 system DLls such as PMMERGE
 *      and DOSCALLS. It is assumed that the specified
 *      module is already loaded.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_INVALID_NAME
 *
 *      --  ERROR_INVALID_ORDINAL
 *
 *      plus the error codes of DosLoadModule.
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

APIRET doshQueryProcAddr(PCSZ pcszModuleName,       // in: module name (e.g. "PMMERGE")
                         ULONG ulOrdinal,           // in: proc ordinal
                         PFN *ppfn)                 // out: proc address
{
    HMODULE hmod;
    APIRET  arc;
    if (!(arc = DosQueryModuleHandle((PSZ)pcszModuleName,
                                     &hmod)))
    {
        if ((arc = DosQueryProcAddr(hmod,
                                    ulOrdinal,
                                    NULL,
                                    ppfn)))
        {
            // the CP programming guide and reference says use
            // DosLoadModule if DosQueryProcAddr fails with this error
            if (arc == ERROR_INVALID_HANDLE)
            {
                if (!(arc = DosLoadModule(NULL,
                                          0,
                                          (PSZ)pcszModuleName,
                                          &hmod)))
                {
                    arc = DosQueryProcAddr(hmod,
                                           ulOrdinal,
                                           NULL,
                                           ppfn);
                }
            }
        }
    }

    return arc;
}

/*
 *@@ doshResolveImports:
 *      this function loads the module called pszModuleName
 *      and resolves imports dynamically using DosQueryProcAddress.
 *
 *      To specify the functions to be imported, a RESOLVEFUNCTION
 *      array is used. In each of the array items, specify the
 *      name of the function and a pointer to a function pointer
 *      where to store the resolved address.
 *
 *@@added V0.9.3 (2000-04-29) [umoeller]
 */

APIRET doshResolveImports(PCSZ pcszModuleName,    // in: DLL to load
                          HMODULE *phmod,       // out: module handle
                          PCRESOLVEFUNCTION paResolves, // in/out: function resolves
                          ULONG cResolves)      // in: array item count (not array size!)
{
    CHAR    szName[CCHMAXPATH];
    APIRET  arc;

    if (!(arc = DosLoadModule(szName,
                              sizeof(szName),
                              (PSZ)pcszModuleName,
                              phmod)))
    {
        ULONG  ul;
        for (ul = 0;
             ul < cResolves;
             ul++)
        {
            if (arc = DosQueryProcAddr(*phmod,
                                       0,               // ordinal, ignored
                                       (PSZ)paResolves[ul].pcszFunctionName,
                                       paResolves[ul].ppFuncAddress))
                // error:
                break;
        }

        if (arc)
            // V0.9.16 (2001-12-08) [umoeller]
            DosFreeModule(*phmod);
    }

    return arc;
}

/*
 *@@category: Helpers\Control program helpers\Performance (CPU load) helpers
 *      helpers around DosPerfSysCall.
 */

/* ******************************************************************
 *
 *   Performance Counters (CPU Load)
 *
 ********************************************************************/

/*
 *@@ doshPerfOpen:
 *      initializes the OS/2 DosPerfSysCall API for
 *      the calling thread.
 *
 *      Note: This API is not supported on all OS/2
 *      versions. I believe it came up with some Warp 4
 *      fixpak. The API is resolved dynamically by
 *      this function (using DosQueryProcAddr).
 *
 *      This properly initializes the internal counters
 *      which the OS/2 kernel uses for this API. Apparently,
 *      with newer kernels (FP13/14), IBM has chosen to no
 *      longer do this automatically, which is the reason
 *      why many "pulse" utilities display garbage with these
 *      fixpaks.
 *
 *      After NO_ERROR is returned, DOSHPERFSYS.cProcessors
 *      contains the no. of processors found on the system.
 *      All pointers in DOSHPERFSYS then point to arrays
 *      which have exactly cProcessors array items.
 *
 *      So after NO_ERROR was returned here, you can keep
 *      calling doshPerfGet to get a current snapshot of the
 *      IRQ and user loads for each CPU. Note that interrupts
 *      are only processed on CPU 0 on SMP systems.
 *
 *      Call doshPerfClose to clean up resources allocated
 *      by this function.
 *
 *      For more sample code, take a look at the "Pulse" widget
 *      in the XWorkplace sources (src\xcenter\w_pulse.c).
 *
 *      Example code:
 *
 +      PDOSHPERFSYS pPerf = NULL;
 +      APIRET arc;
 +      if (!(arc = doshPerfOpen(&pPerf)))
 +      {
 +          // this should really be in a timer,
 +          // e.g. once per second
 +          ULONG   ulCPU;
 +          if (!(arc = doshPerfGet(&pPerf)))
 +          {
 +              // go thru all CPUs
 +              for (ulCPU = 0; ulCPU < pPerf->cProcessors; ulCPU++)
 +              {
 +                  LONG lUserLoadThis = pPerf->palLoads[ulCPU];
 +                  ...
 +              }
 +          }
 +
 +          // clean up
 +          doshPerfClose(&pPerf);
 +      }
 +
 *
 *@@added V0.9.7 (2000-12-02) [umoeller]
 *@@changed V0.9.9 (2001-03-14) [umoeller]: added interrupt loads; thanks phaller
 */

APIRET doshPerfOpen(PDOSHPERFSYS *ppPerfSys)  // out: new DOSHPERFSYS structure
{
    APIRET  arc = NO_ERROR;

    // allocate DOSHPERFSYS structure
    if (!(*ppPerfSys = (PDOSHPERFSYS)malloc(sizeof(DOSHPERFSYS))))
        arc = ERROR_NOT_ENOUGH_MEMORY;
    else
    {
        // initialize structure
        PDOSHPERFSYS pPerfSys = *ppPerfSys;
        memset(pPerfSys, 0, sizeof(*pPerfSys));

        // resolve DosPerfSysCall API entry
        if (!(arc = DosLoadModule(NULL,
                                  0,
                                  "DOSCALLS",
                                  &pPerfSys->hmod)))
        {
            if (!(arc = DosQueryProcAddr(pPerfSys->hmod,
                                         976,
                                         "DosPerfSysCall",
                                         (PFN*)&pPerfSys->pDosPerfSysCall)))
            {
                // OK, we got the API: initialize!
                if (!(arc = pPerfSys->pDosPerfSysCall(CMD_KI_ENABLE,
                                                      0,
                                                      0,
                                                      0)))
                {
                    pPerfSys->fInitialized = TRUE;
                            // call CMD_KI_DISABLE later

                    if (!(arc = pPerfSys->pDosPerfSysCall(CMD_PERF_INFO,
                                                          0,
                                                          (ULONG)&pPerfSys->cProcessors,
                                                          0)))
                    {
                        ULONG   ul = 0,
                                cProcs = pPerfSys->cProcessors,
                                cbDouble = cProcs * sizeof(double),
                                cbLong = cProcs * sizeof(LONG);

                        // allocate arrays
                        if (    (!(pPerfSys->paCPUUtils = (PCPUUTIL)calloc(cProcs,
                                                                sizeof(CPUUTIL))))
                             || (!(pPerfSys->padBusyPrev
                                    = (double*)malloc(cbDouble)))
                             || (!(pPerfSys->padTimePrev
                                    = (double*)malloc(cbDouble)))
                             || (!(pPerfSys->padIntrPrev
                                    = (double*)malloc(cbDouble)))
                             || (!(pPerfSys->palLoads
                                    = (PLONG)malloc(cbLong)))
                             || (!(pPerfSys->palIntrs
                                    = (PLONG)malloc(cbLong)))
                           )
                            arc = ERROR_NOT_ENOUGH_MEMORY;
                        else
                        {
                            for (ul = 0; ul < cProcs; ul++)
                            {
                                pPerfSys->padBusyPrev[ul] = 0.0;
                                pPerfSys->padTimePrev[ul] = 0.0;
                                pPerfSys->padIntrPrev[ul] = 0.0;
                                pPerfSys->palLoads[ul] = 0;
                                pPerfSys->palIntrs[ul] = 0;
                            }
                        }
                    }
                }
            } // end if (arc == NO_ERROR)
        } // end if (arc == NO_ERROR)

        if (arc)
            // error: clean up
            doshPerfClose(ppPerfSys);

    } // end else if (!*ppPerfSys)

    return arc;
}

/*
 *@@ doshPerfGet:
 *      calculates a current snapshot of the system load,
 *      compared with the load which was calculated on
 *      the previous call.
 *
 *      If you want to continually measure the system CPU
 *      load, this is the function you will want to call
 *      regularly -- e.g. with a timer once per second.
 *
 *      Call this ONLY if doshPerfOpen returned NO_ERROR,
 *      or you'll get crashes.
 *
 *      If this call returns NO_ERROR, you get LONG load
 *      values for each CPU in the system in the arrays
 *      in DOSHPERFSYS (in per-mille, 0-1000).
 *
 *      There are two arrays:
 *
 *      -- DOSHPERFSYS.palLoads contains the "user" load
 *         for each CPU.
 *
 *      -- DOSHPERFSYS.palIntrs contains the "IRQ" load
 *         for each CPU.
 *
 *      Sum up the two values to get the total load for
 *      each CPU.
 *
 *      For example, if there are two CPUs, after this call,
 *
 *      -- DOSHPERFSYS.palLoads[0] contains the "user" load
 *         of the first CPU,
 *
 *      -- DOSHPERFSYS.palLoads[0] contains the "user" load
 *         of the second CPU.
 *
 *      See doshPerfOpen for example code.
 *
 *@@added V0.9.7 (2000-12-02) [umoeller]
 *@@changed V0.9.9 (2001-03-14) [umoeller]: added interrupt loads; thanks phaller
 */

APIRET doshPerfGet(PDOSHPERFSYS pPerfSys)
{
    APIRET arc = NO_ERROR;
    if (!pPerfSys->pDosPerfSysCall)
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        if (!(arc = pPerfSys->pDosPerfSysCall(CMD_KI_RDCNT,
                                              (ULONG)pPerfSys->paCPUUtils,
                                              0, 0)))
        {
            // go thru all processors
            ULONG ul = 0;
            for (; ul < pPerfSys->cProcessors; ul++)
            {
                PCPUUTIL    pCPUUtilThis = &pPerfSys->paCPUUtils[ul];

                double      dTime = LL2F(pCPUUtilThis->ulTimeHigh,
                                         pCPUUtilThis->ulTimeLow);
                double      dBusy = LL2F(pCPUUtilThis->ulBusyHigh,
                                         pCPUUtilThis->ulBusyLow);
                double      dIntr = LL2F(pCPUUtilThis->ulIntrHigh,
                                         pCPUUtilThis->ulIntrLow);

                double      *pdBusyPrevThis = &pPerfSys->padBusyPrev[ul];
                double      *pdTimePrevThis = &pPerfSys->padTimePrev[ul];
                double      *pdIntrPrevThis = &pPerfSys->padIntrPrev[ul];

                // avoid division by zero
                double      dTimeDelta;
                if (dTimeDelta = (dTime - *pdTimePrevThis))
                {
                    pPerfSys->palLoads[ul]
                        = (LONG)( (double)(   (dBusy - *pdBusyPrevThis)
                                            / dTimeDelta
                                            * 1000.0
                                          )
                                );
                    pPerfSys->palIntrs[ul]
                        = (LONG)( (double)(   (dIntr - *pdIntrPrevThis)
                                            / dTimeDelta
                                            * 1000.0
                                          )
                                );
                }
                else
                {
                    // no clear readings are available
                    pPerfSys->palLoads[ul] = 0;
                    pPerfSys->palIntrs[ul] = 0;
                }

                *pdTimePrevThis = dTime;
                *pdBusyPrevThis = dBusy;
                *pdIntrPrevThis = dIntr;
            }
        }
    }

    return arc;
}

/*
 *@@ doshPerfClose:
 *      frees all resources allocated by doshPerfOpen.
 *
 *@@added V0.9.7 (2000-12-02) [umoeller]
 *@@changed V0.9.9 (2001-02-06) [umoeller]: removed disable; this broke the WarpCenter
 *@@changed V0.9.9 (2001-03-14) [umoeller]: fixed memory leak
 *@@changed V0.9.9 (2001-03-14) [umoeller]: added interrupt loads; thanks phaller
 */

APIRET doshPerfClose(PDOSHPERFSYS *ppPerfSys)
{
    APIRET arc = NO_ERROR;
    PDOSHPERFSYS pPerfSys;
    if (    (!ppPerfSys)
         || (!(pPerfSys = *ppPerfSys))
       )
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        // do not call this, this messes up the WarpCenter V0.9.9 (2001-02-06) [umoeller]
        // if (pPerfSys->fInitialized) pPerfSys->pDosPerfSysCall(CMD_KI_DISABLE, 0, 0, 0);

        if (pPerfSys->paCPUUtils)
            free(pPerfSys->paCPUUtils);
        if (pPerfSys->padBusyPrev)
            free(pPerfSys->padBusyPrev);
        if (pPerfSys->padTimePrev)
            free(pPerfSys->padTimePrev);
        if (pPerfSys->padIntrPrev)
            free(pPerfSys->padIntrPrev);
        if (pPerfSys->palLoads)             // was missing V0.9.9 (2001-03-14) [umoeller]
            free(pPerfSys->palLoads);
        if (pPerfSys->palIntrs)
            free(pPerfSys->palIntrs);

        if (pPerfSys->hmod)
            DosFreeModule(pPerfSys->hmod);
        free(pPerfSys);
        *ppPerfSys = NULL;
    }

    return arc;
}

/*
 *@@category: Helpers\Control program helpers\Process management
 *      helpers for starting subprocesses.
 */

/* ******************************************************************
 *
 *   Process helpers
 *
 ********************************************************************/

STATIC PVOID    G_pvLocalInfoSeg = NULL;


extern "C" APIRET16 APIENTRY16 DOS16GETINFOSEG(PSEL pselGlobal,
                                      PSEL pselLocal);

#define Dos16GetInfoSeg DOS16GETINFOSEG
//USHORT _Far16 _Pascal Dos16GetInfoSeg(PSEL pselGlobal,
//                                      PSEL pselLocal);
/*
 * GetInfoSegs:
 *
 */

STATIC VOID GetInfoSegs(VOID)
{
    SEL     GlobalInfoSegSelector,
            LocalInfoSegSelector;

    // get selectors (old 16-bit API; this gets called only once)
    Dos16GetInfoSeg(&GlobalInfoSegSelector,
                    &LocalInfoSegSelector);
    // thunk
    /* G_pvGlobalInfoSeg = (PVOID)(   (GlobalInfoSegSelector << 0x000D)
                                 & 0x01fff0000); */
    G_pvLocalInfoSeg  = (PVOID)(   (LocalInfoSegSelector << 0x000D)
                                 & 0x01fff0000);
}

/*
 *@@ doshMyPID:
 *      returns the PID of the current process.
 *
 *      This uses an interesting hack which is way
 *      faster than DosGetInfoBlocks.
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 */

ULONG doshMyPID(VOID)
{
    if (!G_pvLocalInfoSeg)
        // first call:
        GetInfoSegs();

    // PID is at offset 0 in the local info seg
    return *(PUSHORT)G_pvLocalInfoSeg;
}

/*
 *@@ doshMyParentPID:
 *      returns the PID of the parent of the current process.
 *
 *      This uses an interesting hack which is way
 *      faster than DosGetInfoBlocks.
 *
 *@@added V1.0.2 (2003-11-13) [umoeller]
 */

ULONG doshMyParentPID(VOID)
{
    if (!G_pvLocalInfoSeg)
        // first call:
        GetInfoSegs();

    // parent PID is at offset 2 in the local info seg
    return *(PUSHORT)((PBYTE)G_pvLocalInfoSeg + 2);
}

/*
 *@@ doshMyTID:
 *      returns the TID of the current thread.
 *
 *      This uses an interesting hack which is way
 *      faster than DosGetInfoBlocks.
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 */

ULONG doshMyTID(VOID)
{
    if (!G_pvLocalInfoSeg)
        // first call:
        GetInfoSegs();

    // TID is at offset 6 in the local info seg
    return *(PUSHORT)((PBYTE)G_pvLocalInfoSeg + 6);
}

/*
 *@@ doshExecVIO:
 *      executes cmd.exe with the /c parameter
 *      and pcszExecWithArgs. This is equivalent
 *      to the C library system() function,
 *      except that an OS/2 error code is returned
 *      and that this works with the VAC subsystem
 *      library.
 *
 *      This uses DosExecPgm and handles the sick
 *      argument passing.
 *
 *      If NO_ERROR is returned, *plExitCode receives
 *      the exit code of the process. If the process
 *      was terminated abnormally, *plExitCode receives:
 *
 *      -- -1: hard error halt
 *      -- -2: 16-bit trap
 *      -- -3: DosKillProcess
 *      -- -4: 32-bit exception
 *
 *@@added V0.9.4 (2000-07-27) [umoeller]
 */

APIRET doshExecVIO(PCSZ pcszExecWithArgs,
                   PLONG plExitCode)            // out: exit code (ptr can be NULL)
{
    APIRET arc = NO_ERROR;

    if (strlen(pcszExecWithArgs) > 255)
        arc = ERROR_INSUFFICIENT_BUFFER;
    else
    {
        CHAR szObj[CCHMAXPATH];
        RESULTCODES res = {0};
        CHAR szBuffer[300];

        // DosExecPgm expects two args in szBuffer:
        // --  cmd.exe\0
        // --  then the actual argument, terminated by two 0 bytes.
        memset(szBuffer, 0, sizeof(szBuffer));
        strcpy(szBuffer, "cmd.exe\0");
        sprintf(szBuffer + 8, "/c %s",
                pcszExecWithArgs);

        arc = DosExecPgm(szObj, sizeof(szObj),
                         EXEC_SYNC,
                         szBuffer,
                         0,
                         &res,
                         "cmd.exe");
        if ((arc == NO_ERROR) && (plExitCode))
        {
            if (res.codeTerminate == 0)
                // normal exit:
                *plExitCode = res.codeResult;
            else
                *plExitCode = -((LONG)res.codeTerminate);
        }
    }

    return arc;
}

/*
 *@@ doshQuickStartSession:
 *      this is a shortcut to DosStartSession w/out having to
 *      deal with all the messy parameters.
 *
 *      This one starts pszPath as a child session and passes
 *      pszParams to it.
 *
 *      In usPgmCtl, OR any or none of the following (V0.9.0):
 *      --  SSF_CONTROL_NOAUTOCLOSE (0x0008): do not close automatically.
 *              This bit is used only for VIO Windowable apps and ignored
 *              for all other applications.
 *      --  SSF_CONTROL_MINIMIZE (0x0004)
 *      --  SSF_CONTROL_MAXIMIZE (0x0002)
 *      --  SSF_CONTROL_INVISIBLE (0x0001)
 *      --  SSF_CONTROL_VISIBLE (0x0000)
 *
 *      Specifying 0 will therefore auto-close the session and make it
 *      visible.
 *
 *      If (fWait), this function will create a termination queue
 *      and not return until the child session has ended. Be warned,
 *      this blocks the calling thread.
 *
 *      Otherwise the function will return immediately.
 *
 *      The session and process IDs of the child session will be
 *      written to *pulSID and *ppid. Of course, in "wait" mode,
 *      these are no longer valid after this function returns.
 *
 *      Returns the error code of DosStartSession.
 *
 *      Note: According to CPREF, calling DosStartSession calls
 *      DosExecPgm in turn for all full-screen, VIO, and PM sessions.
 *
 *@@changed V0.9.0 [umoeller]: prototype changed to include usPgmCtl
 *@@changed V0.9.1 (99-12-30) [umoeller]: queue was sometimes not closed. Fixed.
 *@@changed V0.9.3 (2000-05-03) [umoeller]: added fForeground
 *@@changed V0.9.14 (2001-08-03) [umoeller]: fixed potential queue leak
 *@@changed V0.9.14 (2001-08-03) [umoeller]: fixed memory leak in wait mode; added pusReturn to prototype
 *@@changed V1.0.2  (2003-02-04) [jsmall  ]: inserted new 3rd parameter for session type
 *@@changed V1.0.2  (2003-02-04) [jsmall  ]: changed code so when fWait = FALSE then session is started as independent, not as a child
 *@@changed V1.0.5 (2006-04-01) [pr]: Added pcszTitle parameter
 */

APIRET doshQuickStartSession(PCSZ pcszPath,         // in: program to start
                             PCSZ pcszParams,       // in: parameters for program
                             PCSZ pcszTitle,        // in: window title
                             USHORT usSessionType,  // in: session type
                             BOOL fForeground,      // in: if TRUE, session will be in foreground
                             USHORT usPgmCtl,       // in: STARTDATA.PgmControl
                             BOOL fWait,            // in: wait for termination?
                             PULONG pulSID,         // out: session ID (req.)
                             PPID ppid,             // out: process ID (req.)
                             PUSHORT pusReturn)     // out: in wait mode, session's return code (ptr can be NULL)
{
    APIRET      arc = NO_ERROR;
    // queue stuff
    const char  *pcszQueueName = "\\queues\\xwphlpsw.que";
    HQUEUE      hq = 0;
    PID         qpid = 0;

    if (fWait)
    {
        if (!(arc = DosCreateQueue(&hq,
                                   QUE_FIFO | QUE_CONVERT_ADDRESS,
                                   (PSZ)pcszQueueName)))
            arc = DosOpenQueue(&qpid, &hq, (PSZ)pcszQueueName);
    }

    if (!arc)    // V0.9.14 (2001-08-03) [umoeller]
    {
        STARTDATA   SData;
        CHAR        szObjBuf[CCHMAXPATH];

        SData.Length  = sizeof(STARTDATA);
//      SData.Related = SSF_RELATED_CHILD; //INDEPENDENT;
        SData.Related = (fWait) ? SSF_RELATED_CHILD : SSF_RELATED_INDEPENDENT;
                // V1.0.2 (2003-02-04) [jsmall]
        SData.FgBg    = (fForeground) ? SSF_FGBG_FORE : SSF_FGBG_BACK;
                // V0.9.3 (2000-05-03) [umoeller]
        SData.TraceOpt = SSF_TRACEOPT_NONE;

        // V1.0.5 (2006-04-01) [pr]
        if (pcszTitle && *pcszTitle)
            SData.PgmTitle = (PSZ)pcszTitle;      // title for window
        else
            SData.PgmTitle = (PSZ)pcszPath;

        SData.PgmName = (PSZ)pcszPath;
        SData.PgmInputs = (PBYTE)pcszParams;

        SData.TermQ = (PBYTE)((fWait) ? (PSZ)pcszQueueName : NULL);
        SData.Environment = 0;
        SData.InheritOpt = SSF_INHERTOPT_PARENT;
//      SData.SessionType = SSF_TYPE_DEFAULT;
        SData.SessionType = usSessionType;    // V1.0.2 (2003-02-04)  [jsmall]
        SData.IconFile = 0;
        SData.PgmHandle = 0;

        SData.PgmControl = usPgmCtl;

        SData.InitXPos  = 30;
        SData.InitYPos  = 40;
        SData.InitXSize = 200;
        SData.InitYSize = 140;
        SData.Reserved = 0;
        SData.ObjectBuffer  = szObjBuf;
        SData.ObjectBuffLen = (ULONG)sizeof(szObjBuf);

        if (    (!(arc = DosStartSession(&SData, pulSID, ppid)))
             && (fWait)
           )
        {
            // block on the termination queue, which is written
            // to when the subprocess ends
            REQUESTDATA rqdata;
            ULONG       cbData = 0;
            PULONG      pulData = NULL;
            BYTE        elpri;

            rqdata.pid = qpid;
            if (!(arc = DosReadQueue(hq,                // in: queue handle
                                     &rqdata,           // out: pid and ulData
                                     &cbData,           // out: size of data returned
                                     (PVOID*)&pulData,  // out: data returned
                                     0,                 // in: remove first element in queue
                                     0,                 // in: wait for queue data (block thread)
                                     &elpri,            // out: element's priority
                                     0)))               // in: event semaphore to be posted
            {
                if (!rqdata.ulData)
                {
                    // child session ended:
                    // V0.9.14 (2001-08-03) [umoeller]

                    // *pulSID = (*pulData) & 0xffff;
                    if (pusReturn)
                        *pusReturn = ((*pulData) >> 16) & 0xffff;

                }
                // else: continue looping

                if (pulData)
                    DosFreeMem(pulData);
            }
        }
    }

    if (hq)
        DosCloseQueue(hq);

    return arc;
}


