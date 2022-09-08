#include "kal.h"

APIRET16 APIENTRY16 DOSCLOSE(HFILE hf)
{
  return DosClose(hf);
}
