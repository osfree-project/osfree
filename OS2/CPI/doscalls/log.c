/*  Output to system log
 *
 */

#include <os2.h>

APIRET __cdecl KalLogWrite(PSZ s);

APIRET APIENTRY DosLogWrite(PSZ s)
{
  return KalLogWrite(s);
}
