#define INCL_DOSFILEMGR          /* File Manager values */

#define INCL_DOSERRORS           /* DOS Error values    */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {

   HFILE  hfFileHandle   = 0L;     /* Handle for file being manipulated */

   ULONG  ulAction       = 0;      /* Action taken by DosOpen */

   FHLOCK FileHandleLock = 0;      /* File handle lock   */



   ULONG  ulWrote        = 0;      /* Number of bytes written by DosWrite */

   UCHAR  uchFileName[20]  = "dospman.dat",     /* Name of file */

          uchFileData[4]   = "DATA";            /* Data to write to file */

   APIRET rc             = NO_ERROR;            /* Return code */



   /* Open the file dosman.dat.  Use an existing file or create a new */

   /* one if it doesn't exist.                                      */

   rc = DosProtectOpen(uchFileName, &hfFileHandle, &ulAction, 4L,

              FILE_ARCHIVED | FILE_NORMAL,

              OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,

              OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE  |

              OPEN_ACCESS_READWRITE, 0L, &FileHandleLock);

   if (rc != NO_ERROR) {

      printf("DosOpen error: return code = %u\n", rc);

      return 1;

   }



   rc = DosProtectWrite (hfFileHandle, (PVOID) uchFileData,

                  sizeof (uchFileData), &ulWrote, FileHandleLock);

   if (rc != NO_ERROR) {

      printf("DosWrite error: return code = %u\n", rc);

      return 1;

   }



   rc = DosResetBuffer (hfFileHandle);

   if (rc != NO_ERROR) {

      printf("DosResetBuffer error: return code = %u\n", rc);

      return 1;

   } /* endif */



   rc = DosProtectSetFileSize (hfFileHandle, 8L, FileHandleLock);

   if (rc != NO_ERROR) {

      printf("DosSetFileSize error: return code = %u\n", rc);

      return 1;

   }



   return NO_ERROR;

}
