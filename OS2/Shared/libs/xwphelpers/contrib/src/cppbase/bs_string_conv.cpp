
/*
 *@@sourcefile bs_string_conv.cpp:
 *      implementation for BSUniCodec.
 *
 *@@header "cppbase\bs_string.h"
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

/*
 *      This file Copyright (C) 2002-2008 Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSSEMAPHORES
#define INCL_DOSNLS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "setup.h"

#include "helpers\stringh.h"
#include "helpers\xstring.h"

// base includes
#include "cppbase\bs_base.h"
#include "cppbase\bs_string.h"
#include "cppbase\bs_errors.h"

#include "encodings\base.h"

#pragma hdrstop

#define CP_UTF8		1208

DEFINE_CLASS(BSUniCodec, BSRoot);

/*
 *@@ FindEncodingID:
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

static ENCID FindEncodingID(USHORT usCodepage,
                            BOOL *pfDouble)     // out: TRUE if double-byte cp
{
    ENCBYTECOUNT bc;
    ENCID id = encFindIdForCodepage(usCodepage, NULL, &bc);
    if (    id != UNSUPPORTED
         && (    (bc == SINGLE)
              || (bc == DOUBLE)
            )
       )
    {
        if (pfDouble)
            *pfDouble = (bc == DOUBLE);

        return (id);
    }

    throw BSUnsupportedCPExcpt(usCodepage);
}

/* ******************************************************************
 *
 *  BSUniCodec implementation
 *
 ********************************************************************/

/*
 *@@ BSUniCodec:
 *      constructor. Creates the internal conversion
 *      object by calling encCreateCodec.
 *
 *@@changed WarpIN V1.0.18 (2008-09-24) [pr]: added codepage 1208 support @@fixes 1127
 */

BSUniCodec::BSUniCodec(unsigned short usCodepage)      // in: codepage
    : BSRoot(tBSUniCodec),
      _usCodepage(usCodepage)
{
    ENCID id;

    // WarpIN V1.0.18
    if (usCodepage == CP_UTF8)
    {
	_pCodec = NULL;
	return;
    }

    id = FindEncodingID(usCodepage, &_fDouble);
    if (_fDouble)
    {
        APIRET arc;
        COUNTRYCODE cc = { 0, usCodepage };
        if (arc = DosQueryDBCSEnv(sizeof(_achDBCS),
                                  &cc,
                                  _achDBCS))
        {
            CHAR sz[200];
            sprintf(sz,
                    "DosQueryDBCSEnv returned error %d for codepage %d",
                    arc,
                    usCodepage);
            throw BSExcptBase(sz);
        }
    }

    _pCodec = encCreateCodec(id);
}

/*
 *@@ ~BSUniCodec:
 *      destructor. Frees the internal conversion object
 *      by calling encFreeCodec.
 *
 *@@changed WarpIN V1.0.18 (2008-09-24) [pr]: added codepage 1208 support @@fixes 1127
 */

BSUniCodec::~BSUniCodec()
{
    if (_pCodec)
    {
	encFreeCodec((PCONVERSION*)&_pCodec);
	_pCodec = NULL;
    }
}

/*
 *@@ IsLeadByte:
 *      returns TRUE if c is a DBCS lead byte.
 *
 *@@added V0.9.19 (2002-04-02) [umoeller]
 */

static BOOL IsLeadByte(CHAR c,          // in: character to test
                       PSZ pachDBCS)    // in: DBCS array from DosQueryDBCSEnv
{
    while (*pachDBCS)
    {
        if (    (c >= *pachDBCS++)
             && (c <= *pachDBCS++)
           )
            return TRUE;
    }

    return FALSE;
}

/*
 *@@ Codepage2Uni:
 *      converts the given string from codepage-specific
 *      to UTF-8. Used by BSString::assignUtf8 and
 *      others.
 *
 *@@changed WarpIN V1.0.18 (2008-09-24) [pr]: added codepage 1208 support @@fixes 1127
 */

void BSUniCodec::Codepage2Uni(BSUString &ustr,           // out: target
                              const char *pcszCP,        // in: cp string
                              unsigned long ulLength)    // in: length of cp string
{
    PCONVERSION pTable = (PCONVERSION)_pCodec;

    // WarpIN V1.0.18
    if (QueryCodepage() == CP_UTF8)
    {
	ustr.assignUtf8(pcszCP);
	return;
    }

    XSTRING xstrNew;
    xstrInit(&xstrNew, ulLength + 1);

    ULONG ul;
    for (ul = 0;
         ul < ulLength;
         ++ul)
    {
        unsigned short c = pcszCP[ul];

        if (_fDouble)
        {
            // we're using a double-byte codepage:
            // check if this is a DBCS char
            if (IsLeadByte(c, _achDBCS))
                c = (c << 8) | pcszCP[++ul];
        }

        // convert this char to Unicode, using the current codepage
        unsigned long ulUni = encChar2Uni(pTable, c);

        if (ulUni == 0xFFFF)
            // shouldn't happen
            xstrcatc(&xstrNew, '?');
        else if (ulUni < 0x80)
            xstrcatc(&xstrNew, (CHAR)ulUni);
        else if (ulUni < 0x800)
        {
            xstrcatc(&xstrNew, (CHAR)(0xC0 | ulUni>>6));
            xstrcatc(&xstrNew, (CHAR)(0x80 | ulUni & 0x3F));
        }
        else if (ulUni < 0x10000)
        {
            xstrcatc(&xstrNew, (CHAR)(0xE0 | ulUni>>12));
            xstrcatc(&xstrNew, (CHAR)(0x80 | ulUni>>6 & 0x3F));
            xstrcatc(&xstrNew, (CHAR)(0x80 | ulUni & 0x3F));
        }
        /* else if (ulUni < 0x200000)
        {
            xstrcatc(&xstrNew, (CHAR)(0xF0 | ulUni>>18));
            xstrcatc(&xstrNew, (CHAR)(0x80 | ulUni>>12 & 0x3F));
            xstrcatc(&xstrNew, (CHAR)(0x80 | ulUni>>6 & 0x3F));
            xstrcatc(&xstrNew, (CHAR)(0x80 | ulUni & 0x3F));
        } */
        else
        {
            CHAR sz[100];
            sprintf(sz,
                    "Unsupported Unicode character %u at offset %u in string",
                    ulUni,
                    ul);
            throw BSExcptBase(sz);
        }
    }

    // copy back
    ustr._take_from(xstrNew);
}

/*
 *@@ Uni2Codepage:
 *      converts the given string from UTF-8 to
 *      codepage-specific. Used by BSUString::assignCP
 *      and others.
 *
 *      Characters that are not supported with the
 *      given codepage are replaced by '?'.
 *
 *@@changed WarpIN V1.0.18 (2008-09-24) [pr]: added codepage 1208 support @@fixes 1127
 */

void BSUniCodec::Uni2Codepage(BSString &str,
                              const char *pcszUni,
                              unsigned long ulLength)
{
    // WarpIN V1.0.18
    if (QueryCodepage() == CP_UTF8)
    {
	str.assign(pcszUni);
	return;
    }

    PCONVERSION pTable = (PCONVERSION)_pCodec;

    XSTRING xstrNew;
    xstrInit(&xstrNew, ulLength + 1);

    const char *pFirst = pcszUni;

    while (*pcszUni)
    {
        unsigned long ulUni = encDecodeUTF8(&pcszUni);
        unsigned short us = encUni2Char(pTable, ulUni);
        if (us > 0xFF)
        {
            if (us == 0xFFFF)
                us = '?';
            else if (_fDouble)
            {
                // we're using a double-byte codepage:
                // store lead byte first
                xstrcatc(&xstrNew, (CHAR)(us >> 8));
            }
            else
            {
                // not double-byte codepage: then we can't handle > 0xFF
                CHAR sz[100];
                sprintf(sz,
                        "Unsupported Unicode character %u at offset %u in string",
                        ulUni,
                        pcszUni - pFirst);
                throw BSExcptBase(sz);

            }
        }
        xstrcatc(&xstrNew, (CHAR)us);
    }

    // copy back
    str._take_from(xstrNew);
}

