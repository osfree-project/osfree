/*
  Used functions:
    WinInitialize
    WinTerminite
    WinCreateMsgQueue
    WinDestroyMsgQueue
*/

#define INCL_WIN
#define INCL_GPI
#include <os2.h>

INT main ( VOID )
{
   HAB         habAnchor ;
   HMQ         hmqQueue ;

   // Initialize this thread
   habAnchor = WinInitialize ( 0 ) ;

   hmqQueue = WinCreateMsgQueue ( habAnchor, 0 ) ;

   WinDestroyMsgQueue ( hmqQueue ) ;

   // Terminate this thread
   WinTerminate ( habAnchor ) ;
   return 0 ;
}


