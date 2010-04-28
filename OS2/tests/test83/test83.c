#define INCL_DOSFILEMGR   /* File Manager values */

 #define INCL_DOSERRORS    /* DOS error values */

 #include <os2.h>

 #include <stdio.h>



int main(VOID) {



 PVOID   pvDataBuffer    = NULL;     /* Arguments for the file system driver */

 APIRET  rc              = NO_ERROR; /* Return code */



    rc = DosFSAttach("Q:",                   /* Drive letter */

                     "\\LAN\\LANTOOLS",      /* Remote file system driver */

                     pvDataBuffer,           /* User-supplied arguments */

                     0,                      /* No arguments supplied */

                     FS_ATTACH);             /* Attach to the file system */



    if (rc != NO_ERROR) {

        printf("DosFSAttach error: return code = %u\n", rc);

        return 1;

    }



    return NO_ERROR;

}
