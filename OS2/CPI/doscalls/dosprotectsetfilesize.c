#include <os2.h>

APIRET APIENTRY  DosProtectSetFileSize(HFILE hFile,
                                       ULONG cbSize,
                                       FHLOCK fhFileHandleLockID)
{
  LONGLONG cbSizeL;

  cbSizeL.ulLo=cbSize;
  cbSizeL.ulHi=0;

  return DosProtectSetFileSizeL(hFile,
                                cbSizeL,
                                fhFileHandleLockID);
}
