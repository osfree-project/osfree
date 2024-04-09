#define INCL_DOSNLS

#include "kal.h"

APIRET APIENTRY      DosSetCp(ULONG cp, ULONG reserved);

USHORT APIENTRY16     DOS16SETCP(USHORT codepage, USHORT reserved)
{
  return DosSetCp(codepage, reserved);
}
