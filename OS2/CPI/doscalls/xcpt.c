#include "kal.h"

APIRET APIENTRY      DosSetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
  APIRET rc = NO_ERROR;
  log("%s enter\n", __FUNCTION__);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}


APIRET APIENTRY      DosUnsetExceptionHandler(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
  APIRET rc = NO_ERROR;
  log("%s exit\n", __FUNCTION__);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}


APIRET APIENTRY      DosRaiseException(PEXCEPTIONREPORTRECORD pexcept)
{
  APIRET rc = NO_ERROR;
  log("%s enter\n", __FUNCTION__);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}


APIRET APIENTRY      DosUnwindException(PEXCEPTIONREGISTRATIONRECORD phandler,
                                        PVOID pTargetIP,
                                        PEXCEPTIONREPORTRECORD pERepRec)
{
  APIRET rc = NO_ERROR;
  log("%s enter\n", __FUNCTION__);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY      DosSetSignalExceptionFocus(BOOL32 flag,
                                                PULONG pulTimes)
{
  APIRET rc = NO_ERROR;
  log("%s enter\n", __FUNCTION__);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

APIRET APIENTRY      DosSendSignalException(PID apid,
                                            ULONG exception)
{
  APIRET rc = NO_ERROR;
  log("%s enter\n", __FUNCTION__);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}
