#include <stdio.h>
#include <stdlib.h>

#define INCL_DOS
#include <os2.h>

int main(int iArgc, PSZ pszArgv[])
{
APIRET rc;
FILESTATUS3 fStat;

   if (iArgc < 2)
      exit(1);

   rc = DosQueryPathInfo(pszArgv[1],
       FIL_STANDARD,
       &fStat,
       sizeof fStat);
   if (rc)
      {
      printf("DosQueryPathInfo returned %u\n", rc);
      exit(1);
      }
   printf("Size = %lu\n", fStat.cbFileAlloc);
   return 0;


}

