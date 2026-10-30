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
