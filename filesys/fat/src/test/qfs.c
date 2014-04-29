#include <stdio.h>
#include <stdlib.h>

#define INCL_DOS
#include <os2.h>



int main(int iArgc, PSZ rgArgv[])
{
USHORT rc;
ULONG ulMemSize;

   rc = DosQSysInfo(17, (PBYTE)&ulMemSize, sizeof ulMemSize);
   printf("DosQSysInfo returned %u\n", rc);
   printf("ulMem = %lu\n", ulMemSize);
   return 0;
}

