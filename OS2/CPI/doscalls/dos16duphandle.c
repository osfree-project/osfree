#include "kal.h"

APIRET16 APIENTRY16 DOS16DUPHANDLE(USHORT OldFileHandle, PUSHORT NewFileHandle)
{
  HFILE Hfile;
  APIRET rc;
  if (*NewFileHandle==0xffff)
  {
    Hfile=0xffffffff;
  } else {
    Hfile=*NewFileHandle;
  }
  rc=DosDupHandle(OldFileHandle, &Hfile);
  *NewFileHandle=Hfile;
  return rc;
}
