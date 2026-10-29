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

#include "F_config.hpp"   /* FREEPM_MUTEX_NAME */

#if defined(__OS2__) || defined(__OS2V2__)
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
#else
#include <windows.h>
typedef unsigned long APIRET;
#endif

/*! @brief Handler callback type, matching the one in PMTCP. */
typedef void (APIENTRY *PMC_HANDLER)(void *obj, int ncmd, int data, int threadNum);

/*! @brief Prototype for _startServerThreads (ordinal 3). */
typedef APIRET (APIENTRY *PMC_START_SRV)(void *handl);

/*! @brief Prototype for _F_SendDataToClient (ordinal 11). */
typedef APIRET (APIENTRY *PMC_SEND_DATA)(void *recvobj, void *data, int len);

/*! @brief Global pointer to the send function, used by the handler. */
static PMC_SEND_DATA g_SendData = 0;

/*! @brief Handle of the server presence mutex. */
#if defined(__OS2__) || defined(__OS2V2__)
static HMTX g_hmtx = NULLHANDLE;
#else
static HANDLE g_hMutex = NULL;
#endif

/*! @brief Cleanup function, registered via atexit and signal handlers.
    Closes the mutex and releases any held resources. */
static void CleanupServer(void)
{
    fprintf(stderr, "PMSERVER: cleaning up\n");
    fflush(stderr);

#if defined(__OS2__) || defined(__OS2V2__)
    if (g_hmtx != NULLHANDLE)
    {
        DosCloseMutexSem(g_hmtx);
        g_hmtx = NULLHANDLE;
    }
#else
    if (g_hMutex != NULL)
    {
        CloseHandle(g_hMutex);
        g_hMutex = NULL;
    }
#endif
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
    unsigned long reply = (unsigned long)ncmd;

    /* --- Õ¿◊¿ÀŒ “–¿——»–Œ¬ » --- */
    fprintf(stderr, "PMSERVER: IN  -> recvobj=%p, ncmd=0x%x, data=0x%x, thread=%d\n", recvobj, ncmd, data, threadNum);
    fflush(stderr);
    /* ---  ŒÕ≈÷ “–¿——»–Œ¬ » --- */

    if (g_SendData) {
        /* --- Õ¿◊¿ÀŒ “–¿——»–Œ¬ » --- */
        fprintf(stderr, "PMSERVER: CALL-> _F_SendDataToClient(recvobj=%p, data=%p, len=%d)\n", recvobj, &reply, (int)sizeof(reply));
        fflush(stderr);
        /* ---  ŒÕ≈÷ “–¿——»–Œ¬ » --- */
        
        APIRET rc = g_SendData(recvobj, &reply, sizeof(reply));
        
        /* --- Õ¿◊¿ÀŒ “–¿——»–Œ¬ » --- */
        fprintf(stderr, "PMSERVER: RET <- _F_SendDataToClient returned %u (0x%x)\n", rc, rc);
        fflush(stderr);
        /* ---  ŒÕ≈÷ “–¿——»–Œ¬ » --- */
    } else {
        /* --- Õ¿◊¿ÀŒ “–¿——»–Œ¬ » --- */
        fprintf(stderr, "PMSERVER: ERROR - g_SendData is NULL!\n");
        fflush(stderr);
        /* ---  ŒÕ≈÷ “–¿——»–Œ¬ » --- */
    }
}

/*! @brief Entry point.
    @return 0 on success, 1 on error. */
int main(int argc, char *argv[])
{
    HMODULE hmod = 0;
    APIRET  rc;
    void   *pfn = 0;
    PMC_START_SRV pStartSrv = 0;

    (void)argc;
    (void)argv;

    fprintf(stderr, "PMSERVER: starting\n");
    fflush(stderr);

    /* Create the server presence mutex expected by PMCLIENT. */
#if defined(__OS2__) || defined(__OS2V2__)
    rc = DosCreateMutexSem(FREEPM_MUTEX_NAME, &g_hmtx, 0, FALSE);
    if (rc != NO_ERROR)
    {
        fprintf(stderr, "PMSERVER: failed to create mutex '%s', rc=%u\n", FREEPM_MUTEX_NAME, rc);
        system("pause");
        return 1;
    }
#else
    g_hMutex = CreateMutexA(NULL, FALSE, FREEPM_MUTEX_NAME);
    if (g_hMutex == NULL)
    {
        fprintf(stderr, "PMSERVER: failed to create mutex '%s', err=%u\n", FREEPM_MUTEX_NAME, GetLastError());
        system("pause");
        return 1;
    }
#endif

    /* Register termination handlers. */
    atexit(CleanupServer);
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
#if defined(__WIN32__) || defined(_WIN32)
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)SignalHandler, TRUE);
#endif

#if defined(__OS2__) || defined(__OS2V2__)
    rc = DosLoadModule(NULL, 0, "PMTCP", &hmod);
    if (rc) { fprintf(stderr, "PMSERVER: cannot load PMTCP.DLL, rc=%u\n", rc); system("pause"); return 1; }
    
    rc = DosQueryProcAddr(hmod, 3, 0, (PFN*)&pfn);
    if (rc) { fprintf(stderr, "PMSERVER: missing ordinal 3\n"); system("pause"); return 1; }
    pStartSrv = (PMC_START_SRV)pfn;
    
    rc = DosQueryProcAddr(hmod, 11, 0, (PFN*)&pfn);
    if (rc) { fprintf(stderr, "PMSERVER: missing ordinal 11\n"); system("pause"); return 1; }
    g_SendData = (PMC_SEND_DATA)pfn;
#else
    hmod = LoadLibraryA("PMTCP.DLL");
    if (!hmod) { fprintf(stderr, "PMSERVER: cannot load PMTCP.DLL\n"); system("pause"); return 1; }
    
    pfn = GetProcAddress(hmod, MAKEINTRESOURCEA(3));
    if (!pfn) { fprintf(stderr, "PMSERVER: missing ordinal 3\n"); system("pause"); return 1; }
    pStartSrv = (PMC_START_SRV)pfn;
    
    pfn = GetProcAddress(hmod, MAKEINTRESOURCEA(11));
    if (!pfn) { fprintf(stderr, "PMSERVER: missing ordinal 11\n"); system("pause"); return 1; }
    g_SendData = (PMC_SEND_DATA)pfn;
#endif

    fprintf(stderr, "PMSERVER: listening on port 9876\n");
    fflush(stderr);

    /* Blocks forever in the accept loop. */
    rc = pStartSrv((void*)pm_handler);
    fprintf(stderr, "PMSERVER: _startServerThreads returned %u\n", rc);
    fflush(stderr);
    
    system("pause");
    return (rc == 0) ? 0 : 1;
}
