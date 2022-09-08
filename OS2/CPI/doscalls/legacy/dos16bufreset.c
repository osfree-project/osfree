#include "kal.h"

APIRET16 APIENTRY16 DOS16BUFRESET(HFILE hf)
{
  HFILE hfFileHandle;
  if (hf==0xffff) 
  {
    hfFileHandle=0xffffffff;
  } else {
    hfFileHandle=hf;
  }
  return DosResetBuffer(hfFileHandle);
}
