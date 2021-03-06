#define INCL_DOSFILEMGR   /* File Manager values */

 #define INCL_DOSERRORS    /* DOS error values    */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {



 UCHAR       uchFileName[]   = "DOSFDEL.DAT";   /* File we want to delete    */

 HFILE       fhDelFile       = 0;               /* File handle from DosOpen  */

 FILESTATUS3 fsts3FileInfo   = {{0}};  /* Information associated with file   */

 ULONG       ulBufferSize    = sizeof(FILESTATUS3); /* File info buffer size */

 ULONG       ulOpenAction    = 0;                 /* Action taken by DosOpen */

 APIRET      rc              = NO_ERROR;          /* Return code             */

 FHLOCK      FHLock          = 0;                 /* File handle lock        */



                 /* Create a read-only file */



  rc = DosProtectOpen(uchFileName, &fhDelFile,

               &ulOpenAction, 10L, FILE_READONLY,

               OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,

               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE, 0L, &FHLock);

  if (rc != NO_ERROR) {

     printf("DosProtectOpen error: return code = %u\n", rc);

     return 1;

  }



  rc = DosProtectQueryFileInfo(fhDelFile, FIL_STANDARD,

               &fsts3FileInfo, ulBufferSize, FHLock);   /* Get standard info */

  if (rc != NO_ERROR) {

      printf("DosProtectQueryFileInfo error: return code = %u\n", rc);

      return 1;

  } else { printf("File %s created read-only.\n",uchFileName); }



    /*   Change the file attributes to be "normal"  */



    fsts3FileInfo.attrFile  = FILE_NORMAL;

    rc = DosProtectSetFileInfo(fhDelFile, FIL_STANDARD,

                        &fsts3FileInfo, ulBufferSize, FHLock);

    if (rc != NO_ERROR) {

        printf("DosProtectSetFileInfo error: return code = %u\n", rc);

        return 1;

    }



    rc = DosProtectClose(fhDelFile, FHLock);

    /* Should verify that (rc != NO_ERROR) here... */



           /* Delete the file */



    rc = DosForceDelete(uchFileName);

    if (rc != NO_ERROR) {

        printf("DosForceDelete error: return code = %u\n", rc);

        return 1;

    } else {

        printf("File %s has been deleted.\n",uchFileName);

    }  /* endif */



   return NO_ERROR;

}
