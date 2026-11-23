/*!
 * @file test_pmhab.cpp
 * @brief Session + HAB lifecycle test through PMCLIENT (single-thread).
 *
 * Uses the synchronous pair Fc_SesSendCommand: it posts cmd+data, reads
 * the 4-byte ulResult and returns it. (APIRET)-1 means transport error.
 *
 * Covers:
 *   Fc_SesConnect(NULL)        -> session established (idClient allocated)
 *   NWinInitialize(tid)        -> new HAB (iHab), returned as ulResult
 *   NWinGetLastError(iHab)     -> 0
 *   NWinInitialize(tid) again  -> 0 (already attached to this recvobj)
 *   NWinTerminate(iHab)        -> 1
 *   NWinGetLastError(iHab)     -> PMERR_INVALID_HAB (HAB destroyed)
 *   Fc_SesDisconnect()         -> EXIT + transport close
 *
 * Multi-thread HAB allocation is covered by test_pmhab_mt; RECONNECT by
 * test_pmhab_reconnect.
 *
 * Exit code: 0 if all checks passed, 1 otherwise.
 */
#include <stdio.h>
#include <string.h>

#include "F_platform.h"
#include "pmclient.h"
#include "F_cmd.h"
#include "win_wire.h"

/*! @brief Accumulated failures. */
static int g_failures = 0;

/*! @brief Print PASS/FAIL for one check. */
static void check(int ok, const char *what)
{
    printf("PMHAB_TEST: [%s] %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) g_failures++;
}

/*!
 * @brief Send one WIN command via Fc_SesSendCommand and return the
 *        server-side function result.
 *
 * @param [in]  fnCode  NWin* opcode (win_wire.h).
 * @param [in]  data    Primary argument (tid or iHab).
 * @param [out] out     Server result (iHab / BOOL / PMERR_*).
 * @return 0 on success, -1 on transport error.
 */
static int SendWinCmd(ULONG fnCode, ULONG data, ULONG *out)
{
    APIRET rc;

    rc = Fc_SesSendCommand(F_CMD_WIN(fnCode), data);
    if (rc == (APIRET)-1) {
        printf("  SendWinCmd(0x%x): transport error\n", (unsigned)fnCode);
        return -1;
    }
    if (out) *out = (ULONG)rc;
    return 0;
}

int main(void)
{
    APIRET rc;
    ULONG  iHab   = 0;
    ULONG  result = 0;

    printf("PMHAB_TEST: starting\n"); fflush(stdout);

    /* ---- 1. Fc_SesConnect: establish this thread's session ---- */
    rc = Fc_SesConnect(NULL);
    if (rc != 0) {
        check(0, "Fc_SesConnect");
        printf("PMHAB_TEST: RESULT FAIL\n");
        return 1;
    }
    check(1, "Fc_SesConnect");

    /* ---- 2. NWinInitialize(tid=1) -> new HAB ---- */
    if (SendWinCmd(NWinInitialize, 1, &iHab) != 0) {
        check(0, "WinInitialize transport"); goto done;
    }
    check(iHab != 0, "WinInitialize(tid=1) returns non-zero HAB");
    printf("PMHAB_TEST: iHab=%lu\n", (unsigned long)iHab); fflush(stdout);

    /* ---- 3. NWinGetLastError(iHab) -> 0 ---- */
    if (SendWinCmd(NWinGetLastError, iHab, &result) != 0) {
        check(0, "WinGetLastError transport"); goto done;
    }
    check(result == 0, "WinGetLastError(iHab) = 0");

    /* ---- 4. NWinInitialize(same tid again) -> 0 (already attached) ---- */
    if (SendWinCmd(NWinInitialize, 1, &result) != 0) {
        check(0, "WinInitialize(2nd) transport"); goto done;
    }
    check(result == 0, "WinInitialize(same tid again) = 0");

    /* ---- 5. NWinTerminate(iHab) -> 1 ---- */
    if (SendWinCmd(NWinTerminate, iHab, &result) != 0) {
        check(0, "WinTerminate transport"); goto done;
    }
    check(result == 1, "WinTerminate(iHab) = 1");

    /* ---- 6. NWinGetLastError(iHab after terminate) -> PMERR_INVALID_HAB ---- */
    if (SendWinCmd(NWinGetLastError, iHab, &result) != 0) {
        check(0, "WinGetLastError(post) transport"); goto done;
    }
    check(result == (ULONG)PMERR_INVALID_HAB,
          "WinGetLastError(iHab after terminate) = PMERR_INVALID_HAB");

done:
    Fc_SesDisconnect();

    if (g_failures == 0) {
        printf("PMHAB_TEST: RESULT PASS\n");
        return 0;
    }
    printf("PMHAB_TEST: RESULT FAIL (%d)\n", g_failures);
    return 1;
}
