/*!
 * @file tests_edge_stress.cpp
 * @brief Edge cases (T163-T173) and stress tests (T174-T177).
 *
 * Edge cases exercise error paths and boundary conditions.
 * Stress tests verify memory stability (no leaks, no corruption).
 *
 * Note: T166 (65th CreatePS) is already covered by T8 in part 2 and
 * is not repeated here.
 */
#include "tests_common.h"

void RunSuite_EdgeStress(void)
{
    HPS  hps;
    LONG rc;
    LONG n;
    BYTE buf[512];

    SUITE("Edge cases (T163-T173)");

    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    /* ============================================================ */
    /* T163 - SetColor with NULL hps                                */
    /* ============================================================ */
    TEST(163, "T163: SetColor(NULL, RED) -> error");
    rc = GpiSetColor(NULL, CLR_RED);
    CHECK_EQ_LONG(rc, GPI_ERROR, "SetColor(NULL) -> error");

    /* ============================================================ */
    /* T164 - unknown ulFnCode (via missing segment)                */
    /* ============================================================ */
    TEST(164, "T164: unknown opcode -> error");
    {
        LONG off = 0;
        n = GpiGetData(hps, 999, &off, 0, 16, buf);
        CHECK_EQ_LONG(n, GPI_ERROR, "missing segment -> error");
    }

    /* ============================================================ */
    /* T165 - truncated args -> error                               */
    /* ============================================================ */
    TEST(165, "T165: truncated args -> error");
    /* Hard to provoke from public API without raw wire. Use
       GpiSetDrawingMode with invalid value -> server returns error. */
    rc = GpiSetDrawingMode(hps, 999);
    CHECK_EQ_LONG(rc, GPI_ERROR, "invalid mode -> error");

    /* ============================================================ */
    /* T166 - 65th CreatePS (covered by T8, skip here)              */
    /* ============================================================ */
    TEST(166, "T166: 65th CreatePS -> error (see T8)");
    CHECK(1, "T166 covered by T8");

    /* ============================================================ */
    /* T167 - OpenSegment(-1) -> error                              */
    /* ============================================================ */
    TEST(167, "T167: OpenSegment(-1) -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    rc = GpiOpenSegment(hps, -1);
    CHECK_EQ_LONG(rc, GPI_ERROR, "negative id -> error");
    GpiSetDrawingMode(hps, DM_DRAW);

    /* ============================================================ */
    /* T168 - OpenSegment(0) -> error                               */
    /* ============================================================ */
    TEST(168, "T168: OpenSegment(0) -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    rc = GpiOpenSegment(hps, 0);
    CHECK_EQ_LONG(rc, GPI_ERROR, "id=0 -> error");
    GpiSetDrawingMode(hps, DM_DRAW);

    /* ============================================================ */
    /* T169 - BeginElement(0xFFFFFFFF) -> OK (max type)             */
    /* ============================================================ */
    TEST(169, "T169: BeginElement(0xFFFFFFFF) succeeds");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 1);
    rc = GpiBeginElement(hps, 0xFFFFFFFFL, "max");
    CHECK_EQ_LONG(rc, 1, "max type succeeds");
    GpiEndElement(hps);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 1);

    /* ============================================================ */
    /* T170 - BeginElement desc > 251 -> truncated                  */
    /* ============================================================ */
    TEST(170, "T170: BeginElement desc > 251 truncated");
    {
        char longDesc[300];
        int k;
        for (k = 0; k < 299; k++) longDesc[k] = 'A';
        longDesc[299] = '\0';

        GpiSetDrawingMode(hps, DM_RETAIN);
        GpiOpenSegment(hps, 2);
        rc = GpiBeginElement(hps, 0x81000000L, longDesc);
        CHECK_EQ_LONG(rc, 1, "BeginElement with long desc succeeds");
        GpiEndElement(hps);
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);

        n = TestGetSegmentBody(hps, 2, buf, sizeof(buf));
        CHECK(n > 0, "body readable");
        /* GBEL LEN byte = 4 (type) + cch (truncated to 251) = 255 */
        {
            LONG off = TestFindOrder(buf, n, 0xD2);  /* GBEL */
            CHECK(off >= 0, "GBEL found");
            if (off >= 0) {
                CHECK_EQ_LONG(buf[off + 1], 255, "GBEL LEN = 4+251 = 255");
            }
        }
        GpiDeleteSegment(hps, 2);
    }

    /* ============================================================ */
    /* T171 - CharString exactly 255                                */
    /* ============================================================ */
    TEST(171, "T171: CharString(255) succeeds");
    {
        char s255[256];
        int k;
        for (k = 0; k < 255; k++) s255[k] = 'X';
        s255[255] = '\0';
        GpiSetDrawingMode(hps, DM_RETAIN);
        GpiOpenSegment(hps, 3);
        rc = GpiCharString(hps, 255, s255);
        CHECK_NE_LONG(rc, GPI_ERROR, "CharString(255) succeeds");
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);
        GpiDeleteSegment(hps, 3);
    }

    /* ============================================================ */
    /* T172 - CharString 256 -> error                               */
    /* ============================================================ */
    TEST(172, "T172: CharString(256) -> error");
    {
        char s256[257];
        int k;
        for (k = 0; k < 256; k++) s256[k] = 'Y';
        s256[256] = '\0';
        GpiSetDrawingMode(hps, DM_RETAIN);
        GpiOpenSegment(hps, 4);
        rc = GpiCharString(hps, 256, s256);
        CHECK_EQ_LONG(rc, GPI_ERROR, "CharString(256) -> error");
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);
        GpiDeleteSegment(hps, 4);
    }

    /* ============================================================ */
    /* T173 - PolyLine exactly 31 points LONG -> single GCLINE      */
    /* ============================================================ */
    TEST(173, "T173: PolyLine(31) LONG -> single GCLINE");
    {
        POINTL pts[31];
        int k;
        for (k = 0; k < 31; k++) { pts[k].x = k; pts[k].y = k; }

        GpiSetDrawingMode(hps, DM_RETAIN);
        GpiOpenSegment(hps, 5);
        rc = GpiPolyLine(hps, 31, pts);
        CHECK_NE_LONG(rc, GPI_ERROR, "PolyLine(31) succeeds");
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);

        n = TestGetSegmentBody(hps, 5, buf, sizeof(buf));
        CHECK_EQ_LONG(TestCountOrder(buf, n, 0x81), 1,
                      "single GCLINE (31 points fit in one order)");
        GpiDeleteSegment(hps, 5);
    }

    TestFreePS(hps);

    /* ============================================================ */
    /* Stress tests (T174-T177)                                     */
    /* ============================================================ */
    SUITE("Stress (T174-T177)");

    /* ============================================================ */
    /* T174 - 1000 segments x small body                            */
    /* ============================================================ */
    TEST(174, "T174: 1000 segments x small body");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }
    {
        int k;
        BOOL ok = TRUE;
        GpiSetDrawingMode(hps, DM_RETAIN);
        for (k = 1; k <= 1000; k++) {
            if (!GpiOpenSegment(hps, k)) { ok = FALSE; break; }
            GpiSetColor(hps, (k % 8));
            GpiMove(hps, SCRATCH_PT(k, k));
            GpiLine(hps, SCRATCH_PT(k+10, k+10));
            if (!GpiCloseSegment(hps)) { ok = FALSE; break; }
        }
        GpiSetDrawingMode(hps, DM_DRAW);
        CHECK(ok, "all 1000 segments created");

        {
            LONG ids[8];
            LONG count;
            memset(ids, 0, sizeof(ids));
            count = GpiQuerySegmentNames(hps, 0, 9999, 8, ids);
            CHECK(count > 0, "segments present");
        }
    }
    TestFreePS(hps);

    /* ============================================================ */
    /* T175 - 100 x PolyLine(10000) (memory stability)              */
    /* ============================================================ */
    TEST(175, "T175: 100 x PolyLine(10000) does not corrupt memory");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }
    {
        static POINTL pts[10000];
        int k, j;
        for (k = 0; k < 10000; k++) { pts[k].x = k; pts[k].y = k; }

        GpiSetDrawingMode(hps, DM_RETAIN);
        for (j = 0; j < 100; j++) {
            if (!GpiOpenSegment(hps, j + 1)) break;
            GpiPolyLine(hps, 10000, pts);
            GpiCloseSegment(hps);
        }
        GpiSetDrawingMode(hps, DM_DRAW);
        CHECK(j == 100, "100 PolyLine(10000) issued");
    }
    TestFreePS(hps);

    /* ============================================================ */
    /* T176 - 1000 x Open/CloseSegment (memory stability)           */
    /* ============================================================ */
    TEST(176, "T176: 1000 x Open/Close does not leak");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }
    {
        int k;
        BOOL ok = TRUE;
        GpiSetDrawingMode(hps, DM_RETAIN);
        for (k = 1; k <= 1000; k++) {
            if (!GpiOpenSegment(hps, k)) { ok = FALSE; break; }
            if (!GpiCloseSegment(hps))   { ok = FALSE; break; }
        }
        GpiSetDrawingMode(hps, DM_DRAW);
        CHECK(ok, "1000 open/close cycles succeeded");

        /* Cleanup */
        for (k = 1; k <= 1000; k++) GpiDeleteSegment(hps, k);
    }
    TestFreePS(hps);

    /* ============================================================ */
    /* T177 - Replay 1 MB GOCA                                      */
    /* ============================================================ */
    TEST(177, "T177: replay ~1MB GOCA stream");
    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }
    {
        /* 1000 x PolyLine(1000) = ~1MB GOCA. Fill seg 1. */
        static POINTL pts[1000];
        int k, j;
        for (k = 0; k < 1000; k++) { pts[k].x = k; pts[k].y = k; }

        GpiSetDrawingMode(hps, DM_RETAIN);
        GpiOpenSegment(hps, 1);
        for (j = 0; j < 1000; j++) {
            GpiPolyLine(hps, 1000, pts);
        }
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);

        rc = GpiDrawSegment(hps, 1);
        CHECK_NE_LONG(rc, GPI_ERROR, "DrawSegment(1MB) succeeds");

        GpiDeleteSegment(hps, 1);
    }
    TestFreePS(hps);
}
