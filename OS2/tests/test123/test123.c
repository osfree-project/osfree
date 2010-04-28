#define INCL_DOSSESMGR   /* Session Manager values */

#define INCL_DOSERRORS   /* DOS error values       */

#include <os2.h>

#include <stdio.h>



int main (VOID) {

   PSZ      szAppName = "C:\\OS2\\SYSLOG.EXE";  /* Application name      */

   ULONG    AppType = 0;            /* Application type flags (returned) */

   APIRET   rc      = NO_ERROR;     /* Return code                       */



   rc = DosQueryAppType(szAppName, &AppType);

                     /* On successful return, the AppType      */

                     /*   variable contains a set of bit flags */

                     /*   that describe the application type   */

                     /*   of the specified executable file     */



   if (rc != NO_ERROR) {

      printf("DosQueryAppType error: return code = %u\n", rc);

      return 1;

   } else {

    printf("Appname = %s\n", szAppName);

    printf("Apptype = %d\n", AppType & FAPPTYP_EXETYPE);

    printf(" Window API?     %s\n", (AppType & FAPPTYP_WINDOWAPI) ? "Y" : "N");

    printf(" Window compat?  %s\n", (AppType & FAPPTYP_WINDOWCOMPAT) ? "Y" : "N");

    printf(" Family API?     %s\n", (AppType & FAPPTYP_BOUND) ? "Y" : "N");

    printf(" PC/DOS format?  %s\n", (AppType & FAPPTYP_DOS) ? "Y" : "N");

    printf(" DLL?            %s\n",

                      (AppType & (FAPPTYP_DLL | FAPPTYP_PROTDLL) ) ? "Y" : "N");

   }

   return NO_ERROR;

}
