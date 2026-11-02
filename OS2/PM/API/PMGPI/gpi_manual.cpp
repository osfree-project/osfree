/*!
 * @file gpi_manual.c
 * @brief Hand-written client-side PMGPI proxies.
 */
#define INCL_GPI
#include <string.h>
#include "F_platform.h"
#include "F_cmd.h"
#include "gpi_entry.h"

/*!
 * @brief Unified logging function (defined in unimpl.cpp).
 * @param [in] fmt  printf-style format string.
 * @param [in] ...  Arguments for the format string.
 */
extern void log(const char *fmt, ...);

/*!
 * @def GPICallEnter
 * @brief Log the entry of a GPI proxy with its arguments.
 * @param [in] fmt  printf-style format string for the arguments.
 * @param [in] ...  Arguments for the format string.
 */
#define GPICallEnter(fmt, ...)  log("[PMGPI] IN : %s(" fmt ")\n", __FUNCTION__, ##__VA_ARGS__)

/*!
 * @def GPICallResult
 * @brief Log the exit of a GPI proxy with its result.
 * @param [in] fmt  printf-style format string for the result.
 * @param [in] ...  Arguments for the format string.
 */
#define GPICallResult(fmt, ...) log("[PMGPI] OUT: %s -> " fmt "\n", __FUNCTION__, ##__VA_ARGS__)

/*!
 * @def GPI_RESULT_RAW
 * @brief Generic proxy epilogue.
 * @param [in] cast_type  Return type of the enclosing proxy.
 * @param [in] fmt        printf-style format for the result value.
 * @param [in] expr       Expression to pass to @p fmt.
 */
#define GPI_RESULT_RAW(cast_type, fmt, expr) \
    do { \
        if (rc != 0) { \
            GPICallResult("GPI_ERROR (rc=%d)", rc); \
            return (cast_type)GPI_ERROR; \
        } \
        GPICallResult(fmt, expr); \
        return (cast_type)result; \
    } while (0)

/*! @def GPI_RESULT_BOOL @brief Epilogue for @c BOOL. */
#define GPI_RESULT_BOOL()     GPI_RESULT_RAW(BOOL,   "%d",    (int)result)
/*! @def GPI_RESULT_LONG @brief Epilogue for @c LONG. */
#define GPI_RESULT_LONG()     GPI_RESULT_RAW(LONG,   "%ld",   (long)result)
/*! @def GPI_RESULT_ULONG @brief Epilogue for @c ULONG. */
#define GPI_RESULT_ULONG()    GPI_RESULT_RAW(ULONG,  "0x%lx", (unsigned long)result)
/*! @def GPI_RESULT_FIXED @brief Epilogue for @c FIXED. */
#define GPI_RESULT_FIXED()    GPI_RESULT_RAW(FIXED,  "%u",    (unsigned)result)
/*! @def GPI_RESULT_HANDLE @brief Epilogue for a handle type. @param [in] t Handle type. */
#define GPI_RESULT_HANDLE(t)  GPI_RESULT_RAW(t,      "%p",    (void*)result)

/*! @def MAX_STR @brief Max length of a marshaled string (including NUL). */
#define MAX_STR 256

/* ======================================================================== */
/*  Struct tags used by marshaling                                          */
/* ======================================================================== */

typedef struct _GpiCreatePS_Args {
    HAB   hab;
    HDC   hdc;
    SIZEL psizlSize;
    ULONG flOptions;
} GpiCreatePS_Args, *PGpiCreatePS_Args;

typedef struct _GpiErrorSegmentData_Args {
    LONG lSegment;
    LONG lContext;
} GpiErrorSegmentData_Args, *PGpiErrorSegmentData_Args;

typedef struct _GpiCorrelateChain_Args {
    LONG   lType;
    POINTL ptlPick;
    LONG   lMaxHits;
    LONG   lMaxDepth;
    LONG   alHits[1];
} GpiCorrelateChain_Args, *PGpiCorrelateChain_Args;

typedef struct _GpiCorrelateFrom_Args {
    LONG   lFirstSegment;
    LONG   lLastSegment;
    LONG   lType;
    POINTL ptlPick;
    LONG   lMaxHits;
    LONG   lMaxDepth;
    LONG   alSegTag[1];
} GpiCorrelateFrom_Args, *PGpiCorrelateFrom_Args;

typedef struct _GpiCorrelateSegment_Args {
    LONG   lSegment;
    LONG   lType;
    POINTL ptlPick;
    LONG   lMaxHits;
    LONG   lMaxDepth;
    LONG   alSegTag[1];
} GpiCorrelateSegment_Args, *PGpiCorrelateSegment_Args;

typedef struct _GpiQuerySegmentNames_Args {
    LONG lFirstSegid;
    LONG lLastSegid;
    LONG lMax;
    LONG alSegids[1];
} GpiQuerySegmentNames_Args, *PGpiQuerySegmentNames_Args;

typedef struct _GpiGetData_Args {
    LONG lSegid;
    LONG lOffset;
    LONG lFormat;
    LONG lLength;
    BYTE abData[1];
} GpiGetData_Args, *PGpiGetData_Args;

typedef struct _GpiPutData_Args {
    LONG lFormat;
    LONG lCount;
    BYTE abData[1];
} GpiPutData_Args, *PGpiPutData_Args;

typedef struct _GpiBeginElement_Args {
    LONG lType;
    char szDesc[MAX_STR];
} GpiBeginElement_Args, *PGpiBeginElement_Args;

/* ======================================================================== */
/*  Proxies                                                                 */
/* ======================================================================== */

/*!
 * @brief GpiCreatePS.
 * @param [in] hab HAB.
 * @param [in] hdc HDC.
 * @param [in] psizlSize PSIZEL.
 * @param [in] flOptions ULONG.
 * @return Handle, or GPI_ERROR.
 */
HPS APIENTRY GpiCreatePS(HAB hab, HDC hdc, PSIZEL psizlSize, ULONG flOptions)
{
    GpiCreatePS_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hab=%p, hdc=%p, psizlSize={%ld,%ld}, flOptions=0x%lx",
                 (void*)hab, (void*)hdc,
                 psizlSize ? (long)psizlSize->cx : 0L,
                 psizlSize ? (long)psizlSize->cy : 0L,
                 (unsigned long)flOptions);

    if (psizlSize == NULL) {
        GPICallResult("GPI_ERROR (null psizlSize)");
        return (HPS)GPI_ERROR;
    }

    args.hab       = hab;
    args.hdc       = hdc;
    args.psizlSize = *psizlSize;
    args.flOptions = flOptions;

    rc = GpiDispatch((HPS)0, NGpiCreatePS, &args, sizeof(args), &result);
    if (rc == 0 && (ULONG)result != (ULONG)GPI_ERROR && result != 0)
        GpiPsCreate((HPS)result, flOptions);

    GPI_RESULT_HANDLE(HPS);
}

/*!
 * @brief GpiQueryPS.
 * @param [in] hps HPS.
 * @param [out] psizlSize PSIZEL.
 * @return Implementation-defined value.
 */
ULONG APIENTRY GpiQueryPS(HPS hps, PSIZEL psizlSize)
{
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, psizlSize=%p", (void*)hps, (void*)psizlSize);
    rc = GpiDispatch(hps, NGpiQueryPS, psizlSize, sizeof(SIZEL), &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiErrorSegmentData.
 * @param [in] hps HPS.
 * @param [in] plSegment PLONG.
 * @param [in] plContext PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiErrorSegmentData(HPS hps, PLONG plSegment, PLONG plContext)
{
    GpiErrorSegmentData_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, plSegment=%p, plContext=%p",
                 (void*)hps, (void*)plSegment, (void*)plContext);
    args.lSegment = plSegment ? *plSegment : 0;
    args.lContext = plContext ? *plContext : 0;

    rc = GpiDispatch(hps, NGpiErrorSegmentData, &args, sizeof(args), &result);
    if (rc == 0) {
        if (plSegment) *plSegment = args.lSegment;
        if (plContext) *plContext = args.lContext;
    }
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiCorrelateChain.
 * @param [in] hps HPS.
 * @param [in] lType LONG.
 * @param [in] pptlPick PPOINTL.
 * @param [in] lMaxHits LONG.
 * @param [in] lMaxDepth LONG.
 * @param [in] pl2 PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCorrelateChain(HPS hps, LONG lType, PPOINTL pptlPick,
                                LONG lMaxHits, LONG lMaxDepth, PLONG pl2)
{
    PGpiCorrelateChain_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiCorrelateChain_Args) - sizeof(LONG)
                 + sizeof(LONG) * lMaxHits;

    GPICallEnter("hps=%p, lType=%ld, pptlPick=%p, lMaxHits=%ld, lMaxDepth=%ld, pl2=%p",
                 (void*)hps, (long)lType, (void*)pptlPick,
                 (long)lMaxHits, (long)lMaxDepth, (void*)pl2);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiCorrelateChain_Args)pv;

    pArgs->lType     = lType;
    pArgs->ptlPick   = *pptlPick;
    pArgs->lMaxHits  = lMaxHits;
    pArgs->lMaxDepth = lMaxDepth;
    memset(pArgs->alHits, 0, sizeof(LONG) * lMaxHits);

    rc = GpiDispatch(hps, NGpiCorrelateChain, pArgs, cbArgs, &result);
    if (rc == 0 && pl2) memcpy(pl2, pArgs->alHits, sizeof(LONG) * lMaxHits);

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryTag.
 * @param [in] hps HPS.
 * @param [out] plTag PLONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryTag(HPS hps, PLONG plTag)
{
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, plTag=%p", (void*)hps, (void*)plTag);
    rc = GpiDispatch(hps, NGpiQueryTag, plTag, sizeof(LONG), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryPickApertureSize.
 * @param [in] hps HPS.
 * @param [out] psizlSize PSIZEL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryPickApertureSize(HPS hps, PSIZEL psizlSize)
{
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, psizlSize=%p", (void*)hps, (void*)psizlSize);
    rc = GpiDispatch(hps, NGpiQueryPickApertureSize, psizlSize, sizeof(SIZEL), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryPickAperturePosition.
 * @param [in] hps HPS.
 * @param [out] pptlPoint PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryPickAperturePosition(HPS hps, PPOINTL pptlPoint)
{
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, pptlPoint=%p", (void*)hps, (void*)pptlPoint);
    rc = GpiDispatch(hps, NGpiQueryPickAperturePosition, pptlPoint, sizeof(POINTL), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryBoundaryData.
 * @param [in] hps HPS.
 * @param [out] prclBoundary PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryBoundaryData(HPS hps, PRECTL prclBoundary)
{
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, prclBoundary=%p", (void*)hps, (void*)prclBoundary);
    rc = GpiDispatch(hps, NGpiQueryBoundaryData, prclBoundary, sizeof(RECTL), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiCorrelateFrom.
 * @param [in] hps HPS.
 * @param [in] lFirstSegment LONG.
 * @param [in] lLastSegment LONG.
 * @param [in] lType LONG.
 * @param [in] pptlPick PPOINTL.
 * @param [in] lMaxHits LONG.
 * @param [in] lMaxDepth LONG.
 * @param [in] plSegTag PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCorrelateFrom(HPS hps, LONG lFirstSegment, LONG lLastSegment,
                               LONG lType, PPOINTL pptlPick,
                               LONG lMaxHits, LONG lMaxDepth, PLONG plSegTag)
{
    PGpiCorrelateFrom_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiCorrelateFrom_Args) - sizeof(LONG)
                 + sizeof(LONG) * lMaxHits;

    GPICallEnter("hps=%p, lFirstSegment=%ld, lLastSegment=%ld, lType=%ld, pptlPick=%p, lMaxHits=%ld, lMaxDepth=%ld, plSegTag=%p",
                 (void*)hps, (long)lFirstSegment, (long)lLastSegment, (long)lType,
                 (void*)pptlPick, (long)lMaxHits, (long)lMaxDepth, (void*)plSegTag);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiCorrelateFrom_Args)pv;

    pArgs->lFirstSegment = lFirstSegment;
    pArgs->lLastSegment  = lLastSegment;
    pArgs->lType         = lType;
    pArgs->ptlPick       = *pptlPick;
    pArgs->lMaxHits      = lMaxHits;
    pArgs->lMaxDepth     = lMaxDepth;
    memset(pArgs->alSegTag, 0, sizeof(LONG) * lMaxHits);

    rc = GpiDispatch(hps, NGpiCorrelateFrom, pArgs, cbArgs, &result);
    if (rc == 0 && plSegTag) memcpy(plSegTag, pArgs->alSegTag, sizeof(LONG) * lMaxHits);

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiCorrelateSegment.
 * @param [in] hps HPS.
 * @param [in] lSegment LONG.
 * @param [in] lType LONG.
 * @param [in] pptlPick PPOINTL.
 * @param [in] lMaxHits LONG.
 * @param [in] lMaxDepth LONG.
 * @param [in] alSegTag PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCorrelateSegment(HPS hps, LONG lSegment, LONG lType,
                                  PPOINTL pptlPick, LONG lMaxHits, LONG lMaxDepth,
                                  PLONG alSegTag)
{
    PGpiCorrelateSegment_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiCorrelateSegment_Args) - sizeof(LONG)
                 + sizeof(LONG) * lMaxHits;

    GPICallEnter("hps=%p, lSegment=%ld, lType=%ld, pptlPick=%p, lMaxHits=%ld, lMaxDepth=%ld, alSegTag=%p",
                 (void*)hps, (long)lSegment, (long)lType, (void*)pptlPick,
                 (long)lMaxHits, (long)lMaxDepth, (void*)alSegTag);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiCorrelateSegment_Args)pv;

    pArgs->lSegment  = lSegment;
    pArgs->lType     = lType;
    pArgs->ptlPick   = *pptlPick;
    pArgs->lMaxHits  = lMaxHits;
    pArgs->lMaxDepth = lMaxDepth;
    memset(pArgs->alSegTag, 0, sizeof(LONG) * lMaxHits);

    rc = GpiDispatch(hps, NGpiCorrelateSegment, pArgs, cbArgs, &result);
    if (rc == 0 && alSegTag) memcpy(alSegTag, pArgs->alSegTag, sizeof(LONG) * lMaxHits);

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQuerySegmentNames.
 * @param [in] hps HPS.
 * @param [in] lFirstSegid LONG.
 * @param [in] lLastSegid LONG.
 * @param [in] lMax LONG.
 * @param [out] alSegids PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQuerySegmentNames(HPS hps, LONG lFirstSegid, LONG lLastSegid,
                                   LONG lMax, PLONG alSegids)
{
    PGpiQuerySegmentNames_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQuerySegmentNames_Args) - sizeof(LONG)
                 + sizeof(LONG) * lMax;

    GPICallEnter("hps=%p, lFirstSegid=%ld, lLastSegid=%ld, lMax=%ld, alSegids=%p",
                 (void*)hps, (long)lFirstSegid, (long)lLastSegid, (long)lMax,
                 (void*)alSegids);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiQuerySegmentNames_Args)pv;

    pArgs->lFirstSegid = lFirstSegid;
    pArgs->lLastSegid  = lLastSegid;
    pArgs->lMax        = lMax;
    memset(pArgs->alSegids, 0, sizeof(LONG) * lMax);

    rc = GpiDispatch(hps, NGpiQuerySegmentNames, pArgs, cbArgs, &result);
    if (rc == 0 && alSegids) memcpy(alSegids, pArgs->alSegids, sizeof(LONG) * lMax);

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiGetData.
 * @param [in] hps HPS.
 * @param [in] lSegid LONG.
 * @param [out] plOffset PLONG.
 * @param [in] lFormat LONG.
 * @param [in] lLength LONG.
 * @param [out] pbData PBYTE.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiGetData(HPS hps, LONG lSegid, PLONG plOffset, LONG lFormat,
                         LONG lLength, PBYTE pbData)
{
    PGpiGetData_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiGetData_Args) - 1 + lLength;

    GPICallEnter("hps=%p, lSegid=%ld, plOffset=%p, lFormat=%ld, lLength=%ld, pbData=%p",
                 (void*)hps, (long)lSegid, (void*)plOffset, (long)lFormat,
                 (long)lLength, (void*)pbData);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiGetData_Args)pv;

    pArgs->lSegid  = lSegid;
    pArgs->lOffset = plOffset ? *plOffset : 0;
    pArgs->lFormat = lFormat;
    pArgs->lLength = lLength;
    memset(pArgs->abData, 0, lLength);

    rc = GpiDispatch(hps, NGpiGetData, pArgs, cbArgs, &result);
    if (rc == 0) {
        if (plOffset) *plOffset = pArgs->lOffset;
        if (pbData)   memcpy(pbData, pArgs->abData, lLength);
    }

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiPutData.
 * @param [in] hps HPS.
 * @param [in] lFormat LONG.
 * @param [in] plCount PLONG.
 * @param [in] pbData PBYTE.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiPutData(HPS hps, LONG lFormat, PLONG plCount, PBYTE pbData)
{
    PGpiPutData_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG count  = plCount ? *plCount : 0;
    ULONG cbArgs = sizeof(GpiPutData_Args) - 1 + count;

    GPICallEnter("hps=%p, lFormat=%ld, plCount=%p, pbData=%p",
                 (void*)hps, (long)lFormat, (void*)plCount, (void*)pbData);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiPutData_Args)pv;

    pArgs->lFormat = lFormat;
    pArgs->lCount  = count;
    if (pbData && count) memcpy(pArgs->abData, pbData, count);

    rc = GpiDispatch(hps, NGpiPutData, pArgs, cbArgs, &result);

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiBeginElement.
 * @param [in] hps HPS.
 * @param [in] lType LONG.
 * @param [in] pszDesc PCSZ.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiBeginElement(HPS hps, LONG lType, PCSZ pszDesc)
{
    GpiBeginElement_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, lType=%ld, pszDesc=%p", (void*)hps, (long)lType, (void*)pszDesc);
    args.lType = lType;
    if (pszDesc) strncpy(args.szDesc, pszDesc, MAX_STR - 1);
    else         args.szDesc[0] = '\0';
    args.szDesc[MAX_STR - 1] = '\0';

    rc = GpiDispatch(hps, NGpiBeginElement, &args, sizeof(args), &result);
    GPI_RESULT_BOOL();
}

/* ======================================================================== */
/*  Struct tags used by marshaling (Part 2)                                 */
/* ======================================================================== */

typedef struct _GpiElement_Args {
    LONG lType;
    char szDesc[MAX_STR];
    LONG lLength;
    BYTE abData[1];
} GpiElement_Args, *PGpiElement_Args;

typedef struct _GpiQueryElement_Args {
    LONG lOff;
    LONG lMaxLength;
    BYTE abData[1];
} GpiQueryElement_Args, *PGpiQueryElement_Args;

typedef struct _GpiQueryElementType_Args {
    LONG lType;
    LONG lLength;
    char szData[MAX_STR];
} GpiQueryElementType_Args, *PGpiQueryElementType_Args;

typedef struct _GpiQuerySegmentTransformMatrix_Args {
    LONG lSegid;
    LONG lCount;
    MATRIXLF aMatlf[1];
} GpiQuerySegmentTransformMatrix_Args, *PGpiQuerySegmentTransformMatrix_Args;

typedef struct _GpiSetSegmentTransformMatrix_Args {
    LONG lSegid;
    LONG lCount;
    LONG lOptions;
    MATRIXLF aMatlf[1];
} GpiSetSegmentTransformMatrix_Args, *PGpiSetSegmentTransformMatrix_Args;

typedef struct _GpiConvert_Args {
    LONG lSrc;
    LONG lTarg;
    LONG lCount;
    POINTL aptl[1];
} GpiConvert_Args, *PGpiConvert_Args;

typedef struct _GpiConvertWithMatrix_Args {
    LONG lCountp;
    LONG lCount;
    MATRIXLF matlf;
    POINTL aptl[1];
} GpiConvertWithMatrix_Args, *PGpiConvertWithMatrix_Args;

typedef struct _GpiQueryModelTransformMatrix_Args {
    LONG lCount;
    MATRIXLF aMatlf[1];
} GpiQueryModelTransformMatrix_Args, *PGpiQueryModelTransformMatrix_Args;

typedef struct _GpiSetModelTransformMatrix_Args {
    LONG lCount;
    LONG lOptions;
    MATRIXLF aMatlf[1];
} GpiSetModelTransformMatrix_Args, *PGpiSetModelTransformMatrix_Args;

typedef struct _GpiCallSegmentMatrix_Args {
    LONG lSegment;
    LONG lCount;
    LONG lOptions;
    MATRIXLF aMatlf[1];
} GpiCallSegmentMatrix_Args, *PGpiCallSegmentMatrix_Args;

typedef struct _GpiQueryDefaultViewMatrix_Args {
    LONG lCount;
    MATRIXLF aMatlf[1];
} GpiQueryDefaultViewMatrix_Args, *PGpiQueryDefaultViewMatrix_Args;

typedef struct _GpiSetDefaultViewMatrix_Args {
    LONG lCount;
    LONG lOptions;
    MATRIXLF aMatlf[1];
} GpiSetDefaultViewMatrix_Args, *PGpiSetDefaultViewMatrix_Args;

/* ======================================================================== */
/*  Proxies (Part 2)                                                        */
/* ======================================================================== */

/*!
 * @brief GpiElement.
 * @param [in] hps HPS.
 * @param [in] lType LONG.
 * @param [in] pszDesc PCSZ.
 * @param [in] lLength LONG.
 * @param [in] pbData PBYTE.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiElement(HPS hps, LONG lType, PCSZ pszDesc, LONG lLength, PBYTE pbData)
{
    PGpiElement_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiElement_Args) - 1 + lLength;

    GPICallEnter("hps=%p, lType=%ld, pszDesc=%p, lLength=%ld, pbData=%p",
                 (void*)hps, (long)lType, (void*)pszDesc, (long)lLength, (void*)pbData);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiElement_Args)pv;

    pArgs->lType = lType;
    if (pszDesc) strncpy(pArgs->szDesc, pszDesc, MAX_STR - 1);
    else         pArgs->szDesc[0] = '\0';
    pArgs->szDesc[MAX_STR - 1] = '\0';
    pArgs->lLength = lLength;
    if (pbData && lLength) memcpy(pArgs->abData, pbData, lLength);

    rc = GpiDispatch(hps, NGpiElement, pArgs, cbArgs, &result);

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryElement.
 * @param [in] hps HPS.
 * @param [in] lOff LONG.
 * @param [in] lMaxLength LONG.
 * @param [out] pbData PBYTE.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryElement(HPS hps, LONG lOff, LONG lMaxLength, PBYTE pbData)
{
    PGpiQueryElement_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryElement_Args) - 1 + lMaxLength;

    GPICallEnter("hps=%p, lOff=%ld, lMaxLength=%ld, pbData=%p",
                 (void*)hps, (long)lOff, (long)lMaxLength, (void*)pbData);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiQueryElement_Args)pv;

    pArgs->lOff = lOff;
    pArgs->lMaxLength = lMaxLength;
    memset(pArgs->abData, 0, lMaxLength);

    rc = GpiDispatch(hps, NGpiQueryElement, pArgs, cbArgs, &result);
    if (rc == 0 && pbData && lMaxLength > 0) {
        memcpy(pbData, pArgs->abData, lMaxLength);
    }

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryElementType.
 * @param [in] hps HPS.
 * @param [out] plType PLONG.
 * @param [in] lLength LONG.
 * @param [in] pszData PCSZ.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryElementType(HPS hps, PLONG plType, LONG lLength, PCSZ pszData)
{
    GpiQueryElementType_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, plType=%p, lLength=%ld, pszData=%p",
                 (void*)hps, (void*)plType, (long)lLength, (void*)pszData);

    args.lType = 0;
    args.lLength = lLength;
    if (pszData) strncpy(args.szData, pszData, MAX_STR - 1);
    else         args.szData[0] = '\0';
    args.szData[MAX_STR - 1] = '\0';

    rc = GpiDispatch(hps, NGpiQueryElementType, &args, sizeof(args), &result);
    if (rc == 0 && plType) *plType = args.lType;

    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQuerySegmentTransformMatrix.
 * @param [in] hps HPS.
 * @param [in] lSegid LONG.
 * @param [in] lCount LONG.
 * @param [out] pmatlfArray PMATRIXLF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQuerySegmentTransformMatrix(HPS hps, LONG lSegid, LONG lCount, PMATRIXLF pmatlfArray)
{
    PGpiQuerySegmentTransformMatrix_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQuerySegmentTransformMatrix_Args) - sizeof(MATRIXLF)
                 + sizeof(MATRIXLF) * lCount;

    GPICallEnter("hps=%p, lSegid=%ld, lCount=%ld, pmatlfArray=%p",
                 (void*)hps, (long)lSegid, (long)lCount, (void*)pmatlfArray);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQuerySegmentTransformMatrix_Args)pv;

    pArgs->lSegid = lSegid;
    pArgs->lCount = lCount;
    memset(pArgs->aMatlf, 0, sizeof(MATRIXLF) * lCount);

    rc = GpiDispatch(hps, NGpiQuerySegmentTransformMatrix, pArgs, cbArgs, &result);
    if (rc == 0 && pmatlfArray) {
        memcpy(pmatlfArray, pArgs->aMatlf, sizeof(MATRIXLF) * lCount);
    }

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiSetSegmentTransformMatrix.
 * @param [in] hps HPS.
 * @param [in] lSegid LONG.
 * @param [in] lCount LONG.
 * @param [in] pmatlfarray PMATRIXLF.
 * @param [in] lOptions LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetSegmentTransformMatrix(HPS hps, LONG lSegid, LONG lCount, PMATRIXLF pmatlfarray, LONG lOptions)
{
    PGpiSetSegmentTransformMatrix_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiSetSegmentTransformMatrix_Args) - sizeof(MATRIXLF)
                 + sizeof(MATRIXLF) * lCount;

    GPICallEnter("hps=%p, lSegid=%ld, lCount=%ld, pmatlfarray=%p, lOptions=%ld",
                 (void*)hps, (long)lSegid, (long)lCount, (void*)pmatlfarray, (long)lOptions);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiSetSegmentTransformMatrix_Args)pv;

    pArgs->lSegid = lSegid;
    pArgs->lCount = lCount;
    pArgs->lOptions = lOptions;
    if (pmatlfarray) {
        memcpy(pArgs->aMatlf, pmatlfarray, sizeof(MATRIXLF) * lCount);
    } else {
        memset(pArgs->aMatlf, 0, sizeof(MATRIXLF) * lCount);
    }

    rc = GpiDispatch(hps, NGpiSetSegmentTransformMatrix, pArgs, cbArgs, &result);

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiConvert.
 * @param [in] hps HPS.
 * @param [in] lSrc LONG.
 * @param [in] lTarg LONG.
 * @param [in] lCount LONG.
 * @param [in] aptlPoints PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiConvert(HPS hps, LONG lSrc, LONG lTarg, LONG lCount, PPOINTL aptlPoints)
{
    PGpiConvert_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiConvert_Args) - sizeof(POINTL)
                 + sizeof(POINTL) * lCount;

    GPICallEnter("hps=%p, lSrc=%ld, lTarg=%ld, lCount=%ld, aptlPoints=%p",
                 (void*)hps, (long)lSrc, (long)lTarg, (long)lCount, (void*)aptlPoints);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiConvert_Args)pv;

    pArgs->lSrc = lSrc;
    pArgs->lTarg = lTarg;
    pArgs->lCount = lCount;
    if (aptlPoints) {
        memcpy(pArgs->aptl, aptlPoints, sizeof(POINTL) * lCount);
    } else {
        memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);
    }

    rc = GpiDispatch(hps, NGpiConvert, pArgs, cbArgs, &result);
    if (rc == 0 && aptlPoints) {
        memcpy(aptlPoints, pArgs->aptl, sizeof(POINTL) * lCount);
    }

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiConvertWithMatrix.
 * @param [in] hps HPS.
 * @param [in] lCountp LONG.
 * @param [in] aptlPoints PPOINTL.
 * @param [in] lCount LONG.
 * @param [in] pmatlfArray PMATRIXLF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiConvertWithMatrix(HPS hps, LONG lCountp, PPOINTL aptlPoints, LONG lCount, PMATRIXLF pmatlfArray)
{
    PGpiConvertWithMatrix_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiConvertWithMatrix_Args) - sizeof(POINTL)
                 + sizeof(POINTL) * lCountp;

    GPICallEnter("hps=%p, lCountp=%ld, aptlPoints=%p, lCount=%ld, pmatlfArray=%p",
                 (void*)hps, (long)lCountp, (void*)aptlPoints, (long)lCount, (void*)pmatlfArray);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiConvertWithMatrix_Args)pv;

    pArgs->lCountp = lCountp;
    pArgs->lCount = lCount;
    if (pmatlfArray) pArgs->matlf = *pmatlfArray;
    else             memset(&pArgs->matlf, 0, sizeof(MATRIXLF));

    if (aptlPoints) {
        memcpy(pArgs->aptl, aptlPoints, sizeof(POINTL) * lCountp);
    } else {
        memset(pArgs->aptl, 0, sizeof(POINTL) * lCountp);
    }

    rc = GpiDispatch(hps, NGpiConvertWithMatrix, pArgs, cbArgs, &result);
    if (rc == 0 && aptlPoints) {
        memcpy(aptlPoints, pArgs->aptl, sizeof(POINTL) * lCountp);
    }

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryModelTransformMatrix.
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [out] pmatlfArray PMATRIXLF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryModelTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray)
{
    PGpiQueryModelTransformMatrix_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryModelTransformMatrix_Args) - sizeof(MATRIXLF)
                 + sizeof(MATRIXLF) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, pmatlfArray=%p",
                 (void*)hps, (long)lCount, (void*)pmatlfArray);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryModelTransformMatrix_Args)pv;

    pArgs->lCount = lCount;
    memset(pArgs->aMatlf, 0, sizeof(MATRIXLF) * lCount);

    rc = GpiDispatch(hps, NGpiQueryModelTransformMatrix, pArgs, cbArgs, &result);
    if (rc == 0 && pmatlfArray) {
        memcpy(pmatlfArray, pArgs->aMatlf, sizeof(MATRIXLF) * lCount);
    }

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiSetModelTransformMatrix.
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] pmatlfArray PMATRIXLF.
 * @param [in] lOptions LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetModelTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray, LONG lOptions)
{
    PGpiSetModelTransformMatrix_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiSetModelTransformMatrix_Args) - sizeof(MATRIXLF)
                 + sizeof(MATRIXLF) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, pmatlfArray=%p, lOptions=%ld",
                 (void*)hps, (long)lCount, (void*)pmatlfArray, (long)lOptions);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiSetModelTransformMatrix_Args)pv;

    pArgs->lCount = lCount;
    pArgs->lOptions = lOptions;
    if (pmatlfArray) {
        memcpy(pArgs->aMatlf, pmatlfArray, sizeof(MATRIXLF) * lCount);
    } else {
        memset(pArgs->aMatlf, 0, sizeof(MATRIXLF) * lCount);
    }

    rc = GpiDispatch(hps, NGpiSetModelTransformMatrix, pArgs, cbArgs, &result);

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiCallSegmentMatrix.
 * @param [in] hps HPS.
 * @param [in] lSegment LONG.
 * @param [in] lCount LONG.
 * @param [in] pmatlfArray PMATRIXLF.
 * @param [in] lOptions LONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiCallSegmentMatrix(HPS hps, LONG lSegment, LONG lCount, PMATRIXLF pmatlfArray, LONG lOptions)
{
    PGpiCallSegmentMatrix_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiCallSegmentMatrix_Args) - sizeof(MATRIXLF)
                 + sizeof(MATRIXLF) * lCount;

    GPICallEnter("hps=%p, lSegment=%ld, lCount=%ld, pmatlfArray=%p, lOptions=%ld",
                 (void*)hps, (long)lSegment, (long)lCount, (void*)pmatlfArray, (long)lOptions);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiCallSegmentMatrix_Args)pv;

    pArgs->lSegment = lSegment;
    pArgs->lCount = lCount;
    pArgs->lOptions = lOptions;
    if (pmatlfArray) {
        memcpy(pArgs->aMatlf, pmatlfArray, sizeof(MATRIXLF) * lCount);
    } else {
        memset(pArgs->aMatlf, 0, sizeof(MATRIXLF) * lCount);
    }

    rc = GpiDispatch(hps, NGpiCallSegmentMatrix, pArgs, cbArgs, &result);

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryDefaultViewMatrix.
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [out] pmatlfArray PMATRIXLF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryDefaultViewMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray)
{
    PGpiQueryDefaultViewMatrix_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryDefaultViewMatrix_Args) - sizeof(MATRIXLF)
                 + sizeof(MATRIXLF) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, pmatlfArray=%p",
                 (void*)hps, (long)lCount, (void*)pmatlfArray);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryDefaultViewMatrix_Args)pv;

    pArgs->lCount = lCount;
    memset(pArgs->aMatlf, 0, sizeof(MATRIXLF) * lCount);

    rc = GpiDispatch(hps, NGpiQueryDefaultViewMatrix, pArgs, cbArgs, &result);
    if (rc == 0 && pmatlfArray) {
        memcpy(pmatlfArray, pArgs->aMatlf, sizeof(MATRIXLF) * lCount);
    }

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiSetDefaultViewMatrix.
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] pmatlfarray PMATRIXLF.
 * @param [in] lOptions LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetDefaultViewMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfarray, LONG lOptions)
{
    PGpiSetDefaultViewMatrix_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiSetDefaultViewMatrix_Args) - sizeof(MATRIXLF)
                 + sizeof(MATRIXLF) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, pmatlfarray=%p, lOptions=%ld",
                 (void*)hps, (long)lCount, (void*)pmatlfarray, (long)lOptions);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiSetDefaultViewMatrix_Args)pv;

    pArgs->lCount = lCount;
    pArgs->lOptions = lOptions;
    if (pmatlfarray) {
        memcpy(pArgs->aMatlf, pmatlfarray, sizeof(MATRIXLF) * lCount);
    } else {
        memset(pArgs->aMatlf, 0, sizeof(MATRIXLF) * lCount);
    }

    rc = GpiDispatch(hps, NGpiSetDefaultViewMatrix, pArgs, cbArgs, &result);

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/* ======================================================================== */
/*  Struct tags used by marshaling (Part 3)                                 */
/* ======================================================================== */

typedef struct _GpiQueryViewingTransformMatrix_Args {
    LONG lCount;
    MATRIXLF aMatlf[1];
} GpiQueryViewingTransformMatrix_Args, *PGpiQueryViewingTransformMatrix_Args;

typedef struct _GpiSetViewingTransformMatrix_Args {
    LONG lCount;
    LONG lOptions;
    MATRIXLF aMatlf[1];
} GpiSetViewingTransformMatrix_Args, *PGpiSetViewingTransformMatrix_Args;

typedef struct _GpiTranslate_Args {
    MATRIXLF matlf;
    LONG lOptions;
    POINTL ptl;
} GpiTranslate_Args, *PGpiTranslate_Args;

typedef struct _GpiScale_Args {
    MATRIXLF matlf;
    LONG lOptions;
    FIXED afx[2];
    POINTL ptl;
} GpiScale_Args, *PGpiScale_Args;

typedef struct _GpiRotate_Args {
    MATRIXLF matlf;
    LONG lOptions;
    FIXED fxAngle;
    POINTL ptl;
} GpiRotate_Args, *PGpiRotate_Args;

/* ======================================================================== */
/*  Proxies (Part 3)                                                        */
/* ======================================================================== */

/*!
 * @brief GpiQueryPageViewport.
 * @param [in] hps HPS.
 * @param [out] prclViewport PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryPageViewport(HPS hps, PRECTL prclViewport)
{
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, prclViewport=%p", (void*)hps, (void*)prclViewport);
    rc = GpiDispatch(hps, NGpiQueryPageViewport, prclViewport, sizeof(RECTL), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryViewingTransformMatrix.
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [out] pmatlfArray PMATRIXLF.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryViewingTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray)
{
    PGpiQueryViewingTransformMatrix_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryViewingTransformMatrix_Args) - sizeof(MATRIXLF)
                 + sizeof(MATRIXLF) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, pmatlfArray=%p",
                 (void*)hps, (long)lCount, (void*)pmatlfArray);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryViewingTransformMatrix_Args)pv;

    pArgs->lCount = lCount;
    memset(pArgs->aMatlf, 0, sizeof(MATRIXLF) * lCount);

    rc = GpiDispatch(hps, NGpiQueryViewingTransformMatrix, pArgs, cbArgs, &result);
    if (rc == 0 && pmatlfArray) {
        memcpy(pmatlfArray, pArgs->aMatlf, sizeof(MATRIXLF) * lCount);
    }

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiSetViewingTransformMatrix.
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [in] pmatlfArray PMATRIXLF.
 * @param [in] lOptions LONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetViewingTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray, LONG lOptions)
{
    PGpiSetViewingTransformMatrix_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiSetViewingTransformMatrix_Args) - sizeof(MATRIXLF)
                 + sizeof(MATRIXLF) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, pmatlfArray=%p, lOptions=%ld",
                 (void*)hps, (long)lCount, (void*)pmatlfArray, (long)lOptions);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiSetViewingTransformMatrix_Args)pv;

    pArgs->lCount = lCount;
    pArgs->lOptions = lOptions;
    if (pmatlfArray) {
        memcpy(pArgs->aMatlf, pmatlfArray, sizeof(MATRIXLF) * lCount);
    } else {
        memset(pArgs->aMatlf, 0, sizeof(MATRIXLF) * lCount);
    }

    rc = GpiDispatch(hps, NGpiSetViewingTransformMatrix, pArgs, cbArgs, &result);

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiTranslate.
 * @param [in] hps HPS.
 * @param [in,out] pmatlf PMATRIXLF.
 * @param [in] lOptions LONG.
 * @param [in] pptl PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiTranslate(HPS hps, PMATRIXLF pmatlf, LONG lOptions, PPOINTL pptl)
{
    GpiTranslate_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, pmatlf=%p, lOptions=%ld, pptl=%p",
                 (void*)hps, (void*)pmatlf, (long)lOptions, (void*)pptl);

    if (pmatlf) args.matlf = *pmatlf;
    else        memset(&args.matlf, 0, sizeof(MATRIXLF));
    args.lOptions = lOptions;
    if (pptl) args.ptl = *pptl;
    else      { args.ptl.x = 0; args.ptl.y = 0; }

    rc = GpiDispatch(hps, NGpiTranslate, &args, sizeof(args), &result);
    if (rc == 0 && pmatlf) *pmatlf = args.matlf;

    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiScale.
 * @param [in] hps HPS.
 * @param [in,out] pmatlf PMATRIXLF.
 * @param [in] lOptions LONG.
 * @param [in] afx PFIXED.
 * @param [in] pptl PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiScale(HPS hps, PMATRIXLF pmatlf, LONG lOptions, PFIXED afx, PPOINTL pptl)
{
    GpiScale_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, pmatlf=%p, lOptions=%ld, afx=%p, pptl=%p",
                 (void*)hps, (void*)pmatlf, (long)lOptions, (void*)afx, (void*)pptl);

    if (pmatlf) args.matlf = *pmatlf;
    else        memset(&args.matlf, 0, sizeof(MATRIXLF));
    args.lOptions = lOptions;
    if (afx) {
        args.afx[0] = afx[0];
        args.afx[1] = afx[1];
    } else {
        args.afx[0] = 0;
        args.afx[1] = 0;
    }
    if (pptl) args.ptl = *pptl;
    else      { args.ptl.x = 0; args.ptl.y = 0; }

    rc = GpiDispatch(hps, NGpiScale, &args, sizeof(args), &result);
    if (rc == 0 && pmatlf) *pmatlf = args.matlf;

    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiRotate.
 * @param [in] hps HPS.
 * @param [in,out] pmatlf PMATRIXLF.
 * @param [in] lOptions LONG.
 * @param [in] fxAngle FIXED.
 * @param [in] pptl PPOINTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiRotate(HPS hps, PMATRIXLF pmatlf, LONG lOptions, FIXED fxAngle, PPOINTL pptl)
{
    GpiRotate_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, pmatlf=%p, lOptions=%ld, fxAngle=%u, pptl=%p",
                 (void*)hps, (void*)pmatlf, (long)lOptions, (unsigned)fxAngle, (void*)pptl);

    if (pmatlf) args.matlf = *pmatlf;
    else        memset(&args.matlf, 0, sizeof(MATRIXLF));
    args.lOptions = lOptions;
    args.fxAngle = fxAngle;
    if (pptl) args.ptl = *pptl;
    else      { args.ptl.x = 0; args.ptl.y = 0; }

    rc = GpiDispatch(hps, NGpiRotate, &args, sizeof(args), &result);
    if (rc == 0 && pmatlf) *pmatlf = args.matlf;

    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryGraphicsField.
 * @param [in] hps HPS.
 * @param [out] prclField PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryGraphicsField(HPS hps, PRECTL prclField)
{
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, prclField=%p", (void*)hps, (void*)prclField);
    rc = GpiDispatch(hps, NGpiQueryGraphicsField, prclField, sizeof(RECTL), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryViewingLimits.
 * @param [in] hps HPS.
 * @param [out] prclLimits PRECTL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryViewingLimits(HPS hps, PRECTL prclLimits)
{
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, prclLimits=%p", (void*)hps, (void*)prclLimits);
    rc = GpiDispatch(hps, NGpiQueryViewingLimits, prclLimits, sizeof(RECTL), &result);
    GPI_RESULT_BOOL();
}

/* ======================================================================== */
/*  Struct tags used by marshaling (Part 4)                                 */
/* ======================================================================== */

typedef struct _GpiCreateLogColorTable_Args {
    ULONG flOptions;
    LONG  lFormat;
    LONG  lStart;
    LONG  lCount;
    LONG  alTable[1];
} GpiCreateLogColorTable_Args, *PGpiCreateLogColorTable_Args;

typedef struct _GpiQueryColorData_Args {
    LONG lCount;
    LONG aData[1];
} GpiQueryColorData_Args, *PGpiQueryColorData_Args;

typedef struct _GpiQueryLogColorTable_Args {
    ULONG flOptions;
    LONG  lStart;
    LONG  lCount;
    LONG  aData[1];
} GpiQueryLogColorTable_Args, *PGpiQueryLogColorTable_Args;

typedef struct _GpiQueryRealColors_Args {
    ULONG flOptions;
    LONG  lStart;
    LONG  lCount;
    LONG  aColors[1];
} GpiQueryRealColors_Args, *PGpiQueryRealColors_Args;

typedef struct _GpiCreatePalette_Args {
    ULONG flOptions;
    ULONG ulFormat;
    ULONG ulCount;
    ULONG aTable[1];
} GpiCreatePalette_Args, *PGpiCreatePalette_Args;

typedef struct _GpiAnimatePalette_Args {
    ULONG ulFormat;
    ULONG ulStart;
    ULONG ulCount;
    ULONG aTable[1];
} GpiAnimatePalette_Args, *PGpiAnimatePalette_Args;

typedef struct _GpiSetPaletteEntries_Args {
    ULONG ulFormat;
    ULONG ulStart;
    ULONG ulCount;
    ULONG aTable[1];
} GpiSetPaletteEntries_Args, *PGpiSetPaletteEntries_Args;

typedef struct _GpiQueryPaletteInfo_Args {
    ULONG flOptions;
    ULONG ulStart;
    ULONG ulCount;
    ULONG aData[1];
} GpiQueryPaletteInfo_Args, *PGpiQueryPaletteInfo_Args;

/* ======================================================================== */
/*  Proxies (Part 4)                                                        */
/* ======================================================================== */

/*!
 * @brief GpiCreateLogColorTable.
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] lFormat LONG.
 * @param [in] lStart LONG.
 * @param [in] lCount LONG.
 * @param [in] alTable PLONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiCreateLogColorTable(HPS hps, ULONG flOptions, LONG lFormat, LONG lStart, LONG lCount, PLONG alTable)
{
    PGpiCreateLogColorTable_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiCreateLogColorTable_Args) - sizeof(LONG)
                 + sizeof(LONG) * lCount;

    GPICallEnter("hps=%p, flOptions=0x%lx, lFormat=%ld, lStart=%ld, lCount=%ld, alTable=%p",
                 (void*)hps, (unsigned long)flOptions, (long)lFormat, (long)lStart,
                 (long)lCount, (void*)alTable);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiCreateLogColorTable_Args)pv;

    pArgs->flOptions = flOptions;
    pArgs->lFormat   = lFormat;
    pArgs->lStart    = lStart;
    pArgs->lCount    = lCount;
    if (alTable) memcpy(pArgs->alTable, alTable, sizeof(LONG) * lCount);
    else         memset(pArgs->alTable, 0, sizeof(LONG) * lCount);

    rc = GpiDispatch(hps, NGpiCreateLogColorTable, pArgs, cbArgs, &result);

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryColorData.
 * @param [in] hps HPS.
 * @param [in] lCount LONG.
 * @param [out] alArray PLONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiQueryColorData(HPS hps, LONG lCount, PLONG alArray)
{
    PGpiQueryColorData_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryColorData_Args) - sizeof(LONG)
                 + sizeof(LONG) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, alArray=%p", (void*)hps, (long)lCount, (void*)alArray);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryColorData_Args)pv;

    pArgs->lCount = lCount;
    memset(pArgs->aData, 0, sizeof(LONG) * lCount);

    rc = GpiDispatch(hps, NGpiQueryColorData, pArgs, cbArgs, &result);
    if (rc == 0 && alArray) memcpy(alArray, pArgs->aData, sizeof(LONG) * lCount);

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryLogColorTable.
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] lStart LONG.
 * @param [in] lCount LONG.
 * @param [out] alArray PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryLogColorTable(HPS hps, ULONG flOptions, LONG lStart, LONG lCount, PLONG alArray)
{
    PGpiQueryLogColorTable_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryLogColorTable_Args) - sizeof(LONG)
                 + sizeof(LONG) * lCount;

    GPICallEnter("hps=%p, flOptions=0x%lx, lStart=%ld, lCount=%ld, alArray=%p",
                 (void*)hps, (unsigned long)flOptions, (long)lStart, (long)lCount,
                 (void*)alArray);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiQueryLogColorTable_Args)pv;

    pArgs->flOptions = flOptions;
    pArgs->lStart    = lStart;
    pArgs->lCount    = lCount;
    memset(pArgs->aData, 0, sizeof(LONG) * lCount);

    rc = GpiDispatch(hps, NGpiQueryLogColorTable, pArgs, cbArgs, &result);
    if (rc == 0 && alArray) memcpy(alArray, pArgs->aData, sizeof(LONG) * lCount);

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryRealColors.
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] lStart LONG.
 * @param [in] lCount LONG.
 * @param [out] alColors PLONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryRealColors(HPS hps, ULONG flOptions, LONG lStart, LONG lCount, PLONG alColors)
{
    PGpiQueryRealColors_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryRealColors_Args) - sizeof(LONG)
                 + sizeof(LONG) * lCount;

    GPICallEnter("hps=%p, flOptions=0x%lx, lStart=%ld, lCount=%ld, alColors=%p",
                 (void*)hps, (unsigned long)flOptions, (long)lStart, (long)lCount,
                 (void*)alColors);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiQueryRealColors_Args)pv;

    pArgs->flOptions = flOptions;
    pArgs->lStart    = lStart;
    pArgs->lCount    = lCount;
    memset(pArgs->aColors, 0, sizeof(LONG) * lCount);

    rc = GpiDispatch(hps, NGpiQueryRealColors, pArgs, cbArgs, &result);
    if (rc == 0 && alColors) memcpy(alColors, pArgs->aColors, sizeof(LONG) * lCount);

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiCreatePalette.
 * @param [in] hab HAB.
 * @param [in] flOptions ULONG.
 * @param [in] ulFormat ULONG.
 * @param [in] ulCount ULONG.
 * @param [in] aulTable PULONG.
 * @return Handle, or GPI_ERROR.
 */
HPAL APIENTRY GpiCreatePalette(HAB hab, ULONG flOptions, ULONG ulFormat, ULONG ulCount, PULONG aulTable)
{
    PGpiCreatePalette_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiCreatePalette_Args) - sizeof(ULONG)
                 + sizeof(ULONG) * ulCount;

    GPICallEnter("hab=%p, flOptions=0x%lx, ulFormat=0x%lx, ulCount=%lu, aulTable=%p",
                 (void*)hab, (unsigned long)flOptions, (unsigned long)ulFormat,
                 (unsigned long)ulCount, (void*)aulTable);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (HPAL)GPI_ERROR;
    }
    pArgs = (PGpiCreatePalette_Args)pv;

    pArgs->flOptions = flOptions;
    pArgs->ulFormat  = ulFormat;
    pArgs->ulCount   = ulCount;
    if (aulTable) memcpy(pArgs->aTable, aulTable, sizeof(ULONG) * ulCount);
    else          memset(pArgs->aTable, 0, sizeof(ULONG) * ulCount);

    rc = GpiDispatch((HPS)hab, NGpiCreatePalette, pArgs, cbArgs, &result);

    DosFreeMem(pv);
    GPI_RESULT_HANDLE(HPAL);
}

/*!
 * @brief GpiDeletePalette.
 * @param [in] hpal HPAL.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiDeletePalette(HPAL hpal)
{
    ULONG result;
    int   rc;

    GPICallEnter("hpal=%p", (void*)hpal);
    rc = GpiDispatch((HPS)hpal, NGpiDeletePalette, NULL, 0, &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiAnimatePalette.
 * @param [in] hpal HPAL.
 * @param [in] ulFormat ULONG.
 * @param [in] ulStart ULONG.
 * @param [in] ulCount ULONG.
 * @param [in] aulTable PULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiAnimatePalette(HPAL hpal, ULONG ulFormat, ULONG ulStart, ULONG ulCount, PULONG aulTable)
{
    PGpiAnimatePalette_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiAnimatePalette_Args) - sizeof(ULONG)
                 + sizeof(ULONG) * ulCount;

    GPICallEnter("hpal=%p, ulFormat=0x%lx, ulStart=%lu, ulCount=%lu, aulTable=%p",
                 (void*)hpal, (unsigned long)ulFormat, (unsigned long)ulStart,
                 (unsigned long)ulCount, (void*)aulTable);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiAnimatePalette_Args)pv;

    pArgs->ulFormat = ulFormat;
    pArgs->ulStart  = ulStart;
    pArgs->ulCount  = ulCount;
    if (aulTable) memcpy(pArgs->aTable, aulTable, sizeof(ULONG) * ulCount);
    else          memset(pArgs->aTable, 0, sizeof(ULONG) * ulCount);

    rc = GpiDispatch((HPS)hpal, NGpiAnimatePalette, pArgs, cbArgs, &result);

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiSetPaletteEntries.
 * @param [in] hpal HPAL.
 * @param [in] ulFormat ULONG.
 * @param [in] ulStart ULONG.
 * @param [in] ulCount ULONG.
 * @param [in] aulTable PULONG.
 * @return TRUE on success, FALSE on error.
 */
BOOL APIENTRY GpiSetPaletteEntries(HPAL hpal, ULONG ulFormat, ULONG ulStart, ULONG ulCount, PULONG aulTable)
{
    PGpiSetPaletteEntries_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiSetPaletteEntries_Args) - sizeof(ULONG)
                 + sizeof(ULONG) * ulCount;

    GPICallEnter("hpal=%p, ulFormat=0x%lx, ulStart=%lu, ulCount=%lu, aulTable=%p",
                 (void*)hpal, (unsigned long)ulFormat, (unsigned long)ulStart,
                 (unsigned long)ulCount, (void*)aulTable);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiSetPaletteEntries_Args)pv;

    pArgs->ulFormat = ulFormat;
    pArgs->ulStart  = ulStart;
    pArgs->ulCount  = ulCount;
    if (aulTable) memcpy(pArgs->aTable, aulTable, sizeof(ULONG) * ulCount);
    else          memset(pArgs->aTable, 0, sizeof(ULONG) * ulCount);

    rc = GpiDispatch((HPS)hpal, NGpiSetPaletteEntries, pArgs, cbArgs, &result);

    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryPaletteInfo.
 * @param [in] hpal HPAL.
 * @param [in] hps HPS.
 * @param [in] flOptions ULONG.
 * @param [in] ulStart ULONG.
 * @param [in] ulCount ULONG.
 * @param [out] aulArray PULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryPaletteInfo(HPAL hpal, HPS hps, ULONG flOptions, ULONG ulStart, ULONG ulCount, PULONG aulArray)
{
    PGpiQueryPaletteInfo_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryPaletteInfo_Args) - sizeof(ULONG)
                 + sizeof(ULONG) * ulCount;

    GPICallEnter("hpal=%p, hps=%p, flOptions=0x%lx, ulStart=%lu, ulCount=%lu, aulArray=%p",
                 (void*)hpal, (void*)hps, (unsigned long)flOptions,
                 (unsigned long)ulStart, (unsigned long)ulCount, (void*)aulArray);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiQueryPaletteInfo_Args)pv;

    pArgs->flOptions = flOptions;
    pArgs->ulStart   = ulStart;
    pArgs->ulCount   = ulCount;
    memset(pArgs->aData, 0, sizeof(ULONG) * ulCount);

    rc = GpiDispatch(hps, NGpiQueryPaletteInfo, pArgs, cbArgs, &result);
    if (rc == 0 && aulArray) memcpy(aulArray, pArgs->aData, sizeof(ULONG) * ulCount);

    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiResizePalette.
 * @param [in] hpal HPAL.
 * @param [in] newsize ULONG.
 * @return Implementation-defined value.
 */
ULONG APIENTRY GpiResizePalette(HPAL hpal, ULONG newsize)
{
    ULONG result;
    int   rc;

    GPICallEnter("hpal=%p, newsize=%lu", (void*)hpal, (unsigned long)newsize);
    rc = GpiDispatch((HPS)hpal, NGpiResizePalette, &newsize, sizeof(newsize), &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiQueryNearestPaletteIndex.
 * @param [in] hpal HPAL.
 * @param [in] color ULONG.
 * @return GPI_OK, GPI_HITS, or GPI_ERROR.
 */
LONG APIENTRY GpiQueryNearestPaletteIndex(HPAL hpal, ULONG color)
{
    ULONG result;
    int   rc;

    GPICallEnter("hpal=%p, color=0x%lx", (void*)hpal, (unsigned long)color);
    rc = GpiDispatch((HPS)hpal, NGpiQueryNearestPaletteIndex, &color, sizeof(color), &result);
    GPI_RESULT_LONG();
}

/* ======================================================================== */
/*  Struct tags used by marshaling (Part 5)                                 */
/* ======================================================================== */

typedef struct _GpiPolyLine_Args {
    LONG lCount;
    POINTL aptl[1];
} GpiPolyLine_Args, *PGpiPolyLine_Args;

typedef struct _GpiCharString_Args {
    LONG lCount;
    char szString[1];
} GpiCharString_Args, *PGpiCharString_Args;

typedef struct _GpiCharStringAt_Args {
    POINTL ptl;
    LONG lCount;
    char szString[1];
} GpiCharStringAt_Args, *PGpiCharStringAt_Args;

typedef struct _GpiSetAttrs_Args {
    LONG lPrimType;
    ULONG flAttrMask;
    ULONG flDefMask;
    BYTE abBundle[1];
} GpiSetAttrs_Args, *PGpiSetAttrs_Args;

typedef struct _GpiQueryAttrs_Args {
    LONG lPrimType;
    ULONG flAttrMask;
    BYTE abBundle[1];
} GpiQueryAttrs_Args, *PGpiQueryAttrs_Args;

typedef struct _GpiOval_Args {
    POINTL aptl[2];
    FIXED fxStart;
    FIXED fxSweep;
    ULONG ulOptions;
} GpiOval_Args, *PGpiOval_Args;

typedef struct _GpiPolyFilletSharp_Args {
    LONG lCount;
    POINTL aptl[1];
    FIXED afx[1];
} GpiPolyFilletSharp_Args, *PGpiPolyFilletSharp_Args;

typedef struct _GpiQueryCharStringPos_Args {
    ULONG flOptions;
    LONG lCount;
    char szString[1];
    LONG aXinc[1];
    POINTL aptl[1];
} GpiQueryCharStringPos_Args, *PGpiQueryCharStringPos_Args;

typedef struct _GpiQueryCharStringPosAt_Args {
    POINTL ptlStart;
    ULONG flOptions;
    LONG lCount;
    char szString[1];
    LONG aXinc[1];
    POINTL aptl[1];
} GpiQueryCharStringPosAt_Args, *PGpiQueryCharStringPosAt_Args;

typedef struct _GpiQueryTabbedTextExtent_Args {
    LONG lCount;
    LONG lTabCount;
    char szString[1];
    ULONG aTabStops[1];
} GpiQueryTabbedTextExtent_Args, *PGpiQueryTabbedTextExtent_Args;

typedef struct _GpiQueryTextBox_Args {
    LONG lCount1;
    LONG lCount2;
    char szString[1];
    POINTL aptl[1];
} GpiQueryTextBox_Args, *PGpiQueryTextBox_Args;

typedef struct _GpiQueryTextAlignment_Args {
    LONG lHoriz;
    LONG lVert;
} GpiQueryTextAlignment_Args, *PGpiQueryTextAlignment_Args;

typedef struct _GpiCharStringPos_Args {
    RECTL prclRect;
    ULONG flOptions;
    LONG lCount;
    char szString[1];
    LONG aAdx[1];
} GpiCharStringPos_Args, *PGpiCharStringPos_Args;

typedef struct _GpiTabbedCharStringAt_Args {
    POINTL ptStart;
    RECTL prclRect;
    ULONG flOptions;
    LONG lCount;
    LONG lTabCount;
    LONG lTabOrigin;
    char szString[1];
    ULONG aTabStops[1];
} GpiTabbedCharStringAt_Args, *PGpiTabbedCharStringAt_Args;

typedef struct _GpiCharStringPosAt_Args {
    POINTL ptlStart;
    RECTL prclRect;
    ULONG flOptions;
    LONG lCount;
    char szString[1];
    LONG aAdx[1];
} GpiCharStringPosAt_Args, *PGpiCharStringPosAt_Args;

typedef struct _GpiImage_Args {
    LONG lFormat;
    SIZEL size;
    LONG lLength;
    BYTE abData[1];
} GpiImage_Args, *PGpiImage_Args;

typedef struct _GpiComment_Args {
    LONG lLength;
    BYTE abData[1];
} GpiComment_Args, *PGpiComment_Args;

/* ======================================================================== */
/*  Proxies (Part 5)                                                        */
/* ======================================================================== */

/*!
 * @brief GpiPolyLine.
 */
LONG APIENTRY GpiPolyLine(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    PGpiPolyLine_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiPolyLine_Args) - sizeof(POINTL) + sizeof(POINTL) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, aptlPoints=%p", (void*)hps, (long)lCount, (void*)aptlPoints);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiPolyLine_Args)pv;
    pArgs->lCount = lCount;
    if (aptlPoints) memcpy(pArgs->aptl, aptlPoints, sizeof(POINTL) * lCount);
    else            memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);

    rc = GpiDispatch(hps, NGpiPolyLine, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiPolyLineDisjoint.
 */
LONG APIENTRY GpiPolyLineDisjoint(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    PGpiPolyLine_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiPolyLine_Args) - sizeof(POINTL) + sizeof(POINTL) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, aptlPoints=%p", (void*)hps, (long)lCount, (void*)aptlPoints);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiPolyLine_Args)pv;
    pArgs->lCount = lCount;
    if (aptlPoints) memcpy(pArgs->aptl, aptlPoints, sizeof(POINTL) * lCount);
    else            memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);

    rc = GpiDispatch(hps, NGpiPolyLineDisjoint, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiCharString.
 */
LONG APIENTRY GpiCharString(HPS hps, LONG lCount, PCH pchString)
{
    PGpiCharString_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiCharString_Args) - 1 + lCount;

    GPICallEnter("hps=%p, lCount=%ld, pchString=%p", (void*)hps, (long)lCount, (void*)pchString);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiCharString_Args)pv;
    pArgs->lCount = lCount;
    if (pchString) memcpy(pArgs->szString, pchString, lCount);

    rc = GpiDispatch(hps, NGpiCharString, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiCharStringAt.
 */
LONG APIENTRY GpiCharStringAt(HPS hps, PPOINTL pptlPoint, LONG lCount, PCH pchString)
{
    PGpiCharStringAt_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiCharStringAt_Args) - 1 + lCount;

    GPICallEnter("hps=%p, pptlPoint=%p, lCount=%ld, pchString=%p", (void*)hps, (void*)pptlPoint, (long)lCount, (void*)pchString);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiCharStringAt_Args)pv;
    pArgs->ptl = *pptlPoint;
    pArgs->lCount = lCount;
    if (pchString) memcpy(pArgs->szString, pchString, lCount);

    rc = GpiDispatch(hps, NGpiCharStringAt, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiSetAttrs.
 */
BOOL APIENTRY GpiSetAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, ULONG flDefMask, PBUNDLE ppbunAttrs)
{
    PGpiSetAttrs_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG bundleSize = 0;
    switch (lPrimType) {
        case PRIM_LINE: bundleSize = sizeof(LINEBUNDLE); break;
        case PRIM_CHAR: bundleSize = sizeof(CHARBUNDLE); break;
        case PRIM_MARKER: bundleSize = sizeof(MARKERBUNDLE); break;
        case PRIM_AREA: bundleSize = sizeof(AREABUNDLE); break;
        default: bundleSize = sizeof(LINEBUNDLE); break;
    }
    ULONG cbArgs = sizeof(GpiSetAttrs_Args) - 1 + bundleSize;

    GPICallEnter("hps=%p, lPrimType=%ld, flAttrMask=0x%lx, flDefMask=0x%lx, ppbunAttrs=%p", (void*)hps, (long)lPrimType, (unsigned long)flAttrMask, (unsigned long)flDefMask, (void*)ppbunAttrs);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiSetAttrs_Args)pv;
    pArgs->lPrimType = lPrimType;
    pArgs->flAttrMask = flAttrMask;
    pArgs->flDefMask = flDefMask;
    if (ppbunAttrs) memcpy(pArgs->abBundle, ppbunAttrs, bundleSize);
    else            memset(pArgs->abBundle, 0, bundleSize);

    rc = GpiDispatch(hps, NGpiSetAttrs, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryAttrs.
 */
LONG APIENTRY GpiQueryAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, PBUNDLE ppbunAttrs)
{
    PGpiQueryAttrs_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG bundleSize = 0;
    switch (lPrimType) {
        case PRIM_LINE: bundleSize = sizeof(LINEBUNDLE); break;
        case PRIM_CHAR: bundleSize = sizeof(CHARBUNDLE); break;
        case PRIM_MARKER: bundleSize = sizeof(MARKERBUNDLE); break;
        case PRIM_AREA: bundleSize = sizeof(AREABUNDLE); break;
        default: bundleSize = sizeof(LINEBUNDLE); break;
    }
    ULONG cbArgs = sizeof(GpiQueryAttrs_Args) - 1 + bundleSize;

    GPICallEnter("hps=%p, lPrimType=%ld, flAttrMask=0x%lx, ppbunAttrs=%p", (void*)hps, (long)lPrimType, (unsigned long)flAttrMask, (void*)ppbunAttrs);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiQueryAttrs_Args)pv;
    pArgs->lPrimType = lPrimType;
    pArgs->flAttrMask = flAttrMask;
    memset(pArgs->abBundle, 0, bundleSize);

    rc = GpiDispatch(hps, NGpiQueryAttrs, pArgs, cbArgs, &result);
    if (rc == 0 && ppbunAttrs) memcpy(ppbunAttrs, pArgs->abBundle, bundleSize);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryCurrentPosition.
 */
BOOL APIENTRY GpiQueryCurrentPosition(HPS hps, PPOINTL pptlPoint)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, pptlPoint=%p", (void*)hps, (void*)pptlPoint);
    rc = GpiDispatch(hps, NGpiQueryCurrentPosition, pptlPoint, sizeof(POINTL), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryArcParams.
 */
BOOL APIENTRY GpiQueryArcParams(HPS hps, PARCPARAMS parcpArcParams)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, parcpArcParams=%p", (void*)hps, (void*)parcpArcParams);
    rc = GpiDispatch(hps, NGpiQueryArcParams, parcpArcParams, sizeof(ARCPARAMS), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiOval.
 */
LONG APIENTRY GpiOval(HPS hps, PPOINTL aptlPoints, FIXED fxStartAngle, FIXED fxSweepAngle, ULONG ulOptions)
{
    GpiOval_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, aptlPoints=%p, fxStartAngle=%u, fxSweepAngle=%u, ulOptions=0x%lx", (void*)hps, (void*)aptlPoints, (unsigned)fxStartAngle, (unsigned)fxSweepAngle, (unsigned long)ulOptions);
    if (aptlPoints) { args.aptl[0] = aptlPoints[0]; args.aptl[1] = aptlPoints[1]; }
    else            { memset(args.aptl, 0, sizeof(args.aptl)); }
    args.fxStart = fxStartAngle;
    args.fxSweep = fxSweepAngle;
    args.ulOptions = ulOptions;

    rc = GpiDispatch(hps, NGpiOval, &args, sizeof(args), &result);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiPolyFillet.
 */
LONG APIENTRY GpiPolyFillet(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    PGpiPolyLine_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiPolyLine_Args) - sizeof(POINTL) + sizeof(POINTL) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, aptlPoints=%p", (void*)hps, (long)lCount, (void*)aptlPoints);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiPolyLine_Args)pv;
    pArgs->lCount = lCount;
    if (aptlPoints) memcpy(pArgs->aptl, aptlPoints, sizeof(POINTL) * lCount);
    else            memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);

    rc = GpiDispatch(hps, NGpiPolyFillet, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiPolySpline.
 */
LONG APIENTRY GpiPolySpline(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    PGpiPolyLine_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiPolyLine_Args) - sizeof(POINTL) + sizeof(POINTL) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, aptlPoints=%p", (void*)hps, (long)lCount, (void*)aptlPoints);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiPolyLine_Args)pv;
    pArgs->lCount = lCount;
    if (aptlPoints) memcpy(pArgs->aptl, aptlPoints, sizeof(POINTL) * lCount);
    else            memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);

    rc = GpiDispatch(hps, NGpiPolySpline, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiPolyFilletSharp.
 */
LONG APIENTRY GpiPolyFilletSharp(HPS hps, LONG lCount, PPOINTL aptlPoints, PFIXED afxPoints)
{
    PGpiPolyFilletSharp_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiPolyFilletSharp_Args) - sizeof(POINTL) - sizeof(FIXED)
                 + sizeof(POINTL) * lCount + sizeof(FIXED) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, aptlPoints=%p, afxPoints=%p", (void*)hps, (long)lCount, (void*)aptlPoints, (void*)afxPoints);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiPolyFilletSharp_Args)pv;
    pArgs->lCount = lCount;
    if (aptlPoints) memcpy(pArgs->aptl, aptlPoints, sizeof(POINTL) * lCount);
    else            memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);
    if (afxPoints)  memcpy(pArgs->afx, afxPoints, sizeof(FIXED) * lCount);
    else            memset(pArgs->afx, 0, sizeof(FIXED) * lCount);

    rc = GpiDispatch(hps, NGpiPolyFilletSharp, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryPatternRefPoint.
 */
BOOL APIENTRY GpiQueryPatternRefPoint(HPS hps, PPOINTL pptlRefPoint)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, pptlRefPoint=%p", (void*)hps, (void*)pptlRefPoint);
    rc = GpiDispatch(hps, NGpiQueryPatternRefPoint, pptlRefPoint, sizeof(POINTL), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryCharStringPos.
 */
BOOL APIENTRY GpiQueryCharStringPos(HPS hps, ULONG flOptions, LONG lCount, PCH pchString, PLONG alXincrements, PPOINTL aptlPositions)
{
    PGpiQueryCharStringPos_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryCharStringPos_Args) - 1 - sizeof(LONG) - sizeof(POINTL)
                 + lCount + sizeof(LONG) * lCount + sizeof(POINTL) * lCount;

    GPICallEnter("hps=%p, flOptions=0x%lx, lCount=%ld, pchString=%p, alXincrements=%p, aptlPositions=%p", (void*)hps, (unsigned long)flOptions, (long)lCount, (void*)pchString, (void*)alXincrements, (void*)aptlPositions);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryCharStringPos_Args)pv;
    pArgs->flOptions = flOptions;
    pArgs->lCount = lCount;
    if (pchString) memcpy(pArgs->szString, pchString, lCount);
    memset(pArgs->aXinc, 0, sizeof(LONG) * lCount);
    memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);

    rc = GpiDispatch(hps, NGpiQueryCharStringPos, pArgs, cbArgs, &result);
    if (rc == 0) {
        if (alXincrements) memcpy(alXincrements, pArgs->aXinc, sizeof(LONG) * lCount);
        if (aptlPositions) memcpy(aptlPositions, pArgs->aptl, sizeof(POINTL) * lCount);
    }
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryCharStringPosAt.
 */
BOOL APIENTRY GpiQueryCharStringPosAt(HPS hps, PPOINTL pptlStart, ULONG flOptions, LONG lCount, PCH pchString, PLONG alXincrements, PPOINTL aptlPositions)
{
    PGpiQueryCharStringPosAt_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryCharStringPosAt_Args) - 1 - sizeof(LONG) - sizeof(POINTL)
                 + lCount + sizeof(LONG) * lCount + sizeof(POINTL) * lCount;

    GPICallEnter("hps=%p, pptlStart=%p, flOptions=0x%lx, lCount=%ld, pchString=%p, alXincrements=%p, aptlPositions=%p", (void*)hps, (void*)pptlStart, (unsigned long)flOptions, (long)lCount, (void*)pchString, (void*)alXincrements, (void*)aptlPositions);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryCharStringPosAt_Args)pv;
    if (pptlStart) pArgs->ptlStart = *pptlStart;
    pArgs->flOptions = flOptions;
    pArgs->lCount = lCount;
    if (pchString) memcpy(pArgs->szString, pchString, lCount);
    memset(pArgs->aXinc, 0, sizeof(LONG) * lCount);
    memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);

    rc = GpiDispatch(hps, NGpiQueryCharStringPosAt, pArgs, cbArgs, &result);
    if (rc == 0) {
        if (alXincrements) memcpy(alXincrements, pArgs->aXinc, sizeof(LONG) * lCount);
        if (aptlPositions) memcpy(aptlPositions, pArgs->aptl, sizeof(POINTL) * lCount);
    }
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryTabbedTextExtent.
 */
LONG APIENTRY GpiQueryTabbedTextExtent(HPS hps, LONG lCount, PCH pchString, LONG lTabCount, PULONG puTabStops)
{
    PGpiQueryTabbedTextExtent_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryTabbedTextExtent_Args) - 1 - sizeof(ULONG)
                 + lCount + sizeof(ULONG) * lTabCount;

    GPICallEnter("hps=%p, lCount=%ld, pchString=%p, lTabCount=%ld, puTabStops=%p", (void*)hps, (long)lCount, (void*)pchString, (long)lTabCount, (void*)puTabStops);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiQueryTabbedTextExtent_Args)pv;
    pArgs->lCount = lCount;
    pArgs->lTabCount = lTabCount;
    if (pchString) memcpy(pArgs->szString, pchString, lCount);
    memset(pArgs->aTabStops, 0, sizeof(ULONG) * lTabCount);

    rc = GpiDispatch(hps, NGpiQueryTabbedTextExtent, pArgs, cbArgs, &result);
    if (rc == 0 && puTabStops) memcpy(puTabStops, pArgs->aTabStops, sizeof(ULONG) * lTabCount);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryTextBox.
 */
BOOL APIENTRY GpiQueryTextBox(HPS hps, LONG lCount1, PCH pchString, LONG lCount2, PPOINTL aptlPoints)
{
    PGpiQueryTextBox_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryTextBox_Args) - 1 - sizeof(POINTL)
                 + lCount1 + sizeof(POINTL) * lCount2;

    GPICallEnter("hps=%p, lCount1=%ld, pchString=%p, lCount2=%ld, aptlPoints=%p", (void*)hps, (long)lCount1, (void*)pchString, (long)lCount2, (void*)aptlPoints);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryTextBox_Args)pv;
    pArgs->lCount1 = lCount1;
    pArgs->lCount2 = lCount2;
    if (pchString) memcpy(pArgs->szString, pchString, lCount1);
    memset(pArgs->aptl, 0, sizeof(POINTL) * lCount2);

    rc = GpiDispatch(hps, NGpiQueryTextBox, pArgs, cbArgs, &result);
    if (rc == 0 && aptlPoints) memcpy(aptlPoints, pArgs->aptl, sizeof(POINTL) * lCount2);
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryDefCharBox.
 */
BOOL APIENTRY GpiQueryDefCharBox(HPS hps, PSIZEL psizlSize)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, psizlSize=%p", (void*)hps, (void*)psizlSize);
    rc = GpiDispatch(hps, NGpiQueryDefCharBox, psizlSize, sizeof(SIZEL), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryCharBox.
 */
BOOL APIENTRY GpiQueryCharBox(HPS hps, PSIZEF psizfxSize)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, psizfxSize=%p", (void*)hps, (void*)psizfxSize);
    rc = GpiDispatch(hps, NGpiQueryCharBox, psizfxSize, sizeof(SIZEF), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryCharAngle.
 */
BOOL APIENTRY GpiQueryCharAngle(HPS hps, PGRADIENTL pgradlAngle)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, pgradlAngle=%p", (void*)hps, (void*)pgradlAngle);
    rc = GpiDispatch(hps, NGpiQueryCharAngle, pgradlAngle, sizeof(GRADIENTL), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryCharShear.
 */
BOOL APIENTRY GpiQueryCharShear(HPS hps, PPOINTL pptlShear)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, pptlShear=%p", (void*)hps, (void*)pptlShear);
    rc = GpiDispatch(hps, NGpiQueryCharShear, pptlShear, sizeof(POINTL), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryTextAlignment.
 */
BOOL APIENTRY GpiQueryTextAlignment(HPS hps, PLONG plHoriz, PLONG plVert)
{
    GpiQueryTextAlignment_Args args;
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, plHoriz=%p, plVert=%p", (void*)hps, (void*)plHoriz, (void*)plVert);
    args.lHoriz = 0;
    args.lVert = 0;

    rc = GpiDispatch(hps, NGpiQueryTextAlignment, &args, sizeof(args), &result);
    if (rc == 0) {
        if (plHoriz) *plHoriz = args.lHoriz;
        if (plVert) *plVert = args.lVert;
    }
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiCharStringPos.
 */
LONG APIENTRY GpiCharStringPos(HPS hps, PRECTL prclRect, ULONG flOptions, LONG lCount, PCH pchString, PLONG alAdx)
{
    PGpiCharStringPos_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiCharStringPos_Args) - 1 - sizeof(LONG)
                 + lCount + sizeof(LONG) * lCount;

    GPICallEnter("hps=%p, prclRect=%p, flOptions=0x%lx, lCount=%ld, pchString=%p, alAdx=%p", (void*)hps, (void*)prclRect, (unsigned long)flOptions, (long)lCount, (void*)pchString, (void*)alAdx);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiCharStringPos_Args)pv;
    if (prclRect) pArgs->prclRect = *prclRect;
    pArgs->flOptions = flOptions;
    pArgs->lCount = lCount;
    if (pchString) memcpy(pArgs->szString, pchString, lCount);
    if (alAdx) memcpy(pArgs->aAdx, alAdx, sizeof(LONG) * lCount);
    else       memset(pArgs->aAdx, 0, sizeof(LONG) * lCount);

    rc = GpiDispatch(hps, NGpiCharStringPos, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiTabbedCharStringAt.
 */
LONG APIENTRY GpiTabbedCharStringAt(HPS hps, PPOINTL pPtStart, PRECTL prclRect, ULONG flOptions, LONG lCount, PCH pchString, LONG lTabCount, PULONG puTabStops, LONG lTabOrigin)
{
    PGpiTabbedCharStringAt_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiTabbedCharStringAt_Args) - 1 - sizeof(ULONG)
                 + lCount + sizeof(ULONG) * lTabCount;

    GPICallEnter("hps=%p, pPtStart=%p, prclRect=%p, flOptions=0x%lx, lCount=%ld, pchString=%p, lTabCount=%ld, puTabStops=%p, lTabOrigin=%ld", (void*)hps, (void*)pPtStart, (void*)prclRect, (unsigned long)flOptions, (long)lCount, (void*)pchString, (long)lTabCount, (void*)puTabStops, (long)lTabOrigin);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiTabbedCharStringAt_Args)pv;
    if (pPtStart) pArgs->ptStart = *pPtStart;
    if (prclRect) pArgs->prclRect = *prclRect;
    pArgs->flOptions = flOptions;
    pArgs->lCount = lCount;
    pArgs->lTabCount = lTabCount;
    pArgs->lTabOrigin = lTabOrigin;
    if (pchString) memcpy(pArgs->szString, pchString, lCount);
    if (puTabStops) memcpy(pArgs->aTabStops, puTabStops, sizeof(ULONG) * lTabCount);

    rc = GpiDispatch(hps, NGpiTabbedCharStringAt, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiCharStringPosAt.
 */
LONG APIENTRY GpiCharStringPosAt(HPS hps, PPOINTL pptlStart, PRECTL prclRect, ULONG flOptions, LONG lCount, PCH pchString, PLONG alAdx)
{
    PGpiCharStringPosAt_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiCharStringPosAt_Args) - 1 - sizeof(LONG)
                 + lCount + sizeof(LONG) * lCount;

    GPICallEnter("hps=%p, pptlStart=%p, prclRect=%p, flOptions=0x%lx, lCount=%ld, pchString=%p, alAdx=%p", (void*)hps, (void*)pptlStart, (void*)prclRect, (unsigned long)flOptions, (long)lCount, (void*)pchString, (void*)alAdx);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiCharStringPosAt_Args)pv;
    if (pptlStart) pArgs->ptlStart = *pptlStart;
    if (prclRect) pArgs->prclRect = *prclRect;
    pArgs->flOptions = flOptions;
    pArgs->lCount = lCount;
    if (pchString) memcpy(pArgs->szString, pchString, lCount);
    if (alAdx) memcpy(pArgs->aAdx, alAdx, sizeof(LONG) * lCount);
    else       memset(pArgs->aAdx, 0, sizeof(LONG) * lCount);

    rc = GpiDispatch(hps, NGpiCharStringPosAt, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryCharExtra.
 */
BOOL APIENTRY GpiQueryCharExtra(HPS hps, PFIXED Extra)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, Extra=%p", (void*)hps, (void*)Extra);
    rc = GpiDispatch(hps, NGpiQueryCharExtra, Extra, sizeof(FIXED), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryCharBreakExtra.
 */
BOOL APIENTRY GpiQueryCharBreakExtra(HPS hps, PFIXED BreakExtra)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, BreakExtra=%p", (void*)hps, (void*)BreakExtra);
    rc = GpiDispatch(hps, NGpiQueryCharBreakExtra, BreakExtra, sizeof(FIXED), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiPolyMarker.
 */
LONG APIENTRY GpiPolyMarker(HPS hps, LONG lCount, PPOINTL aptlPoints)
{
    PGpiPolyLine_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiPolyLine_Args) - sizeof(POINTL) + sizeof(POINTL) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, aptlPoints=%p", (void*)hps, (long)lCount, (void*)aptlPoints);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiPolyLine_Args)pv;
    pArgs->lCount = lCount;
    if (aptlPoints) memcpy(pArgs->aptl, aptlPoints, sizeof(POINTL) * lCount);
    else            memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);

    rc = GpiDispatch(hps, NGpiPolyMarker, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryMarkerBox.
 */
BOOL APIENTRY GpiQueryMarkerBox(HPS hps, PSIZEF psizfxSize)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, psizfxSize=%p", (void*)hps, (void*)psizfxSize);
    rc = GpiDispatch(hps, NGpiQueryMarkerBox, psizfxSize, sizeof(SIZEF), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiImage.
 */
LONG APIENTRY GpiImage(HPS hps, LONG lFormat, PSIZEL psizlImageSize, LONG lLength, PBYTE pbData)
{
    PGpiImage_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiImage_Args) - 1 + lLength;

    GPICallEnter("hps=%p, lFormat=%ld, psizlImageSize=%p, lLength=%ld, pbData=%p", (void*)hps, (long)lFormat, (void*)psizlImageSize, (long)lLength, (void*)pbData);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiImage_Args)pv;
    pArgs->lFormat = lFormat;
    if (psizlImageSize) pArgs->size = *psizlImageSize;
    else                memset(&pArgs->size, 0, sizeof(SIZEL));
    pArgs->lLength = lLength;
    if (pbData && lLength) memcpy(pArgs->abData, pbData, lLength);

    rc = GpiDispatch(hps, NGpiImage, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiComment.
 */
BOOL APIENTRY GpiComment(HPS hps, LONG lLength, PBYTE pbData)
{
    PGpiComment_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiComment_Args) - 1 + lLength;

    GPICallEnter("hps=%p, lLength=%ld, pbData=%p", (void*)hps, (long)lLength, (void*)pbData);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiComment_Args)pv;
    pArgs->lLength = lLength;
    if (pbData && lLength) memcpy(pArgs->abData, pbData, lLength);

    rc = GpiDispatch(hps, NGpiComment, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/* ======================================================================== */
/*  Struct tags used by marshaling (Part 6)                                 */
/* ======================================================================== */

typedef struct _GpiCreateLogFont_Args {
    LONG lLcid;
    char szName[MAX_STR];
    FATTRS attrs;
} GpiCreateLogFont_Args, *PGpiCreateLogFont_Args;

typedef struct _GpiLoadFonts_Args {
    char szFilename[MAX_STR];
} GpiLoadFonts_Args, *PGpiLoadFonts_Args;

typedef struct _GpiQueryFonts_Args {
    ULONG flOptions;
    LONG lReqFonts;
    LONG lMetricsLength;
    char szFacename[MAX_STR];
    BYTE abMetrics[1];
} GpiQueryFonts_Args, *PGpiQueryFonts_Args;

typedef struct _GpiQueryFontMetrics_Args {
    LONG lMetricsLength;
    BYTE abMetrics[1];
} GpiQueryFontMetrics_Args, *PGpiQueryFontMetrics_Args;

typedef struct _GpiQueryKerningPairs_Args {
    LONG lCount;
    KERNINGPAIRS aData[1];
} GpiQueryKerningPairs_Args, *PGpiQueryKerningPairs_Args;

typedef struct _GpiQueryWidthTable_Args {
    LONG lFirstChar;
    LONG lCount;
    LONG aData[1];
} GpiQueryWidthTable_Args, *PGpiQueryWidthTable_Args;

typedef struct _GpiQuerySetIds_Args {
    LONG lCount;
    BYTE abData[1]; /* LONG aTypes[lCount]; LONG aLcids[lCount]; char aNames[lCount][MAX_STR]; */
} GpiQuerySetIds_Args, *PGpiQuerySetIds_Args;

typedef struct _GpiQueryFaceString_Args {
    FACENAMEDESC desc;
    LONG length;
    char szFamily[MAX_STR];
    char szCompound[MAX_STR];
} GpiQueryFaceString_Args, *PGpiQueryFaceString_Args;

typedef struct _GpiQueryLogicalFont_Args {
    LONG lcid;
    LONG length;
    char szName[MAX_STR];
    FATTRS attrs;
} GpiQueryLogicalFont_Args, *PGpiQueryLogicalFont_Args;

typedef struct _GpiQueryFullFontFileDescs_Args {
    LONG lCount;
    LONG lNamesBuffLength;
    char szFilename[MAX_STR];
    BYTE abNames[1];
} GpiQueryFullFontFileDescs_Args, *PGpiQueryFullFontFileDescs_Args;

typedef struct _GpiQueryFontFileDescriptions_Args {
    LONG lCount;
    char szFilename[MAX_STR];
    BYTE abDescs[1];
} GpiQueryFontFileDescriptions_Args, *PGpiQueryFontFileDescriptions_Args;

/* ======================================================================== */
/*  Proxies (Part 6)                                                        */
/* ======================================================================== */

/*!
 * @brief GpiQueryRasterizerCaps.
 */
BOOL APIENTRY GpiQueryRasterizerCaps(PRASTERIZERCAPS p1)
{
    ULONG result;
    int   rc;
    GPICallEnter("p1=%p", (void*)p1);
    rc = GpiDispatch((HPS)0, NGpiQueryRasterizerCaps, p1, sizeof(RASTERIZERCAPS), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiCreateLogFont.
 */
LONG APIENTRY GpiCreateLogFont(HPS hps, PSTR8 pName, LONG lLcid, PFATTRS pfatAttrs)
{
    GpiCreateLogFont_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, pName=%p, lLcid=%ld, pfatAttrs=%p", (void*)hps, (void*)pName, (long)lLcid, (void*)pfatAttrs);
    
    args.lLcid = lLcid;
    if (pName) strncpy(args.szName, (char*)pName, MAX_STR - 1);
    else       args.szName[0] = '\0';
    args.szName[MAX_STR - 1] = '\0';
    
    if (pfatAttrs) args.attrs = *pfatAttrs;
    else           memset(&args.attrs, 0, sizeof(FATTRS));

    rc = GpiDispatch(hps, NGpiCreateLogFont, &args, sizeof(args), &result);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiLoadFonts.
 */
BOOL APIENTRY GpiLoadFonts(HAB hab, PCSZ pszFilename)
{
    GpiLoadFonts_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hab=%p, pszFilename=%p", (void*)hab, (void*)pszFilename);
    if (pszFilename) strncpy(args.szFilename, pszFilename, MAX_STR - 1);
    else             args.szFilename[0] = '\0';
    args.szFilename[MAX_STR - 1] = '\0';

    rc = GpiDispatch((HPS)hab, NGpiLoadFonts, &args, sizeof(args), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiUnloadFonts.
 */
BOOL APIENTRY GpiUnloadFonts(HAB hab, PCSZ pszFilename)
{
    GpiLoadFonts_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hab=%p, pszFilename=%p", (void*)hab, (void*)pszFilename);
    if (pszFilename) strncpy(args.szFilename, pszFilename, MAX_STR - 1);
    else             args.szFilename[0] = '\0';
    args.szFilename[MAX_STR - 1] = '\0';

    rc = GpiDispatch((HPS)hab, NGpiUnloadFonts, &args, sizeof(args), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryFonts.
 */
LONG APIENTRY GpiQueryFonts(HPS hps, ULONG flOptions, PCSZ pszFacename, PLONG plReqFonts, LONG lMetricsLength, PFONTMETRICS afmMetrics)
{
    PGpiQueryFonts_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    LONG  req = plReqFonts ? *plReqFonts : 0;
    ULONG cbMetrics = (ULONG)req * lMetricsLength;
    ULONG cbArgs = sizeof(GpiQueryFonts_Args) - 1 + cbMetrics;

    GPICallEnter("hps=%p, flOptions=0x%lx, pszFacename=%p, plReqFonts=%p, lMetricsLength=%ld, afmMetrics=%p", (void*)hps, (unsigned long)flOptions, (void*)pszFacename, (void*)plReqFonts, (long)lMetricsLength, (void*)afmMetrics);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiQueryFonts_Args)pv;
    
    pArgs->flOptions = flOptions;
    pArgs->lReqFonts = req;
    pArgs->lMetricsLength = lMetricsLength;
    if (pszFacename) strncpy(pArgs->szFacename, pszFacename, MAX_STR - 1);
    else             pArgs->szFacename[0] = '\0';
    pArgs->szFacename[MAX_STR - 1] = '\0';
    memset(pArgs->abMetrics, 0, cbMetrics);

    rc = GpiDispatch(hps, NGpiQueryFonts, pArgs, cbArgs, &result);
    if (rc == 0) {
        if (plReqFonts) *plReqFonts = pArgs->lReqFonts;
        if (afmMetrics && cbMetrics > 0) memcpy(afmMetrics, pArgs->abMetrics, cbMetrics);
    }
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryFontMetrics.
 */
BOOL APIENTRY GpiQueryFontMetrics(HPS hps, LONG lMetricsLength, PFONTMETRICS pfmMetrics)
{
    PGpiQueryFontMetrics_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryFontMetrics_Args) - 1 + lMetricsLength;

    GPICallEnter("hps=%p, lMetricsLength=%ld, pfmMetrics=%p", (void*)hps, (long)lMetricsLength, (void*)pfmMetrics);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryFontMetrics_Args)pv;
    pArgs->lMetricsLength = lMetricsLength;
    memset(pArgs->abMetrics, 0, lMetricsLength);

    rc = GpiDispatch(hps, NGpiQueryFontMetrics, pArgs, cbArgs, &result);
    if (rc == 0 && pfmMetrics) memcpy(pfmMetrics, pArgs->abMetrics, lMetricsLength);
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryKerningPairs.
 */
LONG APIENTRY GpiQueryKerningPairs(HPS hps, LONG lCount, PKERNINGPAIRS akrnprData)
{
    PGpiQueryKerningPairs_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryKerningPairs_Args) - sizeof(KERNINGPAIRS) + sizeof(KERNINGPAIRS) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, akrnprData=%p", (void*)hps, (long)lCount, (void*)akrnprData);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiQueryKerningPairs_Args)pv;
    pArgs->lCount = lCount;
    memset(pArgs->aData, 0, sizeof(KERNINGPAIRS) * lCount);

    rc = GpiDispatch(hps, NGpiQueryKerningPairs, pArgs, cbArgs, &result);
    if (rc == 0 && akrnprData) memcpy(akrnprData, pArgs->aData, sizeof(KERNINGPAIRS) * lCount);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryWidthTable.
 */
BOOL APIENTRY GpiQueryWidthTable(HPS hps, LONG lFirstChar, LONG lCount, PLONG alData)
{
    PGpiQueryWidthTable_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryWidthTable_Args) - sizeof(LONG) + sizeof(LONG) * lCount;

    GPICallEnter("hps=%p, lFirstChar=%ld, lCount=%ld, alData=%p", (void*)hps, (long)lFirstChar, (long)lCount, (void*)alData);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryWidthTable_Args)pv;
    pArgs->lFirstChar = lFirstChar;
    pArgs->lCount = lCount;
    memset(pArgs->aData, 0, sizeof(LONG) * lCount);

    rc = GpiDispatch(hps, NGpiQueryWidthTable, pArgs, cbArgs, &result);
    if (rc == 0 && alData) memcpy(alData, pArgs->aData, sizeof(LONG) * lCount);
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQuerySetIds.
 */
BOOL APIENTRY GpiQuerySetIds(HPS hps, LONG lCount, PLONG alTypes, PSTR8 aNames, PLONG allcids)
{
    PGpiQuerySetIds_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQuerySetIds_Args) - 1 + sizeof(LONG) * lCount * 2 + lCount * MAX_STR;

    GPICallEnter("hps=%p, lCount=%ld, alTypes=%p, aNames=%p, allcids=%p", (void*)hps, (long)lCount, (void*)alTypes, (void*)aNames, (void*)allcids);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQuerySetIds_Args)pv;
    pArgs->lCount = lCount;

    LONG *pTypes = (LONG *)pArgs->abData;
    LONG *pLcids = pTypes + lCount;
    char *pNames = (char *)(pLcids + lCount);
    
    memset(pTypes, 0, sizeof(LONG) * lCount);
    memset(pLcids, 0, sizeof(LONG) * lCount);
    memset(pNames, 0, lCount * MAX_STR);

    rc = GpiDispatch(hps, NGpiQuerySetIds, pArgs, cbArgs, &result);
    if (rc == 0) {
        if (alTypes) memcpy(alTypes, pTypes, sizeof(LONG) * lCount);
        if (allcids) memcpy(allcids, pLcids, sizeof(LONG) * lCount);
        if (aNames)  memcpy(aNames, pNames, lCount * MAX_STR);
    }
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryFaceString.
 */
ULONG APIENTRY GpiQueryFaceString(HPS PS, PCSZ FamilyName, PFACENAMEDESC attrs, LONG length, PCSZ CompoundFaceName)
{
    GpiQueryFaceString_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("PS=%p, FamilyName=%p, attrs=%p, length=%ld, CompoundFaceName=%p", (void*)PS, (void*)FamilyName, (void*)attrs, (long)length, (void*)CompoundFaceName);
    
    if (attrs) args.desc = *attrs;
    else       memset(&args.desc, 0, sizeof(FACENAMEDESC));
    args.length = length;
    
    if (FamilyName) strncpy(args.szFamily, FamilyName, MAX_STR - 1);
    else            args.szFamily[0] = '\0';
    args.szFamily[MAX_STR - 1] = '\0';
    
    if (CompoundFaceName) strncpy(args.szCompound, CompoundFaceName, MAX_STR - 1);
    else                  args.szCompound[0] = '\0';
    args.szCompound[MAX_STR - 1] = '\0';

    rc = GpiDispatch(PS, NGpiQueryFaceString, &args, sizeof(args), &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiQueryLogicalFont.
 */
BOOL APIENTRY GpiQueryLogicalFont(HPS PS, LONG lcid, PSTR8 name, PFATTRS attrs, LONG length)
{
    GpiQueryLogicalFont_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("PS=%p, lcid=%ld, name=%p, attrs=%p, length=%ld", (void*)PS, (long)lcid, (void*)name, (void*)attrs, (long)length);
    
    args.lcid = lcid;
    args.length = length;
    memset(args.szName, 0, MAX_STR);
    memset(&args.attrs, 0, sizeof(FATTRS));

    rc = GpiDispatch(PS, NGpiQueryLogicalFont, &args, sizeof(args), &result);
    if (rc == 0) {
        if (name)  strncpy((char*)name, args.szName, MAX_STR - 1);
        if (attrs) *attrs = args.attrs;
    }
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryFontAction.
 */
ULONG APIENTRY GpiQueryFontAction(HAB anchor, ULONG options)
{
    ULONG result;
    int   rc;
    GPICallEnter("anchor=%p, options=0x%lx", (void*)anchor, (unsigned long)options);
    rc = GpiDispatch((HPS)anchor, NGpiQueryFontAction, &options, sizeof(options), &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiLoadPublicFonts.
 */
BOOL APIENTRY GpiLoadPublicFonts(HAB hab, PCSZ pszFilename)
{
    GpiLoadFonts_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hab=%p, pszFilename=%p", (void*)hab, (void*)pszFilename);
    if (pszFilename) strncpy(args.szFilename, pszFilename, MAX_STR - 1);
    else             args.szFilename[0] = '\0';
    args.szFilename[MAX_STR - 1] = '\0';

    rc = GpiDispatch((HPS)hab, NGpiLoadPublicFonts, &args, sizeof(args), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiUnloadPublicFonts.
 */
BOOL APIENTRY GpiUnloadPublicFonts(HAB hab, PCSZ pszFilename)
{
    GpiLoadFonts_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hab=%p, pszFilename=%p", (void*)hab, (void*)pszFilename);
    if (pszFilename) strncpy(args.szFilename, pszFilename, MAX_STR - 1);
    else             args.szFilename[0] = '\0';
    args.szFilename[MAX_STR - 1] = '\0';

    rc = GpiDispatch((HPS)hab, NGpiUnloadPublicFonts, &args, sizeof(args), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryFullFontFileDescs.
 */
LONG APIENTRY GpiQueryFullFontFileDescs(HAB hab, PCSZ pszFilename, PLONG plCount, PVOID pNames, PLONG plNamesBuffLength)
{
    PGpiQueryFullFontFileDescs_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    LONG  count = plCount ? *plCount : 0;
    LONG  buffLen = plNamesBuffLength ? *plNamesBuffLength : 0;
    ULONG cbArgs = sizeof(GpiQueryFullFontFileDescs_Args) - 1 + buffLen;

    GPICallEnter("hab=%p, pszFilename=%p, plCount=%p, pNames=%p, plNamesBuffLength=%p", (void*)hab, (void*)pszFilename, (void*)plCount, (void*)pNames, (void*)plNamesBuffLength);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiQueryFullFontFileDescs_Args)pv;
    
    pArgs->lCount = count;
    pArgs->lNamesBuffLength = buffLen;
    if (pszFilename) strncpy(pArgs->szFilename, pszFilename, MAX_STR - 1);
    else             pArgs->szFilename[0] = '\0';
    pArgs->szFilename[MAX_STR - 1] = '\0';
    memset(pArgs->abNames, 0, buffLen);

    rc = GpiDispatch((HPS)hab, NGpiQueryFullFontFileDescs, pArgs, cbArgs, &result);
    if (rc == 0) {
        if (plCount) *plCount = pArgs->lCount;
        if (plNamesBuffLength) *plNamesBuffLength = pArgs->lNamesBuffLength;
        if (pNames && buffLen > 0) memcpy(pNames, pArgs->abNames, buffLen);
    }
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryFontFileDescriptions.
 */
LONG APIENTRY GpiQueryFontFileDescriptions(HAB hab, PCSZ pszFilename, PLONG plCount, PFFDESCS affdescsNames)
{
    PGpiQueryFontFileDescriptions_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    LONG  count = plCount ? *plCount : 0;
    ULONG cbArgs = sizeof(GpiQueryFontFileDescriptions_Args) - 1 + sizeof(FFDESCS) * count;

    GPICallEnter("hab=%p, pszFilename=%p, plCount=%p, affdescsNames=%p", (void*)hab, (void*)pszFilename, (void*)plCount, (void*)affdescsNames);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)"); return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiQueryFontFileDescriptions_Args)pv;
    
    pArgs->lCount = count;
    if (pszFilename) strncpy(pArgs->szFilename, pszFilename, MAX_STR - 1);
    else             pArgs->szFilename[0] = '\0';
    pArgs->szFilename[MAX_STR - 1] = '\0';
    memset(pArgs->abDescs, 0, sizeof(FFDESCS) * count);

    rc = GpiDispatch((HPS)hab, NGpiQueryFontFileDescriptions, pArgs, cbArgs, &result);
    if (rc == 0) {
        if (plCount) *plCount = pArgs->lCount;
        if (affdescsNames) memcpy(affdescsNames, pArgs->abDescs, sizeof(FFDESCS) * count);
    }
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/* ======================================================================== */
/*  Struct tags used by marshaling (Part 7)                                 */
/* ======================================================================== */

typedef struct _GpiBitBlt_Args {
    HPS    hpsSource;
    LONG   lCount;
    LONG   lRop;
    ULONG  flOptions;
    POINTL aptl[1];
} GpiBitBlt_Args, *PGpiBitBlt_Args;

typedef struct _GpiWCBitBlt_Args {
    HBITMAP hbmSource;
    LONG    lCount;
    LONG    lRop;
    ULONG   flOptions;
    POINTL  aptl[1];
} GpiWCBitBlt_Args, *PGpiWCBitBlt_Args;

typedef struct _GpiCreateBitmap_Args {
    BITMAPINFOHEADER2 hdr;
    ULONG             flOptions;
    ULONG             cbInitData;
    ULONG             cbInfo;
    BYTE              abData[1];
} GpiCreateBitmap_Args, *PGpiCreateBitmap_Args;

typedef struct _GpiSetBitmapBits_Args {
    LONG  lScanStart;
    LONG  lScans;
    ULONG cbBuffer;
    ULONG cbInfo;
    BYTE  abData[1];
} GpiSetBitmapBits_Args, *PGpiSetBitmapBits_Args;

typedef struct _GpiSetBitmapDimension_Args {
    HBITMAP hbm;
    SIZEL   size;
} GpiSetBitmapDimension_Args, *PGpiSetBitmapDimension_Args;

typedef struct _GpiQueryBitmapBits_Args {
    LONG  lScanStart;
    LONG  lScans;
    ULONG cbBuffer;
    ULONG cbInfo;
    BYTE  abData[1];
} GpiQueryBitmapBits_Args, *PGpiQueryBitmapBits_Args;

typedef struct _GpiQueryBitmapDimension_Args {
    HBITMAP hbm;
    SIZEL   size;
} GpiQueryBitmapDimension_Args, *PGpiQueryBitmapDimension_Args;

typedef struct _GpiQueryBitmapParameters_Args {
    HBITMAP hbm;
    BITMAPINFOHEADER hdr;
} GpiQueryBitmapParameters_Args, *PGpiQueryBitmapParameters_Args;

typedef struct _GpiQueryBitmapInfoHeader_Args {
    HBITMAP hbm;
    BITMAPINFOHEADER2 hdr;
} GpiQueryBitmapInfoHeader_Args, *PGpiQueryBitmapInfoHeader_Args;

typedef struct _GpiQueryDeviceBitmapFormats_Args {
    LONG lCount;
    LONG aData[1];
} GpiQueryDeviceBitmapFormats_Args, *PGpiQueryDeviceBitmapFormats_Args;

typedef struct _GpiDrawBits_Args {
    LONG   lCount;
    LONG   lRop;
    ULONG  flOptions;
    ULONG  cbInfo;
    ULONG  cbBits;
    POINTL aptl[1];
} GpiDrawBits_Args, *PGpiDrawBits_Args;

/* ======================================================================== */
/*  Proxies (Part 7)                                                        */
/* ======================================================================== */

/*!
 * @brief GpiBitBlt.
 */
LONG APIENTRY GpiBitBlt(HPS hpsTarget, HPS hpsSource, LONG lCount, PPOINTL aptlPoints, LONG lRop, ULONG flOptions)
{
    PGpiBitBlt_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiBitBlt_Args) - sizeof(POINTL) + sizeof(POINTL) * lCount;

    GPICallEnter("hpsTarget=%p, hpsSource=%p, lCount=%ld, aptlPoints=%p, lRop=%ld, flOptions=0x%lx",
                 (void*)hpsTarget, (void*)hpsSource, (long)lCount, (void*)aptlPoints, (long)lRop, (unsigned long)flOptions);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiBitBlt_Args)pv;

    pArgs->hpsSource = hpsSource;
    pArgs->lCount    = lCount;
    pArgs->lRop      = lRop;
    pArgs->flOptions = flOptions;
    if (aptlPoints) memcpy(pArgs->aptl, aptlPoints, sizeof(POINTL) * lCount);
    else            memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);

    rc = GpiDispatch(hpsTarget, NGpiBitBlt, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiDeleteBitmap.
 */
BOOL APIENTRY GpiDeleteBitmap(HBITMAP hbm)
{
    ULONG result;
    int   rc;
    GPICallEnter("hbm=%p", (void*)hbm);
    rc = GpiDispatch((HPS)0, NGpiDeleteBitmap, &hbm, sizeof(HBITMAP), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiWCBitBlt.
 */
LONG APIENTRY GpiWCBitBlt(HPS hpsTarget, HBITMAP hbmSource, LONG lCount, PPOINTL aptlPoints, LONG lRop, ULONG flOptions)
{
    PGpiWCBitBlt_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiWCBitBlt_Args) - sizeof(POINTL) + sizeof(POINTL) * lCount;

    GPICallEnter("hpsTarget=%p, hbmSource=%p, lCount=%ld, aptlPoints=%p, lRop=%ld, flOptions=0x%lx",
                 (void*)hpsTarget, (void*)hbmSource, (long)lCount, (void*)aptlPoints, (long)lRop, (unsigned long)flOptions);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiWCBitBlt_Args)pv;

    pArgs->hbmSource = hbmSource;
    pArgs->lCount    = lCount;
    pArgs->lRop      = lRop;
    pArgs->flOptions = flOptions;
    if (aptlPoints) memcpy(pArgs->aptl, aptlPoints, sizeof(POINTL) * lCount);
    else            memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);

    rc = GpiDispatch(hpsTarget, NGpiWCBitBlt, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiCreateBitmap.
 */
HBITMAP APIENTRY GpiCreateBitmap(HPS hps, PBITMAPINFOHEADER2 pbmpNew, ULONG flOptions, PBYTE pbInitData, PBITMAPINFO2 pbmiInfoTable)
{
    PGpiCreateBitmap_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbInit = 0;
    ULONG cbInfo = 0;

    if (pbmpNew) {
        ULONG width = pbmpNew->cx;
        ULONG height = pbmpNew->cy;
        ULONG bits = pbmpNew->cBitCount;
        if (bits > 0 && height > 0) {
            cbInit = ((width * bits + 31) / 32) * 4 * height;
        }
    }
    if (pbmiInfoTable) {
        ULONG colors = pbmiInfoTable->cclrUsed;
        if (colors == 0 && pbmiInfoTable->cBitCount <= 8 && pbmiInfoTable->cBitCount > 0) {
            colors = 1 << pbmiInfoTable->cBitCount;
        }
        cbInfo = sizeof(BITMAPINFOHEADER2) + colors * sizeof(RGB2);
    }
    ULONG cbArgs = sizeof(GpiCreateBitmap_Args) - 1 + cbInit + cbInfo;

    GPICallEnter("hps=%p, pbmpNew=%p, flOptions=0x%lx, pbInitData=%p, pbmiInfoTable=%p",
                 (void*)hps, (void*)pbmpNew, (unsigned long)flOptions, (void*)pbInitData, (void*)pbmiInfoTable);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (HBITMAP)GPI_ERROR;
    }
    pArgs = (PGpiCreateBitmap_Args)pv;

    if (pbmpNew) pArgs->hdr = *pbmpNew;
    else memset(&pArgs->hdr, 0, sizeof(BITMAPINFOHEADER2));
    pArgs->flOptions  = flOptions;
    pArgs->cbInitData = cbInit;
    pArgs->cbInfo     = cbInfo;
    
    if (pbInitData && cbInit > 0) memcpy(pArgs->abData, pbInitData, cbInit);
    if (pbmiInfoTable && cbInfo > 0) memcpy(pArgs->abData + cbInit, pbmiInfoTable, cbInfo);

    rc = GpiDispatch(hps, NGpiCreateBitmap, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_HANDLE(HBITMAP);
}

/*!
 * @brief GpiSetBitmapBits.
 */
LONG APIENTRY GpiSetBitmapBits(HPS hps, LONG lScanStart, LONG lScans, PBYTE pbBuffer, PBITMAPINFO2 pbmiInfoTable)
{
    PGpiSetBitmapBits_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbBuffer = 0;
    ULONG cbInfo = 0;

    if (pbmiInfoTable) {
        ULONG width = pbmiInfoTable->cx;
        ULONG bits = pbmiInfoTable->cBitCount;
        ULONG colors = pbmiInfoTable->cclrUsed;
        if (bits > 0 && width > 0) {
            cbBuffer = ((width * bits + 31) / 32) * 4 * lScans;
        }
        if (colors == 0 && bits <= 8 && bits > 0) {
            colors = 1 << bits;
        }
        cbInfo = sizeof(BITMAPINFOHEADER2) + colors * sizeof(RGB2);
    }
    ULONG cbArgs = sizeof(GpiSetBitmapBits_Args) - 1 + cbBuffer + cbInfo;

    GPICallEnter("hps=%p, lScanStart=%ld, lScans=%ld, pbBuffer=%p, pbmiInfoTable=%p",
                 (void*)hps, (long)lScanStart, (long)lScans, (void*)pbBuffer, (void*)pbmiInfoTable);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiSetBitmapBits_Args)pv;

    pArgs->lScanStart = lScanStart;
    pArgs->lScans     = lScans;
    pArgs->cbBuffer   = cbBuffer;
    pArgs->cbInfo     = cbInfo;
    
    if (pbBuffer && cbBuffer > 0) memcpy(pArgs->abData, pbBuffer, cbBuffer);
    if (pbmiInfoTable && cbInfo > 0) memcpy(pArgs->abData + cbBuffer, pbmiInfoTable, cbInfo);

    rc = GpiDispatch(hps, NGpiSetBitmapBits, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiSetBitmapDimension.
 */
BOOL APIENTRY GpiSetBitmapDimension(HBITMAP hbm, PSIZEL psizlBitmapDimension)
{
    GpiSetBitmapDimension_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hbm=%p, psizlBitmapDimension=%p", (void*)hbm, (void*)psizlBitmapDimension);
    args.hbm = hbm;
    if (psizlBitmapDimension) args.size = *psizlBitmapDimension;
    else { args.size.cx = 0; args.size.cy = 0; }

    rc = GpiDispatch((HPS)0, NGpiSetBitmapDimension, &args, sizeof(args), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryBitmapBits.
 */
LONG APIENTRY GpiQueryBitmapBits(HPS hps, LONG lScanStart, LONG lScans, PBYTE pbBuffer, PBITMAPINFO2 pbmiInfoTable)
{
    PGpiQueryBitmapBits_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbBuffer = 0;
    ULONG cbInfo = 0;

    if (pbmiInfoTable) {
        ULONG width = pbmiInfoTable->cx;
        ULONG bits = pbmiInfoTable->cBitCount;
        ULONG colors = pbmiInfoTable->cclrUsed;
        if (bits > 0 && width > 0) {
            cbBuffer = ((width * bits + 31) / 32) * 4 * lScans;
        }
        if (colors == 0 && bits <= 8 && bits > 0) {
            colors = 1 << bits;
        }
        cbInfo = sizeof(BITMAPINFOHEADER2) + colors * sizeof(RGB2);
    }
    ULONG cbArgs = sizeof(GpiQueryBitmapBits_Args) - 1 + cbBuffer + cbInfo;

    GPICallEnter("hps=%p, lScanStart=%ld, lScans=%ld, pbBuffer=%p, pbmiInfoTable=%p",
                 (void*)hps, (long)lScanStart, (long)lScans, (void*)pbBuffer, (void*)pbmiInfoTable);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiQueryBitmapBits_Args)pv;

    pArgs->lScanStart = lScanStart;
    pArgs->lScans     = lScans;
    pArgs->cbBuffer   = cbBuffer;
    pArgs->cbInfo     = cbInfo;
    
    memset(pArgs->abData, 0, cbBuffer + cbInfo);
    if (pbmiInfoTable && cbInfo > 0) memcpy(pArgs->abData + cbBuffer, pbmiInfoTable, cbInfo);

    rc = GpiDispatch(hps, NGpiQueryBitmapBits, pArgs, cbArgs, &result);
    if (rc == 0) {
        if (pbBuffer && cbBuffer > 0) memcpy(pbBuffer, pArgs->abData, cbBuffer);
        if (pbmiInfoTable && cbInfo > 0) memcpy(pbmiInfoTable, pArgs->abData + cbBuffer, cbInfo);
    }
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryBitmapDimension.
 */
BOOL APIENTRY GpiQueryBitmapDimension(HBITMAP hbm, PSIZEL psizlBitmapDimension)
{
    GpiQueryBitmapDimension_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hbm=%p, psizlBitmapDimension=%p", (void*)hbm, (void*)psizlBitmapDimension);
    args.hbm = hbm;
    memset(&args.size, 0, sizeof(SIZEL));

    rc = GpiDispatch((HPS)0, NGpiQueryBitmapDimension, &args, sizeof(args), &result);
    if (rc == 0 && psizlBitmapDimension) *psizlBitmapDimension = args.size;
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryBitmapParameters.
 */
BOOL APIENTRY GpiQueryBitmapParameters(HBITMAP hbm, PBITMAPINFOHEADER pbmpData)
{
    GpiQueryBitmapParameters_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hbm=%p, pbmpData=%p", (void*)hbm, (void*)pbmpData);
    args.hbm = hbm;
    memset(&args.hdr, 0, sizeof(BITMAPINFOHEADER));

    rc = GpiDispatch((HPS)0, NGpiQueryBitmapParameters, &args, sizeof(args), &result);
    if (rc == 0 && pbmpData) *pbmpData = args.hdr;
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryBitmapInfoHeader.
 */
BOOL APIENTRY GpiQueryBitmapInfoHeader(HBITMAP hbm, PBITMAPINFOHEADER2 pbmpData)
{
    GpiQueryBitmapInfoHeader_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hbm=%p, pbmpData=%p", (void*)hbm, (void*)pbmpData);
    args.hbm = hbm;
    memset(&args.hdr, 0, sizeof(BITMAPINFOHEADER2));

    rc = GpiDispatch((HPS)0, NGpiQueryBitmapInfoHeader, &args, sizeof(args), &result);
    if (rc == 0 && pbmpData) *pbmpData = args.hdr;
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryDeviceBitmapFormats.
 */
BOOL APIENTRY GpiQueryDeviceBitmapFormats(HPS hps, LONG lCount, PLONG alArray)
{
    PGpiQueryDeviceBitmapFormats_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryDeviceBitmapFormats_Args) - sizeof(LONG) + sizeof(LONG) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, alArray=%p", (void*)hps, (long)lCount, (void*)alArray);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryDeviceBitmapFormats_Args)pv;
    pArgs->lCount = lCount;
    memset(pArgs->aData, 0, sizeof(LONG) * lCount);

    rc = GpiDispatch(hps, NGpiQueryDeviceBitmapFormats, pArgs, cbArgs, &result);
    if (rc == 0 && alArray) memcpy(alArray, pArgs->aData, sizeof(LONG) * lCount);
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryPel.
 */
LONG APIENTRY GpiQueryPel(HPS hps, PPOINTL pptlPoint)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, pptlPoint=%p", (void*)hps, (void*)pptlPoint);
    rc = GpiDispatch(hps, NGpiQueryPel, pptlPoint, sizeof(POINTL), &result);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiDrawBits.
 */
LONG APIENTRY GpiDrawBits(HPS hps, PVOID pBits, PBITMAPINFO2 pbmiInfoTable, LONG lCount, PPOINTL aptlPoints, LONG lRop, ULONG flOptions)
{
    PGpiDrawBits_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbInfo = 0;
    ULONG cbBits = 0;

    if (pbmiInfoTable) {
        ULONG width = pbmiInfoTable->cx;
        ULONG height = pbmiInfoTable->cy;
        ULONG bits = pbmiInfoTable->cBitCount;
        ULONG colors = pbmiInfoTable->cclrUsed;
        if (bits > 0 && height > 0) {
            cbBits = ((width * bits + 31) / 32) * 4 * height;
        }
        if (colors == 0 && bits <= 8 && bits > 0) {
            colors = 1 << bits;
        }
        cbInfo = sizeof(BITMAPINFOHEADER2) + colors * sizeof(RGB2);
    }
    ULONG cbArgs = sizeof(GpiDrawBits_Args) - sizeof(POINTL) + sizeof(POINTL) * lCount + cbInfo + cbBits;

    GPICallEnter("hps=%p, pBits=%p, pbmiInfoTable=%p, lCount=%ld, aptlPoints=%p, lRop=%ld, flOptions=0x%lx",
                 (void*)hps, (void*)pBits, (void*)pbmiInfoTable, (long)lCount, (void*)aptlPoints, (long)lRop, (unsigned long)flOptions);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiDrawBits_Args)pv;

    pArgs->lCount    = lCount;
    pArgs->lRop      = lRop;
    pArgs->flOptions = flOptions;
    pArgs->cbInfo    = cbInfo;
    pArgs->cbBits    = cbBits;
    
    if (aptlPoints) memcpy(pArgs->aptl, aptlPoints, sizeof(POINTL) * lCount);
    else            memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);

    BYTE *pDest = (BYTE*)(pArgs->aptl + lCount);
    if (pbmiInfoTable && cbInfo > 0) memcpy(pDest, pbmiInfoTable, cbInfo);
    if (pBits && cbBits > 0) memcpy(pDest + cbInfo, pBits, cbBits);

    rc = GpiDispatch(hps, NGpiDrawBits, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/* ======================================================================== */
/*  Struct tags used by marshaling (Part 8)                                 */
/* ======================================================================== */

typedef struct _GpiCreateRegion_Args {
    LONG  lCount;
    RECTL arcl[1];
} GpiCreateRegion_Args, *PGpiCreateRegion_Args;

typedef struct _GpiQueryRegionBox_Args {
    HRGN  hrgn;
    RECTL rclBound;
} GpiQueryRegionBox_Args, *PGpiQueryRegionBox_Args;

typedef struct _GpiQueryRegionRects_Args {
    HRGN    hrgn;
    RECTL   rclBound;
    RGNRECT rgnrcControl;
    RECTL   arclRect[1];
} GpiQueryRegionRects_Args, *PGpiQueryRegionRects_Args;

typedef struct _GpiSetRegion_Args {
    HRGN  hrgn;
    LONG  lCount;
    RECTL arcl[1];
} GpiSetRegion_Args, *PGpiSetRegion_Args;

typedef struct _GpiSetClipRegion_Args {
    HRGN hrgn;
    HRGN hrgnOld;
} GpiSetClipRegion_Args, *PGpiSetClipRegion_Args;

typedef struct _GpiLoadMetaFile_Args {
    char szFilename[MAX_STR];
} GpiLoadMetaFile_Args, *PGpiLoadMetaFile_Args;

typedef struct _GpiPlayMetaFile_Args {
    HMF  hmf;
    LONG lCount1;
    LONG lCount2;
    LONG lSegCount;
    LONG aOptarray[1];
    char szDesc[MAX_STR];
} GpiPlayMetaFile_Args, *PGpiPlayMetaFile_Args;

typedef struct _GpiQueryMetaFileBits_Args {
    HMF  hmf;
    LONG lOffset;
    LONG lLength;
    BYTE abData[1];
} GpiQueryMetaFileBits_Args, *PGpiQueryMetaFileBits_Args;

typedef struct _GpiSaveMetaFile_Args {
    HMF  hmf;
    char szFilename[MAX_STR];
} GpiSaveMetaFile_Args, *PGpiSaveMetaFile_Args;

typedef struct _GpiSetMetaFileBits_Args {
    HMF  hmf;
    LONG lOffset;
    LONG lLength;
    BYTE abData[1];
} GpiSetMetaFileBits_Args, *PGpiSetMetaFileBits_Args;

typedef struct _GpiQueryDefAttrs_Args {
    LONG  lPrimType;
    ULONG flAttrMask;
    BYTE  abBundle[1];
} GpiQueryDefAttrs_Args, *PGpiQueryDefAttrs_Args;

typedef struct _GpiSetDefAttrs_Args {
    LONG  lPrimType;
    ULONG flAttrMask;
    BYTE  abBundle[1];
} GpiSetDefAttrs_Args, *PGpiSetDefAttrs_Args;

typedef struct _GpiPolygons_Args {
    ULONG ulCount;
    ULONG flOptions;
    ULONG flModel;
    ULONG ulTotalPoints;
    BYTE  abData[1]; /* interleaved: LONG numPts; POINTL pts[numPts]; ... */
} GpiPolygons_Args, *PGpiPolygons_Args;

typedef struct _GpiCreatePolygonRegion_Args {
    ULONG ulCount;
    ULONG flOptions;
    ULONG ulTotalPoints;
    BYTE  abData[1];
} GpiCreatePolygonRegion_Args, *PGpiCreatePolygonRegion_Args;

typedef struct _GpiStrokeInkPath_Args {
    LONG   lPath;
    LONG   lCount;
    ULONG  flOptions;
    POINTL aptl[1];
} GpiStrokeInkPath_Args, *PGpiStrokeInkPath_Args;

/* ======================================================================== */
/*  Regions                                                                 */
/* ======================================================================== */

/*!
 * @brief GpiCreateRegion.
 */
HRGN APIENTRY GpiCreateRegion(HPS hps, LONG lCount, PRECTL arclRectangles)
{
    PGpiCreateRegion_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiCreateRegion_Args) - sizeof(RECTL) + sizeof(RECTL) * lCount;

    GPICallEnter("hps=%p, lCount=%ld, arclRectangles=%p", (void*)hps, (long)lCount, (void*)arclRectangles);
    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (HRGN)GPI_ERROR;
    }
    pArgs = (PGpiCreateRegion_Args)pv;

    pArgs->lCount = lCount;
    if (arclRectangles) memcpy(pArgs->arcl, arclRectangles, sizeof(RECTL) * lCount);
    else                memset(pArgs->arcl, 0, sizeof(RECTL) * lCount);

    rc = GpiDispatch(hps, NGpiCreateRegion, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_HANDLE(HRGN);
}

/*!
 * @brief GpiQueryRegionBox.
 */
LONG APIENTRY GpiQueryRegionBox(HPS hps, HRGN hrgn, PRECTL prclBound)
{
    GpiQueryRegionBox_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, hrgn=%p, prclBound=%p", (void*)hps, (void*)hrgn, (void*)prclBound);
    args.hrgn = hrgn;
    memset(&args.rclBound, 0, sizeof(RECTL));

    rc = GpiDispatch(hps, NGpiQueryRegionBox, &args, sizeof(args), &result);
    if (rc == 0 && prclBound) *prclBound = args.rclBound;
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryRegionRects.
 */
BOOL APIENTRY GpiQueryRegionRects(HPS hps, HRGN hrgn, PRECTL prclBound, PRGNRECT prgnrcControl, PRECTL prclRect)
{
    PGpiQueryRegionRects_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG crc = prgnrcControl ? prgnrcControl->crc : 0;
    ULONG cbArgs = sizeof(GpiQueryRegionRects_Args) - sizeof(RECTL) + sizeof(RECTL) * crc;

    GPICallEnter("hps=%p, hrgn=%p, prclBound=%p, prgnrcControl=%p, prclRect=%p",
                 (void*)hps, (void*)hrgn, (void*)prclBound,
                 (void*)prgnrcControl, (void*)prclRect);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryRegionRects_Args)pv;

    pArgs->hrgn = hrgn;
    if (prclBound) pArgs->rclBound = *prclBound;
    else           memset(&pArgs->rclBound, 0, sizeof(RECTL));
    if (prgnrcControl) pArgs->rgnrcControl = *prgnrcControl;
    else               memset(&pArgs->rgnrcControl, 0, sizeof(RGNRECT));
    memset(pArgs->arclRect, 0, sizeof(RECTL) * crc);

    rc = GpiDispatch(hps, NGpiQueryRegionRects, pArgs, cbArgs, &result);
    if (rc == 0) {
        if (prgnrcControl) *prgnrcControl = pArgs->rgnrcControl;
        if (prclRect && crc) memcpy(prclRect, pArgs->arclRect, sizeof(RECTL) * crc);
    }
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiSetRegion.
 */
BOOL APIENTRY GpiSetRegion(HPS hps, HRGN hrgn, LONG lcount, PRECTL arclRectangles)
{
    PGpiSetRegion_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiSetRegion_Args) - sizeof(RECTL) + sizeof(RECTL) * lcount;

    GPICallEnter("hps=%p, hrgn=%p, lcount=%ld, arclRectangles=%p",
                 (void*)hps, (void*)hrgn, (long)lcount, (void*)arclRectangles);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiSetRegion_Args)pv;

    pArgs->hrgn   = hrgn;
    pArgs->lCount = lcount;
    if (arclRectangles) memcpy(pArgs->arcl, arclRectangles, sizeof(RECTL) * lcount);
    else                memset(pArgs->arcl, 0, sizeof(RECTL) * lcount);

    rc = GpiDispatch(hps, NGpiSetRegion, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiSetClipRegion.
 */
LONG APIENTRY GpiSetClipRegion(HPS hps, HRGN hrgn, PHRGN phrgnOld)
{
    GpiSetClipRegion_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hps=%p, hrgn=%p, phrgnOld=%p", (void*)hps, (void*)hrgn, (void*)phrgnOld);
    args.hrgn    = hrgn;
    args.hrgnOld = NULL;

    rc = GpiDispatch(hps, NGpiSetClipRegion, &args, sizeof(args), &result);
    if (rc == 0 && phrgnOld) *phrgnOld = args.hrgnOld;
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryClipBox.
 */
LONG APIENTRY GpiQueryClipBox(HPS hps, PRECTL prclBound)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, prclBound=%p", (void*)hps, (void*)prclBound);
    rc = GpiDispatch(hps, NGpiQueryClipBox, prclBound, sizeof(RECTL), &result);
    GPI_RESULT_LONG();
}

/* ======================================================================== */
/*  Metafiles                                                               */
/* ======================================================================== */

/*!
 * @brief GpiCopyMetaFile.
 */
HMF APIENTRY GpiCopyMetaFile(HMF hmf)
{
    ULONG result;
    int   rc;
    GPICallEnter("hmf=%p", (void*)hmf);
    rc = GpiDispatch((HPS)0, NGpiCopyMetaFile, &hmf, sizeof(HMF), &result);
    GPI_RESULT_HANDLE(HMF);
}

/*!
 * @brief GpiDeleteMetaFile.
 */
BOOL APIENTRY GpiDeleteMetaFile(HMF hmf)
{
    ULONG result;
    int   rc;
    GPICallEnter("hmf=%p", (void*)hmf);
    rc = GpiDispatch((HPS)0, NGpiDeleteMetaFile, &hmf, sizeof(HMF), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiLoadMetaFile.
 */
HMF APIENTRY GpiLoadMetaFile(HAB hab, PCSZ pszFilename)
{
    GpiLoadMetaFile_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hab=%p, pszFilename=%p", (void*)hab, (void*)pszFilename);
    if (pszFilename) strncpy(args.szFilename, pszFilename, MAX_STR - 1);
    else             args.szFilename[0] = '\0';
    args.szFilename[MAX_STR - 1] = '\0';

    rc = GpiDispatch((HPS)hab, NGpiLoadMetaFile, &args, sizeof(args), &result);
    GPI_RESULT_HANDLE(HMF);
}

/*!
 * @brief GpiPlayMetaFile.
 */
LONG APIENTRY GpiPlayMetaFile(HPS hps, HMF hmf, LONG lCount1, PLONG alOptarray, PLONG plSegCount, LONG lCount2, PCSZ pszDesc)
{
    PGpiPlayMetaFile_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiPlayMetaFile_Args) - sizeof(LONG)
                 + sizeof(LONG) * lCount1;

    GPICallEnter("hps=%p, hmf=%p, lCount1=%ld, alOptarray=%p, plSegCount=%p, lCount2=%ld, pszDesc=%p",
                 (void*)hps, (void*)hmf, (long)lCount1, (void*)alOptarray,
                 (void*)plSegCount, (long)lCount2, (void*)pszDesc);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiPlayMetaFile_Args)pv;

    pArgs->hmf       = hmf;
    pArgs->lCount1   = lCount1;
    pArgs->lCount2   = lCount2;
    pArgs->lSegCount = plSegCount ? *plSegCount : 0;
    if (alOptarray) memcpy(pArgs->aOptarray, alOptarray, sizeof(LONG) * lCount1);
    else            memset(pArgs->aOptarray, 0, sizeof(LONG) * lCount1);
    if (pszDesc) strncpy(pArgs->szDesc, pszDesc, MAX_STR - 1);
    else         pArgs->szDesc[0] = '\0';
    pArgs->szDesc[MAX_STR - 1] = '\0';

    rc = GpiDispatch(hps, NGpiPlayMetaFile, pArgs, cbArgs, &result);
    if (rc == 0 && plSegCount) *plSegCount = pArgs->lSegCount;
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiQueryMetaFileBits.
 */
BOOL APIENTRY GpiQueryMetaFileBits(HMF hmf, LONG lOffset, LONG lLength, PBYTE pbData)
{
    PGpiQueryMetaFileBits_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiQueryMetaFileBits_Args) - 1 + lLength;

    GPICallEnter("hmf=%p, lOffset=%ld, lLength=%ld, pbData=%p",
                 (void*)hmf, (long)lOffset, (long)lLength, (void*)pbData);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryMetaFileBits_Args)pv;

    pArgs->hmf     = hmf;
    pArgs->lOffset = lOffset;
    pArgs->lLength = lLength;
    memset(pArgs->abData, 0, lLength);

    rc = GpiDispatch((HPS)0, NGpiQueryMetaFileBits, pArgs, cbArgs, &result);
    if (rc == 0 && pbData) memcpy(pbData, pArgs->abData, lLength);
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryMetaFileLength.
 */
LONG APIENTRY GpiQueryMetaFileLength(HMF hmf)
{
    ULONG result;
    int   rc;
    GPICallEnter("hmf=%p", (void*)hmf);
    rc = GpiDispatch((HPS)0, NGpiQueryMetaFileLength, &hmf, sizeof(HMF), &result);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiSaveMetaFile.
 */
BOOL APIENTRY GpiSaveMetaFile(HMF hmf, PCSZ pszFilename)
{
    GpiSaveMetaFile_Args args;
    ULONG result;
    int   rc;

    GPICallEnter("hmf=%p, pszFilename=%p", (void*)hmf, (void*)pszFilename);
    args.hmf = hmf;
    if (pszFilename) strncpy(args.szFilename, pszFilename, MAX_STR - 1);
    else             args.szFilename[0] = '\0';
    args.szFilename[MAX_STR - 1] = '\0';

    rc = GpiDispatch((HPS)0, NGpiSaveMetaFile, &args, sizeof(args), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiSetMetaFileBits.
 */
BOOL APIENTRY GpiSetMetaFileBits(HMF hmf, LONG lOffset, LONG lLength, PBYTE pbBuffer)
{
    PGpiSetMetaFileBits_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiSetMetaFileBits_Args) - 1 + lLength;

    GPICallEnter("hmf=%p, lOffset=%ld, lLength=%ld, pbBuffer=%p",
                 (void*)hmf, (long)lOffset, (long)lLength, (void*)pbBuffer);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiSetMetaFileBits_Args)pv;

    pArgs->hmf     = hmf;
    pArgs->lOffset = lOffset;
    pArgs->lLength = lLength;
    if (pbBuffer) memcpy(pArgs->abData, pbBuffer, lLength);

    rc = GpiDispatch((HPS)0, NGpiSetMetaFileBits, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/* ======================================================================== */
/*  Defaults                                                                */
/* ======================================================================== */

/*!
 * @brief GpiQueryDefArcParams.
 */
BOOL APIENTRY GpiQueryDefArcParams(HPS hps, PARCPARAMS parcpArcParams)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, parcpArcParams=%p", (void*)hps, (void*)parcpArcParams);
    rc = GpiDispatch(hps, NGpiQueryDefArcParams, parcpArcParams, sizeof(ARCPARAMS), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryDefAttrs.
 */
BOOL APIENTRY GpiQueryDefAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, PBUNDLE ppbunAttrs)
{
    PGpiQueryDefAttrs_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG bundleSize = 0;

    switch (lPrimType) {
        case PRIM_LINE:   bundleSize = sizeof(LINEBUNDLE);   break;
        case PRIM_CHAR:   bundleSize = sizeof(CHARBUNDLE);   break;
        case PRIM_MARKER: bundleSize = sizeof(MARKERBUNDLE); break;
        case PRIM_AREA:   bundleSize = sizeof(AREABUNDLE);   break;
        default:          bundleSize = sizeof(LINEBUNDLE);   break;
    }
    ULONG cbArgs = sizeof(GpiQueryDefAttrs_Args) - 1 + bundleSize;

    GPICallEnter("hps=%p, lPrimType=%ld, flAttrMask=0x%lx, ppbunAttrs=%p",
                 (void*)hps, (long)lPrimType, (unsigned long)flAttrMask, (void*)ppbunAttrs);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiQueryDefAttrs_Args)pv;

    pArgs->lPrimType  = lPrimType;
    pArgs->flAttrMask = flAttrMask;
    memset(pArgs->abBundle, 0, bundleSize);

    rc = GpiDispatch(hps, NGpiQueryDefAttrs, pArgs, cbArgs, &result);
    if (rc == 0 && ppbunAttrs) memcpy(ppbunAttrs, pArgs->abBundle, bundleSize);
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryDefTag.
 */
BOOL APIENTRY GpiQueryDefTag(HPS hps, PLONG plTag)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, plTag=%p", (void*)hps, (void*)plTag);
    rc = GpiDispatch(hps, NGpiQueryDefTag, plTag, sizeof(LONG), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiQueryDefViewingLimits.
 */
BOOL APIENTRY GpiQueryDefViewingLimits(HPS hps, PRECTL prclLimits)
{
    ULONG result;
    int   rc;
    GPICallEnter("hps=%p, prclLimits=%p", (void*)hps, (void*)prclLimits);
    rc = GpiDispatch(hps, NGpiQueryDefViewingLimits, prclLimits, sizeof(RECTL), &result);
    GPI_RESULT_BOOL();
}

/*!
 * @brief GpiSetDefAttrs.
 */
BOOL APIENTRY GpiSetDefAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, PBUNDLE ppbunAttrs)
{
    PGpiSetDefAttrs_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG bundleSize = 0;

    switch (lPrimType) {
        case PRIM_LINE:   bundleSize = sizeof(LINEBUNDLE);   break;
        case PRIM_CHAR:   bundleSize = sizeof(CHARBUNDLE);   break;
        case PRIM_MARKER: bundleSize = sizeof(MARKERBUNDLE); break;
        case PRIM_AREA:   bundleSize = sizeof(AREABUNDLE);   break;
        default:          bundleSize = sizeof(LINEBUNDLE);   break;
    }
    ULONG cbArgs = sizeof(GpiSetDefAttrs_Args) - 1 + bundleSize;

    GPICallEnter("hps=%p, lPrimType=%ld, flAttrMask=0x%lx, ppbunAttrs=%p",
                 (void*)hps, (long)lPrimType, (unsigned long)flAttrMask, (void*)ppbunAttrs);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (BOOL)GPI_ERROR;
    }
    pArgs = (PGpiSetDefAttrs_Args)pv;

    pArgs->lPrimType  = lPrimType;
    pArgs->flAttrMask = flAttrMask;
    if (ppbunAttrs) memcpy(pArgs->abBundle, ppbunAttrs, bundleSize);
    else            memset(pArgs->abBundle, 0, bundleSize);

    rc = GpiDispatch(hps, NGpiSetDefAttrs, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_BOOL();
}

/* ======================================================================== */
/*  Polygons / ink paths                                                    */
/* ======================================================================== */

/*!
 * @brief GpiPolygons.
 */
LONG APIENTRY GpiPolygons(HPS hps, ULONG ulCount, PPOLYGON paplgn, ULONG flOptions, ULONG flModel)
{
    PGpiPolygons_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbData = 0;
    ULONG i;

    if (paplgn && ulCount) {
        for (i = 0; i < ulCount; i++) {
            cbData += sizeof(ULONG) + sizeof(POINTL) * paplgn[i].ulPoints;
        }
    }
    ULONG cbArgs = sizeof(GpiPolygons_Args) - 1 + cbData;

    GPICallEnter("hps=%p, ulCount=%lu, paplgn=%p, flOptions=0x%lx, flModel=0x%lx",
                 (void*)hps, (unsigned long)ulCount, (void*)paplgn,
                 (unsigned long)flOptions, (unsigned long)flModel);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiPolygons_Args)pv;

    pArgs->ulCount       = paplgn ? ulCount : 0;
    pArgs->flOptions     = flOptions;
    pArgs->flModel       = flModel;
    pArgs->ulTotalPoints = 0;

    if (paplgn && ulCount) {
        BYTE *pCur = pArgs->abData;
        for (i = 0; i < ulCount; i++) {
            memcpy(pCur, &paplgn[i].ulPoints, sizeof(ULONG));
            pCur += sizeof(ULONG);
            if (paplgn[i].aPointl && paplgn[i].ulPoints) {
                memcpy(pCur, paplgn[i].aPointl, sizeof(POINTL) * paplgn[i].ulPoints);
            }
            pCur += sizeof(POINTL) * paplgn[i].ulPoints;
            pArgs->ulTotalPoints += paplgn[i].ulPoints;
        }
    }

    rc = GpiDispatch(hps, NGpiPolygons, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/*!
 * @brief GpiCreatePolygonRegion.
 */
HRGN APIENTRY GpiCreatePolygonRegion(HPS hps, ULONG ulCount, PPOLYGON paplgn, ULONG flOptions)
{
    PGpiCreatePolygonRegion_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbData = 0;
    ULONG i;

    if (paplgn && ulCount) {
        for (i = 0; i < ulCount; i++) {
            cbData += sizeof(ULONG) + sizeof(POINTL) * paplgn[i].ulPoints;
        }
    }
    ULONG cbArgs = sizeof(GpiCreatePolygonRegion_Args) - 1 + cbData;

    GPICallEnter("hps=%p, ulCount=%lu, paplgn=%p, flOptions=0x%lx",
                 (void*)hps, (unsigned long)ulCount, (void*)paplgn,
                 (unsigned long)flOptions);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (HRGN)GPI_ERROR;
    }
    pArgs = (PGpiCreatePolygonRegion_Args)pv;

    pArgs->ulCount       = paplgn ? ulCount : 0;
    pArgs->flOptions     = flOptions;
    pArgs->ulTotalPoints = 0;

    if (paplgn && ulCount) {
        BYTE *pCur = pArgs->abData;
        for (i = 0; i < ulCount; i++) {
            memcpy(pCur, &paplgn[i].ulPoints, sizeof(ULONG));
            pCur += sizeof(ULONG);
            if (paplgn[i].aPointl && paplgn[i].ulPoints) {
                memcpy(pCur, paplgn[i].aPointl, sizeof(POINTL) * paplgn[i].ulPoints);
            }
            pCur += sizeof(POINTL) * paplgn[i].ulPoints;
            pArgs->ulTotalPoints += paplgn[i].ulPoints;
        }
    }

    rc = GpiDispatch(hps, NGpiCreatePolygonRegion, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_HANDLE(HRGN);
}

/*!
 * @brief GpiStrokeInkPath.
 */
LONG APIENTRY GpiStrokeInkPath(HPS hps, LONG lPath, LONG lCount, PPOINTL aptlPoints, ULONG flOptions)
{
    PGpiStrokeInkPath_Args pArgs;
    PVOID pv = NULL;
    ULONG result;
    int   rc;
    ULONG cbArgs = sizeof(GpiStrokeInkPath_Args) - sizeof(POINTL) + sizeof(POINTL) * lCount;

    GPICallEnter("hps=%p, lPath=%ld, lCount=%ld, aptlPoints=%p, flOptions=0x%lx",
                 (void*)hps, (long)lPath, (long)lCount, (void*)aptlPoints,
                 (unsigned long)flOptions);

    if (DosAllocMem(&pv, cbArgs, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GPICallResult("GPI_ERROR (nomem)");
        return (LONG)GPI_ERROR;
    }
    pArgs = (PGpiStrokeInkPath_Args)pv;

    pArgs->lPath     = lPath;
    pArgs->lCount    = lCount;
    pArgs->flOptions = flOptions;
    if (aptlPoints) memcpy(pArgs->aptl, aptlPoints, sizeof(POINTL) * lCount);
    else            memset(pArgs->aptl, 0, sizeof(POINTL) * lCount);

    rc = GpiDispatch(hps, NGpiStrokeInkPath, pArgs, cbArgs, &result);
    DosFreeMem(pv);
    GPI_RESULT_LONG();
}

/* ======================================================================== */
/*  Server placeholder entry points                                         */
/* ======================================================================== */

/*!
 * @brief GpiSetDCOwner (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiSetDCOwner(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiSetDCOwner, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiSetBitmapOwner (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiSetBitmapOwner(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiSetBitmapOwner, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiSetRegionOwner (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiSetRegionOwner(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiSetRegionOwner, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiInstallIFIFont (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiInstallIFIFont(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiInstallIFIFont, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiOpenMetafile (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiOpenMetafile(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiOpenMetafile, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiCloseMetafile (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiCloseMetafile(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiCloseMetafile, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiConvPSH (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiConvPSH(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiConvPSH, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiMtAssociate (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiMtAssociate(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiMtAssociate, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiMtDisassociate (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiMtDisassociate(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiMtDisassociate, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiEnableYInversion (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiEnableYInversion(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiEnableYInversion, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiQueryYInversion (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiQueryYInversion(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiQueryYInversion, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiSaveMetaFile2 (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiSaveMetaFile2(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiSaveMetaFile2, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiAllocateDCData (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiAllocateDCData(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiAllocateDCData, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/*!
 * @brief GpiQueryDCData (stub; server prototype is incomplete).
 * @return GPI_ERROR.
 */
EXTERN ULONG APIENTRY GpiQueryDCData(void)
{
    ULONG result;
    int   rc;
    GPICallEnter("");
    rc = GpiDispatch((HPS)0, NGpiQueryDCData, NULL, 0, &result);
    GPI_RESULT_ULONG();
}

/* ======================================================================== */
/*  Undocumented / Dev* stubs (no wire code yet)                            */
/* ======================================================================== */

/*!
 * @def FPM_UNDOC_STUB
 * @brief Placeholder for an entry point without a wire code.
 * @param [in] name Function name.
 */
#define FPM_UNDOC_STUB(name) \
    EXTERN ULONG APIENTRY name(void) \
    { \
        GPICallEnter(""); \
        GPICallResult("GPI_ERROR"); \
        return (ULONG)GPI_ERROR; \
    }

FPM_UNDOC_STUB(undoc625)
FPM_UNDOC_STUB(undoc626)
FPM_UNDOC_STUB(undoc630)
FPM_UNDOC_STUB(undoc631)
FPM_UNDOC_STUB(undoc632)
FPM_UNDOC_STUB(undoc633)
FPM_UNDOC_STUB(undoc634)
FPM_UNDOC_STUB(undoc635)
FPM_UNDOC_STUB(undoc636)
FPM_UNDOC_STUB(undoc637)
FPM_UNDOC_STUB(undoc638)
FPM_UNDOC_STUB(undoc639)
FPM_UNDOC_STUB(undoc640)
FPM_UNDOC_STUB(undoc641)
FPM_UNDOC_STUB(undoc642)
FPM_UNDOC_STUB(undoc643)
FPM_UNDOC_STUB(undoc646)
FPM_UNDOC_STUB(undoc651)
FPM_UNDOC_STUB(undoc652)
FPM_UNDOC_STUB(undoc653)
FPM_UNDOC_STUB(undoc658)
FPM_UNDOC_STUB(undoc659)
FPM_UNDOC_STUB(undoc660)
FPM_UNDOC_STUB(undoc661)
FPM_UNDOC_STUB(undoc662)
FPM_UNDOC_STUB(undoc663)
FPM_UNDOC_STUB(undoc666)
FPM_UNDOC_STUB(undoc667)
FPM_UNDOC_STUB(undoc668)
FPM_UNDOC_STUB(undoc669)
FPM_UNDOC_STUB(undoc722)
FPM_UNDOC_STUB(undoc724)
FPM_UNDOC_STUB(undoc725)
FPM_UNDOC_STUB(undoc727)
FPM_UNDOC_STUB(undoc730)

FPM_UNDOC_STUB(DspDefaultResolution)
FPM_UNDOC_STUB(DevQueryDisplayResolutions)
FPM_UNDOC_STUB(DspInitSystemDriverName)

FPM_UNDOC_STUB(GPIBEGININKPATH)
FPM_UNDOC_STUB(GPIENDINKPATH)
FPM_UNDOC_STUB(GPISTROKEINKPATH)

FPM_UNDOC_STUB(PaxGetPolyFillMode)
FPM_UNDOC_STUB(PaxSetPolyFillMode)
FPM_UNDOC_STUB(PaxBeginPath)
FPM_UNDOC_STUB(PaxCloseFigure)
FPM_UNDOC_STUB(PaxEndPath)
FPM_UNDOC_STUB(PaxFillPath)
FPM_UNDOC_STUB(PaxFlattenPath)
FPM_UNDOC_STUB(PaxPathToRegion)
FPM_UNDOC_STUB(PaxStrokeAndFillPath)
FPM_UNDOC_STUB(PaxStrokePath)
FPM_UNDOC_STUB(PaxWidenPath)
FPM_UNDOC_STUB(PaxArc)
FPM_UNDOC_STUB(PaxChord)
FPM_UNDOC_STUB(PaxEllipse)
FPM_UNDOC_STUB(PaxLineDDA)
FPM_UNDOC_STUB(PaxPie)
FPM_UNDOC_STUB(PaxPolyBezier)
FPM_UNDOC_STUB(PaxPolyBezierAt)

/*!
 * @brief DevCloseDC.
 * @param [in] hdc HDC.
 * @return NULL.
 */
EXTERN HMF APIENTRY DevCloseDC(HDC hdc)
{
    GPICallEnter("hdc=%p", (void*)hdc);
    GPICallResult("NULL");
    (void)hdc;
    return (HMF)NULL;
}

/*!
 * @brief DevEscape.
 * @return GPI_ERROR.
 */
EXTERN LONG APIENTRY DevEscape(HDC hdc, LONG lCmd, LONG lInCount,
                               PBYTE pbIn, PLONG plOutCount, PBYTE pbOut)
{
    GPICallEnter("hdc=%p, lCmd=%ld, lInCount=%ld, pbIn=%p, plOutCount=%p, pbOut=%p",
                 (void*)hdc, (long)lCmd, (long)lInCount, (void*)pbIn,
                 (void*)plOutCount, (void*)pbOut);
    GPICallResult("GPI_ERROR");
    (void)hdc; (void)lCmd; (void)lInCount; (void)pbIn;
    (void)plOutCount; (void)pbOut;
    return (LONG)GPI_ERROR;
}

/*!
 * @brief DevQueryCaps.
 * @return GPI_ERROR.
 */
EXTERN BOOL APIENTRY DevQueryCaps(HDC hdc, LONG lStart, LONG lCount, PLONG alCaps)
{
    GPICallEnter("hdc=%p, lStart=%ld, lCount=%ld, alCaps=%p",
                 (void*)hdc, (long)lStart, (long)lCount, (void*)alCaps);
    GPICallResult("GPI_ERROR");
    (void)hdc; (void)lStart; (void)lCount; (void)alCaps;
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief DevQueryDeviceNames.
 * @return GPI_ERROR.
 */
EXTERN BOOL APIENTRY DevQueryDeviceNames(HAB hab, PCSZ pszDriverName,
                                          PLONG plDriverTypes,
                                          PSTR32 pszDriverNames,
                                          PSTR64 pszDeviceNames,
                                          PLONG plDriverSizes,
                                          PSTR16 pszLogAddress)
{
    GPICallEnter("hab=%p, pszDriverName=%p, plDriverTypes=%p, pszDriverNames=%p, pszDeviceNames=%p, plDriverSizes=%p, pszLogAddress=%p",
                 (void*)hab, (void*)pszDriverName, (void*)plDriverTypes,
                 (void*)pszDriverNames, (void*)pszDeviceNames,
                 (void*)plDriverSizes, (void*)pszLogAddress);
    GPICallResult("GPI_ERROR");
    (void)hab; (void)pszDriverName; (void)plDriverTypes;
    (void)pszDriverNames; (void)pszDeviceNames; (void)plDriverSizes;
    (void)pszLogAddress;
    return (BOOL)GPI_ERROR;
}

/*!
 * @brief DevQueryHardcopyCaps.
 * @return GPI_ERROR.
 */
EXTERN LONG APIENTRY DevQueryHardcopyCaps(HDC hdc, LONG lStart, LONG lCount,
                                          PHCINFO ahci)
{
    GPICallEnter("hdc=%p, lStart=%ld, lCount=%ld, ahci=%p",
                 (void*)hdc, (long)lStart, (long)lCount, (void*)ahci);
    GPICallResult("GPI_ERROR");
    (void)hdc; (void)lStart; (void)lCount; (void)ahci;
    return (LONG)GPI_ERROR;
}

/*!
 * @brief DevPostDeviceModes.
 * @return GPI_ERROR.
 */
EXTERN LONG APIENTRY DevPostDeviceModes(HAB hab, PDRIVDATA pdriv,
                                        PCSZ pszDriverName, PCSZ pszDeviceName,
                                        PCSZ pszLogAddress, ULONG flOptions)
{
    GPICallEnter("hab=%p, pdriv=%p, pszDriverName=%p, pszDeviceName=%p, pszLogAddress=%p, flOptions=0x%lx",
                 (void*)hab, (void*)pdriv, (void*)pszDriverName,
                 (void*)pszDeviceName, (void*)pszLogAddress,
                 (unsigned long)flOptions);
    GPICallResult("GPI_ERROR");
    (void)hab; (void)pdriv; (void)pszDriverName; (void)pszDeviceName;
    (void)pszLogAddress; (void)flOptions;
    return (LONG)GPI_ERROR;
}

/*!
 * @brief DevOpenDC.
 * @return NULL.
 */
EXTERN HDC APIENTRY DevOpenDC(HAB hab, LONG lType, PCSZ pszToken,
                              LONG lCount, PDEVOPENDATA pdop, HDC hdc)
{
    GPICallEnter("hab=%p, lType=%ld, pszToken=%p, lCount=%ld, pdop=%p, hdc=%p",
                 (void*)hab, (long)lType, (void*)pszToken, (long)lCount,
                 (void*)pdop, (void*)hdc);
    GPICallResult("NULL");
    (void)hab; (void)lType; (void)pszToken; (void)lCount;
    (void)pdop; (void)hdc;
    return (HDC)NULL;
}

#undef FPM_UNDOC_STUB

