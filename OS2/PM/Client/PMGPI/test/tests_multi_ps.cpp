/*!
 * @file tests_multi_ps.cpp
 * @brief Multi-PS tests (T178-T179).
 *
 * Verifies that two PS are independent: state in one does not leak
 * into the other.
 */
#include "tests_common.h"

void RunSuite_MultiMicro(void)
{
    HPS   hps1;
    HPS   hps2;
    LONG  rc;
    BYTE  buf[256];
    LONG  n;
    SIZEL sz100;

    SUITE("Multi-PS (T178-T179)");

    sz100.cx = 100; sz100.cy = 100;

    /* ============================================================ */
    /* T178 - two PS, SetColor in both, state independent           */
    /* ============================================================ */
    TEST(178, "T178: two PS independent (SetColor in both)");
    hps1 = GpiCreatePS((HAB)1, (HDC)0, &sz100, 0);
    hps2 = GpiCreatePS((HAB)1, (HDC)0, &sz100, 0);
    if (!hps1 || !hps2 || hps1 == (HPS)GPI_ERROR || hps2 == (HPS)GPI_ERROR) {
        CHECK(0, "CreatePS failed");
        return;
    }

    /* Build seg in PS1 with RED, in PS2 with BLUE */
    GpiSetDrawingMode(hps1, DM_RETAIN);
    GpiOpenSegment(hps1, 1);
    GpiSetColor(hps1, CLR_RED);
    GpiMove(hps1, SCRATCH_PT(10, 10));
    GpiLine(hps1, SCRATCH_PT(100, 100));
    GpiCloseSegment(hps1);
    GpiSetDrawingMode(hps1, DM_DRAW);

    GpiSetDrawingMode(hps2, DM_RETAIN);
    GpiOpenSegment(hps2, 1);
    GpiSetColor(hps2, CLR_BLUE);
    GpiMove(hps2, SCRATCH_PT(20, 20));
    GpiLine(hps2, SCRATCH_PT(200, 200));
    GpiCloseSegment(hps2);
    GpiSetDrawingMode(hps2, DM_DRAW);

    /* Read PS1 seg body, first order must be GSCOL with RED */
    n = TestGetSegmentBody(hps1, 1, buf, sizeof(buf));
    CHECK(n > 0, "PS1 seg readable");
    if (n > 0) {
        CHECK_EQ_LONG(buf[0], 0x0A, "PS1 starts with GSCOL");
        CHECK_EQ_LONG(buf[1], CLR_RED, "PS1 color = RED");
    }

    /* Read PS2 seg body, must be BLUE */
    n = TestGetSegmentBody(hps2, 1, buf, sizeof(buf));
    CHECK(n > 0, "PS2 seg readable");
    if (n > 0) {
        CHECK_EQ_LONG(buf[0], 0x0A, "PS2 starts with GSCOL");
        CHECK_EQ_LONG(buf[1], CLR_BLUE, "PS2 color = BLUE");
    }

    /* ============================================================ */
    /* T179 - DestroyPS(hps1) does not affect hps2                  */
    /* ============================================================ */
    TEST(179, "T179: DestroyPS(hps1) does not affect hps2");
    rc = GpiDestroyPS(hps1);
    CHECK_EQ_LONG(rc, 1, "DestroyPS(hps1) succeeds");

    /* hps2 still functional: can read seg, can draw */
    n = TestGetSegmentBody(hps2, 1, buf, sizeof(buf));
    CHECK(n > 0, "PS2 seg still readable after PS1 destroy");
    if (n > 0) {
        CHECK_EQ_LONG(buf[1], CLR_BLUE, "PS2 color still BLUE");
    }

    rc = GpiDrawSegment(hps2, 1);
    CHECK_NE_LONG(rc, GPI_ERROR, "PS2 DrawSegment still works");

    GpiDeleteSegment(hps2, 1);
    GpiDestroyPS(hps2);
}
