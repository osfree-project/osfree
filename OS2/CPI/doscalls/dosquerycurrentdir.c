#include <os2.h>

APIRET __cdecl KalQueryCurrentDir(ULONG disknum,
                                    PBYTE pBuf,
                                    PULONG pcbBuf);

APIRET APIENTRY  DosQueryCurrentDir(ULONG disknum,
                                    PBYTE pBuf,
                                    PULONG pcbBuf)
{
  return KalQueryCurrentDir(disknum,
                            pBuf,
                            pcbBuf);
}
