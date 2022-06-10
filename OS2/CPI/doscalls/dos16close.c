#include "kal.h"

APIRET16 APIENTRY16 DOS16CLOSE(HFILE hf)
{
  return DosClose(hf);
}
