#define INCL_GRE_LINES
#define INCL_GRE_ARCS
#define INCL_GRE_STRINGS
#define INCL_GRE_MARKERS
#define INCL_GRE_PATHS
#define INCL_GRE_PALETTE
#define INCL_PM
#include <F_platform.hpp>

#define INCL_DEV
#define INCL_GREALL
#include "pmddi.h"
#include "pmord.h"

#include <string.h>
#include <stdlib.h>
#include "goca_parser.hpp"

/* ---------- big-endian чтение (GOCA хранит старшим байтом вперЄд) ---------- */

static SHORT  RD_S16(const BYTE *p) { return (SHORT)((USHORT)p[0] << 8 | p[1]); }
static USHORT RD_U16(const BYTE *p) { return (USHORT)((USHORT)p[0] << 8 | p[1]); }
static LONG   RD_S32(const BYTE *p) {
    return ((LONG)p[0] << 24) | ((LONG)p[1] << 16) | ((LONG)p[2] << 8) | (LONG)p[3];
}

/* ---------- длина ордера в потоке ---------------------------------------- */

static ULONG OrderLength(const BYTE *p, ULONG cbAvail)
{
    if (cbAvail < 1) return 0;
    UCHAR oc = p[0];

    if (BYTE_ORDER(oc))       return 1;                 /* GNOP1, GESD */
    if (oc == OCODE_VLONG) {                            /* extended */
        if (cbAvail < 4) return 0;
        return 4 + RD_U16(p + 2);
    }
    if (SHORT_ORDER(oc))      return 2;                 /* 2-byte */
    /* long format */
    if (cbAvail < 2) return 0;
    return 2 + p[1];
}

/* ---------- вспомогательные ---------------------------------------------- */

/* GOCA POINTS (SHORT x,y) -> POINTL (LONG x,y) */
static POINTL PtsToPtl(const BYTE *p) {
    POINTL pt;
    pt.x = RD_S16(p);
    pt.y = RD_S16(p + 2);
    return pt;
}

/* »звлечь массив POINTS из массива байт длины (cb/4) в массив POINTL. */
static PPOINTL AllocPtlArray(const BYTE *pData, ULONG nPts)
{
    PPOINTL pptl = (PPOINTL)malloc(nPts * sizeof(POINTL));
    if (!pptl) return NULL;
    for (ULONG i = 0; i < nPts; i++) {
        pptl[i].x = RD_S16(pData + i * 4);
        pptl[i].y = RD_S16(pData + i * 4 + 2);
    }
    return pptl;
}

static void RefreshCurPos(GOCA_CTX *c)
{
    GreGetCurrentPosition(c->hdc, &c->ptlCurPos);
}

/* ---------- обработчики конкретных ордеров ------------------------------- */

/* GSCP Ч Set Current Position */
static LONG DoGSCP(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    if (cb < 6) return GOCA_ERR_TRUNC;
    POINTL pt = PtsToPtl(p + 2);
    GreSetCurrentPosition(c->hdc, &pt);
    c->ptlCurPos = pt;
    return GOCA_OK;
}

/* GLINE / GCLINE */
static LONG DoLine(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG len = p[1];
    if (cb < 2 + len) return GOCA_ERR_TRUNC;

    ULONG nTotal;
    PPOINTL pptl;

    if (bCur) {                                   /* GCLINE */
        if ((len & 3) || len == 0) return GOCA_ERR_FORMAT;
        ULONG nPts = len / 4;
        nTotal = nPts + 1;
        pptl = (PPOINTL)malloc(nTotal * sizeof(POINTL));
        if (!pptl) return GOCA_ERR_NOMEM;
        RefreshCurPos(c);
        pptl[0] = c->ptlCurPos;
        for (ULONG i = 0; i < nPts; i++) {
            pptl[i+1].x = RD_S16(p + 2 + i*4);
            pptl[i+1].y = RD_S16(p + 2 + i*4 + 2);
        }
    } else {                                      /* GLINE */
        if (len < 4 || ((len - 4) & 3)) return GOCA_ERR_FORMAT;
        nTotal = (len - 4) / 4 + 1;
        pptl = AllocPtlArray(p + 2, nTotal);
        if (!pptl) return GOCA_ERR_NOMEM;
    }

    GrePolyLine(c->hdc, pptl, nTotal);
    c->ptlCurPos = pptl[nTotal - 1];
    free(pptl);
    return GOCA_OK;
}

/* GMRK / GCMRK */
static LONG DoMark(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG len = p[1];
    if (cb < 2 + len) return GOCA_ERR_TRUNC;

    ULONG nTotal;
    PPOINTL pptl;

    if (bCur) {
        if ((len & 3) || len == 0) return GOCA_ERR_FORMAT;
        ULONG nPts = len / 4;
        nTotal = nPts + 1;
        pptl = (PPOINTL)malloc(nTotal * sizeof(POINTL));
        if (!pptl) return GOCA_ERR_NOMEM;
        RefreshCurPos(c);
        pptl[0] = c->ptlCurPos;
        for (ULONG i = 0; i < nPts; i++) {
            pptl[i+1].x = RD_S16(p + 2 + i*4);
            pptl[i+1].y = RD_S16(p + 2 + i*4 + 2);
        }
    } else {
        if (len < 4 || ((len - 4) & 3)) return GOCA_ERR_FORMAT;
        nTotal = (len - 4) / 4 + 1;
        pptl = AllocPtlArray(p + 2, nTotal);
        if (!pptl) return GOCA_ERR_NOMEM;
    }

    GrePolyMarker(c->hdc, pptl, nTotal);
    c->ptlCurPos = pptl[nTotal - 1];
    free(pptl);
    return GOCA_OK;
}

/* GCHST / GCCHST */
static LONG DoCharString(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG len = p[1];
    if (cb < 2 + len) return GOCA_ERR_TRUNC;

    POINTL ptStart;
    const BYTE *pStr;
    ULONG cch;

    if (bCur) {                                   /* GCCHST */
        RefreshCurPos(c);
        ptStart = c->ptlCurPos;
        pStr = p + 2;
        cch  = len;
    } else {                                      /* GCHST */
        if (len < 4) return GOCA_ERR_FORMAT;
        ptStart = PtsToPtl(p + 2);
        pStr = p + 6;
        cch  = len - 4;
    }

    if (cch == 0) return GOCA_OK;

    GreSetCurrentPosition(c->hdc, &ptStart);
    GreCharString(c->hdc, (LONG)cch, (PCH)pStr);
    /* ѕо спеке GCHST устанавливает current position в ptStart;
       GCCHST Ч нет. Ќаш GreSetCurrentPosition выше уже выставил еЄ. */
    c->ptlCurPos = ptStart;
    return GOCA_OK;
}

/* GBOX / GCBOX */
static LONG DoBox(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG len = p[1];
    if (cb < 2 + len) return GOCA_ERR_TRUNC;

    /* fbFlags, reserved, затем углы, затем HAXIS/VAXIS */
    UCHAR fbFlags = p[2];
    BOXPARAMS bp;
    memset(&bp, 0, sizeof(bp));

    if (bCur) {                                   /* GCBOX: 2-й угол */
        if (len < 6) return GOCA_ERR_FORMAT;
        RefreshCurPos(c);
        bp.x1 = c->ptlCurPos.x;
        bp.y1 = c->ptlCurPos.y;
        bp.x2 = RD_S16(p + 4);
        bp.y2 = RD_S16(p + 6);
        if (len >= 10) { bp.cx = RD_S16(p + 8); bp.cy = RD_S16(p + 10); }
    } else {                                      /* GBOX: два угла */
        if (len < 10) return GOCA_ERR_FORMAT;
        bp.x1 = RD_S16(p + 4); bp.y1 = RD_S16(p + 6);
        bp.x2 = RD_S16(p + 8); bp.y2 = RD_S16(p + 10);
        if (len >= 14) { bp.cx = RD_S16(p + 12); bp.cy = RD_S16(p + 14); }
    }

    BOOL bBoundary = (fbFlags & 0x40) != 0;   /* GCBOX_BOUNDARY */
    BOOL bFill     = (fbFlags & 0x20) != 0;   /* GCBOX_FILL     */
    if (bBoundary && bFill)   GreBoxBoth     (c->hdc, &bp);
    else if (bBoundary)       GreBoxBoundary (c->hdc, &bp);
    else if (bFill)           GreBoxInterior (c->hdc, &bp);

    if (!bCur) { c->ptlCurPos.x = bp.x2; c->ptlCurPos.y = bp.y2; }
    return GOCA_OK;
}

/* GPARC / GCPARC */
static LONG DoPartialArc(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG len = p[1];
    if (cb < 2 + len) return GOCA_ERR_TRUNC;

    const BYTE *q = p + 2;
    ULONG off = 0;

    if (!bCur) {                                 /* GPARC: start x/y */
        /* start игнорируем Ч GrePartialArc т€нет старт сам */
        off += 4;
    }
    /* центры и углы */
    if (len < off + 4 + 2 + 4 + 4) return GOCA_ERR_FORMAT;

    POINTL ptCenter;
    ptCenter.x = RD_S16(q + off);      off += 2;
    ptCenter.y = RD_S16(q + off);      off += 2;
    FIXED fxMult = (FIXED)RD_S16(q + off); off += 2;
    LONG  lStart = RD_S32(q + off);    off += 4;
    LONG  lSweep = RD_S32(q + off);

    GrePartialArc(c->hdc, &ptCenter, fxMult, lStart, lSweep);
    return GOCA_OK;
}

/* GCBEZ / GBEZ */
static LONG DoBezier(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG len = p[1];
    if (cb < 2 + len) return GOCA_ERR_TRUNC;

    ULONG nPts;
    PPOINTL pptl;
    if (bCur) {                                   /* GCBEZ */
        if ((len & 3) || len == 0) return GOCA_ERR_FORMAT;
        nPts = len / 4 + 1;
        pptl = (PPOINTL)malloc(nPts * sizeof(POINTL));
        if (!pptl) return GOCA_ERR_NOMEM;
        RefreshCurPos(c);
        pptl[0] = c->ptlCurPos;
        for (ULONG i = 0; i < nPts - 1; i++) {
            pptl[i+1].x = RD_S16(p + 2 + i*4);
            pptl[i+1].y = RD_S16(p + 2 + i*4 + 2);
        }
    } else {                                      /* GBEZ */
        if (len < 8 || ((len - 4) & 3)) return GOCA_ERR_FORMAT;
        nPts = (len - 4) / 4 + 1;
        pptl = AllocPtlArray(p + 2, nPts);
        if (!pptl) return GOCA_ERR_NOMEM;
    }
    GrePolySpline(c->hdc, pptl, nPts);
    c->ptlCurPos = pptl[nPts - 1];
    free(pptl);
    return GOCA_OK;
}

/* –асширенные CharString (very long) */
static LONG DoVLongCharString(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bGiven)
{
    /* FE | qualifier | len(2) | fbFlags | res | ptRect[0] | ptRect[1] | cchStr | str | adx[] */
    if (cb < 4 + 12) return GOCA_ERR_TRUNC;
    const BYTE *q = p + 4;
    UCHAR fbFlags = q[0];

    POINTL ptStart;
    if (bGiven) { ptStart = PtsToPtl(q + 2); }
    else        { RefreshCurPos(c); ptStart = c->ptlCurPos; }

    USHORT cch = RD_U16(q + 10);
    if (cb < 4 + 12 + cch) return GOCA_ERR_TRUNC;
    const BYTE *pStr = q + 12;
    if (cch == 0) return GOCA_OK;

    RECTL rcl;
    rcl.xLeft = ptStart.x; rcl.yBottom = ptStart.y;
    rcl.xRight = ptStart.x + 0x7FFF; rcl.yTop = ptStart.y + 0x7FFF;

    ULONG fl = CHS_DEFAULT;
    if (fbFlags & 0x40) fl |= CHS_CLIP;   /* GCCHSTE_CLIP */

    GreCharStringPos(c->hdc, &ptStart, &rcl, fl, (LONG)cch,
                     (PCH)pStr, NULL, NULL);
    c->ptlCurPos = ptStart;
    return GOCA_OK;
}

/* ---------- инициализаци€ ----------------------------------------------- */

void GocaInit(GOCA_CTX *c, HDC hdc, PBYTE pb, ULONG cb)
{
    memset(c, 0, sizeof(*c));
    c->hdc = hdc;
    c->pbStream = pb;
    c->cbStream = cb;

    /* ƒефолтные bundle'ы. ¬ реальном приложении сюда полезно положить
       GreQueryDeviceCaps/дефолты приложени€. */
    GreGetAttributes(hdc, ATTR_LINEBUNDLE,   PRIM_LINES,   &c->lb);
    GreGetAttributes(hdc, ATTR_CHARBUNDLE,   PRIM_CHARS,   &c->cb);
    GreGetAttributes(hdc, ATTR_MARKERBUNDLE, PRIM_MARKERS, &c->mb);
    GreGetAttributes(hdc, ATTR_AREABUNDLE,   PRIM_AREAS,   &c->ab);

    RefreshCurPos(c);
}

/* ---------- основной цикл ------------------------------------------------ */

LONG GocaParseStream(GOCA_CTX *c)
{
    c->uPos = 0;
    c->lError = 0;

    while (c->uPos < c->cbStream) {
        const BYTE *p = c->pbStream + c->uPos;
        ULONG cbAvail = c->cbStream - c->uPos;
        ULONG cbOrder = OrderLength(p, cbAvail);

        if (cbOrder == 0 || cbOrder > cbAvail) {
            c->lError = GOCA_ERR_TRUNC;
            c->uErrOffset = c->uPos;
            c->uErrOrder = p[0];
            return c->lError;
        }

        UCHAR oc = p[0];
        LONG  rc = GOCA_OK;
        BOOL  bHandled = FALSE;

        /* пользовательский обработчик Ч приоритет */
        if (c->pfnHandler) {
            bHandled = c->pfnHandler(c, oc, p, cbOrder);
        }

        if (!bHandled) {
            switch (oc) {

            /* --- 1/2-byte --- */
            case OCODE_GNOP1: break;
            case OCODE_GESD:  break;

            case OCODE_GSMX:   /* Set Mix */
                c->lb.lMixMode = c->cb.lMixMode = c->mb.lMixMode =
                c->ab.lMixMode = p[1];
                GreSetAttributes(c->hdc, ATTR_LINEBUNDLE,   PRIM_LINES,   &c->lb);
                GreSetAttributes(c->hdc, ATTR_CHARBUNDLE,   PRIM_CHARS,   &c->cb);
                GreSetAttributes(c->hdc, ATTR_MARKERBUNDLE, PRIM_MARKERS, &c->mb);
                GreSetAttributes(c->hdc, ATTR_AREABUNDLE,   PRIM_AREAS,   &c->ab);
                break;

            case OCODE_GSCOL: { /* индекс цвета в Standard Color Table */
                COLOR clr = (COLOR)(p[1] | 0xFF000000);
                c->lb.lColor = c->cb.lColor = c->mb.lColor = c->ab.lColor = clr;
                GreSetAttributes(c->hdc, ATTR_LINEBUNDLE,   PRIM_LINES,   &c->lb);
                GreSetAttributes(c->hdc, ATTR_CHARBUNDLE,   PRIM_CHARS,   &c->cb);
                GreSetAttributes(c->hdc, ATTR_MARKERBUNDLE, PRIM_MARKERS, &c->mb);
                GreSetAttributes(c->hdc, ATTR_AREABUNDLE,   PRIM_AREAS,   &c->ab);
                break; }

            case OCODE_GSECOL: { /* extended color, 2-byte index */
                COLOR clr = (COLOR)((LONG)p[1] | ((LONG)p[2] << 8) | 0xFF000000);
                c->lb.lColor = c->cb.lColor = c->mb.lColor = c->ab.lColor = clr;
                GreSetAttributes(c->hdc, ATTR_LINEBUNDLE,   PRIM_LINES,   &c->lb);
                GreSetAttributes(c->hdc, ATTR_CHARBUNDLE,   PRIM_CHARS,   &c->cb);
                GreSetAttributes(c->hdc, ATTR_MARKERBUNDLE, PRIM_MARKERS, &c->mb);
                GreSetAttributes(c->hdc, ATTR_AREABUNDLE,   PRIM_AREAS,   &c->ab);
                break; }

            case OCODE_GSLT:   /* Line Type */
                c->lb.lLineType = p[1];
                GreSetAttributes(c->hdc, ATTR_LINEBUNDLE, PRIM_LINES, &c->lb);
                break;

            case OCODE_GSLW:   /* Line Width (integer multiplier) */
                c->lb.fxWidth    = MAKEFIXED(p[1], 0);
                c->lb.lGeomWidth = p[1];
                GreSetAttributes(c->hdc, ATTR_LINEBUNDLE, PRIM_LINES, &c->lb);
                break;

            case OCODE_GSFLW:  /* Fractional Line Width */
                c->lb.fxWidth    = MAKEFIXED(p[1], p[2]);
                c->lb.lGeomWidth = p[1];
                GreSetAttributes(c->hdc, ATTR_LINEBUNDLE, PRIM_LINES, &c->lb);
                break;

            case OCODE_GSLE:   /* Line End */
                c->lb.usLineEnd = p[1];
                GreSetAttributes(c->hdc, ATTR_LINEBUNDLE, PRIM_LINES, &c->lb);
                break;

            case OCODE_GSLJ:   /* Line Join */
                c->lb.usLineJoin = p[1];
                GreSetAttributes(c->hdc, ATTR_LINEBUNDLE, PRIM_LINES, &c->lb);
                break;

            case OCODE_GSMT:   /* Marker Symbol */
                c->mb.usSymbol = p[1];
                GreSetAttributes(c->hdc, ATTR_MARKERBUNDLE, PRIM_MARKERS, &c->mb);
                break;

            case OCODE_GSPS:   /* Pattern Set */
                c->ab.usSet = p[1];
                GreSetAttributes(c->hdc, ATTR_AREABUNDLE, PRIM_AREAS, &c->ab);
                break;

            case OCODE_GSPT:   /* Pattern Symbol */
                c->ab.usSymbol = p[1];
                GreSetAttributes(c->hdc, ATTR_AREABUNDLE, PRIM_AREAS, &c->ab);
                break;

            case OCODE_GSCD:   /* Character Direction */
                c->cb.usDirection = p[1];
                GreSetAttributes(c->hdc, ATTR_CHARBUNDLE, PRIM_CHARS, &c->cb);
                break;

            case OCODE_GSCR:   /* Character Precision */
                c->cb.usPrecision = p[1] & GSCR_PRECISION;
                GreSetAttributes(c->hdc, ATTR_CHARBUNDLE, PRIM_CHARS, &c->cb);
                break;

            case OCODE_GSCS:   /* Character Set */
                c->cb.usCodePage = p[1];   /* упрощение: кодова€ страница */
                GreSetAttributes(c->hdc, ATTR_CHARBUNDLE, PRIM_CHARS, &c->cb);
                break;

            /* --- GBAR/GEAR --- */
            case OCODE_GBAR:
                c->uAreaFlags = p[1];
                c->bInArea = TRUE;
                {
                    ULONG fl = 0;
                    if (p[1] & 0x40) fl |= BA_BOUNDARY; else fl |= BA_NOBOUNDARY;
                    if (p[1] & 0x20) fl |= BA_WINDING;  else fl |= BA_ALTERNATE;
                    GreBeginArea(c->hdc, fl);
                }
                break;

            case OCODE_GEAR:
                if (c->bInArea) {
                    GreEndArea(c->hdc, 0);
                    c->bInArea = FALSE;
                }
                break;

            /* --- long orders --- */
            case OCODE_GSCP:   rc = DoGSCP(c, p, cbOrder); break;
            case OCODE_GLINE:  rc = DoLine(c, p, cbOrder, FALSE); break;
            case OCODE_GCLINE: rc = DoLine(c, p, cbOrder, TRUE);  break;
            case OCODE_GMRK:   rc = DoMark(c, p, cbOrder, FALSE); break;
            case OCODE_GCMRK:  rc = DoMark(c, p, cbOrder, TRUE);  break;
            case OCODE_GCHST:  rc = DoCharString(c, p, cbOrder, FALSE); break;
            case OCODE_GCCHST: rc = DoCharString(c, p, cbOrder, TRUE);  break;
            case OCODE_GBOX:   rc = DoBox(c, p, cbOrder, FALSE); break;
            case OCODE_GCBOX:  rc = DoBox(c, p, cbOrder, TRUE);  break;
            case OCODE_GPARC:  rc = DoPartialArc(c, p, cbOrder, FALSE); break;
            case OCODE_GCPARC: rc = DoPartialArc(c, p, cbOrder, TRUE);  break;
            case OCODE_GBEZ:   rc = DoBezier(c, p, cbOrder, FALSE); break;
            case OCODE_GCBEZ:  rc = DoBezier(c, p, cbOrder, TRUE);  break;

            /* --- very long --- */
            case OCODE_VLONG:
                switch (p[1]) {
                case OCODEQ_GCHSTE:  rc = DoVLongCharString(c, p, cbOrder, TRUE);  break;
                case OCODEQ_GCCHSTE: rc = DoVLongCharString(c, p, cbOrder, FALSE); break;
                default: break;   /* tabbed/escape Ч оставл€ем пользователю */
                }
                break;

            default:
                /* Ќеизвестный/неподдержанный ордер Ч просто пропускаем.
                   ≈сли нужен диагностический вывод Ч включите callback. */
                break;
            }
        }

        if (rc != GOCA_OK) {
            c->lError = rc;
            c->uErrOffset = c->uPos;
            c->uErrOrder = oc;
            return rc;
        }
        c->uPos += cbOrder;
    }
    return GOCA_OK;
}

const char *GocaErrorName(LONG lError)
{
    switch (lError) {
    case GOCA_OK:          return "OK";
    case GOCA_ERR_TRUNC:   return "truncated order";
    case GOCA_ERR_FORMAT:  return "malformed order";
    case GOCA_ERR_NOMEM:   return "out of memory";
    case GOCA_ERR_USER:    return "user handler failure";
    default:               return "unknown";
    }
}

const char *GocaOrderName(UCHAR uCode)
{
    switch (uCode) {
    case OCODE_GNOP1: return "GNOP1";  case OCODE_GSCP:  return "GSCP";
    case OCODE_GLINE: return "GLINE";  case OCODE_GCLINE:return "GCLINE";
    case OCODE_GMRK:  return "GMRK";   case OCODE_GCMRK: return "GCMRK";
    case OCODE_GCHST: return "GCHST";  case OCODE_GCCHST:return "GCCHST";
    case OCODE_GBOX:  return "GBOX";   case OCODE_GCBOX: return "GCBOX";
    case OCODE_GPARC: return "GPARC";  case OCODE_GCPARC:return "GCPARC";
    case OCODE_GBAR:  return "GBAR";   case OCODE_GEAR:  return "GEAR";
    case OCODE_VLONG: return "VLONG";
    default:          return "?";
    }
}
