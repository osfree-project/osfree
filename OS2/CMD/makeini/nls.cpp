
/*
 *@@sourcefile nls.c:
 *      contains a few helpers for National Language Support (NLS),
 *      such as printing strings with the format specified by
 *      the "Country" object.
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  nls*        NLS helpers
 *
 *      This file is new with 0.9.16, but contains functions
 *      formerly in stringh.c.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\nls.h"
 *@@added V0.9.16 (2001-10-11) [umoeller]
 */

/*
 *      Copyright (C) 1997-2007 Ulrich M”ller.
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

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSNLS
#define INCL_DOSDATETIME
#define INCL_DOSERRORS
#define INCL_WINSHELLDATA
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unidef.h>
#include <uconv.h>

// XWP's setup.h replaces strchr and the like, and
// we want the originals in here
#define DONT_REPLACE_FOR_DBCS
#include "setup.h"                      // code generation and debugging options

#include "dosh.h"
#include "nls.h"
#include "prfh.h"
#include "standards.h"

#pragma hdrstop

// WarpIN V1.0.15 (2007-03-26) [pr]: @@fixes 936
// UCONV functions
int (* CALLCONV G_UniCreateUconvObject)(UniChar *code_set, UconvObject *uobj) = NULL;
int (* CALLCONV G_UniStrFromUcs)(UconvObject co, char *target, UniChar *source,
    int len) = NULL;
int (* CALLCONV G_UniFreeUconvObject)(UconvObject uobj) = NULL;
// LIBUNI functions
int (* CALLCONV G_UniCreateLocaleObject)(int locale_spec_type,
    const void *locale_spec, LocaleObject *locale_object_ptr) = NULL;
int (* CALLCONV G_UniQueryLocaleItem)(const LocaleObject locale_object,
    LocaleItem item, UniChar **info_item_addr_ptr) = NULL;
size_t (* CALLCONV G_UniStrlen)(const UniChar *ucs1) = NULL;
int (* CALLCONV G_UniFreeMem)(void *memory_ptr) = NULL;
int (* CALLCONV G_UniFreeLocaleObject)(LocaleObject locale_object) = NULL;

/*
 * G_aResolveFromUCONV:
 *      functions resolved from UCONV.DLL.
 *      Used with doshResolveImports.
 */

static const RESOLVEFUNCTION G_aResolveFromUCONV[] =
{
    "UniCreateUconvObject", (PFN*)&G_UniCreateUconvObject,
    "UniStrFromUcs", (PFN*)&G_UniStrFromUcs,
    "UniFreeUconvObject", (PFN*)&G_UniFreeUconvObject
};

/*
 * G_aResolveFromLIBUNI:
 *      functions resolved from LIBUNI.DLL.
 *      Used with doshResolveImports.
 */

static const RESOLVEFUNCTION G_aResolveFromLIBUNI[] =
{
    "UniCreateLocaleObject", (PFN*)&G_UniCreateLocaleObject,
    "UniQueryLocaleItem", (PFN*)&G_UniQueryLocaleItem,
    "UniStrlen", (PFN*)&G_UniStrlen,
    "UniFreeMem", (PFN*)&G_UniFreeMem,
    "UniFreeLocaleObject", (PFN*)&G_UniFreeLocaleObject
};

static BOOL fUniResolved = FALSE, fUniOK = FALSE;

/*
 *@@category: Helpers\National Language Support
 *      See nls.c.
 */

/* ******************************************************************
 *
 *   DBCS support
 *
 ********************************************************************/

#define MAX_LEADBYTE        256

#pragma pack(1)

typedef struct _DBCSVECTOR
{
   BYTE bLow;
   BYTE bHigh;
} DBCSVECTOR;

#pragma pack()

BOOL        G_afLeadByte[MAX_LEADBYTE] = {0};
ULONG       G_fDBCS = 2;       // not queried yet
COUNTRYCODE G_cc = { 0, 0 };
DBCSVECTOR  G_aDBCSVector[8];

/*
 *@@ nlsQueryCodepage:
 *      returns the current process codepage as a ULONG.
 *
 *@@added V1.0.2 (2003-02-07) [umoeller]
 */

ULONG nlsQueryCodepage(VOID)
{
    ULONG   acp[8];
    ULONG   cb = 0;
    if (DosQueryCp(sizeof(acp),
                   acp,
                   &cb))
        return 437;         // I think this is still the system default

    return acp[0];
}

/*
 *@@ nlsDBCS:
 *      returns TRUE if the system is currently using DBCS.
 *
 *@@added V0.9.19 (2002-06-13) [umoeller]
 *@@changed V0.9.20 (2002-07-03) [umoeller]: fixed, this never worked
 */

BOOL nlsDBCS(VOID)
{
    APIRET arc;

    if (G_fDBCS != 2)
        // already queried:
        return G_fDBCS;

    // V0.9.20 (2002-07-03) [umoeller]
    // assume a non-DBCS system UNLESS the below
    // loop gives us something meaningful; even
    // on non-DBCS systems like mine, DosQueryDBCSEnv
    // does not return an error
    G_fDBCS = FALSE;

    if (!(arc = DosQueryDBCSEnv(8 * sizeof(DBCSVECTOR),
                                &G_cc,
                                (PCHAR)G_aDBCSVector)))
    {
        int i;
        for (i = 0;
             i < 8;
             ++i)
        {
            if (    (G_aDBCSVector[i].bLow)
                 && (G_aDBCSVector[i].bHigh)
               )
            {
                int n;
                for (n = G_aDBCSVector[i].bLow;
                     n <= G_aDBCSVector[i].bHigh;
                     ++n)
                    G_afLeadByte[n] = TRUE;

                G_fDBCS = TRUE;
            }
            else
                break;
        }
    }

    return G_fDBCS;
}

/*
 *@@ nlsQueryDBCSChar:
 *      returns the type of the DBCS character with
 *      the given index. Note that the index is the
 *      DBCS character index, not just the array
 *      index into the CHAR array.
 *
 *      Returns:
 *
 *      --  TYPE_SBCS: ulOfs is single byte.
 *
 *      --  TYPE_DBCS_1ST: ulOfs is a double-byte lead char.
 *
 *      --  TYPE_DBCS_2ND: ulOfs is a double-byte trail char.
 *
 *      Preconditions:
 *
 *      --  nlsDBCS must have been called to initialize our
 *          globals, and must have returned TRUE.
 *
 *@@added V0.9.19 (2002-06-13) [umoeller]
 */

ULONG nlsQueryDBCSChar(PCSZ pcszString,
                       ULONG ulOfs)

{
    ULONG ulDBCSType = TYPE_SBCS;
    ULONG i;

    for (i = 0;
         i <= ulOfs;
         ++i)
    {
        switch (ulDBCSType)
        {
            case TYPE_SBCS:
            case TYPE_DBCS_2ND:
                ulDBCSType = G_afLeadByte[pcszString[i]];
            break;

            case TYPE_DBCS_1ST :
                ulDBCSType = TYPE_DBCS_2ND;
            break;
        }
    }

    return ulDBCSType;
}

/*
 *@@ nlschr:
 *      replacement for strchr with DBCS support.
 *
 *      If the system is not running with DBCS,
 *      this calls plain strchr automatically.
 *
 *@@added V0.9.19 (2002-06-13) [umoeller]
 *@@changed V0.9.20 (2002-07-22) [umoeller]: optimized
 *@@changed V0.9.20 (2002-07-22) [lafaix]: optimized
 */

PSZ nlschr(PCSZ p, char c)
{
    PCSZ    p2;
    ULONG   ulDBCSType = TYPE_SBCS;

    if (!nlsDBCS())
        // not DBCS:
        return strchr(p, c);

    // we're on DBCS:

    // we can't find c if it is a leading byte
    if (G_afLeadByte[c] != TYPE_SBCS)
        return NULL;

    for (p2 = p;
         *p2;
         ++p2)
    {
        // check _previous_ DBCS type and refresh
        // DBCS type accordingly
        switch (ulDBCSType)
        {
            case TYPE_SBCS:
            case TYPE_DBCS_2ND:
                ulDBCSType = G_afLeadByte[*p2];
                // we can safely do the test here (and skip rechecking
                // the type) because c can't possibly be a leading byte
                // V0.9.20 (2002-07-22) [lafaix]
                if (*p2 == c)
                    return (PSZ)p2;
            break;

            case TYPE_DBCS_1ST :
                ulDBCSType = TYPE_DBCS_2ND;
            break;
        }
    }

    /*  old code V0.9.20 (2002-07-22) [umoeller]
    // we're on DBCS:
    for (p2 = p;
         *p2;
         ++p2)
    {
        if (*p2 == c)
        {
            // match: return this only if it's SBCS;
            // if it's a DBCS lead char, skip it
            switch (ulDBCS = nlsQueryDBCSChar(p, p2 - p))
            {
                case TYPE_SBCS:
                    return (PSZ)p2;

                case TYPE_DBCS_1ST:
                    ++p2;
            }
        }
    }
    */

    return NULL;
}

/*
 *@@ nlsrchr:
 *      replacement for strrchr with DBCS support.
 *
 *      If the system is not running with DBCS,
 *      this calls plain strrchr automatically.
 *
 *@@added V0.9.19 (2002-06-13) [umoeller]
 *@@changed V0.9.20 (2002-07-22) [lafaix]: optimized
 */

PSZ nlsrchr(PCSZ p, char c)
{
    PCSZ    p2,
            pLast = NULL;
    ULONG   ulDBCSType = TYPE_SBCS;

    if (!nlsDBCS())
        // not DBCS:
        return strrchr(p, c);

    // we're on DBCS:

    // we can't find c if it is a leading byte
    if (G_afLeadByte[c] != TYPE_SBCS)
        return NULL;

    for (p2 = p;
         *p2;
         ++p2)
    {
        // check _previous_ DBCS type and refresh
        // DBCS type accordingly
        switch (ulDBCSType)
        {
            case TYPE_SBCS:
            case TYPE_DBCS_2ND:
                ulDBCSType = G_afLeadByte[*p2];
                if (*p2 == c)
                    pLast = p2;
            break;

            case TYPE_DBCS_1ST :
                ulDBCSType = TYPE_DBCS_2ND;
            break;
        }
    }

    return (PSZ)pLast;

    // old code V0.9.20 (2002-07-22) [lafaix]
    /*
    ulLength = strlen(p);
    for (p2 = p + ulLength - 1;
         p2 >= p;
         --p2)
    {
        if (*p2 == c)
        {
            // match: return this only if it's SBCS;
            // if it's a DBCS trail char, skip it
            switch (ulDBCS = nlsQueryDBCSChar(p, p2 - p))
            {
                case TYPE_SBCS:
                    return (PSZ)p2;

                case TYPE_DBCS_2ND:
                    --p2;
            }
        }
    }

    return NULL;
    */
}

/* ******************************************************************
 *
 *   Country-dependent formatting
 *
 ********************************************************************/

/*
 *@@ nlsGetAMPM:
 *
 *@@added V1.0.1 (2003-01-17) [umoeller]
 *@@changed V1.0.4 (2005-10-15) [pr]: Added support for Locale object settings on MCP systems @@fixes 614
 *@@changed V1.0.5 (2006-05-29) [pr]: Read Country rather than Locale settings on Warp 4 FP13+ @@fixes 614
 *@@changed WarpIN V1.0.15 (2007-03-26) [pr]: Rewritten to load UCONV/LIBUNI functions dynamically @@fixes 936
 */

VOID nlsGetAMPM(PCOUNTRYAMPM pampm)
{
    if (   (doshIsWarp4()==3)
        && (!fUniResolved)
       )
    {
        HMODULE hmodUCONV = NULLHANDLE,
                hmodLIBUNI = NULLHANDLE;

        fUniResolved = TRUE;
        if (   (doshResolveImports("UCONV.DLL",
                                   &hmodUCONV,
                                   G_aResolveFromUCONV,
                                   sizeof(G_aResolveFromUCONV) / sizeof(G_aResolveFromUCONV[0]))
                == NO_ERROR)
            && (doshResolveImports("LIBUNI.DLL",
                                   &hmodLIBUNI,
                                   G_aResolveFromLIBUNI,
                                   sizeof(G_aResolveFromLIBUNI) / sizeof(G_aResolveFromLIBUNI[0]))
                == NO_ERROR)
           )
            fUniOK = TRUE;
    }

    if (   (doshIsWarp4()==3)   // V1.0.5 (2006-05-29) [pr]
        && fUniOK
       )
    {
        UconvObject         uconv_object;

        if (G_UniCreateUconvObject((UniChar *)L"",
                                   &uconv_object) == ULS_SUCCESS)
        {
            LocaleObject locale_object;

            if (G_UniCreateLocaleObject(UNI_UCS_STRING_POINTER,
                                        (UniChar *)L"",
                                        &locale_object) == ULS_SUCCESS)
            {
                int i;
                struct LOCALE_ITEMLIST
                {
                    LocaleItem      lclItem;
                    PVOID           vTarget;
                    int             iType;
                } itemList[] = {
                    { LOCI_s2359, &pampm->sz2359, 3 },
                    { LOCI_s1159, &pampm->sz1159, 3 }
                               };

                for (i = 0;
                     i < sizeof(itemList) / sizeof(itemList[0]);
                     i++)
                {
                    UniChar *pItem;

                    if (G_UniQueryLocaleItem(locale_object,
                                             itemList[i].lclItem,
                                             &pItem) == ULS_SUCCESS)
                    {
                        int iLen = G_UniStrlen(pItem) + 1;
                        PSZ pszResult = (PSZ)malloc(iLen);

                        if (G_UniStrFromUcs(uconv_object,
                                            pszResult,
                                            pItem,
                                            iLen) == ULS_SUCCESS)
                        {
                            switch(itemList[i].iType)
                            {
                                case 1:
                                    *((ULONG *) itemList[i].vTarget) = atol(pszResult);
                                    break;

                                case 2:
                                    *((CHAR *) itemList[i].vTarget) = pszResult[0];
                                    break;

                                case 3:
                                    strcpy ((PCHAR)(itemList[i].vTarget), pszResult);
                                    break;
                            }
                        }

                        free(pszResult);
                        G_UniFreeMem(pItem);
                    }
                }

                G_UniFreeLocaleObject(locale_object);
            }

            G_UniFreeUconvObject(uconv_object);
        }
    }
    else
    {
        PrfQueryProfileString(HINI_USER,
                              "PM_National",
                              "s2359",        // key
                              "PM",           // default
                              pampm->sz2359,
                              sizeof(pampm->sz2359));

        PrfQueryProfileString(HINI_USER,
                              "PM_National",
                              "s1159",        // key
                              "AM",           // default
                              pampm->sz1159,
                              sizeof(pampm->sz1159));
    }
}

/*
 *@@ nlsQueryCountrySettings:
 *      this returns the most frequently used country settings
 *      all at once into a COUNTRYSETTINGS structure (prfh.h).
 *      This data corresponds to the user settings in the
 *      WPS "Country" object (which writes the data in "PM_National"
 *      in OS2.INI).
 *
 *      In case a key cannot be found, the following (English)
 *      default values are set:
 *      --  ulDateFormat = 0 (English date format, mm.dd.yyyy);
 *      --  ulTimeFormat = 0 (12-hour clock);
 *      --  cDateSep = '/' (date separator);
 *      --  cTimeSep = ':' (time separator);
 *      --  cDecimal = '.' (decimal separator).
 *      --  cThousands = ',' (thousands separator).
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.7 (2000-12-02) [umoeller]: added cDecimal
 *@@changed V1.0.4 (2005-10-15) [bvl]: Added support for Locale object settings on MCP systems @@fixes 614
 *@@changed V1.0.4 (2005-10-29) [pr]: Rewritten to prevent memory leaks and errors
 *@@changed V1.0.5 (2006-05-29) [pr]: Read Country rather than Locale settings on Warp 4 FP13+ @@fixes 614
 *@@changed WarpIN V1.0.15 (2007-03-26) [pr]: Rewritten to load UCONV/LIBUNI functions dynamically @@fixes 936
 */

VOID nlsQueryCountrySettings(PCOUNTRYSETTINGS2 pcs2)
{
    if (pcs2)
    {
        PCOUNTRYSETTINGS pcs = &pcs2->cs;

        if (   (doshIsWarp4()==3)
            && (!fUniResolved)
           )
        {
            HMODULE hmodUCONV = NULLHANDLE,
                    hmodLIBUNI = NULLHANDLE;

            fUniResolved = TRUE;
            if (   (doshResolveImports("UCONV.DLL",
                                       &hmodUCONV,
                                       G_aResolveFromUCONV,
                                       sizeof(G_aResolveFromUCONV) / sizeof(G_aResolveFromUCONV[0]))
                    == NO_ERROR)
                && (doshResolveImports("LIBUNI.DLL",
                                       &hmodLIBUNI,
                                       G_aResolveFromLIBUNI,
                                       sizeof(G_aResolveFromLIBUNI) / sizeof(G_aResolveFromLIBUNI[0]))
                    == NO_ERROR)
               )
                fUniOK = TRUE;
        }

        if (   (doshIsWarp4()==3)       // V1.0.5 (2006-05-29) [pr]
            && fUniOK
           )
        {
            UconvObject         uconv_object;

            if (G_UniCreateUconvObject((UniChar *)L"",
                                       &uconv_object) == ULS_SUCCESS)
            {
                LocaleObject locale_object;

                if (G_UniCreateLocaleObject(UNI_UCS_STRING_POINTER,
                                            (UniChar *)L"",
                                            &locale_object) == ULS_SUCCESS)
                {
                    int i;
                    struct LOCALE_ITEMLIST
                    {
                        LocaleItem      lclItem;
                        PVOID           vTarget;
                        int             iType;
                    } itemList[] = {
                        { LOCI_iDate, &pcs->ulDateFormat, 1 },
                        { LOCI_iTime, &pcs->ulTimeFormat, 1 },
                        { LOCI_sDate, &pcs->cDateSep, 2 },
                        { LOCI_sTime, &pcs->cTimeSep, 2 },
                        { LOCI_sDecimal, &pcs->cDecimal, 2 },
                        { LOCI_sThousand, &pcs->cThousands, 2 }
                                   };

                    for (i = 0;
                         i < sizeof(itemList) / sizeof(itemList[0]);
                         i++)
                    {
                        UniChar *pItem;

                        if (G_UniQueryLocaleItem(locale_object,
                                                 itemList[i].lclItem,
                                                 &pItem) == ULS_SUCCESS)
                        {
                            int iLen = G_UniStrlen(pItem) + 1;
                            PSZ pszResult = (PSZ)malloc(iLen);

                            if (G_UniStrFromUcs(uconv_object,
                                                pszResult,
                                                pItem,
                                                iLen) == ULS_SUCCESS)
                            {
                                switch(itemList[i].iType)
                                {
                                    case 1:
                                        *((ULONG *) itemList[i].vTarget) = atol(pszResult);
                                        break;

                                    case 2:
                                        *((CHAR *) itemList[i].vTarget) = pszResult[0];
                                        break;

                                    case 3:
                                        strcpy ((PCHAR)(itemList[i].vTarget), pszResult);
                                        break;
                                }
                            }

                            free(pszResult);
                            G_UniFreeMem(pItem);
                        }
                    }

                    G_UniFreeLocaleObject(locale_object);
                }

                G_UniFreeUconvObject(uconv_object);
            }
        }
        else
        {
            pcs->ulDateFormat = PrfQueryProfileInt(HINI_USER,
                                                   (PSZ)PMINIAPP_NATIONAL,
                                                   "iDate",
                                                   0);
            pcs->ulTimeFormat = PrfQueryProfileInt(HINI_USER,
                                                   (PSZ)PMINIAPP_NATIONAL,
                                                   "iTime",
                                                   0);
            pcs->cDateSep = prfhQueryProfileChar(HINI_USER,
                                                 (PSZ)PMINIAPP_NATIONAL,
                                                 "sDate",
                                                 '/');
            pcs->cTimeSep = prfhQueryProfileChar(HINI_USER,
                                                 (PSZ)PMINIAPP_NATIONAL,
                                                 "sTime",
                                                 ':');
            pcs->cDecimal = prfhQueryProfileChar(HINI_USER,
                                                 (PSZ)PMINIAPP_NATIONAL,
                                                 "sDecimal",
                                                 '.');
            pcs->cThousands = prfhQueryProfileChar(HINI_USER,
                                                   (PSZ)PMINIAPP_NATIONAL,
                                                   "sThousand",
                                                   ',');
        }

        nlsGetAMPM(&pcs2->ampm);
    }
}

/*
 *@@ nlsThousandsULong:
 *      converts a ULONG into a decimal string, while
 *      inserting thousands separators into it. Specify
 *      the separator character in cThousands.
 *
 *      Returns pszTarget so you can use it directly
 *      with sprintf and the "%s" flag.
 *
 *      For cThousands, you should use the data in
 *      OS2.INI ("PM_National" application), which is
 *      always set according to the "Country" object.
 *      You can use nlsQueryCountrySettings to
 *      retrieve this setting.
 *
 *      Use nlsThousandsDouble for "double" values.
 *
 *@@changed V0.9.20 (2002-07-03) [umoeller]: optimized
 */

PSZ nlsThousandsULong(PSZ pszTarget,       // out: decimal as string
                      ULONG ul,            // in: decimal to convert
                      CHAR cThousands)     // in: separator char (e.g. '.')
{
    CHAR    szTemp[30];
    USHORT  ust = 0,
            uss,
            usLen = sprintf(szTemp, "%lu", ul);  // V0.9.20 (2002-07-03) [umoeller]

    for (uss = 0;
         uss < usLen;
         ++uss)
    {
        if (uss)
            if (0 == ((usLen - uss) % 3))
            {
                pszTarget[ust] = cThousands;
                ust++;
            }

        pszTarget[ust++] = szTemp[uss];
    }

    pszTarget[ust] = '\0';

    return pszTarget;
}

/*
 * strhThousandsULong:
 *      wrapper around nlsThousandsULong for those
 *      who used the XFLDR.DLL export.
 *
 *added V0.9.16 (2001-10-11) [umoeller]
 */

PSZ APIENTRY strhThousandsULong(PSZ pszTarget,       // out: decimal as string
                                ULONG ul,            // in: decimal to convert
                                CHAR cThousands)     // in: separator char (e.g. '.')
{
    return nlsThousandsULong(pszTarget, ul, cThousands);
}

/*
 *@@ nlsThousandsDouble:
 *      like nlsThousandsULong, but for a "double"
 *      value. Note that after-comma values are truncated.
 *
 *@@changed V0.9.20 (2002-07-03) [umoeller]: optimized
 */

PSZ nlsThousandsDouble(PSZ pszTarget,
                       double dbl,
                       CHAR cThousands)
{
    USHORT ust, uss, usLen;
    CHAR   szTemp[40];
    usLen = sprintf(szTemp, "%.0f", floor(dbl)); // V0.9.20 (2002-07-03) [umoeller]

    ust = 0;
    for (uss = 0; uss < usLen; uss++)
    {
        if (uss)
            if (((usLen - uss) % 3) == 0)
            {
                pszTarget[ust] = cThousands;
                ust++;
            }
        pszTarget[ust] = szTemp[uss];
        ust++;
    }
    pszTarget[ust] = '\0';

    return pszTarget;
}

/*
 *@@ nlsVariableDouble:
 *      like nlsThousandsULong, but for a "double" value, and
 *      with a variable number of decimal places depending on the
 *      size of the quantity.
 *
 *@@added V0.9.6 (2000-11-12) [pr]
 *@@changed V0.9.20 (2002-07-03) [umoeller]: now using PCSZ pcszUnits
 */

PSZ nlsVariableDouble(PSZ pszTarget,
                      double dbl,
                      PCSZ pcszUnits,
                      CHAR cThousands)
{
    if (dbl < 100.0)
        sprintf(pszTarget, "%.2f%s", dbl, pcszUnits);
    else
        if (dbl < 1000.0)
            sprintf(pszTarget, "%.1f%s", dbl, pcszUnits);
        else
            strcat(nlsThousandsDouble(pszTarget, dbl, cThousands),
                   pcszUnits);

    return pszTarget;
}

/*
 *@@ nlsFileDate:
 *      converts file date data to a string (to pszBuf).
 *      You can pass any FDATE structure to this function,
 *      which are returned in those FILEFINDBUF* or
 *      FILESTATUS* structs by the Dos* functions.
 *
 *      ulDateFormat is the PM setting for the date format,
 *      as set in the "Country" object, and can be queried using
 +              PrfQueryProfileInt(HINI_USER, "PM_National", "iDate", 0);
 *
 *      meaning:
 *      --  0   mm.dd.yyyy  (English)
 *      --  1   dd.mm.yyyy  (e.g. German)
 *      --  2   yyyy.mm.dd  (Japanese, ISO)
 *      --  3   yyyy.dd.mm
 *
 *      cDateSep is used as a date separator (e.g. '.').
 *      This can be queried using:
 +          prfhQueryProfileChar(HINI_USER, "PM_National", "sDate", '/');
 *
 *      Alternatively, you can query all the country settings
 *      at once using nlsQueryCountrySettings (prfh.c).
 *
 *@@changed V0.9.0 (99-11-07) [umoeller]: now calling nlsDateTime
 *@@changed V1.0.1 (2003-01-17) [umoeller]: prototype changed for optimization, this was not exported
 */

VOID nlsFileDate(PSZ pszBuf,            // out: string returned
                 const FDATE *pfDate,   // in: date information
                 const COUNTRYSETTINGS2 *pcs)
{
    nlsDate(pcs,
            pszBuf,
            pfDate->year + 1980,
            pfDate->month,
            pfDate->day);
}

/*
 *@@ nlsFileTime:
 *      converts file time data to a string (to pszBuf).
 *      You can pass any FTIME structure to this function,
 *      which are returned in those FILEFINDBUF* or
 *      FILESTATUS* structs by the Dos* functions.
 *
 *      ulTimeFormat is the PM setting for the time format,
 *      as set in the "Country" object, and can be queried using
 +              PrfQueryProfileInt(HINI_USER, "PM_National", "iTime", 0);
 *      meaning:
 *      --  0   12-hour clock
 *      --  >0  24-hour clock
 *
 *      cDateSep is used as a time separator (e.g. ':').
 *      This can be queried using:
 +              prfhQueryProfileChar(HINI_USER, "PM_National", "sTime", ':');
 *
 *      Alternatively, you can query all the country settings
 *      at once using nlsQueryCountrySettings (prfh.c).
 *
 *@@changed V0.8.5 (99-03-15) [umoeller]: fixed 12-hour crash
 *@@changed V0.9.0 (99-11-07) [umoeller]: now calling nlsDateTime
 *@@changed V1.0.1 (2003-01-17) [umoeller]: prototype changed for optimization, this was not exported
 */

VOID nlsFileTime(PSZ pszBuf,           // out: string returned
                 const FTIME *pfTime,        // in: time information
                 const COUNTRYSETTINGS2 *pcs)
{
    nlsTime(pcs,
            pszBuf,
            pfTime->hours,
            pfTime->minutes,
            pfTime->twosecs * 2);
}

/*
 *@@ nlsDate:
 *
 *@@added V1.0.1 (2003-01-17) [umoeller]
 */

VOID nlsDate(const COUNTRYSETTINGS2 *pcs2,
             PSZ pszDate,                   // out: date string returned
             USHORT year,
             BYTE month,
             BYTE day)
{
    switch (pcs2->cs.ulDateFormat)
    {
        case 0:  // mm.dd.yyyy  (English)
            sprintf(pszDate, "%02d%c%02d%c%04d",
                    month,
                        pcs2->cs.cDateSep,
                    day,
                        pcs2->cs.cDateSep,
                    year);
        break;

        case 1:  // dd.mm.yyyy  (e.g. German)
            sprintf(pszDate, "%02d%c%02d%c%04d",
                    day,
                        pcs2->cs.cDateSep,
                    month,
                        pcs2->cs.cDateSep,
                    year);
        break;

        case 2: // yyyy.mm.dd  (ISO and Japanese)
            sprintf(pszDate, "%04d%c%02d%c%02d",
                    year,
                        pcs2->cs.cDateSep,
                    month,
                        pcs2->cs.cDateSep,
                    day);
        break;

        default: // yyyy.dd.mm
            sprintf(pszDate, "%04d%c%02d%c%02d",
                    year,
                        pcs2->cs.cDateSep,
                    day,
                        pcs2->cs.cDateSep,
                    month);
        break;
    }
}

/*
 *@@ nlsTime:
 *
 *@@added V1.0.1 (2003-01-17) [umoeller]
 */

VOID nlsTime(const COUNTRYSETTINGS2 *pcs2,
             PSZ pszTime,                   // out: time string returned
             BYTE hours,
             BYTE minutes,
             BYTE seconds)
{
    if (!pcs2->cs.ulTimeFormat)
    {
        // for 12-hour clock, we need additional INI data
        if (hours >= 12)  // V0.9.16 (2001-12-05) [pr] if (hours > 12)
        {
            // yeah cool Paul, now we get 00:20 PM if it's 20 past noon
            // V0.9.18 (2002-02-13) [umoeller]
            ULONG ulHours;
            if (!(ulHours = hours % 12))
                ulHours = 12;

            // >= 12h: PM.
            sprintf(pszTime, "%02d%c%02d%c%02d %s",
                    // leave 12 == 12 (not 0)
                    ulHours,
                        pcs2->cs.cTimeSep,
                    minutes,
                        pcs2->cs.cTimeSep,
                    seconds,
                    pcs2->ampm.sz2359);
        }
        else
        {
            // < 12h: AM
            sprintf(pszTime, "%02d%c%02d%c%02d %s",
                    hours,
                        pcs2->cs.cTimeSep,
                    minutes,
                        pcs2->cs.cTimeSep,
                    seconds,
                    pcs2->ampm.sz1159);
        }
    }
    else
        // 24-hour clock
        sprintf(pszTime, "%02d%c%02d%c%02d",
                hours,
                    pcs2->cs.cTimeSep,
                minutes,
                    pcs2->cs.cTimeSep,
                seconds);
}

/*
 *@@ nlsDateTime:
 *      converts Control Program DATETIME info
 *      into two strings. See nlsFileDate and nlsFileTime
 *      for more detailed parameter descriptions.
 *
 *      Use of this function is deprecated. Use the
 *      speedier nlsDateTime2 instead.
 *
 *@@added V0.9.0 (99-11-07) [umoeller]
 *@@changed V0.9.16 (2001-12-05) [pr]: fixed AM/PM hour bug
 *@@changed V0.9.18 (2002-02-13) [umoeller]: fixed AM/PM hour bug fix
 *@@changed V1.0.1 (2003-01-17) [umoeller]: extracted nlsDate, nlsTime
 */

VOID nlsDateTime(PSZ pszDate,          // out: date string returned (can be NULL)
                 PSZ pszTime,          // out: time string returned (can be NULL)
                 const DATETIME *pDateTime,  // in: date/time information
                 ULONG ulDateFormat,   // in: date format (0-3); see nlsFileDate
                 CHAR cDateSep,        // in: date separator (e.g. '.')
                 ULONG ulTimeFormat,   // in: 24-hour time format (0 or 1); see nlsFileTime
                 CHAR cTimeSep)        // in: time separator (e.g. ':')
{
    COUNTRYSETTINGS2 cs2;
    if (pszDate)
    {
        cs2.cs.ulDateFormat = ulDateFormat;
        cs2.cs.cDateSep = cDateSep;
        nlsDate(&cs2,
                pszDate,                   // out: date string returned
                pDateTime->year,
                pDateTime->month,
                pDateTime->day);
    }

    if (pszTime)
    {
        cs2.cs.ulTimeFormat = ulTimeFormat;
        cs2.cs.cTimeSep = cTimeSep;
        nlsGetAMPM(&cs2.ampm);
        nlsTime(&cs2,
                pszTime,
                pDateTime->hours,
                pDateTime->minutes,
                pDateTime->seconds);
    }
}

/*
 * strhDateTime:
 *      wrapper around nlsDateTime for those who used
 *      the XFLDR.DLL export.
 */

VOID APIENTRY strhDateTime(PSZ pszDate,          // out: date string returned (can be NULL)
                           PSZ pszTime,          // out: time string returned (can be NULL)
                           DATETIME *pDateTime,  // in: date/time information
                           ULONG ulDateFormat,   // in: date format (0-3); see nlsFileDate
                           CHAR cDateSep,        // in: date separator (e.g. '.')
                           ULONG ulTimeFormat,   // in: 24-hour time format (0 or 1); see nlsFileTime
                           CHAR cTimeSep)        // in: time separator (e.g. ':')
{
    nlsDateTime(pszDate,
                pszTime,
                pDateTime,
                ulDateFormat,
                cDateSep,
                ulTimeFormat,
                cTimeSep);
}

/*
 *@@ nlsDateTime2:
 *      speedier version of nlsDateTime that caches
 *      all information and needs _no_ Prf* call
 *      any more.
 *
 *@@added V1.0.1 (2003-01-17) [umoeller]
 */

VOID nlsDateTime2(PSZ pszDate,
                  PSZ pszTime,
                  const DATETIME *pDateTime,
                  const COUNTRYSETTINGS2 *pcs2)
{
    if (pszDate)
        nlsDate(pcs2,
                pszDate,
                pDateTime->year,
                pDateTime->month,
                pDateTime->day);

    if (pszTime)
        nlsTime(pcs2,
                pszTime,
                pDateTime->hours,
                pDateTime->minutes,
                pDateTime->seconds);
}

CHAR G_szUpperMap[257];
BOOL G_fUpperMapInited = FALSE;

/*
 *@@ InitUpperMap:
 *      initializes the case map for nlsUpper.
 *
 *@@added V0.9.20 (2002-07-25) [umoeller]
 */

STATIC VOID InitUpperMap(VOID)
{
    ULONG ul;
    COUNTRYCODE cc;
    BOOL fDBCS = nlsDBCS();

    for (ul = 0;
         ul < 256;
         ++ul)
    {
        G_szUpperMap[ul] = (CHAR)ul;

        if (    (fDBCS)
             && (G_afLeadByte[ul] != TYPE_SBCS)
           )
            G_szUpperMap[ul] = ' ';
    }

    G_szUpperMap[256] = '\0';

    cc.country = 0;         // use system country code
    cc.codepage = 0;        // use process default codepage
    DosMapCase(255,
               &cc,
               G_szUpperMap + 1);

    G_fUpperMapInited = TRUE;
}

/*
 *@@ nlsUpper:
 *      quick hack for upper-casing a string.
 *
 *      This now returns the length of the given string always
 *      (V0.9.20).
 *
 *      Remarks:
 *
 *      --  On the first call, we build a case map table by
 *          calling DosMapCase with the default system country
 *          code and the process's codepage. Since DosMapCase
 *          is terribly slow with all the 16-bit thunking
 *          involved, we can then use our table without having
 *          to use the API ever again.
 *
 *      --  As a result, if the process codepage changes for
 *          any reason, this function will not pick up the
 *          change.
 *
 *      --  This has provisions for DBCS, which hopefully
 *          work.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: speedup, changed prototype
 */

ULONG nlsUpper(PSZ psz)            // in/out: string
{
    ULONG   ul = 0;
    PSZ     p;

    if (!G_fUpperMapInited)
        InitUpperMap();

    if (p = psz)
    {
        while (*p++ = G_szUpperMap[*p])
            ++ul;
    }

    return ul;
}


