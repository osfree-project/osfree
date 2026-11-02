/*! @file pmserver.cpp
    @brief Minimal FreePM test server.

    Loads the PMTCP backend dynamically, resolves the server ordinals,
    and enters the accept loop. For each command received, it sends
    back a 4-byte reply containing the command code itself.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "F_platform.h"   /* Platform, GPI/Dev, Dos* (OS/2 signatures) */
#include "F_config.hpp"   /* FREEPM_MUTEX_NAME */
#include "F_cmd.h"        /* F_CMD_* and F_CMD_CLASS_* / F_CMD_GET_* */

/*!
 * @brief Unified logging function for the mini server.
 *
 * Defined in dispatch.cpp. Formats the message to stderr and flushes
 * it immediately. Kept as an extern declaration here so that both
 * translation units share the same implementation without pulling
 * in the full unimpl.cpp from PMGPI.
 *
 * @param [in] fmt  printf-style format string.
 * @param [in] ...  Arguments for the format string.
 */
extern void log(const char *fmt, ...);

#ifndef FREEPM_MUTEX_NAME
  #if defined(__OS2__) || defined(__OS2V2__)
    /*!
     * @def FREEPM_MUTEX_NAME
     * @brief Name of the server presence mutex (OS/2).
     *
     * Fallback in case F_config.hpp was not included or did not define it.
     * OS/2 uses the DOSCALLS semaphore namespace.
     */
    #define FREEPM_MUTEX_NAME "\\SEM32\\_FREE_PM"
  #else
    /*!
     * @def FREEPM_MUTEX_NAME
     * @brief Name of the server presence mutex (Win32).
     *
     * Fallback in case F_config.hpp was not included or did not define it.
     * Win32 uses the Global\ namespace to make the mutex visible across
     * terminal services sessions.
     */
    #define FREEPM_MUTEX_NAME "Global\\_FREE_PM"
  #endif
#endif

#if 0
/*! @brief Handler callback type, matching the one in PMTCP. */
typedef void (APIENTRY *PMC_HANDLER)(void *obj, int ncmd, int data, int threadNum);

/*! @brief Prototype for _startServerThreads (ordinal 3). */
typedef APIRET (APIENTRY *PMC_START_SRV)(void *handl);

/*! @brief Prototype for _F_RecvDataFromClient (ordinal 10). */
typedef APIRET (APIENTRY *PMC_RECV_DATA)(void *recvobj, void *data, int *len, int maxlen);

/*! @brief Prototype for _F_SendDataToClient (ordinal 11). */
typedef APIRET (APIENTRY *PMC_SEND_DATA)(void *recvobj, void *data, int len);
#else
#if defined(__OS2__) || defined(__OS2V2__)
#define PMCAPIENTRY _System
#elif defined(__WIN32__) || defined(_WIN32)
#define PMCAPIENTRY __stdcall
#endif

/*! @brief Handler callback type, matching the one in PMTCP. */
typedef void (PMCAPIENTRY *PMC_HANDLER)(void *obj, int ncmd, int data, int threadNum);

/*! @brief Prototype for _startServerThreads (ordinal 3). */
typedef APIRET (PMCAPIENTRY *PMC_START_SRV)(void *handl);

/*! @brief Prototype for _F_RecvDataFromClient (ordinal 10). */
typedef APIRET (PMCAPIENTRY *PMC_RECV_DATA)(void *recvobj, void *data, int *len, int maxlen);

/*! @brief Prototype for _F_SendDataToClient (ordinal 11). */
typedef APIRET (PMCAPIENTRY *PMC_SEND_DATA)(void *recvobj, void *data, int len);
#endif

/*! @brief Global pointer to the send function, used by the handler. */
static PMC_SEND_DATA g_SendData = 0;

/*! @brief Global pointer to the recv function, used by the handler. */
static PMC_RECV_DATA g_RecvData = 0;

/*! @brief Handle of the server presence mutex. */
static HMTX g_hmtx = 0;

/*!
 * @brief External GPI dispatcher (implemented in dispatch.cpp).
 *
 * @param [in] hps      Presentation space handle.
 * @param [in] ulFnCode Function code (NGpi*).
 * @param [in] pArgs    Marshaled arguments.
 * @param [in] cbArgs   Size of pArgs in bytes.
 * @return GPI result.
 */
extern ULONG DispatchGpi(HPS hps, ULONG ulFnCode, PBYTE pArgs, ULONG cbArgs,
                         PULONG pcbPayload);

/*! @brief Cleanup function, registered via atexit and signal handlers.
    Closes the mutex and releases any held resources. */
static void CleanupServer(void)
{
    log("PMSERVER: cleaning up\n");

    if (g_hmtx != 0)
    {
        DosCloseMutexSem(g_hmtx);
        g_hmtx = 0;
    }
}

/*! @brief Signal handler for abnormal termination.
    @param [in] sig Signal number. */
static void SignalHandler(int sig)
{
    (void)sig;
    CleanupServer();
    exit(1);
}

/*!
 * @brief Handle a GPI-class command.
 *
 * Wire format (from the client):
 *   ULONG  cbArgs
 *   BYTE   args[cbArgs]
 *
 * Response (FPM_RESP_HEADER, 16 bytes):
 *   ULONG  ulStatus
 *   ULONG  ulResult
 *   ULONG  cbPayload
 *   ULONG  ulLastError
 *
 * @param [in] recvobj  Client connection object.
 * @param [in] fnCode   NGpi* opcode (low 16 bits of the wire command).
 * @param [in] hps      Presentation space handle (from the 32-bit data arg).
 */
static void HandleGpiCall(void *recvobj, ULONG fnCode, HPS hps)
{
    struct {
        ULONG ulStatus;
        ULONG ulResult;
        ULONG cbPayload;
        ULONG ulLastError;
    } resp;
    ULONG cbArgs    = 0;
    ULONG cbPayload = 0;
    PVOID pv        = NULL;
    PBYTE pArgs     = NULL;
    int   rc, len;

    if (!g_RecvData) {
        log("PMSERVER: ERROR - g_RecvData is NULL!\n");
        return;
    }

    rc = g_RecvData(recvobj, &cbArgs, &len, sizeof(cbArgs));
    if (rc == 0 && len == (int)sizeof(cbArgs)) {
        if (cbArgs > 0) {
            if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
                log("PMSERVER: ERROR - DosAllocMem(%u) failed\n", (unsigned)cbArgs);
                resp.ulStatus    = 1;
                resp.ulResult    = (ULONG)GPI_ERROR;
                resp.cbPayload   = 0;
                resp.ulLastError = 0;
                if (g_SendData) g_SendData(recvobj, &resp, sizeof(resp));
                return;
            }
            pArgs = (PBYTE)pv;
            rc = g_RecvData(recvobj, pArgs, &len, cbArgs);
        }

        resp.ulStatus    = 0;
        resp.ulResult    = DispatchGpi(hps, fnCode, pArgs, cbArgs, &cbPayload);
        resp.cbPayload   = cbPayload;
        resp.ulLastError = 0;

        if (g_SendData) {
            /* Header first. */
            g_SendData(recvobj, &resp, sizeof(resp));

            /* Then the payload, if any (DispatchGpi filled pArgs in place). */
            if (cbPayload > 0 && pArgs != NULL && cbPayload <= cbArgs)
                g_SendData(recvobj, pArgs, (int)cbPayload);
        }

        if (pv) DosFreeMem(pv);
    } else {
        resp.ulStatus    = 1;
        resp.ulResult    = (ULONG)GPI_ERROR;
        resp.cbPayload   = 0;
        resp.ulLastError = 0;

        if (g_SendData) g_SendData(recvobj, &resp, sizeof(resp));
    }
}

/*! @brief Test handler: echoes back the command code.
    @param [in] recvobj   Client connection object.
    @param [in] ncmd      Received command code.
    @param [in] data      Received 32-bit argument.
    @param [in] threadNum Thread index (always 0 for now). */
static void PMCAPIENTRY pm_handler(void *recvobj, int ncmd, int data, int threadNum)
{
    int rc, len;
    char buf[256];

    /* --- START OF TRACING --- */
    log("PMSERVER: IN  -> recvobj=%p, ncmd=0x%x, data=0x%x, thread=%d\n",
        recvobj, ncmd, data, threadNum);
    /* --- END OF TRACING --- */

    if (F_CMD_GET_CLASS((ULONG)ncmd) == F_CMD_CLASS_GPI)
    {
        HandleGpiCall(recvobj, F_CMD_GET_OPCODE((ULONG)ncmd), (HPS)data);
        return;
    }

    switch (ncmd)
    {
        case F_CMD_GET_IHAB:
        {
            int inf[2];
            inf[0] = 1;
            inf[1] = 1;
            if (g_SendData) g_SendData(recvobj, inf, sizeof(inf));
            break;
        }

        case F_CMD_CLIENT_EXIT:
        {
            log("PMSERVER: Client exit (ihab=0x%x)\n", data);
            break;
        }

        case F_CMD_GET_HPS:
        {
            HPS hps = (HPS)1;
            log("PMSERVER: F_CMD_GET_HPS(hwnd=0x%x) -> returning hps=1\n", data);
            if (g_SendData) g_SendData(recvobj, &hps, sizeof(HPS));
            break;
        }

        case F_CMD_RELEASE_HPS:
        {
            int rc1 = 0;
            log("PMSERVER: F_CMD_RELEASE_HPS(hps=0x%x) -> ok\n", data);
            if (g_SendData) g_SendData(recvobj, &rc1, sizeof(int));
            break;
        }

        case F_CMD_DB_PRINT:
        {
            if (g_RecvData) {
                g_RecvData(recvobj, (char *)buf, &len, sizeof(buf));
                log("DEBUG: %s\n", buf);
            }
            break;
        }

        case F_CMD_FATAL_COMMON:
        {
            if (g_RecvData) {
                g_RecvData(recvobj, (char *)buf, &len, sizeof(buf));
                log("FATAL: %s\n", buf);
            }
            break;
        }

        default:
        {
            unsigned long reply = (unsigned long)ncmd;
            if (g_SendData) {
                /* --- START OF TRACING --- */
                log("PMSERVER: CALL-> _F_SendDataToClient(recvobj=%p, data=%p, len=%d)\n",
                    recvobj, &reply, (int)sizeof(reply));
                /* --- END OF TRACING --- */

                APIRET rc2 = g_SendData(recvobj, &reply, sizeof(reply));

                /* --- START OF TRACING --- */
                log("PMSERVER: RET <- _F_SendDataToClient returned %u (0x%x)\n", rc2, rc2);
                /* --- END OF TRACING --- */
            } else {
                /* --- START OF TRACING --- */
                log("PMSERVER: ERROR - g_SendData is NULL!\n");
                /* --- END OF TRACING --- */
            }
            break;
        }
    }
}

/*! @brief Entry point.
    @param [in] argc Argument count (unused).
    @param [in] argv Argument vector (unused).
    @return 0 on success, 1 on error.
    @retval 0 Server terminated normally.
    @retval 1 Initialization or runtime error. */
int main(int argc, char *argv[])
{
    HMODULE hmod = 0;
    APIRET  rc;
    PFN     pfn = 0;
    PMC_START_SRV pStartSrv = 0;
    char    szFail[256];

    (void)argc;
    (void)argv;

    log("PMSERVER: starting\n");

    /* Create the server presence mutex expected by PMCLIENT. */
    rc = DosCreateMutexSem((PSZ)FREEPM_MUTEX_NAME, &g_hmtx, 0, FALSE);
    if (rc != 0)
    {
        log("PMSERVER: failed to create mutex '%s', rc=%u\n", FREEPM_MUTEX_NAME, rc);
        system("pause");
        return 1;
    }

    /* Register termination handlers. */
    atexit(CleanupServer);
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    rc = DosLoadModule(szFail, sizeof(szFail), (PSZ)"PMTCP", &hmod);
    if (rc) { log("PMSERVER: cannot load PMTCP.DLL, rc=%u\n", rc); system("pause"); return 1; }

    rc = DosQueryProcAddr(hmod, 3, 0, &pfn);
    if (rc) { log("PMSERVER: missing ordinal 3\n"); system("pause"); return 1; }
    pStartSrv = (PMC_START_SRV)pfn;

    rc = DosQueryProcAddr(hmod, 10, 0, &pfn);
    if (rc) { log("PMSERVER: missing ordinal 10\n"); system("pause"); return 1; }
    g_RecvData = (PMC_RECV_DATA)pfn;

    rc = DosQueryProcAddr(hmod, 11, 0, &pfn);
    if (rc) { log("PMSERVER: missing ordinal 11\n"); system("pause"); return 1; }
    g_SendData = (PMC_SEND_DATA)pfn;

    log("PMSERVER: listening on port 9876\n");

    /* Blocks forever in the accept loop. */
    rc = pStartSrv((void*)pm_handler);
    log("PMSERVER: _startServerThreads returned %u\n", rc);

    system("pause");
    return (rc == 0) ? 0 : 1;
}