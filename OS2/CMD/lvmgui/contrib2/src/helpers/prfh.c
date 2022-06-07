
/*
 *@@sourcefile prfh.c:
 *      contains those Presentation Manager helper functions
 *      which deal with Profile (Prf*) functions.
 *      This file is new with V0.82.
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes:
 *      --  prfh*   Prf (profile, INI) helper functions
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\prfh.h"
 */

/*
 *      Copyright (C) 1997-2000 Ulrich M”ller.
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

#define INCLUDE_PRFH_PRIVATE
#include "helpers\prfh.h"

#pragma hdrstop

/*
 *@@category: Helpers\Profile (INI) helpers
 */

/*
 *@@ prfhQueryKeysForApp:
 *      returns the keys list for an INI application. This
 *      list is copied into a newly allocated buffer, of which
 *      the address is returned.
 *
 *      Returns NULL upon errors.
 *
 *      If the return value is != NULL, the PSZ points to a new
 *      buffer which contains all the keys within the pszApp
 *      application. Each key name in the list is terminated with
 *      a null character. The last string in the list is terminated
 *      with two null characters.
 *
 *      The returned buffer should be freed later using free().
 *
 *      <B>Example</B> for iterating over a keys list:
 *
 +          PSZ pszKeysList = prfhQueryKeysForApp(...);
 +          if (pszKeysList)
 +          {
 +              PSZ pKey2 = pszKeysList;
 +
 +              while (*pKey2 != 0)
 +              {
 +                  ...     // pKey2 has the current key now
 +                  pKey2 += strlen(pKey2)+1; // next key
 +              }
 +              free(pszKeysList);
 +          }
 *
 *      You can also use this function to query the applications
 *      list for hIni, if you specifiy pszApp as NULL.
 *
 *@@changed V0.9.12 (2001-05-12) [umoeller]: changed prototypes to return APIRET now
 *@@changed V0.9.19 (2002-04-11) [pr]: Fixed app. with no keys
 */

APIRET prfhQueryKeysForApp(HINI hIni,      // in: INI handle (can be HINI_USER or HINI_SYSTEM)
                           const char *pcszApp, // in: application to query list for (or NULL for applications list)
                           PSZ *ppszKeys)   // out: keys list (newly allocated)
{
    APIRET  arc = NO_ERROR;
    PSZ     pKeys = NULL;
    ULONG   ulSizeOfKeysList = 0;

    // get size of keys list for pszApp
    if (!PrfQueryProfileSize(hIni, (PSZ)pcszApp, NULL, &ulSizeOfKeysList))
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
            if (!PrfQueryProfileData(hIni, (PSZ)pcszApp, NULL, pKeys, &ulSizeOfKeysList))
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

#ifdef __DEBUG_MALLOC_ENABLED__ // setup.h, helpers\memdebug.c

/*
 *@@ prfhQueryProfileDataDebug:
 *      debug version of prfhQueryProfileData, which is
 *      automatically mapped to if __XWPMEMDEBUG__ is defined.
 *
 *@@added V0.9.1 (99-12-20) [umoeller]
 *@@changed V0.9.3 (2000-04-20) [umoeller]: this called malloc(0) if the key existed, but was empty. Fixed.
 */

PSZ (prfhQueryProfileDataDebug)(HINI hIni,      // in: INI handle (can be HINI_USER or HINI_SYSTEM)
                              const char *pcszApp,      // in: application to query
                              const char *pcszKey,      // in: key to query
                              PULONG pcbBuf,   // out: size of the returned buffer
                              const char *file,
                              unsigned long line,
                              const char *function)
{
    PSZ     pData = NULL;
    ULONG   ulSizeOfData = 0;

    // get size of data for pszApp/pszKey
    if (PrfQueryProfileSize(hIni, (PSZ)pcszApp, (PSZ)pcszKey, &ulSizeOfData))
    {
        if (ulSizeOfData)
        {
            pData = (PSZ)memdMalloc(ulSizeOfData, file, line, function);
            if (!PrfQueryProfileData(hIni, (PSZ)pcszApp, (PSZ)pcszKey, pData, &ulSizeOfData))
            {
                free(pData);
                pData = NULL;
            }
        }
    }

    if (pcbBuf)
        *pcbBuf = ulSizeOfData;

    return pData;
}

#else

/*
 *@@ prfhQueryProfileData:
 *      similar to PrfQueryProfileData, but this one copies
 *      the data into a newly allocated buffer, of which the
 *      address is returned.
 *
 *      Returns NULL upon errors, for example if the specified
 *      key doesn't exist or doesn't contain any data.
 *
 *      If pcbBuf != NULL, this func will write the size of
 *      the allocated buffer into *pcbBuf.
 *
 *      The returned buffer should be freed later using free().
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.3 (2000-04-20) [umoeller]: this called malloc(0) if the key existed, but was empty. Fixed.
 */

PSZ (prfhQueryProfileData)(HINI hIni,           // in: INI handle (can be HINI_USER or HINI_SYSTEM)
                         const char *pcszApp,   // in: application to query
                         const char *pcszKey,   // in: key to query
                         PULONG pcbBuf)         // out: size of the returned buffer; ptr can be NULL
{
    PSZ     pData = NULL;
    ULONG   ulSizeOfData;

    // get size of data for pszApp/pszKey
    if (    (PrfQueryProfileSize(hIni, (PSZ)pcszApp, (PSZ)pcszKey, &ulSizeOfData))
         && (ulSizeOfData)
         && (pData = (PSZ)malloc(ulSizeOfData))
       )
    {
        if (!PrfQueryProfileData(hIni, (PSZ)pcszApp, (PSZ)pcszKey, pData, &ulSizeOfData))
        {
            free(pData);
            pData = NULL;
        }
    }

    if (pcbBuf)
        *pcbBuf = ulSizeOfData;

    return pData;
}

#endif

/*
 *@@ prfhQueryProfileChar:
 *      this query the first character of profile data.
 *      This is mostly useful with the PM country settings
 *      in OS2.INI:
 *      -- date separator: "PM_National", "sDate"
 *      -- time separator: "PM_National", "sTime"
 */

CHAR prfhQueryProfileChar(HINI hini,        // in: INI handle (can be HINI_USER or HINI_SYSTEM)
                          const char *pcszApp,       // in: application to query
                          const char *pcszKey,       // in: key to query
                          CHAR cDefault)    // in: default to return if not found
{
    CHAR    szTemp[5],
            szDefault[5];
    szDefault[0] = cDefault;
    szDefault[1] = 0;
    PrfQueryProfileString(HINI_USER, (PSZ)pcszApp, (PSZ)pcszKey,
                          szDefault,
                          szTemp, sizeof(szTemp)-1);
    return szTemp[0];
}

/*
 *@@ prfhQueryColor:
 *      returns a system color in OS2.INI's PM_Colors as a LONG.
 */

LONG prfhQueryColor(const char *pcszKeyName,
                    const char *pcszDefault)
{
    CHAR szColor[30];
    ULONG r, g, b;
    PrfQueryProfileString(
                HINI_USER,
                "PM_Colors",
                (PSZ)pcszKeyName,
                (PSZ)pcszDefault,
                szColor,
                sizeof(szColor)-1);
    sscanf(szColor, "%lu %lu %lu ", &r, &g, &b);
    return (LONG)(r * 0x10000 + g * 0x100 + b);
}

/*
 *@@ prfhCopyKey:
 *      this copies one key from the given profile and application
 *      to another one.
 *
 *      pszTargetApp may be in the same profile (and must be
 *      different from pszSourceApp then) or in a different
 *      profile (and can be the same then).
 *
 *      You must specify all parameters. You cannot specify pszKey
 *      as NULL to have a whole application copied. Use prfhCopyApp
 *      for that.
 *      No check is made for this.
 *
 *      Returns:
 *      --  0: no error
 *
 *      --  PRFERR_DATASIZE: couldn't query data size for key
 *
 *      --  PRFERR_MEMORY: couldn't allocate memory
 *
 *      --  PRFERR_READ: couldn't read data from source (PrfQueryProfileData error)
 *
 *      --  PRFERR_WRITE: couldn't write data to target (PrfWriteProfileData error)
 *
 *@@added V0.9.0 [umoeller]
 */

APIRET prfhCopyKey(HINI hiniSource,       // in: source profile (can be HINI_USER or HINI_SYSTEM)
                   const char *pcszSourceApp,      // in: source application
                   const char *pcszKey,            // in: source/target key
                   HINI hiniTarget,       // in: target profile (can be HINI_USER or HINI_SYSTEM)
                   const char *pcszTargetApp)      // in: target app
{
    ULONG   ulSizeOfData = 0,
            ulrc = 0;       // return: no error

    if (PrfQueryProfileSize(hiniSource, (PSZ)pcszSourceApp, (PSZ)pcszKey, &ulSizeOfData))
    {
        PSZ pData = 0;

        // copy data
        if (ulSizeOfData == 0)
        {
            // data size == 0: this shouldn't really happen,
            // but if it does, we'll just create a NULL string.
            // Users have reported that some INI files seem to
            // contain those "empty" keys. I don't see how these
            // can exist, but they seem to...
            pData = (PSZ)malloc(1);
            *pData = 0;
        }
        else
            pData = (PSZ)malloc(ulSizeOfData);

        if (pData)
        {
            if (PrfQueryProfileData(hiniSource,
                                    (PSZ)pcszSourceApp,
                                    (PSZ)pcszKey,
                                    pData,
                                    &ulSizeOfData))
            {
                if (!PrfWriteProfileData(hiniTarget,
                                         (PSZ)pcszTargetApp,
                                         (PSZ)pcszKey,
                                         pData,
                                         ulSizeOfData))
                    ulrc = PRFERR_WRITE;
            }
            else
                ulrc = PRFERR_READ;

            free(pData);
        }
        else
            ulrc = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
        ulrc = PRFERR_DATASIZE;

    return ulrc;
}

/*
 *@@ prfhCopyApp:
 *      this copies one key from the given profile and application
 *      to another one.
 *
 *      You can use this function in several contexts:
 *
 *      -- copy one application within the same profile
 *         (i.e. hiniSource == hiniTarget);
 *         in this case, pszSourceApp must be != pszTargetApp;
 *
 *      -- copy an application from one profile to another
 *         (i.e. hiniSource != hiniTarget);
 *         in this case, pszSourceApp can be == pszTargetApp
 *         (but can be different also).
 *
 *      WARNING: This does _not_ check for whether the target
 *      application exists already. This has two consequences:
 *
 *      --  existing data will be overwritten without warning;
 *
 *      --  if the existing target application has keys that are
 *          not in the source application, they are not deleted.
 *          As a result, you might end up with more keys than
 *          in the source application.
 *
 *      So you should delete the target application before
 *      calling this function, like this:
 *
 +          PrfWriteProfileString(hiniTarget, pszTargetApp, NULL, NULL);
 *
 *      You must specify all parameters. You cannot specify pszApp
 *      as NULL to have a whole profile copied. Use prfhCopyProfile
 *      for that.
 *      No check is made for this.
 *
 *      Returns:
 *
 *      --  0: no error
 *
 *      --  PRFERR_KEYSLIST: couldn't query keys for pszSourceApp
 *
 *      --  PRFERR_DATASIZE: couldn't query data size for key
 *
 *      --  PRFERR_MEMORY: couldn't allocate memory
 *
 *      --  PRFERR_READ: couldn't read data from source (PrfQueryProfileData error)
 *
 *      --  PRFERR_WRITE: couldn't write data to target (PrfWriteProfileData error)
 *
 *@@added V0.9.0 [umoeller]
 */

APIRET prfhCopyApp(HINI hiniSource,   // in: source profile (can be HINI_USER or HINI_SYSTEM)
                   const char *pcszSourceApp,  // in: source application
                   HINI hiniTarget,   // in: target profile (can be HINI_USER or HINI_SYSTEM)
                   const char *pcszTargetApp,  // in: name of pszSourceApp in hiniTarget
                   PSZ pszErrorKey)   // out: failing key in case of error; ptr can be NULL
{
    APIRET  arc = NO_ERROR;
    PSZ     pszKeysList = NULL;

    if (pszErrorKey)
        *pszErrorKey = 0;

    if (!(arc = prfhQueryKeysForApp(hiniSource,
                                    (PSZ)pcszSourceApp,
                                    &pszKeysList)))
    {
        PSZ pKey2 = pszKeysList;

        while (*pKey2 != 0)
        {
            // copy this key
            arc = prfhCopyKey(hiniSource,
                              pcszSourceApp,
                              pKey2,
                              hiniTarget,
                              pcszTargetApp);
            if (arc)
            {
                // error: copy failing key to buffer
                if (pszErrorKey)
                    strcpy(pszErrorKey, pKey2);
                break;
            }
            pKey2 += strlen(pKey2)+1;
        } // end while (*pKey2 != 0)

        free (pszKeysList);
    }

    return arc;
}

/*
 *@@ prfhRenameKey:
 *      renames a key in an INI file.
 *
 *      Since there's no such thing as a PrfRename,
 *      what we do here is load the old data, write
 *      it under a new key, and delete the old data.
 *
 *      Returns:
 *
 *      --  0: no error
 *
 *      --  PRFERR_INVALID_KEY: pcszApp or pcszOldKey do not exist.
 *
 *      --  PRFERR_KEY_EXISTS: pcszNewApp/pcszNewKey is already occupied.
 *
 *      --  PRFERR_WRITE: couldn't write data to target (PrfWriteProfileData error)
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 */

ULONG prfhRenameKey(HINI hini,
                    const char *pcszOldApp,
                    const char *pcszOldKey, // in: key to rename
                    const char *pcszNewApp, // in: new app (if NULL, pcszOldApp is used)
                    const char *pcszNewKey) // in: new name for pcszOldKey
{
    ULONG ulrc = 0;

    ULONG   cbData = 0;
    PSZ pszData = prfhQueryProfileData(hini,
                                       pcszOldApp,
                                       pcszOldKey,
                                       &cbData);
    if (!pszData)
        // not found:
        ulrc = PRFERR_INVALID_KEY;
    else
    {
        ULONG   cb;

        if (!pcszNewApp)
            // is NULL:
            pcszNewApp = pcszOldApp;

        // make sure target doesn't exist
        if (    (PrfQueryProfileSize(hini,
                                     (PSZ)pcszNewApp,
                                     (PSZ)pcszNewKey,
                                     &cb))
             && (cb)
           )
            ulrc = PRFERR_KEY_EXISTS;
        else
        {
            if (!PrfWriteProfileData(hini,
                                     (PSZ)pcszNewApp,
                                     (PSZ)pcszNewKey,
                                     pszData,
                                     cbData))
                ulrc = PRFERR_WRITE;
            else
            {
                // success writing:
                // delete old
                PrfWriteProfileData(hini,
                                    (PSZ)pcszOldApp,
                                    (PSZ)pcszOldKey,
                                    NULL,
                                    0);
            }
        }

        free(pszData);
    }

    return ulrc;
}

/*
 *@@ prfhSetUserProfile:
 *      calls PrfReset to change the current user
 *      profile (normally OS2.INI) to the specified
 *      INI file.
 *
 *@@added V0.9.4 (2000-07-19) [umoeller]
 *@@changed V0.9.19 (2002-04-02) [umoeller]: now returning APIRET
 */

APIRET prfhSetUserProfile(HAB hab,
                          const char *pcszUserProfile)     // in: new user profile (.INI)
{
    APIRET arc = NO_ERROR;

    // find out current profile names
    PRFPROFILE Profiles;
    Profiles.cchUserName = Profiles.cchSysName = 0;
    // first query their file name lengths
    if (PrfQueryProfile(hab, &Profiles))
    {
        // allocate memory for filenames
        Profiles.pszUserName  = (PSZ)malloc(Profiles.cchUserName);
        Profiles.pszSysName  = (PSZ)malloc(Profiles.cchSysName);

        if ((Profiles.pszSysName) && (Profiles.pszUserName))
        {
            // get filenames
            if (PrfQueryProfile(hab, &Profiles))
            {
                // _Pmpf(("Old user profile: %s", Profiles.pszUserName));

                // change INIs
                free(Profiles.pszUserName);
                Profiles.pszUserName = (PSZ)pcszUserProfile;
                Profiles.cchUserName = strlen(pcszUserProfile) + 1;
                if (!PrfReset(hab, &Profiles))
                    arc = PRFERR_RESET;
                free(Profiles.pszSysName);
            }
            else
                arc = PRFERR_QUERY;
        }
        else
            arc = PRFERR_QUERY;
    }
    else
        arc = PRFERR_QUERY;

    return arc;
}


