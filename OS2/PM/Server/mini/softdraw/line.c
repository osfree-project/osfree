#include "softdraw.h"

VOID DrawLineBresenham(PLINEINFO pLine) {
    if (!pLine || !pLine->pDstBmapInfo || !pLine->pDstBmapInfo->pBits) {
        return;
    }

    PBMAPINFO pDst = pLine->pDstBmapInfo;
    ULONG color = pLine->ulFGColor;
    
    // Используем отсеченные (clipped) точки, если они заданы
    LONG x0 = pLine->ptlClipStart.x;
    LONG y0 = pLine->ptlClipStart.y;
    LONG x1 = pLine->ptlClipEnd.x;
    LONG y1 = pLine->ptlClipEnd.y;
    
    // Если отсеченные точки не заданы, используем начальную и конечную
    if (pLine->ptlClipStart.x == 0 && pLine->ptlClipStart.y == 0) {
        x0 = pLine->ptlStart.x;
        y0 = pLine->ptlStart.y;
        x1 = pLine->ptlEnd.x;
        y1 = pLine->ptlEnd.y;
    }

    LONG dx = labs(x1 - x0);
    LONG dy = labs(y1 - y0);
    LONG sx = (x0 < x1) ? 1 : -1;
    LONG sy = (y0 < y1) ? 1 : -1;
    LONG err = dx - dy;
    
    // Начальная ошибка может быть передана извне (lClipStartError)
    if (pLine->lClipStartError) {
        err = pLine->lClipStartError;
    }

    while (1) {
        // Рисуем пиксель, если он в пределах битмапа
        if (x0 >= 0 && x0 < (LONG)pDst->ulWidth && 
            y0 >= 0 && y0 < (LONG)pDst->ulHeight) {
            // Используем ту же логику SetPixel, что и в bitblt.c
            // (для краткости здесь предполагается 32-битный цвет)
            ULONG offset = (y0 * pDst->ulBytesPerLine) + (x0 * 4);
            *(PULONG)(pDst->pBits + offset) = color;
        }

        if (x0 == x1 && y0 == y1) break;

        LONG e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

ULONG _System SDLine(PLINEINFO pInput, PVOID pOutput) {
    if (!pInput) {
        return RC_ERROR;
    }
    
    DrawLineBresenham(pInput);
    
    return RC_SUCCESS;
}
