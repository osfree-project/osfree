
/*
 *@@sourcefile drivdlgs.h:
 *      header file for driver configuration dialogs on
 *      the "Drivers" page in the "OS/2 kernel" object.
 *
 *@@added V0.9.0
 *@@include #include <os2.h>
 *@@include #include "config\drivdlgs.h"
 */

/*
 *      Copyright (C) 1999-2003 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define DRVF_CMDREF     0x0001      // driver is documented in CMDREF
#define DRVF_NOPARAMS   0x0002      // driver accepts no parameters
#define DRVF_CONFIGTOOL 0x0004      // driver is documented in CFGDAT.INF ConfigTool document
                                    // added V1.0.4 (2005-05-07) [chennecke]

#define DRVF_BASEDEV    0x00010000  // BASEDEV=
#define DRVF_DEVICE     0x00020000  // DEVICE=
#define DRVF_IFS        0x00040000  // IFS=
#define DRVF_OTHER      0x00080000  // RUN=, CALL=, ?!?

typedef struct _DRIVERDLGDATA *PDRIVERDLGDATA;

typedef BOOL EXPENTRY FNSHOWDRIVERDLG(HWND hwndOwner,
                                      PDRIVERDLGDATA pDlgData);
typedef FNSHOWDRIVERDLG *PFNSHOWDRIVERDLG;

/*
 *@@ DRIVERSPEC:
 *      structure for declaring drivers known to
 *      the "OS/2 Kernel" object. One of these
 *      is created for each driver which can
 *      be displayed on the "Drivers" page,
 *      according to the NLS DRVRSxxx.TXT file
 *      in the XWorkplace /BIN/ subdirectory.
 *      This is done by cfgDriversInitPage (xfsys.c).
 *
 *@@added V0.9.0
 *@@changed V0.9.3 (2000-04-10) [umoeller]: added pszVersion to DRIVERSPEC
 */

typedef struct _DRIVERSPEC
{
    PSZ        pszKeyword;          // e.g. "BASEDEV="
    PSZ        pszFilename;         // e.g. "IBM1S506.ADD", without path, upper-cased
    PSZ        pszDescription;      // e.g. "IBM IDE driver"
    PSZ        pszVersion;          // driver version or NULL if n/a
    ULONG      ulFlags;             // DRVF_* flags

    PFNSHOWDRIVERDLG pfnShowDriverDlg;   // function responsible for showing the dialog
                                    // (called when "Configure" button gets pressed)

    /* HMODULE    hmodConfigDlg;       // module handle with dlg template or null if no dlg exists
    ULONG      idConfigDlg;         // resource ID of config dlg or null if no dlg exists
    PFNWP      pfnwpConfigure;      // dialog proc for "Configure" dialog or null if none
    */
} DRIVERSPEC, *PDRIVERSPEC;

/*
 *@@ DRIVERDLGDATA:
 *      structure for communication between a
 *      driver dialog func and the main "Drivers"
 *      notebook page in "OS/2 Kernel".
 *      See the top of drivdlgs.c for details.
 *
 *@@added V0.9.0
 */

typedef struct _DRIVERDLGDATA
{
    PVOID       pvKernel;
    const char  *pcszKernelTitle;               // in: title of OS/2 Kernel object
    const DRIVERSPEC *pDriverSpec;              // in: driver specs; do not modify
    CHAR        szParams[500];                  // in/out: as in CONFIG.SYS
    PVOID       pvUser;                         // for data needed in dialog
} DRIVERDLGDATA;

typedef ULONG EXPENTRY FNCHECKDRIVERNAME(HMODULE hmodPlugin,
                                         HMODULE hmodXFLDR,
                                         PDRIVERSPEC pSpec,
                                         PSZ pszErrorMsg);
typedef FNCHECKDRIVERNAME *PFNCHECKDRIVERNAME;

BOOL APIENTRY drvLoadPlugins(HAB hab);
VOID APIENTRY drvUnloadPlugins(VOID);
BOOL APIENTRY drvConfigSupported(PDRIVERSPEC pSpec);

// exports

BOOL APIENTRY drvDisplayHelp(PVOID pvKernel,
                             PCSZ pcszHelpFile,
                             ULONG ulHelpPanel);
typedef BOOL APIENTRY DRVDISPLAYHELP(PVOID pvKernel,
                                     PCSZ pcszHelpFile,
                                     ULONG ulHelpPanel);
typedef DRVDISPLAYHELP *PDRVDISPLAYHELP;

int APIENTRY drv_memicmp(void *buf1, void *buf2, unsigned int cnt);
typedef int APIENTRY DRV_MEMICMP(void *buf1, void *buf2, unsigned int cnt);
typedef DRV_MEMICMP *PDRV_MEMICMP;

int drv_sprintf(char *pBuf, PCSZ pcszFormat, ...);
typedef int DRV_SPRINTF(char *pBuf, PCSZ pcszFormat, ...);
typedef DRV_SPRINTF *PDRV_SPRINTF;

char* APIENTRY drv_strtok(char *string1, PCSZ string2);
typedef char* APIENTRY DRV_STRTOK(char *string1, PCSZ string2);
typedef DRV_STRTOK *PDRV_STRTOK;

char* drv_strstr(const char *string1, const char *string2);
typedef char* DRV_STRSTR(const char *string1, const char *string2);
typedef DRV_STRSTR *PDRV_STRSTR;



