#define INCL_WIN
#ifdef __OS2__
#include "F_platform.h"
#else
#include <os2win.h>
#endif
#include <stdio.h>
#include <string.h>

int main(void)
{
    /* C89: все объ€влени€ в начале функции */
    HAB    hab;
    HMQ    hmq;
    HPS    hps;
    POINTL ptl1, ptl2;
    RECTL  rcl;
    
    BOOL   rc_set_color1;
    BOOL   rc_move1;
    LONG   rc_line;
    BOOL   rc_set_color2;
    BOOL   rc_move2;
    LONG   rc_box;
    BOOL   rc_rel_ps;
    BOOL   rc_destroy_mq;
    BOOL   rc_term;

    printf("--- testgpi START ---\n");

    printf("Calling WinInitialize(0)\n");
    hab = WinInitialize(0);
    printf("WinInitialize -> %p\n", (void*)hab);
    if (!hab) return 1;

    /* WinGetScreenPS требует наличи€ очереди сообщений */
    printf("Calling WinCreateMsgQueue(hab=%p, 0)\n", (void*)hab);
    hmq = WinCreateMsgQueue(hab, 0);
    printf("WinCreateMsgQueue -> %p\n", (void*)hmq);
    if (!hmq) { WinTerminate(hab); return 1; }

    printf("Calling WinGetScreenPS(HWND_DESKTOP)\n");
    hps = WinGetScreenPS(HWND_DESKTOP);
    printf("WinGetScreenPS -> %p\n", (void*)hps);
    if (!hps) {
        WinDestroyMsgQueue(hmq);
        WinTerminate(hab);
        return 1;
    }

    /*  расна€ лини€ */
    ptl1.x = 100; ptl1.y = 100;
    ptl2.x = 400; ptl2.y = 300;

    printf("Calling GpiSetColor(hps=%p, CLR_RED=%ld)\n", (void*)hps, (long)CLR_RED);
    rc_set_color1 = GpiSetColor(hps, CLR_RED);
    printf("GpiSetColor -> %d\n", (int)rc_set_color1);

    printf("Calling GpiMove(hps=%p, ptl1={%ld, %ld})\n", (void*)hps, (long)ptl1.x, (long)ptl1.y);
    rc_move1 = GpiMove(hps, &ptl1);
    printf("GpiMove -> %d\n", (int)rc_move1);

    printf("Calling GpiLine(hps=%p, ptl2={%ld, %ld})\n", (void*)hps, (long)ptl2.x, (long)ptl2.y);
    rc_line = GpiLine(hps, &ptl2);
    printf("GpiLine -> %ld\n", (long)rc_line);

    /* —иний пр€моугольник Ч документально корректное использование GpiBox */
    rcl.xLeft   = 150;
    rcl.yBottom = 150;
    rcl.xRight  = 350;
    rcl.yTop    = 250;

    printf("Calling GpiSetColor(hps=%p, CLR_BLUE=%ld)\n", (void*)hps, (long)CLR_BLUE);
    rc_set_color2 = GpiSetColor(hps, CLR_BLUE);
    printf("GpiSetColor -> %d\n", (int)rc_set_color2);

    printf("Calling GpiMove(hps=%p, origin={%ld, %ld})\n", (void*)hps, (long)rcl.xLeft, (long)rcl.yBottom);
    rc_move2 = GpiMove(hps, (PPOINTL)&rcl.xLeft);   /* текуща€ позици€ = левый-нижний угол */
    printf("GpiMove -> %d\n", (int)rc_move2);

    ptl2.x = rcl.xRight;
    ptl2.y = rcl.yTop;

    printf("Calling GpiBox(hps=%p, DRO_OUTLINE, ptl2={%ld, %ld}, 0, 0)\n", (void*)hps, (long)ptl2.x, (long)ptl2.y);
    rc_box = GpiBox(hps, DRO_OUTLINE, &ptl2, 0, 0); /* противоположный угол */
    printf("GpiBox -> %ld\n", (long)rc_box);

    printf("Calling GpiSetDrawingMode(hps=%p, DM_RETAIN=%ld)\n",
           (void*)hps, (long)DM_RETAIN);
    GpiSetDrawingMode(hps, DM_RETAIN);

    printf("Calling GpiOpenSegment(hps=%p, 1)\n", (void*)hps);
    GpiOpenSegment(hps, 1);

    printf("Calling GpiSetColor(hps=%p, CLR_RED=%ld) [in segment]\n",
           (void*)hps, (long)CLR_RED);
    GpiSetColor(hps, CLR_RED);

    printf("Calling GpiMove(hps=%p, ptl1={%ld, %ld}) [in segment]\n",
           (void*)hps, (long)ptl1.x, (long)ptl1.y);
    GpiMove(hps, &ptl1);

    printf("Calling GpiLine(hps=%p, ptl2={%ld, %ld}) [in segment]\n",
           (void*)hps, (long)ptl2.x, (long)ptl2.y);
    GpiLine(hps, &ptl2);

    printf("Calling GpiCloseSegment(hps=%p)\n", (void*)hps);
    GpiCloseSegment(hps);

    printf("Calling GpiSetDrawingMode(hps=%p, DM_DRAW=%ld)\n",
           (void*)hps, (long)DM_DRAW);
    GpiSetDrawingMode(hps, DM_DRAW);

    printf("Calling GpiDrawSegment(hps=%p, 1)\n", (void*)hps);
    GpiDrawSegment(hps, 1);


{
    LONG segids[4] = {0};
    LONG n;

    printf("Calling GpiQuerySegmentNames(hps=%p, 1, 10, 4)\n", (void*)hps);
    n = GpiQuerySegmentNames(hps, 1, 10, 4, segids);
    printf("GpiQuerySegmentNames -> %ld (ids: %ld, %ld, %ld, %ld)\n",
           (long)n, (long)segids[0], (long)segids[1],
           (long)segids[2], (long)segids[3]);

    printf("Calling GpiQuerySegmentAttrs(hps=%p, 1, ATTR_VISIBLE)\n", (void*)hps);
    printf("GpiQuerySegmentAttrs -> %ld (ATTR_ON=%ld)\n",
           (long)GpiQuerySegmentAttrs(hps, 1, ATTR_VISIBLE),
           (long)ATTR_ON);
}

{
    /* SetSegmentAttrs / QuerySegmentAttrs на не-дефолтный атрибут */
    GpiSetSegmentAttrs(hps, 1, ATTR_DYNAMIC, ATTR_ON);
    printf("GpiQuerySegmentAttrs(1, ATTR_DYNAMIC) -> %ld\n",
           (long)GpiQuerySegmentAttrs(hps, 1, ATTR_DYNAMIC));
    GpiSetSegmentAttrs(hps, 1, ATTR_DYNAMIC, ATTR_OFF);

    /* GetData/PutData round-trip */
{
    BYTE  goca[64];
    LONG  offset = 0, n, n2;
    BOOL  rc;

    n = GpiGetData(hps, 1, &offset, 0, sizeof(goca), goca);
    printf("GpiGetData(seg 1) -> %ld bytes, offset=%ld\n",
           (long)n, (long)offset);

    GpiSetDrawingMode(hps, DM_RETAIN);
    rc = GpiOpenSegment(hps, 2);
    printf("GpiOpenSegment(2) -> %d\n", (int)rc);

    n2 = GpiPutData(hps, 0, &n, goca);
    printf("GpiPutData(seg 2) -> %ld (n=%ld)\n", (long)n2, (long)n);

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    printf("GpiDeleteSegment(2) -> %d\n", (int)GpiDeleteSegment(hps, 2));
}

    /* DeleteSegment */
    GpiDeleteSegment(hps, 2);
}

{
    /* Step E: DrawChain / DrawDynamics / RemoveDynamics / DrawFrom */
    LONG rc;
    BOOL open_rc;

    printf("=== Step E test: DrawChain / DrawDynamics ===\n");

    /* Segment 1: ensure chained and non-dynamic */
    GpiSetSegmentAttrs(hps, 1, ATTR_CHAINED, ATTR_ON);
    GpiSetSegmentAttrs(hps, 1, ATTR_DYNAMIC, ATTR_OFF);

    /* Create segment 3 (dynamic) */
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 3);
    GpiSetColor(hps, CLR_GREEN);
    GpiMove(hps, &ptl1);
    GpiLine(hps, &ptl2);
    GpiCloseSegment(hps);
    GpiSetSegmentAttrs(hps, 3, ATTR_DYNAMIC, ATTR_ON);
    GpiSetDrawingMode(hps, DM_DRAW);

    printf("GpiDrawChain(hps=%p) - expect only seg 1 in [GOCA]\n", (void*)hps);
    rc = GpiDrawChain(hps);
    printf("GpiDrawChain -> %d\n", (int)rc);

    printf("GpiDrawDynamics(hps=%p) - expect only seg 3 with GSMX/GSBMX XOR/LEAVEALONE\n",
           (void*)hps);
    rc = GpiDrawDynamics(hps);
    printf("GpiDrawDynamics -> %d\n", (int)rc);

    printf("GpiRemoveDynamics(hps=%p, 1, 10) - expect only seg 3, same mix\n",
           (void*)hps);
    rc = GpiRemoveDynamics(hps, 1, 10);
    printf("GpiRemoveDynamics -> %d\n", (int)rc);

    /* Step E: DrawFrom (positive range, dynamic filtered out, empty range) */
    printf("=== Step E test: DrawFrom ===\n");

    printf("GpiDrawFrom(hps=%p, 1, 1) - expect only seg 1\n", (void*)hps);
    rc = GpiDrawFrom(hps, 1, 1);
    printf("GpiDrawFrom(1,1) -> %d\n", (int)rc);

    printf("GpiDrawFrom(hps=%p, 2, 10) - expect empty (seg 1 out of range, seg 3 dynamic)\n",
           (void*)hps);
    rc = GpiDrawFrom(hps, 2, 10);
    printf("GpiDrawFrom(2,10) -> %d\n", (int)rc);

    printf("GpiDrawFrom(hps=%p, 1, 10) - expect only seg 1\n", (void*)hps);
    rc = GpiDrawFrom(hps, 1, 10);
    printf("GpiDrawFrom(1,10) -> %d\n", (int)rc);

    /* Step E: open-segment guard (Draw* must refuse while a segment is open) */
    printf("=== Step E test: open-segment guard ===\n");

    GpiSetDrawingMode(hps, DM_RETAIN);
    open_rc = GpiOpenSegment(hps, 4);
    printf("GpiOpenSegment(4) -> %d\n", (int)open_rc);

    printf("GpiDrawChain(hps=%p) - expect GPI_ERROR (seg 4 open)\n", (void*)hps);
    rc = GpiDrawChain(hps);
    printf("GpiDrawChain (open seg) -> %d (expect GPI_ERROR)\n", (int)rc);

    GpiCloseSegment(hps);
    GpiDeleteSegment(hps, 4);
    GpiSetDrawingMode(hps, DM_DRAW);

    /* Step E: invisible segment filtered out of DrawChain */
    printf("=== Step E test: invisible segment filtered ===\n");

    GpiSetSegmentAttrs(hps, 1, ATTR_VISIBLE, ATTR_OFF);

    printf("GpiDrawChain(hps=%p) - expect empty (seg 1 invisible)\n", (void*)hps);
    rc = GpiDrawChain(hps);
    printf("GpiDrawChain (invisible) -> %d\n", (int)rc);

    GpiSetSegmentAttrs(hps, 1, ATTR_VISIBLE, ATTR_ON);

    GpiDeleteSegment(hps, 3);
}

{
    LONG prio, ids[8] = {0};

    printf("=== Step F test: Segment Priority ===\n");

    /* Isolate: remove seg 1 left over from Step E so chain starts empty. */
    GpiDeleteSegment(hps, 1);

    /* Build 3 chained non-dynamic segments: 10, 20, 30 */
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 10); GpiSetColor(hps, CLR_RED);
    GpiMove(hps, &ptl1); GpiLine(hps, &ptl2); GpiCloseSegment(hps);
    GpiOpenSegment(hps, 20); GpiSetColor(hps, CLR_GREEN);
    GpiMove(hps, &ptl1); GpiLine(hps, &ptl2); GpiCloseSegment(hps);
    GpiOpenSegment(hps, 30); GpiSetColor(hps, CLR_BLUE);
    GpiMove(hps, &ptl1); GpiLine(hps, &ptl2); GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiSetSegmentAttrs(hps, 10, ATTR_CHAINED, ATTR_ON);
    GpiSetSegmentAttrs(hps, 20, ATTR_CHAINED, ATTR_ON);
    GpiSetSegmentAttrs(hps, 30, ATTR_CHAINED, ATTR_ON);
    GpiSetSegmentAttrs(hps, 10, ATTR_DYNAMIC, ATTR_OFF);
    GpiSetSegmentAttrs(hps, 20, ATTR_DYNAMIC, ATTR_OFF);
    GpiSetSegmentAttrs(hps, 30, ATTR_DYNAMIC, ATTR_OFF);

    /* Query: 0/HIGHER -> 30, 0/LOWER -> 10 */
    prio = GpiQuerySegmentPriority(hps, 0, HIGHER_PRI);
    printf("GpiQuerySegmentPriority(0, HIGHER) -> %ld (expect 30)\n", (long)prio);
    prio = GpiQuerySegmentPriority(hps, 0, LOWER_PRI);
    printf("GpiQuerySegmentPriority(0, LOWER)  -> %ld (expect 10)\n", (long)prio);
    prio = GpiQuerySegmentPriority(hps, 20, HIGHER_PRI);
    printf("GpiQuerySegmentPriority(20, HIGHER)-> %ld (expect 30)\n", (long)prio);
    prio = GpiQuerySegmentPriority(hps, 20, LOWER_PRI);
    printf("GpiQuerySegmentPriority(20, LOWER) -> %ld (expect 10)\n", (long)prio);
    prio = GpiQuerySegmentPriority(hps, 30, HIGHER_PRI);
    printf("GpiQuerySegmentPriority(30, HIGHER)-> %ld (expect 0)\n", (long)prio);

    /* Move 30 to lowest priority (before 10) */
    printf("GpiSetSegmentPriority(30, 10, LOWER_PRI) -> %d\n",
           (int)GpiSetSegmentPriority(hps, 30, 10, LOWER_PRI));
    prio = GpiQuerySegmentPriority(hps, 0, LOWER_PRI);
    printf("Lowest after move -> %ld (expect 30)\n", (long)prio);

    /* Move 30 to highest priority (after end) */
    printf("GpiSetSegmentPriority(30, 0, HIGHER_PRI) -> %d\n",
           (int)GpiSetSegmentPriority(hps, 30, 0, HIGHER_PRI));
    prio = GpiQuerySegmentPriority(hps, 0, HIGHER_PRI);
    printf("Highest after move -> %ld (expect 30)\n", (long)prio);

    GpiDeleteSegment(hps, 10);
    GpiDeleteSegment(hps, 20);
    GpiDeleteSegment(hps, 30);
}

{
    printf("=== Step G test: DeleteSegments range ===\n");
    /* Create segs 100..102 */
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 100); GpiCloseSegment(hps);
    GpiOpenSegment(hps, 101); GpiCloseSegment(hps);
    GpiOpenSegment(hps, 102); GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    /* Delete 101..101 (only middle) */
    printf("GpiDeleteSegments(101,101) -> %d\n",
           (int)GpiDeleteSegments(hps, 101, 101));

    /* Verify: names should be 100, 102 */
    {
        LONG ids[8] = {0}, n = GpiQuerySegmentNames(hps, 0, 999, 8, ids);
        printf("Remaining: n=%ld ids=%ld,%ld,%ld\n",
               (long)n, (long)ids[0], (long)ids[1], (long)ids[2]);
    }
    GpiDeleteSegment(hps, 100);
    GpiDeleteSegment(hps, 102);
}


/* ==================================================================== */
/*  Step H: Element editing                                             */
/* ==================================================================== */

{
    /* H1: BeginElement / EndElement / QueryElementType / QueryElement */
    LONG lType, cbReq, got, label;
    BYTE elem[64];
    char szTri[] = "Tri";
    LONG i;

    printf("\n=== Step H1: BeginElement / EndElement ===\n");

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 200);

    GpiLabel(hps, 10);                               /* elem 1: GLBL(10) */
    GpiBeginElement(hps, 0x81000001L, szTri);        /* elem 2 start     */
      GpiMove(hps, &ptl1);
      GpiLine(hps, &ptl2);
    GpiEndElement(hps);                              /* elem 2 end       */
    GpiLabel(hps, 20);                               /* elem 3: GLBL(20) */
    GpiCloseSegment(hps);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 200);

    for (i = 1; i <= 3; i++) {
        GpiSetElementPointer(hps, i);
        lType = 0;
        cbReq = GpiQueryElementType(hps, &lType, 0, 0);
        memset(elem, 0, sizeof(elem));
        got = GpiQueryElement(hps, 0, sizeof(elem), elem);
        label = -1;
        if (got == 6 && elem[0] == 0xD3) {
            label = (LONG)elem[2] | ((LONG)elem[3] << 8)
                  | ((LONG)elem[4] << 16) | ((LONG)elem[5] << 24);
        }
        printf("  Elem %ld: type=0x%lx cbReq=%ld got=%ld label=%ld body0=0x%02x\n",
               (long)i, lType, cbReq, got, label, elem[0]);
    }
    printf("  Expect: elem1 type=0xD3 label=10;"
           " elem2 type=0x81000001 body0=0x21; elem3 type=0xD3 label=20\n");

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 200);
}

{
    /* H2: Label navigation via SetElementPointerAtLabel + Offset */
    LONG ptr;
    BOOL rc;

    printf("\n=== Step H2: Label navigation ===\n");

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 201);
    GpiLabel(hps, 10);      /* elem 1 */
    GpiLabel(hps, 20);      /* elem 2 */
    GpiLabel(hps, 30);      /* elem 3 */
    GpiLabel(hps, 40);      /* elem 4 */
    GpiCloseSegment(hps);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 201);

    GpiSetElementPointer(hps, 0);
    ptr = GpiQueryElementPointer(hps);
    printf("  ptr=0: %ld\n", ptr);

    rc = GpiSetElementPointerAtLabel(hps, 30);
    ptr = GpiQueryElementPointer(hps);
    printf("  after SetElementPointerAtLabel(30): ptr=%ld rc=%d (expect 3, 1)\n",
           ptr, (int)rc);

    GpiOffsetElementPointer(hps, 1);
    ptr = GpiQueryElementPointer(hps);
    printf("  offset+1: ptr=%ld (expect 4)\n", ptr);

    rc = GpiSetElementPointerAtLabel(hps, 999);
    printf("  SetElementPointerAtLabel(999) -> rc=%d (expect 0)\n", (int)rc);

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 201);
}

{
    /* H3: GpiElement */
    BYTE body[32];
    BYTE elem[32];
    LONG lType, cbReq, got;
    char szBlue[] = "Blue";

    printf("\n=== Step H3: GpiElement ===\n");

    /* body = GSCOL(0x0A,1) + GCLINE(0x81, 8, x=100,y=100) = 12 bytes */
    body[0]  = 0x0A; body[1]  = 0x01;
    body[2]  = 0x81; body[3]  = 0x08;
    body[4]  = 100; body[5]  = 0; body[6]  = 0; body[7]  = 0;
    body[8]  = 100; body[9]  = 0; body[10] = 0; body[11] = 0;

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 202);
    GpiElement(hps, 0x81000002L, szBlue, 12, body);
    GpiCloseSegment(hps);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 202);

    GpiSetElementPointer(hps, 1);
    lType = 0;
    cbReq = GpiQueryElementType(hps, &lType, 0, 0);
    memset(elem, 0, sizeof(elem));
    got = GpiQueryElement(hps, 0, sizeof(elem), elem);
    printf("  Elem 1: type=0x%lx cbReq=%ld got=%ld"
           " body0=0x%02x body2=0x%02x\n",
           lType, cbReq, got, elem[0], elem[2]);
    printf("  Expect: type=0x81000002 cbReq=12 got=12 body0=0x0A body2=0x81\n");

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 202);
}

{
    /* H4: INSERT vs REPLACE */
    LONG lType, cbReq, got, label, ptr;
    BYTE elem[16];
    LONG i;

    printf("\n=== Step H4: SEGEM_INSERT vs SEGEM_REPLACE ===\n");

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 203);
    GpiLabel(hps, 10);
    GpiLabel(hps, 20);
    GpiLabel(hps, 30);
    GpiCloseSegment(hps);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 203);

    /* INSERT after elem 1 -> ptr becomes 2 */
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiSetElementPointer(hps, 1);
    GpiLabel(hps, 15);
    ptr = GpiQueryElementPointer(hps);
    printf("  INSERT at ptr 1: ptr=%ld (expect 2)\n", ptr);

    /* REPLACE elem 2 (was 15) -> ptr stays 2 */
    GpiSetEditMode(hps, SEGEM_REPLACE);
    GpiSetElementPointer(hps, 2);
    GpiLabel(hps, 25);
    ptr = GpiQueryElementPointer(hps);
    printf("  REPLACE at ptr 2: ptr=%ld (expect 2)\n", ptr);

    /* Verify: expect labels 10, 25, 20, 30 */
    GpiSetElementPointer(hps, 1);
    for (i = 1; i <= 4; i++) {
        lType = 0;
        cbReq = GpiQueryElementType(hps, &lType, 0, 0);
        memset(elem, 0, sizeof(elem));
        got = GpiQueryElement(hps, 0, sizeof(elem), elem);
        label = -1;
        if (got == 6 && elem[0] == 0xD3) {
            label = (LONG)elem[2] | ((LONG)elem[3] << 8)
                  | ((LONG)elem[4] << 16) | ((LONG)elem[5] << 24);
        }
        printf("  Elem %ld: label=%ld\n", (long)i, label);
        GpiOffsetElementPointer(hps, 1);
    }
    printf("  Expect: 10, 25, 20, 30\n");

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 203);
}

{
    /* H5: DeleteElement */
    LONG ptr;
    BOOL rc;

    printf("\n=== Step H5: DeleteElement ===\n");

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 204);
    GpiLabel(hps, 10);   /* elem 1 */
    GpiLabel(hps, 20);   /* elem 2 */
    GpiLabel(hps, 30);   /* elem 3 */
    GpiCloseSegment(hps);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 204);

    GpiSetElementPointer(hps, 2);
    rc = GpiDeleteElement(hps);
    ptr = GpiQueryElementPointer(hps);
    printf("  DeleteElement(2): rc=%d ptr=%ld (expect 1, 1)\n",
           (int)rc, ptr);

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 204);
}

{
    /* H6: DeleteElementRange */
    LONG lType, cbReq, got, label;
    BYTE elem[16];
    BOOL rc;
    LONG i;

    printf("\n=== Step H6: DeleteElementRange ===\n");

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 205);
    GpiLabel(hps, 10);   /* elem 1 */
    GpiLabel(hps, 20);   /* elem 2 */
    GpiLabel(hps, 30);   /* elem 3 */
    GpiLabel(hps, 40);   /* elem 4 */
    GpiCloseSegment(hps);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 205);

    rc = GpiDeleteElementRange(hps, 2, 3);
    printf("  DeleteElementRange(2,3): rc=%d\n", (int)rc);

    /* Expect remaining labels: 10, 40 */
    GpiSetElementPointer(hps, 1);
    for (i = 1; i <= 2; i++) {
        lType = 0;
        cbReq = GpiQueryElementType(hps, &lType, 0, 0);
        memset(elem, 0, sizeof(elem));
        got = GpiQueryElement(hps, 0, sizeof(elem), elem);
        label = -1;
        if (got == 6 && elem[0] == 0xD3) {
            label = (LONG)elem[2] | ((LONG)elem[3] << 8)
                  | ((LONG)elem[4] << 16) | ((LONG)elem[5] << 24);
        }
        printf("  Elem %ld: label=%ld\n", (long)i, label);
        GpiOffsetElementPointer(hps, 1);
    }
    printf("  Expect: 10, 40\n");

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 205);
}

{
    /* H7: DeleteElementsBetweenLabels */
    LONG lType, cbReq, got, label;
    BYTE elem[16];
    BOOL rc;
    LONG i;

    printf("\n=== Step H7: DeleteElementsBetweenLabels ===\n");

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 206);
    GpiLabel(hps, 10);   /* elem 1 (boundary) */
    GpiLabel(hps, 20);   /* elem 2 (deleted) */
    GpiLabel(hps, 30);   /* elem 3 (deleted) */
    GpiLabel(hps, 40);   /* elem 4 (boundary) */
    GpiCloseSegment(hps);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 206);

    rc = GpiDeleteElementsBetweenLabels(hps, 10, 40);
    printf("  DeleteElementsBetweenLabels(10,40): rc=%d\n", (int)rc);

    /* Expect remaining labels: 10, 40 */
    GpiSetElementPointer(hps, 1);
    for (i = 1; i <= 2; i++) {
        lType = 0;
        cbReq = GpiQueryElementType(hps, &lType, 0, 0);
        memset(elem, 0, sizeof(elem));
        got = GpiQueryElement(hps, 0, sizeof(elem), elem);
        label = -1;
        if (got == 6 && elem[0] == 0xD3) {
            label = (LONG)elem[2] | ((LONG)elem[3] << 8)
                  | ((LONG)elem[4] << 16) | ((LONG)elem[5] << 24);
        }
        printf("  Elem %ld: label=%ld\n", (long)i, label);
        GpiOffsetElementPointer(hps, 1);
    }
    printf("  Expect: 10, 40\n");

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 206);
}

{
    /* H8: pointer clamping */
    LONG ptr;

    printf("\n=== Step H8: pointer clamping ===\n");

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 207);
    GpiLabel(hps, 10);
    GpiLabel(hps, 20);
    GpiLabel(hps, 30);
    GpiCloseSegment(hps);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 207);

    GpiSetElementPointer(hps, 100);
    ptr = GpiQueryElementPointer(hps);
    printf("  SetPointer(100): ptr=%ld (expect 3)\n", ptr);

    GpiSetElementPointer(hps, -5);
    ptr = GpiQueryElementPointer(hps);
    printf("  SetPointer(-5): ptr=%ld (expect 0)\n", ptr);

    GpiSetElementPointer(hps, 1);
    GpiOffsetElementPointer(hps, 100);
    ptr = GpiQueryElementPointer(hps);
    printf("  Offset(+100) from 1: ptr=%ld (expect 3)\n", ptr);

    GpiOffsetElementPointer(hps, -100);
    ptr = GpiQueryElementPointer(hps);
    printf("  Offset(-100): ptr=%ld (expect 0)\n", ptr);

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 207);
}

{
    /* H9: CloseSegment with open element bracket -> implicit GEEL */
    LONG lType, cbReq;
    BOOL rc;
    char szOpen[] = "Open";

    printf("\n=== Step H9: CloseSegment with open bracket ===\n");

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 208);
    GpiBeginElement(hps, 0x81000005L, szOpen);
    GpiMove(hps, &ptl1);
    /* Intentionally do NOT call GpiEndElement */
    rc = GpiCloseSegment(hps);
    printf("  CloseSegment (bracket open): rc=%d (expect 1)\n", (int)rc);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 208);
    GpiSetElementPointer(hps, 1);
    lType = 0;
    cbReq = GpiQueryElementType(hps, &lType, 0, 0);
    printf("  Elem 1: type=0x%lx cbReq=%ld (expect 0x81000005, 10=GSCP)\n",
           lType, cbReq);

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 208);
}

{
    /* H10: DrawSegment with mixed bracket content - visual */
    BOOL rc;
    char szBox[] = "Box";

    printf("\n=== Step H10: DrawSegment with bracket ===\n");

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 209);
    GpiSetColor(hps, CLR_RED);
    GpiBeginElement(hps, 0x81000006L, szBox);
    GpiMove(hps, &ptl1);
    GpiBox(hps, DRO_OUTLINE, &ptl2, 0, 0);
    GpiEndElement(hps);
    GpiLabel(hps, 999);
    GpiCloseSegment(hps);

    GpiSetDrawingMode(hps, DM_DRAW);
    printf("  DrawSegment(209) - expect [GOCA] GSCOL/GSCP/GCBOX\n");
    rc = GpiDrawSegment(hps, 209);
    printf("  DrawSegment: rc=%d\n", (int)rc);

    GpiDeleteSegment(hps, 209);
}

{
    /* H11: SetEditMode is invalid inside an element bracket */
    BOOL   rc_begin, rc_mode, rc_end, rc_close;
    char   szH11[] = "H11";

    printf("\n=== Step H11: SetEditMode invalid in element bracket ===\n");

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 210);
    rc_begin = GpiBeginElement(hps, 0x81000010L, szH11);
    rc_mode  = GpiSetEditMode(hps, SEGEM_REPLACE);
    rc_end   = GpiEndElement(hps);
    rc_close = GpiCloseSegment(hps);

    printf("  BeginElement -> %d\n", (int)rc_begin);
    printf("  SetEditMode(REPLACE) inside bracket -> %d (expect 0)\n",
           (int)rc_mode);
    printf("  EndElement -> %d\n", (int)rc_end);
    printf("  CloseSegment -> %d\n", (int)rc_close);

    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 210);
}

{
    /* H12: PutData is invalid in SEGEM_REPLACE */
    BYTE   body[10];
    LONG   count;
    BOOL   rc_mode, rc_put, rc_close;

    printf("\n=== Step H12: PutData invalid in SEGEM_REPLACE ===\n");

    /* GCLINE with one endpoint = 10 bytes. */
    body[0] = 0x81; body[1] = 0x08;
    body[2] = 100; body[3] = 0; body[4] = 0; body[5] = 0;
    body[6] = 100; body[7] = 0; body[8] = 0; body[9] = 0;

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 211);
    rc_mode = GpiSetEditMode(hps, SEGEM_REPLACE);
    count   = 10;
    rc_put  = GpiPutData(hps, 0, &count, body);
    GpiSetEditMode(hps, SEGEM_INSERT);   /* restore before close */
    rc_close = GpiCloseSegment(hps);

    printf("  SetEditMode(REPLACE) -> %d\n", (int)rc_mode);
    printf("  PutData in REPLACE -> %d (expect 0)\n", (int)rc_put);
    printf("  CloseSegment -> %d\n", (int)rc_close);

    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 211);
}

{
    /* H13: PutData requires the pointer at the last element */
    BYTE   body[10];
    LONG   count, ptr;
    BOOL   rc_put_zero, rc_put_last;

    printf("\n=== Step H13: PutData requires pointer at last element ===\n");

    body[0] = 0x81; body[1] = 0x08;
    body[2] = 100; body[3] = 0; body[4] = 0; body[5] = 0;
    body[6] = 100; body[7] = 0; body[8] = 0; body[9] = 0;

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiSetEditMode(hps, SEGEM_INSERT);
    GpiOpenSegment(hps, 212);
    GpiLabel(hps, 10);   /* elem 1 */
    GpiLabel(hps, 20);   /* elem 2 */
    GpiCloseSegment(hps);

    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, 212);

    GpiSetElementPointer(hps, 0);
    ptr = GpiQueryElementPointer(hps);
    count = 10;
    rc_put_zero = GpiPutData(hps, 0, &count, body);
    printf("  SetElementPointer(0) -> ptr=%ld\n", ptr);
    printf("  PutData with ptr=0 -> %d (expect 0)\n", (int)rc_put_zero);

    GpiSetElementPointer(hps, 2);
    ptr = GpiQueryElementPointer(hps);
    count = 10;
    rc_put_last = GpiPutData(hps, 0, &count, body);
    printf("  SetElementPointer(2) -> ptr=%ld\n", ptr);
    printf("  PutData with ptr=N -> %d (expect 1)\n", (int)rc_put_last);

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);
    GpiDeleteSegment(hps, 212);
}

    printf("Calling WinReleasePS(hps=%p)\n", (void*)hps);
    rc_rel_ps = WinReleasePS(hps); /* об€зательное освобождение */
    printf("WinReleasePS -> %d\n", (int)rc_rel_ps);

    printf("Calling WinDestroyMsgQueue(hmq=%p)\n", (void*)hmq);
    rc_destroy_mq = WinDestroyMsgQueue(hmq); /* очистка очереди перед завершением */
    printf("WinDestroyMsgQueue -> %d\n", (int)rc_destroy_mq);

    printf("Calling WinTerminate(hab=%p)\n", (void*)hab);
    rc_term = WinTerminate(hab);
    printf("WinTerminate -> %d\n", (int)rc_term);

    printf("--- testgpi END ---\n");
    return 0;
}
