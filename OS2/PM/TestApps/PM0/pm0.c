/*
  Used functions:
    WinInitialize
    WinTerminite
*/

#define INCL_WIN
#define INCL_GPI
#include <os2.h>

INT main ( VOID )
{
   HAB         habAnchor;

   // Initialize this thread
   habAnchor = WinInitialize ( 0 );

   // Try to initialize this thread again (for error check)
   habAnchor = WinInitialize ( 0 );

   // Terminate this thread
   WinTerminate ( habAnchor );

   // Terminate this thread again
   WinTerminate ( habAnchor );
   return 0 ;
}


