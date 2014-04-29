#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "portable.h"

int main(void)
{
INT iHandle;
LONG lSize;
PBYTE pBuffer;
BYTE szFile[14];
USHORT usIndex;

   iHandle = open("HENK.JPG", O_BINARY|O_RDONLY);
   if (iHandle < 0)
      {
      printf("Error: Cannot open HENK.JGP");
      exit(1);
      }
   lSize = filelength(iHandle);
   if (lSize > 50000)
      {
      printf("Error: file is too large");
      exit(1);
      }
   pBuffer = malloc((size_t)lSize);
   if (!pBuffer)
      {
      printf("Error: Not enough memory!");
      exit(1);
      }
   read(iHandle, pBuffer, (size_t)lSize);
   close(iHandle);


   for (usIndex = 0; usIndex < 1000; usIndex++)
      {
      sprintf(szFile, "F%4.4u.JPG", usIndex);
      iHandle = open(szFile, O_BINARY|O_CREAT|O_WRONLY, S_IWRITE);
      if (iHandle < 0)
         {
         printf("Error: Cannot create %s\n", szFile);
         exit(1);
         }
      printf("%s\n", szFile);
      write(iHandle, pBuffer, (size_t)lSize);
      close(iHandle);
      }


   return 0;
}

