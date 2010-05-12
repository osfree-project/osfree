#include <os2.h>

APIRET __cdecl KalSetCurrentDir(PCSZ  pszDir);

APIRET APIENTRY  DosSetCurrentDir(PCSZ  pszDir)
{
  return KalSetCurrentDir(pszDir);
}
