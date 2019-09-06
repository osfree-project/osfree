#include "kal.h"

APIRET unimplemented(char *func);

APIRET APIENTRY  DosListIOL(LONG  ulCmdMode,
                            LONG  ulNumentries,
                            PVOID pListIOL)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  log("ulCmdMode=%lx\n", ulCmdMode);
  log("ulNumentries=%lx\n", ulNumentries);
  rc = unimplemented(__FUNCTION__);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}


APIRET  APIENTRY        DosListIO(ULONG ulCmdMode,
                                  ULONG ulNumentries,
                                  PLISTIO pListIO)
{
  PLISTIO  q;
  PLISTIOL pListIOL, p;
  ULONG rc, rc2;
  int i;

  log("%s enter\n", __FUNCTION__);
  rc2 = DosAllocMem((void **)&pListIOL, ulNumentries * sizeof(LISTIOL),
                   PAG_COMMIT | PAG_READ | PAG_WRITE);

  if (rc2)
  {
    rc = rc2;
    goto DOSLISTIO_EXIT;
  }

  for (i = 0, p = pListIOL, q = pListIO; i < ulNumentries; i++, p++, q++)
  {
    p->hFile         = q->hFile;
    p->CmdFlag       = q->CmdFlag;
    p->Offset.ulLo   = q->Offset;
    p->Offset.ulHi   = 0;
    p->pBuffer       = q->pBuffer;
    p->NumBytes      = q->NumBytes;
  }

  rc = DosListIOL(ulCmdMode,
                  ulNumentries,
                  pListIOL);

  for (i = 0, p = pListIOL, q = pListIO; i < ulNumentries; i++, p++, q++)
  {
    q->Offset  = p->Offset.ulLo;
    q->pBuffer = p->pBuffer;
    q->Actual  = p->Actual;
  }

  rc2 = DosFreeMem(pListIOL);

  if (rc2)
  {
    rc = rc2;
  }

DOSLISTIO_EXIT:
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

#if 0

typedef struct _LISTIO_CB {
    HFILE hFile;
    ULONG CmdFlag;
    LONG  Offset;
    PVOID pBuffer;
    ULONG NumBytes;
    ULONG Actual;
    ULONG RetCode;
    ULONG Reserved;
    ULONG Reserved2[3];
    ULONG Reserved3[2];
} LISTIO, *PLISTIO;

typedef struct _LISTIO_CBL {
    HFILE    hFile;
    ULONG    CmdFlag;
    LONGLONG Offset;
    PVOID    pBuffer;
    ULONG    NumBytes;
    ULONG    Actual;
    ULONG    RetCode;
    ULONG    Reserved;
    ULONG    Reserved2[3];
    ULONG    Reserved3[2];
} LISTIOL, *PLISTIOL;

#endif
