
/*
 *@@sourcefile cnrsort.c:
 *      this file contains the new folder sort comparison functions only.
 *      It is used by the new sort methods in xfldr.c.
 *
 *      For introductory information on how XFolder extended sorting
 *      is implemented, refer to the XFolder programming guide.
 *
 *      This file was called "xfsort.c" in V0.80.
 *
 *      When sorting a container, one has to provide comparison
 *      functions which will be called by PM's internal sort
 *      algorithm (I don't know if it's a quicksort or whatever,
 *      but this doesn't matter anyway). In other words, one does
 *      _not_ have to write some sort algorithm oneself,
 *      but only provide a comparision function.
 *
 *      These functions are used in two contexts:
 *      1)  with the CM_SORTRECORD msg for sorting just once;
 *      2)  in the CNRINFO.pSortRecord field, when a container
 *          should always be sorted.
 *
 *      Sort comparison functions need to be like this:
 +          SHORT EXPENTRY fnCompareExt(PMINIRECORDCORE pmrc1,
 +                                      PMINIRECORDCORE pmrc2,
 +                                      PVOID pStorage)
 *
 *      Note: the information in the PM reference is flat out wrong.
 *      Container sort functions need to return the following:
 +          0   pmrc1 == pmrc2
 +         -1   pmrc1 <  pmrc2
 +         +1   pmrc1 >  pmrc2
 *
 *      All these sort functions are used by XFolder in the context
 *      of the extended sort functions. All the "pmrc" parameters
 *      therefore point to WPS MINIRECORDCOREs (not RECORDCOREs). To
 *      compare object titles, we can simply use the pszIcon fields.
 *      If we need the actual SOM objects, we can use the
 *      WPS's OBJECT_FROM_PREC macro.
 *
 *      Most of these use WinCompareString instead of strcmp so that
 *      PM's country settings are respected.
 *
 *@@header "shared\cnrsort.h"
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

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSSEMAPHORES

#define INCL_WINCOUNTRY
#define INCL_WINSTDCNR
#include <os2.h>

#include <stdio.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"                     // XFolder

#include "shared\cnrsort.h"
#include "shared\kernel.h"              // XWorkplace Kernel

#include "filesys\object.h"             // XFldObject implementation

#pragma hdrstop

/*
 *@@ fnCompareExt:
 *      container sort comparison function for
 *      sorting records by file name extension.
 *
 *      This one should work with any MINIRECORDCORE,
 *      not just Desktop objects.
 */

SHORT EXPENTRY fnCompareExt(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage)
{
    if ((pmrc1->pszIcon) && (pmrc2->pszIcon))
    {
        // find last dot char in icon titles;
        // strrchr finds LAST occurence (as opposed to strchr)
        PSZ pDot1 = strrchr(pmrc1->pszIcon, '.');
        PSZ pDot2 = strrchr(pmrc2->pszIcon, '.');
        if (pDot1 == NULL)
            if (pDot2 == NULL)
                // both titles have no extension: return names comparison
                switch (WinCompareStrings(G_habThread1,
                                          0,
                                          0,
                                          pmrc1->pszIcon,
                                          pmrc2->pszIcon,
                                          0))
                {
                    case WCS_LT: return -1;
                    case WCS_GT: return 1;
                    default: return 0;
                }
            else
                // pmrc1 has no ext, but pmrc2 does:
                return -1;
        else
            if (pDot2 == NULL)
                // pmrc1 has extension, but pmrc2 doesn't:
                return 1;

        // both records have extensions:
        // compare extensions
        switch (WinCompareStrings(G_habThread1,
                                  0,
                                  0,
                                  pDot1, pDot2,
                                  0))
        {
            case WCS_LT: return -1;
            case WCS_GT: return 1;
            default:
                // same ext: compare names
                switch (WinCompareStrings(G_habThread1,
                                          0,
                                          0,
                                          pmrc1->pszIcon,
                                          pmrc2->pszIcon,
                                          0))
                {
                    case WCS_LT: return -1;
                    case WCS_GT: return 1;
                }
        }
    }

    return 0;
}

/*
 *@@ fnCompareExtFoldersFirst:
 *
 *@@added V0.9.12 (2001-05-20) [umoeller]
 *@@changed V0.9.18 (2002-03-23) [umoeller]: optimized
 */

SHORT EXPENTRY fnCompareExtFoldersFirst(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage)
{
    WPObject *pobj1 = OBJECT_FROM_PREC(pmrc1);
    WPObject *pobj2 = OBJECT_FROM_PREC(pmrc2);

    WPObject *pobjDeref1 = _xwpResolveIfLink(pobj1);
    WPObject *pobjDeref2 = _xwpResolveIfLink(pobj2);
    BOOL IsFldr1 = (pobjDeref1)
                      ? _somIsA(pobjDeref1, _WPFolder)
                      : FALSE;      // treat broken shadows as non-folders
    BOOL IsFldr2 = (pobjDeref2)
                      ? _somIsA(pobjDeref2, _WPFolder)
                      : FALSE;      // treat broken shadows as non-folders

    if (IsFldr1 && IsFldr2)
        // both are folders:
        // compare by name then
        return fnCompareName(pmrc1, pmrc2, pStorage);
    else if (IsFldr1)
        return -1;
    else if (IsFldr2)
        return 1;

    // none of the two are folders:
    // only then compare extensions
    return fnCompareExt(pmrc1, pmrc2, pStorage);
}

/*
 *@@ fnCompareName:
 *      comparison func for sort by name.
 *
 *      This one should work with any MINIRECORDCORE,
 *      not just Desktop objects.
 */

SHORT EXPENTRY fnCompareName(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage)
{
    pStorage = pStorage; // to keep the compiler happy
    if ((pmrc1->pszIcon) && (pmrc2->pszIcon))
        switch (WinCompareStrings(G_habThread1,
                                  0,
                                  0,
                                  pmrc1->pszIcon,
                                  pmrc2->pszIcon,
                                  0))
        {
            case WCS_LT: return -1;
            case WCS_GT: return 1;
        }

    return 0;
}

/*
 *@@ fnCompareNameFoldersFirst:
 *
 *@@added V0.9.12 (2001-05-20) [umoeller]
 */

SHORT EXPENTRY fnCompareNameFoldersFirst(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage)
{
    WPObject *pobj1 = OBJECT_FROM_PREC(pmrc1);
    WPObject *pobj2 = OBJECT_FROM_PREC(pmrc2);

    WPObject *pobjDeref1 = _xwpResolveIfLink(pobj1);
    WPObject *pobjDeref2 = _xwpResolveIfLink(pobj2);
    BOOL IsFldr1 = (pobjDeref1)
                      ? _somIsA(pobjDeref1, _WPFolder)
                      : FALSE;      // treat broken shadows as non-folders
    BOOL IsFldr2 = (pobjDeref2)
                      ? _somIsA(pobjDeref2, _WPFolder)
                      : FALSE;      // treat broken shadows as non-folders

    if (IsFldr1 == IsFldr2)
        // both are folders, or both are NOT folders:
        // compare by name then
        return fnCompareName(pmrc1, pmrc2, pStorage);
    else if (IsFldr1)
        return -1;

    // else if IsFldr2)
    return 1;
}

/*
 *@@ fnCompareType:
 *      comparison func for sort by type (.TYPE EA)
 *
 *@@changed V0.9.18 (2002-03-23) [umoeller]: now comparing by name if types are equal
 */

SHORT EXPENTRY fnCompareType(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage)
{
    // get SOM objects from cnr record cores
    WPObject *pobj1 = OBJECT_FROM_PREC(pmrc1);
    WPObject *pobj2 = OBJECT_FROM_PREC(pmrc2);
    PSZ pType1 = NULL, pType2 = NULL;
    if ((pobj1) && (pobj2))
    {
        if (_somIsA(pobj1, _WPFileSystem))
            pType1 = _wpQueryType(pobj1);
        if (_somIsA(pobj2, _WPFileSystem))
            pType2 = _wpQueryType(pobj2);

        if (pType1)
        {
            if (pType2)
            {
                switch (WinCompareStrings(G_habThread1,
                                          0,
                                          0,
                                          pType1,
                                          pType2,
                                          0))
                {
                    case WCS_LT: return -1;
                    case WCS_GT: return 1;
                }
            }
            else
                // obj1 has type, obj2 has not
                return -1;
        }
        else
            if (pType2)
                // obj1 has NO type, but obj2 does
                return 1;

    }

    // types are equal, or both have no type:
    // compare by name instead
    // V0.9.18 (2002-03-23) [umoeller]
    return fnCompareName(pmrc1, pmrc2, pStorage);
}

/*
 *@@ fnCompareTypeFoldersFirst:
 *      comparison func for sort by type (.TYPE EA),
 *      but folders first.
 */

SHORT EXPENTRY fnCompareTypeFoldersFirst(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage)
{
    WPObject *pobj1 = OBJECT_FROM_PREC(pmrc1);
    WPObject *pobj2 = OBJECT_FROM_PREC(pmrc2);

    WPObject *pobjDeref1 = _xwpResolveIfLink(pobj1);
    WPObject *pobjDeref2 = _xwpResolveIfLink(pobj2);
    BOOL IsFldr1 = (pobjDeref1)
                      ? _somIsA(pobjDeref1, _WPFolder)
                      : FALSE;      // treat broken shadows as non-folders
    BOOL IsFldr2 = (pobjDeref2)
                      ? _somIsA(pobjDeref2, _WPFolder)
                      : FALSE;      // treat broken shadows as non-folders

    if (IsFldr1 && IsFldr2)
        // both are folders:
        // compare by name then
        return fnCompareName(pmrc1, pmrc2, pStorage);
    else if (IsFldr1)
        return -1;
    else if (IsFldr2)
        return 1;

    // none of the two are folders:
    // only then compare types
    return fnCompareType(pmrc1, pmrc2, pStorage);
}

/*
 *@@ fnCompareClass:
 *      comparison func for sort by object class
 *
 *@@changed V0.9.18 (2002-03-23) [umoeller]: now comparing by name if classes are equal
 */

SHORT EXPENTRY fnCompareClass(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage)
{
    // get SOM objects from cnr record cores
    WPObject *pobj1 = OBJECT_FROM_PREC(pmrc1);
    WPObject *pobj2 = OBJECT_FROM_PREC(pmrc2);

    PSZ pszClass1 = _wpclsQueryTitle(_somGetClass(pobj1));
    PSZ pszClass2 = _wpclsQueryTitle(_somGetClass(pobj2));

    if ((pszClass1) && (pszClass2))
        switch (WinCompareStrings(G_habThread1,
                                  0,
                                  0,
                                  pszClass1,
                                  pszClass2,
                                  0))
        {
            case WCS_LT: return -1;
            case WCS_GT: return 1;
        }

    // classes are equal:
    // compare by name instead
    // V0.9.18 (2002-03-23) [umoeller]
    return fnCompareName(pmrc1, pmrc2, pStorage);
}

/*
 *@@ fnCompareClassFoldersFirst:
 *      comparison func for sort by type (.TYPE EA),
 *      but folders first.
 */

SHORT EXPENTRY fnCompareClassFoldersFirst(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2, PVOID pStorage)
{
    WPObject *pobj1 = OBJECT_FROM_PREC(pmrc1);
    WPObject *pobj2 = OBJECT_FROM_PREC(pmrc2);

    WPObject *pobjDeref1 = _xwpResolveIfLink(pobj1);
    WPObject *pobjDeref2 = _xwpResolveIfLink(pobj2);
    BOOL IsFldr1 = (pobjDeref1)
                      ? _somIsA(pobjDeref1, _WPFolder)
                      : FALSE;      // treat broken shadows as non-folders
    BOOL IsFldr2 = (pobjDeref2)
                      ? _somIsA(pobjDeref2, _WPFolder)
                      : FALSE;      // treat broken shadows as non-folders

    if (IsFldr1 && IsFldr2)
        // both are folders:
        // compare by name then
        return fnCompareName(pmrc1, pmrc2, pStorage);
    else if (IsFldr1)
        return -1;
    else if (IsFldr2)
        return 1;

    // none of the two are folders:
    // only then compare classes
    return fnCompareClass(pmrc1, pmrc2, pStorage);
}

