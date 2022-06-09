
/*
 *@@sourcefile vcard.c:
 *
 *      Function prefixes:
 *      --  vcf*   replacement profile (INI) functions
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\vcard.h"
 *@@added V0.9.16 (2002-02-02) [umoeller]
 */

/*
 *      Copyright (C) 2002 Ulrich M”ller.
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

#define INCL_DOSPROFILE
#define INCL_DOSNLS
#define INCL_DOSERRORS
#define INCL_WINCOUNTRY
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/dosh.h"
#include "helpers/linklist.h"
#include "helpers/nls.h"
#include "helpers/standards.h"
#include "helpers/stringh.h"
#include "helpers/xstring.h"

#define INCLUDE_VCARD_PRIVATE
#define INCLUDE_VCARD_ALL
#include "helpers/vcard.h"

#pragma hdrstop

/*
 *@@category: Helpers\vCard parser
 *      See vcard.c.
 */

/* ******************************************************************
 *
 *   Private declarations
 *
 ********************************************************************/

STATIC VOID FreeList(PLINKLIST *ppll);

/* ******************************************************************
 *
 *   vCard parser
 *
 ********************************************************************/

/*
 *@@ Translate:
 *
 */

STATIC VOID Translate(PXSTRING pstr,
                      ULONG cpCurrent)
{
    ULONG ul;

    // there may be MIME-encoded strings in here
    xstrDecode2(pstr, '=');

    for (ul = 0;
         ul < pstr->ulLength;
         ul++)
    {
        PSZ pc;

        if (*(pc = &pstr->psz[ul]) > 127)
            *pc = WinCpTranslateChar(0,
                                     1004,
                                     *pc,
                                     cpCurrent);
    }
}

/*
 *@@ DecodeStringList:
 *
 *@@added V0.9.16 (2002-02-02) [umoeller]
 */

STATIC APIRET DecodeStringList(PCSZ pStart,
                               PCSZ pEnd,
                               PXSTRING *ppaStrings,
                               PULONG pcStrings,
                               PXSTRING *ppstrLast,        // out: last string stored
                               ULONG cpCurrent)            // in: current codepage
{
    if (!pStart || !pEnd)
        return ERROR_BAD_FORMAT;

    while (pStart)
    {
        PCSZ pNext;

        // we can have several parameters:
        // PHOTO;VALUE=URL;TYPE=GIF:http://www.abc.com/dir_photos/my_photo.gif
        // so find end of this param
        PCSZ pEndOfParam;
        if (pNext = strchr(pStart + 1, ';'))
            pEndOfParam = pNext;
        else
            pEndOfParam = pEnd;

        (*pcStrings)++;

        // append a new XSTRING to the array
        if (*ppaStrings = (PXSTRING)realloc(*ppaStrings,      // NULL on first call
                                            *pcStrings * sizeof(XSTRING)))
        {
            PXSTRING paStrings = *ppaStrings;
            PXSTRING pstrParamThis = &paStrings[(*pcStrings) - 1];
            ULONG cb;

            xstrInit(pstrParamThis, 0);

            if (cb = pEndOfParam - pStart - 1)
            {
                if (    (cb > 6)
                     && (!memcmp(pStart + 1, "TYPE=", 5))
                   )
                    xstrcpy(pstrParamThis,
                            pStart + 6,
                            // up to colon
                            cb - 5);
                else
                    xstrcpy(pstrParamThis,
                            pStart + 1,
                            // up to colon
                            cb);

                Translate(pstrParamThis,
                          cpCurrent);
            }

            if (ppstrLast)
                // caller wants last string used:
                *ppstrLast = pstrParamThis;
        }
        else
            return ERROR_NOT_ENOUGH_MEMORY;

        pStart = pNext;
    }

    return NO_ERROR;
}

/*
 *@@ Tokenize:
 *
 *      A property is the definition of an individual attribute describing
 *      the vCard. A property takes the following form:
 +
 +          PropertyName [';' PropertyParameters] ':'  PropertyValue
 +
 *      as shown in the following example:
 +
 +          TEL;HOME:+1-919-555-1234
 +
 *      A property takes the form of one or more lines of text. The
 *      specification of property names and property parameters is
 *      case insensitive.
 *
 *      The property name can be one of a set of pre-defined strings.
 *      The property name, along with an optional grouping label,
 *      must appear as the first characters on a line.
 *      In the previous example, "TEL" is the name of the Telephone
 *      Number property.
 *
 *      Property values are specified as strings. In the previous
 *      example, "+1-919-555-1234" is the formatted value for the
 *      Telephone Number property.
 *
 *      A property value can be further qualified with a property
 *      parameter expression. Property parameter expressions are
 *      delimited from the property name with a semicolon.
 *      A semicolon in a property parameter value must be escaped
 *      with a backslash character. The property parameter expressions
 *      are specified as either a name=value or a value string. The
 *      value string can be specified alone in those cases where the
 *      value is unambiguous. For example a complete property parameter
 *      specification might be:
 +
 +      NOTE;ENCODING=QUOTED-PRINTABLE:Don't remember to order Girl=
 +          Scout cookies from Stacey today!
 +
 *      A valid short version of the same property parameter
 *      specification might be:
 +
 +      NOTE;QUOTED-PRINTABLE:Don t remember to order Girl=
 +          Scout cookies from Stacey today!
 *
 *      Continuation across several lines is possible by starting
 *      continuation lines with spaces. During parsing, any sequence
 *      of CRLF followed immediately by spaces is considered a
 *      continuation and will be removed in the returned value.
 *
 *      Standard properties:
 *
 *      --  FN: formatted name (what is displayed as the name).
 *
 *      --  N: structured name (family name;
 *                              given name;
 *                              addtl. names;
 *                              name prefix;
 *                              name suffix)
 *
 *          e.g.    N:Public;John;Quinlan;Mr.;Esq.
 *                  N:Veni, Vidi, Vici;The Restaurant.
 *
 *      --  PHOTO: photo of vCard's owner
 *
 +              PHOTO;VALUE=URL:file:///jqpublic.gif
 +
 +              PHOTO;ENCODING=BASE64;TYPE=GIF:
 +                  R0lGODdhfgA4AOYAAAAAAK+vr62trVIxa6WlpZ+fnzEpCEpzlAha/0Kc74+PjyGM
 +                  SuecKRhrtX9/fzExORBSjCEYCGtra2NjYyF7nDGE50JrhAg51qWtOTl7vee1MWu1
 +                  50o5e3PO/3sxcwAx/4R7GBgQOcDAwFoAQt61hJyMGHuUSpRKIf8A/wAY54yMjHtz
 *
 *      --  BDAY: birthday
 +
 +              BDAY:19950415
 +
 +              BDAY:1995-04-15
 *
 *      --  ADR: delivery address (compound: Post Office Address;
 *                                           Extended Address;
 *                                           Street;
 *                                           Locality;
 *                                           Region;
 *                                           Postal Code;
 *                                           Country)
 +
 +              ADR;DOM;HOME:P.O. Box 101;Suite 101;123 Main Street;Any Town;CA;91921-1234;
 *
 *      --  LABEL: delivery label (formatted)
 *
 *      --  TEL: telephone
 *
 *      --  EMAIL
 *
 *      --  MAILER: email software used by individual
 *
 *      --  TZ: timezone
 *
 *      --  GEO: geographic position
 *
 *      --  TITLE: job title etc.
 *
 *      --  ROLE: business role
 *
 *      --  LOGO: company logo or something
 *
 *      --  ORG: organization name
 *
 *      --  NOTE: a comment
 *
 *      --  REV: when vCard was last modified
 *
 *      --  SOUND: sound data
 *
 *      --  URL: where to find up-to-date information
 *
 *      --  UID: unique vCard identifier
 *
 *      --  VERSION: vCard version info (2.1)
 *
 *      --  KEY: public key
 *
 *      --  X-*: extension
 */

STATIC APIRET Tokenize(ULONG ulLevel,
                       PSZ *ppszInput,
                       PLINKLIST pllParent,
                       ULONG cpCurrent)
{
    PSZ         pLineThis = *ppszInput;
    ULONG       cbPropertyName;
    APIRET      arc = NO_ERROR;
    ULONG       ul = 0;
    PXSTRING    pstrPrevValue = NULL;

    PVCFPROPERTY pPrevProp = NULL;

    if (    (!ppszInput)
         || (!(*ppszInput))
         || (!pllParent)
       )
        return ERROR_INVALID_PARAMETER;

    while (!arc)
    {
        PSZ     pNextEOL = strhFindEOL(pLineThis, NULL);    // never NULL

        if (*pLineThis == ' ')
        {
            // continuation from previous line:
            // append to previous value string, if we had one
            if (!pstrPrevValue)
            {
                arc = ERROR_BAD_FORMAT;
                break;
            }
            else
            {
                // skip the spaces
                PSZ p = pLineThis + 1;
                while (*p == ' ')
                    p++;
                if (*p != '\n')
                    // line not empty:
                    xstrcat(pstrPrevValue,
                            p - 1,      // add one space always!
                            pNextEOL - p + 1);
            }
        }
        else
        {
            PSZ pNextColon;
            if (!(pNextColon = strchr(pLineThis, ':')))
            {
                arc = ERROR_BAD_FORMAT;
                break;
            }

            if (    (pNextColon < pNextEOL)
                 && (*pLineThis != '\n')       // not empty line
               )
            {
                // ADR;DOM;HOME:P.O. Box 101;Suite 101;123 Main Street;Any Town;CA;91921-1234;
                // colon before EOL: then we have at least a value
                PSZ     pNextSemicolon;
                ULONG   cbPropertyNameThis,
                        cbLineThis = pNextEOL - pLineThis;
                PVCFPROPERTY pProp;

                *pNextColon = '\0';
                nlsUpper(pLineThis);

                if (pNextSemicolon = strchr(pLineThis, ';'))
                    // we have a parameter:
                    cbPropertyNameThis = pNextSemicolon - pLineThis;
                else
                    cbPropertyNameThis = pNextColon - pLineThis;

                // special properties:
                if (!strcmp(pLineThis, "BEGIN"))
                {
                    // begin of vCard object:
                    // this is either the root object or a nested one;
                    // in any case, recurse!
                    if (    (*pNextEOL)
                         && (pPrevProp)
                       )
                    {
                        PSZ pszSubInput = pNextEOL + 1;
                        pPrevProp->pllSubList = lstCreate(FALSE);
                        if (!(arc = Tokenize(ulLevel + 1,
                                             &pszSubInput,
                                             pPrevProp->pllSubList,
                                             cpCurrent)))
                        {
                            // continue after this chunk
                            // (pszSubinput points to after end:vcard now)
                            pNextEOL = pszSubInput;
                        }
                    }
                    else
                        arc = ERROR_BAD_FORMAT;

                    if (arc)
                        break;

                }
                else if (!strcmp(pLineThis, "END"))
                {
                    // end of this vCard:
                    // store address of end:vcard for parent call
                    *ppszInput = pNextEOL;
                    break;
                }
                // any other property:
                else if (pProp = NEW(VCFPROPERTY))
                {
                    CHAR cSaved2;

                    ZERO(pProp);

                    // 1) store property name
                    xstrInit(&pProp->strProperty, 0);
                    xstrcpy(&pProp->strProperty,
                            pLineThis,
                            cbPropertyNameThis);

                    // 2) store parameters
                    DecodeStringList(pNextSemicolon,
                                     pNextColon,
                                     &pProp->pastrParameters,
                                     &pProp->cParameters,
                                     NULL,
                                     cpCurrent);

                    // 3) store values
                    cSaved2 = *pNextEOL;
                    *pNextEOL = '\0';
                    DecodeStringList(pNextColon,
                                     pNextEOL,
                                     &pProp->pastrValues,
                                     &pProp->cValues,
                                     &pstrPrevValue,        // for line continuations
                                     cpCurrent);
                    *pNextEOL = cSaved2;

                    // add the property to the parent's list
                    lstAppendItem(pllParent,
                                  pProp);

                    // store the prop for next loop in case
                    // we need to recurse for nested vCards
                    pPrevProp = pProp;
                }
                else
                    arc = ERROR_NOT_ENOUGH_MEMORY;

                *pNextColon = ':';
            }
        }

        if (!*pNextEOL)
            // no more lines:
            break;

        pLineThis = pNextEOL + 1;
    }

    return arc;
}

/*
 *@@ FindValues:
 *
 */

STATIC PVCFPROPERTY FindValues(PLINKLIST pll,
                               PCSZ pcszProperty,
                               PCSZ pcszParameter)
{
    PLISTNODE   pNode;

    for (pNode = lstQueryFirstNode(pll);
         pNode;
         pNode = pNode->pNext)
    {
        PVCFPROPERTY pProp = (PVCFPROPERTY)pNode->pItemData;

        if (!strcmp(pProp->strProperty.psz, pcszProperty))
        {
            if (!pcszParameter)     // or parameter matches @@todo
            {
                return pProp;
            }
        }
    }

    return NULL;
}

/*
 *@@ CopyStrings:
 *
 */

STATIC VOID CopyStrings(PVCFPROPERTY pProp,
                        PCSZ *papcszValues,
                        ULONG cValues)
{
    ULONG ul;

    memset(papcszValues, 0, sizeof(PSZ) * cValues);

    if (pProp)
    {
        if (cValues > pProp->cValues)
            cValues = pProp->cValues;

        for (ul = 0;
             ul < cValues;
             ul++)
        {
            papcszValues[ul] = pProp->pastrValues[ul].psz;
        }
    }
}

/*
 *@@ GetFlagStrings:
 *
 *      My application to the "obfuscated C contest".
 */

STATIC ULONG GetFlagStrings(PXSTRING pastrParameters,
                            ULONG cParameters,
                            const PCSZ **apcsz,
                            const ULONG *afl,
                            ULONG cStrings,
                            ULONG flDefault)
{
    ULONG ul, ul2;
    ULONG fl = 0;
    if (!cParameters)
        fl = flDefault;
    else for (ul = 0;
              ul < cParameters;
              ul++)
    {
        PCSZ pcszThis = pastrParameters[ul].psz;
        for (ul2 = 0;
             ul2 < cStrings;
             ul2++)
        {
            if (!strcmp(pcszThis, *apcsz[ul2]))
                fl |= afl[ul2];
        }
    }

    return fl;
}

STATIC const PCSZ *apcszAddress[] =
    {
        &VCF_TYPE_ADR_DOM,
        &VCF_TYPE_ADR_INTL,
        &VCF_TYPE_ADR_POSTAL,
        &VCF_TYPE_ADR_PARCEL,
        &VCF_TYPE_ADR_HOME,
        &VCF_TYPE_ADR_WORK
    };

STATIC const ULONG aflAddress[] =
    {
        VCF_ADDRFL_DOM,
        VCF_ADDRFL_INTL,
        VCF_ADDRFL_POSTAL,
        VCF_ADDRFL_PARCEL,
        VCF_ADDRFL_HOME,
        VCF_ADDRFL_WORK
    };

/*
 *@@ AppendAddress:
 *
 */

STATIC VOID AppendAddress(PVCARD pvc,
                          PVCFPROPERTY pProp)
{
    if (pvc->paDeliveryAddresses = (PVCADDRESS)realloc(
                                pvc->paDeliveryAddresses,
                                (pvc->cDeliveryAddresses + 1) * sizeof(VCADDRESS)))
    {
        PVCADDRESS pThis = &pvc->paDeliveryAddresses[(pvc->cDeliveryAddresses)++];

        CopyStrings(pProp,
                    pThis->apcszAddress,
                    7);
        pThis->fl = GetFlagStrings(pProp->pastrParameters,
                                   pProp->cParameters,
                                   apcszAddress,
                                   aflAddress,
                                   ARRAYITEMCOUNT(apcszAddress),
                                   VCF_ADDRFL_INTL | VCF_ADDRFL_WORK
                                        | VCF_ADDRFL_POSTAL | VCF_ADDRFL_PARCEL);
    }
}

/*
 *@@ AppendLabel:
 *
 */

STATIC VOID AppendLabel(PVCARD pvc,
                        PVCFPROPERTY pProp)
{
    if (pvc->paLabels = (PVCLABEL)realloc(
                                pvc->paLabels,
                                (pvc->cLabels + 1) * sizeof(VCLABEL)))
    {
        PVCLABEL pThis = &pvc->paLabels[(pvc->cLabels)++];

        CopyStrings(pProp,
                    &pThis->pcszLabel,
                    1);
        pThis->fl = GetFlagStrings(pProp->pastrParameters,
                                   pProp->cParameters,
                                   apcszAddress,
                                   aflAddress,
                                   ARRAYITEMCOUNT(apcszAddress),
                                   VCF_ADDRFL_INTL | VCF_ADDRFL_WORK
                                        | VCF_ADDRFL_POSTAL | VCF_ADDRFL_PARCEL);
    }
}

STATIC const PCSZ *apcszPhone[] =
    {
        &VCF_TYPE_TEL_PREF,
        &VCF_TYPE_TEL_WORK,
        &VCF_TYPE_TEL_HOME,
        &VCF_TYPE_TEL_VOICE,
        &VCF_TYPE_TEL_FAX,
        &VCF_TYPE_TEL_MSG,
        &VCF_TYPE_TEL_CELL,
        &VCF_TYPE_TEL_PAGER,
        &VCF_TYPE_TEL_BBS,
        &VCF_TYPE_TEL_MODEM,
        &VCF_TYPE_TEL_CAR,
        &VCF_TYPE_TEL_ISDN,
        &VCF_TYPE_TEL_VIDEO
    };

STATIC const ULONG aflPhone[] =
    {
        VCF_PHONEFL_PREF,
        VCF_PHONEFL_WORK,
        VCF_PHONEFL_HOME,
        VCF_PHONEFL_VOICE,
        VCF_PHONEFL_FAX,
        VCF_PHONEFL_MSG,
        VCF_PHONEFL_CELL,
        VCF_PHONEFL_PAGER,
        VCF_PHONEFL_BBS,
        VCF_PHONEFL_MODEM,
        VCF_PHONEFL_CAR,
        VCF_PHONEFL_ISDN,
        VCF_PHONEFL_VIDEO
    };

/*
 *@@ AppendTel:
 *
 */

STATIC VOID AppendTel(PVCARD pvc,
                      PVCFPROPERTY pProp)
{
    if (pvc->paPhones = (PVCPHONE)realloc(pvc->paPhones,
                                          (pvc->cPhones + 1) * sizeof(VCPHONE)))
    {
        ULONG ul;
        PVCPHONE pThis = &pvc->paPhones[(pvc->cPhones)++];

        CopyStrings(pProp,
                    &pThis->pcszNumber,
                    1);

        pThis->fl = GetFlagStrings(pProp->pastrParameters,
                                   pProp->cParameters,
                                   apcszPhone,
                                   aflPhone,
                                   ARRAYITEMCOUNT(apcszPhone),
                                   VCF_PHONEFL_VOICE);
    }
}

/*
 *@@ vcfRead:
 *
 */

APIRET vcfRead(PCSZ pcszFilename,
               PVCARD *ppvCard)         // out: vCard handle
{
    APIRET  arc;
    PSZ     pszData = NULL;
    ULONG   cbRead;
    if (!(arc = doshLoadTextFile(pcszFilename,
                                 &pszData,
                                 &cbRead)))
    {
        XSTRING str;
        PSZ p;

        COUNTRYCODE cc = {0};
        COUNTRYINFO ci = {0};
        ULONG cb;

        DosQueryCtryInfo(sizeof(ci),
                         &cc,
                         &ci,
                         &cb);

        xstrInitSet2(&str, pszData, cbRead - 1);
        xstrConvertLineFormat(&str, CRLF2LF);

        if (    (p = strhistr(str.psz, "BEGIN:VCARD"))
             && (p = strhFindEOL(p, NULL))
           )
        {
            PLINKLIST pll = lstCreate(FALSE);
            if (!(arc = Tokenize(0,
                                 &p,
                                 pll,
                                 ci.codepage)))
            {
                PVCARD pvc;
                if (!(pvc = NEW(VCARD)))
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                else
                {
                    PVCFPROPERTY pProp;
                    PLISTNODE pNode;

                    ZERO(pvc);


                    pvc->pll = pll;

                    // now go set up the data fields
                    if (pProp = FindValues(pll,
                                           VCFPROP_FN,
                                           NULL))
                        CopyStrings(pProp,
                                    &pvc->pcszFormattedName,
                                    1);

                    if (pProp = FindValues(pll,
                                           VCFPROP_N,
                                           NULL))
                        CopyStrings(pProp,
                                    pvc->apcszName,
                                    5);

                    if (pProp = FindValues(pll,
                                           VCFPROP_EMAIL,
                                           NULL))
                        CopyStrings(pProp,
                                    &pvc->pcszEmail,
                                    1);

                    if (pProp = FindValues(pll,
                                           VCFPROP_TITLE,
                                           NULL))
                        CopyStrings(pProp,
                                    &pvc->pcszJobTitle,
                                    1);

                    for (pNode = lstQueryFirstNode(pll);
                         pNode;
                         pNode = pNode->pNext)
                    {
                        pProp = (PVCFPROPERTY)pNode->pItemData;

                        if (!strcmp(pProp->strProperty.psz, VCFPROP_ADR))
                            AppendAddress(pvc,
                                          pProp);
                        else if (!strcmp(pProp->strProperty.psz, VCFPROP_LABEL))
                            AppendLabel(pvc,
                                        pProp);
                        else if (!strcmp(pProp->strProperty.psz, VCFPROP_TEL))
                            AppendTel(pvc,
                                      pProp);
                    }

                    *ppvCard = pvc;
                }
            }

            if (arc)
                FreeList(&pll);
        }
        else
            arc = ERROR_BAD_FORMAT;

        xstrClear(&str);
    }

    return arc;
}

/*
 *@@ FreeList:
 *
 */

STATIC VOID FreeList(PLINKLIST *ppll)
{
    PLISTNODE pNode = lstQueryFirstNode(*ppll);
    while (pNode)
    {
        PVCFPROPERTY pProp = (PVCFPROPERTY)pNode->pItemData;
        ULONG ul;

        xstrClear(&pProp->strProperty);

        if (pProp->pllSubList)
            FreeList(&pProp->pllSubList);

        if (pProp->pastrParameters)
        {
            for (ul = 0;
                 ul < pProp->cParameters;
                 ul++)
                xstrClear(&pProp->pastrParameters[ul]);

            free(pProp->pastrParameters);
        }

        if (pProp->pastrValues)
        {
            for (ul = 0;
                 ul < pProp->cValues;
                 ul++)
                xstrClear(&pProp->pastrValues[ul]);

            free(pProp->pastrValues);
        }

        pNode = pNode->pNext;
    }

    lstFree(ppll);
}

/*
 *@@ vcfFree:
 *
 */

APIRET vcfFree(PVCARD *ppvCard)
{
    PVCARD pvc;
    if (    (!ppvCard)
         || (!(pvc = *ppvCard))
       )
        return ERROR_INVALID_PARAMETER;
    else
    {
        FREE(pvc->paDeliveryAddresses);

        FreeList(&pvc->pll);

        free(pvc);
        *ppvCard = NULL;
    }

    return NO_ERROR;
}


