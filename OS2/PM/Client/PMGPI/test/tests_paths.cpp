/*!
 * @file tests_paths.cpp
 * @brief Path tests (T127-T139).
 *
 * Paths are already implemented on the server (NGpiBeginPath/EndPath/
 * CloseFigure/FillPath/OutlinePath/ModifyPath/SetClipPath/StrokePath).
 * Rendering of paths (GreBeginPath etc.) is NOT implemented (Step O),
 * but the GOCA order emission and bookkeeping (lOpenPath, bPathDefined)
 * work. Tests verify the emitted orders and the state guards.
 */
#include "tests_common.h"

#define OC_GBPTH   0xD0
#define OC_GEPTH   0x7F
#define OC_GCFIG   0x7D
#define OC_GFPTH   0xD7
#define OC_GOPTH   0xD9
#define OC_GMPTH   0xD8
#define OC_GSCPTH  0xB4
#define OC_GSCP    0x21
#define OC_GCLINE  0x81

void RunSuite_Paths(void)
{
    HPS  hps;
    LONG rc;
    BYTE buf[256];
    LONG n;

    SUITE("Paths (T127-T139)");

    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    /* ============================================================ */
    /* T127 - BeginPath(1) + EndPath -> GBPTH + GEPTH               */
    /* ============================================================ */
    TEST(127, "T127: BeginPath(1)+EndPath emits GBPTH + GEPTH");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 1);
    rc = GpiBeginPath(hps, 1);
    CHECK_EQ_LONG(rc, 1, "BeginPath(1) succeeds");
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiLine(hps, SCRATCH_PT(100, 100));
    rc = GpiEndPath(hps);
    CHECK_EQ_LONG(rc, 1, "EndPath succeeds");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 1, buf, sizeof(buf));
    CHECK(n > 0, "body non-empty");
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GBPTH), 1, "1 GBPTH");
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GEPTH), 1, "1 GEPTH");
    GpiDeleteSegment(hps, 1);

    /* ============================================================ */
    /* T128 - BeginPath(2) -> error (only id=1 allowed)             */
    /* ============================================================ */
    TEST(128, "T128: BeginPath(2) -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 2);
    rc = GpiBeginPath(hps, 2);
    CHECK_EQ_LONG(rc, GPI_ERROR, "BeginPath(2) -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 2);

    /* ============================================================ */
    /* T129 - BeginPath while already open -> error                 */
    /* ============================================================ */
    TEST(129, "T129: BeginPath while open -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 3);
    GpiBeginPath(hps, 1);
    rc = GpiBeginPath(hps, 1);
    CHECK_EQ_LONG(rc, GPI_ERROR, "duplicate BeginPath -> error");
    GpiEndPath(hps);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 3);

    /* ============================================================ */
    /* T130 - EndPath without Begin -> error                        */
    /* ============================================================ */
    TEST(130, "T130: EndPath without Begin -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 4);
    rc = GpiEndPath(hps);
    CHECK_EQ_LONG(rc, GPI_ERROR, "EndPath without Begin -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 4);

    /* ============================================================ */
    /* T131 - CloseFigure inside path                               */
    /* ============================================================ */
    TEST(131, "T131: CloseFigure inside path emits GCFIG");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 5);
    GpiBeginPath(hps, 1);
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiLine(hps, SCRATCH_PT(100, 100));
    rc = GpiCloseFigure(hps);
    CHECK_EQ_LONG(rc, 1, "CloseFigure succeeds");
    GpiEndPath(hps);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 5, buf, sizeof(buf));
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GCFIG), 1, "1 GCFIG");
    GpiDeleteSegment(hps, 5);

    /* ============================================================ */
    /* T132 - CloseFigure without path -> error                     */
    /* ============================================================ */
    TEST(132, "T132: CloseFigure without path -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 6);
    rc = GpiCloseFigure(hps);
    CHECK_EQ_LONG(rc, GPI_ERROR, "CloseFigure without path -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 6);

    /* ============================================================ */
    /* T133 - FillPath after EndPath                                */
    /* ============================================================ */
    TEST(133, "T133: FillPath emits GFPTH");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 7);
    GpiBeginPath(hps, 1);
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiLine(hps, SCRATCH_PT(100, 100));
    GpiEndPath(hps);
    rc = GpiFillPath(hps, 1, FPATH_ALTERNATE);
    CHECK_EQ_LONG(rc, 1, "FillPath succeeds");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 7, buf, sizeof(buf));
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GFPTH), 1, "1 GFPTH");
    GpiDeleteSegment(hps, 7);

    /* ============================================================ */
    /* T134 - FillPath without EndPath -> error                     */
    /* ============================================================ */
    TEST(134, "T134: FillPath without EndPath -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 8);
    rc = GpiFillPath(hps, 1, FPATH_ALTERNATE);
    CHECK_EQ_LONG(rc, GPI_ERROR, "FillPath without EndPath -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 8);

    /* ============================================================ */
    /* T135 - OutlinePath                                           */
    /* ============================================================ */
    TEST(135, "T135: OutlinePath emits GOPTH");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 9);
    GpiBeginPath(hps, 1);
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiLine(hps, SCRATCH_PT(100, 100));
    GpiEndPath(hps);
    rc = GpiOutlinePath(hps, 1, 0);
    CHECK_EQ_LONG(rc, 1, "OutlinePath succeeds");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 9, buf, sizeof(buf));
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GOPTH), 1, "1 GOPTH");
    GpiDeleteSegment(hps, 9);

    /* ============================================================ */
    /* T136 - ModifyPath (MPATH_STROKE) keeps path defined          */
    /* ============================================================ */
    TEST(136, "T136: ModifyPath(STROKE) emits GMPTH, path stays defined");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 10);
    GpiBeginPath(hps, 1);
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiLine(hps, SCRATCH_PT(100, 100));
    GpiEndPath(hps);
    rc = GpiModifyPath(hps, 1, MPATH_STROKE);
    CHECK_EQ_LONG(rc, 1, "ModifyPath succeeds");
    /* After ModifyPath, path is still defined: FillPath should work */
    rc = GpiFillPath(hps, 1, FPATH_ALTERNATE);
    CHECK_EQ_LONG(rc, 1, "FillPath after ModifyPath succeeds (path still defined)");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 10, buf, sizeof(buf));
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GMPTH), 1, "1 GMPTH");
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GFPTH), 1, "1 GFPTH");
    GpiDeleteSegment(hps, 10);

    /* ============================================================ */
    /* T137 - SetClipPath(1, 0) consumes path                       */
    /* ============================================================ */
    TEST(137, "T137: SetClipPath(1) emits GSCPTH and consumes path");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 11);
    GpiBeginPath(hps, 1);
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiLine(hps, SCRATCH_PT(100, 100));
    GpiEndPath(hps);
    rc = GpiSetClipPath(hps, 1, 0);
    CHECK_EQ_LONG(rc, 1, "SetClipPath(1) succeeds");

    /* After SetClipPath, path is consumed: second SetClipPath(1) fails */
    rc = GpiSetClipPath(hps, 1, 0);
    CHECK_EQ_LONG(rc, GPI_ERROR, "second SetClipPath(1) -> error (consumed)");

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 11, buf, sizeof(buf));
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GSCPTH), 1, "1 GSCPTH");
    GpiDeleteSegment(hps, 11);

    /* ============================================================ */
    /* T138 - SetClipPath(0) reset works regardless of state        */
    /* ============================================================ */
    TEST(138, "T138: SetClipPath(0) reset works regardless");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 12);
    /* No path defined, but reset should still work */
    rc = GpiSetClipPath(hps, 0, 0);
    CHECK_EQ_LONG(rc, 1, "SetClipPath(0) reset succeeds");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 12, buf, sizeof(buf));
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GSCPTH), 1, "1 GSCPTH (reset)");
    GpiDeleteSegment(hps, 12);

    /* ============================================================ */
    /* T139 - StrokePath                                            */
    /* ============================================================ */
    TEST(139, "T139: StrokePath emits GFPTH with MODIFY bit");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 13);
    GpiBeginPath(hps, 1);
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiLine(hps, SCRATCH_PT(100, 100));
    GpiEndPath(hps);
    rc = GpiStrokePath(hps, 1, 0);
    CHECK_EQ_LONG(rc, 1, "StrokePath succeeds");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 13, buf, sizeof(buf));
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GFPTH), 1, "1 GFPTH");
    {
        LONG off = TestFindOrder(buf, n, OC_GFPTH);
        if (off >= 0) {
            /* StrokePath -> FillPath with GFPTH_WINDING|GFPTH_MODIFY */
            BYTE flags = buf[off + 2];
            CHECK((flags & 0x40) != 0, "WINDING bit set (0x40)");
            CHECK((flags & 0x20) != 0, "MODIFY bit set (0x20)");
        }
    }
    GpiDeleteSegment(hps, 13);

    TestFreePS(hps);
}
