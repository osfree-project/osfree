/*!
 * @file tests_legacy_port.cpp
 * @brief Tests ported from testgpi.cpp that were missing from the
 *        structured suite (gaps G1-G7).
 *
 * Each test exercises a code path that testgpi.cpp covered but the
 * structured suite did not:
 *   G1 (T205) - immediate DM_DRAW without segment
 *   G2 (T206) - SEGEM_INSERT vs SEGEM_REPLACE semantics
 *   G3 (T207) - CloseSegment with open bracket -> implicit GEEL last
 *   G4 (T208) - CloseSegment with open path -> no GEPTH
 *   G5 (T209) - SetEditMode inside bracket -> error
 *   G6 (T210) - SrvDeliver pointer advance regression (I9-C)
 *   G7 (T211) - QueryElementType for GLBL single order
 */
#include "tests_common.h"

#define OC_GSCOL   0x0A
#define OC_GLBL    0xD3
#define OC_GBEL    0xD2
#define OC_GEEL    0x49
#define OC_GEPTH   0x7F
#define OC_GBPTH   0xD0
#define OC_GSCP    0x21
#define OC_GCLINE  0x81

void RunSuite_LegacyPort(void)
{
    HPS  hps;
    LONG rc;
    BYTE buf[256];
    LONG n;

    SUITE("Legacy port from testgpi.cpp (G1-G7)");

    /* ============================================================ */
    /* T205 / G1 - immediate DM_DRAW without segment                */
    /* ============================================================ */
    TEST(205, "G1: DM_DRAW SetColor/Move/Line/Box without segment");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    GpiSetDrawingMode(hps, DM_DRAW);
    GpiSetColor(hps, CLR_RED);
    GpiMove(hps, SCRATCH_PT(100, 100));
    GpiLine(hps, SCRATCH_PT(400, 300));
    GpiSetColor(hps, CLR_BLUE);
    GpiMove(hps, SCRATCH_PT(150, 150));
    GpiBox(hps, DRO_OUTLINE, SCRATCH_PT(350, 250), 0, 0);

    /* No segment should exist: pbGoca must be empty */
    {
        LONG ids[8];
        LONG count;
        memset(ids, 0, sizeof(ids));
        count = GpiQuerySegmentNames(hps, 0, 9999, 8, ids);
        CHECK_EQ_LONG(count, 0, "no segments created by DM_DRAW");
    }
    TestFreePS(hps);

    /* ============================================================ */
    /* T206 / G2 - SEGEM_INSERT vs SEGEM_REPLACE                    */
    /* ============================================================ */
    TEST(206, "G2: INSERT advances pointer, REPLACE replaces in place");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 1);
    GpiLabel(hps, 10);
    GpiLabel(hps, 20);
    GpiLabel(hps, 30);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    /* Reopen, INSERT after elem 1 */
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 1);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiSetElementPointer(hps, 1);
    GpiLabel(hps, 15);
    CHECK_EQ_LONG(GpiQueryElementPointer(hps), 2,
                  "INSERT: pointer advanced to 2");

    /* REPLACE elem 2 */
    GpiSetEditMode(hps, SEGEM_REPLACE);
    GpiSetElementPointer(hps, 2);
    GpiLabel(hps, 25);
    CHECK_EQ_LONG(GpiQueryElementPointer(hps), 2,
                  "REPLACE: pointer stayed at 2");

    /* Verify labels: 10, 25, 20, 30 */
    GpiSetEditMode(hps, SEGEM_INSERT);
    {
        LONG expected[4];
        LONG k;
        expected[0] = 10;
        expected[1] = 25;
        expected[2] = 20;
        expected[3] = 30;
        for (k = 0; k < 4; k++) {
            BYTE e[16];
            LONG label;
            GpiSetElementPointer(hps, k + 1);
            GpiQueryElement(hps, 0, sizeof(e), e);
            label = (LONG)((ULONG)e[2] | ((ULONG)e[3] << 8)
                  | ((ULONG)e[4] << 16) | ((ULONG)e[5] << 24));
            CHECK_EQ_LONG(label, expected[k], "label after INSERT+REPLACE");
        }
    }
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 1);
    TestFreePS(hps);

    /* ============================================================ */
    /* T207 / G3 - CloseSegment with open bracket -> GEEL last      */
    /* ============================================================ */
    TEST(207, "G3: CloseSegment with open bracket emits GEEL last");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 1);
    GpiBeginElement(hps, 0x81000099L, "I9");
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiLine(hps, SCRATCH_PT(100, 100));
    /* Intentionally no EndElement */
    rc = GpiCloseSegment(hps);
    CHECK_EQ_LONG(rc, 1, "CloseSegment with open bracket succeeds");
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 1, buf, sizeof(buf));
    CHECK(n > 0, "body readable");
    /* GEEL must be present and be the last 2 bytes */
    {
        LONG off = TestFindOrder(buf, n, OC_GEEL);
        CHECK(off >= 0, "GEEL present");
        CHECK_EQ_LONG(off, n - 2, "GEEL is last order (offset = n-2)");
    }
    /* No GEPTH */
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GEPTH), 0, "no GEPTH");
    GpiDeleteSegment(hps, 1);
    TestFreePS(hps);

    /* ============================================================ */
    /* T208 / G4 - CloseSegment with open path -> NO GEPTH          */
    /* ============================================================ */
    TEST(208, "G4: CloseSegment with open path emits no GEPTH");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 1);
    GpiBeginPath(hps, 1);
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiLine(hps, SCRATCH_PT(100, 100));
    /* Intentionally no EndPath */
    rc = GpiCloseSegment(hps);
    CHECK_EQ_LONG(rc, 1, "CloseSegment with open path succeeds");
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 1, buf, sizeof(buf));
    CHECK(n > 0, "body readable");
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GEPTH), 0,
                  "no GEPTH emitted (path terminated, not closed)");
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GBPTH), 1, "GBPTH present");
    GpiDeleteSegment(hps, 1);
    TestFreePS(hps);

    /* ============================================================ */
    /* T209 / G5 - SetEditMode inside bracket -> error              */
    /* ============================================================ */
    TEST(209, "G5: SetEditMode inside element bracket -> error");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 1);
    rc = GpiBeginElement(hps, 0x81000010L, "H11");
    CHECK_EQ_LONG(rc, 1, "BeginElement succeeds");
    rc = GpiSetEditMode(hps, SEGEM_REPLACE);
    CHECK_EQ_LONG(rc, GPI_ERROR, "SetEditMode inside bracket -> error");
    GpiEndElement(hps);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 1);
    TestFreePS(hps);

    /* ============================================================ */
    /* T210 / G6 - SrvDeliver pointer advance regression (I9-C)     */
    /* ============================================================ */
    TEST(210, "G6: GSCOL preamble precedes GBEL bracket in body");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 1);
    GpiSetColor(hps, CLR_RED);                /* element 1: GSCOL */
    GpiBeginElement(hps, 0x810000AAL, "C");   /* element 2: bracket */
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiEndElement(hps);
    rc = GpiCloseSegment(hps);
    CHECK_EQ_LONG(rc, 1, "CloseSegment succeeds");
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 1, buf, sizeof(buf));
    CHECK(n > 0, "body readable");
    {
        LONG posGSCOL = TestFindOrder(buf, n, OC_GSCOL);
        LONG posGBEL  = TestFindOrder(buf, n, OC_GBEL);
        CHECK(posGSCOL >= 0, "GSCOL present");
        CHECK(posGBEL  >= 0, "GBEL present");
        CHECK(posGSCOL < posGBEL,
              "GSCOL preamble BEFORE GBEL (pointer advanced)");
    }
    GpiDeleteSegment(hps, 1);
    TestFreePS(hps);

    /* ============================================================ */
    /* T211 / G7 - QueryElementType for GLBL                        */
    /* ============================================================ */
    TEST(211, "G7: QueryElementType for GLBL single order = 0xD3");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 1);
    GpiLabel(hps, 42);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 1);
    GpiSetElementPointer(hps, 1);
    {
        LONG lType = 0;
        GpiQueryElementType(hps, &lType, 0, NULL);
        CHECK_EQ_LONG(lType, OC_GLBL, "GLBL type = 0xD3");
    }
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 1);
    TestFreePS(hps);
}
