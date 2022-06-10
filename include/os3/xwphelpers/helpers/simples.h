
/*
 *@@sourcefile simples.h:
 *      some basic typical OS/2 definitions that are used in
 *      some helpers include files that should also work
 *      when os2.h is not included.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 */

/*      Copyright (C) 2002 Ulrich M”ller.
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


#ifndef __SIMPLES_DEFINED
    // define some basic things to make this work even with standard C
    #if (!defined OS2_INCLUDED) && (!defined _OS2_H) && (!defined __SIMPLES_DEFINED)   // changed V0.9.0 (99-10-22) [umoeller]
        typedef void VOID;
        typedef unsigned long BOOL;
        typedef long LONG;
        typedef long *PLONG;
        typedef unsigned long ULONG;
        typedef unsigned long *PULONG;
        typedef unsigned char *PSZ;
        typedef const unsigned char *PCSZ;
        #define TRUE (BOOL)1
        #define FALSE (BOOL)0

        #ifdef __IBMCPP__               // added V0.9.0 (99-10-22) [umoeller]
            #define APIENTRY _System
        #endif

        #define __SIMPLES_DEFINED

    #endif

#endif


