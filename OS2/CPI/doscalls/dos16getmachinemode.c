#include "kal.h"

APIRET16 APIENTRY16 DOS16GETMACHINEMODE(PBYTE pMachMode)
{
  return *pMachMode = 1; 
}
