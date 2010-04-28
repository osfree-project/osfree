#define INCL_DOSFILEMGR          /* File Manager values */

#define INCL_DOSERRORS           /* DOS Error values    */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {

   HFILE  hfFileHandle   = 0L;

   ULONG  ulAction       = 0;

   ULONG  ulBytesRead    = 0;

   ULONG  ulWrote        = 0;

   ULONG  ulLocal        = 0;

   UCHAR  uchFileName[20]  = "dosprot.dat",

          uchFileData[100] = " ";

   FHLOCK FileHandleLock = 0;        /* File handle lock   */

   APIRET rc             = NO_ERROR; /* Return code */



   /* Open the file test.dat.  Make it read/write, open it */

   /* if it already exists and create it if it is new.     */

   rc = DosProtectOpen(uchFileName,             /* File path name          */

                &hfFileHandle,                  /* File handle             */

                &ulAction,                      /* Action taken            */

                100L,                           /* File primary allocation */

                FILE_ARCHIVED | FILE_NORMAL,    /* File attribute          */

                OPEN_ACTION_CREATE_IF_NEW |

                OPEN_ACTION_OPEN_IF_EXISTS,     /* Open function type      */

                OPEN_FLAGS_NOINHERIT |

                OPEN_SHARE_DENYNONE  |

                OPEN_ACCESS_READWRITE,          /* Open mode of the file   */

                0L,                             /* No extended attribute   */

                &FileHandleLock);               /* File handle lock id     */

   if (rc != NO_ERROR) {

      printf("DosProtectOpen error: return code = %u\n", rc);

      return 1;

   } else {

     printf ("DosProtectOpen: Action taken = %u\n", ulAction);

   } /* endif */



   /* Write a string to the file */

   strcpy (uchFileData, "testing...\n3...\n2...\n1\n");



   rc = DosProtectWrite (hfFileHandle,       /* File handle                  */

                  (PVOID) uchFileData,       /* String to be written         */

                  sizeof (uchFileData),      /* Size of string to be written */

                  &ulWrote,                  /* Bytes actually written       */

                  FileHandleLock);           /* File handle lock id          */

   if (rc != NO_ERROR) {

      printf("DosProtectWrite error: return code = %u\n", rc);

      return 1;

   } else {

      printf ("DosProtectWrite: Bytes written = %u\n", ulWrote);

   } /* endif */



   /* Move the file pointer back to the beginning of the file */

   rc = DosProtectSetFilePtr (hfFileHandle,    /* File Handle          */

                       0L,                     /* Offset               */

                       FILE_BEGIN,             /* Move from BOF        */

                       &ulLocal,               /* New location address */

                       FileHandleLock);        /* File handle lock id  */

   if (rc != NO_ERROR) {

      printf("DosSetFilePtr error: return code = %u\n", rc);

      return 1;

   }



   /* Read the first 100 bytes of the file */

   rc = DosProtectRead (hfFileHandle,         /* File Handle                 */

                 uchFileData,                 /* String to be read           */

                 100L,                        /* Length of string to be read */

                 &ulBytesRead,                /* Bytes actually read         */

                 FileHandleLock);             /* File handle lock id         */

   if (rc != NO_ERROR) {

      printf("DosProtectRead error: return code = %u\n", rc);

      return 1;

   } else {

      printf("DosProtectRead: Bytes read = %u\n%s\n", ulBytesRead, uchFileData);

   } /* endif */



   rc = DosProtectClose(hfFileHandle, FileHandleLock);   /* Close the file */

   if (rc != NO_ERROR) {

      printf("DosProtectClose error: return code = %u\n", rc);

      return 1;

    }

   return NO_ERROR;

}

