 #define INCL_DOSFILEMGR   /* File Manager values */

 #define INCL_DOSERRORS    /* DOS error values    */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {

 UCHAR        uchFileName[80] = "C:\\STARTUP.CMD";  /* File to manipulate    */

 FILESTATUS3  fsts3ConfigInfo = {{0}};       /* Buffer for file information */

 ULONG        ulBufSize     = sizeof(FILESTATUS3);  /* Size of above buffer */

 APIRET       rc            = NO_ERROR;      /* Return code                 */



    rc = DosQueryPathInfo(uchFileName,   /* Path and name of file           */

                          FIL_STANDARD,  /* Request standard (Level 1) info */

                          &fsts3ConfigInfo, /* Buffer for file information  */

                          ulBufSize);    /* Size of buffer                  */

    if (rc != NO_ERROR) {

        printf("DosQueryPathInfo error: return code = %u\n", rc);

        return 1;

    }



    printf("%s ---  File size: %u bytes\n",uchFileName, fsts3ConfigInfo.cbFile);

    printf("Last updated: %d/%d/%d; %d:%2.2d\n",

            fsts3ConfigInfo.fdateLastWrite.month,        /* Month            */

            fsts3ConfigInfo.fdateLastWrite.day,          /* Day              */

            (fsts3ConfigInfo.fdateLastWrite.year+1980L), /* Years since 1980 */

            fsts3ConfigInfo.ftimeLastWrite.hours,        /* Hours            */

            fsts3ConfigInfo.ftimeLastWrite.minutes);     /* Minutes          */



  return NO_ERROR;

}


