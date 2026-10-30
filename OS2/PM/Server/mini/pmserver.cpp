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
#include <malloc.h>

#include "F_platform.hpp" /* Platform abstraction (PMC_*) */
#include "F_config.hpp"   /* FREEPM_MUTEX_NAME */
#include "FreePM_cmd.hpp" /* F_CMD_* and NGpi* (via gpi_wire.h) */

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

/*! @brief Handler callback type, matching the one in PMTCP. */
typedef void (APIENTRY *PMC_HANDLER)(void *obj, int ncmd, int data, int threadNum);

/*! @brief Prototype for _startServerThreads (ordinal 3). */
typedef APIRET (APIENTRY *PMC_START_SRV)(void *handl);

/*! @brief Prototype for _F_RecvDataFromClient (ordinal 10). */
typedef APIRET (APIENTRY *PMC_RECV_DATA)(void *recvobj, void *data, int *len, int maxlen);

/*! @brief Prototype for _F_SendDataToClient (ordinal 11). */
typedef APIRET (APIENTRY *PMC_SEND_DATA)(void *recvobj, void *data, int len);

/*! @brief Global pointer to the send function, used by the handler. */
static PMC_SEND_DATA g_SendData = 0;

/*! @brief Global pointer to the recv function, used by the handler. */
static PMC_RECV_DATA g_RecvData = 0;

/*! @brief Handle of the server presence mutex. */
static PMC_HMTX g_hmtx = 0;

/*!
 * @brief External GPI dispatcher (implemented in dispatch.cpp).
 *
 * @param [in] hps      Presentation space handle.
 * @param [in] ulFnCode Function code (NGpi*).
 * @param [in] pArgs    Marshaled arguments.
 * @param [in] cbArgs   Size of pArgs in bytes.
 * @return GPI result (GPI_ERROR for all cases in the stub).
 */
extern ULONG DispatchGpi(HPS hps, ULONG ulFnCode, PBYTE pArgs, ULONG cbArgs);

/*! @brief Cleanup function, registered via atexit and signal handlers.
    Closes the mutex and releases any held resources. */
static void CleanupServer(void)
{
    log("PMSERVER: cleaning up\n");

    if (g_hmtx != 0)
    {
        PMC_MutexClose(g_hmtx);
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

/*! @brief Test handler: echoes back the command code.
    @param [in] recvobj   Client connection object.
    @param [in] ncmd      Received command code.
    @param [in] data      Received 32-bit argument.
    @param [in] threadNum Thread index (always 0 for now). */
static void APIENTRY pm_handler(void *recvobj, int ncmd, int data, int threadNum)
{
    int rc, len;
    char buf[256];

    /* --- START OF TRACING --- */
    log("PMSERVER: IN  -> recvobj=%p, ncmd=0x%x, data=0x%x, thread=%d\n", recvobj, ncmd, data, threadNum);
    /* --- END OF TRACING --- */

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

        case F_CMD_GPI_DISPATCH:
        {
            struct { ULONG ulFnCode; ULONG cbArgs; ULONG ulFlags; } call;
            struct { ULONG ulStatus; ULONG ulResult; ULONG cbPayload; ULONG ulLastError; } resp;
            ULONG hps = (ULONG)data;
            PBYTE pArgs = NULL;

            if (!g_RecvData) {
                log("PMSERVER: ERROR - g_RecvData is NULL!\n");
                return;
            }

            rc = g_RecvData(recvobj, &call, &len, sizeof(call));
            if (rc == 0 && len == sizeof(call)) {
                if (call.cbArgs > 0) {
                    pArgs = (PBYTE)malloc(call.cbArgs);
                    if (pArgs) {
                        rc = g_RecvData(recvobj, pArgs, &len, call.cbArgs);
                    }
                }

                resp.ulStatus = 0;
                resp.ulResult = DispatchGpi((HPS)hps, call.ulFnCode, pArgs, call.cbArgs);
                resp.cbPayload = 0;
                resp.ulLastError = 0;

                if (pArgs) free(pArgs);
            } else {
                resp.ulStatus = 1;
                resp.ulResult = (ULONG)GPI_ERROR;
                resp.cbPayload = 0;
                resp.ulLastError = 0;
            }

            if (g_SendData) g_SendData(recvobj, &resp, sizeof(resp));
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
                log("PMSERVER: CALL-> _F_SendDataToClient(recvobj=%p, data=%p, len=%d)\n", recvobj, &reply, (int)sizeof(reply));
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
    PMC_HMODULE hmod = 0;
    APIRET  rc;
    void   *pfn = 0;
    PMC_START_SRV pStartSrv = 0;

    (void)argc;
    (void)argv;

    log("PMSERVER: starting\n");

    /* Create the server presence mutex expected by PMCLIENT. */
    rc = PMC_CreateMutex(FREEPM_MUTEX_NAME, &g_hmtx);
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
#if defined(__WIN32__) || defined(_WIN32)
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)SignalHandler, TRUE);
#endif

    rc = PMC_LoadModule("PMTCP", &hmod);
    if (rc) { log("PMSERVER: cannot load PMTCP.DLL, rc=%u\n", rc); system("pause"); return 1; }

    rc = PMC_QueryOrdinal(hmod, 3, &pfn);
    if (rc) { log("PMSERVER: missing ordinal 3\n"); system("pause"); return 1; }
    pStartSrv = (PMC_START_SRV)pfn;

    rc = PMC_QueryOrdinal(hmod, 10, &pfn);
    if (rc) { log("PMSERVER: missing ordinal 10\n"); system("pause"); return 1; }
    g_RecvData = (PMC_RECV_DATA)pfn;

    rc = PMC_QueryOrdinal(hmod, 11, &pfn);
    if (rc) { log("PMSERVER: missing ordinal 11\n"); system("pause"); return 1; }
    g_SendData = (PMC_SEND_DATA)pfn;

    log("PMSERVER: listening on port 9876\n");

    /* Blocks forever in the accept loop. */
    rc = pStartSrv((void*)pm_handler);
    log("PMSERVER: _startServerThreads returned %u\n", rc);

    system("pause");
    return (rc == 0) ? 0 : 1;
}
