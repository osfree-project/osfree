/*!
 * @file tests_primitives.cpp
 * @brief Primitive tests (T34-T56).
 *
 * Strategy: open a segment, perform the primitive, close, read body via
 * GpiGetData, scan for expected GOCA order code and verify structure.
 *
 * This exercises BOTH the emit path (server-side GocaEmit*) and the
 * retain path (SrvDeliver -> pbGoca append), without needing real GRE.
 */
#include "tests_common.h"

/* GOCA order codes used in tests */
#define OC_GSCOL   0x0A
#define OC_GSECOL  0x26
#define OC_GSCP    0x21
#define OC_GCLINE  0x81
#define OC_GCBOX   0x80
#define OC_GCMRK   0x82
#define OC_GCCHST  0x83
#define OC_GCHST   0xC3
#define OC_GSAP    0x22
#define OC_GSLT    0x18
#define OC_GSLW    0x19
#define OC_GSCS    0x38
#define OC_GSMX    0x0C
#define OC_GSPRP   0xA0

/* Helper: begin capture (open segment, set DM_RETAIN). */
static void CaptureBegin(HPS hps, LONG segId)
{
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, segId);
}

/* Body capture structure. */
typedef struct {
    BYTE buf[256];
    LONG n;
} BodyCapture;

/* Helper: end capture (close segment, read body via GetData). */
static void CaptureEnd(HPS hps, BodyCapture *pCap)
{
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    pCap->n = TestGetSegmentBody(hps, 1, pCap->buf, sizeof(pCap->buf));
}

void RunSuite_Primitives(void)
{
    HPS hps;
    LONG rc;
    BodyCapture cap;

    SUITE("Primitives (T34-T56)");

    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    /* ============================================================ */
    /* T34 - SetColor (0..255) -> GSCOL                             */
    /* ============================================================ */
    TEST(34, "T34: SetColor(CLR_RED) emits GSCOL");
    CaptureBegin(hps, 1);
    GpiSetColor(hps, CLR_RED);
    CaptureEnd(hps, &cap);
    CHECK(cap.n > 0, "body non-empty");
    CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GSCOL), 1, "1 GSCOL");
    {
        LONG off = TestFindOrder(cap.buf, cap.n, OC_GSCOL);
        CHECK(off >= 0, "GSCOL found");
        if (off >= 0) {
            CHECK_EQ_LONG(cap.buf[off + 1], CLR_RED, "GSCOL data = CLR_RED");
        }
    }

    /* ============================================================ */
    /* T35 - SetColor (256..65535) -> GSECOL                        */
    /* ============================================================ */
    TEST(35, "T35: SetColor(0x1234) emits GSECOL");
    CaptureBegin(hps, 1);
    GpiSetColor(hps, 0x1234);
    CaptureEnd(hps, &cap);
    CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GSECOL), 1, "1 GSECOL");
    {
        LONG off = TestFindOrder(cap.buf, cap.n, OC_GSECOL);
        if (off >= 0) {
            USHORT col = (USHORT)(cap.buf[off + 2] | (cap.buf[off + 3] << 8));
            CHECK_EQ_INT(col, 0x1234, "GSECOL data = 0x1234");
        }
    }

    /* ============================================================ */
    /* T36 - SetColor(-1) fails                                     */
    /* ============================================================ */
    TEST(36, "T36: SetColor(-1) returns GPI_ERROR");
    CaptureBegin(hps, 1);
    rc = GpiSetColor(hps, -1);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 1);
    CHECK_EQ_LONG(rc, GPI_ERROR, "SetColor(-1) -> error");

    /* ============================================================ */
    /* T37 - SetColor(0x10000) fails                                */
    /* ============================================================ */
    TEST(37, "T37: SetColor(0x10000) returns GPI_ERROR");
    CaptureBegin(hps, 1);
    rc = GpiSetColor(hps, 0x10000);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 1);
    CHECK_EQ_LONG(rc, GPI_ERROR, "SetColor(0x10000) -> error");

    /* ============================================================ */
    /* T38 - Move -> GSCP                                           */
    /* ============================================================ */
    TEST(38, "T38: Move({100,100}) emits GSCP");
    CaptureBegin(hps, 1);
    GpiMove(hps, SCRATCH_PT(100, 100));
    CaptureEnd(hps, &cap);
    CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GSCP), 1, "1 GSCP");
    {
        LONG off = TestFindOrder(cap.buf, cap.n, OC_GSCP);
        if (off >= 0) {
            LONG x = (LONG)((ULONG)cap.buf[off+2] | ((ULONG)cap.buf[off+3]<<8) |
                            ((ULONG)cap.buf[off+4]<<16) | ((ULONG)cap.buf[off+5]<<24));
            LONG y = (LONG)((ULONG)cap.buf[off+6] | ((ULONG)cap.buf[off+7]<<8) |
                            ((ULONG)cap.buf[off+8]<<16) | ((ULONG)cap.buf[off+9]<<24));
            CHECK_EQ_LONG(x, 100, "GSCP x = 100");
            CHECK_EQ_LONG(y, 100, "GSCP y = 100");
        }
    }

    /* ============================================================ */
    /* T39 - Line -> GCLINE                                         */
    /* ============================================================ */
    TEST(39, "T39: Line({200,200}) emits GCLINE");
    CaptureBegin(hps, 1);
    GpiMove(hps, SCRATCH_PT(100, 100));
    GpiLine(hps, SCRATCH_PT(200, 200));
    CaptureEnd(hps, &cap);
    CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GCLINE), 1, "1 GCLINE");

    /* ============================================================ */
    /* T40 - Box(DRO_FILL) -> GCBOX with FILL bit                   */
    /* ============================================================ */
    TEST(40, "T40: Box(DRO_FILL) -> GCBOX with FILL=0x40");
    CaptureBegin(hps, 1);
    GpiMove(hps, SCRATCH_PT(100, 100));
    GpiBox(hps, DRO_FILL, SCRATCH_PT(200, 200), 0, 0);
    CaptureEnd(hps, &cap);
    CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GCBOX), 1, "1 GCBOX");
    {
        LONG off = TestFindOrder(cap.buf, cap.n, OC_GCBOX);
        if (off >= 0) {
            BYTE flags = cap.buf[off + 2];
            CHECK((flags & 0x40) != 0, "FILL bit set");
            CHECK((flags & 0x20) == 0, "BOUNDARY bit not set");
        }
    }

    /* ============================================================ */
    /* T41 - Box(DRO_OUTLINE) -> GCBOX with BOUNDARY bit            */
    /* ============================================================ */
    TEST(41, "T41: Box(DRO_OUTLINE) -> GCBOX with BOUNDARY=0x20");
    CaptureBegin(hps, 1);
    GpiMove(hps, SCRATCH_PT(100, 100));
    GpiBox(hps, DRO_OUTLINE, SCRATCH_PT(200, 200), 0, 0);
    CaptureEnd(hps, &cap);
    {
        LONG off = TestFindOrder(cap.buf, cap.n, OC_GCBOX);
        if (off >= 0) {
            BYTE flags = cap.buf[off + 2];
            CHECK((flags & 0x20) != 0, "BOUNDARY bit set");
            CHECK((flags & 0x40) == 0, "FILL bit not set");
        }
    }

    /* ============================================================ */
    /* T42 - Box(FILL|OUTLINE) with rounded corners                 */
    /* ============================================================ */
    TEST(42, "T42: Box(DRO_OUTLINEFILL, 10,10) -> both flags + axes");
    CaptureBegin(hps, 1);
    GpiMove(hps, SCRATCH_PT(100, 100));
    GpiBox(hps, DRO_OUTLINEFILL, SCRATCH_PT(200, 200), 10, 10);
    CaptureEnd(hps, &cap);
    {
        LONG off = TestFindOrder(cap.buf, cap.n, OC_GCBOX);
        if (off >= 0) {
            BYTE flags = cap.buf[off + 2];
            CHECK((flags & 0x20) != 0, "BOUNDARY bit set");
            CHECK((flags & 0x40) != 0, "FILL bit set");
            /* LEN byte: 2 (flags+res) + 8 (corner) + 8 (axes) = 18 */
            CHECK_EQ_LONG(cap.buf[off + 1], 18, "GCBOX LEN = 18 (with axes)");
        }
    }

    /* ============================================================ */
    /* T43 - PolyLine(3) -> single GCLINE (3 points in one order)   */
    /* ============================================================ */
    TEST(43, "T43: PolyLine(3) emits 1 GCLINE with 3 points");
    {
        POINTL pts[3];
        pts[0].x = 10; pts[0].y = 10;
        pts[1].x = 20; pts[1].y = 20;
        pts[2].x = 30; pts[2].y = 30;
        CaptureBegin(hps, 1);
        GpiPolyLine(hps, 3, pts);
        CaptureEnd(hps, &cap);
        CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GCLINE), 1, "1 GCLINE");
        {
            LONG off = TestFindOrder(cap.buf, cap.n, OC_GCLINE);
            if (off >= 0) {
                /* LEN = 3 * 2 * coordSize. For LONG (4) -> 3*8 = 24. */
                LONG len = cap.buf[off + 1];
                CHECK(len == 24 || len == 12,
                      "GCLINE LEN = 24 (LONG) or 12 (SHORT)");
            }
        }
    }

    /* ============================================================ */
    /* T44 - PolyLine(0) fails                                      */
    /* ============================================================ */
    TEST(44, "T44: PolyLine(0) returns error");
    CaptureBegin(hps, 1);
    rc = GpiPolyLine(hps, 0, NULL);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 1);
    CHECK_EQ_LONG(rc, GPI_ERROR, "PolyLine(0) -> error");

    /* ============================================================ */
    /* T45 - PolyLine(10000) works (large payload)                  */
    /* ============================================================ */
    TEST(45, "T45: PolyLine(10000) succeeds (large payload)");
    {
        static POINTL pts[10000];
        LONG k;
        for (k = 0; k < 10000; k++) { pts[k].x = k; pts[k].y = k; }
        CaptureBegin(hps, 1);
        rc = GpiPolyLine(hps, 10000, pts);
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);
        CHECK_NE_LONG(rc, GPI_ERROR, "PolyLine(10000) succeeds");
        GpiDeleteSegment(hps, 1);
    }

    /* ============================================================ */
    /* T46 - Marker -> GCMRK                                        */
    /* ============================================================ */
    TEST(46, "T46: Marker emits GCMRK");
    CaptureBegin(hps, 1);
    GpiMarker(hps, SCRATCH_PT(100, 100));
    CaptureEnd(hps, &cap);
    CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GCMRK), 1, "1 GCMRK");

    /* ============================================================ */
    /* T47 - PolyMarker(3)                                          */
    /* ============================================================ */
    TEST(47, "T47: PolyMarker(3) emits GCMRK(s)");
    {
        POINTL pts[3];
        pts[0].x = 10; pts[0].y = 10;
        pts[1].x = 20; pts[1].y = 20;
        pts[2].x = 30; pts[2].y = 30;
        CaptureBegin(hps, 1);
        GpiPolyMarker(hps, 3, pts);
        CaptureEnd(hps, &cap);
        CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GCMRK), 1, "1 GCMRK");
    }

    /* ============================================================ */
    /* T48 - CharString -> GCCHST                                   */
    /* ============================================================ */
    TEST(48, "T48: CharString(\"Hello\") emits GCCHST");
    CaptureBegin(hps, 1);
    GpiCharString(hps, 5, "Hello");
    CaptureEnd(hps, &cap);
    CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GCCHST), 1, "1 GCCHST");
    {
        LONG off = TestFindOrder(cap.buf, cap.n, OC_GCCHST);
        if (off >= 0) {
            CHECK_EQ_LONG(cap.buf[off + 1], 5, "GCCHST LEN = 5");
            CHECK(memcmp(&cap.buf[off + 2], "Hello", 5) == 0, "text = Hello");
        }
    }

    /* ============================================================ */
    /* T49 - CharString("") fails                                   */
    /* ============================================================ */
    TEST(49, "T49: CharString(\"\") returns error");
    CaptureBegin(hps, 1);
    rc = GpiCharString(hps, 0, "");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 1);
    CHECK_EQ_LONG(rc, GPI_ERROR, "CharString(0) -> error");

    /* ============================================================ */
    /* T50 - CharString(256) fails                                  */
    /* ============================================================ */
    TEST(50, "T50: CharString(256) returns error (max 255)");
    {
        char big[257];
        int k;
        for (k = 0; k < 256; k++) big[k] = 'A';
        big[256] = '\0';
        CaptureBegin(hps, 1);
        rc = GpiCharString(hps, 256, big);
        GpiCloseSegment(hps);
        GpiSetDrawingMode(hps, DM_DRAW);
        GpiDeleteSegment(hps, 1);
        CHECK_EQ_LONG(rc, GPI_ERROR, "CharString(256) -> error");
    }

    /* ============================================================ */
    /* T51 - CharStringAt -> GCHST                                  */
    /* ============================================================ */
    TEST(51, "T51: CharStringAt({0,0}, \"Hi\") emits GCHST");
    CaptureBegin(hps, 1);
    GpiCharStringAt(hps, SCRATCH_PT(0, 0), 2, "Hi");
    CaptureEnd(hps, &cap);
    CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GCHST), 1, "1 GCHST");

    /* ============================================================ */
    /* T52 - SetArcParams -> GSAP                                   */
    /* ============================================================ */
    TEST(52, "T52: SetArcParams emits GSAP");
    {
        ARCPARAMS ap;
        ap.lP = 1; ap.lQ = 1; ap.lR = 0; ap.lS = 0;
        CaptureBegin(hps, 1);
        GpiSetArcParams(hps, &ap);
        CaptureEnd(hps, &cap);
        CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GSAP), 1, "1 GSAP");
    }

    /* ============================================================ */
    /* T53 - SetAttrs(LINE, COLOR|TYPE) -> GSCOL + GSLT             */
    /* ============================================================ */
    TEST(53, "T53: SetAttrs(LINE, COLOR|TYPE) -> GSCOL + GSLT");
    {
        LINEBUNDLE lb;
        memset(&lb, 0, sizeof(lb));
        lb.lColor = CLR_RED;
        lb.usType = 3;   /* dotted */
        CaptureBegin(hps, 1);
        GpiSetAttrs(hps, PRIM_LINE, LBB_COLOR | LBB_TYPE, 0, (PBUNDLE)&lb);
        CaptureEnd(hps, &cap);
        CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GSCOL), 1, "1 GSCOL");
        CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GSLT),  1, "1 GSLT");
    }

    /* ============================================================ */
    /* T54 - SetAttrs(CHAR, COLOR|SET) -> GSCOL + GSCS              */
    /* ============================================================ */
    TEST(54, "T54: SetAttrs(CHAR, COLOR|SET) -> GSCOL + GSCS");
    {
        CHARBUNDLE cb;
        memset(&cb, 0, sizeof(cb));
        cb.lColor = CLR_BLUE;
        cb.usSet  = 1;
        CaptureBegin(hps, 1);
        GpiSetAttrs(hps, PRIM_CHAR, CBB_COLOR | CBB_SET, 0, (PBUNDLE)&cb);
        CaptureEnd(hps, &cap);
        CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GSCOL), 1, "1 GSCOL");
        CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GSCS),  1, "1 GSCS");
    }

    /* ============================================================ */
    /* T55 - SetAttrs(MARKER, COLOR) -> GSCOL                       */
    /* ============================================================ */
    TEST(55, "T55: SetAttrs(MARKER, COLOR) -> GSCOL");
    {
        MARKERBUNDLE mb;
        memset(&mb, 0, sizeof(mb));
        mb.lColor = CLR_GREEN;
        CaptureBegin(hps, 1);
        GpiSetAttrs(hps, PRIM_MARKER, MBB_COLOR, 0, (PBUNDLE)&mb);
        CaptureEnd(hps, &cap);
        CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GSCOL), 1, "1 GSCOL");
    }

    /* ============================================================ */
    /* T56 - SetAttrs(AREA, REF_POINT) -> GSPRP                     */
    /* ============================================================ */
    TEST(56, "T56: SetAttrs(AREA, REF_POINT) -> GSPRP");
    {
        AREABUNDLE ab;
        memset(&ab, 0, sizeof(ab));
        ab.ptlRefPoint.x = 50;
        ab.ptlRefPoint.y = 50;
        CaptureBegin(hps, 1);
        GpiSetAttrs(hps, PRIM_AREA, ABB_REF_POINT, 0, (PBUNDLE)&ab);
        CaptureEnd(hps, &cap);
        CHECK_EQ_LONG(TestCountOrder(cap.buf, cap.n, OC_GSPRP), 1, "1 GSPRP");
    }

    TestFreePS(hps);
}
