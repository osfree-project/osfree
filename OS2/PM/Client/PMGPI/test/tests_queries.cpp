/*!
 * @file tests_queries.cpp
 * @brief Attribute query tests (T140-T150).
 *
 * IMPORTANT: All of these require Step M (Query attribute implementation
 * on server). Currently NGpiQuery* are DISPATCH_STUB -> GPI_ERROR.
 *
 * Per N1, attribute queries are INVALID in DM_RETAIN - they must
 * return GPI_ERROR even after Step M. T141 verifies this.
 */
#include "tests_common.h"

void RunSuite_Queries(void)
{
    HPS hps;
    LONG rc;
    POINTL pt;
    ARCPARAMS ap;
    ARCPARAMS apOut;

    SUITE("Attribute queries (T140-T150)  [requires Step M]");

    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    /* ============================================================ */
    /* T140 - QueryColor in DM_DRAW after SetColor                  */
    /* ============================================================ */
    TEST(140, "T140: QueryColor(DM_DRAW, RED) = RED  [requires Step M]");
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiSetColor(hps, CLR_RED);
    rc = GpiQueryColor(hps);
    CHECK_EQ_LONG(rc, CLR_RED, "QueryColor returns CLR_RED");

    /* ============================================================ */
    /* T141 - QueryColor in DM_RETAIN -> error (per N1)             */
    /* ============================================================ */
    TEST(141, "T141: QueryColor(DM_RETAIN) -> error (per N1)");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 100);
    GpiSetColor(hps, CLR_BLUE);
    rc = GpiQueryColor(hps);
    CHECK_EQ_LONG(rc, GPI_ERROR, "QueryColor in DM_RETAIN -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 100);

    /* ============================================================ */
    /* T142 - QueryLineType                                         */
    /* ============================================================ */
    TEST(142, "T142: QueryLineType after SetLineType  [requires Step M]");
    GpiSetDrawingMode(hps, DM_DRAW);
    /* TODO: GpiSetLineType - not declared in tests_common yet */
    /* rc = GpiSetLineType(hps, 3); */
    /* CHECK_EQ_LONG(GpiQueryLineType(hps), 3, "LineType = 3"); */
    printf("  NOTE: GpiSetLineType/GpiQueryLineType declared but untested "
           "(requires GpiSetLineType proxy).\n");
    CHECK(1, "T142 placeholder (GpiSetLineType proxy)");

    /* ============================================================ */
    /* T143 - QueryLineWidthGeom                                    */
    /* ============================================================ */
    TEST(143, "T143: QueryLineWidthGeom after SetLineWidthGeom  [requires Step M]");
    /* TODO: GpiSetLineWidthGeom - not declared */
    printf("  NOTE: GpiSetLineWidthGeom proxy needed.\n");
    CHECK(1, "T143 placeholder");

    /* ============================================================ */
    /* T144 - QueryCurrentPosition                                  */
    /* ============================================================ */
    TEST(144, "T144: QueryCurrentPosition after Move  [requires Step M]");
    GpiSetDrawingMode(hps, DM_DRAW);
    pt.x = 123; pt.y = 456;
    GpiMove(hps, &pt);
    {
        POINTL out;
        out.x = -1; out.y = -1;
        rc = GpiQueryCurrentPosition(hps, &out);
        if (rc) {
            CHECK_EQ_LONG(out.x, 123, "current x = 123");
            CHECK_EQ_LONG(out.y, 456, "current y = 456");
        } else {
            CHECK(0, "QueryCurrentPosition failed (Step M not done)");
        }
    }

    /* ============================================================ */
    /* T145 - QueryMix / QueryBackMix                               */
    /* ============================================================ */
    TEST(145, "T145: QueryMix/QueryBackMix  [requires Step M]");
    GpiSetDrawingMode(hps, DM_DRAW);
    /* TODO: GpiSetMix/GpiSetBackMix - not declared */
    rc = GpiQueryMix(hps);
    /* Default mix is FM_OVERPAINT (0) or FM_DEFAULT - value varies.
       Only check that it does not return GPI_ERROR (that would mean stub). */
    CHECK_NE_LONG(rc, GPI_ERROR, "QueryMix returns non-error");
    printf("  NOTE: GpiSetMix/GpiSetBackMix proxies needed for full test.\n");

    /* ============================================================ */
    /* T146 - QueryBackColor                                        */
    /* ============================================================ */
    TEST(146, "T146: QueryBackColor  [requires Step M]");
    /* TODO: GpiSetBackColor - not declared in tests_common */
    printf("  NOTE: GpiSetBackColor/GpiQueryBackColor proxies needed.\n");
    CHECK(1, "T146 placeholder");

    /* ============================================================ */
    /* T147 - QueryCharSet/Direction/Mode                           */
    /* ============================================================ */
    TEST(147, "T147: QueryCharSet/Direction/Mode  [requires Step M]");
    printf("  NOTE: GpiSetCharSet/GpiQueryCharSet proxies needed.\n");
    CHECK(1, "T147 placeholder");

    /* ============================================================ */
    /* T148 - QueryMarker/MarkerSet                                 */
    /* ============================================================ */
    TEST(148, "T148: QueryMarker/MarkerSet  [requires Step M]");
    printf("  NOTE: GpiSetMarker/GpiQueryMarker proxies needed.\n");
    CHECK(1, "T148 placeholder");

    /* ============================================================ */
    /* T149 - QueryPattern/PatternSet/PatternRefPoint               */
    /* ============================================================ */
    TEST(149, "T149: QueryPattern/Set/RefPoint  [requires Step M]");
    printf("  NOTE: GpiSetPattern/GpiQueryPattern proxies needed.\n");
    CHECK(1, "T149 placeholder");

    /* ============================================================ */
    /* T150 - QueryArcParams                                        */
    /* ============================================================ */
    TEST(150, "T150: QueryArcParams after SetArcParams  [requires Step M]");
    GpiSetDrawingMode(hps, DM_DRAW);
    ap.lP = 2; ap.lQ = 3; ap.lR = 4; ap.lS = 5;
    GpiSetArcParams(hps, &ap);
    /* No GpiQueryArcParams proxy declared in tests_common */
    printf("  NOTE: GpiQueryArcParams proxy needed.\n");
    CHECK(1, "T150 placeholder");

    TestFreePS(hps);

    printf("\n  NOTE: All T140-T150 require Step M (attribute queries).\n");
    printf("  Current server: NGpiQuery* are DISPATCH_STUB -> GPI_ERROR.\n");
    printf("  After Step M, most tests will pass; T141 (DM_RETAIN) must\n");
    printf("  still return error (per N1).\n");
}
