#define INCL_DOSMISC    /* Miscellaneous values */

#define INCL_DOSERRORS  /* DOS Error values */

#include <os2.h>

#include <stdio.h>



int main(VOID) {



typedef struct _MYCPINFO

 {  USHORT       cCP;          /* Number of code pages */

    USHORT       CP1;          /* First code page      */

    ULONG        dtype;

 } MYCPINFO;



UCHAR     uchBuffer[20] = {0};            /* Buffer for the returned list */

MYCPINFO  CPinfo         = {0};           /* Pointer to buffer */

ULONG     ulDataLen      = 0;             /* Length of data returned */

APIRET    rc             = NO_ERROR;      /* Return code */



 rc = DosQueryMessageCP( (PUCHAR) &CPinfo,

                         sizeof(MYCPINFO),

                         "SAMPLE.MSG",

                         &ulDataLen );



 if (rc != NO_ERROR) {

      printf("DosQueryMessageCP error: return code = %u\n", rc);

      return 1;

 } else {

      printf("SAMPLE.MSG:  %u Code Page(s) - Code Page 1 = %u.\n",

                                        CPinfo.cCP, CPinfo.CP1);

 } /* endif */



return NO_ERROR;

}
