
/*
 * netscdde.c:
 *      this is the main (and only) C file for the
 *      Netscape DDE interface. This code is much more
 *      messy than XWorkplace's. It's a rather quick hack
 *      done in about two days with DDE code stolen from
 *      various places, so don't expect this code to be
 *      pretty.
 *
 *      Use the undocumented "-D" parameter on the command
 *      line to start NetscDDE in "debug" mode, which will
 *      display a frame window with a menu where you may
 *      debug the DDE messages. This window is invisible
 *      when "-D" is not used. (Ugly, huh.)
 *
 *      Netscape's DDE topics are (horribly) documented
 *      for all Netscape versions at
 *      http://developer.netscape.com/library/documentation/communicator/DDE
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
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

#define INCL_DOSMEMMGR
#define INCL_DOSMODULEMGR
#define INCL_DOSSESMGR
#define INCL_DOSMISC

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINDIALOGS
#define INCL_WINTIMER
#define INCL_WINPOINTERS
#define INCL_WINSHELLDATA
#define INCL_WINPROGRAMLIST
#define INCL_WINLISTBOXES
#define INCL_WINDDE
#include <os2.h>

#include <stdio.h>
#include <string.h>

#define DONT_REPLACE_FOR_DBCS       // do not replace strchr with DBCS version
#define DONT_REPLACE_MALLOC
#include "setup.h"

#include "xwpapi.h"
#include "shared\helppanels.h"
#include "netscdde.h"
#include "dlgids.h"

MRESULT EXPENTRY fnwpMain(HWND, ULONG, MPARAM, MPARAM);

HAB             G_hab;
HWND            G_hwndDebug = NULLHANDLE,
                G_hwndListbox = NULLHANDLE,
                G_hServerWnd = NULLHANDLE;
PFNWP           G_SysWndProc;

// NLS Resource DLL
CHAR            G_szNLSDLL[2*CCHMAXPATH];
HMODULE         G_hmodNLS = NULLHANDLE;

CHAR            G_szURL[400] = "";

ULONG           G_idTimer = 0;

CONVCONTEXT     G_context;

const char      DEFAULT_BROWSER[] = "NETSCAPE.EXE";
const char      NETSCDDE_TITLE[] = "Netscape DDE";

CHAR            G_szDDENetscape[] = "NETSCAPE",   // DDE server name
                G_szNetscapeApp[CCHMAXPATH] = "", // default program to start
                                                        // if not running
                G_szNetscapeParams[CCHMAXPATH] = "",  // space for params

                G_szStartupDir[CCHMAXPATH] = "";        // startup dir V0.9.16 (2001-10-02) [umoeller]
PSZ             G_szOpenURLTopic = "WWW_OpenURL";     // open URL DDE topic

                                                        // (see Netscape docs)

// options flags, modified by command line interface
BOOL            G_optNewWindow = FALSE,
                G_optDebug = FALSE,
                G_optExecute = TRUE,
                G_optConfirmStart = TRUE,
                G_optMinimized = FALSE,
                G_optHidden = FALSE,
                G_optQuiet = FALSE;           // "-q", don't show status windows

BOOL            G_NetscapeFound = FALSE;

// status window handle
HWND            G_hwndContacting = NULLHANDLE;

VOID GetNLSString(PSZ pszBuf,
                  ULONG cbBuf,
                  ULONG id)
{
    if (!WinLoadString(G_hab,
                       G_hmodNLS,
                       id,
                       cbBuf,
                       pszBuf))
        sprintf(pszBuf,
                "Cannot load string resource %d from \"%s\"",
                id,
                G_szNLSDLL);
}

/*
 *@@ strhistr:
 *      like strstr, but case-insensitive.
 *
 *@@changed V0.9.0 [umoeller]: crashed if null pointers were passed, thanks Rdiger Ihle
 */

PSZ strhistr(PCSZ string1, PCSZ string2)
{
    PSZ prc = NULL;

    if ((string1) && (string2))
    {
        PSZ pszSrchIn = strdup(string1);
        PSZ pszSrchFor = strdup(string2);

        if ((pszSrchIn) && (pszSrchFor))
        {
            strupr(pszSrchIn);
            strupr(pszSrchFor);

            if (prc = strstr(pszSrchIn, pszSrchFor))
            {
                // prc now has the first occurence of the string,
                // but in pszSrchIn; we need to map this
                // return value to the original string
                prc = (prc-pszSrchIn) // offset in pszSrchIn
                      + (PSZ)string1;
            }
        }
        if (pszSrchFor)
            free(pszSrchFor);
        if (pszSrchIn)
            free(pszSrchIn);
    }
    return prc;
}

/*
 *@@ strlcpy:
 *      copies src to string dst of size siz.  At most siz-1 characters
 *      will be copied.  Always NUL terminates, unless siz == 0.
 *
 *      Returns strlen(src); if retval >= siz, truncation occurred.
 *
 *      Taken from the OpenBSD sources at
 *
 +          ftp://ftp.openbsd.org/pub/OpenBSD/src/lib/libc/string/strlcpy.c
 *
 *      Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *      All rights reserved.
 *
 *      OpenBSD licence applies (see top of that file).
 *
 *@@added V1.0.1 (2003-01-29) [umoeller]
 */

size_t strlcpy(char *dst,
               const char *src,
               size_t siz)
{
    register char       *d = dst;
    register const char *s = src;
    register size_t     n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0)
    {
        do
        {
            if ((*d++ = *s++) == 0)
                break;
        } while (--n != 0);
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0)
    {
        if (siz != 0)
            *d = '\0';      /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return (s - src - 1);    /* count does not include NUL */
}

/*
 *@@ strlcat:
 *      appends src to string dst of size siz. Unlike strncat,
 *      siz is the full size of dst, not space left. At most
 *      siz-1 characters will be copied.  Always NUL terminates,
 *      unless siz <= strlen(dst).
 *
 *      Returns strlen(src) + MIN(siz, strlen(initial dst)),
 *      in other words, strlen(dst) after the concatenation.
 *      If retval >= siz, truncation occurred.
 *
 *      Taken from the OpenBSD sources at
 *
 +          ftp://ftp.openbsd.org/pub/OpenBSD/src/lib/libc/string/strlcat.c
 *
 *      Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *      All rights reserved.
 *
 *      OpenBSD licence applies (see top of that file).
 *
 *@@added V1.0.1 (2003-01-29) [umoeller]
 */

size_t strlcat(char *dst,
               const char *src,
               size_t siz)
{
    register char       *d = dst;
    register const char *s = src;
    register size_t     n = siz;
    size_t              dlen;

    /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != '\0')
        d++;
    dlen = d - dst;
    n = siz - dlen;

    if (n == 0)
        return(dlen + strlen(s));
    while (*s != '\0')
    {
        if (n != 1)
        {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';

    return (dlen + (s - src));   /* count does not include NUL */
}

/*
 * ShowMessage:
 *      add a string to the listbox.
 */

void ShowMessage(PCSZ pcszFormat, ...)
{
    CHAR szbuf[1000];
    va_list     args;
    va_start(args, pcszFormat);
    vsprintf(szbuf, pcszFormat, args);
    va_end(args);

    WinSendMsg(G_hwndListbox,
               LM_INSERTITEM,
               MPFROMSHORT(LIT_END),
               szbuf);
}

/*
 *@@ DisplayError:
 *
 *@@added V0.9.16 (2001-10-02) [umoeller]
 */

VOID DisplayError(PCSZ pcszFormat, ...)
{
    CHAR szbuf[1000];
    va_list     args;
    va_start(args, pcszFormat);
    vsprintf(szbuf, pcszFormat, args);
    va_end(args);

    WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                  szbuf,
                  (PSZ)NETSCDDE_TITLE,
                  0,
                  MB_CANCEL | MB_MOVEABLE);
    if (G_hwndDebug)
        WinPostMsg(G_hwndDebug, WM_QUIT, 0, 0);
}

/*
 * CenterWindow:
 *      centers a window within its parent window.
 *      The window should not be visible to avoid flickering.
 */

void CenterWindow(HWND hwnd)
{
    RECTL           rclParent;
    RECTL           rclWindow;

    WinQueryWindowRect(hwnd, &rclWindow);
    WinQueryWindowRect(WinQueryWindow(hwnd, QW_PARENT), &rclParent);

    rclWindow.xLeft = (rclParent.xRight - rclWindow.xRight) / 2;
    rclWindow.yBottom = (rclParent.yTop - rclWindow.yTop) / 2;

    WinSetWindowPos(hwnd, NULLHANDLE, rclWindow.xLeft, rclWindow.yBottom,
                    0, 0, SWP_MOVE | SWP_SHOW);
}

/*
 * WinCenteredDlgBox:
 *      just like WinDlgBox, but the window is centered
 */

ULONG WinCenteredDlgBox(HWND hwndParent,
                        HWND hwndOwner,
                        PFNWP pfnDlgProc,
                        HMODULE hmod,
                        ULONG idDlg,
                        PVOID pCreateParams)
{
    ULONG           ulReply = DID_CANCEL;
    HWND            hwndDlg = WinLoadDlg(hwndParent, hwndOwner, pfnDlgProc,
                                         hmod, idDlg, pCreateParams);

    if (hwndDlg)
    {
        CenterWindow(hwndDlg);
        ulReply = WinProcessDlg(hwndDlg);
        WinDestroyWindow(hwndDlg);
    }
    else
        DisplayError("Cannot load dialog");

    return ulReply;
}

/*
 * ExplainParams:
 *      this displays the dlg box which explains
 *      NetscDDE's usage in brief; called when the
 *      parameters on the cmd line don't seem to
 *      be complete
 *
 *@@changed V0.9.16 (2001-10-02) [umoeller]: now displaying xwp help panel
 */

VOID ExplainParams(VOID)
{
    BOOL f = FALSE;
    CHAR szMsg[256];
    GetNLSString(szMsg, sizeof(szMsg), ID_NDSI_SYNTAX);
    if (WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                      szMsg,
                      (PSZ)NETSCDDE_TITLE,
                      0, MB_YESNO | MB_MOVEABLE)
            == MBID_YES)
    {
        // tell xwp to display the help
        PXWPGLOBALSHARED   pXwpGlobalShared = NULL;
        if (!(DosGetNamedSharedMem((PVOID*)&pXwpGlobalShared,
                                   SHMEM_XWPGLOBAL,
                                   PAG_READ | PAG_WRITE)))
        {
            f = WinPostMsg(pXwpGlobalShared->hwndAPIObject,
                           APIM_SHOWHELPPANEL,
                           (MPARAM)ID_XSH_NETSCAPEDDE,
                           0);
            DosFreeMem(pXwpGlobalShared);
        }

        if (!f)
            DisplayError("Cannot open the %s online help. Maybe XWorkplace is not installed or not running.",
                         NETSCDDE_TITLE);
    }
}

/*
 *@@ LoadNLS:
 *      NetscDDE NLS interface.
 *
 *@@added V0.9.1 (99-12-19) [umoeller]
 */

BOOL LoadNLS(VOID)
{
    BOOL Proceed = TRUE;

    if (PrfQueryProfileString(HINI_USER,
                              "XWorkplace",
                              "XFolderPath",
                              "",
                              G_szNLSDLL,
                              sizeof(G_szNLSDLL))
                   < 3)

    {
        DisplayError("NetscapeDDE was unable to determine the location of the "
                     "XWorkplace National Language Support DLL, which is "
                     "required for operation. The OS2.INI file does not contain "
                     "this information. "
                     "NetscapeDDE cannot proceed. Please re-install XWorkplace.");
        Proceed = FALSE;
    }
    else
    {
        CHAR    szLanguageCode[50] = "";

        // now compose module name from language code
        PrfQueryProfileString(HINI_USERPROFILE,
                              "XWorkplace", "Language",
                              "001",
                              (PVOID)szLanguageCode,
                              sizeof(szLanguageCode));
        // allow '?:\' for boot drive
        // V0.9.19 (2002-06-08) [umoeller]
        if (G_szNLSDLL[0] == '?')
        {
            ULONG ulBootDrive;
            DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                            &ulBootDrive,
                            sizeof(ulBootDrive));
            G_szNLSDLL[0] = (CHAR)ulBootDrive + 'A' - 1;
        }

        strcat(G_szNLSDLL, "\\bin\\xfldr");
        strcat(G_szNLSDLL, szLanguageCode);
        strcat(G_szNLSDLL, ".dll");

        // try to load the module
        if (DosLoadModule(NULL,
                          0,
                          G_szNLSDLL,
                          &G_hmodNLS))
        {
            DisplayError("NetscapeDDE was unable to load \"%s\", "
                         "the National Language DLL which "
                         "is specified for XWorkplace in OS2.INI.",
                         G_szNLSDLL);
            Proceed = FALSE;
        }
    }

    return Proceed;
}

/*
 * main:
 *      program entry point; accepts URLs on the command line.
 *
 *@@changed V0.9.1 (2000-02-07) [umoeller]: added "-q" option
 *@@changed V0.9.19 (2002-03-28) [umoeller]: now using WPUrl default browser settings
 */

int main(int argc,
         char *argv[])
{
    HMQ             hmq;
    FRAMECDATA      fcd;
    QMSG            qmsg;
    BOOL            Proceed = TRUE;

    if (!(G_hab = WinInitialize(0)))
        return 1;

    if (!(hmq = WinCreateMsgQueue(G_hab, 0)))
        return 1;

    // now attempt to find the XWorkplace NLS resource DLL,
    // which we need for all resources (new with XWP 0.9.0)
    Proceed = LoadNLS();

    // load browser path from USER_INI
    // V0.9.19 (2002-03-28) [umoeller]
    if (PrfQueryProfileString(HINI_USERPROFILE,
                              "WPURLDEFAULTSETTINGS", "DefaultBrowserExe",
                              (PSZ)DEFAULT_BROWSER,
                              G_szNetscapeApp,
                              sizeof(G_szNetscapeApp)))
    {
        // make sure this is not NETSCDDE.EXE, or we'll have
        // plenty of problems restarting ourselves infinitely
        // V0.9.19 (2002-04-02) [umoeller]
        if (strhistr(G_szNetscapeApp, "netscdde.exe"))
            strlcpy(G_szNetscapeApp, DEFAULT_BROWSER, sizeof(G_szNetscapeApp));
    }

    // load browser startup dir from USER_INI
    // V0.9.19 (2002-03-28) [umoeller]
    PrfQueryProfileString(HINI_USERPROFILE,
                          "WPURLDEFAULTSETTINGS", "DefaultWorkingDir",
                          "",
                          G_szStartupDir,
                          sizeof(G_szStartupDir));

    if (Proceed)
    {
        // parse parameters on cmd line
        if (argc > 1)
        {
            SHORT           i = 0;

            while (i++ < argc - 1)
            {
                if (argv[i][0] == '-')
                {
                    SHORT           i2;

                    for (i2 = 1; i2 < strlen(argv[i]); i2++)
                    {
                        switch (argv[i][i2])
                        {
                            case 'n':
                                G_optNewWindow = TRUE;
                                break;

                            case 'x':
                                G_optExecute = FALSE;
                                break;

                            case 'm':
                                G_optMinimized = TRUE;
                                break;

                            case 'h':
                                G_optHidden = TRUE;
                                break;

                            case 'X':
                                G_optConfirmStart = FALSE;
                                break;

                            case 'p':   // netscape path

                                if (i < argc)
                                {
                                    strlcpy(G_szNetscapeApp, argv[i + 1], sizeof(G_szNetscapeApp));
                                    i++;
                                    i2 = 1000;
                                }
                                else
                                {
                                    ExplainParams();
                                    Proceed = FALSE;
                                }
                                break;

                            case 's':   // startup dir V0.9.16 (2001-10-02) [umoeller]
                            {
                                if (i < argc)
                                {
                                    strlcpy(G_szStartupDir, argv[i + 1], sizeof(G_szStartupDir));
                                    i++;
                                    i2 = 1000;
                                }
                                else
                                {
                                    ExplainParams();
                                    Proceed = FALSE;
                                }
                                break;
                            }

                            case 'S':   // DDE server name V0.9.16 (2001-10-02) [umoeller]
                                if (i < argc)
                                {
                                    strlcpy(G_szDDENetscape, argv[i + 1], sizeof(G_szDDENetscape));
                                    i++;
                                    i2 = 1000;
                                }
                                else
                                {
                                    ExplainParams();
                                    Proceed = FALSE;
                                }
                                break;

                            case 'P':   // netscape parameters

                                if (i < argc)
                                {
                                    strlcpy(G_szNetscapeParams, argv[i + 1], sizeof(G_szNetscapeParams));
                                    i++;
                                    i2 = 1000;
                                }
                                else
                                {
                                    ExplainParams();
                                    Proceed = FALSE;
                                }
                                break;

                            case 'D':   // debug, show list box window w/ DDE msgs

                                G_optDebug = TRUE;
                                break;

                            case 'q': // added V0.9.1 (2000-02-07) [umoeller]
                                G_optQuiet = TRUE;
                                break;

                            default:    // unknown parameter
                                ExplainParams();
                                Proceed = FALSE;
                                break;
                        }
                    }
                }
                else
                {
                    // no option ("-"): seems to be URL
                    _PmpfF(("got URL [%s]", argv[i]));
                    // if the URL contains spaces, we enclose it in quotes
                    // V1.0.0 (2002-09-09) [umoeller]: not only spaces,
                    // but other weird characters too
                    if (strpbrk(argv[i], " &|="))
                        sprintf(G_szURL, "\"%s\"", argv[i]);
                    else
                        strlcpy(G_szURL, argv[i], sizeof(G_szURL));
                }
            }
        }

        if (strlen(G_szURL) == 0)
        {
            // no URL given: explain
            ExplainParams();
            Proceed = FALSE;
        }

        if (Proceed)
        {
            // OK, parameters seemed to be correct:
            // create the main window, which is only
            // visible in Debug mode ("-D" param). But
            // even if not in debug mode, this window is
            // used for DDE message processing.
            fcd.cb = sizeof(FRAMECDATA);
            fcd.flCreateFlags = FCF_TITLEBAR |
                                   FCF_SYSMENU |
                                   FCF_MENU |
                                   FCF_SIZEBORDER |
                                   FCF_SHELLPOSITION |
                                   FCF_MINMAX |
                                   FCF_TASKLIST;

            fcd.hmodResources = NULLHANDLE;
            // set our resource key (so PM can find menus, icons, etc).
            fcd.idResources = DDEC;
            // create the frame
            G_hwndDebug = WinCreateWindow(HWND_DESKTOP,
                                          WC_FRAME,
                                          (PSZ)NETSCDDE_TITLE,
                                          0, 0, 0, 0, 0,
                                          NULLHANDLE,
                                          HWND_TOP,
                                          DDEC,
                                          &fcd,
                                          NULL);

            if (!G_hwndDebug)
                return 1;

            // set the NetscDDE icon for the frame window
            WinSendMsg(G_hwndDebug,
                       WM_SETICON,
                       (MPARAM)WinLoadPointer(HWND_DESKTOP, G_hmodNLS,
                                              ID_ND_ICON),
                       NULL);

            // create a list window child
            G_hwndListbox = WinCreateWindow(G_hwndDebug,
                                            WC_LISTBOX,
                                            NULL,
                                            LS_HORZSCROLL,
                                            0, 0, 0, 0,
                                            G_hwndDebug,
                                            HWND_BOTTOM,
                                            FID_CLIENT,
                                            NULL,
                                            NULL);

            // we must intercept the frame window's messages;
            // we save the return value (the current WndProc),
            // so we can pass it all the other messages the frame gets.
            G_SysWndProc = WinSubclassWindow(G_hwndDebug, (PFNWP) fnwpMain);

            // the window we just created is normally invisible; we
            // will only display it if the (undocumented) "-D" option
            // was given on the command line.
            if (G_optDebug)
            {
                WinShowWindow(G_hwndDebug, TRUE);
                ShowMessage("Entering msg loop");
            }

            // now show "Contacting Netscape"
            if (!G_optQuiet)
            {
                G_hwndContacting = WinLoadDlg(HWND_DESKTOP, G_hwndDebug,
                                            WinDefDlgProc,
                                            G_hmodNLS, ID_NDD_CONTACTING,
                                            0);
                WinShowWindow(G_hwndContacting, TRUE);
            }

            // now post msg to main window to initiate DDE
            if (!G_optDebug)
                WinPostMsg(G_hwndDebug, WM_COMMAND, MPFROM2SHORT(IDM_INITIATE, 0), 0);

            //  standard PM message loop
            while (WinGetMsg(G_hab, &qmsg, NULLHANDLE, 0, 0))
            {
                WinDispatchMsg(G_hab, &qmsg);
            }
        }                           // end if (proceed)

        // clean up on the way out
        WinDestroyWindow(G_hwndContacting);
        G_hwndContacting = NULLHANDLE;
    }

    WinDestroyMsgQueue(hmq);
    WinTerminate(G_hab);

    return 0;
}

/*
 * DDERequest:
 *      this routine tries to post a DDE request to Netscape
 *      and returns TRUE only if this was successful.
 */

BOOL DDERequest(HWND hwndClient,
                PSZ pszItemString)
{
    ULONG           mem;
    PID             pid;
    TID             tid;
    PDDESTRUCT      pddeStruct;
    PSZ             pszDDEItemName;

    // get some sharable memory
    DosAllocSharedMem((PVOID)&mem,
                      NULL,
                      sizeof(DDESTRUCT) + 1000,
                      PAG_COMMIT |
                      PAG_READ |
                      PAG_WRITE |
                      OBJ_GIVEABLE);

    // get the server's ID and give it access
    // to the shared memory
    WinQueryWindowProcess(G_hServerWnd, &pid, &tid);
    DosGiveSharedMem(&mem, pid, PAG_READ | PAG_WRITE);

    /* here is definition for DDESTRUCT, for further reference:
     * typedef struct _DDESTRUCT {
     * ULONG    cbData;
     * This is the length of data that occurs after the offabData parameter. If no
     * data exists, this field should contain a zero (0).
     * USHORT   fsStatus;       /  Status of the data exchange.
     * DDE_FACK
     * Positive acknowledgement
     * DDE_FBUSY
     * Application is busy
     * DDE_FNODATA
     * No data transfer for advise
     * DDE_FACKREQ
     * Acknowledgements are requested
     * DDE_FRESPONSE
     * Response to WM_DDE_REQUEST
     * DDE_NOTPROCESSED
     * DDE message not understood
     * DDE_FAPPSTATUS
     * A 1-byte field of bits that are reserved for application-specific returns.
     * USHORT   usFormat;       /  Data format.
     * USHORT   offszItemName;  /  Offset to item name.
     * This is the offset to the item name from the start of this structure. Item
     * name is a null (0x00) terminated string. If no item name exists, there must
     * be a single null (0x00) character in this position. (That is, ItemName is
     * ALWAYS a null terminated string.)
     *
     * USHORT   offabData;      /  Offset to beginning of data.
     * This is the offset to the data, from the start of this structure. This field
     * should be calculated regardless of the presence of data. If no data exists,
     * cbData must be zero (0).
     *
     * For compatibility reasons, this data should not contain embedded pointers.
     * Offsets should be used instead.
     *
     * --  CHAR     szItemName[]    /  offset: offszItemName
     * --  BYTE     abData[]        /  offset: offabData
     * } DDESTRUCT; */

    // setup DDE data structures
    pddeStruct = (PDDESTRUCT) mem;
    pddeStruct->fsStatus = 0;   // DDE_FACKREQ;            // Status

    pddeStruct->usFormat = DDEFMT_TEXT;     // Text format

    // go past end of data structure for the item name
    pddeStruct->offszItemName = sizeof(DDESTRUCT);

    pszDDEItemName = ((BYTE*)pddeStruct) + pddeStruct->offszItemName;
    strcpy(pszDDEItemName, pszItemString);

    // go past end of data structure
    // (plus past the name) for the data
    pddeStruct->offabData = strlen(pszDDEItemName) + 1;
    // offset to BEGINNING of data
    pddeStruct->cbData = 500;
    // length of the data

    ShowMessage(__FUNCTION__ ": sending request \"%s\"",
                pszItemString);

    // post our request to the server program
    if (G_NetscapeFound = WinDdePostMsg(G_hServerWnd,
                                        hwndClient,
                                        WM_DDE_REQUEST,
                                        pddeStruct,
                                        0))
                    // WinDdePostMsg frees the shared mem!
        ShowMessage("    --> success");
    else
        ShowMessage("    --> failed");

    return G_NetscapeFound;
}

/*
 * fnwpMain:
 *      window procedure for the main NetscDDE window, which
 *      is only visible in Debug mode ("-D" option), mostly
 *      processing DDE messages. If we're in debug mode, this
 *      routine waits for certain menu selections, otherwise
 *      the corresponding messages will be posted automatically.
 *
 *@@changed V0.9.4 (2000-07-10) [umoeller]: added DDE conflicts fix by Rousseau de Pantalon
 *@@changed V0.9.19 (2002-03-28) [umoeller]: Opera fix
 *@@changed V0.9.19 (2002-04-02) [umoeller]: replaced ugly message box
 */

MRESULT EXPENTRY fnwpMain(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PSZ             szData;
    PDDESTRUCT      pddeStruct;
    ULONG           mem;

    CHAR            szBuffer[200];

    switch (msg)
    {

        // all answers to the WinDDEInitate call arrive here
        case WM_DDE_INITIATEACK:
        {
            PDDEINIT        pddeInit;
            PSZ             szInApp, szInTopic;
            static BOOL bNetscapeAnswered = FALSE;

            pddeInit = (PDDEINIT)mp2;
            szInApp = pddeInit->pszAppName;
            szInTopic = pddeInit->pszTopic;
            G_hServerWnd = (HWND)mp1;

            ShowMessage("WM_DDE_INITIATEACK (resp to WinDDEInitiate)");
            ShowMessage("  application: \"%s\"",
                        pddeInit->pszAppName);
            ShowMessage("  topic: \"%s\"",
                        pddeInit->pszTopic);

            // RDP 2000-07-07 07:24:18
            // There was no check on which application responded.
            // This made NETSCDDE fail when another DDE-aware application,
            // like EPM, was running.
            // Now the handle from mp1 is only assigned if the application
            // responding is Netscape.
            // If the app is not Netscape then the handle is nullified.
            // I don't know if assigning 0 to the handle is correct but
            // is seems to solve the problem.

            // V0.9.19 (2002-03-28) [umoeller]
            // Opera fix: use stricmp instead of strcmp
            if (!stricmp(pddeInit->pszAppName,
                         G_szDDENetscape))     // V0.9.16 (2001-10-02) [umoeller]
            {
                // ShowMessage("!! Netscape answered.");
                G_hServerWnd = (HWND)mp1;
                bNetscapeAnswered = TRUE;
            }
            else
            {
                // ShowMessage("!! Other application aswered.");
                G_hServerWnd = (HWND)0;
            }
        }
        break;

        // all answers to DDE requests arrive here
        case WM_DDE_DATA:
        {
            ShowMessage("!! Received data from Netscape: ");
            pddeStruct = (PDDESTRUCT) mp2;
            DosGetSharedMem(pddeStruct, PAG_READ | PAG_WRITE);
            szData = (BYTE *) (pddeStruct + (pddeStruct->offabData));
            ShowMessage(szData);
        }
        break;

        // menu item processing (in debug mode, otherwise these
        // WM_COMMAND msgs have been posted automatically)
        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                // start DDE conversation: this was posted
                // by "main" before the PM loop was entered
                // (even if we're in debug mode)
                case IDM_INITIATE:
                    // WinPostMsg(G_hwndListbox, LM_DELETEALL, 0, 0);
                    ShowMessage("IDM_INITIATE:");
                    ShowMessage("Topic: \"%s\"", G_szOpenURLTopic);
                    G_context.cb = sizeof(CONVCONTEXT);
                    G_context.fsContext = 0;
                    WinDdeInitiate(hwndFrame, G_szDDENetscape,
                                   G_szOpenURLTopic, &G_context);
                    if (!G_optDebug)
                        // if we're not in debug mode, post subsequent
                        // menu commands automatically
                        WinPostMsg(hwndFrame, WM_COMMAND, MPFROM2SHORT(IDM_CHAIN2, 0), 0);
                break;

                // "Open URL": request data from server
                case IDM_OPENURL:
                {
                    ShowMessage("IDM_OPENURL");
                    ShowMessage("  URL: \"%s\"", G_szURL);
                    strlcpy(szBuffer, G_szURL, sizeof(szBuffer));
                    strlcat(szBuffer, ",,0xFFFFFFFF,0x0", sizeof(szBuffer));
                    DDERequest(hwndFrame, szBuffer);
                }
                break;

                // "Open URL in new window": request data from server,
                // but with different parameters
                case IDM_OPENURLNEW:
                {
                    ShowMessage("IDM_OPENURLNEW");
                    ShowMessage("  URL: \"%s\"", G_szURL);
                    strlcpy(szBuffer, G_szURL, sizeof(szBuffer));
                    strlcat(szBuffer, ",,0x0,0x0", sizeof(szBuffer));
                    DDERequest(hwndFrame, szBuffer);
                }
                break;

                /*
                 * IDM_CHAIN2:
                 *      this is posted after DDE_INITIATE was
                 *      successful
                 */

                case IDM_CHAIN2:
                {
                    if (G_optNewWindow)
                        WinPostMsg(G_hwndDebug, WM_COMMAND, MPFROM2SHORT(IDM_OPENURLNEW, 0), 0);
                    else
                        WinPostMsg(G_hwndDebug, WM_COMMAND, MPFROM2SHORT(IDM_OPENURL, 0), 0);
                    WinPostMsg(G_hwndDebug, WM_COMMAND, MPFROM2SHORT(IDM_CHAIN3, 0), 0);
                }
                break;

                /*
                 * IDM_CHAIN3:
                 *      this is posted to close the whole thing; we just need
                 *      another msg before going for IDM_CLOSE, or some DDE
                 *      msgs might get lost
                 */

                case IDM_CHAIN3:
                    WinPostMsg(G_hwndDebug, WM_COMMAND, MPFROM2SHORT(IDM_CLOSE, 0), 0);
                break;

                case IDM_FULLSEQUENCE:
                    WinPostMsg(G_hwndDebug, WM_COMMAND, MPFROM2SHORT(IDM_INITIATE, 0), 0);
                    WinPostMsg(G_hwndDebug, WM_COMMAND, MPFROM2SHORT(IDM_CHAIN2, 0), 0);
                break;


                /*
                 * IDM_CLOSE:
                 *      this is posted to close the whole thing
                 */

                case IDM_CLOSE:
                    WinDdePostMsg(G_hServerWnd,
                                  hwndFrame,
                                  WM_DDE_TERMINATE,
                                  NULL,
                                  DDEPM_RETRY);
                    ShowMessage("DDE connection closed.");

                    if (!G_optDebug)
                        WinPostMsg(hwndFrame, WM_COMMAND, MPFROM2SHORT(IDM_DELAYEXIT, 0), 0);
                break;

                /*
                 * IDM_DELAYEXIT:
                 *      this is posted after IDM_CLOSE; we will now
                 *      check for whether the DDE conversation with
                 *      Netscape was successful and, if not, start
                 *      a new instance of Netscape according to the
                 *      command line parameters
                 */

                case IDM_DELAYEXIT:
                {
                    if (    (!G_NetscapeFound)
                         && (G_optExecute)
                       )
                    {
                        CHAR szStart[256];
                        GetNLSString(szStart, sizeof(szStart), ID_NDSI_STARTNETSCAPE);

                        // confirm start netscape
                        if (    (!G_optConfirmStart)
                             // get rid of this hideously ugly dialog
                             || (WinMessageBox(HWND_DESKTOP,
                                               HWND_DESKTOP,
                                               szStart,
                                               (PSZ)NETSCDDE_TITLE,
                                               0,
                                               MB_YESNO | MB_MOVEABLE)
                                       == MBID_YES)
                                               /* WinCenteredDlgBox(HWND_DESKTOP,
                                                   G_hwndDebug,
                                                   WinDefDlgProc,
                                                   G_hmodNLS,
                                                   ID_NDD_QUERYSTART,
                                                   NULL)
                                      == DID_OK) */
                           )
                        {
                            UCHAR           achObjBuf[256] = "";

                            CHAR            szArgs[CCHMAXPATH];

                            HWND            hwndNotify = HWND_DESKTOP;
                            PROGDETAILS     pd;
                            HAPP            happ;

                            // destroy "Contacting", create "Starting Netscape"
                            // window
                            WinDestroyWindow(G_hwndContacting);
                            G_hwndContacting = NULLHANDLE;

                            if (!G_optQuiet)
                            {
                                G_hwndContacting = WinLoadDlg(HWND_DESKTOP, G_hwndDebug,
                                                            WinDefDlgProc,
                                                            G_hmodNLS, ID_NDD_STARTING,
                                                            0);
                                WinShowWindow(G_hwndContacting, TRUE);
                            }

                            strlcpy(szArgs, G_szNetscapeParams, sizeof(szArgs));
                            strlcat(szArgs, " ", sizeof(szArgs));
                            strlcat(szArgs, G_szURL, sizeof(szArgs));

                            // now start app
                            memset(&pd, 0, sizeof(pd));
                            pd.Length = sizeof(pd);
                            pd.progt.progc = PROG_DEFAULT;
                            pd.progt.fbVisible = SHE_VISIBLE;
                            pd.pszExecutable = G_szNetscapeApp;
                            pd.pszParameters = szArgs;
                            pd.pszStartupDir = G_szStartupDir;

                            if (!(happ = WinStartApp(NULLHANDLE,
                                                     &pd,
                                                     szArgs,
                                                     NULL,
                                                     SAF_INSTALLEDCMDLINE)))
                            {
                                DisplayError("WinStartApp failed for app \"%s\", params \"%s\", startup dir \"%s\"",
                                             G_szNetscapeApp,
                                             szArgs,
                                             G_szStartupDir);
                            }
                        }
                    }
                    // keep "Contacting" / "Starting" window visible for two seconds
                    G_idTimer = WinStartTimer(G_hab, hwndFrame, 1, 2000);
                    break;
                }

                // User closes the window
                case IDM_EXIT:
                    WinPostMsg(hwndFrame, WM_CLOSE, 0, 0);
                break;
            }
            break;

        case WM_TIMER:
            // after two seconds, close status window
            WinStopTimer(G_hab, hwndFrame, G_idTimer);
            WinPostMsg(hwndFrame, WM_CLOSE, 0, 0);
        break;

        // Send the message to the usual WC_FRAME WndProc
        default:
            return G_SysWndProc(hwndFrame, msg, mp1, mp2);
    }

    return FALSE;
}


