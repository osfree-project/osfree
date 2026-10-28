/*!
 * @file gpi_entry.c
 * @brief Client-side dispatcher (stub).
 */
#define INCL_GPI
#include <os2.h>
#include "gpi_entry.h"

/*!
 * @brief Client dispatcher (stub).
 * @param [in] hps Presentation space handle.
 * @param [in] ulFnCode Function code (NGpi*).
 * @param [in] pArgs Marshaled arguments.
 * @param [in] cbArgs Size of pArgs in bytes.
 * @param [out] pulResult Receives result from server.
 * @return 0 on success, non-zero on transport error.
 * @retval 1 Transport error (until real transport is wired).
 */
ULONG GpiDispatch(HPS hps, ULONG ulFnCode,
                  const void *pArgs, ULONG cbArgs,
                  ULONG *pulResult)
{
    (void)hps;
    (void)ulFnCode;
    (void)pArgs;
    (void)cbArgs;
    *pulResult = 0;
    return 1;
}
