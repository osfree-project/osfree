/*+---------------------------------+*/
/*| Interface with PM client        |*/
/*+---------------------------------+*/

#if defined(__OS2__) || defined(__OS2V2__)
#include <os2def.h>
#else
#include "F_platform.hpp"
#endif

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
APIRET EXPENTRY PmcPostCommand(ULONG ulCommand, ULONG ulData);
APIRET EXPENTRY PmcSendData(PVOID pvData, LONG cbData);
APIRET EXPENTRY PmcGetData(PVOID pvBuffer, PLONG pcbReceived, LONG cbBuffer);
APIRET EXPENTRY PmcSendCommand(ULONG ulCommand, ULONG ulParameter);
APIRET EXPENTRY PmcSendCommandData(ULONG ulCommand, ULONG ulParameter,
                                   PVOID pvData, LONG cbData);

#ifdef __cplusplus
  }
#else


#endif
