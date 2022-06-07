
/*
 *@@sourcefile math.h:
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include "helpers\math.h"
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

#ifndef MATH_HEADER_INCLUDED
    #define MATH_HEADER_INCLUDED

    int XWPENTRY mathGCD(int m, int n);

    int XWPENTRY mathIsSquare(int x);

    int XWPENTRY mathFindFactor(int n);

    int XWPENTRY mathIsPrime(unsigned n);

    typedef int XWPENTRY FNFACTORCALLBACK(int iFactor,
                                          int iPower,
                                          void *pUser);
    typedef FNFACTORCALLBACK *PFNFACTORCALLBACK;

    int XWPENTRY mathFactorBrute(int n,
                                 PFNFACTORCALLBACK pfnCallback,
                                 void *pUser);

    int XWPENTRY mathFactorPrime(double n,
                                 PFNFACTORCALLBACK pfnCallback,
                                 void *pUser);

    int mathGCDMulti(int *paNs,
                     int cNs);

#endif


