/* PM_api.h */
/* API wrapper for FreePM programms that use PM  */
#ifndef PM_FREEPM_API_H
#define PM_FREEPM_API_H


#ifdef __cplusplus
      extern "C" {
#endif

/*****************************************************************/
/* Utility for conversion FreePM to PM predefined window handles */
/* Should be used called from PM_api.cpp                         */
/*****************************************************************/
HWND  F_convertHandleToPM(HWND hwnd);
/*************************************************/

HMQ     APIENTRY F_WinCreateMsgQueue(HAB hab, LONG cmsg);


HPS  APIENTRY F_WinGetPS(HWND hwnd);

ULONG APIENTRY F_WinQueryVisibleRegion( HWND hwnd,
                                     HRGN hrgn);

HRGN  APIENTRY F_GpiCreateRegion(HPS hps,
                               LONG lCount,
                               PRECTL arclRectangles);

BOOL  APIENTRY F_WinQueryWindowRect(HWND hwnd,
                                       PRECTL prclDest);


BOOL  APIENTRY F_WinMapWindowPoints(HWND hwndFrom,
                                     HWND hwndTo,
                                     PPOINTL prgptl,
                                     LONG cwpt);

HWND  APIENTRY F_WinWindowFromDC(HDC hdc);

HDC   APIENTRY F_WinQueryWindowDC(HWND hwnd);

HDC   APIENTRY F_WinOpenWindowDC(HWND hwnd);
BOOL  APIENTRY F_WinIsWindowVisible(HWND hwnd);

//BOOL   APIENTRY F_WinQueryClassInfo(HAB hab,
//                                    PCSZ  pszClassName,
//                                    PCLASSINFO pClassInfo);
//defined in F_win.h

HWND  APIENTRY F_WinCreateStdWindow(HWND hwndParent,
                                    ULONG flStyle,
                                    PULONG pflCreateFlags,
                                    PSZ pszClientClass,
                                    PSZ pszTitle,
                                    ULONG styleClient,
                                    HMODULE hmod,
                                    ULONG idResources,
                                    PHWND phwndClient);

HWND    APIENTRY F_WinCreateWindow(HWND hwndParent,
                                 PCSZ  pszClass,
                                 PCSZ  pszName,
                                 ULONG flStyle,
                                 LONG x,
                                 LONG y,
                                 LONG cx,
                                 LONG cy,
                                 HWND hwndOwner,
                                 HWND hwndInsertBehind,
                                 ULONG id,
                                 PVOID pCtlData,
                                 PVOID pPresParams);

BOOL    APIENTRY F_WinRegisterClass(HAB hab,
                                  PCSZ  pszClassName,
                                  PFNWP pfnWndProc,
                                  ULONG flStyle,
                                  ULONG cbWindowData);

BOOL    APIENTRY F_WinShowWindow(HWND hwnd,
                                  BOOL fShow);

BOOL    APIENTRY F_WinDestroyWindow(HWND hwnd);

BOOL   APIENTRY F_WinSetWindowPos(HWND hwnd,
                                HWND hwndInsertBehind,
                                LONG x,
                                LONG y,
                                LONG cx,
                                LONG cy,
                                ULONG fl);

BOOL   APIENTRY F_WinQueryWindowPos(HWND hwnd,
                                    PSWP pswp);
BOOL   APIENTRY F_WinQueryPointerPos(HWND hwndDesktop,
                                     PPOINTL pptl);

BOOL    APIENTRY F_WinSetWindowText(HWND hwnd,
                                  PCSZ  pszText);


   BOOL    APIENTRY F_WinGetMsg(HAB hab,
                              PQMSG pqmsg,
                              HWND hwndFilter,
                              ULONG msgFilterFirst,
                              ULONG msgFilterLast);

   BOOL    APIENTRY F_WinPeekMsg(HAB hab,
                               PQMSG pqmsg,
                               HWND hwndFilter,
                               ULONG msgFilterFirst,
                               ULONG msgFilterLast,
                               ULONG fl);

   MRESULT APIENTRY F_WinDispatchMsg(HAB hab,
                                   PQMSG pqmsg);

   BOOL    APIENTRY F_WinPostMsg(HWND hwnd,
                               ULONG msg,
                               MPARAM mp1,
                               MPARAM mp2);

   BOOL    APIENTRY F_WinEndPaint(HPS hps);

   HPS   APIENTRY F_WinBeginPaint(HWND hwnd,
                                HPS hps,
                                PRECTL prclPaint);

   BOOL    APIENTRY F_WinReleasePS(HPS hps);

   BOOL     APIENTRY F_WinSetPointerPos(HWND hwndDesktop,
                                      LONG x,
                                      LONG y);

   BOOL  APIENTRY F_WinCalcFrameRect(HWND hwndFrame,
                                   PRECTL prcl,
                                   BOOL fClient);

   MRESULT APIENTRY F_WinDefWindowProc(HWND hwnd,
                                     ULONG msg,
                                     MPARAM mp1,
                                     MPARAM mp2);

   BOOL     APIENTRY F_WinSetPointer(HWND hwndDesktop,
                                   HPOINTER hptrNew);

   HPOINTER APIENTRY F_WinLoadPointer(HWND hwndDesktop,
                                    HMODULE hmod,
                                    ULONG idres);

      BOOL  APIENTRY F_GpiCreateLogColorTable(HPS hps,
                                            ULONG flOptions,
                                            LONG lFormat,
                                            LONG lStart,
                                            LONG lCount,
                                            PLONG alTable);

      BOOL  APIENTRY F_GpiSetPattern(HPS hps,
                                   LONG lPatternSymbol);

      BOOL  APIENTRY F_GpiSetPatternSet(HPS hps,
                                      LONG lSet);

      LONG APIENTRY F_WinGetKeyState(HWND hwndDesktop,
                                   LONG vkey);

      BOOL APIENTRY F_GpiDeletePalette(HPAL hpal);

      BOOL  APIENTRY F_GpiDestroyRegion(HPS hps,
                                      HRGN hrgn);

/*********************************************/

BOOL  APIENTRY F_GpiQueryDeviceBitmapFormats(HPS hps,LONG lCount, PLONG alArray);

BOOL  APIENTRY F_GpiQueryRegionRects(HPS hps,
                                         HRGN hrgn,
                                         PRECTL prclBound,
                                         PRGNRECT prgnrcControl,
                                         PRECTL prclRect);

BOOL APIENTRY  F_GpiSetColor(HPS hps, LONG lColor);

BOOL APIENTRY  F_GpiMove(HPS hps, PPOINTL pptlPoint);
LONG APIENTRY  F_GpiLine(HPS hps, PPOINTL pptlEndPoint);


#ifdef __cplusplus
        }
#endif

#endif
  //PM_FREEPM_API_H
