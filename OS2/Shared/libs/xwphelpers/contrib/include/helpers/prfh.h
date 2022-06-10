
/*
 *@@sourcefile prfh.h:
 *      header file for prfh.c. See remarks there.
 *
 *      This file is new with V0.82.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_WINWINDOWMGR
 *@@include #define INCL_WINSHELLDATA
 *@@include #include <os2.h>
 *@@include #include <stdio.h>
 *@@include #include "helpers\prfh.h"
 */

/*      Copyright (C) 1997-2000 Ulrich M”ller.
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

#ifndef PRFH_HEADER_INCLUDED
    #define PRFH_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Errors
     *
     ********************************************************************/

    #define ERROR_PRF_FIRST             42000

    #define PRFERR_DATASIZE             (ERROR_PRF_FIRST + 1)
                    // couldn't query data size for key
    #define PRFERR_READ                 (ERROR_PRF_FIRST + 2)
                    // couldn't read data from source (PrfQueryProfileData error)
    #define PRFERR_WRITE                (ERROR_PRF_FIRST + 3)
                    // couldn't write data to target (PrfWriteProfileData error)
    #define PRFERR_APPSLIST             (ERROR_PRF_FIRST + 4)
                    // couldn't query apps list
    #define PRFERR_KEYSLIST             (ERROR_PRF_FIRST + 5)
                    // couldn't query keys list
    #define PRFERR_ABORTED              (ERROR_PRF_FIRST + 6)
                    // aborted by user
    #define PRFERR_QUERY                (ERROR_PRF_FIRST + 7)
                    // PrfQueryProfile failed
    #define PRFERR_INVALID_FILE_NAME    (ERROR_PRF_FIRST + 8)
                    // profile names don't contain .INI
    #define PRFERR_INVALID_KEY          (ERROR_PRF_FIRST + 9)
    #define PRFERR_KEY_EXISTS           (ERROR_PRF_FIRST + 10)

    #define PRFERR_RESET                (ERROR_PRF_FIRST + 11)
                    // PrfReset failed V0.9.19 (2002-04-02) [umoeller]

    #define PRFERR_INVALID_APP_NAME     (ERROR_PRF_FIRST + 12)
                    // V1.0.0 (2002-09-17) [umoeller]
    #define PRFERR_INVALID_KEY_NAME     (ERROR_PRF_FIRST + 13)
                    // V1.0.0 (2002-09-17) [umoeller]

    #define ERROR_PRF_LAST              (ERROR_PRF_FIRST + 13)

    /* ******************************************************************
     *
     *   Strings
     *
     ********************************************************************/

    // DECLARE_PRFH_STRING is a handy macro which saves us from
    // keeping two string lists in both the .h and the .c file.
    // If this include file is included from the .c file, the
    // string is defined as a global variable. Otherwise
    // it is only declared as "extern" so other files can
    // see it.

    #ifdef INCLUDE_PRFH_PRIVATE
        #define DECLARE_PRFH_STRING(str, def) const char *str = def
    #else
        #define DECLARE_PRFH_STRING(str, def) extern const char *str;
    #endif

    /*
     * OS2.INI applications
     *
     */

    // NLS settings section
    DECLARE_PRFH_STRING(PMINIAPP_NATIONAL,      "PM_National");

    // system font settings section
    DECLARE_PRFH_STRING(PMINIAPP_SYSTEMFONTS,   "PM_SystemFonts");
    DECLARE_PRFH_STRING(PMINIKEY_DEFAULTFONT,   "DefaultFont");
    DECLARE_PRFH_STRING(PMINIKEY_ICONTEXTFONT,  "IconText");
    DECLARE_PRFH_STRING(PMINIKEY_MENUSFONT,     "Menus");

    // installed fonts section
    DECLARE_PRFH_STRING(PMINIAPP_FONTS,         "PM_Fonts");

    // general WPS settings
    DECLARE_PRFH_STRING(WPINIAPP_WORKPLACE,     "PM_Workplace");
    DECLARE_PRFH_STRING(WPINIKEY_MENUBAR,       "FolderMenuBar");

    DECLARE_PRFH_STRING(WPINIKEY_SHORTMENUS,    "FolderMenus");
            // V0.9.19 (2002-04-17) [umoeller]

    // class replacements list V1.0.0 (2002-08-26) [umoeller]
    DECLARE_PRFH_STRING(WPINIAPP_REPLACEMENTS, "PM_Workplace:ReplaceList");

    // abstract objects per folder handle
    DECLARE_PRFH_STRING(WPINIAPP_FDRCONTENT,    "PM_Abstract:FldrContent");
    // all defined abstract objects on the system
    DECLARE_PRFH_STRING(WPINIAPP_OBJECTS,       "PM_Abstract:Objects");
    // their icons, if set individually
    DECLARE_PRFH_STRING(WPINIAPP_ICONS,         "PM_Abstract:Icons");

    // object ID's (<WP_DESKTOP> etc.)
    DECLARE_PRFH_STRING(WPINIAPP_LOCATION,      "PM_Workplace:Location");

    // folder positions
    DECLARE_PRFH_STRING(WPINIAPP_FOLDERPOS,     "PM_Workplace:FolderPos");

    // palette positions
    DECLARE_PRFH_STRING(WPINIAPP_PALETTEPOS,    "PM_Workplace:PalettePos");
    // ???
    DECLARE_PRFH_STRING(WPINIAPP_STATUSPOS,     "PM_Workplace:StatusPos");
    // startup folders
    DECLARE_PRFH_STRING(WPINIAPP_STARTUP,       "PM_Workplace:Startup");
    // all the defined templates on the system
    DECLARE_PRFH_STRING(WPINIAPP_TEMPLATES,     "PM_Workplace:Templates");

    // all work area folders
    DECLARE_PRFH_STRING(WPINIAPP_WORKAREARUNNING, "FolderWorkareaRunningObjects");
    // spooler windows ?!?
    DECLARE_PRFH_STRING(WPINIAPP_JOBCNRPOS,     "PM_PrintObject:JobCnrPos");

    // associations by type ("Plain Text")
    DECLARE_PRFH_STRING(WPINIAPP_ASSOCTYPE, "PMWP_ASSOC_TYPE");
    // associations by filter ("*.txt")
    DECLARE_PRFH_STRING(WPINIAPP_ASSOCFILTER, "PMWP_ASSOC_FILTER");
    // checksums ?!?
    DECLARE_PRFH_STRING(WPINIAPP_ASSOC_CHECKSUM, "PMWP_ASSOC_CHECKSUM");

    /*
     * OS2SYS.INI applications
     *
     */

    DECLARE_PRFH_STRING(WPINIAPP_ACTIVEHANDLES, "PM_Workplace:ActiveHandles");
    DECLARE_PRFH_STRING(WPINIAPP_HANDLES, "PM_Workplace:Handles");
    DECLARE_PRFH_STRING(WPINIAPP_HANDLESAPP, "HandlesAppName");

    /*
     * some default WPS INI keys:
     *
     */

    DECLARE_PRFH_STRING(WPOBJID_DESKTOP, "<WP_DESKTOP>");

    DECLARE_PRFH_STRING(WPOBJID_KEYB, "<WP_KEYB>");
    DECLARE_PRFH_STRING(WPOBJID_MOUSE, "<WP_MOUSE>");
    DECLARE_PRFH_STRING(WPOBJID_CNTRY, "<WP_CNTRY>");
    DECLARE_PRFH_STRING(WPOBJID_SOUND, "<WP_SOUND>");
    DECLARE_PRFH_STRING(WPOBJID_SYSTEM, "<WP_SYSTEM>"); // V0.9.9
    DECLARE_PRFH_STRING(WPOBJID_POWER, "<WP_POWER>");
    DECLARE_PRFH_STRING(WPOBJID_WINCFG, "<WP_WINCFG>");

    DECLARE_PRFH_STRING(WPOBJID_HIRESCLRPAL, "<WP_HIRESCLRPAL>");
    DECLARE_PRFH_STRING(WPOBJID_LORESCLRPAL, "<WP_LORESCLRPAL>");
    DECLARE_PRFH_STRING(WPOBJID_FNTPAL, "<WP_FNTPAL>");
    DECLARE_PRFH_STRING(WPOBJID_SCHPAL96, "<WP_SCHPAL96>");

    DECLARE_PRFH_STRING(WPOBJID_LAUNCHPAD, "<WP_LAUNCHPAD>");
    DECLARE_PRFH_STRING(WPOBJID_WARPCENTER, "<WP_WARPCENTER>");

    DECLARE_PRFH_STRING(WPOBJID_SPOOL, "<WP_SPOOL>");
    DECLARE_PRFH_STRING(WPOBJID_VIEWER, "<WP_VIEWER>");
    DECLARE_PRFH_STRING(WPOBJID_SHRED, "<WP_SHRED>");
    DECLARE_PRFH_STRING(WPOBJID_CLOCK, "<WP_CLOCK>");

    DECLARE_PRFH_STRING(WPOBJID_START, "<WP_START>");
    DECLARE_PRFH_STRING(WPOBJID_TEMPS, "<WP_TEMPS>");
    DECLARE_PRFH_STRING(WPOBJID_DRIVES, "<WP_DRIVES>");

    /* ******************************************************************
     *
     *   Functions
     *
     ********************************************************************/

    APIRET prfhQueryKeysForApp(HINI hIni,
                               const char *pcszApp,
                               PSZ *ppszKeys);

    #ifdef __DEBUG_MALLOC_ENABLED__ // setup.h, helpers\memdebug.c
        PSZ prfhQueryProfileDataDebug(HINI hIni,
                                      const char *pcszApp,
                                      const char *pcszKey,
                                      PULONG pcbBuf,
                                      const char *file,
                                      unsigned long line,
                                      const char *function);
        #define prfhQueryProfileData(a, b, c, d) prfhQueryProfileDataDebug((a), (b), (c), (d), __FILE__, __LINE__, __FUNCTION__)
    #else
        PSZ prfhQueryProfileData(HINI hIni,
                                 const char *pcszApp,
                                 const char *pcszKey,
                                 PULONG pcbBuf);
    #endif

    CHAR prfhQueryProfileChar(HINI hini,
                              const char *pcszApp,
                              const char *pcszKey,
                              CHAR cDefault);

    LONG prfhQueryColor(const char *pcszKeyName, const char *pcszDefault);

    APIRET prfhCopyKey(HINI hiniSource,
                       const char *pcszSourceApp,
                       const char *pcszKey,
                       HINI hiniTarget,
                       const char *pcszTargetApp);

    APIRET prfhCopyApp(HINI hiniSource,
                       const char *pcszSourceApp,
                       HINI hiniTarget,
                       const char *pcszTargetApp,
                       PSZ pszErrorKey);

    ULONG prfhRenameKey(HINI hini,
                        const char *pcszOldApp,
                        const char *pcszOldKey,
                        const char *pcszNewApp,
                        const char *pcszNewKey);

    APIRET prfhSetUserProfile(HAB hab,
                              const char *pcszUserProfile);

    ULONG prfhINIError(ULONG ulOptions,
                       FILE* fLog,
                       PFNWP fncbError,
                       PSZ pszErrorString);

    ULONG prfhINIError2(ULONG ulOptions,
                        const char *pcszINI,
                        FILE* fLog,
                        PFNWP fncbError,
                        PSZ pszErrorString);

    BOOL prfhCopyProfile(HAB hab,
                         FILE* fLog,
                         HINI hOld,
                         PSZ pszOld,
                         PSZ pszNew,
                         PFNWP fncbUpdate,
                         HWND hwnd, ULONG msg, ULONG ulCount, ULONG ulMax,
                         PFNWP fncbError);

    APIRET prfhSaveINIs(HAB hab,
                        FILE* fLog,
                        PFNWP fncbUpdate,
                        HWND hwnd, ULONG msg,
                        PFNWP fncbError);

#endif

#if __cplusplus
}
#endif

