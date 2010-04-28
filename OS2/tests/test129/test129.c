#define INCL_DOSNLS     /* National Language Support values */

#define INCL_DOSERRORS  /* DOS Error values */

#include <os2.h>

#include <stdio.h>



int main(VOID) {

COUNTRYCODE  ctrycodeInfo      = {0};        /* Country code information */

UCHAR        uchDBCSInfo[12]   = {0};        /* DBCS information buffer */

APIRET       rc                = NO_ERROR;   /* A return code */



ctrycodeInfo.country = 0;    /* Current country */

ctrycodeInfo.codepage = 0;   /* Current codepage */



rc = DosQueryDBCSEnv(sizeof(uchDBCSInfo),   /* Size of buffer */

                     &ctrycodeInfo,         /* Country code information */

                     uchDBCSInfo);          /* DBCS information buffer */



if (rc != NO_ERROR) {

   printf("DosQueryDBCSEnv error: return code = %u\n", rc);

   return 1;

 } else {

      /* For non-DBCS countries, these will be 4 bytes of 0 */

   printf("DBCS 1st range definition: %2.2x %2.2x\n",

                            uchDBCSInfo[0], uchDBCSInfo[1]);

   printf("     2nd range definition: %2.2x %2.2x\n",

                            uchDBCSInfo[2], uchDBCSInfo[3]);

 } /* endif */





return NO_ERROR;

}
