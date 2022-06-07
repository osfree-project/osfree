
/*
 *@@sourcefile encodings.c:
 *      character encoding support. Handles all kinds
 *      of legacy codepages (including most OS/2 codepages)
 *      and Unicode in the form of UTF-8 and translations
 *      between then.
 *
 *      See encCreateCodec for an introduction.
 *
 *      See http://www.ietf.org/rfc/rfc2279.txt for
 *      RFC 2279, which defines UTF-8.
 *
 *      Be warned, compilation of this file takes a long
 *      time because this includes all the complex codepages
 *      from include\encodings.
 *
 *@@header "encodings\base.h"
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

/*
 *      Copyright (C) 2001-2008 Ulrich M”ller.
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

#include <stdlib.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\standards.h"

#include "encodings\base.h"

#include "encodings\unicase.h"

#include "encodings\alltables.h"        // this takes a very long time

#pragma hdrstop

/*
 *@@category: Helpers\National Language Support\Encodings
 *      See encodings.c.
 */

/*
 *@@ G_aEncodings:
 *      list of all encodings supported by this engine
 *      (i.e. we have a corresponding codepage in
 *      include\encodings\*.h) together with some
 *      additional information for each encoding,
 *      such as the corresponding OS/2 codepage
 *      number and a descriptive string.
 *
 *      For a way too extensive list of codepage
 *      names, see "http://www.iana.org/assignments/character-sets".
 *
 *@@added V [umoeller]
 */

struct
{
    ENCID               id;                 // engine ID (enum)
    PXWPENCODINGMAP     pMap;               // ptr to map from include\encodings\*.h
    unsigned long       cEntries;           // entries in map (array item count)
    unsigned short      usCodepageOS2;      // corresponding OS/2 codepage or 0 if none
                                            // V1.0.0 (2002-08-21) [umoeller]
    unsigned short      usLatin;            // ISO 8859-X correspondance or 0
    ENCBYTECOUNT        bc;
    const char          *pcszDescription;   // description
} G_aEncodings[] =
    {
        #define ENCODINGENTRY(id)   enc_ ## id, G_ ## id, ARRAYITEMCOUNT(G_ ## id)

        ENCODINGENTRY(cp437),      437,  0, SINGLE, "DOS Latin US",
        ENCODINGENTRY(cp737),      737,  0, SINGLE, "DOS Greek",
        ENCODINGENTRY(cp775),      775,  0, SINGLE, "DOS BaltRim",
        ENCODINGENTRY(cp850),      850,  0, SINGLE, "DOS Latin 1",
        ENCODINGENTRY(cp852),      852,  0, SINGLE, "DOS Latin 2",               // default in Hungary,
                                                                                 // Romania, Poland
        ENCODINGENTRY(cp855),      855,  0, SINGLE, "DOS Cyrillic",
        ENCODINGENTRY(cp857),      857,  0, SINGLE, "DOS Latin 5 (Turkish)",
        ENCODINGENTRY(cp860),      860,  0, SINGLE, "DOS Portuguese",
        ENCODINGENTRY(cp861),      861,  0, SINGLE, "DOS Icelandic",
        ENCODINGENTRY(cp862),      862,  0, SINGLE, "DOS Hebrew",
        ENCODINGENTRY(cp863),      863,  0, SINGLE, "DOS Canadian French",
        ENCODINGENTRY(cp864),      864,  0, SINGLE, "DOS Arabic",                // default in Egypt
        ENCODINGENTRY(cp865),      865,  0, SINGLE, "DOS Nordic",
        ENCODINGENTRY(cp866),      866,  0, SINGLE, "DOS Cyrillic Russian",      // default in Russia
        ENCODINGENTRY(cp869),      869,  0, SINGLE, "DOS Greek2",
        ENCODINGENTRY(cp874),      874,  0, SINGLE, "DOS Thai (TIS-620)",        // default in Thailand

        ENCODINGENTRY(cp932),      932 /* or 943?*/ ,
                                         0, DOUBLE, "Japanese Windows",
        ENCODINGENTRY(cp936),      936 /* or 946?*/ ,
                                         0, DOUBLE, "Chinese",
        ENCODINGENTRY(cp949),      949 /* was 951, fixed V1.0.2 (2003-09-19) [umoeller] */ ,
                                         0, DOUBLE, "Korean",
        ENCODINGENTRY(cp950),      950 /* was 947, fixed V1.0.2 (2003-09-19) [umoeller] */ ,
                                         0, DOUBLE, "Taiwan Big-5",           // default in China?

        ENCODINGENTRY(cp1004),    1004,  0, SINGLE, "Windows Extended",
        ENCODINGENTRY(cp1250),    1250,  0, SINGLE, "Windows Latin 2",
        ENCODINGENTRY(cp1251),    1251,  0, SINGLE, "Windows Cyrillic",
        ENCODINGENTRY(cp1252),    1252,  0, SINGLE, "Windows Latin 1",
        ENCODINGENTRY(cp1253),    1253,  0, SINGLE, "Windows Greek",
        ENCODINGENTRY(cp1254),    1254,  0, SINGLE, "Windows Turkish",
        ENCODINGENTRY(cp1255),    1255,  0, SINGLE, "Windows Hebrew",
        ENCODINGENTRY(cp1256),    1256,  0, SINGLE, "Windows Arabic",
        ENCODINGENTRY(cp1257),    1257,  0, SINGLE, "Windows Latin-4",
        ENCODINGENTRY(cp1258),    1258,  0, UNKNOWN, "unknown",
        ENCODINGENTRY(cp1386),    1386,  0, DOUBLE, "Chinese (IBM)",  // WarpIN V1.0.16 (2008-02-19) [pr]
        ENCODINGENTRY(iso8859_1),  819,  1, SINGLE, "ISO/IEC 8859-1:1998 (Latin-1)",
        ENCODINGENTRY(iso8859_2),  912,  2, SINGLE, "ISO 8859-2:1999 (Latin-2)",
        ENCODINGENTRY(iso8859_3),  913,  3, SINGLE, "ISO/IEC 8859-3:1999 (Latin-3)",
        ENCODINGENTRY(iso8859_4),  914,  4, SINGLE, "ISO/IEC 8859-4:1998 (Latin-4)",
        ENCODINGENTRY(iso8859_5),  915,  5, SINGLE, "ISO 8859-5:1999 (Cyrillic)",
        ENCODINGENTRY(iso8859_6), 1089,  6, SINGLE, "ISO 8859-6:1999 (Arabic)",
        ENCODINGENTRY(iso8859_7),  813,  7, SINGLE, "ISO 8859-7:1987 (Greek)",   // default in Greece
        ENCODINGENTRY(iso8859_8),  916,  8, SINGLE, "ISO/IEC 8859-8:1999 (Hebrew)",
        ENCODINGENTRY(iso8859_9),  920,  9, SINGLE, "ISO/IEC 8859-9:1999 (Latin-5)",
        ENCODINGENTRY(iso8859_10),   0, 10, SINGLE, "ISO/IEC 8859-10:1998",
        ENCODINGENTRY(iso8859_13),   0, 13, SINGLE, "ISO/IEC 8859-13:1998",
        ENCODINGENTRY(iso8859_14),   0, 14, SINGLE, "ISO/IEC 8859-14:1998",
        ENCODINGENTRY(iso8859_15), 923, 15, SINGLE, "ISO/IEC 8859-15:1999",

        UNSUPPORTED, NULL, 0,     1200,  0, MULTI_UNICODE, "Unicode UCS-2",
        UNSUPPORTED, NULL, 0,     1208,  0, MULTI_UNICODE, "Unicode UTF-8"
    };

/*
 *@@ ENCCASEFOLD:
 *
 *@@added V0.9.20 (2002-07-03) [umoeller]
 */

typedef struct _ENCCASEFOLD
{
    unsigned long   cEntries;
    unsigned long   aulFolds[1];
} ENCCASEFOLD, *PENCCASEFOLD;

STATIC PENCCASEFOLD G_pFold = NULL;

/*
 *@@ encGetTable:
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

int encGetTable(ENCID id,
                PXWPENCODINGMAP *ppMap,
                unsigned long *pcEntries)
{
    unsigned long ul;
    for (ul = 0;
         ul < ARRAYITEMCOUNT(G_aEncodings);
         ul++)
    {
        if (G_aEncodings[ul].id == id)
        {
            *ppMap = G_aEncodings[ul].pMap;
            *pcEntries = G_aEncodings[ul].cEntries;
            return 1;
        }
    }

    return 0;
}

/*
 *@@ encFindIdForCodepage:
 *      returns the ENCID for the given OS/2
 *      codepage, or UNSUPPORTED if there's none.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 *@@changed V1.0.2 (2003-09-19) [umoeller]: fixed Korean codepage from 951 to 949
 */

ENCID encFindIdForCodepage(unsigned short usCodepage,       // in: codepage to find
                           const char **ppcszDescription,   // out: codepage description; ptr can be NULL
                           ENCBYTECOUNT *pByteCount)        // out: SINGLE or DOUBLE; ptr can be NULL
{
    unsigned long ul;
    for (ul = 0;
         ul < ARRAYITEMCOUNT(G_aEncodings);
         ul++)
    {
        if (G_aEncodings[ul].usCodepageOS2 == usCodepage)

        {
            if (ppcszDescription)
                *ppcszDescription = G_aEncodings[ul].pcszDescription;
            if (pByteCount)
                *pByteCount = G_aEncodings[ul].bc;
            return G_aEncodings[ul].id;
        }
    }

    return UNSUPPORTED;
}

/*
 *@@ encCreateCodec:
 *      creates a codec that can be used for conversion between
 *      Unicode and codepaged characters (and vice versa).
 *
 *      A codec essentially consists of two tables which can
 *      be used for quick index-based lookups in both directions.
 *      This function goes thru the tables provided in
 *      include\encodings\*.h and builds the codec tables
 *      from them.
 *
 *      This function takes an encoding ID as input. Each
 *      codepage table in include\encodings\*.h has one
 *      of those IDs assigned. Use encFindIdForCodepage
 *      to find the ID for a given OS/2 codepage.
 *
 *      Use codecs carefully and only when they are really
 *      needed for a specific conversion. Building a codec
 *      is expensive, so you should create a codec once
 *      and reuse it for future conversions. In addition,
 *      create codecs only for the codepages that are
 *      actually used. Each codec will take up to
 *      n * sizeof(USHORT) bytes, where n is the highest
 *      Unicode character used in the codepage.
 *
 *      Codec remarks:
 *
 *      --  All codepages share the first 128 characters
 *          (0-0x7F) with ASCII.
 *
 *      --  Since the first 128 characters (0-0x7F) in
 *          Unicode are equivalent to ASCII also, codecs
 *          are not needed if you process ASCII strings
 *          only.
 *
 *      --  Since the next 128 characters (0x80-0xFF) in
 *          Unicode are equivalent to ISO/IEC 8859-1
 *          (Latin-1), codecs aren't needed for those
 *          strings either.
 *
 *          Note that codepoints 0x80-0x9F are undefined
 *          in Latin-1 but used as control sequences in
 *          Unicode.
 *
 *      --  As far as I know, codepage 1252, which is
 *          used per default under Windows, is equivalent
 *          to Latin 1 except that it also defines
 *          codepoints 0x80-0x9F to certain DTP characters.
 *
 *      --  From my testing, codepage 1004 (which is
 *          described as "Windows-compatible" in most OS/2
 *          docs) is the same as codepage 1252, except for
 *          character 0xAF.
 *
 *      Unfortunately, OS/2 uses codepage 850 on most
 *      systems (and Windows uses OS/2 codepage 1252),
 *      so for conversion between those, codecs are needed.
 *
 *      This works and is presently used in WarpIN.
 */

PCONVERSION encCreateCodec(ENCID id)
{
    PXWPENCODINGMAP pEncodingMap;
    unsigned long cArrayEntries;

    if (encGetTable(id,
                    &pEncodingMap,
                    &cArrayEntries))
    {
        unsigned short usHighestCP = 0,
                       usHighestUni = 0;
        unsigned long ul;

        // step 1:
        // run through the table and calculate the highest
        // character entry used
        for (ul = 0;
             ul < cArrayEntries;
             ul++)
        {
            if (pEncodingMap[ul].usCP > usHighestCP)
                usHighestCP = pEncodingMap[ul].usCP;
            if (pEncodingMap[ul].usUni > usHighestUni)
                usHighestUni = pEncodingMap[ul].usUni;
        }

        // step 2: allocate encoding table
        if (usHighestCP && usHighestUni)
        {
            PCONVERSION pTableNew;
            if (pTableNew = NEW(CONVERSION))
            {
                unsigned long cbEntriesUniFromCP
                    = (usHighestCP + 1) * sizeof(unsigned short);
                unsigned long cbEntriesCPFromUni
                    = (usHighestUni + 1) * sizeof(unsigned short);

                ZERO(pTableNew);

                pTableNew->usHighestCP = usHighestCP;
                pTableNew->usHighestUni = usHighestUni;

                if (    (pTableNew->ausEntriesUniFromCP
                            = (unsigned short*)malloc(cbEntriesUniFromCP))
                     && (pTableNew->ausEntriesCPFromUni
                            = (unsigned short*)malloc(cbEntriesCPFromUni))
                   )
                {
                    // step 3: fill encoding tables

                    memset(pTableNew->ausEntriesUniFromCP,
                           0xFF,
                           cbEntriesUniFromCP);
                    memset(pTableNew->ausEntriesCPFromUni,
                           0xFF,
                           cbEntriesCPFromUni);

                    for (ul = 0;
                         ul < cArrayEntries;
                         ul++)
                    {
                        PXWPENCODINGMAP pEntry = &pEncodingMap[ul];

                        pTableNew->ausEntriesUniFromCP[pEntry->usCP] = pEntry->usUni;

                        pTableNew->ausEntriesCPFromUni[pEntry->usUni] = pEntry->usCP;
                    }

                    return pTableNew;
                }

                free(pTableNew);
            }
        }
    }

    return NULL;
}

/*
 *@@ encFreeCodec:
 *      frees a codec created with encCreateCodec
 *      and sets the given pointer to NULL.
 *
 *      This works and is presently used in WarpIN.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

void encFreeCodec(PCONVERSION *ppTable)         // in: ptr to codec ptr returned by encCreateCodec
{
    PCONVERSION pTable;
    if (pTable = *ppTable)
    {
        if (pTable->ausEntriesUniFromCP)
            free(pTable->ausEntriesUniFromCP);
        if (pTable->ausEntriesCPFromUni)
            free(pTable->ausEntriesCPFromUni);
        free(pTable);
        *ppTable = NULL;
    }
}

/*
 *@@ encChar2Uni:
 *      converts a codepage-specific character
 *      to Unicode, using the given conversion
 *      table from encCreateCodec().
 *
 *      Returns 0xFFFF on errors, which is unlikely
 *      with Unicode though.
 *
 *      This works and is presently used in WarpIN.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

unsigned long encChar2Uni(PCONVERSION pTable,
                          unsigned short c)
{
    if (    (pTable)
         && (c <= pTable->usHighestCP)
       )
        return pTable->ausEntriesUniFromCP[c];

    return 0xFFFF;
}

/*
 *@@ encUni2Char:
 *      converts a Unicode character to the
 *      codepage specified by the given
 *      conversion table from encCreateCodec().
 *
 *      Returns 0xFFFF if the Unicode character
 *      has no codepage equivalent.
 *
 *      This works and is presently used in WarpIN.
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

unsigned short encUni2Char(PCONVERSION pTable,
                           unsigned long ulUni)
{
    if (    (pTable)
         && (ulUni <= pTable->usHighestUni)
       )
        return pTable->ausEntriesCPFromUni[ulUni];

    return 0xFFFF;
}

/*
 *@@ encDecodeUTF8:
 *      decodes one UTF-8 character and returns
 *      the Unicode value or -1 if the character
 *      is invalid.
 *
 *      On input, *ppch is assumed to point to
 *      the first byte of the UTF-8 char to be
 *      read.
 *
 *      This function will advance *ppch by at
 *      least one byte (or more if the UTF-8
 *      char initially pointed to introduces
 *      a multi-byte sequence).
 *
 *      This returns -1 if *ppch points to an
 *      invalid encoding (in which case the
 *      pointer is advanced anyway).
 *
 *      This returns 0 if *ppch points to a
 *      null character.
 *
 *      This works and is presently used in WarpIN.
 *
 *@@added V0.9.14 (2001-08-09) [umoeller]
 */

unsigned long encDecodeUTF8(const char **ppch)
{
    unsigned long   ulChar;
    unsigned long   ulCount;
    int             fIllegal;

    if (!(ulChar = **ppch))
        // null is null
        return 0;

    // if (ulChar < 0x80): simple, one byte only... use that

    if (ulChar < 0x80)
    {
        (*ppch)++;
        return ulChar;
    }

    ulCount = 1;
    fIllegal = 0;

    // note: 0xc0 and 0xc1 are reserved and
    // cannot appear as the first UTF-8 byte

    if (    (ulChar >= 0xc2)
         && (ulChar < 0xe0)
       )
    {
        // that's two bytes
        ulCount = 2;
        ulChar &= 0x1f;
    }
    else if ((ulChar & 0xf0) == 0xe0)
    {
        // three bytes
        ulCount = 3;
        ulChar &= 0x0f;
    }
    else if ((ulChar & 0xf8) == 0xf0)
    {
        // four bytes
        ulCount = 4;
        ulChar &= 0x07;
    }
    else if ((ulChar & 0xfc) == 0xf8)
    {
        // five bytes
        ulCount = 5;
        ulChar &= 0x03;
    }
    else if ((ulChar & 0xfe) == 0xfc)
    {
        // six bytes
        ulCount = 6;
        ulChar &= 0x01;
    }
    else
        ++fIllegal;

    if (!fIllegal)
    {
        // go for the second and more bytes then
        int ul2;

        for (ul2 = 1;
             ul2 < ulCount;
             ++ul2)
        {
            unsigned long ulChar2 = *((*ppch) + ul2);

            if (!(ulChar2 & 0xc0)) //  != 0x80)
            {
                ++fIllegal;
                break;
            }

            ulChar <<= 6;
            ulChar |= ulChar2 & 0x3f;
        }
    }

    if (fIllegal)
    {
        // skip all the following characters
        // until we find something with bit 7 off
        do
        {
            ulChar = *(++(*ppch));
            if (!ulChar)
                break;
        } while (ulChar & 0x80);
    }
    else
        *ppch += ulCount;

    return ulChar;
}

/*
 *@@ encInitCase:
 *      creates a casefold for later use with
 *      encToUpper.
 *
 *      This only uses one-byte sequences from
 *      the Unicode case folding table (see
 *      include\encodings\unicase.h), so this
 *      cannot be used for expanding characters
 *      at this point.
 *
 *      Returns 1 (TRUE) on success.
 *
 *      This works and is presently used in WarpIN.
 *
 *@@added V0.9.20 (2002-07-03) [umoeller]
 */

int encInitCase(void)
{
    unsigned long   ul,
                    cEntries = 0,
                    cb;

    for (ul = 0;
         ul < ARRAYITEMCOUNT(G_aCaseFolds);
         ++ul)
    {
        // ignore CASEFL_T (duplicate entries for i chars)
        // and CASEFL_F (expansions)
        if (    (G_aCaseFolds[ul].fl & (CASEFL_C | CASEFL_S))
             && (G_aCaseFolds[ul].ulLow > cEntries)
           )
            cEntries = G_aCaseFolds[ul].ulLow;
    }

    cb = sizeof(ENCCASEFOLD) + cEntries * sizeof(unsigned long);
    if (G_pFold = (PENCCASEFOLD)malloc(cb))
    {
        memset(G_pFold, 0, cb);
        G_pFold->cEntries = cEntries;

        for (ul = 0;
             ul < ARRAYITEMCOUNT(G_aCaseFolds);
             ++ul)
        {
            if (G_aCaseFolds[ul].fl & (CASEFL_C | CASEFL_S))
                G_pFold->aulFolds[G_aCaseFolds[ul].ulLow] = G_aCaseFolds[ul].c1;
        }

        return 1;
    }

    return 0;
}

/*
 *@@ encToUpper:
 *      converts the given unicode character to
 *      upper case, if possible, or returns
 *      ulUni back if Unicode doesn't define
 *      an upper-case character for it.
 *
 *      Special cases:
 *
 *      --  Returns 0 for 0.
 *
 *      Preconditions:
 *
 *      --  You must call encInitCase before
 *          the first call.
 *
 *      This works and is presently used in WarpIN.
 *
 *@@added V0.9.20 (2002-07-03) [umoeller]
 */

unsigned long encToUpper(unsigned long ulUni)
{
    unsigned long ulFold;

    if (    (ulUni < G_pFold->cEntries)
         && (ulFold = G_pFold->aulFolds[ulUni])
       )
        return ulFold;

    return ulUni;
}

/*
 *@@ encicmp:
 *      like stricmp, but for UTF-8 strings.
 *      This uses encToUpper for the comparisons.
 *
 *      Like stricmp, this returns:
 *
 *      --  -1 if pcsz1 is less than pcsz2
 *      --  0 if pcsz1 is equal to pcsz2
 *      --  +1 if pcsz1 is greater than pcsz2
 *
 *      However, this does not crash on passing
 *      in NULL strings.
 *
 *      Preconditions:
 *
 *      --  You must call encInitCase before
 *          the first call.
 *
 *      This works and is presently used in WarpIN.
 *
 *@@added V0.9.20 (2002-07-03) [umoeller]
 */

int encicmp(const char *pcsz1,
            const char *pcsz2)
{
    const char  *p1 = pcsz1,
                *p2 = pcsz2;

    unsigned long ul1, ul2;

    do
    {
        // encDecodeUTF8 returns null for null, so this is safe
        ul1 = encToUpper(encDecodeUTF8(&p1));
        ul2 = encToUpper(encDecodeUTF8(&p2));

        if (ul1 < ul2)
            return -1;
        if (ul1 > ul2)
            return +1;

        // both are equal: check for null bytes then
        if (!ul1)
            if (!ul2)
                return 0;
            else
                // ul1 is null, but ul2 isn't:
                return -1;
        else
            if (!ul2)
                // ul1 is not null, but ul2 is:
                return +1;

        // both are non-null: continue

    } while (1);

    return 0;
}

