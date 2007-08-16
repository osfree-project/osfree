
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
 *         will blow up. The functions are reeantrant though, so for different
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

#include "helpers\linklist.h"
#include "helpers\prfh.h"
#include "helpers\xprf.h"

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
 */

PXINIAPPDATA FindApp(PXINI pXIni,           // in: profile opened with xprfOpenProfile
                     const char *pcszApp)
{
    PXINIAPPDATA pReturn = NULL;
    PLISTNODE pAppNode = lstQueryFirstNode(&pXIni->llApps);
    while (pAppNode)
    {
        PXINIAPPDATA pAppDataThis = (PXINIAPPDATA)pAppNode->pItemData;
        if (strcmp(pAppDataThis->pszAppName, pcszApp) == 0)
        {
            pReturn = pAppDataThis;
            break;
        }

        pAppNode = pAppNode->pNext;
    }

    return (pReturn);
}

/*
 * CreateApp:
 *      creates a new application in the specified
 *      profile structure and appends it to the list.
 *      This does NOT check for whether the app is
 *      already in the profile.
 *
 *      Private helper.
 */

PXINIAPPDATA CreateApp(PXINI pXIni,         // in: profile opened with xprfOpenProfile
                       const char *pcszApp)
{
    PXINIAPPDATA pAppData = (PXINIAPPDATA)malloc(sizeof(XINIAPPDATA));
    if (pAppData)
    {
        pAppData->pszAppName = strdup(pcszApp);
        lstInit(&pAppData->llKeys, FALSE);
        pAppData->cKeys = 0;

        // store in INI's apps list
        lstAppendItem(&pXIni->llApps, pAppData);
        pXIni->cApps++;
    }

    return (pAppData);
}

/*
 * FindKey:
 *      attempts to find the specified key
 *      in the given application.
 *      Returns NULL if not found.
 *
 *      Private helper.
 */

PXINIKEYDATA FindKey(PXINIAPPDATA pAppData,
                     const char *pcszKey)
{
    PXINIKEYDATA pReturn = NULL;
    PLISTNODE pKeyNode = lstQueryFirstNode(&pAppData->llKeys);
    while (pKeyNode)
    {
        PXINIKEYDATA pKeyDataThis = (PXINIKEYDATA)pKeyNode->pItemData;
        if (strcmp(pKeyDataThis->pszKeyName, pcszKey) == 0)
        {
            pReturn = pKeyDataThis;
            break;
        }

        pKeyNode = pKeyNode->pNext;
    }

    return (pReturn);
}

/*
 * CreateKey:
 *      creates a new key in the specified application
 *      structure and appends it to the list.
 *      This does NOT check for whether the key is
 *      already in the application.
 *
 *      Private helper.
 */

PXINIKEYDATA CreateKey(PXINIAPPDATA pAppData,
                       const char *pcszKey,     // in: key name
                       PBYTE pbData,            // in: data for key
                       ULONG cbData)            // in: sizeof (*pbData)
{
    PXINIKEYDATA pKeyData = (PXINIKEYDATA)malloc(sizeof(XINIKEYDATA));
    if (pKeyData)
    {
        pKeyData->pszKeyName = strdup(pcszKey);

        pKeyData->pbData = (PBYTE)malloc(cbData);
        if (pKeyData->pbData)
        {
            memcpy(pKeyData->pbData, pbData, cbData);
            pKeyData->cbData = cbData;
            // store in app's keys list
            lstAppendItem(&pAppData->llKeys, pKeyData);
            pAppData->cKeys++;
        }
        else
        {
            // malloc failed:
            free(pKeyData);
            pKeyData = 0;
        }
    }

    return (pKeyData);
}

/*
 * FreeKey:
 *      frees the specified key. Does not remove
 *      the key from the keys list in XINIAPPDATA
 *      however.
 *
 *      Private helper.
 */

VOID FreeKey(PXINIKEYDATA pKeyDataThis)
{
    if (pKeyDataThis->pszKeyName)
        free(pKeyDataThis->pszKeyName);
    if (pKeyDataThis->pbData)
        free(pKeyDataThis->pbData);
    free(pKeyDataThis);
}

/*
 * FreeApp:
 *      frees the specified application. Does not remove
 *      the application from the application list in XINI
 *      however.
 *
 *      Private helper.
 */

VOID FreeApp(PXINIAPPDATA pAppDataThis)
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

BOOL FreeINI(PXINI pXIni)       // in: profile opened with xprfOpenProfile
{
    BOOL brc = FALSE;

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

    return (brc);
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
 */

BOOL ReadINI(PXINI pXIni)       // in: profile opened with xprfOpenProfile
{
    BOOL brc = FALSE;
    FILESTATUS3 fs3;

    if (DosProtectQueryFileInfo(pXIni->hFile,
                                FIL_STANDARD,
                                &fs3,
                                sizeof(fs3),
                                pXIni->hLock)
            == NO_ERROR)
    {
        PBYTE   pbFileData = (PBYTE)malloc(fs3.cbFile);
        if (pbFileData)
        {
            ULONG ulSet = 0;
            APIRET arc = DosProtectSetFilePtr(pXIni->hFile,
                                              0,
                                              FILE_BEGIN,
                                              &ulSet,
                                              pXIni->hLock);
            if (    (arc == NO_ERROR)
                 && (ulSet == 0)
               )
            {
                ULONG cbRead = 0;
                arc = DosProtectRead(pXIni->hFile,
                                     pbFileData,
                                     fs3.cbFile,
                                     &cbRead,
                                     pXIni->hLock);
                if (    (arc == NO_ERROR)
                     && (cbRead == fs3.cbFile)
                   )
                {
                    PINIFILE_HEADER pHeader = (PINIFILE_HEADER)pbFileData;
                    if (pHeader->magic == 0xFFFFFFFF)
                    {
                        ULONG   ulAppOfs = pHeader->offFirstApp;

                        // it was a valid profile, so return TRUE
                        brc = TRUE;

                        // create-applications loop
                        while ((ulAppOfs) && (brc))
                        {
                            // application struct
                            PINIFILE_APP pApp = (PINIFILE_APP)(pbFileData + ulAppOfs);
                            ULONG   ulKeysOfs = pApp->offFirstKeyInApp;
                            PXINIAPPDATA pIniApp
                                = CreateApp(pXIni,
                                            (PBYTE)(pbFileData + pApp->offAppName));
                            if (!pIniApp)
                            {
                                brc = FALSE;
                                break;
                            }

                            // create-keys loop
                            while ((ulKeysOfs) && (brc))
                            {
                                PINIFILE_KEY pKey = (PINIFILE_KEY)(pbFileData + ulKeysOfs);

                                PXINIKEYDATA pIniKey
                                    = CreateKey(pIniApp,
                                                (PSZ)(pbFileData + pKey->offKeyName),
                                                (PBYTE)(pbFileData + pKey->offKeyData),
                                                pKey->lenKeyData);

                                if (!pKey)
                                {
                                    brc = FALSE;
                                    break;
                                }

                                // next key; can be null
                                ulKeysOfs = pKey->offNextKeyInApp;
                            }

                            // next application; can be null
                            ulAppOfs = pApp->offNextApp;
                        }
                    }
                }
            }

            free(pbFileData);
        }
    }

    return (brc);
}

/*
 * WriteINI:
 *      writes the entire data structure back to disk.
 *      Does not close the file.
 *
 *      Private helper.
 */

BOOL WriteINI(PXINI pXIni)      // in: profile opened with xprfOpenProfile
{
    BOOL    brc = FALSE;
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
    pbData2Write = (PBYTE)malloc(ulTotalFileSize);
    if (pbData2Write)
    {
        APIRET arc = NO_ERROR;

        // set header in buffer
        PINIFILE_HEADER pHeader = (PINIFILE_HEADER)pbData2Write;
        // pointer into buffer for current write
        // (used below)
        ULONG   ulCurOfs = sizeof(INIFILE_HEADER);

        // 1) set up header

        pHeader->magic = 0xFFFFFFFF;
        if (pXIni->cApps)
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
        arc = DosProtectSetFilePtr(pXIni->hFile,
                                   0,
                                   FILE_BEGIN,
                                   &ulSet,
                                   pXIni->hLock);
        if (arc == NO_ERROR)
        {
            ULONG cbWritten = 0;
            arc = DosProtectWrite(pXIni->hFile,
                                  pbData2Write,
                                  ulTotalFileSize,
                                  &cbWritten,
                                  pXIni->hLock);
            if (arc == NO_ERROR)
            {
                arc = DosProtectSetFileSize(pXIni->hFile,
                                            ulTotalFileSize,
                                            pXIni->hLock);
                if (arc == NO_ERROR)
                    brc = TRUE;
            }
        }

        free(pbData2Write);
    }

    return (brc);
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

    if (pcszFilename)
        if (strlen(pcszFilename) < CCHMAXPATH - 1)
        {
            HFILE   hFile = NULLHANDLE;
            ULONG   ulAction = 0;
            FHLOCK  hLock = 0;
            arc = DosProtectOpen((PSZ)pcszFilename,
                                 &hFile,
                                 &ulAction,
                                 1024,          // initial size
                                 FILE_NORMAL,
                                 OPEN_ACTION_CREATE_IF_NEW
                                    | OPEN_ACTION_OPEN_IF_EXISTS,
                                 OPEN_FLAGS_FAIL_ON_ERROR
                                    | OPEN_FLAGS_NO_CACHE
                                    | OPEN_FLAGS_SEQUENTIAL
                                    | OPEN_SHARE_DENYREADWRITE
                                    | OPEN_ACCESS_READWRITE,
                                 NULL, // no EAs
                                 &hLock);
            if (arc == NO_ERROR)
            {
                pXIni = (PXINI)malloc(sizeof(XINI));
                if (!pXIni)
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                else
                {
                    // OK: initialize XINI
                    memset(pXIni, 0, sizeof(XINI));
                    memcpy(pXIni->acMagic, XINI_MAGIC_BYTES, sizeof(XINI_MAGIC_BYTES));
                    strcpy(pXIni->szFilename, pcszFilename);
                    pXIni->hFile = hFile;
                    pXIni->hLock = hLock;

                    lstInit(&pXIni->llApps, FALSE);

                    if (ulAction == FILE_CREATED)
                        // file newly created: rewrite on close
                        pXIni->fDirty = TRUE;
                    else
                        // file existed: read data
                        if (!ReadINI(pXIni))
                        {
                            // error:
                            FreeINI(pXIni);
                            pXIni = NULL;
                            arc = PRFERR_READ;
                        }

                    if ((pXIni) && (ppxini))
                        *ppxini = pXIni;
                }
            }
        }

    return (arc);
}

/*
 *@@ xprfQueryProfileData:
 *      similar to PrfQueryProfileData.
 *
 *      @@todo: Still to be written.
 */

/* BOOL xprfQueryProfileData(PXINI hIni,            // in: profile opened with xprfOpenProfile
                          const char *pcszApp,
                          const char *pcszKey,
                          PVOID pBuffer,
                          PULONG pulBufferMax)
{
    BOOL brc = FALSE;

    return (brc);
} */

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
 */

BOOL xprfWriteProfileData(PXINI hIni,           // in: profile opened with xprfOpenProfile
                          const char *pcszApp,  // in: application name
                          const char *pcszKey,  // in: key name or NULL
                          PVOID pData,          // in: data to write or NULL
                          ULONG ulDataLen)      // in: sizeof(*pData) or null
{
    BOOL brc = FALSE;

    if (hIni)
    {
        PXINI pXIni = (PXINI)hIni;
        if (memcmp(pXIni->acMagic, XINI_MAGIC_BYTES, sizeof(XINI_MAGIC_BYTES))
                        == 0)
        {
            // check if application exists
            PXINIAPPDATA pAppData = FindApp(pXIni,
                                            pcszApp);

            // now check: does caller want entire application deleted?
            if (!pcszKey)
            {
                // yes, delete application: did we find it?
                if (pAppData)
                {
                    // yes: kill that
                    FreeApp(pAppData);
                    // and remove from list
                    brc = lstRemoveItem(&pXIni->llApps, pAppData);
                    pXIni->cApps--;
                    // rewrite profile on close
                    pXIni->fDirty = TRUE;
                }
                else
                    // application doesn't exist:
                    brc = TRUE;
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
                        // find key
                        PXINIKEYDATA pKeyData = FindKey(pAppData,
                                                        pcszKey);
                        if (pKeyData)
                        {
                            // key exists: kill that
                            FreeKey(pKeyData);
                            // and remove from app's keys list
                            brc = lstRemoveItem(&pAppData->llKeys, pKeyData);
                            pAppData->cKeys--;
                            // rewrite profile on close
                            pXIni->fDirty = TRUE;
                        }
                        else
                            // key doesn't even exist:
                            brc = TRUE;
                    }
                    else
                        // app doesn't even exist:
                        brc = TRUE;
                }
                else
                {
                    // key and data specified: let's add something...

                    if (!pAppData)
                        // app doesn't exist yet:
                        // create
                        pAppData = CreateApp(pXIni,
                                             pcszApp);

                    if (pAppData)
                    {
                        // found or created app:
                        // check if key exists
                        PXINIKEYDATA pKeyData = FindKey(pAppData,
                                                        pcszKey);
                        if (!pKeyData)
                            // doesn't exist yet:
                            // create
                            pKeyData = CreateKey(pAppData,
                                                 pcszKey,
                                                 (PBYTE)pData,
                                                 ulDataLen);

                        if (pKeyData)
                        {
                           // mark as dirty
                           pXIni->fDirty = TRUE;
                           brc = TRUE;
                        }
                    }
                }
            }
        }
    }

    return (brc);
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
 */

BOOL xprfCloseProfile(PXINI hIni)       // in: profile opened with xprfOpenProfile
{
    BOOL brc = FALSE;

    if (hIni)
    {
        PXINI pXIni = (PXINI)hIni;
        if (memcmp(pXIni->acMagic, XINI_MAGIC_BYTES, sizeof(XINI_MAGIC_BYTES))
                        == 0)
        {
            brc = TRUE;

            if (pXIni->fDirty)
                brc = WriteINI(pXIni);

            if (brc)
            {
                APIRET arc = DosProtectClose(pXIni->hFile,
                                             pXIni->hLock);
                if (arc == NO_ERROR)
                {
                    pXIni->hFile = 0;
                    pXIni->hLock = 0;

                    FreeINI(pXIni);
                }
                else
                    brc = FALSE;
            }
        }
    }

    return (brc);
}


