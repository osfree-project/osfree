#define INCL_DOSFILEMGR          /* File Manager values */

#define INCL_DOSERRORS           /* DOS Error values    */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {

   UCHAR  uchFileName[20]  = "VERIFY.DAT",     /* Name of file to use      */

          uchFileData[10] = "SampleData";      /* Stuff to put in file     */

   HFILE  hfFileHandle   = 0L;                 /* Handle for user file     */

   BOOL32 fUserVerify    = 0;                  /* User Verify flag setting */

   ULONG  ulAction       = 0,                  /* Action done by DosOpen   */

          ulWritten      = 0;                  /* Number of bytes written  */

   APIRET rc             = NO_ERROR;           /* Return code              */



   rc = DosQueryVerify(&fUserVerify);     /* Get current setting of VERIFY */

   if (rc != NO_ERROR) {

     printf("DosQueryVerify error: return code = %u\n", rc);

     return 1;

   } else {

     printf ("Original setting of Verify=%s\n",(fUserVerify) ? "On" : "Off");

   } /* endif */



   rc = DosSetVerify(1);                  /* Set VERIFY=ON                 */

   if (rc != NO_ERROR) {

     printf("DosSetVerify error: return code = %u\n", rc);

     return 1;  }



   /* Open the file VERIFY.DAT for read/write.  Create it if necessary */



   rc = DosOpen(uchFileName, &hfFileHandle, &ulAction,

                10L, FILE_NORMAL,

                OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,

                OPEN_SHARE_DENYNONE  | OPEN_ACCESS_READWRITE, 0L);



   if (rc != NO_ERROR) {

      printf("DosOpen error: return code = %u\n", rc);

      return 1; }



         /* Write critical data to the file */



   rc = DosWrite (hfFileHandle, (PVOID) uchFileData,

                  sizeof(uchFileData), &ulWritten);

   if (rc == NO_ERROR) {

      printf ("%u bytes written to file %s with Verify=On\n",

               ulWritten,uchFileName);

   }

   rc = DosSetVerify(fUserVerify);         /* Restore user's verify value */

   if (rc != NO_ERROR) {

     printf("DosSetVerify error: return code = %u\n", rc);

     return 1;

   }



   rc = DosClose(hfFileHandle);            /* Close the file              */



   return NO_ERROR;

}
