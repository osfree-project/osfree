#include "kal.h"

APIRET16 APIENTRY16 DOS16RMDIR(char * _Seg16 pszDir, ULONG reserved)
{
  return DosDeleteDir(pszDir);
}
