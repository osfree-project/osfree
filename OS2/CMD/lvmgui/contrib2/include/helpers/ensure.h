
/*
 *@@sourcefile ensure.h:
 *      header file for solid code support.
 *
 *      This header contains a set of helpers macros to help writing
 *      solid code.
 *
 *      You can only use those macros in functions that have a
 *      return type of APIRET.
 *
 *      Depending on whether a possible action requires additionnal
 *      cleanup or not, you can choose between two sets of macros.
 *
 *      If no extra cleanup is necessary, the simple ENSURE_OK,
 *      ENSURE(call) and ENSURE_FAIL(rc) can be freely used.
 *
 *      ENSURE_OK simply ends the execution of the function and
 *      returns NO_ERROR.
 *
 *      ENSURE(call) ends the execution of the function if call is
 *      not NO_ERROR.  Otherwise, nothing occurs.
 *
 *      ENSURE_FAIL(rc) ends the execution of the function and
 *      returns rc.
 *
 *      If some extra cleanup is required (say, dynamically allocated
 *      memory, etc.), then you must use ENSURE_BEGIN, ENSURE_SAFE(call),
 *      ENSURE_SAFEFAIL(rc), ENSURE_FINALLY, and ENSURE_END.
 *
 *      Those macros must be used in a block.  The block has to start
 *      with ENSURE_BEGIN, and must end with ENSURE_FINALLY ... ENSURE_END.
 *
 *      Here is an example:
 *
 +      APIRET foo(...)
 +      {
 +          ENSURE_BEGIN;
 +          ...
 +
 +          ENSURE_SAFE(bar());
 +          ...
 +
 +          ENSURE_FINALLY;
 +              // cleanup code
 +              ...
 +          ENSURE_END;
 +      }
 *
 *      The ENSURE_SAFE(call ) is the safe ENSURE(call) equivalent, and
 *      ENSURE_SAFEFAIL(rc) is the safe ENSURE_FAIL(rc) equivalent.
 *
 *      The cleanup code is only executed if ENSURE_SAFEFAIL has been
 *      called or if ENSURE_SAFE(call) resulted in call returning
 *      something that is not NO_ERROR.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include "helpers\ensure.h"
 */

/*      This file Copyright (C) 2001 Martin Lafaix.
 *
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef ENSURE_HEADER_INCLUDED
    #define ENSURE_HEADER_INCLUDED

    #define ENSURE_OK            return (NO_ERROR)
    #define ENSURE_FAIL(rc)      return (rc)
    #define ENSURE(call)         { APIRET __ensure_rc = call; \
                                   if (__ensure_rc != NO_ERROR) \
                                       return (__ensure_rc); }

    #define ENSURE_BEGIN         APIRET __ensuresafe_rc = NO_ERROR

    #define ENSURE_FINALLY       ENSURE_SAFEBLOCK: ; \
                                 if (__ensuresafe_rc != NO_ERROR) \
                                 {

    #define ENSURE_END           return (__ensuresafe_rc); }

    #define ENSURE_SAFE(call)    { __ensuresafe_rc = call; \
                                   if (__ensuresafe_rc != NO_ERROR) \
                                       goto ENSURE_SAFEBLOCK; }

    #define ENSURE_SAFEFAIL(rc)  { __ensuresafe_rc = rc; goto ENSURE_SAFEBLOCK; }
#endif

#if __cplusplus
}
#endif

