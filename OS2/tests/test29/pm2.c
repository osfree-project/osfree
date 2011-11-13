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
   QMSG        qmMsg ;

   // Initialize this thread
   habAnchor = WinInitialize ( 0 ) ;

   hmqQueue = WinCreateMsgQueue ( habAnchor, 0 ) ;

   while (WinGetMsg (habAnchor,
                     &qmMsg,
                     NULLHANDLE,
                     0,
                     0))
     WinDispatchMsg (habAnchor, &qmMsg);


   WinDestroyMsgQueue (hmqQueue);

   // Terminate this thread
   WinTerminate (habAnchor);
   return 0 ;
}


