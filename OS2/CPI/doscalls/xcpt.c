#include "kal.h"

APIRET APIENTRY      DosSetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
  log("%s\n", __FUNCTION__);
  return NO_ERROR;
}


APIRET APIENTRY      DosUnsetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
  log("%s\n", __FUNCTION__);
  return NO_ERROR;
}


APIRET APIENTRY      DosRaiseException(PEXCEPTIONREPORTRECORD pexcept)
{
  log("%s\n", __FUNCTION__);
  return NO_ERROR;
}


APIRET APIENTRY      DosUnwindException(PEXCEPTIONREGISTRATIONRECORD phandler,
                                        PVOID pTargetIP,
                                        PEXCEPTIONREPORTRECORD pERepRec)
{
  log("%s\n", __FUNCTION__);
  return NO_ERROR;
}

APIRET APIENTRY      DosSetSignalExceptionFocus(BOOL32 flag,
                                                PULONG pulTimes)
{
  log("%s\n", __FUNCTION__);
  return NO_ERROR;
}

APIRET APIENTRY      DosSendSignalException(PID apid,
                                            ULONG exception)
{
  log("%s\n", __FUNCTION__);
  return NO_ERROR;
}
