#define INCL_DOSFILEMGR   /* File Manager values */
#define INCL_DOSERRORS    /* DOS error values    */
#include <os2.h>
#include <stdio.h>
#include <string.h>

#include "minicmd.h"

int dir_cmd (int argc, char **argv)
{
    HDIR          hdirFindHandle = HDIR_CREATE;
    FILEFINDBUF3  FindBuffer     = {0};      /* Returned from FindFirst/Next */
    ULONG         ulResultBufLen = sizeof(FILEFINDBUF3);
    ULONG         ulFindCount    = 1;        /* Look for 1 file at a time    */
    APIRET        rc             = NO_ERROR; /* Return code                  */
    char          *mask;

    if (argc > 2)
      return 1;

    if (argc == 1)
      mask = strdup("*");
    else
      mask = argv[1];

    rc = DosFindFirst (mask,                 /* File pattern - all files     */
                       &hdirFindHandle,      /* Directory search handle      */
                       FILE_NORMAL,          /* Search attribute             */
                       &FindBuffer,          /* Result buffer                */
                       ulResultBufLen,       /* Result buffer length         */
                       &ulFindCount,         /* Number of entries to find    */
                       FIL_STANDARD);        /* Return Level 1 file info     */

    if (argc == 1)
      free(mask);

    if (rc)
    {
      rc = DosFindClose(hdirFindHandle);    /* Close our directory handle */
      putchar('\n');
      return 0;
    }

    printf ("%s  ", FindBuffer.achName);   /* Print file name             */

    /* Keep finding the next file until there are no more files */
    for (;;)
    {
       ulFindCount = 1;                      /* Reset find count.            */

       rc = DosFindNext(hdirFindHandle,      /* Directory handle             */
                        &FindBuffer,         /* Result buffer                */
                        ulResultBufLen,      /* Result buffer length         */
                        &ulFindCount);       /* Number of entries to find    */

       if (rc)
          break;

          printf ("%s  ", FindBuffer.achName);    /* Print file name */
    }

    rc = DosFindClose(hdirFindHandle);    /* Close our directory handle */
    putchar('\n');

    return 0;
}
