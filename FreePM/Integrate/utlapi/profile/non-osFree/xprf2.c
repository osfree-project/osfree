
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

#define INCL_DOSERRORS
#define INCL_WINSHELLDATA
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"
#include "helpers\prfh.h"
#include "helpers\stringh.h"
#include "helpers\xprf.h"

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
                   PSZ pszSourceApp,      // in: source application
                   PSZ pszKey,            // in: source/target key
                   PXINI hiniTarget,      // in: target profile opened with xprfOpenProfile
                   PSZ pszTargetApp)      // in: target app
{
    ULONG   ulSizeOfData = 0;
    APIRET  arc = NO_ERROR;

    if (PrfQueryProfileSize(hiniSource, pszSourceApp, pszKey, &ulSizeOfData))
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
                                    pszSourceApp,
                                    pszKey,
                                    pData,
                                    &ulSizeOfData))
            {
                if (!xprfWriteProfileData(hiniTarget,
                                          pszTargetApp,
                                          pszKey,
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

    return (arc);
}

/*
 *@@ xprfCopyApp:
 *      copies a single application from an Prf* HINI to an xprf* PXINI.
 *      hiniTarget must therefore have been opened using
 *      xprfOpenProfile.
 *
 *      This calls xprfCopyKey for each key in the application.
 *
 *      This returns 0 (NO_ERROR) if copying succeeded. Otherwise either
 *      an OS/2 error code (ERROR_*) or one of the profile error
 *      codes defined in prfh.h is returned.
 */

APIRET xprfCopyApp(HINI hiniSource,   // in: source profile (can be HINI_USER or HINI_SYSTEM)
                   PSZ pszSourceApp,  // in: source application
                   PXINI hiniTarget,  // in: target profile opened with xprfOpenProfile
                   PSZ pszTargetApp,  // in: name of pszSourceApp in hiniTarget
                   PSZ pszErrorKey)   // out: failing key in case of error; ptr can be NULL
{
    APIRET arc = NO_ERROR;
    PSZ pszKeysList;

    if (pszErrorKey)
        *pszErrorKey = 0;

    pszKeysList = prfhQueryKeysForApp(hiniSource, pszSourceApp);
    if (pszKeysList)
    {
        PSZ pKey2 = pszKeysList;

        while (*pKey2 != 0)
        {
            // copy this key
            arc = xprfCopyKey(hiniSource,
                              pszSourceApp,
                              pKey2,
                              hiniTarget,
                              pszTargetApp);
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
    else
        arc = PRFERR_KEYSLIST;

    return (arc);
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
                       PSZ pszNew,          // in: new filename (can be fully qualified)
                       PFN_PRF_PROGRESS pfnProgressCallback,
                       ULONG ulUser,        // in: passed to pfnProgressCallback
                       ULONG ulCount,       // in: index of INI being copied (0 <= ulCount <= ulMax)
                       ULONG ulMax)         // in: maximum index (for progress); 0 means 1 INI, 1 means 2 INIs, ...
{
    APIRET  arc = NO_ERROR;
    PXINI   pxiniNew = NULL;
    ULONG   ulSizeOfAppsList;

    if (!pszNew)
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        DosDelete(pszNew);

        // open new profile
        arc = xprfOpenProfile(pszNew,
                              &pxiniNew);

        // get size of applications list
        if (arc == NO_ERROR)
        {
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
                    arc = xprfCopyApp(hOld,
                                      pApp2,
                                      pxiniNew,
                                      pApp2,
                                      szErrorKey);

                    if (pfnProgressCallback)
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

    return (arc);
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
                    ULONG ulUser)
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
            arc = xprfCopyProfile(HINI_SYSTEM,
                                  szSysNew,              // new filename
                                  pfnProgressCallback,
                                  ulUser, 0, 1);
        /*
         * create OS2SYS.XFL:
         *
         */

        if (arc == NO_ERROR)
            arc = xprfCopyProfile(HINI_USER,
                                  szUserNew,              // new filename
                                  pfnProgressCallback,
                                  ulUser, 1, 1);
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
        arc = DosMove(szSysNew, Profiles.pszSysName);
        if (arc == NO_ERROR)
            arc = DosMove(szUserNew, Profiles.pszUserName);
    }

    if (Profiles.pszSysName)
        free(Profiles.pszSysName);
    if (Profiles.pszUserName)
        free(Profiles.pszUserName);

    return (arc);
}

// testing

/* BOOL _Optlink fnCallback(ULONG ulUser, ULONG ulNow, ULONG ulMax)
{
    printf("\r done %03d%%", ulNow * 100 / ulMax);
    return (TRUE);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
        printf("Syntax: xprf2 <source.ini> <target.ini>\n");
    else
    {
        HAB hab = WinInitialize(0);

        // HINI hiniSource = PrfOpenProfile(hab, argv[1]);
        // if (hiniSource)
        {
            APIRET arc = xprfCopyProfile(hiniSource,
                                         argv[2],
                                         fnCallback,
                                         0, 0, 0);
            xprfSaveProfiles(hab, NULL, 0);
//             if (arc)


   //              printf("xprfCopyProfile returned %d.\n", arc);
        }
        // else
            // printf("Cannot open %s\n", argv[1]);

        WinTerminate(hab);
    }

    return (0);
} */

