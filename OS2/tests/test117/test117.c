#define INCL_DOSFILEMGR       /* File Manager values */

#define INCL_DOSERRORS        /* DOS Error values    */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {



HFILE     FileHandle   = NULLHANDLE;  /* File handle */

ULONG     Action       = 0,           /* Action taken by DosOpen */

          Wrote        = 0,           /* Number of bytes written by DosWrite */

          i            = 0;           /* Loop index */

CHAR      FileData[40] = "Forty bytes of demonstration text data\r\n";

APIRET    rc           = NO_ERROR;    /* Return code */

FHLOCK    FHLock       = 0;           /* File handle lock   */

FILELOCK  LockArea     = {0},         /* Area of file to lock */

          UnlockArea   = {0};         /* Area of file to unlock */



rc = DosProtectOpen("flock.dat",                   /* File to open */

             &FileHandle,                   /* File handle */

             &Action,                       /* Action taken */

             4000L,                         /* File primary allocation */

             FILE_ARCHIVED,                 /* File attributes */

             FILE_OPEN | FILE_CREATE,       /* Open function type */

             OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,

             0L,                            /* No extended attributes */

             &FHLock);                      /* File handle lock */

if (rc != NO_ERROR) {                       /* If open failed */

   printf("DosProtectOpen error: return code = %u\n", rc);

   return 1;

}



LockArea.lOffset = 0L;              /* Start locking at beginning of file */

LockArea.lRange =  40L;             /* Use a lock range of 40 bytes       */



         /* Write 8000 bytes to the file, 40 bytes at a time */



for (i=0; i<200; ++i) {



  rc = DosProtectSetFileLocks(FileHandle,        /* File handle   */

                       &UnlockArea,       /* Unlock previous record (if any) */

                       &LockArea,         /* Lock current record */

                       2000L,             /* Lock time-out value of 2 seconds */

                       0L,                /* Exclusive lock, not atomic */

                       FHLock);           /* File handle lock */

  if (rc != NO_ERROR) {

     printf("DosProtectSetFileLocks error: return code = %u\n", rc);

     return 1;

  }

  rc = DosProtectWrite(FileHandle, FileData, sizeof(FileData), &Wrote, FHLock);

  if (rc != NO_ERROR) {

     printf("DosProtectWrite error: return code = %u\n", rc);

     return 1;

  }



   UnlockArea = LockArea;      /* Will unlock this record on next iteration */

   LockArea.lOffset += 40L;    /* Prepare to lock next record               */



  } /* endfor - 8000 bytes written */



rc = DosProtectClose(FileHandle,FHLock);    /* Close file, release any locks */

/* Should check if (rc != NO_ERROR) here .... */



return NO_ERROR;

}
