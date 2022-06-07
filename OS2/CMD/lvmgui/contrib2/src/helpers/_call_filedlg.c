#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSPROCESS
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMENUS
#define INCL_WINBUTTONS
#define INCL_WINPOINTERS
#define INCL_WINSTDCNR
#define INCL_WINSTDFILE
#define INCL_WINTIMER

#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "setup.h"                      // code generation and debugging options

#include "..\..\..\xworkplace\include\xwpapi.h"

#include "helpers\call_file_dlg.c"
#include "helpers\cnrh.h"
#include "helpers\comctl.h"
#include "helpers\nls.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\winh.h"
#include "helpers\gpih.h"

PCSZ    WC_CLIENT = "MyClient";

/*
 *@@ NewWinFileDlg:
 *      replacement for WinFileDlg. Use similarly.
 */

HWND APIENTRY NewWinFileDlg(HWND hwndOwner,
                            PFILEDLG pfd)       // WinFileDlg
{
    HWND        hwndReturn = NULLHANDLE;
    BOOL        fCallDefault = TRUE;

    hwndReturn = ImplCallFileDlg(hwndOwner, pfd, &fCallDefault);

    if (fCallDefault)
        // something went wrong:
        hwndReturn = WinFileDlg(HWND_DESKTOP,
                                hwndOwner,
                                pfd);

    return hwndReturn;
}

/*
 *@@ ShowFileDlg:
 *
 */

VOID ShowFileDlg(HWND hwndFrame)
{
    FILEDLG         fd;

    memset(&fd, 0, sizeof(FILEDLG));
    fd.cbSize = sizeof(FILEDLG);
    fd.fl = FDS_CENTER | FDS_OPEN_DIALOG;

    strcpy(fd.szFullFile, "C:\\*");

    if (NewWinFileDlg(hwndFrame,
                      &fd))
    {
        CHAR sz[1000];
        sprintf(sz, "got: \"%s\"", fd.szFullFile);
        WinMessageBox(HWND_DESKTOP, hwndFrame,
                      sz,
                      "File:",
                      0,
                      MB_OK | MB_MOVEABLE);
    }
    else
        WinMessageBox(HWND_DESKTOP, hwndFrame,
                      "file dlg returned FALSE",
                      "File:",
                      0,
                      MB_OK | MB_MOVEABLE);
}

/*
 *@@ GROUPRECORD:
 *
 */

typedef struct _GROUPRECORD
{
    RECORDCORE      recc;

    ULONG           gid;
    CHAR            szGroupName[100];    // group name
    PSZ             pszMembers;

} GROUPRECORD, *PGROUPRECORD;

PFNWP G_pfnwpFrameOrig;

#define RECORD_COUNT        20000

PGROUPRECORD G_paRecs[RECORD_COUNT];

CHAR    G_szText[] = "Longer test string with increasing length";

/*
 *@@ fnwpSubclFrame:
 *
 */

MRESULT EXPENTRY fnwpSubclFrame(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_COMMAND:
            if (SHORT1FROMMP(mp1) == 1000)
                ShowFileDlg(hwndFrame);
        break;

        case WM_CONTROL:
            if (SHORT1FROMMP(mp1) == FID_CLIENT)
            {
                switch (SHORT2FROMMP(mp1))
                {
                    case CN_VIEWPORTCHANGED:
                    {
                        PCNRVIEWPORT pcvp = (PCNRVIEWPORT)mp2;

                        CHAR szTemp[100],
                             szNLS1[20],
                             szNLS2[20],
                             szNLS3[20],
                             szNLS4[20],
                             szNLS5[20],
                             szNLS6[20];
                        sprintf(szTemp,
                                "Win: (%s/%s), WArea: (%s/%s), scrl: (%s/%s)",
                                nlsThousandsULong(szNLS1,
                                                  pcvp->szlWin.cx,
                                                  '.'),
                                nlsThousandsULong(szNLS2,
                                                  pcvp->szlWin.cy,
                                                  '.'),
                                nlsThousandsULong(szNLS3,
                                                  pcvp->szlWorkarea.cx,
                                                  '.'),
                                nlsThousandsULong(szNLS4,
                                                  pcvp->szlWorkarea.cy,
                                                  '.'),
                                nlsThousandsULong(szNLS5,
                                                  pcvp->ptlScroll.x,
                                                  '.'),
                                nlsThousandsULong(szNLS6,
                                                  pcvp->ptlScroll.y,
                                                  '.'));
                        WinSetWindowText(WinWindowFromID(hwndFrame, FID_STATUSBAR),
                                         szTemp);
                    }
                    break;

                    case CN_ENTER:
                    {
                        CHAR sz[100];
                        PCSZ pcsz;
                        PNOTIFYRECORDENTER pnre = (PNOTIFYRECORDENTER)mp2;
                        if (!pnre->pRecord)
                            pcsz = "Whitespace";
                        else
                        {
                            sprintf(sz, "group %d", ((PGROUPRECORD)pnre->pRecord)->gid);
                            pcsz = sz;
                        }
                        winhDebugBox(hwndFrame,
                                     "Enter pressed",
                                     pcsz);
                    }
                    break;
                }
            }
        break;

        case WM_TIMER:
            if ((ULONG)mp1 == 1)
            {
                static ul = 0,
                       ul2 = 1;

                HWND    hwndCnr = WinWindowFromID(hwndFrame, FID_CLIENT);

                PGROUPRECORD precThis = G_paRecs[ul];

                ULONG ulTimeNow;
                DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                                &ulTimeNow,
                                sizeof(ulTimeNow));

                strhncpy0(precThis->szGroupName,
                          G_szText,
                          ul2++);

                WinSendMsg(hwndCnr,
                           CM_INVALIDATERECORD,
                           (MPARAM)&precThis,
                           MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));

                if (++ul >= RECORD_COUNT)
                    ul = 0;
                if (ul2 > strlen(G_szText))
                    ul2 = 1;
            }
            else
                mrc = G_pfnwpFrameOrig(hwndFrame, msg, mp1, mp2);
        break;

        default:
            mrc = G_pfnwpFrameOrig(hwndFrame, msg, mp1, mp2);
        break;

    }

    return mrc;
}

/*
 *@@ main:
 *
 */

int main(int argc, char *argv[])
{
    HAB             hab;
    HMQ             hmq;

    #undef TBBS_TEXT
    #define TBBS_TEXT 0

    #define TBBS_COMMON TBBS_AUTORESIZE | TBBS_FLAT | TBBS_HILITE | WS_VISIBLE

    CHAR            szOpen[200],
                    szExit[200];

    TOOLBARCONTROL  aControls[] =
        {
            /*
            WC_CCTL_TBBUTTON,
            szExit,
            TBBS_COMMON | TBBS_BIGICON | TBBS_TEXT | TBBS_SYSCOMMAND,
            SC_CLOSE,
            10,
            10,

            WC_CCTL_TBBUTTON,
            szExit,
            TBBS_COMMON | TBBS_BIGICON,
            0,
            10,
            10,

            WC_CCTL_TBBUTTON,
            szExit,
            TBBS_COMMON | TBBS_MINIICON,
            0,
            10,
            10,
            */

            WC_CCTL_TBBUTTON,
            szOpen,
            TBBS_COMMON | TBBS_MINIICON | TBBS_TEXT,
            1000,
            10,
            10,

            WC_CCTL_TBBUTTON,
            "Toggle\ntest",
            TBBS_COMMON | TBBS_TEXT | TBBS_CHECK,
            1001,
            10,
            10,

            WC_CCTL_SEPARATOR,
            NULL,
            WS_VISIBLE | SEPS_VERTICAL,
            1002,
            10,
            10,

            WC_CCTL_TBBUTTON,
            "Group 1",
            TBBS_COMMON | TBBS_TEXT | TBBS_RADIO | TBBS_CHECKINITIAL,
            1101,
            10,
            10,

            WC_CCTL_TBBUTTON,
            "Group 2",
            TBBS_COMMON | TBBS_TEXT | TBBS_RADIO,
            1102,
            10,
            10,

            WC_CCTL_TBBUTTON,
            "Group 3",
            TBBS_COMMON | TBBS_TEXT | TBBS_RADIO,
            1103,
            10,
            10,

        };

    EXTFRAMECDATA   xfd =
        {
            NULL,                               // pswpFrame
            FCF_TITLEBAR
                  | FCF_SYSMENU
                  | FCF_MINMAX
                  | FCF_SIZEBORDER
                  | FCF_NOBYTEALIGN
                  | FCF_SHELLPOSITION
                  | FCF_TASKLIST,
            XFCF_TOOLBAR | XFCF_FORCETBOWNER | XFCF_STATUSBAR,
            WS_VISIBLE,                         // ulFrameStyle
            "Test File Dialog",                 // pcszFrameTitle
            0,                                  // ulResourcesID
#ifdef __USE_PM_CNR__
            WC_CONTAINER,
#else
            WC_CCTL_CNR,
#endif
            WS_VISIBLE | CCS_EXTENDSEL,       // flStyleClient
            0,                                  // ulID
            NULL,
            HINI_USER,
            "XWorkplace Test Apps",
            "CallFileDlgPos",

            ARRAYITEMCOUNT(aControls),
            aControls
        };

    HWND            hwndFrame,
                    hwndClient,
                    hwndStatusBar,
                    hwndToolBar,
                    hwndMenu,
                    hwndSubmenu;
    QMSG            qmsg;

    hab = WinInitialize(0);
    hmq = WinCreateMsgQueue(hab, 0);

    winhInitGlobals();

    ctlRegisterToolbar(hab);
    ctlRegisterSeparatorLine(hab);
    ctlRegisterXCnr(hab);

    sprintf(szOpen,
            "#%d#Open",
            WinQuerySysPointer(HWND_DESKTOP,
                               SPTR_ICONINFORMATION,
                               FALSE));

    sprintf(szExit,
            "#%d#Exit",
            WinQuerySysPointer(HWND_DESKTOP,
                               SPTR_ICONWARNING,
                               FALSE));

    hwndFrame = ctlCreateStdWindow(&xfd, &hwndClient);

    hwndToolBar = WinWindowFromID(hwndFrame, FID_TOOLBAR);
    hwndStatusBar = WinWindowFromID(hwndFrame, FID_STATUSBAR);

    WinSetWindowText(hwndToolBar, "Tool bar");
    WinSetWindowText(hwndStatusBar, "Status bar");

    WinSendMsg(hwndFrame, WM_UPDATEFRAME, MPNULL, MPNULL);

    if (G_pfnwpFrameOrig = WinSubclassWindow(hwndFrame, fnwpSubclFrame))
    {
        XFIELDINFO  xfi[4];
        PFIELDINFO      pfi = NULL;
        int i = 0;
        PGROUPRECORD preccFirst;

        CHAR    szTest[12];
        ULONG   len;

        len = strlcpy(szTest, "~Group name", sizeof(szTest));
        strhKillChar(szTest, '~', &len);

        // set up cnr details view
        xfi[i].ulFieldOffset = FIELDOFFSET(GROUPRECORD, gid);
        xfi[i].pszColumnTitle = "gid";
        xfi[i].ulDataType = CFA_ULONG;
        xfi[i++].ulOrientation = CFA_RIGHT;

        xfi[i].ulFieldOffset = FIELDOFFSET(GROUPRECORD, recc.pszIcon);
        xfi[i].pszColumnTitle = szTest; // "Group name";   // @@todo localize
        xfi[i].ulDataType = CFA_STRING;
        xfi[i++].ulOrientation = CFA_CENTER;

        xfi[i].ulFieldOffset = FIELDOFFSET(GROUPRECORD, pszMembers);
        xfi[i].pszColumnTitle = "Members";   // @@todo localize
        xfi[i].ulDataType = CFA_STRING;
        xfi[i++].ulOrientation = CFA_LEFT;

        pfi = cnrhSetFieldInfos(hwndClient,
                                xfi,
                                i,             // array item count
                                TRUE,          // draw lines
                                0);            // return first column

        BEGIN_CNRINFO()
        {
            cnrhSetView(CV_DETAIL | CA_DETAILSVIEWTITLES);
            CnrInfo_.cyLineSpacing = 10;
            ulSendFlags_ |= CMA_LINESPACING;
        } END_CNRINFO(hwndClient);

        if (preccFirst = (PGROUPRECORD)cnrhAllocRecords(hwndClient,
                                                        sizeof(GROUPRECORD),
                                                        RECORD_COUNT))
        {
            PGROUPRECORD preccThis = preccFirst,
                         preccEmph;
            ULONG   ul = 0;
            while (preccThis)
            {
                G_paRecs[ul] = preccThis;

                if (ul & 1)
                    preccThis->recc.flRecordAttr |= CRA_FILTERED;

                if (ul == 4)
                    preccEmph = preccThis;

                preccThis->gid = ul++;
                sprintf(preccThis->szGroupName, "group %d", preccThis->gid);
                preccThis->recc.pszIcon = preccThis->szGroupName;

                preccThis->pszMembers = "longer string than title";

                preccThis = (PGROUPRECORD)preccThis->recc.preccNextRecord;
            }

            cnrhInsertRecords(hwndClient,
                              NULL,
                              (PRECORDCORE)preccFirst,
                              TRUE,
                              NULL,
                              CRA_RECORDREADONLY,
                              RECORD_COUNT);

            WinSendMsg(hwndClient,
                       CM_SETRECORDEMPHASIS,
                       (MPARAM)preccEmph,
                       MPFROM2SHORT(TRUE, CRA_SELECTED));
        }
    }

    WinStartTimer(hab, hwndFrame, 1, 100);

    while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
        WinDispatchMsg(hab, &qmsg);

    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

    return 0;
}


