/*!
 * @file tests_segments_basic.cpp
 * @brief Basic segment tests (T57-T77).
 *
 * Covers: open/close, draw, delete, names, attrs, GetData, PutData,
 * round-trip.
 */
#include "tests_common.h"

#define OC_GSCOL   0x0A
#define OC_GSCP    0x21
#define OC_GCLINE  0x81
#define OC_GBSEG   0x70

/* Helper: fill seg with one colored line. */
static BOOL MakeLineSeg(HPS hps, LONG segId, LONG color)
{
    if (!GpiOpenSegment(hps, segId)) return FALSE;
    GpiSetColor(hps, color);
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiLine(hps, SCRATCH_PT(100, 100));
    return GpiCloseSegment(hps);
}

void RunSuite_SegmentsBasic(void)
{
    HPS  hps;
    LONG rc;
    BYTE buf[512];
    LONG n;

    SUITE("Segments basic (T57-T77)");

    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    /* ============================================================ */
    /* T57 - OpenSegment + Move + Line + Close                      */
    /* ============================================================ */
    TEST(57, "T57: OpenSegment(1) + Move + Line + Close");
    GpiSetDrawingMode(hps, DM_RETAIN);
    rc = GpiOpenSegment(hps, 1);
    CHECK_EQ_LONG(rc, 1, "OpenSegment(1) succeeds");
    GpiSetColor(hps, CLR_RED);
    GpiMove(hps, SCRATCH_PT(10, 10));
    GpiLine(hps, SCRATCH_PT(100, 100));
    rc = GpiCloseSegment(hps);
    CHECK_EQ_LONG(rc, 1, "CloseSegment succeeds");
    GpiSetDrawingMode(hps, DM_DRAW);

    n = TestGetSegmentBody(hps, 1, buf, sizeof(buf));
    CHECK(n > 0, "body non-empty");
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GSCOL), 1, "1 GSCOL in body");
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GSCP),  1, "1 GSCP in body");
    CHECK_EQ_LONG(TestCountOrder(buf, n, OC_GCLINE),1, "1 GCLINE in body");

    /* ============================================================ */
    /* T58 - OpenSegment while already open -> error                */
    /* ============================================================ */
    TEST(58, "T58: OpenSegment(2) while seg 1 open -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 2);
    rc = GpiOpenSegment(hps, 3);   /* should fail */
    CHECK_EQ_LONG(rc, GPI_ERROR, "second OpenSegment fails");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegments(hps, 1, 3);

    /* ============================================================ */
    /* T59 - CloseSegment without OpenSegment -> error              */
    /* ============================================================ */
    TEST(59, "T59: CloseSegment without Open -> error");
    rc = GpiCloseSegment(hps);
    CHECK_EQ_LONG(rc, GPI_ERROR, "CloseSegment without open -> error");

    /* ============================================================ */
    /* T60 - OpenSegment while path open -> error                   */
    /* ============================================================ */
    TEST(60, "T60: OpenSegment while path open -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 10);
    GpiBeginPath(hps, 1);
    rc = GpiOpenSegment(hps, 11);
    CHECK_EQ_LONG(rc, GPI_ERROR, "OpenSegment while path open -> error");
    GpiEndPath(hps);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 10);

    /* ============================================================ */
    /* T61 - OpenSegment in DM_DRAW -> error                        */
    /* ============================================================ */
    TEST(61, "T61: OpenSegment in DM_DRAW -> error");
    GpiSetDrawingMode(hps, DM_DRAW);
    rc = GpiOpenSegment(hps, 20);
    CHECK_EQ_LONG(rc, GPI_ERROR, "OpenSegment in DM_DRAW -> error");

    /* ============================================================ */
    /* T62 - DrawSegment replays orders                             */
    /* ============================================================ */
    TEST(62, "T62: DrawSegment(1) replays body");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLineSeg(hps, 1, CLR_RED);
    GpiSetDrawingMode(hps, DM_DRAW);
    rc = GpiDrawSegment(hps, 1);
    CHECK_NE_LONG(rc, GPI_ERROR, "DrawSegment(1) succeeds");

    /* ============================================================ */
    /* T63 - DrawSegment(missing) -> error                          */
    /* ============================================================ */
    TEST(63, "T63: DrawSegment(999) -> error");
    rc = GpiDrawSegment(hps, 999);
    CHECK_EQ_LONG(rc, GPI_ERROR, "DrawSegment(999) -> error");

    /* ============================================================ */
    /* T64 - DeleteSegment                                          */
    /* ============================================================ */
    TEST(64, "T64: DeleteSegment(1) succeeds, then DrawSegment fails");
    rc = GpiDeleteSegment(hps, 1);
    CHECK_EQ_LONG(rc, 1, "DeleteSegment(1) succeeds");
    rc = GpiDrawSegment(hps, 1);
    CHECK_EQ_LONG(rc, GPI_ERROR, "DrawSegment(1) after delete -> error");

    /* ============================================================ */
    /* T65 - DeleteSegment on open segment -> error                 */
    /* ============================================================ */
    TEST(65, "T65: DeleteSegment on open segment -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 5);
    rc = GpiDeleteSegment(hps, 5);
    CHECK_EQ_LONG(rc, GPI_ERROR, "DeleteSegment(open) -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 5);

    /* ============================================================ */
    /* T66 - DeleteSegments range                                   */
    /* ============================================================ */
    TEST(66, "T66: DeleteSegments(100, 102) deletes only range");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLineSeg(hps, 100, CLR_RED);
    MakeLineSeg(hps, 101, CLR_GREEN);
    MakeLineSeg(hps, 102, CLR_BLUE);
    GpiSetDrawingMode(hps, DM_DRAW);

    rc = GpiDeleteSegments(hps, 101, 101);
    CHECK_EQ_LONG(rc, 1, "DeleteSegments(101,101) succeeds");

    {
        LONG ids[8];
        LONG count;
        memset(ids, 0, sizeof(ids));
        count = GpiQuerySegmentNames(hps, 0, 999, 8, ids);
        CHECK_EQ_LONG(count, 2, "2 segments left");
        CHECK_EQ_LONG(ids[0], 100, "ids[0] = 100");
        CHECK_EQ_LONG(ids[1], 102, "ids[1] = 102");
    }
    GpiDeleteSegment(hps, 100);
    GpiDeleteSegment(hps, 102);

    /* ============================================================ */
    /* T67 - QuerySegmentNames                                      */
    /* ============================================================ */
    TEST(67, "T67: QuerySegmentNames returns correct ids");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLineSeg(hps, 10, CLR_RED);
    MakeLineSeg(hps, 20, CLR_GREEN);
    MakeLineSeg(hps, 30, CLR_BLUE);
    GpiSetDrawingMode(hps, DM_DRAW);

    {
        LONG ids[8];
        LONG count;
        memset(ids, 0, sizeof(ids));
        count = GpiQuerySegmentNames(hps, 0, 999, 8, ids);
        CHECK_EQ_LONG(count, 3, "3 segments");
        CHECK_EQ_LONG(ids[0], 10, "ids[0] = 10");
        CHECK_EQ_LONG(ids[1], 20, "ids[1] = 20");
        CHECK_EQ_LONG(ids[2], 30, "ids[2] = 30");
    }

    /* ============================================================ */
    /* T68 - QuerySegmentAttrs (default VISIBLE = ON)               */
    /* ============================================================ */
    TEST(68, "T68: QuerySegmentAttrs(10, ATTR_VISIBLE) = ATTR_ON");
    CHECK_EQ_LONG(GpiQuerySegmentAttrs(hps, 10, ATTR_VISIBLE), ATTR_ON,
                  "VISIBLE default = ON");

    /* ============================================================ */
    /* T69 - SetSegmentAttrs(ATTR_DYNAMIC, ON) + Query              */
    /* ============================================================ */
    TEST(69, "T69: SetSegmentAttrs(10, DYNAMIC, ON) + Query");
    rc = GpiSetSegmentAttrs(hps, 10, ATTR_DYNAMIC, ATTR_ON);
    CHECK_EQ_LONG(rc, 1, "SetSegmentAttrs succeeds");
    CHECK_EQ_LONG(GpiQuerySegmentAttrs(hps, 10, ATTR_DYNAMIC), ATTR_ON,
                  "DYNAMIC = ON after set");
    GpiSetSegmentAttrs(hps, 10, ATTR_DYNAMIC, ATTR_OFF);
    CHECK_EQ_LONG(GpiQuerySegmentAttrs(hps, 10, ATTR_DYNAMIC), ATTR_OFF,
                  "DYNAMIC = OFF after reset");

    /* ============================================================ */
    /* T70 - GetData (valid offset, valid len)                      */
    /* ============================================================ */
    TEST(70, "T70: GetData(10, 0, 100) returns body");
    {
        LONG offset = 0;
        memset(buf, 0xEE, sizeof(buf));
        n = GpiGetData(hps, 10, &offset, 0, 100, buf);
        CHECK(n > 0, "GetData > 0");
        CHECK_EQ_LONG(offset, n, "offset = n");
        CHECK_EQ_LONG(buf[0], OC_GSCOL, "body starts with GSCOL");
    }

    /* ============================================================ */
    /* T71 - GetData (past-end offset) -> 0 bytes                   */
    /* ============================================================ */
    TEST(71, "T71: GetData with offset past end returns 0");
    {
        LONG offset = 1000;
        n = GpiGetData(hps, 10, &offset, 0, 100, buf);
        CHECK_EQ_LONG(n, 0, "0 bytes returned");
    }

    /* ============================================================ */
    /* T72 - GetData on empty segment -> 0 bytes                    */
    /* ============================================================ */
    TEST(72, "T72: GetData on empty segment returns 0");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 50);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    {
        LONG offset = 0;
        n = GpiGetData(hps, 50, &offset, 0, 100, buf);
        CHECK_EQ_LONG(n, 0, "0 bytes (empty segment)");
    }
    GpiDeleteSegment(hps, 50);

    /* ============================================================ */
    /* T73 - PutData into new segment                               */
    /* ============================================================ */
    TEST(73, "T73: PutData copies body into new segment");
    {
        BYTE body[16];
        LONG bodyLen, count;
        LONG off = 0;

        /* Prepare body: GSCOL(2) + GCLINE(10) = 12 bytes */
        body[0]  = OC_GSCOL; body[1]  = CLR_RED;
        body[2]  = OC_GCLINE; body[3] = 8;
        body[4]  = 10; body[5] = 0; body[6] = 0; body[7] = 0;
        body[8]  = 100; body[9] = 0; body[10] = 0; body[11] = 0;
        bodyLen = 12;

        GpiSetDrawingMode(hps, DM_RETAIN);
        GpiOpenSegment(hps, 51);
        count = bodyLen;
        rc = GpiPutData(hps, 0, &count, body);
        CHECK_NE_LONG(rc, GPI_ERROR, "PutData succeeds");
        CHECK_EQ_LONG(count, bodyLen, "all bytes consumed");
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);

        n = GpiGetData(hps, 51, &off, 0, sizeof(buf), buf);
        CHECK_EQ_LONG(n, bodyLen, "body length = 12");
        CHECK(memcmp(buf, body, bodyLen) == 0, "body matches input");
    }
    GpiDeleteSegment(hps, 51);

    /* ============================================================ */
    /* T74 - PutData with GBSEG in body (should skip GBSEG)         */
    /* ============================================================ */
    TEST(74, "T74: PutData skips GBSEG in input body");
    {
        BYTE body[32];
        LONG bodyLen, count;
        LONG off = 0;

        /* GBSEG (14 B) + GSCOL (2 B) */
        memset(body, 0, sizeof(body));
        body[0] = OC_GBSEG; body[1] = 0x0C;
        body[2] = 0xAA; body[3] = 0; body[4] = 0; body[5] = 0;
        body[6] = 0; body[7] = 0; body[8] = 0; body[9] = 0;
        body[10] = 0; body[11] = 0; body[12] = 0; body[13] = 0;
        body[14] = OC_GSCOL; body[15] = CLR_BLUE;
        bodyLen = 16;

        GpiSetDrawingMode(hps, DM_RETAIN);
        GpiOpenSegment(hps, 52);
        count = bodyLen;
        rc = GpiPutData(hps, 0, &count, body);
        CHECK_NE_LONG(rc, GPI_ERROR, "PutData succeeds");
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);

        n = GpiGetData(hps, 52, &off, 0, sizeof(buf), buf);
        CHECK_EQ_LONG(n, 2, "body length = 2 (only GSCOL)");
        CHECK_EQ_LONG(buf[0], OC_GSCOL, "only GSCOL, no GBSEG");
    }
    GpiDeleteSegment(hps, 52);

    /* ============================================================ */
    /* T75 - PutData in SEGEM_REPLACE -> error                      */
    /* ============================================================ */
    TEST(75, "T75: PutData in SEGEM_REPLACE -> error");
    {
        BYTE body[2];
        LONG count;
        body[0] = OC_GSCOL; body[1] = CLR_RED;
        count = 2;
        GpiSetDrawingMode(hps, DM_RETAIN);
        GpiOpenSegment(hps, 53);
        GpiSetEditMode(hps, SEGEM_REPLACE);
        rc = GpiPutData(hps, 0, &count, body);
        CHECK_EQ_LONG(rc, GPI_ERROR, "PutData in REPLACE -> error");
        GpiSetEditMode(hps, SEGEM_INSERT);
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);
        GpiDeleteSegment(hps, 53);
    }

    /* ============================================================ */
    /* T76 - PutData with pointer not at end -> error               */
    /* ============================================================ */
    TEST(76, "T76: PutData with pointer not at last -> error");
    {
        BYTE body[2];
        LONG count;
        body[0] = OC_GSCOL; body[1] = CLR_RED;
        count = 2;

        /* Build seg with two elements */
        GpiSetDrawingMode(hps, DM_RETAIN);
        GpiOpenSegment(hps, 54);
        GpiSetColor(hps, CLR_RED);
        GpiSetColor(hps, CLR_GREEN);
        GpiCloseSegment(hps);

        /* Reopen, set pointer to 0 (not at end) */
        GpiOpenSegment(hps, 54);
        GpiSetElementPointer(hps, 0);
        rc = GpiPutData(hps, 0, &count, body);
        CHECK_EQ_LONG(rc, GPI_ERROR, "PutData at ptr=0 -> error");

        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);
        GpiDeleteSegment(hps, 54);
    }

    /* ============================================================ */
    /* T77 - GetData(seg1) -> PutData(seg2) -> identical bodies     */
    /* ============================================================ */
    TEST(77, "T77: GetData -> PutData round-trip preserves body");
    {
        LONG offset = 0;
        LONG origLen, newLen;

        /* Build seg 60 */
        GpiSetDrawingMode(hps, DM_RETAIN);
        MakeLineSeg(hps, 60, CLR_RED);
        GpiSetDrawingMode(hps, DM_DRAW);

        /* Read into buf */
        origLen = GpiGetData(hps, 60, &offset, 0, sizeof(buf), buf);
        CHECK(origLen > 0, "read seg 60");

        /* Write into seg 61 */
        GpiSetDrawingMode(hps, DM_RETAIN);
        GpiOpenSegment(hps, 61);
        {
            LONG count = origLen;
            rc = GpiPutData(hps, 0, &count, buf);
            CHECK_NE_LONG(rc, GPI_ERROR, "PutData into seg 61");
        }
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);

        /* Read seg 61, compare */
        {
            BYTE buf2[512];
            LONG offset2 = 0;
            newLen = GpiGetData(hps, 61, &offset2, 0, sizeof(buf2), buf2);
            CHECK_EQ_LONG(newLen, origLen, "same body length");
            CHECK(memcmp(buf, buf2, origLen) == 0, "byte-identical body");
        }
        GpiDeleteSegments(hps, 60, 61);
    }

    GpiDeleteSegments(hps, 0, 9999);
    TestFreePS(hps);
}
