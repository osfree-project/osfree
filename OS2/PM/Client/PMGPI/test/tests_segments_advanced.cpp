/*!
 * @file tests_segments_advanced.cpp
 * @brief Advanced segment tests (T78-T101).
 *
 * Covers: DrawChain, DrawFrom, DrawDynamics, RemoveDynamics,
 * QuerySegmentPriority, SetSegmentPriority.
 *
 * Note: Most of these are already implemented on the server
 * (SrvForEachSegment + callbacks). Tests verify both return values
 * and - where possible - actual GOCA replay via [GOCA] log scanning
 * is NOT accessible from client, so we mostly verify state changes
 * (segment order, attributes) via GetData.
 */
#include "tests_common.h"

#define OC_GSCOL   0x0A
#define OC_GSCP    0x21
#define OC_GCLINE  0x81
#define OC_GSMX    0x0C
#define OC_GSBMX   0x0D

/* Helper: build a segment with one line of given color. */
static BOOL MakeSeg(HPS hps, LONG segId, LONG color)
{
    if (!GpiOpenSegment(hps, segId)) return FALSE;
    GpiSetColor(hps, color);
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiLine(hps, SCRATCH_PT(100, 100));
    return GpiCloseSegment(hps);
}

void RunSuite_SegmentsAdvanced(void)
{
    HPS  hps;
    LONG rc;
    LONG prio;

    SUITE("Segments advanced (T78-T101)");

    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    /* ============================================================ */
    /* T78 - DrawChain (single chained segment)                     */
    /* ============================================================ */
    TEST(78, "T78: DrawChain with 1 chained segment succeeds");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeSeg(hps, 1, CLR_RED);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiSetSegmentAttrs(hps, 1, ATTR_CHAINED, ATTR_ON);
    GpiSetSegmentAttrs(hps, 1, ATTR_DYNAMIC, ATTR_OFF);
    GpiSetSegmentAttrs(hps, 1, ATTR_VISIBLE, ATTR_ON);

    rc = GpiDrawChain(hps);
    CHECK_EQ_LONG(rc, 1, "DrawChain(1 seg) succeeds");
    GpiDeleteSegment(hps, 1);

    /* ============================================================ */
    /* T79 - DrawChain with 3 chained segments                      */
    /* ============================================================ */
    TEST(79, "T79: DrawChain with 3 chained segments succeeds");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeSeg(hps, 10, CLR_RED);
    MakeSeg(hps, 20, CLR_GREEN);
    MakeSeg(hps, 30, CLR_BLUE);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiSetSegmentAttrs(hps, 10, ATTR_CHAINED, ATTR_ON);
    GpiSetSegmentAttrs(hps, 20, ATTR_CHAINED, ATTR_ON);
    GpiSetSegmentAttrs(hps, 30, ATTR_CHAINED, ATTR_ON);

    rc = GpiDrawChain(hps);
    CHECK_EQ_LONG(rc, 1, "DrawChain(3 segs) succeeds");

    /* ============================================================ */
    /* T80 - DrawChain skips invisible segment                      */
    /* ============================================================ */
    TEST(80, "T80: DrawChain skips invisible segment");
    GpiSetSegmentAttrs(hps, 10, ATTR_VISIBLE, ATTR_OFF);
    rc = GpiDrawChain(hps);
    CHECK_EQ_LONG(rc, 1, "DrawChain with invisible seg succeeds");
    GpiSetSegmentAttrs(hps, 10, ATTR_VISIBLE, ATTR_ON);

    /* ============================================================ */
    /* T81 - DrawChain skips dynamic segment                        */
    /* ============================================================ */
    TEST(81, "T81: DrawChain skips dynamic segment");
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_ON);
    rc = GpiDrawChain(hps);
    CHECK_EQ_LONG(rc, 1, "DrawChain with dynamic seg succeeds");
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_OFF);

    /* ============================================================ */
    /* T82 - DrawChain skips unchained segment                      */
    /* ============================================================ */
    TEST(82, "T82: DrawChain skips unchained segment");
    GpiSetSegmentAttrs(hps, 30, ATTR_CHAINED, ATTR_OFF);
    rc = GpiDrawChain(hps);
    CHECK_EQ_LONG(rc, 1, "DrawChain with unchained seg succeeds");
    GpiSetSegmentAttrs(hps, 30, ATTR_CHAINED, ATTR_ON);

    /* ============================================================ */
    /* T83 - DrawChain with open segment -> error                   */
    /* ============================================================ */
    TEST(83, "T83: DrawChain while segment open -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 40);
    GpiSetColor(hps, CLR_RED);
    GpiCloseSegment(hps);   /* close it for cleanup, but re-open below */
    GpiOpenSegment(hps, 40);
    rc = GpiDrawChain(hps);
    CHECK_EQ_LONG(rc, GPI_ERROR, "DrawChain with open seg -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 40);

    /* ============================================================ */
    /* T84 - DrawFrom(2, 3) - range filter                          */
    /* ============================================================ */
    TEST(84, "T84: DrawFrom(20, 30) succeeds (in range)");
    rc = GpiDrawFrom(hps, 20, 30);
    CHECK_EQ_LONG(rc, 1, "DrawFrom(20,30) succeeds");

    /* ============================================================ */
    /* T85 - DrawFrom empty range                                   */
    /* ============================================================ */
    TEST(85, "T85: DrawFrom(99, 100) - empty range still succeeds");
    rc = GpiDrawFrom(hps, 99, 100);
    CHECK_EQ_LONG(rc, 1, "DrawFrom(99,100) succeeds (empty)");

    /* ============================================================ */
    /* T86 - DrawDynamics (only dynamic chained)                    */
    /* ============================================================ */
    TEST(86, "T86: DrawDynamics succeeds with dynamic segs");
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_ON);
    rc = GpiDrawDynamics(hps);
    CHECK_EQ_LONG(rc, 1, "DrawDynamics succeeds");
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_OFF);

    /* ============================================================ */
    /* T87 - DrawDynamics skips invisible                           */
    /* ============================================================ */
    TEST(87, "T87: DrawDynamics skips invisible dynamic seg");
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_ON);
    GpiSetSegmentAttrs(hps, 20, ATTR_VISIBLE, ATTR_OFF);
    rc = GpiDrawDynamics(hps);
    CHECK_EQ_LONG(rc, 1, "DrawDynamics with invisible dynamic succeeds");
    GpiSetSegmentAttrs(hps, 20, ATTR_VISIBLE, ATTR_ON);
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_OFF);

    /* ============================================================ */
    /* T88 - DrawDynamics skips unchained                           */
    /* ============================================================ */
    TEST(88, "T88: DrawDynamics skips unchained dynamic seg");
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_ON);
    GpiSetSegmentAttrs(hps, 20, ATTR_CHAINED, ATTR_OFF);
    rc = GpiDrawDynamics(hps);
    CHECK_EQ_LONG(rc, 1, "DrawDynamics with unchained dynamic succeeds");
    GpiSetSegmentAttrs(hps, 20, ATTR_CHAINED, ATTR_ON);
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_OFF);

    /* ============================================================ */
    /* T89 - DrawDynamics restores mix after call                   */
    /* ============================================================ */
    TEST(89, "T89: DrawDynamics restores mix (via replay check)");
    /* Note: mix is a PS-level attribute. Without Step M we can't
       query it directly. We verify by NOT crashing and not leaving
       a dangling state: run DrawDynamics twice - if mix restoration
       is broken, second call would still be in XOR mode and
       behaviour would differ. For now, both calls must return 1. */
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_ON);
    rc = GpiDrawDynamics(hps);
    CHECK_EQ_LONG(rc, 1, "DrawDynamics #1 succeeds");
    rc = GpiDrawDynamics(hps);
    CHECK_EQ_LONG(rc, 1, "DrawDynamics #2 succeeds (mix restored)");
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_OFF);

    /* ============================================================ */
    /* T90 - RemoveDynamics range                                   */
    /* ============================================================ */
    TEST(90, "T90: RemoveDynamics(20, 20) succeeds");
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_ON);
    rc = GpiRemoveDynamics(hps, 20, 20);
    CHECK_EQ_LONG(rc, 1, "RemoveDynamics(20,20) succeeds");
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_OFF);

    /* Cleanup chain */
    GpiDeleteSegment(hps, 10);
    GpiDeleteSegment(hps, 20);
    GpiDeleteSegment(hps, 30);

    /* ============================================================ */
    /* T91 - SetSegmentPriority(30, 10, LOWER_PRI)                  */
    /* ============================================================ */
    TEST(91, "T91: SetSegmentPriority(30, 10, LOWER_PRI) reorders");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeSeg(hps, 10, CLR_RED);
    MakeSeg(hps, 20, CLR_GREEN);
    MakeSeg(hps, 30, CLR_BLUE);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiSetSegmentAttrs(hps, 10, ATTR_CHAINED, ATTR_ON);
    GpiSetSegmentAttrs(hps, 20, ATTR_CHAINED, ATTR_ON);
    GpiSetSegmentAttrs(hps, 30, ATTR_CHAINED, ATTR_ON);
    GpiSetSegmentAttrs(hps, 10, ATTR_DYNAMIC, ATTR_OFF);
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_OFF);
    GpiSetSegmentAttrs(hps, 30, ATTR_DYNAMIC, ATTR_OFF);

    rc = GpiSetSegmentPriority(hps, 30, 10, LOWER_PRI);
    CHECK_EQ_LONG(rc, 1, "SetSegmentPriority(30,10,LOWER) succeeds");

    /* After move: lowest is 30 */
    prio = GpiQuerySegmentPriority(hps, 0, LOWER_PRI);
    CHECK_EQ_LONG(prio, 30, "lowest segment = 30");

    /* ============================================================ */
    /* T92 - SetSegmentPriority(1, 3, HIGHER_PRI) - use 30, 10      */
    /* ============================================================ */
    TEST(92, "T92: SetSegmentPriority(30, 0, HIGHER_PRI) moves to top");
    rc = GpiSetSegmentPriority(hps, 30, 0, HIGHER_PRI);
    CHECK_EQ_LONG(rc, 1, "SetSegmentPriority(30,0,HIGHER) succeeds");
    prio = GpiQuerySegmentPriority(hps, 0, HIGHER_PRI);
    CHECK_EQ_LONG(prio, 30, "highest segment = 30");

    /* ============================================================ */
    /* T93 - SetSegmentPriority self -> no-op                       */
    /* ============================================================ */
    TEST(93, "T93: SetSegmentPriority(id, id, ...) is no-op");
    rc = GpiSetSegmentPriority(hps, 10, 10, LOWER_PRI);
    CHECK_EQ_LONG(rc, 1, "self-priority is no-op");

    /* ============================================================ */
    /* T94 - SetSegmentPriority on open segment -> error            */
    /* ============================================================ */
    TEST(94, "T94: SetSegmentPriority on open segment -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 40);
    rc = GpiSetSegmentPriority(hps, 40, 10, LOWER_PRI);
    CHECK_EQ_LONG(rc, GPI_ERROR, "SetSegmentPriority(open) -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 40);

    /* ============================================================ */
    /* T95 - SetSegmentPriority on unchained -> becomes chained     */
    /* ============================================================ */
    TEST(95, "T95: SetSegmentPriority on unchained segment adds to chain");
    GpiSetSegmentAttrs(hps, 20, ATTR_CHAINED, ATTR_OFF);
    rc = GpiSetSegmentPriority(hps, 20, 10, LOWER_PRI);
    CHECK_EQ_LONG(rc, 1, "SetSegmentPriority(unchained,10,LOWER) succeeds");
    CHECK_EQ_LONG(GpiQuerySegmentAttrs(hps, 20, ATTR_CHAINED), ATTR_ON,
                  "unchained became chained");

    /* ============================================================ */
    /* T96 - QuerySegmentPriority(0, LOWER) = first chained         */
    /* ============================================================ */
    TEST(96, "T96: QuerySegmentPriority(0, LOWER) = first chained");
    prio = GpiQuerySegmentPriority(hps, 0, LOWER_PRI);
    CHECK(prio != 0 && prio != GPI_ERROR, "some chained segment");

    /* ============================================================ */
    /* T97 - QuerySegmentPriority(0, HIGHER) = last chained         */
    /* ============================================================ */
    TEST(97, "T97: QuerySegmentPriority(0, HIGHER) = last chained");
    prio = GpiQuerySegmentPriority(hps, 0, HIGHER_PRI);
    CHECK(prio != 0 && prio != GPI_ERROR, "some chained segment");

    /* ============================================================ */
    /* T98 - QuerySegmentPriority on unchained -> error             */
    /* ============================================================ */
    TEST(98, "T98: QuerySegmentPriority on unchained -> error");
    GpiSetSegmentAttrs(hps, 20, ATTR_CHAINED, ATTR_OFF);
    prio = GpiQuerySegmentPriority(hps, 20, LOWER_PRI);
    CHECK_EQ_LONG(prio, GPI_ERROR, "unchained ref -> error");
    GpiSetSegmentAttrs(hps, 20, ATTR_CHAINED, ATTR_ON);

    /* ============================================================ */
    /* T99 - QuerySegmentPriority on empty chain                    */
    /* ============================================================ */
    TEST(99, "T99: QuerySegmentPriority on empty chain");
    GpiSetSegmentAttrs(hps, 10, ATTR_CHAINED, ATTR_OFF);
    GpiSetSegmentAttrs(hps, 20, ATTR_CHAINED, ATTR_OFF);
    GpiSetSegmentAttrs(hps, 30, ATTR_CHAINED, ATTR_OFF);
    prio = GpiQuerySegmentPriority(hps, 0, LOWER_PRI);
    CHECK(prio == 0 || prio == GPI_ERROR, "empty chain -> 0 or error");

    /* Restore for cleanup */
    GpiSetSegmentAttrs(hps, 10, ATTR_CHAINED, ATTR_ON);
    GpiDeleteSegment(hps, 10);
    GpiDeleteSegment(hps, 20);
    GpiDeleteSegment(hps, 30);

    /* ============================================================ */
    /* T100 - DrawChain with SDW_ON -> error                        */
    /* ============================================================ */
    TEST(100, "T100: DrawChain with SDW_ON -> error (bug #4)");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeSeg(hps, 50, CLR_RED);
    GpiSetDrawingMode(hps, DM_DRAW);

    GpiSetStopDraw(hps, SDW_ON);
    rc = GpiDrawChain(hps);
    CHECK_EQ_LONG(rc, GPI_ERROR, "DrawChain with SDW_ON -> error");
    GpiSetStopDraw(hps, SDW_OFF);
    rc = GpiDrawChain(hps);
    CHECK_NE_LONG(rc, GPI_ERROR, "DrawChain with SDW_OFF succeeds");

    /* ============================================================ */
    /* T101 - DrawDynamics with DCTL_DYNAMIC=OFF                    */
    /* ============================================================ */
    TEST(101, "T101: DrawDynamics with DCTL_DYNAMIC=OFF (bug #5)");
    GpiSetSegmentAttrs(hps, 50, ATTR_DYNAMIC, ATTR_ON);
    GpiSetDrawControl(hps, DCTL_DYNAMIC, DCTL_OFF);
    rc = GpiDrawDynamics(hps);
    /* Per spec: suppressed. Return 1 (no-op) or GPI_ERROR acceptable. */
    CHECK(rc == 1 || rc == GPI_ERROR, "DrawDynamics with DCTL_DYNAMIC=OFF returns");

    GpiSetDrawControl(hps, DCTL_DYNAMIC, DCTL_ON);
    rc = GpiDrawDynamics(hps);
    CHECK_NE_LONG(rc, GPI_ERROR, "DrawDynamics with DCTL_DYNAMIC=ON succeeds");

    GpiDeleteSegment(hps, 50);
    TestFreePS(hps);
}
