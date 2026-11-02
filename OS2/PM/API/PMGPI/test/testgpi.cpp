#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#ifdef __OS2__
#include <os2.h>
#else
#include <os2win.h>
#endif
#include <stdio.h>

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
