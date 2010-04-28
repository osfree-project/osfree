#define INCL_DOSFILEMGR   /* File Manager values */

#define INCL_DOSERRORS    /* DOS error values    */

#include <os2.h>

#include <stdio.h>



HFILE  hfFileHandle          = 0L;                /* File Handle             */

ULONG  ulAction              = 0;                 /* Action taken            */

UCHAR  uchNewDirName[10]     = "newdir";          /* New directory name      */

PEAOP2 peaop2NewDirAttribute = NULL;              /* New directory attributes */

UCHAR  uchOldPathName[40]    = "first.dat";       /* Old path name string    */

UCHAR  uchNewPathName[40]    = "newdir\\second.dat"; /* New path name string */

APIRET rc                    = NO_ERROR;          /* Return code             */



int main(VOID) {



   /* Create a file "first.dat" in the current directory */



   rc = DosOpen("first.dat", &hfFileHandle, &ulAction,

                100L, FILE_NORMAL, FILE_CREATE | OPEN_ACTION_OPEN_IF_EXISTS,

                OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE, 0L);

   if (rc != NO_ERROR) {

      printf("DosOpen error: return code = %u\n", rc);

      return 1;        }



   rc = DosClose(hfFileHandle);          /* Close the file (it contains junk) */

   if (rc != NO_ERROR) {

      printf("DosClose error: return code = %u\n", rc);

      return 1;        }



   /* Create a new subdirectory within the current directory */



   rc = DosCreateDir(uchNewDirName, peaop2NewDirAttribute);

   if (rc != NO_ERROR) {

     printf("DosCreateDir error: return code = %u\n", rc);

     return 1;         }



   /* Move the file "first.dat" from the current directory to

      the new directory "newdir", and rename it "second.dat" */



   rc = DosMove(uchOldPathName, uchNewPathName);

   if (rc != NO_ERROR) {

     printf("DosMove error: return code = %u\n", rc);

     return 1;

   } else {

     printf("DosMove:  Move from %s to %s complete.\n",

             uchOldPathName, uchNewPathName); }



 return NO_ERROR;

}
