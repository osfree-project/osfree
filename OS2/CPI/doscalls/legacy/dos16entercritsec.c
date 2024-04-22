#include <kal.h>

USHORT APIENTRY16  DOS16ENTERCRITSEC(void)
{
  return DosEnterCritSec();
}
