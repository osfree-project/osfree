/*!
 * @file test_pmhab_mt.cpp
 * @brief Multi-thread per-thread session test through PMCLIENT.
 *
 * Spawns N worker threads. Each worker does the full session lifecycle
 * through Fc_Ses*:
 *
 *     Fc_SesConnect  ->  NWinInitialize(tid)
 *                    ->  NWinGetLastError(hab) -> 0
 *                    ->  NWinTerminate(hab)    -> 1
 *     Fc_SesDisconnect
 *
 * All threads share the same idClient (process-wide on the client,
 * g_idClient) but each has its own per-thread session slot in
 * PMCLIENT (aConns[tid]) and its own HAB on the server (Fs_Hab,
 * keyed by (idClient, tid)). PMCLIENT sends idClient != 0 on every
 * CONNECT after the first; the server locates the existing process
 * and allocates a new thread.
 *
 * Wire: Fc_SesSendCommand (sync: PostCommand + GetData(4)). The
 * server replies with a bare 4-byte ulResult; no cbArgs is read, no
 * payload, no 16-byte FPM_RESP_HEADER. (APIRET)-1 from
 * Fc_SesSendCommand signals a transport error.
 *
 * Exit code: 0 if all workers pass, 1 otherwise.
 */
#include <stdio.h>
#include <string.h>
#include <process.h>

#include "F_platform.h"
#include "pmclient.h"
#include "F_cmd.h"
#include "win_wire.h"

#define N_WORKERS 3

/*! @brief Per-worker result slot. */
struct WorkerState {
    int  index;
    int  result;    /* -2 = not started, -1 = fail, 0 = success */
    char why[160];
};

/*! @brief Return this thread's OS/2-style thread id. */
static ULONG QueryMyTid(void)
{
    PTIB ptib; PPIB ppib;
    DosGetInfoBlocks(&ptib, &ppib);
    return (ULONG)ptib->tib_ptib2->tib2_ultid;
}

/*!
 * @brief Send one WIN command via Fc_SesSendCommand.
 * @param [in]  fnCode  NWin* opcode.
 * @param [in]  data    Primary argument.
 * @param [out] out     Server ulResult.
 * @return 0 on success, -1 on transport error.
 */
static int SendWinCmd(ULONG fnCode, ULONG data, ULONG *out)
{
    APIRET rc = Fc_SesSendCommand(F_CMD_WIN(fnCode), data);
    if (rc == (APIRET)-1) return -1;
    if (out) *out = (ULONG)rc;
    return 0;
}

/*! @brief Worker: full session lifecycle, on its own client thread. */
static void WorkerProc(void *param)
{
    struct WorkerState *w = (struct WorkerState *)param;
    APIRET              rc;
    ULONG               hab    = 0;
    ULONG               result = 0;
    ULONG               mytid;

    mytid = QueryMyTid();

    DosSleep(w->index * 20);

    rc = Fc_SesConnect(NULL);
    if (rc != 0) {
        w->result = -1;
        snprintf(w->why, sizeof(w->why),
                 "tid=%lu Fc_SesConnect rc=%d",
                 (unsigned long)mytid, (int)rc);
        return;
    }

    if (SendWinCmd(NWinInitialize, mytid, &hab) != 0 ||
        hab == 0 || hab == (ULONG)-1) {
        w->result = -1;
        snprintf(w->why, sizeof(w->why),
                 "tid=%lu NWinInitialize failed (hab=0x%lx)",
                 (unsigned long)mytid, (unsigned long)hab);
        Fc_SesDisconnect();
        return;
    }

    DosSleep(50);

    if (SendWinCmd(NWinGetLastError, hab, &result) != 0 || result != 0) {
        w->result = -1;
        snprintf(w->why, sizeof(w->why),
                 "tid=%lu NWinGetLastError(hab=%lu) result=%lu",
                 (unsigned long)mytid, (unsigned long)hab,
                 (unsigned long)result);
        SendWinCmd(NWinTerminate, hab, &result);
        Fc_SesDisconnect();
        return;
    }

    if (SendWinCmd(NWinTerminate, hab, &result) != 0 || result != 1) {
        w->result = -1;
        snprintf(w->why, sizeof(w->why),
                 "tid=%lu NWinTerminate(hab=%lu) result=%lu",
                 (unsigned long)mytid, (unsigned long)hab,
                 (unsigned long)result);
        Fc_SesDisconnect();
        return;
    }

    Fc_SesDisconnect();
    w->result = 0;
    snprintf(w->why, sizeof(w->why), "tid=%lu ok (hab=%lu)",
             (unsigned long)mytid, (unsigned long)hab);
}

int main(void)
{
    struct WorkerState ws[N_WORKERS];
    ULONG              th[N_WORKERS];
    int                i, failures = 0, notstarted = 0;

    printf("PMHAB_MT_TEST: spawning %d workers\n", N_WORKERS);
    fflush(stdout);

    memset(ws, 0, sizeof(ws));
    for (i = 0; i < N_WORKERS; i++) ws[i].result = -2;

    for (i = 0; i < N_WORKERS; i++) {
        ws[i].index = i;
#if defined(__OS2__) || defined(__OS2V2__)
        th[i] = (ULONG)_beginthread(WorkerProc, NULL, 65536, &ws[i]);
#else
        th[i] = (ULONG)_beginthread(WorkerProc, 65536, &ws[i]);
#endif
    }

    for (i = 0; i < N_WORKERS; i++) {
        if (th[i] == (ULONG)-1 || th[i] == 0) {
            ws[i].result = -1;
            snprintf(ws[i].why, sizeof(ws[i].why), "create failed");
        } else {
            DosWaitThread(&th[i], DCWW_WAIT);
        }
    }

    for (i = 0; i < N_WORKERS; i++) {
        if (ws[i].result == -2) {
            printf("PMHAB_MT_TEST: thread %d [NEVER RAN]\n", i);
            notstarted++;
        } else if (ws[i].result == 0) {
            printf("PMHAB_MT_TEST: thread %d [PASS] %s\n", i, ws[i].why);
        } else {
            printf("PMHAB_MT_TEST: thread %d [FAIL] %s\n", i, ws[i].why);
            failures++;
        }
    }

    if (failures > 0 || notstarted > 0) {
        printf("PMHAB_MT_TEST: RESULT FAIL (%d failed, %d not started of %d)\n",
               failures, notstarted, N_WORKERS);
        return 1;
    }
    printf("PMHAB_MT_TEST: RESULT PASS\n");
    return 0;
}
