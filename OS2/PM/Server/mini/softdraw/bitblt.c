#include "softdraw.h"

// Вспомогательная функция для получения цвета пикселя с учетом глубины цвета
static ULONG GetPixel(PBMAPINFO pBmap, LONG x, LONG y) {
    if (!pBmap || !pBmap->pBits || x < 0 || x >= (LONG)pBmap->ulWidth || y < 0 || y >= (LONG)pBmap->ulHeight) {
        return 0;
    }
    
    ULONG bytesPerPixel = pBmap->ulBitCount / 8;
    if (bytesPerPixel == 0) bytesPerPixel = 1; // Для 1bpp
    
    ULONG offset = (y * pBmap->ulBytesPerLine) + (x * bytesPerPixel);
    PBYTE pPixel = pBmap->pBits + offset;
    
    switch (pBmap->ulBitCount) {
        case 1:  return (*pPixel & (0x80 >> (x % 8))) ? 1 : 0;
        case 4:  return (*pPixel >> ((x % 2) ? 0 : 4)) & 0x0F;
        case 8:  return *pPixel;
        case 16: return *(PUSHORT)pPixel;
        case 24: return (*(PULONG)pPixel) & 0xFFFFFF;
        case 32: return *(PULONG)pPixel;
        default: return 0;
    }
}

// Вспомогательная функция для установки цвета пикселя
static VOID SetPixel(PBMAPINFO pBmap, LONG x, LONG y, ULONG color) {
    if (!pBmap || !pBmap->pBits || x < 0 || x >= (LONG)pBmap->ulWidth || y < 0 || y >= (LONG)pBmap->ulHeight) {
        return;
    }

    ULONG bytesPerPixel = pBmap->ulBitCount / 8;
    if (bytesPerPixel == 0) bytesPerPixel = 1;
    
    ULONG offset = (y * pBmap->ulBytesPerLine) + (x * bytesPerPixel);
    PBYTE pPixel = pBmap->pBits + offset;

    switch (pBmap->ulBitCount) {
        case 1:
            if (color) *pPixel |= (0x80 >> (x % 8));
            else       *pPixel &= ~(0x80 >> (x % 8));
            break;
        case 4:
            if (x % 2) *pPixel = (*pPixel & 0xF0) | (color & 0x0F);
            else       *pPixel = (*pPixel & 0x0F) | ((color & 0x0F) << 4);
            break;
        case 8:  *pPixel = (BYTE)color; break;
        case 16: *(PUSHORT)pPixel = (USHORT)color; break;
        case 24: *(PULONG)pPixel = (*(PULONG)pPixel & 0xFF000000) | (color & 0xFFFFFF); break;
        case 32: *(PULONG)pPixel = color; break;
    }
}

ULONG _System SDBitBlt(PBITBLTINFO pInput, PVOID pOutput) {
    if (!pInput || !pInput->pDstBmapInfo || !pInput->pDstBmapInfo->pBits) {
        return RC_ERROR;
    }

    PBMAPINFO pDst = pInput->pDstBmapInfo;
    PBMAPINFO pSrc = pInput->pSrcBmapInfo;
    PBMAPINFO pPat = pInput->pPatBmapInfo;
    ULONG rop = pInput->ulROP;
    ULONG flags = pInput->ulBltFlags;

    // Обрабатываем все запрошенные Blt-операции (cBlits)
    for (ULONG blt = 0; blt < pInput->cBlits; blt++) {
        PRECTL prclDst = &pInput->prclDstBounds[blt];
        PBLTRECT pBltRect = (pInput->abrDst) ? &pInput->abrDst[blt] : NULL;
        
        LONG dstXStart = prclDst->xLeft;
        LONG dstYStart = prclDst->yTop;
        LONG dstXEnd = prclDst->xRight;
        LONG dstYEnd = prclDst->yBottom;

        // Смещения источника и паттерна для текущего Blt
        LONG srcOrgX = (pInput->aptlSrcOrg) ? pInput->aptlSrcOrg[blt].x : 0;
        LONG srcOrgY = (pInput->aptlSrcOrg) ? pInput->aptlSrcOrg[blt].y : 0;
        LONG patOrgX = (pInput->aptlPatOrg) ? pInput->aptlPatOrg[blt].x : 0;
        LONG patOrgY = (pInput->aptlPatOrg) ? pInput->aptlPatOrg[blt].y : 0;

        // Цикл по всем строкам целевого прямоугольника
        for (LONG y = dstYStart; y < dstYEnd; y++) {
            for (LONG x = dstXStart; x < dstXEnd; x++) {
                // --- 1. Получение цвета пикселя НАЗНАЧЕНИЯ ---
                ULONG dstColor = GetPixel(pDst, x, y);

                // --- 2. Получение цвета пикселя ИСТОЧНИКА ---
                ULONG srcColor = 0;
                if (pSrc && pSrc->pBits) {
                    LONG srcX = srcOrgX + (x - dstXStart);
                    LONG srcY = srcOrgY + (y - dstYStart);
                    srcColor = GetPixel(pSrc, srcX, srcY);
                }

                // --- 3. Получение цвета пикселя ПАТТЕРНА ---
                ULONG patColor = 0;
                if (pPat && pPat->pBits) {
                    LONG patX = patOrgX + (x - dstXStart);
                    LONG patY = patOrgY + (y - dstYStart);
                    patColor = GetPixel(pPat, patX, patY);
                }

                // --- 4. Обработка флагов (прозрачность, инверсия) ---
                if ((flags & BF_SOURCE_TRANSPARENT) && (srcColor == pInput->ulSrcBGColor)) {
                    continue; // Пропускаем прозрачный пиксель
                }
                if (flags & BF_SRC_INVERT) srcColor = ~srcColor;
                if (flags & BF_PAT_INVERT) patColor = ~patColor;
                if (flags & BF_DST_INVERT) dstColor = ~dstColor;

                // --- 5. Применение растровой операции ---
                ULONG resultColor = ApplyROP3(rop, srcColor, patColor, dstColor);

                // --- 6. Обработка фонового ROP (если установлен флаг) ---
                if (flags & BF_APPLY_BACK_ROP) {
                    // Упрощенная логика: если пиксель "неактивен" по маске,
                    // применяем фоновый ROP. Точная логика зависит от маски.
                    // В реальном SOFTDRAW это используется для монохромных паттернов.
                }

                // --- 7. Запись результата в НАЗНАЧЕНИЕ ---
                SetPixel(pDst, x, y, resultColor);
            }
        }
    }

    return RC_SUCCESS;
}