 #define INCL_DOSMISC

 #define INCL_DOSERRORS

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {



 UCHAR   uchBeginLIBPATH[512] = "";       /* Begin LIBPATH value returned */

 UCHAR   uchEndLIBPATH[512]   = "";       /* End LIBPATH value returned   */

 APIRET  rc                   = NO_ERROR;    /* Return code               */



    rc = DosSetExtLIBPATH("C:\\TOOL_X\\VERS_20\\DLL",

                          BEGIN_LIBPATH);    /* Add to beginning LIBPATH */

    if (rc != NO_ERROR) {

        printf("DosSetExtLIBPATH error: return code = %u\n", rc);

        return 1;

    }



    rc = DosSetExtLIBPATH("C:\\TOOL_X\\VERS_10\\DLL",

                          END_LIBPATH);      /* Add to ending LIBPATH   */

    if (rc != NO_ERROR) {

        printf("DosSetExtLIBPATH error: return code = %u\n", rc);

        return 1;

    }



    rc = DosQueryExtLIBPATH(uchBeginLIBPATH,

                            BEGIN_LIBPATH);   /* Query the BeginLIBPATH */

    if (rc != NO_ERROR) {

        printf("DosQueryExtLIBPATH error: return code = %u\n", rc);

        return 1;

    }



    rc = DosQueryExtLIBPATH(uchEndLIBPATH,

                            END_LIBPATH);     /* Query the EndLIBPATH   */

    if (rc != NO_ERROR) {

        printf("DosQueryExtLIBPATH error: return code = %u\n", rc);

        return 1;

    }



    printf(" BeginLIBPATH = %s\n", uchBeginLIBPATH);

    printf("   EndLIBPATH = %s\n", uchEndLIBPATH);



  return NO_ERROR;

}
