/*! @file pmserver.cpp
    @brief Minimal FreePM test server with session management.

    The server itself lives in Fs_Server. main() loads PMTCP, wires
    its send/recv functions into Fs_Server, starts the monitor thread,
    and enters the PMTCP accept loop. Shutdown is handled by the
    monitor thread: when the last client session ends, it calls
    exit(0), which runs atexit() and lets the OS tear down the
    remaining PMTCP threads.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "F_platform.h"
#include "F_config.h"
#include "F_cmd.h"
#include "F_debug.h"
#include "Fs_config.hpp"
#include "Fs_server.hpp"

/*! @brief Server-specific debug configuration.

    Applied once at the start of main() before any output, so that
    server traces go to a dedicated file (pmserver.log) rather than
    competing with clients for the default freepm.log. The path is
    hardcoded for now; a server-side .ini reader will replace it
    later.
*/
static const F_DbgConfig F_DBG_SERVER_CFG = {
    "pmserver.log",
    F_DBG_DEFAULT_ROTATENUMBER,
    F_DBG_DEFAULT_ROTATESIZE,
    F_DBG_DEFAULT_BUFFERED,
    F_DBG_DEFAULT_ANSICOLOR,
    F_DBG_DEFAULT_BEEPONFATAL,
    F_DBG_DEFAULT_USESYSLOG,
    F_DBG_DEFAULT_THRESHOLDLEVEL
};

#if defined(__OS2__) || defined(__OS2V2__)
#define PMCAPIENTRY _System
#elif defined(__WIN32__) || defined(_WIN32)
#define PMCAPIENTRY __stdcall
#endif

typedef void (PMCAPIENTRY *PMC_HANDLER)(void *obj, int ncmd, int data, int threadNum);
typedef APIRET (PMCAPIENTRY *PMC_START_SRV)(void *handl, void *disc_cb);
typedef APIRET (PMCAPIENTRY *PMC_RECV_DATA)(void *recvobj, void *data, int *len, int maxlen);
typedef APIRET (PMCAPIENTRY *PMC_SEND_DATA)(void *recvobj, void *data, int len);

static HMTX      g_hmtx = 0;
static Fs_Server g_server;

static void CleanupServer(void)
{
    F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_INFO, "PMSERVER: cleaning up");
    if (g_hmtx != 0) {
        DosCloseMutexSem(g_hmtx);
        g_hmtx = 0;
    }
}

static void SignalHandler(int sig)
{
    (void)sig;
    CleanupServer();
    exit(1);
}

static void PMCAPIENTRY pm_handler(void *recvobj, int ncmd, int data, int threadNum)
{
    g_server.HandleCommand(recvobj, ncmd, data, threadNum);
}

static void PMCAPIENTRY pm_disconnect(void *recvobj)
{
    g_server.HandleDisconnect(recvobj);
}

int main(int argc, char *argv[])
{
    HMODULE hmod = 0;
    APIRET  rc;
    PFN     pfn = 0;
    PMC_START_SRV pStartSrv = 0;
    char    szFail[256];

    (void)argc;
    (void)argv;

    /* Configure the debug subsystem before any F_DbgPrint call.
       This is the consumer-side init: it applies pmserver.log as
       the log file and the default levels. */
    F_DbgInit(&F_DBG_SERVER_CFG, F_DBG_DEFAULT_LEVELS);

    F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_INFO, "PMSERVER: starting");

    rc = DosCreateMutexSem((PSZ)FREEPM_MUTEX_NAME, &g_hmtx, 0, FALSE);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_ERROR,
                   "PMSERVER: failed to create mutex '%s', rc=%u",
                   FREEPM_MUTEX_NAME, rc);
        system("pause");
        return 1;
    }

    atexit(CleanupServer);
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    rc = DosLoadModule(szFail, sizeof(szFail), (PSZ)"PMTCP", &hmod);
    if (rc) {
        F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_ERROR,
                   "PMSERVER: cannot load PMTCP.DLL, rc=%u", rc);
        return 1;
    }

    /* было: rc = DosQueryProcAddr(hmod, 3, 0, &pfn); */
    rc = DosQueryProcAddr(hmod, 8, 0, &pfn);   /* Fs_TrnRunListener */
    if (rc) {
        F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_ERROR,
                   "PMSERVER: missing ordinal 8");
        return 1;
    }
    pStartSrv = (PMC_START_SRV)pfn;

    {
        PMC_RECV_DATA pfnRecv = NULL;
        PMC_SEND_DATA pfnSend = NULL;

        /* было: rc = DosQueryProcAddr(hmod, 10, 0, &pfn); */
        rc = DosQueryProcAddr(hmod, 11, 0, &pfn);   /* Fs_TrnGetData */
        if (rc) {
            F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_ERROR,
                       "PMSERVER: missing ordinal 11");
            return 1;
        }
        pfnRecv = (PMC_RECV_DATA)pfn;

        /* было: rc = DosQueryProcAddr(hmod, 11, 0, &pfn); */
        rc = DosQueryProcAddr(hmod, 9, 0, &pfn);    /* Fs_TrnPostData */
        if (rc) {
            F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_ERROR,
                       "PMSERVER: missing ordinal 9");
            return 1;
        }
        pfnSend = (PMC_SEND_DATA)pfn;

        g_server.SetWire((Fs_PfnRecv)pfnRecv, (Fs_PfnSend)pfnSend);
    }

    g_server.StartMonitor();

    F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_INFO,
               "PMSERVER: monitor started, listening on port 9876");

    rc = pStartSrv((void*)pm_handler, (void*)pm_disconnect);
    F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_INFO,
               "PMSERVER: _startServerThreads returned %u", rc);

    return (rc == 0) ? 0 : 1;
}
