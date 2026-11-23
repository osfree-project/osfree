/*!
 * @file test_pmhab_ping.cpp
 * @brief F_CMD_CLIENT_PING through PMCLIENT.
 *
 * Verifies that PING is fire-and-forget and does not desynchronise
 * the recv stream. The client sends three PINGs (no reply from the
 * server), then a sync request (NWinGetLastError) whose reply must
 * be readable normally.
 *
 * If PING produced any reply, the next Fc_SesSendCommand would read
 * the PING reply as its own and the assertion on the result would
 * fail. A successful round-trip is itself proof that PING is
 * reply-free on the wire.
 *
 * Wire: HAB commands use Fc_SesSendCommand (sync, 4-byte reply).
 *       PING uses Fc_SesPostCommand (fire-and-forget, no reply).
 *
 * Exit code: 0 on success, 1 on failure.
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
    printf("PMHAB_PING: [%s] %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) g_failures++;
}

/*!
 * @brief Send one WIN command via Fc_SesSendCommand.
 * @return 0 on success, -1 on transport error.
 */
static int SendWinCmd(ULONG fnCode, ULONG data, ULONG *out)
{
    APIRET rc = Fc_SesSendCommand(F_CMD_WIN(fnCode), data);
    if (rc == (APIRET)-1) return -1;
    if (out) *out = (ULONG)rc;
    return 0;
}

int main(void)
{
    APIRET rc;
    ULONG  hab    = 0;
    ULONG  result = 0;
    int    i;

    printf("PMHAB_PING: starting\n"); fflush(stdout);

    rc = Fc_SesConnect(NULL);
    if (rc != 0) {
        check(0, "Fc_SesConnect");
        printf("PMHAB_PING: RESULT FAIL\n");
        return 1;
    }
    check(1, "Fc_SesConnect");

    /* Baseline: NWinInitialize works before PING. */
    if (SendWinCmd(NWinInitialize, 1, &hab) != 0 ||
        hab == 0 || hab == (ULONG)-1) {
        check(0, "WinInitialize before PING");
        Fc_SesDisconnect();
        printf("PMHAB_PING: RESULT FAIL\n");
        return 1;
    }
    check(1, "WinInitialize before PING");

    /* Three PINGs in a row. PING produces no reply. The server
       touches the session on each, and the recv stream is untouched. */
    for (i = 0; i < 3; i++) {
        rc = Fc_SesPostCommand(F_CMD_CLIENT_PING, 0);
        if (rc != 0) {
            printf("PMHAB_PING: PING #%d rc=%d\n", i + 1, (int)rc);
            check(0, "PING send");
            SendWinCmd(NWinTerminate, hab, &result);
            Fc_SesDisconnect();
            printf("PMHAB_PING: RESULT FAIL\n");
            return 1;
        }
    }
    check(1, "three PINGs sent");

    /* If PING had produced any reply, the next GetData(4) inside
       Fc_SesSendCommand would consume the PING reply instead of
       NWinGetLastError's, and the assertion below would fail. */
    if (SendWinCmd(NWinGetLastError, hab, &result) != 0 || result != 0) {
        check(0, "WinGetLastError after PINGs");
    } else {
        check(1, "WinGetLastError after PINGs = 0 (stream in sync)");
    }

    SendWinCmd(NWinTerminate, hab, &result);
    Fc_SesDisconnect();

    if (g_failures == 0) {
        printf("PMHAB_PING: RESULT PASS\n");
        return 0;
    }
    printf("PMHAB_PING: RESULT FAIL (%d)\n", g_failures);
    return 1;
}
