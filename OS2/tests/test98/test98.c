#define INCL_DOSFILEMGR          /* File Manager values */

#define INCL_DOSERRORS           /* DOS Error values    */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(void) {

   HFILE  hfFileHandle   = 0L;     /* Handle for file being manipulated */

   ULONG  ulAction       = 0;      /* Action taken by DosOpen */

   ULONG  ulBytesRead    = 0;      /* Number of bytes read by DosRead */

   ULONG  ulWrote        = 0;      /* Number of bytes written by DosWrite */

   ULONG  ulLocal        = 0;      /* File pointer position after DosSetFilePtr */

   UCHAR  uchFileName[20]  = "dostest.dat",     /* Name of file */

          uchFileData[100] = " ";               /* Data to write to file */

   APIRET rc             = NO_ERROR;            /* Return code */



   /* Open the file test.dat.  Use an existing file or create a new */

   /* one if it doesn't exist.                                      */

   rc = DosOpen(uchFileName,                    /* File path name */

                &hfFileHandle,                  /* File handle */

                &ulAction,                      /* Action taken */

                100L,                           /* File primary allocation */

                FILE_ARCHIVED | FILE_NORMAL,    /* File attribute */

                OPEN_ACTION_CREATE_IF_NEW |

                OPEN_ACTION_OPEN_IF_EXISTS,     /* Open function type */

                OPEN_FLAGS_NOINHERIT |

                OPEN_SHARE_DENYNONE  |

                OPEN_ACCESS_READWRITE,          /* Open mode of the file */

                0L);                            /* No extended attribute */



   if (rc != NO_ERROR) {

      printf("DosOpen error: return code = %u\n", rc);

      return 1;

   } else {

     printf ("DosOpen: Action taken = %ld\n", ulAction);

   } /* endif */

   /* Write a string to the file */

   strcpy (uchFileData, "testing...\n1...\n2...\n3\n");



   rc = DosWrite (hfFileHandle,                /* File handle */

                  (PVOID) uchFileData,         /* String to be written */

                  sizeof (uchFileData),        /* Size of string to be written */

                  &ulWrote);                   /* Bytes actually written */



   if (rc != NO_ERROR) {

      printf("DosWrite error: return code = %u\n", rc);

      return 1;

   } else {

      printf ("DosWrite: Bytes written = %u\n", ulWrote);

   } /* endif */



   /* Move the file pointer back to the beginning of the file */

   rc = DosSetFilePtr (hfFileHandle,           /* File Handle */

                       0L,                     /* Offset */

                       FILE_BEGIN,             /* Move from BOF */

                       &ulLocal);              /* New location address */

   if (rc != NO_ERROR) {

      printf("DosSetFilePtr error: return code = %u\n", rc);

      return 1;

   }



   /* Read the first 100 bytes of the file */

   rc = DosRead (hfFileHandle,                /* File Handle */

                 uchFileData,                 /* String to be read */

                 100L,                        /* Length of string to be read */

                 &ulBytesRead);               /* Bytes actually read */



   if (rc != NO_ERROR) {

      printf("DosRead error: return code = %u\n", rc);

      return 1;

   } else {

      printf ("DosRead: Bytes read = %u\n%s\n", ulBytesRead, uchFileData);

   } /* endif */



   rc = DosClose(hfFileHandle);                /* Close the file */



   if (rc != NO_ERROR) {

      printf("DosClose error: return code = %u\n", rc);

      return 1;

   }

   return NO_ERROR;

}

The following example shows how to open a communications port:



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



