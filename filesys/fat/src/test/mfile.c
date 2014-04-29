#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <share.h>
#include <conio.h>

#define INCL_DOS
#include "os2.h"
#include "portable.h"

BOOL DeleteFile(int iNr);
BOOL CreateFile(int iNr);

#define MAX_FILES 249

int main(void)
{
INT iNr;
static HFILE rghFile[MAX_FILES];
#ifdef __IBMC__
APIRET rc;
#else
USHORT rc;
#endif

   DosSetMaxFH(255);

   while (!kbhit())
      {
      printf("Opening all files ");
      for (iNr = 0; iNr < MAX_FILES; iNr++)
         {
         rghFile[iNr] = CreateFile(iNr);
         DosSleep(1L);
         printf(".");
         }
      printf("done!\n");
      printf("Closing all files ");
      for (iNr = 0; iNr < MAX_FILES; iNr++)
         {
         if (rghFile[iNr])
            {
            rc = DosClose(rghFile[iNr]);
            if (rc)
               {
               printf("DosClose failed, rc = %d\n", rc);
               getch();
               }
            DosSleep(1L);
            printf(".");
            }
         }
      printf("done!\n");
      }

   return 0;

}

HFILE CreateFile(int iNr)
{
BYTE szFileName[20];
HFILE hFile;
#ifdef __IBMC__
APIRET rc;
ULONG ulAction;
#else
USHORT rc;
USHORT ulAction;
#endif

   sprintf(szFileName, "FILE%4.4u.DAT", iNr);

   rc = DosOpen(szFileName,
      &hFile,
      &ulAction,
      0L, /* new size */
      0,  /* attributes */
      FILE_CREATE | FILE_TRUNCATE,
      OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
      0L);
   if (rc)
      {
      printf("DosOpen on %s failed, rc = %u\n", szFileName, rc);
      getch();
      return 0;
      }

   return hFile;
}

BOOL DeleteFile(int iNr)
{
BYTE szFileName[20];

   sprintf(szFileName, "FILE%4.4u.DAT", iNr);
   if (unlink(szFileName))
      {
      printf("Deleting %s failed.\n", szFileName);
      return FALSE;
      }
   printf("%s deleted.\n", szFileName);
   return TRUE;
}

