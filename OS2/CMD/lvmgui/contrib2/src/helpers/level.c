
/*
 *@@sourcefile level.c:
 *      contains helper functions for querying SYSLEVEL files.
 *
 *      This is all new with V0.9.2.
 *
 *      Function prefixes:
 *      --  lvl*    syslevel helpers
 *
 *@@added V0.9.2 (2000-03-08) [umoeller]
 *@@header "helpers\level.h"
 */

/*
 *      Copyright (C) 1994 Martin Lafaix (EDM/2 2-05).
 *      Copyright (C) 2000 Ulrich M”ller.
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

#define INCL_DOSFILEMGR
#include <os2.h>

#include <stdlib.h>
#include <string.h>

#include "setup.h"

#define SYSLEVEL_PRIVATE
#include "helpers\level.h"

// redefine the FIELDOFFSET macro; the one
// in the OS/2 header files doesn't work with C++
#undef FIELDOFFSET
#define FIELDOFFSET(type, field)    ((ULONG)&(((type *)0)->field))

/*
 *@@category: Helpers\Control program helpers\SYSLEVEL parsing
 *      helpers for parsing SYSLEVEL files. See lvlOpenLevelFile.
 */

/* ******************************************************************
 *
 *   Interface
 *
 ********************************************************************/

/*
 *@@ lvlOpenLevelFile:
 *      finds, opens, or creates a SYSLEVEL file. *phFile
 *      receives the DosOpen file handle, which can then be
 *      passed to lvlQueryLevelFileData and such. Alternatively,
 *      you can use DosOpen directly.
 *
 *      ulOpenMode can be:
 *
 *      -- OLF_OPEN:
 *              open the specified file. The default behavior is to scan
 *              the current disk, starting from the current directory.
 *              It returns the first SYSLEVEL file whose extension
 *              matches pszName. You can override this by
 *              combining OLF_OPEN with OLF_SCANDISK or
 *              OLF_CHECKID.
 *
 *      -- OLF_SCANDISKS:
 *              scans all disks (starting from C:). Use this flag to
 *              modify the default OLF_OPEN behavior.
 *
 *      -- OLF_CHECKID:
 *              finds file(s) whose ID matches the specified one. Use
 *              this flag to override the default OLF_OPEN behavior.
 *
 *      -- OLF_CREATE:
 *              creates the specified file in the current directory. A
 *              valid pszName and ID should be provided.
 */

APIRET lvlOpenLevelFile(PSZ pszName,        // in: syslevel file name extension
                        PHFILE phFile,      // out: Dos file handle
                        ULONG ulOpenMode,   // in: OLF_* flags
                        PSZ pszCID)         // in: if OLF_CHECKID is specified, the ID
{
    APIRET rc = 0;
    ULONG ulAction;
    CHAR achFileName[CCHMAXPATHCOMP];

    switch (ulOpenMode)
    {
        case OLF_OPEN:
        case OLF_OPEN | OLF_CHECKID:
            if (pszName || pszCID)
            {
                rc = _findsyslevelfile(pszName, pszCID, achFileName);

                if (rc == 18)
                    rc = _searchsubdirs(pszName, pszCID, achFileName);

                if (rc == 0)
                    rc = DosOpen(achFileName,
                                 phFile,
                                 &ulAction,
                                 0,
                                 FILE_NORMAL,
                                 FILE_OPEN,
                                 OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                                 0);

                if (rc == 18)
                    rc = 2;
            }
            else
                rc = ERROR_INVALID_PARAMETER;
            break;

        case OLF_SCANDISKS:
        case OLF_SCANDISKS | OLF_CHECKID:
        case OLF_SCANDISKS | OLF_OPEN:
        case OLF_SCANDISKS | OLF_OPEN | OLF_CHECKID:
            if (pszName || pszCID)
            {
                ULONG ulDrive, ulDriveMap, ulCurrent, ulFound = 0;

                rc = DosQueryCurrentDisk(&ulDrive, &ulDriveMap);
                ulCurrent = _firstdrive(ulDriveMap);

                while (ulCurrent && !ulFound)
                {
                    rc = _findsyslevelfile(pszName, pszCID, achFileName);
                    if (rc == 18)
                        rc = _searchsubdirs(pszName, pszCID, achFileName);

                    if (rc == 0)
                    {
                        ulFound = 1;

                        rc = DosOpen(achFileName,
                                     phFile,
                                     &ulAction,
                                     0,
                                     FILE_NORMAL,
                                     FILE_OPEN,
                                     OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                                     0);
                    }
                    else
                        ulCurrent = _nextdrive(ulDriveMap, ulCurrent);
                }

                DosSetDefaultDisk(ulDrive);
                if (ulFound == 0)
                    rc = 2;
            }
            else
                rc = ERROR_INVALID_PARAMETER;
            break;

        case OLF_CREATE:
            if (pszName && pszCID)
            {
                SYSLEVELHEADER slh;
                SYSLEVELDATA sld;

                strcat(strcpy(achFileName, "syslevel."), pszName);

                rc = DosOpen(achFileName,
                             phFile,
                             &ulAction,
                             0,
                             FILE_NORMAL,
                             FILE_CREATE,
                             OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                             0);

                slh.h_magic[0] = slh.h_magic[1] = 0xFF;
                strcpy((PSZ)slh.h_name, "SYSLEVEL");
                for (ulAction = 0; ulAction < sizeof(slh.h_reserved1); ulAction++)
                    slh.h_reserved1[ulAction] = 0;
                slh.h_updated = 0;
                for (ulAction = 0; ulAction < sizeof(slh.h_reserved2); ulAction++)
                    slh.h_reserved2[ulAction] = 0;
                slh.h_data = sizeof(SYSLEVELHEADER);

                if (rc == 0)
                    rc = _writesyslevelheader(*phFile, &slh);

                memset(&sld, 0, sizeof(sld));
                sld.d_kind[0] = SLK_STANDARD;
                sld.d_reserved3 = sld.d_reserved4 = 0x5F;
                memcpy(sld.d_cid, pszCID, sizeof(sld.d_cid));

                if (rc == 0)
                    rc = DosWrite(*phFile, &sld, sizeof(SYSLEVELDATA), &ulAction);
            }
            else
                rc = ERROR_INVALID_PARAMETER;
            break;
        default:
            rc = ERROR_INVALID_PARAMETER;
            break;
    }

    return rc;
}

/*
 *@@ lvlQueryLevelFile:
 *      finds all SYSLEVEL files on the system and
 *      writes them to pBuffer.
 */

APIRET lvlQueryLevelFile(PSZ pszName,
                         PSZ pszCID,
                         PVOID pBuffer,
                         ULONG ulBufSize)
{
    ULONG ulDrive, ulDriveMap, ulCurrent;
    CHAR achFileName[CCHMAXPATHCOMP];
    APIRET rc = DosQueryCurrentDisk(&ulDrive, &ulDriveMap);

    strcat(strcpy(achFileName, "syslevel."),
           (pszName == NULL) ? (PSZ) "???" : pszName);
    memset(pBuffer, 0, ulBufSize);

    ulCurrent = _firstdrive(ulDriveMap);

    while (ulCurrent && (rc == 0))
    {
        rc = _findallsyslevelfiles(achFileName, pszCID, &pBuffer, &ulBufSize);

        if (rc == 0)
            ulCurrent = _nextdrive(ulDriveMap, ulCurrent);
    }

    if (rc == 0)
        rc = DosSetDefaultDisk(ulDrive);

    return rc;
}

/*
 *@@ lvlQueryLevelFileData:
 *      parses syslevel data from a SYSLEVEL file previously
 *      opened with lvlOpenLevelFile. Alternatively, you can
 *      open the SYSLEVEL file directly using DosOpen.
 *
 *      ulWhat can be:
 *
 *      -- QLD_MAJORVERSION
 *              Query (or update) the major version field. It's a
 *              one-character field. It should be in the range '0'-'9'.
 *              The value is placed in (or taken from) the first
 *              character of pBuffer. (Buffer size should be at least 1.)
 *
 *      -- QLD_MINORVERSION
 *              Query (or update) the minor version field. It's a
 *              two-character field. It should be in range '00'-'99'. The
 *              value is placed in (or taken from) the first two chars
 *              of pBuffer. (Buffer size should be at least 2.)
 *
 *      -- QLD_REVISION
 *              Query (or update) the revision field. It's should fit in
 *              a character. If it's '0', there's no revision available. It
 *              can be a letter as well as a digit. The value is placed
 *              in (or taken from) the first character of pBuffer.
 *              (Buffer size should be at least 1.)
 *
 *      -- QLD_KIND
 *              Query (or update) the kind field. The value is placed
 *              in (or taken from) the first character of *pBuffer.
 *              (Buffer size should be at least 1.)
 *
 *      -- QLD_CURRENTCSD
 *              Query (or update) the current CSD level (when you
 *              update this field, its old value is copied to the old
 *              CSD level field). It's a seven-character field, and it
 *              does not have to be null-terminated. The value is
 *              placed in (or taken from) the first seven characters of
 *              pBuffer. (Buffer size should be at least 7.)
 *
 *      -- QLD_PREVIOUSCSD
 *              Query the previous CSD level. You can't update this
 *              field. The value is placed in the first seven chars of
 *              pBuffer. (Buffer size should be at least 7.)
 *              Note: CSD levels are not null-terminated. Be careful
 *              when using such a returned value.
 *
 *      -- QLD_TITLE
 *              Query (or update) the component title field. It's an
 *              eighty-character string (required ending null
 *              included). The value is placed in (or taken from) the
 *              first eighty characters of pBuffer. On input, the buffer
 *              size should be at least 80. On output, the buffer size
 *              can exceed 80, but the written string is truncated
 *              (and null-terminated) to eighty characters.
 *
 *      -- QLD_ID
 *              Query (or update) the component ID field. It's a
 *              nine-character field. It does not have to be
 *              null-terminated. The value is placed in (or taken
 *              from) the first nine characters of pBuffer. (Buffer size
 *              should be at least 9.)
 *              Note: IDs are not null-terminated. Be careful when
 *              using such a returned value.
 *
 *      -- QLD_TYPE
 *              Query (or update) the component type field.
 */

APIRET lvlQueryLevelFileData(HFILE hFile,       // in: file handle returned by lvlOpenLevelFile
                             ULONG ulWhat,      // in: QLD* flags
                             PVOID pBuffer,     // in/out: data
                             ULONG ulBufSize,   // in: sizeof(*pBuffer)
                             PULONG pulSize)    // out: size required if
                                                // ERROR_INSUFFICIENT_BUFFER is returned;
                                                // otherwise bytes written
{
    APIRET rc = _locatesysleveldata(hFile);
    PSYSLEVELDATA psld = _allocsysleveldata(hFile);

    if (rc == 0)
        rc = pBuffer ? rc : ERROR_INVALID_PARAMETER;
    if (rc == 0)
        rc = _readsysleveldata(hFile, psld);

    if (rc == 0)
        switch (ulWhat)
        {
            case QLD_MAJORVERSION:
                if (ulBufSize >= 1)
                    _achBuffer[0] = '0' + (psld->d_version[0] >> 4);
                else
                    rc = ERROR_INSUFFICIENT_BUFFER;
                _setsize(1);
                break;
            case QLD_MINORVERSION:
                if (ulBufSize >= 2)
                {
                    _achBuffer[0] = '0' + (psld->d_version[0] & 0x0F);
                    _achBuffer[1] = '0' + psld->d_version[1];
                }
                else
                    rc = ERROR_INSUFFICIENT_BUFFER;
                _setsize(2);
                break;
            case QLD_REVISION:
                if (ulBufSize >= 1)
                    _achBuffer[0] = '0' + psld->d_revision[0];
                else
                    rc = ERROR_INSUFFICIENT_BUFFER;
                _setsize(1);
                break;
            case QLD_KIND:
                _getfield(d_kind);
                break;
            case QLD_CURRENTCSD:
                _getfield(d_clevel);
                break;
            case QLD_PREVIOUSCSD:
                _getfield(d_plevel);
                break;
            case QLD_TITLE:
                _getfield(d_title);
                break;
            case QLD_ID:
                _getfield(d_cid);
                break;
            case QLD_TYPE:
                if (ulBufSize >= strlen((PSZ)psld->d_type))
                    memcpy(pBuffer, psld->d_type, strlen((PSZ)psld->d_type));
                else
                    rc = ERROR_INSUFFICIENT_BUFFER;
                _setsize(strlen((PSZ)psld->d_type));
                break;
            default:
                rc = ERROR_INVALID_PARAMETER;
                break;
        }

    _freesysleveldata(psld);

    return rc;
}

/*
 *@@ lvlWriteLevelFileData:
 *      writes data to a SYSLEVEL file.
 */

APIRET lvlWriteLevelFileData(HFILE hFile,
                             ULONG ulWhat,
                             PVOID pBuffer,
                             ULONG ulBufSize,
                             PULONG pulSize)
{
    APIRET rc = _locatesysleveldata(hFile);
    PSYSLEVELDATA psld = _allocsysleveldata(hFile);

    if (rc == 0)
        rc = pBuffer ? rc : ERROR_INVALID_PARAMETER;
    if (rc == 0)
        rc = _readsysleveldata(hFile, psld);

    if (rc == 0)
        switch (ulWhat)
        {
            case QLD_MAJORVERSION:
                if (ulBufSize >= 1)
                {
                    psld->d_version[0] &= 0x0F;
                    psld->d_version[0] |= (_achBuffer[0] - '0') << 4;
                }
                else
                    rc = ERROR_INSUFFICIENT_BUFFER;
                _setsize(1);
                break;
            case QLD_MINORVERSION:
                if (ulBufSize >= 2)
                {
                    psld->d_version[0] &= 0xF0;
                    psld->d_version[0] |= (_achBuffer[0] - '0') && 0x0F;
                    psld->d_version[1] = _achBuffer[1] - '0';
                }
                else
                    rc = ERROR_INSUFFICIENT_BUFFER;
                _setsize(2);
                break;
            case QLD_REVISION:
                _setfield(d_revision);
                break;
            case QLD_KIND:
                _setfield(d_kind);
                break;
            case QLD_CURRENTCSD:
                memcpy(psld->d_plevel, psld->d_clevel, sizeof(psld->d_plevel));
                _setfield(d_clevel);
                break;
            case QLD_TITLE:
                memcpy(psld->d_title, pBuffer, min(ulBufSize, sizeof(psld->d_title)));
                _setsize(min(ulBufSize, sizeof(psld->d_title)));
                break;
            case QLD_ID:
                _setfield(d_cid);
                break;
            case QLD_TYPE:
                break;
            default:
                rc = ERROR_INVALID_PARAMETER;
                break;
        }

    if (rc == 0)
        rc = _locatesysleveldata(hFile);
    if (rc == 0)
        rc = _writesysleveldata(hFile, psld);

    _freesysleveldata(psld);

    return rc;
}

/* ******************************************************************
 *
 *   Local helper functions
 *
 ********************************************************************/

/*
 * _readsysleveldata:
 *
 */

STATIC APIRET _readsysleveldata(HFILE hFile,
                                PSYSLEVELDATA psld)
{
    ULONG ulSize;

    return DosRead(hFile, psld, sizeof(SYSLEVELDATA), &ulSize);
}

/*
 * _writesysleveldata:
 *
 */

STATIC APIRET _writesysleveldata(HFILE hFile,
                                 PSYSLEVELDATA psld)
{
    ULONG ulSize;

    return DosWrite(hFile, psld, sizeof(SYSLEVELDATA), &ulSize);
}

/*
 * _locatesysleveldata:
 *
 */

STATIC APIRET _locatesysleveldata(HFILE hFile)
{
    ULONG ulPos, ulData;
    APIRET rc = DosSetFilePtr(hFile,
                              FIELDOFFSET(SYSLEVELHEADER, h_data),
                              FILE_BEGIN,
                              &ulPos);

    rc = rc ? rc : DosRead(hFile, &ulData, sizeof(ulData), &ulPos);

    return rc ? rc : DosSetFilePtr(hFile, ulData, FILE_BEGIN, &ulPos);
}

/*
 * _allocsysleveldata:
 *
 */

STATIC PSYSLEVELDATA _allocsysleveldata(HFILE hFile)
{
    return (PSYSLEVELDATA)malloc(sizeof(SYSLEVELDATA));
}

/*
 * _freesysleveldata:
 *
 */

STATIC VOID _freesysleveldata(PSYSLEVELDATA psld)
{
    if (psld)
        free(psld);
}

/*
 * _writesyslevelheader:
 *
 */

STATIC APIRET _writesyslevelheader(HFILE hFile,
                                   PSYSLEVELHEADER pslh)
{
    ULONG ulSize;

    return DosWrite(hFile, pslh, sizeof(SYSLEVELHEADER), &ulSize);
}

/*
 * _isasyslevelfile:
 *
 */

STATIC BOOL _isasyslevelfile(PSZ pszFile,
                             PSZ pszCID)
{
    HFILE hFile;
    ULONG ulAction;
    SYSLEVELHEADER slh;
    APIRET rc = DosOpen(pszFile,
                        &hFile,
                        &ulAction,
                        0,
                        FILE_NORMAL,
                        FILE_OPEN,
                        OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                        0);

    if (rc == 0)
        rc = DosRead(hFile, &slh, sizeof(slh), &ulAction);

    ulAction = (slh.h_magic[0] == 0xFF) && (slh.h_magic[1] == 0xFF);
    if (ulAction)
        ulAction = strcmp((PSZ)slh.h_name, "SYSLEVEL") == 0;
    if (ulAction)
        if (pszCID)
        {
            CHAR achCID[9];
            ULONG ulCID;

            rc = lvlQueryLevelFileData(hFile, QLD_ID, achCID, 9, &ulCID);
            ulAction = (strcmp(achCID, pszCID) == 0);
        }

    rc = DosClose(hFile);

    return ulAction;
}

/*
 * _getfullname:
 *
 */

STATIC APIRET _getfullname(PVOID pBuffer,
                           PSZ pszFileName)
{
    ULONG ulDrive, ulDriveMap, ulBufSize = CCHMAXPATHCOMP;
    APIRET rc = DosQueryCurrentDisk(&ulDrive, &ulDriveMap);

    _achBuffer[0] = '@' + ulDrive;
    _achBuffer[1] = ':';
    _achBuffer[2] = '\\';
    rc = DosQueryCurrentDir(0, _achBuffer + 3, &ulBufSize);
    strcat(strcat((PSZ)pBuffer, "\\"), pszFileName);

    return rc;
}

/*
 * _findsyslevelfile:
 *
 */

STATIC APIRET _findsyslevelfile(PSZ pszName,
                                PSZ pszCID,
                                PVOID pBuffer)
{
    HDIR hDirHandle = HDIR_CREATE;
    FILEFINDBUF3 ffb;
    ULONG ulCount = 1, ulFound = 0;
    CHAR achFileName[CCHMAXPATHCOMP] = "syslevel.";
    APIRET rc = 0;

    if (pszName)
        strcat(achFileName, pszName);
    else
        strcat(achFileName, "???");

    rc = DosFindFirst(achFileName,
                      &hDirHandle,
                      FILE_NORMAL,
                      (PVOID) & ffb,
                      sizeof(ffb),
                      &ulCount,
                      FIL_STANDARD);

    while ((!ulFound) && (rc == 0))
    {
        if (_isasyslevelfile(ffb.achName, pszCID))
        {
            _getfullname(pBuffer, ffb.achName);
            ulFound = 1;
            break;
        }

        rc = DosFindNext(hDirHandle, (PVOID) & ffb, sizeof(ffb), &ulCount);
    }

    if ((rc == 0) || (rc == 18))
        rc = DosFindClose(hDirHandle);

    return ulFound ? 0 : 18;
}

/*
 * _findallsyslevelfiles:
 *
 */

STATIC APIRET _findallsyslevelfiles(PSZ achFileName,
                                    PSZ pszCID,
                                    PPVOID ppBuffer,
                                    PULONG pulBufSize)
{
    HDIR hDirHandle = HDIR_CREATE;
    FILEFINDBUF3 ffb;
    ULONG ulCount = 1, ulSize;
    APIRET rc = DosFindFirst(achFileName,
                             &hDirHandle,
                             FILE_NORMAL,
                             (PVOID) & ffb,
                             sizeof(ffb),
                             &ulCount,
                             FIL_STANDARD);

    while (rc == 0)
    {
        if (_isasyslevelfile(ffb.achName, pszCID))
        {
            CHAR achBuf[CCHMAXPATHCOMP];

            _getfullname(achBuf, ffb.achName);
            ulSize = strlen(achBuf);
            if ((*pulBufSize) > ulSize)
            {
                *pulBufSize -= ulSize + 1;
                memcpy(*ppBuffer, achBuf, ulSize);
                *ppBuffer = ((CHAR *) (*ppBuffer)) + ulSize + 1;
            }
            else
                rc = ERROR_INSUFFICIENT_BUFFER;
        }

        if (rc == 0)
            rc = DosFindNext(hDirHandle, (PVOID) & ffb, sizeof(ffb), &ulCount);
    }

    if ((rc == 0) || (rc == 18))
    {
        rc = DosFindClose(hDirHandle);
        if (rc == 6)
            rc = 0;
    }

    hDirHandle = HDIR_CREATE;
    if (rc == 0)
        rc = DosFindFirst("*",
                          &hDirHandle,
                          MUST_HAVE_DIRECTORY,
                          (PVOID) & ffb,
                          sizeof(ffb),
                          &ulCount,
                          FIL_STANDARD);

    while (rc == 0)
    {
        if (ffb.achName[0] == '.' &&
            (ffb.achName[1] == 0 || (ffb.achName[1] == '.' && ffb.achName[2] == 0)))
        {
        }
        else
        {
            rc = DosSetCurrentDir(ffb.achName);

            if (rc == 0)
                rc = _findallsyslevelfiles(achFileName, pszCID, ppBuffer, pulBufSize);

            if ((rc == 0) || (rc == 18))
                rc = DosSetCurrentDir("..");
        }

        if (rc == 0)
            rc = DosFindNext(hDirHandle, (PVOID) & ffb, sizeof(ffb), &ulCount);
    }

    if ((rc == 0) || (rc == 18))
    {
        rc = DosFindClose(hDirHandle);
        if (rc == 6)
            rc = 0;
    }

    return rc;
}

/*
 * _searchsubdirs:
 *
 */

STATIC APIRET _searchsubdirs(PSZ pszName,
                             PSZ pszCID,
                             PVOID pBuffer)
{
    HDIR hDirHandle = HDIR_CREATE;
    FILEFINDBUF3 ffb;
    ULONG ulCount = 1, ulFound = 0;
    APIRET rc = DosFindFirst("*",
                             &hDirHandle,
                             MUST_HAVE_DIRECTORY,
                             (PVOID) & ffb,
                             sizeof(ffb),
                             &ulCount,
                             FIL_STANDARD);

    while ((!ulFound) && (rc == 0))
    {
        if (ffb.achName[0] == '.' &&
            (ffb.achName[1] == 0 || (ffb.achName[1] == '.' && ffb.achName[2] == 0)))
        {
        }
        else
        {
            rc = DosSetCurrentDir(ffb.achName);

            if (rc == 0)
                rc = _findsyslevelfile(pszName, pszCID, pBuffer);

            if (rc == 18)
                rc = _searchsubdirs(pszName, pszCID, pBuffer);

            if (rc == 0)
                ulFound = 1;

            if ((rc == 0) || (rc == 18))
                rc = DosSetCurrentDir("..");
        }

        if ((!ulFound) && (rc == 0))
            rc = DosFindNext(hDirHandle, (PVOID) & ffb, sizeof(ffb), &ulCount);
    }

    rc = DosFindClose(hDirHandle);

    return ulFound ? 0 : 18;
}

/*
 * _firstdrive:
 *
 */

STATIC ULONG _firstdrive(ULONG ulDriveMap)
{
    return _nextdrive(ulDriveMap, 2);
}

/*
 * _nextdrive:
 *
 */

STATIC ULONG _nextdrive(ULONG ulDriveMap,
                        ULONG ulCurrent)
{
    ULONG ulNext = ulCurrent + 1;

    while (ulNext < 27)
        if (ulDriveMap & (0x1 << (ulNext - 1)))
            break;
        else
            ulNext++;

    if (ulNext < 27)
    {
        DosSetDefaultDisk(ulNext);
        DosSetCurrentDir("\\");
    }

    return (ulNext == 27) ? 0 : ulNext;
}
