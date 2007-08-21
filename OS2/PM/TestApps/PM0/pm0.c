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
   HAB         habAnchor ;

   // Initialize this thread
   habAnchor = WinInitialize ( 0 ) ;

   // Terminate this thread
   WinTerminate ( habAnchor ) ;
   return 0 ;
}


