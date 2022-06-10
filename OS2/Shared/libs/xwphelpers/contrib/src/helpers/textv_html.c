
/*
 *@@sourcefile textv_html.c:
 *      this code converts HTML code to escape sequences for the
 *      XTextView control (textview.c).
 *
 *      This code is in part ugly spaghetti, but this is intentional to
 *      make this HTML parser FAST. In general, you get about double or
 *      triple the speed compared to Netscape 4.6 on OS/2. This code
 *      doesn't understand all of HTML though, but you get most of HTML 2.
 *      There's no tables or frames at this point.
 *
 *      The entry point into this mess is txvConvertFromHTML, which
 *      is easy to use.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\textv_html.h"
 *
 *@@added V0.9.3 (2000-05-10) [umoeller]
 */

/*
 *      Copyright (C) 2000 Ulrich M”ller.
 *      This program is part of the XWorkplace package.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/linklist.h"
#include "helpers/stringh.h"
#include "helpers/textview.h"

#include "helpers/textv_html.h"

/*
 *@@category: Helpers\PM helpers\Window classes\XTextView control\HTML conversion
 *      see textv_html.c.
 */

/* ******************************************************************
 *
 *   Declarations
 *
 ********************************************************************/

/*
 *@@ LISTDESC:
 *      structure stored in COPYTARGET to
 *      hold list information (UL, OL, ... tags).
 *
 *@@added V0.9.3 (2000-05-07) [umoeller]
 */

typedef struct _LISTDESC
{
    ULONG   ulListType;         // 0: unordered (UL)
                                // 1: ordered (OL)
                                // 2: definition lists (DL)
    ULONG   ulItem;             // list enumeration; 1 on first item,
                                // 2 on next, ...
} LISTDESC, *PLISTDESC;

/*
 *@@ COPYTARGET:
 *      monster structure which holds the current
 *      status of the HTML converter while conversion
 *      is taking place. This stores input/output pointers
 *      and various flags to avoid duplicate line breaks
 *      and such.
 *
 *      One instance of this is created in txvConvertFromHTML
 *      on the stack and then passed to all the sub-function
 *      calls.
 *
 *@@added V0.9.3 (2000-05-06) [umoeller]
 */

typedef struct _COPYTARGET
{
    PSZ     pSource;        // ptr into source string;
                            // valid ONLY while we're in a tag handler
    PSZ     pNewSource;     // can be set by tag handler to skip characters;
                            // this is set to NULL before calling a tag
                            // handler; if this is still NULL, default
                            // processing occurs

    // new string:
    PSZ     pszNew;         // memory buffer
    ULONG   cbNew;          // size of buffer (reallocated)
    PSZ     pTarget;        // current char ptr into pszNew

    // saved character while tag handler is being called
    CHAR    cSaved;

    PSZ     *ppszTitle;     // out: title (ptr can be NULL)
                            // V0.9.20 (2002-08-10) [umoeller]

    // formatting flags while going through the text
    BOOL    fSkipNextSpace;
                // if TRUE, subsequent spaces are skipped
    BOOL    fNeedsLinebreak;
                // if TRUE, \n is inserted before any other character
    BOOL    fSkipNextLinebreak;
                // if TRUE, subsequent linebreaks are skipped
    BOOL    fPRE;
                // are we currently in a PRE tag?
    BOOL    fInLink;
                // are we currently in a A HREF= tag?

    // arguments (attributes) for tag handlers
    PSZ     pszAttributes;  // != NULL while a tag handler is being called
                            // and attributes exist for the tag

    // anchors count
    // USHORT  usAnchorIndex;  // start with 1      removed V0.9.20 (2002-08-10) [umoeller]

    // list maintenance
    ULONG   ulListLevel;    // if > 0, we're in a UL or OL block;
                            // raised for each block
    ULONG   ulUnorderedListLevel; // raised with each UL block to keep track
                                // of bullets
    ULONG   ulOrderedListLevel; // raised with each UL block to keep track
                                // of 1), 2), a), b)... numbering
    ULONG   ulCurrentListType;  // current list type (from highest LISTDESC)
    BOOL    fInDT;          // TRUE if we're currently in a DT tag
    LINKLIST llLists;       // stack of LISTDESC items
} COPYTARGET, *PCOPYTARGET;

typedef VOID FNPROCESSTAG(PCOPYTARGET pct);
typedef FNPROCESSTAG *PFNPROCESSTAG;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

/* ******************************************************************
 *
 *   Append-char helpers
 *
 ********************************************************************/

#define COPYTARGETALLOC     100000

/*
 *@@ AppendChar:
 *      helper for txvConvertFromHTML to
 *      append a char to the target string
 *      in COPYTARGET.
 *      This performs a few additional checks
 *      and manages memory.
 *
 *@@added V0.9.3 (2000-05-06) [umoeller]
 */

STATIC VOID AppendChar(PCOPYTARGET pct,  // in/out: formatting buffer
                       unsigned char c)
{
    // calculate ofs where to store next char
    ULONG   cbOfsNext = pct->pTarget - pct->pszNew;
    if (cbOfsNext >= pct->cbNew)      // have we reached the buffer size yet?
    {
        // more mem needed:
        pct->cbNew += COPYTARGETALLOC;
        pct->pszNew = (PSZ)realloc(pct->pszNew, pct->cbNew);
                    // if first call, pszNew is NULL, and realloc
                    // behaves just like malloc
        // adjust target, because ptr might have changed
        pct->pTarget = pct->pszNew + cbOfsNext;
    }

    // append character
    *pct->pTarget++ = c;
}

/*
 *@@ AppendString:
 *      appends the characters in *ach,
 *      which must be null-terminated.
 *      Does NOT append a null character though.
 *
 *@@added V0.9.3 (2000-05-06) [umoeller]
 */

STATIC VOID AppendString(PCOPYTARGET pct,  // in/out: formatting buffer
                         char *ach)
{
    ULONG cbAppend = strlen(ach);
    ULONG ul;
    PSZ pSource;

    // calculate ofs where to store next char
    ULONG   cbOfsNext = pct->pTarget - pct->pszNew;
    while (cbOfsNext + cbAppend >= pct->cbNew)
    {
        // more mem needed:
        pct->cbNew += COPYTARGETALLOC;
        pct->pszNew = (PSZ)realloc(pct->pszNew, pct->cbNew);
                    // if first call, pszNew is NULL, and realloc
                    // behaves just like malloc
        // adjust target, because ptr might have changed
        pct->pTarget = pct->pszNew + cbOfsNext;
    }

    // append characters
    pSource = ach;
    for (ul = 0;
         ul < cbAppend;
         ul++)
        *pct->pTarget++ = *pSource++;
}

/*
 *@@ AppendLinebreakCheck:
 *      checks if a linebreak is needed and
 *      inserts one if so.
 *
 *@@added V0.9.3 (2000-05-17) [umoeller]
 */

STATIC VOID AppendLinebreakCheck(PCOPYTARGET pct)
{
    if ((!pct->fPRE) && (pct->fNeedsLinebreak))
    {
        // yes: insert linebreak; this resets pct->fNeedsLinebreak
        if (!pct->fSkipNextLinebreak)
        {
            AppendChar(pct, '\n');

            if ((pct->ulListLevel) && (!pct->fInDT))
                // if we're in a list, add a tab also,
                // because we'll have a negative first-line margin
                AppendString(pct, TXVESC_TAB);
        }
        pct->fNeedsLinebreak = FALSE;
    }
}

/*
 *@@ AppendEscapeWithDecimal:
 *      appends the specified escape code
 *      with a three-digit decimal parameter.
 *      Calls AppendString in turn.
 *
 *@@added V0.9.3 (2000-05-07) [umoeller]
 */

STATIC VOID AppendEscapeWith3Decimals(PCOPYTARGET pct,  // in/out: formatting buffer
                                      char *ach,
                                      USHORT us)
{
    CHAR szDecimal[10];
    if (us > 999)
        us = 999;
    sprintf(szDecimal, "%03d", us);
    // append escape
    AppendString(pct, ach);
    AppendString(pct, szDecimal);
}

/*
 *@@ AppendEscapeWith4Decimals:
 *
 *@@added V0.9.3 (2000-05-07) [umoeller]
 */

STATIC VOID AppendEscapeWith4Decimals(PCOPYTARGET pct,  // in/out: formatting buffer
                                      char *ach,
                                      USHORT us)
{
    CHAR szDecimal[10];
    if (us > 9999)
        us = 9999;
    sprintf(szDecimal, "%04d", us);
    // append escape
    AppendString(pct, ach);
    AppendString(pct, szDecimal);
}

/* ******************************************************************
 *
 *   Tag converter functions
 *
 ********************************************************************/

/*
 *@@ StartList:
 *      starts a list (UL or OL).
 *      This uses a linked list in COPYTARGET
 *      to keep a pseudo-stack for nested lists.
 *
 *@@added V0.9.3 (2000-05-08) [umoeller]
 */

STATIC VOID StartList(PCOPYTARGET pct,     // in/out: formatting buffer
                      ULONG ulListType)    // list type:
                                           // 0: unordered (UL)
                                           // 1: ordered (OL)
                                           // 2: definition lists (DL)
{
    PLISTDESC pListDesc;

    // raise list level
    pct->ulListLevel++;

    if (ulListType == 0)
        // unordered:
        pct->ulUnorderedListLevel++;
    else if (ulListType == 1)
        // ordered:
        pct->ulOrderedListLevel++;

    // create LISTDESC and store it on stack
    pListDesc = (PLISTDESC)malloc(sizeof(LISTDESC));
    pListDesc->ulListType
        = pct->ulCurrentListType
        = ulListType;
    pListDesc->ulItem = 1;

    lstAppendItem(&pct->llLists,
                  pListDesc);

    AppendEscapeWith4Decimals(pct,
                              TXVESC_LEFTMARGIN,
                              pct->ulListLevel * 5);
    AppendEscapeWith3Decimals(pct,
                              TXVESC_FIRSTLINEMARGIN_LEFT,
                              (ulListType == 2)
                                ? 5     // for definition lists
                                : 3);       // negative!
    // add \n before any other character
    pct->fNeedsLinebreak = TRUE;
}

/*
 *@@ StopList:
 *      stops a list (UL or OL).
 *
 *@@added V0.9.3 (2000-05-07) [umoeller]
 */

STATIC VOID StopList(PCOPYTARGET pct)
{
    if (pct->ulListLevel)
    {
        PLISTNODE pNode;

        // lower list level
        pct->ulListLevel--;
        AppendEscapeWith4Decimals(pct,
                                  TXVESC_LEFTMARGIN,
                                  pct->ulListLevel * 5);
        AppendEscapeWith3Decimals(pct,
                                  TXVESC_FIRSTLINEMARGIN_LEFT,
                                  (pct->ulListLevel)
                                        ? 3 // we still have a list level (nested)
                                        : 0);
        pct->fNeedsLinebreak = TRUE;

        // remove the LISTDESC from the stack
        pNode = lstNodeFromIndex(&pct->llLists,
                                 pct->ulListLevel); // this has been lowered already
        if (pNode)
        {
            PLISTDESC pListDesc = (PLISTDESC)pNode->pItemData;
            if (pListDesc->ulListType == 0)
                // was unordered:
                pct->ulUnorderedListLevel--;
            else if (pListDesc->ulListType == 1)
                // was ordered:
                pct->ulOrderedListLevel--;

            lstRemoveNode(&pct->llLists, pNode);

            // update COPYTARGET with previous list level
            if (pct->ulListLevel)
            {
                // we're still in a list (nested lists):
                PLISTDESC pListDesc2 = (PLISTDESC)lstItemFromIndex(&pct->llLists,
                                                         pct->ulListLevel - 1);
                if (pListDesc2)
                    pct->ulCurrentListType = pListDesc2->ulListType;
            }
        }
    }
    // else: buggy HTML code, ignore
}

/*
 *@@ TagTITLE:
 *
 *@@added V0.9.3 (2000-05-19) [umoeller]
 */

STATIC VOID TagTITLE(PCOPYTARGET pct)
{
    // pSource currently points to <TITLE tag
    PSZ pSource = pct->pSource + strlen(pct->pSource);
        // points to temporary null byte in main buffer now
    *pSource = pct->cSaved;

    if (pSource = strchr(pct->pSource, '>'))
    {
        PSZ pNextOpen;
        if (pNextOpen = strchr(pSource, '<'))
        {
            // extract title
            if (pct->ppszTitle)
                *(pct->ppszTitle) = strhSubstr(pSource + 1, pNextOpen);
                        // adjusted V0.9.20 (2002-08-10) [umoeller]

            if (strnicmp(pNextOpen + 1, "/TITLE", 6) == 0)
            {
                // closing /TITLE tag found:
                // search on after that
                if (pct->pNewSource = strchr(pNextOpen, '>'))
                    pct->pNewSource++;
            }
        }
    }
}

/*
 *@@ TagP:
 *
 */

STATIC VOID TagP(PCOPYTARGET pct)
{
    // append newline:
    // add \n before any other character
    pct->fNeedsLinebreak = TRUE;

    /* if (pct->ulListLevel)
    {
        // if we are currently in a list, we must also
        // add a tab escape, because we have set
        // the first line margin to the left of the
        // left margin
        AppendString(pct,
                     TXVESC_TAB);
    } */
}

STATIC VOID TagBR(PCOPYTARGET pct)
{
    AppendChar(pct,
               '\r');

    if (pct->ulListLevel)
    {
        // if we are currently in a list, we must also
        // add a tab escape, because we have set
        // the first line margin to the left of the
        // left margin
        AppendString(pct,
                     TXVESC_TAB);
    }
    if (!pct->fPRE)
        pct->fSkipNextSpace = TRUE;
}

STATIC VOID TagPRE(PCOPYTARGET pct)
{
    // start of PRE tag:
    // add \n before any other character
    // pct->fNeedsLinebreak = TRUE;
    AppendChar(pct, '\n');
    pct->fNeedsLinebreak = FALSE;
    /* AppendString(pct,
                 TXVESC_PRE_BEGIN); */
    AppendEscapeWith3Decimals(pct,
                              TXVESC_SET_FONT,
                              1);   // monospaced font
    AppendEscapeWith4Decimals(pct,
                              TXVESC_SPACEBEFORE,
                              0);   // no spacing before
    AppendEscapeWith4Decimals(pct,
                              TXVESC_SPACEAFTER,
                              0);   // no spacing after
    // disable word-wrapping
    AppendString(pct,
                 TXVESC_WORDWRAP "0");
    pct->fPRE = TRUE;
    pct->fSkipNextSpace = FALSE;
}

STATIC VOID TagXPRE(PCOPYTARGET pct)
{
    pct->fPRE = FALSE;
    AppendEscapeWith3Decimals(pct,
                              TXVESC_SET_FONT,
                              0);   // standard font
    AppendString(pct, TXVESC_SPACEBEFORE);
    AppendString(pct, "####");  // reset to default
    AppendString(pct, TXVESC_SPACEAFTER);
    AppendString(pct, "####");  // reset to default
    // re-enable word-wrapping
    AppendString(pct,
                 TXVESC_WORDWRAP "1"
                 "\n");         // force line break
    pct->fNeedsLinebreak = FALSE;
    // refuse to add \n even if we have another "p" coming up
    pct->fSkipNextLinebreak = TRUE;
    pct->fSkipNextSpace = TRUE;
}

STATIC VOID TagH1(PCOPYTARGET pct)
{
    pct->fNeedsLinebreak = TRUE;
    AppendEscapeWith3Decimals(pct,
                              TXVESC_POINTSIZE_REL,
                              200);   // double size
    AppendString(pct,
                 TXVESC_BOLD_BEGIN);
}

STATIC VOID TagXH1(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_BOLD_END);
    AppendEscapeWith3Decimals(pct,
                              TXVESC_POINTSIZE_REL,
                              100);   // regular size
    // add \n before any other character
    pct->fNeedsLinebreak = TRUE;
}

STATIC VOID TagH2(PCOPYTARGET pct)
{
    pct->fNeedsLinebreak = TRUE;
    AppendEscapeWith3Decimals(pct,
                              TXVESC_POINTSIZE_REL,
                              175);   // size in percent of regular point size
    AppendString(pct,
                 TXVESC_BOLD_BEGIN);
}

STATIC VOID TagXH2(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_BOLD_END);
    AppendEscapeWith3Decimals(pct,
                              TXVESC_POINTSIZE_REL,
                              100);   // regular size
    // add \n before any other character
    pct->fNeedsLinebreak = TRUE;
}

STATIC VOID TagH3(PCOPYTARGET pct)
{
    pct->fNeedsLinebreak = TRUE;
    AppendEscapeWith3Decimals(pct,
                              TXVESC_POINTSIZE_REL,
                              150);   // size in percent of regular point size
    AppendString(pct,
                 TXVESC_BOLD_BEGIN);
}

STATIC VOID TagXH3(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_BOLD_END);
    AppendEscapeWith3Decimals(pct,
                              TXVESC_POINTSIZE_REL,
                              100);   // size in percent of regular point size
    // add \n before any other character
    pct->fNeedsLinebreak = TRUE;
}

STATIC VOID TagH4(PCOPYTARGET pct)
{
    pct->fNeedsLinebreak = TRUE;
    AppendEscapeWith3Decimals(pct,
                              TXVESC_POINTSIZE_REL,
                              125);   // size in percent of regular point size
    AppendString(pct,
                 TXVESC_BOLD_BEGIN);
}

STATIC VOID TagXH4(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_BOLD_END);
    AppendEscapeWith3Decimals(pct,
                              TXVESC_POINTSIZE_REL,
                              100);   // regular size
    // add \n before any other character
    pct->fNeedsLinebreak = TRUE;
}

STATIC VOID TagH5(PCOPYTARGET pct)
{
    pct->fNeedsLinebreak = TRUE;
    AppendEscapeWith3Decimals(pct,
                              TXVESC_POINTSIZE_REL,
                              100);   // size in percent of regular point size
    AppendString(pct,
                 TXVESC_BOLD_BEGIN);
}

STATIC VOID TagXH5(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_BOLD_END);
    AppendEscapeWith3Decimals(pct,
                              TXVESC_POINTSIZE_REL,
                              100);   // regular size
    // add \n before any other character
    pct->fNeedsLinebreak = TRUE;
}

STATIC VOID TagH6(PCOPYTARGET pct)
{
    pct->fNeedsLinebreak = TRUE;
    AppendEscapeWith3Decimals(pct,
                              TXVESC_POINTSIZE_REL,
                              80 );   // size in percent of regular point size
    AppendString(pct,
                 TXVESC_BOLD_BEGIN);
}

STATIC VOID TagXH6(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_BOLD_END);
    AppendEscapeWith3Decimals(pct,
                              TXVESC_POINTSIZE_REL,
                              100);   // regular size
    // add \n before any other character
    pct->fNeedsLinebreak = TRUE;
}

STATIC VOID TagUL(PCOPYTARGET pct)
{
    StartList(pct,
              0);       // unordered
}

STATIC VOID TagXUL(PCOPYTARGET pct)
{
    StopList(pct);
}

STATIC VOID TagOL(PCOPYTARGET pct)
{
    StartList(pct,
              1);       // ordered
}

STATIC VOID TagXOL(PCOPYTARGET pct)
{
    StopList(pct);
}

STATIC VOID TagLI(PCOPYTARGET pct)
{
    PLISTDESC pListDesc;
    CHAR     szMarker[20] = TXVESC_MARKER "\x01";

    if (pct->ulListLevel)
    {
        // we're in a list:
        pListDesc = (PLISTDESC)lstItemFromIndex(&pct->llLists,
                                                pct->ulListLevel - 1);
        if (pListDesc)
        {
            if (pListDesc->ulListType == 1)
                // is ordered list:
                sprintf(szMarker, "%lu.", (pListDesc->ulItem)++);
            else if (pListDesc->ulListType == 0)
                // is unordered list:
                // set bullet type according to unordered nesting
                szMarker[2] = pct->ulUnorderedListLevel;
        }
    }

    // add \n before any other character
    // pct->fNeedsLinebreak = TRUE;
    // if (pct->fNeedsLinebreak)
    {
        AppendChar(pct, '\n');
        pct->fNeedsLinebreak = FALSE;
    }

    AppendString(pct, szMarker);
    AppendString(pct, TXVESC_TAB);
}

STATIC VOID TagDL(PCOPYTARGET pct)
{
    StartList(pct,
              2);       // definition list
}

STATIC VOID TagXDL(PCOPYTARGET pct)
{
    StopList(pct);
    pct->fInDT = FALSE;
}

STATIC VOID TagDT(PCOPYTARGET pct)
{
    pct->fNeedsLinebreak = TRUE;
    pct->fInDT = TRUE;
}

STATIC VOID TagDD(PCOPYTARGET pct)
{
    pct->fNeedsLinebreak = TRUE;
    AppendString(pct, TXVESC_TAB);
    if (!pct->fPRE)
        pct->fSkipNextSpace = TRUE;
    pct->fInDT = FALSE;
}

STATIC VOID TagTR(PCOPYTARGET pct)
{
    pct->fNeedsLinebreak = TRUE;
}

STATIC VOID TagB(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_BOLD_BEGIN);
}

STATIC VOID TagXB(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_BOLD_END);
}

STATIC VOID TagI(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_ITALICS_BEGIN);
}

STATIC VOID TagXI(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_ITALICS_END);
}

STATIC VOID TagU(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_UNDERLINE_BEGIN);
}

STATIC VOID TagXU(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_UNDERLINE_END);
}

STATIC VOID TagSTRIKE(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_STRIKE_BEGIN);
}

STATIC VOID TagXSTRIKE(PCOPYTARGET pct)
{
    AppendString(pct,
                 TXVESC_STRIKE_END);
}

STATIC VOID TagCODE(PCOPYTARGET pct)
{
    AppendEscapeWith3Decimals(pct,
                              TXVESC_SET_FONT,
                              1);   // monospaced font
}

STATIC VOID TagXCODE(PCOPYTARGET pct)
{
    AppendEscapeWith3Decimals(pct,
                              TXVESC_SET_FONT,
                              0);   // regular font
}

STATIC VOID TagA(PCOPYTARGET pct)
{
    CHAR    szAnchor[10];
    PSZ     pHREF = NULL;

    pct->fInLink = FALSE;

    if (pct->pszAttributes)
    {
        // we have attributes:
        PSZ pszClosingTag;
        if (pszClosingTag = strchr(pct->pszAttributes, '>'))
        {
            ULONG ulOfs = 0;

            /*
             * HREF attribute:
             *
             */

            PSZ pNAME = 0;

            // replace '>' with null char to mark end of search
            *pszClosingTag = 0;

            if (pHREF = strhGetTextAttr(pct->pszAttributes, "HREF", &ulOfs))
                // OK, we got a link target:
                pct->fInLink = TRUE;
                            // do not free

            /*
             * NAME attribute:
             *
             */

            if (pNAME = strhGetTextAttr(pct->pszAttributes, "NAME", &ulOfs))
            {
                AppendString(pct,
                             TXVESC_ANCHORNAME);
                AppendString(pct,
                             pNAME);
                // must be terminated with 0xFF
                AppendChar(pct, 0xFF);
                free(pNAME);
            }

            // restore '>'
            *pszClosingTag = '>';
        }
    }

    if (pHREF)
    {
        AppendString(pct,
                     TXVESC_LINK_BEGIN);
        AppendString(pct,
                     pHREF);
        // must be terminated with 0xFF
        AppendChar(pct, 0xFF);

        free(pHREF);
    }
}

STATIC VOID TagXA(PCOPYTARGET pct)
{
    if (pct->fInLink)
    {
        AppendString(pct,
                     TXVESC_LINK_END);
        pct->fInLink = FALSE;
    }
}

/* ******************************************************************
 *
 *   Tag helpers
 *
 ********************************************************************/

/*
 *@@ FindTagProcessor:
 *      returns the Tag* function which handles the
 *      given tag or NULL if there's none.
 *
 *@@added V0.9.4 (2000-06-10) [umoeller]
 */

STATIC PFNPROCESSTAG FindTagProcessor(PSZ pszTag)
{
    PFNPROCESSTAG pProcessor = NULL;

    CHAR    c0,
            c1;

    BOOL fEndOfTag = FALSE;

    PSZ pCheck = pszTag,
        p2;
    if (*pCheck == '/')
    {
        // end of tag:
        fEndOfTag = TRUE;
        pCheck++;
    }

    c0 = *pCheck;
    c1 = *(pCheck + 1);

    p2 = pCheck + 2;

    switch (c0)
    {
        case 'A':
        case 'a':
            switch (c1)
            {
                case 0: // A
                    if (!fEndOfTag)
                        return TagA;
                    else
                        return TagXA;
                case 'D': // ADDRESS
                case 'd': // ADDRESS
                    if (stricmp(p2, "DRESS") == 0)
                    {
                        if (!fEndOfTag)
                            return TagI;
                        else
                            return TagXI;
                    }
            }
        break;

        case 'B':
        case 'b':
            switch (c1)
            {
                case 0:
                    if (!fEndOfTag)
                        return TagB;
                    else
                        return TagXB;

                case 'R':   // BR
                case 'r':   // BR
                    if (*p2 == 0)
                        if (!fEndOfTag)
                            return TagBR;
            }
        break;

        case 'C':
        case 'c':
            switch (c1)
            {
                case 'I': // CITE
                case 'i': // CITE
                    if (stricmp(p2, "TE") == 0)
                    {
                        if (!fEndOfTag)
                            return TagI;
                        else
                            return TagXI;
                    }
                break;

                case 'O':
                case 'o':
                    if (stricmp(p2, "DE") == 0)
                    {
                        if (!fEndOfTag)
                            return TagCODE;
                        else
                            return TagXCODE;
                    }
                break;
            }
        break;

        case 'D':
        case 'd':
            switch (c1)
            {
                case 'D': // DD
                case 'd': // DD
                    if ((*p2 == 0) && (!fEndOfTag))
                        return TagDD;
                break;

                case 'I': // DIR
                case 'i': // DIR
                    if (*p2 == 'R')
                        if (*(pCheck + 3) == 0)
                        {
                            if (!fEndOfTag)
                                return TagUL;
                            else
                                return TagXUL;
                        }
                break;

                case 'L': // DL
                case 'l': // DL
                    if (*p2 == 0)
                    {
                        if (!fEndOfTag)
                            return TagDL;
                        else
                            return TagXDL;
                    }
                break;

                case 'T': // DT
                case 't': // DT
                    if ((*p2 == 0) && (!fEndOfTag))
                        return TagDT;
                break;
            }
        break;

        case 'E':
        case 'e':
            if ( (c1 == 'M') || (c1 == 'm') )  // EM
                if (*p2 == 0)
                {
                    if (!fEndOfTag)
                        return TagI;
                    else
                        return TagXI;
                }
        break;

        case 'H':
        case 'h':
            if (c1)
                if (*p2 == 0)
                    switch (c1)
                    {
                        case '1':
                            if (!fEndOfTag)
                                return TagH1;
                            else
                                return TagXH1;
                        case '2':
                            if (!fEndOfTag)
                                return TagH2;
                            else
                                return TagXH2;
                        case '3':
                            if (!fEndOfTag)
                                return TagH3;
                            else
                                return TagXH3;
                        case '4':
                            if (!fEndOfTag)
                                return TagH4;
                            else
                                return TagXH4;
                        case '5':
                            if (!fEndOfTag)
                                return TagH5;
                            else
                                return TagXH5;
                        case '6':
                            if (!fEndOfTag)
                                return TagH6;
                            else
                                return TagXH6;
                    }
        break;

        case 'I':
        case 'i':
            if (c1 == 0)
            {
                if (!fEndOfTag)
                    return TagI;
                else
                    return TagXI;
            }
        break;

        case 'L':
        case 'l':
            if ((c1 == 'I') || (c1 == 'i'))
                if (*p2 == 0)
                    return TagLI;
        break;

        case 'M':
        case 'm':
            if (stricmp(p2, "NU") == 0)
            {
                if (!fEndOfTag)
                    return TagUL;
                else
                    return TagXUL;
            }
        break;

        case 'O':
        case 'o':
            if ((c1 == 'L') || (c1 == 'l'))
                if (*p2 == 0)
                {
                    if (!fEndOfTag)
                        return TagOL;
                    else
                        return TagXOL;
                }
        break;

        case 'P':
        case 'p':
            switch (c1)
            {
                case 0:
                    if (!fEndOfTag)
                        return TagP;
                break;

                case 'R': // PRE
                case 'r': // PRE
                    if ((*p2 == 'E') || (*p2 == 'e'))
                        if (*(pCheck + 3) == 0)
                        {
                            if (!fEndOfTag)
                                return TagPRE;
                            else
                                return TagXPRE;
                        }
                break;
            }
        break;

        case 'S':
        case 's':
            switch (c1)
            {
                case 'T': // STRONG
                case 't': // STRONG
                    if (stricmp(p2, "RONG") == 0)
                    {
                        if (!fEndOfTag)
                            return TagB;
                        else
                            return TagXB;
                    }
                    else if (stricmp(p2, "RIKE") == 0)
                    {
                        if (!fEndOfTag)
                            return TagSTRIKE;
                        else
                            return TagXSTRIKE;
                    }
                break;

                case 'A':
                case 'a':
                    if (stricmp(p2, "MP") == 0)
                    {
                        if (!fEndOfTag)
                            return TagCODE;
                        else
                            return TagXCODE;
                    }
                break;
            }
        break;

        case 'T':
        case 't':
            switch (c1)
            {
                case 'R':
                case 'r':
                    if (*p2 == 0)
                        return TagTR;
                break;

                case 'I':
                case 'i':
                    if (stricmp(p2, "TLE") == 0)
                        return TagTITLE;
                break;

                case 'T': // TT
                case 't':
                    if (*p2 == 0)
                    {
                        if (!fEndOfTag)
                            return TagCODE;
                        else
                            return TagXCODE;
                    }
                break;
            }
        break;

        case 'U':
        case 'u':
            switch (c1)
            {
                case 0:
                    if (!fEndOfTag)
                        return TagU;
                    else
                        return TagXU;

                case 'L':
                case 'l':
                    if (*p2 == 0)
                    {
                        if (!fEndOfTag)
                            return TagUL;
                        else
                            return TagXUL;
                    }
                break;
            }
        break;

        case 'V':
        case 'v':
            if (stricmp(p2, "R") == 0)
            {
                if (!fEndOfTag)
                    return TagI;
                else
                    return TagXI;
            }
        break;

        case 'X':
        case 'x':
            if (stricmp(p2, "MP") == 0) // XMP
            {
                if (!fEndOfTag)
                    return TagPRE;
                else
                    return TagXPRE;
            }
        break;
    }

    return pProcessor;
}

/*
 *@@ HandleTag:
 *      called by txvConvertFromHTML when a "<" character
 *      is found in the source buffer. This calls
 *      FindTagProcessor in turn to find the Tag*
 *      function which handles the tag.
 *
 *@@added V0.9.3 (2000-05-18) [umoeller]
 */

STATIC VOID HandleTag(PCOPYTARGET pct)
{
    PSZ     pStartOfTag = pct->pSource;
    // '<' == begin of tag:

    // is it a comment?  <!-- ... -->
    if (strncmp(pStartOfTag + 1, "!--", 3) == 0)
    {
        // start of comment:
        // find end of comment
        PSZ pEnd = strstr(pStartOfTag, "-->");
        if (pEnd)
            // found:
            // search on after end of comment
            pct->pSource = pEnd + 3;
        else
        {
            // end of comment not found:
            // stop formatting...
            pct->pSource++;
            return;
        }
    }
    else
    {
        // no comment:
        // find end of tag
        PSZ     p2 = pStartOfTag + 1,
                pNextClose = 0,     // receives first '>' after '<'
                pNextSpace = 0;     // receives first ' ' after '<'
        BOOL    fCont = TRUE;
        while (fCont)
        {
            switch (*p2)
            {
                case ' ':
                case '\r':
                case '\n':
                    // store first space after '<'
                    if (!pNextSpace)
                        pNextSpace = p2;
                    // overwrite line breaks with spaces;
                    // otherwise we cannot handle tags which go across
                    // several lines, which is valid HTML
                    *p2 = ' ';
                break;

                case '>':   // end of tag found:
                    pNextClose = p2;
                    fCont = FALSE;
                break;

                case '<':
                    // another opening tag:
                    // that's an HTML error
                    AppendChar(pct,
                               *pct->pSource++);
                    fCont = FALSE;
                break;

                case 0:
                    fCont = FALSE;
                break;
            }
            p2++;
        }

        if (pNextClose)
        {
            // end of tag found:
            ULONG cbTag;
            // PSZ pStartOfAttrs = 0;

            if ((pNextSpace) && (pNextSpace < pNextClose))
            {
                // we have attributes:
                cbTag = pNextSpace - (pStartOfTag + 1);
                // pStartOfAttrs = pNextSpace;
            }
            else
                cbTag = pNextClose - (pStartOfTag + 1);

            if (!cbTag)
            {
                // happens if we have a "<>" in the text:
                // just insert the '<>' and go on, we have no tag here
                AppendChar(pct,
                           *pct->pSource++);
                AppendChar(pct,
                           *pct->pSource++);
            }
            else
            {
                PFNPROCESSTAG pTagProcessor;

                pct->cSaved = *(pStartOfTag + cbTag + 1);
                // add a null terminator
                *(pStartOfTag + cbTag + 1) = 0;

                // find corresponding tag converter function
                // from G_TagProcessors map
                pTagProcessor = FindTagProcessor(pStartOfTag + 1); // pszTag);

                // restore char under null terminator
                *(pStartOfTag + cbTag + 1) = pct->cSaved;

                // reset new source ptr; the tag handler
                // can modify this
                pct->pNewSource = NULL;

                if (pTagProcessor)
                {
                    // tag understood:

                    // terminate string after closing tag
                    pct->cSaved = *(pNextClose + 1); // can be null byte!
                    *(pNextClose + 1) = 0;

                    // did we have attributes?
                    if (pNextSpace)
                        pct->pszAttributes = pNextSpace;

                    // finally, call the tag handler
                    (pTagProcessor)    // function
                        (pct);          // argument

                    *(pNextClose + 1) = pct->cSaved;
                }

                if (pct->pNewSource == NULL)
                    // tag handler needs no special processing:
                    // skip '>' too
                    pct->pSource = pNextClose + 1;
                else
                    // tag handler has skipped something:
                    pct->pSource = pct->pNewSource;
            }
        }
    }
}

/*
 *@@ ConvertEscape:
 *      called by HandleEscape to find the ANSI (CP 1004)
 *      character for the given escape sequence (pszTag).
 *
 *      pszTag must be null-terminated and contain only
 *      the stuff between "&" and ";".
 *
 *      This is really ugly spaghetti, but it's the fastest
 *      way to do it.
 *
 *@@added V0.9.4 (2000-06-10) [umoeller]
 */

STATIC unsigned char ConvertEscape(PSZ pszTag)
{
    CHAR c0, c1;
    CHAR crc = 0;

    PSZ p2 = pszTag + 2;

    c0 = *pszTag;
    c1 = *(pszTag + 1);

    switch (c0)
    {
        case 'a':
            switch (c1)
            {
                case 'a':
                    if (strcmp(p2, "cute") == 0)
                        return 225;
                break;

                case 'c':
                    if (strcmp(p2, "irc") == 0)
                        return 226;
                    else if (strcmp(p2, "ute") == 0)
                        return 180;
                break;

                case 'e':
                    if (strcmp(p2, "lig") == 0)
                        return 230;
                break;

                case 'g':
                    if (strcmp(p2, "rave") == 0)
                        return 224;
                break;

                case 'm':
                    if (strcmp(p2, "p") == 0)
                        return '&';
                break;

                case 'r':
                    if (strcmp(p2, "ing") == 0)
                        return 229;
                break;

                case 't':
                    if (strcmp(p2, "ilde") == 0)
                        return 227;
                break;

                case 'u':
                    if (strcmp(p2, "ml") == 0)
                        return 228;
                break;
            }
        break;

        case 'b':
            if (strcmp(pszTag + 1, "rvbar") == 0)
                return 166;
        break;

        case 'c':
            switch (c1)
            {
                case 'c':
                    if (strcmp(p2, "edil") == 0)
                        return 231;
                break;

                case 'e':
                    if (strcmp(p2, "dil") == 0)
                        return 184;
                    else if (strcmp(p2, "nt") == 0)
                        return 162;
                break;

                case 'o':
                    if (strcmp(p2, "py") == 0)
                        return 169;
                break;

                case 'u':
                    if (strcmp(p2, "rren") == 0)
                        return 164;
            }
        break;

        case 'd':
            switch (c1)
            {
                case 'e':
                    if (strcmp(p2, "g") == 0) return 176;
                break;

                case 'i':
                    if (strcmp(p2, "vide") == 0) return 247;
                break;
            }
        break;

        case 'e':
            switch (c1)
            {
                case 'a':
                    if (strcmp(p2, "cute") == 0) return 233;
                break;

                case 'c':
                    if (strcmp(p2, "irc") == 0) return 234;
                break;

                case 'g':
                    if (strcmp(p2, "rave") == 0) return 232;
                break;

                case 't':
                    if (strcmp(p2, "h") == 0) return 240;
                break;

                case 'u':
                    if (strcmp(p2, "ml") == 0) return 235;
                break;
            }
        break;

        case 'f':
            switch (c1)
            {
                case 'r':
                    if (strcmp(p2, "ac14") == 0) return 188;
                    if (strcmp(p2, "ac12") == 0) return 189;
                    if (strcmp(p2, "ac34") == 0) return 190;
                break;
            }
        break;

        case 'g':
            switch (c1)
            {
                case 't':
                    if (*p2 == 0) return '>';
            }
        break;

        case 'i':
            switch (c1)
            {
                case 'a':
                    if (strcmp(p2, "cute") == 0) return 237;
                break;

                case 'c':
                    if (strcmp(p2, "irc") == 0) return 238;
                break;

                case 'g':
                    if (strcmp(p2, "rave") == 0) return 236;
                break;

                case 'e':
                    if (strcmp(p2, "xcl") == 0) return 161;
                break;

                case 'q':
                    if (strcmp(p2, "uest") == 0) return 191;
                break;

                case 'u':
                    if (strcmp(p2, "ml") == 0) return 239;
            }
        break;

        case 'l':
            switch (c1)
            {
                case 't':
                    if (*p2 == 0)
                        return '<';
                break;

                case 'a':
                    if (strcmp(p2, "quo") == 0) return 171;
            }
        break;

        case 'm':
            switch (c1)
            {
                case 'a':
                    if (strcmp(p2, "cr") == 0) return 175;
                break;

                case 'i':
                    if (strcmp(p2, "cro") == 0) return 181;
                    if (strcmp(p2, "ddot") == 0) return 183;
                break;
            }
        break;

        case 'n':
            switch (c1)
            {
                case 'b':
                    if (strcmp(p2, "sp") == 0) return 160;
                break;

                case 'o':
                    if (strcmp(p2, "t") == 0) return 172;
                break;

                case 't':
                    if (strcmp(p2, "ilde") == 0) return 241;
            }
        break;

        case 'o':
            switch (c1)
            {
                case 'a':
                    if (strcmp(p2, "cute") == 0) return 243;
                break;

                case 'c':
                    if (strcmp(p2, "irc") == 0) return 244;
                break;

                case 'g':
                    if (strcmp(p2, "rave") == 0) return 242;
                break;

                case 'r':
                    if (strcmp(p2, "df") == 0) return 170;
                    if (strcmp(p2, "dm") == 0) return 186;
                break;

                case 's':
                    if (strcmp(p2, "lash") == 0) return 248;
                break;

                case 't':
                    if (strcmp(p2, "ilde") == 0) return 245;
                break;

                case 'u':
                    if (strcmp(p2, "ml") == 0) return 246;
            }
        break;

        case 'p':
            switch (c1)
            {
                case 'a':
                    if (strcmp(p2, "ra") == 0) return 182;
                break;

                case 'l':
                    if (strcmp(p2, "usmn") == 0) return 177;
                break;

                case 'o':
                    if (strcmp(p2, "und") == 0) return 163;
            }
        break;

        case 'q':
            if (strcmp(pszTag, "quot") == 0) return  '"';
        break;

        case 'r':
            if (strcmp(pszTag, "raquo") == 0) return 187;
            if (strcmp(pszTag, "reg") == 0) return 174;
        break;

        case 's':
            switch (c1)
            {
                case 'z':
                    if (strcmp(p2, "lig") == 0) return 223;
                break;

                case 'e':
                    if (strcmp(p2, "ct") == 0) return 167;
                break;

                case 'h':
                    if (strcmp(p2, "y") == 0) return 173;
                break;

                case 'u':
                    if (strcmp(p2, "p1") == 0) return 185;
                    if (strcmp(p2, "p2") == 0) return 178;
                    if (strcmp(p2, "p3") == 0) return 179;
            }
        break;

        case 't':
            if (strcmp(pszTag, "thorn") == 0) return 254;
            if (strcmp(pszTag, "times") == 0) return 215;
        break;

        case 'u':
            switch (c1)
            {
                case 'a':
                    if (strcmp(p2, "cute") == 0) return 250;
                break;

                case 'c':
                    if (strcmp(p2, "irc") == 0) return 251;
                break;

                case 'g':
                    if (strcmp(p2, "rave") == 0) return 249;
                break;

                case 'm':
                    if (strcmp(p2, "l") == 0) return 168;
                break;

                case 'u':
                    if (strcmp(p2, "ml") == 0) return 252;
            }
        break;

        case 'y':
            if (strcmp(pszTag, "yacute") == 0) return 253;
            if (strcmp(pszTag, "yen") == 0) return 165;
            if (strcmp(pszTag, "yuml") == 0) return 255;
        break;

        case 'A':
            switch (c1)
            {
                case 'u':
                    if (strcmp(p2, "ml") == 0) return 196;
                break;

                case 'a':
                    if (strcmp(p2, "cute") == 0) return 193;
                break;

                case 'c':
                    if (strcmp(p2, "irc") == 0) return 194;
                break;

                case 'E':
                    if (strcmp(p2, "lig") == 0) return 198;
                break;

                case 'g':
                    if (strcmp(p2, "rave") == 0) return 192;
                break;

                case 'r':
                    if (strcmp(p2, "ing") == 0) return 197;
                break;

                case 't':
                    if (strcmp(p2, "ilde") == 0) return 195;
            }
        break;

        case 'C':
            if (strcmp(pszTag, "Ccedil") == 0) return 199;
        break;

        case 'E':
            if (strcmp(pszTag, "Ecirc") == 0) return 202;
            if (strcmp(pszTag, "Eacute") == 0) return 201;
            if (strcmp(pszTag, "Egrave") == 0) return 200;
            if (strcmp(pszTag, "ETH") == 0) return 208;
            if (strcmp(pszTag, "Euml") == 0) return 203;
        break;

        case 'I':
            if (strcmp(pszTag, "Icirc") == 0) return 206;
            if (strcmp(pszTag, "Iacute") == 0) return 205;
            if (strcmp(pszTag, "Igrave") == 0) return 204;
            if (strcmp(pszTag, "Iuml") == 0) return 207;
        break;

        case 'N':
            if (strcmp(pszTag, "Ntilde") == 0) return 209;
        break;

        case 'O':
            switch (c1)
            {
                case 'u':
                    if (strcmp(p2, "ml") == 0) return 214;
                break;

                case 'a':
                    if (strcmp(p2, "cute") == 0) return 211;
                break;

                case 'c':
                    if (strcmp(p2, "irc") == 0) return 212;
                break;

                case 'g':
                    if (strcmp(p2, "rave") == 0) return 210;
                break;

                case 't':
                    if (strcmp(p2, "ilde") == 0) return 213;
                break;

                case 's':
                    if (strcmp(p2, "lash") == 0) return 216;
            }
        break;

        case 'U':
            switch (c1)
            {
                case 'a':
                    if (strcmp(p2, "cute") == 0) return 218;
                break;

                case 'c':
                    if (strcmp(p2, "irc") == 0) return 219;
                break;

                case 'g':
                    if (strcmp(p2, "rave") == 0) return 217;
                break;

                case 'u':
                    if (strcmp(p2, "ml") == 0) return 220;
            }
        break;

        case 'T':
            if (strcmp(pszTag, "THORN") == 0) return 222;
        break;

        case 'Y':
            if (strcmp(pszTag, "Yacute") == 0) return 221;
        break;
    }

    return crc;
}

/*
 *@@ HandleEscape:
 *      called by txvConvertFromHTML when a "&" character
 *      is found in the source buffer. This calls
 *      ConvertEscape in turn.
 *
 *@@added V0.9.3 (2000-05-18) [umoeller]
 */

STATIC VOID HandleEscape(PCOPYTARGET pct)
{
    // ampersand:
    // replace special characters
    PSZ     pStartOfTag = pct->pSource;
    // find end of tag
    PSZ     p2 = pStartOfTag,
            pNextClose = 0,
            pNextSpace = 0;
    BOOL fCont = TRUE;
    while (fCont)
    {
        switch (*p2)
        {
            case 0:
                fCont = FALSE;
            break;

            case ';':
                pNextClose = p2;
                fCont = FALSE;
            break;

            case ' ':
                if (!pNextSpace)
                    pNextSpace = p2;
            break;
        }
        p2++;
    }

    if (!pNextClose)
        // no closing tag found:
        // just insert the '&' and go on, we have no tag here
        AppendChar(pct,
                   *pct->pSource++);
    else
    {
        if ((pNextSpace) && (pNextSpace < pNextClose))
            // space before ';':
            // just insert the '&' and go on, we have no tag here
            AppendChar(pct,
                       *pct->pSource++);
        else if ((!pNextClose) || (pNextClose <= pStartOfTag + 1))
            AppendChar(pct,
                       *pct->pSource++);
        else
        {
            ULONG ulCode = 0;

            // create substring with tag
            PSZ pszTag = pStartOfTag + 1;
            *pNextClose = 0;

            if (*pszTag == '#')
            {
                // latin-1 or Unicode encoding (&#000;)
                ulCode = atoi(pszTag + 1);

                // next input: char after ';'
                pct->pSource = pNextClose + 1;
            }
            else
            {
                // named entity:
                // find char code corresponding to escape
                // from G_EscapeProcessors map
                ulCode = ConvertEscape(pszTag);
                if (ulCode)
                    // tag supported:
                    pct->pSource = pNextClose + 1;
                else
                    // tag not supported:
                    ulCode = *pct->pSource++;
            }

            // restore closing tag which we overwrote
            *pNextClose = ';';

            if (ulCode)
            {
                AppendLinebreakCheck(pct);

                AppendChar(pct,
                           (CHAR)ulCode);
                pct->fSkipNextSpace = FALSE;
            }
        }
    }
}

/* ******************************************************************
 *
 *   Entry points
 *
 ********************************************************************/

/*
 *@@ txvConvertFromHTML:
 *      this modifies the given text string (which should
 *      be the complete BODY block of any HTML file) so
 *      that all HTML tags are removed and replaced with
 *      escape sequences that the XTextView control understands.
 *
 *      The buffer gets reallocated by this function, so it
 *      must be free()'able.
 *
 *      So, to have the XTextView control display an HTML file,
 *      do this:
 *
 *      1)  Load an HTML file into a buffer allocated by malloc().
 *
 *      2)  Call txvConvertFromHTML.
 *
 *      3)  Call WinSetWindowText on the XTextView control with
 *          the modified buffer.
 *
 *      This understands the following limited subset of HTML:
 *
 *      Paragraph tags:
 *
 *      -- P, BR
 *      -- PRE, /PRE
 *      -- UL, /UL, OL, /OL, LI
 *      -- DL, /DL, DT, DD
 *      -- H1, /H1 thru H6, /H6
 *      -- Comments (<!-- .... -->)
 *
 *      Character tags:
 *
 *      -- B, /B, STRONG, /STRONG
 *      -- I, /I, EM, /EM, VAR, /VAR, CITE, /CITE
 *      -- CODE, /CODE, SAMP, /SAMP, KBD, /KBD, TT, /TT
 *      -- U, /U
 *      -- STRIKE, /STRIKE
 *      -- CODE, /CODE
 *
 *      The most obvious limitation is that neither tables
 *      nor frames are supported. Also forget about CSS
 *      and JavaScript, of course.
 *
 *      All the ampersand (&amp; something) sequences defined
 *      in HTML 3 are properly translated.
 *
 *      Note: Those are translated to the ANSI (MS-Windows,
 *      OS/2 codepage 1004) character set. This has the
 *      following characteristics:
 *
 *      -- Codes 0-127 are identical to ASCII and thus
 *         ISO 8559-1 ("Latin 1") also.
 *
 *      -- Codes 160-255 are identical to ISO 8559-1 ("Latin 1").
 *
 *      -- Codes 128-159 are NOT defined in ISO 8559-1, but
 *         Netscape treats those as ANSI as well, so we do too.
 *
 *      As a result, consider the output to be in OS/2 codepage
 *      1004. Either set your codepage to that (WinSetCp)
 *      or translate the output (WinCpTranslateString).
 *
 *      &#xxx; tags (with xxx being a decimal) are considered
 *      ANSI codes as well. Even though HTML 4.0 allows Unicode
 *      characters > 255 to be inserted this way, we ignore
 *      those. Unicode chars from 0 to 255 are identical to
 *      ANSI, so for &#000; to &#255;, we are HTML-compliant.
 *
 *      All other tags are completely thrown out.
 *
 *@@added V0.9.3 (2000-05-06) [umoeller]
 *@@changed V0.9.20 (2002-08-10) [umoeller]: changed prototype
 */

BOOL txvConvertFromHTML(PSZ *ppszText,          // in/out: text (gets reallocated)
                        PSZ *ppszTitle,         // out: if != NULL, receives malloc'd buffer with HTML title
                        PULONG pulProgress,     // out: progress (ptr can be NULL)
                        PBOOL pfCancel)         // in: cancel flag (ptr can be NULL)
{
    BOOL    brc = TRUE;

    ULONG   cbSource = strlen(*ppszText);

    COPYTARGET  ct = {0};

    lstInit(&ct.llLists,
            TRUE);      // free items

    ct.ppszTitle = ppszTitle;       // V0.9.20 (2002-08-10) [umoeller]
                // can be NULL

    ct.pSource = *ppszText;
    // skip leading spaces
    ct.fSkipNextSpace = TRUE;

    // step 2:
    // actual tags formatting

    while (TRUE)
    {
        CHAR    c = *ct.pSource;

        if (pfCancel)
            if (*pfCancel)
            {
                brc = FALSE;
                break;
            }

        if (!c)
            // null terminator reached:
            break;

        // calculate progress
        if (pulProgress)
            *pulProgress = ((ct.pSource - *ppszText)     // characters done
                            * 100
                            / cbSource);              // characters total

        switch (c)
        {
            case '<':
                HandleTag(&ct);
            break;

            case '&':
                HandleEscape(&ct);
            break;

            case '\r':
                // skip
                if (!ct.fSkipNextSpace)
                {
                    AppendChar(&ct,
                               ' ');
                    // ct.fNeedsLinebreak = FALSE;
                    // but skip leading spaces which might follow
                    if (!ct.fPRE)
                        ct.fSkipNextSpace = TRUE;
                }
                ct.pSource++;
            break;

            case '\t':
            {
                if (ct.fPRE)
                {
                    ULONG ul;
                    for (ul = 0;
                         ul < 8;
                         ul++)
                        AppendChar(&ct,
                                   ' ');
                }
                else
                {
                    // not in PRE block:
                    if (    (!ct.fSkipNextSpace)
                         // && (!ct.fNeedsLinebreak)
                       )
                        // last was not space: copy
                        AppendChar(&ct,
                                   ' ');

                    ct.fSkipNextSpace = TRUE;
                }

                // skip the tab
                ct.pSource++;
            break; }

            case '\n':
            {
                // newline char:
                if (!ct.fPRE)
                {
                    // if not in PRE mode, replace with space
                    if (!ct.fSkipNextSpace)
                    {
                        AppendChar(&ct,
                                   ' ');
                        // ct.fNeedsLinebreak = FALSE;
                        // but skip leading spaces which might follow
                        ct.fSkipNextSpace = TRUE;
                    }
                }
                else
                    // in PRE mode, preserve line breaks
                    AppendChar(&ct, '\n'); // ct.fNeedsLinebreak = TRUE;

                ct.pSource++;
            break; }

            case '\xFF':
            {
                AppendChar(&ct,
                           ' ');
                ct.pSource++;
            break; }

            case ' ':
                if (!ct.fPRE)
                {
                    // is space, and not in PRE block:
                    if (    (!ct.fSkipNextSpace)
                         // && (!ct.fNeedsLinebreak)
                       )
                        // last was not space: copy
                        AppendChar(&ct,
                                   ' ');

                    ct.fSkipNextSpace = TRUE;
                }
                else
                    // in PRE, always add all spaces
                    AppendChar(&ct,
                               ' ');
                ct.pSource++;
            break;

            default:
                // if we're not inserting escapes or anything,
                // check if a linebreak is needed
                AppendLinebreakCheck(&ct);

                AppendChar(&ct,
                           *ct.pSource++);
                ct.fSkipNextSpace = FALSE;
                ct.fSkipNextLinebreak = FALSE;

        } // end switch (*pSource);
    } // end while (*pSource)
    AppendChar(&ct,
               '\n');
    // append null-terminator
    AppendChar(&ct,
               0);

    free(*ppszText);
    *ppszText = ct.pszNew;

    lstClear(&ct.llLists);

    return brc;
}


