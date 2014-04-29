#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <fcntl.h>
#include <share.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <time.h>

#define INCL_DOS
#include <os2.h>

#include "portable.h"

char bBlock[0x8000];

int main(int iArgc, PSZ rgArgv[])
{
int iHandle;
long lSize;
clock_t tStart, tEnd, tThis;
time_t timeStart;
long lMax = 10L*1024L*1024L;
long blocks;
long tTotal;


   if (iArgc > 1)
      lMax = atol(rgArgv[1]) * 1024L * 1024L;

   printf("Writing %lu bytes...\n", lMax);

   timeStart = time(NULL);
   iHandle = sopen("TEST.DAT", O_BINARY | O_WRONLY | O_CREAT | O_TRUNC, SH_DENYRW, S_IWRITE);
   if (iHandle < 0)
      {
      printf("Cannot open file!");
      exit(1);
      }
   blocks = 0;
   tTotal = 0;
   for (lSize = 0L; lSize < lMax; lSize += sizeof bBlock)
      {
//      DosSleep(1L);
      tStart = clock();
      if (write(iHandle, bBlock, sizeof bBlock) == -1)
         {
         printf("ERROR: DISK FULL!");
         exit(1);
         }
      tEnd = clock();
      tThis = tEnd - tStart;
      tTotal += (tEnd - tStart);
      blocks++;
#if 0
      printf("This %4lu Averadge: %.2lf\n",
         tThis,
         (double)tTotal / blocks);
#endif
      }
   close(iHandle);

   printf("Writing %lu bytes took %lu tics\n",
      lSize, (long)clock());
   printf("%lu seconds\n", time(NULL) - timeStart);
   return 0;
}

