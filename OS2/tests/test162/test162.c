 #define INCL_DOS

 #define INCL_DOSERRORS       /* DOS error values */

 #include <os2.h>

 #include <stdio.h>



int main(VOID) {



   PSZ       PathValue         = "";        /* PATH environment variable    */

   UCHAR     SearchResult[256] = "";        /* Result of PATH search        */

   APIRET    rc                = NO_ERROR;  /* Return code                  */



   rc=DosScanEnv("PATH",&PathValue);  /* Get contents of PATH environment

                                         variable                         */

   if (rc != NO_ERROR) {

       printf("DosScanEnv error: return code = %u\n",rc);

       return 1;

   } else {

       printf("PATH is:\n%s\n\n", PathValue);

   }

      /* Scan the current directory and path for the VIEW.EXE program.

         Ignore any errors from network drives which may not be in use. */



   rc=DosSearchPath(SEARCH_CUR_DIRECTORY | SEARCH_IGNORENETERRS,

                    PathValue,               /* Path value just obtained */

                    "VIEW.EXE",              /* Name of file to look for */

                    SearchResult,            /* Result of the search     */

                    sizeof(SearchResult));   /* Length of search buffer  */



   if (rc != NO_ERROR) {

       printf("DosSearchPath error: return code = %u\n",rc);

       return 1;

   } else {

       printf("Found desired file -- %s\n", SearchResult);

   }

  return NO_ERROR;

 }
