#define INCL_DOSMISC
#include <os2.h>

APIRET APIENTRY  DosPutMessage(HFILE hfile,
                                  ULONG cbMsg,
                                  PCHAR pBuf)
{
  ULONG ulActual;
  return DosWrite(hfile, pBuf, cbMsg, &ulActual);
}
