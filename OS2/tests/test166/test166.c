#define INCL_DOSFILEMGR   /* File Manager values */

#define INCL_DOSERRORS    /* DOS Error values    */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {

   UCHAR   achOrigDirName[256] = "";            /* Original directory name  */

   UCHAR   achNewDirName[256]  = "\\TEMPPROG";  /* New directory to create  */

   UCHAR   achDirName[256]     = "";          /* Directory name for queries */

   ULONG   cbDirPathLen    = 0;               /* Length of directory path   */

   PEAOP2  pEABuf       = NULL;   /* Extended Attribute buffer pointer      */

   ULONG   ulDriveNum   = 0;      /* Drive number: current=0, A=1, B=2, ... */

   APIRET  rc           = NO_ERROR;      /* Return code                     */



   cbDirPathLen = (ULONG) sizeof(achOrigDirName);

   rc = DosQueryCurrentDir(ulDriveNum, achOrigDirName, &cbDirPathLen);



   if (rc != NO_ERROR) {

      printf("DosQueryCurrentDir error: return code = %u\n", rc);

      return 1;

   } else printf ("Original dir. = \\%s\n", achOrigDirName);



   pEABuf = NULL;  /* Indicate no EAs are to be defined for the directory  */

   rc = DosCreateDir(achNewDirName, pEABuf);   /* Create the new directory */



   if (rc != NO_ERROR) {

      printf("DosCreateDir error: return code = %u\n", rc);

      return 1;

   }



   rc = DosSetCurrentDir (achNewDirName);   /* Change to new directory     */



   ulDriveNum = 0;

   cbDirPathLen = (ULONG) sizeof(achDirName);

   rc = DosQueryCurrentDir(ulDriveNum, achDirName, &cbDirPathLen);



   if (rc != NO_ERROR) {

      printf("DosQueryCurrentDir error: return code = %u\n", rc);

      return 1;

   } else printf ("Current dir.  = \\%s\n", achDirName);



   strcpy(achDirName,"\\");



   rc = DosSetCurrentDir (achDirName);     /* Change to root directory     */

   rc = DosDeleteDir (achNewDirName);      /* Delete the new directory     */



   return NO_ERROR;

}
