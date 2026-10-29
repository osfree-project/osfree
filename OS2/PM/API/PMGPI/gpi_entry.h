/*!
 * @file gpi_entry.h
 * @brief Client-side dispatcher interface.
 */
#ifndef GPI_ENTRY_H_INCLUDED
#define GPI_ENTRY_H_INCLUDED

#define INCL_GPI
#include "gpi_platform.h"

/*!
 * @brief Client-side dispatcher.
 * @param [in] hps Presentation space handle.
 * @param [in] ulFnCode Function code (NGpi*).
 * @param [in] pArgs Marshaled arguments.
 * @param [in] cbArgs Size of pArgs in bytes.
 * @param [out] pulResult Receives result from server.
 * @return 0 on success, non-zero on transport error.
 * @retval 1 Transport error (stub).
 */
ULONG GpiDispatch(HPS hps, ULONG ulFnCode,
                  const void *pArgs, ULONG cbArgs,
                  ULONG *pulResult);

#endif
