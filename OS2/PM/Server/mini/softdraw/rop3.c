#include "softdraw.h"

/**
 * Выполняет все 256 ROP3-операций.
 * @param rop  Код ROP3 (0-255).
 * @param src  Значение пикселя источника (S).
 * @param pat  Значение пикселя паттерна (P).
 * @param dst  Значение пикселя назначения (D).
 * @return Результирующее значение пикселя.
 */
ULONG ApplyROP3(ULONG rop, ULONG src, ULONG pat, ULONG dst) {
    ULONG result = 0;
    ULONG bit;

    // Проходим по всем 32 битам (максимальная глубина цвета)
    for (bit = 0; bit < 32; bit++) {
        // Извлекаем i-й бит из каждого операнда
        ULONG s = (src >> bit) & 1;
        ULONG p = (pat >> bit) & 1;
        ULONG d = (dst >> bit) & 1;
        ULONG r = 0;

        // Вычисляем результат для i-го бита на основе кода ROP.
        // Каждый бит кода ROP (от 0 до 7) определяет,
        // какие термы (S, P, D) участвуют в логической операции.
        // Это стандартный метод декодирования ROP3.
        if (rop & 0x01) r |= (s & p & d);
        if (rop & 0x02) r |= (s & p & ~d);
        if (rop & 0x04) r |= (s & ~p & d);
        if (rop & 0x08) r |= (s & ~p & ~d);
        if (rop & 0x10) r |= (~s & p & d);
        if (rop & 0x20) r |= (~s & p & ~d);
        if (rop & 0x40) r |= (~s & ~p & d);
        if (rop & 0x80) r |= (~s & ~p & ~d);

        // Устанавливаем i-й бит результата
        result |= (r << bit);
    }

    return result;
}
