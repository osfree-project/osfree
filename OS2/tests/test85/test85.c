 #define INCL_DOSNLS        /* National Language Support values */

 #define INCL_DOSDATETIME   /* Date and time values */

 #define INCL_DOSERRORS     /* DOS error values */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {



 COUNTRYCODE  Country    = {0};   /* Country code info (0 = current country) */

 COUNTRYINFO  CtryInfo   = {0};   /* Buffer for country-specific information */

 ULONG        ulInfoLen  = 0;

 DATETIME     DateTime   = {0};       /* Date and time information           */

 APIRET       rc         = NO_ERROR;  /* Return code                         */



   rc = DosQueryCtryInfo(sizeof(CtryInfo), &Country,

                          &CtryInfo, &ulInfoLen);



   if (rc != NO_ERROR) {

       printf("DosQueryCtryInfo error: return code = %u\n",rc);

       return 1;

   }



   rc = DosGetDateTime(&DateTime);  /* Retrieve the current date and time  */



   if (rc != NO_ERROR) {

      printf ("DosGetDateTime error : return code = %u\n", rc);

      return 1;

   } else {

      switch (CtryInfo.fsDateFmt) {



      case(1):                                               /* dd/mm/yy */

      printf("Today is %d%s%d%s%d\n", DateTime.day, CtryInfo.szDateSeparator,

              DateTime.month, CtryInfo.szDateSeparator, DateTime.year);

        break;



      case(2):                                               /* yy/mm/dd */

      printf("Today is %d%s%d%s%d\n", DateTime.year, CtryInfo.szDateSeparator,

              DateTime.month, CtryInfo.szDateSeparator, DateTime.day);

        break;



      default:                                               /* mm/dd/yy */

      printf("Today is %d%s%d%s%d\n", DateTime.month, CtryInfo.szDateSeparator,

              DateTime.day, CtryInfo.szDateSeparator, DateTime.year);

        break;



       } /* endswitch */

   }

     return NO_ERROR;

   }
