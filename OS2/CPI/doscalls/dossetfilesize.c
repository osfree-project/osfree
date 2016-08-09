#include "kal.h"

APIRET APIENTRY  DosSetFileSize(HFILE hFile,
                                ULONG cbSize)
{
  LONGLONG cbSizeL;

  log("%s\n", __FUNCTION__);
  log("hFile=%lx\n", hFile);
  log("cbSize=%lx\n", cbSize);

  cbSizeL.ulLo=cbSize;
  cbSizeL.ulHi=0;

  return DosSetFileSizeL(hFile,
                         cbSizeL);
}
