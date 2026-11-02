/*!
 * @file gpi_entry.h
 * @brief Client-side dispatcher interface.
 */
#ifndef GPI_ENTRY_H_INCLUDED
#define GPI_ENTRY_H_INCLUDED

#define INCL_GPI
#include "F_platform.h"

#ifndef GPI_CLIENT
#define GPI_CLIENT
#endif
#include "gpi_ps.h"

#ifdef __cplusplus
extern "C" {
#endif

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
