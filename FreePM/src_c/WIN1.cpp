/*
 $Id: WIN1.CPP,v 1.3 2002/09/06 14:59:33 evgen2 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POKA 0

#include "F_config.hpp"
#include "F_def.hpp"

#define F_INCL_WIN
#define F_INCL_GPI
#include "F_OS2.hpp"
#include "F_win.hpp"

#define CLS_CLIENT               "WindowClass"

MRESULT EXPENTRY ClientWndProc ( HWND hwndWnd,
   ULONG ulMsg,
   MPARAM mpParm1,
   MPARAM mpParm2 ) ;

INT main ( VOID )
{
   HAB         habAnchor ;
   HMQ         hmqQueue ;
   ULONG       ulFlags ;
   HWND        hwndFrame ;
   HWND        hwndClient ;
   BOOL        bLoop ;
   QMSG        qmMsg ;

   habAnchor = WinInitialize ( 0 ) ;


   hmqQueue = WinCreateMsgQueue ( habAnchor, 0 ) ;

   WinRegisterClass ( habAnchor,
                      CLS_CLIENT,
                      ClientWndProc,
                      0,
                      0 ) ;

   ulFlags = FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER |
             FCF_MINMAX | FCF_SHELLPOSITION | FCF_TASKLIST ;

   hwndFrame = WinCreateStdWindow ( HWND_DESKTOP,
                                    WS_VISIBLE,
                                    &ulFlags,
                                    CLS_CLIENT,
                                    "WIN1 Titlebar",
                                    0,
                                    NULLHANDLE,
                                    0,
                                    &hwndClient ) ;

   if ( hwndFrame != NULLHANDLE ) {
      bLoop = WinGetMsg ( habAnchor,
                          &qmMsg,
                          NULLHANDLE,
                          0,
                          0 ) ;
      while ( bLoop ) {
         WinDispatchMsg ( habAnchor, &qmMsg ) ;
         bLoop = WinGetMsg ( habAnchor,
                             &qmMsg,
                             NULLHANDLE,
                             0,
                             0 ) ;
      } /* endwhile */

      WinDestroyWindow ( hwndFrame ) ;
   } /* endif */

   WinDestroyMsgQueue ( hmqQueue ) ;
   WinTerminate ( habAnchor ) ;
   return 0 ;
}

MRESULT EXPENTRY ClientWndProc ( HWND hwndWnd,
                                 ULONG ulMsg,
                                 MPARAM mpParm1,
                                 MPARAM mpParm2 )
{
#if POKA
   switch ( ulMsg ) {
   case WM_ERASEBACKGROUND:
      return MRFROMSHORT ( TRUE ) ;

   default:
      return WinDefWindowProc ( hwndWnd,
                                ulMsg,
                                mpParm1,
                                mpParm2 ) ;
   } /* endswitch */

#endif //POKA
   return MRFROMSHORT ( FALSE ) ;
}

