
/*
 *@@sourcefile apps.h:
 *      header file for apps.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_WINPROGRAMLIST
 *@@include #include <os2.h>
 *@@include #include "helpers/apps.h"
 */

/*      This file Copyright (C) 1997-2001 Ulrich M”ller.
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

#ifndef APPS_HEADER_INCLUDED
    #define APPS_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Environment helpers
     *
     ********************************************************************/

    ULONG appQueryEnvironmentLen(PCSZ pcszEnvironment);

    /*
     *@@ DOSENVIRONMENT:
     *      structure holding an array of environment
     *      variables (papszVars). This is initialized
     *      appGetEnvironment,
     *
     *@@added V0.9.4 (2000-07-19) [umoeller]
     */

    typedef struct _DOSENVIRONMENT
    {
        ULONG       cVars;              // count of vars in papzVars
        PSZ         *papszVars;         // array of PSZ's to environment strings (VAR=VALUE)
    } DOSENVIRONMENT, *PDOSENVIRONMENT;

    APIRET appParseEnvironment(const char *pcszEnv,
                               PDOSENVIRONMENT pEnv);

    APIRET appGetEnvironment(PDOSENVIRONMENT pEnv);

    PSZ* appFindEnvironmentVar(PDOSENVIRONMENT pEnv,
                               PCSZ pszVarName);

    APIRET appSetEnvironmentVar(PDOSENVIRONMENT pEnv,
                                PCSZ pszNewEnv,
                                BOOL fAddFirst);

    APIRET appSetEnvironmentVars(PDOSENVIRONMENT pEnv,
                                 PCSZ pcszEnv);

    APIRET appConvertEnvironment(PDOSENVIRONMENT pEnv,
                                 PSZ *ppszEnv,
                                 PULONG pulSize);

    APIRET appFreeEnvironment(PDOSENVIRONMENT pEnv);

    #ifdef INCL_WINPROGRAMLIST
        // moved extra PROG_* defs to exeh.h V1.0.1 (2003-01-17) [umoeller]

        APIRET appQueryAppType(const char *pcszExecutable,
                               PULONG pulDosAppType,
                               PULONG pulWinAppType);

        ULONG appIsWindowsApp(ULONG ulProgCategory);

    /* ******************************************************************
     *
     *   Application start
     *
     ********************************************************************/

        APIRET appQueryDefaultWin31Environment(PSZ *ppsz);

        #define APP_RUN_FULLSCREEN      0x0001
        #define APP_RUN_ENHANCED        0x0002
        #define APP_RUN_STANDARD        0x0004
        #define APP_RUN_SEPARATE        0x0008

        APIRET appBuildProgDetails(PPROGDETAILS *ppDetails,
                                   const PROGDETAILS *pcProgDetails,
                                   ULONG ulFlags);

        APIRET XWPENTRY appStartApp(HWND hwndNotify,
                                    const PROGDETAILS *pcProgDetails,
                                    ULONG ulFlags,
                                    HAPP *phapp,
                                    ULONG cbFailingName,
                                    PSZ pszFailingName);

        BOOL XWPENTRY appWaitForApp(HWND hwndNotify,
                                    HAPP happ,
                                    PULONG pulExitCode);

        APIRET appQuickStartApp(const char *pcszFile,
                                ULONG ulProgType,
                                const char *pcszArgs,
                                const char *pcszWorkingDir,
                                HAPP *phapp,
                                PULONG pulExitCode);

        APIRET appOpenURL(PCSZ pcszURL,
                          PSZ pszAppStarted,
                          ULONG cbAppStarted);

    #endif

#endif

#if __cplusplus
}
#endif

