
/*
 *@@sourcefile w_diskfree.c:
 *      XCenter "Disk Usage" widget.
 *
 *      This is all new with V0.9.9. Thanks to fonz for the
 *      contribution.
 *
 *@@added V0.9.9 (2001-02-28) [umoeller]
 *@@header "shared\center.h"
 */

// #define YURIEXT 1

#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS

#define INCL_WIN
#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINDIALOGS
#define INCL_WININPUT
#define INCL_WINSWITCHLIST
#define INCL_WINRECTANGLES
#define INCL_WINPOINTERS
#define INCL_WINSYS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS
#define INCL_WINWORKPLACE

#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#define DONT_REPLACE_MALLOC             // in case mem debug is enabled
#define DONT_REPLACE_FOR_DBCS           // do not replace strchr with DBCS version
#include "setup.h"                      // code generation and debugging options

// disable wrappers, because we're not linking statically
#ifdef DOSH_STANDARDWRAPPERS
    #undef DOSH_STANDARDWRAPPERS
#endif
#ifdef WINH_STANDARDWRAPPERS
    #undef WINH_STANDARDWRAPPERS
#endif

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\timer.h"
#include "helpers\prfh.h"               // INI file helper routines;
                                        // this include is required for some
                                        // of the structures in shared\center.h
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\center.h"              // public XCenter interfaces
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs

#include "config\drivdlgs.h"            // driver configuration dialogs

#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

// None currently.

/* ******************************************************************
 *
 *   XCenter widget class definition
 *
 ********************************************************************/

/*
 *      This contains the name of the PM window class and
 *      the XCENTERWIDGETCLASS definition(s) for the widget
 *      class(es) in this DLL.
 */

#define DISKFREE_SHOW_FS        0x01
#define ID_DBLCLKTIMERID        0x0102
#define WNDCLASS_WIDGET_SAMPLE "XWPCenterDiskfreeWidget"

void EXPENTRY WgtShowSettingsDlg(PWIDGETSETTINGSDLGDATA pData);

static const XCENTERWIDGETCLASS G_WidgetClasses[]
    = {
          WNDCLASS_WIDGET_SAMPLE,     // PM window class name
          0,                          // additional flag, not used here
          "DiskfreeWidget",           // internal widget class name
          (PCSZ)(XCENTER_STRING_RESOURCE | ID_CRSI_WIDGET_DISKFREE_WC),
                                      // widget class name displayed to user
                                      // (NLS DLL) V0.9.19 (2002-05-07) [umoeller]
          WGTF_SIZEABLE |       // V0.9.19 (2002-06-12) [umoeller]
            WGTF_TRAYABLE,
          WgtShowSettingsDlg          // our settings dialog
      };

/* ******************************************************************
 *
 *   Function imports from XFLDR.DLL
 *
 ********************************************************************/

/*
 *      To reduce the size of the widget DLL, it can
 *      be compiled with the VAC subsystem libraries.
 *      In addition, instead of linking frequently
 *      used helpers against the DLL again, you can
 *      import them from XFLDR.DLL, whose module handle
 *      is given to you in the INITMODULE export.
 *
 *      Note that importing functions from XFLDR.DLL
 *      is _not_ a requirement. We only do this to
 *      avoid duplicate code.
 *
 *      For each funtion that you need, add a global
 *      function pointer variable and an entry to
 *      the G_aImports array. These better match.
 *
 *      The actual imports are then made by WgtInitModule.
 */

// resolved function pointers from XFLDR.DLL
PCMNLOADDLG pcmnLoadDlg = NULL;
PCMNQUERYDEFAULTFONT pcmnQueryDefaultFont = NULL;
PCMNQUERYHELPLIBRARY pcmnQueryHelpLibrary = NULL;
PCMNQUERYMAINRESMODULEHANDLE pcmnQueryMainResModuleHandle = NULL;
PCMNQUERYNLSMODULEHANDLE pcmnQueryNLSModuleHandle = NULL;
PCMNSETCONTROLSFONT pcmnSetControlsFont = NULL;

PDOSHENUMDRIVES pdoshEnumDrives = NULL;
PDOSHQUERYDISKSIZE pdoshQueryDiskSize = NULL;
PDOSHQUERYDISKFREE pdoshQueryDiskFree = NULL;
PDOSHQUERYDISKFSTYPE pdoshQueryDiskFSType = NULL;

PDRV_SPRINTF pdrv_sprintf = NULL;

PTMRSTARTXTIMER ptmrStartXTimer = NULL;
PTMRSTOPXTIMER ptmrStopXTimer = NULL;

PCTRDISPLAYHELP pctrDisplayHelp = NULL;
PCTRFREESETUPVALUE pctrFreeSetupValue = NULL;
PCTRPARSECOLORSTRING pctrParseColorString = NULL;
PCTRSCANSETUPSTRING pctrScanSetupString = NULL;
PCTRSETSETUPSTRING pctrSetSetupString = NULL;

PGPIHDRAW3DFRAME pgpihDraw3DFrame = NULL;
PGPIHDRAW3DFRAME2 pgpihDraw3DFrame2 = NULL;
PGPIHSWITCHTORGB pgpihSwitchToRGB = NULL;

PWINHFREE pwinhFree = NULL;
PWINHQUERYPRESCOLOR pwinhQueryPresColor = NULL;
PWINHQUERYWINDOWFONT pwinhQueryWindowFont = NULL;
PWINHSETWINDOWFONT pwinhSetWindowFont = NULL;
PWINHCENTERWINDOW pwinhCenterWindow = NULL;

PXSTRCAT pxstrcat = NULL;
PXSTRCLEAR pxstrClear = NULL;
PXSTRINIT pxstrInit = NULL;

static const RESOLVEFUNCTION G_aImports[] =
    {
        "cmnLoadDlg", (PFN*)&pcmnLoadDlg,
        "cmnQueryDefaultFont", (PFN*)&pcmnQueryDefaultFont,
        "cmnQueryHelpLibrary", (PFN*)&pcmnQueryHelpLibrary,
        "cmnQueryMainResModuleHandle", (PFN*)&pcmnQueryMainResModuleHandle,
        "cmnQueryNLSModuleHandle", (PFN*)&pcmnQueryNLSModuleHandle,
        "cmnSetControlsFont", (PFN*)&pcmnSetControlsFont,

        "doshEnumDrives", (PFN*)&pdoshEnumDrives,
        "doshQueryDiskSize", (PFN*)&pdoshQueryDiskSize,
        "doshQueryDiskFree", (PFN*)&pdoshQueryDiskFree,
        "doshQueryDiskFSType", (PFN*)&pdoshQueryDiskFSType,

        "drv_sprintf", (PFN*)&pdrv_sprintf,

        "tmrStartXTimer", (PFN*)&ptmrStartXTimer,
        "tmrStopXTimer", (PFN*)&ptmrStopXTimer,

        "ctrDisplayHelp", (PFN*)&pctrDisplayHelp,
        "ctrFreeSetupValue", (PFN*)&pctrFreeSetupValue,
        "ctrParseColorString", (PFN*)&pctrParseColorString,
        "ctrScanSetupString", (PFN*)&pctrScanSetupString,
        "ctrSetSetupString", (PFN*)&pctrSetSetupString,

        "gpihDraw3DFrame", (PFN*)&pgpihDraw3DFrame,
        "gpihDraw3DFrame2", (PFN*)&pgpihDraw3DFrame2,
        "gpihSwitchToRGB", (PFN*)&pgpihSwitchToRGB,

        "winhFree", (PFN*)&pwinhFree,
        "winhQueryPresColor", (PFN*)&pwinhQueryPresColor,
        "winhQueryWindowFont", (PFN*)&pwinhQueryWindowFont,
        "winhSetWindowFont", (PFN*)&pwinhSetWindowFont,
        "winhCenterWindow", (PFN*)&pwinhCenterWindow,

        "xstrcat", (PFN*)&pxstrcat,
        "xstrClear", (PFN*)&pxstrClear,
        "xstrInit", (PFN*)&pxstrInit
    };

/* ******************************************************************
 *
 *   Private widget instance data
 *
 ********************************************************************/

/*
 *@@ DISKFREESETUP:
 *      instance data to which setup strings correspond.
 *      This is also a member of DISKFREEPRIVATE.
 *
 *      Putting these settings into a separate structure
 *      is no requirement technically. However, once the
 *      widget uses a settings dialog, the dialog must
 *      support changing the widget settings even if the
 *      widget doesn't currently exist as a window, so
 *      separating the setup data from the widget window
 *      data will come in handy for managing the setup
 *      strings.
 *
 *@@changed V0.9.19 (2002-06-12) [umoeller]: added cx field, we're sizeable now
 */

typedef struct _DISKFREESETUP
{
    long        lcolBackground,         // background color
                lcolForeground;         // foreground color (for text)

    PSZ         pszFont;
            // if != NULL, non-default font (in "8.Helv" format);
            // this has been allocated using local malloc()!

    CHAR        chDrive;      // if ch == '*' we are in 'multi-view'
    LONG        lShow;        // eg FILETYPE

    LONG        cx;           // width (we're sizeable now)
                              // V0.9.19 (2002-06-12) [umoeller]

} DISKFREESETUP, *PDISKFREESETUP;

/*
 *@@ DISKFREEPRIVATE:
 *      more window data for the widget.
 *
 *      An instance of this is created on WM_CREATE in
 *      fnwpSampleWidget and stored in XCENTERWIDGET.pUser.
 */

typedef struct _DISKFREEPRIVATE
{
    PXCENTERWIDGET pWidget;
            // reverse ptr to general widget data ptr; we need
            // that all the time and don't want to pass it on
            // the stack with each function call

    DISKFREESETUP Setup;
            // widget settings that correspond to a setup string

    HPOINTER hptrHand;
    HPOINTER hptrDrive;
    HPOINTER hptrDrives[3];

    char     szDrives[27];
    BYTE     bFSIcon;

    char     chAktDrive;
    char     szAktDriveType[12];
    double   dAktDriveFree;
    double   dAktDriveSize;

    ULONG    ulTimerID;

} DISKFREEPRIVATE, *PDISKFREEPRIVATE;


// not defined in the toolkit-headers
BOOL APIENTRY WinStretchPointer(HPS hps,
                                long lX,
                                long ly,
                                long lcy,
                                long lcx,
                                HPOINTER hptr,
                                ULONG ulHalf);

// prototypes
void ScanSwitchList(PDISKFREEPRIVATE pPrivate);

BOOL GetDriveInfo(PDISKFREEPRIVATE pPrivate);

void GetDrive(HWND hwnd,
              PXCENTERWIDGET pWidget,
              BOOL fNext); // if fNext = FALSE, ut returns the prev. drive

CHAR ValidateDrive(CHAR chDrive);

MRESULT EXPENTRY fnwpSettingsDlg(HWND hwnd,
                                 ULONG msg,
                                 MPARAM mp1,
                                 MPARAM mp2);

/* ******************************************************************
 *
 *   Widget setup management
 *
 ********************************************************************/

/*
 *      This section contains shared code to manage the
 *      widget's settings. This can translate a widget
 *      setup string into the fields of a binary setup
 *      structure and vice versa. This code is used by
 *      an open widget window, but could be shared with
 *      a settings dialog, if you implement one.
 */

/*
 *@@ WgtClearSetup:
 *      cleans up the data in the specified setup
 *      structure, but does not free the structure
 *      itself.
 */

void WgtClearSetup(PDISKFREESETUP pSetup)
{
    if (pSetup)
    {
        if (pSetup->pszFont)
        {
            free(pSetup->pszFont);
            pSetup->pszFont = NULL;
        }
    }
}

/*
 *@@ WgtScanSetup:
 *      scans the given setup string and translates
 *      its data into the specified binary setup
 *      structure.
 *
 *      NOTE: It is assumed that pSetup is zeroed
 *      out. We do not clean up previous data here.
 *
 *@@changed V1.0.2 (2003-02-03) [umoeller]: changed default text color
 */

void WgtScanSetup(PCSZ pcszSetupString,
                  PDISKFREESETUP pSetup)
{
    PSZ p;

    // width
    if (p = pctrScanSetupString(pcszSetupString,
                                "WIDTH"))
    {
        pSetup->cx = atoi(p);
        pctrFreeSetupValue(p);
    }
    else
        pSetup->cx = 100;

    // background color
    if (p = pctrScanSetupString(pcszSetupString,
                                "BGNDCOL"))
    {
        pSetup->lcolBackground = pctrParseColorString(p);
        pctrFreeSetupValue(p);
    }
    else
        // default color:
        pSetup->lcolBackground = WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0);

    // text color:
    if (p = pctrScanSetupString(pcszSetupString,
                                "TEXTCOL"))
    {
        pSetup->lcolForeground = pctrParseColorString(p);
        pctrFreeSetupValue(p);
    }
    else
        pSetup->lcolForeground = WinQuerySysColor(HWND_DESKTOP,
                                                  SYSCLR_WINDOWTEXT,        // changed V1.0.2 (2003-02-03) [umoeller]
                                                  0);

    // font:
    // we set the font presparam, which automatically
    // affects the cached presentation spaces
    if (p = pctrScanSetupString(pcszSetupString,
                                "FONT"))
    {
        pSetup->pszFont = strdup(p);
        pctrFreeSetupValue(p);
    }
    // else: leave this field null

    // view:  *..multi-view | otherwise..single-view where setup-string is drive-letter
    if (p = pctrScanSetupString(pcszSetupString,
                                "VIEW"))
    {
        pSetup->chDrive = ValidateDrive(*p);  // V0.9.11 (2001-04-19) [pr]: Validate drive letter
        pctrFreeSetupValue(p);
    }
    else
        pSetup->chDrive = '*';


    // different 'show-styles'
    if (p = pctrScanSetupString(pcszSetupString,
                                "SHOW"))
    {
        pSetup->lShow = atol(p);
        pctrFreeSetupValue(p);
    }
    else
        pSetup->lShow = 0;
}

/*
 *@@ WgtSaveSetup:
 *      composes a new setup string.
 *      The caller must invoke xstrClear on the
 *      string after use.
 *
 *@@changed V0.9.20 (2002-07-03) [umoeller]: WIDTH save was missing, fixed
 */

void WgtSaveSetup(PXSTRING pstrSetup,       // out: setup string (is cleared first)
                  PDISKFREESETUP pSetup)
{
    CHAR    szTemp[100];
    pxstrInit(pstrSetup, 100);

    // width was missing V0.9.20 (2002-07-03) [umoeller]
    pdrv_sprintf(szTemp, "WIDTH=%d;",
                 pSetup->cx);
    pxstrcat(pstrSetup, szTemp, 0);

    pdrv_sprintf(szTemp, "BGNDCOL=%06lX;",
                 pSetup->lcolBackground);
    pxstrcat(pstrSetup, szTemp, 0);

    pdrv_sprintf(szTemp, "TEXTCOL=%06lX;",
                 pSetup->lcolForeground);
    pxstrcat(pstrSetup, szTemp, 0);

    if (pSetup->pszFont)
    {
        // non-default font:
        pdrv_sprintf(szTemp, "FONT=%s;",
                     pSetup->pszFont);
        pxstrcat(pstrSetup, szTemp, 0);
    }

    // view:  *..multi-view | otherwise..single-view where setup-string is drive-letter
    pdrv_sprintf(szTemp, "VIEW=%c;",
                 pSetup->chDrive);
    pxstrcat(pstrSetup, szTemp, 0);

    // different 'show-styles'
    pdrv_sprintf(szTemp, "SHOW=%02d;",
                 pSetup->lShow);
    pxstrcat(pstrSetup, szTemp, 0);
}

/* ******************************************************************
 *
 *   Widget settings dialog
 *
 ********************************************************************/

#define WMXINT_SETUP    WM_USER+1805

/*
 *@@ fnwpSettingsDlg:
 *      dialog proc for the winlist settings dialog.
 */

MRESULT EXPENTRY fnwpSettingsDlg(HWND hwnd,
                                 ULONG msg,
                                 MPARAM mp1,
                                 MPARAM mp2)
{
    MRESULT mrc = 0;
    static PWIDGETSETTINGSDLGDATA pData;


    switch(msg)
    {
        case WM_INITDLG:
            pData = (PWIDGETSETTINGSDLGDATA)mp2;
            WinPostMsg(hwnd, WMXINT_SETUP, (MPARAM)0, (MPARAM)0); // otherwise all auto(radio)controls are resetted??
        break;

        case WMXINT_SETUP:
        {
            // setup-string-handling
            PDISKFREESETUP pSetup = (PDISKFREESETUP)malloc(sizeof(DISKFREESETUP));

            // set max.length of entryfield to 1
            WinSendDlgItemMsg(hwnd, 106,
                              EM_SETTEXTLIMIT,
                              MPFROMSHORT(1),
                              (MPARAM)0);

            if (pSetup)
            {
                memset(pSetup, 0, sizeof(DISKFREESETUP));
                // store this in WIDGETSETTINGSDLGDATA
                pData->pUser = pSetup;

                WgtScanSetup(pData->pcszSetupString, pSetup);

                if (pSetup->chDrive == '*')
                    WinSendDlgItemMsg(hwnd, 101,
                                      BM_CLICK,
                                      MPFROMSHORT(TRUE),
                                      (MPARAM)0);
                else
                {
                    char sz[2];

                    WinSendDlgItemMsg(hwnd, 102,
                                      BM_CLICK,
                                      MPFROMSHORT(TRUE),
                                      (MPARAM)0);

                    sz[0] = pSetup->chDrive;
                    sz[1] = '\0';
                    WinSetDlgItemText(hwnd, 106, sz);
                }

                if (pSetup->lShow & DISKFREE_SHOW_FS)
                    WinCheckButton(hwnd, 107, 1);
                else
                    WinCheckButton(hwnd, 107, 0);
            }
        }
        break;

        case WM_DESTROY:
        {
            if (pData)
            {
                PDISKFREESETUP pSetup = (PDISKFREESETUP)pData->pUser;
                if (pSetup)
                {
                    WgtClearSetup(pSetup);
                    free(pSetup);
                } // end if (pSetup)
             } // end if (pData)
        }
        break;


        case WM_CONTROL:
        {
            if (SHORT2FROMMP(mp1) == BN_CLICKED)
            {
                if (SHORT1FROMMP(mp1) == 101) // multi-view
                {
                    // disable groupbox+children
                    WinEnableWindow(WinWindowFromID(hwnd, 104), FALSE);
                    WinEnableWindow(WinWindowFromID(hwnd, 105), FALSE);
                    WinEnableWindow(WinWindowFromID(hwnd, 106), FALSE);
                }
                else if (SHORT1FROMMP(mp1) == 102) // single-view
                {
                    WinEnableWindow(WinWindowFromID(hwnd, 104), TRUE);
                    WinEnableWindow(WinWindowFromID(hwnd, 105), TRUE);
                    WinEnableWindow(WinWindowFromID(hwnd, 106), TRUE);
                }
            }
        }
        break;

        case WM_COMMAND:
        {
            switch(SHORT1FROMMP(mp1))
            {
                case 110: // ok-button
                {
                    XSTRING strSetup;
                    PDISKFREESETUP pSetup = (PDISKFREESETUP)pData->pUser;
                    // 'store' settings in pSetup
                    if (!WinSendDlgItemMsg(hwnd, 101,
                                           BM_QUERYCHECKINDEX,
                                           (MPARAM)0,
                                           (MPARAM)0))
                        // radiobutton 1 is checked -> multi-view
                        pSetup->chDrive = '*';
                    else
                    {
                        // radiobutton 2 is checked -> single-view
                        char sz[2] = {0};
                        WinQueryDlgItemText(hwnd, 106, 2, (PSZ)sz);
                        pSetup->chDrive = ValidateDrive(sz[0]);  // V0.9.11 (2001-04-19) [pr]: Validate drive letter
                    }

                    // 'show-styles'
                    pSetup->lShow = 0;
                    if (WinQueryButtonCheckstate(hwnd, 107))
                        pSetup->lShow |= DISKFREE_SHOW_FS;

                    // something has changed:
                    WgtSaveSetup(&strSetup,
                                 pSetup);
                    pData->pctrSetSetupString(pData->hSettings,
                                              strSetup.psz);
                    pxstrClear(&strSetup);

                    WinDismissDlg(hwnd, TRUE);
                }
                break;
            }
        }
        break;

        default:
            mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
    }

    return(mrc);
}

/*
 *@@ WwgtShowSettingsDlg:
 *      this displays the winlist widget's settings
 *      dialog.
 *
 *      This procedure's address is stored in
 *      XCENTERWIDGET so that the XCenter knows that
 *      we can do this.
 *
 *      When calling this function, the XCenter expects
 *      it to display a modal dialog and not return
 *      until the dialog is destroyed. While the dialog
 *      is displaying, it would be nice to have the
 *      widget dynamically update itself.
 *
 *@@changed V0.9.11 (2001-04-18) [umoeller]: moved dialog to XFLDR001.DLL
 */

void EXPENTRY WgtShowSettingsDlg(PWIDGETSETTINGSDLGDATA pData)
{
    HWND hwnd;

    if (hwnd = pcmnLoadDlg(pData->hwndOwner,
                           fnwpSettingsDlg,
                           ID_CRD_DISKFREEWGT_SETTINGS, // V0.9.11 (2001-04-18) [umoeller]
                           // pass original setup string with WM_INITDLG
                           (PVOID)pData))
    {
        pcmnSetControlsFont(hwnd,
                            1,
                            10000);

        pwinhCenterWindow(hwnd);         // invisibly

        // go!!
        WinProcessDlg(hwnd);

        WinDestroyWindow(hwnd);
    }
}

/* ******************************************************************
 *
 *   Callbacks stored in XCENTERWIDGETCLASS
 *
 ********************************************************************/

// If you implement a settings dialog, you must write a
// "show settings dlg" function and store its function pointer
// in XCENTERWIDGETCLASS.

/* ******************************************************************
 *
 *   PM window class implementation
 *
 ********************************************************************/

/*
 *      This code has the actual PM window class.
 *
 */

/*
 *@@ WgtCreate:
 *      implementation for WM_CREATE.
 *
 *@@changed V0.9.11 (2001-04-18) [umoeller]: moved icons to XWPRES.DLL
 *@@changed V0.9.19 (2002-04-17) [umoeller]: "Help" context menu item was disabled, fixed
 */

MRESULT WgtCreate(HWND hwnd,
                  PXCENTERWIDGET pWidget)
{
    MRESULT mrc = 0;
    HMODULE hmodRes = pcmnQueryMainResModuleHandle(); // V0.9.11 (2001-04-18) [umoeller]
    // PSZ p;
    PDISKFREEPRIVATE pPrivate = (PDISKFREEPRIVATE)malloc(sizeof(DISKFREEPRIVATE));
    memset(pPrivate, 0, sizeof(DISKFREEPRIVATE));
    // link the two together
    pWidget->pUser = pPrivate;
    pPrivate->pWidget = pWidget;

    // initialize binary setup structure from setup string
    WgtScanSetup(pWidget->pcszSetupString,
                 &pPrivate->Setup);

    // help was missing V0.9.19 (2002-04-17) [umoeller]
    pWidget->pcszHelpLibrary = pcmnQueryHelpLibrary();
    pWidget->ulHelpPanelID = ID_XSH_WIDGET_DISKFREE_WC;

    // set window font (this affects all the cached presentation
    // spaces we use)
    pwinhSetWindowFont(hwnd,
                       (pPrivate->Setup.pszFont)
                        ? pPrivate->Setup.pszFont
                        // default font: use the same as in the rest of XWorkplace:
                        : pcmnQueryDefaultFont());

    pPrivate->hptrDrive = WinLoadPointer(HWND_DESKTOP,
                                         hmodRes,
                                         ID_ICON_DRIVE);

    pPrivate->hptrHand  = WinLoadPointer(HWND_DESKTOP,
                                         hmodRes,
                                         ID_POINTER_HAND);


    pPrivate->hptrDrives[0] = WinLoadPointer(HWND_DESKTOP,
                                             hmodRes,
                                             ID_ICON_DRIVE_NORMAL);

    pPrivate->hptrDrives[1] = WinLoadPointer(HWND_DESKTOP,
                                             hmodRes,
                                             ID_ICON_DRIVE_LAN);

    pPrivate->hptrDrives[2] = WinLoadPointer(HWND_DESKTOP,
                                             hmodRes,
                                             ID_ICON_DRIVE_CD);

    pdoshEnumDrives((PSZ)pPrivate->szDrives,
                    NULL,
                    TRUE);

    if (pPrivate->Setup.chDrive == '*')
        pPrivate->chAktDrive = *pPrivate->szDrives;
    else
        pPrivate->chAktDrive = pPrivate->Setup.chDrive;

    GetDriveInfo(pPrivate);

    // start update timer
    pPrivate->ulTimerID = ptmrStartXTimer((PXTIMERSET)pPrivate->pWidget->pGlobals->pvXTimerSet,
                                          hwnd,
                                          1,
                                          5000);

    pWidget->ulHelpPanelID = ID_XSH_WIDGET_DISKFREE_WC;

    return mrc;
}

/*
 *@@ MwgtControl:
 *      implementation for WM_CONTROL.
 *
 *      The XCenter communicates with widgets thru
 *      WM_CONTROL messages. At the very least, the
 *      widget should respond to XN_QUERYSIZE because
 *      otherwise it will be given some dumb default
 *      size.
 *
 *@@added V0.9.7 (2000-12-14) [umoeller]
 */

BOOL WgtControl(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    BOOL brc = FALSE;

    // get widget data from QWL_USER (stored there by WM_CREATE)
    PXCENTERWIDGET pWidget;
    PDISKFREEPRIVATE pPrivate;
    if (    (pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER))
         && (pPrivate = (PDISKFREEPRIVATE)pWidget->pUser)
       )
    {
        USHORT  usID = SHORT1FROMMP(mp1),
                usNotifyCode = SHORT2FROMMP(mp1);

        // is this from the XCenter client?
        if (usID == ID_XCENTER_CLIENT)
        {
            // yes:

            switch (usNotifyCode)
            {
                /*
                 * XN_QUERYSIZE:
                 *      XCenter wants to know our size.
                 */

                case XN_QUERYSIZE:
                {
                    PSIZEL pszl = (PSIZEL)mp2;
                    pszl->cx = pPrivate->Setup.cx;  // desired width
                    pszl->cy = 20;                  // desired minimum height
                    brc = TRUE;
                }
                break;

                /*
                 * XN_SETUPCHANGED:
                 *      XCenter has a new setup string for
                 *      us in mp2.
                 *
                 *      NOTE: This only comes in with settings
                 *      dialogs.
                 */

                case XN_SETUPCHANGED:
                {
                    PCSZ pcszNewSetupString = (const char*)mp2;

                    // reinitialize the setup data
                    WgtClearSetup(&pPrivate->Setup);
                    WgtScanSetup(pcszNewSetupString, &pPrivate->Setup);

                    // V0.9.11 (2001-04-19) [pr]: Don't change drive when selecting multi-view
                    if (pPrivate->Setup.chDrive != '*')
                        pPrivate->chAktDrive = pPrivate->Setup.chDrive;


                    GetDriveInfo(pPrivate);

                    WinInvalidateRect(pWidget->hwndWidget, NULL, FALSE);
                }
                break;
            }
        }
    } // end if (pPrivate)

    return brc;
}

/*
 *@@ WgtPaint:
 *      implementation for WM_PAINT.
 *
 *@@changed V1.0.2 (2003-12-10) [pr]: Fixed bad display if drive could not be accessed @@fixes 525
 */

void WgtPaint(HWND hwnd,
              PXCENTERWIDGET pWidget)
{
    HPS hps;
    if (hps = WinBeginPaint(hwnd, NULLHANDLE, NULL))
    {
        PDISKFREEPRIVATE pPrivate;
        if (pPrivate = (PDISKFREEPRIVATE)pWidget->pUser)
        {
            RECTL       rclWin;
            POINTL      aptlText[TXTBOX_COUNT];
            BYTE        bxCorr;
            char        szText[64];
            double      dPercentFree = 0;


            // now paint frame
            WinQueryWindowRect(hwnd,
                               &rclWin);
            pgpihSwitchToRGB(hps);

            WinFillRect(hps,
                        &rclWin,                // exclusive
                        pPrivate->Setup.lcolBackground);


            // draw border
            if (pPrivate->pWidget->pGlobals->flDisplayStyle & XCS_SUNKBORDERS)
            {
                 ULONG ulBorder = 1;
                 RECTL rcl2;


                 memcpy(&rcl2, &rclWin, sizeof(RECTL));
                 rcl2.xRight--;
                 rcl2.yTop--;

                 pgpihDraw3DFrame2(hps,
                                   &rcl2,
                                   ulBorder,
                                   pPrivate->pWidget->pGlobals->lcol3DDark,
                                   pPrivate->pWidget->pGlobals->lcol3DLight);

                 /*
                 rclWin.xLeft += ulBorder;
                 rclWin.yBottom += ulBorder;
                 rclWin.xRight -= ulBorder;
                 rclWin.yTop -= ulBorder;
                 */
            }

            // calculate percent
            // V1.0.2 (2003-12-10) [pr]: @@fixes 525
            if (pPrivate->dAktDriveSize)
                dPercentFree = pPrivate->dAktDriveFree * 100 / pPrivate->dAktDriveSize;
            else
                dPercentFree = 0;

            if (pPrivate->Setup.chDrive == '*') // == multi-view-clickable
            {
                // draw drive-icon
                WinStretchPointer(hps,
                                  rclWin.xLeft + 3,
                                  (rclWin.yTop - rclWin.yBottom - 11) / 2 + 1,
                                  21,
                                  11,
                                  pPrivate->hptrDrives[pPrivate->bFSIcon],
                                  DP_NORMAL);
                                                             // pPrivate->dAktDriveSize/1024/1024...100%
                // print drive-data                             pPrivate->dAktDriveFree/1024/1024...x%
                // V0.9.11 (2001-04-19) [pr]: Fixed show drive type
                if (pPrivate->Setup.lShow & DISKFREE_SHOW_FS)
                    pdrv_sprintf(szText,
                                 "%c: (%s)  %.fMB (%.f%%)",
                                 pPrivate->chAktDrive,
                                 pPrivate->szAktDriveType,
                                 pPrivate->dAktDriveFree/1024/1024,
                                 dPercentFree);
                else
                    pdrv_sprintf(szText,
                                 "%c:  %.fMB (%.f%%)",
                                 pPrivate->chAktDrive,
                                 pPrivate->dAktDriveFree/1024/1024,
                                 dPercentFree);

                bxCorr = 30;
            }
            else
            {
                // draw drive-icon
                WinStretchPointer(hps,
                                  rclWin.xLeft,
                                  (rclWin.yTop-rclWin.yBottom - 11) / 2 + 1,
                                  21,
                                  11,
                                  pPrivate->hptrDrives[pPrivate->bFSIcon],
                                  DP_NORMAL);

                if (pPrivate->Setup.lShow & DISKFREE_SHOW_FS)
                    pdrv_sprintf(szText,
                                 "%c: (%s)  %.fMB",
                                 pPrivate->chAktDrive,
                                 pPrivate->szAktDriveType,
                                 pPrivate->dAktDriveFree/1024/1024);
                else
                    pdrv_sprintf(szText,
                                 "%c:  %.fMB",
                                 pPrivate->chAktDrive,
                                 pPrivate->dAktDriveFree/1024/1024);

                bxCorr = 24;

                //rclWin.xLeft += 24;
            }

            // add 70pixel for grah
            #ifdef YURIEXT
            bxCorr += (50 + 5 + 5);
            #endif

            // now check if we have enough space
            /*  disabled, we're sizeable now V0.9.19 (2002-06-12) [umoeller]
            GpiQueryTextBox(hps,
                            strlen(szText),
                            szText,
                            TXTBOX_COUNT,
                            aptlText);

            if (    ((aptlText[TXTBOX_TOPRIGHT].x+bxCorr) > (rclWin.xRight+2))
                 || pPrivate->lCX == 10
               )
            {
                // we need more space: tell XCenter client
                pPrivate->lCX = (aptlText[TXTBOX_TOPRIGHT].x + bxCorr + 6);

                WinPostMsg(WinQueryWindow(hwnd, QW_PARENT),
                           XCM_SETWIDGETSIZE,
                           (MPARAM)hwnd,
                           (MPARAM)pPrivate->lCX);
            }
            else */
            {
                RECTL rcGraph = rclWin;

                // sufficient space:
                #ifdef YURIEXT
                rclWin.xLeft += (bxCorr - 50 - 5 - 5);
                #else
                rclWin.xLeft += bxCorr;
                #endif

                WinDrawText(hps,
                            strlen(szText),
                            szText,
                            &rclWin,
                            pPrivate->Setup.lcolForeground,
                            pPrivate->Setup.lcolBackground,
                            DT_LEFT| DT_VCENTER);

                #ifdef YURIEXT
                // graph border
                rcGraph.yTop    -= 2;
                rcGraph.yBottom += 2;
                rcGraph.xRight = rcGraph.xRight - 5;
                rcGraph.xLeft  = rcGraph.xRight - 50;
                pgpihDraw3DFrame(hps,
                                 &rcGraph,
                                 1,
                                 pPrivate->pWidget->pGlobals->lcol3DDark,
                                 pPrivate->pWidget->pGlobals->lcol3DLight);
                /*
                rcGraph.yTop    --;
                rcGraph.yBottom ++;
                rcGraph.xRight  --;
                rcGraph.xLeft   ++;
                */

                pgpihDraw3DFrame(hps, &rcGraph, 1, 0x000000, 0xC8C8C8);
                // graph background
                rcGraph.yBottom++;
                rcGraph.xLeft++;
                WinFillRect(hps, &rcGraph, 0x008080);
                // percent free
                rcGraph.xRight = rcGraph.xLeft + (rcGraph.xRight - rcGraph.xLeft) * dPercentFree / 100;
                WinFillRect(hps, &rcGraph, 0x808080);
                // border around free
                if (dPercentFree>3)
                {
                    rcGraph.yTop    --;
                    rcGraph.xRight  --;
                    pgpihDraw3DFrame(hps, &rcGraph, 1, 0xFFFFFF, 0x000000);
                }
                #endif
            }
        }
        WinEndPaint(hps);  // V0.9.11 (2001-04-19) [pr]: Moved to correct place
    }
}

/*
 *@@ WgtPresParamChanged:
 *      implementation for WM_PRESPARAMCHANGED.
 *
 *      While this isn't exactly required, it's a nice
 *      thing for a widget to react to colors and fonts
 *      dropped on it. While we're at it, we also save
 *      these colors and fonts in our setup string data.
 *
 *@@changed V0.9.13 (2001-06-21) [umoeller]: changed XCM_SAVESETUP call for tray support
 *@@changed V1.0.8 (2007-08-05) [pr]: rewrote this mess @@fixes 994
 */

void WgtPresParamChanged(HWND hwnd,
                         ULONG ulAttrChanged,
                         PXCENTERWIDGET pWidget)
{
    PDISKFREEPRIVATE pPrivate;
    if (pPrivate = (PDISKFREEPRIVATE)pWidget->pUser)
    {
        BOOL fInvalidate = TRUE;

        switch (ulAttrChanged)  // V1.0.8 (2007-08-05) [pr]
        {
            case 0:     // layout palette thing dropped
                // update our setup data; the presparam has already
                // been changed, so we can just query it
                pPrivate->Setup.lcolBackground
                    = pwinhQueryPresColor(hwnd,
                                          PP_BACKGROUNDCOLOR,
                                          FALSE,
                                          SYSCLR_DIALOGBACKGROUND);
                pPrivate->Setup.lcolForeground
                    = pwinhQueryPresColor(hwnd,
                                          PP_FOREGROUNDCOLOR,
                                          FALSE,
                                          SYSCLR_WINDOWTEXT);
            break;

            case PP_BACKGROUNDCOLOR:    // background color (no ctrl pressed)
                pPrivate->Setup.lcolBackground
                    = pwinhQueryPresColor(hwnd,
                                          PP_BACKGROUNDCOLOR,
                                          FALSE,
                                          SYSCLR_DIALOGBACKGROUND);
            break;

            case PP_FOREGROUNDCOLOR:    // foreground color (ctrl pressed)
                pPrivate->Setup.lcolForeground
                    = pwinhQueryPresColor(hwnd,
                                          PP_FOREGROUNDCOLOR,
                                          FALSE,
                                          SYSCLR_WINDOWTEXT);
            break;

            case PP_FONTNAMESIZE:       // font dropped:
            {
                PSZ pszFont;
                if (pPrivate->Setup.pszFont)
                {
                    free(pPrivate->Setup.pszFont);
                    pPrivate->Setup.pszFont = NULL;
                }

                if (pszFont = pwinhQueryWindowFont(hwnd))
                {
                    // we must use local malloc() for the font;
                    // the winh* code uses a different C runtime
                    pPrivate->Setup.pszFont = strdup(pszFont);
                    pwinhFree(pszFont);
                }
            }
            break;

            default:
                fInvalidate = FALSE;
        }

        if (fInvalidate)
        {
            // something has changed:
            XSTRING strSetup;

            // repaint
            WinInvalidateRect(hwnd, NULL, FALSE);

            // recompose our setup string
            WgtSaveSetup(&strSetup,
                         &pPrivate->Setup);
            if (strSetup.ulLength)
                // we have a setup string:
                // tell the XCenter to save it with the XCenter data
                // changed V0.9.13 (2001-06-21) [umoeller]:
                // post it to parent instead of fixed XCenter client
                // to make this trayable
                WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), // pPrivate->pWidget->pGlobals->hwndClient,
                           XCM_SAVESETUP,
                           (MPARAM)hwnd,
                           (MPARAM)strSetup.psz);

            pxstrClear(&strSetup);
        }
    } // end if (pPrivate)
}

/*
 *@@ GetDriveInfo:
 *      Returns TRUE if the drive info has changed
 *      and the display should therefore be updated.
 *
 *@@changed V0.9.11 (2001-04-25) [umoeller]: added error checking
 *@@changed V1.0.2 (2003-12-10) [pr]: Fixed bad display if drive could not be accessed @@fixes 525
 */

BOOL GetDriveInfo(PDISKFREEPRIVATE pPrivate)
{
    double dOldDriveFree = pPrivate->dAktDriveFree;

    APIRET arc;

    strcpy(pPrivate->szAktDriveType,"???");
    pPrivate->bFSIcon = 0;
    pPrivate->dAktDriveFree = 0;
    pPrivate->dAktDriveSize = 0;
    if (!(arc = pdoshQueryDiskFSType(pPrivate->chAktDrive-64,
                                     (PSZ)pPrivate->szAktDriveType,
                                     sizeof(pPrivate->szAktDriveType))))
    {
        if (!strcmp("LAN", pPrivate->szAktDriveType))
            pPrivate->bFSIcon = 1;
        else if (!strcmp("CDFS", pPrivate->szAktDriveType))
            pPrivate->bFSIcon = 2;
        else
            pPrivate->bFSIcon = 0;

        if (    (!(arc = pdoshQueryDiskFree(pPrivate->chAktDrive-64,
                                            &pPrivate->dAktDriveFree)))
             && (!(arc = pdoshQueryDiskSize(pPrivate->chAktDrive-64,
                                            &pPrivate->dAktDriveSize)))
           )
            return((BOOL)pPrivate->dAktDriveFree != dOldDriveFree);
    }

    return TRUE;
}

/*
 *@@ GetDrive:
 *
 */

void GetDrive(HWND hwnd,
              PXCENTERWIDGET pWidget,
              BOOL fNext)
{
    PDISKFREEPRIVATE pPrivate;
    if (pPrivate = (PDISKFREEPRIVATE)pWidget->pUser)
    {
         // V0.9.11 (2001-04-19) [pr]: Rewrite to use drive char. rather than pointers
         CHAR *pCurrent = strchr(pPrivate->szDrives, pPrivate->chAktDrive);

         if (pCurrent == NULL)
             pPrivate->chAktDrive = *pPrivate->szDrives;
         else
         {
            if (fNext)
            {
                // return the next drive
                if (pCurrent >= pPrivate->szDrives+strlen(pPrivate->szDrives)-1)
                    pPrivate->chAktDrive = *pPrivate->szDrives;
                else
                    pPrivate->chAktDrive = *(pCurrent+1);
            }
            else
            {
                // return the prev drive
                if (pCurrent == pPrivate->szDrives)
                    pPrivate->chAktDrive = *(pPrivate->szDrives+strlen(pPrivate->szDrives)-1);
                else
                    pPrivate->chAktDrive = *(pCurrent-1);
            }
        }

        GetDriveInfo(pPrivate);

        WinInvalidateRect(hwnd,
                          NULLHANDLE,
                          TRUE);
    }
}

/*
 *@@ ValidateDrive:
 *      checks for valid drive letter or *, converts lower to upper case.
 *
 *@@added V0.9.11 (2001-04-19) [pr]
 */

CHAR ValidateDrive(CHAR chDrive)
{
    if (    (chDrive >= 'a')
         && (chDrive <= 'z')
       )
        chDrive &= ~0x20;

    if (    (chDrive != '*')
         && (    (chDrive < 'A')
              || (chDrive > 'Z')
            )
       )
        chDrive = 'C';

    return(chDrive);
}


/*
 *@@ WgtDestroy:
 *      implementation for WM_DESTROY.
 *
 *      This must clean up all allocated resources.
 */

void WgtDestroy(HWND hwnd,
                PXCENTERWIDGET pWidget)
{
    PDISKFREEPRIVATE pPrivate;
    if (pPrivate = (PDISKFREEPRIVATE)pWidget->pUser)
    {
        if (pPrivate->ulTimerID)
           ptmrStopXTimer((PXTIMERSET)pPrivate->pWidget->pGlobals->pvXTimerSet,
                          hwnd,
                          pPrivate->ulTimerID);

        WinDestroyPointer(pPrivate->hptrDrive);
        WinDestroyPointer(pPrivate->hptrHand);

        WinDestroyPointer(pPrivate->hptrDrives[0]);
        WinDestroyPointer(pPrivate->hptrDrives[1]);
        WinDestroyPointer(pPrivate->hptrDrives[2]);


        WgtClearSetup(&pPrivate->Setup);

        free(pPrivate);
                // pWidget is cleaned up by DestroyWidgets
    }
}

/*
 *@@ fnwpSampleWidget:
 *      window procedure for the Diskfree widget class.
 *
 *      There are a few rules which widget window procs
 *      must follow. See XCENTERWIDGETCLASS in center.h
 *      for details.
 *
 *      Other than that, this is a regular window procedure
 *      which follows the basic rules for a PM window class.
 *
 *@@changed V0.9.11 (2001-04-18) [umoeller]: couple of fixes for the winproc.
 *@@changed V0.9.19 (2002-05-07) [umoeller]: added double-click for open and timer, thanks yuri
 *@@changed V0.9.20 (2002-07-03) [umoeller]: width was never saved, fixed
 */

MRESULT EXPENTRY fnwpSampleWidget(HWND hwnd,
                                  ULONG msg,
                                  MPARAM mp1,
                                  MPARAM mp2)
{
    MRESULT mrc = 0;

    // get widget data from QWL_USER (stored there by WM_CREATE)
    PXCENTERWIDGET pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER); // this ptr is valid after WM_CREATE

    switch(msg)
    {
        /*
         * WM_CREATE:
         *      as with all widgets, we receive a pointer to the
         *      XCENTERWIDGET in mp1, which was created for us.
         *
         *      The first thing the widget MUST do on WM_CREATE
         *      is to store the XCENTERWIDGET pointer (from mp1)
         *      in the QWL_USER window word by calling:
         *
         *          WinSetWindowPtr(hwnd, QWL_USER, mp1);
         *
         *      We use XCENTERWIDGET.pUser for allocating
         *      DISKFREEPRIVATE for our own stuff.
         */

        case WM_CREATE:
            WinSetWindowPtr(hwnd, QWL_USER, mp1);
            pWidget = (PXCENTERWIDGET)mp1;
            if ((pWidget) && (pWidget->pfnwpDefWidgetProc))
                mrc = WgtCreate(hwnd, pWidget);
            else
                // stop window creation!!
                mrc = (MRESULT)TRUE;
        break;

        case WM_CONTROL:
            mrc = (MRESULT)WgtControl(hwnd, mp1, mp2);
        break;

        case WM_MOUSEMOVE:
            // This was wrong. You must call the default window procedure unless you
            // handle all cases.  V0.9.11 (2001-04-27) [pr]
            if (pWidget)
            {
                PDISKFREEPRIVATE pPrivate = (PDISKFREEPRIVATE)pWidget->pUser;

                if (pPrivate->Setup.chDrive == '*')
                {
                    WinSetPointer(HWND_DESKTOP, pPrivate->hptrHand);
                    mrc = (MRESULT)TRUE;        // V0.9.11 (2001-04-18) [umoeller]
                }

                if (!mrc)
                    mrc = pWidget->pfnwpDefWidgetProc(hwnd, msg, mp1, mp2);
            }
        break;

#ifdef YURIEXT
        case WM_BUTTON1DBLCLK:  // Open disk object
            if (pWidget)
            {
                PDISKFREEPRIVATE pPrivate = (PDISKFREEPRIVATE)pWidget->pUser;
                CHAR   szTemp[32];
                HOBJECT hObject;

                // stop click timer
                WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, ID_DBLCLKTIMERID);

                // object id
                pdrv_sprintf(szTemp, "<WP_DRIVE_%c>", pPrivate->chAktDrive);
                if (hObject = WinQueryObject(szTemp))
                   if (WinOpenObject(hObject, 0, TRUE))
                      WinOpenObject(hObject, 0, TRUE); // OPEN_DEFAULT
            }

            mrc = (MRESULT)TRUE;        // V0.9.11 (2001-04-18) [umoeller]
                                        // you processed the msg, so return TRUE
        break;

        case WM_BUTTON1CLICK:
        {
           ULONG dtTimeout = WinQuerySysValue(HWND_DESKTOP, SV_DBLCLKTIME) + 30;

           // start click timer
           WinStartTimer(WinQueryAnchorBlock(hwnd),
                         hwnd,
                         ID_DBLCLKTIMERID,
                         dtTimeout);
        }
        break;
#else
        case WM_BUTTON1CLICK:
        case WM_BUTTON1DBLCLK:
            if (pWidget)
            {
                PDISKFREEPRIVATE pPrivate = (PDISKFREEPRIVATE)pWidget->pUser;
                if (pPrivate->Setup.chDrive=='*')
                {
                    if (WinGetKeyState(HWND_DESKTOP, VK_CTRL) & 0x8000)
                        GetDrive(hwnd, pWidget, FALSE);
                    else
                        GetDrive(hwnd, pWidget, TRUE);
                }
            }

            mrc = (MRESULT)TRUE;        // V0.9.11 (2001-04-18) [umoeller]
                                        // you processed the msg, so return TRUE
        break;
#endif

        case WM_PAINT:
            WgtPaint(hwnd, pWidget);
        break;

        case WM_TIMER:
            if (pWidget)
            {
                if ((ULONG)mp1 == ID_DBLCLKTIMERID)
                {
                    PDISKFREEPRIVATE pPrivate = (PDISKFREEPRIVATE)pWidget->pUser;
                    // stop click timer
                    WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, ID_DBLCLKTIMERID);

                    if (pPrivate->Setup.chDrive == '*')
                    {
                        if (WinGetKeyState(HWND_DESKTOP, VK_CTRL) & 0x8000)
                            GetDrive(hwnd, pWidget, FALSE);
                        else
                            GetDrive(hwnd, pWidget, TRUE);
                    }
                }
                else
                {
                    // get private data from that widget data
                    PDISKFREEPRIVATE pPrivate = (PDISKFREEPRIVATE)pWidget->pUser;
                    // V0.9.11 (2001-04-19) [pr]: Update drive list
                    /* pdoshEnumDrives(pPrivate->szDrives,
                                    NULL,
                                    TRUE);      // skip removeables
                    */
                    // V0.9.11 (2001-04-25) [umoeller]: removed this again...
                    // this is outright dangerous.
                    // If this fails, e.g. because a CHKDSK is in progress, this
                    // is dangerous, because the user gets the white error box
                    // on each timer tick, making it almost impossible to close the
                    // XCenter. So if this failed for any reason, stop the timer.
                    if (GetDriveInfo(pPrivate)) // if values have changed update, display
                        WinInvalidateRect(hwnd, NULLHANDLE, TRUE);
                }
            }
        break;

        /*
         * WM_WINDOWPOSCHANGED:
         *      save new width in setup string V0.9.20 (2002-07-03) [umoeller].
         */

        case WM_WINDOWPOSCHANGED:
        {
            PSWP pswpNew = (PSWP)mp1,
                 pswpOld = pswpNew + 1;
            if (    (pswpNew->fl & SWP_SIZE)
                 && (pswpNew->cx != pswpOld->cx)
               )
            {
                XSTRING strSetup;
                PDISKFREEPRIVATE pPrivate = (PDISKFREEPRIVATE)pWidget->pUser;
                pPrivate->Setup.cx = pswpNew->cx;
                WgtSaveSetup(&strSetup,
                             &pPrivate->Setup);
                if (strSetup.ulLength)
                    WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
                               XCM_SAVESETUP,
                               (MPARAM)hwnd,
                               (MPARAM)strSetup.psz);
                pxstrClear(&strSetup);
            }
        }
        break;

        /*
         * WM_PRESPARAMCHANGED:
         *
         */

        case WM_PRESPARAMCHANGED:
            if (pWidget)
                // this gets sent before this is set!
                WgtPresParamChanged(hwnd, (ULONG)mp1, pWidget);
        break;

        /*
         * WM_DESTROY:
         *      clean up. This _must_ be passed on to
         *      ctrDefWidgetProc.
         */

        case WM_DESTROY:
            WgtDestroy(hwnd, pWidget);
            // we _MUST_ pass this on, or the default widget proc
            // cannot clean up.
            mrc = pWidget->pfnwpDefWidgetProc(hwnd, msg, mp1, mp2);
        break;

        default:
            mrc = pWidget->pfnwpDefWidgetProc(hwnd, msg, mp1, mp2);
    } // end switch(msg)

    return mrc;
}

/* ******************************************************************
 *
 *   Exported procedures
 *
 ********************************************************************/

/*
 *@@ WgtInitModule:
 *      required export with ordinal 1, which must tell
 *      the XCenter how many widgets this DLL provides,
 *      and give the XCenter an array of XCENTERWIDGETCLASS
 *      structures describing the widgets.
 *
 *      With this call, you are given the module handle of
 *      XFLDR.DLL. For convenience, you may resolve imports
 *      for some useful functions which are exported thru
 *      src\shared\xwp.def. See the code below.
 *
 *      This function must also register the PM window classes
 *      which are specified in the XCENTERWIDGETCLASS array
 *      entries. For this, you are given a HAB which you
 *      should pass to WinRegisterClass. For the window
 *      class style (4th param to WinRegisterClass),
 *      you should specify
 *
 +          CS_PARENTCLIP | CS_SIZEREDRAW | CS_SYNCPAINT
 *
 *      Your widget window _will_ be resized, even if you're
 *      not planning it to be.
 *
 *      This function only gets called _once_ when the widget
 *      DLL has been successfully loaded by the XCenter. If
 *      there are several instances of a widget running (in
 *      the same or in several XCenters), this function does
 *      not get called again. However, since the XCenter unloads
 *      the widget DLLs again if they are no longer referenced
 *      by any XCenter, this might get called again when the
 *      DLL is re-loaded.
 *
 *      There will ever be only one load occurence of the DLL.
 *      The XCenter manages sharing the DLL between several
 *      XCenters. As a result, it doesn't matter if the DLL
 *      has INITINSTANCE etc. set or not.
 *
 *      If this returns 0, this is considered an error, and the
 *      DLL will be unloaded again immediately.
 *
 *      If this returns any value > 0, *ppaClasses must be
 *      set to a static array (best placed in the DLL's
 *      global data) of XCENTERWIDGETCLASS structures,
 *      which must have as many entries as the return value.
 *
 *@@changed V0.9.11 (2001-04-18) [umoeller]: added more imports from dosh.c
 */

ULONG EXPENTRY WgtInitModule(HAB hab,         // XCenter's anchor block
                             HMODULE hmodPlugin, // module handle of the widget DLL
                             HMODULE hmodXFLDR,    // XFLDR.DLL module handle
                             PCXCENTERWIDGETCLASS *ppaClasses,
                             PSZ pszErrorMsg)  // if 0 is returned, 500 bytes of error msg
{
    ULONG   ulrc = 0,
            ul = 0;
    BOOL    fImportsFailed = FALSE;

    // resolve imports from XFLDR.DLL (this is basically
    // a copy of the doshResolveImports code, but we can't
    // use that before resolving...)
    for (ul = 0;
         ul < sizeof(G_aImports) / sizeof(G_aImports[0]); // array item count
         ul++)
    {
        if (DosQueryProcAddr(hmodXFLDR,
                             0,               // ordinal, ignored
                             (PSZ)G_aImports[ul].pcszFunctionName,
                             G_aImports[ul].ppFuncAddress)
                    != NO_ERROR)
        {
            strcpy(pszErrorMsg, "Import ");
            strcat(pszErrorMsg, G_aImports[ul].pcszFunctionName);
            strcat(pszErrorMsg, " failed.");
            fImportsFailed = TRUE;
            break;
        }
    }

    if (!fImportsFailed)
    {
        // all imports OK:
        // register our PM window class
        if (!WinRegisterClass(hab,
                              WNDCLASS_WIDGET_SAMPLE,
                              fnwpSampleWidget,
                              CS_PARENTCLIP | CS_SIZEREDRAW | CS_SYNCPAINT,
                              sizeof(PDISKFREEPRIVATE))
                                    // extra memory to reserve for QWL_USER
                             )
            strcpy(pszErrorMsg, "WinRegisterClass failed.");
        else
        {
            // no error:
            // return widget classes
            *ppaClasses = G_WidgetClasses;

            // return no. of classes in this DLL (one here):
            ulrc = sizeof(G_WidgetClasses) / sizeof(G_WidgetClasses[0]);
        }
    }

    return ulrc;
}

/*
 *@@ WgtUnInitModule:
 *      optional export with ordinal 2, which can clean
 *      up global widget class data.
 *
 *      This gets called by the XCenter right before
 *      a widget DLL gets unloaded. Note that this
 *      gets called even if the "init module" export
 *      returned 0 (meaning an error) and the DLL
 *      gets unloaded right away.
 */

void EXPENTRY WgtUnInitModule(void)
{
}


/*
 *@@ MwgtQueryVersion:
 *      this new export with ordinal 3 can return the
 *      XWorkplace version number which is required
 *      for this widget to run. For example, if this
 *      returns 0.9.10, this widget will not run on
 *      earlier XWorkplace versions.
 *
 *      NOTE: This export was mainly added because the
 *      prototype for the "Init" export was changed
 *      with V0.9.9. If this returns 0.9.9, it is
 *      assumed that the INIT export understands
 *      the new FNWGTINITMODULE_099 format (see center.h).
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 *@@changed V0.9.11 (2001-04-18) [umoeller]: now reporting 0.9.11 because we need the newer imports
 */

void EXPENTRY WgtQueryVersion(PULONG pulMajor,
                              PULONG pulMinor,
                              PULONG pulRevision)
{
    *pulMajor = XFOLDER_MAJOR;              // dlgids.h
    *pulMinor = XFOLDER_MINOR;
    *pulRevision = XFOLDER_REVISION;
}


