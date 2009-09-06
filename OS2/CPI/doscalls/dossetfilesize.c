#include <os2.h>

APIRET APIENTRY  DosSetFileSize(HFILE hFile,
                                ULONG cbSize)
{
  LONGLONG cbSizeL;

  cbSizeL.ulLo=cbSize;
  cbSizeL.ulHi=0;

  return DosSetFileSizeL(hFile,
                         cbSizeL);
}
