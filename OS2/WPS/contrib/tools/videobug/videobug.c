/*
 *  videobug.c:
 *      displays a bitmap in the window's client window, scaled
 *      to the exact size of the client.
 *
 *      With SDD 7.1 (August 7, 2002), the bug shows up when
 *      the bitmap is sized down horizontally (cxClient < cxBitmap)
 *      and sized _up_ vertically (cyClient > cyBitmap).
 */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_GPIBITMAPS
#include <os2.h>

#include <stdio.h>

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

HPS                 G_hpsMem;
HBITMAP             G_hBitmap = NULLHANDLE;
BITMAPINFOHEADER2   G_bmih;
CHAR                G_szError[200] = "";

/* ******************************************************************
 *
 *   Helper funcs
 *
 ********************************************************************/

/*
 *@@ LoadBitmapFile:
 *      this loads the specified bitmap file into
 *      the given HPS. Note that the bitmap is _not_
 *      yet selected into the HPS.
 *
 *      Calls gpihCreateBitmapFromFile for handling the resource
 *      data. See remarks there for how we select a bitmap from
 *      bitmap arrays, which is _different_ from GpiLoadBitmap.
 *
 *      Returns:
 *
 *      --  NO_ERROR: *phbm has received new HBITMAP,
 *          to be freed with GpiDeleteBitmap.
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_INVALID_DATA: file exists, but we
 *          can't understand its format.
 *
 *      plus the error codes from DosOpen and DosRead.
 */

APIRET LoadBitmapFile(HBITMAP *phbm,        // out: bitmap if NO_ERROR
                      HAB hab,              // in: anchor block
                      PCSZ pcszBmpFile)     // in: bitmap filename
{
    APIRET      arc;
    HFILE       hFile = 0;
    FILESTATUS3 fs3;
    ULONG       ulAction;
    PBYTE       pData = NULL;
    ULONG       cbRead;

    if (!pcszBmpFile || !phbm)
        return ERROR_INVALID_PARAMETER;

    if (    (!(arc = DosOpen((PSZ)pcszBmpFile,
                             &hFile,
                             &ulAction,
                             0,
                             FILE_NORMAL,
                             OPEN_ACTION_FAIL_IF_NEW
                                 | OPEN_ACTION_OPEN_IF_EXISTS,
                             OPEN_FLAGS_SEQUENTIAL
                                 | OPEN_SHARE_DENYWRITE
                                 | OPEN_ACCESS_READONLY,
                             NULL)))
            /* get file size */
         && (!(arc = DosQueryFileInfo(hFile,
                                      FIL_STANDARD,
                                      &fs3,
                                      sizeof(fs3))))
            /* allocate memory */
         && (!(arc = DosAllocMem((PVOID*)&pData,
                                 fs3.cbFile,
                                 PAG_COMMIT | PAG_READ | PAG_WRITE)))

            /* read in the ENTIRE file */
         && (!(arc = DosRead(hFile,
                             pData,
                             fs3.cbFile,
                             &cbRead)))
       )
    {
        /* we only handle single bitmaps for now, no bitmap arrays */
        PBITMAPFILEHEADER2 pbfh = (PBITMAPFILEHEADER2)pData;
        if (pbfh->usType != BFT_BMAP)   /* "BM" */
        {
            arc = ERROR_INVALID_DATA;
        }
        else
        {
            HDC hdcMem;
            PSZ pszData[4] = { "Display", NULL, NULL, NULL };
            // create new memory DC
            if ((hdcMem = DevOpenDC(hab,
                                    OD_MEMORY,      // create memory DC
                                    "*",            // token: do not take INI info
                                    4,              // item count in pszData
                                    (PDEVOPENDATA)pszData,
                                    NULLHANDLE)))    // compatible with screen
            {
                // memory DC created successfully:
                // create compatible PS
                SIZEL   szlPage = {0, 0};
                if ((G_hpsMem = GpiCreatePS(hab,
                                          hdcMem,      // HDC to associate HPS with (GPIA_ASSOC);
                                                       // mandatory for GPIT_MICRO
                                          &szlPage,    // is (0, 0) == screen size
                                          PU_PELS      // presentation page units: pixels
                                            | GPIA_ASSOC    // associate with hdcMem (req. for GPIT_MICRO)
                                            | GPIT_MICRO)))  // micro presentation space
                {
                    if (!(*phbm = GpiCreateBitmap(G_hpsMem,
                                                  &pbfh->bmp2,
                                                  CBM_INIT,
                                                  (PBYTE)pbfh + pbfh->offBits,
                                                  (PBITMAPINFO2)&pbfh->bmp2)))
                        arc = ERROR_INVALID_DATA;
                    else
                        GpiSetBitmap(G_hpsMem,
                                     *phbm);
                }
            }
        }
    }

    if (hFile)
        DosClose(hFile);

    if (pData)
        DosFreeMem(pData);
    return arc;
}

/* ******************************************************************
 *
 *   Client proc
 *
 ********************************************************************/

/*
 *@@ fnwpTestClient:
 *
 */

MRESULT EXPENTRY fnwpTestClient(HWND hwndClient, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_WINDOWPOSCHANGED:
            if (((PSWP)mp1)->fl & SWP_SIZE)
            {
                WinInvalidateRect(hwndClient, NULL, FALSE);
            }
        break;

        /*
         * WM_PAINT:
         *
         */

        case WM_PAINT:
        {
            HPS hps;
            if (hps = WinBeginPaint(hwndClient, NULLHANDLE, NULL))
            {
                RECTL   rclWin;
                CHAR    szTemp[100];

                WinQueryWindowRect(hwndClient, &rclWin);

                if (G_hBitmap)
                {
                    // bitmap was loaded:
                    POINTL  aptl[4];

                    // aptl[0]: target bottom-left
                    aptl[0].x = 0;
                    aptl[0].y = 0;

                    // aptl[1]: target top-right (inclusive!)
                    aptl[1].x = rclWin.xRight - 1;
                    aptl[1].y = rclWin.yTop - 1;

                    // aptl[2]: source bottom-left
                    aptl[2].x = 0;
                    aptl[2].y = 0;

                    // aptl[3]: source top-right (exclusive!)
                    aptl[3].x = G_bmih.cx;
                    aptl[3].y = G_bmih.cy;

                    GpiBitBlt(hps,
                              G_hpsMem,
                              4L,             // must always be 4
                              &aptl[0],       // points array
                              ROP_SRCCOPY,
                              BBO_IGNORE);
                }
                else
                    WinFillRect(hps, &rclWin, CLR_WHITE);

                sprintf(szTemp,
                        "Bitmap cx = %d, cy =  %d",
                        G_bmih.cx, G_bmih.cy);
                WinDrawText(hps,
                            -1,
                            szTemp,
                            &rclWin,
                            CLR_BLACK,
                            CLR_WHITE,
                            DT_LEFT | DT_TOP);

                sprintf(szTemp,
                        "Window cx = %d, cy =  %d",
                        rclWin.xRight, rclWin.yTop);
                WinDrawText(hps,
                            -1,
                            szTemp,
                            &rclWin,
                            CLR_BLACK,
                            CLR_WHITE,
                            DT_LEFT | DT_BOTTOM);

                WinEndPaint(hps);
            }
        }
        break;

        default:
            mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ main:
 *
 */

int main(void)
{
    HAB     hab = WinInitialize(0);
    HMQ     hmq = WinCreateMsgQueue(hab, 0);
    QMSG    qmsg;
    ULONG   flFrame =   FCF_TITLEBAR
                      | FCF_SYSMENU
                      | FCF_MINMAX
                      | FCF_SIZEBORDER
                      | FCF_NOBYTEALIGN
                      | FCF_TASKLIST;

    HWND    hwndMain,
            hwndClient;
    RECTL   rcl;
    APIRET  arc;

    WinRegisterClass(hab,
                     "TestClientClass",
                     fnwpTestClient,
                     CS_SIZEREDRAW,
                     0);

    /* try to load the bitmap */
    if (arc = LoadBitmapFile(&G_hBitmap,
                             hab,
                             "videobug.bmp"))
        sprintf(G_szError,
                "Error %d loading bitmap",
                arc);
    else
    {
        G_bmih.cbFix = sizeof(G_bmih);
        GpiQueryBitmapInfoHeader(G_hBitmap,
                                 &G_bmih);
    }

    /* create main window */
    hwndMain = WinCreateStdWindow(HWND_DESKTOP,
                                  WS_VISIBLE,
                                  &flFrame,
                                  "TestClientClass",
                                  "Video driver bug",
                                  WS_VISIBLE,
                                  NULLHANDLE,
                                  0,
                                  &hwndClient);

    rcl.xLeft = 0;
    rcl.yBottom = 0;
    rcl.xRight = G_bmih.cx;
    rcl.yTop = G_bmih.cy;
    WinCalcFrameRect(hwndMain,
                     &rcl,
                     FALSE);
    WinSetWindowPos(hwndMain,
                    HWND_TOP,
                    100,
                    100,
                    rcl.xRight - rcl.xLeft,
                    rcl.yTop - rcl.yBottom,
                    SWP_MOVE | SWP_SIZE | SWP_ZORDER | SWP_ACTIVATE);

    while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
    {
        WinDispatchMsg(hab, &qmsg);
    }

    WinDestroyWindow(hwndMain);

    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

    return 0;
}
