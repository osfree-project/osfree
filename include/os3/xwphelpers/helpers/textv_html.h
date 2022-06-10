
/*
 *@@sourcefile textv_html.h:
 *      header file for textv_html.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 */

/*      Copyright (C) 2000 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\linklist.h"
 *@@include #include "helpers\textv_html.h"
 */

#if __cplusplus
extern "C" {
#endif

#ifndef TXV_HTML_HEADER_INCLUDED
    #define TXV_HTML_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   HTML converter
     *
     ********************************************************************/

    BOOL txvConvertFromHTML(PSZ *ppszText,
                            PSZ *ppszTitle,
                            PULONG pulProgress,
                            PBOOL pfCancel);
#endif

#if __cplusplus
}
#endif

