#include "kal.h"


APIRET16 APIENTRY16 DOS16MKDIR(char * _Seg16 dirname, ULONG reserved)
{
  return DosCreateDir(dirname, NULL);
}
