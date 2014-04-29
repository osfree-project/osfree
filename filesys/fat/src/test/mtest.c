#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <share.h>
#include <conio.h>

#include "portable.h"

BYTE bBuffer[32768];

int main(void)
{
INT iFile1, iFile2;
INT iLen;
ULONG ulOffset;


//   iFile1 = sopen("F:\\pics\\test.dat", O_RDWR | O_BINARY | O_CREAT | O_TRUNC, SH_DENYNO, S_IWRITE);
   iFile1 = sopen("F:\\pics\\test.dat", O_RDWR | O_BINARY | O_TRUNC, SH_DENYNO, S_IWRITE);
   if (iFile1 < 0)
      {
      printf("Open failed, rc = %d\n", _doserrno);
//      exit(1);
      }
   printf("Writing 64K data..\n");
   write(iFile1, bBuffer, sizeof bBuffer);
   write(iFile1, bBuffer, sizeof bBuffer);
   getch();

   printf("Opening and truncating file..\n");
   iFile2 = sopen("F:\\pics\\test.dat", O_RDWR | O_BINARY | O_CREAT | O_TRUNC, SH_DENYNO, S_IWRITE);
   if (iFile2 < 0)
      {
      printf("Open failed, rc = %d\n", _doserrno);
      exit(1);
      }
   getch();

   ulOffset = tell(iFile1);
   iLen = read(iFile1, bBuffer, sizeof bBuffer);
   printf("Reading at offset %lu returned %u bytes (Error %d)\n",
      ulOffset, iLen, _doserrno);

   printf("Writing 32k at offset %lu..\n", ulOffset);
   write(iFile1, bBuffer, sizeof bBuffer);
   getch();

   close(iFile2);
   close(iFile1);


   return 0;
}

