#include "kal.h"

APIRET16 APIENTRY16 DOS16RMDIR(PSZ DirName, ULONG Reserved)
{
  return DosDeleteDir(DirName);
}
