#include "kal.h"

USHORT APIENTRY16     DOS16PROTECTCLOSE(HFILE hFile,
                                        FHLOCK fhFileHandleLockID)
{
  return DosProtectClose(hFile, fhFileHandleLockID);
}
