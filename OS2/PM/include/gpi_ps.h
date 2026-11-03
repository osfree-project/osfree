/*!
 * @file gpi_ps.h
 * @brief Per-PS state, shared between client and server.
 *
 * One structure and one inline implementation. Common fields are always
 * present; server-only fields are compiled in under GPI_SERVER,
 * client-only under GPI_CLIENT. Exactly one of these must be defined
 * before including this header.
 *
 * The GpiPs* helpers are static inline; the only thing each side must
 * provide is the table itself:
 *   client: GPI_PS g_psTable[GPI_MAX_PS];  in gpi_entry.cpp
 *   server: GPI_PS g_psTable[GPI_MAX_PS];  in dispatch.cpp
 */
#ifndef GPI_PS_H
#define GPI_PS_H

#include "goca_common.h"
#include <string.h>

#if !defined(GPI_CLIENT) && !defined(GPI_SERVER)
#error Define GPI_CLIENT or GPI_SERVER before including gpi_ps.h
#endif
#if defined(GPI_CLIENT) && defined(GPI_SERVER)
#error Define only one of GPI_CLIENT / GPI_SERVER
#endif

#ifdef GPI_SERVER
#include "goca_parser.hpp"   /* GocaInit (Gre-dependent) */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Maximum number of simultaneous PS entries. */
#define GPI_MAX_PS 64

#ifndef PS_IS_RECORDING
#define PS_IS_RECORDING(m)  ((m) == DM_RETAIN || (m) == DM_DRAWANDRETAIN)
#endif
#ifndef PS_IS_DRAWING
#define PS_IS_DRAWING(m)    ((m) == DM_DRAW   || (m) == DM_DRAWANDRETAIN)
#endif

#ifndef SEGEM_INSERT
#define SEGEM_INSERT   1L
#endif
#ifndef SEGEM_REPLACE
#define SEGEM_REPLACE  2L
#endif

#ifndef SEGATTR_BIT
/*! @brief Bit mask for one attribute in ulInitialSegAttrs. */
#define SEGATTR_BIT(a)  (1UL << (ULONG)(a))
#endif

/*!
 * @struct _GPI_PS
 * @brief Presentation space state.
 */
typedef struct _GPI_PS {
    BOOL     inUse;      /*!< Slot occupied.                              */
    HPS      hps;        /*!< Handle used as map key.                     */
    ULONG    flOptions;  /*!< Last flOptions from GpiCreatePS/GpiSetPS.   */
    GOCA_CTX goca;       /*!< GOCA context (coord size, endian, caches).  */

#ifdef GPI_SERVER
    /* ---- GOCA stream -------------------------------------------------- */
    PBYTE    pbGoca;       /*!< Accumulated GOCA stream.                  */
    ULONG    cbGoca;       /*!< Bytes used.                               */
    ULONG    cbGocaAlloc;  /*!< Capacity.                                 */

    /* ---- Segment-edit state ------------------------------------------ */
    LONG     lOpenSeg;     /*!< id of open segment, 0 if none.            */
    ULONG    uOpenSegStart;/*!< Offset of its GBSEG in pbGoca.            */

    /* ---- GPI state --------------------------------------------------- */
    ULONG    ulDrawingMode;     /*!< DM_DRAW / DM_RETAIN / DM_DRAWANDRETAIN */
    ULONG    ulEditMode;        /*!< SEGEM_INSERT / SEGEM_REPLACE           */
    ULONG    ulInitialSegAttrs; /*!< SEGATTR_BIT bits                       */
    ULONG    ulDrawControl;     /*!< DCTL_* bits                            */
    ULONG    ulStopDraw;        /*!< SDW_OFF / SDW_ON                       */

    /* ---- Segment editing (GpiBeginElement .. GpiEndElement) ---------- */
    LONG     lCurrentElement;   /* 0 = before first; 1..N = element index  */
    BOOL     bInElement;        /* TRUE between BeginElement/EndElement    */
    ULONG    uOpenElementStart; /* offset in pbGoca where current bracket begins */
    /* ---- Path state --------------------------------------------------- */
    LONG     lOpenPath;     /* 0 = no path state; 1 = between Begin/End  */
    BOOL     bPathDefined;  /* TRUE = EndPath seen, awaiting Fill/...    */
#endif
} GPI_PS, *PGPI_PS;

/*! @brief Single PS table per binary; defined in gpi_entry.cpp / dispatch.cpp. */
extern GPI_PS g_psTable[GPI_MAX_PS];

/* --------------------------------------------------------------------- */
/*  Common implementation                                                */
/* --------------------------------------------------------------------- */

/*! @brief Find a PS entry by handle. */
static inline PGPI_PS GpiPsFind(HPS hps)
{
    int i;
    PGPI_PS ps;

    if (hps == NULL) return (PGPI_PS)NULL;

    /* Pointer case: NGpiCreatePS returns a pointer into g_psTable as
       the client-facing HPS. On the server side that value is in the
       range of g_psTable and identifies the slot directly. On the
       client side the value is a server-process address and this
       branch never matches. */
    ps = (PGPI_PS)hps;
    if (ps >= g_psTable && ps < g_psTable + GPI_MAX_PS && ps->inUse)
        return ps;

    /* Handle case: match by stored .hps value (client side; server
       auto-create). */
    for (i = 0; i < GPI_MAX_PS; i++) {
        if (g_psTable[i].inUse &&
            g_psTable[i].hps != NULL &&
            g_psTable[i].hps == hps)
            return &g_psTable[i];
    }
    return (PGPI_PS)NULL;
}

/*! @brief Register or update a PS entry. */
static inline PGPI_PS GpiPsCreate(HPS hps, ULONG flOptions)
{
    int i;

    if (hps == NULL && (ULONG)hps == (ULONG)GPI_ERROR)
        return (PGPI_PS)NULL;

    /* Update in place when hps is a real handle. Server-side
       NGpiCreatePS calls with hps == NULL to request a fresh slot. */
    if (hps != NULL) {
        for (i = 0; i < GPI_MAX_PS; i++) {
            if (g_psTable[i].inUse && g_psTable[i].hps == hps) {
                g_psTable[i].flOptions = flOptions;
                GocaSetFormatFromOptions(&g_psTable[i].goca, flOptions);
                return &g_psTable[i];
            }
        }
    }

    for (i = 0; i < GPI_MAX_PS; i++) {
        if (!g_psTable[i].inUse) {
            GPI_PS *ps = &g_psTable[i];

            memset(ps, 0, sizeof(*ps));
            ps->inUse     = TRUE;
            ps->hps       = hps;
            ps->flOptions = flOptions;

#ifdef GPI_SERVER
            GocaInit(&ps->goca, (HDC)hps, NULL, 0);
            ps->pbGoca          = NULL;
            ps->cbGoca          = 0;
            ps->cbGocaAlloc     = 0;
            ps->lOpenSeg        = 0;
            ps->uOpenSegStart   = 0;
            ps->ulDrawingMode   = DM_DRAW;
            ps->ulEditMode      = SEGEM_INSERT;
            ps->ulInitialSegAttrs =
                SEGATTR_BIT(ATTR_VISIBLE) |
                SEGATTR_BIT(ATTR_CHAINED) |
                SEGATTR_BIT(ATTR_FASTCHAIN) |
                SEGATTR_BIT(ATTR_PROP_DETECTABLE) |
                SEGATTR_BIT(ATTR_PROP_VISIBLE);
            ps->ulDrawControl   =
                (1UL << (DCTL_ERASE - 1)) |
                (1UL << (DCTL_DISPLAY - 1)) |
                (1UL << (DCTL_BOUNDARY - 1)) |
                (1UL << (DCTL_DYNAMIC - 1)) |
                (1UL << (DCTL_CORRELATE - 1));
            ps->ulStopDraw      = SDW_OFF;
            ps->lCurrentElement = 0;
            ps->bInElement      = FALSE;
            ps->uOpenElementStart = 0;
            ps->lOpenPath         = 0;
            /* On client, memset above has zeroed goca; on server,
               GocaInit has re-zeroed it and set hdc. */
#endif
            GocaSetFormatFromOptions(&ps->goca, flOptions);
            return ps;
        }
    }
    return (PGPI_PS)NULL;
}

/*! @brief Find or auto-create a PS entry. */
static inline PGPI_PS GpiPsGet(HPS hps)
{
    PGPI_PS ps = GpiPsFind(hps);
    if (ps) return ps;
    return GpiPsCreate(hps, 0);
}

/*! @brief Update flOptions for an existing PS entry. */
static inline void GpiPsSetOptions(HPS hps, ULONG flOptions)
{
    PGPI_PS ps = GpiPsFind(hps);
    if (ps) {
        ps->flOptions = flOptions;
        GocaSetFormatFromOptions(&ps->goca, flOptions);
    }
}

/*! @brief Release a PS entry. */
static inline void GpiPsDestroy(HPS hps)
{
    PGPI_PS ps = GpiPsFind(hps);
    if (!ps) return;

#ifdef GPI_SERVER
    if (ps->pbGoca) {
        DosFreeMem(ps->pbGoca);
        ps->pbGoca      = NULL;
        ps->cbGoca      = 0;
        ps->cbGocaAlloc = 0;
    }
#endif
    memset(ps, 0, sizeof(*ps));
}

/* --------------------------------------------------------------------- */
/*  Server-only helpers                                                  */
/* --------------------------------------------------------------------- */

#ifdef GPI_SERVER

/*! @brief Append GOCA bytes to ps->pbGoca, growing as needed. */
static inline BOOL GpiPsGocaAppend(PGPI_PS ps, const BYTE *p, ULONG cb)
{
    PVOID pvNew;
    ULONG newAlloc;

    if (!ps || !p || cb == 0) return FALSE;

    if (ps->cbGoca + cb > ps->cbGocaAlloc) {
        newAlloc = ps->cbGocaAlloc ? ps->cbGocaAlloc * 2 : 256;
        while (newAlloc < ps->cbGoca + cb) newAlloc *= 2;
        pvNew = NULL;
        if (DosAllocMem(&pvNew, newAlloc,
                        PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
            return FALSE;
        if (ps->pbGoca && ps->cbGoca) {
            memcpy(pvNew, ps->pbGoca, ps->cbGoca);
            DosFreeMem(ps->pbGoca);
        }
        ps->pbGoca      = (PBYTE)pvNew;
        ps->cbGocaAlloc = newAlloc;
    }
    memcpy(ps->pbGoca + ps->cbGoca, p, cb);
    ps->cbGoca += cb;
    return TRUE;
}

/*!
 * @brief Find [*pStart, *pEnd) byte range of segment lSegId in ps->pbGoca.
 *        A segment starts at its GBSEG and ends at the next GBSEG or
 *        end of the buffer.
 */
static inline BOOL GpiPsGocaFindSeg(PGPI_PS ps, LONG lSegId,
                                    PULONG pStart, PULONG pEnd)
{
    ULONG off, start, end;

    if (!ps || !ps->pbGoca || lSegId <= 0) return FALSE;

    off = 0;
    while (off < ps->cbGoca) {
        const BYTE *p = ps->pbGoca + off;
        ULONG cb = GocaOrderLength(&ps->goca, p, ps->cbGoca - off);
        if (cb == 0) break;

        if (p[0] == OCODE_GBSEG && cb >= 6) {
            ULONG name = GocaGetU32(&ps->goca, p + 2);
            if ((LONG)name == lSegId) {
                start = off;
                end   = ps->cbGoca;
                off  += cb;
                while (off < ps->cbGoca) {
                    const BYTE *q = ps->pbGoca + off;
                    ULONG cq = GocaOrderLength(&ps->goca, q,
                                               ps->cbGoca - off);
                    if (cq == 0) break;
                    if (q[0] == OCODE_GBSEG) { end = off; break; }
                    off += cq;
                }
                if (pStart) *pStart = start;
                if (pEnd)   *pEnd   = end;
                return TRUE;
            }
        }
        off += cb;
    }
    return FALSE;
}

/*! @brief Delete bytes [start,end) from ps->pbGoca. */
static inline BOOL GpiPsGocaDelete(PGPI_PS ps, ULONG start, ULONG end)
{
    ULONG tail;
    if (!ps || !ps->pbGoca) return FALSE;
    if (start > end || end > ps->cbGoca) return FALSE;
    tail = ps->cbGoca - end;
    if (tail > 0)
        memmove(ps->pbGoca + start, ps->pbGoca + end, tail);
    ps->cbGoca -= (end - start);
    return TRUE;
}

#endif /* GPI_SERVER */

#ifdef __cplusplus
}
#endif
#endif /* GPI_PS_H */
