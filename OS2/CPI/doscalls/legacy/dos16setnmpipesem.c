#include "kal.h"

USHORT APIENTRY16  DOS16SETNMPIPESEM(HPIPE Handle, HSEM SemHandle, USHORT KeyHandle)
{
  return DosSetNPipeSem(Handle, SemHandle, KeyHandle);
}
