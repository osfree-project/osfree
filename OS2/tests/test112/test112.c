 #define INCL_DOSFILEMGR   /* File Manager values */

 #define INCL_DOSERRORS    /* DOS error values    */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {



 UCHAR       uchFileName[]   = "DOSPQFH.DAT";   /* File to manipulate        */

 HFILE       fhQryFile       = 0;               /* File handle from DosOpen  */

 FILESTATUS3 fsts3FileInfo   = {{0}};  /* Information associated with file   */

 ULONG       ulOpenAction    = 0;                 /* Action taken by DosOpen */

 ULONG       FHState         = 0;                 /* File Handle State       */

 APIRET      rc              = NO_ERROR;          /* Return code             */

 FHLOCK      FileHandleLock  = 0;                 /* File handle lock        */



  rc = DosProtectOpen(uchFileName, &fhQryFile,

               &ulOpenAction, 10L, FILE_NORMAL,

               OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,

               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE, 0L,

               &FileHandleLock);

  if (rc != NO_ERROR) {

     printf("DosProtectOpen error: return code = %u\n", rc);

     return 1;

  }



  rc = DosProtectQueryFHState(fhQryFile, &FHState, FileHandleLock);

  if (rc != NO_ERROR) {

      printf("DosProtectQueryFHState error: return code = %u\n", rc);

      return 1;

  } else printf("FHState is: %x\n", FHState);



    /*   Change state to indicate that data should not be cached */



    FHState &= 0x7F88;                  /* Turn off non-participating bits */

    rc = DosProtectSetFHState(fhQryFile, FHState | OPEN_FLAGS_NO_CACHE,

                              FileHandleLock);

    if (rc != NO_ERROR) {

        printf("DosProtectSetFHState error: return code = %u\n", rc);

        return 1;

    }



    rc = DosProtectClose(fhQryFile, FileHandleLock);

    /* Should check if (rc != NO_ERROR) here */



    rc = DosDelete(uchFileName);      /* Delete the file */

    if (rc != NO_ERROR) {

        printf("DosDelete error: return code = %u\n", rc);

        return 1;

    } else {

        printf("File %s has been deleted.\n",uchFileName);

    } /* endif */



   return NO_ERROR;

}
