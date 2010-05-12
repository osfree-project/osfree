#include <os2.h>

VOID __cdecl KalExit(const ULONG action, const ULONG result);

VOID APIENTRY DosExit(const ULONG action, const ULONG result)
{
  KalExit(action, result);
}
