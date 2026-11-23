/*!
 * @file tests_gorder.cpp
 * @brief GORDER class tests (T200+).
 *
 * ALL of these are PLACEHOLDERS. GORDER is defined in F_cmd.h
 * (F_CMD_CLASS_GORDER = 0x0002) but NOT implemented in dispatch.cpp
 * (no F_CMD_CLASS_GORDER case in pm_handler).
 *
 * Per the plan, GORDER is implemented AFTER NGpi is complete (P8).
 * These tests are registered now so that:
 *   1. The test IDs are reserved.
 *   2. The plan for GORDER coverage is documented in code.
 *   3. When P8 lands, these become real tests without renumbering.
 *
 * Expected wire (per F_cmd.h + planned P8 design):
 *   1-byte order: cmd(4) + hps(4) = 8 B, no payload
 *                 oc in cmd[7:0], data (if 2-byte) in cmd[15:8]
 *   long order:   cmd(4) + hps(4) + LEN(1) + data(LEN)
 *   vlong order:  cmd(4) + hps(4) + qual(1) + LEN16(2) + data(LEN16)
 *   batch:        cmd(4) + hps(4) + cbBatch(4) + GOCA-stream
 */
#include "tests_common.h"

void RunSuite_Gorder(void)
{
    SUITE("GORDER (T200-T204)  [P8, not yet implemented]");

    /* ============================================================ */
    /* T200 - 2-byte order packed in cmd (no payload)               */
    /* ============================================================ */
    TEST(200, "T200: 2-byte GORDER (GSCOL) packed in cmd  [P8]");
    /* Planned: F_CMD_ORDER(GSCOL) | (RED << 8) -> wire 8 B.
       Server: oc = cmd & 0xFF, data = (cmd >> 8) & 0xFF,
               dispatch to SrvDeliver(buf[GSCOL, data], 2). */
    printf("  NOTE: requires P8 (GORDER class in dispatch.cpp).\n");
    CHECK(1, "T200 placeholder");

    /* ============================================================ */
    /* T201 - long order with LEN + data                            */
    /* ============================================================ */
    TEST(201, "T201: long GORDER (GSCP) with LEN+data  [P8]");
    /* Planned: cmd = F_CMD_ORDER(GSCP); payload = [LEN, x, y].
       Server: read LEN, read coords, dispatch to SrvDeliver. */
    printf("  NOTE: requires P8.\n");
    CHECK(1, "T201 placeholder");

    /* ============================================================ */
    /* T202 - batch of 1000 GSCOL in one round-trip                 */
    /* ============================================================ */
    TEST(202, "T202: batch 1000xGSCOL in 1 round-trip  [P8]");
    /* Planned: cmd = F_CMD_ORDER_BATCH;
       payload = [cbBatch(4), GOCA-stream].
       Server: read cbBatch, parse stream via GocaParseStream.
       Verify: 1 round-trip, 1000xGSCOL in pbGoca. */
    printf("  NOTE: requires P8 + batch opcode.\n");
    CHECK(1, "T202 placeholder");

    /* ============================================================ */
    /* T203 - PS binding reduces per-call wire                      */
    /* ============================================================ */
    TEST(203, "T203: PS binding (F_CMD_BIND_PS) reduces wire  [P8]");
    /* Planned: F_CMD_BIND_PS(hps) once. Subsequent GORDER calls
       omit hps. Wire for 2-byte GORDER: cmd(4) + len(4) + 0 = 8 B
       -> after self-delimiting: cmd(4) = 4 B. */
    printf("  NOTE: requires P8 + PS bind protocol.\n");
    CHECK(1, "T203 placeholder");

    /* ============================================================ */
    /* T204 - self-delimiting GORDER (no cbOrder)                   */
    /* ============================================================ */
    TEST(204, "T204: self-delimiting GORDER (GocaWirePayloadLen)  [P8]");
    /* Planned: server reads oc from cmd, calls GocaWirePayloadLen(oc),
       reads payload accordingly. No cbOrder. */
    printf("  NOTE: requires P8 + GocaWirePayloadLen.\n");
    CHECK(1, "T204 placeholder");
}
