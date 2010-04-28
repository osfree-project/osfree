#define INCL_DOSNLS     /* DOS National Language Support values */

#define INCL_DOSERRORS  /* DOS Error values */

#include <os2.h>

#include <stdio.h>

#include <string.h>



#define COUNTRY_CODE 0       /* Country code             (0 = current) */

#define NLS_CODEPAGE 0       /* Code page for conversion (0 = current) */



int main(VOID) {



COUNTRYCODE Country       = {0};        /* Country code  */

CHAR        uchString[80] = "";         /* String        */

APIRET      rc            = 0;          /* Return code   */



   Country.country  = COUNTRY_CODE;     /* Country code  */



   Country.codepage = NLS_CODEPAGE;     /* Code page     */



   strcpy(uchString, "Capitalize this entire sTrInG, please!");



   printf("Original string is:  %s\n", uchString);



   rc = DosMapCase(sizeof(uchString),   /* Length of string to convert */

                   &Country,            /* Country and code page info  */

                   uchString);          /* String to convert           */



   if (rc != NO_ERROR) {

      printf("DosMapCase error: return code = %u\n", rc);

      return 1;

   } else {

      printf("Converted string is: %s\n", uchString);

   } /* endif */



   return NO_ERROR;

}

