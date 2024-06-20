#include "kal.h"
#include <string.h>
#include <stdlib.h>

APIRET APIENTRY DosTrueGetMessage(void *msgSeg, PCHAR *pTable, ULONG cTable, PCHAR pBuf,
                                  ULONG cbBuf, ULONG msgnumber,
                                  PSZ pszFile, PULONG pcbMsg);

USHORT APIENTRY16     DOS16TRUEGETMESSAGE(char * _Seg16 * _Seg16 ivTable,
                                          USHORT ivCount, char * _Seg16 pData, USHORT cbData,
                                          USHORT msgNum, char * _Seg16 pszFileName,
                                          USHORT * _Seg16 msgLen, void * _Seg16 pMsgSeg)
{
  USHORT rc;
  ULONG cbMsg=*msgLen;
  int i;
  PSZ ivTable32[10];
  
  for (i=0;i<ivCount;i++)
  {
	  ivTable32[i]=malloc(strlen(ivTable[i])+1);
	  strcpy(ivTable32[i], ivTable[i]);
  }
  
  rc=DosTrueGetMessage(pMsgSeg, ivTable32, ivCount, pData, cbData, msgNum, pszFileName, &cbMsg);
  *msgLen=cbMsg;

  for (i=0;i<ivCount;i++)
  {
	  free(ivTable32[i]);
  }

  return rc;
}
