#include <kal.h>

USHORT APIENTRY16  DOS16EXITCRITSEC(void)
{
  return DosExitCritSec();
}
