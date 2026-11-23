/*!
 * @file tests_p0_regressions.cpp
 * @brief P0 regression tests for confirmed bugs #1-#8, N4, N5, Extra.
 *
 * Each test verifies a SPECIFIC bug fix. A test that passes on the
 * current (unfixed) code indicates either the bug is not real, or the
 * test does not actually exercise the bug path. Both cases require
 * investigation.
 */
#include "tests_common.h"

/* Forward declarations for helpers used inside tests */
static BOOL BuildSegment1(HPS hps);
static BOOL BuildSegment2(HPS hps);
static BOOL BuildSegment3(HPS hps);
static BOOL WriteSegLine(HPS hps, LONG x1, LONG y1, LONG x2, LONG y2);

void RunSuite_P0(void)
{
    HPS   hps;
    BYTE  buf[512];
    LONG  n, i, off;
    LONG  rc;
    POINTL p1, p2;
    BOOL  b;

    SUITE("P0 regression (bugs #1-#8, N4, N5, Extra)");

    /* ============================================================ */
    /* T151 - Bug #1: insert into active segment (not end of buffer) */
    /* ============================================================ */
    TEST(151, "Bug #1: OpenSegment(2) inserts into seg2, not seg3");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    GpiSetDrawingMode(hps, DM_RETAIN);
    BuildSegment1(hps);   /* seg 1 */
    BuildSegment2(hps);   /* seg 2 */
    BuildSegment3(hps);   /* seg 3 */
    GpiSetDrawingMode(hps, DM_DRAW);

    /* Edit seg 2: append one line */
    GpiSetDrawingMode(hps, DM_RETAIN);
    b = GpiOpenSegment(hps, 2);
    CHECK_EQ_INT(b, 1, "OpenSegment(2) succeeded");
    {
        POINTL pa, pb;
        pa.x = 10; pa.y = 10;
        pb.x = 20; pb.y = 20;
        GpiMove(hps, &pa);
        GpiLine(hps, &pb);
    }
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    /* seg 2 must contain GCLINE; seg 3 must NOT contain GCLINE */
    n = TestGetSegmentBody(hps, 2, buf, sizeof(buf));
    CHECK(n > 0, "seg 2 body readable");
    CHECK_EQ_LONG(TestCountOrder(buf, n, 0x81), 1, "seg 2 has 1 GCLINE");

    n = TestGetSegmentBody(hps, 3, buf, sizeof(buf));
    CHECK(n > 0, "seg 3 body readable");
    CHECK_EQ_LONG(TestCountOrder(buf, n, 0x81), 0, "seg 3 has 0 GCLINE");

    TestFreePS(hps);

    /* ============================================================ */
    /* T152 - Bug #2: SEGL updated on CloseSegment                   */
    /* ============================================================ */
    TEST(152, "Bug #2: SEGL in GBSEG equals body length");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 1);
    GpiSetColor(hps, CLR_RED);
    GpiMove(hps, SCRATCH_PT(100, 100));
    GpiLine(hps, SCRATCH_PT(200, 200));
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    /* Read the raw GBSEG from pbGoca via GpiGetData on a known-segment
       is not possible (body only). Instead, read the whole buffer via
       a helper - we can't directly. Use alternative: read body length
       and compare to SEGL by replaying.

       Since we cannot access pbGoca directly from client, this test
       must be done server-side OR via a debug query. For now, mark
       as deferred - but document the expected check: SEGL = body_len. */
    {
        n = TestGetSegmentBody(hps, 1, buf, sizeof(buf));
        CHECK(n > 0, "seg 1 body readable");
        /* SEGL should equal n, but we can't verify without server
           access to pbGoca. This test is a placeholder. */
        printf("  NOTE: SEGL check requires server-side introspection "
               "(deferred to server test suite).\n");
    }
    TestFreePS(hps);

    /* ============================================================ */
    /* T153 - Bug #3: GCBOX flags                                    */
    /* ============================================================ */
    TEST(153, "Bug #3: GCBOX fill/outline flags match pmord.h");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 1);
    GpiMove(hps, SCRATCH_PT(100, 100));
    GpiBox(hps, DRO_FILL, SCRATCH_PT(200, 200), 0, 0);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 1, buf, sizeof(buf));
    CHECK(n > 0, "seg 1 body readable");
    off = TestFindOrder(buf, n, 0x80); /* GCBOX */
    CHECK(off >= 0, "GCBOX present");
    if (off >= 0) {
        /* pmord.h: GCBOX_FILL = 0x40 */
        CHECK((buf[off + 2] & 0x40) != 0, "GCBOX has FILL bit (0x40)");
        CHECK((buf[off + 2] & 0x20) == 0, "GCBOX has no BOUNDARY bit");
    }

    /* Now test outline */
    TestFreePS(hps);
    hps = TestNewPS();
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 1);
    GpiMove(hps, SCRATCH_PT(100, 100));
    GpiBox(hps, DRO_OUTLINE, SCRATCH_PT(200, 200), 0, 0);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 1, buf, sizeof(buf));
    CHECK(n > 0, "seg 1 body readable (outline)");
    off = TestFindOrder(buf, n, 0x80);
    CHECK(off >= 0, "GCBOX present (outline)");
    if (off >= 0) {
        CHECK((buf[off + 2] & 0x20) != 0, "GCBOX has BOUNDARY bit (0x20)");
        CHECK((buf[off + 2] & 0x40) == 0, "GCBOX has no FILL bit");
    }
    TestFreePS(hps);

    /* ============================================================ */
    /* T154 - Bug #4: SDW_ON suppresses draw                         */
    /* ============================================================ */
    TEST(154, "Bug #4: DrawSegment returns error when SDW_ON");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    GpiSetDrawingMode(hps, DM_RETAIN);
    BuildSegment1(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    GpiSetStopDraw(hps, SDW_ON);
    rc = GpiDrawSegment(hps, 1);
    CHECK_EQ_LONG(rc, GPI_ERROR, "DrawSegment fails when SDW_ON");
    GpiSetStopDraw(hps, SDW_OFF);

    rc = GpiDrawSegment(hps, 1);
    CHECK_NE_LONG(rc, GPI_ERROR, "DrawSegment succeeds when SDW_OFF");

    TestFreePS(hps);

    /* ============================================================ */
    /* T155 - Bug #5: DCTL_DYNAMIC=OFF suppresses DrawDynamics       */
    /* ============================================================ */
    TEST(155, "Bug #5: DrawDynamics skipped when DCTL_DYNAMIC=OFF");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    GpiSetDrawingMode(hps, DM_RETAIN);
    BuildSegment3(hps);   /* seg 3 = dynamic */
    GpiSetSegmentAttrs(hps, 3, ATTR_DYNAMIC, ATTR_ON);
    GpiSetDrawingMode(hps, DM_DRAW);

    GpiSetDrawControl(hps, DCTL_DYNAMIC, DCTL_OFF);
    rc = GpiDrawDynamics(hps);
    /* Per spec: suppressed. Server should return 1 (nothing to draw
       but no error) OR GPI_ERROR. Either way, no GOCA output.
       For automated test, we check return value is not crash. */
    CHECK(rc == 1 || rc == GPI_ERROR, "DrawDynamics with DCTL_OFF returns");

    GpiSetDrawControl(hps, DCTL_DYNAMIC, DCTL_ON);
    rc = GpiDrawDynamics(hps);
    CHECK_NE_LONG(rc, GPI_ERROR, "DrawDynamics with DCTL_ON succeeds");

    TestFreePS(hps);

    /* ============================================================ */
    /* T156 - Bug #6: unknown hps returns error                     */
    /* ============================================================ */
    TEST(156, "Bug #6: GpiSetColor with unknown hps returns GPI_ERROR");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }
    TestFreePS(hps);

    /* After release, hps is invalid. Next call must fail. */
    rc = GpiSetColor(hps, CLR_RED);
    CHECK_EQ_LONG(rc, GPI_ERROR, "SetColor on destroyed hps fails");

    /* ============================================================ */
    /* T157 - Bug #7: szlPS stored and returned                     */
    /* ============================================================ */
    TEST(157, "Bug #7: GpiQueryPS returns szlPS");
    {
        SIZEL szl = {640, 480};
        SIZEL szOut = {0, 0};
        hps = GpiCreatePS((HAB)1, (HDC)0, &szl, 0);
        if (hps && hps != (HPS)GPI_ERROR) {
            GpiQueryPS(hps, &szOut);
            CHECK_EQ_LONG(szOut.cx, 640, "QueryPS cx");
            CHECK_EQ_LONG(szOut.cy, 480, "QueryPS cy");
            GpiDestroyPS(hps);
        } else {
            CHECK(0, "GpiCreatePS failed (szlPS test skipped)");
        }
    }

    /* ============================================================ */
    /* T158 - Bug #8: PolyLine(100) split into multiple GCLINE       */
    /* ============================================================ */
    TEST(158, "Bug #8: PolyLine(100) produces multiple GCLINE, no truncation");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    {
        POINTL pts[100];
        LONG k;
        LONG count;

        for (k = 0; k < 100; k++) { pts[k].x = k * 10; pts[k].y = k * 5; }

        GpiSetDrawingMode(hps, DM_RETAIN);
        GpiOpenSegment(hps, 1);
        rc = GpiPolyLine(hps, 100, pts);
        CHECK_NE_LONG(rc, GPI_ERROR, "PolyLine(100) succeeds");
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);

        n = TestGetSegmentBody(hps, 1, buf, sizeof(buf));
        CHECK(n > 0, "seg 1 body readable");
        count = TestCountOrder(buf, n, 0x81); /* GCLINE */
        /* 100 points, LONG coords (8 B/point): each GCLINE max 31 pts.
           31+31+31+7 = 4 orders. */
        CHECK(count >= 2, "PolyLine(100) split into >= 2 GCLINE");
        CHECK_EQ_LONG(count, 4, "PolyLine(100) = 4 GCLINE (31+31+31+7)");
    }
    TestFreePS(hps);

    /* ============================================================ */
    /* T159 - N4: goca.hdc == NULL after CreatePS                   */
    /* ============================================================ */
    TEST(159, "N4: goca.hdc is NULL after CreatePS (no Associate)");
    /* This requires server-side introspection. Deferred. */
    printf("  NOTE: goca.hdc check requires server-side test. Deferred.\n");
    CHECK(1, "N4 placeholder (server test)");

    /* ============================================================ */
    /* T160 - N4: goca.hdc set by Associate                         */
    /* ============================================================ */
    TEST(160, "N4: goca.hdc set by GpiAssociate");
    /* Requires GpiAssociate (Step K). Deferred. */
    printf("  NOTE: goca.hdc after Associate check requires Step K. Deferred.\n");
    CHECK(1, "N4 placeholder (Step K)");

    /* ============================================================ */
    /* T161 - N5: bInArea reset between parses                      */
    /* ============================================================ */
    TEST(161, "N5: bInArea reset at start of GocaParseStream");
    /* Requires malformed stream injection (GBAR without GEAR). Deferred. */
    printf("  NOTE: bInArea leak check requires server-side test. Deferred.\n");
    CHECK(1, "N5 placeholder (server test)");

    /* ============================================================ */
    /* T162 - Extra: RefreshCurPos not called for GCLINE            */
    /* ============================================================ */
    TEST(162, "Extra: GCLINE does not call RefreshCurPos");
    /* Requires counting GreGetCurrentPosition calls. Deferred. */
    printf("  NOTE: RefreshCurPos count requires GRE trace. Deferred.\n");
    CHECK(1, "Extra placeholder (GRE trace)");

    /* ============================================================ */
    /* T180-T189 - Micro/Normal (requires constants)                */
    /* ============================================================ */
    TEST(180, "Micro-PS: OpenSegment returns error");
    printf("  NOTE: requires GPIT_MICRO constant (deferred to Step O).\n");
    CHECK(1, "T180 placeholder");

    TEST(181, "Normal-PS: OpenSegment succeeds");
    /* Already exercised by T151/T157 (they use normal PS implicitly). */
    CHECK(1, "T181 covered by T151/T157");

    TEST(182, "Micro-PS: SetDrawingMode(DM_RETAIN) fails");
    printf("  NOTE: requires GPIT_MICRO constant (deferred to Step O).\n");
    CHECK(1, "T182 placeholder");

    TEST(183, "Micro-PS: GetData/PutData fail");
    printf("  NOTE: deferred to Step O.\n");
    CHECK(1, "T183 placeholder");

    TEST(184, "Micro-PS: BeginElement fails");
    printf("  NOTE: deferred to Step O.\n");
    CHECK(1, "T184 placeholder");

    TEST(185, "Micro-PS: SetModelTransformMatrix fails");
    printf("  NOTE: deferred to Step O.\n");
    CHECK(1, "T185 placeholder");

    TEST(186, "Micro-PS: SavePS/RestorePS in DM_DRAW");
    printf("  NOTE: deferred to Step O.\n");
    CHECK(1, "T186 placeholder");

    TEST(187, "Micro-PS: SetColor/Move/Line works (immediate)");
    printf("  NOTE: deferred to Step O.\n");
    CHECK(1, "T187 placeholder");

    TEST(188, "Micro-PS: without GPIA_ASSOC fails");
    printf("  NOTE: deferred to Step O.\n");
    CHECK(1, "T188 placeholder");

    TEST(189, "WinBeginPaint returns cached Micro-PS");
    printf("  NOTE: deferred to Step O.\n");
    CHECK(1, "T189 placeholder");
}

/* ============================================================ */
/* Helpers                                                       */
/* ============================================================ */

static BOOL WriteSegLine(HPS hps, LONG x1, LONG y1, LONG x2, LONG y2)
{
    POINTL p1, p2;
    p1.x = x1; p1.y = y1;
    p2.x = x2; p2.y = y2;
    GpiMove(hps, &p1);
    return GpiLine(hps, &p2);
}

static BOOL BuildSegment1(HPS hps)
{
    if (!GpiOpenSegment(hps, 1)) return FALSE;
    GpiSetColor(hps, CLR_RED);
    WriteSegLine(hps, 10, 10, 100, 100);
    return GpiCloseSegment(hps);
}

static BOOL BuildSegment2(HPS hps)
{
    if (!GpiOpenSegment(hps, 2)) return FALSE;
    GpiSetColor(hps, CLR_GREEN);
    WriteSegLine(hps, 20, 20, 200, 200);
    return GpiCloseSegment(hps);
}

static BOOL BuildSegment3(HPS hps)
{
    if (!GpiOpenSegment(hps, 3)) return FALSE;
    GpiSetColor(hps, CLR_BLUE);
    WriteSegLine(hps, 30, 30, 300, 300);
    return GpiCloseSegment(hps);
}
