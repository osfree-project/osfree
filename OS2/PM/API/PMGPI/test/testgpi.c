#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#ifdef __OS2__
#include <os2.h>
#else
#include <os2win.h>
#endif

int main(void)
{
    HAB    hab;
    HMQ    hmq;
    HPS    hps;
    POINTL ptl1, ptl2;
    RECTL  rcl;

    hab = WinInitialize(0);
    if (!hab) return 1;

    /* WinGetScreenPS требует наличия очереди сообщений */
    hmq = WinCreateMsgQueue(hab, 0);
    if (!hmq) { WinTerminate(hab); return 1; }

    hps = WinGetScreenPS(HWND_DESKTOP);
    if (!hps) {
        WinDestroyMsgQueue(hmq);
        WinTerminate(hab);
        return 1;
    }

    /* Красная линия */
    ptl1.x = 100; ptl1.y = 100;
    ptl2.x = 400; ptl2.y = 300;
    GpiSetColor(hps, CLR_RED);
    GpiMove(hps, &ptl1);
    GpiLine(hps, &ptl2);

    /* Синий прямоугольник — документально корректное использование GpiBox */
    rcl.xLeft   = 150;
    rcl.yBottom = 150;
    rcl.xRight  = 350;
    rcl.yTop    = 250;

    GpiSetColor(hps, CLR_BLUE);
    GpiMove(hps, (PPOINTL)&rcl.xLeft);   /* текущая позиция = левый-нижний угол */
    ptl2.x = rcl.xRight;
    ptl2.y = rcl.yTop;
    GpiBox(hps, DRO_OUTLINE, &ptl2, 0, 0); /* противоположный угол */

    WinReleasePS(hps); /* обязательное освобождение */

    WinDestroyMsgQueue(hmq); /* очистка очереди перед завершением */
    WinTerminate(hab);
    return 0;
}
