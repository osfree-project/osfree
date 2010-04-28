 #define INCL_DOSFILEMGR   /* File Manager values */

 #define INCL_DOSERRORS    /* DOS error values    */

 #include <os2.h>

 #include <stdio.h>

 #include <string.h>



int main(VOID) {

   UCHAR   achSourceString[80]  = "config.sys"; /* String to transform */

   UCHAR   achEditString[80]    = "*.cpy";      /* Editing string */

   UCHAR   achTargetString[200] = "";           /* Destination string buffer */

   APIRET  rc                   = NO_ERROR;     /* Return code */



   rc = DosSetDefaultDisk(3);    /* Set drive to C: (1=A, 2=B, 3=C, ...)      */

   if (rc != NO_ERROR) {

      printf("DosSetDefaultDisk error: return code = %u\n", rc);

      return 1;

   }



   rc = DosSetCurrentDir ("\\");  /*  Set directory to root */

   if (rc != NO_ERROR) {

      printf("DosSetCurrentDir error: return code = %u\n", rc);

      return 1;

   }



           /* Transform "CONFIG.SYS" using "*.CPY" to "CONFIG.CPY" */



   rc = DosEditName(1, achSourceString, achEditString, achTargetString, 200);



   if (rc != NO_ERROR) {

      printf("DosEditName error: return code = %u\n", rc);

      return 1;

   }



           /* Copy contents of CONFIG.SYS to the backup file */



   rc = DosCopy(achSourceString,             /* Name of file to be copied    */

                achTargetString,             /* Name of the target file      */

                DCPY_EXISTING);   /* Copy even if target file already exists */



   if (rc != NO_ERROR) {

      printf("DosCopy error: return code = %u\n", rc);

      return 1;

   } else  printf ("Backup file %s created.\n", achTargetString);

   return NO_ERROR;

}
