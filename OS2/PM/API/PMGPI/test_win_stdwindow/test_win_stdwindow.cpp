/*!
 * @file test_win_stdwindow.cpp
 * @brief Перекрывающиеся окна, смена Z-order с задержками.
 *
 * Сценарий (8 фаз, по 2 с):
 *   1. Создание W1 и W2, перекрытие (250,150)-(400,280).
 *   2. Painting в различных порядках с задержкой DWELL_MS.
 *   3. Z-order меняется через WinSetWindowPos(..., SWP_ZORDER).
 *   4. После каждой смены — перерисовка, чтобы видеть эффект.
 *
 * Что ожидается при корректной реализации Z-order:
 *   В области перекрытия видно СОДЕРЖИМОЕ ВЕРХНЕГО окна.
 *   Перерисовка нижнего окна НЕ меняет перекрытие.
 *
 * Что видно в текущей реализации (Z-order не влияет на клип):
 *   В области перекрытия видно СОДЕРЖИМОЕ ОКНА, НАРИСОВАННОГО ПОСЛЕДНИМ.
 *   Перерисовка нижнего затирает верхнее — это и демонстрируется.
 *
 * Цвета: raw RGB 0x00BBGGRR, <= 0xFFFF (B=0). CLR_* не используются —
 * они трактуются как индексы, а палитра/цветовая таблица в сервере
 * пока не реализованы (CreateLogColorTable — стаб).
 */

#define INCL_PM
#include "F_platform.h"
#include "F_cmd.h"
#include "F_debug.h"

#include <stdio.h>
#include <string.h>

/* Цвета (raw RGB, <= 0xFFFF) */
#define COLOR_RED    0x0000FFL
#define COLOR_GREEN  0x00FF00L

/* Задержка между фазами, мс */
#define DWELL_MS     2000

/* ---------- Test harness ---------- */

static int g_pass = 0;
static int g_fail = 0;

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

#define TEST_PUMP_SENTINEL  (0x0401UL)

/**
 * @brief Вычитать очередь до sentinel.
 * @param [in] hab      Anchor block.
 * @param [in] maxMsgs  Предохранитель.
 * @return Число диспатченных; -1 при ошибке WinPostMsg.
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

/**
 * @brief Залить весь client одним цветом (DRO_FILL|DRO_OUTLINE).
 * @param [in] hwnd   Client HWND.
 * @param [in] color  Цвет raw RGB <= 0xFFFF.
 * @return TRUE при успехе.
 */
static BOOL PaintSolid(HWND hwnd, LONG color)
{
    RECTL  rcl;
    HPS    hps;
    POINTL a, b;

    hps = WinBeginPaint(hwnd, NULLHANDLE, &rcl);
    if (hps == NULLHANDLE) return FALSE;

    a.x = rcl.xLeft;
    a.y = rcl.yBottom;
    b.x = rcl.xRight  - 1;
    b.y = rcl.yTop    - 1;

    GpiSetColor(hps, color);
    GpiMove(hps, &a);
    GpiBox(hps, DRO_FILL | DRO_OUTLINE, &b, 0, 0);

    WinEndPaint(hps);
    return TRUE;
}

/* ---------- Test ---------- */

/**
 * @brief Точка входа.
 * @param [in] argc Не используется.
 * @param [in] argv Не используется.
 * @return 0 при отсутствии провалов.
 */
int main(int argc, char *argv[])
{
    HAB   hab   = NULLHANDLE;
    HMQ   hmq   = NULLHANDLE;
    HWND  frame1 = NULLHANDLE, client1 = NULLHANDLE, title1 = NULLHANDLE;
    HWND  frame2 = NULLHANDLE, client2 = NULLHANDLE, title2 = NULLHANDLE;
    HWND  frame3 = NULLHANDLE, client3 = NULLHANDLE;
    ULONG fcf;
    BOOL  ok;

    (void)argc; (void)argv;

    printf("=== test_win_stdwindow (overlap + Z-order cycling) ===\n");

    /* ---------------- Setup ---------------- */

    hab = WinInitialize(0);
    CHECK(hab != NULLHANDLE, "WinInitialize -> HAB=%lu",
          (unsigned long)hab);
    if (!hab) goto done;

    hmq = WinCreateMsgQueue(hab, 0);
    CHECK(hmq != NULLHANDLE, "WinCreateMsgQueue -> HMQ=%lu",
          (unsigned long)hmq);
    if (!hmq) goto cleanup;

    /* W1: frame (100,100) 300x200 > client (100,100)-(400,280) */
    fcf = FCF_TITLEBAR;
    frame1 = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE, &fcf,
                                "Win1", "Window One", 0, 0, 0, &client1);
    CHECK(frame1 != NULLHANDLE && client1 != NULLHANDLE,
          "WinCreateStdWindow W1 -> frame=%lu client=%lu",
          (unsigned long)frame1, (unsigned long)client1);

    /* W2: frame (250,150) 240x160 > client (250,150)-(490,290)
     * Перекрытие client'ов: (250,150)-(400,280) = 150x130 */
    fcf = FCF_TITLEBAR;
    frame2 = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE, &fcf,
                                "Win2", "Window Two", 0, 0, 0, &client2);
    CHECK(frame2 != NULLHANDLE && client2 != NULLHANDLE,
          "WinCreateStdWindow W2 -> frame=%lu client=%lu",
          (unsigned long)frame2, (unsigned long)client2);

    ok = WinSetWindowPos(frame1, NULLHANDLE, 100, 100, 300, 200,
                         SWP_MOVE | SWP_SIZE);
    CHECK(ok == TRUE, "WinSetWindowPos W1 (100,100) 300x200");

    ok = WinSetWindowPos(frame2, NULLHANDLE, 250, 150, 240, 160,
                         SWP_MOVE | SWP_SIZE);
    CHECK(ok == TRUE, "WinSetWindowPos W2 (250,150) 240x160");

    /* Вычитать WM_SIZE, накопленные от SWP_SIZE */
    (void)PumpQueue(hab, 16);

    title1 = WinWindowFromID(frame1, FID_TITLEBAR);
    title2 = WinWindowFromID(frame2, FID_TITLEBAR);

    /* ---------------- Phase 1 ---------------- */
    printf("\n[Phase 1] Default Z-order: W2 top (создан последним).\n");
    printf("  Рисуем W1 (красный), затем W2 (зелёный).\n");
    PaintSolid(client1, COLOR_RED);
    PaintSolid(client2, COLOR_GREEN);
    printf("  ОЖИДАЕТСЯ: в перекрытии ЗЕЛЁНЫЙ (W2 сверху).\n");
    DosSleep(DWELL_MS);

    /* ---------------- Phase 2 ---------------- */
    printf("\n[Phase 2] Перерисовать W1 (нижнее).\n");
    printf("  Корректно: W1 обрезан, перекрытие не меняется (зелёный).\n");
    printf("  БАГ: W1 не обрезан, перекрытие становится КРАСНЫМ.\n");
    PaintSolid(client1, COLOR_RED);
    DosSleep(DWELL_MS);

    /* ---------------- Phase 3 ---------------- */
    printf("\n[Phase 3] SWP_ZORDER: поднять W1 выше W2.\n");
    WinSetWindowPos(frame1, HWND_TOP, 0, 0, 0, 0, SWP_ZORDER);
    printf("  Рисуем W1 (красный) — теперь он сверху.\n");
    PaintSolid(client1, COLOR_RED);
    printf("  ОЖИДАЕТСЯ: в перекрытии КРАСНЫЙ (W1 сверху).\n");
    DosSleep(DWELL_MS);

    /* ---------------- Phase 4 ---------------- */
    printf("\n[Phase 4] Перерисовать W2 (нижнее).\n");
    printf("  Корректно: W2 обрезан, перекрытие не меняется (красный).\n");
    printf("  БАГ: W2 не обрезан, перекрытие становится ЗЕЛЁНЫМ.\n");
    PaintSolid(client2, COLOR_GREEN);
    DosSleep(DWELL_MS);

    /* ---------------- Phase 5 ---------------- */
    printf("\n[Phase 5] SWP_ZORDER: поднять W2 выше W1.\n");
    WinSetWindowPos(frame2, HWND_TOP, 0, 0, 0, 0, SWP_ZORDER);
    printf("  Рисуем W2 (зелёный) — теперь он сверху.\n");
    PaintSolid(client2, COLOR_GREEN);
    printf("  ОЖИДАЕТСЯ: в перекрытии ЗЕЛЁНЫЙ (W2 сверху).\n");
    DosSleep(DWELL_MS);

    /* ---------------- Phase 6 ---------------- */
    printf("\n[Phase 6] Перерисовать оба в корректном порядке:\n");
    printf("          W1 (нижнее) > W2 (верхнее).\n");
    PaintSolid(client1, COLOR_RED);
    PaintSolid(client2, COLOR_GREEN);
    printf("  ОЖИДАЕТСЯ: ЗЕЛЁНЫЙ (верхнее нарисовано последним).\n");
    DosSleep(DWELL_MS);

    /* ---------------- Phase 7 ---------------- */
    printf("\n[Phase 7] Перерисовать оба в НЕПРАВИЛЬНОМ порядке:\n");
    printf("          W2 (верхнее) > W1 (нижнее).\n");
    PaintSolid(client2, COLOR_GREEN);
    PaintSolid(client1, COLOR_RED);
    printf("  Корректный движок: ЗЕЛЁНЫЙ (нижнее обрезано).\n");
    printf("  БАГ:               КРАСНЫЙ (нижнее нарисовано последним).\n");
    DosSleep(DWELL_MS);

    /* ---------------- Phase 8 ---------------- */
    printf("\n[Phase 8] SWP_ZORDER: снова поднять W1. Рисуем W1 (красный).\n");
    WinSetWindowPos(frame1, HWND_TOP, 0, 0, 0, 0, SWP_ZORDER);
    PaintSolid(client1, COLOR_RED);
    printf("  ОЖИДАЕТСЯ: КРАСНЫЙ (W1 снова сверху).\n");
    DosSleep(DWELL_MS);

    /* ---------------- Sanity: unsupported FCF_* ---------------- */
    fcf = FCF_TITLEBAR | FCF_SHELLPOSITION;
    frame3 = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE, &fcf,
                                "Win3", "Window Three", 0, 0, 0, &client3);
    CHECK(frame3 == NULLHANDLE,
          "FCF_SHELLPOSITION rejected -> NULLHANDLE (got %lu)",
          (unsigned long)frame3);

    /* ---------------- Cleanup ---------------- */

    if (frame1 != NULLHANDLE) WinDestroyWindow(frame1);
    if (frame2 != NULLHANDLE) WinDestroyWindow(frame2);
    if (frame3 != NULLHANDLE) WinDestroyWindow(frame3);

cleanup:
    if (hmq != NULLHANDLE) WinDestroyMsgQueue(hmq);
    if (hab != NULLHANDLE) WinTerminate(hab);

done:
    printf("\n=== Result: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
