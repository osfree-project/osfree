/*+---------------------------------+*/
/*| Interface with PM client        |*/
/*+---------------------------------+*/

#if defined(__OS2__) || defined(__OS2V2__)
#include <os2def.h>
#else
#include "gpi_platform.h"
#endif

#ifdef __cplusplus
      extern "C" {
#endif

/*! @file pmclient.h
    @brief Client transport API, implemented in PMCLIENT.DLL.

    The connection to the FreePM server is opened at PMCLIENT.DLL load
    and closed at unload; the handle is internal and not exposed.

    Naming and types follow OS/2 CP API and SAA CPI conventions.

    Async pair: PmcPostCommand / PmcSendData - return immediately.
    Sync pair : PmcSendCommand / PmcSendCommandData - return server reply.
    Get       : PmcGetData - blocking read, equivalent to WinGetMsg.

    Note: declarations use APIENTRY (__stdcall on Win32, _System on OS/2).
    The actual DLL exports use EXPENTRY (__estdcall) to avoid name
    decoration; the client links by ordinal, so APIENTRY is sufficient. */

/*! @brief Post a command with a 32-bit argument to the server (async).
    @param [in] ulCommand Command code.
    @param [in] ulData    32-bit argument.
    @return 0 on success, non-zero on transport error. */
APIRET APIENTRY PmcPostCommand(ULONG ulCommand, ULONG ulData);

/*! @brief Send raw data to the server (async).
    @param [in] pvData Pointer to the payload.
    @param [in] cbData Payload size in bytes.
    @return 0 on success, non-zero on transport error. */
APIRET APIENTRY PmcSendData(PVOID pvData, LONG cbData);

/*! @brief Get raw data from the server (blocking).
    @param [out] pvBuffer    Destination buffer.
    @param [out] pcbReceived Receives the number of bytes read.
    @param [in]  cbBuffer    Size of the destination buffer.
    @return 0 on success, non-zero on transport error. */
APIRET APIENTRY PmcGetData(PVOID pvBuffer, PLONG pcbReceived, LONG cbBuffer);

/*! @brief Send a command with a 32-bit parameter and wait for reply (sync).
    @param [in] ulCommand   Command code.
    @param [in] ulParameter 32-bit parameter.
    @return Server reply code, or a transport error code. */
APIRET APIENTRY PmcSendCommand(ULONG ulCommand, ULONG ulParameter);

/*! @brief Send a command with a payload and wait for reply (sync).
    @param [in] ulCommand   Command code.
    @param [in] ulParameter 32-bit parameter.
    @param [in] pvData      Pointer to the payload.
    @param [in] cbData      Payload size in bytes.
    @return Server reply code, or a transport error code. */
APIRET APIENTRY PmcSendCommandData(ULONG ulCommand, ULONG ulParameter,
                                   PVOID pvData, LONG cbData);

#ifdef __cplusplus
  }
#else


#endif
