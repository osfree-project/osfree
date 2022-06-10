/***************************************************************************
 *      Created 2005  eCo Software                                         *
 *                                                                         *
 *      DISCLAIMER OF WARRANTIES.  The following [enclosed] code is        *
 *      sample code created by eCo Software. This sample code is not part  *
 *      of any standard or eCo Software product and is provided to you     *
 *      solely for the purpose of assisting you in the development of your *
 *      applications.  The code is provided "AS IS", without               *
 *      warranty of any kind. eCo Software shall not be liable for any     *
 *      damages arising out of your use of the sample code, even if they   *
 *      have been advised of the possibility of such damages.              *
 *-------------------------------------------------------------------------*/

// Tnx to VicTor Smirnoff for template

#define INCL_PM
#define INCL_DOSPROCESS        // For DosSleep
#define INCL_WIN
#include <os2.h>

#include <os2.h>
#include <stdlib.h>            // For max()
#include <string.h>            // For string()
#include <acpi.h>
#include <acpiapi.h>
#include "ButtonJin.h"

//-------------------------------------------------------------
// Prototype
//-------------------------------------------------------------
MRESULT EXPENTRY WndDlgProc( HWND, ULONG, MPARAM, MPARAM );
VOID SetDlgItemPos( LONG, LONG );
VOID DisplayError( ULONG );
VOID BeepError( int );


//-------------------------------------------------------------
// Constants
//-------------------------------------------------------------
#define RETURN_ERROR         1      // Error code
#define BEEP_WARN_FREQ    3000      // Error tone
#define BEEP_WARN_DUR      200      // Error latency
#define BEEP_WARN_PAUSE     40      // Error pause
#define BEEP_WARN_INIT       2      // Error count
#define BEEP_WARN_QUEUE      4      // Error number
#define MSG_WARN_SIZE       80      // Error length


//-------------------------------------------------------------
// Global
//-------------------------------------------------------------
HAB  hab;
HWND hwndDlg;

SWP swpDefault;                    //  Dialog's size by default
SWP swpMini;                       //  Dialog's size before minmaze

CHAR szAppTitle[MAXNAMEL];
CHAR szAppName[] = "ACPI Dialog Blank";
CHAR szKeyName[] = "Initial Params";

//=======================================================
// Begin from here
//=======================================================
int main(
    void)
{
    HMQ    hmq;
    QMSG   qmsg;

    APIRET rc;
    ACPI_API_HANDLE Hdl;
    ULONG State;

    if (AcpiTkValidateVersion(ACPI_TK_VERSION_MAJOR, ACPI_TK_VERSION_MINOR)) {
        return 1;
    }
    rc=AcpiTkStartApi(&Hdl);                                    // Starting ACPI
    if (rc) return 1;
    while (1)
    {
    /****************************************************************
     *   IF YOU WANT CHECK SLEEP BUTTON:
     *   change SET_POWERBUTTON to SET_SLEEPBUTTON
     ***************************************************************/
           rc = AcpiTkWaitPressButton(&Hdl,SET_POWERBUTTON,-1,&State);    // Wait Press Button forever
           if (rc)                                                // Error?
           {                                                            // Go out
               BeepError( BEEP_WARN_INIT );                             // Alarm to user
               rc = AcpiTkEndApi(&Hdl);                                   // Close ACPI
               return RETURN_ERROR;
           }
           if (State == 0)  continue;                                // Not Pressed? Continue

           // Init windows interface
           hab = WinInitialize( 0 );
           if (hab == NULLHANDLE)
           {
               BeepError( BEEP_WARN_INIT );                             // Alarm to user
               return RETURN_ERROR;
           }

//-------------------------------------------------------------
           hmq = WinCreateMsgQueue( hab, 0 );
           if (hmq == NULLHANDLE)
           {
               BeepError( BEEP_WARN_QUEUE );
               return RETURN_ERROR;
           }

//-------------------------------------------------------------
           WinLoadString( hab,(HMODULE)0L,IDS_APPTITLE,MAXNAMEL,(PSZ)szAppTitle );
//-------------------------------------------------------------
           hwndDlg = WinLoadDlg( HWND_DESKTOP, // Where
                                 HWND_DESKTOP, // Who
                                 WndDlgProc,   // subprogram for
                                 (HMODULE)0,   // 0 -  EXE   #0 -  DLL
                                 ID_MAIN,      // ID in resorce
                                 NULL);        // Data for Init
           if (hwndDlg == NULLHANDLE)
           {
               DisplayError(IDS_CREATEMAIN);
               WinDestroyMsgQueue(hmq);
               WinTerminate(hab);
               return RETURN_ERROR;
           }
//-------------------------------------------------------------
           while (WinGetMsg(hab, &qmsg, 0, 0, 0 ))
                  WinDispatchMsg( hab, &qmsg );
//-------------------------------------------------------------
           WinDestroyWindow(hwndDlg);
           WinDestroyMsgQueue(hmq);
           WinTerminate(hab);

    }

    rc = AcpiTkEndApi(&Hdl);
    return ( 0 );
}

//=======================================================================
MRESULT EXPENTRY
WndDlgProc(
    HWND   hwnd,
    ULONG  msg,
    MPARAM mp1,
    MPARAM mp2 )
{
    switch (msg)
    {
    //-------------------------------------------------------------
    // WM_INITDLG
    //-------------------------------------------------------------
            case WM_INITDLG:
            {
                 HPOINTER hptrIcon;

                 hptrIcon = WinLoadPointer(HWND_DESKTOP, 0L, ID_MAINICON);
                 WinPostMsg(hwnd, WM_SETICON, (MPARAM)hptrIcon, (MPARAM)0L);
                 WinSetWindowText(hwnd, szAppTitle);
                 WinPostMsg(hwnd, MY_QUERYSIZE, NULL, NULL);
            }  break;
            //-------------------------------------------------------------
            // MY_QUERYSIZE
            //-------------------------------------------------------------
            case MY_QUERYSIZE:
            {
                 SWP     swp;
                 SWCNTRL swctl;
                 HSWITCH hswitch;

                 swctl.hwnd                  = hwndDlg;
                 swctl.hwndIcon              = hwndDlg;
                 swctl.hprog                 = NULLHANDLE;
                 swctl.idProcess             = 0;
                 swctl.idSession             = 0;
                 swctl.uchVisibility         = SWL_VISIBLE;
                 swctl.fbJump                = SWL_JUMPABLE;
                 swctl.szSwtitle[0]          = 0x00;
                 swctl.szSwtitle[MAXNAMEL+1] = 0x00;
                 strncpy(swctl.szSwtitle, szAppTitle, MAXNAMEL);

                 hswitch = WinAddSwitchEntry(&swctl);

                 WinQueryWindowPos(hwnd, (PSWP)&swp);
                 swpDefault = swp;
                 swpMini    = swp;

                 if (!WinRestoreWindowPos( (PSZ)szAppName, (PSZ)szKeyName, hwndDlg ))
                     WinPostMsg( hwnd, MY_SETSIZE, MPFROMLONG(swpDefault.cx), MPFROMLONG(swpDefault.cy));
            } break;
            //-------------------------------------------------------------
            // MY_SETSIZE
            //-------------------------------------------------------------
            case MY_SETSIZE:
            {
                 SWP  swp;
                 LONG deltaX, deltaY;

                 WinSetWindowPos(hwnd, HWND_TOP, 0L, 0L, 0L, 0L, SWP_ACTIVATE);

                 WinQueryWindowPos( hwnd, (PSWP)&swp );
                 if (!(swp.fl & SWP_MINIMIZE))
                 {
                     deltaX = swp.cx - LONGFROMMP(mp1);
                     deltaY = swp.cy - LONGFROMMP(mp2);

                     if (deltaX!=0 || deltaY!=0 )
                         SetDlgItemPos( deltaX, deltaY );

                     WinShowWindow( hwnd, TRUE );
                 }
            } break;
            //-------------------------------------------------------------
            // WM_ADJUSTWINDOWPOS
            //-------------------------------------------------------------
            case WM_ADJUSTWINDOWPOS:
            {
                 SWP swp;

                 WinQueryWindowPos( hwnd, (PSWP)&swp );

                 //-------------------------------------------------------------
                 // SWP_MINIMIZE
                 //-------------------------------------------------------------
                 if (((PSWP)mp1)->fl & SWP_MINIMIZE)
                      WinQueryWindowPos( hwnd, (PSWP)&swpMini);

                 //-------------------------------------------------------------
                 // SWP_RESTORE || SWP_MAXIMIZE
                 //-------------------------------------------------------------
                 else if ((((PSWP)mp1)->fl & SWP_RESTORE) ||
                          (((PSWP)mp1)->fl & SWP_MAXIMIZE))
                      {
                          if (swp.fl & SWP_MINIMIZE)
                          {
                              ((PSWP)mp1)->fl = ((PSWP)mp1)->fl & ~SWP_ACTIVATE;
                              WinPostMsg( hwnd, MY_SETSIZE, MPFROMLONG(swpMini.cx), MPFROMLONG(swpMini.cy));
                          }
                          else
                          {
                              WinPostMsg( hwnd, MY_SETSIZE, MPFROMLONG(swp.cx), MPFROMLONG(swp.cy));
                          }
                       }
                       //-------------------------------------------------------------
                       // SWP_SIZE
                       //-------------------------------------------------------------
                       else if (((PSWP)mp1)->fl & SWP_SIZE)
                            {
                               if ((((PSWP)mp1)->cx<swpDefault.cx) ||
                                   (((PSWP)mp1)->cy<swpDefault.cy) )
                               {
                                   ((PSWP)mp1)->cx = max( swpDefault.cx, ((PSWP)mp1)->cx );
                                   ((PSWP)mp1)->cy = max( swpDefault.cy, ((PSWP)mp1)->cy );
                               }
                               WinPostMsg( hwnd, MY_SETSIZE, MPFROMLONG(swp.cx), MPFROMLONG(swp.cy) );
                            }
            }  return WinDefDlgProc( hwnd, msg, mp1, mp2 );
            //-------------------------------------------------------------
            // WM_CLOSE
            //-------------------------------------------------------------
            case WM_CLOSE:
                 WinStoreWindowPos((PSZ)szAppName, (PSZ)szKeyName, hwnd);
                 WinDismissDlg( hwnd, TRUE );
                 WinPostMsg( hwnd, WM_QUIT, NULL, NULL );
                 break;
            //-------------------------------------------------------------
            // WM_COMMAND
            //-------------------------------------------------------------
            case WM_COMMAND:
                 switch (SHORT1FROMMP( mp1 ))
                 {
                        //-------------------------------------------------------------
                        // User Press ESC
                        //-------------------------------------------------------------
                        case DID_CANCEL:
                             break;
                        //-------------------------------------------------------------
                        // User press button EXIT
                        // (***) WM_CLOSE need redispatch after (nonmodal)
                        //-------------------------------------------------------------
                        case ID_EXIT:
                             WinPostMsg( hwnd, WM_CLOSE, NULL, NULL );
                             break;
                        //-------------------------------------------------------------
                        // Here can you dialog
                        //
                        // case :
                        //    ...
                        //    break;
                        //-------------------------------------------------------------

                        //-------------------------------------------------------------
                        // WinDefDlgProc, by default
                        //-------------------------------------------------------------
                        default:
                             return WinDefDlgProc( hwnd, msg, mp1, mp2 );
                        } break;
            //-------------------------------------------------------------
            // WinDefDlgProc, by default
            //-------------------------------------------------------------
            default:
               return WinDefDlgProc( hwnd, msg, mp1, mp2 );
       }
       return (MRESULT)FALSE;
}


//=======================================================================
// Positions elements of dialog
//=======================================================================
VOID
SetDlgItemPos(
    LONG dX,
    LONG dY )
{
    HWND hwnd;
    SWP swp;

    // Hidden all elements of dialog
    WinShowWindow( WinWindowFromID( hwndDlg, ID_ICON), FALSE );
    WinShowWindow( WinWindowFromID( hwndDlg, ID_EXIT), FALSE );

    // Positions ICO
    hwnd = WinWindowFromID( hwndDlg, ID_ICON);
    WinQueryWindowPos( hwnd, (PSWP)&swp );
    WinSetWindowPos(hwnd,
                    HWND_TOP,
                    swp.x,
                    swp.y + dY,
                    swp.cx,
                    swp.cy,
                    SWP_MOVE );
    // Positions Button EXIT
    hwnd = WinWindowFromID(hwndDlg, ID_EXIT);
    WinQueryWindowPos(hwnd, (PSWP)&swp );
    WinSetWindowPos(hwnd,
                    HWND_TOP,
                    swp.x + dX,
                    swp.y,
                    swp.cx,
                    swp.cy,
                    SWP_MOVE );
    // Show elements
    WinShowWindow( WinWindowFromID( hwndDlg, ID_ICON), TRUE );
    WinShowWindow( WinWindowFromID( hwndDlg, ID_EXIT), TRUE );
}


//=======================================================================
// Error message box
//=======================================================================
VOID
DisplayError(
    ULONG idMsg)
{
    char cMsg[MSG_WARN_SIZE];

    WinLoadString(hab, (HMODULE)0L, idMsg, MSG_WARN_SIZE, (PSZ)cMsg);
    WinMessageBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  (PCH)cMsg,
                  (PCH)szAppTitle,
                  ID_MSGBOX,
                  MB_OK | MB_APPLMODAL | MB_MOVEABLE | MB_ERROR );
}


//=======================================================================
// Signal of error
//=======================================================================
VOID
BeepError(
    int n)
{
    int i;

    for (i=0; i < n; i++ )
    {
         DosBeep( BEEP_WARN_FREQ, BEEP_WARN_DUR );
         DosSleep( BEEP_WARN_PAUSE );
    }
}
