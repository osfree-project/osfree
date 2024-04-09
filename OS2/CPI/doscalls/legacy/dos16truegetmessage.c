#include "kal.h"

APIRET APIENTRY DosTrueGetMessage(void *msgSeg, PCHAR *pTable, ULONG cTable, PCHAR pBuf,
                                  ULONG cbBuf, ULONG msgnumber,
                                  PSZ pszFile, PULONG pcbMsg);

USHORT APIENTRY16     DOS16TRUEGETMESSAGE(void * _Seg16 pMsgSeg, char * _Seg16 * _Seg16 ivTable,
                                          USHORT ivCount, char * _Seg16 pData, USHORT cbData,
                                          USHORT msgNum, char * _Seg16 pszFileName,
                                          USHORT * _Seg16 msgLen)
{
  USHORT rc;
  ULONG cbMsg=*msgLen;
  
  rc=DosTrueGetMessage(pMsgSeg, ivTable, ivCount, pData, cbData, msgNum, pszFileName, &cbMsg);
  *msgLen=cbMsg;
  return rc;
}
