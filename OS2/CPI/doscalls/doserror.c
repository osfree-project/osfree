#include <os2.h>

APIRET __cdecl KalError(ULONG errorno);

APIRET APIENTRY DosError(ULONG errorno)
{
  return KalError(errorno);
}
