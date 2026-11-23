#include "softdraw.h"

/**
 * Точка входа для VMAN.
 * @param pInput  Код функции: 1 для SDBitBlt, 2 для SDLine.
 * @param pOutput Не используется.
 * @return Указатель на запрошенную функцию или 0.
 */
ULONG _System SDEntry(PVOID pInput, PVOID pOutput) {
    ULONG functionCode = (ULONG)pInput;

    switch (functionCode) {
        case 1:  return (ULONG)SDBitBlt;
        case 2:  return (ULONG)SDLine;
        default: return 0;
    }
}