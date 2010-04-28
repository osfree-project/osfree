#define INCL_DOSFILEMGR   /* File Manager values */

#define INCL_DOSERRORS    /* DOS Error values    */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {

   ULONG   ulDriveNum   = 0;      /* Drive number (A=1, B=2, C=3, ...)    */

   ULONG   ulDriveMap   = 0;      /* Mapping of valid drives              */

   ULONG   i            = 0;      /* A loop index                         */

   APIRET  rc           = NO_ERROR;  /* Return code                       */



   rc = DosQueryCurrentDisk (&ulDriveNum, &ulDriveMap);



   if (rc != NO_ERROR) {

      printf ("DosQueryCurrentDisk error : return code = %u\n", rc);

      return 1;

   }

   printf ("Current disk = %c\n", 'A' + ulDriveNum - 1);

   printf ("Logical disks: ");

   printf ("A B C D E F G H I J K L M N O P Q R S T U V W X Y Z\n");

   printf ("        valid? ");



     /* Each bit in the ulDriveMap corresponds to a specific logical drive.

        bit 0 = A:, bit 1 = B:, ... , bit 24 = Y:, bit 25 = Z:

        For each drive, shift the bit string to the left to get rid of

        the bits before the one we want, then shift that result right 31

        bits to leave just the one we are interested in. */



  for (i = 0; i < 26; i++) {

     printf (( (ulDriveMap<<(31-i)) >> 31) ? "Y " : "- ");

    }

   printf ("\n");                          /* Print a newline character */



   return NO_ERROR;

}
