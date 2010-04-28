 #define INCL_DOSDEVICES   /* Device values    */

 #define INCL_DOSERRORS    /* DOS error values */

 #include <os2.h>

 #include <stdio.h>



int main (VOID) {



 USHORT  usNumDrives  = 0;                  /* Data return buffer        */

 ULONG   ulDataLen    = sizeof(USHORT);     /* Data return buffer length */

 APIRET  rc           = NO_ERROR;           /* Return code               */



 /* Request a count of the number of partitionable disks in the system */



    rc = DosPhysicalDisk(INFO_COUNT_PARTITIONABLE_DISKS,

                         &usNumDrives,

                         ulDataLen,

                         NULL,         /* No parameter for this function */

                         0L);



    if (rc != NO_ERROR) {

       printf("DosPhysicalDisk error: return code = %u\n", rc);

       return 1;

    } else {

       printf("DosPhysicalDisk:  %u partitionable disk(s)\n",usNumDrives);

    }



   return NO_ERROR;

}
