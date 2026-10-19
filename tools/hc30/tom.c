/*!
 * @file tom.c
 * @brief |TOMAP implementation.
 */
#include "tom.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

/*!
 * @struct TomRec
 * @brief |TOMAP generator state.
 */
typedef struct { HVECTOR vOffsets; } TomRec;

/*!
 * @def TOM_FROM_HANDLE
 * @brief Convert an HTOM handle to its TomRec pointer.
 * @param[in] h Handle.
 */
#define TOM_FROM_HANDLE(h) ((TomRec*)(h))
/*!
 * @def TOM_HANDLE_FROM
 * @brief Convert a TomRec pointer to an HTOM handle.
 * @param[in] d Record pointer.
 */
#define TOM_HANDLE_FROM(d) ((HANDLE)(d))

/*!
 * @brief Create an empty |TOMAP generator.
 *
 * @param[out] phTom Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phTom is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY TomCreate(PHTOM phTom)
{
    TomRec* p; APIRET rc;
    if (!phTom) return ERROR_INVALID_PARAMETER;
    *phTom = NULLHANDLE;
    p = (TomRec*)malloc(sizeof(TomRec));
    if (!p) return ERROR_NOT_ENOUGH_MEMORY;
    memset(p, 0, sizeof(*p));
    rc = VectorCreate(sizeof(ULONG), 0, NULL, &p->vOffsets);
    if (rc != NO_ERROR) { free(p); return rc; }
    *phTom = TOM_HANDLE_FROM(p);
    return NO_ERROR;
}

/*!
 * @brief Destroy a |TOMAP generator and release all its data.
 *
 * @param[in] hTom Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR  Success. Also for NULLHANDLE.
 */
APIRET APIENTRY TomDestroy(HTOM hTom)
{
    TomRec* p;
    if (hTom == NULLHANDLE) return NO_ERROR;
    p = TOM_FROM_HANDLE(hTom);
    if (p->vOffsets) VectorDestroy(p->vOffsets);
    free(p);
    return NO_ERROR;
}

/*!
 * @brief Append a topic offset.
 *
 * @param[in] hTom     Handle. Not NULLHANDLE.
 * @param[in] ulOffset Byte offset of a topic header in |TOPIC.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hTom is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY TomAddOffset(HTOM hTom, ULONG ulOffset)
{
    TomRec* p;
    if (hTom == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    p = TOM_FROM_HANDLE(hTom);
    return VectorAdd(p->vOffsets, &ulOffset);
}

/*!
 * @brief Query the number of stored offsets.
 *
 * @param[in]  hTom     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY TomQueryOffsetCount(HTOM hTom, PULONG pulCount)
{
    if (hTom == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(TOM_FROM_HANDLE(hTom)->vOffsets, pulCount);
}

/*!
 * @brief Serialize the |TOMAP file to a stream.
 *
 * @param[in] hTom Handle. Not NULLHANDLE.
 * @param[in] f    Output stream. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hTom or @a f is NULL.
 */
APIRET APIENTRY TomWrite(HTOM hTom, FILE* f)
{
    TomRec* p; ULONG n, i;
    if (hTom == NULLHANDLE || !f) return ERROR_INVALID_PARAMETER;
    p = TOM_FROM_HANDLE(hTom);
    VectorGetCount(p->vOffsets, &n);
    for (i = 0; i < n; i++) {
        ULONG o;
        VectorGetItem(p->vOffsets, i, &o, sizeof(o), NULL);
        fwrite(&o, 4, 1, f);
    }
    return NO_ERROR;
}
