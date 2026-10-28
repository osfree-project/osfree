/*+---------------------------------+*/
/*| Interface with PM client        |*/
/*+---------------------------------+*/

#include <os2def.h>

#ifdef __cplusplus
      extern "C" {
#endif

/* Client transport API, implemented in PMCLIENT.DLL.
   The connection to the FreePM server is opened at PMCLIENT.DLL load
   and closed at unload; the handle is internal and not exposed.

   Naming and types follow OS/2 CP API and SAA CPI conventions.

   Async pair: PmcPostCommand / PmcSendData - return immediately.
   Sync pair : PmcSendCommand / PmcSendCommandData - return server reply.
   Get       : PmcGetData - blocking read, equivalent to WinGetMsg. */
APIRET APIENTRY PmcPostCommand(ULONG ulCommand, ULONG ulData);
APIRET APIENTRY PmcSendData(PVOID pvData, LONG cbData);
APIRET APIENTRY PmcGetData(PVOID pvBuffer, PLONG pcbReceived, LONG cbBuffer);
APIRET APIENTRY PmcSendCommand(ULONG ulCommand, ULONG ulParameter);
APIRET APIENTRY PmcSendCommandData(ULONG ulCommand, ULONG ulParameter,
                                   PVOID pvData, LONG cbData);

#ifdef __cplusplus
  }
#else

#endif
