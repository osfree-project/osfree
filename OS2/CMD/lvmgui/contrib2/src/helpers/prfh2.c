
/*
 *@@sourcefile prfh2.c:
 *      contains more Presentation Manager helper functions
 *      which deal with Profile (Prf*) functions.
 *
 *      This file is new with V0.9.4 (2000-07-26) [umoeller].
 *
 *      As opposed to the functions in prfh.c, these require
 *      linking against other helpers. As a result, these have
 *      been separated from prfh.c to allow linking against
 *      prfh.obj only.
 *
 *      Function prefixes:
 *      --  prfh*   Prf (profile, INI) helper functions
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\prfh.h"
 *@@added V0.9.4 (2000-07-27) [umoeller]
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

#define INCL_DOSPROFILE
#define INCL_DOSERRORS
#define INCL_WINSHELLDATA
#define INCL_WINERRORS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"
#include "helpers\stringh.h"

#include "helpers\prfh.h"
#include "helpers\xprf.h"

#pragma hdrstop

/*
 *@@category: Helpers\Profile (INI) helpers
 */

/*
 * prfhINIError:
 *      this is called whenever an error occurs saving a
 *      profile. This will compose an error string and
 *      all the callback func fncbError.
 *
 *@@changed V0.9.0 [umoeller]: changed prototype's logfile to stdio FILE*
 */

ULONG prfhINIError(ULONG ulOptions,
                   FILE* fLog,
                   PFNWP fncbError,
                   PSZ pszErrorString)
{
    ULONG ulrc;
    CHAR szError2[2000];

    if (fncbError)
    {
        if (ulOptions == MB_ABORTRETRYIGNORE)
            sprintf(szError2, "%s"
                              "\nPress 'Abort' to abort saving the INI files. This will restart the WPS to avoid loss of data."
                              "\nPress 'Retry' to attempt saving this INI file again. "
                              "\nPress 'Ignore' to ignore this error. This might help, but "
                              "also lead to more errors or loss of the currently processed data.",
                              pszErrorString);
        else
            sprintf(szError2, "%s\nPress 'Cancel' to abort shutdown. ", pszErrorString);
        ulrc = ( (ULONG)(*fncbError)(0, 0, szError2, (MPARAM)ulOptions) );

    }
    else
        ulrc = (MBID_ABORT);

    if (fLog)
    {
        fprintf(fLog, "    Error occurred: %s\n    Return code: %s (%lu)\n",
                pszErrorString,
                    (ulrc == MBID_ABORT) ? "MBID_ABORT"
                    : (ulrc == MBID_IGNORE) ? "MBID_IGNORE"
                    : (ulrc == MBID_RETRY) ? "MBID_RETRY"
                    : "unknown",
                ulrc);
        fflush(fLog);
    }

    return ulrc;
}

/*
 * prfhINIError2:
 *      like the previous func, but with additional info
 *      for the error string.
 *
 *@@changed V0.9.0 [umoeller]: changed prototype's logfile to stdio FILE*
 */

ULONG prfhINIError2(ULONG ulOptions,
                    const char *pcszINI,
                    FILE* fLog,
                    PFNWP fncbError,
                    PSZ pszErrorString)
{
    CHAR szError2[2000];
    sprintf(szError2, "An error occurred copying the profile %s: \n%s",
            pcszINI, pszErrorString);
    return prfhINIError(ulOptions, fLog, fncbError, szError2);
}

/*
 *@@ prfhCopyProfile:
 *      this function copies a given profile entirely into a
 *      new file. hOld is the handle, pszOld the filename
 *      of the profile to be copied (e.g. HINI_USERPROFILE
 *      and "?:\OS2\OS2.INI"; pszNew must point to a
 *      buffer which will contain the new filename ending
 *      in ".XFL" after prfhCopyProfile returns
 *      (e.g. "?:\OS2\OS2.XFL").
 *
 *      You may specify a callback procedure which gets
 *      called upon every copied application in the INI
 *      file; this way, you can provide a progress bar.
 *      The second callback, fncbError, gets called upon
 *      errors.
 *
 *@@changed V0.9.0 [umoeller]: this now calls prfhCopyApp.
 *@@changed V0.9.0 [umoeller]: fixed crashes with lower-case INI file names.
 *@@changed V0.9.0 [umoeller]: changed prototype's logfile to stdio FILE*
 *@@changed V0.9.4 (2000-07-15) [umoeller]: this never reported any errors, causing 0-byte INI files without warning. Fixed.
 */

BOOL prfhCopyProfile(HAB hab,               // in:  anchor block
                     FILE* fLog,            // in:  stdio logfile or NULL for no log
                     HINI hOld,             // in:  HINI to copy
                     PSZ pszOld,            // in:  fully qualif. filename of hOld
                     PSZ pszNew,            // out: new filename
                     PFNWP fncbUpdate,      // in:  progress callback
                     HWND hwnd, ULONG msg, ULONG ulCount, ULONG ulMax,
                                            // in:  passed to fncbUpdate
                     PFNWP fncbError)       // in:  error callback
{
    // BOOL    rc = TRUE;
    HINI    hNew;
    PSZ     pApps, // pKeys,
            pApp2; // , pKey2;
    ULONG   ulSizeOfAppsList; // , ulSizeOfKeysList; // , ulSizeOfData;
    // CHAR    szLog[1000];     // removed V0.9.0
    #define MBID_NOERROR 999
    ULONG   ulErrorStatus = MBID_NOERROR;
    PSZ     p_;
    CHAR    szError2[2000];

    if (fLog)
    {
        fprintf(fLog, "    Entering prfhCopyProfile, params: "
                      "hiniOld 0x%lX, pszOld %s, pfnwpCallback 0x%lX, "
                      "hwnd 0x%lX, msg 0x%lX, ulCount 0x%lX, ulMax 0x%lX\n",
                hOld,
                pszOld,
                (ULONG)fncbUpdate,
                hwnd,
                msg,
                ulCount,
                ulMax);
        fflush(fLog);
    }

    if ( (!pszOld) || (!pszNew) )
        ulErrorStatus = prfhINIError2(MB_CANCEL, pszOld, fLog, fncbError,
                                      "Invalid name buffers given.");

    if (ulErrorStatus == MBID_NOERROR)
    {
        strupr(pszOld);
        strcpy(pszNew, pszOld);
        p_ = strhistr(pszNew, ".INI");
        if (!p_)
            ulErrorStatus = prfhINIError2(MB_CANCEL, pszNew, fLog, fncbError,
                                          "Error composing new filename.");
    }

    if (ulErrorStatus == MBID_NOERROR)
    {
        strcpy(p_, ".XFL");

        // delete new profile, in case something's left
        if (fLog)
            fprintf(fLog, "    DEL %s\n", pszNew);

        DosDelete(pszNew);

        // open new profile
        if (fLog)
            fprintf(fLog, "    PrfOpenProfile %s\n", pszNew);

        hNew = PrfOpenProfile(hab, pszNew);
        if (!hNew)
            ulErrorStatus = prfhINIError2(MB_CANCEL, pszNew, fLog, fncbError,
                                          "Error creating temporary profile.");
    }

    // get size of applications list
    if (ulErrorStatus == MBID_NOERROR)
    {
        if (!PrfQueryProfileSize(hOld, NULL, NULL, &ulSizeOfAppsList))
            ulErrorStatus = prfhINIError2(MB_CANCEL, pszNew, fLog, fncbError,
                    "Error querying applications list size.");
        else
        if (ulSizeOfAppsList == 0)
            ulErrorStatus = prfhINIError2(MB_CANCEL, pszNew, fLog, fncbError,
                "Size of applications list cannot be 0.");
    }

    if (ulErrorStatus == MBID_NOERROR)
    {
        // do-while loop in case of MBID_RETRY
        do
        {
            ulErrorStatus = MBID_NOERROR; // if we have a retry in the do-while loop

            // get applications list
            pApps = (PSZ)malloc(ulSizeOfAppsList);
            pApp2 = pApps;
            if (!PrfQueryProfileData(hOld,
                                     NULL,
                                     NULL,
                                     pApps,
                                     &ulSizeOfAppsList))
                ulErrorStatus = prfhINIError2(MB_CANCEL, pszNew, fLog, fncbError,
                                              "Could not query application list.");

            // applications loop

            while (   (*pApp2 != 0)
                   && (ulErrorStatus == MBID_NOERROR)
                  )
            {
                CHAR szErrorKey[1000];
                // copy application (this will call prfhCopyKey in turn)
                ULONG ulrc = prfhCopyApp(hOld,
                                         pApp2,
                                         hNew,
                                         pApp2,
                                         szErrorKey);

                ulErrorStatus = MBID_ABORT;

                switch (ulrc)
                {
                    case 0: // no error
                        ulErrorStatus = MBID_NOERROR;
                    break;

                    case PRFERR_KEYSLIST:
                        // couldn't query keys list
                        sprintf(szError2,
                                "Error querying keys list for \"%s\".",
                                pApp2);
                    break;

                    case PRFERR_DATASIZE:
                        // couldn't query data size for key
                        sprintf(szError2,
                                "Error querying data size for \"%s\"/\"%s\".",
                                pApp2, szErrorKey);
                    break;

                    case ERROR_NOT_ENOUGH_MEMORY:
                        // couldn't allocate memory
                        sprintf(szError2,
                                "Error allocating memory for copying \"%s\"/\"%s\".",
                                pApp2, szErrorKey);
                    break;

                    case PRFERR_READ:
                        // couldn't read data from source (PrfQueryProfileData error)
                        sprintf(szError2,
                                "Error reading data from app \"%s\", key \"%s\".",
                                pApp2, szErrorKey);
                    break;

                    case PRFERR_WRITE:
                        // couldn't write data to target (PrfWriteProfileData error)
                        sprintf(szError2,
                                "Error writing data to app \"%s\", key \"%s\".",
                                pApp2, szErrorKey);
                    break;
                }

                if (ulErrorStatus != MBID_NOERROR) // V0.9.4 (2000-07-15) [umoeller]
                {
                    ulErrorStatus = prfhINIError2(MB_ABORTRETRYIGNORE, pszNew, fLog, fncbError,
                                                  szError2);
                }

                if (fncbUpdate)
                {
                    ULONG ulNow2, ulMax2;
                    ulNow2 = ((1000*(pApp2-pApps))/ulSizeOfAppsList) + (ulCount*1000);
                    ulMax2 = (ulMax+1)*1000; // ulAppsSize;
                    (*fncbUpdate)(hwnd, msg, (MPARAM)ulNow2, (MPARAM)ulMax2);
                }

                // go for next app
                pApp2 += strlen(pApp2)+1;

                if (ulErrorStatus == MBID_IGNORE)
                    // error occurred, but user pressed "Ignore":
                    // skip this app
                    ulErrorStatus = MBID_NOERROR;

            } // end while (*pApp2 != 0) && MBID_NOERROR

            if (pApps)
                free(pApps);

            if (fLog)
                fprintf(fLog, "    Done copying apps\n");

        } while (ulErrorStatus == MBID_RETRY);
    } // end if (ulErrorOccurred == MBID_NOERROR)

    if (fLog)
        fprintf(fLog, "    PrfCloseProfile %s\n", pszNew);

    PrfCloseProfile(hNew);
    if (fncbUpdate)
        (*fncbUpdate)(hwnd, msg, (MPARAM)(ulCount+1), (MPARAM)(ulMax+1));

    if (fLog)
    {
        fprintf(fLog, "    Leaving prfhCopyProfile.\n");
        fflush(fLog);
    }

    return (ulErrorStatus == MBID_NOERROR); // FALSE if error occurred
}

/*
 *@@ prfhSaveINIs:
 *      this will enforce saving of OS2.INI and OS2SYS.INI
 *      by calling prfhCopyProfile (above) on them; the therefrom
 *      resulting ".XFL" files will replace the original
 *      ".INI" files.
 *      Specify fncbUpdate and fncbError like in prfhCopyProfile.
 *
 *@@changed V0.9.0 [umoeller]: fixed crashes with lower-case INI file names.
 *@@changed V0.9.0 [umoeller]: changed prototype's logfile to stdio FILE*
 */

APIRET prfhSaveINIs(HAB hab,               // in:  anchor block
                    FILE* fLog,            // in:  stdio logfile or NULL for no log
                    PFNWP fncbUpdate,      // in:  progress callback
                    HWND hwnd, ULONG msg,  // in:  params passed to fncbUpdate
                    PFNWP fncbError)       // in:  error callback
{
    PRFPROFILE Profiles;
    APIRET  arc = NO_ERROR, arc2;
    BOOL    brc = TRUE;

    // FILESTATUS3 fs3;
    CHAR    szSysNew[CCHMAXPATH],
            szUserNew[CCHMAXPATH],
            szSysBackup[CCHMAXPATH],
            szUserBackup[CCHMAXPATH];

    // the following flag may be one of the following:
    //        MBID_NOERROR--- everything's fine, continue
    //        MBID_IGNORE --- error occurred, but ignore
    //        MBID_RETRY  --- error occurred, but retry
    //        MBID_ABORT  --- error occurred, abort saving
    ULONG   ulErrorOccurred = MBID_IGNORE;

    if (fLog)
        fprintf(fLog, "  Entering prfhSaveINIs...\n");

    /*
     * get system profiles:
     *
     */

    Profiles.cchUserName = Profiles.cchSysName = 0;
    brc = PrfQueryProfile(hab, &Profiles);

    if (!brc)
        ulErrorOccurred = prfhINIError(MB_CANCEL, fLog, fncbError,
                                      "Error querying system profiles size.");

    if (ulErrorOccurred == MBID_IGNORE)
    {
        Profiles.pszUserName  = (PSZ)malloc(Profiles.cchUserName);
        Profiles.pszSysName  = (PSZ)malloc(Profiles.cchSysName);
        if (    (Profiles.pszSysName == NULL)
             || (Profiles.pszUserName == NULL)
           )
            ulErrorOccurred = prfhINIError(MB_CANCEL, fLog, fncbError,
                    "Error allocating memory (1).");
    }

    if (ulErrorOccurred == MBID_IGNORE)
    {
        if (!PrfQueryProfile(hab, &Profiles))
            ulErrorOccurred = prfhINIError(MB_CANCEL, fLog, fncbError,
                    "Error querying profiles (2).");
    }

    if (ulErrorOccurred == MBID_IGNORE)
    {
        if (fLog)
            fprintf(fLog, "  System profiles are %s, %s\n",
                    Profiles.pszUserName, Profiles.pszSysName);

        /*
         * create new profile names:
         *
         */

        strcpy(szSysBackup, Profiles.pszSysName);
        strcpy(strhistr(szSysBackup, ".INI"), ".BAK");
        strcpy(szUserBackup, Profiles.pszUserName);
        strcpy(strhistr(szUserBackup, ".INI"), ".BAK");

        /*
         * create OS2SYS.XFL:
         *
         */

        if (fLog)
            fprintf(fLog, "  Storing %s in *.XFL\n",
                    Profiles.pszSysName);

        if (!prfhCopyProfile(hab,
                             fLog,
                             HINI_SYSTEMPROFILE,
                             Profiles.pszSysName,   // old filename
                             szSysNew,              // new filename
                             fncbUpdate, hwnd, msg, 0, 1,
                             fncbError))
        {
            // abort, since prfhCopyProfile already has error handling
            ulErrorOccurred = MBID_ABORT;
        }
    }

    if (ulErrorOccurred == MBID_IGNORE)
    {
        // create OS2.XFL

        if (fLog)
            fprintf(fLog, "  Storing %s in *.XFL\n",
                    Profiles.pszUserName);

        if (!prfhCopyProfile(hab,
                             fLog,
                             HINI_USERPROFILE,
                             Profiles.pszUserName, szUserNew,
                             fncbUpdate, hwnd, msg, 1, 1,
                             fncbError))
        {
            // abort, since prfhCopyProfile already has error handling
            ulErrorOccurred = MBID_ABORT;
        }
    }

    /*
     * renaming stuff for OS2SYS.INI
     *
     */

    if (ulErrorOccurred == MBID_IGNORE)
    {
        do
        {
            ulErrorOccurred = MBID_IGNORE; // if we have a retry in the do-while loop

            if (ulErrorOccurred == MBID_IGNORE)
            {
                // attrib -r -s -h -a OS2SYS.BAK
                if (fLog)
                    fprintf(fLog, "  ATTRIB -R -S -H -A %s\n",
                            szSysBackup);

                arc2 = doshSetPathAttr(szSysBackup, FILE_NORMAL);
                if (fLog)
                    fprintf(fLog, "    rc2: %lu\n", arc2);

                // delete OS2SYS.BAK
                if (fLog)
                    fprintf(fLog, "  DEL %s\n",
                            szSysBackup);

                arc2 = DosDelete(szSysBackup);
                if (fLog)
                    fprintf(fLog, "    rc2: %lu\n", arc2);

                // attrib -r -s -h -a OS2SYS.INI
                if (fLog)
                    fprintf(fLog, "  ATTRIB -R -S -H -A %s\n",
                            Profiles.pszSysName);

                arc2 = doshSetPathAttr(Profiles.pszSysName, FILE_NORMAL);
                if (fLog)
                    fprintf(fLog, "    rc2: %lu\n", arc2);

                // move OS2SYS.INI OS2SYS.BAK
                if (fLog)
                    fprintf(fLog, "  MOVE %s %s\n",
                            Profiles.pszSysName, szSysBackup);
                arc = DosMove(Profiles.pszSysName, szSysBackup);
                if (fLog)
                    fprintf(fLog, "    rc: %lu\n", arc);

                if (arc)
                    ulErrorOccurred = prfhINIError(MB_ABORTRETRYIGNORE, fLog, fncbError, "Error moving original system profile to backup.");
            }

            if (ulErrorOccurred == MBID_IGNORE)
            {
                if (fLog)
                    fprintf(fLog, "  MOVE %s %s\n",
                            szSysNew, Profiles.pszSysName);
                arc = DosMove(szSysNew, Profiles.pszSysName);
                if (fLog)
                    fprintf(fLog, "    rc: %lu\n", arc);
                if (arc)
                    ulErrorOccurred = prfhINIError(MB_ABORTRETRYIGNORE, fLog, fncbError, "Error moving newly created profile to system profile.");
            }
        } while (ulErrorOccurred == MBID_RETRY);
    }

    /*
     * renaming stuff for OS2.INI
     *
     */

    if (ulErrorOccurred == MBID_IGNORE)
    {
        do
        {
            ulErrorOccurred = MBID_IGNORE; // if we have a retry in the do-while loop

            if (ulErrorOccurred == MBID_IGNORE)
            {
                // attrib -r -s -h -a OS2.BAK
                if (fLog)
                    fprintf(fLog, "  ATTRIB -R -S -H -A %s\n",
                            szUserBackup);

                arc2 = doshSetPathAttr(szUserBackup, FILE_NORMAL);
                if (fLog)
                    fprintf(fLog, "    rc2: %lu\n", arc2);

                // delete OS2.BAK
                if (fLog)
                    fprintf(fLog,"  DEL %s\n",
                            szUserBackup);

                arc2 = DosDelete(szUserBackup);
                if (fLog)
                    fprintf(fLog, "    rc2: %lu\n", arc2);

                // attrib -r -s -h -a OS2.INI
                if (fLog)
                    fprintf(fLog, "  ATTRIB -R -S -H -A %s\n",
                            Profiles.pszUserName);
                arc2 = doshSetPathAttr(Profiles.pszUserName, FILE_NORMAL);
                if (fLog)
                    fprintf(fLog, "    rc2: %lu\n", arc2);

                // move OS2.INI OS2.BAK
                if (fLog)
                    fprintf(fLog, "  MOVE %s %s\n",
                            Profiles.pszUserName, szUserBackup);
                arc = DosMove(Profiles.pszUserName, szUserBackup);
                if (fLog)
                    fprintf(fLog, "    rc: %lu\n", arc);

                if (arc)
                    ulErrorOccurred = prfhINIError(MB_ABORTRETRYIGNORE, fLog, fncbError, "Error moving original user profile to backup.");
            }

            if (ulErrorOccurred == MBID_IGNORE)
            {
                // move OS2.XFL OS2.INI
                if (fLog)
                    fprintf(fLog, "  MOVE %s %s\n",
                            szUserNew, Profiles.pszUserName);

                arc = DosMove(szUserNew, Profiles.pszUserName);
                if (fLog)
                    fprintf(fLog, "    rc: %lu\n", arc);

                if (arc)
                    ulErrorOccurred = prfhINIError(MB_ABORTRETRYIGNORE, fLog, fncbError, "Error moving newly created profile to user profile.");
            }
        } while (ulErrorOccurred == MBID_RETRY);
    }

    // DosExitCritSec();

    if (ulErrorOccurred != MBID_IGNORE)
    {
        DosMove(szSysBackup, Profiles.pszSysName);
        DosMove(szUserBackup, Profiles.pszUserName);
    }

    if (Profiles.pszSysName)
        free(Profiles.pszSysName);
    if (Profiles.pszUserName)
        free(Profiles.pszUserName);

    if (fLog)
        fprintf(fLog, "  Done with prfhSaveINIs\n");

    if (ulErrorOccurred != MBID_IGNORE)
        return 999;
    else
        return NO_ERROR;
}

#ifdef __PRFH2_MAIN__

int main(int argc, char* argv[])
{
    APIRET  arc = NO_ERROR;
    PCSZ    pcszSource,
            pcszTarget,
            pcszApp;
    PXINI   piniSource;
    HINI    hiniTarget;
    BOOL    fCloseSource = FALSE,
            fCloseTarget = FALSE;
    HAB     hab = WinInitialize(0);
    CHAR    szFailing[1000];

    if (argc < 4)
    {
        printf("%d\nprfh2 <sourcefile> <targetfile> <appname>\n", argc);
        exit(2);
    }

    pcszSource = argv[1];
    pcszTarget = argv[2];
    pcszApp = argv[3];

    if (arc = xprfOpenProfile(pcszSource,
                              &piniSource))
    {
        printf("xprfOpenProfile returned %d opening source INI \"%s\"\n",
               arc,
               pcszSource);
    }
    else
        fCloseSource = TRUE;

    if (!strcmp(pcszTarget, "USER"))
        hiniTarget = HINI_USER;
    else if (!strcmp(pcszTarget, "SYSTEM"))
        hiniTarget = HINI_SYSTEM;
    else
    {
        if (!(hiniTarget = PrfOpenProfile(hab, (PSZ)pcszTarget)))
        {
            printf("Cannot open source INI \"%s\"\n", pcszTarget);
            arc = -1;
        }
        else
            fCloseTarget = TRUE;
    }

    if (!arc)
    {
        if (arc = xprfCopyApp2(piniSource,
                               pcszApp,
                               hiniTarget,
                               pcszApp,
                               szFailing))
        {
            printf("Error %d copying application \"%s\" from %s to %s\n",
                   arc,
                   pcszApp,
                   pcszSource,
                   pcszTarget);
            printf("Failing key: \"%s\"\n",
                   szFailing);
        }
        else
            printf("Application \"%s\" successfully copied from %s to %s\n",
                   pcszApp,
                   pcszSource,
                   pcszTarget);
    }

    if (arc)
    {
        ERRORID eid;
        eid = WinGetLastError(hab);

        printf("WinGetLastError returned 0x%lX\n",
               eid);
    }


    if (fCloseSource)
        xprfCloseProfile(piniSource);
    if (fCloseTarget)
        PrfCloseProfile(hiniTarget);

    WinTerminate(hab);

    exit(arc);
}

#endif
