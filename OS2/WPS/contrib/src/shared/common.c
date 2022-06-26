
/*
 *@@sourcefile common.c:
 *      this file contains functions that are common to all
 *      parts of XWorkplace. This is really an unsorted
 *      collection of very miscellaneous items that only
 *      have in common that they are "common" somehow.
 *
 *      These functions mainly deal with the following features:
 *
 *      -- module handling (cmnQueryMainModuleHandle etc.);
 *
 *      -- NLS management (cmnQueryNLSModuleHandle,
 *         cmnQueryNLSStrings);
 *
 *      -- global settings (cmnQuerySetting, cmnSetSetting);
 *
 *      -- miscellaneous dialog wrappers for our own NLS
 *         string support (cmnMessageBox and many others).
 *
 *      Note that the system sound functions have been exported
 *      to helpers\syssound.c (V0.9.0).
 *
 *@@header "shared\common.h"
 */

/*
 *      Copyright (C) 1997-2010 Ulrich M”ller.
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

#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#define INCL_DOSNLS
#define INCL_DOSERRORS

#define INCL_WINMESSAGEMGR
#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR        // SC_CLOSE etc.
#define INCL_WINPOINTERS
#define INCL_WININPUT
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINMENUS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINSTDFILE
#define INCL_WINSTDCNR
#define INCL_WINLISTBOXES
#define INCL_WINCOUNTRY
#define INCL_WINPROGRAMLIST
#define INCL_WINSYS
#define INCL_WINSHELLDATA       // Prf* functions

#define INCL_GPILOGCOLORTABLE
#define INCL_GPIBITMAPS
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <io.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\apps.h"               // application helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\exeh.h"               // executable helpers
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\procstat.h"           // DosQProcStat handling
#include "helpers\regexp.h"             // extended regular expressions
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\textview.h"           // PM XTextView control
#include "helpers\tree.h"               // red-black binary trees
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles
#include "helpers\xstring.h"            // extended string helpers

#include "helpers\nlscache.h"           // NLS string cache

#include "helpers\tmsgfile.h"           // "text message file" handling (for cmnGetMessage)

#include "expat\expat.h"                // XWPHelpers expat XML parser
#include "helpers\xml.h"                // XWPHelpers XML engine

// SOM headers which don't crash with prec. header files
#include "xtrash.ih"                    // XWPTrashCan; needed for empty trash
#include "xfdesk.ih"

// XWorkplace implementation headers
#include "bldlevel.h"                   // XWorkplace build level definitions
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "xwpapi.h"                     // public XWorkplace definitions
#define INCLUDE_COMMON_PRIVATE
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling
#include "shared\xsetup.h"              // XWPSetup implementation

#include "filesys\filedlg.h"            // replacement file dialog implementation
#include "filesys\icons.h"              // icons handling
#include "filesys\statbars.h"           // status bar translation logic
#include "filesys\xthreads.h"           // extra XWorkplace threads

// headers in /hook
#include "hook\xwphook.h"

#include "media\media.h"                // XWorkplace multimedia support

#include "helpers\xwpsecty.h"           // XWorkplace Security

// other SOM headers

#pragma hdrstop

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// main module (XFLDR.DLL)
static char             G_szDLLFile[CCHMAXPATH];
static HMODULE          G_hmodDLL = NULLHANDLE;

// res module (XWPRES.DLL)
static HMODULE          G_hmodRes = NULLHANDLE;

// NLS
static HMODULE          G_hmodNLS = NULLHANDLE;

// paths
static CHAR             G_szXWPBasePath[CCHMAXPATH] = "";        // V0.9.16 (2002-01-13) [umoeller]

static CHAR             G_szXWPThemeDir[CCHMAXPATH] = "";
static CHAR             G_szHelpLibrary[CCHMAXPATH] = "";
static CHAR             G_szMessageFile[CCHMAXPATH] = "";

// static GLOBALSETTINGS  G_GlobalSettings = {0};
            // removed V0.9.16 (2002-01-05) [umoeller]
// array of ULONGs with values for cmnGetSetting; this
// is filled on startup
static ULONG            G_aulSettings[___LAST_SETTING];
extern ULONG            *G_pulVarMenuOfs = NULL; // V1.0.0 (2002-08-28) [umoeller]

#ifndef __NOTURBOFOLDERS__
static BOOL             G_fTurboSettingsEnabled = FALSE;
            // set by cmnEnableTurboFolders() V0.9.16 (2002-01-05) [umoeller]
#endif

#ifndef __NOICONREPLACEMENTS__
static HMODULE          G_hmodIconsDLL = NULLHANDLE;
#endif
static CHAR             G_szLanguageCode[20] = "";

static HPOINTER         G_hptrDlgIcon = NULLHANDLE;
            // XWP icon for message boxes and stuff
            // V0.9.16 (2001-11-10) [umoeller]

static COUNTRYSETTINGS2 G_CountrySettings;                  // V0.9.6 (2000-11-12) [umoeller]
static BOOL             G_fCountrySettingsLoaded = FALSE;

static ULONG            G_ulCurHelpPanel = 0;      // holds help panel for dialog

static CHAR             G_szStatusBarFont[100] = "";
static CHAR             G_szSBTextNoneSel[CCHMAXMNEMONICS] = "",
                        G_szSBTextMultiSel[CCHMAXMNEMONICS] = "";
static ULONG            G_ulStatusBarHeight = 0;

static CHAR             G_szRunDirectory[CCHMAXPATH]; // V0.9.14

static PTMFMSGFILE      G_pXWPMsgFile = NULL;        // V0.9.16 (2001-10-08) [umoeller]

extern BOOL             G_fIsWarp4 = FALSE;     // V0.9.19 (2002-04-24) [umoeller]

#ifdef __DEBUG__
    // here's the debug settings array referenced by the
    // PMPF_* macros in setup.h
    extern char G_aDebugs[__LAST_DBGSET] = {0};
#endif

static CHAR     G_szCopyright[5] = "";

static PCSZ     G_pcszBldlevel = BLDLEVEL_VERSION,
                G_pcszBldDate = __DATE__,
                G_pcszNewLine = "\n",
                G_pcszNBSP = "\xFF",      // non-breaking space
                G_pcszContactUser = CONTACT_ADDRESS_USER,
                G_pcszContactDev = CONTACT_ADDRESS_DEVEL,
                G_pcszCopyChar = "\xB8",       // in codepage 850
                G_pcszCopyright = G_szCopyright;

static BOOL     G_fEntitiesHacked = FALSE;

static const STRINGENTITY G_aEntities[] =
    {
        "&copy;", &G_pcszCopyright,
        "&xwp;", &ENTITY_XWORKPLACE,
        "&os2;", &ENTITY_OS2,
        "&winos2;", &ENTITY_WINOS2,
        "&warpcenter;", &ENTITY_WARPCENTER,
        "&xcenter;", &ENTITY_XCENTER,
        "&xbutton;", &ENTITY_XBUTTON,
        "&xsd;", &ENTITY_XSHUTDOWN,
        "&version;", &G_pcszBldlevel,
        "&date;", &G_pcszBldDate,
        "&pgr;", &ENTITY_PAGER,
        "&nl;", &G_pcszNewLine,
        "&nbsp;", &G_pcszNBSP,
        "&contact-user;", &G_pcszContactUser,
        "&contact-dev;", &G_pcszContactDev,
    };

// Declare C runtime prototypes, because there are no headers
// for these:

// _CRT_init is the C run-time environment initialization function.
// It will return 0 to indicate success and -1 to indicate failure.
int _CRT_init(void);

// _CRT_term is the C run-time environment termination function.
// It only needs to be called when the C run-time functions are statically
// linked, as is the case with XFolder.
void _CRT_term(void);

VOID LoadDaemonNLSStrings(VOID);

/* ******************************************************************
 *
 *   Modules and paths
 *
 ********************************************************************/

/*
 *@@ _DLL_InitTerm:
 *      this special function gets called automatically by the
 *      OS/2 module manager during DosLoadModule processing, on
 *      the thread which invoked DosLoadModule.
 *
 *      Since this is a SOM DLL for the WPS, this probably gets
 *      called when the first XWorkplace WPS class is accessed.
 *      Since that is XFldObject, this gets called right when the
 *      WPS is starting. I suspect this gets called somewhere in
 *      somFindClass(), but who knows.
 *
 *      In addition, this gets called when the WPS process ends,
 *      e.g. due to a Desktop restart or trap. Since the WPS is
 *      the only process loading this DLL, we need not bother
 *      with details.
 *
 *      Defining this function is my preferred way of getting the
 *      DLL's module handle, instead of querying the SOM kernel
 *      for the module name, like this is done in most WPS sample
 *      classes provided by IBM. I have found this to be much
 *      easier and less error-prone when several classes are put
 *      into one DLL (as is the case with XWorkplace).
 *
 *      Besides, this is faster, since we store the module handle
 *      in a global variable which can later quickly be retrieved
 *      using cmnQueryMainModuleHandle.
 *
 *      Since OS/2 calls this function directly, it must have
 *      _System linkage.
 *
 *      Note: We must then link using the /NOE option, because
 *      the VAC++ runtimes also contain a _DLL_Initterm, and the
 *      linker gets in trouble otherwise. The XWorkplace makefile
 *      takes care of this.
 *
 *      This function must return 0 upon errors or 1 otherwise.
 *
 *@@changed V0.9.0 [umoeller]: reworked locale initialization
 *@@changed V0.9.0 [umoeller]: moved this func here from module.c
 */

unsigned long _System _DLL_InitTerm(unsigned long hmod,
                                    unsigned long fTerminate)
{
    if (!fTerminate)
    {
        // DLL being loaded:

        // store the DLL handle in the global variable so that
        // cmnQueryMainModuleHandle() below can return it
        G_hmodDLL = hmod;

        // now initialize the C run-time environment before we
        // call any runtime functions
        if (_CRT_init() == -1)
           return 0;  // error

        if (DosQueryModuleName(hmod, CCHMAXPATH, G_szDLLFile))
            DosBeep(100, 100);
    }
    else
    {
        // DLL being freed: cleanup runtime
        _CRT_term();
    }

    // a non-zero value must be returned to indicate success
    return 1;
}

/*
 *@@ cmnQueryMainCodeModuleHandle:
 *      this may be used to retrieve the module handle
 *      of XFLDR.DLL, which was stored by _DLL_InitTerm.
 *
 *      Note that this returns the _main_ module handle
 *      (XFLDR.DLL). There are two more query-module
 *      functions:
 *
 *      -- To get the NLS module handle (for dialogs,
 *         strings and other NLS resources), use
 *         cmnQueryNLSModuleHandle.
 *
 *      -- To get the main resource module handle (for
 *         resources which are the same with all languages),
 *         use cmnQueryMainResModuleHandle.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from module.c
 *@@changed V0.9.7 (2000-12-13) [umoeller]: renamed from cmnQueryMainModuleHandle
 */

HMODULE cmnQueryMainCodeModuleHandle(VOID)
{
    return G_hmodDLL;
}

/*
 *@@ cmnQueryMainCodeModuleFilename:
 *      this may be used to retrieve the fully
 *      qualified file name of the DLL
 *      which was stored by _DLL_InitTerm.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from module.c
 */

PCSZ cmnQueryMainCodeModuleFilename(VOID)
{
    return G_szDLLFile;
}

/*
 *@@ cmnQueryMainResModuleHandle:
 *      this may be used to retrieve the module handle
 *      of XWPRES.DLL, which contains resources that
 *      are independent of language (icons, bitmaps etc.).
 *
 *      This loads the DLL on the first call.
 *
 *      This has been added with V0.9.7 to separate the
 *      resources out of the main module handle to speed
 *      up link time, which became annoyingly slow with
 *      all the resources.
 *
 *@@added V0.9.7 (2000-12-13) [umoeller]
 */

HMODULE cmnQueryMainResModuleHandle(VOID)
{
    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            if (G_hmodRes == NULLHANDLE)
            {
                // not loaded yet:
                CHAR    szError[100],
                        szResModule[CCHMAXPATH];

                if (cmnQueryXWPBasePath(szResModule))
                {
                    APIRET arc = NO_ERROR;
                    strcat(szResModule, "\\bin\\xwpres.dll");
                    if (arc = DosLoadModule(szError,
                                            sizeof(szError),
                                            szResModule,
                                            &G_hmodRes))
                        cmnLog(__FILE__, __LINE__, __FUNCTION__,
                               "Error %d occurred loading \"%s\".",
                               arc, szResModule);
                }
            }
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        krnUnlock();

    return G_hmodRes;
}

/*
 *@@ cmnQueryXWPBasePath:
 *      this routine returns the path of where XFolder was installed,
 *      i.e. the parent directory of where the xfldr.dll file
 *      resides, without a trailing backslash (e.g. "C:\XFolder").
 *
 *      The buffer to copy this to is assumed to be CCHMAXPATH in size.
 *
 *      As opposed to versions before V0.81, OS2.INI is no longer
 *      needed for this to work. The path is retrieved from the
 *      DLL directly by evaluating what was passed to _DLL_InitTerm.
 *
 *@@changed V0.9.7 (2000-12-02) [umoeller]: renamed from cmnQueryXFolderBasePath
 *@@changed V0.9.16 (2002-01-13) [umoeller]: optimized
 */

BOOL cmnQueryXWPBasePath(PSZ pszPath)
{
    BOOL    brc = FALSE;
    BOOL    fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            if (!G_szXWPBasePath[0])
            {
                // first call: V0.9.16 (2002-01-13) [umoeller]
                PCSZ pszDLL;
                if (pszDLL = cmnQueryMainCodeModuleFilename())
                {
                    // copy until last backslash minus four characters
                    // (leave out "\bin\xfldr.dll")
                    PSZ     pszLastSlash = strrchr(pszDLL, '\\');
                    ULONG   cbBase = (pszLastSlash - pszDLL) - 4;
                    memcpy(G_szXWPBasePath,
                           pszDLL,
                           cbBase);
                    pszPath[cbBase] = '\0';
                    brc = TRUE;
                }
            }
            else
                brc = TRUE;
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        krnUnlock();

    if (brc)
        strlcpy(pszPath, G_szXWPBasePath, CCHMAXPATH);

    return brc;
}

/*
 *@@ cmnQueryLanguageCode:
 *      returns PSZ to three-digit language code (e.g. "001").
 *      This points to a global variable, so do NOT change.
 *
 *@@changed V0.9.0 (99-11-14) [umoeller]: made this reentrant, finally
 */

PCSZ cmnQueryLanguageCode(VOID)
{
    BOOL    fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            if (G_szLanguageCode[0] == '\0')
                PrfQueryProfileString(HINI_USERPROFILE,
                                      (PSZ)INIAPP_XWORKPLACE,
                                      (PSZ)INIKEY_LANGUAGECODE,
                                      (PSZ)DEFAULT_LANGUAGECODE,
                                      (PVOID)G_szLanguageCode,
                                      sizeof(G_szLanguageCode));

            G_szLanguageCode[3] = '\0';

            PMPF_LANGCODES(( "%s", G_szLanguageCode));
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        krnUnlock();

    return G_szLanguageCode;
}

/*
 *@@ cmnSetLanguageCode:
 *      changes XFolder's language to three-digit language code in
 *      pszLanguage (e.g. "001"). This does not reload the NLS DLL,
 *      but only change the setting.
 *
 *@@changed V0.9.0 (99-11-14) [umoeller]: made this reentrant, finally
 */

BOOL cmnSetLanguageCode(PCSZ pcszLanguage)
{
    BOOL brc = FALSE;

    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            if (!pcszLanguage)
                pcszLanguage = DEFAULT_LANGUAGECODE;

            strlcpy(G_szLanguageCode, pcszLanguage, sizeof(G_szLanguageCode));
            G_szLanguageCode[3] = 0;

            brc = PrfWriteProfileString(HINI_USERPROFILE,
                                        (PSZ)INIAPP_XWORKPLACE,
                                        (PSZ)INIKEY_LANGUAGECODE,
                                        G_szLanguageCode);
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        krnUnlock();

    return brc;
}

/*
 *@@ cmnQueryHelpLibrary:
 *      returns PSZ to full help library path in XFolder directory,
 *      depending on where XFolder was installed and on the current
 *      language (e.g. "C:\XFolder\help\xfldr001.hlp").
 *
 *      This PSZ points to a global variable, so you better not
 *      change it.
 *
 *@@changed V0.9.0 (99-11-14) [umoeller]: made this reentrant, finally
 */

PCSZ cmnQueryHelpLibrary(VOID)
{
    PCSZ rc = 0;

    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            if (cmnQueryXWPBasePath(G_szHelpLibrary))
            {
                // path found: append helpfile
                sprintf(G_szHelpLibrary + strlen(G_szHelpLibrary),
                        "\\help\\xfldr%s.hlp",
                        cmnQueryLanguageCode());

                PMPF_LANGCODES(("%s", G_szHelpLibrary ));

                rc = G_szHelpLibrary;
            }
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        krnUnlock();

    return rc;
}

/*
 *@@ cmnHelpNotFound:
 *      displays an error msg that the given help panel
 *      could not be found.
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 */

VOID cmnHelpNotFound(ULONG ulPanelID)
{
    CHAR sz[100];
    PCSZ psz = cmnQueryHelpLibrary();
    PCSZ apsz[] =
        {  sz,
           psz };
    sprintf(sz, "%d", ulPanelID);

    cmnMessageBoxExt(NULLHANDLE,
                        104,            // title
                        apsz,
                        2,
                        134,
                        MB_OK);
}

/*
 *@@ cmnDisplayHelp:
 *      displays an XWorkplace help panel,
 *      using wpDisplayHelp.
 *      If somSelf == NULL, we'll query the
 *      active desktop.
 */

BOOL cmnDisplayHelp(WPObject *somSelf,
                    ULONG ulPanelID)
{
    BOOL brc = FALSE;
    if (somSelf == NULL)
        somSelf = cmnQueryActiveDesktop();

    if (somSelf)
    {
        if (!(brc = _wpDisplayHelp(somSelf,
                                   ulPanelID,
                                   (PSZ)cmnQueryHelpLibrary())))
            // complain
            cmnHelpNotFound(ulPanelID);

    }
    return brc;
}

/*
 *@@ cmnQueryMessageFile:
 *      returns PSZ to full message file path in XFolder directory,
 *      depending on where XFolder was installed and on the current
 *      language (e.g. "C:\XFolder\help\xfldr001.tmf").
 *
 *@@changed V0.9.0 [umoeller]: changed, this now returns the TMF file (tmsgfile.c).
 *@@changed V0.9.0 (99-11-14) [umoeller]: made this reentrant, finally
 */

PCSZ cmnQueryMessageFile(VOID)
{
    PCSZ rc = 0;

    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            if (cmnQueryXWPBasePath(G_szMessageFile))
            {
                // path found: append message file
                sprintf(G_szMessageFile + strlen(G_szMessageFile),
                        "\\help\\xfldr%s.tmf",
                        cmnQueryLanguageCode());

                PMPF_LANGCODES(("%s", G_szMessageFile));

                rc = G_szMessageFile;
            }
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        krnUnlock();

    return rc;
}

#ifndef __NOBOOTLOGO__

/*
 *@@ cmnQueryBootLogoFile:
 *      this returns the boot logo file as stored
 *      in OS2.INI. If it is not stored there,
 *      we return the default xfolder.bmp in
 *      the XFolder installation directories.
 *
 *      The return value of this function must
 *      be free()'d after use.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.0 (99-11-14) [umoeller]: made this reentrant, finally
 */

PSZ cmnQueryBootLogoFile(VOID)
{
    PSZ pszReturn;
    if (!(pszReturn = prfhQueryProfileData(HINI_USER,
                                           INIAPP_XWORKPLACE,
                                           INIKEY_BOOTLOGOFILE,
                                           NULL)))
    {
        // INI data not found: return default file
        CHAR szBootLogoFile[CCHMAXPATH];
        cmnQueryXWPBasePath(szBootLogoFile);
        strcat(szBootLogoFile,
               "\\bootlogo\\xfolder.bmp");
        pszReturn = strdup(szBootLogoFile);
    }

    return pszReturn;
}

#endif

/*
 *@@ cmnQueryNLSModuleHandle:
 *      returns the module handle of the language-dependent XFolder
 *      National Language Support DLL (XFLDRxxx.DLL).
 *
 *      This is called in two situations:
 *
 *          1) with (fEnforceReload == FALSE) everytime some part
 *             of XFolder needs the NLS resources (e.g. for dialogs);
 *             this only loads the NLS DLL on the very first call
 *             then, whose module handle is cached for subsequent calls.
 *
 *          2) with (fEnforceReload == TRUE) only when the user changes
 *             XFolder's language in the "Workplace Shell" object.
 *
 *      If the DLL is (re)loaded, this function also initializes
 *      all language-dependent XWorkplace components.
 *      This function also checks for whether the NLS DLL has a
 *      decent version level to support this XFolder version.
 *
 *@@changed V0.9.0 [umoeller]: added various NLS strings
 *@@changed V0.9.0 (99-11-14) [umoeller]: made this reentrant, finally
 *@@changed V0.9.7 (2000-12-09) [umoeller]: restructured to fix mutex hangs with load errors
 *@@changed V0.9.9 (2001-03-07) [umoeller]: now loading strings from array
 *@@changed V0.9.19 (2002-04-02) [umoeller]: msg file wasn't reloaded on NLS change, fixed
 *@@changed V0.9.19 (2002-04-24) [umoeller]: version checks never worked, fixed
 *@@changed V0.9.19 (2002-04-24) [umoeller]: reverting to 001 on errors now
 *@@changed V1.0.0 (2002-09-15) [lafaix]: notify the daemon if fEnforceReload == TRUE
 *@@changed V1.0.1 (2002-12-08) [umoeller]: fixed stupid hwndDaemonObject log message @@fixes 64
 *@@changed V1.0.9 (2010-01-30) [pr]: modify NLS revision check @@fixes 1097
 */

HMODULE cmnQueryNLSModuleHandle(BOOL fEnforceReload)
{
    HMODULE hmodReturn = NULLHANDLE,
            hmodLoaded = NULLHANDLE;

    // load resource DLL if it's not loaded yet or a reload is enforced
    if (    (G_hmodNLS == NULLHANDLE)
         || (fEnforceReload)
       )
    {
        BOOL    fRetry = TRUE;
        CHAR    szResourceModuleName[CCHMAXPATH];
        CHAR    szError[1000] = "";

        while (fRetry)      // V0.9.19 (2002-04-24) [umoeller]
        {
            fRetry = FALSE;

            // get the XFolder path first
            if (!cmnQueryXWPBasePath(szResourceModuleName))
                strcpy(szError, "cmnQueryXWPBasePath failed.");
            else
            {
                APIRET arc = NO_ERROR;
                // now compose module name from language code
                strcat(szResourceModuleName, "\\bin\\xfldr");
                strcat(szResourceModuleName, cmnQueryLanguageCode());
                strcat(szResourceModuleName, ".dll");

                // try to load the module
                if (arc = DosLoadModule(NULL,
                                        0,
                                        szResourceModuleName,
                                        &hmodLoaded))
                {
                    // error:
                    // display an error string;
                    // since we don't have NLS, this must be in English...
                    sprintf(szError,
                            "XWorkplace was unable to load its National "
                            "Language Support DLL \"%s\". DosLoadModule returned "
                            "error %d.",
                            szResourceModuleName,
                            arc);
                }
            }

            if (hmodLoaded)
            {
                // module loaded alright!
                // hmodLoaded has the new module handle
                HAB habDesktop = G_habThread1;

                if (fEnforceReload)
                {
                    // if fEnforceReload == TRUE, we will load a test string from
                    // the module to see if it has at least the version level which
                    // this XFolder version requires. This is done using a #define
                    // in dlgids.h: XFOLDER_VERSION is compiled as a string resource
                    // into both the NLS DLL and into the main DLL (this file),
                    // so we always have the versions in there automatically.
                    // MINIMUM_NLS_VERSION (dlgids.h too) contains the minimum
                    // NLS version level that this XFolder version requires.
                    CHAR    szTest[30] = "";
                    LONG    lLength;
                    ULONG   ulMajor, ulMinor, ulRevision;
                    cmnSetDlgHelpPanel(-1);
                    lLength = WinLoadString(habDesktop,
                                            hmodLoaded, // bullshit G_hmodNLS,
                                            ID_XSSI_XFOLDERVERSION,
                                            sizeof(szTest), szTest);

                    PMPF_LANGCODES(("%s", szResourceModuleName, szTest));

                    if (lLength == 0)
                    {
                        // version string not found: complain
                        sprintf(szError,
                                "The requested file \"%s\" is not an XWorkplace National Language Support DLL.",
                                szResourceModuleName);
                    }
                    // V0.9.19 (2002-04-24) [umoeller]
                    else
                    {
                        // V1.0.9 (2010-01-30) [pr]
                        int iCount, iMajor, iMinor, iRevision;

                        iCount = sscanf(szTest, "%u.%u.%u", &iMajor, &iMinor, &iRevision);
                        if (   iCount == 3
                            && iMajor == MINIMUM_NLS_MAJOR
                            && iMinor == MINIMUM_NLS_MINOR
                            && iRevision >= MINIMUM_NLS_REVISION
                           )
                        {
                            // new module is OK:
                            hmodReturn = hmodLoaded;
                        }
                        else
                        {
                            // version level not sufficient:
                            // load dialog from _old_ NLS DLL which says
                            // that the DLL is too old; if user presses
                            // "Cancel", we abort loading the DLL
                            // V0.9.19 (2002-04-24) [umoeller]
                            // no, this is not working on startup since we
                            // have no old NLS DLL
                            sprintf(szError,
                                    "The requested National Language Support module \"%s\" "
                                    "(%s) is older than the minimum acceptable version "
                                    "for the XWorkplace version that is running "
                                    "(%s). Loading this module might lead to serious problems or "
                                    "frequent error messages. Do you wish to load the module anyway?",
                                    szResourceModuleName,
                                    szTest,
                                    MINIMUM_NLS_VERSION);
                            if (WinMessageBox(HWND_DESKTOP,
                                              NULLHANDLE,
                                              szError,
                                              "XWorkplace: NLS Warning",
                                              0,
                                              MB_MOVEABLE | MB_YESNO)
                                    == MBID_NO)
                            {
                                sprintf(szError,
                                        "The new National Language Support DLL \"%s\" was not loaded.",
                                        szResourceModuleName);
                                // revert to English below
                            }
                            else
                            {
                                // user wants outdated module:
                                hmodReturn = hmodLoaded;
                                szError[0] = '\0';
                            }
                        }
                    }
                } // end if (fEnforceReload)
                else
                    // no enfore reload: that's OK always
                    hmodReturn = hmodLoaded;
            } // end if (hmodLoaded)

            if (szError[0])
            {
                // error occurred:
                // if current language is not English, revert to 001
                // and retry V0.9.19 (2002-04-24) [umoeller]
                if (strcmp(DEFAULT_LANGUAGECODE, cmnQueryLanguageCode()))
                {
                    strcat(szError, " Reverting to 001 for US English.");
                    cmnSetLanguageCode(DEFAULT_LANGUAGECODE);
                    fRetry = TRUE;
                }

                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       szError);

                winhDebugBox(NULLHANDLE,
                             "XWorkplace: NLS Error",
                             szError);
            }
        } // while (fRetry)      // V0.9.19 (2002-04-24) [umoeller]
    } // end if (    (G_hmodNLS == NULLHANDLE)  || (fEnforceReload) )
    else
        // no (re)load neccessary:
        // use old module (this must be != NULLHANDLE now)
        hmodReturn = G_hmodNLS;

    // V0.9.7 (2000-12-09) [umoeller]
    // alright, now we have:
    // --  hmodLoaded: != NULLHANDLE if we loaded a new module.
    // --  hmodReturn: != NULLHANDLE if the new module is OK.

    if (hmodLoaded)                    // new module loaded here?
    {
        if (hmodReturn == NULLHANDLE)      // but error?
            DosFreeModule(hmodLoaded);
        else
        {
            // module loaded, and OK:
            // replace the global module handle for NLS,
            // and reload all NLS strings...
            // do this safely.
            HMODULE hmodOld = G_hmodNLS;
            BOOL fLocked = FALSE;

            if (krnLock(__FILE__, __LINE__, __FUNCTION__))
            {
                G_hmodNLS = hmodLoaded;
                krnUnlock();
            }

            if (hmodOld)
            {
                // reinitialize string cache in helpers code V1.0.1 (2002-12-11) [umoeller]
                nlsInitStrings(G_habThread1,
                               G_hmodNLS,
                               G_aEntities,
                               ARRAYITEMCOUNT(G_aEntities));

                // close TMF message file to force reload
                // V0.9.19 (2002-04-02) [umoeller]
                TRY_LOUD(excpt1)
                {
                    if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
                    {
                        if (G_pXWPMsgFile)
                            tmfCloseMessageFile(&G_pXWPMsgFile);
                    }
                }
                CATCH(excpt1) { } END_CATCH();

                if (fLocked)
                {
                    krnUnlock();
                    fLocked = FALSE;
                }

                // after all this, unload the old resource module
                DosFreeModule(hmodOld);

                // V1.0.1 (2002-12-08) [umoeller]
                cmnLoadDaemonNLSStrings();
            }

            // fill the shared buffer with the daemon-specific
            // resources V1.0.0 (2002-09-15) [lafaix]

            // moved this up so we only call this in the "language changed"
            // case; otherwise this produces a log entry with
            // "pXwpGlobalShared->hwndDaemonObject is NULLHANDLE"
            // from krnSendDaemonMsg on every WPS startup
            // V1.0.1 (2002-12-08) [umoeller]
            // cmnLoadDaemonNLSStrings();
        }
    }

    if (hmodReturn == NULLHANDLE)
        // error:
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "Returning NULLHANDLE. Some error occurred.");

    // return (new?) module handle
    return hmodReturn;
}

/*
 *@@ cmnLoadDlg:
 *      wrapper around WinLoadDlg to load something
 *      from the NLS resource DLL. This will also display
 *      a message box if loading the dialog failed...
 *      might be useful for NLS translators to receive
 *      a meaningful message if the resource DLLs are
 *      not working correctly.
 *
 *@@added V0.9.19 (2002-04-24) [umoeller]
 */

HWND cmnLoadDlg(HWND hwndOwner,
                PFNWP pfnwp,
                ULONG idResource,
                PVOID pvCreateParam)
{
    HWND hwnd;

    if (!(hwnd = WinLoadDlg(HWND_DESKTOP,           // parent
                            hwndOwner,
                            pfnwp,
                            cmnQueryNLSModuleHandle(FALSE),
                            idResource,
                            pvCreateParam)))
    {
        CHAR sz[100];
        sprintf(sz,
                "Error loading dialog %d from NLS resource DLL.",
                idResource);
        cmnMessageBox(hwndOwner,
                      "NLS Error",
                      sz,
                      NULLHANDLE, // no help
                      MB_CANCEL);
    }

    return hwnd;
}

/*
 *@@ cmnLoadMenu:
 *      wrapper around WinLoadMenu which sets the menu's
 *      font correctly to the system menu font.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

HWND cmnLoadMenu(HWND hwndOwner,
                 HMODULE hmod,
                 ULONG id)
{
    HWND hwndMenu;
    if (hwndMenu = WinLoadMenu(hwndOwner,
                               hmod,
                               id))
    {
        PSZ pszFont;
        if (pszFont = winhQueryMenuSysFont())
        {
            winhSetWindowFont(hwndMenu,
                              pszFont);
            free(pszFont);
        }
        else
            winhSetWindowFont(hwndMenu,
                              cmnQueryDefaultFont());
    }
    else
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "Error loading menu ID 0x%lX from module 0x%lX",
               id,
               hmod);

    return hwndMenu;
}

/* ******************************************************************
 *
 *   Error logging
 *
 ********************************************************************/

/*
 *@@ cmnLog:
 *      logs a message to the XWorkplace log file
 *      in the root directory of the boot drive.
 *
 *@@added V0.9.2 (2000-03-06) [umoeller]
 */

VOID cmnLog(PCSZ pcszSourceFile, // in: source file name
            ULONG ulLine,               // in: source line
            PCSZ pcszFunction,   // in: function name
            PCSZ pcszFormat,     // in: format string (like with printf)
            ...)                        // in: additional stuff (like with printf)
{
    va_list     args;
    CHAR        szLogFilename[100];
    FILE        *fileLog = 0;

#ifndef __NOEXCEPTIONBEEPS__
    DosBeep(100, 50);
#endif

    if (    (doshCreateLogFilename(szLogFilename,
                                   XFOLDER_LOGLOG,
                                   F_ALLOW_BOOTROOT_LOGFILE))
         && (fileLog = fopen(szLogFilename, "a"))  // text file, append
       )
    {
        DATETIME dt;
        DosGetDateTime(&dt);
        fprintf(fileLog,
                "%04d-%02d-%02d %02d:%02d:%02d:%02d T%03d "
                "%s (%s, line %d):\n    ",
                dt.year, dt.month, dt.day,
                dt.hours, dt.minutes, dt.seconds, dt.hundredths,
                doshMyTID(),
                pcszFunction, pcszSourceFile, ulLine);
        va_start(args, pcszFormat);
        vfprintf(fileLog, pcszFormat, args);
        va_end(args);
        fprintf(fileLog, "\n");
        fclose (fileLog);
    }
}

/* ******************************************************************
 *
 *   NLS strings
 *
 ********************************************************************/

/*
 *@@ cmnInitEntities:
 *      called from initMain to initialize NLS-dependent
 *      parts of the entities.
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 */

VOID cmnInitEntities(VOID)
{
    // get the current process codepage for the WPS
    ULONG   ulcp = nlsQueryCodepage();      // V1.0.2 (2003-02-07) [umoeller]

    if (ulcp == 850)
        strlcpy(G_szCopyright, G_pcszCopyChar, sizeof(G_szCopyright));
    else
    {
        WinCpTranslateString(G_habThread1,
                             850,
                             (PSZ)G_pcszCopyChar,
                             ulcp,
                             sizeof(G_szCopyright),
                             G_szCopyright);
        if (G_szCopyright[0] == '\xFF')
            memcpy(G_szCopyright, "(C)", 4);
    }

    nlsInitStrings(G_habThread1,
                   cmnQueryNLSModuleHandle(FALSE),
                   G_aEntities,
                   ARRAYITEMCOUNT(G_aEntities));
}

/*
 *@@ cmnGetString:
 *      returns an XWorkplace NLS string.
 *
 *      On input, specify one of the ID_XSSI_* identifiers
 *      specified in dlgids.h.
 *
 *      This function completely replaces the NLSSTRINGS array
 *      which was present in all XFolder and XWorkplace versions
 *      up to V0.9.9. This function has the following advantages:
 *
 *      -- Memory is only consumed for strings that are actually
 *         used. The NLSSTRINGS array had become terribly big,
 *         and lots of strings were loaded that were never used.
 *
 *      -- Desktop startup should be a bit faster because we don't have
 *         to load a thousand strings at startup.
 *
 *      -- The memory buffer holding the string is probably close
 *         to the rest of the heap data that the caller allocated,
 *         so this might lead to less memory page fragmentation.
 *
 *      -- To add a new NLS string, before this mechanism existed,
 *         three files had to be changed (and kept in sync): common.h
 *         to add a field to the NLSSTRINGS structure, dlgids.h to
 *         add the string ID, and xfldrXXX.rc to add the resource.
 *         With the new mechanism, there's no need to change common.h
 *         any more, so the danger of forgetting something is a bit
 *         reduced. Anyway, fewer recompiles are needed (maybe),
 *         and sending in patches to the code is a bit easier.
 *
 *      The way this works is that the function maintains a
 *      fast cache of string IDs and only loads the string
 *      resources on demand from the XWorkplace NLS DLL. If
 *      a string ID is queried for the first time, the string
 *      is loaded. Otherwise the cached copy is returned.
 *
 *      There is a slight overhead to this function compared to
 *      simply getting a static string from an array, because
 *      the cache needs to be searched for the string ID. However,
 *      this uses a binary tree (balanced according to string IDs)
 *      internally, so this is quite fast still.
 *
 *      This never releases the strings again, unless the
 *      NLS DLL is reloaded (see cmnQueryNLSModuleHandle).
 *
 *      This never returns NULL. Even if loading the string failed,
 *      a string is returned; in that case, it's a meaningful error
 *      message specifying the ID that failed.
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 *@@changed V0.9.16 (2001-10-19) [umoeller]: fixed bad string count which was never set
 *@@changed V0.9.16 (2002-01-26) [umoeller]: optimized heap locality
 *@@changed V1.0.1 (2002-12-11) [umoeller]: now using nlsGetString for the implementation
 */

PCSZ cmnGetString(ULONG ulStringID)
{
    return nlsGetString(ulStringID);
}

/*
 *@@ cmnGetString2:
 *      like cmnGetString, but does not return a PCSZ,
 *      but instead copies the string into the given
 *      buffer of the caller.
 *
 *      Returns strlen(pszBuf) always.
 *
 *@@added V1.0.1 (2003-01-30) [umoeller]
 */

ULONG cmnGetString2(PSZ pszBuf,                     // out: new string
                    ULONG ulStringID,               // in: string ID (passed to cmnGetString)
                    ULONG cbBuf)                    // in: size of pszBuf
{
    return strlcpy(pszBuf,
                   nlsGetString(ulStringID),
                   cbBuf);
}

/*
 *@@ cmnGetStringNoMnemonic:
 *      calls cmnGetString and copies the result
 *      to pszBuf, removing a single tilde (~)
 *      character if found.
 *
 *      Returns strlen(pszBuf) always.
 *
 *@@added V1.0.1 (2003-01-30) [umoeller]
 */

ULONG cmnGetStringNoMnemonic(PSZ pszBuf,            // out: new string
                             ULONG ulStringID,      // in: string ID (passed to cmnGetString)
                             ULONG cbBuf)           // in: size of pszBuf
{
    PSZ    p;
    ULONG   ulLen = strlcpy(pszBuf,
                            cmnGetString(ulStringID),
                            cbBuf);

    strhKillChar(pszBuf,
                 '~',
                 &ulLen);

    return ulLen;
}

/*
 *@@ cmnLoadDaemonNLSStrings:
 *      loads all daemon-specific strings in the structure shared with
 *      the daemon.
 *
 *      The NLS strings are packed in the XWPGLOBALSHARED.achNLSStrings
 *      field, and the end of the array is denoted by a double null
 *      sequence:
 *
 +          string 1\0string 2\0 ... string n\0\0
 *
 *      The daemon-specific strings have IDs between ID_DMSI_FIRST
 *      and ID_DMSI_LAST, inclusive.
 *
 *      If the shared buffer is full, no more strings are added.
 *
 *@@added V1.0.0 (2002-09-15) [lafaix]
 */

VOID cmnLoadDaemonNLSStrings(VOID)
{
    BOOL    fLocked = FALSE;

    // check if this is != NULL, because on my system we
    // get called from cmnQueryNLSModuleHandle when this
    // pointer is still NULL
    // V1.0.0 (2002-09-17) [umoeller]

    TRY_LOUD(excpt1)
    {
        if (    (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
             && (G_pXwpGlobalShared)
           )
        {
            USHORT  us;
            PSZ     pszBuf = G_pXwpGlobalShared->achNLSStrings;

            PMPF_LANGCODES(("G_pXwpGlobalShared = 0x%lX, pszBuf = 0x%lX",
                            G_pXwpGlobalShared,
                            pszBuf));

            for (us = ID_DMSI_FIRST;
                 us < ID_DMSI_LAST + 1;
                 us++)
            {
                LONG    lLength;

                PMPF_LANGCODES(("loading id %d from module 0x%lX",
                                us,
                                G_hmodNLS));

                if (!(lLength = WinLoadString(G_habThread1,
                                              G_hmodNLS,
                                              us,
                                              256,
                                              pszBuf)))
                    // load failed
                    lLength = sprintf(pszBuf,
                                      "string resource %d not found in module 0x%lX",
                                      us,
                                      G_hmodNLS);

                pszBuf += lLength + 1;

                // check if there is still enough room for another string
                if ((pszBuf - G_pXwpGlobalShared->achNLSStrings) > (3072+1-256))
                    break;
            }

            *pszBuf = 0;

            // made post a send V1.0.0 (2002-09-17) [umoeller]
            krnSendDaemonMsg(XDM_NLSCHANGED, MPNULL, MPNULL);
        }
    }
    CATCH(excpt1)
    {
    }
    END_CATCH();

    if (fLocked)
        krnUnlock();
}

// some frequently used dialog controls, these
// are exported!
// V0.9.16 (2001-10-08) [umoeller]

const CONTROLDEF
    G_OKButton = LOADDEF_DEFPUSHBUTTON(DID_OK),
            // added V0.9.19 (2002-04-17) [umoeller]
    G_CancelButton = LOADDEF_PUSHBUTTON(DID_CANCEL),
            // added V0.9.19 (2002-04-17) [umoeller]
    G_UndoButton = LOADDEF_PUSHBUTTON(DID_UNDO),
    G_DefaultButton = LOADDEF_PUSHBUTTON(DID_DEFAULT),
    G_HelpButton = LOADDEF_HELPPUSHBUTTON(DID_HELP),
    G_Spacing = CONTROLDEF_TEXT(
                            NULL,
                            -1,
                            8,
                            1),
    G_AddButton = LOADDEF_PUSHBUTTON(DID_ADD),
            // added V1.0.0 (2002-09-10) [lafaix]
    G_EditButton = LOADDEF_PUSHBUTTON(DID_EDIT),
            // added V1.0.0 (2002-09-10) [lafaix]
    G_RemoveButton = LOADDEF_PUSHBUTTON(DID_REMOVE);
            // added V1.0.0 (2002-09-10) [lafaix]

/*
 * G_aStringIDs:
 *      array of LOADSTRING structures specifying the
 *      NLS strings to be loaded at startup.
 *
 *      This array has been removed again... we now have
 *      the new cmnGetString function V0.9.9 (2001-04-04) [umoeller].
 *      If you need to look up the old id -> psz pairs,
 *      look at src\shared\OldStringIDs.txt.
 *
 *added V0.9.9 (2001-03-07) [umoeller]
 *removed again V0.9.9 (2001-04-04) [umoeller]
 */

/* ******************************************************************
 *
 *   Pointers
 *
 ********************************************************************/

#ifndef __NOICONREPLACEMENTS__

/*
 *@@ cmnQueryThemeDirectory:
 *      returns the name of the directory that was
 *      set to contain the current icon theme.
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

PCSZ cmnQueryThemeDirectory(VOID)
{
    PCSZ    pReturn = NULL;
    BOOL    fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            if (!G_szXWPThemeDir[0])
            {
                // first call: V0.9.16 (2002-01-13) [umoeller]
                ULONG cb = sizeof(G_szXWPThemeDir);
                if (    (PrfQueryProfileData(HINI_USER,
                                            (PSZ)INIAPP_XWORKPLACE,
                                            (PSZ)INIKEY_THEMESDIR,  // "ThemesDir"
                                            G_szXWPThemeDir,
                                            &cb))
                     && (cb > 4)
                   )
                    // OK, use that one
                    ;
                else if (cmnQueryXWPBasePath(G_szXWPThemeDir))
                {
                    if (G_fIsWarp4)
                        strcat(G_szXWPThemeDir, "\\themes\\warp4");
                    else
                        strcat(G_szXWPThemeDir, "\\themes\\warp3");
                }
            }

            pReturn = G_szXWPThemeDir;
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        krnUnlock();

    return pReturn;
}

#endif

/*
 *@@ ICONTREENODE:
 *      tree node structure for storing a standard
 *      icon that was loaded. Created for each
 *      standard icon that was loaded thru cmnGetStandardIcon.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

typedef struct _ICONTREENODE
{
    TREE        Tree;           // ulkey has the STDICON_* id

    HPOINTER    hptr;           // HPOINTER that was built

    PSZ         pszIconFile;    // fully q'fied file name,
                                // if icon was loaded from file
                                // (malloc'd)
    HMODULE     hmod;           // if icon was loaded from module
    ULONG       ulResID;        // if icon was loaded from module
} ICONTREENODE, *PICONTREENODE;

HMTX        G_hmtxIconsCache = NULLHANDLE;
TREE        *G_IconsCache;
LONG        G_cIconsInCache = 0;

/*
 *@@ LockIcons:
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 *@@changed V0.9.20 (2002-07-31) [umoeller]: renamed, returning APIRET now
 */

STATIC APIRET LockIconsCache(VOID)
{
    APIRET arc;

    if (G_hmtxIconsCache)
        return DosRequestMutexSem(G_hmtxIconsCache, SEM_INDEFINITE_WAIT);

    // first call:
    if (!(arc = DosCreateMutexSem(NULL,
                                  &G_hmtxIconsCache,
                                  0,
                                  TRUE)))       // request
    {
        treeInit(&G_IconsCache,
                 &G_cIconsInCache);
    }

    return arc;
}

/*
 *@@ UnlockIcons:
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

STATIC VOID UnlockIconsCache(VOID)
{
    DosReleaseMutexSem(G_hmtxIconsCache);
}

#define XWP_MODULE_BIT 0x80000000

typedef struct _STDICON
{
    ULONG       ulStdIcon;          // STDICON_* id

    PCSZ        pcszFilename;       // filename of replacement icon for theming
    ULONG       ulPMWP;             // ptr ID in pmwp.dll if pcszFilename not found
                                    // or icon replacements are disabled;
                                    // if the XWP_MODULE_BIT is set, use xwpres.dll
                                    // instead
} STDICON, *PSTDICON;

typedef const struct _STDICON *PCSTDICON;

static const STDICON aStdIcons[] =
    {
        {
            STDICON_SHADOWBROKEN,   // added V1.0.2 (2003-02-07) [umoeller]
            "shadow2.ico",
            44,                     // standard broken shadow icon in pmwp.dll
        },
        {
            STDICON_SHADOWOVERLAY,  // added V0.9.20 (2002-07-31) [umoeller]
            "shadow.ico",
            XWP_MODULE_BIT | ID_ICONXWPSHADOWOVERLAY,
        },
        {
            STDICON_TEMPLATE,       // added V0.9.20 (2002-08-04) [umoeller]
            "template.ico",
            20,                     // standard template icon in pmwp.dll
        },
        {
            STDICON_PM,
            "prgpm.ico",            // no icon in icons.dll yet
            3,                      // standard default program icon in pmwp.dll
                // or this one? 25
        },
        {
            STDICON_WIN16,
            "prgwin16.ico",         // no icon in icons.dll yet
            52,                     // standard win-os2 icon in pmwp.dll
        },
        {
            STDICON_WIN32,
            "prgwin32.ico",         // no icon in icons.dll yet
            52,                     // standard win-os2 icon in pmwp.dll
        },
        {
            STDICON_OS2WIN,
            "prgos2wn.ico",         // 108,
            2,                      // standard os2win icon in pmwp.dll
        },
        {
            STDICON_OS2FULLSCREEN,
            "prgos2fs.ico",         // 107,
            4,                      // standard os2full icon in pmwp.dll
        },
        {
            STDICON_DOSWIN,
            "prgdoswn.ico",         // 105,
            46,                     // standard doswin icon in pmwp.dll
        },
        {
            STDICON_DOSFULLSCREEN,
            "prgdosfs.ico",         // 104,
            1,                      // standard dosfull icon in pmwp.dll
        },
        {
            STDICON_DLL,
            "prgdll.ico",           // 103,
            25,                     // standard program (non-pm) icon in pmwp.dll
        },
        {
            STDICON_DRIVER,
            "prgdrivr.ico",         // 106,
            25,                     // standard program (non-pm) icon in pmwp.dll
        },
        {
            STDICON_PROG_UNKNOWN,
            "prgunkwn.ico",        // 102,
            25,                     // standard program (non-pm) icon in pmwp.dll
        },
        {
            STDICON_DATAFILE,
            "datafile.ico",         // 0,
            24,                     // standard datafile icon in pmwp.dll
        },
        {
            STDICON_TRASH_EMPTY,
            "trashemp.ico",         // 112,
            XWP_MODULE_BIT | ID_ICONXWPTRASHEMPTY
        },
        {
            STDICON_TRASH_FULL,
            "trashful.ico",         // 113,
            XWP_MODULE_BIT | ID_ICONXWPTRASHFILLED
        },
        {
            STDICON_DESKTOP_CLOSED,
            "desk0.ico",
            56                      // standard desktop icon in pmwp.dll
        },
        {
            STDICON_DESKTOP_OPEN,
            "desk1.ico",
            56                      // standard desktop icon in pmwp.dll
                                    // (there is no open desktop icon)
        },
        {
            STDICON_FOLDER_CLOSED,
            "folder0.ico",
            26                      // standard folder icon in pmwp.dll
        },
        {
            STDICON_FOLDER_OPEN,
            "folder1.ico",
            6                       // standard open folder icon in pmwp.dll
        },

        // tool bar icons V1.0.1 (2002-11-30) [umoeller]

        {
            STDICON_TB_REFRESH,
            "t_refr.ico",
            XWP_MODULE_BIT | ID_TB_RERESH
        },
        {
            STDICON_TB_FIND,
            "t_find.ico",
            XWP_MODULE_BIT | ID_TB_FIND
        },
        {
            STDICON_TB_HELP,
            "t_help.ico",
            XWP_MODULE_BIT | ID_TB_HELP
        },
        {
            STDICON_TB_MULTIPLECOLUMNS,
            "t_mulcol.ico",
            XWP_MODULE_BIT | ID_TB_MULTIPLECOLUMNS
        },
        {
            STDICON_TB_DETAILS,
            "t_detail.ico",
            XWP_MODULE_BIT | ID_TB_DETAILS
        },
        {
            STDICON_TB_SMALLICONS,
            "t_smlico.ico",
            XWP_MODULE_BIT | ID_TB_SMALLICONS
        }
    };

/*
 *@@ LoadNewIcon:
 *      called from cmnGetStandardIcon if the given
 *      standard icon ID was used for the first time,
 *      in which case we need to load a new icon.
 *
 *      Returns the ICONTREENODE that was created,
 *      in which hptr has received the HPOINTER that
 *      was created here.
 *
 *      Caller must hold the icons mutex.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

STATIC PICONTREENODE LoadNewIcon(ULONG ulStdIcon)
{
    HPOINTER hptrReturn = NULLHANDLE;
    PICONTREENODE pNode;

    PSZ         pszIconFile = NULL;         // if icon was loaded from file, malloc()
    HMODULE     hmod = NULLHANDLE;          // if icon was loaded from module
    ULONG       ulResID = NULLHANDLE;       // if icon was loaded from module

    // look up the STDICON_* id in the array
    ULONG ul;
    PCSTDICON pStdIcon = NULL;

    for (ul = 0;
         ul < ARRAYITEMCOUNT(aStdIcons);
         ul++)
    {
        if (aStdIcons[ul].ulStdIcon == ulStdIcon)
        {
            pStdIcon = &aStdIcons[ul];
            break;
        }
    }

    if (pStdIcon)
    {
        // id found:

#ifndef __NOICONREPLACEMENTS__
        // check if we have an icon in ICONS.DLL
        if (    (pStdIcon->pcszFilename)
             && (cmnQuerySetting(sfIconReplacements))
           )
        {
            CHAR szFilename[CCHMAXPATH];
            sprintf(szFilename,
                    "%s\\%s",
                    cmnQueryThemeDirectory(),
                    pStdIcon->pcszFilename);

            if (!icoLoadICOFile(szFilename,
                                &hptrReturn,
                                NULL,
                                NULL))
            {
                // icon loaded:
                pszIconFile = strdup(szFilename);
            }

            /* HMODULE hmodIcons;
            if (hmodIcons = cmnQueryIconsDLL())     // loads on first call
            {
                hmod = hmodIcons;
                // ulResID = pStdIcon->ulIconsDLL;
                hptrReturn = WinLoadPointer(HWND_DESKTOP,
                                            hmod,
                                            ulResID);
            } */
        }
#endif

        if (!hptrReturn)
        {
            // icon replacements disabled, or icon not found,
            // or icons.dll id was null in the first place:

            // load a default icon from either PMWP or XWPRES
            if (pStdIcon->ulPMWP & XWP_MODULE_BIT)
                // load from XWPRES.DLL
                hmod = cmnQueryMainResModuleHandle();
            else
                // load from PMWP.DLL
                DosQueryModuleHandle("PMWP", &hmod);

            ulResID = pStdIcon->ulPMWP & ~XWP_MODULE_BIT;

            if (!(hptrReturn = WinLoadPointer(HWND_DESKTOP,
                                              hmod,
                                              ulResID)))
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "Cannot load icon id %d", ulStdIcon);
        }
    } // end if (pStdIcon)

    // we always create an entry even if loading
    // fails to avoid having to reload that if
    // icons.dll is missing or something
    if (pNode = NEW(ICONTREENODE))
    {
        ZERO(pNode);
        pNode->Tree.ulKey = ulStdIcon;

        pNode->hptr = hptrReturn;

        // set source fields so cmnGetStandardIcon can build
        // an ICONINFO too
        pNode->pszIconFile = pszIconFile;
        pNode->hmod = hmod;
        pNode->ulResID = ulResID;

        treeInsert(&G_IconsCache,
                   &G_cIconsInCache,
                   (TREE*)pNode,
                   treeCompareKeys);
    }

    return pNode;
}

static PICONTREENODE G_pLastIconTreeNode = NULL;
static ULONG         G_ulLastStdIconId = NULLHANDLE;

/*
 *@@ FindIconTreeNode:
 *
 *@@added V0.9.20 (2002-07-31) [umoeller]
 */

STATIC PICONTREENODE FindIconTreeNode(ULONG ulStdIconId)
{
    PICONTREENODE pNode;

    // use a cache if we have the same icon twice
    if (    (ulStdIconId == G_ulLastStdIconId)
         && (G_pLastIconTreeNode)
       )
        return G_pLastIconTreeNode;

    if (pNode = (PICONTREENODE)treeFind(G_IconsCache,
                                        ulStdIconId,
                                        treeCompareKeys))
    {
        // found: cache it for next time
        G_ulLastStdIconId = ulStdIconId;
        G_pLastIconTreeNode = pNode;
    }

    return pNode;
}

/*
 *@@ cmnGetStandardIcon:
 *      returns a HPOINTER for the given STDICON_* id.
 *
 *      If icon replacements are enabled, this will
 *      try to find an icon file in the current themes
 *      directory. Otherwise, or if no such icon exists
 *      for that id, an icon from PMWP.DLL is returned
 *      instead.
 *
 *      As a result, unless something goes very wrong,
 *      this will always load a pointer for the given
 *      STDICON_* ID. The data is then cached so several
 *      calls with the same ID will return the same
 *      data.
 *
 *      The _output_ depends on the pointers that are
 *      passed in:
 *
 *      --  If (phptr != NULL), *phptr receives the
 *          HPOINTER that was built for the given icon
 *          ID. Note that the HPOINTER is built even
 *          if (phptr == NULL) for later calls.
 *
 *      --  If (pcbIconInfo != NULL), this puts the required size
 *          for the ICONINFO to be returned into that buffer.
 *          This is to support wpQueryIconData with a NULL
 *          pIconInfo where the required size must be returned.
 *
 *      --  If (pIconInfo != NULL), this assumes that pIconInfo
 *          points to an ICONINFO structure with a sufficient
 *          size for returning the icon data.
 *          See XWPProgramFile::wpQueryIconData for information
 *          about the formats. This is to support the second
 *          wpQueryIconData call where the caller has allocated
 *          a sufficient buffer and wants the data.
 *          Note that we cannot check whether the buffer is large
 *          enough to hold the data because the stupid
 *          wpQueryIconData method definition has no way to tell
 *          how large the input buffer really is (since it only
 *          _returns_ the size of the data). Bad design, really.
 *          This function will always set ICONINFO.fFormat to
 *          either ICON_FILE or ICON_RESOURCE, depending on
 *          where the internal standard icon was retrieved from.
 *
 *      As indicated above, for the same ID, this will reuse
 *      the same HPOINTER to reduce the load on PM. This is
 *      unlike the WPS which loads even default icons several
 *      times.
 *
 *      As a result, NEVER FREE the icon that is returned.
 *      If you set the icon on an object via _wpSetIcon,
 *      make sure you unset the OBJSTYLE_NOTDEFAULTICON
 *      style flag afterwards, or the icon will be nuked
 *      when the object goes dormant... taking the other
 *      objects' icons with it.
 *
 *      This function is now the central agency for all
 *      the XWorkplace icons and is used from the following
 *      places:
 *
 *      --  The various wpclsQueryIconData overrides to
 *          change WPS default icons (folders, data files,
 *          desktops). To support ICON_FILE, we have also
 *          overridden M_XFldObject::wpclsSetIconData (see
 *          remarks there).
 *
 *      --  The trash can to load the open and closed
 *          icons.
 *
 *      --  Program (files) to get default icons for the
 *          various executable types.
 *
 *      Returns:
 *
 *      --  NO_ERROR: data was set.
 *
 *      --  ERROR_TIMEOUT: internal mutex failed somehow.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  ERROR_PROTECTION_VIOLATION
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 *@@changed V0.9.18 (2002-03-19) [umoeller]: added pcbIconInfo for proper wpQueryIconData support
 */

APIRET cmnGetStandardIcon(ULONG ulStdIcon,
                          HPOINTER *phptr,      // out: if != NULL, newly build icon handle
                          PULONG pcbIconInfo,   // out: if != NULL, size of ICONINFO buffer required
                          PICONINFO pIconInfo)  // out: if != NULL, icon info
{
    BOOL        fLocked = FALSE;
    APIRET      arc = NO_ERROR;

    TRY_LOUD(excpt1)
    {
        if (!(fLocked = !LockIconsCache()))
            arc = ERROR_TIMEOUT;        // V0.9.18 (2002-03-16) [umoeller]
        else
        {
            // icon loaded yet?
            PICONTREENODE pNode;
            if (!(pNode = FindIconTreeNode(ulStdIcon)))         // V0.9.20 (2002-07-31) [umoeller]
                // no: create a new node
                if (!(pNode = LoadNewIcon(ulStdIcon)))
                    arc = ERROR_NOT_ENOUGH_MEMORY;

            if (pNode)
            {
                ULONG cbRequired = sizeof(ICONINFO);
                ULONG ulNameLen = 0;

                if (pNode->pszIconFile)
                {
                    // loaded from file:
                    ulNameLen = strlen(pNode->pszIconFile);
                    cbRequired += ulNameLen + 1;
                }

                // output data, depending on what
                // the caller wants
                if (phptr)
                    *phptr = pNode->hptr;

                if (pcbIconInfo)
                    *pcbIconInfo = cbRequired;          // V0.9.18 (2002-03-19) [umoeller]

                if (pIconInfo)
                {
                    ZERO(pIconInfo);
                    pIconInfo->cb = cbRequired;
                    if (ulNameLen)
                    {
                        // loaded from file:
                        PSZ psz = (PSZ)(pIconInfo + 1);
                        pIconInfo->fFormat = ICON_FILE;
                        memcpy(psz, pNode->pszIconFile, ulNameLen + 1);
                        pIconInfo->pszFileName = psz;
                    }
                    else
                    {
                        // loaded from resource:
                        pIconInfo->fFormat = ICON_RESOURCE;
                        pIconInfo->hmod = pNode->hmod;
                        pIconInfo->resid = pNode->ulResID;
                    }
                }
            }
        }
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    if (fLocked)
        UnlockIconsCache();

    return arc;
}

/*
 *@@ cmnIsStandardIcon:
 *      returns TRUE if hptrIcon is a standard icon
 *      that was loaded through cmnGetStandardIcon
 *      and must therefore not be freed.
 *
 *@@added V0.9.16 (2001-12-18) [umoeller]
 *@@changed V0.9.18 (2002-03-24) [umoeller]: this never returned TRUE, dammit! fixed, now XFldObject::wpUnInitData works
 */

BOOL cmnIsStandardIcon(HPOINTER hptrIcon)
{
    BOOL        fLocked = FALSE,
                brc = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = !LockIconsCache())
        {
            TREE *t = treeFirst(G_IconsCache);
            while (t)
            {
                if (((PICONTREENODE)t)->hptr == hptrIcon)
                {
                    brc = TRUE;
                    break;
                }

                t = treeNext(t);
            }
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (fLocked)
        UnlockIconsCache();

    return brc;
}

/* ******************************************************************
 *
 *   Status bar settings
 *
 ********************************************************************/

/*
 *@@ cmnQueryStatusBarSetting:
 *      returns a PSZ to a certain status bar setting, which
 *      may be:
 *      --      SBS_STATUSBARFONT       font (e.g. "9.WarpSans")
 *      --      SBS_TEXTNONESEL         mnemonics for no-object mode
 *      --      SBS_TEXTMULTISEL        mnemonics for multi-object mode
 *
 *      Note that there is no key for querying the mnemonics for
 *      one-object mode, because this is handled by the functions
 *      in statbars.c to provide different data depending on the
 *      class of the selected object.
 *
 *@@changed V0.9.0 (99-11-14) [umoeller]: made this reentrant, finally
 *@@changed V0.9.16 (2001-12-02) [umoeller]: now loading on demand
 */

PCSZ cmnQueryStatusBarSetting(USHORT usSetting)
{
    PCSZ rc = 0;

    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            switch (usSetting)
            {
                case SBS_STATUSBARFONT:
                    if (!G_szStatusBarFont[0])
                    {
                        // first call:
                        PrfQueryProfileString(HINI_USERPROFILE,
                                              (PSZ)INIAPP_XWORKPLACE,
                                              (PSZ)INIKEY_STATUSBARFONT,
                                              (PSZ)cmnQueryDefaultFont(),   // V0.9.16 (2001-12-02) [umoeller]
                                              &G_szStatusBarFont,
                                              sizeof(G_szStatusBarFont));
                        sscanf(G_szStatusBarFont, "%d.*%s", &(G_ulStatusBarHeight));
                        G_ulStatusBarHeight += 15;
                        /* _PmpfF(("got font %s, height is %d",
                                G_szStatusBarFont,
                                G_ulStatusBarHeight)); */
                    }

                    rc = G_szStatusBarFont;
                break;

                case SBS_TEXTNONESEL:
                    if (!G_szSBTextNoneSel[0])
                    {
#ifndef __NOCFGSTATUSBARS__
                        // first call:
                        if (!PrfQueryProfileString(HINI_USERPROFILE,
                                                   (PSZ)INIAPP_XWORKPLACE,
                                                   (PSZ)INIKEY_SBTEXTNONESEL,
                                                   NULL,
                                                   &G_szSBTextNoneSel,
                                                   sizeof(G_szSBTextNoneSel)))
#endif
                            WinLoadString(G_habThread1,     // kernel.c
                                          cmnQueryNLSModuleHandle(FALSE),
                                          ID_XSSI_SBTEXTNONESEL,
                                          sizeof(G_szSBTextNoneSel), G_szSBTextNoneSel);
                    }
                    rc = G_szSBTextNoneSel;
                break;

                case SBS_TEXTMULTISEL:
                    if (!G_szSBTextMultiSel[0])
                    {
#ifndef __NOCFGSTATUSBARS__
                        // first call:
                        if (!PrfQueryProfileString(HINI_USERPROFILE,
                                                   (PSZ)INIAPP_XWORKPLACE,
                                                   (PSZ)INIKEY_SBTEXTMULTISEL,
                                                   NULL,
                                                   &G_szSBTextMultiSel,
                                                   sizeof(G_szSBTextMultiSel)))
#endif
                            WinLoadString(G_habThread1,     // kernel.c
                                          cmnQueryNLSModuleHandle(FALSE),
                                          ID_XSSI_SBTEXTMULTISEL,
                                          sizeof(G_szSBTextMultiSel), G_szSBTextMultiSel);
                    }

                    rc = G_szSBTextMultiSel;
                break;
            }
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        krnUnlock();

    return rc;
}

/*
 *@@ cmnSetStatusBarSetting:
 *      sets usSetting to pszSetting. If pszSetting == NULL, the
 *      default value will be loaded from the XFolder NLS DLL.
 *      usSetting works just like in cmnQueryStatusBarSetting.
 *
 *@@changed V0.9.0 (99-11-14) [umoeller]: made this reentrant, finally
 *@@changed V0.9.16 (2001-09-29) [umoeller]: now using XWP default font for status bars instead of 8.Helv always
 *@@changed V0.9.16 (2001-12-02) [umoeller]: fixed status bar settings problems
 */

BOOL cmnSetStatusBarSetting(USHORT usSetting, PSZ pszSetting)
{
    BOOL    brc = FALSE;

    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            HAB     habDesktop = WinQueryAnchorBlock(HWND_DESKTOP);
            HMODULE hmodResource = cmnQueryNLSModuleHandle(FALSE);

            brc = TRUE;

            switch (usSetting)
            {
                case SBS_STATUSBARFONT:
                {
                    CHAR szDummy[CCHMAXMNEMONICS];
                    if (pszSetting)
                    {
                        strlcpy(G_szStatusBarFont, pszSetting, sizeof(G_szStatusBarFont));
                        PrfWriteProfileString(HINI_USERPROFILE,
                                              (PSZ)INIAPP_XWORKPLACE,
                                              (PSZ)INIKEY_STATUSBARFONT,
                                              G_szStatusBarFont);
                    }
                    else
                    {
                        // NULL:
                        strlcpy(G_szStatusBarFont,
                                cmnQueryDefaultFont(),      // V0.9.16 (2001-09-29) [umoeller]
                                sizeof(G_szStatusBarFont));
                        PrfWriteProfileString(HINI_USERPROFILE,
                                              (PSZ)INIAPP_XWORKPLACE,
                                              (PSZ)INIKEY_STATUSBARFONT,
                                              NULL);
                    }
                    sscanf(G_szStatusBarFont, "%d.%s", &(G_ulStatusBarHeight), &szDummy);
                    G_ulStatusBarHeight += 15;
                }
                break;

#ifndef __NOCFGSTATUSBARS__

                case SBS_TEXTNONESEL:
                {
                    if (pszSetting)
                    {
                        strlcpy(G_szSBTextNoneSel, pszSetting, sizeof(G_szSBTextNoneSel));
                        PrfWriteProfileString(HINI_USERPROFILE,
                                              (PSZ)INIAPP_XWORKPLACE,
                                              (PSZ)INIKEY_SBTEXTNONESEL,
                                              G_szSBTextNoneSel);
                    }
                    else
                    {
                        WinLoadString(habDesktop,
                                      hmodResource, ID_XSSI_SBTEXTNONESEL,
                                      sizeof(G_szSBTextNoneSel), G_szSBTextNoneSel);
                        PrfWriteProfileString(HINI_USERPROFILE,
                                              (PSZ)INIAPP_XWORKPLACE,
                                              (PSZ)INIKEY_SBTEXTNONESEL,
                                              NULL);
                    }
                }
                break;

                case SBS_TEXTMULTISEL:
                {
                    if (pszSetting)
                    {
                        strlcpy(G_szSBTextMultiSel, pszSetting, sizeof(G_szSBTextMultiSel));
                        PrfWriteProfileString(HINI_USERPROFILE,
                                              (PSZ)INIAPP_XWORKPLACE,
                                              (PSZ)INIKEY_SBTEXTMULTISEL,
                                              G_szSBTextMultiSel);
                    }
                    else
                    {
                        WinLoadString(habDesktop,
                                      hmodResource, ID_XSSI_SBTEXTMULTISEL,
                                      sizeof(G_szSBTextMultiSel), G_szSBTextMultiSel);
                        PrfWriteProfileString(HINI_USERPROFILE,
                                              (PSZ)INIAPP_XWORKPLACE,
                                              (PSZ)INIKEY_SBTEXTMULTISEL,
                                              NULL);
                    }
                }
                break;

#endif

                default:
                    brc = FALSE;

            } // end switch(usSetting)
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        krnUnlock();

    return brc;
}

/*
 *@@ cmnQueryStatusBarHeight:
 *      returns the height of the status bars according to the
 *      current settings in pixels. This was calculated when
 *      the status bar font was set.
 */

ULONG cmnQueryStatusBarHeight(VOID)
{
    return G_ulStatusBarHeight;
}

/* ******************************************************************
 *
 *   Global settings
 *
 ********************************************************************/

#pragma pack(4)

/*
 *@@ OLDGLOBALSETTINGS:
 *      old GLOBALSETTINGS structure used before
 *      XWorkplace V0.9.16.
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 */

typedef struct _OLDGLOBALSETTINGS
{
    ULONG       __fIconReplacements,
                MenuCascadeMode,
                FullPath,
                    // enable "full path in title"
                KeepTitle,
                    // "full path in title": keep existing title
                RemoveX,
                AppdParam,
                __fMoveRefreshNow;
                    // move "Refresh now" to main context menu

    ULONG       MaxPathChars,
                    // maximum no. of chars for "full path in title"
                DefaultMenuItems;
                    // ready-made CTXT_* flags for wpFilterPopupMenu

    LONG        VarMenuOffset;
                    // variable menu offset, "Paranoia" page

    ULONG       fAddSnapToGridDefault;
                    // V0.9.0, was: AddSnapToGridItem
                    // default setting for adding "Snap to grid";
                    // can be overridden in XFolder instance settings

    // "snap to grid" values
    LONG        GridX,
                GridY,
                GridCX,
                GridCY;

    ULONG       fFolderHotkeysDefault,
                    // V0.9.0, was: FolderHotkeysDefault
                    // default setting for enabling folder hotkeys;
                    // can be overridden in XFolder instance settings
                TemplatesOpenSettings;
                    // open settings after creating from template;
                    // 0: do nothing after creation
                    // 1: open settings notebook
                    // 2: make title editable
// XFolder 0.52
    ULONG       RemoveLockInPlaceItem,
                    // XFldObject, Warp 4 only
                RemoveFormatDiskItem,
                    // XFldDisk
                RemoveCheckDiskItem,
                    // XFldDisk
                RemoveViewMenu,
                    // XFolder, Warp 4 only
                RemovePasteItem,
                    // XFldObject, Warp 4 only

                ulRemoved1,             // was: DebugMode,
                AddCopyFilenameItem;
                    // default setting for "Copy filename" (XFldDataFile)
                    // can be overridden in XFolder instance settings
    ULONG       __flXShutdown;
                    // XSD_* shutdown settings
    ULONG       __ulRemoved3, // was: NoWorkerThread,
                    // "Paranoia" page
                __fNoSubclassing,
                    // "Paranoia" page
                TreeViewAutoScroll,
                    // XFolder
                ShowStartupProgress;
                    // XFldStartup
    ULONG       ulStartupObjectDelay;
                    // was: ulStartupDelay;
                    // there's a new ulStartupInitialDelay with V0.9.4 (bottom)
                    // XFldStartup

// XFolder 0.70
    ULONG       __fAddFolderContentItem,
                    // general "Folder content" submenu; this does
                    // not affect favorite folders, which are set
                    // for each folder individually
                __fFolderContentShowIcons,
                    // show icons in folder content menus (both
                    // "folder content" and favorite folders)
                fDefaultStatusBarVisibility,
                    // V0.9.0, was: StatusBar;
                    // default visibility of status bars (XFldWPS),
                    // can be overridden in XFolder instance settings
                    // (unlike fEnableStatusBars below, XWPSetup)
                SBStyle;
                    // status bar style
    LONG        lSBBgndColor,
                lSBTextColor;
                    // status bar colors; can be changed via drag'n'drop
    ULONG       TemplatesReposition;
                    // reposition new objects after creating from template
    USHORT      __usLastRebootExt;
                    // XShutdown: last extended reboot item
    ULONG       AddSelectSomeItem,
                    // XFolder: enable "Select by name"
                __fExtFolderSort,
                    // V0.9.0, was: ReplaceSort;
                    // enable XFolder extended sorting (XWPSetup)
                AlwaysSort,
                    // default "always sort" flag (BOOL)
                _removed1, // DefaultSort,
                    // default sort criterion
                    // moved this down V0.9.12 (2001-05-18) [umoeller]
                __disabled1, // CleanupINIs,
                    // disabled for now V0.9.12 (2001-05-15) [umoeller]

// XFolder 0.80
                _fShowBootupStatus,
                ulRemoved3;
                    // V0.9.0, was: WpsShowClassInfo;
    ULONG       SBForViews,
                    // XFldWPS: SBV_xxx flags
                __fReplFileExists,
                    // V0.9.0, was: ReplConfirms;
                    // XFldWPS, replace "File Exists" dialog
                __fBootLogo,
                    // V0.9.0, was: ShowXFolderAnim
                    // XFldDesktop "Startup" page: show boot logo
                FileAttribs,
                    // XFldDataFile: show "File attributes" submenu
                __fReplaceIconPage,
                    // V0.9.0, was: ShowInternals
                    // XFldObject: add "Object" page to all settings notebooks
                    // V0.9.16 (2001-10-15) [umoeller]: now replacing icon
                    // page instead, renamed also
                ExtendFldrViewMenu;
                    // XFolder: extend Warp 4 "View" submenu

// XWorkplace 0.9.0
    BOOL        fNoExcptBeeps,
                    // XWPSetup "Paranoia": disable exception beeps
                fUse8HelvFont,
                    // XWPSetup "Paranoia": use "8.Helv" font for dialogs;
                    // on Warp 3, this is enabled per default
                __fReplaceFilePage,
                    // XFolder/XFldDataFile: replace three "File" pages
                    // into one
                __fExtAssocs,
                    // XFldDataFile/XFldWPS: extended associations

                // Desktop menu items
                fDTMSort,
                fDTMArrange,
                fDTMSystemSetup,
                fDTMLockup,
                fDTMShutdown,
                fDTMShutdownMenu,

                _ulRemoved4, // fIgnoreFilters,
                    // XFldDataFile/XFldWPS: extended associations
                fMonitorCDRoms,
                __fRestartWPS,
                    // XWPSetup: enable "Restart Desktop"
                __fXShutdown,
                    // XWPSetup: enable XShutdown

                __fEnableStatusBars,
                    // XWPSetup: whether to enable the status bars at all;
                    // unlike fDefaultStatusBarVisibility above
                __fEnableSnap2Grid,
                    // XWPSetup: whether to enable "Snap to grid" at all;
                    // unlike fAddSnapToGridDefault above
                __fEnableFolderHotkeys;
                    // XWPSetup: whether to enable folder hotkeys at all;
                    // unlike fFolderHotkeysDefault above

    BYTE        bDefaultWorkerThreadPriority,
                    // XWPSetup "Paranoia": default priority of Worker thread:
                    //      0: idle +/-0
                    //      1: idle +31
                    //      2: regular +/-0

                fXSystemSounds,
                    // XWPSetup: enable extended system sounds
                fWorkerPriorityBeep,
                    // XWPSetup "Paranoia": beep on priority change

                _bBootLogoStyle,
                    // XFldDesktop "Startup" page:
                    // boot logo style:
                    //      0 = transparent
                    //      1 = blow-up

                bDereferenceShadows,
                    // XFldWPS "Status bars" page 2:
                    // deference shadows flag
                    // changed V0.9.5 (2000-10-07) [umoeller]: now bit flags...
                    // -- STBF_DEREFSHADOWS_SINGLE        0x01
                    // -- STBF_DEREFSHADOWS_MULTIPLE      0x02

    // trashcan settings
                __fTrashDelete,
                __fRemoved1, // fTrashEmptyStartup,
                __fRemoved2; // fTrashEmptyShutdown;
    ULONG       ulTrashConfirmEmpty;
                    // TRSHEMPTY_* flags

    BYTE        __fReplDriveNotReady;
                    // XWPSetup: replace "Drive not ready" dialog

    ULONG       ulIntroHelpShown;
                    // HLPS_* flags for various classes, whether
                    // an introductory help page has been shown
                    // the first time it's been opened

    BYTE        __fEnableXWPHook;
                    // XWPSetup: enable hook (enables object hotkeys,
                    // mouse movement etc.)

    BYTE        __fReplaceArchiving;
                    // XWPSetup: enable Desktop archiving replacement

    BYTE        fAniMouse;
                    // XWPSetup: enable "animated mouse pointers" page in XWPMouse

    BYTE        fNumLockStartup;
                    // XFldDesktop "Startup": set NumLock to ON on Desktop startup

    BYTE        fEnableXPager;
                    // XWPSetup "XPager virtual desktops"; this will cause
                    // XDM_STARTSTOPPAGER to be sent to the daemon

    BYTE        fShowHotkeysInMenus;
                    // on XFldWPS "Hotkeys" page

// XWorkplace 0.9.3

    BYTE        fNoFreakyMenus;
                    // on XWPSetup "Paranoia" page

    BYTE        __fReplaceTrueDelete;
                    // replace "true delete" also?
                    // on XWPSetup "Features" page

// XWorkplace 0.9.4
    BYTE        _fFdrDefaultDoc,
                    // folder default documents enabled?
                    // "Workplace Shell" "View" page
                _fFdrDefaultDocView;
                    // "default doc = folder default view"
                    // "Workplace Shell" "View" page

    BYTE        __fResizeSettingsPages;
                    // XWPSetup: allow resizing of WPS notebook pages?

    ULONG       ulStartupInitialDelay;
                    // XFldStartup: initial delay

// XWorkplace 0.9.5

#ifdef __REPLHANDLES__
    BYTE        fReplaceHandles;
                    // XWPSetup: replace handles management?
#else
    BYTE        fDisabled2;
#endif
    BYTE        _bSaveINIS;
                    // XShutdown: save-INIs method:
                    // -- 0: new method (xprf* APIs)
                    // -- 1: old method (Prf* APIs)
                    // -- 2: do not save

// XWorkplace 0.9.7
    BYTE        fFixLockInPlace;
                    // "Workplace Shell" menus p3: submenu, checkmark
    BYTE        fDTMLogoffNetwork;
                    // "Logoff network now" desktop menu item (XFldDesktop)

// XWorkplace 0.9.9
    BYTE        fFdrAutoRefreshDisabled;
                    // "Folder auto-refresh" on "Workplace Shell" "View" page;
                    // this only has an effect if folder auto-refresh has
                    // been replaced in XWPSetup in the first place
// XWorkplace V0.9.12
    BYTE        bDefaultFolderView;
                    // "default folder view" on XFldWPS "View" page:
                    // -- 0: inherit from parent (default, standard WPS)
                    // -- OPEN_CONTENTS (1): icon view
                    // -- OPEN_TREE (101): tree view
                    // -- OPEN_DETAILS (102): details view

    BYTE        fFoldersFirst;
                    // global sort setting for "folders first"
                    // (TRUE or FALSE)
    LONG        lDefSortCrit;
                    // new global sort criterion (moved this down here
                    // because the value is incompatible with the earlier
                    // setting above, which has been disabled);
                    // this is a LONG because it can have negative values
                    // (see XFolder::xwpSetFldrSort)

    BYTE        __fFixClassTitles;
                    // XWPSetup: override wpclsQueryTitle?

    BYTE        fExtendCloseMenu;
                    // XFldWPS "View" page

    BYTE        fWriteXWPStartupLog;
                    // V0.9.14 (2001-08-21) [umoeller]

// V0.9.16
    BYTE        __fTurboFolders;
                    // V0.9.16 (2001-10-25) [umoeller]
    BYTE        __fNewFileDlg;
                    // V0.9.16 (2001-12-02) [umoeller]
} OLDGLOBALSETTINGS, *POLDGLOBALSETTINGS;

#pragma pack()

/*
 *@@ G_aSettingInfos:
 *      describes the various XWPSETTING's available
 *      to XWorkplace. If you add a new XWPSETTING,
 *      you _must_ add an entry here, or everything
 *      will blow up.
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 *@@changed V0.9.19 (2002-04-17) [umoeller]: adjusted for new menu handling
 *@@changed V1.0.1 (2002-12-15) [umoeller]: removed ext assocs setting @@fixes 231
 */

static const SETTINGINFO G_aSettingInfos[] =
    {
#ifndef __NOICONREPLACEMENTS__
        sfIconReplacements, FIELDOFFSET(OLDGLOBALSETTINGS, __fIconReplacements), 4,
            SP_SETUP_FEATURES, 0,
            "fIconReplacements",
#endif

        // added V0.9.20 (2002-07-19) [umoeller]
        sfHelpReplacements, -1, 0,
            SP_SETUP_FEATURES, 0,
            "fHelpReplacements",

/* #ifndef __NOMOVEREFRESHNOW__
        sfMoveRefreshNow, FIELDOFFSET(OLDGLOBALSETTINGS, __fMoveRefreshNow), 4,
            SP_MENUS_FILE, 0,
            "fMoveRefreshNow",
#endif */

// #ifndef __ALWAYSSUBCLASS__   removed V1.0.0 (2002-09-12) [umoeller]
//         sfNoSubclassing, FIELDOFFSET(OLDGLOBALSETTINGS, __fNoSubclassing), 4,
//             SP_SETUP_PARANOIA, 0,
//             "fNoSubclassing",
// #endif

#ifndef __NOFOLDERCONTENTS__
        sfFolderContentShowIcons, FIELDOFFSET(OLDGLOBALSETTINGS, __fFolderContentShowIcons), 4,
            SP_MENUSETTINGS, 1,
            "fFolderContentShowIcons",
#endif

#ifndef __NOFDRDEFAULTDOCS__
        sfFdrDefaultDoc, FIELDOFFSET(OLDGLOBALSETTINGS, _fFdrDefaultDoc), 1,
            SP_WPS_FOLDERVIEWS, 0,
            "fFdrDefaultDoc",
        sfFdrDefaultDocView, FIELDOFFSET(OLDGLOBALSETTINGS, _fFdrDefaultDocView), 1,
            SP_WPS_FOLDERVIEWS, 0,
            "fFdrDefaultDocView",
#endif

#ifndef __NOBOOTLOGO__
        sfBootLogo, FIELDOFFSET(OLDGLOBALSETTINGS, __fBootLogo), 4,
            SP_DTP_STARTUP, 0,
            "fBootLogo",
        sulBootLogoStyle, FIELDOFFSET(OLDGLOBALSETTINGS, _bBootLogoStyle), 1,
            SP_DTP_STARTUP, 0,
            "ulBootLogoStyle",
#endif

        sfReplaceFilePage, FIELDOFFSET(OLDGLOBALSETTINGS, __fReplaceFilePage), 4,
            SP_SETUP_FEATURES, 0,
            "fReplaceFilePage",

#ifndef __NOCFGSTATUSBARS__
        sfStatusBars, FIELDOFFSET(OLDGLOBALSETTINGS, __fEnableStatusBars), 4,
            SP_SETUP_FEATURES, 0,
            "fStatusBars",
#endif

        // V1.0.1 (2002-11-30) [umoeller]
        sfToolBars, -1, 0,
            SP_SETUP_FEATURES, 0,       // @@todo
            "fToolBars",

#ifndef __NOSNAPTOGRID__
        sfSnap2Grid, FIELDOFFSET(OLDGLOBALSETTINGS, __fEnableSnap2Grid), 4,
            SP_SETUP_FEATURES, 0,
            "fSnap2Grid",
        sfAddSnapToGridDefault, FIELDOFFSET(OLDGLOBALSETTINGS, fAddSnapToGridDefault), 4,
            SP_3SNAPTOGRID, 1,
            "fAddSnapToGridDefault",
        sulGridX, FIELDOFFSET(OLDGLOBALSETTINGS, GridX), 4,
            SP_3SNAPTOGRID, 15,
            "ulGridX",
        sulGridY, FIELDOFFSET(OLDGLOBALSETTINGS, GridY), 4,
            SP_3SNAPTOGRID, 10,
            "ulGridY",
        sulGridCX, FIELDOFFSET(OLDGLOBALSETTINGS, GridCX), 4,
            SP_3SNAPTOGRID, 20,
            "ulGridCX",
        sulGridCY, FIELDOFFSET(OLDGLOBALSETTINGS, GridCY), 4,
            SP_3SNAPTOGRID, 35,
            "ulGridCY",
#endif

        sfFolderHotkeys, FIELDOFFSET(OLDGLOBALSETTINGS, __fEnableFolderHotkeys), 4,
            SP_SETUP_FEATURES, 0,
            "FolderHotkeys",
        sfFolderHotkeysDefault, FIELDOFFSET(OLDGLOBALSETTINGS, fFolderHotkeysDefault), 4,
            SP_4ACCELERATORS, 1,
            "FolderHotkeysDefault",
        sfShowHotkeysInMenus, FIELDOFFSET(OLDGLOBALSETTINGS, fShowHotkeysInMenus), 1,
            SP_4ACCELERATORS, 1,
            "fShowHotkeysInMenus",

// #ifndef __ALWAYSRESIZESETTINGSPAGES__            setting removed V1.0.1 (2002-12-14) [umoeller]
//         sfResizeSettingsPages, FIELDOFFSET(OLDGLOBALSETTINGS, __fResizeSettingsPages), 1,
//             SP_SETUP_FEATURES, 0,
//             "fResizeSettingsPages",
// #endif

        sfReplaceIconPage, FIELDOFFSET(OLDGLOBALSETTINGS, __fReplaceIconPage), 4,
            SP_SETUP_FEATURES, 0,
            "fReplaceIconPage",

        sfReplaceFileExists, FIELDOFFSET(OLDGLOBALSETTINGS, __fReplFileExists), 4,
            SP_SETUP_FEATURES, 0,
            "fReplaceFileExists",

        sfFixClassTitles, FIELDOFFSET(OLDGLOBALSETTINGS, __fFixClassTitles), 1,
            SP_SETUP_FEATURES, 0,
            "fFixClassTitles",

        sfReplaceArchiving, FIELDOFFSET(OLDGLOBALSETTINGS, __fReplaceArchiving), 1,
            SP_SETUP_FEATURES, 0,
            "fReplaceArchiving",

#ifndef __NEVERNEWFILEDLG__
        sfNewFileDlg, FIELDOFFSET(OLDGLOBALSETTINGS, __fNewFileDlg), 1,
            SP_SETUP_FEATURES, 0,
            "fNewFileDlg",
#endif

#ifndef __NOXSHUTDOWN__
        sfXShutdown, FIELDOFFSET(OLDGLOBALSETTINGS, __fXShutdown), 4,
            SP_SETUP_FEATURES, 0,
            "XShutdown",
        sfRestartDesktop, FIELDOFFSET(OLDGLOBALSETTINGS, __fRestartWPS), 4,
            SP_SETUP_FEATURES, 0,
            "RestartDesktop",
        sflXShutdown, FIELDOFFSET(OLDGLOBALSETTINGS, __flXShutdown), 4,
            SP_DTP_SHUTDOWN,
                    XSD_WPS_CLOSEWINDOWS | XSD_REBOOT | XSD_ANIMATE_SHUTDOWN,
            "flXShutdown",
        sulSaveINIS, FIELDOFFSET(OLDGLOBALSETTINGS, _bSaveINIS), 1,
            SP_DTP_SHUTDOWN, 0, // new method, V0.9.5 (2000-08-16) [umoeller]
            "ulSaveINIS",
#endif

#ifndef __NEVERCHECKDESKTOP__
        sfCheckDesktop, -1, 0,
            SP_SETUP_FEATURES, 1,
            "fCheckDesktop",
        sfPrePopulateDesktop, -1, 0,
            SP_SETUP_FEATURES, 0,       // default is off
            "fPrePopulateDesktop",
#endif

        sfExtendedSorting, FIELDOFFSET(OLDGLOBALSETTINGS, __fExtFolderSort), 4,
            SP_SETUP_FEATURES, 0,
            "fExtendedSorting",
        sfXWPHook, FIELDOFFSET(OLDGLOBALSETTINGS, __fEnableXWPHook), 1,
            SP_SETUP_FEATURES, 0,
            "fXWPHook",
#ifndef __NOPAGER__
        sfEnableXPager, FIELDOFFSET(OLDGLOBALSETTINGS, fEnableXPager), 1,
            SP_SETUP_FEATURES, 0,
            "fEnableXPager",
#endif

// #ifndef __NEVEREXTASSOCS__       ext assocs setting removed V1.0.1 (2002-12-15) [umoeller]
//         sfExtAssocs, FIELDOFFSET(OLDGLOBALSETTINGS, __fExtAssocs), 4,
//             SP_SETUP_FEATURES, 0,
//             "fExtAssocs",

//         sfDatafileOBJHANDLE moved to turbo folders setting
//         // sfDatafileOBJHANDLE added with V0.9.20 (2002-08-04) [umoeller]
//         // if this is FALSE, we do not pass data files with the
//         // WP_OBJHANDLE environment variable to avoid creating handles
//         // all the time
//         sfDatafileOBJHANDLE, -1, 0,
//             SP_FILETYPES, TRUE,
//             "fDatafileOBJHANDLE",
// #endif

        // added lazy icons V0.9.20 (2002-07-25) [umoeller]
        sflOwnerDrawIcons, -1, 0,
            SP_WPS_FOLDERVIEWS, OWDRFL_LAZYLOADICON | OWDRFL_SHADOWOVERLAY,
            "sflOwnerDrawIcons",

#ifndef __NEVERREPLACEDRIVENOTREADY__
        sfReplaceDriveNotReady, FIELDOFFSET(OLDGLOBALSETTINGS, __fReplDriveNotReady), 1,
            SP_SETUP_FEATURES, 0,
            "fReplaceDriveNotReady",
#endif

        // added this V0.9.20 (2002-08-08) [umoeller]
        sfReplacePaste, -1, 0,
            SP_SETUP_FEATURES, 0,
            "fReplacePaste",

        // adjusted the following two V0.9.19 (2001-04-13) [umoeller]
        sfReplaceDelete, FIELDOFFSET(OLDGLOBALSETTINGS, __fTrashDelete), 1,
            SP_SETUP_FEATURES, 0,
            "fReplaceDelete",
        sfAlwaysTrueDelete, -1, 0,
            SP_TRASHCAN_SETTINGS, 0,
            "fAlwaysTrueDelete",
#ifndef __NOBOOTUPSTATUS__
        sfShowBootupStatus, FIELDOFFSET(OLDGLOBALSETTINGS, _fShowBootupStatus), 4,
            SP_DTP_STARTUP, 0,
            "fShowBootupStatus",
#endif

#ifndef __NOTURBOFOLDERS__
        sfTurboFolders, FIELDOFFSET(OLDGLOBALSETTINGS, __fTurboFolders), 1,
            SP_SETUP_FEATURES, 0,
            "fTurboFolders",

        // sfDatafileOBJHANDLE added with V0.9.20 (2002-08-04) [umoeller]
        // sfDatafileOBJHANDLE moved to turbo folders setting V1.0.1 (2002-12-15) [umoeller]
        // if this is FALSE, we do not pass data files with the
        // WP_OBJHANDLE environment variable to avoid creating handles
        // all the time
        sfDatafileOBJHANDLE, -1, 0,
            SP_FILETYPES, TRUE,
            "fDatafileOBJHANDLE",
#endif

        sfFdrSplitViews, -1, 0,            // V1.0.0 (2002-09-09) [umoeller]
            SP_SETUP_FEATURES, 0,
            "fFdrSplitViews",

        sulVarMenuOfs, FIELDOFFSET(OLDGLOBALSETTINGS, VarMenuOffset), 4,
            SP_SETUP_PARANOIA, 700,
            "ulVarMenuOffset",

        sfMenuCascadeMode, FIELDOFFSET(OLDGLOBALSETTINGS, MenuCascadeMode), 4,
            SP_26CONFIGITEMS, 1,
            "fMenuCascadeMode",

        sfFixLockInPlace, FIELDOFFSET(OLDGLOBALSETTINGS, fFixLockInPlace), 1,
            SP_MENUSETTINGS, 1,     // default changed V1.0.0 (2002-09-20) [umoeller]
            "fFixLockInPlace",

        // folder view settings
        sfFullPath, FIELDOFFSET(OLDGLOBALSETTINGS, FullPath), 4,
            SP_WPS_FOLDERVIEWS, 0,     // default changed V0.9.19 (2002-04-25) [umoeller]
            "fFullPath",
        sfKeepTitle, FIELDOFFSET(OLDGLOBALSETTINGS, KeepTitle), 4,
            SP_WPS_FOLDERVIEWS, 1,
            "fKeepTitle",
        sulMaxPathChars, FIELDOFFSET(OLDGLOBALSETTINGS, MaxPathChars), 4,
            SP_WPS_FOLDERVIEWS, 25,
            "ulMaxPathChars",
        sfRemoveX, FIELDOFFSET(OLDGLOBALSETTINGS, RemoveX), 4,
            SP_26CONFIGITEMS, 1,
            "fRemoveX",
        sfAppdParam, FIELDOFFSET(OLDGLOBALSETTINGS, AppdParam), 4,
            SP_26CONFIGITEMS, 1,
            "fAppdParam",
        sulTemplatesOpenSettings, FIELDOFFSET(OLDGLOBALSETTINGS, TemplatesOpenSettings), 4,
            SP_26CONFIGITEMS, BM_INDETERMINATE,
            "ulTemplatesOpenSettings",
        sfTemplatesReposition, FIELDOFFSET(OLDGLOBALSETTINGS, TemplatesReposition), 4,
            SP_26CONFIGITEMS, 1,
            "fTemplatesReposition",
        sfTreeViewAutoScroll, FIELDOFFSET(OLDGLOBALSETTINGS, TreeViewAutoScroll), 4,
            SP_WPS_FOLDERVIEWS, 1,
            "fTreeViewAutoScroll",

        // status bar settings
        sfDefaultStatusBarVisibility, FIELDOFFSET(OLDGLOBALSETTINGS, fDefaultStatusBarVisibility), 4,
            SP_STATUSBARS1, 1,
            "fDefaultStatusBarVisibility",
        sulSBStyle, FIELDOFFSET(OLDGLOBALSETTINGS, SBStyle), 4,
            SP_STATUSBARS1, SBSTYLE_WARP4MENU,  // Overridden in cmnLoadGlobalSettings()
            "ulSBStyle",
        slSBBgndColor, FIELDOFFSET(OLDGLOBALSETTINGS, lSBBgndColor), 4,
            SP_STATUSBARS1, RGBCOL_GRAY,        // Overridden in cmnLoadGlobalSettings()
            "lSBBgndColor",
        slSBTextColor, FIELDOFFSET(OLDGLOBALSETTINGS, lSBTextColor), 4,
            SP_STATUSBARS1, RGBCOL_BLACK,       // Overridden in cmnLoadGlobalSettings()
            "lSBTextColor",
        sflSBForViews, FIELDOFFSET(OLDGLOBALSETTINGS, SBForViews), 4,
            SP_STATUSBARS1, SBV_ICON | SBV_DETAILS | SBV_SPLIT,
                            // added split V1.0.1 (2002-12-08) [umoeller]
            "flSBForViews",
#ifndef __NOCFGSTATUSBARS__
        sflDereferenceShadows, FIELDOFFSET(OLDGLOBALSETTINGS, bDereferenceShadows), 1,
            SP_STATUSBARS1, STBF_DEREFSHADOWS_SINGLE,
            "flDereferenceShadows",
#endif

        // tool bar settings V1.0.1 (2002-12-08) [umoeller]
        sfDefaultToolBarVisibility, -1, 0,
            SP_TOOLBARS1, 1,
            "fDefaultToolBarVisibility",
        sflToolBarStyle, -1, 0,
            SP_TOOLBARS1, TBBS_TEXT | TBBS_BIGICON | TBBS_FLAT | TBBS_HILITE,
            "sflToolBarStyle",
        sflTBForViews, -1, 0,
            SP_TOOLBARS1, SBV_ICON | SBV_DETAILS | SBV_SPLIT,
            "flTBForViews",
        sfTBToolTips, -1, 0,
            SP_TOOLBARS1, TRUE,
            "fTBToolTips",

        // startup settings
        sfShowStartupProgress, FIELDOFFSET(OLDGLOBALSETTINGS, ShowStartupProgress), 4,
            SP_STARTUPFOLDER, 1,
            "fShowStartupProgress",
        sulStartupInitialDelay, FIELDOFFSET(OLDGLOBALSETTINGS, ulStartupInitialDelay), 4,
            SP_STARTUPFOLDER, 1000,
            "ulStartupInitialDelay",
        sulStartupObjectDelay, FIELDOFFSET(OLDGLOBALSETTINGS, ulStartupObjectDelay), 4,
            SP_STARTUPFOLDER, 1000,
            "ulStartupObjectDelay",
        sfNumLockStartup, FIELDOFFSET(OLDGLOBALSETTINGS, fNumLockStartup), 1,
            SP_DTP_STARTUP, 0,
            "fNumLockStartup",
        sfWriteXWPStartupLog, FIELDOFFSET(OLDGLOBALSETTINGS, fWriteXWPStartupLog), 1,
            SP_DTP_STARTUP, 1,
            "fWriteXWPStartupLog",

        // folder sort settings
        sfAlwaysSort, FIELDOFFSET(OLDGLOBALSETTINGS, AlwaysSort), 4,
            SP_FLDRSORT_GLOBAL, FALSE,
            "fAlwaysSort",
        sfFoldersFirst, FIELDOFFSET(OLDGLOBALSETTINGS, fFoldersFirst), 1,
            SP_FLDRSORT_GLOBAL, FALSE,
            "fFoldersFirst",
        slDefSortCrit, FIELDOFFSET(OLDGLOBALSETTINGS, lDefSortCrit), 4,
            SP_FLDRSORT_GLOBAL, -2,        // sort by name
            "lDefSortCrit",

#ifndef __NOPARANOIA__
        // paranoia settings
        sfNoExcptBeeps, FIELDOFFSET(OLDGLOBALSETTINGS, fNoExcptBeeps), 4,
            SP_SETUP_PARANOIA, 0,
            "fNoExcptBeeps",
        sfUse8HelvFont, FIELDOFFSET(OLDGLOBALSETTINGS, fUse8HelvFont), 4,
            SP_SETUP_PARANOIA, 0,       // Overridden in cmnLoadGlobalSettings()
            "fUse8HelvFont",
        sulDefaultWorkerThreadPriority, FIELDOFFSET(OLDGLOBALSETTINGS, bDefaultWorkerThreadPriority), 1,
            SP_SETUP_PARANOIA, 1, // idle +31
            "ulDefaultWorkerThreadPriority",
        sfWorkerPriorityBeep, FIELDOFFSET(OLDGLOBALSETTINGS, fWorkerPriorityBeep), 1,
            SP_SETUP_PARANOIA, 0,
            "fWorkerPriorityBeep",
        // sfNoFreakyMenus, FIELDOFFSET(OLDGLOBALSETTINGS, fNoFreakyMenus), 1,
        //     SP_SETUP_PARANOIA, 0,
        //     "fNoFreakyMenus",            removed V1.0.0 (2002-08-26) [umoeller]
#endif

        // misc
#ifndef __NOXSYSTEMSOUNDS__
        sfXSystemSounds, FIELDOFFSET(OLDGLOBALSETTINGS, fXSystemSounds), 1,
            SP_SETUP_FEATURES, 0,
            "fXSystemSounds",
#endif
        susLastRebootExt, FIELDOFFSET(OLDGLOBALSETTINGS, __usLastRebootExt), 2,
            0, 0,
            "usLastRebootExt",
        sflTrashConfirmEmpty, FIELDOFFSET(OLDGLOBALSETTINGS, ulTrashConfirmEmpty), 4,
            SP_TRASHCAN_SETTINGS,
                    TRSHCONF_DESTROYOBJ | TRSHCONF_EMPTYTRASH,
            "flTrashConfirmEmpty",
        sflIntroHelpShown, FIELDOFFSET(OLDGLOBALSETTINGS, ulIntroHelpShown), 4,
            0, 0,
            "flIntroHelpShown",
        sfFdrAutoRefreshDisabled, FIELDOFFSET(OLDGLOBALSETTINGS, fFdrAutoRefreshDisabled), 1,
            SP_WPS_FOLDERVIEWS, 0,
            "fFdrAutoRefreshDisabled",

        sulDefaultFolderView, FIELDOFFSET(OLDGLOBALSETTINGS, bDefaultFolderView), 1,
            SP_WPS_FOLDERVIEWS, 0,
            "ulDefaultFolderView",

        // the following are new with V0.9.19
        sflMenuObjectWPS, -1, 0,
            SP_MENUITEMS, 0,
            "flMenuObjectWPS",
        sflMenuObjectXWP, -1, 0,
            SP_MENUITEMS, 0,
            "flMenuObjectXWP",
        sflMenuFileWPS, -1, 0,
            SP_MENUITEMS, 0,
            "flMenuFileWPS",
        sflMenuFileXWP, -1, 0,
            SP_MENUITEMS, 0,
            "flMenuFileXWP",
        sflMenuFolderWPS, -1, 0,
            SP_MENUITEMS, 0,
            "flMenuFolderWPS",
        sflMenuFolderXWP, -1, 0,
            SP_MENUITEMS, 0,
            "flMenuFolderXWP",
        sflMenuDesktopWPS, -1, 0,
            SP_MENUITEMS, 0,
            "flMenuDesktopWPS",
        sflMenuDesktopXWP, -1, 0,
            SP_MENUITEMS, 0,
            "flMenuDesktopXWP",
        sflMenuDiskWPS, -1, 0,
            SP_MENUITEMS, 0,
            "flMenuDiskWPS",
        sflMenuDiskXWP, -1, 0,
            SP_MENUITEMS, 0,
            "flMenuDiskXWP",
    };

/*
 *@@ cmnFindSettingInfo:
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 */

PCSETTINGINFO cmnFindSettingInfo(XWPSETTING s)
{
    ULONG ul2;
    for (ul2 = 0;
         ul2 < ARRAYITEMCOUNT(G_aSettingInfos);
         ul2++)
    {
        if (s == G_aSettingInfos[ul2].s)
        {
            return &G_aSettingInfos[ul2];
        }
    }

    return NULL;
}

/*
 *@@ ConvertOldGlobalSettings:
 *      calls cmnSetSetting for each possible value
 *      with the corresponding value from the
 *      given OLDGLOBALSETTINGS struct.
 *
 *      As a result, this creates a new-format INI
 *      key for each entry in the old OLDGLOBALSETTINGS.
 *
 *      Gets called from cmnLoadGlobalSettings if
 *      an old-style GLOBALSETTINGS INI entry was
 *      found. The caller should nuke that INI entry.
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 */

STATIC VOID ConvertOldGlobalSettings(POLDGLOBALSETTINGS pOld)
{
    ULONG s, ul2;

    for (s = 0;
         s < ___LAST_SETTING;
         s++)
    {
        // look up the corresponding SETTINGINFO
        PCSETTINGINFO pStore;
        if (    (pStore = cmnFindSettingInfo(s))
                // does entry exist?
             && (pStore->ulOffsetIntoOld != -1)
           )
        {
            // unfortunately me dumb ass chose to use BYTE values
            // for many old global settings, which can't be converted
            // unless we take this into account
            LONG lSetting;
            BYTE b;
            SHORT sh;
            // _Pmpf(("  %s has %d bytes",  pStore->pcszIniKey, pStore->cbOld));
            switch (pStore->cbOld)
            {
                case 1:
                    b = *((PBYTE)pOld + pStore->ulOffsetIntoOld);
                    // _Pmpf(("        byte value %d",  b));
                    lSetting = (ULONG)b;
                break;

                case 2:
                    sh = *((PSHORT)((PBYTE)pOld + pStore->ulOffsetIntoOld));
                    lSetting  = sh;
                break;

                case 4:
                    lSetting = *((PULONG)((PBYTE)pOld + pStore->ulOffsetIntoOld));
                break;
            }
            cmnSetSetting(s, lSetting);
        }
    }
}

/*
 *@@ cmnLoadOneSetting:
 *      loads the data for the given SETTINGINFO from OS2.INI
 *      or sets the default value if no data is found.
 *
 *@@added V1.0.1 (2003-01-25) [umoeller]
 *changed V1.0.4 (2005-03-13) [pr]: Override non-constant settings
 */

ULONG cmnLoadOneSetting(PCSETTINGINFO pThis)
{
    PULONG  pulThis = &G_aulSettings[pThis->s];
    ULONG cb = sizeof(ULONG);
    ULONG ulDefaultValue = pThis->ulDefaultValue;

    // V1.0.4 (2005-03-12) [pr]
    switch(pThis->s)
    {
        case sulSBStyle:
            ulDefaultValue = G_fIsWarp4 ? SBSTYLE_WARP4MENU : SBSTYLE_WARP3RAISED;
            break;

        case slSBBgndColor:
            ulDefaultValue = WinQuerySysColor(HWND_DESKTOP, SYSCLR_INACTIVEBORDER, 0);
            break;

        case slSBTextColor:
            ulDefaultValue = WinQuerySysColor(HWND_DESKTOP, SYSCLR_OUTPUTTEXT, 0);
            break;

#ifndef __NOPARANOIA__
        case sfUse8HelvFont:
            ulDefaultValue = !G_fIsWarp4;
            break;
#endif
    }

    // _Pmpf(("      trying to load %s", pThis->pcszIniKey));
    if (!PrfQueryProfileData(HINI_USER,
                             (PSZ)INIAPP_XWORKPLACE,
                             (PSZ)pThis->pcszIniKey,
                             pulThis,
                             &cb))
    {
        // data not found: use default then
        // _Pmpf(("            PrfQueryProfileData failed"));
        *pulThis = ulDefaultValue;
    }
    else if (cb != sizeof(ULONG))
    {
        // _Pmpf(("            cb is %d", cb));
        *pulThis = ulDefaultValue;
    }

    return *pulThis;
}

/*
 *@@ cmnLoadGlobalSettings:
 *      loads the global settings from OS2.INI.
 *
 *      This is a private function and not prototyped.
 *      It must only get called from init.c during
 *      XWorkplace initialization BEFORE anyone else
 *      uses cmnQuerySetting. All global settings
 *      will be reinitialized here.
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 *@@changed V1.0.4 (2005-03-13) [pr]: Override non-constant settings
 */

VOID cmnLoadGlobalSettings(VOID)
{
    ULONG       cb,
                ul2;
    POLDGLOBALSETTINGS pSettings;

    #ifdef __DEBUG__
        // in debug mode, load debug flags, if we have any
        // V1.0.0 (2002-09-02) [umoeller]
        memset(G_aDebugs, 0, sizeof(G_aDebugs));
        cb = sizeof(G_aDebugs);
        PrfQueryProfileData(HINI_USER,
                            (PSZ)INIAPP_XWORKPLACE,
                            (PSZ)INIKEY_DEBUGFLAGS,
                            G_aDebugs,
                            &cb);
    #endif

    if (    (PrfQueryProfileSize(HINI_USER,
                                 (PSZ)INIAPP_XWORKPLACE,
                                 (PSZ)INIKEY_GLOBALSETTINGS,
                                 &cb))
         && (cb)
         && (pSettings = malloc(sizeof(OLDGLOBALSETTINGS)))
       )
    {
        // we have an old GLOBALSETTINGS structure:
        // convert it to new format

        // first set all settings to safe defaults
        // according to the table

        // we can't use cmnSetDefaultSettings here
        // because that would modify the INI entries
        for (ul2 = 0;
             ul2 < ARRAYITEMCOUNT(G_aSettingInfos);
             ul2++)
        {
            PCSETTINGINFO pThis = &G_aSettingInfos[ul2];
            ULONG ulDefaultValue = pThis->ulDefaultValue;

            // V1.0.4 (2005-03-12) [pr]
            switch(pThis->s)
            {
                case sulSBStyle:
                    ulDefaultValue = G_fIsWarp4 ? SBSTYLE_WARP4MENU : SBSTYLE_WARP3RAISED;
                    break;

                case slSBBgndColor:
                    ulDefaultValue = WinQuerySysColor(HWND_DESKTOP, SYSCLR_INACTIVEBORDER, 0);
                    break;

                case slSBTextColor:
                    ulDefaultValue = WinQuerySysColor(HWND_DESKTOP, SYSCLR_OUTPUTTEXT, 0);
                    break;

#ifndef __NOPARANOIA__
                case sfUse8HelvFont:
                    ulDefaultValue = !G_fIsWarp4;
                    break;
#endif
            }

            G_aulSettings[pThis->s] = ulDefaultValue;
        }

        ZERO(pSettings);
        cb = sizeof(OLDGLOBALSETTINGS);
        PrfQueryProfileData(HINI_USER,
                            (PSZ)INIAPP_XWORKPLACE,
                            (PSZ)INIKEY_GLOBALSETTINGS,
                            pSettings,
                            &cb);
        // _PmpfF(("Converting old settings"));
        ConvertOldGlobalSettings(pSettings);
        free(pSettings);

        // delete the old settings
        PrfWriteProfileData(HINI_USER,
                            (PSZ)INIAPP_XWORKPLACE,
                            (PSZ)INIKEY_GLOBALSETTINGS,
                            NULL,
                            0);
    }
    else
    {
        // no GLOBALSETTINGS structure any more:
        // load settings explicitly
        // _PmpfF(("no old settings, loading new"));
        for (ul2 = 0;
             ul2 < ARRAYITEMCOUNT(G_aSettingInfos);
             ul2++)
        {
            cmnLoadOneSetting(&G_aSettingInfos[ul2]);
        }
    }

    // set global variable to var menu offset
    // V1.0.0 (2002-08-28) [umoeller]
    G_pulVarMenuOfs = &G_aulSettings[sulVarMenuOfs];
}

/*
 *@@ cmnSetDefaultSettings:
 *      resets those Global Settings which correspond to usSettingsPage
 *      in the System notebook to the default values.
 *
 *      usSettingsPage must be one of the SP_* flags def'd in common.h.
 *      This approach allows to reset the settings to default values
 *      both for a single page (in the various settings notebooks)
 *      and, when this function gets called for each of the settings
 *      pages in cmnLoadGlobalSettings, globally.
 *
 *      Warning: If (usSettingsPage == 0), this will globally reset
 *      all settings on the system to the defaults.
 *
 *@@changed V0.9.0 [umoeller]: greatly extended for all the new settings pages
 *@@changed V0.9.16 (2001-12-02) [umoeller]: fixed status bar settings problems
 *@@changed V0.9.16 (2002-01-05) [umoeller]: rewritten
 */

BOOL cmnSetDefaultSettings(USHORT usSettingsPage)
{
    // run through all setting infos
    ULONG ul2;
    for (ul2 = 0;
         ul2 < ARRAYITEMCOUNT(G_aSettingInfos);
         ul2++)
    {
        PCSETTINGINFO pThis = &G_aSettingInfos[ul2];
        if (    (usSettingsPage == 0)
             || (pThis->ulSettingsPageID == usSettingsPage)
           )
        {
            // data must be reset:
            cmnSetSetting(pThis->s,
                          pThis->ulDefaultValue);
        }
    }

    return TRUE;
}

#ifdef __DEBUG__

ULONG cmnQuerySettingDebug(XWPSETTING s,
                           PCSZ pcszSourceFile,
                           ULONG ulLine,
                           PCSZ pcszFunction)
{
#ifndef __NOTURBOFOLDERS__
    if (s == sfTurboFolders)
        return G_fTurboSettingsEnabled;
#endif

    if (s < ___LAST_SETTING)
        return G_aulSettings[s];

#ifdef __DEBUG__
    cmnLog(pcszSourceFile, ulLine, pcszFunction,
           __FUNCTION__ " warning: Invalid setting %d queried.", s);
#endif

    return FALSE;
}

#else

/*
 *@@ cmnQuerySetting:
 *      returns the ULONG setting specified by the
 *      given XWPSETTING enumeration.
 *
 *      This code replaces the GLOBALSETTINGS that
 *      have been in XFolder from the first version
 *      on.
 *
 *      An XWPSETTING is simply an index into our
 *      private settings array. So instead of
 +
 +          PCGLOBALSETTINGS pGlobalSettings = cmnQueryGlobalSettings();
 +          value = pGlobalSettings->fIconReplacements;
 +
 *      the call now simply goes
 +
 +          value = cmnQuerySetting(sfIconReplacements);
 +
 *      Of course this requires that the "s" value is
 *      a valid entry in the XWPSETTING enum.
 *
 *      Note that this function is very fast since we
 *      can do an array lookup. By contrast, cmnSetSetting
 *      might take a while.
 *
 *@@added V0.9.16 (2001-10-11) [umoeller]
 */

ULONG cmnQuerySetting(XWPSETTING s)
{
#ifndef __NOTURBOFOLDERS__
    if (s == sfTurboFolders)
        return G_fTurboSettingsEnabled;
#endif

    if (s < ___LAST_SETTING)
        return G_aulSettings[s];

#ifdef __DEBUG__
    cmnLog(__FILE__, __LINE__, __FUNCTION__,
           "Warning: Invalid feature %d queried.", s);
#endif

    return FALSE;
}

#endif

/*
 *@@ cmnSetSetting:
 *      sets the specified global setting to the
 *      given value. Warning, this modifies the
 *      system's behavior globally; call this only
 *      if you know what you're doing.
 *
 *      Returns TRUE, unless "s" is out of range.
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 *@@changed V0.9.19 (2002-06-02) [umoeller]: fixed bad return value
 */

BOOL cmnSetSetting(XWPSETTING s,
                   ULONG ulValue)
{
    if (s < ___LAST_SETTING)
    {
        PCSETTINGINFO pStore;
        if (pStore = cmnFindSettingInfo(s))
        {
            PULONG pulWrite;

            // store value
            G_aulSettings[s] = ulValue;

            // if this is a non-default value, write to OS2.INI;
            // otherwise delete the key... the default value
            // will then be used by cmnLoadGlobalSettings, and
            // this allows us to change defaults between releases
            if (ulValue != pStore->ulDefaultValue)
                pulWrite = &ulValue;
            else
                pulWrite = NULL;

            return PrfWriteProfileData(HINI_USER,
                                       (PSZ)INIAPP_XWORKPLACE,
                                       (PSZ)pStore->pcszIniKey,
                                       pulWrite,
                                       sizeof(ULONG));
                // return was missing V0.9.19 (2002-06-02) [umoeller]
        }
    }

    cmnLog(__FILE__, __LINE__, __FUNCTION__,
           "Warning: Invalid setting %d set.", s);

    return FALSE;
}

/*
 *@@ cmnBackupSettings:
 *      makes a backup of a bunch of settings,
 *      which is returned in a new buffer.
 *      With paSettings, specify an array of
 *      exactly cItems XWPSETTING entries.
 *
 *      This returns an array of SETTINGSBACKUP
 *      structures where each "s" has the XWPSETTING
 *      index that was passed in and the "ul"
 *      has the corresponding value. The array
 *      has cItems entries as well. The caller
 *      is responsible for free()'ing that buffer.
 *
 *      This function is very useful for notebook
 *      pages to back up global settings for "undo",
 *      since you can also simply store the pointer
 *      which is returned in the NOTEBOOKPAGE.pUser
 *      parameter to be cleaned up automatically.
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 */

PSETTINGSBACKUP cmnBackupSettings(const XWPSETTING *paSettings,
                                  ULONG cItems)         // in: array item count (NOT array size)
{
    PSETTINGSBACKUP p1;

    if (p1 = (PSETTINGSBACKUP)malloc(sizeof(SETTINGSBACKUP) * cItems))
    {
        PSETTINGSBACKUP p = p1;
        ULONG ul;
        for (ul = 0;
             ul < cItems;
             ++ul, ++p)
        {
            p->s = paSettings[ul];
            p->ul = cmnQuerySetting(p->s);
        }
    }

    return p1;
}

/*
 *@@ cmnRestoreSettings:
 *      reverse to cmnBackupSettings, this sets
 *      all the settings from the backup data.
 *
 *      To this function, pass in what was returned
 *      by cmnBackupSettings. cItems _must_ be the
 *      same as in that call or we'll crash.
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 */

VOID cmnRestoreSettings(PSETTINGSBACKUP paSettingsBackup,
                        ULONG cItems)           // in: array item count (NOT array size)
{
    if (paSettingsBackup)
    {
        ULONG ul;
        for (ul = 0;
             ul < cItems;
             ul++)
        {
            cmnSetSetting(paSettingsBackup->s,
                          paSettingsBackup->ul);
            paSettingsBackup++;
        }
    }
}

#ifndef __NOTURBOFOLDERS__

/*
 *@@ cmnTurboFoldersEnabled:
 *      returns the "real" turbo folders setting,
 *      while cmnQuerySetting would return the
 *      filtered feature setting.
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

BOOL cmnTurboFoldersEnabled(VOID)
{
    return G_aulSettings[sfTurboFolders];
}

/*
 *@@ cmnEnableTurboFolders:
 *      private function called by XWPFileSystem::wpclsInitData
 *      to give notification that XWPFileSystem is ready.
 *
 *      Now it might be safe to return TRUE for
 *      cmnGetSetting(sfTurboFolders).
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 */

VOID cmnEnableTurboFolders(VOID)
{
    if (G_aulSettings[sfTurboFolders])
        G_fTurboSettingsEnabled = TRUE;
}

#endif

/* ******************************************************************
 *
 *   Object setup sets V0.9.9 (2001-01-29) [umoeller]
 *
 ********************************************************************/

/*
 *@@ cmnSetupInitData:
 *      setup set helper to be used in a wpInitData override.
 *      See XWPSETUPENTRY for an introduction.
 *
 *      This initializes each entry with the lDefault value
 *      from the XWPSETUPENTRY.
 *
 *      This initializes STG_LONG_DEC, STG_BOOL, and STG_LONG_RGB
 *      entries with a copy of XWPSETUPENTRY.lDefault,
 *      but not STG_BITFLAG fields. For this reason, for bit
 *      fields, always define a preceding STG_LONG entry.
 *
 *      For STG_PSZ, if (lDefault != NULL), this makes a
 *      strdup() copy of that string. It is the responsibility
 *      of the caller to clean that up.
 *
 *@@added V0.9.9 (2001-01-29) [umoeller]
 *@@changed V0.9.20 (2002-07-12) [umoeller]: made array pointer const
 */

VOID cmnSetupInitData(const XWPSETUPENTRY *paSettings, // in: object's setup set
                      ULONG cSettings,       // in: array item count (NOT array size)
                      PVOID somThis)         // in: instance's somThis pointer
{
    ULONG   ul = 0;

    for (ul = 0;
         ul < cSettings;
         ul++)
    {
        const XWPSETUPENTRY *pSettingThis = &paSettings[ul];

        switch (pSettingThis->ulType)
        {
            case STG_LONG_DEC:
            case STG_BOOL:
            case STG_LONG_RGB:
            {
                PLONG plData = (PLONG)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                *plData = pSettingThis->lDefault;
            }
            break;

            // ignore STG_BITFIELD

            case STG_PSZ:
            case STG_PSZARRAY:
            {
                PSZ *ppszData = (PSZ*)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                if (pSettingThis->lDefault)
                    *ppszData = strdup((PSZ)pSettingThis->lDefault);
                else
                    // no default given:
                    *ppszData = NULL;
            }
            break;

            case STG_BINARY:
            {
                PVOID pData = ((PBYTE)somThis + pSettingThis->ulOfsOfData);
                memset(pData, 0, pSettingThis->ulExtra);    // struct size
            }
            break;
        }
    }
}

/*
 *@@ cmnSetupBuildString:
 *      setup set helper to be used in an xwpQuerySetup2 override.
 *      See XWPSETUPENTRY for an introduction.
 *
 *      This appends new setup strings to the specified XSTRING,
 *      which should be safely initialized and, if not empty,
 *      should already end with a semicolon. XWPSETUPENTRY's that
 *      have (pcszSetupString == NULL) or are set to the default
 *      value are skipped.
 *
 *      -- For STG_LONG_DEC, this appends "KEYWORD=%d;".
 *
 *      -- For STG_LONG_RGB, this appends "KEYWORD=red green blue;".
 *
 *      -- For STG_BOOL and STG_BITFLAG, this appends "KEYWORD={YES|NO};".
 *
 *@@added V0.9.7 (2001-01-25) [umoeller]
 *@@changed V0.9.20 (2002-07-12) [umoeller]: made array pointer const
 */

VOID cmnSetupBuildString(const XWPSETUPENTRY *paSettings, // in: object's setup set
                         ULONG cSettings,       // in: array item count (NOT array size)
                         PVOID somThis,         // in: instance's somThis pointer
                         PXSTRING pstr)         // out: setup string

{
    ULONG   ul = 0;
    CHAR    szTemp[100];

    for (ul = 0;
         ul < cSettings;
         ul++)
    {
        const XWPSETUPENTRY *pSettingThis = &paSettings[ul];

        // setup string supported for this?
        if (pSettingThis->pcszSetupString)
        {
            // yes:

            switch (pSettingThis->ulType)
            {
                case STG_LONG_DEC:
                {
                    PLONG plData = (PLONG)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                    if (*plData != pSettingThis->lDefault)
                    {
                        sprintf(szTemp,
                                "%s=%d;",
                                pSettingThis->pcszSetupString,
                                *plData);
                        xstrcat(pstr, szTemp, 0);
                    }
                }
                break;

                case STG_LONG_RGB:      // V0.9.16 (2002-01-26) [umoeller]
                {
                    PLONG plData = (PLONG)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                    if (*plData != pSettingThis->lDefault)
                    {
                        sprintf(szTemp,
                                "%s=%d %d %d;",
                                pSettingThis->pcszSetupString,
                                GET_RED(*plData),
                                GET_GREEN(*plData),
                                GET_BLUE(*plData));
                        xstrcat(pstr, szTemp, 0);
                    }
                }
                break;

                case STG_BOOL:
                {
                    PBOOL plData = (PBOOL)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                    if (*plData != (BOOL)pSettingThis->lDefault)
                    {
                        sprintf(szTemp,
                                "%s=%s;",
                                pSettingThis->pcszSetupString,
                                (*plData == TRUE)
                                    ? "YES"
                                    : "NO");
                        xstrcat(pstr, szTemp, 0);
                    }
                }
                break;

                case STG_BITFLAG:
                {
                    PULONG pulData = (PULONG)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                    if (    ((*pulData) & pSettingThis->ulExtra)            // bitmask
                         != (ULONG)pSettingThis->lDefault
                       )
                    {
                        sprintf(szTemp,
                                "%s=%s;",
                                pSettingThis->pcszSetupString,
                                ((*pulData) & pSettingThis->ulExtra)        // bitmask
                                    ? "YES"
                                    : "NO");
                        xstrcat(pstr, szTemp, 0);
                    }
                }
                break;

                case STG_PSZ:
                {
                    PSZ *ppszData = (PSZ*)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                    ULONG ulDataLen;
                    if (    *ppszData
                         && (ulDataLen = strlen(*ppszData))
                         && strhcmp(*ppszData, (PSZ)pSettingThis->lDefault)
                       )
                    {
                        // not default value:
                        ULONG ul1 = strlen(pSettingThis->pcszSetupString);
                        xstrReserve(pstr,   pstr->cbAllocated
                                          + ul1
                                          + ulDataLen
                                          + 5);

                        xstrcat(pstr, pSettingThis->pcszSetupString, ul1);
                        xstrcatc(pstr, '=');
                        xstrcat(pstr, *ppszData, ulDataLen);
                        xstrcatc(pstr, ';');
                    }
                }
                break;
            }
        } // end if (pSettingThis->pcszSetupString)
    }
}

/*
 *@@ cmnSetupScanString:
 *      setup set helper to be used in a wpSetup override.
 *      See XWPSETUPENTRY for an introduction.
 *
 *      -- For STG_LONG, this expects "KEYWORD=%d;" strings.
 *
 *      -- For STG_BOOL and STG_BITFLAG, this expects
 *         "KEYWORD={YES|NO};" strings.
 *
 *      -- for STG_PSZ, this expects "KEYWORD=STRING;" strings.
 *
 *      Returns FALSE only if values were not set properly.
 *      If pszSetupString does not contain any keywords that
 *      are listed in the XWPSETUPENTRY array, we still return
 *      TRUE, so you can return this func's return value from
 *      wpSetup. (Not specifying a string is not an error.)
 *
 *@@added V0.9.7 (2001-01-25) [umoeller]
 *@@changed V0.9.20 (2002-07-12) [umoeller]: made array pointer const
 *@@changed V0.9.20 (2002-07-12) [umoeller]: pcSuccess can be NULL now
 */

BOOL cmnSetupScanString(WPObject *somSelf,
                        const XWPSETUPENTRY *paSettings, // in: object's setup set
                        ULONG cSettings,         // in: array item count (NOT array size)
                        PVOID somThis,           // in: instance's somThis pointer
                        PSZ pszSetupString,      // in: setup string from wpSetup
                        PULONG pcSuccess)        // out: items successfully parsed and set; ptr can be NULL
{
    BOOL    brc = TRUE;
    CHAR    szValue[500];
    ULONG   cbValue;
    ULONG   ul = 0;

    for (ul = 0;
         ul < cSettings;
         ul++)
    {
        const XWPSETUPENTRY *pSettingThis = &paSettings[ul];

        // setup string supported for this?
        if (pSettingThis->pcszSetupString)
        {
            // yes:
            cbValue = sizeof(szValue);
            if (_wpScanSetupString(somSelf,
                                   pszSetupString,
                                   (PSZ)pSettingThis->pcszSetupString,
                                   szValue,
                                   &cbValue))
            {
                // setting found:
                // see what to do with it
                switch (pSettingThis->ulType)
                {
                    case STG_LONG_DEC:
                    {
                        LONG lValue = atoi(szValue);
                        if (    (lValue >= pSettingThis->lMin)
                             && (lValue <= pSettingThis->lMax)
                           )
                        {
                            PLONG plData = (PLONG)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                            if (*plData != lValue)
                            {
                                // data changed:
                                *plData = lValue;
                                if (pcSuccess)      // V0.9.20 (2002-07-12) [umoeller]
                                    (*pcSuccess)++;
                            }
                        }
                        else
                            brc = FALSE;
                    }
                    break;

                    case STG_LONG_RGB: // V0.9.16 (2002-01-26) [umoeller]
                    {
                        ULONG ulRed, ulGreen, ulBlue;
                        if (    (3 == sscanf(szValue,
                                             "%d %d %d",
                                             &ulRed,
                                             &ulGreen,
                                             &ulBlue))
                             && (ulRed <= 255)
                             && (ulGreen <= 255)
                             && (ulBlue <= 255)
                           )
                        {
                            PLONG plData = (PLONG)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                            LONG lValue = MAKE_RGB(ulRed, ulGreen, ulBlue);
                            if (*plData != lValue)
                            {
                                // data changed:
                                *plData = lValue;
                                if (pcSuccess)
                                    (*pcSuccess)++;
                            }
                        }
                        else
                            brc = FALSE;
                    }
                    break;

                    case STG_BOOL:
                    {
                        BOOL fNew;
                        if (!stricmp(szValue, "YES"))
                            fNew = TRUE;
                        else if (!stricmp(szValue, "NO"))
                            fNew = FALSE;
                        else
                            brc = FALSE;

                        if (brc)
                        {
                            PBOOL plData = (PBOOL)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                            if (*plData != fNew)
                            {
                                *plData = fNew;
                                if (pcSuccess)
                                    (*pcSuccess)++;
                            }
                        }
                    }
                    break;

                    case STG_BITFLAG:
                    {
                        ULONG   ulNew = 0;
                        if (!stricmp(szValue, "YES"))
                            ulNew = pSettingThis->ulExtra;      // bitmask
                        else if (!stricmp(szValue, "NO"))
                            ulNew = 0;
                        else
                            brc = FALSE;

                        if (brc)
                        {
                            PULONG pulData = (PULONG)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                            if (    ((*pulData) & pSettingThis->ulExtra)    // bitmask
                                 != ulNew
                               )
                            {
                                *pulData = (  // clear the bit first:
                                              ((*pulData) & ~pSettingThis->ulExtra) // bitmask
                                              // set it if set
                                            | ulNew);

                                if (pcSuccess)
                                    (*pcSuccess)++;
                            }
                        }
                    }
                    break;

                    case STG_PSZ:
                    {
                        PSZ *ppszData = (PSZ*)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                        if (*ppszData)
                        {
                            // we have something already:
                            free(*ppszData);
                            *ppszData = NULL;
                        }

                        *ppszData = strdup(szValue);
                        if (pcSuccess)
                            (*pcSuccess)++;
                    }
                    break;
                }
            }

            if (!brc)
                // error occurred:
                break;
        } // end if (pSettingThis->pcszSetupString)
    }

    return brc;
}

/*
 *@@ cmnSetupSave:
 *      setup set helper to be used in a wpSaveState override.
 *      See XWPSETUPENTRY for an introduction.
 *
 *      This invokes wpSave* on each setup set entry.
 *
 *      Returns FALSE if values were not saved properly.
 *
 *@@added V0.9.9 (2001-01-29) [umoeller]
 *@@changed V0.9.20 (2002-07-12) [umoeller]: made array pointer const
 */

BOOL cmnSetupSave(WPObject *somSelf,
                  const XWPSETUPENTRY *paSettings, // in: object's setup set
                  ULONG cSettings,         // in: array item count (NOT array size)
                  PCSZ pcszClassName, // in: class name to be used with wpSave*
                  PVOID somThis)           // in: instance's somThis pointer
{
    BOOL    brc = TRUE;
    ULONG   ul = 0;

    for (ul = 0;
         ul < cSettings;
         ul++)
    {
        const XWPSETUPENTRY *pSettingThis = &paSettings[ul];

        if (pSettingThis->ulKey)
        {
            switch (pSettingThis->ulType)
            {
                case STG_LONG_DEC:
                case STG_BOOL:
                case STG_LONG_RGB:
                // case STG_BITFLAG:
                {
                    PULONG pulData = (PULONG)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                    if (!_wpSaveLong(somSelf,
                                     (PSZ)pcszClassName,
                                     pSettingThis->ulKey,
                                     *pulData))
                    {
                        // error:
                        brc = FALSE;
                        break;
                    }
                }
                break;

                case STG_PSZ:
                {
                    PSZ *ppszData = (PSZ*)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                    if (!_wpSaveString(somSelf,
                                       (PSZ)pcszClassName,
                                       pSettingThis->ulKey,
                                       *ppszData))
                    {
                        // error:
                        brc = FALSE;
                        break;
                    }
                }
                break;
            }
        }
    }

    return brc;
}

/*
 *@@ cmnSetupRestore:
 *      setup set helper to be used in a wpRestoreState override.
 *      See XWPSETUPENTRY for an introduction.
 *
 *      This invokes wpRestoreLong on each setup set entry.
 *
 *      For each entry, only if wpRestoreLong succeeded,
 *      the corresponding value in the instance data is
 *      overwritten. Otherwise it is undefined, so this
 *      does not replace the need to call cmnSetupInitData
 *      on wpInitData.
 *
 *@@added V0.9.9 (2001-01-29) [umoeller]
 *@@changed V0.9.20 (2002-07-12) [umoeller]: made array pointer const
 */

BOOL cmnSetupRestore(WPObject *somSelf,
                     const XWPSETUPENTRY *paSettings, // in: object's setup set
                     ULONG cSettings,         // in: array item count (NOT array size)
                     PCSZ pcszClassName, // in: class name to be used with wpRestore*
                     PVOID somThis)           // in: instance's somThis pointer
{
    BOOL    brc = TRUE;
    ULONG   ul = 0;

    for (ul = 0;
         ul < cSettings;
         ul++)
    {
        const XWPSETUPENTRY *pSettingThis = &paSettings[ul];

        if (pSettingThis->ulKey)
        {
            switch (pSettingThis->ulType)
            {
                case STG_LONG_DEC:
                case STG_BOOL:
                case STG_LONG_RGB:
                // case STG_BITFLAG:
                {
                    ULONG   ulTemp = 0;
                    if (_wpRestoreLong(somSelf,
                                       (PSZ)pcszClassName,
                                       pSettingThis->ulKey,
                                       &ulTemp))
                    {
                        // only if found,
                        // replace value
                        PULONG pulData = (PULONG)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                        *pulData = ulTemp;
                    }
                }
                break;

                case STG_PSZ:
                {
                    ULONG cbValue;
                    if (    _wpRestoreString(somSelf,
                                             (PSZ)pcszClassName,
                                             pSettingThis->ulKey,
                                             NULL,      // get size
                                             &cbValue)
                         && cbValue
                       )
                    {
                        PSZ *ppszData = (PSZ*)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                        if (*ppszData)
                        {
                            // we have something already:
                            free(*ppszData);
                            *ppszData = NULL;
                        }
                        *ppszData = (PSZ)malloc(cbValue + 1);
                        _wpRestoreString(somSelf,
                                         (PSZ)pcszClassName,
                                         pSettingThis->ulKey,
                                         *ppszData,
                                         &cbValue);
                    }
                }
                break;
            }
        }
    }

    return brc;
}

/*
 *@@ cmnSetupSetDefaults:
 *      resets part of an object's setup set to default
 *      values. Useful for the "Default" button on a
 *      notebook page.
 *
 *      This requires two arrays as input:
 *
 *      -- PXWPSETUPENTRY paSettings specifies the object's
 *         setup set, as with the other cmnSetup* functions.
 *         This is used to retrieve the default values.
 *
 *      -- PULONG paulOffsets specifies an array of ULONG's.
 *         Each ULONG in that array must specify the
 *         FIELDOFFSET matching one of the items in the
 *         XWPSETUPENTRY array.
 *
 *      This function goes thru the "paulOffsets" array and
 *      finds the corresponding offset in the "paSettings"
 *      setup set array. If found, the corresponding value
 *      (offset to the somThis pointer) is reset to the default.
 *
 *      Again, this ignores STG_BITFIELD entries.
 *
 *      Returns the no. of values successfully changed,
 *      which should match cOffsets.
 *
 *@@added V0.9.9 (2001-01-29) [umoeller]
 *@@changed V0.9.20 (2002-07-12) [umoeller]: made array pointer const
 */

ULONG cmnSetupSetDefaults(const XWPSETUPENTRY *paSettings, // in: object's setup set
                          ULONG cSettings,          // in: array item count (NOT array size)
                          PULONG paulOffsets,
                          ULONG cOffsets,           // in: array item count (NOT array size)
                          PVOID somThis)            // in: instance's somThis pointer
{
    ULONG   ulrc = 0,
            ulOfsThis = 0;

    // go thru the offsets array
    for (ulOfsThis = 0;
         ulOfsThis < cOffsets;
         ulOfsThis++)
    {
        PULONG pulOfsOfDataThis = &paulOffsets[ulOfsThis];

        // now go thru the setup set and find the first entry
        // which matches this offset
        ULONG ulSettingThis = 0;
        for (ulSettingThis = 0;
             ulSettingThis < cSettings;
             ulSettingThis++)
        {
            const XWPSETUPENTRY *pSettingThis = &paSettings[ulSettingThis];

            if (pSettingThis->ulOfsOfData == *pulOfsOfDataThis)
            {
                // found:
                switch (pSettingThis->ulType)
                {
                    case STG_LONG_DEC:
                    case STG_BOOL:
                    case STG_LONG_RGB:
                     // but skip STG_BITFLAG
                    {
                        // reset value
                        PLONG plData = (PLONG)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                        *plData = pSettingThis->lDefault;
                        // raise return count
                        ulrc++;
                    }
                    break;

                    case STG_PSZ:
                    {
                        PSZ *ppszData = (PSZ*)((PBYTE)somThis + pSettingThis->ulOfsOfData);
                        if (*ppszData)
                        {
                            // we have something already:
                            free(*ppszData);
                            *ppszData = NULL;
                        }
                        if (pSettingThis->lDefault)
                            *ppszData = strdup((PSZ)pSettingThis->lDefault);
                    }
                    break;
                }

                break;
            }

        } // for (ulSettingThis = 0;
    } // for (ulOfsThis = 0;

    return ulrc;
}

/*
 *@@ cmnSetupRestoreBackup:
 *      resets part of an object's setup set to values
 *      that have been backed up before.
 *      Useful for the "Undo" button on a notebook page.
 *
 *      As opposed to cmnSetupSetDefaults, this only needs
 *      the "paulOffsets" array.
 *
 *      As with cmnSetupSetDefaults, "PULONG paulOffsets"
 *      specifies an array of ULONG's. Each ULONG in that
 *      array must specify the FIELDOFFSET of a setting
 *      from somThis.
 *
 *      This function goes thru the "paulOffsets" array and
 *      copies the value at each offset from pBackup to
 *      somThis. This does NO MEMORY MANAGEMENT for STG_PSZ
 *      values.
 *
 *      Returns the no. of values successfully changed,
 *      which should match cOffsets.
 *
 *@@added V0.9.9 (2001-01-29) [umoeller]
 */

ULONG cmnSetupRestoreBackup(PULONG paulOffsets,
                            ULONG cOffsets,           // in: array item count (NOT array size)
                            PVOID somThis,            // in: instance's somThis pointer
                            PVOID pBackup)            // in: backup of somThis
{
    ULONG   ulrc = 0,
            ulOfsThis = 0;

    // go thru the offsets array
    for (ulOfsThis = 0;
         ulOfsThis < cOffsets;
         ulOfsThis++)
    {
        PULONG pulOfsOfDataThis = &paulOffsets[ulOfsThis];

        // restore value
        PLONG plTarget = (PLONG)((PBYTE)somThis + *pulOfsOfDataThis);
        PLONG plSource = (PLONG)((PBYTE)pBackup + *pulOfsOfDataThis);
        *plTarget = *plSource;
        // raise return count
        ulrc++;
    } // for (ulOfsThis = 0;

    return ulrc;
}

/* ******************************************************************
 *
 *   Object locks
 *
 ********************************************************************/

/*
 *@@ cmnLockObject:
 *      calls _wpRequestObjectMutexSem on somSelf
 *      and returns somSelf if successful, NULL
 *      otherwise.
 *
 *@@added V0.9.19 (2002-06-15) [umoeller]
 */

WPObject* cmnLockObject(WPObject *somSelf)
{
    return ( (!_wpRequestObjectMutexSem(somSelf, SEM_INDEFINITE_WAIT))
                ? somSelf
                : NULL);
}

/* ******************************************************************
 *
 *   Trash can setup
 *
 ********************************************************************/

/*
 *@@ cmnTrashCanReady:
 *      returns TRUE if the trash can classes are
 *      installed and the default trash can exists.
 *
 *      This does not check for whether "delete to
 *      trash can" is enabled. Query the
 *      global settings to find out.
 *
 *@@added V0.9.1 (2000-02-01) [umoeller]
 *@@changed V0.9.4 (2000-08-03) [umoeller]: moved this here from fileops.c
 */

BOOL cmnTrashCanReady(VOID)
{
    BOOL brc = FALSE;
    // PCKERNELGLOBALS pKernelGlobals = krnQueryGlobals();
    M_XWPTrashCan *pTrashCanClass;
    if (pTrashCanClass = _XWPTrashCan)
    {
        if (_xwpclsQueryDefaultTrashCan(pTrashCanClass))
            brc = TRUE;
    }

    return brc;
}

/*
 *@@ cmnEnableTrashCan:
 *      enables or disables the XWorkplace trash can
 *      altogether after displaying a confirmation prompt.
 *
 *      This does all of the following:
 *      -- (de)register XWPTrashCan and XWPTrashObject;
 *      -- enable "delete into trashcan" support;
 *      -- create or destroy the default trash can.
 *
 *@@added V0.9.1 (2000-02-01) [umoeller]
 *@@changed V0.9.4 (2000-08-03) [umoeller]: moved this here from fileops.c
 *@@changed V0.9.9 (2001-04-08) [umoeller]: wrong item ID
 */

BOOL cmnEnableTrashCan(HWND hwndOwner,     // for message boxes
                       BOOL fEnable)
{
    BOOL    brc = FALSE;

    if (fEnable)
    {
        // enable:
        BOOL    fCreateObject = FALSE;

        if (    (!winhIsClassRegistered(G_pcszXWPTrashCan))
             || (!winhIsClassRegistered(G_pcszXWPTrashObject))
           )
        {
            // classes not registered yet:
            if (cmnMessageBoxExt(hwndOwner,
                                 148,       // XWPSetup
                                 NULL, 0,
                                 170,       // "register trash can?"
                                 MB_YESNO)
                    == MBID_YES)
            {
                // CHAR szRegisterError[500];

                HPOINTER hptrOld = winhSetWaitPointer();

                if (WinRegisterObjectClass((PSZ)G_pcszXWPTrashCan,
                                           (PSZ)cmnQueryMainCodeModuleFilename()))
                    if (WinRegisterObjectClass((PSZ)G_pcszXWPTrashObject,
                                               (PSZ)cmnQueryMainCodeModuleFilename()))
                    {
                        fCreateObject = TRUE;
                        brc = TRUE;
                    }

                WinSetPointer(HWND_DESKTOP, hptrOld);

                if (!brc)
                    // error:
                    cmnMessageBoxExt(hwndOwner,
                                     148,
                                     NULL, 0,
                                     171, // "error"
                                     MB_CANCEL);
            }
        }
        else
            fCreateObject = TRUE;

        if (fCreateObject)
        {
            if (NULLHANDLE == WinQueryObject((PSZ)XFOLDER_TRASHCANID))
            {
                brc = setCreateStandardObject(hwndOwner,
                                              220,        // XWPTrashCan
                                              FALSE,      // no confirm
                                              FALSE);     // XWP object
            }
            else
                brc = TRUE;

#ifndef __ALWAYSTRASHANDTRUEDELETE__
            if (brc)
                cmnSetSetting(sfReplaceDelete, TRUE);
#endif
        }
    } // end if (fEnable)
    else
    {
#ifndef __ALWAYSTRASHANDTRUEDELETE__
        cmnSetSetting(sfReplaceDelete, FALSE);
#endif

        if (krnQueryLock())
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "Global lock already requested.");
        else
        {
            // disable:
            if (cmnMessageBoxExt(hwndOwner,
                                 148,       // XWPSetup
                                 NULL, 0,
                                 172,       // "deregister trash can?"
                                 MB_YESNO | MB_DEFBUTTON2)
                    == MBID_YES)
            {
                XWPTrashCan *pDefaultTrashCan;
                if (pDefaultTrashCan = _xwpclsQueryDefaultTrashCan(_XWPTrashCan))
                    _wpFree(pDefaultTrashCan);
                WinDeregisterObjectClass((PSZ)G_pcszXWPTrashCan);
                WinDeregisterObjectClass((PSZ)G_pcszXWPTrashObject);

                cmnMessageBoxExt(hwndOwner,
                                 148,       // XWPSetup
                                 NULL, 0,
                                 173,       // "done, restart Desktop"
                                 MB_OK);
            }
        }
    }

    return brc;
}

/*
 *@@ cmnDeleteIntoDefTrashCan:
 *      moves a single object to the default trash can.
 *
 *@@added V0.9.4 (2000-08-03) [umoeller]
 *@@changed V0.9.9 (2001-02-06) [umoeller]: renamed from cmnMove2DefTrashCan
 */

BOOL cmnDeleteIntoDefTrashCan(WPObject *pObject)
{
    XWPTrashCan *pDefaultTrashCan;
    if (pDefaultTrashCan = _xwpclsQueryDefaultTrashCan(_XWPTrashCan))
        return _xwpDeleteIntoTrashCan(pDefaultTrashCan,
                                      pObject);

    return FALSE;
}

/*
 *@@ cmnEmptyDefTrashCan:
 *      quick interface to empty the default trash can
 *      without having to include all the trash can headers.
 *
 *      See XWPTrashCan::xwpEmptyTrashCan for the description
 *      of the parameters.
 *
 *@@added V0.9.4 (2000-08-03) [umoeller]
 *@@changed V0.9.7 (2001-01-17) [umoeller]: now returning ULONG
 *@@changed V0.9.19 (2002-06-02) [umoeller]: fixed error code
 */

APIRET cmnEmptyDefTrashCan(HAB hab,        // in: synchronously?
                           PULONG pulDeleted, // out: if TRUE is returned, no. of deleted objects; can be 0
                           HWND hwndConfirmOwner) // in: if != NULLHANDLE, confirm empty
{
    XWPTrashCan *pDefaultTrashCan;
    if (pDefaultTrashCan = _xwpclsQueryDefaultTrashCan(_XWPTrashCan))
        return _xwpEmptyTrashCan(pDefaultTrashCan,
                                 hab,
                                 pulDeleted,
                                 hwndConfirmOwner);

    return FOPSERR_NO_TRASHCAN; // V0.9.19 (2002-06-02) [umoeller]
}

/* ******************************************************************
 *
 *   Bug reports
 *
 ********************************************************************/

/*
 *@@ cmnBugReport:
 *      produces a bug report.
 *
 *      pstr is assumed to be initialized and is not
 *      erased. That is, we'll always append to that
 *      string.
 *
 *      Line format is \n only (not \r\n).
 *
 *@@added V1.0.0 (2002-08-28) [umoeller]
 */

VOID cmnBugReport(PXSTRING pstr)
{
    ULONG           aulBuf[3];
    PCSZ            pcszVersion = "unknown";
    APIRET          arc;
    PQPROCSTAT16    pqp;
    DATETIME        DT;
    POBJCLASS       pClassList;
    CHAR            szKernelFile[] = "?:\\OS2KRNL";
    PEXECUTABLE     pexeKernel;
    ULONG           ul;

    /*
     * Header:
     *
     */

    xstrcat(pstr, "XWorkplace bug report\n", 0);
    xstrcat(pstr, "---------------------\n", 0);

    DosGetDateTime(&DT);
    xstrCatf(pstr,
             "\nDate: %04d-%02d-%02d, Time: %02d:%02d:%02d\n",
             DT.year, DT.month, DT.day,
             DT.hours, DT.minutes, DT.seconds);
    xstrcat(pstr,
            "--------------------------------\n",
            0);

    xstrcat(pstr,
            "\nPlease take a look at the root directory of your boot drive \n"
            "if you find any *.log files there. Send this report and those \n"
            "files to " CONTACT_ADDRESS_USER ". Thank you.\n",
            0);

    xstrcat(pstr, "\nRunning XWorkplace version: " BLDLEVEL_VERSION " built " __DATE__ "\n", 0);

    /*
     * OS/2 Kernel:
     *
     */

    DosQuerySysInfo(QSV_VERSION_MAJOR,      // 11
                    QSV_VERSION_REVISION,   // 13
                    aulBuf, sizeof(aulBuf));
    // Warp 3 is reported as 20.30
    // Warp 4 is reported as 20.40
    // Aurora is reported as 20.45
    if (aulBuf[0] == 20)
    {
        switch (aulBuf[1])
        {
            case 30: pcszVersion = "Warp 3"; break;
            case 40: pcszVersion = "Warp 4"; break;
            case 45: pcszVersion = "WSeB kernel"; break;
        }
    }
    xstrCatf(pstr,
             "Running OS/2 version: %u.%u.%u (%s)\n",
             aulBuf[0],
             aulBuf[1],
             aulBuf[2],
             pcszVersion);

    szKernelFile[0] = doshQueryBootDrive();
    if (arc = exehOpen(szKernelFile,
                       &pexeKernel))
        xstrCatf(pstr,
                 "exehOpen returned %d for %s\n",
                 arc,
                 szKernelFile);
    else
    {
        if (arc = exehQueryBldLevel(pexeKernel))
            xstrCatf(pstr,
                     "exehQueryBldLevel returned %d for %s\n",
                     arc,
                     szKernelFile);
        else
        {
            xstrCatf(pstr,
                     "%s build level: %s\n",
                     szKernelFile,
                     pexeKernel->pszDescription);
        }

        exehClose(&pexeKernel);
    }

    DosQuerySysInfo(QSV_TOTPHYSMEM,
                    QSV_TOTPHYSMEM,
                    aulBuf,
                    sizeof(aulBuf));
    xstrCatf(pstr,
             "Installed physical memory: %d MB\n",
             aulBuf[0] / 1024 / 1024);

    #ifndef QSV_NUMPROCESSORS
        #define QSV_NUMPROCESSORS       26
    #endif

    if (arc = DosQuerySysInfo(QSV_NUMPROCESSORS,
                              QSV_NUMPROCESSORS,
                              aulBuf,
                              sizeof(aulBuf)))
        xstrCatf(pstr,
                 "Error %d getting QSV_NUMPROCESSORS\n",
                 arc);
    else
        xstrCatf(pstr,
                 "Number of processors: %d\n",
                 aulBuf[0]);

    #ifndef QSV_VIRTUALADDRESSLIMIT
        #define QSV_VIRTUALADDRESSLIMIT 30
    #endif

    if (arc = DosQuerySysInfo(QSV_VIRTUALADDRESSLIMIT,
                              QSV_VIRTUALADDRESSLIMIT,
                              aulBuf,
                              sizeof(aulBuf)))
        xstrCatf(pstr,
                 "Error %d getting QSV_VIRTUALADDRESSLIMIT\n",
                 arc);
    else
        xstrCatf(pstr,
                 "Virtual address limit: 0x%lX (%d)\n",
                 aulBuf[0],
                 aulBuf[0]);

    /*
     * dump running processes:
     *
     */

    xstrcat(pstr,
            "\nRunning processes:\n",
            0);

    if (arc = prc16GetInfo(&pqp))
        xstrCatf(pstr,
                 "Error %d occurred getting process list\n",
                 arc);
    else
    {
        PQPROCESS16 pProcess;
        ULONG       cProcesses = 0;

        xstrcat(pstr,
                "    PID    PPID   Module\n",
                0);

        for ( pProcess = (PQPROCESS16)PTR(pqp->ulProcesses, 0);
              pProcess->ulType != 3;
              pProcess = (PQPROCESS16)PTR(pProcess->ulThreadList,
                                          pProcess->usThreads * sizeof(QTHREAD16))
            )
        {
            CHAR    szModule[CCHMAXPATH];
            if (arc = DosQueryModuleName(pProcess->usHModule,
                                         sizeof(szModule),
                                         szModule))
                sprintf(szModule,
                        "[DosQueryModuleName returned %d for HMODULE 0x%lX]",
                        arc,
                        pProcess->usHModule);
            xstrCatf(pstr,
                     "    0x%04lX 0x%04lX %s\n",
                     pProcess->usPID,
                     pProcess->usParentPID,
                     szModule);

            ++cProcesses;
        }

        prc16FreeInfo(pqp);
    }

    /*
     * dump WPS classes:
     *
     */

    xstrcat(pstr, "\nInstalled WPS classes:\n", 0);

    if (!(pClassList = (POBJCLASS)winhQueryWPSClassList()))
        xstrcat(pstr, "Cannot get class list\n", 0);
    else
    {
        POBJCLASS pThis = pClassList;

        while (pThis)
        {
            CHAR    szTemp[500];
            PCSZ    pcszTemp;
            HMODULE hmod;
            if (!DosQueryModuleHandle(pThis->pszModName, &hmod))
            {
                ul = sprintf(szTemp,
                             "HMODULE 0x%04lX loaded from ",
                             hmod);
                DosQueryModuleName(hmod,
                                   sizeof(szTemp) - ul,
                                   szTemp + ul);
                pcszTemp = szTemp;
            }
            else
                pcszTemp = "not loaded";


            xstrCatf(pstr,
                     "    %s -- %s\n"
                     "        %s\n",
                     pThis->pszClassName,
                     pThis->pszModName,
                     pcszTemp);

            pThis = pThis->pNext;
        }

        free(pClassList);
    }

    /*
     * dump XWP settings:
     *
     */

    xstrcat(pstr, "\nGlobal XWorkplace settings:\n", 0);

    for (ul = 0;
         ul < ARRAYITEMCOUNT(G_aSettingInfos);
         ++ul)
    {
        ULONG   ulValue = cmnQuerySetting(G_aSettingInfos[ul].s);
        xstrCatf(pstr,
                 "    %s is 0x%lX (%d)\n",
                 G_aSettingInfos[ul].pcszIniKey,
                 ulValue,
                 ulValue);
    }

}

/* ******************************************************************
 *
 *   Product info
 *
 ********************************************************************/

#ifndef __EWORKPLACE__

/*
 *@@ cmnAddProductInfoMenuItem:
 *      adds the XWP product info menu item to the menu.
 *
 *@@added V0.9.9 (2001-04-05) [umoeller]
 */

BOOL cmnAddProductInfoMenuItem(WPFolder *somSelf,
                               HWND hwndMenu)   // in: main menu with "Help" submenu
{
    BOOL brc = FALSE;

    MENUITEM mi;

    PMPF_MENUS(("  Inserting 'Product info'"));

    // get handle to the WPObject's "Help" submenu in the
    // the folder's popup menu
    if (winhQueryMenuItem(hwndMenu,
                          WPMENUID_HELP,
                          TRUE,
                          &mi))
    {
        // mi.hwndSubMenu now contains "Help" submenu handle,
        // which we add items to now
        cmnInsertSeparator(mi.hwndSubMenu, MIT_END);
        winhInsertMenuItem(mi.hwndSubMenu,
                           MIT_END,
                           *G_pulVarMenuOfs + ID_XFMI_OFS_PRODINFO,
                           cmnGetString(ID_XSSI_PRODUCTINFO),  // pszProductInfo
                           MIS_TEXT, 0);
        brc = TRUE;
    }
    // else: "Help" menu not found, but this can
    // happen in Warp 4 folder menu bars

    return brc;
}

#endif

#ifdef __ECSPRODUCTINFO__
#define INFO_WIDTH  150
#else
#define INFO_WIDTH  250
#endif

static CONTROLDEF
    ProductInfoBitmap = CONTROLDEF_BITMAP(
                            NULLHANDLE,     // replaced with HBITMAP below
                            ID_XFD_PRODLOGO),

    ProductInfoText1 =
        {
            WC_STATIC,
            NULL,               // XWorkplace or eCS Desktop
            WS_VISIBLE | SS_TEXT | DT_LEFT | DT_TOP | DT_WORDBREAK,
            -1,
            "9.WarpSans Bold",
            {INFO_WIDTH, -1},
            COMMON_SPACING
        },
    ProductInfoSepLine1 =
        {
            WC_CCTL_SEPARATOR,
            NULL,
            WS_VISIBLE,
            9998,
            NULL,
            {INFO_WIDTH, 4},
            COMMON_SPACING
        },
    ProductInfoText3 = CONTROLDEF_XTEXTVIEW_HTML(NULL, ID_XSDI_INFO_STRING, INFO_WIDTH, NULL),
    ProductInfoSepLine2 = CONTROLDEF_SEPARATORLINE(9999, INFO_WIDTH, 4),
    ProductInfoInstalledMemoryTxt = LOADDEF_TEXT(ID_XSDI_INFO_MAINMEM_TXT),
    ProductInfoInstalledMemoryValue = CONTROLDEF_TEXT_RIGHT(
                            NULL,
                            ID_XSDI_INFO_MAINMEM_VALUE,
                            40,
                            SZL_AUTOSIZE),
    ProductInfoFreeMemoryTxt = LOADDEF_TEXT(ID_XSDI_INFO_FREEMEM_TXT),
    ProductInfoFreeMemoryValue = CONTROLDEF_TEXT_RIGHT(
                            NULL,
                            ID_XSDI_INFO_FREEMEM_VALUE,
                            40,
                            SZL_AUTOSIZE),
    ProductInfoSepVert = CONTROLDEF_TEXT(" ", -1, 10, 2),
    ProductInfoInstalledKBytes = CONTROLDEF_TEXT(
                            "KBytes",
                            -1,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE),
    ProductInfoFreeKBytes = CONTROLDEF_TEXT(
                            NULL,
                            -1,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE),
    BugReportButton = CONTROLDEF_PUSHBUTTON(
                            LOAD_STRING,
                            ID_XSDI_INFO_BUGREPORT,
                            SZL_AUTOSIZE,
                            STD_BUTTON_HEIGHT);


static const DLGHITEM dlgProductInfo[] =
    {
        START_TABLE_ALIGN,            // root table, required
            START_ROW(ROW_VALIGN_CENTER),
                CONTROL_DEF(&ProductInfoBitmap),
                START_TABLE,
                    START_ROW(0),
                        CONTROL_DEF(&ProductInfoText1),
                    START_ROW(0),
                        CONTROL_DEF(&ProductInfoSepLine1),
                    START_ROW(0),
                        CONTROL_DEF(&ProductInfoText3),
                    START_ROW(0),
                        CONTROL_DEF(&ProductInfoSepLine2),
                    START_ROW(0),
                        START_TABLE_ALIGN,
                            START_ROW(ROW_VALIGN_CENTER),
                                CONTROL_DEF(&ProductInfoInstalledMemoryTxt),
                                CONTROL_DEF(&ProductInfoInstalledMemoryValue),
                                CONTROL_DEF(&ProductInfoInstalledKBytes),
                            START_ROW(ROW_VALIGN_CENTER),
                                CONTROL_DEF(&ProductInfoFreeMemoryTxt),
                                CONTROL_DEF(&ProductInfoFreeMemoryValue),
                                CONTROL_DEF(&ProductInfoFreeKBytes),
                        END_TABLE,
                END_TABLE,
            START_ROW(ROW_VALIGN_CENTER),
                START_ROW(0),
                    CONTROL_DEF(&ProductInfoSepVert),
            START_ROW(ROW_VALIGN_CENTER),
                CONTROL_DEF(&ProductInfoSepVert),
                START_TABLE,
                    START_ROW(0),
                        CONTROL_DEF(&G_OKButton),
#ifndef __EWORKPLACE__
                        CONTROL_DEF(&BugReportButton),
#endif
                END_TABLE,
        END_TABLE
    };

/*
 *@@ fnwpProductInfo:
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 *@@changed V1.0.0 (2002-08-28) [umoeller]: added bug report
 */

STATIC MRESULT EXPENTRY fnwpProductInfo(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc;

    switch (msg)
    {
        case WM_CONTROL:
            if (    (SHORT1FROMMP(mp1) == ID_XSDI_INFO_STRING)
                 && (SHORT2FROMMP(mp1) == TXVN_LINK)
               )
            {
                CHAR szTemp[CCHMAXPATH];

                if (!strcmp(mp2, "inf"))
                {
                    HAPP happ;

                    cmnQueryXWPBasePath(szTemp);
                    sprintf(szTemp + strlen(szTemp),
                            "\\xfldr%s.inf",
                            cmnQueryLanguageCode());

                    appQuickStartApp("view.exe",
                                     PROG_PM,
                                     szTemp,
                                     NULL,
                                     &happ,
                                     NULL);         // don't wait
                }
                else if (!strcmp(mp2, "copying"))
                {
                    WPObject *pobj;
                    cmnQueryXWPBasePath(szTemp);
                    strcat(szTemp, "\\COPYING");
                    if (pobj = _wpclsQueryObjectFromPath(_WPFileSystem, szTemp))
                        krnPostThread1ObjectMsg(T1M_OPENOBJECTFROMPTR,
                                                (MPARAM)pobj,
                                                (MPARAM)OPEN_DEFAULT);
                }
                else if (!strnicmp(mp2, "http://", 7))
                {
                    CHAR    szBrowser[CCHMAXPATH];
                    APIRET  arc;
                    if (arc = appOpenURL((PCSZ)mp2,
                                         szBrowser,
                                         sizeof(szBrowser)))
                    {
                        XSTRING str2, str3;
                        PCSZ    apcsz = szBrowser;
                        xstrInit(&str2, 0);
                        xstrInit(&str3, 0);
                        cmnGetMessage(&apcsz,
                                      1,
                                      &str2,
                                      245); // An error occurred while &xwp; was trying to start your system's default browser (%1):
                        cmnGetMessage(NULL,
                                      0,
                                      &str3,
                                      246); // Check your default browser settings in any URL object.
                        cmnDosErrorMsgBox(hwnd,
                                          NULL, // in: string for %1 message or NULL
                                          cmnGetString(ID_XSDI_INFO_TITLE),
                                          str2.psz,
                                          arc,
                                          str3.psz,
                                          MB_OK,
                                          FALSE);       // short format
                        xstrClear(&str2);
                        xstrClear(&str3);
                    }
                }
            }
        break;

        case WM_COMMAND:
            if ((ULONG)mp1 == ID_XSDI_INFO_BUGREPORT)
            {
                XSTRING str;
                xstrInit(&str, 0);

                // compose the bug report
                cmnBugReport(&str);

                winhSetClipboardText(WinQueryAnchorBlock(hwnd),
                                     str.psz,
                                     str.ulLength + 1);

                cmnMessageBoxExt(hwnd,
                                 121, // &xwp;
                                 NULL,
                                 0,
                                 251,
                                 DID_OK);

                xstrClear(&str);
            }
            else
                mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
        break;

        default:
            mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

APIRET16 APIENTRY16 Dos16MemAvail(PULONG pulAvailMem);

/*
 *@@ cmnShowProductInfo:
 *      shows the XWorkplace "Product info" dlg.
 *      This calls WinProcessDlg in turn.
 *
 *@@added V0.9.1 (2000-02-13) [umoeller]
 *@@changed V0.9.5 (2000-10-07) [umoeller]: now using cmn_fnwpProductInfo
 *@@changed V0.9.13 (2001-06-23) [umoeller]: added hwndOwner
 *@@changed V0.9.20 (2002-08-10) [umoeller]: quite heavily rewritten
 */

VOID cmnShowProductInfo(HWND hwndOwner,     // in: owner window or NULLHANDLE
                        ULONG ulSound)      // in: sound intex to play
{
    // advertise for myself
    HPS         hps;
    HBITMAP     hbmLogo = NULLHANDLE;
    HWND        hwndInfo;
    XSTRING     strInfo,
                strInfoECS1;

    CHAR        szPhysMem[50],
                szFreeMem[50],
                szFreeMemKBytes[50];
    ULONG       ulMem, ulFreeMem;

#ifndef __NOXSYSTEMSOUNDS__
    cmnPlaySystemSound(ulSound);
#endif

    // bitmap
    if (hps = WinGetPS(HWND_DESKTOP))
    {
        hbmLogo = GpiLoadBitmap(hps,
                                cmnQueryMainResModuleHandle(),
                                ID_XWPBIGLOGO,
                                0,
                                0); // no stretch
        ProductInfoBitmap.pcszText = (PCSZ)hbmLogo;
        WinReleasePS(hps);
    }

    xstrInit(&strInfoECS1, 0);
    cmnGetMessageExt(NULL, 0,
                     &strInfoECS1,
                     "PRODUCTINFO");
    ProductInfoText1.pcszText = strInfoECS1.psz;

    xstrInit(&strInfo, 0);
    cmnGetMessage(NULL, 0,
                  &strInfo,
                  140);
    txvStripLinefeeds(&strInfo.psz, 4);
    ProductInfoText3.pcszText = strInfo.psz;

    DosQuerySysInfo(QSV_TOTPHYSMEM,
                    QSV_TOTPHYSMEM,
                    &ulMem, sizeof(ulMem));
    nlsThousandsULong(szPhysMem, ulMem / 1024, cmnQueryThousandsSeparator());
    ProductInfoInstalledMemoryValue.pcszText = szPhysMem;

    Dos16MemAvail(&ulFreeMem);
    nlsThousandsULong(szFreeMem, ulFreeMem / 1024, cmnQueryThousandsSeparator());
    ProductInfoFreeMemoryValue.pcszText = szFreeMem;

    sprintf(szFreeMemKBytes,
            "KBytes (%u %%)",
            (ULONG)((double)ulFreeMem * 100 / ulMem));
    ProductInfoFreeKBytes.pcszText = szFreeMemKBytes;

    if (!dlghCreateDlg(&hwndInfo,
                       hwndOwner,
                       FCF_FIXED_DLG,
                       fnwpProductInfo, // WinDefDlgProc,   V0.9.20 (2002-08-10) [umoeller]
                       cmnGetString(ID_XSDI_INFO_TITLE),
                       dlgProductInfo,
                       ARRAYITEMCOUNT(dlgProductInfo),
                       NULL,
                       cmnQueryDefaultFont()))
    {
        winhCenterWindow(hwndInfo);
        WinProcessDlg(hwndInfo);
        winhDestroyWindow(&hwndInfo);
    }

    if (hbmLogo)
        GpiDeleteBitmap(hbmLogo);

    if (strInfo.psz)
        free(strInfo.psz);

    xstrClear(&strInfoECS1);
}

/* ******************************************************************
 *
 *   Miscellaneae
 *
 ********************************************************************/

/*
 *@@ cmnQueryCountrySettings:
 *      returns the global COUNTRYSETTINGS (see helpers\prfh.c)
 *      as set in the "Country" object, which are cached for speed.
 *
 *      If (fReload == TRUE), the settings are re-read.
 *
 *@@added V0.9.6 (2000-11-12) [umoeller]
 */

PCOUNTRYSETTINGS2 cmnQueryCountrySettings(BOOL fReload)
{
    if ((!G_fCountrySettingsLoaded) || (fReload))
    {
        nlsQueryCountrySettings(&G_CountrySettings);
        G_fCountrySettingsLoaded = TRUE;
    }

    return &G_CountrySettings;
}

/*
 *@@ cmnQueryThousandsSeparator:
 *      returns the thousands separator from the "Country"
 *      object.
 *
 *@@added V0.9.6 (2000-11-12) [umoeller]
 */

CHAR cmnQueryThousandsSeparator(VOID)
{
    PCOUNTRYSETTINGS2 p = cmnQueryCountrySettings(FALSE);
    return p->cs.cThousands;
}

/*
 *@@ cmnIsValidHotkey:
 *      returns TRUE if the specified key combo can
 *      be used as a hotkey without endangering the
 *      system.
 *
 *@@added V0.9.4 (2000-08-03) [umoeller]
 */

BOOL cmnIsValidHotkey(USHORT usFlags,
                      USHORT usKeyCode)
{
    return (
                // must be a virtual key
                (  (  ((usFlags & KC_VIRTUALKEY) != 0)
                // or Ctrl or Alt must be pressed
                   || ((usFlags & KC_CTRL) != 0)
                   || ((usFlags & KC_ALT) != 0)
                // or one of the Win95 keys must be pressed
                   || (   ((usFlags & KC_VIRTUALKEY) == 0)
                       && (     (usKeyCode == 0xEC00)
                            ||  (usKeyCode == 0xED00)
                            ||  (usKeyCode == 0xEE00)
                          )
                   )
                )
                // OK:
                // filter out lone modifier keys
                && (    ((usFlags & KC_VIRTUALKEY) == 0)
                     || (   (usKeyCode != VK_SHIFT)     // shift
                         && (usKeyCode != VK_CTRL)     // ctrl
                         && (usKeyCode != VK_ALT)     // alt
                // and filter out the tab key too
                         && (usKeyCode != VK_TAB)     // tab
                        )
                   )
                )
           );
}

/*
 *@@ cmnDescribeKey:
 *      this stores a description of a certain
 *      key into pszBuf, using the NLS DLL strings.
 *      usFlags is as in WM_CHAR.
 *
 *      If (usFlags & KC_VIRTUALKEY), usKeyCode must
 *      be usvk of WM_CHAR (VK_* code), or usch otherwise.
 *
 *      Returns TRUE if this was a valid key combo.
 *
 *@@changed V1.0.1 (2003-01-30) [umoeller]: added cbBuf parameter, fixed buf overflows
 *@@changed V1.0.9 (2012-02-27) [pr]: support hotkey description @@fixes 249
 */

BOOL cmnDescribeKey(PSZ pszBuf,            // out: key description
                    USHORT usFlags,
                    USHORT usKeyCode,
                    PCSZ pcszDescription,
                    ULONG cbBuf)           // in: size of pszBuf
{
    BOOL brc = TRUE;

    ULONG ulID = 0;
    PCSZ pcszCopy = NULL;

    *pszBuf = 0;
    if (usFlags & KC_CTRL)
        cmnGetString2(pszBuf, ID_XSSI_KEY_CTRL, cbBuf);
    if (usFlags & KC_SHIFT)
        strlcat(pszBuf, cmnGetString(ID_XSSI_KEY_SHIFT), cbBuf);
    if (usFlags & KC_ALT)
        strlcat(pszBuf, cmnGetString(ID_XSSI_KEY_Alt), cbBuf);

    if (pcszDescription)
        pcszCopy = pcszDescription;
    else
        if (usFlags & KC_VIRTUALKEY)
        {
            switch (usKeyCode)
            {
                case VK_BACKSPACE: ulID = ID_XSSI_KEY_BACKSPACE; break; // pszBackspace
                case VK_TAB: ulID = ID_XSSI_KEY_TAB; break; // pszTab
                case VK_BACKTAB: ulID = ID_XSSI_KEY_BACKTABTAB; break; // pszBacktab
                case VK_NEWLINE: ulID = ID_XSSI_KEY_ENTER; break; // pszEnter
                case VK_ESC: ulID = ID_XSSI_KEY_ESC; break; // pszEsc
                case VK_SPACE: ulID = ID_XSSI_KEY_SPACE; break; // pszSpace
                case VK_PAGEUP: ulID = ID_XSSI_KEY_PAGEUP; break; // pszPageup
                case VK_PAGEDOWN: ulID = ID_XSSI_KEY_PAGEDOWN; break; // pszPagedown
                case VK_END: ulID = ID_XSSI_KEY_END; break; // pszEnd
                case VK_HOME: ulID = ID_XSSI_KEY_HOME; break; // pszHome
                case VK_LEFT: ulID = ID_XSSI_KEY_LEFT; break; // pszLeft
                case VK_UP: ulID = ID_XSSI_KEY_UP; break; // pszUp
                case VK_RIGHT: ulID = ID_XSSI_KEY_RIGHT; break; // pszRight
                case VK_DOWN: ulID = ID_XSSI_KEY_DOWN; break; // pszDown
                case VK_PRINTSCRN: ulID = ID_XSSI_KEY_PRINTSCRN; break; // pszPrintscrn
                case VK_INSERT: ulID = ID_XSSI_KEY_INSERT; break; // pszInsert
                case VK_DELETE: ulID = ID_XSSI_KEY_DELETE; break; // pszDelete
                case VK_SCRLLOCK: ulID = ID_XSSI_KEY_SCRLLOCK; break; // pszScrlLock
                case VK_NUMLOCK: ulID = ID_XSSI_KEY_NUMLOCK; break; // pszNumLock
                case VK_ENTER: ulID = ID_XSSI_KEY_ENTER; break; // pszEnter
                case VK_F1: pcszCopy = "F1"; break;
                case VK_F2: pcszCopy = "F2"; break;
                case VK_F3: pcszCopy = "F3"; break;
                case VK_F4: pcszCopy = "F4"; break;
                case VK_F5: pcszCopy = "F5"; break;
                case VK_F6: pcszCopy = "F6"; break;
                case VK_F7: pcszCopy = "F7"; break;
                case VK_F8: pcszCopy = "F8"; break;
                case VK_F9: pcszCopy = "F9"; break;
                case VK_F10: pcszCopy = "F10"; break;
                case VK_F11: pcszCopy = "F11"; break;
                case VK_F12: pcszCopy = "F12"; break;
                case VK_F13: pcszCopy = "F13"; break;
                case VK_F14: pcszCopy = "F14"; break;
                case VK_F15: pcszCopy = "F15"; break;
                case VK_F16: pcszCopy = "F16"; break;
                case VK_F17: pcszCopy = "F17"; break;
                case VK_F18: pcszCopy = "F18"; break;
                case VK_F19: pcszCopy = "F19"; break;
                case VK_F20: pcszCopy = "F20"; break;
                case VK_F21: pcszCopy = "F21"; break;
                case VK_F22: pcszCopy = "F22"; break;
                case VK_F23: pcszCopy = "F23"; break;
                case VK_F24: pcszCopy = "F24"; break;
                default: brc = FALSE; break;
            }
        } // end if (usFlags & KC_VIRTUALKEY)
        else
        {
            switch (usKeyCode)
            {
                case 0xEC00: ulID = ID_XSSI_KEY_WINLEFT; break; // pszWinLeft
                case 0xED00: ulID = ID_XSSI_KEY_WINRIGHT; break; // pszWinRight
                case 0xEE00: ulID = ID_XSSI_KEY_WINMENU; break; // pszWinMenu
                default:
                {
                    CHAR szTemp[2];
                    if (usKeyCode >= 'a' && usKeyCode <= 'z')
                        szTemp[0] = (CHAR)usKeyCode-32;
                    else
                        szTemp[0] = (CHAR)usKeyCode;
                    szTemp[1] = '\0';
                    strcat(pszBuf, szTemp);
                }
            }
        }

    if (ulID)
        pcszCopy = cmnGetString(ulID);

    if (pcszCopy)
        strlcat(pszBuf, pcszCopy, cbBuf);

    PMPF_KEYS(("Key: %s, usKeyCode: 0x%lX, usFlags: 0x%lX", pszBuf, usKeyCode, usFlags));

    return brc;
}

/*
 *@@ cmnAddCloseMenuItem:
 *      adds a "Close" menu item to the given menu.
 *
 *@@added V0.9.7 (2000-12-21) [umoeller]
 */

VOID cmnAddCloseMenuItem(HWND hwndMenu)
{
    // add "Close" menu item
    cmnInsertSeparator(hwndMenu, MIT_END);
    winhInsertMenuItem(hwndMenu,
                       MIT_END,
                       WPMENUID_CLOSE,
                       cmnGetString(ID_XSSI_CLOSE),  // "~Close", // pszClose
                       MIS_TEXT, 0);
}

/*
 *@@ cmnInsertSeparator:
 *      adds a separator item to the given menu.
 *
 *@@added V1.0.1 (2002-12-08) [umoeller]
 */

VOID cmnInsertSeparator(HWND hwndMenu,
                        SHORT sPosition)       // in: probably MIT_END
{
    winhInsertMenuSeparator(hwndMenu,
                            sPosition,
                            *G_pulVarMenuOfs + ID_XFMI_OFS_SEPARATOR);
}

/*
 *@@ cmnQueryFCF:
 *      returns either FCF_HIDEMAX or FCF_MINMAX, depending
 *      on the object button setting of this object.
 *
 *      Useful helper when creating a custom view.
 *
 *@@added V1.0.1 (2002-12-08) [umoeller]
 */

ULONG cmnQueryFCF(WPObject *somSelf)
{
    ULONG   ulButton = _wpQueryButtonAppearance(somSelf);
    if (ulButton == DEFAULTBUTTON)
        ulButton = PrfQueryProfileInt(HINI_USER,
                                      "PM_ControlPanel",
                                      "MinButtonType",
                                      HIDEBUTTON);

    if (ulButton == HIDEBUTTON)
        return FCF_HIDEMAX;     // hide and maximize

    return FCF_MINMAX;      // minimize and maximize
}

/*
 *@@ cmnRegisterView:
 *      helper for the typical wpAddToObjUseList/wpRegisterView
 *      sequence.
 *
 *      With pUseItem, pass in a USEITEM structure which must be
 *      immediately followed by a VIEWITEM structure. The buffer
 *      pointed to by pUseItem must be valid while the view exists,
 *      so you best store this in the view's window words somewhere.
 *
 *      This function then calls wpRegisterView with the specified
 *      frame window handle and view title. Tilde chars (~) are
 *      removed from the view title so you can easily use the
 *      menu item's text.
 *
 *@@added V0.9.11 (2001-04-18) [umoeller]
 *@@changed V1.0.1 (2003-01-30) [umoeller]: optimized
 */

BOOL cmnRegisterView(WPObject *somSelf,
                     PUSEITEM pUseItem,     // in: USEITEM, immediately followed by VIEWITEM
                     ULONG ulViewID,        // in: view ID == menu item ID
                     HWND hwndFrame,        // in: frame window handle of new view (must be WC_FRAME)
                     PCSZ pcszViewTitle) // in: view title for wpRegisterView (tilde chars are removed)
{
    BOOL        brc = FALSE;
    PSZ         pszViewTitle;
    ULONG       lenViewTitle;

    if (pszViewTitle = strhdup(pcszViewTitle, &lenViewTitle))
    {
        PVIEWITEM   pViewItem = (PVIEWITEM)(((PBYTE)pUseItem) + sizeof(USEITEM));
        // add the use list item to the object's use list
        pUseItem->type    = USAGE_OPENVIEW;
        pUseItem->pNext   = NULL;
        memset(pViewItem, 0, sizeof(VIEWITEM));
        pViewItem->view   = ulViewID;
        pViewItem->handle = hwndFrame;
        if (!_wpAddToObjUseList(somSelf, pUseItem))
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "_wpAddToObjUseList failed.");
        else
        {
            // create view title: remove ~ char
            strhKillChar(pszViewTitle,
                         '~',
                         &lenViewTitle);

            brc = _wpRegisterView(somSelf,
                                  hwndFrame,
                                  pszViewTitle);
        }

        free(pszViewTitle);
    }

    return brc;
}

#ifndef __NOXSYSTEMSOUNDS__

/*
 *@@ cmnPlaySystemSound:
 *      this posts a msg to the XFolder Media thread to
 *      have it play a system sound. This does sufficient
 *      error checking and returns FALSE if playing the
 *      sound failed.
 *
 *      usIndex may be any of the MMSOUND_* values defined
 *      in helpers\syssound.h and shared\common.h.
 *
 *@@changed V0.9.3 (2000-04-10) [umoeller]: "Sounds" setting in XWPSetup wasn't respected; fixed
 */

BOOL cmnPlaySystemSound(USHORT usIndex)
{
    BOOL brc = FALSE;
    if (cmnQuerySetting(sfXSystemSounds))    // V0.9.3 (2000-04-10) [umoeller]
    {
        // check if the XWPMedia subsystem is working
        if (xmmQueryStatus() == MMSTAT_WORKING)
        {
            brc = xmmPostPartyMsg(XMM_PLAYSYSTEMSOUND,
                                  (MPARAM)usIndex,
                                  MPNULL);
        }
    }

    return brc;
}

#endif

/*
 *@@ cmnIsADesktop:
 *      returns TRUE if somSelf is a WPDesktop
 *      instance.
 *
 *@@added V0.9.14 (2001-07-28) [umoeller]
 */

BOOL cmnIsADesktop(WPObject *somSelf)
{
    return _somIsA(somSelf, _WPDesktop);
}

/*
 *@@ cmnQueryActiveDesktop:
 *      wrapper for wpclsQueryActiveDesktop. This
 *      has been implemented so that this method
 *      gets called only once (speed). Also, this
 *      saves us from including wpdesk.h in every
 *      source file.
 *
 *@@added V0.9.3 (2000-04-17) [umoeller]
 */

WPObject* cmnQueryActiveDesktop(VOID)
{
    return _wpclsQueryActiveDesktop(_WPDesktop);
}

/*
 *@@ cmnQueryActiveDesktopHWND:
 *      wrapper for wpclsQueryActiveDesktopHWND. This
 *      has been implemented so that this method
 *      gets called only once (speed). Also, this
 *      saves us from including wpdesk.h in every
 *      source file.
 *
 *@@added V0.9.3 (2000-04-17) [umoeller]
 */

HWND cmnQueryActiveDesktopHWND(VOID)
{
    return _wpclsQueryActiveDesktopHWND(_WPDesktop);
}

/*
 *@@ cmnIsObjectFromForeignDesktop:
 *      returns TRUE if somSelf is a foreign desktop
 *      or sits below a foreign desktop.
 *
 *      A desktop is considered "foreign" if it isn't
 *      the current one.
 *
 *@@added V0.9.16 (2001-12-06) [umoeller]
 */

BOOL cmnIsObjectFromForeignDesktop(WPObject *somSelf)
{
    BOOL fForeign = FALSE;
    WPObject *pCheck = somSelf;
    WPDesktop *pActiveDesktop = cmnQueryActiveDesktop();
    while (pCheck)
    {
        if (    (_somIsA(pCheck, _WPDesktop))
             && (pCheck != pActiveDesktop)
           )
        {
            // yo, this is foreign:
            fForeign = TRUE;
            break;
        }

        pCheck = _wpQueryFolder(pCheck);
    }

    return fForeign;
}

/*
 *@@ cmnQueryObjectFromID:
 *      this returns an object for an object ID
 *      (those things in angle brackets) or NULL
 *      if not found.
 *
 *@@added V0.9.20 (2002-08-04) [umoeller]
 */

WPObject* cmnQueryObjectFromID(PCSZ pcszObjectID)   // in: object ID (e.g. "<WP_DESKTOP>")
{
    ULONG       ulHandle,
                cbHandle;

    // the WPS stores all the handles as plain ULONGs
    // (four bytes)
    cbHandle = sizeof(ulHandle);
    if (PrfQueryProfileData(HINI_USER,
                            (PSZ)WPINIAPP_LOCATION, // "PM_Workplace:Location",
                            (PSZ)pcszObjectID,                  // key
                            &ulHandle,
                            &cbHandle))
        return _wpclsQueryObject(_WPObject, ulHandle);

    return NULL;
}

/* ******************************************************************
 *
 *   "Run" dialog
 *
 ********************************************************************/

static PCSZ G_apcszExtensions[]
    = {
                "EXE",
                "COM",
                "CMD",
                "BAT"
      };

/*
 *@@ StripParams:
 *      returns a new string with the executable
 *      name only.
 *
 *      Examples:
 *
 *      --  e c:\config.sys will return "e".
 *
 *      --  "my program" param will return "my program"
 *          (without quotes).
 *
 *@@added V0.9.11 (2001-04-18) [umoeller]
 */

STATIC PSZ StripParams(PSZ pcszCommand,
                       PSZ *ppParams)      // out: ptr to first char of params
{
    PSZ pszReturn = NULL;

    if (pcszCommand && strlen(pcszCommand))
    {
        // parse the command line to check if we have
        // parameters
        if (*pcszCommand == '\"')
        {
            PSZ pSecondQuote;
            if (pSecondQuote = strchr(pcszCommand + 1, '\"'))
            {
                pszReturn = strhSubstr(pcszCommand + 1, pSecondQuote);
                if (ppParams)
                    *ppParams = pSecondQuote + 1;
            }
        }
        else
        {
            // no quote first:
            // find first space --> parameters
            PSZ pSpace;
            if (pSpace = strchr(pcszCommand, ' '))
            {
                pszReturn = strhSubstr(pcszCommand, pSpace);
                if (ppParams)
                    *ppParams = pSpace + 1;
            }
        }

        if (!pszReturn)
            pszReturn = strdup(pcszCommand);
    }

    return pszReturn;
}

/*
 *@@ GetExeFromControl:
 *      returns a fully qualified executable
 *      name from the text in a control.
 *
 *@@added V0.9.14 (2001-08-23) [pr]
 */

STATIC APIRET GetExeFromControl(HWND hwnd,
                                PSZ pszExecutable,
                                USHORT usExeLength)
{
    APIRET arc = ERROR_FILE_NOT_FOUND;

    PSZ pszCommand;
    if (pszCommand = winhQueryWindowText(hwnd))
    {
        // we got a command:
        PSZ pszExec;
        if (pszExec = StripParams(pszCommand,
                                  NULL))
        {
            if (!(arc = doshFindExecutable(pszExec,
                                           pszExecutable,
                                           usExeLength,
                                           G_apcszExtensions,
                                           ARRAYITEMCOUNT(G_apcszExtensions))))
                nlsUpper(pszExecutable);

            // _PmpfF(("doshFindExecutable returned %d", arc));

            free(pszExec);
        }

        free(pszCommand);
    }

    return(arc);
}

/*
 *@@ LoadRunHistory:
 *      Loads the Run dialog's combo box with
 *      the history list from the INI file.
 *
 *@@added V0.9.14 (2001-08-23) [pr]
 */

STATIC BOOL LoadRunHistory(HWND hwnd)
{
    USHORT i;
    BOOL   bOK = FALSE;

    for (i = 0; i < RUN_MAXITEMS; i++)
    {
        CHAR szKey[32], szData[CCHMAXPATH];

        sprintf(szKey, "%s%02u", INIKEY_RUNHISTORY, i);
        if (PrfQueryProfileString(HINI_USER,
                                  (PSZ)INIAPP_XCENTER,
                                  szKey,
                                  NULL,
                                  szData,
                                  sizeof(szData)))
        {
            WinInsertLboxItem(hwnd, i, szData);
            if (i == 0)
            {
                WinSetWindowText(hwnd, szData);
                bOK = !GetExeFromControl(hwnd, szData, sizeof(szData));
            }
        }
    }

    return(bOK);
}

/*
 *@@ SaveRunHistory:
 *      Saves the Run dialog's combo box
 *      history list to the INI file.
 *
 *@@added V0.9.14 (2001-08-23) [pr]
 */

STATIC VOID SaveRunHistory(HWND hwnd)
{
    USHORT i;

    for (i = 0; i < RUN_MAXITEMS; i++)
    {
        CHAR szKey[32], szData[CCHMAXPATH];

        sprintf(szKey, "%s%02u", INIKEY_RUNHISTORY, i);
        if (WinQueryLboxItemText(hwnd, i, szData, sizeof(szData)))
            PrfWriteProfileString(HINI_USER, (PSZ) INIAPP_XCENTER, szKey, szData);
        else
            break;
    }
}

/*
 *@@ UpdateRunHistory:
 *      Updates the Run dialog's combo box
 *      history list and changes the saved
 *      directory for the Browse dialog.
 *
 *@@added V0.9.14 (2001-08-23) [pr]
 */

STATIC VOID UpdateRunHistory(HWND hwnd)
{
    CHAR szData[CCHMAXPATH];
    USHORT i, usCount;
    BOOL bFound = FALSE;
    PSZ pszExec;

    WinQueryWindowText(hwnd, sizeof(szData), szData);
    usCount = WinQueryLboxCount(hwnd);
    for (i = 0; i < usCount; i++)
    {
        CHAR szHistory[CCHMAXPATH];

        if (   WinQueryLboxItemText(hwnd, i, szHistory, sizeof(szHistory))
            && (!stricmp(szData, szHistory))
           )
        {
            bFound = TRUE;
            break;
        }
    }

    if (bFound)
        WinDeleteLboxItem(hwnd, i);
    else
        if (usCount == RUN_MAXITEMS)
            WinDeleteLboxItem(hwnd, RUN_MAXITEMS - 1);

    WinInsertLboxItem(hwnd, 0, szData);
    if (pszExec = StripParams(szData, NULL))
    {
        PSZ p;

        for (p = pszExec + strlen(pszExec); p >= pszExec; p--)
            if (*p != '\\' && *p != ':')
                *p = '\0';
            else
                break;

        strlcpy(G_szRunDirectory, pszExec, sizeof(G_szRunDirectory));
        free(pszExec);
    }
}

/*
 *@@ fnwpRunCommandLine:
 *      window proc for "run" dialog.
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 *@@changed V0.9.11 (2001-04-18) [umoeller]: fixed parameters
 *@@changed V0.9.11 (2001-04-25) [umoeller]: fixed fully qualified executables
 *@@changed V0.9.14 (2001-08-23) [pr]: added more options and Browse button
 */

STATIC MRESULT EXPENTRY fnwpRunCommandLine(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_CONTROL:
        {
            USHORT usid = SHORT1FROMMP(mp1);
            USHORT usNotifyCode = SHORT2FROMMP(mp1);
            CHAR szExecutable[CCHMAXPATH] = "";
            ULONG ulDosAppType, ulWinAppType;

            switch (usid)
            {
                case ID_XFD_RUN_COMMAND:
                    if (usNotifyCode == CBN_EFCHANGE)
                    {
                        BOOL bOK, bIsWinProg;
                        HWND hwndOK = WinWindowFromID(hwnd, DID_OK);
                        HWND hwndCancel = WinWindowFromID(hwnd, DID_CANCEL);
                        HWND hwndCommand = (HWND)mp2;

                        // Remove leading spaces
                        WinQueryWindowText(hwndCommand, sizeof(szExecutable), szExecutable);
                        if (szExecutable[0] == ' ')
                        {
                            PSZ p;

                            for (p = szExecutable; *p == ' '; p++)
                                ;

                            WinSetWindowText(hwndCommand, p);
                            WinSendMsg(hwndCommand,
                                       WM_CHAR,
                                       MPFROM2SHORT(KC_VIRTUALKEY, 0),
                                       MPFROM2SHORT(0, VK_HOME));
                        }

                        bOK = !GetExeFromControl(hwndCommand,
                                                 szExecutable,
                                                 sizeof(szExecutable));
                        bIsWinProg = (    (bOK)
                                       && (!appQueryAppType(szExecutable,
                                                            &ulDosAppType,
                                                            &ulWinAppType))
                                       && (ulWinAppType == PROG_31_ENHSEAMLESSCOMMON)
                                     );

                        WinEnableWindow(hwndOK, bOK);
                        if (!bOK)
                        {
                            HWND hwndTmp = hwndOK;
                            hwndOK = hwndCancel;
                            hwndCancel = hwndTmp;
                            // do not display the full path
                            // if the file wasn't found
                            szExecutable[0] = '\0';
                                    // V0.9.16 (2001-10-08) [umoeller]
                        }

                        WinSetWindowULong(hwnd,
                                          QWL_DEFBUTTON,
                                          hwndOK); // V0.9.15
                        WinSetWindowBits(hwndOK, QWL_STYLE, -1, WS_GROUP | BS_DEFAULT );
                        WinSetWindowBits(hwndCancel, QWL_STYLE, 0, WS_GROUP | BS_DEFAULT);
                        WinInvalidateRect(hwndOK, NULL, FALSE);
                        WinInvalidateRect(hwndCancel, NULL, FALSE);
                        WinEnableControl(hwnd,
                                          ID_XFD_RUN_WINOS2_GROUP,
                                          bIsWinProg);
                        WinEnableControl(hwnd,
                                          ID_XFD_RUN_ENHANCED,
                                          bIsWinProg);
                        WinEnableControl(hwnd,
                                          ID_XFD_RUN_SEPARATE,
                                          (   (bIsWinProg)
                                           && (!winhIsDlgItemChecked(hwnd,
                                                                     ID_XFD_RUN_FULLSCREEN))));
                        WinSetDlgItemText(hwnd,
                                          ID_XFD_RUN_FULLPATH,
                                          szExecutable);
                    }
                break;

                case ID_XFD_RUN_FULLSCREEN:
                    if (   (usNotifyCode == BN_CLICKED)
                        || (usNotifyCode == BN_DBLCLICKED)
                       )
                    {
                        BOOL bOK = GetExeFromControl(WinWindowFromID(hwnd, ID_XFD_RUN_COMMAND),
                                                     szExecutable,
                                                     sizeof(szExecutable));
                        BOOL bIsWinProg = (    (bOK)
                                            && (!appQueryAppType(szExecutable,
                                                                 &ulDosAppType,
                                                                 &ulWinAppType))
                                            && (ulWinAppType == PROG_31_ENHSEAMLESSCOMMON)
                                          );
                        WinEnableControl(hwnd,
                                          ID_XFD_RUN_SEPARATE,
                                          (    (bIsWinProg)
                                            && (!winhIsDlgItemChecked(hwnd, usid))));
                    }

                break;
            }
        }
        break;

        case WM_COMMAND:
        {
            USHORT usid = SHORT1FROMMP(mp1);

            switch(usid)
            {
                case DID_BROWSE:        // ID changed V0.9.19 (2002-04-25) [umoeller]
                {
                    FILEDLG filedlg;
                    static const APSZ typelist[] =
                        {
                            "DOS Command File",
                            "Executable",
                            "OS/2 Command File",        // V0.9.16 (2001-09-29) [umoeller]
                            NULL
                        };
                    static const PSZ pszFilespec = "*.COM;*.EXE;*.CMD;*.BAT";

                    memset(&filedlg, '\0', sizeof(filedlg));
                    filedlg.cbSize = sizeof(filedlg);
                    filedlg.fl = FDS_OPEN_DIALOG | FDS_CENTER;
                    if (   strlen(G_szRunDirectory) + strlen(pszFilespec)
                         < sizeof(filedlg.szFullFile)
                       )
                    {
                        strlcpy(filedlg.szFullFile, G_szRunDirectory, sizeof(filedlg.szFullFile));
                        strlcat(filedlg.szFullFile, pszFilespec, sizeof(filedlg.szFullFile));
                    }
                    else
                        strlcpy(filedlg.szFullFile, pszFilespec, sizeof(filedlg.szFullFile));

                    filedlg.papszITypeList = (PAPSZ)typelist;
                    if (    (WinFileDlg(HWND_DESKTOP, hwnd, &filedlg))
                         && (filedlg.lReturn == DID_OK)
                       )
                    {
                        PSZ p;

                        WinSetDlgItemText(hwnd, ID_XFD_RUN_COMMAND, filedlg.szFullFile);
                        for (p = filedlg.szFullFile + strlen(filedlg.szFullFile);
                             p >= filedlg.szFullFile;
                             p--)
                            if (*p != '\\' && *p != ':')
                                *p = '\0';
                            else
                                break;

                        strlcpy(G_szRunDirectory, filedlg.szFullFile, sizeof(G_szRunDirectory));
                    }
                }
                break;

                default:
                    mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);

                break;
            }
        }
        break;

        case WM_HELP:
            cmnDisplayHelp(NULL,
                           ID_XSH_RUN);
        break;

        default:
            mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

#define RUNDLG_WIDTH        (4 * STD_BUTTON_WIDTH + 6 * COMMON_SPACING)
#define WIN16_TABLE_WIDTH   (RUNDLG_WIDTH / 2)
#define WIN16_GROUP_WIDTH   (WIN16_TABLE_WIDTH + 2 * COMMON_SPACING + 2 * GROUP_INNER_SPACING_X)
#define LEFT_COLUMN_WIDTH   (RUNDLG_WIDTH - WIN16_GROUP_WIDTH)

static const CONTROLDEF
    RunIntro = LOADDEF_TEXT_WORDBREAK_MNEMONIC(ID_XFD_RUN_INTRO, RUNDLG_WIDTH),
    RunCommandDrop = CONTROLDEF_DROPDOWN(ID_XFD_RUN_COMMAND, RUNDLG_WIDTH, 150),
    RunFullpath = CONTROLDEF_TEXT_RIGHT("M", ID_XFD_RUN_FULLPATH, RUNDLG_WIDTH, SZL_AUTOSIZE),
    RunMinimizedCB = CONTROLDEF_AUTOCHECKBOX(LOAD_STRING, ID_XFD_RUN_MINIMIZED, LEFT_COLUMN_WIDTH, SZL_AUTOSIZE),
    RunFullscreenCB = CONTROLDEF_AUTOCHECKBOX(LOAD_STRING, ID_XFD_RUN_FULLSCREEN, LEFT_COLUMN_WIDTH, SZL_AUTOSIZE),
    RunAutoCloseCB = CONTROLDEF_AUTOCHECKBOX(LOAD_STRING, ID_XFD_RUN_AUTOCLOSE, LEFT_COLUMN_WIDTH, SZL_AUTOSIZE),
    RunWinos2Group = LOADDEF_GROUP(ID_XFD_RUN_WINOS2_GROUP, WIN16_TABLE_WIDTH),
    RunEnhancedCB = LOADDEF_AUTOCHECKBOX(ID_XFD_RUN_ENHANCED),
    RunSeparateCB = LOADDEF_AUTOCHECKBOX(ID_XFD_RUN_SEPARATE),
    RunStartupDirTxt = LOADDEF_TEXT(ID_XFD_RUN_STARTUPDIR_TXT),
    RunStartupDirEF = CONTROLDEF_ENTRYFIELD(NULL, ID_XFD_RUN_STARTUPDIR, RUNDLG_WIDTH, SZL_AUTOSIZE),
    RunBrowseButton = LOADDEF_PUSHBUTTON(DID_BROWSE);

static const DLGHITEM G_dlgRun[] =
    {
        START_TABLE,
            START_ROW(0),
                CONTROL_DEF(&RunIntro),
            START_ROW(0),
                CONTROL_DEF(&RunCommandDrop),
            START_ROW(0),
                CONTROL_DEF(&RunFullpath),
            START_ROW(ROW_VALIGN_CENTER),
                START_TABLE,
                    START_ROW(0),
                        CONTROL_DEF(&RunMinimizedCB),
                    START_ROW(0),
                        CONTROL_DEF(&RunFullscreenCB),
                    START_ROW(0),
                        CONTROL_DEF(&RunAutoCloseCB),
                END_TABLE,
                START_GROUP_TABLE(&RunWinos2Group),
                    START_ROW(0),
                        CONTROL_DEF(&RunEnhancedCB),
                    START_ROW(0),
                        CONTROL_DEF(&RunSeparateCB),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&RunStartupDirTxt),
            START_ROW(0),
                CONTROL_DEF(&RunStartupDirEF),
            START_ROW(0),
                CONTROL_DEF(&G_OKButton),
                CONTROL_DEF(&G_CancelButton),
                CONTROL_DEF(&RunBrowseButton),
                CONTROL_DEF(&G_HelpButton),
        END_TABLE
    };

/*
 *@@ cmnRunCommandLine:
 *      displays a prompt dialog in which the user can
 *      enter a command line and then runs that command
 *      line using winhStartApp.
 *
 *      Returns the HAPP that was started or NULLHANDLE,
 *      e.g. if an error occurred or the user cancelled
 *      the dialog.
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 *@@changed V0.9.11 (2001-04-18) [umoeller]: fixed parameters
 *@@changed V0.9.11 (2001-04-18) [umoeller]: fixed entry field lengths
 *@@changed V0.9.12 (2001-05-26) [umoeller]: added return value
 *@@changed V0.9.14 (2001-07-28) [umoeller]: fixed parameter handling which was ignored
 *@@changed V0.9.14 (2001-08-07) [pr]: changed dialog handling, fixed Win-OS/2 full-screen hang
 *@@changed V0.9.14 (2001-08-23) [pr]: added more options & Browse button
 *@@changed V0.9.19 (2002-04-25) [umoeller]: now using dialog formatter
 */

HAPP cmnRunCommandLine(HWND hwndOwner,              // in: owner window or NULLHANDLE for active desktop
                       PCSZ pcszStartupDir)  // in: startup dir or NULL
{
    HAPP        happ = NULLHANDLE;

    TRY_LOUD(excpt1)
    {
        static HWND hwndDlg = NULLHANDLE;

        // activate the current Run dialog if user tries to open a new one V0.9.14
        if (hwndDlg)
        {
            HWND    hwnd = hwndDlg, hwndTmp;

            // find the Browse dialog if it is open
            HENUM   henum = WinBeginEnumWindows(HWND_DESKTOP);
            while (hwndTmp = WinGetNextWindow(henum))
                if (WinQueryWindow(hwndTmp, QW_OWNER) == hwndDlg)
                {
                    hwnd = hwndTmp;
                    break;
                }
            WinEndEnumWindows(henum);

            WinSetFocus (HWND_DESKTOP, hwnd);
            return(happ);
        }

        /* V0.9.14 This is a very bad idea as it means the desktop is disabled;
        this is one of the things that causes Win-OS/2 full screen to fail
        V0.9.14 (2001-08-03) [pr]
        if (!hwndOwner)
            hwndOwner = cmnQueryActiveDesktopHWND(); */

        if (!dlghCreateDlg(&hwndDlg,
                           hwndOwner,
                           FCF_FIXED_DLG,
                           fnwpRunCommandLine,
                           cmnGetString(ID_XFD_RUN_TITLE),
                           G_dlgRun,
                           ARRAYITEMCOUNT(G_dlgRun),
                           NULL,
                           cmnQueryDefaultFont()))
        {
            HWND    hwndCommand = WinWindowFromID(hwndDlg, ID_XFD_RUN_COMMAND),
                    hwndStartup = WinWindowFromID(hwndDlg, ID_XFD_RUN_STARTUPDIR);

            winhSetDlgItemText(hwndDlg, ID_XFD_RUN_FULLPATH, "");

            winhSetEntryFieldLimit(hwndCommand, CCHMAXPATH);
            if (LoadRunHistory(hwndCommand))
            {
                HWND hwndOK = WinWindowFromID(hwndDlg, DID_OK);
                HWND hwndCancel = WinWindowFromID(hwndDlg, DID_CANCEL);

                WinEnableWindow(hwndOK, TRUE);
                WinSetWindowULong(hwndDlg, QWL_DEFBUTTON, DID_OK);
                WinSetWindowBits(hwndOK, QWL_STYLE, -1, WS_GROUP | BS_DEFAULT );
                WinSetWindowBits(hwndCancel, QWL_STYLE, 0, WS_GROUP | BS_DEFAULT);
            }

            winhSetEntryFieldLimit(hwndStartup, CCHMAXPATH);
            WinSetWindowText(hwndStartup, pcszStartupDir);

            cmnSetControlsFont(hwndDlg, 1, 10000);
            winhSetDlgItemChecked(hwndDlg, ID_XFD_RUN_AUTOCLOSE, TRUE);
            winhSetDlgItemChecked(hwndDlg, ID_XFD_RUN_ENHANCED, TRUE); // V0.9.14
            winhCenterWindow(hwndDlg);

            WinSetFocus(HWND_DESKTOP, hwndCommand);

            // go!
            if (WinProcessDlg(hwndDlg) == DID_OK)
            {
                PSZ pszCommand = winhQueryWindowText(hwndCommand);
                PSZ pszStartup = winhQueryWindowText(hwndStartup);

                if (pszCommand)
                {
                    APIRET  arc = NO_ERROR;
                    PSZ     pszExec,
                            pParams = NULL;
                    CHAR    szExecutable[CCHMAXPATH];

                    UpdateRunHistory(hwndCommand);
                    SaveRunHistory(hwndCommand);
                    if (!pszStartup)
                    {
                        pszStartup = strdup("?:\\");
                        *pszStartup = doshQueryBootDrive();
                    }

                    pszExec = StripParams(pszCommand,
                                          &pParams);
                    if (!pszExec)
                        arc = ERROR_INVALID_PARAMETER;
                    else
                    {
                        arc = doshFindExecutable(pszExec,
                                                 szExecutable,
                                                 sizeof(szExecutable),
                                                 G_apcszExtensions,
                                                 ARRAYITEMCOUNT(G_apcszExtensions));
                        free(pszExec);
                    }

                    if (arc != NO_ERROR)
                    {
                        PSZ pszError;
                        if (pszError = doshQuerySysErrorMsg(arc))
                        {
                            cmnMessageBox(hwndOwner,
                                          pszCommand,
                                          pszError,
                                          NULLHANDLE, // no help
                                          MB_CANCEL);
                            free(pszError);
                        }
                    }
                    else
                    {
                        PROGDETAILS pd;
                        ULONG   ulDosAppType, ulFlags = 0;
                        memset(&pd, 0, sizeof(pd));

                        if (!(arc = appQueryAppType(szExecutable,
                                                    &ulDosAppType,
                                                    &pd.progt.progc)))
                        {
                            pd.progt.fbVisible = SHE_VISIBLE;
                            pd.pszExecutable = szExecutable;
                            nlsUpper(szExecutable);
                            pd.pszParameters = (PSZ)pParams;
                            pd.pszStartupDir = pszStartup;

                            pd.swpInitial.hwndInsertBehind = HWND_TOP; // V0.9.14
                            if (winhIsDlgItemChecked(hwndDlg, ID_XFD_RUN_MINIMIZED))
                                pd.swpInitial.fl = SWP_MINIMIZE;
                            else
                                pd.swpInitial.fl = SWP_ACTIVATE; // V0.9.14

                            if (!winhIsDlgItemChecked(hwndDlg, ID_XFD_RUN_AUTOCLOSE))
                                pd.swpInitial.fl |= SWP_NOAUTOCLOSE; // V0.9.14

                            if (winhIsDlgItemChecked(hwndDlg, ID_XFD_RUN_FULLSCREEN))
                                ulFlags |= APP_RUN_FULLSCREEN;

                            if (winhIsDlgItemChecked(hwndDlg, ID_XFD_RUN_ENHANCED))
                                ulFlags |= APP_RUN_ENHANCED;
                            else
                                ulFlags |= APP_RUN_STANDARD;

                            if (winhIsDlgItemChecked(hwndDlg, ID_XFD_RUN_SEPARATE))
                                ulFlags |= APP_RUN_SEPARATE;

                            arc = appStartApp(NULLHANDLE,        // no notify
                                              &pd,
                                              ulFlags, //V0.9.14
                                              &happ,
                                              0,
                                              NULL);
                        }
                    }
                }

                if (pszCommand)
                    free(pszCommand);
                if (pszStartup)
                    free(pszStartup);
            }

            winhDestroyWindow(&hwndDlg);
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    return happ;      // V0.9.12 (2001-05-26) [umoeller]
}

/*
 *@@ cmnQueryDefaultFont:
 *      this returns the font to be used for dialogs.
 *      If the "Use 8.Helv" checkbox is enabled on
 *      the "Paranoia" page, we return "8.Helv",
 *      otherwise "9.WarpSans". The returned font
 *      string is static, so don't attempt to free it.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V1.0.4 (2005-08-29) [bvl]: Return 'Combined' fonts on DBCS systems to show DBCS characters properly @@fixes 655
 */

PCSZ cmnQueryDefaultFont(VOID)
{
#ifndef __NOPARANOIA__
    if (cmnQuerySetting(sfUse8HelvFont))
        if (nlsDBCS())
            return "8.Helv Combined";
        else
            return "8.Helv";
    else
#endif
        if (nlsDBCS())
            return "9.WarpSans Combined";
        else
            return "9.WarpSans";
}

/*
 *@@ cmnSetControlsFont:
 *      this sets the font presentation parameters for a dialog
 *      window. See winhSetControlsFont for the parameters.
 *      This calls cmnQueryDefaultFont in turn.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.7 (2000-12-13) [umoeller]: removed krnLock(), which wasn't needed here
 */

VOID cmnSetControlsFont(HWND hwnd,
                        SHORT usIDMin,
                        SHORT usIDMax)
{
    winhSetControlsFont(hwnd,
                        usIDMin,
                        usIDMax,
                        (PSZ)cmnQueryDefaultFont());
}

/*
 *@@ cmnQueryDlgIcon:
 *      returns the handle of the XWP dialog icon,
 *      which is used in message boxes and such.
 *      This is loaded once and used forevermore.
 *
 *@@added V0.9.16 (2001-11-10) [umoeller]
 */

HPOINTER cmnQueryDlgIcon(VOID)
{
    if (!G_hptrDlgIcon)
        G_hptrDlgIcon = WinLoadPointer(HWND_DESKTOP,
                                       cmnQueryMainResModuleHandle(),
                                       ID_ICONDLG);
    return G_hptrDlgIcon;
}

ULONG   G_MsgBoxHelpPanel = NULLHANDLE;

/*
 *@@ fnHelpCallback:
 *      help callback specified with
 *
 *@@added V0.9.19 (2002-04-24) [umoeller]
 */

VOID APIENTRY fnHelpCallback(HWND hwndDlg)
{
    if (G_MsgBoxHelpPanel)
        cmnDisplayHelp(NULL, G_MsgBoxHelpPanel);
}

/*
 *@@ cmnMessageBox:
 *      this is the generic function for displaying XFolder
 *      message boxes. This is very similar to WinMessageBox,
 *      but looks a lot better, especially since IBM chose
 *      to make message boxes so small with FP13. In
 *      addition, an XFolder icon is displayed.
 *
 *      Currently the following flStyle's are supported:
 *
 *      -- MB_OK                      0x0000
 *      -- MB_OKCANCEL                0x0001
 *      -- MB_RETRYCANCEL             0x0002
 *      -- MB_ABORTRETRYIGNORE        0x0003
 *      -- MB_YESNO                   0x0004
 *      -- MB_YESNOCANCEL             0x0005
 *      -- MB_CANCEL                  0x0006
 *      -- MB_ENTER                   0x0007 (not implemented yet)
 *      -- MB_ENTERCANCEL             0x0008 (not implemented yet)
 *
 *      -- MB_YES_YES2ALL_NO          0x0009
 *          This is new: this has three buttons called "Yes"
 *          (MBID_YES), "Yes to all" (MBID_YES2ALL), "No" (MBID_NO).
 *
 *      -- MB_DEFBUTTON2            (for two-button styles)
 *      -- MB_DEFBUTTON3            (for three-button styles)
 *
 *      -- MB_ICONHAND
 *      -- MB_ICONEXCLAMATION
 *
 *      Returns MBID_* codes like WinMessageBox.
 *
 *@@changed V0.9.0 [umoeller]: added support for MB_YESNOCANCEL
 *@@changed V0.9.0 [umoeller]: fixed default button bugs
 *@@changed V0.9.0 [umoeller]: added WinAlarm sound support
 *@@changed V0.9.3 (2000-05-05) [umoeller]: extracted cmnLoadMessageBoxDlg
 *@@changed V0.9.13 (2001-06-23) [umoeller]: completely rewritten, now using dlghMessageBox
 *@@changed V0.9.19 (2002-04-24) [umoeller]: added help panel
 */

ULONG cmnMessageBox(HWND hwndOwner,     // in: owner
                    PCSZ pcszTitle,     // in: msgbox title
                    PCSZ pcszMessage,   // in: msgbox text
                    ULONG ulHelpPanel,  // in: help panel or null
                    ULONG flStyle)      // in: MB_* flags
{
    ULONG   ulrc = DID_CANCEL;

    // set our extended exception handler
    TRY_LOUD(excpt1)
    {
        // no, this must not be static, or NLS changes
        // won't be picked up V0.9.19 (2002-04-17) [umoeller]
        /* static */ MSGBOXSTRINGS Strings;

        // first call: load all the strings
        Strings.pcszYes = cmnGetString(DID_YES);
        Strings.pcszNo = cmnGetString(DID_NO);
        Strings.pcszOK = cmnGetString(DID_OK);
        Strings.pcszCancel = cmnGetString(DID_CANCEL);
        Strings.pcszAbort = cmnGetString(DID_ABORT);
        Strings.pcszRetry = cmnGetString(DID_RETRY);
        Strings.pcszIgnore = cmnGetString(DID_IGNORE);
        Strings.pcszEnter = "Enter"; // never used anyway
        Strings.pcszYesToAll = cmnGetString(DID_YES2ALL);
        Strings.pcszHelp = cmnGetString(DID_HELP);      // V0.9.19 (2002-04-24) [umoeller]

        if (ulHelpPanel)
            G_MsgBoxHelpPanel = ulHelpPanel;

        // now using new dynamic dialog routines
        // V0.9.13 (2001-06-23) [umoeller]
        ulrc = dlghMessageBox(hwndOwner,
                              cmnQueryDlgIcon(),
                              pcszTitle,
                              pcszMessage,
                              (ulHelpPanel)
                                ? fnHelpCallback
                                : NULL,
                              flStyle,
                              cmnQueryDefaultFont(),
                              &Strings);

        if (ulHelpPanel)
            G_MsgBoxHelpPanel = NULLHANDLE;

    }
    CATCH(excpt1) { } END_CATCH();

    return ulrc;
}

/*
 *@@ cmnGetMessageExt:
 *      retrieves a message string from the XWorkplace
 *      TMF message file. The message is specified
 *      using the TMF message ID string directly.
 *      This gets called from cmnGetMessage.
 *
 *      The XSTRING is assumed to be initialized.
 *
 *@@added V0.9.4 (2000-06-17) [umoeller]
 *@@changed V0.9.16 (2001-10-08) [umoeller]: now using XSTRING
 */

APIRET cmnGetMessageExt(PCSZ *pTable,     // in: replacement PSZ table or NULL
                        ULONG ulTable,     // in: size of that table or 0
                        PXSTRING pstr,     // in/out: string
                        PCSZ pcszMsgID)    // in: msg ID to retrieve
{
    APIRET  arc = NO_ERROR;
    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        PMPF_LANGCODES(("%s", pcszMsgID));

        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            if (!G_pXWPMsgFile)
            {
                // first call:
                // go load the XWP message file
                arc = tmfOpenMessageFile(cmnQueryMessageFile(),
                                         &G_pXWPMsgFile);
            }

            if (!arc)
            {
                arc = tmfGetMessage(G_pXWPMsgFile,
                                    pcszMsgID,
                                    pstr,
                                    pTable,
                                    ulTable);

                PMPF_LANGCODES(("tmfGetMessage rc: %d", arc));

                if (!arc)
                    nlsReplaceEntities(pstr);
                else
                {
                    CHAR sz[500];
                    sprintf(sz,
                            "Message %s not found in %s, rc = %d",
                            pcszMsgID,
                            cmnQueryMessageFile(),
                            arc);
                    xstrcpy(pstr, sz, 0);
                }
            }
        }
    }
    CATCH(excpt1) { } END_CATCH();

    if (fLocked)
        krnUnlock();

    return arc;
}

/*
 *@@ cmnGetMessage:
 *      like DosGetMessage, but automatically uses the
 *      (NLS) XFolder message file.
 *      The parameters are exactly like with DosGetMessage.
 *      The message code (ulMsgNumber) is automatically
 *      converted to a TMF message ID.
 *
 *      The XSTRING is assumed to be initialized.
 *
 *      <B>Returns:</B> the error code of tmfGetMessage.
 *
 *@@changed V0.9.0 [umoeller]: changed, this now uses the TMF file format (tmsgfile.c).
 *@@changed V0.9.4 (2000-06-18) [umoeller]: extracted cmnGetMessageExt
 *@@changed V0.9.16 (2001-10-08) [umoeller]: now using XSTRING
 */

APIRET cmnGetMessage(PCSZ *pTable,     // in: replacement PSZ table or NULL
                     ULONG ulTable,     // in: size of that table or 0
                     PXSTRING pstr,     // in/out: string
                     ULONG ulMsgNumber) // in: msg number to retrieve
{
    CHAR szMessageName[40];
    // create string message identifier from ulMsgNumber
    sprintf(szMessageName, "XFL%04d", ulMsgNumber);

    return cmnGetMessageExt(pTable, ulTable, pstr, szMessageName);
}

/*
 *@@ cmnMessageBoxHelp:
 *      like cmnMessageBoxExt, but with string substitution
 *      (see cmnGetMessage for more); substitution only
 *      takes place for the message specified with ulMessage,
 *      not for the title.
 *
 */

ULONG cmnMessageBoxHelp(HWND hwndOwner,   // in: owner window
                        ULONG ulTitle,    // in: msg number for title
                        PCSZ *pTable,     // in: replacement table for ulMessage
                        ULONG ulTable,    // in: array count in *pTable
                        ULONG ulMessage,  // in: msg number for message
                        ULONG ulHelpPanel,  // in: help panel or NULLHANDLE
                        ULONG flStyle)    // in: msg box style flags (cmnMessageBox)
{
    ULONG ulrc;

    XSTRING strTitle, strMessage;
    xstrInit(&strTitle, 0);
    xstrInit(&strMessage, 0);

    cmnGetMessage(NULL, 0,
                  &strTitle,
                  ulTitle);
    cmnGetMessage(pTable, ulTable,
                  &strMessage,
                  ulMessage);

    ulrc = cmnMessageBox(hwndOwner,
                         strTitle.psz,
                         strMessage.psz,
                         ulHelpPanel,
                         flStyle);

    xstrClear(&strTitle);
    xstrClear(&strMessage);

    return ulrc;
}

/*
 *@@ cmnMessageBoxExt:
 *      like cmnMessageBoxExt, but with string substitution
 *      (see cmnGetMessage for more); substitution only
 *      takes place for the message specified with ulMessage,
 *      not for the title.
 *
 *      Now calls cmnMessageBoxHelp in turn.
 *
 *@@added V0.9.19 (2002-04-24) [umoeller]
 */

ULONG cmnMessageBoxExt(HWND hwndOwner,   // in: owner window
                       ULONG ulTitle,    // in: msg number for title
                       PCSZ *pTable,     // in: replacement table for ulMessage
                       ULONG ulTable,    // in: array count in *pTable
                       ULONG ulMessage,  // in: msg number for message
                       ULONG flStyle)    // in: msg box style flags (cmnMessageBox)
{
    return cmnMessageBoxHelp(hwndOwner,
                             ulTitle,
                             pTable,
                             ulTable,
                             ulMessage,
                             NULLHANDLE,     // no help
                             flStyle);
}

/*
 *@@ cmnDescribeError:
 *      attempts to find an error description for the
 *      various error codes used by OS/2, the XWorkplace
 *      helpers, and XWorkplace itself.
 *
 *@@added V0.9.19 (2002-03-28) [umoeller]
 *@@changed V0.9.19 (2002-06-13) [umoeller]: added ERROR_PROTECTION_VIOLATION
 *@@changed V0.9.20 (2002-07-16) [umoeller]: optimized
 */

VOID cmnDescribeError(PXSTRING pstr,        // in/out: string buffer (must be init'ed)
                      APIRET arc,           // in: error code
                      PSZ pszReplString,    // in: string for %1 message or NULL
                      BOOL fShowExplanation) // in: if TRUE, we'll retrieve an explanation as with the HELP command
{
    PCSZ pcszErrorClass = NULL,
         pcszErrorDescription = "[no description available]";

    XSTRING str2;
    xstrInit(&str2, 0);

    #define IS_IN_RANGE(a, b, c) (((a) >= (b)) && ((a) <= (c)))

    if (IS_IN_RANGE(arc, ERROR_XML_FIRST, ERROR_XML_LAST))
    {
        PCSZ p;
        pcszErrorClass = "XML error";
        if (p = xmlDescribeError(arc))
            pcszErrorDescription = p;
    }
    else if (IS_IN_RANGE(arc, ERROR_WPH_FIRST, ERROR_WPH_LAST))
    {
        pcszErrorClass = "Handles engine error";

        switch (arc)
        {
            case ERROR_WPH_NO_BASECLASS_DATA:
                pcszErrorDescription = "Cannot find WPS base class data in OS2.INI, PM_Workplace:BaseClass";
            break;

            case ERROR_WPH_NO_ACTIVEHANDLES_DATA:
                pcszErrorDescription = "Cannot find active handles in OS2SYS.INI, PM_Workplace:ActiveHandles";
            break;

            case ERROR_WPH_INCOMPLETE_BASECLASS_DATA:
                pcszErrorDescription = "Incomplete baseclass data in OS2SYS.INI, PM_Workplace:ActiveHandles";
            break;

            case ERROR_WPH_NO_HANDLES_DATA:
                pcszErrorDescription = "No handle blocks in OS2SYS.INI";
            break;

            case ERROR_WPH_CORRUPT_HANDLES_DATA:
                pcszErrorDescription = "Corrupt handles data, format not understood";
            break;

                // cannot determine format (invalid keywords)
            case ERROR_WPH_INVALID_PARENT_HANDLE:
                pcszErrorDescription = "Invalid parent handle found";
            break;

            case ERROR_WPH_CANNOT_FIND_HANDLE:
                pcszErrorDescription = "Cannot find handle";
            break;

            case ERROR_WPH_DRIV_TREEINSERT_FAILED:
                pcszErrorDescription = "treeInsert failed for DRIV node (probably duplicate)";
            break;

            case ERROR_WPH_NODE_TREEINSERT_FAILED:
                pcszErrorDescription = "treeInsert failed for NODE node (probably duplicate)";
            break;

            case ERROR_WPH_NODE_BEFORE_DRIV:
                pcszErrorDescription = "Corrupt handles data (NODE node before DRIV node)";
            break;

            case ERROR_WPH_NO_MATCHING_DRIVE_BLOCK:
                pcszErrorDescription = "Cannot find matching drive block";
            break;

            case ERROR_WPH_NO_MATCHING_ROOT_DIR:
                pcszErrorDescription = "Cannot find matching root directory";
            break;

            case ERROR_WPH_NOT_FILESYSTEM_HANDLE:
                pcszErrorDescription = "Given handle is not a file-system handle";
            break;

            case ERROR_WPH_PRFQUERYPROFILESIZE_BLOCK:
                pcszErrorDescription = "PrfQueryProfileSize failed on handles data block from OS2SYS.INI";
            break;

            case ERROR_WPH_PRFQUERYPROFILEDATA_BLOCK:
                pcszErrorDescription = "PrfQueryProfileData failed on handles data block from OS2SYS.INI";
            break;

        }
    }
    else if (IS_IN_RANGE(arc, ERROR_PRF_FIRST, ERROR_PRF_LAST))
    {
        pcszErrorClass = "Profile engine error";
    }
    else if (IS_IN_RANGE(arc, ERROR_DLG_FIRST, ERROR_DLG_LAST))
    {
        pcszErrorClass = "Dialog engine error";
    }
    else if (IS_IN_RANGE(arc, ERROR_REGEXP_FIRST, ERROR_REGEXP_LAST))
    {
        pcszErrorClass = "Regular expression error";

        switch (arc)
        {
            case EREE_UNF_SUB:
                pcszErrorDescription = "Unfinished sub-expression; invalid \"()\" nesting";
            break;

            case EREE_UNEX_RANGE:
                pcszErrorDescription = "Unexpected range specifier '-'";
            break;

            case EREE_UNF_RANGE:
                pcszErrorDescription = "Unfinished range specification; invalid \"[]\" nesting";
            break;

            case EREE_UNF_CCLASS:
                pcszErrorDescription = "Unfinished character class; must be \"[:class:]\"";
            break;

            case EREE_UNEX_RSQR:
                pcszErrorDescription = "Unexpected ']'";
            break;

            case EREE_UNEX_RPAR:
                pcszErrorDescription = "Unexpected ')'";
            break;

            case EREE_UNEX_QUERY:
                pcszErrorDescription = "Unexpected '?'";
            break;

            case EREE_UNEX_PLUS:
                pcszErrorDescription = "Unexpected '+'";
            break;

            case EREE_UNEX_STAR:
                pcszErrorDescription = "Unexpected '*'";
            break;

            case EREE_UNEX_LCUR:
                pcszErrorDescription = "Unexpected '{'";
            break;

            case EREE_UNEX_RCUR:
                pcszErrorDescription = "Unexpected '}'";
            break;

            case EREE_BAD_CREP_M:
                pcszErrorDescription = "Bad minimum in counted repetition";
            break;

            case EREE_BAD_CREP_N:
                pcszErrorDescription = "Bad maximum in counted repetition";
            break;

            case EREE_UNF_CREP:
                pcszErrorDescription = "Unfinished counted repetition";
            break;

            case EREE_BAD_CREP:
                pcszErrorDescription = "Bad counted repetition";
            break;

            case EREE_TOO_MANY_SUB:
                pcszErrorDescription = "Too many sub-expressions";
            break;

            case EREE_COMPILE_FSM:
                pcszErrorDescription = "Regular expression is too complex to process";
            break;

            case EREE_POSIX_COLLATING:
                pcszErrorDescription = "POSIX collating symbols not supported";
            break;

            case EREE_POSIX_EQUIVALENCE:
                pcszErrorDescription = "POSIX equivalence classes not supported";
            break;

            case EREE_POSIX_CCLASS_BAD:
                pcszErrorDescription = "Bad POSIX character class";
            break;

            case EREE_BAD_BACKSLASH:
                pcszErrorDescription = "Bad '\\' in substitution string";
            break;

            case EREE_BAD_BACKREF:
                pcszErrorDescription = "Bad backreference in substitution string";
            break;

            case EREE_SUBS_LEN:
                pcszErrorDescription = "Substituted string is too long";
            break;
        }
    }
    else if (IS_IN_RANGE(arc, ERROR_XWP_FIRST, ERROR_XWP_LAST))
    {
        #ifndef __XWPLITE__
            pcszErrorClass = "XWorkplace error";
        #else
            pcszErrorClass = "eComStation Workplace Shell error";
        #endif

        switch (arc)
        {
            case FOPSERR_NOT_HANDLED_ABORT:
                pcszErrorDescription = "Unhandled file operations error";
            break;

            case FOPSERR_INVALID_OBJECT:
                pcszErrorDescription = "Invalid object in file operation";
            break;

            case FOPSERR_NO_OBJECTS_FOUND:
                pcszErrorDescription = "No objects found for file operation";
            break;

                    // no objects found to process
            case FOPSERR_INTEGRITY_ABORT:
                pcszErrorDescription = "Data integrity error with file operation";
            break;

            case FOPSERR_FILE_THREAD_CRASHED:
                pcszErrorDescription = "File thread crashed";
            break;

                    // fopsFileThreadProcessing crashed
            case FOPSERR_CANCELLEDBYUSER:
                pcszErrorDescription = "File operation cancelled by user";
            break;

            case FOPSERR_NO_TRASHCAN:
                pcszErrorDescription = "Cannot find trash can";
            break;

                    // trash can doesn't exist, cannot delete
                    // V0.9.16 (2001-11-10) [umoeller]
            /* case FOPSERR_MOVE2TRASH_READONLY:
                pcszErrorDescription = "Cannot move read-only file to trash can";
            break; */
                    // moving WPFileSystem which has read-only:
                    // this should prompt the user

            case FOPSERR_MOVE2TRASH_NOT_DELETABLE:
                pcszErrorDescription = "Cannot move undeletable file to trash can";
            break;
                    // moving non-deletable to trash can: this should abort

            /* case FOPSERR_DELETE_CONFIRM_FOLDER:
                pcszErrorDescription = "FOPSERR_DELETE_CONFIRM_FOLDER";
            break; */
                    // deleting WPFolder and "delete folder" confirmation is on:
                    // this should prompt the user (non-fatal)
                    // V0.9.16 (2001-12-06) [umoeller]

            /* case FOPSERR_DELETE_READONLY:
                pcszErrorDescription = "FOPSERR_DELETE_READONLY";
            break; */
                    // deleting WPFileSystem which has read-only flag;
                    // this should prompt the user (non-fatal)

            case FOPSERR_DELETE_NOT_DELETABLE:
                pcszErrorDescription = "File is not deletable";
            break;
                    // deleting not-deletable; this should abort

            case FOPSERR_TRASHDRIVENOTSUPPORTED:
                pcszErrorDescription = "Drive is not supported by trash can";
            break;

            case FOPSERR_WPFREE_FAILED:
                pcszErrorDescription = "wpFree failed on file-system object";
            break;

            case FOPSERR_LOCK_FAILED:
                pcszErrorDescription = "Cannot get file operations lock";
            break;
                    // requesting object mutex failed

            case FOPSERR_START_FAILED:
                pcszErrorDescription = "Cannot start file task";
            break;
                    // fopsStartTask failed

            case FOPSERR_POPULATE_FOLDERS_ONLY:
                pcszErrorDescription = "FOPSERR_POPULATE_FOLDERS_ONLY";
            break;
                    // fopsAddObjectToTask works on folders only with XFT_POPULATE

            case FOPSERR_POPULATE_FAILED:
                pcszErrorDescription = "Cannot populate folder";
            break;
                    // wpPopulate failed on folder during XFT_POPULATE

            case FOPSERR_WPQUERYFILENAME_FAILED:
                pcszErrorDescription = "wpQueryFilename failed";
            break;
                    // wpQueryFilename failed

            case FOPSERR_WPSETATTR_FAILED:
                pcszErrorDescription = "Unable to set new attributes for file";
            break;
                    // wpSetAttr failed

            case FOPSERR_GETNOTIFYSEM_FAILED:
                pcszErrorDescription = "Cannot get notify semaphore";
            break;
                    // fdrGetNotifySem failed

            case FOPSERR_REQUESTFOLDERMUTEX_FAILED:
                pcszErrorDescription = "Cannot get folder semaphore";
            break;
                    // wpshRequestFolderSem failed

            case FOPSERR_NOT_FONT_FILE:
                pcszErrorDescription = "Given object is not a font file";
            break;
                    // with XFT_INSTALLFONTS: non-XWPFontFile passed

            case FOPSERR_FONT_ALREADY_INSTALLED:
                pcszErrorDescription = "Font is already installed";
            break;
                    // with XFT_INSTALLFONTS: XWPFontFile is already installed

            case FOPSERR_NOT_FONT_OBJECT:
                pcszErrorDescription = "FOPSERR_NOT_FONT_OBJECT";
            break;
                    // with XFT_DEINSTALLFONTS: non-XWPFontObject passed

            case FOPSERR_FONT_ALREADY_DELETED:
                pcszErrorDescription = "Font is no longer present in OS2.INI";
            break;
                    // with XFT_DEINSTALLFONTS: font no longer present in OS2.INI.

            case FOPSERR_FONT_STILL_IN_USE:
                pcszErrorDescription = "Font is still in use";
            break;
                    // with XFT_DEINSTALLFONTS: font is still in use;
                    // this is only a warning, it will be gone after a reboot

            case ERROR_XCENTER_FIRST:
                pcszErrorDescription = "ERROR_XCENTER_FIRST";
            break;

            case XCERR_INVALID_ROOT_WIDGET_INDEX:
                pcszErrorDescription = "XCERR_INVALID_ROOT_WIDGET_INDEX";
            break;

            case XCERR_ROOT_WIDGET_INDEX_IS_NO_TRAY:
                pcszErrorDescription = "XCERR_ROOT_WIDGET_INDEX_IS_NO_TRAY";
            break;

            case XCERR_INVALID_TRAY_INDEX:
                pcszErrorDescription = "XCERR_INVALID_TRAY_INDEX";
            break;

            case XCERR_INVALID_SUBWIDGET_INDEX:
                pcszErrorDescription = "XCERR_INVALID_SUBWIDGET_INDEX";
            break;

            case BASEERR_BUILDPTR_FAILED:
                pcszErrorDescription = "BASEERR_BUILDPTR_FAILED";
            break;

            case BASEERR_DAEMON_DEAD:
                pcszErrorDescription = "BASEERR_DAEMON_DEAD";
            break;
        }
    }
    else if (IS_IN_RANGE(arc, ERROR_XWPSEC_FIRST, ERROR_XWPSEC_LAST))
    {
        pcszErrorClass = XWORKPLACE_STRING " security error";

        switch (arc)
        {
            case XWPSEC_INTEGRITY:
                pcszErrorDescription = "Data integrity error";
            break;

            case XWPSEC_INVALID_DATA:
                pcszErrorDescription = "Invalid data.";
            break;

            case XWPSEC_CANNOT_GET_MUTEX:
                pcszErrorDescription = "Cannot get mutex.";
            break;

            case XWPSEC_CANNOT_START_DAEMON:
                pcszErrorDescription = "Cannot start daemon.";
            break;

            case XWPSEC_INSUFFICIENT_AUTHORITY:
                pcszErrorDescription = "Insufficient authority for processing this request.";
            break;

            case XWPSEC_HSUBJECT_EXISTS:
                pcszErrorDescription = "Subject handle is already in use.";
            break;

            case XWPSEC_INVALID_HSUBJECT:
                pcszErrorDescription = "Subject handle is invalid.";
            break;

            case XWPSEC_INVALID_PID:
                pcszErrorDescription = "Invalid process ID.";
            break;

            case XWPSEC_NO_CONTEXTS:
                pcszErrorDescription = "No contexts.";
            break;

            case XWPSEC_USER_EXISTS:
                pcszErrorDescription = "User exists already.";
            break;

            case XWPSEC_NO_USERS:
                pcszErrorDescription = "No users in user database.";
            break;

            case XWPSEC_NO_GROUPS:
                pcszErrorDescription = "No groups in user database.";
            break;

            case XWPSEC_INVALID_ID:
                pcszErrorDescription = "Invalid user or group ID.";
            break;

            case XWPSEC_NOT_AUTHENTICATED:
                pcszErrorDescription = "Authentication failed.";
            break;

            case XWPSEC_NO_USER_PROFILE:
                pcszErrorDescription = "User profile (OS2.INI) could not be found.";
            break;

            case XWPSEC_CANNOT_START_SHELL:
                pcszErrorDescription = "Cannot start shell.";
            break;

            case XWPSEC_INVALID_PROFILE:
                pcszErrorDescription = "Invalid profile.";
            break;

            case XWPSEC_NO_LOGON:
                pcszErrorDescription = "User is not logged on.";
            break;

            case XWPSEC_DB_GROUP_SYNTAX:
                pcszErrorDescription = "Syntax error with group entries in user database.";
            break;

            case XWPSEC_DB_USER_SYNTAX:
                pcszErrorDescription = "Syntax error with user entries in user database.";
            break;

            case XWPSEC_DB_INVALID_GROUPID:
                pcszErrorDescription = "Invalid group ID in user database.";
            break;

            case XWPSEC_DB_ACL_SYNTAX:
                pcszErrorDescription = "Syntax error in ACL database.";
            break;

            case XWPSEC_RING0_NOT_FOUND:
                pcszErrorDescription = "Error contacting ring-0 device driver.";
            break;

            case XWPSEC_QUEUE_INVALID_CMD:
                pcszErrorDescription = "Invalid command code in security queue.";
            break;
        }
    }
    else
    {
        // probably OS/2 error:
        APIRET arc2;

        // get error message for APIRET
        PSZ     pszTable = (pszReplString) ? pszReplString : "?";

        CHAR    szMsgBuf[1000];
        ULONG   ulLen = 0;

        // there are a few messages where OS/2 doesn't provide errors
        // so check these first
        // V0.9.19 (2002-06-12) [umoeller]

        switch (arc)
        {
            case ERROR_PROTECTION_VIOLATION:
                cmnGetMessage(NULL, 0,
                              pstr,
                              237);
            break;

            default:
                if (!(arc2 = DosGetMessage(&pszTable, 1,
                                           szMsgBuf, sizeof(szMsgBuf),
                                           arc,
                                           "OSO001.MSG",        // default OS/2 message file
                                           &ulLen)))
                {
                    szMsgBuf[ulLen] = 0;
                    xstrcpy(pstr, szMsgBuf, 0);

                    if (fShowExplanation)
                    {
                        // get help too
                        if (!(arc2 = DosGetMessage(&pszTable, 1,
                                                   szMsgBuf, sizeof(szMsgBuf),
                                                   arc,
                                                   "OSO001H.MSG",        // default OS/2 help message file
                                                   &ulLen)))
                        {
                            szMsgBuf[ulLen] = 0;
                            xstrcatc(pstr, '\n');
                            xstrcat(pstr, szMsgBuf, 0);
                        }
                    }
                }
                else
                {
                    // cannot find msg:
                    CHAR szError3[20];
                    PCSZ apsz = szError3;
                    sprintf(szError3, "%d", arc);
                    cmnGetMessage(&apsz,
                                  1,
                                  pstr,
                                  219);          // "error %d occurred"
                }
            break;

        }
    }

    if (pcszErrorClass)
        xstrPrintf(pstr,
                   "%s (%d): %s",
                   pcszErrorClass,
                   arc,
                   pcszErrorDescription);

    xstrClear(&str2);
}

/*
 *@@ cmnErrorMsgBox:
 *      displays a message box with an error description
 *      for the given error code (as returned by
 *      cmnDescribeError).
 *
 *      This expects a TMF message number whose %1 placeholder
 *      will be replaced with the error description.
 *
 *      Returns the return value from cmnMessageBoxExt.
 *
 *@@added V0.9.19 (2002-04-17) [umoeller]
 */

ULONG cmnErrorMsgBox(HWND hwndOwner,        // in: owner window
                     APIRET arc,            // in: DOS or XWP error code to get msg for
                     ULONG ulMsg,           // in: TMF msg no. where %1 will be replaced with error msg
                     ULONG flFlags,         // in: as in cmnMessageBox flStyle
                     BOOL fShowExplanation) // in: as in cmnDescribeError
{
    ULONG ulrc;
    XSTRING str;
    xstrInit(&str, 0);
    cmnDescribeError(&str, arc, NULL, fShowExplanation);

    if (ulMsg)
    {
        PCSZ pcsz = str.psz;
        ulrc = cmnMessageBoxExt(hwndOwner,
                                104,
                                &pcsz,
                                1,
                                ulMsg,
                                flFlags);
    }
    else
        ulrc = cmnMessageBox(hwndOwner,
                             XWORKPLACE_STRING,
                             str.psz,
                             NULLHANDLE, // no help
                             flFlags);

    xstrClear(&str);

    return ulrc;
}

/*
 *@@ cmnDosErrorMsgBox:
 *      displays an extended message box with an error
 *      description for the given error code (as returned
 *      by cmnDescribeError).
 *
 *      As opposed to cmnErrorMsgBox, this allows you to
 *      specify strings as C strings.
 *
 *      Returns the return value from cmnMessageBoxExt.
 *
 *@@added V0.9.1 (2000-02-08) [umoeller]
 *@@changed V0.9.3 (2000-04-09) [umoeller]: added error explanation
 *@@changed V0.9.13 (2001-06-14) [umoeller]: reduced stack consumption
 *@@changed V0.9.16 (2001-12-08) [umoeller]: added pcszPrefix/Suffix
 *@@changed V0.9.16 (2001-12-18) [umoeller]: fixed bad owner window
 *@@changed V0.9.19 (2002-03-28) [umoeller]: now using cmnDescribeError
 */

ULONG cmnDosErrorMsgBox(HWND hwndOwner,     // in: owner window
                        PSZ pszReplString,  // in: string for %1 message or NULL
                        PCSZ pcszTitle,     // in: msgbox title
                        PCSZ pcszPrefix,    // in: string before error or NULL
                        APIRET arc,         // in: DOS or XWP error code to get msg for
                        PCSZ pcszSuffix,    // in: string after error or NULL
                        ULONG ulFlags,      // in: as in cmnMessageBox flStyle
                        BOOL fShowExplanation) // in: as in cmnDescribeError
{
    ULONG   mbrc = 0;
    XSTRING strError,
            str2;
    APIRET  arc2 = NO_ERROR;

    xstrInit(&strError, 0);
    xstrInit(&str2, 0);

    if (pcszPrefix)
    {
        xstrcpy(&strError, pcszPrefix, 0);
        xstrcat(&strError, "\n\n", 0);
    }

    cmnDescribeError(&str2, arc, pszReplString, fShowExplanation);
    xstrcats(&strError, &str2);

    if (pcszSuffix)
    {
        xstrcatc(&strError, '\n');
        xstrcat(&strError, pcszSuffix, 0);
    }

    mbrc = cmnMessageBox(hwndOwner,     // fixed V0.9.16 (2001-12-18) [umoeller]
                         pcszTitle,
                         strError.psz,
                         NULLHANDLE, // no help
                         ulFlags);
    xstrClear(&strError);
    xstrClear(&str2);

    return mbrc;
}

/*
 *@@ cmnProgramErrorMsgBox:
 *      reports an error from a program startup and
 *      prompts the user whether to open the settings
 *      notebook instead.
 *
 *      Returns MBID_YES or MBID_NO.
 *
 *      Special cases:
 *
 *      --  For BASEERR_DAEMON_DEAD, this instead prompts
 *          the user whether to restart the daemon and
 *          returns MBID_NO always.
 *
 *      --  For ERROR_INTERRUPT (which is returned from
 *          a prompt dialog when "Cancel" is pressed),
 *          this does nothing and returns MBID_NO.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 *@@changed V0.9.19 (2002-03-28) [umoeller]: now offering to restart daemon
 *@@changed V0.9.19 (2002-03-28) [umoeller]: fixed error msg with ERROR_INTERRUPT
 */

ULONG cmnProgramErrorMsgBox(HWND hwndOwner,
                            WPObject *pProgram,
                            PSZ pszFailingName,     // in: failing module from progOpenProgram or NULL
                            APIRET arc)
{
    ULONG   ulrc = MBID_NO;

    switch (arc)
    {
        case ERROR_INTERRUPT:           // V0.9.19 (2002-03-28) [umoeller]
        case ERROR_SMG_START_IN_BACKGROUND:     // V1.0.4 (2005-02-27) [pr]: @@fixes 635
        break;

        case BASEERR_DAEMON_DEAD:     // V0.9.19 (2002-03-28) [umoeller]
            if (cmnMessageBoxExt(hwndOwner,
                                 104,
                                 NULL, 0,
                                 232,       // cannot contact daemn, restart now?
                                 MB_YESNO)
                    == MBID_YES)
            {
                krnStartDaemon();
            }
        break;

        default:
        {
            XSTRING strTitle,
                    strPrefix,
                    strSuffix;
            PCSZ     psz = _wpQueryTitle(pProgram);

            xstrInit(&strTitle, 0);
            xstrInit(&strPrefix, 0);
            xstrInit(&strSuffix, 0);

            cmnGetMessage(NULL, 0, &strTitle, 227);     // cannot start program
            cmnGetMessage(&psz, 1, &strPrefix, 228);     // error starting %1
            cmnGetMessage(NULL, 0, &strSuffix, 229);     // open settings?

            ulrc = cmnDosErrorMsgBox(hwndOwner,
                                     pszFailingName,
                                     strTitle.psz,
                                     strPrefix.psz,
                                     arc,
                                     strSuffix.psz,
                                     MB_YESNO,
                                     TRUE);

            xstrClear(&strTitle);
            xstrClear(&strPrefix);
            xstrClear(&strSuffix);
        }
    }

    return ulrc;
}

/*
 *@@ cmnTextEntryBox:
 *      wrapper around dlghTextEntryBox, which
 *      was moved to dialog.c with V0.9.15.
 *
 *@@added V0.9.13 (2001-06-19) [umoeller]
 *@@changed V0.9.15 (2001-09-14) [umoeller]: moved actual code to dialog.c
 */

PSZ cmnTextEntryBox(HWND hwndOwner,
                    PCSZ pcszTitle,
                    PCSZ pcszDescription,
                    PCSZ pcszDefault,
                    ULONG ulMaxLen,
                    ULONG fl)
{
    return dlghTextEntryBox(hwndOwner,
                            pcszTitle,
                            pcszDescription,
                            pcszDefault,
                            cmnGetString(DID_OK),
                            cmnGetString(DID_CANCEL),
                            ulMaxLen,
                            fl,
                            cmnQueryDefaultFont());
}

/*
 *@@ cmnSetDlgHelpPanel:
 *      sets help panel before calling fnwpDlgGeneric.
 */

VOID cmnSetDlgHelpPanel(ULONG ulHelpPanel)
{
    G_ulCurHelpPanel = ulHelpPanel;
}

/*
 *@@  cmn_fnwpDlgWithHelp:
 *          this is the dlg procedure for XFolder dlg boxes;
 *          it can process WM_HELP messages. All other messages
 *          are passed to WinDefDlgProc.
 *
 *          Use cmnSetDlgHelpPanel to set the help panel before
 *          using this dlg proc.
 *
 *@@changed V0.9.2 (2000-03-04) [umoeller]: renamed from fnwpDlgGeneric
 */

MRESULT EXPENTRY cmn_fnwpDlgWithHelp(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = NULL;

    switch (msg)
    {
        case WM_HELP:
        {
            // HMODULE hmodResource = cmnQueryNLSModuleHandle(FALSE);
            /* WM_HELP is received by this function when F1 or a "help" button
               is pressed in a dialog window. */
            // ulCurHelpPanel is set by instance methods before creating a
            // dialog box in order to link help topics to the displayed
            // dialog box. Possible values are:
            //      0: open online reference ("<XWP_REF>", INF book)
            //    > 0: open help topic in xfldr.hlp
            //     -1: ignore WM_HELP */

            if (G_ulCurHelpPanel > 0)
            {
                // replaced all the following V0.9.16 (2001-10-15) [umoeller]
                cmnDisplayHelp(NULL, G_ulCurHelpPanel);
                /* WPObject    *pHelpSomSelf = cmnQueryActiveDesktop();
                if (pHelpSomSelf)
                {
                    PCSZ pszHelpLibrary;
                    BOOL fProcessed = FALSE;
                    if (pszHelpLibrary = cmnQueryHelpLibrary())
                        // path found: display help panel
                        if (_wpDisplayHelp(pHelpSomSelf, G_ulCurHelpPanel, (PSZ)pszHelpLibrary))
                            fProcessed = TRUE;

                    if (!fProcessed)
                        cmnMessageBoxExt(HWND_DESKTOP, 104, 134, MB_OK);
                } */
            }
            else if (G_ulCurHelpPanel == 0)
            {
                HOBJECT     hobjRef = 0;
                // open online reference
                // G_ulCurHelpPanel = -1; // ignore further WM_HELP messages: this one suffices
                hobjRef = WinQueryObject((PSZ)XFOLDER_USERGUIDE);
                if (hobjRef)
                    WinOpenObject(hobjRef, OPEN_DEFAULT, TRUE);
                else
                    cmnMessageBoxExt(HWND_DESKTOP,
                                     104,
                                     NULL, 0,
                                     137,
                                     MB_OK);

            } // end else; if ulCurHelpPanel is < 0, nothing happens
            mrc = NULL;
        }
        break;  // end case WM_HELP

        default:
            mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
        break;
    }

    return mrc;
}

/*
 *@@ cmnFileDlg2:
 *
 *@@added V0.9.19 (2002-04-24) [umoeller]
 */

BOOL cmnFileDlg2(HWND hwndOwner,    // in: owner for file dlg
                 PSZ pszFile,       // in: file mask; out: fully q'd filename
                                    //    (should be CCHMAXPATH in size)
                 ULONG flFlags,     // in: any combination of the following:
                                    // -- WINH_FOD_SAVEDLG: save dlg; else open dlg
                                    // -- WINH_FOD_INILOADDIR: load FOD path from INI
                                    // -- WINH_FOD_INISAVEDIR: store FOD path to INI on OK
                 HINI hini,         // in: INI file to load/store last path from (can be HINI_USER)
                 PCSZ pcszApplication, // in: INI application to load/store last path from
                 PCSZ pcszKey,        // in: INI key to load/store last path from
                 BOOL fUseNewFileDlg)   // in: if TRUE, use filedlg.c
{
    HWND hwndFileDlg;
    FILEDLG fd;
    memset(&fd, 0, sizeof(FILEDLG));
    fd.cbSize = sizeof(FILEDLG);
    fd.fl = FDS_CENTER;

    if (flFlags & WINH_FOD_SAVEDLG)
        fd.fl |= FDS_SAVEAS_DIALOG;
    else
        fd.fl |= FDS_OPEN_DIALOG;

    // default: copy pszFile
    strlcpy(fd.szFullFile, pszFile, sizeof(fd.szFullFile));

    // _PmpfF(("pszFile = %s", pszFile));

    if (    (hini)
         && (flFlags & WINH_FOD_INILOADDIR)
         // overwrite with initial directory for FOD from OS2.INI
         && (PrfQueryProfileString(hini,
                                   (PSZ)pcszApplication,
                                   (PSZ)pcszKey,
                                   "",      // default string V0.9.9 (2001-02-10) [umoeller]
                                   fd.szFullFile,
                                   sizeof(fd.szFullFile)-10)
                     >= 2)
       )
    {
        // found: append the original file mask
        // V0.9.16 (2001-10-19) [umoeller]
        PCSZ p;
        PCSZ pcszMask = "*";

        if (!(p = strrchr(pszFile, '\\')))
            p = pszFile;
        else
            p++;

        if (    (strchr(p, '*'))
             || (strchr(p, '?'))
           )
            // caller has specified file mask:
            pcszMask = p;       // exclude backslash

        strcat(fd.szFullFile, "\\");
        strcat(fd.szFullFile, pcszMask);
    }

    // _PmpfF(("fd.szFullFile now = %s", fd.szFullFile));

    if (fUseNewFileDlg)
        hwndFileDlg = fdlgFileDlg(hwndOwner, // owner
                                  NULL,
                                  &fd);
    else
        hwndFileDlg = WinFileDlg(HWND_DESKTOP,
                                 hwndOwner,
                                 &fd);

    if (    (hwndFileDlg)
         && (fd.lReturn == DID_OK)
       )
    {
        // _PmpfF(("got DID_OK"));

        // save path back?
        if (    (hini)
             && (flFlags & WINH_FOD_INISAVEDIR)
           )
        {
            // get the directory that was used
            PSZ p;
            if (p = strrchr(fd.szFullFile, '\\'))
            {
                // contains directory:
                // copy to OS2.INI
                PSZ pszDir;
                if (pszDir = strhSubstr(fd.szFullFile, p))
                {
                    PrfWriteProfileString(hini,
                                          (PSZ)pcszApplication,
                                          (PSZ)pcszKey,
                                          pszDir);
                    free(pszDir);
                }
            }
        }

        strlcpy(pszFile, fd.szFullFile, CCHMAXPATH);

        return TRUE;
    }

    return FALSE;
}

/*
 *@@ cmnFileDlg:
 *      same as winhFileDlg, but uses fdlgFileDlg
 *      instead.
 *
 *@@added V0.9.9 (2001-03-10) [umoeller]
 */

BOOL cmnFileDlg(HWND hwndOwner,    // in: owner for file dlg
                PSZ pszFile,       // in: file mask; out: fully q'd filename
                                   //    (should be CCHMAXPATH in size)
                ULONG flFlags,     // in: any combination of the following:
                                   // -- WINH_FOD_SAVEDLG: save dlg; else open dlg
                                   // -- WINH_FOD_INILOADDIR: load FOD path from INI
                                   // -- WINH_FOD_INISAVEDIR: store FOD path to INI on OK
                HINI hini,         // in: INI file to load/store last path from (can be HINI_USER)
                PCSZ pcszApplication, // in: INI application to load/store last path from
                PCSZ pcszKey)        // in: INI key to load/store last path from
{
    BOOL fUseNewFileDlg = FALSE;

#ifndef __NEVERNEWFILEDLG__
    fUseNewFileDlg = cmnQuerySetting(sfNewFileDlg);
#endif

    return cmnFileDlg2(hwndOwner,
                       pszFile,
                       flFlags,
                       hini,
                       pcszApplication,
                       pcszKey,
                       fUseNewFileDlg);
}

/* ******************************************************************
 *
 *   WPS debugging
 *
 ********************************************************************/

#ifdef __DEBUG__

    /*
     *@@ cmnDebugWindowProc:
     *
     *@@added V1.0.1 (2002-12-19) [umoeller]
     */

    MRESULT cmnDebugWindowProc(PCSZ pcszSourceFile,
                               ULONG ulLine,
                               PCSZ pcszFunction,
                               PFNWP pfnwp,
                               HWND hwnd,
                               ULONG msg,
                               MPARAM mp1,
                               MPARAM mp2)
    {
        MRESULT mrc;

        TRY_QUIET(excpt1)
        {
            mrc = pfnwp(hwnd, msg, mp1, mp2);
        }
        CATCH(excpt1)
        {
            CHAR szClass[100] = "unknown";
            WinQueryClassName(hwnd, sizeof(szClass), szClass);
            cmnLog(pcszSourceFile, ulLine, pcszFunction,
                   "Exception calling pfnwp 0x%lX, hwnd 0x%lX [%s], msg 0x%lX, mp1 0x%lX, mp2 0x%lX",
                   pfnwp,
                   hwnd,
                   szClass,
                   msg,
                   mp1,
                   mp2);
        } END_CATCH();

        return mrc;
    }

#endif

/*
 *@@ cmnIdentifyView:
 *      returns the name of the given view type as
 *      a string (e.g. "OPEN_CONTENTS"), or a localized
 *      "unknown" string if not recognized.
 *
 *@@added V1.0.0 (2002-08-31) [umoeller]
 *@@changed V1.0.1 (2002-12-08) [umoeller]: moved this here from fdrmenus.c
 */

PCSZ cmnIdentifyView(ULONG ulView)
{
    switch (ulView)
    {
        #define CHECKVIEW(v) case v: return # v

        CHECKVIEW(OPEN_DEFAULT);
        CHECKVIEW(OPEN_HELP);
        CHECKVIEW(OPEN_RUNNING);
        CHECKVIEW(OPEN_PROMPTDLG);
        CHECKVIEW(OPEN_PALETTE);
        CHECKVIEW(CLOSED_ICON);
        CHECKVIEW(OPEN_CONTENTS);
        CHECKVIEW(OPEN_TREE);
        CHECKVIEW(OPEN_SETTINGS);
        CHECKVIEW(OPEN_DETAILS);

        default:
            switch (ulView - *G_pulVarMenuOfs)
            {
                CHECKVIEW(ID_XFMI_OFS_XWPVIEW);
                CHECKVIEW(ID_XFMI_OFS_SPLITVIEW);
                CHECKVIEW(ID_XFMI_OFS_SPLIT_SUBTREE);       // V1.0.1 (2002-12-08) [umoeller]
                CHECKVIEW(ID_XFMI_OFS_SPLIT_SUBFILES);
            }
    }

    return cmnGetString(ID_SDDI_APMVERSION); // "unknown"
}

#ifdef __DEBUG__

    /*
     *@@ cmnIdentifyRestoreID:
     *      this returns a string to identify the
     *      "restore ID" used in wpRestoreString,
     *      wpRestoreData, wpRestoreLong.
     *
     *      This is useful for debugging all those
     *      keys that are undocumented.
     *
     *      This returns a static PSZ, so do not
     *      free it.
     *
     *@@added V0.9.1 (2000-01-17) [umoeller]
     *@@changed V1.0.1 (2002-12-08) [umoeller]: moved this here from wpsh.c
     */

    PCSZ cmnIdentifyRestoreID(PSZ pszClass,     // in: class name (as in wpRestore*)
                              ULONG ulKey)      // in: value ID (as in wpRestore*)
    {
        if (!strcmp(pszClass, G_pcszWPObject))
        {
            switch (ulKey)
            {
                case 1:
                    return "IDKEY_OBJID";
                case 2:
                    return "IDKEY_OBJHELPPANEL";
                case 6:
                    return "IDKEY_OBJSZID";
                case 7:
                    return "IDKEY_OBJSTYLE";
                case 8:
                    return "IDKEY_OBJMINWIN";
                case 9:
                    return "IDKEY_OBJCONCURRENT";
                case 10:
                    return "IDKEY_OBJVIEWBUTTON";
                case 11:
                    return "IDKEY_OBJLONGS";
                case 12:
                    return "IDKEY_OBJSTRINGS";
            }
        }
        else if (!strcmp(pszClass, "WPFileSystem"))
        {
            switch (ulKey)
            {
                case 4:
                    return "IDKEY_FSYSMENUCOUNT";
                case 3:
                    return "IDKEY_FSYSMENUARRAY";
            }
        }
        else if (!strcmp(pszClass, G_pcszWPFolder))
        {
            switch (ulKey)
            {
                case IDKEY_FDRCONTENTATTR    : // 2900
                    return "IDKEY_FDRCONTENTATTR";
                case IDKEY_FDRTREEATTR       : // 2901
                    return "IDKEY_FDRTREEATTR";
                case IDKEY_FDRCVLFONT        : // 2902
                    return "IDKEY_FDRCVLFONT";
                case IDKEY_FDRCVNFONT        : // 2903
                    return "IDKEY_FDRCVNFONT";
                case IDKEY_FDRCVIFONT        : // 2904
                    return "IDKEY_FDRCVIFONT";
                case IDKEY_FDRTVLFONT        : // 2905
                    return "IDKEY_FDRTVLFONT";
                case IDKEY_FDRTVNFONT        : // 2906
                    return "IDKEY_FDRTVNFONT";
                case IDKEY_FDRDETAILSATTR    : // 2907
                    return "IDKEY_FDRDETAILSATTR";
                case IDKEY_FDRDVFONT         : // 2908
                    return "IDKEY_FDRDVFONT";
                case IDKEY_FDRDETAILSCLASS   : // 2909
                    return "IDKEY_FDRDETAILSCLASS";
                case IDKEY_FDRICONPOS        : // 2910
                    return "IDKEY_FDRICONPOS";
                case IDKEY_FDRINVISCOLUMNS   : // 2914
                    return "IDKEY_FDRINVISCOLUMNS";
                case IDKEY_FDRINCCLASS       : // 2920
                    return "IDKEY_FDRINCCLASS";
                case IDKEY_FDRINCNAME        : // 2921
                    return "IDKEY_FDRINCNAME";
                case IDKEY_FDRFSYSSEARCHINFO : // 2922
                    return "IDKEY_FDRFSYSSEARCHINFO";
                case IDKEY_FILTERCONTENT     : // 2923
                    return "IDKEY_FILTERCONTENT";
                case IDKEY_CNRBACKGROUND     : // 2924
                    return "IDKEY_CNRBACKGROUND";
                case IDKEY_FDRINCCRITERIA    : // 2925
                    return "IDKEY_FDRINCCRITERIA";
                case IDKEY_FDRICONVIEWPOS    : // 2926
                    return "IDKEY_FDRICONVIEWPOS";
                case IDKEY_FDRSORTCLASS      : // 2927
                    return "IDKEY_FDRSORTCLASS";
                case IDKEY_FDRSORTATTRIBS    : // 2928
                    return "IDKEY_FDRSORTATTRIBS";
                case IDKEY_FDRSORTINFO       : // 2929
                    return "IDKEY_FDRSORTINFO";
                case IDKEY_FDRSNEAKYCOUNT    : // 2930
                    return "IDKEY_FDRSNEAKYCOUNT";
                case IDKEY_FDRLONGARRAY      : // 2931
                    return "IDKEY_FDRLONGARRAY";
                case IDKEY_FDRSTRARRAY       : // 2932
                    return "IDKEY_FDRSTRARRAY";
                case IDKEY_FDRCNRBACKGROUND  : // 2933
                    return "IDKEY_FDRCNRBACKGROUND";
                case IDKEY_FDRBKGNDIMAGEFILE : // 2934
                    return "IDKEY_FDRBKGNDIMAGEFILE";
                case IDKEY_FDRBACKGROUND     : // 2935
                    return "IDKEY_FDRBACKGROUND";
                case IDKEY_FDRSELFCLOSE      : // 2936
                    return "IDKEY_FDRSELFCLOSE";

                case 2937:
                    return "IDKEY_FDRODMENUBARON";
                case 2938:
                    return "IDKEY_FDRGRIDINFO";
                case 2939:
                    return "IDKEY_FDRTREEVIEWCONTENTS";
            }
        }

        return "unknown";
    }

    /*
     *@@ cmnDumpTaskRec:
     *
     *@@added V0.9.1 (2000-02-01) [umoeller]
     *@@changed V1.0.1 (2002-12-08) [umoeller]: moved this here from wpsh.c
     */

    VOID cmnDumpTaskRec(WPObject *somSelf,
                        const char *pcszMethodName,
                        PTASKREC pTaskRec)
    {
        _Pmpf(("%s: dumping task rec 0x%lX for obj 0x%lX (%s)",
                pcszMethodName,
                pTaskRec,
                somSelf,
                _wpQueryTitle(somSelf) ));

        if (pTaskRec)
        {
            ULONG   ul = 0;
            CHAR    szFolder[CCHMAXPATH] = "null";

            while (pTaskRec)
            {
                if (pTaskRec->folder)
                    _wpQueryFilename(pTaskRec->folder, szFolder, TRUE);
                else
                    memcpy(szFolder, "null", 5);
                _Pmpf(("Index: %d", ul));
                _Pmpf(("    useCount: %d", pTaskRec->useCount));
                _Pmpf(("    pStdDlg: 0x%lX", pTaskRec->pStdDlg));
                _Pmpf(("    folder: 0x%lX (%s)", pTaskRec->folder, szFolder ));
                _Pmpf(("    xOrigin: %d", pTaskRec->xOrigin));
                _Pmpf(("    yOrigin: %d", pTaskRec->yOrigin));
                _Pmpf(("    pszTitle: 0x%lX (%s)",
                            pTaskRec->pszTitle,
                            (pTaskRec->pszTitle) ? pTaskRec->pszTitle : "NULL"));
                _Pmpf(("    posAfterRecord: 0x%lX", pTaskRec->positionAfterRecord));
                _Pmpf(("    keepAssocs: %d", pTaskRec->fKeepAssociations));
                _Pmpf(("    pReserved: 0x%lX", pTaskRec->pReserved));

                pTaskRec = pTaskRec->next;
                ul++;
            }
        }
        else
            _Pmpf(("    pTaskRec is NULL"));
    }

#endif


