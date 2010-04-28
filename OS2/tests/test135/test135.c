 #define INCL_DOSFILEMGR   /* File manager values  */

 #define INCL_DOSERRORS    /* DOS error values     */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {



 typedef struct _FSINFOBUF

  {  ULONG        ulVolser;   /* Volume serial number */

     VOLUMELABEL  vol;        /* Volume label         */

  } FSINFOBUF;

 typedef FSINFOBUF *PFSINFOBUF;



 ULONG       ulDriveNumber = 0;          /* Drive number */

 FSINFOBUF   VolumeInfo    = {0};        /* File system info buffer */

 APIRET      rc            = NO_ERROR;   /* Return code */



    ulDriveNumber = 3;          /* Specify drive C (A=1, B=2, C=3, ...) */



    rc = DosQueryFSInfo(ulDriveNumber,

                        FSIL_VOLSER,      /* Request volume information */

                        &VolumeInfo,      /* Buffer for information     */

                        sizeof(FSINFOBUF));  /* Size of buffer          */



    if (rc != NO_ERROR) {

        printf("DosQueryFSInfo error: return code = %u\n", rc);

        return 1;

    } else {

        printf("Volume label: %s\n", VolumeInfo.vol.szVolLabel);

    }

    return NO_ERROR;

  }


#define INCL_DOSFILEMGR   /* File Manager values */

#define INCL_DOSERRORS    /* DOS Error values    */

#include <os2.h>

#include <stdio.h>



int main (VOID) {

   ULONG   aulFSInfoBuf[40] = {0};         /* File system info buffer     */

   APIRET  rc               = NO_ERROR;    /* Return code                 */



   rc = DosQueryFSInfo(3L,                     /* Drive number 3 (C:)     */

                       FSIL_ALLOC,             /* Level 1 allocation info */

                       (PVOID)aulFSInfoBuf,    /* Buffer                  */

                       sizeof(aulFSInfoBuf));  /* Size of buffer          */



   if (rc != NO_ERROR) {

      printf("DosQueryFSInfo error: return code = %u\n", rc);

      return 1;

   } else {

      printf ("%12ld bytes in each allocation unit.\n",

               aulFSInfoBuf[1] * (USHORT)aulFSInfoBuf[4]);

             /* (Sectors per allocation unit) * (Bytes per sector) */

      printf ("%12ld total allocation units.\n", aulFSInfoBuf[2]);

      printf ("%12ld available allocation units on disk.\n", aulFSInfoBuf[3]);

   }

   DosExit(EXIT_THREAD,aulFSInfoBuf[3]);  /* Return available allocation units

                                             to the initiating process      */

   return NO_ERROR;

}
