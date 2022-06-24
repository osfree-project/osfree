
/*
 *@@sourcefile init.c:
 *      this file contains the XWorkplace initialization code
 *      which runs at WPS startup.
 *
 *      This code used to be in kernel.c and has been moved
 *      to this file with V0.9.16.
 *
 *      There are two entry points into all this:
 *
 *      --  initMain gets called when XFldObject gets initialized,
 *          i.e. right when the WPS starts up. This allows us
 *          to do lots of things even before the WPS is fully
 *          initialized and the Desktop folder opens.
 *
 *      --  initRepairDesktopIfBroken gets called later after
 *          the file-system classes are initialized.
 *
 *      --  initDesktopPopulated gets called after the desktop
 *          has populated to perform startup folder processing
 *          and other things.
 *
 *@@header "shared\init.h"
 */

/*
 *      Copyright (C) 1997-2008 Ulrich M”ller.
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

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSQUEUES
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINTIMER
#define INCL_WINSYS
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINPROGRAMLIST     // needed for PROGDETAILS, wppgm.h
#define INCL_WINSWITCHLIST
#define INCL_WINSHELLDATA
#define INCL_WINSTDFILE
#include <os2.h>
// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <io.h>                 // access etc.
#include <fcntl.h>
#include <sys\stat.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\apps.h"               // application helpers
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\procstat.h"           // DosQProcStat handling
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\textview.h"           // PM XTextView control
#include "helpers\tree.h"               // red-black binary trees
#include "helpers\winh.h"               // PM helper routines
#define INCLUDE_WPHANDLE_PRIVATE
#include "helpers\wphandle.h"           // file-system object handles
#include "helpers\xstring.h"            // extended string helpers
#include "helpers\xwpsecty.h"           // XWorkplace Security

// SOM headers which don't crash with prec. header files
#include "xfstart.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "xwpapi.h"                     // public XWorkplace definitions

#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\init.h"                // XWorkplace initialization
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#include "filesys\folder.h"             // XFolder implementation
#include "filesys\refresh.h"            // folder auto-refresh
#include "filesys\xthreads.h"           // extra XWorkplace threads

#include "media\media.h"                // XWorkplace multimedia support

#include "startshut\archives.h"         // archiving declarations
#include "startshut\shutdown.h"         // XWorkplace eXtended Shutdown

// headers in /hook
#include "hook\xwphook.h"

// other SOM headers
#pragma hdrstop

#pragma library("SOMTK")

/* ******************************************************************
 *
 *   Private prototypes
 *
 ********************************************************************/

VOID krnCreateObjectWindows(VOID);

#ifdef __XWPMEMDEBUG__
VOID krnMemoryError(PCSZ pcszMsg);
#endif

VOID cmnLoadGlobalSettings(VOID);

BOOL cmnTurboFoldersEnabled(VOID);

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// extern KERNELGLOBALS    G_KernelGlobals;            // kernel.c

// Desktop startup date and time (initMain) moved here V1.0.1 (2002-12-20) [umoeller]
static DATETIME         G_StartupDateTime = {0};

extern PIBMDRIVEDATA    G_paDriveData = NULL;
extern ULONG            G_ulDriveDataType = 0; // V1.0.4 (2005-10-09) [pr]

static THREADINFO       G_tiSentinel = {0};

#define DESKTOP_VALID               0
#define NO_ACTIVE_HANDLES           1       // V1.0.0 (2002-09-20) [umoeller]
#define HANDLES_BROKEN              2       // V0.9.20 (2002-08-04) [umoeller]
#define NO_DESKTOP_ID               3
#define DESKTOP_HANDLE_NOT_FOUND    4
#define DESKTOP_DIR_DOESNT_EXIST    5
#define DESKTOP_IS_NO_DIRECTORY     6

static ULONG            G_ulDesktopValid = -1;      // unknown at this point
static APIRET           G_arcHandles = 0;               // V0.9.20 (2002-08-04) [umoeller]

static HOBJECT          G_hobjDesktop;
static CHAR             G_szDesktopPath[CCHMAXPATH];

static HMTX             G_hmtxLog = NULLHANDLE;

static PXFILE           G_pStartupLogFile = NULL;

static ULONG            G_flPanic = 0;              // panic flags moved here V1.0.1 (2002-12-20) [umoeller]

/* ******************************************************************
 *
 *   Startup logging
 *
 ********************************************************************/

/*
 *@@ LockLog:
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

BOOL LockLog(VOID)
{
    if (G_hmtxLog)
        return !DosRequestMutexSem(G_hmtxLog, SEM_INDEFINITE_WAIT);

    // first call:
    return !DosCreateMutexSem(NULL,         // unnamed
                              &G_hmtxLog,
                              0,            // unshared
                              TRUE);        // request now
}

/*
 *@@ UnlockLog:
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

VOID UnlockLog(VOID)
{
    DosReleaseMutexSem(G_hmtxLog);
}

/*
 *@@ initLog:
 *      logs the given printf string to xwpstart.log.
 *      This func uses a mutex for serialization.
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

void initLog(const char* pcszFormat,
             ...)
{
    APIRET arc = NO_ERROR;
    BOOL fLocked;

    if (fLocked = LockLog())
    {
        if (G_pStartupLogFile && pcszFormat)
        {
            DATETIME dt;
            CHAR szTemp[2000];
            ULONG   ulLength;

            DosGetDateTime(&dt);
            if (ulLength = sprintf(szTemp,
                                   "%04d-%02d-%02d %02d:%02d:%02d:%02d T%03d ",
                                   dt.year, dt.month, dt.day,
                                   dt.hours, dt.minutes, dt.seconds, dt.hundredths,
                                   doshMyTID()))
            {
                if (!(arc = doshWrite(G_pStartupLogFile,
                                      ulLength,
                                      szTemp)))
                {
                    va_list arg_ptr;
                    va_start(arg_ptr, pcszFormat);
                    ulLength = vsprintf(szTemp, pcszFormat, arg_ptr);
                    va_end(arg_ptr);

                    if (G_pStartupLogFile->flOpenMode & XOPEN_BINARY)
                        // if we're in binary mode, we need to add \r too
                        szTemp[ulLength++] = '\r';
                    szTemp[ulLength++] = '\n';

                    arc = doshWrite(G_pStartupLogFile,
                                    ulLength,
                                    szTemp);
                }
            }
        }

        UnlockLog();
    }
}

/* ******************************************************************
 *
 *   Panic flags
 *
 ********************************************************************/

/*
 *@@ initQueryPanicFlags:
 *
 *@@added V1.0.1 (2002-12-20) [umoeller]
 */

ULONG initQueryPanicFlags(VOID)
{
    return G_flPanic;
}

/* ******************************************************************
 *
 *   XWorkplace initialization part 1
 *   (M_XFldObject::wpclsInitData)
 *
 ********************************************************************/

/*
 *@@ G_apszXFolderKeys:
 *      XFolder INI keys to be copied when upgrading
 *      from XFolder to XWorkplace.
 */

static const char **G_appszXFolderKeys[]
        = {
                &INIKEY_GLOBALSETTINGS  , // "GlobalSettings"
                &INIKEY_ACCELERATORS    , // "Accelerators"
#ifndef __NOFOLDERCONTENTS__
                &INIKEY_FAVORITEFOLDERS , // "FavoriteFolders"
#endif
#ifndef __NOQUICKOPEN__
                &INIKEY_QUICKOPENFOLDERS, // "QuickOpenFolders"
#endif
                &INIKEY_WNDPOSSTARTUP   , // "WndPosStartup"
                &INIKEY_WNDPOSNAMECLASH , // "WndPosNameClash"
                &INIKEY_NAMECLASHFOCUS  , // "NameClashLastFocus"
#ifndef __NOCFGSTATUSBARS__
                &INIKEY_STATUSBARFONT   , // "SB_Font"
                &INIKEY_SBTEXTNONESEL   , // "SB_NoneSelected"
                &INIKEY_SBTEXT_WPOBJECT , // "SB_WPObject"
                &INIKEY_SBTEXT_WPPROGRAM, // "SB_WPProgram"
                &INIKEY_SBTEXT_WPFILESYSTEM, // "SB_WPDataFile"
                &INIKEY_SBTEXT_WPURL       , // "SB_WPUrl"
                &INIKEY_SBTEXT_WPDISK   , // "SB_WPDisk"
                &INIKEY_SBTEXT_WPFOLDER , // "SB_WPFolder"
                &INIKEY_SBTEXTMULTISEL  , // "SB_MultiSelected"
                &INIKEY_SB_LASTCLASS    , // "SB_LastClass"
#endif
                &INIKEY_DLGFONT         , // "DialogFont"
                &INIKEY_BOOTMGR         , // "RebootTo"
                &INIKEY_AUTOCLOSE        // "AutoClose"
          };

/*
 *@@ appWaitForApp:
 *      waits for the specified application to terminate.
 *
 *      Returns:
 *
 *      -1: Error starting app (happ was zero, msg box displayed).
 *
 *      Other: Return code of the application.
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 */

STATIC ULONG WaitForApp(PCSZ pcszTitle,
                        HAPP happ)
{
    ULONG   ulrc = -1;

    if (!happ)
    {
        // error:
        PCSZ apsz[] = {pcszTitle};
        cmnMessageBoxExt(NULLHANDLE,
                            121,       // xwp
                            apsz,
                            1,
                            206,       // cannot start %1
                            MB_OK);
    }
    else
    {
        // app started:
        // enter a modal message loop until we get the
        // WM_APPTERMINATENOTIFY for happ. Then we
        // know the app is done.
        HWND    hwndThread1Object = krnQueryGlobals()->hwndThread1Object;
        HAB     hab = WinQueryAnchorBlock(hwndThread1Object);
        QMSG    qmsg;
        // ULONG   ulXFixReturnCode = 0;
        while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
        {
            if (    (qmsg.msg == WM_APPTERMINATENOTIFY)
                 && (qmsg.hwnd == hwndThread1Object)
                 && (qmsg.mp1 == (MPARAM)happ)
               )
            {
                // xfix has terminated:
                // get xfix return code from mp2... this is:
                // -- 0: everything's OK, continue.
                // -- 1: handle section was rewritten, restart Desktop
                //       now.
                ulrc = (ULONG)qmsg.mp2;
                // do not dispatch this
                break;
            }

            WinDispatchMsg(hab, &qmsg);
        }
    }

    return ulrc;
}

static const CONTROLDEF
#ifndef __NOBOOTLOGO__
    SkipBootLogoCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_PANIC_SKIPBOOTLOGO),
#endif
#ifndef __NOXWPSTARTUP__
    SkipStartupFolderCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_PANIC_SKIPXFLDSTARTUP),
#endif
#ifndef __NOQUICKOPEN__
    SkipQuickOpenCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_PANIC_SKIPQUICKOPEN),
#endif
    SkipArchivingCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_PANIC_NOARCHIVING),
#ifndef __NEVERCHECKDESKTOP__
    DisableCheckDesktopCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_PANIC_DISABLECHECKDESKTOP),
#endif
    DisableReplRefreshCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_PANIC_DISABLEREPLREFRESH),
#ifndef __NOTURBOFOLDERS__
    DisableTurboFoldersCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_PANIC_DISABLETURBOFOLDERS),
#endif
    DisableFeaturesCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_PANIC_DISABLEFEATURES),
#ifndef __NOICONREPLACEMENTS__
    DisableReplIconsCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_PANIC_DISABLEREPLICONS),
#endif
    RemoveHotkeysCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_PANIC_REMOVEHOTKEYS),
#ifndef __NOPAGER__
    DisableXPagerCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_PANIC_DISABLEPAGER),
#endif
    DisableMultimediaCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_PANIC_DISABLEMULTIMEDIA),

    // @@fixes 361 V1.0.2 (2003-03-07) [umoeller]
    #define MY_BUTTON_WIDTH (STD_BUTTON_WIDTH * 3 / 2)

    ContinueButton = CONTROLDEF_DEFPUSHBUTTON(LOAD_STRING, ID_XFDI_PANIC_CONTINUE, MY_BUTTON_WIDTH, STD_BUTTON_HEIGHT),
    XFixButton = CONTROLDEF_PUSHBUTTON(LOAD_STRING, ID_XFDI_PANIC_XFIX, MY_BUTTON_WIDTH, STD_BUTTON_HEIGHT),
    CmdButton = CONTROLDEF_PUSHBUTTON(LOAD_STRING, ID_XFDI_PANIC_CMD, MY_BUTTON_WIDTH, STD_BUTTON_HEIGHT),
    ShutdownButton = CONTROLDEF_PUSHBUTTON(LOAD_STRING, ID_XFDI_PANIC_SHUTDOWN, MY_BUTTON_WIDTH, STD_BUTTON_HEIGHT),
    ContinueText = LOADDEF_TEXT(ID_XFDI_PANIC_CONTINUE_TXT),
    XFixText = LOADDEF_TEXT(ID_XFDI_PANIC_XFIX_TXT),
    CmdText = LOADDEF_TEXT(ID_XFDI_PANIC_CMD_TXT),
    ShutdownText = LOADDEF_TEXT(ID_XFDI_PANIC_SHUTDOWN_TXT);

static const DLGHITEM G_dlgPanic[] =
    {
        START_TABLE,            // root table, required
#ifndef __NOBOOTLOGO__
            START_ROW(0),
                CONTROL_DEF(&SkipBootLogoCB),
#endif
#ifndef __NOXWPSTARTUP__
            START_ROW(0),
                CONTROL_DEF(&SkipStartupFolderCB),
#endif
#ifndef __NOQUICKOPEN__
            START_ROW(0),
                CONTROL_DEF(&SkipQuickOpenCB),
#endif
            START_ROW(0),
                CONTROL_DEF(&SkipArchivingCB),
#ifndef __NEVERCHECKDESKTOP__
            START_ROW(0),
                CONTROL_DEF(&DisableCheckDesktopCB),
#endif
            START_ROW(0),
                CONTROL_DEF(&DisableReplRefreshCB),
#ifndef __NOTURBOFOLDERS__
            START_ROW(0),
                CONTROL_DEF(&DisableTurboFoldersCB),
#endif
            START_ROW(0),
                CONTROL_DEF(&DisableFeaturesCB ),
#ifndef __NOICONREPLACEMENTS__
            START_ROW(0),
                CONTROL_DEF(&DisableReplIconsCB),
#endif
            START_ROW(0),
                CONTROL_DEF(&RemoveHotkeysCB),
#ifndef __NOPAGER__
            START_ROW(0),
                CONTROL_DEF(&DisableXPagerCB),
#endif
            START_ROW(0),
                CONTROL_DEF(&DisableMultimediaCB),

            START_ROW(ROW_VALIGN_CENTER),
                START_TABLE_ALIGN,
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&ContinueButton),
                        CONTROL_DEF(&ContinueText),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&XFixButton),
                        CONTROL_DEF(&XFixText),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&CmdButton),
                        CONTROL_DEF(&CmdText),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&ShutdownButton),
                        CONTROL_DEF(&ShutdownText),
                END_TABLE,
        END_TABLE
    };

/*
 *@@ StartCmdExe:
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

STATIC APIRET StartCmdExe(HWND hwndNotify,
                          HAPP *phappCmd)
{
    PROGDETAILS pd = {0};
    pd.Length = sizeof(pd);
    pd.progt.progc = PROG_WINDOWABLEVIO;
    pd.progt.fbVisible = SHE_VISIBLE;
    pd.pszExecutable = "*";        // use OS2_SHELL
    return appStartApp(hwndNotify,
                       &pd,
                       0, // V0.9.14
                       phappCmd,
                       0,
                       NULL);
}

/*
 *@@ RunXFix:
 *      starts xfix. Returns TRUE if xfix returned
 *      a non-zero value, i.e. if the handles section
 *      was changed.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

STATIC BOOL RunXFix(VOID)
{
    CHAR        szXfix[CCHMAXPATH];
    PROGDETAILS pd = {0};
    HAPP        happXFix;
    cmnQueryXWPBasePath(szXfix);
    strcat(szXfix, "\\bin\\xfix.exe");

    pd.Length = sizeof(pd);
    pd.progt.progc = PROG_PM;
    pd.progt.fbVisible = SHE_VISIBLE;
    pd.pszExecutable = szXfix;
    if (!appStartApp(krnQueryGlobals()->hwndThread1Object,
                     &pd,
                     0, // V0.9.14
                     &happXFix,
                     0,
                     NULL))
        if (WaitForApp(szXfix,
                       happXFix)
            == 1)
            return TRUE;

    return FALSE;
}

/*
 *@@ ShowPanicDlg:
 *      displays the "Panic" dialog if either fForceShow
 *      is TRUE or the "Shift" key is pressed.
 *
 *@@added V0.9.16 (2001-10-08) [umoeller]
 *@@changed V0.9.16 (2001-10-25) [umoeller]: added "disable refresh", "disable turbo fdrs"
 *@@changed V0.9.17 (2002-02-05) [umoeller]: added fForceShow and "disable check desktop"
 *@@changed V1.0.2 (2003-03-07) [umoeller]: enlarged buttons a bit @@fixes 361
 */

STATIC VOID ShowPanicDlg(BOOL fForceShow)      // V0.9.17 (2002-02-05) [umoeller]
{
    BOOL    fRepeat = fForceShow;

    while (    (doshQueryShiftState())
            || (fRepeat)       // set to TRUE after xfix V0.9.7 (2001-01-24) [umoeller]
          )
    {
        // shift pressed: show "panic" dialog
        ULONG   ulrc = 0;
        APIRET  arc;
        HWND hwndPanic;
        fRepeat = FALSE;

        if (!(arc = dlghCreateDlg(&hwndPanic,
                                  NULLHANDLE,
                                  FCF_FIXED_DLG,
                                  WinDefDlgProc,
                                  cmnGetString(ID_XFDI_PANIC_TITLE),
                                  G_dlgPanic,
                                  ARRAYITEMCOUNT(G_dlgPanic),
                                  NULL,
                                  cmnQueryDefaultFont())))
        {
            winhCenterWindow(hwndPanic);

            // disable items which are irrelevant
#ifndef __NOBOOTLOGO__
            WinEnableControl(hwndPanic, ID_XFDI_PANIC_SKIPBOOTLOGO,

                              cmnQuerySetting(sfBootLogo));
#endif
#ifndef __ALWAYSREPLACEARCHIVING__
            WinEnableControl(hwndPanic, ID_XFDI_PANIC_NOARCHIVING,
                              cmnQuerySetting(sfReplaceArchiving));
#endif
#ifndef __NEVERCHECKDESKTOP__
            WinEnableControl(hwndPanic, ID_XFDI_PANIC_DISABLECHECKDESKTOP,
                              cmnQuerySetting(sfCheckDesktop));
#endif
            WinEnableControl(hwndPanic, ID_XFDI_PANIC_DISABLEREPLREFRESH,
                              krnReplaceRefreshEnabled());
#ifndef __NOTURBOFOLDERS__
            WinEnableControl(hwndPanic, ID_XFDI_PANIC_DISABLETURBOFOLDERS,
                              cmnTurboFoldersEnabled());
#endif
#ifndef __NOICONREPLACEMENTS__
            WinEnableControl(hwndPanic, ID_XFDI_PANIC_DISABLEREPLICONS,
                              cmnQuerySetting(sfIconReplacements));
#endif
#ifndef __NOPAGER__
            WinEnableControl(hwndPanic, ID_XFDI_PANIC_DISABLEPAGER,
                              cmnQuerySetting(sfEnableXPager));
#endif
            WinEnableControl(hwndPanic, ID_XFDI_PANIC_DISABLEMULTIMEDIA,
                              (xmmQueryStatus() == MMSTAT_WORKING));

            ulrc = WinProcessDlg(hwndPanic);

            switch (ulrc)
            {
                case ID_XFDI_PANIC_CONTINUE:        // continue
                {
#ifndef __NOBOOTLOGO__
                    if (winhIsDlgItemChecked(hwndPanic, ID_XFDI_PANIC_SKIPBOOTLOGO))
                        G_flPanic |= SUF_SKIPBOOTLOGO;
#endif
#ifndef __NOXWPSTARTUP__
                    if (winhIsDlgItemChecked(hwndPanic, ID_XFDI_PANIC_SKIPXFLDSTARTUP))
                        G_flPanic |= SUF_SKIPXFLDSTARTUP;
#endif
#ifndef __NOQUICKOPEN__
                    if (winhIsDlgItemChecked(hwndPanic, ID_XFDI_PANIC_SKIPQUICKOPEN))
                        G_flPanic |= SUF_SKIPQUICKOPEN;
#endif
                    if (winhIsDlgItemChecked(hwndPanic, ID_XFDI_PANIC_NOARCHIVING))
                    {
                        PARCHIVINGSETTINGS pArcSettings = arcQuerySettings();
                        // disable "check archives" flag
                        pArcSettings->ulArcFlags &= ~ARCF_ENABLED;
                    }

#ifndef __NEVERCHECKDESKTOP__
                    if (winhIsDlgItemChecked(hwndPanic, ID_XFDI_PANIC_DISABLECHECKDESKTOP))
                        cmnSetSetting(sfCheckDesktop, FALSE);
#endif
                    if (winhIsDlgItemChecked(hwndPanic, ID_XFDI_PANIC_DISABLEREPLREFRESH))
                        krnEnableReplaceRefresh(FALSE);
#ifndef __NOTURBOFOLDERS__
                    if (winhIsDlgItemChecked(hwndPanic, ID_XFDI_PANIC_DISABLETURBOFOLDERS))
                        cmnSetSetting(sfTurboFolders, FALSE);
#endif

#ifndef __NOICONREPLACEMENTS__
                    if (winhIsDlgItemChecked(hwndPanic, ID_XFDI_PANIC_DISABLEREPLICONS))
                        cmnSetSetting(sfIconReplacements, FALSE);
#endif
#ifndef __NOPAGER__
                    if (winhIsDlgItemChecked(hwndPanic, ID_XFDI_PANIC_DISABLEPAGER))
                        cmnSetSetting(sfEnableXPager, FALSE);  // @@todo
#endif
                    if (winhIsDlgItemChecked(hwndPanic, ID_XFDI_PANIC_DISABLEMULTIMEDIA))
                    {
                        xmmDisable();
                    }
                    if (winhIsDlgItemChecked(hwndPanic, ID_XFDI_PANIC_DISABLEFEATURES))
                        cmnSetDefaultSettings(0);       // reset all!
                    if (winhIsDlgItemChecked(hwndPanic, ID_XFDI_PANIC_REMOVEHOTKEYS))
                        PrfWriteProfileData(HINI_USER,
                                            INIAPP_XWPHOOK,
                                            INIKEY_HOOK_HOTKEYS,
                                            0, 0);      // delete INI key
                }
                break;

                case ID_XFDI_PANIC_XFIX:      // run xfix:
                    if (RunXFix())
                    {
                        // handle section changed:
                        cmnMessageBoxExt(NULLHANDLE,
                                         121,       // xwp
                                         NULL, 0,
                                         205,       // restart wps now.
                                         MB_OK);
                        DosExit(EXIT_PROCESS, 0);
                    }

                    fRepeat = TRUE;
                break;

                case ID_XFDI_PANIC_CMD:         // run cmd.exe
                {
                    HAPP happCmd;
                    if (!StartCmdExe(krnQueryGlobals()->hwndThread1Object,
                                     &happCmd))
                        WaitForApp(getenv("OS2_SHELL"),
                                   happCmd);
                }
                break;

                case ID_XFDI_PANIC_SHUTDOWN:        // shutdown
                    // "Shutdown" pressed:
                    WinShutdownSystem(WinQueryAnchorBlock(HWND_DESKTOP),
                                      WinQueryWindowULong(HWND_DESKTOP, QWL_HMQ));
                    while (TRUE)
                        DosSleep(1000);
            }

            winhDestroyWindow(&hwndPanic);
        }
    }
}

/*
 *@@ ShowStartupDlgs:
 *      this gets called from initMain
 *      to show dialogs while the WPS is starting up.
 *
 *      If XWorkplace was just installed, we'll show
 *      an introductory page and offer to convert
 *      XFolder settings, if found.
 *
 *      If XWorkplace has just been installed, we show
 *      an introductory message that "Shift" will show
 *      the "panic" dialog.
 *
 *      If "Shift" is currently pressed, we'll show the
 *      "Panic" dialog.
 *
 *@@added V0.9.1 (99-12-18) [umoeller]
 *@@changed V0.9.18 (2002-02-06) [umoeller]: fixed multiple XFolder conversions
 */

STATIC VOID ShowStartupDlgs(VOID)
{
    ULONG   cbData = 0;
    PCSZ    pcszXFolderConverted = "_XFolderConv";

    // check if XWorkplace was just installed
#ifndef __XWPLITE__
    if (PrfQueryProfileInt(HINI_USER,
                           (PSZ)INIAPP_XWORKPLACE,
                           (PSZ)INIKEY_JUSTINSTALLED,
                           0x123) != 0x123)
    {
        // yes: explain the "Panic" feature
        cmnMessageBoxExt(HWND_DESKTOP,
                         121,       // "XWorkplace"
                         NULL, 0,
                         159,       // "press shift for panic"
                         MB_OK);
    }
#endif

    /*
     * convert XFolder settings
     *
     */

    // this no longer works V0.9.18 (2002-02-06) [umoeller]
    /* if (PrfQueryProfileSize(HINI_USER,
                            (PSZ)INIAPP_XWORKPLACE,
                            (PSZ)INIKEY_GLOBALSETTINGS,
                            &cbData)
            == FALSE) */
    {
        // XWorkplace keys do _not_ exist:

        // check if we have old XFolder settings
        if (    (PrfQueryProfileSize(HINI_USER,
                                     (PSZ)INIAPP_OLDXFOLDER,
                                     (PSZ)INIKEY_GLOBALSETTINGS,
                                     &cbData))
             // have those not yet been converted?
             // V0.9.18 (2002-02-06) [umoeller]
             && (!PrfQueryProfileSize(HINI_USER,
                                      (PSZ)INIAPP_XWORKPLACE,
                                      (PSZ)pcszXFolderConverted,
                                      &cbData))
           )
        {
            if (cmnMessageBoxExt(HWND_DESKTOP,
                                 121,       // "XWorkplace"
                                 NULL, 0,
                                 160,       // "convert?"
                                 MB_YESNO)
                    == MBID_YES)
            {
                // yes, convert:
                // copy keys from "XFolder" to "XWorkplace"
                ULONG   ul;
                for (ul = 0;
                     ul < sizeof(G_appszXFolderKeys) / sizeof(G_appszXFolderKeys[0]);
                     ul++)
                {
                    prfhCopyKey(HINI_USER,
                                INIAPP_OLDXFOLDER,      // source
                                *G_appszXFolderKeys[ul],
                                HINI_USER,
                                INIAPP_XWORKPLACE);
                }

                cmnLoadGlobalSettings();
            }

            // in any case, set "converted" flag
            // so we won't prompt again
            PrfWriteProfileString(HINI_USER,
                                  (PSZ)INIAPP_XWORKPLACE,
                                  (PSZ)pcszXFolderConverted,
                                  "1");
        }
    }

    /*
     * "Panic" dlg
     *
     */

    ShowPanicDlg(FALSE);        // no force

// #ifndef __ALWAYSSUBCLASS__
//     if (getenv("XWP_NO_SUBCLASSING"))
//         // V0.9.3 (2000-04-26) [umoeller]
//         cmnSetSetting(sfNoSubclassing, TRUE);
// #endif
}

/*
 *@@ ReplaceWheelWatcher:
 *      blocks out the standard WPS "WheelWatcher" thread
 *      (which usually does the DosFindNotify* stuff)
 *      and creates a new thread in XWP instead.
 *
 *      Gets called _after_ the panic dialog, but _before_
 *      the additional XWP threads are started.
 *
 *@@added V0.9.9 (2001-01-31) [umoeller]
 *@@changed V0.9.10 (2001-04-08) [umoeller]: added exception handling
 */

STATIC VOID ReplaceWheelWatcher(VOID)
{
    APIRET      arc = NO_ERROR;

    TRY_LOUD(excpt1)        // V0.9.10 (2001-04-08) [umoeller]
    {
        HQUEUE      hqWheelWatcher = NULLHANDLE;

        if (G_pStartupLogFile)
        {
            PQPROCSTAT16 pInfo;

            initLog("Entering " __FUNCTION__ ":");

            if (!(arc = prc16GetInfo(&pInfo)))
            {
                // find WPS entry in process info
                PQPROCESS16 pProcess;
                if (pProcess = prc16FindProcessFromPID(pInfo,
                                                       G_pidWPS))
                {
                    // we now have the process info for the second PMSHELL.EXE...
                    ULONG       ul;
                    PQTHREAD16  pThread;

                    initLog("  Running WPS threads at this point:");

                    for (ul = 0, pThread = (PQTHREAD16)PTR(pProcess->ulThreadList, 0);
                         ul < pProcess->usThreads;
                         ul++, pThread++ )
                    {
                        // CHAR    sz[100];
                        HENUM   henum;
                        HWND    hwndThis;
                        initLog("    Thread %02d has priority 0x%04lX",
                                          pThread->usTID,
                                          pThread->ulPriority);

                        henum = WinBeginEnumWindows(HWND_OBJECT);
                        while (hwndThis = WinGetNextWindow(henum))
                        {
                            PID pid;
                            TID tid;
                            if (    (WinQueryWindowProcess(hwndThis, &pid, &tid))
                                 && (pid == G_pidWPS)
                                 && (tid == pThread->usTID)
                               )
                            {
                                CHAR szClass[100];
                                WinQueryClassName(hwndThis, sizeof(szClass), szClass);
                                initLog("        object wnd 0x%lX (%s)",
                                                  hwndThis,
                                                  szClass);
                            }
                        }
                        WinEndEnumWindows(henum);
                    } // end for (ul = 0, pThread =...
                }

                prc16FreeInfo(pInfo);       // V0.9.10 (2001-04-08) [umoeller]
            }
            else
            {
                initLog("  !!! Cannot get WPS thread info, prc16GetInfo returned %d",
                                  arc);
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "prc16GetInfo returned %d.", arc);
            }
        }

        // now lock out the WheelWatcher thread...
        // that thread is started AFTER us, and it attempts to
        // create a CP queue of the below name. If it cannot do
        // that, it will simply exit. So... by creating a queue
        // of the same name, the WheelWatcher will get an error
        // later, and exit.
        arc = DosCreateQueue(&hqWheelWatcher,
                             QUE_FIFO,
                             "\\QUEUES\\FILESYS\\NOTIFY");
        initLog("  Created HQUEUE 0x%lX (DosCreateQueue returned %d)",
                          hqWheelWatcher,
                          arc);

        if (arc == NO_ERROR)
        {
            PKERNELGLOBALS pGlobals;
            // we got the queue: then our assumption was valid
            // that we are indeed running _before_ WheelWatcher here...
            // create our own thread instead
            thrCreate(&G_tiSentinel,
                      refr_fntSentinel,
                      NULL,
                      "NotifySentinel",
                      THRF_WAIT,
                      0);           // no data here

            initLog("  Started XWP Sentinel thread, TID: %d",
                              G_tiSentinel.tid);

            if (pGlobals = krnLockGlobals(__FILE__, __LINE__, __FUNCTION__))
            {
                pGlobals->fAutoRefreshReplaced = TRUE;
                krnUnlockGlobals();
            }
        }
    }
    CATCH(excpt1) {} END_CATCH();
}

/*
 *@@ CheckDesktop:
 *      checks if <WP_DESKTOP> can be found on the system.
 *
 *      Returns:
 *
 *      --  DESKTOP_VALID
 *
 *      --  HANDLES_BROKEN: cannot load handles.
 *
 *      --  NO_DESKTOP_ID: <WP_DESKTOP> doesn't exist in OS2.INI.
 *
 *      --  DESKTOP_HANDLE_NOT_FOUND: <WP_DESKTOP> exists, but points
 *          to an invalid handle.
 *
 *      --  DESKTOP_DIR_DOESNT_EXIST: The handle pointed to by <WP_DESKTOP>
 *          points to a directory which doesn't exist.
 *
 *      --  DESKTOP_IS_NO_DIRECTORY: The handle pointed to by <WP_DESKTOP>
 *          points to a file, not a directory.
 *
 *      Gets called from initMain.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

STATIC ULONG CheckDesktop(HHANDLES hHandles)       // in: handles buffer from wphLoadHandles
{
    ULONG   ulResult = DESKTOP_VALID;

    ULONG   cb = sizeof(HOBJECT);
    G_hobjDesktop = NULLHANDLE;
    G_szDesktopPath[0] = '\0';

    initLog("Entering " __FUNCTION__ ":");

    if (!PrfQueryProfileData(HINI_USER,
                             (PSZ)WPINIAPP_LOCATION,      // "PM_Workplace:Location"
                             (PSZ)WPOBJID_DESKTOP,        // "<WP_DESKTOP>"
                             &G_hobjDesktop,
                             &cb))
    {
        initLog("  ERROR: Cannot find <WP_DESKTOP> in PM_Workplace:Location in OS2.INI");
        ulResult = NO_DESKTOP_ID;
    }
    else
        if (!G_hobjDesktop)
        {
            initLog("  ERROR: <WP_DESKTOP> in PM_Workplace:Location in OS2.INI has a null handle");
            ulResult = NO_DESKTOP_ID;
        }
        else
        {
            // OK, check if that handle is valid.
            APIRET arc;
            // is this really a file-system object?
            if (HIUSHORT(G_hobjDesktop) == G_usHiwordFileSystem)
            {
                // use loword only
                USHORT      usObjID = LOUSHORT(G_hobjDesktop);

                memset(G_szDesktopPath, 0, sizeof(G_szDesktopPath));
                if (arc = wphComposePath(hHandles,
                                         usObjID,
                                         G_szDesktopPath,
                                         sizeof(G_szDesktopPath),
                                         NULL))
                {
                    initLog("  ERROR %d resolving <WP_DESKTOP> handle 0x%lX",
                                      arc,
                                      G_hobjDesktop);
                    ulResult = DESKTOP_HANDLE_NOT_FOUND;
                }
                else
                {
                    ULONG ulAttr;

                    initLog("  <WP_DESKTOP> handle 0x%lX points to \"%s\"",
                                      G_hobjDesktop,
                                      G_szDesktopPath);
                    if (arc = doshQueryPathAttr(G_szDesktopPath,
                                                &ulAttr))
                    {
                        initLog("  ERROR: doshQueryPathAttr(%s) returned %d",
                                          G_szDesktopPath,
                                          arc);
                        ulResult = DESKTOP_DIR_DOESNT_EXIST;
                    }
                    else
                        if (0 == (ulAttr & FILE_DIRECTORY))
                        {
                            initLog("  ERROR: Desktop path \"%s\" is not a directory.",
                                              arc);
                            ulResult = DESKTOP_IS_NO_DIRECTORY;
                        }
                }
            }
            else
            {
                initLog("  ERROR: <WP_DESKTOP> handle %d is not a file-system handle (wrong hiword)",
                                  G_hobjDesktop);
                ulResult = DESKTOP_HANDLE_NOT_FOUND;
            }
        }

    return ulResult;
}

/*
 *@@ CheckOneClassOrder:
 *
 *      Returns TRUE if the class list was changed.
 *
 *@@added V1.0.0 (2002-08-26) [umoeller]
 */

BOOL CheckOneClassOrder(PCSZ pcszOriginal,
                        PCSZ pcszXwp,
                        BOOL fShouldBeFirst)
{
    BOOL    brc = FALSE;
    PSZ     pszList;
    ULONG   cbList;

    PMPF_STARTUP(("%s", pcszOriginal));

    if (pszList = prfhQueryProfileData(HINI_USER,
                                       WPINIAPP_REPLACEMENTS, // "PM_Workplace:ReplaceList",
                                       pcszOriginal,
                                       &cbList))
    {
        // build a linked list of classes from the string
        PSZ         pReplThis = pszList;
        LINKLIST    llReplacements;
        ULONG       cReplacements = 0,
                    cIndexXwp = 0,
                    cbTotal = 0;        // re-run in case the list is broken
        XSTRING     strOrderOld,
                    strOrderNew;
        PLISTNODE   pNode,
                    pNodeXwp;
        PCSZ        apcsz[4];

        BOOL        fRewrite = FALSE;

        xstrInit(&strOrderOld, 0);
        xstrInit(&strOrderNew, 0);

        lstInit(&llReplacements,
                FALSE);         // no auto-free, we're storing const strings

        while (*pReplThis)
        {
            ULONG   ulLengthThis;
            BOOL    fBroken = FALSE;
            // the list _should_ be terminated with two nulls,
            // but better be safe than sorry
            if (pReplThis >= pszList + cbList)
                fBroken = TRUE;

            ulLengthThis = strlen(pReplThis);

            if (pReplThis + ulLengthThis > pszList + cbList)
                fBroken = TRUE;

            if (fBroken)
            {
                PMPF_STARTUP(("    format for %s is broken", pcszOriginal));

                // rewrite with two nulls
                if (cReplacements)
                {
                    apcsz[0] = pcszOriginal;
                    if (MBID_YES == cmnMessageBoxExt(NULLHANDLE,
                                                     247, // &xwp; Class Replacements
                                                     apcsz,
                                                     1,
                                                     250,
                                                     MB_YESNO))
                        fRewrite = TRUE;
                }

                break;
            }

            PMPF_STARTUP(("  found replacement class \"%s\"", pReplThis));

            pNode = lstAppendItem(&llReplacements, pReplThis);

            if (!strcmp(pReplThis, pcszXwp))
                pNodeXwp = pNode;

            xstrcat(&strOrderOld, pReplThis, ulLengthThis);
            xstrcatc(&strOrderOld, ' ');

            ++cReplacements;
            pReplThis += ulLengthThis + 1;
            cbTotal += ulLengthThis + 1;
        }

        if (pNodeXwp)
        {
            if (    (    (fShouldBeFirst)
                      && (pNodeXwp != lstQueryFirstNode(&llReplacements))
                    )
                 || (    (!fShouldBeFirst)
                      && (pNodeXwp != lstQueryLastNode(&llReplacements))
                    )
               )
            {
                // order is wrong: reorder
                lstRemoveNode(&llReplacements,
                              pNodeXwp);
                if (fShouldBeFirst)
                    lstInsertItemBefore(&llReplacements,
                                        (PVOID)pcszXwp,
                                        0);
                else
                    lstAppendItem(&llReplacements,
                                  (PVOID)pcszXwp);

                FOR_ALL_NODES(&llReplacements, pNode)
                {
                    xstrcat(&strOrderNew, (PCSZ)pNode->pItemData, 0);
                    xstrcatc(&strOrderNew, ' ');
                }

                apcsz[0] = pcszOriginal;
                apcsz[1] = pcszXwp;
                apcsz[2] = strOrderOld.psz;
                apcsz[3] = strOrderNew.psz;

                if (MBID_YES == cmnMessageBoxExt(NULLHANDLE,
                                                 247, // &xwp; Class Replacements
                                                 apcsz,
                                                 4,
                                                 248,
                                                 MB_YESNO))
                {
                    fRewrite = TRUE;
                }
            }
        }

        if (fRewrite)
        {
            PSZ     pszNew = malloc(cbTotal + 1),
                    pThis = pszNew;
            FOR_ALL_NODES(&llReplacements, pNode)
            {
                ULONG   cbThis = strlen((PCSZ)pNode->pItemData) + 1;
                memcpy(pThis, (PCSZ)pNode->pItemData, cbThis);
                pThis += cbThis;
            }

            *pThis = '\0';

            PrfWriteProfileData(HINI_USER,
                                (PSZ)WPINIAPP_REPLACEMENTS, // "PM_Workplace:ReplaceList",
                                (PSZ)pcszOriginal,
                                pszNew,
                                cbTotal + 1);
            brc = TRUE;
        }

        xstrClear(&strOrderOld);
        xstrClear(&strOrderNew);
        lstClear(&llReplacements);

        free(pszList);
    }

    return brc;
}

/*
 *@@ CheckClassOrder:
 *      runs through the class replacements list in
 *      OS2.INI and checks if the XWP classes are
 *      in the correct order.
 *
 *@@added V1.0.0 (2002-08-26) [umoeller]
 */

VOID CheckClassOrder(VOID)
{
    ULONG   cChanged = 0;

    cChanged += CheckOneClassOrder(G_pcszWPObject,
                                   G_pcszXFldObject,
                                   TRUE);           // should be first
#if 0
    cChanged += CheckOneClassOrder(G_pcszWPDisk,
                                   G_pcszXFldDisk,
                                   TRUE);           // should be first
    cChanged += CheckOneClassOrder(G_pcszWPFileSystem,
                                   G_pcszXWPFileSystem,
                                   TRUE);           // should be first
    cChanged += CheckOneClassOrder(G_pcszWPFolder,
                                   G_pcszXFolder,
                                   TRUE);           // should be first
#endif

    if (cChanged)
    {
        if (MBID_YES == cmnMessageBoxExt(NULLHANDLE,
                                         247, // &xwp; Class Replacements
                                         NULL,
                                         0,
                                         249,
                                         MB_YESNO))
            DosExit(EXIT_PROCESS, 0);
    }
}

/*
 *@@ initMain:
 *      this gets called from M_XFldObject::wpclsInitData
 *      when the WPS is initializing. See remarks there.
 *
 *      From what I've checked, this function always gets
 *      called on thread 1 of PMSHELL.EXE.
 *
 *      As said there, at this point we own the computer
 *      all alone. Roughly speaking, the WPS has the following
 *      status:
 *
 *      --  The SOM kernel appears to be fully initialized.
 *
 *      --  The WPObject class object (_WPObject) is _being_
 *          initialized. I believe that the WPS instantiates
 *          the WPObject class (including its replacements)
 *          explicitly, so at this point, no SOM object
 *          actually exists.
 *
 *      --  A number of WPS threads are already running... I
 *          can count 12 here at the time this function is called.
 *          But they won't interfere with anything we're doing here,
 *          so we can suspend the boot process for as long as we
 *          want to (e.g. for the "panic" dialogs).
 *
 *      So what we're doing here is the following (this is a
 *      bit complex):
 *
 *      a) Initialize XWorkplace's globals: the global settings,
 *         the KERNELGLOBALS, and such.
 *
 *      b) Create the Thread-1 object window (fnwpThread1Object)
 *         and the API object window (fnwpAPIObject).
 *
 *      c) If the "Shift" key is pressed, show the "Panic" dialog
 *         (new with V0.9.0). In that case, we pause the WPS
 *         bootup simply by not returning from this function
 *         until the dialog is dismissed.
 *
 *      d) Hack out the WPS folder auto-refresh threads, if enabled,
 *         and start the Sentinel thread (see ReplaceWheelWatcher).
 *
 *      e) Call CheckDesktop().
 *
 *      f) Call xthrStartThreads to have the additional XWorkplace
 *         threads started. The Speedy thread will then display the
 *         boot logo, if allowed.
 *
 *      g) Call arcCheckIfBackupNeeded (archives.c)
 *         to enable Desktop archiving, if necessary. The WPS will
 *         then archive the Desktop, after we return from this
 *         function (also new with V0.9.0).
 *
 *      h) Start the XWorkplace daemon (XWPDAEMN.EXE, xwpdaemn.c),
 *         which will register the XWorkplace hook (XWPHOOK.DLL,
 *         xwphook.c, all new with V0.9.0). See xwpdaemon.c for details.
 *
 *      This now returns TRUE if the WPS is currently in the
 *      process of starting up and FALSE if wpclsInitData got
 *      called while the WPS was registering classes while it
 *      was already running. V1.0.1 (2003-01-25) [umoeller]
 *
 *@@changed V0.9.0 [umoeller]: renamed from xthrInitializeThreads
 *@@changed V0.9.0 [umoeller]: added dialog for shift key during Desktop startup
 *@@changed V0.9.0 [umoeller]: added XWorkplace daemon/hook
 *@@changed V0.9.0 [umoeller]: added Desktop archiving
 *@@changed V0.9.1 (99-12-19) [umoeller]: added NumLock at startup
 *@@changed V0.9.3 (2000-04-27) [umoeller]: added PM error windows
 *@@changed V0.9.5 (2000-08-10) [umoeller]: added XWPSHELL interface
 *@@changed V0.9.7 (2000-12-13) [umoeller]: moved config.sys path composition here
 *@@changed V0.9.7 (2000-12-17) [umoeller]: got crashes if archiving displayed a msg box; moved archiving up
 *@@changed V0.9.9 (2001-03-23) [umoeller]: added API object window
 *@@changed V0.9.14 (2001-08-21) [umoeller]: finally added setting for writing startup log
 *@@changed V0.9.16 (2001-09-26) [umoeller]: renamed from krnInitializeXWorkplace
 *@@changed V0.9.16 (2001-09-29) [umoeller]: added CheckDesktopValid()
 *@@changed V0.9.17 (2002-02-05) [umoeller]: added option to stop checking for broken desktops
 *@@changed V0.9.19 (2002-04-02) [umoeller]: fixed wrong pager settings after logoff
 *@@changed V0.9.19 (2002-05-01) [umoeller]: changed name of startup log file
 *@@changed V1.0.0 (2002-08-26) [umoeller]: added checks for proper class replacements ordering
 *@@changed V1.0.0 (2002-09-17) [umoeller]: added daemon NLS init
 *@@changed V1.0.1 (2002-12-08) [umoeller]: fixed stupid hwndDaemonObject log message @@fixes 64
 *@@changed V1.0.1 (2003-01-25) [umoeller]: moved window testing in here from wpclsInitData, returning BOOL now
 */

BOOL initMain(VOID)
{
    CHAR        szPath[CCHMAXPATH];
    HENUM       henum;
    HWND        hwndThis;
    PCSETTINGINFO pSettInfo;
    BOOL        fNoExcptBeeps = TRUE,
                fWriteXWPStartupLog = TRUE,
                fLogLocked = FALSE;

    BOOL        brc = FALSE;

    // initialize global vars V0.9.19 (2002-04-24) [umoeller]
    G_fIsWarp4 = doshIsWarp4();
    G_pidWPS = doshMyPID();
    G_tidWorkplaceThread = doshMyTID();
    DosGetDateTime(&G_StartupDateTime);

    winhInitGlobals();          // V1.0.1 (2002-11-30) [umoeller]

    krnInit();                  // V1.0.2 (2003-11-13) [umoeller]

    // Alright, the rest of this code was greatly reordered to
    // allow for getting as much code as possible into the
    // TRY block. Since I suspect that the desktop hangs that
    // many people were getting with XWP 1.0.0 are somehow
    // related to traps in unprotected code, maybe this will
    // help finding out. V1.0.1 (2003-01-25) [umoeller]

    // However, this is not exactly trivial... we must preload
    // the following two global setting fields first in order
    // to be able to do both exception handling and startup
    // logging right:

#ifndef __NOPARANOIA__
    if (pSettInfo = cmnFindSettingInfo(sfNoExcptBeeps))
        fNoExcptBeeps = cmnLoadOneSetting(pSettInfo);
#endif

    if (pSettInfo = cmnFindSettingInfo(sfWriteXWPStartupLog))
        fWriteXWPStartupLog = cmnLoadOneSetting(pSettInfo);

    // moved the following up V0.9.16 (2001-12-08) [umoeller]
#ifdef __XWPMEMDEBUG__
    // set global memory error callback
    G_pMemdLogFunc = krnMemoryError;
#endif

    // now we can register the exception hooks for /helpers/except.c
    excRegisterHooks(krnExceptOpenLogFile,
                     krnExceptExplainXFolder,
                     krnExceptError,
                     !fNoExcptBeeps);

    TRY_LOUD(excpt1)
    {
        APIRET      arc;
        PSZ         pszActiveHandles;
        HOBJECT     hobjDesktop;
        BOOL        fOpenFoldersFound = FALSE;

        if (    (fWriteXWPStartupLog)       // V0.9.14 (2001-08-21) [umoeller]
             && (fLogLocked = LockLog())
           )
        {
            ULONG   cbFile = 0;

            if (    (doshCreateLogFilename(szPath,
                                           STARTUPLOG,
                                           F_ALLOW_BOOTROOT_LOGFILE))
                 && (!(arc = doshOpen(szPath,
                                      XOPEN_READWRITE_APPEND | XOPEN_WRITETHRU,  // not XOPEN_BINARY
                                            // added XOPEN_WRITETHRU V1.0.1 (2003-01-25) [umoeller]
                                      &cbFile,
                                      &G_pStartupLogFile)))
               )
            {
                doshWrite(G_pStartupLogFile,
                          0,
                          "\n\nStartup log opened, entering " __FUNCTION__ "\n");
                doshWrite(G_pStartupLogFile,
                          0,
                          "------------------------------------------------------\n\n");

                initLog(__FUNCTION__ ": PID 0x%lX, TID 0x%lX",
                                  G_pidWPS,
                                  G_tidWorkplaceThread);
            }

            UnlockLog();
            fLogLocked = FALSE;
        }

        // force loading of _all_ the global settings
        cmnLoadGlobalSettings();

        // Set eWP defaults so that XWP picks them up if upgrading from eWP to XWP
        // V1.0.4 (2005-05-01) [pr]
#ifdef __ALWAYSEXTSORT__
        cmnSetSetting(sfExtendedSorting, TRUE);
#endif
#ifdef __ALWAYSFDRHOTKEYS__
        cmnSetSetting(sfFolderHotkeys, TRUE);
#endif
#ifdef __ALWAYSFIXCLASSTITLES__
        cmnSetSetting(sfFixClassTitles, TRUE);
#endif
#ifdef __ALWAYSHOOK__
        cmnSetSetting(sfXWPHook, TRUE);
#endif
#ifdef __ALWAYSOBJHOTKEYS__
        // ???
#endif
#ifdef __ALWAYSREPLACEARCHIVING__
        cmnSetSetting(sfReplaceArchiving, TRUE);
#endif
#ifdef __ALWAYSREPLACEFILEEXISTS__
        cmnSetSetting(sfReplaceFileExists, TRUE);
#endif
#ifdef __ALWAYSREPLACEFILEPAGE__
        cmnSetSetting(sfReplaceFilePage, TRUE);
#endif
#ifdef __ALWAYSREPLACEHELP__
        cmnSetSetting(sfHelpReplacements, TRUE);
#endif
#ifdef __ALWAYSREPLACEICONPAGE__
        cmnSetSetting(sfReplaceIconPage, TRUE);
#endif
#ifdef __ALWAYSREPLACEPASTE__
        cmnSetSetting(sfReplacePaste, TRUE);
#endif
#ifdef __ALWAYSREPLACEREFRESH__
        krnEnableReplaceRefresh(TRUE);
#endif
#ifdef __ALWAYSTRASHANDTRUEDELETE__
        cmnSetSetting(sfReplaceDelete, TRUE);
#endif

        cmnInitEntities();

        // wpclsInitData calls exactly once and then never again,
        // but wpclsInitData also gets called during installation
        // while the WPS is registering classes. So check if we
        // have any open folder windows: if so, the WPS is already
        // running, and we should _not_ perform all the remaining
        // initialization. Otherwise, the WPS is currently starting
        // up.
        // Moved the following code here from wpclsInitData (xfobj.c)
        // V1.0.1 (2003-01-25) [umoeller]

        // check if we have any open folder windows;
        // if so, we're not really in the process of starting
        // up. This check is necessary because this class
        // method also gets called when the classes are installed
        // by WinRegisterObjectClass, unfortunately, and we don't
        // want to start threads etc. then.
        initLog("checking desktop windows");
        henum = WinBeginEnumWindows(HWND_DESKTOP);
        while (hwndThis = WinGetNextWindow(henum))
        {
            if (!WinQueryClassName(hwndThis, sizeof(szPath), szPath))
                break;
            else if (!strcmp(szPath, WC_WPFOLDERWINDOW))
            {
                // folder window:
                fOpenFoldersFound = TRUE;
                break; // V1.0.1 (2003-01-25) [umoeller]
            }
        }
        WinEndEnumWindows(henum);

        initLog("  windows checked, fOpenFoldersFound is BOOL %d",
                fOpenFoldersFound);

        if (!fOpenFoldersFound)
        {
            // only if no open folders are found,
            // do the rest!

            krnCreateObjectWindows();       // V0.9.18 (2002-03-27) [umoeller]
                        // sets G_habThread1 as well

            // if shift is pressed, show "Panic" dialog
            // V0.9.7 (2001-01-24) [umoeller]: moved this behind creation
            // of thread-1 window... we need this for starting xfix from
            // the "panic" dlg.
            // NOTE: This possibly changes global settings, so the wheel
            // watcher evaluation must come AFTER this! Same for turbo
            // folders, which is OK because G_fTurboSettingsEnabled is
            // enabled only in M_XWPFileSystem::wpclsInitData (because
            // it requires XWPFileSystem to be present)
            ShowStartupDlgs();

            // check if "replace folder refresh" is enabled...
            if (krnReplaceRefreshEnabled())
                // yes: kick out WPS wheel watcher thread,
                // start our own one instead
                ReplaceWheelWatcher();

            /*
             *  enable NumLock at startup
             *      V0.9.1 (99-12-19) [umoeller]
             */

            if (cmnQuerySetting(sfNumLockStartup))
                winhSetNumLock(TRUE);

            /*
             * CheckClassOrder:
             *
             */

            CheckClassOrder();

            // assume defaults for the handle hiwords;
            // these should be correct with 95% of all systems
            // V0.9.17 (2002-02-05) [umoeller]
            G_usHiwordAbstract = 2;
            G_usHiwordFileSystem = 3;

            if (arc = wphQueryActiveHandles(HINI_SYSTEM, &pszActiveHandles))
            {
                // OK, new situation here. If XWP is installed via INI.RC,
                // this code gets called on the first WPS startup before
                // any objects are created. In that case, the handles
                // section does indeed no exist yet. We must not bug the
                // user with a report in that situation. So check if we
                // have the special "fCDBoot" entry in os2.ini, which means
                // that we're currently being installed, and only in that
                // case, set G_ulDesktopValid to DESKTOP_VALID to shut up
                // the message box.
                // V1.0.0 (2002-09-20) [umoeller]
                ULONG   fCDBoot = 0,
                        cb = sizeof(fCDBoot);
                PrfQueryProfileData(HINI_USER,
                                    (PSZ)INIAPP_XWORKPLACE,
                                    "fCDBoot",
                                    &fCDBoot,
                                    &cb);
                initLog("fCDBoot is %d", fCDBoot);

                if (fCDBoot)
                {
                    G_ulDesktopValid = DESKTOP_VALID;
                    // assume the 99% defaults for the hiword stuff
                    G_usHiwordAbstract = 2;
                    G_usHiwordFileSystem = 3;

                    // and delete the key
                    PrfWriteProfileData(HINI_USER,
                                        (PSZ)INIAPP_XWORKPLACE,
                                        "fCDBoot",
                                        NULL,
                                        0);
                }
                else
                {
                    initLog("WARNING: wphQueryActiveHandles returned %d", arc);
                    G_ulDesktopValid = NO_ACTIVE_HANDLES;
                            // this was missing, G_ulDesktopValid was still -1 in
                            // that case V1.0.0 (2002-09-20) [umoeller]
                    G_arcHandles = arc;
                }
            }
            else
            {
                HHANDLES hHandles;

                if (arc = wphLoadHandles(HINI_USER,
                                         HINI_SYSTEM,
                                         pszActiveHandles,
                                         &hHandles))
                {
                    initLog("WARNING: wphLoadHandles returned %d", arc);
                    G_ulDesktopValid = HANDLES_BROKEN;
                    G_arcHandles = arc;
                }
                else
                {
                    // get the abstract and file-system handle hiwords for
                    // future use
                    G_usHiwordAbstract = ((PHANDLESBUF)hHandles)->usHiwordAbstract;
                    G_usHiwordFileSystem = ((PHANDLESBUF)hHandles)->usHiwordFileSystem;

#ifndef __NEVERCHECKDESKTOP__
                    if (cmnQuerySetting(sfCheckDesktop)) // V0.9.17 (2002-02-05) [umoeller]
                    {
                        // go build the handles cache explicitly... CheckDesktop
                        // calls wphComposePath which would call this automatically,
                        // but then we can't catch the error code and people get
                        // complaints about a desktop failure even though only
                        // one handle is invalid which doesn't endanger the system,
                        // so give the user a different warning instead if this
                        // fails, and allow him to disable checks for the future
                        // V0.9.17 (2002-02-05) [umoeller]
                        if (arc = wphRebuildNodeHashTable(hHandles,
                                                          TRUE))        // fail on errors
                        {
                            CHAR sz[30];
                            PCSZ psz = sz;
                            initLog("WARNING: wphRebuildNodeHashTable returned %d", arc);
                            sprintf(sz, "%d", arc);
                            if (cmnMessageBoxExt(NULLHANDLE,
                                                    230,        // desktop error
                                                    &psz,
                                                    1,
                                                    231,        // error %d parsing handles...
                                                    MB_YESNO)
                                    == MBID_YES)
                            {
                                ShowPanicDlg(TRUE);     // force
                            }

                            // in any case, do not let the "cannot find desktop"
                            // dialog come up
                            G_ulDesktopValid = DESKTOP_VALID;
                        }
                        else
                        {
                            // go check if the desktop is valid
                            G_ulDesktopValid = CheckDesktop(hHandles);
                        }
                    } // if (cmnQuerySetting(sfCheckDesktop)) // V0.9.17 (2002-02-05) [umoeller]
                    else
#endif // __NEVERCHECKDESKTOP__
                        G_ulDesktopValid = DESKTOP_VALID;

                    wphFreeHandles(&hHandles);
                }

                free(pszActiveHandles);
            }

            if (G_ulDesktopValid != DESKTOP_VALID)
                // if we couldn't find the desktop, disable archiving
                // V0.9.16 (2001-10-25) [umoeller]
                arcForceNoArchiving();

            // initialize multimedia V0.9.3 (2000-04-25) [umoeller]
            xmmInit();
                    // moved this down V0.9.9 (2001-01-31) [umoeller]

            /*
             *  initialize threads
             *
             */

            xthrStartThreads();

            /*
             *  check Desktop archiving (V0.9.0)
             *      moved this up V0.9.7 (2000-12-17) [umoeller];
             *      we get crashes if a msg box is displayed otherwise
             */

#ifndef __ALWAYSREPLACEARCHIVING__
            if (cmnQuerySetting(sfReplaceArchiving))
#endif
                // check whether we need a WPS backup (archives.c)
                arcCheckIfBackupNeeded(krnQueryGlobals()->hwndThread1Object,
                                       T1M_DESTROYARCHIVESTATUS);

            /*
             *  start XWorkplace daemon (XWPDAEMN.EXE)
             *
             */

            // check for the XWPGLOBALSHARED structure, which
            // is used for communication between the daemon
            // and XFLDR.DLL (see src/Daemon/xwpdaemn.c).
            // We take advantage of the fact that OS/2 keeps
            // reference of the processes which allocate or
            // request access to a block of shared memory.
            // The XWPGLOBALSHARED struct is allocated here
            // (just below) and requested by the daemon.
            //
            // -- If requesting the shared memory works at this point,
            //    this means that the daemon is still running!
            //    This happens after a Desktop restart. We'll then
            //    skip the rest.
            //
            // -- If requesting the shared memory fails, this means
            //    that the daemon is _not_ running (the WPS is started
            //    for the first time). We then allocate the shared
            //    memory and start the daemon, which in turn requests
            //    this shared memory block. Note that this also happens
            //    if the daemon stopped for some reason (crash, kill)
            //    and the user then restarts the WPS.

            arc = DosGetNamedSharedMem((PVOID*)&G_pXwpGlobalShared,
                                       SHMEM_XWPGLOBAL,
                                       PAG_READ | PAG_WRITE);

            initLog("Attempted to access " SHMEM_XWPGLOBAL ", DosGetNamedSharedMem returned %d",
                              arc);

            if (arc)
            {
                // shared mem does not exist:
                // --> daemon not running; probably first WPS
                // startup, so we allocate the shared mem now and
                // start the XWorkplace daemon

                initLog("--> XWPDAEMN not running, starting now.");

                arc = DosAllocSharedMem((PVOID*)&G_pXwpGlobalShared,
                                        SHMEM_XWPGLOBAL,
                                        sizeof(XWPGLOBALSHARED), // rounded up to 4KB
                                        PAG_COMMIT | PAG_READ | PAG_WRITE);

                initLog("  DosAllocSharedMem returned %d",
                                  arc);

                if (!arc)
                {
                    // shared mem successfully allocated:
                    memset(G_pXwpGlobalShared, 0, sizeof(XWPGLOBALSHARED));
                    // store the thread-1 object window, which
                    // gets messages from the daemon
                    G_pXwpGlobalShared->hwndThread1Object = krnQueryGlobals()->hwndThread1Object;
                    G_pXwpGlobalShared->hwndAPIObject = krnQueryGlobals()->hwndAPIObject;
                            // V0.9.9 (2001-03-23) [umoeller]
                    G_pXwpGlobalShared->ulWPSStartupCount = 1;
                    // at the first Desktop start, always process startup folder
                    G_pXwpGlobalShared->fProcessStartupFolder = TRUE;

                    // now start the daemon;
                    krnStartDaemon();

                } // end if DosAllocSharedMem

            } // end if DosGetNamedSharedMem
            else
            {
                // shared memory block already exists:
                // this means the daemon is already running
                // and we have a Desktop restart

                initLog("--> XWPDAEMN already running, refreshing.");

                // store new thread-1 object wnd
                G_pXwpGlobalShared->hwndThread1Object = krnQueryGlobals()->hwndThread1Object;
                G_pXwpGlobalShared->hwndAPIObject = krnQueryGlobals()->hwndAPIObject;
                            // V0.9.9 (2001-03-23) [umoeller]

                // increase Desktop startup count
                ++(G_pXwpGlobalShared->ulWPSStartupCount);

                if (G_pXwpGlobalShared->hwndDaemonObject)
                {
                    BOOL fPager;
                    WinSendMsg(G_pXwpGlobalShared->hwndDaemonObject,
                               XDM_HOOKCONFIG,
                               0, 0);
                    WinSendMsg(G_pXwpGlobalShared->hwndDaemonObject,
                               XDM_HOTKEYSCHANGED,
                               0, 0);
                        // cross-process post, synchronously:
                        // this returns only after the hook has been re-initialized

#ifndef __NOPAGER__
                    // refresh the pager, this might have changed
                    // after the user logs off
                    // V0.9.19 (2002-04-02) [umoeller]
                    fPager = cmnQuerySetting(sfEnableXPager);
                    WinSendMsg(G_pXwpGlobalShared->hwndDaemonObject,
                               XDM_STARTSTOPPAGER,
                               (MPARAM)fPager,
                               0);
                    if (fPager)
                        WinSendMsg(G_pXwpGlobalShared->hwndDaemonObject,
                                   XDM_PAGERCONFIG,
                                   (MPARAM)0xFFFFFFFF,      // all flags
                                   0);
#endif // __NOPAGER__
                }
                // we leave the "reuse startup folder" flag alone,
                // because this was already set by XShutdown before
                // the last Desktop restart

                // in either case, load the strings for daemon NLS
                // support V1.0.0 (2002-09-17) [umoeller]

                // no, don't; moved this call to T1M_DAEMONREADY
                // V1.0.1 (2002-12-08) [umoeller]
                // cmnLoadDaemonNLSStrings();
            }

            /*
             *  interface XWPSHELL.EXE
             *
             */

            arc = DosGetNamedSharedMem((PVOID*)&G_pXWPShellShared,
                                       SHMEM_XWPSHELL,
                                       PAG_READ | PAG_WRITE);
            initLog("Attempted to access " SHMEM_XWPSHELL ", DosGetNamedSharedMem returned %d",
                              arc);

            if (!arc)
            {
                // shared memory exists:
                // this means that XWPSHELL.EXE is running...
                // set flag that WPS termination will not provoke
                // logon; this is in case WPS crashes or user
                // restarts WPS. Only "Logoff" desktop menu item
                // will clear that flag.
                G_pXWPShellShared->fNoLogonButRestart = TRUE;

                initLog("--> XWPSHELL running, refreshed; enabling multi-user mode.");
            }
            else
                initLog("--> XWPSHELL not running, going into single-user mode.");

            // close log only after desktop has populated now
            // V0.9.19 (2002-04-02) [umoeller]
            /*
            doshWriteLogEntry(G_pStartupLogFile,
                              "Leaving " __FUNCTION__", closing log.");
            doshClose(&G_pStartupLogFile);
            */

            // register extra window classes

            ctlRegisterToolbar(G_habThread1);       // V1.0.1 (2002-11-30) [umoeller]
            ctlRegisterSeparatorLine(G_habThread1);
            txvRegisterTextView(G_habThread1);

            // After this, startup continues normally...
            // XWorkplace comes in again after the desktop has been fully populated.
            // Our XFldDesktop::wpPopulate then posts FIM_DESKTOPPOPULATED to the
            // File thread, which will do things like the startup folder and such.
            // This will then start the startup thread (fntStartup), which finally
            // checks for whether XWP was just installed; if so, T1M_WELCOME is
            // posted to the thread-1 object window.

            brc = TRUE;

        } // if (!fOpenFoldersFound)
    }
    CATCH(excpt1)
    {
        initLog("WARNING: Crash during XWorkplace initialization!");
    } END_CATCH();

    if (fLogLocked)
        UnlockLog();

    initLog("Leaving initMain");

    return brc;
}

/* ******************************************************************
 *
 *   XWorkplace initialization part 2
 *   (M_XFldDesktop::wpclsInitData)
 *
 ********************************************************************/

/*
 *@@ initRepairDesktopIfBroken:
 *      calls CheckDesktop to find out if the desktop
 *      is valid. If not, we offer a text entry dialog
 *      where the user may the full path of the desktop.
 *
 *      This does _not_ get called from initMain because
 *      initMain gets processed in the context of
 *      M_XFldObject::wpclsInitData, where the file-system
 *      classes are not yet initialized.
 *
 *      Instead, this gets called from
 *      M_XWPProgram::wpclsInitData (yes, WPProgram), which
 *      gets called late enough in the WPS startup cycle
 *      for all file-system classes to be initialized.
 *      Apparently this is still _before_ the Desktop
 *      attempts to open the <WP_DESKTOP> folder.
 *      Essentially, if the desktop was detected as broken
 *      before, we are then changing the desktop's object
 *      ID behind the WPS's back (before it will try to
 *      find the object from it), so this might or might
 *      not work.
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 *@@changed V1.0.1 (2003-02-02) [umoeller]: fixed XWPProgram breaking half of the system @@fixes 266
 */

BOOL initRepairDesktopIfBroken(VOID)
{
    BOOL        brc = FALSE;

    CHAR        szBootRoot[] = "?:\\",
                szMsg[1000] = "";

    WPFolder    *pBootRootFolder;

    initLog("Entering " __FUNCTION__"...");

#if 0
    // this code causes SOM pretty much to blow up its
    // class management. This causes URL folders to crash
    // and trash and font objects to disappear on my new
    // system too (bug #266). Figured this out finally with
    // V1.0.1 (2003-02-02) [umoeller]

    // get the global pointer to the WPS's drive data
    // array; we KNOW that the boot drive is valid,
    // so we call _wpQueryRootFolder to get the root
    // folder for the boot drive and can call
    // _wpQueryDriveData on that
    szBootRoot[0] = doshQueryBootDrive();
    if (pBootRootFolder = _wpclsQueryObjectFromPath(_WPFileSystem, szBootRoot))
    {
        PIBMDRIVEDATA   pDriveDataBoot;

        initLog("  pBootRootFolder is 0x%lX", pBootRootFolder);

        if (pDriveDataBoot = _wpQueryDriveData(pBootRootFolder))
        {
            initLog("  pDriveDataBoot is 0x%lX", pDriveDataBoot);

            // this is the pointer to the n'th item in the
            // global drive data array; the first index in
            // the global array is for drive A:, so we
            // can simply go back to have the first index
            G_paDriveData =   pDriveDataBoot
                            - (szBootRoot[0] - 'A');        // 0 for A:, 1 for B:, ...
        }
        else
            initLog("  WARNING: _wpQueryDriveData returned NULL for boot root folder 0x%lX",
                     pBootRootFolder);
    }
    else
        initLog("  WARNING: _wpclsQueryObjectFromPath(\"%s\") returned NULL",
                szBootRoot);
#else
    // V1.0.1 (2003-02-02) [umoeller]
    // FsQueryDriveData returns the drive data block for the given
    // drive as well, so we can just call this PMWP export without
    // having to blow up the whole WPS. I have checked,
    // FsQueryDriveData("A:") gives us the same address as the
    // G_paDriveData above.
    // V1.0.2 (2003-02-09) [pr] Moved to XFldDisk::wpclsInitData
    // G_paDriveData = FsQueryDriveData("A:");
    // initLog("FsQueryDriveData(A) = 0x%lX", G_paDriveData);
#endif

    // now check if the desktop was considered valid
    // during initMain()
    switch (G_ulDesktopValid)
    {
        case DESKTOP_VALID:
            brc = TRUE;
        break;

        case NO_ACTIVE_HANDLES:     // V1.0.0 (2002-09-20) [umoeller]
            sprintf(szMsg,
                    "The active handles marker was not found in OS2SYS.INI.");
        break;

        case HANDLES_BROKEN:        // V0.9.20 (2002-08-04) [umoeller]
        {
            XSTRING str;
            xstrInit(&str, 0);
            cmnDescribeError(&str,
                             G_arcHandles,
                             NULL,
                             TRUE);
            sprintf(szMsg,
                    "Error description: %s",
                    str.psz);
            xstrClear(&str);
        }
        break;

        case NO_DESKTOP_ID:
            sprintf(szMsg,
                    "The object ID <WP_DESKTOP> was not found in the user profile.");
        break;

        case DESKTOP_HANDLE_NOT_FOUND:
            sprintf(szMsg,
                    "The object ID <WP_DESKTOP> points to the object handle 0x%lX,"
                    "but that handle was not found in the system profile.",
                    G_hobjDesktop);
        break;

        case DESKTOP_DIR_DOESNT_EXIST:
            sprintf(szMsg,
                    "The object ID <WP_DESKTOP> points to \"%s\","
                    "but that path does not exist on the system.",
                    G_szDesktopPath);
        break;

        case DESKTOP_IS_NO_DIRECTORY:
            sprintf(szMsg,
                    "The object ID <WP_DESKTOP> points to \"%s\","
                    "but that path is a file, not a directory.",
                    G_szDesktopPath);
        break;

        default:
            sprintf(szMsg,
                    "Unknown error %d occurred.",
                    G_ulDesktopValid);
        break;
    }

    if (!brc)
    {
        BOOL fRepeat = FALSE;
        HAPP happCmd;

        PCSZ pcszTitle = "Desktop Error";
        PCSZ pcszOKMsg =
                    "\nIf you press \"OK\", the object ID <WP_DESKTOP> will be set "
                    "upon that directory. Please make sure that the path you enter is "
                    "valid.";
        PCSZ pcszRetryMsg =
                    "\nPress \"Retry\" to enter another path.";
        PCSZ pcszCancelMsg =
                    "\nPress \"Cancel\" in order not to set a new object ID and open "
                    "a temporary desktop, which is the default WPS behavior.";

        // start a CMD.EXE for the frightened user
        StartCmdExe(NULLHANDLE,
                    &happCmd);
        do
        {
            XSTRING str;
            PSZ     pszNew;
            CHAR    szDefault[CCHMAXPATH];
            PSZ     pszDesktopEnv;

            fRepeat = FALSE;

            xstrInitCopy(&str,
                         "Your desktop could not be found in the system's INI files.\n",
                         0);
            xstrcat(&str, szMsg, 0);
            xstrcat(&str,
                    "\nA command window has been opened for your convenience. "
                    "Below you can now attempt to enter the full path of where "
                    "your desktop resides.",
                    0);
            xstrcat(&str,
                    pcszOKMsg,
                    0);
            xstrcat(&str,
                    pcszCancelMsg,
                    0);

            // set a meaningful default for the desktop;
            // if the user has set the DESKTOP variable, use that
            if (!DosScanEnv("DESKTOP",
                            &pszDesktopEnv))
                strlcpy(szDefault, pszDesktopEnv, sizeof(szDefault));
            else
                // check if we can find the path that was last
                // saved during XShutdown
                if (PrfQueryProfileString(HINI_USER,
                                          (PSZ)INIAPP_XWORKPLACE,
                                          (PSZ)INIKEY_LASTDESKTOPPATH,
                                          "",       // default
                                          szDefault,
                                          sizeof(szDefault))
                        < 3)
                {
                    // didn't work either:
                    sprintf(szDefault,
                            "%c:\\Desktop",
                            doshQueryBootDrive());
                }

            if (pszNew = cmnTextEntryBox(NULLHANDLE,
                                         pcszTitle,
                                         str.psz,
                                         szDefault,
                                         CCHMAXPATH - 1,
                                         TEBF_SELECTALL))
            {
                // check that path
                APIRET arc;
                ULONG ulAttr;
                PCSZ pcszMsg2 = NULL;
                if (arc = doshQueryPathAttr(pszNew,
                                            &ulAttr))
                    pcszMsg2 = "The path you have entered (\"%s\") does not exist.";
                else
                    if (0 == (ulAttr & FILE_DIRECTORY))
                        pcszMsg2 = "The path you have entered (\"%s\") is a file, not a directory.";

                if (!pcszMsg2)
                {
                    WPFileSystem *pobj;
                    if (    (pobj = _wpclsQueryObjectFromPath(_WPFileSystem,
                                                              pszNew))
                         && (_wpSetObjectID(pobj,
                                            (PSZ)WPOBJID_DESKTOP))
                       )
                    {
                        // alright, this worked:
                        brc = TRUE;
                    }
                    else
                        pcszMsg2 = "Error setting <WP_DESKTOP> on \"%s\".";
                }

                if (pcszMsg2)
                {
                    sprintf(szMsg,
                            pcszMsg2,
                            pszNew);
                    xstrcpy(&str, szMsg, 0);
                    xstrcat(&str, pcszRetryMsg, 0);
                    xstrcat(&str, pcszCancelMsg, 0);

                    if (cmnMessageBox(NULLHANDLE,
                                      pcszTitle,
                                      str.psz,
                                      NULLHANDLE, // no help
                                      MB_RETRYCANCEL)
                            == MBID_RETRY)
                        fRepeat = TRUE;
                }

                free(pszNew);
            }
            xstrClear(&str);

        } while (fRepeat);
    }

    initLog("Leaving " __FUNCTION__ ", returning BOOL %d.", brc);

    return brc;
}

/* ******************************************************************
 *
 *   XWorkplace initialization part 3
 *   (after desktop is populated)
 *
 ********************************************************************/

/*
 *@@ QUICKOPENDATA:
 *
 *@@added V0.9.12 (2001-04-29) [umoeller]
 *@@changed V0.9.16 (2002-01-13) [umoeller]: moved this here from xthreads.c
 */

typedef struct _QUICKOPENDATA
{
    LINKLIST    llQuicks;            // linked list of quick-open folders,
                                    // plain WPFolder* pointers
    HWND        hwndStatus;

    ULONG       cQuicks,
                ulQuickThis;

    BOOL        fCancelled;

} QUICKOPENDATA, *PQUICKOPENDATA;

/*
 *@@ fnwpQuickOpenDlg:
 *      dlg proc for the QuickOpen status window.
 *
 *@@changed V0.9.12 (2001-04-29) [umoeller]: moved this here from kernel.c
 *@@changed V0.9.16 (2002-01-13) [umoeller]: moved this here from xthreads.c
 */

STATIC MRESULT EXPENTRY fnwpQuickOpenDlg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc;

    switch (msg)
    {
        case WM_INITDLG:
            WinSetWindowPtr(hwnd, QWL_USER, mp2);       // ptr to QUICKOPENDATA
            ctlProgressBarFromStatic(WinWindowFromID(hwnd, ID_SDDI_PROGRESSBAR),
                                     PBA_ALIGNCENTER | PBA_BUTTONSTYLE);
            mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
        break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case DID_CANCEL:
                {
                    PQUICKOPENDATA pqod = (PQUICKOPENDATA)WinQueryWindowPtr(hwnd, QWL_USER);
                    pqod->fCancelled = TRUE;
                    // this will cause the callback below to
                    // return FALSE
                }
                break;

                default:
                    mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
            }
        break;

        case WM_SYSCOMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case SC_HIDE:
                {
                    cmnSetSetting(sfShowStartupProgress, 0);
                    mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
                }
                break;

                default:
                    mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
            }
        break;

        default:
            mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ fncbQuickOpen:
 *      callback func for fdrQuickOpen.
 *
 *      If this returns FALSE, processing is
 *      terminated.
 *
 *@@changed V0.9.12 (2001-04-29) [umoeller]: adjusted parameters for new prototype
 *@@changed V0.9.16 (2002-01-13) [umoeller]: moved this here from xthreads.c
 */

STATIC BOOL _Optlink fncbQuickOpen(WPFolder *pFolder,
                                   WPObject *pObject,
                                   ULONG ulNow,
                                   ULONG ulMax,
                                   ULONG ulCallbackParam)
{
    PQUICKOPENDATA pqod = (PQUICKOPENDATA)ulCallbackParam;

    WinSendDlgItemMsg(pqod->hwndStatus, ID_SDDI_PROGRESSBAR,
                      WM_UPDATEPROGRESSBAR,
                      (MPARAM)(
                                  pqod->ulQuickThis * 100
                                   + ( (100 * ulNow) / ulMax )
                              ),
                      (MPARAM)(pqod->cQuicks * 100));

    // if "Cancel" has been pressed, return FALSE
    return !pqod->fCancelled;
}

#ifndef __NOQUICKOPEN__

/*
 *@@ fntQuickOpenFolders:
 *      synchronous thread created from fntStartupThread
 *      to process the quick-open folders.
 *
 *      The thread data is a PQUICKOPENDATA.
 *
 *@@added V0.9.12 (2001-04-29) [umoeller]
 *@@changed V0.9.16 (2002-01-13) [umoeller]: moved this here from xthreads.c
 *@@changed V1.0.4 (2005-08-21) [pr]: Check return code to allow cancel. @@fixes 460
 */

STATIC void _Optlink fntQuickOpenFolders(PTHREADINFO ptiMyself)
{
    PQUICKOPENDATA pqod = (PQUICKOPENDATA)ptiMyself->ulData;
    PLISTNODE pNode;

    for (pNode = lstQueryFirstNode(&pqod->llQuicks);
         pNode;
         pNode = pNode->pNext)
    {
        WPFolder *pFolder = (WPFolder*)pNode->pItemData;

        if (    (pFolder)
             && (_somIsA(pFolder, _WPFolder))
           )
        {
            if (cmnQuerySetting(sfShowStartupProgress))
            {
                CHAR szTemp[256];
                _wpQueryFilename(pFolder, szTemp, TRUE);
                WinSetDlgItemText(pqod->hwndStatus, ID_SDDI_STATUS, szTemp);

                if (cmnQuerySetting(sfShowStartupProgress))
                    WinSendDlgItemMsg(pqod->hwndStatus, ID_SDDI_PROGRESSBAR,
                                      WM_UPDATEPROGRESSBAR,
                                      (MPARAM)(pqod->ulQuickThis * 100),
                                      (MPARAM)(pqod->cQuicks * 100));
            }

            // V1.0.4 (2005-08-21) [pr]
            if (!(fdrQuickOpen(pFolder,
                               fncbQuickOpen,
                               (ULONG)pqod)))
                break;
        }

        pqod->ulQuickThis++;
    }

    // done: set 100%
    if (cmnQuerySetting(sfShowStartupProgress))
        WinSendDlgItemMsg(pqod->hwndStatus, ID_SDDI_PROGRESSBAR,
                          WM_UPDATEPROGRESSBAR,
                          (MPARAM)1,
                          (MPARAM)1);

    DosSleep(500);

    // tell thrRunSync that we're done
    WinPostMsg(ptiMyself->hwndNotify,
               WM_USER,
               0, 0);
}

#endif

/*
 *@@ fntStartupThread:
 *      startup thread, which does the XWorkplace startup
 *      processing.
 *
 *      This is a transient thread created from the File
 *      thread after the desktop window has been opened
 *      (FIM_DESKTOPPOPULATED). This thread now does
 *      all the startup processing (V0.9.12), replacing
 *      the ugly messaging in the file and worker threads
 *      and with the thread-1 object window we had
 *      previously. The file thread now just starts
 *      this thread and need not worry about anything
 *      else... it doesn't even wait for this to finish.
 *
 *      In detail, this does:
 *
 *      --  startup folder processing;
 *
 *      --  quick-open populate;
 *
 *      --  post-installation processing, i.e. after install,
 *          create the objects and such.
 *
 *      This thread is created with a PM message queue
 *      so we can do WinSendMsg in here. Also, the status
 *      windows are now displayed on this thread.
 *
 *      No thread data.
 *
 *@@added V0.9.12 (2001-04-29) [umoeller]
 *@@changed V0.9.13 (2001-06-14) [umoeller]: removed archive marker file destruction, no longer needed
 *@@changed V0.9.14 (2001-07-28) [umoeller]: added exception handling
 *@@changed V0.9.16 (2002-01-13) [umoeller]: moved this here from xthreads.c
 *@@changed V0.9.19 (2002-04-02) [umoeller]: added startup logging
 *@@changed V1.0.9 (2008-06-18) [pr]: ignore template folders @@fixes 1096
 */

STATIC void _Optlink fntStartupThread(PTHREADINFO ptiMyself)
{
    PCKERNELGLOBALS     pKernelGlobals = krnQueryGlobals();

    initLog("Entering " __FUNCTION__"...");

    // sleep a little while more
    // V0.9.4 (2000-08-02) [umoeller]
    DosSleep(cmnQuerySetting(sulStartupInitialDelay));

    TRY_LOUD(excpt1)
    {
        // moved all this to XFldDesktop::wpOpen, there can
        // be a race which causes this to be called before
        // the desktop is open
        // V0.9.19 (2002-04-02) [umoeller]

        /*
        HWND hwndDesktop;

        if (!(hwndDesktop = cmnQueryActiveDesktopHWND()))
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "cmnQueryActiveDesktopHWND returned NULLHANDLE.");

        // notify daemon of WPS desktop window handle
        // V0.9.9 (2001-03-27) [umoeller]: moved this up,
        // we don't have to wait here
        // V0.9.9 (2001-04-08) [umoeller]: wrong, we do
        // need to wait.. apparently, on some systems,
        // this doesn't work otherwise
        krnPostDaemonMsg(XDM_DESKTOPREADY,
                         (MPARAM)hwndDesktop,
                         (MPARAM)0);
        initLog("  posted desktop HWND 0x%lX to daemon", hwndDesktop);
        */

        /*
         *  startup folders
         *
         */

#ifndef __NOXWPSTARTUP__
        // check if startup folder is to be skipped
        if (!(G_flPanic & SUF_SKIPXFLDSTARTUP))
                // V0.9.3 (2000-04-25) [umoeller]
        {
            // OK, process startup folder
            WPFolder    *pFolder;

            initLog("  processing startup folders...");

            // load XFldStartup class if not already loaded
            if (!_XFldStartup)
            {
                PMPF_STARTUP(("Loading XFldStartup class"));

                XFldStartupNewClass(XFldStartup_MajorVersion,
                                    XFldStartup_MinorVersion);
                // and make sure this is never unloaded
                _wpclsIncUsage(_XFldStartup);
            }

            // find startup folder(s)
            for (pFolder = _xwpclsQueryXStartupFolder(_XFldStartup, NULL);
                 pFolder;
                 pFolder = _xwpclsQueryXStartupFolder(_XFldStartup, pFolder))
            {
                // pFolder now has the startup folder to be processed

                PMPF_STARTUP(("found startup folder %s",
                              _wpQueryTitle(pFolder)));

                // skip folders which should only be started on bootup
                // except if we have specified that we want to start
                // them again when restarting the WPS
                if (   (   (_xwpQueryXStartupType(pFolder) != XSTARTUP_REBOOTSONLY)
                        || (krnNeed2ProcessStartupFolder())
                       )
                    && !(_wpQueryStyle(pFolder) & OBJSTYLE_TEMPLATE)  // V1.0.9 (2008-06-18)
                   )
                {
                    ULONG ulTiming = 0;

                    initLog("    processing folder \"%s\"",
                             _wpQueryTitle(pFolder));

                    if (_somIsA(pFolder, _XFldStartup))
                        ulTiming = _xwpQueryXStartupObjectDelay(pFolder);
                    else
                        ulTiming  = cmnQuerySetting(sulStartupObjectDelay);

                    // start the folder contents synchronously;
                    // this func now displays the progress dialog
                    // and does not return until the folder was
                    // fully processed (this calls another thrRunSync
                    // internally, so the SIQ is not blocked)
                    _xwpStartFolderContents(pFolder,
                                            ulTiming);
                }
                else
                   initLog("    skipping folder \"%s\"",
                             _wpQueryTitle(pFolder));
            }

            PMPF_STARTUP(("done with startup folders"));

            initLog("  done with startup folders");

            // done with startup folders:
            krnSetProcessStartupFolder(FALSE); //V0.9.9 (2001-03-19) [pr]
        } // if (!(pKernelGlobals->ulPanicFlags & SUF_SKIPXFLDSTARTUP))
#endif

        /*
         *  quick-open folders
         *
         */

#ifndef __NOQUICKOPEN__
        // "quick open" disabled because Shift key pressed?
        if (!(G_flPanic & SUF_SKIPQUICKOPEN))
        {
            // no:
            // get the quick-open folders
            XFolder *pQuick = NULL;
            QUICKOPENDATA qod;
            memset(&qod, 0, sizeof(qod));
            lstInit(&qod.llQuicks, FALSE);

            for (pQuick = _xwpclsQueryQuickOpenFolder(_XFolder, NULL);
                 pQuick;
                 pQuick = _xwpclsQueryQuickOpenFolder(_XFolder, pQuick))
            {
                lstAppendItem(&qod.llQuicks, pQuick);
            }

            // if we have any quick-open folders: go
            if (qod.cQuicks = lstCountItems(&qod.llQuicks))
            {
                if (cmnQuerySetting(sfShowStartupProgress))
                {
                    qod.hwndStatus = cmnLoadDlg(NULLHANDLE,
                                                fnwpQuickOpenDlg,
                                                ID_XFD_STARTUPSTATUS,
                                                &qod);      // param
                    WinSetWindowText(qod.hwndStatus,
                                     cmnGetString(ID_XFSI_QUICKSTATUS)) ; // pszQuickStatus

                    winhRestoreWindowPos(qod.hwndStatus,
                                         HINI_USER,
                                         INIAPP_XWORKPLACE, INIKEY_WNDPOSSTARTUP,
                                         SWP_MOVE | SWP_SHOW | SWP_ACTIVATE);
                    WinSendDlgItemMsg(qod.hwndStatus, ID_SDDI_PROGRESSBAR,
                                      WM_UPDATEPROGRESSBAR,
                                      (MPARAM)0,
                                      (MPARAM)qod.cQuicks);
                }

                // run the Quick Open thread synchronously
                // which updates the status window
                thrRunSync(ptiMyself->hab,
                           fntQuickOpenFolders,
                           "QuickOpen",
                           (ULONG)&qod);

                if (cmnQuerySetting(sfShowStartupProgress))
                {
                    winhSaveWindowPos(qod.hwndStatus,
                                      HINI_USER,
                                      INIAPP_XWORKPLACE, INIKEY_WNDPOSSTARTUP);
                    winhDestroyWindow(&qod.hwndStatus);
                }

            }
        } // end if (!(pKernelGlobals->ulPanicFlags & SUF_SKIPQUICKOPEN))
#endif

    }
    CATCH(excpt1) {} END_CATCH();

    /*
     *  other stuff
     *
     */

#ifndef __NOBOOTLOGO__
    // destroy boot logo, if present
    xthrPostBushMsg(QM_DESTROYLOGO, 0, 0);
#endif

#ifndef __XWPLITE__
    // if XWorkplace was just installed, check for
    // existence of config folders and
    // display welcome msg
    if (PrfQueryProfileInt(HINI_USER,
                           (PSZ)INIAPP_XWORKPLACE,
                           (PSZ)INIKEY_JUSTINSTALLED,
                           0x123)
            != 0x123)
    {
        // XWorkplace was just installed:
        // delete "just installed" INI key
        PrfWriteProfileString(HINI_USER,
                              (PSZ)INIAPP_XWORKPLACE,
                              (PSZ)INIKEY_JUSTINSTALLED,
                              NULL);

        // say hello on thread 1
        krnPostThread1ObjectMsg(T1M_WELCOME, MPNULL, MPNULL);
    }
#else
    {
        // V1.0.1 (2003-02-02) [umoeller]
        ULONG fl = cmnQuerySetting(sflIntroHelpShown);
        if (!(fl & HLPS_NOSHOWDESKTOP))
        {
            ULONG ulPanel = 0;
            CHAR szHelp[CCHMAXPATH];
            WPObject *pDesktop = cmnQueryActiveDesktop();

            cmnSetSetting(sflIntroHelpShown, fl | HLPS_NOSHOWDESKTOP);

            _wpQueryDefaultHelp(pDesktop,
                                &ulPanel,
                                szHelp);
            // help not displayed yet:
            _wpDisplayHelp(pDesktop,
                           ulPanel,
                           szHelp);
        }
    }
#endif

    // close startup log
    initLog("Leaving " __FUNCTION__", closing log.");
    if (LockLog())
    {
        doshClose(&G_pStartupLogFile);
        UnlockLog();
    }
}

/*
 *@@ initDesktopPopulated:
 *      called when the desktop has finished populating
 *      and XFldDesktop::wpPopulate then posts
 *      FIM_DESKTOPPOPULATED, whose implementation is here.
 *
 *      Runs on the File thread.
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 *@@changed V0.9.19 (2002-04-02) [umoeller]: added startup logging
 */

VOID initDesktopPopulated(VOID)
{
    PKERNELGLOBALS pKernelGlobals = NULL;
    ULONG tid;

    initLog("Entering " __FUNCTION__"...");

    PMPF_STARTUP(("entering"));

    // V0.9.9 (2001-03-10) [umoeller]
    TRY_LOUD(excpt1)
    {
        if (pKernelGlobals = krnLockGlobals(__FILE__, __LINE__, __FUNCTION__))
            pKernelGlobals->fDesktopPopulated = TRUE;
    }
    CATCH(excpt1) {} END_CATCH();

    if (pKernelGlobals)
        krnUnlockGlobals();

    initLog("  Creating startup thread");

    // create the startup thread which now does the
    // processing for us V0.9.12 (2001-04-29) [umoeller]
    if (!(tid = thrCreate(NULL,
                          fntStartupThread,
                          NULL,
                          "StartupThread",
                          THRF_PMMSGQUEUE | THRF_TRANSIENT,
                          0)))
    {
        initLog("  Cannot create startup thread.");
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "Cannot create startup thread.");
    }
    else
        initLog("  Startup thread created, TID %d", tid);

    // moved all the rest to fntStartupThread

    initLog("Leaving " __FUNCTION__);

    PMPF_STARTUP(("leaving"));
}

