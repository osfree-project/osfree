#define INCL_DOSFILEMGR         /* File Manager values */

#define INCL_DOSERRORS          /* DOS Error values */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {



ULONG   DriveNumber   = 1;                    /* Drive 1=A: 2=B: 3=C: ... */

VOLUMELABEL FSInfoBuf = {0};                  /* File system info buffer */

APIRET  rc            = NO_ERROR;             /* Return code */



strcpy(FSInfoBuf.szVolLabel, "MYDISK");    /* Change vol label to MYDISK */

FSInfoBuf.cch = (BYTE) strlen(FSInfoBuf.szVolLabel);



rc = DosSetFSInfo(DriveNumber,            /* Drive number */

                  FSIL_VOLSER,            /* Level of information being set */

                  &FSInfoBuf,             /* Address of input buffer */

                  sizeof(VOLUMELABEL) );  /* Buffer size */

if (rc != NO_ERROR) {

   printf("DosSetFSInfo error: return code = %u\n", rc);

   return 1;

}



return NO_ERROR;

}
