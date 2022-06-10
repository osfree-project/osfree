
/*
 *@@sourcefile standards.h:
 *      some things that are always needed and never
 *      declared in a common place. Here you go.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include "helpers\standards.h"
 */

/*      Copyright (C) 2001 Ulrich M”ller.
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

#ifndef STANDARDS_HEADER_INCLUDED
    #define STANDARDS_HEADER_INCLUDED

    /*
     *@@ NEW:
     *      wrapper around the typical malloc struct
     *      sequence.
     *
     *      Usage:
     *
     +          STRUCT *p = NEW(STRUCT);
     *
     *      This expands to:
     *
     +          STRUCT *p = (STRUCT *)malloc(sizeof(STRUCT));
     *
     *@@added V0.9.9 (2001-04-01) [umoeller]
     */

    #define NEW(type) (type *)malloc(sizeof(type))

    /*
     *@@ ZERO:
     *      wrapper around the typical zero-struct
     *      sequence.
     *
     *      Usage:
     *
     +          ZERO(p)
     *
     *      This expands to:
     *
     +          memset(p, 0, sizeof(*p));
     *
     *@@added V0.9.9 (2001-04-01) [umoeller]
     */

    #define ZERO(ptr) memset(ptr, 0, sizeof(*ptr))

    /*
     *@@ FREE:
     *      wrapper around the typical free() sequence.
     *
     *      Usage:
     *
     +          FREE(p)
     *
     *      This expands to:
     *
     +          if (p)
     +          {
     +              free(p);
     +              p = NULL;
     +          }
     *
     *@@added V0.9.16 (2001-12-08) [umoeller]
     */

    #define FREE(ptr)  { if ((ptr)) { free(ptr); ptr = NULL; } }

    /*
     *@@ ARRAYITEMCOUNT:
     *      helpful macro to count the count of items
     *      in an array. Use this to avoid typos when
     *      having to pass the array item count to
     *      a function.
     *
     *      ULONG   aul[] = { 0, 1, 2, 3, 4 };
     *
     *      ARRAYITEMCOUNT(aul) then expands to:
     *
     +          sizeof(aul) / sizeof(aul[0])
     *
     *      which should return 5. Note that the compiler
     *      should calculate this at compile-time, so that
     *      there is no run-time overhead... and this will
     *      never miscount the array item size.
     *
     *@@added V0.9.9 (2001-01-29) [umoeller]
     */

    #define ARRAYITEMCOUNT(array) (sizeof(array) / sizeof(array[0]))

    /*
     *@@ STRINGORNULL:
     *      helpful macro to avoid passing null strings
     *      to debugging printf calls.
     *
     *@@added V0.9.16 (2002-01-05) [umoeller]
     */

    #define STRINGORNULL(s) (s) ? (s) : "NULL"

    /*
     *@@ STOREIFMAX:
     *      helpful macro to store a in b if it is
     *      larger than b.
     *
     *@@added V1.0.1 (2002-11-30) [umoeller]
     */

    #define STOREIFMAX(a, b) if ((a) > (b)) { b = a; }

    #ifdef _PMPRINTF_
        #define PMPF_RECT(s, prect) _PmpfF(("%s: xLeft %d, xRight %d, yBottom %d, yTop %d", (s), (prect)->xLeft, (prect)->xRight, (prect)->yBottom, (prect)->yTop))
    #else
        #define PMPF_RECT(s, prect)
    #endif

#endif


