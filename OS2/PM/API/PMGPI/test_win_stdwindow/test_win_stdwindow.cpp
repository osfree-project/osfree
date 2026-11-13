/*!
 * @file test_win_stdwindow.cpp
 * @brief WinCreateStdWindow + WinWindowFromID + WinSetWindowPos +
 *        WM_SIZE + WM_PAINT + Gpi* рисование.
 *
 * Проверяет:
 *   1. WinCreateStdWindow -> hwndFrame + hwndClient (оба != NULLHANDLE).
 *   2. WinWindowFromID(hwndFrame, FID_TITLEBAR) -> titlebar.
 *   3. WinWindowFromID(hwndFrame, FID_CLIENT) == hwndClient из п.1.
 *   4. WinSetWindowPos(SWP_MOVE|SWP_SIZE) -> WM_SIZE -> FrameWndProc.
 *   5. WM_PAINT для frame + titlebar -> FrameWndProc/TitleBarWndProc
 *      -> EnsureDC -> GreBoxBoundary/BoxInterior/PolyLine -> GRE2SDL.
 *   6. WinBeginPaint(client) + GpiSetColor + GpiLine + GpiBox +
 *      WinEndPaint -> Gpi* -> GocaEmit -> SrvDeliver ->
 *      GocaParseStream -> GrePolyLine/GreBox* -> GRE2SDL.
 *   7. WinCreateStdWindow с FCF_SHELLPOSITION -> NULLHANDLE.
 *
 * Требует HMQ (WinCreateMsgQueue): WinGetMsg блокирующий.
 *
 * Вывод на экран возможен только после реализации примитивов в
 * GRE2SDL (см. отдельный патч). Без них все Gre* — заглушки, тест
 * пройдёт по API-контракту, но пиксели не появятся.
 */

#define INCL_PM
#include "F_platform.h"
#include "F_cmd.h"
#include "F_debug.h"

#include <stdio.h>
#include <string.h>

/* ---------- Test harness ---------- */

/** @brief Счётчик успешных проверок. */
static int g_pass = 0;

/** @brief Счётчик проваленных проверок. */
static int g_fail = 0;

/**
 * @brief Проверить условие, напечатать PASS/FAIL, обновить счётчики.
 * @param [in] expr  Условие.
 * @param [in] fmt   printf-формат сообщения.
 * @param [in] ...   Аргументы формата.
 */
#define CHECK(expr, fmt, ...) do {                                   \
    if (expr) {                                                       \
        g_pass++;                                                     \
        printf("[PASS] " fmt "\n", ##__VA_ARGS__);                    \
    } else {                                                          \
        g_fail++;                                                     \
        printf("[FAIL] " fmt "  (%s:%d)\n", ##__VA_ARGS__,            \
               __FILE__, __LINE__);                                   \
    }                                                                 \
} while (0)

/* ---------- Helpers ---------- */

/** @brief Sentinel для PumpQueue: постится в очередь, останавливает drain. */
#define TEST_PUMP_SENTINEL  (0x0401UL)  /* WM_USER + 1 */

/**
 * @brief Вычитать очередь до sentinel.
 *
 * WinGetMsg блокирующий: без знания числа сообщений «в лоб» цикл
 * зависнет на первом Get при пустой очереди.  Поэтому сначала
 * постим TEST_PUMP_SENTINEL в очередь вызывающего потока, затем
 * крутим WinGetMsg до встречи с ним.  Все прочие сообщения
 * диспатчатся.
 *
 * @param [in] hab      Anchor block (должен иметь активный HMQ).
 * @param [in] maxMsgs  Предохранитель на число диспатченных.
 * @return Число диспатченных (не-sentinel) сообщений; -1 при
 *         транспортной ошибке / отсутствии HMQ.
 * @retval -1  PostMsg sentinel не удался.
 * @retval  0  Очередь была пуста, только sentinel.
 * @retval >0  Число обработанных сообщений.
 */
static int PumpQueue(HAB hab, int maxMsgs)
{
    QMSG qmsg;
    int  n = 0;

    if (!WinPostMsg(NULLHANDLE, TEST_PUMP_SENTINEL, 0, 0)) return -1;

    while (n < maxMsgs && WinGetMsg(hab, &qmsg, 0, 0, 0)) {
        if (qmsg.msg == TEST_PUMP_SENTINEL) break;
        (void)WinDispatchMsg(hab, &qmsg);
        n++;
    }
    return n;
}

/* ---------- Test ---------- */

/**
 * @brief Точка входа теста.
 * @param [in] argc  Не используется.
 * @param [in] argv  Не используется.
 * @return 0 — все проверки прошли, 1 — есть провалы.
 * @retval 0  g_fail == 0.
 * @retval 1  g_fail > 0.
 */
int main(int argc, char *argv[])
{
    HAB     hab;
    HMQ     hmq;
    HWND    hwndFrame1       = NULLHANDLE;
    HWND    hwndClient1      = NULLHANDLE;
    HWND    hwndTitle        = NULLHANDLE;
    HWND    hwndClientFromId = NULLHANDLE;
    HWND    hwndFrame2       = NULLHANDLE;
    HWND    hwndClient2      = NULLHANDLE;
    POINTL  ptlMove;
    POINTL  ptlLine;
    POINTL  ptlBox;
    ULONG   fcf;
    BOOL    ok;
    int     n;

    (void)argc; (void)argv;

    printf("=== test_win_stdwindow ===\n");

    /* ================================================================ */
    /*  Setup                                                           */
    /* ================================================================ */

    hab = WinInitialize(0);
    CHECK(hab != NULLHANDLE, "WinInitialize -> HAB=%lu",
          (unsigned long)hab);
    if (hab == NULLHANDLE) goto done;

    hmq = WinCreateMsgQueue(hab, 0);
    CHECK(hmq != NULLHANDLE, "WinCreateMsgQueue -> HMQ=%lu",
          (unsigned long)hmq);
    if (hmq == NULLHANDLE) goto cleanup;

    /* ================================================================ */
    /*  Test 1 — WinCreateStdWindow                                     */
    /* ================================================================ */

    fcf = FCF_TITLEBAR;
    hwndFrame1 = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE, &fcf,
                                    "TestClient", "Title", 0, 0, 0,
                                    &hwndClient1);
    CHECK(hwndFrame1 != NULLHANDLE,
          "WinCreateStdWindow -> hwndFrame=%lu",
          (unsigned long)hwndFrame1);
    CHECK(hwndClient1 != NULLHANDLE,
          "WinCreateStdWindow -> hwndClient=%lu",
          (unsigned long)hwndClient1);
    CHECK(fcf == FCF_TITLEBAR,
          "FCF_TITLEBAR honoured (fcf=0x%lx)", (unsigned long)fcf);

    if (hwndFrame1 == NULLHANDLE) goto cleanup;

    /* Сейчас сервер WM_PAINT не постит — drain вернёт 0.
     * Если/когда Phase 4 SrvPostPaint появится, drain подхватит
     * 3 сообщения (frame, titlebar, client). */
    n = PumpQueue(hab, 16);
    CHECK(n >= 0, "PumpQueue after create -> %d message(s)", n);

    /* ================================================================ */
    /*  Test 2 — WinWindowFromID(FID_TITLEBAR)                          */
    /* ================================================================ */

    hwndTitle = WinWindowFromID(hwndFrame1, FID_TITLEBAR);
    CHECK(hwndTitle != NULLHANDLE,
          "WinWindowFromID(FID_TITLEBAR) -> %lu",
          (unsigned long)hwndTitle);

    /* ================================================================ */
    /*  Test 3 — WinWindowFromID(FID_CLIENT)                            */
    /* ================================================================ */

    hwndClientFromId = WinWindowFromID(hwndFrame1, FID_CLIENT);
    CHECK(hwndClientFromId != NULLHANDLE,
          "WinWindowFromID(FID_CLIENT) -> %lu",
          (unsigned long)hwndClientFromId);
    CHECK(hwndClientFromId == hwndClient1,
          "WinWindowFromID(FID_CLIENT) == hwndClient (both %lu)",
          (unsigned long)hwndClient1);

    /* ================================================================ */
    /*  Test 4 — WinSetWindowPos -> WM_SIZE                             */
    /* ================================================================ */

    ok = WinSetWindowPos(hwndFrame1, NULLHANDLE, 100, 100, 300, 200,
                         SWP_MOVE | SWP_SIZE);
    CHECK(ok == TRUE, "WinSetWindowPos(SWP_MOVE|SWP_SIZE) -> TRUE");

    /* SWP_SIZE -> сервер постит WM_SIZE синхронно до ответа. */
    n = PumpQueue(hab, 32);
    CHECK(n >= 1, "WM_SIZE drained (n=%d)", n);

    /* ================================================================ */
    /*  Test 5 — WM_PAINT для frame + titlebar                          */
    /* ================================================================ */
    /*  Для детерминизма постим WM_PAINT вручную — путь
     *  WM_PAINT -> FrameWndProc -> EnsureDC -> GreBoxBoundary
     *  должен быть пройден независимо от Phase 4. */
    WinPostMsg(hwndFrame1, WM_PAINT, 0, 0);
    WinPostMsg(hwndTitle,  WM_PAINT, 0, 0);

    n = PumpQueue(hab, 64);
    CHECK(n >= 2, "WM_PAINT dispatched (%d messages total)", n);

    /* ================================================================ */
    /*  Test 6 — Gpi* рисование в client window                         */
    /* ================================================================ */
    /*  WinBeginPaint -> Normal PS, затем GpiSetColor + GpiLine +
     *  GpiBox -> GocaEmit* -> SrvDeliver -> GocaParseStream ->
     *  GrePolyLine / GreBox* -> GRE2SDL.
     *  WinEndPaint уничтожает PS. */
    {
        RECTL rcl;
        HPS   hps;

        memset(&rcl, 0, sizeof(rcl));
        hps = WinBeginPaint(hwndClient1, NULLHANDLE, &rcl);
        CHECK(hps != NULLHANDLE,
              "WinBeginPaint -> hps=%p (rcl={%ld,%ld,%ld,%ld})",
              (void*)hps,
              (long)rcl.xLeft, (long)rcl.yBottom,
              (long)rcl.xRight, (long)rcl.yTop);

        if (hps != NULLHANDLE) {
            BOOL b;

            b = GpiSetColor(hps, CLR_RED);
            CHECK(b == TRUE, "GpiSetColor(CLR_RED)");

            ptlMove.x = 10;  ptlMove.y = 10;
            b = GpiMove(hps, &ptlMove);
            CHECK(b == TRUE, "GpiMove({10,10})");

            ptlLine.x = 200; ptlLine.y = 150;
            b = GpiLine(hps, &ptlLine);
            CHECK(b == TRUE, "GpiLine({200,150})");

            ptlBox.x = 280;  ptlBox.y = 180;
            b = GpiBox(hps, DRO_OUTLINE, &ptlBox, 0, 0);
            CHECK(b == TRUE, "GpiBox(DRO_OUTLINE,{280,180},0,0)");

            b = WinEndPaint(hps);
            CHECK(b == TRUE, "WinEndPaint");
        }
    }

    /* ================================================================ */
    /*  Test 7 — неподдерживаемый FCF_* отвергается                     */
    /* ================================================================ */

    fcf = FCF_TITLEBAR | FCF_SHELLPOSITION;
    hwndFrame2 = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE, &fcf,
                                    "TestClient2", "Title2", 0, 0, 0,
                                    &hwndClient2);
    CHECK(hwndFrame2 == NULLHANDLE,
          "FCF_SHELLPOSITION rejected -> NULLHANDLE (got %lu)",
          (unsigned long)hwndFrame2);

    /* ================================================================ */
    /*  Cleanup                                                         */
    /* ================================================================ */

cleanup:
    if (hwndFrame1 != NULLHANDLE)
        WinDestroyWindow(hwndFrame1);
    if (hwndFrame2 != NULLHANDLE)
        WinDestroyWindow(hwndFrame2);

    if (hmq != NULLHANDLE)
        WinDestroyMsgQueue(hmq);

    WinTerminate(hab);

done:
    printf("\n=== Result: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
