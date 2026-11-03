/*!
 * @file test_gopth.cpp
 * @brief Practical test on real OS/2: confirm OCODE_GOPTH value.
 *
 * Builds a minimal segment: BeginPath(1) + Move + EndPath + OutlinePath(1).
 * Then asks GpiQueryElementType for every element, and dumps the segment
 * body via GpiGetData. The OutlinePath opcode shows up as either X'D9'
 * (IBM PM Reference 64F0277, Chapter 27) or X'D4' (old pmord.h).
 *
 * Build (OpenWatcom on OS/2):
 *   wcc386 test_gopth.cpp
 *   wlink system os2v2 file test_gopth.obj lib PMGPI PMWIN
 *
 * Run on a real OS/2 with Presentation Manager.
 */

#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>

int main(void)
{
    HAB   hab;
    HPS   hps;
    LONG  seg = 1;
    LONG  off, n, i, lType;
    BYTE  data[1024];

    hab = WinInitialize(0);
    if (!hab) {
        printf("WinInitialize failed\n");
        return 1;
    }

    hps = WinGetScreenPS(HWND_DESKTOP);
    if (!hps) {
        printf("WinGetScreenPS failed\n");
        WinTerminate(hab);
        return 1;
    }

    /* Build the segment: BeginPath(1), Move(0,0), EndPath, OutlinePath(1). */
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, seg);

    GpiBeginPath(hps, 1L);
    {
        POINTL pt = { 0, 0 };
        GpiMove(hps, &pt);
    }
    GpiEndPath(hps);
    GpiOutlinePath(hps, 1L, 0L);

    GpiCloseSegment(hps);

    /* Walk all elements and print their type. */
    GpiSetDrawingMode(hps, DM_RETAIN);
    GpiOpenSegment(hps, seg);

    printf("--- element types ---\n");
    for (i = 1; i <= 10; i++) {
        lType = 0;
        GpiSetElementPointer(hps, i);
        if (GpiQueryElementType(hps, &lType, 0L, NULL) < 0)
            break;
        printf("Elem %ld: 0x%02lX\n", (long)i, (long)lType);
        if (lType == 0xD4L || lType == 0xD9L) {
            printf("  -> OutlinePath opcode = 0x%02lX (%s)\n",
                   (long)lType,
                   (lType == 0xD9L) ? "IBM ref (D9)"
                                    : "old pmord (D4)");
        }
    }

    GpiCloseSegment(hps);
    GpiSetDrawingMode(hps, DM_DRAW);

    /* Dump the raw segment body. */
    off = 0;
    n = GpiGetData(hps, seg, &off, 0L, (LONG)sizeof(data), data);
    printf("--- segment body: %ld bytes ---\n", (long)n);
    for (i = 0; i < n; i++) {
        printf("%02X ", (unsigned)data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    if (n > 0 && (n % 16) != 0) printf("\n");

    GpiDeleteSegment(hps, seg);
    WinReleasePS(hps);
    WinTerminate(hab);
    return 0;
}
