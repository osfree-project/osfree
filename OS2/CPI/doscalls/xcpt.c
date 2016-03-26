#define INCL_BASE
#define INCL_DOSEXCEPTIONS
#include <os2.h>

APIRET APIENTRY      DosSetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
  return NO_ERROR;
}


APIRET APIENTRY      DosUnsetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
  return NO_ERROR;
}


APIRET APIENTRY      DosRaiseException(PEXCEPTIONREPORTRECORD pexcept)
{
  return NO_ERROR;
}


APIRET APIENTRY      DosUnwindException(PEXCEPTIONREGISTRATIONRECORD phandler,
                                        PVOID pTargetIP,
                                        PEXCEPTIONREPORTRECORD pERepRec)
{
  return NO_ERROR;
}

APIRET APIENTRY      DosSetSignalExceptionFocus(BOOL32 flag,
                                                PULONG pulTimes)
{
  return NO_ERROR;
}

APIRET APIENTRY      DosSendSignalException(PID apid,
                                            ULONG exception)
{
  return NO_ERROR;
}
