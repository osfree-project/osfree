#define INCL_DOSFILEMGR          /* File Manager values */

#define INCL_DOSERRORS           /* DOS error values    */

#include <os2.h>

#include <stdio.h>



int main(void) {

   HFILE  hfFileHandle    = 0L;          /* File Handle */

   ULONG  ulAction        = 0;           /* Action taken */

   UCHAR  uchFileName[20] = "test.dat";  /* File path name */

   APIRET rc              = NO_ERROR;    /* Return code */



   /* Create the file test.dat */



   rc = DosOpen(uchFileName,

                &hfFileHandle,

                &ulAction,

                10L,

                FILE_NORMAL,

                FILE_CREATE,

                OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE,

                0L);

   if (rc != NO_ERROR) {

      printf("DosOpen error: return code = %u\n", rc);

      return 1;

   } else {

     printf("DosOpen:   File created = %s\n", uchFileName);

   } /* endif */



   rc = DosClose(hfFileHandle);        /* Close the file */

   if (rc != NO_ERROR) {

     printf("DosClose error: return code = %u\n", rc);

     return 1;

   } /* endif */



   /* Delete file "test.dat" from current directory */



   rc = DosDelete(uchFileName);

   if(rc != NO_ERROR) {

     printf("DosDelete error: return code = %u\n", rc);

     return 1;

   } else {

     printf("DosDelete: File deleted = %s\n", uchFileName);

   } /* endif */



   return NO_ERROR;

}
