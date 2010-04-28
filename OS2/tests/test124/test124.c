 #define INCL_DOSNLS     /* National Language Support values */

 #define INCL_DOSERRORS  /* DOS error values                */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {

 COUNTRYCODE UserInfo       = {0};       /* Country and code page requested */

 UCHAR       achColSeq[256] = {0};       /* Collating sequence              */

 ULONG       ulSeqLen       = 0;         /* Length of sequence returned     */

 ULONG       i              = 0,         /* Two loop indices                */

             j              = 0;

 APIRET      rc             = NO_ERROR;  /* Return code                     */



    UserInfo.country = 0;      /* Request information about current country */

    UserInfo.codepage = 0;     /* ... and current code page                 */



    rc = DosQueryCollate(sizeof(achColSeq),   /* Length of output area      */

                         &UserInfo,           /* Country and codepage info  */

                         achColSeq,           /* Area for collating sequence*/

                         &ulSeqLen);          /* Length of data returned    */



    if (rc != NO_ERROR) {

        printf("DosQueryCollate error: return code = %u\n",rc);

        return 1;

    }

         /* Show the order of the first 128 characters in the sequence */



   if (ulSeqLen >= 8*16) {

     for (i = 0; i < 8; i++) {

       for (j = 0; j < 16; j++) {

          printf("%3u ", achColSeq[i*16+j]);

          } /* endfor (j) */

       printf("\n");

       } /* endfor (i) */

   } else {

      printf("Unable to show first 128 characters... only %u were returned.\n",

              ulSeqLen);

      return 1;

   }  /* endif */



   return NO_ERROR;

}
