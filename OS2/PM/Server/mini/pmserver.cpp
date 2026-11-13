/*! @file pmserver.cpp
    @brief Minimal FreePM test server with session management.

    The server itself lives in Fs_Server. main() reads fpmsrv.ini,
    initialises the debug subsystem with the config-derived levels,
    constructs Fs_Server (so its ctor logs go to pmserver.log, not
    the DllMain default freepm.log), loads PMTCP, wires its send/recv
    functions into Fs_Server, starts the monitor thread, and enters
    the PMTCP accept loop. Shutdown is handled by the monitor thread:
    when the last client session ends, it calls exit(0), which runs
    atexit() and lets the OS tear down the remaining PMTCP threads.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define INCL_BASE
#define INCL_PM
#include "F_platform.h"
#include "F_config.h"
#include "F_cmd.h"
#include "F_debug.h"
#include "Fs_config.hpp"
#include "Fs_server.hpp"

#define DBG_TAG PMSERVER

/*! @brief Server-specific debug configuration.
 *
 * The defaults below are used only if the corresponding keys are not
 * present in fpmsrv.ini. The path is hardcoded for now; a server-side
 * .ini reader will replace it later.
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
/*!
 * @def PMCAPIENTRY
 * @brief Calling convention for PMTCP handler/callback typedefs (OS/2).
 */
#define PMCAPIENTRY _System
#elif defined(__WIN32__) || defined(_WIN32)
/*!
 * @def PMCAPIENTRY
 * @brief Calling convention for PMTCP handler/callback typedefs (Win32).
 */
#define PMCAPIENTRY __stdcall
#endif

/*! @brief PMTCP per-connection command handler. */
typedef void (PMCAPIENTRY *PMC_HANDLER)(void *obj, int ncmd, int data, int threadNum);
/*! @brief PMTCP accept-loop entry point. */
typedef APIRET (PMCAPIENTRY *PMC_START_SRV)(void *handl, void *disc_cb);
/*! @brief PMTCP blocking receive. */
typedef APIRET (PMCAPIENTRY *PMC_RECV_DATA)(void *recvobj, void *data, int *len, int maxlen);
/*! @brief PMTCP blocking send. */
typedef APIRET (PMCAPIENTRY *PMC_SEND_DATA)(void *recvobj, void *data, int len);

/*! @brief Server-side configuration (from fpmsrv.ini).
    Read in main() before any F_DbgPrint call. Fs_Server's ctor copies
    it into m_cfg. Non-static: Fs_server.cpp declares it extern. */
F_ServerConfig FPMs_config;

/*! @brief The server object. Constructed in main() after F_DbgInit
    so its ctor logs land in pmserver.log (not the DllMain default). */
static Fs_Server *g_pServer = NULL;

/*! @brief Transport mutex handle; also the presence marker. */
static HMTX g_hmtx = 0;

/*!
 * @brief atexit/signal cleanup: close the presence mutex.
 */
static void CleanupServer(void)
{
    F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_INFO, "PMSERVER: cleaning up");
    if (g_hmtx != 0) {
        DosCloseMutexSem(g_hmtx);
        g_hmtx = 0;
    }
}

/*!
 * @brief SIGINT/SIGTERM handler: cleanup and exit(1).
 * @param [in] sig  Signal number (unused).
 */
static void SignalHandler(int sig)
{
    (void)sig;
    CleanupServer();
    exit(1);
}

/*!
 * @brief PMTCP command handler: forwards to Fs_Server::HandleCommand.
 * @param [in] recvobj    Transport connection handle.
 * @param [in] ncmd       Wire command code (class + opcode).
 * @param [in] data       Primary argument (class-specific).
 * @param [in] threadNum  PMTCP server thread index.
 */
static void PMCAPIENTRY pm_handler(void *recvobj, int ncmd, int data, int threadNum)
{
    if (g_pServer) g_pServer->HandleCommand(recvobj, ncmd, data, threadNum);
}

/*!
 * @brief PMTCP disconnect callback: forwards to Fs_Server::HandleDisconnect.
 * @param [in] recvobj  Transport connection handle being dropped.
 */
static void PMCAPIENTRY pm_disconnect(void *recvobj)
{
    if (g_pServer) g_pServer->HandleDisconnect(recvobj);
}

/*!
 * @brief Process entry point.
 *
 * Order matters:
 *   1. Read fpmsrv.ini into FPMs_config (before any F_DbgPrint).
 *   2. F_DbgInit with levels from FPMs_config.debugOptions and the
 *      server logfile pmserver.log. Per F_debug spec §11.4, F_DbgInit
 *      does not open the log file: the first write below creates it.
 *      Any file the DllMain default might have opened is closed here;
 *      freepm.log never appears for this process.
 *   3. Create the presence mutex.
 *   4. Load PMTCP and resolve ordinals 8/11/9.
 *   5. Construct Fs_Server (ctor picks up FPMs_config).
 *   6. Start monitor, enter PMTCP accept loop.
 *
 * @param [in] argc  Argument count (unused).
 * @param [in] argv  Argument vector (unused).
 * @return 0 if pStartSrv returned 0, 1 otherwise.
 * @retval 0  Accept loop exited cleanly.
 * @retval 1  Init or accept loop failed.
 */
int main(int argc, char *argv[])
{
    HMODULE hmod = 0;
    APIRET  rc;
    PFN     pfn = 0;
    PMC_START_SRV pStartSrv = 0;
    char    szFail[256];

    (void)argc;
    (void)argv;

    /* 1. Config first, before any F_DbgPrint. */
    FPMs_config.Read("fpmsrv.ini");

    /* 2. Debug init with levels from the config. */
    F_DbgInit(&F_DBG_SERVER_CFG, FPMs_config.debugOptions);

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

        rc = DosQueryProcAddr(hmod, 11, 0, &pfn);   /* Fs_TrnGetData */
        if (rc) {
            F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_ERROR,
                       "PMSERVER: missing ordinal 11");
            return 1;
        }
        pfnRecv = (PMC_RECV_DATA)pfn;

        rc = DosQueryProcAddr(hmod, 9, 0, &pfn);    /* Fs_TrnPostData */
        if (rc) {
            F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_ERROR,
                       "PMSERVER: missing ordinal 9");
            return 1;
        }
        pfnSend = (PMC_SEND_DATA)pfn;

        /* 3. Construct the server now. Ctor picks up FPMs_config and
              its log line lands in pmserver.log. */
        g_pServer = new Fs_Server();
        if (!g_pServer) {
            F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_ERROR,
                       "PMSERVER: new Fs_Server() failed");
            return 1;
        }

        g_pServer->SetWire((Fs_PfnRecv)pfnRecv, (Fs_PfnSend)pfnSend);
    }

    g_pServer->StartMonitor();

    F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_INFO,
               "PMSERVER: monitor started, listening on port 9876");

    rc = pStartSrv((void*)pm_handler, (void*)pm_disconnect);
    F_DbgPrint(F_DBG_SEC_PMSERVER, F_DBG_LVL_INFO,
               "PMSERVER: _startServerThreads returned %u", rc);

    return (rc == 0) ? 0 : 1;
}
