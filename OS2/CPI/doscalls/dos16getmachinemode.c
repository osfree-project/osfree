#include "kal.h"

APIRET16 APIENTRY16 DOS16GETMACHINEMODE(BYTE * _Seg16 pMachMode)
{
  return *pMachMode = 1; 
}
