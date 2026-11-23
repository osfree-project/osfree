/*!
 * @file tests_common.cpp
 * @brief Test framework implementation.
 */
#include "tests_common.h"

int g_tests_pass = 0;
int g_tests_fail = 0;
int g_tests_current_id = 0;
const char *g_current_suite = "";

POINTL g_scratch_pt;
SIZEL  g_scratch_sz;

/*!
 * @brief Create a fresh PS via WinGetScreenPS.
 * @return HPS or NULL on failure.
 */
HPS TestNewPS(void)
{
    HPS hps = WinGetScreenPS(HWND_DESKTOP);
    if (!hps || hps == (HPS)1) {
        /* Test stub: WinGetScreenPS returns (HPS)1 for now.
           Will be replaced with real F_CMD_GET_HPS flow after
           bug #6 fix + Step K. */
        if (hps == (HPS)1) return hps;
        return NULL;
    }
    return hps;
}

/*!
 * @brief Release the PS created by TestNewPS.
 * @param [in] hps Presentation space handle.
 */
void TestFreePS(HPS hps)
{
    if (hps) WinReleasePS(hps);
}

/*!
 * @brief Get segment body via GpiGetData.
 * @param [in] hps    Presentation space handle.
 * @param [in] lSegId Segment identifier.
 * @param [out] buf   Output buffer.
 * @param [in] cbBuf  Buffer capacity.
 * @return Bytes read, or -1 on error.
 */
LONG TestGetSegmentBody(HPS hps, LONG lSegId, PBYTE buf, LONG cbBuf)
{
    LONG offset = 0;
    LONG n;
    n = GpiGetData(hps, lSegId, &offset, 0, cbBuf, buf);
    return n;
}

/*!
 * @brief Return byte length of GOCA order at p.
 * @param [in] p       Pointer to order bytes.
 * @param [in] cbAvail Bytes available.
 * @return Length in bytes (0 on truncation).
 */
LONG TestOrderLen(PBYTE p, LONG cbAvail)
{
    BYTE oc;
    if (cbAvail < 1) return 0;
    oc = p[0];
    /* BYTE_ORDER: 0x00 (GNOP1), 0xFF (GESD) */
    if (oc == 0x00 || oc == 0xFF) return 1;
    /* VLONG */
    if (oc == 0xFE) {
        if (cbAvail < 4) return 0;
        return 4 + (LONG)p[2] + ((LONG)p[3] << 8);
    }
    /* SHORT_ORDER: (oc ^ 0x80) & 0x88 == 0x88 */
    if (((oc ^ 0x80) & 0x88) == 0x88) return 2;
    /* Long order: 2 + p[1] */
    if (cbAvail < 2) return 0;
    return 2 + (LONG)p[1];
}

/*!
 * @brief Find first occurrence of order code in stream.
 * @param [in] buf   GOCA stream.
 * @param [in] cbBuf Stream length.
 * @param [in] oc    Order code to find.
 * @return Byte offset, or -1 if not found.
 */
LONG TestFindOrder(PBYTE buf, LONG cbBuf, BYTE oc)
{
    LONG i = 0;
    while (i < cbBuf) {
        LONG len = TestOrderLen(buf + i, cbBuf - i);
        if (len <= 0) return -1;
        if (buf[i] == oc) return i;
        i += len;
    }
    return -1;
}

/*!
 * @brief Count occurrences of order code in stream.
 * @param [in] buf   GOCA stream.
 * @param [in] cbBuf Stream length.
 * @param [in] oc    Order code to count.
 * @return Number of occurrences.
 */
LONG TestCountOrder(PBYTE buf, LONG cbBuf, BYTE oc)
{
    LONG count = 0;
    LONG i = 0;
    while (i < cbBuf) {
        LONG len = TestOrderLen(buf + i, cbBuf - i);
        if (len <= 0) break;
        if (buf[i] == oc) count++;
        i += len;
    }
    return count;
}

/*!
 * @brief Print test summary.
 * @return Exit code (0 = all passed).
 */
int TestSummary(void)
{
    printf("\n========================================\n");
    printf("TOTAL: %d passed, %d failed\n", g_tests_pass, g_tests_fail);
    printf("========================================\n");
    return (g_tests_fail == 0) ? 0 : 1;
}
