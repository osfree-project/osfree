 #define INCL_DOSDATETIME   /* Date and time values */

 #define INCL_DOSERRORS     /* DOS error values     */

 #include <os2.h>

 #include <stdio.h>



int main(VOID) {



   DATETIME   DateTime = {0};       /* Structure to hold date/time info.   */

   APIRET     rc       = NO_ERROR;  /* Return code                         */



   rc = DosGetDateTime(&DateTime);  /* Retrieve the current date and time  */

   if (rc != NO_ERROR) {

      printf ("DosGetDateTime error : return code = %u\n", rc);

      return 1;

   }



   DateTime.hours = (UCHAR) ((BYTE) DateTime.hours + 1);    /* Set clock ahead

                                                 for Daylight Savings Time  */



   rc = DosSetDateTime(&DateTime);  /* Update the date and time            */



   if (rc!= NO_ERROR) {

      printf ("DosSetDateTime error : return code = %u\n", rc);

      return 1;

   }



   rc = DosGetDateTime(&DateTime);  /* Retrieve the date and time          */



   if (rc!= NO_ERROR) {

      printf ("DosGetDateTime error : return code = %u\n", rc);

      return 1;

   } else {

      printf("Today is %d-%d-%d; the time is now %d:%2.2d\n", DateTime.month,

              DateTime.day, DateTime.year, DateTime.hours, DateTime.minutes);

   }

   return NO_ERROR;

}
