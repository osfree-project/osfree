#ifndef GOCA_PARSER_H
#define GOCA_PARSER_H

#define INCL_PM
#include <F_platform.hpp>
#include "pmord.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GOCA_CTX GOCA_CTX;

/* Необязательный обработчик для расширений. Вернуть TRUE — обработано. */
typedef BOOL (*PFN_GOCA_HANDLER)(GOCA_CTX *pCtx, UCHAR uCode,
                                  const BYTE *pbData, ULONG cbData);

struct _GOCA_CTX {
    HDC     hdc;        /* целевой PS */
    PBYTE   pbStream;   /* поток ордеров */
    ULONG   cbStream;   /* его размер */
    ULONG   uPos;       /* текущее смещение */

    BOOL    bInArea;
    UCHAR   uAreaFlags; /* GBAR flags */

    POINTL  ptlCurPos;  /* кэш текущей позиции */

    /* Кэш атрибутов — обновляется GSLT/GSCOL/... */
    LINEBUNDLE   lb;
    CHARBUNDLE   cb;
    MARKERBUNDLE mb;
    AREABUNDLE   ab;

    PFN_GOCA_HANDLER pfnHandler;
    void   *pUserData;

    /* диагностика */
    LONG    lError;
    ULONG   uErrOffset;
    UCHAR   uErrOrder;
};

#define GOCA_OK            0
#define GOCA_ERR_TRUNC    -1   /* ордер обрезан */
#define GOCA_ERR_FORMAT   -2   /* неверный формат/флаги */
#define GOCA_ERR_NOMEM    -3
#define GOCA_ERR_USER     -4

const char *GocaErrorName(LONG lError);
const char *GocaOrderName(UCHAR uCode);

/* Инициализация: заполняет дефолтные bundle'ы из текущего PS и кэширует
 * текущую позицию. Вызывать перед GocaParseStream. */
void GocaInit(GOCA_CTX *pCtx, HDC hdc, PBYTE pb, ULONG cb);

/* Возвращает GOCA_OK или отрицательный код. */
LONG GocaParseStream(GOCA_CTX *pCtx);

#ifdef __cplusplus
}
#endif
#endif
