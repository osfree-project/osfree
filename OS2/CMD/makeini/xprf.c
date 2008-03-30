
/*
 *@@sourcefile xprf.c:
 *      contains replacement profile (INI) functions.
 *      This file is new with V0.9.5 (2000-08-10) [umoeller].
 *
 *      These functions are quite similar to the regular Prf*
 *      APIs, with the following advantages:
 *
 *      -- They do not use shared memory, but the default
 *         memory allocation functions (malloc() etc.).
 *         This greatly reduces the pressure on the process's
 *         address space which apparently the default Prf*
 *         functions are causing with large INI files.
 *
 *      -- Profiles are written out to disk in one flush, not
 *         in the 32 KB chunks that the regular Prf* functions
 *         use.
 *
 *      Other differences:
 *
 *      -- They cannot be used interchangably. Once you open
 *         a profile using xprfOpenProfile, you must use the
 *         extended functions.
 *
 *      -- The INI file is kept open after xprfOpenProfile
 *         until xprfCloseProfile is called. Besides, we use
 *         DosProtectOpen so that a second open of the same INI
 *         file will always fail until xprfCloseProfile is called
 *         (sorta like a mutex for profiles).
 *         This is independent of the process or thread which
 *         is doing the second open.
 *
 *         This is because since we are not using shared memory,
 *         some other sort of protection had to be introduced
 *         to make sure no two processes operate on the same INI.
 *
 *         Still, these functions are NOT thread-safe for the same profile.
 *         If you open the profile on one thread and write and read
 *         concurrently on two threads, there's no protection, and everything
 *         will blow up. The functions are reentrant though, so for different
 *         profiles there will be no problems.
 *
 *      -- All changes to the INI files using xprfWriteProfileData
 *         are only made to the file in memory. Only xprfCloseProfile
 *         will flush the changes to disk. (I am not sure how the
 *         default PrfCloseProfile handles this on files other than
 *         HINI_USER and HINI_SYSTEM.)
 *
 *      -- HINI_USER and HINI_SYSTEM don't work. To retrieve data
 *         from them, use the standard Prf* functions. You can however
 *         use the new functions to create a duplicate of these files.
 *
 *      -- One similarity: All data items are limited to 64K,
 *         as with the standard profiles. This is not a limitation
 *         of the code, but of the INI file format, which uses
 *         USHORT's for all item length specifications.
 *
 *      Thanks go out to Carsten Arnold for sending the INI file
 *      format information (xprf.h) to me, which enabled me to
 *      create this in the first place.
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes:
 *      --  xprf*   replacement profile (INI) functions
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\xprf.h"
 *@@added V0.9.5 (2000-08-10) [umoeller]
 */

/*
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

#define INCL_DOSERRORS
#define INCL_WINSHELLDATA
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "linklist.h"
#include "prfh.h"
#include "xprf.h"

#pragma hdrstop

/*
 *@@category: Helpers\Profile (INI) replacement functions
 *      See xprf.c.
 */

/* ******************************************************************
 *
 *   Declarations
 *
 ********************************************************************/

/*
 *@@ XINIAPPDATA:
 *      application structure in XINI.
 */

typedef struct _XINIAPPDATA
{
    PSZ         pszAppName;
    LINKLIST    llKeys;             // contains PXINIKEYDATA pointers
    ULONG       cKeys;              // count of items on list
} XINIAPPDATA, *PXINIAPPDATA;

/*
 *@@ XINIKEYDATA:
 *      key/data structure in XINIAPPDATA.
 */

typedef struct _XINIKEYDATA
{
    PSZ         pszKeyName;
    PBYTE       pbData;
    ULONG       cbData;
} XINIKEYDATA, *PXINIKEYDATA;

/* ******************************************************************
 *
 *   Helpers
 *
 ********************************************************************/

/*
 * FindApp:
 *      attempts to find the specified application
 *      in the given profile data.
 *      Returns NULL if not found.
 *
 *      Private helper.
 *
 *@@changed V1.0.0 (2002-09-17) [umoeller]: now returning APIRET
 */

STATIC APIRET FindApp(PXINI pXIni,           // in: profile opened with xprfOpenProfile
                      const char *pcszApp,
                      PXINIAPPDATA *ppAppData)
{
    PLISTNODE pAppNode = lstQueryFirstNode(&pXIni->llApps);
    while (pAppNode)
    {
        PXINIAPPDATA pAppDataThis = (PXINIAPPDATA)pAppNode->pItemData;
        if (!strcmp(pAppDataThis->pszAppName, pcszApp))
        {
            *ppAppData = pAppDataThis;
            return NO_ERROR;
        }

        pAppNode = pAppNode->pNext;
    }

    return PRFERR_INVALID_APP_NAME;
}

/*
 * CreateApp:
 *      creates a new application in the specified
 *      profile structure and appends it to the list.
 *      This does NOT check for whether the app is
 *      already in the profile.
 *
 *      Private helper.
 *
 *@@changed V1.0.0 (2002-09-17) [umoeller]: now returning APIRET
 */

STATIC APIRET CreateApp(PXINI pXIni,         // in: profile opened with xprfOpenProfile
                        const char *pcszApp,
                        PXINIAPPDATA *ppAppData)
{
    PXINIAPPDATA pAppData;
    if (pAppData = (PXINIAPPDATA)malloc(sizeof(XINIAPPDATA)))
    {
        pAppData->pszAppName = strdup(pcszApp);
        lstInit(&pAppData->llKeys, FALSE);
        pAppData->cKeys = 0;

        // store in INI's apps list
        lstAppendItem(&pXIni->llApps, pAppData);

        *ppAppData = pAppData;
        return NO_ERROR;
    }

    return ERROR_NOT_ENOUGH_MEMORY;
}

/*
 * FindKey:
 *      attempts to find the specified key
 *      in the given application.
 *      Returns NULL if not found.
 *
 *      Private helper.
 *
 *@@changed V1.0.0 (2002-09-17) [umoeller]: now returning APIRET
 */

STATIC APIRET FindKey(PXINIAPPDATA pAppData,
                      const char *pcszKey,
                      PXINIKEYDATA *ppKeyData)
{
    PLISTNODE pKeyNode = lstQueryFirstNode(&pAppData->llKeys);
    while (pKeyNode)
    {
        PXINIKEYDATA pKeyDataThis = (PXINIKEYDATA)pKeyNode->pItemData;
        if (!strcmp(pKeyDataThis->pszKeyName, pcszKey))
        {
            *ppKeyData = pKeyDataThis;
            return NO_ERROR;
        }

        pKeyNode = pKeyNode->pNext;
    }

    return PRFERR_INVALID_KEY_NAME;
}

/*
 * CreateKey:
 *      creates a new key in the specified application
 *      structure and appends it to the list.
 *      This does NOT check for whether the key is
 *      already in the application.
 *
 *      Private helper.
 *
 *@@changed V1.0.0 (2002-09-17) [umoeller]: now returning APIRET
 */

STATIC APIRET CreateKey(PXINIAPPDATA pAppData,
                        const char *pcszKey,        // in: key name
                        PBYTE pbData,               // in: data for key
                        ULONG cbData,               // in: sizeof (*pbData)
                        PXINIKEYDATA *ppKeyData)    // out: new key data
{
    PXINIKEYDATA pKeyData;
    if (pKeyData = (PXINIKEYDATA)malloc(sizeof(XINIKEYDATA)))
    {
        pKeyData->pszKeyName = strdup(pcszKey);

        if (pKeyData->pbData = (PBYTE)malloc(cbData))
        {
            memcpy(pKeyData->pbData, pbData, cbData);
            pKeyData->cbData = cbData;
            // store in app's keys list
            lstAppendItem(&pAppData->llKeys, pKeyData);
            pAppData->cKeys++;

            *ppKeyData = pKeyData;
            return NO_ERROR;
        }
        else
            // malloc failed:
            free(pKeyData);
    }

    return ERROR_NOT_ENOUGH_MEMORY;
}

/*
 * FreeKey:
 *      frees the specified key. Does not remove
 *      the key from the keys list in XINIAPPDATA
 *      however.
 *
 *      Private helper.
 */

STATIC VOID FreeKey(PXINIKEYDATA pKeyDataThis)
{
    if (pKeyDataThis->pszKeyName)
        free(pKeyDataThis->pszKeyName);
    if (pKeyDataThis->pbData)
        free(pKeyDataThis->pbData);
    free(pKeyDataThis);
}

/*
 *@@ FreeKeyIfExists:
 *
 *@@added V1.0.0 (2002-09-17) [umoeller]
 */

STATIC VOID FreeKeyIfExists(PXINI pXIni,         // in: profile opened with xprfOpenProfile
                            PXINIAPPDATA pAppData,
                            PCSZ pcszKey)
{
    // find key
    PXINIKEYDATA pKeyData;

    if (!FindKey(pAppData,
                 pcszKey,
                 &pKeyData))
    {
        // key exists: kill that
        FreeKey(pKeyData);
        // and remove from app's keys list
        lstRemoveItem(&pAppData->llKeys, pKeyData);
        pAppData->cKeys--;

        // rewrite profile on close
        pXIni->fDirty = TRUE;
    }
    // else key doesn't exist:
    // nothing to do
}

/*
 * FreeApp:
 *      frees the specified application. Does not remove
 *      the application from the application list in XINI
 *      however.
 *
 *      Private helper.
 */

STATIC VOID FreeApp(PXINIAPPDATA pAppDataThis)
{
    PLISTNODE pKeyNode = lstQueryFirstNode(&pAppDataThis->llKeys);
    while (pKeyNode)
    {
        PXINIKEYDATA pKeyDataThis = (PXINIKEYDATA)pKeyNode->pItemData;

        FreeKey(pKeyDataThis);

        pKeyNode = pKeyNode->pNext;
    }

    if (pAppDataThis->pszAppName)
        free(pAppDataThis->pszAppName);

    lstClear(&pAppDataThis->llKeys);

    free(pAppDataThis);
}

/*
 * FreeINI:
 *      cleans up the specified ini structure entirely.
 *
 *      Private helper.
 */

STATIC VOID FreeINI(PXINI pXIni)       // in: profile opened with xprfOpenProfile
{
    if (pXIni)
    {
        PLISTNODE pAppNode = lstQueryFirstNode(&pXIni->llApps);
        while (pAppNode)
        {
            PXINIAPPDATA pAppDataThis = (PXINIAPPDATA)pAppNode->pItemData;

            FreeApp(pAppDataThis);

            pAppNode = pAppNode->pNext;
        }

        lstClear(&pXIni->llApps);
        free(pXIni);
    }
}

/* ******************************************************************
 *
 *   Read/write data
 *
 ********************************************************************/

/*
 * ReadINI:
 *      reads in the entire INI file from disk and
 *      initializes all internal data structures.
 *
 *      Private helper.
 *
 *@@changed V1.0.0 (2002-09-17) [umoeller]: now returning APIRET
 */

STATIC APIRET ReadINI(PXINI pXIni)      // in: profile opened with xprfOpenProfile
{
    APIRET      arc;
    FILESTATUS3 fs3;

    if (!(arc = DosQueryFileInfo(pXIni->hFile,
                                 FIL_STANDARD,
                                 &fs3,
                                 sizeof(fs3))))
    {
        PBYTE  pbFileData;
        if (!(pbFileData = (PBYTE)malloc(fs3.cbFile)))
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            ULONG ulSet = 0;

            if (!(arc = DosSetFilePtr(pXIni->hFile,
                                      0,
                                      FILE_BEGIN,
                                      &ulSet)))
            {
                ULONG cbRead = 0;
                if (!(arc = DosRead(pXIni->hFile,
                                    pbFileData,
                                    fs3.cbFile,
                                    &cbRead)))
                {
                    if (cbRead != fs3.cbFile)
                        arc = ERROR_NO_DATA;
                    else
                    {
                        PINIFILE_HEADER pHeader = (PINIFILE_HEADER)pbFileData;
                        if (pHeader->magic == 0xFFFFFFFF)
                        {
                            ULONG   ulAppOfs = pHeader->offFirstApp;

                            // create-applications loop
                            while ((ulAppOfs) && (!arc))
                            {
                                // application struct
                                PINIFILE_APP pApp = (PINIFILE_APP)(pbFileData + ulAppOfs);
                                ULONG   ulKeysOfs = pApp->offFirstKeyInApp;
                                PXINIAPPDATA pIniApp;

                                if (arc = CreateApp(pXIni,
                                                    (PBYTE)(pbFileData + pApp->offAppName),
                                                    &pIniApp))
                                    break;

                                // create-keys loop
                                while ((ulKeysOfs) && (!arc))
                                {
                                    PINIFILE_KEY pKey = (PINIFILE_KEY)(pbFileData + ulKeysOfs);

                                    PXINIKEYDATA pIniKey;

                                    if (arc = CreateKey(pIniApp,
                                                        (PSZ)(pbFileData + pKey->offKeyName),
                                                        (PBYTE)(pbFileData + pKey->offKeyData),
                                                        pKey->lenKeyData,
                                                        &pIniKey))
                                        break;

                                    // next key; can be null
                                    ulKeysOfs = pKey->offNextKeyInApp;
                                }

                                // next application; can be null
                                ulAppOfs = pApp->offNextApp;
                            }
                        }
                    }
                }
            }

            free(pbFileData);
        }
    }

    return arc;
}

/*
 * WriteINI:
 *      writes the entire data structure back to disk.
 *      Does not close the file.
 *
 *      Private helper.
 *
 *@@changed V1.0.0 (2002-09-17) [umoeller]: now returning APIRET
 */

STATIC APIRET WriteINI(PXINI pXIni)      // in: profile opened with xprfOpenProfile
{
    APIRET  arc = NO_ERROR;
    ULONG   ulTotalFileSize = sizeof(INIFILE_HEADER);
    ULONG   ulSet = 0;
    PBYTE   pbData2Write = NULL;

    // check how much memory we'll need:
    // one INIFILE_HEADER
    // + for each app: one INIFILE_APP plus app name length
    // + for each key: one INIFILE_KEY plus key name length plus data length

    // go thru all apps
    PLISTNODE pAppNode = lstQueryFirstNode(&pXIni->llApps);

    while (pAppNode)
    {
        PLISTNODE pKeyNode;

        PXINIAPPDATA pAppDataThis = (PXINIAPPDATA)pAppNode->pItemData;

        // one INIFILE_APP plus app name length for each app
        ulTotalFileSize +=   sizeof(INIFILE_APP)
                           + strlen(pAppDataThis->pszAppName)
                           + 1;         // null terminator

        // for each app, go thru all keys
        pKeyNode = lstQueryFirstNode(&pAppDataThis->llKeys);
        while (pKeyNode)
        {
            PXINIKEYDATA pKeyDataThis = (PXINIKEYDATA)pKeyNode->pItemData;
            ulTotalFileSize +=   sizeof(INIFILE_KEY)
                               + strlen(pKeyDataThis->pszKeyName)
                               + pKeyDataThis->cbData
                               + 1;         // null terminator

            pKeyNode = pKeyNode->pNext;
        }

        pAppNode = pAppNode->pNext;
    }

    // allocate buffer for total size
    if (pbData2Write = (PBYTE)malloc(ulTotalFileSize))
    {
        // set header in buffer
        PINIFILE_HEADER pHeader = (PINIFILE_HEADER)pbData2Write;
        // pointer into buffer for current write
        // (used below)
        ULONG   ulCurOfs = sizeof(INIFILE_HEADER);

        // 1) set up header

        pHeader->magic = 0xFFFFFFFF;
        if (lstCountItems(&pXIni->llApps))
            // we have any applications:
            pHeader->offFirstApp = sizeof(INIFILE_HEADER);
                     // offset of first application in file
        else
            // empty INI file:
            pHeader->offFirstApp = 0;
        pHeader->lenFile = ulTotalFileSize;
        pHeader->filler1 = 0;
        pHeader->filler2 = 0;

        // 2) for-all-applications loop

        pAppNode = lstQueryFirstNode(&pXIni->llApps);
        while (pAppNode)
        {
            PXINIAPPDATA pAppDataThis = (PXINIAPPDATA)pAppNode->pItemData;
            ULONG       cbAppName = strlen(pAppDataThis->pszAppName) + 1;

            // layout of application entry in file:
            // -- struct PINIFILE_APP       (ulCurOfs right now)
            // -- application name (null-terminated)
            // --   INIFILE_KEY 1
            // --   INIFILE_KEY 2 ...
            // -- next struct PINIFILE_APP

            // make pointer to application entry
            PINIFILE_APP pIniAppCurrent = (PINIFILE_APP)(pbData2Write + ulCurOfs);

            // write out application entry
            // pIniAppCurrent->offNextApp = 0;
            // pIniAppCurrent->offFirstKeyInApp = ulCurOfs + cbAppName;
            pIniAppCurrent->filler1 = 0;
            pIniAppCurrent->lenAppName
                = pIniAppCurrent->_lenAppName
                = cbAppName;
            // set offset to application name: put this right after the application
            ulCurOfs += sizeof(INIFILE_APP);
            pIniAppCurrent->offAppName = ulCurOfs;

            // write app name (null-terminated)
            memcpy(pbData2Write + ulCurOfs,
                   pAppDataThis->pszAppName,
                   cbAppName);
            ulCurOfs += cbAppName;

            // ulCurOfs points to INIFILE_KEY entry now
            if (pAppDataThis->cKeys)
            {
                // we have keys:
                PLISTNODE pKeyNode = lstQueryFirstNode(&pAppDataThis->llKeys);
                pIniAppCurrent->offFirstKeyInApp = ulCurOfs;

                // 3) for-all-keys loop per application

                while (pKeyNode)
                {
                    PXINIKEYDATA pKeyDataThis = (PXINIKEYDATA)pKeyNode->pItemData;
                    ULONG       cbKeyName = strlen(pKeyDataThis->pszKeyName) + 1;

                    PINIFILE_KEY pIniKeyCurrent = (PINIFILE_KEY)(pbData2Write + ulCurOfs);
                    pIniKeyCurrent->filler1 = 0;
                    ulCurOfs += sizeof(INIFILE_KEY);
                            // has offset to key name now

                    // a) key name
                    pIniKeyCurrent->lenKeyName
                        = pIniKeyCurrent->_lenKeyName
                        = cbKeyName;
                    pIniKeyCurrent->offKeyName = ulCurOfs;
                    memcpy(pbData2Write + ulCurOfs,
                           pKeyDataThis->pszKeyName,
                           cbKeyName);
                    ulCurOfs += cbKeyName;
                            // has offset to data now

                    // b) data
                    pIniKeyCurrent->lenKeyData
                        = pIniKeyCurrent->_lenKeyData
                        = pKeyDataThis->cbData;
                    pIniKeyCurrent->offKeyData = ulCurOfs;
                    memcpy(pbData2Write + ulCurOfs,
                           pKeyDataThis->pbData,
                           pKeyDataThis->cbData);
                    ulCurOfs += pKeyDataThis->cbData;
                            // points to after all key data now;
                            // this receives either the next key/data block
                            // or the next application or nothing

                    // ofs of next key:
                    if (pKeyNode->pNext)
                        pIniKeyCurrent->offNextKeyInApp = ulCurOfs;
                    else
                        // last key:
                        pIniKeyCurrent->offNextKeyInApp = 0;

                    pKeyNode = pKeyNode->pNext;
                }

                // ulCurOfs points to after the last keys entry now
            }
            else
                // no keys:
                pIniAppCurrent->offFirstKeyInApp = 0;

            // done with keys (there may be none!);
            // now set offset to next app in current application
            if (pAppNode->pNext)
                pIniAppCurrent->offNextApp = ulCurOfs;
            else
                // this was the last one:
                pIniAppCurrent->offNextApp = 0;

            // next app
            pAppNode = pAppNode->pNext;
        }

        // write out everything
        if (!(arc = DosSetFilePtr(pXIni->hFile,
                                  0,
                                  FILE_BEGIN,
                                  &ulSet)))
        {
            ULONG cbWritten = 0;
            if (!(arc = DosWrite(pXIni->hFile,
                                 pbData2Write,
                                 ulTotalFileSize,
                                 &cbWritten)))
            {
                if (!(arc = DosSetFileSize(pXIni->hFile,
                                           ulTotalFileSize)))
                    ;
            }
        }

        free(pbData2Write);
    }

    return arc;
}

/* ******************************************************************
 *
 *   API Functions
 *
 ********************************************************************/

/*
 *@@ xprfOpenProfile:
 *      opens an extended profile (XINI). This is similar to
 *      PrfOpenProfile, but cannot be used interchangably.
 *      See the top of this file (xprf.c) for general remarks.
 *
 *      If the full path is not specified, the current
 *      directory is used. This uses DosProtectOpen to
 *      open the profile, so see CPREF for additional remarks.
 *
 *      If the specified profile does not exist yet, it is
 *      created by this function.
 *
 *      Note that you must not open the system profiles
 *      (OS2.INI and OS2SYS.INI) using this function because
 *      this would keep OS/2 from updating them.
 *
 *      This returns 0 (NO_ERROR) on success. Otherwise either
 *      an OS/2 error code (ERROR_*) or one of the profile error
 *      codes defined in prfh.h is returned.
 */

APIRET xprfOpenProfile(const char *pcszFilename,    // in: profile name
                       PXINI *ppxini)               // out: profile handle
{
    APIRET  arc = NO_ERROR;
    PXINI   pXIni = NULL;
    ULONG   ulFilenameLen;

    if (    (!pcszFilename)
         || (!ppxini)
       )
        arc = ERROR_INVALID_PARAMETER;      // V1.0.0 (2002-09-17) [umoeller]
    else if (!(ulFilenameLen = strlen(pcszFilename)))
        arc = ERROR_INVALID_PARAMETER;      // V1.0.0 (2002-09-17) [umoeller]
    else if (ulFilenameLen >= CCHMAXPATH - 1)
        arc = ERROR_FILENAME_EXCED_RANGE;
    else
    {
        HFILE   hFile = NULLHANDLE;
        ULONG   ulAction = 0;
        // FHLOCK  hLock = 0;

        if (!(arc = DosOpen((PSZ)pcszFilename,
                            &hFile,
                            &ulAction,
                            1024,          // initial size
                            FILE_NORMAL,
                            OPEN_ACTION_CREATE_IF_NEW
                               | OPEN_ACTION_OPEN_IF_EXISTS,
                            OPEN_FLAGS_FAIL_ON_ERROR
                               | OPEN_FLAGS_SEQUENTIAL
                               | OPEN_SHARE_DENYREADWRITE
                               | OPEN_ACCESS_READWRITE,
                            NULL)))
        {
            if (!(pXIni = (PXINI)malloc(sizeof(XINI))))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                // OK: initialize XINI
                memset(pXIni, 0, sizeof(XINI));
                memcpy(pXIni->acMagic, XINI_MAGIC_BYTES, sizeof(XINI_MAGIC_BYTES));
                memcpy(pXIni->szFilename,
                       pcszFilename,
                       ulFilenameLen + 1);
                pXIni->hFile = hFile;
                // pXIni->hLock = hLock;

                lstInit(&pXIni->llApps, FALSE);

                if (ulAction == FILE_CREATED)
                    // file newly created: rewrite on close
                    pXIni->fDirty = TRUE;
                else
                    // file existed: read data
                    if (arc = ReadINI(pXIni))
                        // error:
                        FreeINI(pXIni);
            }

            if (!arc)
                *ppxini = pXIni;
            else
                DosClose(hFile);
        }
    }

    return arc;
}

/*
 *@@ xprfQueryProfileSize:
 *      returns the size of INI data, similarly to
 *      what PrfQueryProfileSize does.
 *
 *      If (pszAppName == NULL), this returns the size of
 *      the buffer required to hold the enumerated list
 *      of application names. pszKey is then ignored.
 *
 *      Otherwise, if (pszKeyName == NULL), this returns
 *      the size of the buffer required to hold the
 *      enumerated list of _key_ names for the given
 *      application.
 *
 *      If both pszAppName and pszKeyName are not NULL, this
 *      returns the data size of the given key.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *@@added V1.0.0 (2002-09-17) [umoeller]
 */

APIRET xprfQueryProfileSize(PXINI pXIni,          // in: profile opened with xprfOpenProfile
                            PCSZ pszAppName,      // in: application name or NULL
                            PCSZ pszKeyName,      // in: key name or NULL
                            PULONG pulDataLen)    // out: size of requested data
{
    APIRET  arc = NO_ERROR;
    ULONG   ulDataLen = 0;

    if (!pszAppName)
    {
        PLISTNODE pAppNode = lstQueryFirstNode(&pXIni->llApps);
        while (pAppNode)
        {
            PXINIAPPDATA pAppDataThis = (PXINIAPPDATA)pAppNode->pItemData;
            ulDataLen += strlen(pAppDataThis->pszAppName) + 1;
            pAppNode = pAppNode->pNext;
        }

        // extra byte for terminating extra null
        ++ulDataLen;
    }
    else
    {
        // app specified:
        PXINIAPPDATA pAppData;

        if (!(arc = FindApp(pXIni,
                            pszAppName,
                            &pAppData)))
        {
            // app exists:

            if (!pszKeyName)
            {
                // app != NULL, but key == NULL:
                // return size of keys list
                PLISTNODE pKeyNode = lstQueryFirstNode(&pAppData->llKeys);
                while (pKeyNode)
                {
                    PXINIKEYDATA pKeyDataThis = (PXINIKEYDATA)pKeyNode->pItemData;
                    ulDataLen += strlen(pKeyDataThis->pszKeyName) + 1;
                    pKeyNode = pKeyNode->pNext;
                }

                // extra byte for terminating extra null
                ++ulDataLen;
            }
            else
            {
                // both app and key specified:
                PXINIKEYDATA pKeyData;
                if (!(arc = FindKey(pAppData,
                                    pszKeyName,
                                    &pKeyData)))
                    ulDataLen = pKeyData->cbData;
            }
        }
    }

    if (pulDataLen)
        *pulDataLen = ulDataLen;

    return arc;
}

/*
 *@@ xprfQueryProfileData:
 *      reads data from the given XINI, similarly to
 *      what PrfQueryProfileData does.
 *
 *      If (pszAppName == NULL), this returns the
 *      enumerated list of application names.
 *      pszKey is then ignored. *pulBufferMax
 *      receives the total size of the list excluding
 *      the final NULL character.
 *
 *      Otherwise, if (pszKeyName == NULL), this the
 *      enumerated list of _key_ names for the given
 *      application. *pulBufferMax receives the total
 *      size of the list excluding the final NULL character.
 *
 *      If both pszAppName and pszKeyName are not NULL,
 *      this returns the data of the given key.
 *      pulBufferMax receives the bytes copied.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 */

APIRET xprfQueryProfileData(PXINI pXIni,          // in: profile opened with xprfOpenProfile
                            PCSZ pszAppName,      // in: application name
                            PCSZ pszKeyName,      // in: key name or NULL
                            PVOID pBuffer,        // in: buffer to receive data
                            PULONG pulBufferMax)  // in: buffer size, out: size of written data
{
    APIRET  arc = NO_ERROR;
    ULONG   ulDataLen = 0;

    if (!pszAppName)
    {
        PLISTNODE pAppNode = lstQueryFirstNode(&pXIni->llApps);
        PBYTE   pbTarget = (PSZ)pBuffer;
        ULONG   cbCopied = 0;
        while (pAppNode)
        {
            PXINIAPPDATA pAppDataThis = (PXINIAPPDATA)pAppNode->pItemData;
            ULONG   cbThis = strlen(pAppDataThis->pszAppName) + 1;
            if (cbCopied + cbThis > *pulBufferMax)
                break;
            else
            {
                memcpy(pbTarget + cbCopied,
                       pAppDataThis->pszAppName,
                       cbThis);
                cbCopied += cbThis;
            }

            pAppNode = pAppNode->pNext;
        }

        // extra byte for terminating extra null
        pbTarget[cbCopied] = '\0';
        ulDataLen = cbCopied;       // PMREF says terminating null is not counted
    }
    else
    {
        // app specified:
        PXINIAPPDATA pAppData;

        if (!(arc = FindApp(pXIni,
                            pszAppName,
                            &pAppData)))
        {
            // app exists:

            if (!pszKeyName)
            {
                // app != NULL, but key == NULL:
                // return size of keys list
                PLISTNODE pKeyNode = lstQueryFirstNode(&pAppData->llKeys);
                PBYTE   pbTarget = (PSZ)pBuffer;
                ULONG   cbCopied = 0;
                while (pKeyNode)
                {
                    PXINIKEYDATA pKeyDataThis = (PXINIKEYDATA)pKeyNode->pItemData;
                    ULONG   cbThis = strlen(pKeyDataThis->pszKeyName) + 1;
                    if (cbCopied + cbThis > *pulBufferMax)
                        break;
                    else
                    {
                        memcpy(pbTarget + cbCopied,
                               pKeyDataThis->pszKeyName,
                               cbThis);
                        cbCopied += cbThis;
                    }

                    pKeyNode = pKeyNode->pNext;
                }

                // extra byte for terminating extra null
                pbTarget[cbCopied] = '\0';
                ulDataLen = cbCopied;       // PMREF says terminating null is not counted
            }
            else
            {
                // both app and key specified:
                PXINIKEYDATA pKeyData;
                if (!(arc = FindKey(pAppData,
                                    pszKeyName,
                                    &pKeyData)))
                {
                    ulDataLen = min(pKeyData->cbData,
                                    *pulBufferMax);
                    memcpy(pBuffer,
                           pKeyData->pbData,
                           ulDataLen);
                }
            }
        }
    }

    if (pulBufferMax)
        *pulBufferMax = ulDataLen;

    return arc;
}

/*
 *@@ xprfWriteProfileData:
 *      writes data into an extended profile (XINI).
 *      This operates similar to PrfWriteProfileData,
 *      that is:
 *
 *      -- If all of pcszApp, pcszKey, pData, and
 *         ulDataLen are specified, the given data is
 *         written into the specified application/key
 *         pair.
 *
 *      -- If pcszApp and pcszKey are specified, but
 *         pData and ulDataLen are null, the specified
 *         key is deleted.
 *
 *      -- If pcszApp is specified, but all of pcszKey,
 *         pData, and ulDataLen are null, the entire
 *         specified application is deleted.
 *
 *      You cannot specify HINI_SYSTEM or HINI_USER for
 *      hINi.
 *
 *      Note that if data has been added or removed,
 *      the INI file on disk is not updated automatically.
 *      Instead, our memory copy of it is only marked
 *      as "dirty" so that the file will be rewritten
 *      on xprfCloseProfile.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 */

APIRET xprfWriteProfileData(PXINI pXIni,          // in: profile opened with xprfOpenProfile
                            const char *pcszApp,  // in: application name
                            const char *pcszKey,  // in: key name or NULL
                            PVOID pData,          // in: data to write or NULL
                            ULONG ulDataLen)      // in: sizeof(*pData) or null
{
    APIRET  arc = NO_ERROR;

    if (    (!pXIni)
         || (memcmp(pXIni->acMagic, XINI_MAGIC_BYTES, sizeof(XINI_MAGIC_BYTES)))
       )
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        // check if application exists
        PXINIAPPDATA pAppData;

        if (FindApp(pXIni,
                    pcszApp,
                    &pAppData))
            // not found:
            pAppData = NULL;

        // now check: does caller want entire application deleted?
        if (!pcszKey)
        {
            // yes, delete application: did we find it?
            if (pAppData)
            {
                // yes: kill that
                FreeApp(pAppData);
                // and remove from list
                lstRemoveItem(&pXIni->llApps, pAppData);

                // rewrite profile on close
                pXIni->fDirty = TRUE;
            }
            // else application doesn't exist:
            // nothing to do return NO_ERROR
        }
        else
        {
            // caller has specified key:
            // does caller want a key to be deleted?
            if (!ulDataLen)
            {
                // yes: delete key:
                if (pAppData)
                {
                    // app exists:
                    FreeKeyIfExists(pXIni,
                                    pAppData,
                                    pcszKey);
                }
                // else app doesn't even exist:
                // nothing to do, return NO_ERROR
            }
            else
            {
                // key and data specified: let's add something...

                if (!pAppData)
                    // app doesn't exist yet:
                    // create
                    arc = CreateApp(pXIni,
                                    pcszApp,
                                    &pAppData);

                if (!arc)
                {
                    // found or created app:

                    // delete existing key if it exists
                    PXINIKEYDATA pKeyData;

                    FreeKeyIfExists(pXIni,
                                    pAppData,
                                    pcszKey);

                    // now create new key
                    if (!(arc = CreateKey(pAppData,
                                          pcszKey,
                                          (PBYTE)pData,
                                          ulDataLen,
                                          &pKeyData)))
                       // mark as dirty
                       pXIni->fDirty = TRUE;
                }
            }
        }
    }

    return arc;
}

/*
 *@@ xprfCloseProfile:
 *      closes a profile opened with xprfOpenProfile.
 *      If the profile is "dirty", that is, if any data
 *      has been changed using xprfWriteProfileData, the
 *      file is written back to disk before closing.
 *
 *      You cannot specify HINI_SYSTEM or HINI_USER for
 *      hINi.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 */

APIRET xprfCloseProfile(PXINI pXIni)       // in: profile opened with xprfOpenProfile
{
    APIRET  arc = NO_ERROR;

    if (    (!pXIni)
         || (memcmp(pXIni->acMagic, XINI_MAGIC_BYTES, sizeof(XINI_MAGIC_BYTES)))
       )
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        if (pXIni->fDirty)
            arc = WriteINI(pXIni);

        if (!arc)
        {
            if (!(arc = DosClose(pXIni->hFile)))
            {
                pXIni->hFile = 0;

                FreeINI(pXIni);
            }
        }
    }

    return arc;
}

/*
 *@@ xprfQueryKeysForApp:
 *      the equivalent of prfhQueryKeysForApp for
 *      XINI files.
 *
 *@@added V1.0.0 (2002-09-17) [umoeller]
 */

APIRET xprfQueryKeysForApp(PXINI hIni,      // in: INI handle
                           PCSZ pcszApp,    // in: application to query list for (or NULL for applications list)
                           PSZ *ppszKeys)   // out: keys list (newly allocated)
{
    APIRET  arc = NO_ERROR;
    PSZ     pKeys = NULL;
    ULONG   ulSizeOfKeysList = 0;

    // get size of keys list for pszApp
    if (!xprfQueryProfileSize(hIni, pcszApp, NULL, &ulSizeOfKeysList))
        arc = PRFERR_KEYSLIST;
    else
    {
        if (ulSizeOfKeysList == 0)
            ulSizeOfKeysList = 1;    // V0.9.19 (2002-04-11) [pr]

        if (!(pKeys = (PSZ)malloc(ulSizeOfKeysList)))
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            *pKeys = 0;
            if (!xprfQueryProfileData(hIni, pcszApp, NULL, pKeys, &ulSizeOfKeysList))
                arc = PRFERR_KEYSLIST;
        }
    }

    if (!arc)       // V0.9.12 (2001-05-12) [umoeller]
        *ppszKeys = pKeys;
    else
        if (pKeys)
            free(pKeys);

    return arc;
}

