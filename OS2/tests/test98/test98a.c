#define INCL_DOSFILEMGR          /* File Manager values      */
#define INCL_DOSERRORS           /* DOS Error values         */
#define INCL_DOSPROCESS          /* DOS Process values       */
#define INCL_DOSMISC             /* DOS Miscellanous values  */
#include <os2.h>
#include <stdio.h>
#include <string.h>

int main(void) {

PSZ      pszCommPort    = "COM1";     /* Port name, could use "\\DEV\COM1"  */
HFILE    hPort          = NULLHANDLE; /* Handle for accessing port          */
ULONG    ulAction       = 0L;         /* DosOpen action                     */
ULONG    ulWrote        = 0;          /* Number of bytes written to port    */
UCHAR    uchPortData[100] = " ";      /* Data to write to port              */
APIRET   rc             = NO_ERROR;   /* Return code                        */
DosError( FERR_DISABLEHARDERR);       /* Disable hard error pop-up messages */
rc = DosOpen( pszCommPort,            /* Communications port to open        */
              &hPort,
              &ulAction,              /* Returns action taken by DosOpen    */
              0L,                     /* Not needed for byte stream devices */
              FILE_NORMAL,
              OPEN_ACTION_OPEN_IF_EXISTS,
              OPEN_ACCESS_READWRITE |
              OPEN_FLAGS_NOINHERIT  |
              OPEN_SHARE_DENYREADWRITE ,  /* Prevents us from opening port      */
                                          /* if another application is using it */
                                          /* and prevents other applications    */
                                          /* from using port while we have it   */
              0L);                        /* No extended attributes             */
DosError( FERR_ENABLEHARDERR ) ;               / *   Re - enable   hard   error   pop - ups         * / 


if   ( rc   ! =   NO _ ERROR )   { 
   printf ( " DosOpen   error :   return   code   =   % u \ n " ,   rc ) ; 
   return   1 ; 
}   else   { 
   printf   ( " DosOpen :   Action   taken   =   % ld \ n " ,   ulAction ) ; 
}   / *   endif   * / 



