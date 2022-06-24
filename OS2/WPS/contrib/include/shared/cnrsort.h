
/*
 *@@sourcefile cnrsort.h:
 *      header file for cnrsort.c, which contains the new folder
 *      sort comparison functions.
 *
 *      This file was called "xfsort.h" in V0.80.
 *
 *      See cnrsort.c for explanations.
 *
 *@@include #define INCL_WINSTDCNR
 *@@include #include <os2.h>
 *@@include #include "shared\cnrsort.h"
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef CNRSORT_HEADER_INCLUDED
    #define CNRSORT_HEADER_INCLUDED

    SHORT EXPENTRY fnCompareExt(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

    SHORT EXPENTRY fnCompareExtFoldersFirst(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

    SHORT EXPENTRY fnCompareName(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

    SHORT EXPENTRY fnCompareNameFoldersFirst(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

    SHORT EXPENTRY fnCompareType(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

    SHORT EXPENTRY fnCompareTypeFoldersFirst(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

    SHORT EXPENTRY fnCompareClass(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

    SHORT EXPENTRY fnCompareClassFoldersFirst(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

    SHORT EXPENTRY fnCompareRealName(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

    SHORT EXPENTRY fnCompareSize(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

    SHORT EXPENTRY fnCompareCommonDate(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2,
                ULONG ulWhat);

    SHORT EXPENTRY fnCompareLastWriteDate(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

    SHORT EXPENTRY fnCompareLastAccessDate(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

    SHORT EXPENTRY fnCompareCreationDate(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage);

#endif
