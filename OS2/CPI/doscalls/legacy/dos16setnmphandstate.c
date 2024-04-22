#include "kal.h"

USHORT APIENTRY16  DOS16SETNMPHANDSTATE(HPIPE Handle, USHORT PipeHandleState)
{
  return DosSetNPHState(Handle, PipeHandleState);
}
