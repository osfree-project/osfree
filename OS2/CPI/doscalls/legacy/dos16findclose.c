#include "kal.h"

USHORT APIENTRY16   DOS16FINDCLOSE(HDIR hdir)
{
  return DosFindClose(hdir);
}
