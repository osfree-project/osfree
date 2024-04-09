#include "kal.h"

USHORT APIENTRY16     DOS16SETPROCCP(USHORT cp, USHORT reserved)
{
  return DosSetProcCp(cp);
}
