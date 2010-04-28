#define INCL_DOSFILEMGR   /* File Manager values */

#define INCL_DOSERRORS    /* DOS Error values    */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {

   UCHAR       achNewDir[256]  = "\\HIDEME";           /* Directory name    */

   FILESTATUS3 fsts3PathInfo   = {{0}};                /* Directory info    */

   ULONG       ulBufferSize    = sizeof(FILESTATUS3);  /* Buffer size       */

   APIRET      rc              = NO_ERROR;             /* Return code       */



   rc = DosCreateDir(achNewDir, (PEAOP2) NULL);        /* Create directory

                                                          with no EAs       */

   if (rc != NO_ERROR) {

      printf("DosCreateDir error: return code = %u\n", rc);

      return 1;

   } else {

      printf("Directory %s created.\n",achNewDir);

   }



   rc = DosQueryPathInfo(achNewDir, FIL_STANDARD,

                        &fsts3PathInfo, ulBufferSize); /* Get standard info */

   if (rc != NO_ERROR) {

      printf("DosQueryPathInfo error: return code = %u\n", rc);

      return 1;

   }



   fsts3PathInfo.attrFile  = FILE_HIDDEN;   /* Add HIDDEN attribute to path */



   rc = DosSetPathInfo(achNewDir,           /* Change directory info on     */

                       FIL_STANDARD,        /* the disk using the buffer    */

                       &fsts3PathInfo,      /* we just updated.             */

                       ulBufferSize,

                       DSPI_WRTTHRU );      /* Write data before returning  */

   if (rc != NO_ERROR) {

       printf("DosSetPathInfo error: return code = %u\n", rc);

       return 1;

   } else {

       printf("Directory %s hidden.\n",achNewDir);

   }

       /* Delete the hidden directory.  If this step is omitted, the directory

          can still be manipulated by standard OS/2 commands like CHDIR and

          RMDIR, it will just not be displayed in a DIR command without the

          /AH display option specified.                                     */



   rc = DosDeleteDir (achNewDir);

   if (rc != NO_ERROR) {

       printf ("DosDeleteDir error : return code = %u\n", rc);

       return 1;

   } else {

       printf("Directory %s deleted.\n",achNewDir);

   }



   return NO_ERROR;

}
