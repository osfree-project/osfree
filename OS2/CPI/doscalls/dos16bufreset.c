#include "kal.h"

APIRET16 APIENTRY16 DOS16BUFRESET(USHORT FileHandle)
{
  HFILE hfFileHandle;
  if (FileHandle==0xffff) 
  {
    hfFileHandle=0xffffffff;
  } else {
    hfFileHandle=FileHandle;
  }
  return DosResetBuffer(hfFileHandle);
}

