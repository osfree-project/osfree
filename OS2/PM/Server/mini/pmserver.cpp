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
#include "F_config.hpp"
#include "F_cmd.h"
#include "Fs_config.hpp"
#include "Fs_server.hpp"

extern void log(const char *fmt, ...);

#ifndef FREEPM_MUTEX_NAME
  #if defined(__OS2__) || defined(__OS2V2__)
    #define FREEPM_MUTEX_NAME "\\SEM32\\_FREE_PM"
  #else
    #define FREEPM_MUTEX_NAME "Global\\_FREE_PM"
  #endif
#endif

#if defined(__OS2__) || defined(__OS2V2__)
#define PMCAPIENTRY _System
#elif defined(__WIN32__) || defined(_WIN32)
#define PMCAPIENTRY __stdcall
#endif

typedef void (PMCAPIENTRY *PMC_HANDLER)(void *obj, int ncmd, int data, int threadNum);
typedef APIRET (PMCAPIENTRY *PMC_START_SRV)(void *handl);
typedef APIRET (PMCAPIENTRY *PMC_RECV_DATA)(void *recvobj, void *data, int *len, int maxlen);
typedef APIRET (PMCAPIENTRY *PMC_SEND_DATA)(void *recvobj, void *data, int len);

static HMTX      g_hmtx = 0;
static Fs_Server g_server;

static void CleanupServer(void)
{
    log("PMSERVER: cleaning up\n");
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

    rc = DosCreateMutexSem((PSZ)FREEPM_MUTEX_NAME, &g_hmtx, 0, FALSE);
    if (rc != 0) {
        log("PMSERVER: failed to create mutex '%s', rc=%u\n",
            FREEPM_MUTEX_NAME, rc);
        system("pause");
        return 1;
    }

    atexit(CleanupServer);
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    rc = DosLoadModule(szFail, sizeof(szFail), (PSZ)"PMTCP", &hmod);
    if (rc) {
        log("PMSERVER: cannot load PMTCP.DLL, rc=%u\n", rc);
        system("pause");
        return 1;
    }

    rc = DosQueryProcAddr(hmod, 3, 0, &pfn);
    if (rc) { log("PMSERVER: missing ordinal 3\n");  system("pause"); return 1; }
    pStartSrv = (PMC_START_SRV)pfn;

    {
        PMC_RECV_DATA pfnRecv = NULL;
        PMC_SEND_DATA pfnSend = NULL;

        rc = DosQueryProcAddr(hmod, 10, 0, &pfn);
        if (rc) { log("PMSERVER: missing ordinal 10\n"); system("pause"); return 1; }
        pfnRecv = (PMC_RECV_DATA)pfn;

        rc = DosQueryProcAddr(hmod, 11, 0, &pfn);
        if (rc) { log("PMSERVER: missing ordinal 11\n"); system("pause"); return 1; }
        pfnSend = (PMC_SEND_DATA)pfn;

        g_server.SetWire((Fs_PfnRecv)pfnRecv, (Fs_PfnSend)pfnSend);
    }

    g_server.StartMonitor();

    log("PMSERVER: monitor started, listening on port 9876\n");

    rc = pStartSrv((void*)pm_handler);
    log("PMSERVER: _startServerThreads returned %u\n", rc);

    system("pause");
    return (rc == 0) ? 0 : 1;
}
