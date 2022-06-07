
/*
 *@@sourcefile xprf.h:
 *      header file for xprf.c. See remarks there.
 *
 *      This file was new with V0.9.5 (?).
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_WINSHELLDATA
 *@@include #include <os2.h>
 *@@include #include "helpers\xprf.h"
 */

/*      Copyright (C) 2000-2008 Ulrich M”ller.
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

#if __cplusplus
extern "C" {
#endif

#ifndef XPRF_HEADER_INCLUDED
    #define XPRF_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   OS/2 INI file layout
     *
     ********************************************************************/

    #pragma pack(1)

    /*
     *@@ INIFILE_HEADER:
     *      header of OS/2 INI file. This is
     *      what an OS/2 .INI file starts with
     *      at byte 0.
     *
     *      An OS/2 INI file has the following layout:
     *
     *      -- INIFILE_HEADER
     *      -- first INIFILE_APP, if any
     *      -- name of first application (zero-terminated),
     *              of which the first INIFILE_APP.offAppName
     *              has the offset
     *          -- first INIFILE_KEY of first app
     *          -- name of first key (zero-terminated),
     *              of which the first INIFILE_KEY.offKeyName
     *              has the offset
     *          -- data block of first key, of which the first
     *              INIFILE_KEY.offKeyData has the offset;
     *              the length is in INIFILE_KEY.lenKeyData
     *
     *          -- subsequent INIFILE_KEY's, names, data, if any
     *
     *      -- subsequent INIFILE_APP's, if any
     *              ...
     *
     *@@added V0.9.5 (2000-08-10) [umoeller]
     */

    typedef struct _INIFILE_HEADER
    {
        ULONG magic;       // Magic Footprint, Always $FFFFFFFF (?)
        ULONG offFirstApp; // Offset of First Application in File
        ULONG lenFile;     // Length of INI File
        ULONG filler1;     // Always $00000000 (?)
        ULONG filler2;     // Always $00000000 (?)
    } INIFILE_HEADER, *PINIFILE_HEADER;

    /*
     *@@ INIFILE_APP:
     *      application entry in OS/2 INI file.
     *      The first application comes right after
     *      INIFILE_HEADER. After INIFILE_APP, the
     *      Prf* functions store the app name and
     *      the keys which belong to this application
     *      (INIFILE_KEY).
     *
     *@@added V0.9.5 (2000-08-10) [umoeller]
     */

    typedef struct _INIFILE_APP
    {
        ULONG  offNextApp;       // Offset of Next Application in File
                                 // (0 No More Apps)
        ULONG  offFirstKeyInApp; // Offset of Application's First Key Entry
        ULONG  filler1;          // Always $00000000 (?)
        USHORT lenAppName;       // Length of Application Name
                                 // (incl. terminating \0)
        USHORT _lenAppName;      // Always same as above (?)
        ULONG  offAppName;       // Offset of ASCIIZ Application Name
    } INIFILE_APP, *PINIFILE_APP;

    /*
     *@@ INIFILE_KEY:
     *      key entry in OS/2 INI file.
     *      The first key in an application comes right after
     *      its INIFILE_APP. After INIFILE_KEY, the Prf*
     *      functions store the key name and finally the
     *      data for that key.
     *
     *@@added V0.9.5 (2000-08-10) [umoeller]
     */

    typedef struct _INIFILE_KEY
    {
        ULONG  offNextKeyInApp; // Offset of Next Key in Application
                                // (0 = No More Keys)
        ULONG  filler1;         // Always $00000000 (?)
        USHORT lenKeyName;      // Length of Key Name (incl. terminating \0)
        USHORT _lenKeyName;     // Always same as above (?)
        ULONG  offKeyName;      // Offset of ASCIIZ Key Name
        USHORT lenKeyData;      // Length of Key Data
        USHORT _lenKeyData;     // Always same as above (?)
        ULONG  offKeyData;      // Offset of Key Data (ASCII(Z) or Binary)
    } INIFILE_KEY, *PINIFILE_KEY;

    #pragma pack()

    /* ******************************************************************
     *
     *   API Functions
     *
     ********************************************************************/

    #ifdef LINKLIST_HEADER_INCLUDED

        #define XINI_MAGIC_BYTES "xpRfMa\x03"

        /*
         *@@ XINI:
         *      open INI file. Returned by xprfOpenProfile
         *      and is used in place of HINI by the replacement
         *      INI functions.
         *
         *      Do not modify any data in here.
         */

        typedef struct _XINI
        {
            CHAR    acMagic[sizeof(XINI_MAGIC_BYTES)];
                                // magic bytes for security
                                // removed V1.0.0 (2002-09-20) [umoeller]
            CHAR    szFilename[CCHMAXPATH];

            HFILE   hFile;      // returned by DosProtectOpen
            // FHLOCK  hLock;      // lock ID of DosProtectOpen
                        // removed V1.0.0 (2002-09-20) [umoeller]
            BOOL    fDirty;     // TRUE if changed and needs to be flushed
                                // on close

            // applications list
            LINKLIST    llApps;             // contains PXINIAPPDATA items
        } XINI, *PXINI;
    #else
        typedef void* PXINI;
    #endif

    APIRET xprfOpenProfile(PCSZ pcszFilename,
                           PXINI *ppxini);

    APIRET xprfCloseProfile(PXINI hIni);

    APIRET xprfQueryProfileSize(PXINI pXIni,
                                PCSZ pszAppName,
                                PCSZ pszKeyName,
                                PULONG pulDataLen);

    APIRET xprfQueryProfileData(PXINI pXIni,
                                PCSZ pszAppName,
                                PCSZ pszKeyName,
                                PVOID pBuffer,
                                PULONG pulBufferMax);

    LONG xprfQueryProfileInt(PXINI pXIni,
                             PCSZ pcszApp,
                             PCSZ pcszKey,
                             LONG lDefault);

    APIRET xprfWriteProfileData(PXINI hIni,
                                PCSZ pcszApp,
                                PCSZ pcszKey,
                                PVOID pData,
                                ULONG ulDataLen);

    APIRET xprfWriteProfileString(PXINI pXIni,
                                  PCSZ pcszApp,
                                  PCSZ pcszKey,
                                  PCSZ pcszString);

    APIRET xprfQueryKeysForApp(PXINI hIni,
                               PCSZ pcszApp,
                               PSZ *ppszKeys);

    PSZ xprfhQueryProfileData(PXINI pXIni,
                              PCSZ pcszApp,
                              PCSZ pcszKey,
                              PULONG pcbBuf);

    /* ******************************************************************
     *
     *   Copy API Functions
     *
     ********************************************************************/

    /*
     *@@ FN_PRF_PROGRESS:
     *      prototype for a progress callback used with
     *      xprfCopyProfile and xprfSaveINIs.
     *
     *      Declare your callback like this:
     +          BOOL _Optlink fnProgress(ULONG ulUser,
     +                                         // in: use param specified with
     +                                         // xprfCopyProfile and xprfSaveINIs
     +                                   ULONG ulProgressNow,
     +                                         // in: current progress
     +                                   ULONG ulProgressMax)
     +                                         // in: maximum progress
     *
     *      If this returns FALSE, processing is aborted.
     */

    typedef BOOL (_Optlink FN_PRF_PROGRESS)(ULONG, ULONG, ULONG);
    typedef FN_PRF_PROGRESS *PFN_PRF_PROGRESS;

    APIRET xprfCopyKey(HINI hiniSource,
                       PCSZ pszSourceApp,
                       PCSZ pszKey,
                       PXINI hiniTarget,
                       PCSZ pszTargetApp);

    APIRET xprfCopyKey2(PXINI hiniSource,
                        PCSZ pszSourceApp,
                        PCSZ pszKey,
                        HINI hiniTarget,
                        PCSZ pszTargetApp);

    APIRET xprfCopyApp(HINI hiniSource,
                       PCSZ pszSourceApp,
                       PXINI hiniTarget,
                       PCSZ pszTargetApp,
                       PSZ pszErrorKey);

    APIRET xprfCopyApp2(PXINI hiniSource,
                        PCSZ pszSourceApp,
                        HINI hiniTarget,
                        PCSZ pszTargetApp,
                        PSZ pszErrorKey);

    APIRET xprfCopyProfile(HINI hOld,
                           PCSZ pszNew,
                           PFN_PRF_PROGRESS pfnProgressCallback,
                           ULONG ulUser,
                           ULONG ulCount,
                           ULONG ulMax,
                           PSZ pszFailingApp);

    APIRET xprfSaveINIs(HAB hab,
                        PFN_PRF_PROGRESS pfnProgressCallback,
                        ULONG ulUser,
                        PSZ pszFailingINI,
                        PSZ pszFailingApp,
                        PSZ pszFailingKey);
#endif

#if __cplusplus
}
#endif

