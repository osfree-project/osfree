/*!
 * @file gpi_entry.h
 * @brief Client-side dispatcher interface.
 */
#ifndef GPI_ENTRY_H_INCLUDED
#define GPI_ENTRY_H_INCLUDED

#define INCL_GPI
#include "F_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @struct _GPI_PS
 * @brief Client-side per-PS state.
 *
 * Opaque to callers; owned by gpi_entry.cpp. Holds whatever the client
 * needs to know about a presentation space between calls (initially
 * flOptions as passed to GpiCreatePS / GpiSetPS; extended over time
 * with cached xform, current position, attr snapshots, etc.).
 */
typedef struct _GPI_PS {
    BOOL  inUse;      /*!< Slot occupied.                              */
    HPS   hps;        /*!< Server handle (used as map key).            */
    ULONG flOptions;  /*!< Last flOptions from GpiCreatePS/GpiSetPS.   */
    /* Extend here: matrices, current pos, cached attrs, ... */
} GPI_PS, *PGPI_PS;

/*!
 * @brief Register or update a PS entry.
 * @param [in] hps       Server handle.
 * @param [in] flOptions Options from GpiCreatePS/GpiSetPS.
 * @return Pointer to the entry, or NULL if the table is full or hps is invalid.
 */
PGPI_PS GpiPsCreate(HPS hps, ULONG flOptions);

/*!
 * @brief Find a PS entry by handle.
 * @param [in] hps Server handle.
 * @return Pointer to the entry, or NULL if not found.
 */
PGPI_PS GpiPsFind(HPS hps);

/*!
 * @brief Update flOptions for an existing PS entry (no-op if absent).
 * @param [in] hps       Server handle.
 * @param [in] flOptions Options from GpiSetPS.
 */
void GpiPsSetOptions(HPS hps, ULONG flOptions);

/*!
 * @brief Release a PS entry.
 * @param [in] hps Server handle.
 */
void GpiPsDestroy(HPS hps);

/*!
 * @brief Client dispatcher.
 *
 * @param [in] hps       Presentation space handle.
 * @param [in] ulFnCode  Function code (NGpi*).
 * @param [in] pArgs     Marshaled arguments.
 * @param [in] cbArgs    Size of pArgs in bytes.
 * @param [out] pulResult Receives result from server.
 * @return 0 on success, non-zero on transport error.
 * @retval 0 Successful transport; *pulResult holds the GPI result.
 * @retval 1 Transport-level or protocol error.
 */
ULONG GpiDispatch(HPS hps, ULONG ulFnCode,
                  const void *pArgs, ULONG cbArgs,
                  ULONG *pulResult);

#ifdef __cplusplus
}
#endif

#endif /* GPI_ENTRY_H_INCLUDED */
