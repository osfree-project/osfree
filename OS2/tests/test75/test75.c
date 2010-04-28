 #define INCL_DOSFILEMGR   /* File Manager values */

 #define INCL_DOSERRORS    /* DOS error values */

 #include <os2.h>

 #include <stdio.h>



 int main (VOID) {

    HDIR          hdirFindHandle = HDIR_CREATE;

    FILEFINDBUF3  FindBuffer     = {0};      /* Returned from FindFirst/Next */

    ULONG         ulResultBufLen = sizeof(FILEFINDBUF3);

    ULONG         ulFindCount    = 1;        /* Look for 1 file at a time    */

    APIRET        rc             = NO_ERROR; /* Return code                  */



    rc = DosFindFirst( "*.*",                /* File pattern - all files     */

                       &hdirFindHandle,      /* Directory search handle      */

                       FILE_NORMAL,          /* Search attribute             */

                       &FindBuffer,          /* Result buffer                */

                       ulResultBufLen,       /* Result buffer length         */

                       &ulFindCount,         /* Number of entries to find    */

                       FIL_STANDARD);        /* Return level 1 file info     */



    if (rc != NO_ERROR) {

       printf("DosFindFirst error: return code = %u\n",rc);

       return 1;

    } else {

       printf ("%s\n", FindBuffer.achName);   /* Print file name             */

    } /* endif */



    /* Keep finding the next file until there are no more files */

    while (rc != ERROR_NO_MORE_FILES) {

       ulFindCount = 1;                      /* Reset find count.            */



       rc = DosFindNext(hdirFindHandle,      /* Directory handle             */

                        &FindBuffer,         /* Result buffer                */

                        ulResultBufLen,      /* Result buffer length         */

                        &ulFindCount);       /* Number of entries to find    */



       if (rc != NO_ERROR && rc != ERROR_NO_MORE_FILES) {

          printf("DosFindNext error: return code = %u\n",rc);

          return 1;

       } else {

          printf ("%s\n", FindBuffer.achName);    /* Print file name */

       }

    } /* endwhile */



    rc = DosFindClose(hdirFindHandle);    /* Close our directory handle */

    if (rc != NO_ERROR) {

       printf("DosFindClose error: return code = %u\n",rc);

       return 1;

    }

    return NO_ERROR;

 }
