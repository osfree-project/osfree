/*!
 * @file tests_elements.cpp
 * @brief Element editing tests (T102-T126).
 *
 * Covers: BeginElement, EndElement, Label, Element, QueryElement,
 * QueryElementType, DeleteElement*, SetElementPointer*, element pointer
 * clamping, CloseSegment with open bracket.
 */
#include "tests_common.h"

#define OC_GLBL    0xD3
#define OC_GBEL    0xD2
#define OC_GEEL    0x49
#define OC_GSCOL   0x0A
#define OC_GSCP    0x21
#define OC_GCLINE  0x81

/* Helper: build a segment with N labels (labels 10, 20, 30, ...). */
static BOOL MakeLabelsSeg(HPS hps, LONG segId, LONG nLabels)
{
    LONG k;
    if (!GpiOpenSegment(hps, segId)) return FALSE;
    for (k = 0; k < nLabels; k++) {
        GpiLabel(hps, (k + 1) * 10);
    }
    return GpiCloseSegment(hps);
}

void RunSuite_Elements(void)
{
    HPS   hps;
    LONG  rc;
    LONG  lType;
    LONG  cbReq;
    LONG  got;
    LONG  ptr;
    BYTE  elem[64];
    BYTE  body[32];
    char  szDesc[16];

    SUITE("Elements (T102-T126)");

    hps = TestNewPS();
    if (!hps) { CHECK(0, "TestNewPS failed"); return; }

    /* ============================================================ */
    /* T102 - BeginElement + Move + EndElement                      */
    /* ============================================================ */
    TEST(102, "T102: BeginElement(0x81000000) + body + EndElement");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 100);
    rc = GpiBeginElement(hps, 0x81000000L, "test");
    CHECK_EQ_LONG(rc, 1, "BeginElement succeeds");
    GpiMove(hps, SCRATCH_PT(10, 10));
    rc = GpiEndElement(hps);
    CHECK_EQ_LONG(rc, 1, "EndElement succeeds");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    /* Verify: element 1 exists, type = 0x81000000 */
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 100);
    GpiSetElementPointer(hps, 1);
    lType = 0;
    cbReq = GpiQueryElementType(hps, &lType, 0, NULL);
    CHECK_EQ_LONG(lType, 0x81000000L, "element type = 0x81000000");
    CHECK(cbReq > 0, "cbReq > 0");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 100);

    /* ============================================================ */
    /* T103 - BeginElement with type < 0x81000000 -> error          */
    /* ============================================================ */
    TEST(103, "T103: BeginElement(0x80000000) -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 101);
    rc = GpiBeginElement(hps, 0x80000000L, "bad");
    CHECK_EQ_LONG(rc, GPI_ERROR, "type < 0x81000000 -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 101);

    /* ============================================================ */
    /* T104 - BeginElement while open -> error                      */
    /* ============================================================ */
    TEST(104, "T104: BeginElement while already open -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 102);
    GpiBeginElement(hps, 0x81000001L, "a");
    rc = GpiBeginElement(hps, 0x81000002L, "b");
    CHECK_EQ_LONG(rc, GPI_ERROR, "nested BeginElement -> error");
    GpiEndElement(hps);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 102);

    /* ============================================================ */
    /* T105 - BeginElement in DM_DRAW -> error                      */
    /* ============================================================ */
    TEST(105, "T105: BeginElement in DM_DRAW -> error");
    GpiSetDrawingMode(hps, DM_DRAW);
    rc = GpiBeginElement(hps, 0x81000003L, "x");
    CHECK_EQ_LONG(rc, GPI_ERROR, "BeginElement in DM_DRAW -> error");

    /* ============================================================ */
    /* T106 - EndElement without Begin -> error                     */
    /* ============================================================ */
    TEST(106, "T106: EndElement without Begin -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 103);
    rc = GpiEndElement(hps);
    CHECK_EQ_LONG(rc, GPI_ERROR, "EndElement without Begin -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 103);

    /* ============================================================ */
    /* T107 - Element (bracket + body as one op)                    */
    /* ============================================================ */
    TEST(107, "T107: Element(0x81000000, \"d\", data) creates 1 element");
    /* body = GSCOL(2) + GSCP(10) */
    body[0] = OC_GSCOL; body[1] = CLR_RED;
    body[2] = OC_GSCP;  body[3] = 8;
    body[4] = 100; body[5] = 0; body[6] = 0; body[7] = 0;
    body[8] = 100; body[9] = 0; body[10] = 0; body[11] = 0;

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 104);
    rc = GpiElement(hps, 0x81000000L, "d", 12, body);
    CHECK_NE_LONG(rc, GPI_ERROR, "Element succeeds");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 104);
    GpiSetElementPointer(hps, 1);
    lType = 0;
    GpiQueryElementType(hps, &lType, 0, NULL);
    CHECK_EQ_LONG(lType, 0x81000000L, "element type = 0x81000000");
    got = GpiQueryElement(hps, 0, sizeof(elem), elem);
    CHECK_EQ_LONG(got, 12, "body = 12 bytes");
    CHECK(memcmp(elem, body, 12) == 0, "body matches");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 104);

    /* ============================================================ */
    /* T108 - Element with GBEL/GEEL in body -> error               */
    /* ============================================================ */
    TEST(108, "T108: Element with GBEL/GEEL in body -> error");
    body[0] = OC_GBEL; body[1] = 4;
    body[2] = 0; body[3] = 0; body[4] = 0; body[5] = 0;
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 105);
    rc = GpiElement(hps, 0x81000000L, "d", 6, body);
    CHECK_EQ_LONG(rc, GPI_ERROR, "GBEL in body -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 105);

    /* ============================================================ */
    /* T109 - Label                                                 */
    /* ============================================================ */
    TEST(109, "T109: Label(42) emits GLABL");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 106);
    rc = GpiLabel(hps, 42);
    CHECK_EQ_LONG(rc, 1, "Label succeeds");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 106);
    GpiSetElementPointer(hps, 1);
    got = GpiQueryElement(hps, 0, sizeof(elem), elem);
    CHECK_EQ_LONG(got, 6, "label body = 6 bytes");
    CHECK_EQ_LONG(elem[0], OC_GLBL, "first byte = GLABL");
    {
        LONG label = (LONG)((ULONG)elem[2] | ((ULONG)elem[3] << 8)
                    | ((ULONG)elem[4] << 16) | ((ULONG)elem[5] << 24));
        CHECK_EQ_LONG(label, 42, "label = 42");
    }
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 106);

    /* ============================================================ */
    /* T110 - QueryElementPointer initial = 0                       */
    /* ============================================================ */
    TEST(110, "T110: QueryElementPointer initial = 0");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 110, 3);
    GpiOpenSegment(hps, 110);
    ptr = GpiQueryElementPointer(hps);
    CHECK_EQ_LONG(ptr, 0, "initial pointer = 0");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 110);

    /* ============================================================ */
    /* T111 - SetElementPointer(2) + Query                          */
    /* ============================================================ */
    TEST(111, "T111: SetElementPointer(2) + Query = 2");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 111, 5);
    GpiOpenSegment(hps, 111);
    rc = GpiSetElementPointer(hps, 2);
    CHECK_EQ_LONG(rc, 1, "SetElementPointer(2) succeeds");
    CHECK_EQ_LONG(GpiQueryElementPointer(hps), 2, "pointer = 2");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 111);

    /* ============================================================ */
    /* T112 - SetElementPointer(999) clamps to N                    */
    /* ============================================================ */
    TEST(112, "T112: SetElementPointer(999) clamps to N");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 112, 3);
    GpiOpenSegment(hps, 112);
    GpiSetElementPointer(hps, 999);
    CHECK_EQ_LONG(GpiQueryElementPointer(hps), 3, "pointer clamped to 3");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 112);

    /* ============================================================ */
    /* T113 - SetElementPointer(-1) clamps to 0                     */
    /* ============================================================ */
    TEST(113, "T113: SetElementPointer(-1) clamps to 0");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 113, 3);
    GpiOpenSegment(hps, 113);
    GpiSetElementPointer(hps, 2);
    GpiSetElementPointer(hps, -1);
    CHECK_EQ_LONG(GpiQueryElementPointer(hps), 0, "pointer clamped to 0");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 113);

    /* ============================================================ */
    /* T114 - OffsetElementPointer(+1)                              */
    /* ============================================================ */
    TEST(114, "T114: OffsetElementPointer(+1) increments");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 114, 3);
    GpiOpenSegment(hps, 114);
    GpiSetElementPointer(hps, 1);
    rc = GpiOffsetElementPointer(hps, 1);
    CHECK_EQ_LONG(rc, 1, "Offset+1 succeeds");
    CHECK_EQ_LONG(GpiQueryElementPointer(hps), 2, "pointer = 2");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 114);

    /* ============================================================ */
    /* T115 - OffsetElementPointer(-1)                              */
    /* ============================================================ */
    TEST(115, "T115: OffsetElementPointer(-1) decrements");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 115, 3);
    GpiOpenSegment(hps, 115);
    GpiSetElementPointer(hps, 2);
    rc = GpiOffsetElementPointer(hps, -1);
    CHECK_EQ_LONG(rc, 1, "Offset-1 succeeds");
    CHECK_EQ_LONG(GpiQueryElementPointer(hps), 1, "pointer = 1");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 115);

    /* ============================================================ */
    /* T116 - QueryElement returns body                             */
    /* ============================================================ */
    TEST(116, "T116: QueryElement returns full body");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 116);
    GpiLabel(hps, 10);
    GpiLabel(hps, 20);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 116);
    GpiSetElementPointer(hps, 2);
    got = GpiQueryElement(hps, 0, sizeof(elem), elem);
    CHECK_EQ_LONG(got, 6, "element 2 body = 6 bytes");
    CHECK_EQ_LONG(elem[0], OC_GLBL, "first byte GLABL");
    {
        LONG label = (LONG)((ULONG)elem[2] | ((ULONG)elem[3] << 8)
                    | ((ULONG)elem[4] << 16) | ((ULONG)elem[5] << 24));
        CHECK_EQ_LONG(label, 20, "label = 20");
    }
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 116);

    /* ============================================================ */
    /* T117 - QueryElement with offset > len -> 0 bytes             */
    /* ============================================================ */
    TEST(117, "T117: QueryElement with offset > len -> 0 bytes");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 117, 1);
    GpiOpenSegment(hps, 117);
    GpiSetElementPointer(hps, 1);
    got = GpiQueryElement(hps, 100, sizeof(elem), elem);
    CHECK_EQ_LONG(got, 0, "past-end offset -> 0 bytes");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 117);

    /* ============================================================ */
    /* T118 - QueryElementType single order -> p[0]                 */
    /* ============================================================ */
    TEST(118, "T118: QueryElementType single order returns p[0]");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 118);
    GpiSetColor(hps, CLR_RED);   /* single GSCOL order */
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 118);
    GpiSetElementPointer(hps, 1);
    lType = 0;
    GpiQueryElementType(hps, &lType, 0, NULL);
    CHECK_EQ_LONG(lType, OC_GSCOL, "type = GSCOL (0x0A)");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 118);

    /* ============================================================ */
    /* T119 - QueryElementType bracket -> 4 bytes after GBEL        */
    /* ============================================================ */
    TEST(119, "T119: QueryElementType bracket returns lType");
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 119);
    GpiBeginElement(hps, 0x81000077L, "x");
    GpiMove(hps, SCRATCH_PT(0, 0));
    GpiEndElement(hps);
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 119);
    GpiSetElementPointer(hps, 1);
    lType = 0;
    GpiQueryElementType(hps, &lType, 0, NULL);
    CHECK_EQ_LONG(lType, 0x81000077L, "bracket type = 0x81000077");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 119);

    /* ============================================================ */
    /* T120 - DeleteElement                                         */
    /* ============================================================ */
    TEST(120, "T120: DeleteElement removes current element");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 120, 3);   /* labels 10, 20, 30 */
    GpiOpenSegment(hps, 120);
    GpiSetElementPointer(hps, 2);
    rc = GpiDeleteElement(hps);
    CHECK_EQ_LONG(rc, 1, "DeleteElement succeeds");
    CHECK_EQ_LONG(GpiQueryElementPointer(hps), 1, "pointer = 1 after delete");

    /* Verify: element 2 is now label 30 */
    GpiSetElementPointer(hps, 2);
    GpiQueryElement(hps, 0, sizeof(elem), elem);
    {
        LONG label = (LONG)((ULONG)elem[2] | ((ULONG)elem[3] << 8)
                    | ((ULONG)elem[4] << 16) | ((ULONG)elem[5] << 24));
        CHECK_EQ_LONG(label, 30, "element 2 = label 30");
    }
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 120);

    /* ============================================================ */
    /* T121 - DeleteElementRange(2, 3)                              */
    /* ============================================================ */
    TEST(121, "T121: DeleteElementRange(2, 3) deletes 2..3");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 121, 4);   /* 10, 20, 30, 40 */
    GpiOpenSegment(hps, 121);
    rc = GpiDeleteElementRange(hps, 2, 3);
    CHECK_EQ_LONG(rc, 1, "DeleteElementRange(2,3) succeeds");
    CHECK_EQ_LONG(GpiQueryElementPointer(hps), 1, "pointer = 1");

    /* Verify: elements = 10, 40 */
    GpiSetElementPointer(hps, 1);
    GpiQueryElement(hps, 0, sizeof(elem), elem);
    {
        LONG label = (LONG)((ULONG)elem[2] | ((ULONG)elem[3] << 8)
                    | ((ULONG)elem[4] << 16) | ((ULONG)elem[5] << 24));
        CHECK_EQ_LONG(label, 10, "element 1 = 10");
    }
    GpiSetElementPointer(hps, 2);
    GpiQueryElement(hps, 0, sizeof(elem), elem);
    {
        LONG label = (LONG)((ULONG)elem[2] | ((ULONG)elem[3] << 8)
                    | ((ULONG)elem[4] << 16) | ((ULONG)elem[5] << 24));
        CHECK_EQ_LONG(label, 40, "element 2 = 40");
    }
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 121);

    /* ============================================================ */
    /* T122 - DeleteElementRange(3, 2) reversed                     */
    /* ============================================================ */
    TEST(122, "T122: DeleteElementRange(3, 2) swaps range");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 122, 4);
    GpiOpenSegment(hps, 122);
    rc = GpiDeleteElementRange(hps, 3, 2);   /* reversed */
    CHECK_EQ_LONG(rc, 1, "DeleteElementRange(3,2) succeeds (swapped)");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 122);

    /* ============================================================ */
    /* T123 - DeleteElementsBetweenLabels(10, 40)                   */
    /* ============================================================ */
    TEST(123, "T123: DeleteElementsBetweenLabels(10, 40)");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 123, 4);   /* 10, 20, 30, 40 */
    GpiOpenSegment(hps, 123);
    rc = GpiDeleteElementsBetweenLabels(hps, 10, 40);
    CHECK_EQ_LONG(rc, 1, "DeleteElementsBetweenLabels(10,40) succeeds");

    /* Verify: only 10, 40 remain */
    GpiSetElementPointer(hps, 1);
    GpiQueryElement(hps, 0, sizeof(elem), elem);
    {
        LONG label = (LONG)((ULONG)elem[2] | ((ULONG)elem[3] << 8)
                    | ((ULONG)elem[4] << 16) | ((ULONG)elem[5] << 24));
        CHECK_EQ_LONG(label, 10, "element 1 = 10");
    }
    GpiSetElementPointer(hps, 2);
    GpiQueryElement(hps, 0, sizeof(elem), elem);
    {
        LONG label = (LONG)((ULONG)elem[2] | ((ULONG)elem[3] << 8)
                    | ((ULONG)elem[4] << 16) | ((ULONG)elem[5] << 24));
        CHECK_EQ_LONG(label, 40, "element 2 = 40");
    }
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 123);

    /* ============================================================ */
    /* T124 - DeleteElementsBetweenLabels with same label -> error  */
    /* ============================================================ */
    TEST(124, "T124: DeleteElementsBetweenLabels(10, 10) -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 124, 3);
    GpiOpenSegment(hps, 124);
    rc = GpiDeleteElementsBetweenLabels(hps, 10, 10);
    CHECK_EQ_LONG(rc, GPI_ERROR, "same label -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 124);

    /* ============================================================ */
    /* T125 - SetElementPointerAtLabel(30)                          */
    /* ============================================================ */
    TEST(125, "T125: SetElementPointerAtLabel(30) positions pointer");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 125, 4);   /* 10, 20, 30, 40 */
    GpiOpenSegment(hps, 125);
    GpiSetElementPointer(hps, 0);
    rc = GpiSetElementPointerAtLabel(hps, 30);
    CHECK_EQ_LONG(rc, 1, "SetElementPointerAtLabel(30) succeeds");
    CHECK_EQ_LONG(GpiQueryElementPointer(hps), 3, "pointer = 3");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 125);

    /* ============================================================ */
    /* T126 - SetElementPointerAtLabel(99) -> error                 */
    /* ============================================================ */
    TEST(126, "T126: SetElementPointerAtLabel(99) -> error");
    GpiSetDrawingMode(hps, DM_RETAIN);
    MakeLabelsSeg(hps, 126, 3);
    GpiOpenSegment(hps, 126);
    rc = GpiSetElementPointerAtLabel(hps, 99);
    CHECK_EQ_LONG(rc, GPI_ERROR, "missing label -> error");
    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 126);

    TestFreePS(hps);
}
