
/*
 *@@sourcefile xprf2.c:
 *      copy-profile functions which use the replacement
 *      profile functions in xprf.c.
 *
 *      This can be used for a bomb-proof "save system profiles"
 *      (see xprfSaveINIs).
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

#define INCL_DOSPROFILE
#define INCL_DOSERRORS
#define INCL_WINSHELLDATA
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/dosh.h"
#include "helpers/prfh.h"
#include "helpers/stringh.h"
#include "helpers/xprf.h"

#pragma hdrstop

/*
 *@@category: Helpers\Profile (INI) replacement functions
 */

/* ******************************************************************
 *
 *   Copy API Functions
 *
 ********************************************************************/

/*
 *@@ xprfCopyKey:
 *      copies a single key from an Prf* HINI to an xprf* PXINI.
 *      hiniTarget must therefore have been opened using
 *      xprfOpenProfile.
 *
 *      This returns 0 (NO_ERROR) if copying succeeded. Otherwise either
 *      an OS/2 error code (ERROR_*) or one of the profile error
 *      codes defined in prfh.h is returned.
 */

APIRET xprfCopyKey(HINI hiniSource,       // in: source profile (can be HINI_USER or HINI_SYSTEM)
                   PCSZ pszSourceApp,     // in: source application
                   PCSZ pszKey,           // in: source/target key
                   PXINI hiniTarget,      // in: target profile opened with xprfOpenProfile
                   PCSZ pszTargetApp)     // in: target app
{
    ULONG   ulSizeOfData = 0;
    APIRET  arc = NO_ERROR;

    if (PrfQueryProfileSize(hiniSource,
                            (PSZ)pszSourceApp,
                            (PSZ)pszKey,
                            &ulSizeOfData))
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
            fflush(stdout);
            if (PrfQueryProfileData(hiniSource,
                                    (PSZ)pszSourceApp,
                                    (PSZ)pszKey,
                                    pData,
                                    &ulSizeOfData))
            {
                arc = xprfWriteProfileData(hiniTarget,
                                           pszTargetApp,
                                           pszKey,
                                           pData,
                                           ulSizeOfData);
            }
            else
                arc = PRFERR_READ;

            free(pData);
        }
        else
            arc = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
        arc = PRFERR_DATASIZE;

    return arc;
}

/*
 *@@ xprfCopyKey2:
 *      copies a single key from an xprf* PXINI to a Prf* HINI.
 *      hiniSource must therefore have been opened using
 *      xprfOpenProfile.
 *
 *      This returns 0 (NO_ERROR) if copying succeeded. Otherwise either
 *      an OS/2 error code (ERROR_*) or one of the profile error
 *      codes defined in prfh.h is returned.
 *
 *@@added V1.0.0 (2002-09-17) [umoeller]
 */

APIRET xprfCopyKey2(PXINI hiniSource,   // in: source profile (can be HINI_USER or HINI_SYSTEM)
                    PCSZ pszSourceApp,  // in: source application
                    PCSZ pszKey,        // in: source/target key
                    HINI hiniTarget,    // in: target profile opened with xprfOpenProfile
                    PCSZ pszTargetApp)  // in: target app
{
    ULONG   ulSizeOfData = 0;
    APIRET  arc = NO_ERROR;

    if (xprfQueryProfileSize(hiniSource, pszSourceApp, pszKey, &ulSizeOfData))
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
            fflush(stdout);
            if (xprfQueryProfileData(hiniSource,
                                     pszSourceApp,
                                     pszKey,
                                     pData,
                                     &ulSizeOfData))
            {
                if (!PrfWriteProfileData(hiniTarget,
                                         (PSZ)pszTargetApp,
                                         (PSZ)pszKey,
                                         pData,
                                         ulSizeOfData))
                    arc = PRFERR_WRITE;
            }
            else
                arc = PRFERR_READ;

            free(pData);
        }
        else
            arc = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
        arc = PRFERR_DATASIZE;

    return arc;
}

/*
 *@@ xprfCopyApp:
 *      copies a single application from a Prf* HINI to an xprf* PXINI.
 *      hiniTarget must therefore have been opened using
 *      xprfOpenProfile.
 *
 *      This calls xprfCopyKey for each key in the application.
 *
 *      This returns 0 (NO_ERROR) if copying succeeded. Otherwise either
 *      an OS/2 error code (ERROR_*) or one of the profile error
 *      codes defined in prfh.h is returned.
 */

APIRET xprfCopyApp(HINI hiniSource,     // in: source profile (can be HINI_USER or HINI_SYSTEM)
                   PCSZ pszSourceApp,   // in: source application
                   PXINI hiniTarget,    // in: target profile opened with xprfOpenProfile
                   PCSZ pszTargetApp,   // in: name of pszSourceApp in hiniTarget
                   PSZ pszErrorKey)     // out: failing key in case of error; ptr can be NULL
{
    APIRET arc = NO_ERROR;
    PSZ pszKeysList = NULL;

    if (pszErrorKey)
        *pszErrorKey = 0;

    if (!(arc = prfhQueryKeysForApp(hiniSource,
                                    pszSourceApp,
                                    &pszKeysList)))
    {
        PSZ pKey2 = pszKeysList;

        while (*pKey2 != 0)
        {
            // copy this key
            if (arc = xprfCopyKey(hiniSource,
                                  pszSourceApp,
                                  pKey2,
                                  hiniTarget,
                                  pszTargetApp))
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
    else
        arc = PRFERR_KEYSLIST;

    return arc;
}

/*
 *@@ xprfCopyApp2:
 *      copies a single application from an xprf* PXINI to a Prf* HINI.
 *      hiniTarget must therefore have been opened using
 *      xprfOpenProfile.
 *
 *      This calls xprfCopyKey2 for each key in the application.
 *
 *      This returns 0 (NO_ERROR) if copying succeeded. Otherwise either
 *      an OS/2 error code (ERROR_*) or one of the profile error
 *      codes defined in prfh.h is returned.
 *
 *@@added V1.0.0 (2002-09-17) [umoeller]
 */

APIRET xprfCopyApp2(PXINI hiniSource,   // in: source profile (can be HINI_USER or HINI_SYSTEM)
                    PCSZ pszSourceApp,  // in: source application
                    HINI hiniTarget,    // in: target profile opened with xprfOpenProfile
                    PCSZ pszTargetApp,  // in: name of pszSourceApp in hiniTarget
                    PSZ pszErrorKey)    // out: failing key in case of error; ptr can be NULL
{
    APIRET arc = NO_ERROR;
    PSZ pszKeysList = NULL;

    if (pszErrorKey)
        *pszErrorKey = 0;

    if (!(arc = xprfQueryKeysForApp(hiniSource,
                                    pszSourceApp,
                                    &pszKeysList)))
    {
        PSZ pKey2 = pszKeysList;

        while (*pKey2 != 0)
        {
            // copy this key
            if (arc = xprfCopyKey2(hiniSource,
                                   pszSourceApp,
                                   pKey2,
                                   hiniTarget,
                                   pszTargetApp))
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
    else
        arc = PRFERR_KEYSLIST;

    return arc;
}

/*
 *@@ xprfCopyProfile:
 *      this copies an entire profile.
 *
 *      The source profile must have been opened using the
 *      regular OS/2 PrfOpenProfile. You can also specify
 *      HINI_USER or HINI_SYSTEM.
 *
 *      pszNew specifies the file name for the new profile.
 *      This must end in *.INI. The new profile is opened
 *      using xprfOpenProfile, so see additional remarks there.
 *
 *      This returns 0 (NO_ERROR) on success. Otherwise either
 *      an OS/2 error code (ERROR_*) or one of the profile error
 *      codes defined in prfh.h is returned.
 */

APIRET xprfCopyProfile(HINI hOld,           // in: source profile (can be HINI_USER or HINI_SYSTEM)
                       PCSZ pszNew,         // in: new filename (can be fully qualified)
                       PFN_PRF_PROGRESS pfnProgressCallback,
                       ULONG ulUser,        // in: passed to pfnProgressCallback
                       ULONG ulCount,       // in: index of INI being copied (0 <= ulCount <= ulMax)
                       ULONG ulMax,         // in: maximum index (for progress); 0 means 1 INI, 1 means 2 INIs, ...
                       PSZ pszFailingApp)   // out: failing app on error
{
    APIRET  arc = NO_ERROR;
    PXINI   pxiniNew = NULL;
    ULONG   ulSizeOfAppsList;

    if (pszFailingApp)
        *pszFailingApp = 0;

    if (!pszNew)
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        DosDelete((PSZ)pszNew);

        // open new profile
        if (!(arc = xprfOpenProfile(pszNew,
                                    &pxiniNew)))
        {
            // get size of applications list
            if (!PrfQueryProfileSize(hOld, NULL, NULL, &ulSizeOfAppsList))
                arc = PRFERR_APPSLIST;
            else
                if (ulSizeOfAppsList == 0)
                    arc = PRFERR_APPSLIST;

            if (arc == NO_ERROR)
            {
                // get applications list
                PSZ pApps = (PSZ)malloc(ulSizeOfAppsList);
                PSZ pApp2 = pApps;
                if (!PrfQueryProfileData(hOld,
                                         NULL,
                                         NULL,
                                         pApps,
                                         &ulSizeOfAppsList))
                    arc = PRFERR_APPSLIST;

                // applications loop

                while (   (*pApp2 != 0)
                       && (arc == NO_ERROR)
                      )
                {
                    CHAR szErrorKey[1000];

                    // copy application (this will call prfhCopyKey in turn)
                    if (arc = xprfCopyApp(hOld,
                                          pApp2,
                                          pxiniNew,
                                          pApp2,
                                          szErrorKey))
                    {
                        if (pszFailingApp)
                            strhncpy0(pszFailingApp, pApp2, CCHMAXPATH);
                    }
                    else if (pfnProgressCallback)
                    {
                        ULONG ulNow2, ulMax2;
                        ulNow2 = ((1000*(pApp2-pApps)) / ulSizeOfAppsList) + (ulCount*1000);
                        ulMax2 = (ulMax+1)*1000;
                        if (!pfnProgressCallback(ulUser, ulNow2, ulMax2))
                            // aborted:
                            arc = PRFERR_ABORTED;
                    }

                    // go for next app
                    pApp2 += strlen(pApp2)+1;

                } // end while (*pApp2 != 0) && MBID_NOERROR

                if (pApps)
                    free(pApps);
            }

            xprfCloseProfile(pxiniNew);

            // progress
            if (pfnProgressCallback)
                pfnProgressCallback(ulUser, (ulCount+1) * 1000, (ulMax+1) * 1000);
        }
    }

    return arc;
}

/*
 *@@ xprfSaveINIs:
 *      this rewrites the OS/2 user and system profiles
 *      (OS2.INI and OS2SYS.INI) to disk. This is done
 *      by doing the following:
 *
 *      -- First, both profiles are dumped into two temporary
 *         profiles (using xprfCopyProfile on each of them).
 *         These are put into the same directory as the source
 *         profiles (?:\OS2 normally) with a file name extension
 *         of OS2*.XFL.
 *
 *      -- Only if that succeeded, the original profiles are
 *         renamed to OS2*.BAK. Existing OS2*.BAK files are
 *         deleted.
 *
 *      -- If that succeded, the temporary OS2*.XFL files are
 *         renamed to OS2*.INI.
 *
 *      This is now used during XShutdown to dump the system
 *      profiles before shutting down the system. As opposed to
 *      prfhSaveINIs, which was previously used, this does not
 *      use the Prf* functions for the target profiles so that
 *      those ugly "Error saving INI files" errors are less likely.
 *
 *      This returns 0 (NO_ERROR) if copying succeeded. Otherwise either
 *      an OS/2 error code (ERROR_*) or one of the profile error
 *      codes defined in prfh.h is returned.
 */

APIRET xprfSaveINIs(HAB hab,               // in:  anchor block
                    PFN_PRF_PROGRESS pfnProgressCallback,
                    ULONG ulUser,
                    PSZ pszFailingINI,
                    PSZ pszFailingApp,
                    PSZ pszFailingKey)
{
    PRFPROFILE Profiles;
    APIRET  arc = NO_ERROR;

    // FILESTATUS3 fs3;
    CHAR    szSysNew[CCHMAXPATH],
            szUserNew[CCHMAXPATH],
            szSysBackup[CCHMAXPATH],
            szUserBackup[CCHMAXPATH];

    /*
     * get system profiles:
     *
     */

    Profiles.cchUserName = Profiles.cchSysName = 0;
    if (!PrfQueryProfile(hab, &Profiles))
        arc = PRFERR_QUERY;
    else
    {
        Profiles.pszUserName  = (PSZ)malloc(Profiles.cchUserName);
        Profiles.pszSysName  = (PSZ)malloc(Profiles.cchSysName);
        if (    (Profiles.pszSysName == NULL)
             || (Profiles.pszUserName == NULL)
           )
            arc = PRFERR_QUERY;
        else
            if (!PrfQueryProfile(hab, &Profiles))
                arc = PRFERR_QUERY;
    }

    if (arc == NO_ERROR)
    {
        PSZ _p;

        /*
         * create new profile names:
         *      same as old profiles, but with *.XFL ext.
         */

        // system INI
        strcpy(szSysBackup, Profiles.pszSysName);
        strcpy(szSysNew, Profiles.pszSysName);
        _p = strhistr(szSysBackup, ".INI");
        if (!_p)
            arc = PRFERR_INVALID_FILE_NAME;
        else
            strcpy(_p, ".BAK");
        _p = strhistr(szSysNew, ".INI");
        if (!_p)
            arc = PRFERR_INVALID_FILE_NAME;
        else
            strcpy(_p, ".XFL");

        // user INI
        strcpy(szUserBackup, Profiles.pszUserName);
        strcpy(szUserNew, Profiles.pszUserName);
        _p = strhistr(szUserBackup, ".INI");
        if (!_p)
            arc = PRFERR_INVALID_FILE_NAME;
        else
            strcpy(_p, ".BAK");
        _p = strhistr(szUserNew, ".INI");
        if (!_p)
            arc = PRFERR_INVALID_FILE_NAME;
        else
            strcpy(_p, ".XFL");

        /*
         * create OS2SYS.XFL:
         *
         */

        if (arc == NO_ERROR)
        {
            if (pszFailingINI)
                strcpy(pszFailingINI,
                       szSysNew);

            arc = xprfCopyProfile(HINI_SYSTEM,
                                  szSysNew,              // new filename
                                  pfnProgressCallback,
                                  ulUser,
                                  0,
                                  1,
                                  pszFailingApp);
        }

        /*
         * create OS2SYS.XFL:
         *
         */

        if (arc == NO_ERROR)
        {
            if (pszFailingINI)
                strcpy(pszFailingINI,
                       szUserNew);

            arc = xprfCopyProfile(HINI_USER,
                                  szUserNew,              // new filename
                                  pfnProgressCallback,
                                  ulUser,
                                  1,
                                  1,
                                  pszFailingApp);
        }
    }

    /*
     * renaming stuff for OS2SYS.INI
     *
     */

    if (arc == NO_ERROR)
    {
        // attrib -r -s -h -a OS2SYS.BAK
        doshSetPathAttr(szSysBackup, FILE_NORMAL);
        // delete OS2SYS.BAK
        DosDelete(szSysBackup);
        // attrib -r -s -h -a OS2SYS.INI
        doshSetPathAttr(Profiles.pszSysName, FILE_NORMAL);
        // REN OS2SYS.INI OS2SYS.BAK
        DosMove(Profiles.pszSysName, szSysBackup);
    }

    /*
     * renaming stuff for OS2.INI
     *
     */

    if (arc == NO_ERROR)
    {
        // attrib -r -s -h -a OS2SYS.BAK
        doshSetPathAttr(szUserBackup, FILE_NORMAL);
        // delete OS2SYS.BAK
        DosDelete(szUserBackup);
        // attrib -r -s -h -a OS2SYS.INI
        doshSetPathAttr(Profiles.pszUserName, FILE_NORMAL);
        // REN OS2SYS.INI OS2SYS.BAK
        DosMove(Profiles.pszUserName, szUserBackup);
    }

    if (arc == NO_ERROR)
    {
        // finally, replace system profiles
        if (!(arc = DosMove(szSysNew, Profiles.pszSysName)))
        {
            if (arc = DosMove(szUserNew, Profiles.pszUserName))
            {
                if (pszFailingINI)
                    strcpy(pszFailingINI,
                           szUserNew);
            }
        }
        else
        {
            if (pszFailingINI)
                strcpy(pszFailingINI,
                       szSysNew);
        }
    }

    if (Profiles.pszSysName)
        free(Profiles.pszSysName);
    if (Profiles.pszUserName)
        free(Profiles.pszUserName);

    return arc;
}

// testing

#ifdef __BUILD_XPRF2_MAIN__

BOOL _Optlink fnCallback(ULONG ulUser, ULONG ulNow, ULONG ulMax)
{
    printf("\r done %03d%%", ulNow * 100 / ulMax);
    return TRUE;
}

int main(int argc, char* argv[])
{
    APIRET  arc = 2;

    HAB     hab = WinInitialize(0);

    CHAR    szFailingINI[CCHMAXPATH] = "",
            szFailingApp[CCHMAXPATH] = "",
            szFailingKey[CCHMAXPATH] = "";

    arc = xprfSaveINIs(hab,
                       NULL,
                       0,
                       szFailingINI,
                       szFailingApp,
                       szFailingKey);

    printf("xprfCopyProfile returned %d ('%s', '%s', '%s'.\n",
           arc,
           szFailingINI,
           szFailingApp,
           szFailingKey);

    WinTerminate(hab);

    return arc;
}

#endif

