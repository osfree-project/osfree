#define INCL_DOSFILEMGR       /* File Manager values */

#define INCL_DOSERRORS        /* DOS Error values    */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {



HFILE     FileHandle   = NULLHANDLE;  /* File handle */

ULONG     Action       = 0,           /* Action taken by DosOpen */

          Wrote        = 0;           /* Number of bytes written by DosWrite */

CHAR      FileData[40] = "Forty bytes of demonstration text data\r\n";

APIRET    rc           = NO_ERROR;    /* Return code */



FILELOCK  LockArea     = {0},         /* Area of file to lock */

          UnlockArea   = {0};         /* Area of file to unlock */



rc = DosOpen("canlock.dat",                 /* File to open */

             &FileHandle,                   /* File handle */

             &Action,                       /* Action taken */

             256L,                          /* File primary allocation */

             FILE_ARCHIVED,                 /* File attributes */

             FILE_OPEN | FILE_CREATE,       /* Open function type */

             OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,

             0L);                           /* No extended attributes */

if (rc != NO_ERROR) {                       /* If open failed */

   printf("DosOpen error: return code = %u\n", rc);

   return 1; }



LockArea.lOffset = 0L;              /* Start locking at beginning of file */

LockArea.lRange =  40L;             /* Use a lock range of 40 bytes       */



rc = DosSetFileLocks(FileHandle,        /* File handle   */

                     &UnlockArea,       /* No unlock area */

                     &LockArea,         /* Lock current record */

                     2000L,             /* Lock time-out value of 2 seconds */

                     0L);               /* Exclusive lock, not atomic */

if (rc != NO_ERROR) {

   printf("DosSetFileLocks error: return code = %u\n", rc);

   return 1;

}



rc = DosWrite(FileHandle, FileData, sizeof(FileData), &Wrote);

if (rc != NO_ERROR) {

   printf("DosWrite error: return code = %u\n", rc);

   return 1;

}

/* Should check if (rc != NO_ERROR) here... */



rc = DosCancelLockRequest(FileHandle, &LockArea);

if (rc != NO_ERROR) {

   printf("DosCancelLockRequest error: return code = %u\n", rc);

   return 1;

}

rc = DosClose(FileHandle);

/* Should check if (rc != NO_ERROR) here... */



return NO_ERROR;

}
