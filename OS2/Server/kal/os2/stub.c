//
//
//
//

#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_MISC
#include <os2.h>

PPVOID entry_Table;
APIRET APIENTRY (*func)();

VOID APIENTRY
KalInit(PPVOID entryTable)
{
  // Save OS/2 Server API functions table locally
  entry_Table = entryTable;
}

APIRET APIENTRY
DosWrite(HFILE hFile, PVOID pBuffer,
         ULONG cbWrite, PULONG pcbActual)
{
  // get function address
  func = entry_Table[2];
  if (!func) return ERROR_INVALID_FUNCTION;
  return func(hFile, pBuffer, cbWrite, pcbActual);
}

APIRET APIENTRY
DosFSCtl(PVOID pData, ULONG cbData,
         PULONG pcbData, PVOID pParms,
         ULONG cbParms, PULONG pcbParms,
         ULONG xfunction, PCSZ pszRoute,
         HFILE hFile, ULONG method)
{
  // get function address
  func = entry_Table[3];
  if (!func) return ERROR_INVALID_FUNCTION;
  return func(pData, cbData, pcbData, pParms,
              cbParms, pcbParms, xfunction,
              pszRoute, hFile, method);
}

VOID APIENTRY
DosExit(ULONG action, ULONG result)
{
  // get function address
  func = entry_Table[4];
  if (!func) return;
  func(action, result);
}

APIRET APIENTRY
DosQuerySysInfo(ULONG iStart, ULONG iLast,
                PVOID pBuf, ULONG cbBuf)
{
  // get function address
  func = entry_Table[5];
  if (!func) return ERROR_INVALID_FUNCTION;
  return func(iStart, iLast, pBuf, cbBuf);
}
