/*
  Used functions:
    WinInitialize
    WinTerminite
    WinCreateMsgQueue
    WinDestroyMsgQueue
    WinGetMsg
    WinDispatchMsg
*/

#define INCL_WIN
#define INCL_GPI
#include <os2.h>

INT main ( VOID )
{
   HAB         habAnchor ;
   HMQ         hmqQueue ;
   BOOL        bLoop ;
   QMSG        qmMsg ;

   // Initialize this thread
   habAnchor = WinInitialize ( 0 ) ;

   hmqQueue = WinCreateMsgQueue ( habAnchor, 0 ) ;

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


   WinDestroyMsgQueue ( hmqQueue ) ;

   // Terminate this thread
   WinTerminate ( habAnchor ) ;
   return 0 ;
}


