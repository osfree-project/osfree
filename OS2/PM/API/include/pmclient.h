/*+---------------------------------+*/
/*| Interface with PM client        |*/
/*+---------------------------------+*/

#include "F_platform.h"

#ifdef __cplusplus
      extern "C" {
#endif

/* Client transport API, implemented in PMCLIENT.DLL.
   Per-thread: each thread has its own connection to the FreePM
   server, opened by Fc_TrnConnect, closed by Fc_TrnDisconnect;
   the handle is internal to PMCLIENT and not exposed.

   Naming and types follow OS/2 CP API and SAA CPI conventions.

   Session   : Fc_SesConnect / Fc_SesDisconnect.
   Async pair: Fc_SesPostCommand / Fc_SesPostData - return immediately.
   Sync pair : Fc_SesSendCommand / Fc_SesSendCommandData - return server reply.
   Get       : Fc_SesGetData - blocking read, equivalent to WinGetMsg. */
APIRET EXPENTRY Fc_SesConnect(char *address);
APIRET EXPENTRY Fc_SesDisconnect(void);
APIRET EXPENTRY Fc_SesPostCommand(ULONG ulCommand, ULONG ulData);
APIRET EXPENTRY Fc_SesPostData(PVOID pvData, LONG cbData);
APIRET EXPENTRY Fc_SesGetData(PVOID pvBuffer, PLONG pcbReceived, LONG cbBuffer);
APIRET EXPENTRY Fc_SesSendCommand(ULONG ulCommand, ULONG ulParameter);
APIRET EXPENTRY Fc_SesSendCommandData(ULONG ulCommand, ULONG ulParameter,
                                      PVOID pvData, LONG cbData);

#ifdef __cplusplus
  }
#endif
