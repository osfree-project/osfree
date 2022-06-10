
/*
 *@@sourcefile textview.c:
 *      all-new XTextView control as well as device-independent
 *      text formatting and printing. Whoa.
 *
 *      <B>Text view control</B>
 *
 *      This is a read-only control to display any given text
 *      (PSZ) in any given font. As opposed to a multi-line entry
 *      field (MLE), this can handle multiple fonts and character
 *      and paragraph formatting. Also, this thing sets its scroll
 *      bars right, which is one of the most annoying bugs in the
 *      MLE control.
 *
 *      This is currently in the process of turning into a full-fledged
 *      "rich text" control. For example, the WarpIN "Readme" pages
 *      use this control.
 *
 *      This is all new with V0.9.1. Great changes have been made
 *      with V0.9.3.
 *
 *      To use the text view control, you must call txvRegisterTextView
 *      in your application first. This registers the WC_XTEXTVIEW
 *      window class with PM.
 *
 *      Then create your XTextView using WinCreateWindow. The
 *      XTextView control has an XTEXTVIEWCDATA control data
 *      structure which optionally can be passed to WinCreateWindow
 *      like this:
 *
 +          XTEXTVIEWCDATA xtxvCData;
 +          memset(&xtxvCData, 0, sizeof(xtxvCData));
 +          xtxvCData.cbData = sizeof(xtxvCData);
 +          xtxvCData.flStyle  = XTXF_VSCROLL;
 +          xtxvCData.ulXBorder = 20;
 +          xtxvCData.ulYBorder = 20;
 +          G_hwndProcView = WinCreateWindow(hwndClient,        // parent
 +                                           WC_XTEXTVIEW,      // class
 +                                           "",                // title, always ignored
 +                                           WS_VISIBLE,        // style flags
 +                                           0, 0, 100, 100,    // pos and size
 +                                           hwndClient,        // owner
 +                                           HWND_TOP,          // z-order
 +                                           ID_PROCINFO,       // win ID
 +                                           &xtxvCData,        // control data
 +                                           0);                // presparams
 +
 *      <B>Setting the text to be displayed</B>
 *
 *      The text to be displayed must be passed to the control using
 *      the standard WinSetWindowText function (upon which PM sends a
 *      WM_SETWINDOWPARMS message to the control), which is then
 *      automatically formatted and painted.
 *
 *      However, since the XTextView control is extremely capable,
 *      a few things need to be noted:
 *
 *      -- The text view assumes that lines terminate with \n ONLY.
 *         The \r char is used for soft line breaks (start new line
 *         in the same paragraph, similar to the HTML BR tag). If
 *         you give the control the usual OS/2 \r\n sequence, you
 *         get large spacings. Use txvStripLinefeeds to strip the
 *         \r characters before setting the text.
 *
 *         In short, to give the control any text, do this:
 +
 +              PSZ psz = ...  // whatever, load string
 +              txvStripLinefeeds(&psz);        // reallocates
 +              WinSetWindowText(hwndTextView, psz);
 *
 *      -- The control uses the \xFF (255) character internally as
 *         an escape code for formatting commands. See "Escape codes"
 *         below. If your text contains this character, you should
 *         overwrite all occurences with spaces, or they will be
 *         considered an escape code, which will cause problems.
 *
 *      -- If you don't care about learning all the escape codes,
 *         you can automatically have HTML code converted to the
 *         XTextView format using txvConvertFromHTML, which will
 *         automatically insert all the codes right from plain
 *         HTML. In the above code, use txvConvertFromHTML instead
 *         of txvStripLinefeeds.
 *
 *      <B>Code page support</B>
 *
 *      The XTextView control assumes that the text given to it uses
 *      the same codepage as the message queue (thread) on which
 *      the control is running. So if you need codepage support,
 *      issue WinSetCp before creating the text view control.
 *
 *      <B>Text formatting</B>
 *
 *      The XTextView control has a default paragraph format which
 *      determines how text is formatted. If you don't change this
 *      format, the control performs no word-wrapping and displays
 *      all text "as is", that is, practically no formatting is
 *      performed.
 *
 *      You can change the default paragraph format by sending
 *      TXM_SETPARFORMAT to the control. This takes a XFMTPARAGRAPH
 *      structure for input.
 *
 *      To quickly enable word-wrapping only, we have the extra
 *      TXM_SETWORDWRAP message. This changes the word-wrapping flag
 *      in the default paragraph format only so you don't have to
 *      mess with all the rest.
 *
 *      The XTextView control is extremely fast in formatting. It
 *      does pre-calculations once so that resizing the text
 *      window does not perform a full reformat, but a quick
 *      format based on the pre-calculations.
 *
 *      Presently, formatting is done synchronously. It is planned
 *      to put formatting into a separate thread. Performance is
 *      acceptable already now unless very large texts (> 200 KB)
 *      are formatted (tested on a PII-400 machine).
 *
 *      <B>Presentation Parameters</B>
 *
 *      The XTextView control recognizes the following presentation
 *      parameters:
 *
 *      -- PP_BACKGROUNDCOLOR; if not set, SYSCLR_DIALOGBACKGROUND
 *         (per default gray) is used.
 *
 *      -- PP_FOREGROUNDCOLOR: if not set, SYSCLR_WINDOWSTATICTEXT
 *         (per default blue) is used to signify that the text
 *         cannot be worked on.
 *
 *      -- PP_FONTNAMESIZE: default font. This is the system font,
 *         if not set.
 *
 *      This implies that fonts and colors can be dropped on the
 *      control in the normal way. Font changes will cause a reformat.
 *
 *      Changing those presentation parameters is equivalent to
 *      changing the corresponding fields in the default paragraph
 *      format using TXM_SETPARFORMAT.
 *
 *      <B>Escape codes</B>
 *
 *      All XTextView escape codes start with a \xFF (255) character,
 *      followed by at least one more character. The escape sequences
 *      are variable in length and can have parameters. For details,
 *      see textview.h where all these are listed.
 *
 *      Escape codes are evaluated by txvFormatText during formatting.
 *
 *      If you choose to give the text view control a text which
 *      contains escape codes, you better make sure that you get the
 *      exact codes right, or the text view control can crash. The
 *      control has been optimized for speed, so no checking is done
 *      on escape sequences.
 *
 *      <B>Device-independent text formatting</B>
 *
 *      If the features of the XTextView control satisfy your needs,
 *      there's not much to worry about. However, if you're interested
 *      in formatting the text yourself, here's more:
 *
 *      This file has the txvFormatText function, which is capable
 *      of formatting an input string into any HPS. This works for
 *      windows (used by the text view control) and printers (used
 *      by txvPrint). Word-wrapping is supported. This is used by
 *      the XTextView control internally whenever (re)formatting is
 *      needed: either when the text is set or the formatting parameters
 *      (fonts, margins, etc.) have changed.
 *
 *      These functions are designed to be used in a two-step process:
 *      first format the text (using txvFormatText), then paint it
 *      (using txvPaintText) for viewing or printing.
 *      This speeds up painting dramatically, because formatting
 *      might take some time.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\textview.h"
 *
 *@@added V0.9.1 (2000-02-13) [umoeller]
 */

/*
 *      Copyright (C) 2000-2008 Ulrich M”ller.
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

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WININPUT
#define INCL_WINRECTANGLES
#define INCL_WINPOINTERS
#define INCL_WINSYS
#define INCL_WINSCROLLBARS
#define INCL_WINSTDFONT
#define INCL_WINCOUNTRY

#define INCL_DEV
#define INCL_SPL
#define INCL_SPLDOSPRINT

#define INCL_GPIPRIMITIVES
#define INCL_GPILCIDS
#define INCL_GPILOGCOLORTABLE
#define INCL_GPITRANSFORMS
#define INCL_GPIREGIONS

#define INCL_ERRORS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/comctl.h"
#include "helpers/gpih.h"
#include "helpers/linklist.h"
#include "helpers/stringh.h"
#include "helpers/winh.h"
#include "helpers/xstring.h"            // extended string helpers

#include "helpers/textview.h"
#include "helpers/textv_html.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Window classes\XTextView control
 *      See textview.c.
 */

/* ******************************************************************
 *
 *   Device-independent functions
 *
 ********************************************************************/

/*
 *@@ txvInitFormat:
 *
 */

VOID txvInitFormat(PXFORMATDATA pxfd)
{
    memset(pxfd, 0, sizeof(XFORMATDATA));
    lstInit(&pxfd->llRectangles,
            TRUE);      // auto-free items
    lstInit(&pxfd->llWords,
            TRUE);      // auto-free items
    xstrInit(&pxfd->strOrigText, 0);  // WarpIN V1.0.18
    xstrInit(&pxfd->strViewText, 0);
}

/*
 *@@ SetSubFont:
 *
 *@@added V0.9.3 (2000-05-06) [umoeller]
 */

STATIC VOID SetSubFont(HPS hps,
                       PXFMTFONT pFont,
                       ULONG ulPointSize,
                       PSZ pszFaceName,
                       ULONG flFormat)
{
    CHAR    ac[256];
    ULONG   ul;
    POINTL  ptlStart = {0, 0},
            aptl[257];

    if (pFont->lcid)
    {
        // font already loaded:
        if (GpiQueryCharSet(hps) == pFont->lcid)
            // font currently selected:
            GpiSetCharSet(hps, LCID_DEFAULT);
        GpiDeleteSetId(hps, pFont->lcid);

    }

    if (pszFaceName)
        pFont->lcid = gpihFindFont(hps,
                                   ulPointSize,
                                   TRUE,        // family, not face name
                                   pszFaceName,
                                   flFormat,
                                   &pFont->FontMetrics);
    else
        pFont->lcid = LCID_DEFAULT; // 0

    GpiSetCharSet(hps, pFont->lcid);
    if (pFont->FontMetrics.fsDefn & FM_DEFN_OUTLINE)
        // is outline font:
        gpihSetPointSize(hps, ulPointSize);

    for (ul = 0;
         ul < 256;
         ul++)
        ac[ul] = ul;

    GpiQueryCharStringPosAt(hps,
                            &ptlStart,
                            0,
                            254,        // starting at one
                            ac + 1,     // starting at one
                            NULL,
                            aptl);
    // now compute width of every char
    for (ul = 1;
         ul < 256;
         ul++)
    {
        pFont->alCX[ul] = aptl[ul+1].x - aptl[ul].x;
    }
}

/*
 *@@ SetFormatFont:
 *      creates logical fonts from the specified
 *      font information.
 *
 *@@added V0.9.3 (2000-05-06) [umoeller]
 */

STATIC VOID SetFormatFont(HPS hps,           // in: HPS to select default font into
                          PXFMTCHARACTER pxfmtc, // in/out: format data
                          ULONG ulPointSize, // in: font point size (e.g. 12) or 0
                          PSZ pszFaceName)   // in: font face name (e.g. "Courier") or NULL
{
    pxfmtc->lPointSize = ulPointSize;

    // regular
    SetSubFont(hps,
               &pxfmtc->fntRegular,
               ulPointSize,
               pszFaceName,
               0);

    // bold
    SetSubFont(hps,
               &pxfmtc->fntBold,
               ulPointSize,
               pszFaceName,
               FATTR_SEL_BOLD);

    // italics
    SetSubFont(hps,
               &pxfmtc->fntItalics,
               ulPointSize,
               pszFaceName,
               FATTR_SEL_ITALIC);

    // bold italics
    SetSubFont(hps,
               &pxfmtc->fntBoldItalics,
               ulPointSize,
               pszFaceName,
               FATTR_SEL_BOLD | FATTR_SEL_ITALIC);
}

/*
 *@@ AppendCharNoCheck:
 *
 *@@added V0.9.3 (2000-05-07) [umoeller]
 */

STATIC VOID AppendCharNoCheck(char **ppszNew,
                              PULONG pcbNew,
                              char **ppTarget,
                              char c)
{
    ULONG   cbSizeThis = *ppTarget - *ppszNew;
    if (cbSizeThis >= *pcbNew)
    {
        // more mem needed:
        *pcbNew += 10000;
        *ppszNew = (PSZ)realloc(*ppszNew, *pcbNew);
                    // if first call, pszNew is NULL, and realloc
                    // behaves just like malloc
        // adjust target, because ptr might have changed
        *ppTarget = *ppszNew + cbSizeThis;
    }

    **ppTarget = c;
    (*ppTarget)++;
}

/*
 *@@ txvStripLinefeeds:
 *      this removes all linefeeds (\r) from
 *      the specified string to prepare it
 *      for display with the text view control.
 *
 *      This also replaces tabs (\t) with ulTabSize spaces.
 *
 *      The buffer gets reallocated by this function, so it
 *      must be free()'able.
 *
 *@@added V0.9.3 (2000-05-07) [umoeller]
 *@@changed V0.9.20 (2002-08-10) [umoeller]: now stripping \xFF too
 *@@changed V1.0.18 (2008-11-24) [pr]: fixes \xFF stripping @@fixes 1116
 */

VOID txvStripLinefeeds(char **ppszText,
                       ULONG ulTabSize)
{
    PSZ     pSource = *ppszText;
    ULONG   cbNew = 1000;
    PSZ     pszNew = (PSZ)malloc(cbNew);
    PSZ     pTarget = pszNew;
    ULONG   ul;

    while (*pSource)
    {
        switch (*pSource)
        {
            case '\r':
                pSource++;
            break;

            case '\t':
                for (ul = 0;
                     ul < ulTabSize;
                     ul++)
                    AppendCharNoCheck(&pszNew,
                                      &cbNew,
                                      &pTarget,
                                      ' ');

                // skip the tab
                pSource++;
            break;

            case TXVESC_CHAR:        // V0.9.20 (2002-08-10) [umoeller]
                AppendCharNoCheck(&pszNew,
                                  &cbNew,
                                  &pTarget,
                                  ' ');
                pSource += 3;  // V1.0.18
            break;

            default:
                AppendCharNoCheck(&pszNew,
                                  &cbNew,
                                  &pTarget,
                                  *pSource++);
        }
    }

    AppendCharNoCheck(&pszNew,
                      &cbNew,
                      &pTarget,
                      '\n');
    AppendCharNoCheck(&pszNew,
                      &cbNew,
                      &pTarget,
                      0);

    free(*ppszText);
    *ppszText = pszNew;
}

/* ******************************************************************
 *
 *   Device-independent text formatting
 *
 ********************************************************************/

/*
 *@@ strhFindEOL2:
 *      finds the end of a line.
 *
 *      An "end of line" means the next \r, \n, or \0 character
 *      after *ppszSearchIn.
 *
 *      This returns the pointer to that exact character, which
 *      can be equal or higher than *ppszSearchIn.
 *      This should never return NULL because at some point,
 *      there will be a null byte in your string (unless you have
 *      a heap problem).
 *
 *      If the EOL character is not null (\0), *ppszSearchIN is
 *      advanced to the first character of the _next_ line. This
 *      can be the EOL pointer plus one if you have a UNIX-style
 *      string (\n only at the end of each line) or EOL + 2 for
 *      DOS and OS/2-style EOLs (which have \r\n at the end of
 *      each line).
 *
 *@added V0.9.3 (2000-05-06) [umoeller]
 */

STATIC PSZ strhFindEOL2(PSZ *ppszSearchIn,        // in: where to search
                        PULONG pulOffset)       // out: offset (ptr can be NULL)
{
    PSZ     pThis = *ppszSearchIn,
            prc = NULL;
    while (TRUE)
    {
        if ( (*pThis == '\r') || (*pThis == '\n') || (*pThis == 0) )
        {
            prc = pThis;
            break;
        }
        pThis++;
    }

    // before modifying pointer, store offset
    if (pulOffset)
        *pulOffset = prc - *ppszSearchIn;

    if (*prc == 0)
    {
        // null byte (end of string):
        *ppszSearchIn = prc;
    }
    else
    {
        // not null byte (end of string):
        // skip following newline characters
        if (*prc == '\r')
        {
            if ( *(prc+1) == '\n')
                // we have a \r char next,
                // that's the DOS and OS/2 format (\r\n):
                // skip that too
                *ppszSearchIn = prc + 2;
            else
                *ppszSearchIn = prc + 1;
        }
        else if (*prc == '\n')
            // UNIX format (used by HTML formatter also):
            *ppszSearchIn = prc + 1;
    }

    // now:
    // 1)   prc points to the \r, \n, or \0 character (EOL)
    // 2)   *ppszSearchIn has been advanced to the first character
    //      of the next line or points to the \0 character

    return prc;
}

/* #define TXVFRECTF_EMPTY                  0x0001
#define TXVFRECTF_PARAGRAPHDONE          0x0002
#define TXVFRECTF_WORDSLEFT              0x0004
#define TXVFRECTF_STOPPEDONESCAPE        0x0008
#define TXVFRECTF_ENDOFTEXT              0x0010
   */

/*
 *@@ FORMATLINEBUF:
 *      worker structure to store various data
 *      in txvFormatText in between CreateWord
 *      calls. This has been created for speed
 *      so we don't have to pass all these on
 *      the stack all the time.
 *
 *@@added V0.9.3 (2000-05-06) [umoeller]
 */

typedef struct _FORMATLINEBUF
{
    PSZ             pLastChar;  // ptr to null terminator in text

    // formatting data; this is set by txvFormatText according
    // to escape characters and read by txvCreateRectangle
    XFMTPARAGRAPH   fmtp;
    PXFMTCHARACTER  pfmtc;      // pointer to character formatting data
    PXFMTFONT       pfmtf;      // pointer to font to use

    BOOL            fPre,
                    fBold,
                    fItalics;

    // current anchor
    PCSZ            pcszCurrentLinkTarget;
                        // this is != NULL if we're currently in a link block
                        // and points to an item in XFORMATDATA.llLinks
                        // (simply copied to the word structs that are created)

    // data copied to TXVWORD
    LONG            lcid;
    LONG            lPointSize;
    ULONG           flChar;     // any combination of CHS_UNDERSCORE and CHS_STRIKEOUT

    // counters, ...
    LONG            lXCurrent;  // current X position while adding words to rectangle
} FORMATLINEBUF, *PFORMATLINEBUF;

/*
 *@@ CreateWord:
 *
 *      --  If the word ends with one or several spaces,
 *          ppStartOfWord is set to the beginning of the
 *          next word (non-space character).
 *          pWord->ulFlags is set to 0.
 *
 *      --  If the word ends with an escape character,
 *          ppStartOfWord is set to point to the escape,
 *          which must be handled by the caller.
 *          pWord->ulFlags is set to TXVWORDF_GLUEWITHNEXT.
 *
 *      --  If the word ends with a \n or \r,
 *          ppStartOfWord is set to the beginning of the
 *          next line (first char after \n or \r). This
 *          may be another \n or \r, but the first one
 *          is skipped.
 *          pWord->ulFlags is set to TXVWORDF_LINEBREAK or
 *          TXVWORDF_LINEFEED.
 *
 *@@added V0.9.3 (2000-05-14) [umoeller]
 *@@changed V0.9.20 (2002-08-10) [umoeller]: rewrote link implementation
 */

STATIC PTXVWORD CreateWord(HPS hps,
                           PSZ *ppStartOfWord,
                           PFORMATLINEBUF pflbuf)
{
    PTXVWORD   pWord = NULL;

    // find next word:
    if (**ppStartOfWord)
    {
        PSZ         pWordStart = *ppStartOfWord,
                    pWordEnd = NULL;
        PSZ         pCheck = *ppStartOfWord;
        ULONG       cChars = 0;
                    // cCheck = 0;

        pWord = (PTXVWORD)malloc(sizeof(TXVWORD));
        memset(pWord, 0, sizeof(TXVWORD));
                // this includes fIsEscapeSequence = FALSE;
        pWord->pStart = pWordStart;

        // initially, this has pWordStart pointing
        // to *ppStartOfWord. If a word is found,
        // pWordStart is set to the first char of
        // the word and pWordEnd receives the
        // pointer to the first character after the word (probably space)
        if (strhGetWord(&pWordStart, // in/out
                        pflbuf->pLastChar,
                        " ",
                        "\x0d\x0a \xFF", // in: end chars; includes our escape!
                        &pWordEnd))  // out: first char after word
        {
            // whoa, found a word:
            while (*pWordEnd == ' ')
                pWordEnd++;

            cChars = (pWordEnd - *ppStartOfWord);
        }

        if (cChars)
        {
            POINTL      aptlText[TXTBOX_COUNT];
            // cChars is != 0 if strhGetWord succeeded AND the
            // line is not empty, so go on
            // cCheck = cChars;

            // advance input pointer
            *ppStartOfWord = pWordEnd;

            GpiQueryTextBox(hps,
                            // no. of chars since start of word:
                            cChars,
                            // first char:
                            pCheck,
                            TXTBOX_COUNT,
                            (PPOINTL)&aptlText);

            pWord->cChars = cChars;
            pWord->ulFlags = 0;

            if (cChars)
                pWord->ulCXWithSpaces = aptlText[TXTBOX_TOPRIGHT].x;
            else
                pWord->ulCXWithSpaces = 0;

            pWord->ulCY = aptlText[TXTBOX_TOPRIGHT].y
                          - aptlText[TXTBOX_BOTTOMRIGHT].y;
            // store base line ofs; aptlText[TXTBOX_BOTTOMRIGHT].y is negative
            // if the string has any characters drawn below the base line, e.g.
            // for the "g" and "y" characters
            pWord->ulBaseLineOfs = -aptlText[TXTBOX_BOTTOMRIGHT].y;
        }
        else
        {
            // no word found or empty line:
            pWord->ulCY = pflbuf->pfmtf->FontMetrics.lMaxBaselineExt;
        }

        switch (**ppStartOfWord)
        {
            case TXVESC_CHAR: // '\xFF':
                pWord->ulFlags = TXVWORDF_GLUEWITHNEXT;
            break;

            case '\n':
                pWord->ulFlags = TXVWORDF_LINEBREAK;
                (*ppStartOfWord)++;     // skip \n
            break;

            case '\r':
                pWord->ulFlags = TXVWORDF_LINEFEED;
                (*ppStartOfWord)++;     // skip \r
            break;
        }

        pWord->lcid = pflbuf->pfmtf->lcid;
        pWord->lPointSize = pflbuf->lPointSize;
        pWord->flChar = pflbuf->flChar;

        pWord->pcszLinkTarget = pflbuf->pcszCurrentLinkTarget; // 0 if none
    }

    return pWord;
}

/*
 *@@ ProcessEscapes:
 *      gets called when txvFormatText stops on an
 *      escape character (\xFF). This evaluates the
 *      escape sequence, reacts accordingly, and
 *      advances *ppCurrent to after the escape
 *      sequence so that regular processing can
 *      continue.
 *
 *      There are two types of escape sequences:
 *
 *      -- Those which are only relevant during word processing,
 *         such as character formatting attributes (bold, italics,
 *         font, size, ...). Those affect the TXVWORD structure
 *         directly and are thus never evaluated in step 2,
 *         rectangles correlation.
 *
 *      -- Those which affect spacings, margins, etc. (paragraph
 *         formatting). These need to be re-evaluated even during
 *         "quick" format, without words being recalculated, because
 *         those spacings affect the output rectangles.
 *
 *         If one of those sequences is encountered, this function
 *         appends a special TXVWORD structure to XFORMATDATA.llWords.
 *
 *@@added V0.9.3 (2000-05-07) [umoeller]
 */

STATIC PTXVWORD ProcessEscapes(char **ppCurrent,          // in/out: current position; initially points to esc char
                               PXFORMATDATA pxfd,         // in/out: formatting data
                               PFORMATLINEBUF pflbuf,     // in/out: formatting buffer
                               BOOL fWordsProcessed)      // FALSE during step 1 (words processing),
                                                          // TRUE during step 2 (rectangles correlation)
{
    PTXVWORD pEscapeWord = NULL;

    // this is set to TRUE either above or by txvCreateRectangle if
    // an escape character was found; txvCreateRectangle
    // then sets pCurrent to the escape character (\xFF)
    CHAR    cCode1 = *((*ppCurrent) + 1);
    CHAR    cCode2 = *((*ppCurrent) + 2);
    ULONG   ulSkip = 3; // per default, skip \xFF plus two
    CHAR    szDecimal[10];
    LONG    lDecimal;

    BOOL    fCreateWord = FALSE,
            fPaintEscapeWord = FALSE;

    switch (cCode1)
    {
        case 1:     // change font:
            // three decimals follow specifying the font
            memcpy(szDecimal, (*ppCurrent) + 2, 3);
            szDecimal[3] = 0;
            lDecimal = atoi(szDecimal);
            if (lDecimal == 0)
                pflbuf->pfmtc = &pxfd->fmtcStandard;
            else if (lDecimal == 1)
                pflbuf->pfmtc = &pxfd->fmtcCode;
            ulSkip = 5;
        break;

        case 2:     // B or /B
            if (cCode2 == 1)
                pflbuf->fBold = TRUE;
            else
                pflbuf->fBold = FALSE;
        break;

        case 3:     // I or /I
            if (cCode2 == 1)
                pflbuf->fItalics = TRUE;
            else
                pflbuf->fItalics = FALSE;
        break;

        case 4:     // U or /U
            if (cCode2 == 1)
                pflbuf->flChar |= CHS_UNDERSCORE;
            else
                pflbuf->flChar &= ~CHS_UNDERSCORE;
        break;

        case 5:     // STRIKE or /STRIKE
            if (cCode2 == 1)
                pflbuf->flChar |= CHS_STRIKEOUT;
            else
                pflbuf->flChar &= ~CHS_STRIKEOUT;
        break;

        case 6:     // A HREF= (link)
                    // changed implementation V0.9.20 (2002-08-10) [umoeller]
        {
            // this is variable in length and terminated with
            // another 0xFF char; what's in between is the
            // link target name and gets appended to
            // XFORMATDATA.llLinks
            PSZ pEnd;
            if (pEnd = strchr((*ppCurrent) + 2, 0xFF))
            {
                PSZ pszNewLink = strhSubstr((*ppCurrent) + 2, pEnd);
                lstAppendItem(&pxfd->llLinks,
                              pszNewLink);

                pflbuf->pcszCurrentLinkTarget = pszNewLink;

                ulSkip = pEnd - *ppCurrent + 1;
            }
        }
        break;

        case 7:     // /A HREF (end of link)
            pflbuf->pcszCurrentLinkTarget = NULL;
            ulSkip = 2;
        break;

        case 8:     // A NAME= (anchor name)
        {
            // this is variable in length and terminated with
            // another 0xFF char; we completely ignore this
            // here and just skip the anchor name, this is
            // only used with TXM_JUMPTOANCHORNAME, which then
            // searches the buffer
            PSZ pEnd;
            if (pEnd = strchr((*ppCurrent) + 2, 0xFF))
            {
                ulSkip = pEnd - *ppCurrent + 1;
                // store this with the other words so we can
                // find this word later
                fCreateWord = TRUE;
                // and store this with the rectangles
                fPaintEscapeWord = TRUE;
            }
        }
        break;

        case 0x10:  // relative point size in percent
            // three characters follow specifying the
            // percentage
            memcpy(szDecimal, (*ppCurrent) + 2, 3);
            szDecimal[3] = 0;
            lDecimal = atoi(szDecimal);

            pflbuf->lPointSize = pflbuf->pfmtc->lPointSize * lDecimal / 100;
            ulSkip = 5;
        break;

        case 0x20: // left margin changed:
            memcpy(szDecimal, (*ppCurrent) + 2, 4);   // four decimals xxxx
            szDecimal[4] = 0;
            lDecimal = atoi(szDecimal);

            // this is based on the current average font width, so
            // find this:
            pflbuf->fmtp.lLeftMargin = (lDecimal
                                        * pflbuf->lPointSize);
            ulSkip = 6;
            fCreateWord = TRUE;     // for rectangle correlation
        break;

        case 0x21: // first line margin changed:
            memcpy(szDecimal, (*ppCurrent) + 2, 4);   // +xxx, -xxx
            szDecimal[4] = 0;
            lDecimal = atoi(szDecimal);

            // this is based on the current average font width, so
            // find this:
            pflbuf->fmtp.lFirstLineMargin = (lDecimal
                                             * pflbuf->lPointSize);
            ulSkip = 6;
            fCreateWord = TRUE;     // for rectangle correlation
        break;

        case 0x22: // tab: forward current X to left margin
            pflbuf->lXCurrent = pflbuf->fmtp.lLeftMargin;

            ulSkip = 2;
            fCreateWord = TRUE;     // for rectangle correlation
        break;

        case 0x23: // marker: store this in output, this needs
                   // to be painted
            fCreateWord = TRUE;
            fPaintEscapeWord = TRUE;
            ulSkip = 3;
        break;

        case 0x30:  // spacing before paragraph:
            // four chars follow with either "####" or decimal spacing
            memcpy(szDecimal, (*ppCurrent) + 2, 4);
            szDecimal[4] = 0;
            if (memcmp(szDecimal, "####", 4) == 0)
                // reset to default:
                pflbuf->fmtp.lSpaceBefore = pxfd->fmtpStandard.lSpaceBefore;
            else
            {
                lDecimal = atoi(szDecimal);
                pflbuf->fmtp.lSpaceBefore = lDecimal;
            }
            ulSkip = 6;
            fCreateWord = TRUE;     // for rectangle correlation
        break;

        case 0x31:  // spacing before paragraph:
            // four chars follow with either "####" or decimal spacing
            memcpy(szDecimal, (*ppCurrent) + 2, 4);
            szDecimal[4] = 0;
            if (memcmp(szDecimal, "####", 4) == 0)
                // reset to default:
                pflbuf->fmtp.lSpaceAfter = pxfd->fmtpStandard.lSpaceAfter;
            else
            {
                lDecimal = atoi(szDecimal);
                pflbuf->fmtp.lSpaceAfter = lDecimal;
            }
            ulSkip = 6;
            fCreateWord = TRUE;     // for rectangle correlation
        break;

        case 0x32:  // word-wrapping:
            // here follows a single char being "0" or "1"
            if (    *((*ppCurrent) + 2) == '0')
                pflbuf->fmtp.fWordWrap = FALSE;
            else
                pflbuf->fmtp.fWordWrap = TRUE;
            fCreateWord = TRUE;     // for rectangle correlation
    }

    if (fCreateWord)        // append for rectangle correlation?
        if (!fWordsProcessed)    // are we processing words still (step 1)?
        {
            // yes: append to list for rectangle correlation later
            pEscapeWord = (PTXVWORD)malloc(sizeof(TXVWORD));
            memset(pEscapeWord, 0, sizeof(TXVWORD));
            // mark as escape sequence
            pEscapeWord->pStart = *ppCurrent;
            pEscapeWord->cChars = ulSkip;
            pEscapeWord->cEscapeCode = *(*ppCurrent + 1);
            pEscapeWord->fPaintEscapeWord = fPaintEscapeWord;
            pEscapeWord->pcszLinkTarget = pflbuf->pcszCurrentLinkTarget;
                    // V0.9.20 (2002-08-10) [umoeller]
                    // NULL if none
            if (fPaintEscapeWord)
            {
                pEscapeWord->lX = pflbuf->lXCurrent;
                pEscapeWord->lcid = pflbuf->pfmtf->lcid;
                pEscapeWord->lPointSize = pflbuf->lPointSize;
                pEscapeWord->flChar = pflbuf->flChar;
            }
            lstAppendItem(&pxfd->llWords, pEscapeWord);
        }

    if (!fWordsProcessed)
        // if we're still processing words, advance
        // current pointer by the escape length
        *ppCurrent += ulSkip;

    return pEscapeWord;
}

/*
 *@@ txvFormatText:
 *      this is the core function to text formatting, which
 *      must be done before the text can be painted into an
 *      HPS. See the top of textview.c for details.
 *
 *      Even though this function does not seem to have a
 *      lot of parameters, it is extremely powerful. This
 *      function handles paragraph and character formatting
 *      automatically. See XFMTPARAGRAPH and XFMTCHARACTER
 *      for possible formatting attributes, which are part
 *      of the XFORMATDATA structure passed to this function.
 *
 *      "Formatting" means splitting up any zero-terminated
 *      string (XFORMATDATA.pszViewText) into a possibly
 *      large list of TXVRECTANGLE structures, which each
 *      hold a rectangle to be painted. This allows for
 *      extremely fast painting.
 *
 *      Each TXVRECTANGLE in turn holds several "words" to
 *      be painted. A word consists of a TXVWORD structure
 *      and is normally a sequence of characters between
 *      spaces, \n and \r characters. As an exception, if
 *      escape sequences come up, such a "word" is split up
 *      into several words because character formatting
 *      (font, size, ...) is done on a per-word basis when painting.
 *
 *      This approach allows for quicker word-wrapping when only
 *      the output (paint) rectangle is changed because we don't
 *      have to re-calculate all the character widths (TXVWORD) once we
 *      got the words. Instead, when re-formatting, we just recompose
 *      the rectangles based on the words we calculated already.
 *      Of course, when character widths change (e.g. because
 *      fonts are changed), everything has to be redone.
 *
 *      Processing depends on the current formatting settings
 *      of the XFORMATDATA structure passed to this func and
 *      can become quite complicated:
 *
 *      --  In the simplest possible formatting mode, that is, if
 *          word wrapping is disabled, each such TXVRECTANGLE
 *          structure will hold one paragraph from the text
 *          (that is, the text between two \n chars).
 *
 *      --  If word wrapping is enabled, each paragraph in the text
 *          can consist of several such rectangles if the paragraph
 *          does not fit into one line. In that case, we create
 *          one XFMTRECTANGLE for each line which is needed to
 *          display the paragraph word-wrapped.
 *
 *      This uses an XFORMATDATA structure for input and output
 *      (besides the other parameters).
 *
 *      On input, specify the following:
 *
 *      -- hps: window or printer HPS. This is used for
 *         formatting only, but nothing is painted.
 *
 *      -- XFORMATDATA.pszViewText: the text to be formatted.
 *         This must follow certain conventions; the \xFF,
 *         \r, and \n characters have a special meaning.
 *         See the top of textview.c for details.
 *
 *      -- XFORMATDATA.fmtpStandard, fmtcStandard, fmtcCode:
 *         paragraph and character formatting attributes.
 *         For the simplest possible formatting, memset
 *         all these to 0. Word-wrapping depends on
 *         the paragraph formats.
 *
 *      -- prclView: rectangle for which formatting should take
 *         place. When this is called for a screen window,
 *         this should be the visible area of the window
 *         (WinQueryWindowRect).
 *         When this is called with a printer PS, this should
 *         be the size of a printer page.
 *
 *      This function updates the following:
 *
 *      -- XFORMATDATA.llWords: list of TXVWORD structures,
 *         holding all the "words" in the text as described
 *         above. This list can grow very long, but only needs
 *         to be recalculated when fonts change.
 *
 *      -- XFORMATDATA.llRectangles: list of TXVRECTANGLE
 *         structures, correlating the words on the words list
 *         to paint rectangles.
 *
 *      -- XFORMATDATA.szlWorkspace: total width
 *         and height of the "workspace", i.e. the total space
 *         needed to display the text (in pels). This might
 *         be smaller, the same, or larger than prclView,
 *         depending on whether the text fits into prclView.
 *
 *         When displaying text, you should display scroll bars
 *         if the workspace is larger than the window (prclView).
 *
 *         When printing, if the workspace is larger than the
 *         printer page (prclView), you will need to call
 *         txvPaintText several times for each page.
 *
 *      All coordinates are in world space (PU_PELS).
 *
 *@@changed V0.9.3 (2000-05-06) [umoeller]: largely rewritten; now handling paragraph and character formats
 *@@changed V1.0.18 (2008-11-16) [pr]: bodge formatting to remove unwanted scroll bars @@fixes 1086
 *@@todo TXVWORDF_GLUEWITHNEXT
 */

VOID txvFormatText(HPS hps,             // in: HPS whose font is used for
                                        // calculating text dimensions
                   PXFORMATDATA pxfd,   // in: formatting data
                   PRECTL prclView,     // in: rectangle to format for (window or printer page)
                   BOOL fFullRecalc)    // in: re-calculate word list too? (must be TRUE on the first call)
{
    /* ULONG   ulWinCX = (prclView->xRight - prclView->xLeft),
            ulWinCY = (prclView->yTop - prclView->yBottom); */

    lstClear(&pxfd->llRectangles);
    if (fFullRecalc)
        lstClear(&pxfd->llWords);

    pxfd->szlWorkspace.cx = 0;
    pxfd->szlWorkspace.cy = 0;

    if (pxfd->strViewText.cbAllocated)
    {
        ULONG   ulTextLen = pxfd->strViewText.ulLength;

        FORMATLINEBUF   flbuf;
        LONG            lcidLast = -99,
                        lPointSizeLast = -99;

        memset(&flbuf, 0, sizeof(flbuf));
        // copy default paragraph formatting
        memcpy(&flbuf.fmtp, &pxfd->fmtpStandard, sizeof(flbuf.fmtp));
        // set font
        flbuf.pfmtc = &pxfd->fmtcStandard;
        flbuf.lPointSize = pxfd->fmtcStandard.lPointSize;
        flbuf.pLastChar = pxfd->strViewText.psz + ulTextLen;

        if (ulTextLen)
        {
            ULONG   cWords = 0;

            if (fFullRecalc)
            {
                /*
                 * step 1: create words
                 *
                 */

                PSZ     pCurrent = pxfd->strViewText.psz;

                // loop until null terminator
                while (*pCurrent)
                {
                    PTXVWORD pWord;

                    if (flbuf.fBold)
                    {
                        if (flbuf.fItalics)
                            flbuf.pfmtf = &flbuf.pfmtc->fntBoldItalics;
                        else
                            flbuf.pfmtf = &flbuf.pfmtc->fntBold;
                    }
                    else
                        if (flbuf.fItalics)
                            flbuf.pfmtf = &flbuf.pfmtc->fntItalics;
                        else
                            flbuf.pfmtf = &flbuf.pfmtc->fntRegular;

                    // set font for subsequent calculations,
                    // if changed (this includes the first call)
                    if (lcidLast != flbuf.pfmtf->lcid)
                    {
                        GpiSetCharSet(hps, flbuf.pfmtf->lcid);
                        lcidLast = flbuf.pfmtf->lcid;
                        // force recalc of point size
                        lPointSizeLast = -99;
                    }

                    if (lPointSizeLast != flbuf.lPointSize)
                    {
                        if (flbuf.pfmtf->FontMetrics.fsDefn & FM_DEFN_OUTLINE)
                            // is outline font:
                            gpihSetPointSize(hps, flbuf.lPointSize);
                        lPointSizeLast = flbuf.lPointSize;
                    }

                    if (pWord = CreateWord(hps,
                                           &pCurrent, // advanced to next word
                                           &flbuf))
                    {
                        lstAppendItem(&pxfd->llWords, pWord);

                        /* {
                            CHAR szWord[3000];
                            strhncpy0(szWord, pWord->pStart, min(pWord->cChars, sizeof(szWord)));
                            _Pmpf(("Found word '%s'", szWord));
                        } */

                        cWords++;

                        while (*pCurrent == TXVESC_CHAR) // '\xFF')
                        {
                            // handle escapes;
                            // this advances pCurrent depending on the
                            // escape sequence length and might append
                            // another "word" for the escape sequence
                            // if it's relevant for rectangle correlation
                            ProcessEscapes(&pCurrent,
                                           pxfd,
                                           &flbuf,
                                           FALSE); // fWordsProcessed
                        }
                    }
                    else
                        break;
                }
            } // end if (fFullRecalc)
            else
                cWords = lstCountItems(&pxfd->llWords);

            /*
             * step 2: create rectangles
             *
             */

            if (cWords)
            {
                PLISTNODE pWordNode = lstQueryFirstNode(&pxfd->llWords);

                LONG        lCurrentYTop = prclView->yTop,
                            lOrigYTop = lCurrentYTop;

                BOOL        fRects2Go = TRUE;

                // space before paragraph; this is reset
                // to 0 if we start a new rectangle for
                // the same paragraph
                ULONG       ulYPre = flbuf.fmtp.lSpaceBefore;

                // rectangles loop
                while (fRects2Go)
                {
                    BOOL    fWords2Go = TRUE;
                    ULONG   ulWordsInThisRect = 0;

                    // maximum height of words in this rect
                    ULONG   lWordsMaxCY = 0;

                    // start a new rectangle:
                    PTXVRECTANGLE pRect = (PTXVRECTANGLE)malloc(sizeof(TXVRECTANGLE));
                    lstInit(&pRect->llWords,
                            FALSE);     // no auto-free; the words are stored in the main
                                        // list also, which is freed
                    // rectangle's xLeft;
                    // xRight will be set when we're done with this rectangle
                    pRect->rcl.xLeft = prclView->xLeft + flbuf.fmtp.lLeftMargin;
                    if (ulYPre)
                        // starting new paragraph:
                        // add first-line offset also
                        pRect->rcl.xLeft += flbuf.fmtp.lFirstLineMargin;

                    // current X pos: start with left of rectangle
                    flbuf.lXCurrent = pRect->rcl.xLeft;

                    // max baseline ofs: set to 0, this will be raised
                    pRect->ulMaxBaseLineOfs = 0;

                    // words-per-rectangle loop;
                    // we keep adding words to the rectangle until
                    // a) words no longer fit and word-wrapping is on;
                    // b) a newline or line feed is found;
                    // c) the last word has been reached;
                    while (fWords2Go)
                    {
                        PTXVWORD pWordThis = (PTXVWORD)pWordNode->pItemData;
/*
    #define TXVWORDF_GLUEWITHNEXT       1       // escape
    #define TXVWORDF_LINEBREAK          2       // \n
    #define TXVWORDF_LINEFEED           4       // \r
*/
                        BOOL    fNextWord = FALSE;

                        if (pWordThis->cEscapeCode)
                        {
                            // pseudo-word for escape sequence:
                            // process...
                            ProcessEscapes((PSZ*)&pWordThis->pStart,
                                           pxfd,
                                           &flbuf,
                                           TRUE);

                            // append this sequence only if it's needed
                            // for painting (list markers etc.)
                            if (pWordThis->fPaintEscapeWord)
                            {
                                pWordThis->lX = flbuf.lXCurrent;
                                pWordThis->pRectangle = pRect;
                                lstAppendItem(&pRect->llWords, pWordThis);
                                ulWordsInThisRect++;
                            }

                            fNextWord = TRUE;
                        }
                        else
                        {
                            BOOL    fWordWrapped = FALSE;

                            // not escape sequence, but real word: format...
                            // is word-wrapping on?
                            if (flbuf.fmtp.fWordWrap)
                            {
                                // yes: check if the word still fits
                                // WarpIN V1.0.18 @@todo add fudge factor of 2 - makes things work
                                if (    (flbuf.lXCurrent + pWordThis->ulCXWithSpaces + 2
                                             > prclView->xRight)
                                             // > ulWinCX)
                                        // but always add the first word in the rectangle,
                                        // because otherwise we get infinite loops
                                     && (ulWordsInThisRect > 0)
                                   )
                                    // no:
                                    fWordWrapped = TRUE;
                            }

                            if (fWordWrapped)
                                // start a new rectangle with the current word:
                                fWords2Go = FALSE;
                                        // and do _not_ advance to the next word,
                                        // but start with this word for the next
                                        // rectangle...
                            else
                            {
                                // add this word to the rectangle:

                                // store current X pos in word
                                pWordThis->lX = flbuf.lXCurrent;

                                // increase current X pos by word width
                                flbuf.lXCurrent += pWordThis->ulCXWithSpaces;

                                // store word in rectangle
                                pWordThis->pRectangle = pRect;
                                lstAppendItem(&pRect->llWords, pWordThis);
                                            // @@todo memory leak right here!!!
                                ulWordsInThisRect++;

                                // store highest word width found for this rect
                                if (pWordThis->ulCY > lWordsMaxCY)
                                    lWordsMaxCY = pWordThis->ulCY;

                                // store highest base line ofs found for this rect
                                if (pWordThis->ulBaseLineOfs > pRect->ulMaxBaseLineOfs)
                                    pRect->ulMaxBaseLineOfs = pWordThis->ulBaseLineOfs;

                                // go for next word in any case
                                fNextWord = TRUE;
                            } // end if (!fBreakThisWord)

                            // now check: add more words to this rectangle?
                            if (    (pWordThis->ulFlags == TXVWORDF_LINEBREAK)
                                        // no if linebreak found
                                 || (pWordThis->ulFlags == TXVWORDF_LINEFEED)
                                        // no if linefeed found
                                 || (!fWords2Go)
                                        // no if we're out of words or
                                        // word-break was forced
                               )
                            {
                                // no: finish up this rectangle...

                                // xLeft has been set on top
                                pRect->rcl.xRight = flbuf.lXCurrent;
                                pRect->rcl.yTop = lCurrentYTop - ulYPre;
                                pRect->rcl.yBottom = pRect->rcl.yTop - lWordsMaxCY;

                                // decrease current y top for next line
                                lCurrentYTop = pRect->rcl.yBottom;
                                if (!fRects2Go)
                                    // we're done completely:
                                    // add another one
                                    lCurrentYTop -= lWordsMaxCY;

                                if (fWordWrapped)
                                    // starting with wrapped word in next line:
                                    ulYPre = 0;
                                else
                                    if (pWordThis->ulFlags == TXVWORDF_LINEFEED)
                                        ulYPre = 0;
                                    else if (pWordThis->ulFlags == TXVWORDF_LINEBREAK)
                                    {
                                        // line break:
                                        // set y-pre for next loop
                                        ulYPre = flbuf.fmtp.lSpaceBefore;
                                        // and add paragraph post-y
                                        lCurrentYTop -= flbuf.fmtp.lSpaceAfter;
                                    }

                                // update x extents
                                if (pRect->rcl.xRight > pxfd->szlWorkspace.cx)
                                    pxfd->szlWorkspace.cx = pRect->rcl.xRight;

                                // and quit the inner loop
                                fWords2Go = FALSE;
                            } // end finish up rectangle
                        } // end else if (pWordThis->fIsEscapeSequence)

                        if (fNextWord)
                        {
                            pWordNode = pWordNode->pNext;
                            if (!pWordNode)
                            {
                                // no more to go:
                                // quit
                                fWords2Go = FALSE;
                                fRects2Go = FALSE;
                            }
                        }
                    } // end while (fWords2Go)

                    // store rectangle
                    lstAppendItem(&pxfd->llRectangles, pRect);
                }

                // lCurrentYTop now has the bottommost point we've used;
                // store this as workspace (this might be negative)
                pxfd->szlWorkspace.cy = lOrigYTop - lCurrentYTop;
            }
        }
    }
}

/* ******************************************************************
 *
 *   Device-independent text painting
 *
 ********************************************************************/

/*
 *@@ DrawListMarker:
 *
 *@@added V0.9.3 (2000-05-17) [umoeller]
 */

STATIC VOID DrawListMarker(HPS hps,
                           PRECTL prclLine,        // current line rectangle
                           PTXVWORD pWordThis,    // current word
                           LONG lViewXOfs)         // in: x offset to paint; 0 means rightmost
{
    POINTL ptl;

    ULONG ulBulletSize = pWordThis->lPointSize * 2 / 3; // 2/3 of point size

    ARCPARAMS   arcp = {1, 1, 0, 0};

    // pWordThis->pStart points to the \xFF character;
    // next is the "marker" escape (\x23),
    // next is the marker type
    CHAR cBulletType =  *((pWordThis->pStart) + 2) ;

    switch (cBulletType)
    {
        case 2:     // square (filled box)
            ptl.x = pWordThis->lX - lViewXOfs;
            // center bullet vertically
            ptl.y = prclLine->yBottom
                    +   (   (prclLine->yTop - prclLine->yBottom) // height
                             - ulBulletSize
                        ) / 2;

            GpiMove(hps, &ptl);
            ptl.x += ulBulletSize;
            ptl.y += ulBulletSize;
            GpiBox(hps, DRO_FILL, &ptl, 0, 0);
        break;

        default: // case 1:     // disc (filled circle)
            ptl.x = pWordThis->lX - lViewXOfs;
            // center bullet vertically;
            // the arc is drawn with the current position in its center
            ptl.y = prclLine->yBottom
                    +   (   (prclLine->yTop - prclLine->yBottom) // height
                            / 2
                        );

            GpiSetArcParams(hps, &arcp);
            GpiMove(hps, &ptl);
            GpiFullArc(hps,
                       (cBulletType == 3)
                            ? DRO_OUTLINE
                            : DRO_FILL,
                       MAKEFIXED(ulBulletSize / 2,      // radius!
                                 0));
        break;

    }
}

/*
 *@@ txvPaintText:
 *      device-independent function for painting.
 *      This can only be called after the text has
 *      been formatted (using txvFormatText).
 *
 *      This only paints rectangles which are within
 *      prcl2Paint.
 *
 *      --  For WM_PAINT, set this to the
 *          update rectangle, and set fPaintHalfLines
 *          to TRUE.
 *
 *      --  For printing, set this to the page rectangle,
 *          and set fPaintHalfLines to FALSE.
 *
 *      All coordinates are in world space (PU_PELS).
 *
 *@@changed V0.9.3 (2000-05-05) [umoeller]: fixed wrong visible lines calculations; great speedup painting!
 *@@changed V0.9.3 (2000-05-06) [umoeller]: now using gpihCharStringPosAt
 */

BOOL txvPaintText(HAB hab,
                  HPS hps,             // in: window or printer PS
                  PXFORMATDATA pxfd,
                  PRECTL prcl2Paint,   // in: invalid rectangle to be drawn,
                                       // can be NULL to paint all
                  LONG lViewXOfs,      // in: x offset to paint; 0 means rightmost
                  PLONG plViewYOfs,    // in: y offset to paint; 0 means _top_most;
                                       // out: y offset which should be passed to next call
                                       // (if TRUE is returned and fPaintHalfLines == FALSE)
                  BOOL fPaintHalfLines, // in: if FALSE, lines which do not fully fit on
                                       // the page are dropped (useful for printing)
                  PULONG pulLineIndex) // in: line to start painting with;
                                       // out: next line to paint, if any
                                       // (if TRUE is returned and fPaintHalfLines == FALSE)
{
    BOOL    brc = FALSE,
            fAnyLinesPainted = FALSE;
    ULONG   ulCurrentLineIndex = *pulLineIndex;
    // LONG    lViewYOfsSaved = *plViewYOfs;
    PLISTNODE pRectNode = lstNodeFromIndex(&pxfd->llRectangles,
                                           ulCurrentLineIndex);

    LONG    lcidLast = -99;
    LONG    lPointSizeLast = -99;

    while (pRectNode)
    {
        PTXVRECTANGLE   pLineRcl = (PTXVRECTANGLE)pRectNode->pItemData;
        BOOL            fPaintThis = FALSE;

        // compose rectangle to draw for this line
        RECTL           rclLine;
        rclLine.xLeft = pLineRcl->rcl.xLeft - lViewXOfs;
        rclLine.xRight = pLineRcl->rcl.xRight - lViewXOfs;
        rclLine.yBottom = pLineRcl->rcl.yBottom + *plViewYOfs;
        rclLine.yTop = pLineRcl->rcl.yTop + *plViewYOfs;

        /* if (pmpf)
        {
            CHAR szTemp[100];
            ULONG cb = min(pLineRcl->cLineChars, 99);
            strhncpy0(szTemp, pLineRcl->pStartOfLine, cb);

            _Pmpf(("Checking line %d: '%s'",
                    ulCurrentLineIndex,
                    szTemp));

            _Pmpf(("    (yB stored %d -> in HPS %d against win yB %d)",
                    pLineRcl->rcl.yBottom,
                    rclLine.yBottom,
                    prcl2Paint->yBottom));
        } */

        if (prcl2Paint == NULL)
            // draw all:
            fPaintThis = TRUE;
        else
        {
            BOOL fBottomInPaint = (    (rclLine.yBottom >= prcl2Paint->yBottom)
                                    && (rclLine.yBottom <= prcl2Paint->yTop)
                                  );
            BOOL fTopInPaint =    (    (rclLine.yTop >= prcl2Paint->yBottom)
                                    && (rclLine.yTop <= prcl2Paint->yTop)
                                  );

            if ((fBottomInPaint) && (fTopInPaint))
                // both in update rect:
                fPaintThis = TRUE;
            else
                if (fPaintHalfLines)
                {
                    if ((fBottomInPaint) || (fTopInPaint))
                    // only one in update rect:
                        fPaintThis = TRUE;
                    else
                        // now, for very small update rectangles,
                        // especially with slow scrolling,
                        // we can have the case that the paint rectangle
                        // is only a few pixels high so that the top of
                        // the line is above the repaint, and the bottom
                        // of the line is below it!
                        if (    (rclLine.yTop >= prcl2Paint->yTop)
                             && (rclLine.yBottom <= prcl2Paint->yBottom)
                           )
                            fPaintThis = TRUE;
                }
        }

        if (fPaintThis)
        {
            // rectangle invalid: paint this rectangle
            // by going thru the member words
            PLISTNODE   pWordNode = lstQueryFirstNode(&pLineRcl->llWords);

            POINTL ptlStart;

            while (pWordNode)
            {
                PTXVWORD pWordThis = (PTXVWORD)pWordNode->pItemData;
                ULONG flChar = pWordThis->flChar;

                if (pWordThis->pcszLinkTarget)       // V0.9.20 (2002-08-10) [umoeller]
                    flChar |= CHS_UNDERSCORE;

                // x start: this word's X coordinate
                ptlStart.x = pWordThis->lX - lViewXOfs;
                // y start: bottom line of rectangle plus highest
                // base line offset found in all words (format step 2)
                ptlStart.y = rclLine.yBottom + pLineRcl->ulMaxBaseLineOfs;
                         // pWordThis->ulBaseLineOfs;

                // set font for subsequent calculations,
                // if changed (this includes the first call)
                if (lcidLast != pWordThis->lcid)
                {
                    GpiSetCharSet(hps, pWordThis->lcid);
                    lcidLast = pWordThis->lcid;
                    // force recalc of point size
                    lPointSizeLast = -99;
                }

                if (lPointSizeLast != pWordThis->lPointSize)
                {
                    if (pWordThis->lPointSize)
                        // is outline font:
                        gpihSetPointSize(hps, pWordThis->lPointSize);
                    lPointSizeLast = pWordThis->lPointSize;
                }

                if (!pWordThis->cEscapeCode)
                    // regular word:
                    gpihCharStringPosAt(hps,
                                        &ptlStart,
                                        &rclLine,
                                        flChar,
                                        pWordThis->cChars,
                                        (PSZ)pWordThis->pStart);
                else
                {
                    // check escape code
                    switch (pWordThis->cEscapeCode)
                    {
                        case 0x23:
                            // escape to be painted:
                            DrawListMarker(hps,
                                           &rclLine,
                                           pWordThis,
                                           lViewXOfs);
                        break;
                    }
                }

                // ptlStart.x += pWordThis->ulCXWithSpaces;

                fAnyLinesPainted = TRUE;
                pWordNode = pWordNode->pNext;
            }

            /* {
                LONG lColor = GpiQueryColor(hps);
                POINTL ptl2;
                GpiSetColor(hps, RGBCOL_RED);
                ptl2.x = rclLine.xLeft;
                ptl2.y = rclLine.yBottom;
                GpiMove(hps, &ptl2);
                ptl2.x = rclLine.xRight;
                ptl2.y = rclLine.yTop;
                GpiBox(hps,
                       DRO_OUTLINE,
                       &ptl2,
                       0, 0);
                GpiSetColor(hps, lColor);
            } */

        }
        else
        {
            // this line is no longer fully visible:

            if (fAnyLinesPainted)
            {
                // we had painted lines already:
                // this means that all the following lines are
                // too far below the window, so quit
                /* if (pmpf)
                    _Pmpf(("Quitting with line %d (xL = %d yB = %d)",
                            ulCurrentLineIndex, rclLine.xLeft, rclLine.yBottom)); */

                *pulLineIndex = ulCurrentLineIndex;
                if (pRectNode->pNext)
                {
                    // another line to paint:
                    PTXVRECTANGLE   pLineRcl2 = (PTXVRECTANGLE)pRectNode->pNext->pItemData;
                    // return TRUE
                    brc = TRUE;
                    // and set *plViewYOfs to the top of
                    // the next line, which wasn't visible
                    // on the page any more
                    *plViewYOfs = pLineRcl2->rcl.yTop + *plViewYOfs;
                }
                break;
            }
            // else no lines painted yet:
            // go for next node, because we're still above the visible window
        }

        // next line
        pRectNode = pRectNode->pNext;
        // raise index to return
        ulCurrentLineIndex++;
    }

    if (!fAnyLinesPainted)
        brc = FALSE;

    return brc;
}

/*
 *@@ txvFindWordFromPoint:
 *      returns the list node of the word under the
 *      given point. The list node is from the global
 *      words list in pxfd.
 *
 *@@added V0.9.3 (2000-05-18) [umoeller]
 */

PLISTNODE txvFindWordFromPoint(PXFORMATDATA pxfd,
                               PPOINTL pptl)
{
    PLISTNODE pWordNodeFound = NULL;

    PLISTNODE pRectangleNode = lstQueryFirstNode(&pxfd->llRectangles);
    while ((pRectangleNode) && (!pWordNodeFound))
    {
        PTXVRECTANGLE prclThis = (PTXVRECTANGLE)pRectangleNode->pItemData;
        if (    (pptl->x >= prclThis->rcl.xLeft)
             && (pptl->x <= prclThis->rcl.xRight)
             && (pptl->y >= prclThis->rcl.yBottom)
             && (pptl->y <= prclThis->rcl.yTop)
           )
        {
            // cool, we found the rectangle:
            // now go thru the words in this rectangle
            PLISTNODE pWordNode = lstQueryFirstNode(&prclThis->llWords);
            while (pWordNode)
            {
                PTXVWORD pWordThis = (PTXVWORD)pWordNode->pItemData;

                if (    (pptl->x >= pWordThis->lX)
                     && (pptl->x <= pWordThis->lX + pWordThis->ulCXWithSpaces)
                   )
                {
                    pWordNodeFound = pWordNode;
                    break;
                }
                pWordNode = pWordNode->pNext;
            }
        }
        pRectangleNode = pRectangleNode->pNext;
    }

    return pWordNodeFound;
}

/*
 *@@ txvFindWordFromAnchor:
 *      returns the list node from the global words list
 *      BEFORE the word which represents the escape sequence
 *      containing the specified anchor name.
 *
 *@@added V0.9.4 (2000-06-12) [umoeller]
 */

PLISTNODE txvFindWordFromAnchor(PXFORMATDATA pxfd,
                                const char *pszAnchorName)
{
    PLISTNODE pNodeFound = NULL;

    ULONG cbAnchorName = strlen(pszAnchorName);

    PLISTNODE pWordNode = lstQueryFirstNode(&pxfd->llWords);
    while ((pWordNode) && (!pNodeFound))
    {
        PTXVWORD pWordThis = (PTXVWORD)pWordNode->pItemData;
        if (pWordThis->cEscapeCode == 7)
        {
            // this word is an anchor escape sequence:
            if (strnicmp(pszAnchorName, (pWordThis->pStart + 2), cbAnchorName) == 0)
            {
                // matches: check length
                if (*(pWordThis->pStart + 2 + cbAnchorName) == (char)0xFF)
                    // OK:
                    pNodeFound = pWordNode;
            }
        }

        pWordNode = pWordNode ->pNext;
    }

    if (pNodeFound)
    {
        // anchor found:
        // go backwords in word list until we find a "real" word
        // which is no escape sequence
        while (pNodeFound)
        {
            PTXVWORD pWordThis = (PTXVWORD)pNodeFound->pItemData;
            if (pWordThis->cEscapeCode)
                pNodeFound = pNodeFound->pPrevious;
            else
                break;
        }
    }

    return pNodeFound;
}

/* ******************************************************************
 *
 *   Window-dependent functions
 *
 ********************************************************************/

#define QWL_PRIVATE     4               // V0.9.20 (2002-08-10) [umoeller]

/*
 *@@ TEXTVIEWWINDATA:
 *      view control-internal structure, stored in
 *      QWL_PRIVATE at fnwpTextView.
 *      This is device-dependent on the text view
 *      window.
 */

typedef struct _TEXTVIEWWINDATA
{
    HAB     hab;                // anchor block (for speed)

    HDC     hdc;
    HPS     hps;

    ULONG   flStyle;            // window style flags copied on WM_CREATE
                                // V0.9.20 (2002-08-10) [umoeller]

    LONG    lBackColor,
            lForeColor;

    XTEXTVIEWCDATA cdata;       // control data, as passed to WM_CREATE

    XFORMATDATA xfd;

    SCROLLABLEWINDOW scrw;      // V1.0.1 (2003-01-25) [umoeller]

//     HWND    hwndVScroll,        // vertical scroll bar
//             hwndHScroll;        // horizontal scroll bar

    BOOL    fVScrollVisible,    // TRUE if vscroll is currently used
            fHScrollVisible;    // TRUE if hscroll is currently used

    RECTL   rclViewReal,        // window rect as returned by WinQueryWindowRect
                                // (top right point is inclusive!)
            rclViewPaint,       // same as rclViewReal, but excluding scroll bars
            rclViewText;        // same as rclViewPaint, but excluding cdata borders

//     LONG    lViewXOfs,          // pixels that we have scrolled to the RIGHT; 0 means very left
//             lViewYOfs;          // pixels that we have scrolled to the BOTTOM; 0 means very top

    BOOL    fAcceptsPresParamsNow; // TRUE after first WM_PAINT

    // anchor clicking
    PLISTNODE   pWordNodeFirstInAnchor;  // points to first word which belongs to anchor
    // USHORT      usLastAnchorClicked;    // last anchor which was clicked (1-0xFFFF)
    PCSZ        pcszLastLinkClicked;     // last link that was clicked (points into llLinks)
                                         // V0.9.20 (2002-08-10) [umoeller]

} TEXTVIEWWINDATA, *PTEXTVIEWWINDATA;

/*
 *@@ UpdateTextViewPresData:
 *      called from WM_CREATE and WM_PRESPARAMCHANGED
 *      in fnwpTextView to update the TEXTVIEWWINDATA
 *      from the window's presparams. This calls
 *      txvSetDefaultFormat in turn.
 */

STATIC VOID UpdateTextViewPresData(HWND hwndTextView,
                                   PTEXTVIEWWINDATA ptxvd)
{
    PSZ pszFont;
    ptxvd->lBackColor = winhQueryPresColor(hwndTextView,
                                           PP_BACKGROUNDCOLOR,
                                           FALSE,             // no inherit
                                           SYSCLR_DIALOGBACKGROUND);
    ptxvd->lForeColor = winhQueryPresColor(hwndTextView,
                                           PP_FOREGROUNDCOLOR,
                                           FALSE,             // no inherit
                                           SYSCLR_WINDOWSTATICTEXT);

    if ((pszFont = winhQueryWindowFont(hwndTextView)))
    {
        ULONG ulSize;
        PSZ pszFaceName;
        // _Pmpf(("font: %s", pszFont));
        if (gpihSplitPresFont(pszFont,
                              &ulSize,
                              &pszFaceName))
        {
            SetFormatFont(ptxvd->hps,
                             &ptxvd->xfd.fmtcStandard,
                             ulSize,
                             pszFaceName);
        }
        free(pszFont);
    }
}

/*
 *@@ AdjustViewRects:
 *      updates the internal size-dependent structures
 *      and positions the scroll bars.
 *
 *      This is device-dependent for the text view
 *      control and must be called before FormatText2Screen
 *      so that the view rectangles get calculated right.
 *
 *      Required input in TEXTVIEWWINDATA:
 *
 *      -- rclViewReal: the actual window dimensions.
 *
 *      -- cdata: control data.
 *
 *      Output from this function in TEXTVIEWWINDATA:
 *
 *      -- rclViewPaint: the paint subrectangle (which
 *         is rclViewReal minus scrollbars, if any).
 *
 *      -- rclViewText: the text subrectangle (which
 *         is rclViewPaint minus borders).
 *
 *@@changed WarpIN V1.0.18 (2008-11-16) [pr]: fix cut/paste/typo. errors @@fixes 1086
 */

STATIC VOID AdjustViewRects(HWND hwndTextView,
                            PTEXTVIEWWINDATA ptxvd)
{
    ULONG ulScrollCX = WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL),
          ulScrollCY = WinQuerySysValue(HWND_DESKTOP, SV_CYHSCROLL),
          ulOfs;

    // calculate rclViewPaint:
    // 1) left
    ptxvd->rclViewPaint.xLeft = ptxvd->rclViewReal.xLeft;
    // 2) bottom
    ptxvd->rclViewPaint.yBottom = ptxvd->rclViewReal.yBottom;
    if (ptxvd->fHScrollVisible)
        // if we have a horizontal scroll bar at the bottom,
        // raise bottom by its height
        ptxvd->rclViewPaint.yBottom += ulScrollCY;
    // 3) right
    ptxvd->rclViewPaint.xRight = ptxvd->rclViewReal.xRight;
    if (ptxvd->fVScrollVisible)
        // if we have a vertical scroll bar at the right,
        // subtract its width from the right
        ptxvd->rclViewPaint.xRight -= ulScrollCX;
    ptxvd->rclViewPaint.yTop = ptxvd->rclViewReal.yTop;

    // calculate rclViewText from that
    ptxvd->rclViewText.xLeft = ptxvd->rclViewPaint.xLeft + ptxvd->cdata.ulXBorder;
    ptxvd->rclViewText.yBottom = ptxvd->rclViewPaint.yBottom + ptxvd->cdata.ulYBorder;
    ptxvd->rclViewText.xRight = ptxvd->rclViewPaint.xRight - ptxvd->cdata.ulXBorder;
    ptxvd->rclViewText.yTop = ptxvd->rclViewPaint.yTop - ptxvd->cdata.ulYBorder;  // WarpIN V1.0.18

    // now reposition scroll bars; their sizes may change
    // if either the vertical or horizontal scroll bar has
    // popped up or been hidden
    if (ptxvd->flStyle & XS_VSCROLL)
    {
        // vertical scroll bar enabled:
        ulOfs = 0;
        if (ptxvd->fHScrollVisible)
            ulOfs = ulScrollCY;  // WarpIN V1.0.18
        WinSetWindowPos(ptxvd->scrw.hwndVScroll,
                        HWND_TOP,
                        ptxvd->rclViewReal.xRight - ulScrollCX,
                        ulOfs,          // y
                        ulScrollCX,     // cx
                        ptxvd->rclViewReal.yTop - ulOfs,     // cy
                        SWP_MOVE | SWP_SIZE);
    }

    if (ptxvd->flStyle & XS_HSCROLL)
    {
        ulOfs = 0;
        if (ptxvd->fVScrollVisible)
            ulOfs = ulScrollCX;
        WinSetWindowPos(ptxvd->scrw.hwndHScroll,
                        HWND_TOP,
                        0,
                        0,
                        ptxvd->rclViewReal.xRight - ulOfs,     // cx
                        ulScrollCY, // cy
                        SWP_MOVE | SWP_SIZE);
    }
}

/*
 *@@ FormatText2Screen:
 *      device-dependent version of text formatting
 *      for the text view window. This calls txvFormatText
 *      in turn and updates the view's scroll bars.
 *
 *@@changed V0.9.3 (2000-05-05) [umoeller]: fixed buggy vertical scroll bars
 *@@changed WarpIN V1.0.18 (2008-11-16) [pr]: fix buggy horiz. scroll bars @@fixes 1086
 */

STATIC VOID FormatText2Screen(HWND hwndTextView,
                              PTEXTVIEWWINDATA ptxvd,
                              BOOL fAlreadyRecursing,  // in: set this to FALSE when calling
                              BOOL fFullRecalc)
{
    ULONG   ulWinCX,
            ulWinCY;

    // call device-independent formatter with the
    // window presentation space
    txvFormatText(ptxvd->hps,
                  &ptxvd->xfd,
                  &ptxvd->rclViewText,
                  fFullRecalc);

    ulWinCY = (ptxvd->rclViewText.yTop - ptxvd->rclViewText.yBottom);

    if (ptxvd->scrw.ptlScrollOfs.y < 0)
        ptxvd->scrw.ptlScrollOfs.y = 0;
    if (ptxvd->scrw.ptlScrollOfs.y > ((LONG)ptxvd->xfd.szlWorkspace.cy - ulWinCY))
        ptxvd->scrw.ptlScrollOfs.y = (LONG)ptxvd->xfd.szlWorkspace.cy - ulWinCY;

    // vertical scroll bar enabled at all?
    if (ptxvd->flStyle & XS_VSCROLL)
    {
        BOOL fEnabled = winhUpdateScrollBar(ptxvd->scrw.hwndVScroll,
                                            ulWinCY,
                                            ptxvd->xfd.szlWorkspace.cy,
                                            ptxvd->scrw.ptlScrollOfs.y,
                                            (ptxvd->flStyle & XS_AUTOVHIDE));
        // is auto-hide on?
        if (ptxvd->flStyle & XS_AUTOVHIDE)
        {
            // yes, auto-hide on: did visibility change?
            if (fEnabled != ptxvd->fVScrollVisible)
                // visibility changed:
                // if we're not already recursing,
                // force calling ourselves again
                if (!fAlreadyRecursing)
                {
                    ptxvd->fVScrollVisible = fEnabled;
                    AdjustViewRects(hwndTextView,
                                    ptxvd);
                    FormatText2Screen(hwndTextView,
                                      ptxvd,
                                      TRUE,   // fAlreadyRecursing
                                      FALSE);   // quick format
                }
        }
    }

    ulWinCX = (ptxvd->rclViewText.xRight - ptxvd->rclViewText.xLeft);

    // horizontal scroll bar enabled at all?
    if (ptxvd->flStyle & XS_HSCROLL)
    {
        BOOL fEnabled = winhUpdateScrollBar(ptxvd->scrw.hwndHScroll,
                                            ulWinCX,
                                            ptxvd->xfd.szlWorkspace.cx,
                                            ptxvd->scrw.ptlScrollOfs.x,
                                            (ptxvd->flStyle & XS_AUTOHHIDE));
        // is auto-hide on?
        if (ptxvd->flStyle & XS_AUTOHHIDE)
        {
            // yes, auto-hide on: did visibility change?
            if (fEnabled != ptxvd->fHScrollVisible)
                // visibility changed:
                // if we're not already recursing,
                // force calling ourselves again (at the bottom)
                if (!fAlreadyRecursing)
                {
                    ptxvd->fHScrollVisible = fEnabled;
                    AdjustViewRects(hwndTextView,
                                    ptxvd);
                    FormatText2Screen(hwndTextView,  // WarpIN V1.0.18
                                      ptxvd,
                                      TRUE,   // fAlreadyRecursing
                                      FALSE);   // quick format
                }
        }
    }

    WinInvalidateRect(hwndTextView, NULL, FALSE);
}

/*
 *@@ SetWindowText:
 *      implementation for WM_SETWINDOWPARAMS and
 *      also WM_CREATE to set the window text.
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 *@@changed WarpIN V1.0.18 (2008-11-29) [pr]
 */

VOID SetWindowText(HWND hwndTextView,
                   PTEXTVIEWWINDATA ptxvd,
                   PCSZ pcszText)
{
    if (pcszText && *pcszText)
    {
        PXSTRING pstr = &ptxvd->xfd.strViewText;
        PSZ p;

        // WarpIN V1.0.18
        xstrcpy(&ptxvd->xfd.strOrigText,
                pcszText,
                0);

        switch (ptxvd->flStyle & XS_FORMAT_MASK)
        {
            case XS_PLAINTEXT:          // 0x0100
                // WarpIN V1.0.18
                if (p = strdup(pcszText))
                {
                    PSZ p2;
                    for (p2 = p; p2 = strchr(p2, TXVESC_CHAR); *p2 = ' ');
                    txvStripLinefeeds(&p, 4);
                    xstrset(pstr, p);
                }
            break;

            case XS_HTML:               // 0x0200
                // WarpIN V1.0.18
                if (p = strdup(pcszText))
                {
                    PSZ p2;
                    for (p2 = p; p2 = strchr(p2, TXVESC_CHAR); *p2 = ' ');
                    txvConvertFromHTML(&p, NULL, NULL, NULL);
                    xstrset(pstr, p);
                    xstrConvertLineFormat(pstr,
                                          CRLF2LF);
                }
            break;

            default: // case XS_PREFORMATTED:       // 0x0000
                // no conversion (default)
                xstrcpy(pstr,
                        pcszText,
                        0);
            break;
        }

        // if the last character of the window text is not "\n",
        // add it explicitly here, or our lines processing
        // is being funny
        // V0.9.20 (2002-08-10) [umoeller]
        if (pstr->psz[pstr->ulLength - 1] != '\n')
            xstrcatc(pstr, '\n');

        ptxvd->scrw.ptlScrollOfs.x = 0;
        ptxvd->scrw.ptlScrollOfs.y = 0;
        AdjustViewRects(hwndTextView,
                        ptxvd);
        FormatText2Screen(hwndTextView,
                          ptxvd,
                          FALSE,
                          TRUE);        // full format
    }
}

/*
 *@@ PaintViewText2Screen:
 *      device-dependent version of text painting
 *      for the text view window. This calls txvPaintText
 *      in turn and updates the view's scroll bars.
 */

STATIC VOID PaintViewText2Screen(PTEXTVIEWWINDATA ptxvd,
                                 PRECTL prcl2Paint)  // in: invalid rectangle, can be NULL == paint all
{
    ULONG   ulLineIndex = 0;
    LONG    lYOfs = ptxvd->scrw.ptlScrollOfs.y;
    txvPaintText(ptxvd->hab,
                 ptxvd->hps,        // paint PS: screen
                 &ptxvd->xfd,       // formatting data
                 prcl2Paint,        // update rectangle given to us
                 ptxvd->scrw.ptlScrollOfs.x,  // current X scrolling offset
                 &lYOfs,            // current Y scrolling offset
                 TRUE,              // draw even partly visible lines
                 &ulLineIndex);
}

/*
 *@@ PaintViewFocus:
 *      paint a focus rectangle.
 */

STATIC VOID PaintViewFocus(HPS hps,
                           PTEXTVIEWWINDATA ptxvd,
                           BOOL fFocus)
{
    POINTL  ptl;
    HRGN    hrgn;
    GpiSetClipRegion(hps,
                     NULLHANDLE,
                     &hrgn);
    GpiSetColor(hps,
                (fFocus)
                    ? WinQuerySysColor(HWND_DESKTOP, SYSCLR_HILITEBACKGROUND, 0)
                    : ptxvd->lBackColor);
    GpiSetLineType(hps, LINETYPE_DOT);
    ptl.x = ptxvd->rclViewPaint.xLeft;
    ptl.y = ptxvd->rclViewPaint.yBottom;
    GpiMove(hps, &ptl);
    ptl.x = ptxvd->rclViewPaint.xRight - 1;
    ptl.y = ptxvd->rclViewPaint.yTop - 1;
    GpiBox(hps,
           DRO_OUTLINE,
           &ptl,
           0, 0);
}

/*
 *@@ RepaintWord:
 *
 *@@added V0.9.3 (2000-05-18) [umoeller]
 */

STATIC VOID RepaintWord(PTEXTVIEWWINDATA ptxvd,
                        PTXVWORD pWordThis,
                        LONG lColor)
{
    POINTL ptlStart;
    ULONG flChar = pWordThis->flChar;
    PTXVRECTANGLE pLineRcl = pWordThis->pRectangle;

    RECTL           rclLine;
    rclLine.xLeft = pLineRcl->rcl.xLeft - ptxvd->scrw.ptlScrollOfs.x;
    rclLine.xRight = pLineRcl->rcl.xRight - ptxvd->scrw.ptlScrollOfs.x;
    rclLine.yBottom = pLineRcl->rcl.yBottom + ptxvd->scrw.ptlScrollOfs.y;
    rclLine.yTop = pLineRcl->rcl.yTop + ptxvd->scrw.ptlScrollOfs.y;

    if (pWordThis->pcszLinkTarget)
        flChar |= CHS_UNDERSCORE;

    // x start: this word's X coordinate
    ptlStart.x = pWordThis->lX - ptxvd->scrw.ptlScrollOfs.x;
    // y start: bottom line of rectangle plus highest
    // base line offset found in all words (format step 2)
    ptlStart.y = rclLine.yBottom + pLineRcl->ulMaxBaseLineOfs;
             // pWordThis->ulBaseLineOfs;

    GpiSetCharSet(ptxvd->hps, pWordThis->lcid);
    if (pWordThis->lPointSize)
        // is outline font:
        gpihSetPointSize(ptxvd->hps, pWordThis->lPointSize);

    GpiSetColor(ptxvd->hps,
                lColor);

    if (!pWordThis->cEscapeCode)
    {
        gpihCharStringPosAt(ptxvd->hps,
                            &ptlStart,
                            &rclLine,
                            flChar,
                            pWordThis->cChars,
                            (PSZ)pWordThis->pStart);
    }
    else
        // escape to be painted:
        DrawListMarker(ptxvd->hps,
                       &rclLine,
                       pWordThis,
                       ptxvd->scrw.ptlScrollOfs.x);
}

/*
 *@@ RepaintAnchor:
 *
 *@@added V0.9.3 (2000-05-18) [umoeller]
 */

STATIC VOID RepaintAnchor(PTEXTVIEWWINDATA ptxvd,
                          LONG lColor)
{
    PLISTNODE   pNode = ptxvd->pWordNodeFirstInAnchor;
    PCSZ        pcszLinkTarget = NULL;
    while (pNode)
    {
        PTXVWORD pWordThis = (PTXVWORD)pNode->pItemData;
        if (!pcszLinkTarget)
            // first loop:
            pcszLinkTarget = pWordThis->pcszLinkTarget;
        else
            if (pWordThis->pcszLinkTarget != pcszLinkTarget)
                // first word with different anchor:
                break;

        RepaintWord(ptxvd,
                    pWordThis,
                    lColor);
        pNode = pNode->pNext;
    }
}

/*
 *@@ ProcessCreate:
 *      implementation for WM_CREATE in fnwpTextView.
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 */

STATIC MRESULT ProcessCreate(HWND hwndTextView, MPARAM mp1, MPARAM mp2)
{
    PXTEXTVIEWCDATA     pcd = (PXTEXTVIEWCDATA)mp1;
                // can be NULL
    PCREATESTRUCT       pcs = (PCREATESTRUCT)mp2;

    MRESULT             mrc = (MRESULT)TRUE;     // error
    PTEXTVIEWWINDATA    ptxvd;

    // allocate TEXTVIEWWINDATA for QWL_PRIVATE
    if (ptxvd = (PTEXTVIEWWINDATA)malloc(sizeof(TEXTVIEWWINDATA)))
    {
        SIZEL   szlPage = {0, 0};
        BOOL    fShow = FALSE;

        // query message queue
        HMQ hmq = WinQueryWindowULong(hwndTextView, QWL_HMQ);
        // get codepage of message queue
        ULONG ulCodepage = WinQueryCp(hmq);

        memset(ptxvd, 0, sizeof(TEXTVIEWWINDATA));
        WinSetWindowPtr(hwndTextView, QWL_PRIVATE, ptxvd);

        ptxvd->hab = WinQueryAnchorBlock(hwndTextView);

        ptxvd->hdc = WinOpenWindowDC(hwndTextView);
        ptxvd->hps = GpiCreatePS(ptxvd->hab,
                                 ptxvd->hdc,
                                 &szlPage, // use same page size as device
                                 PU_PELS | GPIT_MICRO | GPIA_ASSOC);

        // copy window style flags V0.9.20 (2002-08-10) [umoeller]
        ptxvd->flStyle = pcs->flStyle;

        gpihSwitchToRGB(ptxvd->hps);

        // set codepage; GPI defaults this to
        // the process codepage
        GpiSetCp(ptxvd->hps, ulCodepage);

        txvInitFormat(&ptxvd->xfd);

        // copy control data, if present
        if (pcd)
            memcpy(&ptxvd->cdata, pcd, pcd->cbData);

        // check values which might cause null divisions
        if (ptxvd->cdata.ulVScrollLineUnit == 0)
            ptxvd->cdata.ulVScrollLineUnit = 15;
        if (ptxvd->cdata.ulHScrollLineUnit == 0)
            ptxvd->cdata.ulHScrollLineUnit = 15;

        ptxvd->fAcceptsPresParamsNow = FALSE;

        // copy window dimensions from CREATESTRUCT
        ptxvd->rclViewReal.xLeft = 0;
        ptxvd->rclViewReal.yBottom = 0;
        ptxvd->rclViewReal.xRight = pcs->cx;
        ptxvd->rclViewReal.yTop = pcs->cy;

        winhCreateScroller(hwndTextView,
                           &ptxvd->scrw,
                           ID_VSCROLL,
                           ID_HSCROLL);

        fShow = ((ptxvd->flStyle & XS_VSCROLL) != 0);
        WinShowWindow(ptxvd->scrw.hwndVScroll, fShow);
        ptxvd->fVScrollVisible = fShow;

        fShow = ((ptxvd->flStyle & XS_HSCROLL) != 0);
        WinShowWindow(ptxvd->scrw.hwndHScroll, fShow);
        ptxvd->fHScrollVisible = fShow;

        if (ptxvd->flStyle & XS_WORDWRAP)
            // word-wrapping should be enabled from the start:
            // V0.9.20 (2002-08-10) [umoeller]
            ptxvd->xfd.fmtpStandard.fWordWrap = TRUE;

        // set "code" format
        SetFormatFont(ptxvd->hps,
                      &ptxvd->xfd.fmtcCode,
                      6,
                      "System VIO");

        // get colors from presparams/syscolors
        UpdateTextViewPresData(hwndTextView, ptxvd);

        AdjustViewRects(hwndTextView,
                        ptxvd);

        if (ptxvd->flStyle & XS_HTML)
        {
            // if we're operating in HTML mode, set a
            // different default paragraph format to
            // make things prettier
            // V0.9.20 (2002-08-10) [umoeller]
            ptxvd->xfd.fmtpStandard.lSpaceBefore = 5;
            ptxvd->xfd.fmtpStandard.lSpaceAfter = 5;
        }

        // setting the window text on window creation never
        // worked V0.9.20 (2002-08-10) [umoeller]
        if (pcs->pszText)
            SetWindowText(hwndTextView,
                          ptxvd,
                          pcs->pszText);

        mrc = (MRESULT)FALSE;        // OK
    }

    return mrc;
}

/*
 *@@ ProcessPaint:
 *      implementation for WM_PAINT in fnwpTextView.
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 */

STATIC VOID ProcessPaint(HWND hwndTextView)
{
    PTEXTVIEWWINDATA    ptxvd;
    if (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
    {
        HRGN    hrgnOldClip;
        RECTL   rclClip;
        RECTL   rcl2Update;

        // get update rectangle
        WinQueryUpdateRect(hwndTextView,
                           &rcl2Update);
        // since we're not using WinBeginPaint,
        // we must validate the update region,
        // or we'll get bombed with WM_PAINT msgs
        WinValidateRect(hwndTextView,
                        NULL,
                        FALSE);

        // reset clip region to "all"
        GpiSetClipRegion(ptxvd->hps,
                         NULLHANDLE,
                         &hrgnOldClip);        // out: old clip region
        // reduce clip region to update rectangle
        GpiIntersectClipRectangle(ptxvd->hps,
                                  &rcl2Update);     // exclusive

        // draw little box at the bottom right
        // (in between scroll bars) if we have
        // both vertical and horizontal scroll bars
        if (    (ptxvd->flStyle & (XS_VSCROLL | XS_HSCROLL))
                == (XS_VSCROLL | XS_HSCROLL)
             && (ptxvd->fVScrollVisible)
             && (ptxvd->fHScrollVisible)
           )
        {
            RECTL   rclBox;
            rclBox.xLeft = ptxvd->rclViewPaint.xRight;
            rclBox.yBottom = 0;
            rclBox.xRight = rclBox.xLeft + WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL);
            rclBox.yTop = WinQuerySysValue(HWND_DESKTOP, SV_CYHSCROLL);
            WinFillRect(ptxvd->hps,
                        &rclBox,
                        WinQuerySysColor(HWND_DESKTOP,
                                         SYSCLR_DIALOGBACKGROUND,
                                         0));
        }

        // paint "view paint" rectangle white;
        // this can be larger than "view text"
        WinFillRect(ptxvd->hps,
                    &ptxvd->rclViewPaint,       // exclusive
                    ptxvd->lBackColor);

        // now reduce clipping rectangle to "view text" rectangle
        rclClip.xLeft = ptxvd->rclViewText.xLeft;
        rclClip.xRight = ptxvd->rclViewText.xRight - 1;
        rclClip.yBottom = ptxvd->rclViewText.yBottom;
        rclClip.yTop = ptxvd->rclViewText.yTop - 1;
        GpiIntersectClipRectangle(ptxvd->hps,
                                  &rclClip);    // exclusive
        // finally, draw text lines in invalid rectangle;
        // this subfunction is smart enough to redraw only
        // the lines which intersect with rcl2Update
        GpiSetColor(ptxvd->hps, ptxvd->lForeColor);
        PaintViewText2Screen(ptxvd,
                             &rcl2Update);

        if (    (!(ptxvd->flStyle & XS_STATIC))
                        // V0.9.20 (2002-08-10) [umoeller]
             && (WinQueryFocus(HWND_DESKTOP) == hwndTextView)
           )
        {
            // we have the focus:
            // reset clip region to "all"
            GpiSetClipRegion(ptxvd->hps,
                             NULLHANDLE,
                             &hrgnOldClip);        // out: old clip region
            PaintViewFocus(ptxvd->hps,
                           ptxvd,
                           TRUE);
        }

        ptxvd->fAcceptsPresParamsNow = TRUE;
    }
}

/*
 *@@ ProcessPresParamChanged:
 *      implementation for WM_PRESPARAMCHANGED in fnwpTextView.
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 */

STATIC VOID ProcessPresParamChanged(HWND hwndTextView, MPARAM mp1)
{
    PTEXTVIEWWINDATA    ptxvd;
    if (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
    {
        switch ((LONG)mp1)
        {
            case 0:     // layout palette thing dropped
            case PP_BACKGROUNDCOLOR:
            case PP_FOREGROUNDCOLOR:
            case PP_FONTNAMESIZE:
                // re-query our presparams
                UpdateTextViewPresData(hwndTextView, ptxvd);
        }

        if (ptxvd->fAcceptsPresParamsNow)
            FormatText2Screen(hwndTextView,
                              ptxvd,
                              FALSE,
                              TRUE);    // full reformat
    }
}

/*
 *@@ ProcessScroll:
 *
 *@@added V1.0.1 (2003-01-25) [umoeller]
 *@@changed WarpIN V1.0.18 (2008-11-16) [pr]: check for horiz. scroll message @@fixes 1086
 */

STATIC VOID ProcessScroll(HWND hwndTextView,
                          ULONG msg,
                          MPARAM mp2)
{
    PTEXTVIEWWINDATA ptxvd;
    if (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
    {
        POINTL  ptlScroll = {0, 0};
        if (    (msg == WM_VSCROLL)
             && (ptxvd->fVScrollVisible)
           )
        {
            LONG cy = ptxvd->rclViewText.yTop - ptxvd->rclViewText.yBottom;
            ptlScroll.y = winhHandleScrollMsg(ptxvd->scrw.hwndVScroll,
                                              &ptxvd->scrw.ptlScrollOfs.y,
                                              cy,
                                              ptxvd->xfd.szlWorkspace.cy,
                                              ptxvd->cdata.ulVScrollLineUnit,
                                              msg,
                                              mp2);
        }
        else if (    (msg == WM_HSCROLL)  // WarpIN V1.0.18
                  && (ptxvd->fHScrollVisible)
                )
        {
            LONG cx = ptxvd->rclViewText.xRight - ptxvd->rclViewText.xLeft;
            ptlScroll.x = winhHandleScrollMsg(ptxvd->scrw.hwndHScroll,
                                              &ptxvd->scrw.ptlScrollOfs.x,
                                              cx,
                                              ptxvd->xfd.szlWorkspace.cx,
                                              ptxvd->cdata.ulHScrollLineUnit,
                                              msg,
                                              mp2);
        }

        if (ptlScroll.x || ptlScroll.y)
            winhScrollWindow(hwndTextView,
                             &ptxvd->rclViewText,
                             &ptlScroll);
    }
}

/*
 *@@ ProcessSetFocus:
 *      implementation for WM_SETFOCUS in fnwpTextView.
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 */

STATIC VOID ProcessSetFocus(HWND hwndTextView, MPARAM mp2)
{
    PTEXTVIEWWINDATA    ptxvd;
    if (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
    {
        if (ptxvd->flStyle & XS_STATIC)
        {
            if (mp2)
            {
                // we're receiving the focus, but shouldn't have it:
                // then behave like the static control does, that is,
                // give focus to the next window in the dialog
                HWND    hwnd = hwndTextView,
                        hwndStart = hwnd;

                while (TRUE)
                {
                    ULONG flStyle;

                    if (!(hwnd = WinQueryWindow(hwnd, QW_NEXT)))
                        hwnd = WinQueryWindow(WinQueryWindow(hwndStart, QW_PARENT), QW_TOP);

                    // avoid endless looping
                    if (hwnd == hwndStart)
                    {
                        if (    (hwnd = WinQueryWindow(hwnd, QW_OWNER))
                             && (hwnd == hwndStart)
                           )
                            hwnd = NULLHANDLE;

                        break;
                    }

                    if (    (flStyle = WinQueryWindowULong(hwnd, QWL_STYLE))
                         && (flStyle & (WS_DISABLED | WS_TABSTOP | WS_VISIBLE)
                                == (WS_TABSTOP | WS_VISIBLE))
                       )
                    {
                        WinSetFocus(HWND_DESKTOP, hwnd);
                        break;
                    }
                };
            }
        }
        else
        {
            HPS hps = WinGetPS(hwndTextView);
            gpihSwitchToRGB(hps);
            PaintViewFocus(hps,
                           ptxvd,
                           (mp2 != 0));
            WinReleasePS(hps);
        }
    }
}

/*
 *@@ ProcessButton1Down:
 *      implementation for WM_BUTTON1DOWN in fnwpTextView.
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 */

STATIC MRESULT ProcessButton1Down(HWND hwndTextView, MPARAM mp1)
{
    MRESULT             mrc = 0;
    PTEXTVIEWWINDATA    ptxvd;

    if (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
    {
        POINTL ptlPos;
        PLISTNODE pWordNodeClicked;

        ptlPos.x = SHORT1FROMMP(mp1) + ptxvd->scrw.ptlScrollOfs.x;
        ptlPos.y = SHORT2FROMMP(mp1) - ptxvd->scrw.ptlScrollOfs.y;

        if (    (!(ptxvd->flStyle & XS_STATIC))
                        // V0.9.20 (2002-08-10) [umoeller]
             && (hwndTextView != WinQueryFocus(HWND_DESKTOP))
           )
            WinSetFocus(HWND_DESKTOP, hwndTextView);

        ptxvd->pcszLastLinkClicked = NULL;

        if (pWordNodeClicked = txvFindWordFromPoint(&ptxvd->xfd,
                                                    &ptlPos))
        {
            PTXVWORD pWordClicked = (PTXVWORD)pWordNodeClicked->pItemData;

            // store link target (can be NULL)
            if (ptxvd->pcszLastLinkClicked = pWordClicked->pcszLinkTarget)
            {
                // word has a link target:
                PLISTNODE   pNode = pWordNodeClicked;

                // reset first word of anchor
                ptxvd->pWordNodeFirstInAnchor = NULL;

                // go back to find the first word which has this anchor,
                // because we need to repaint them all
                while (pNode)
                {
                    PTXVWORD pWordThis = (PTXVWORD)pNode->pItemData;
                    if (pWordThis->pcszLinkTarget == pWordClicked->pcszLinkTarget)
                    {
                        // still has same anchor:
                        // go for previous
                        ptxvd->pWordNodeFirstInAnchor = pNode;
                        pNode = pNode->pPrevious;
                    }
                    else
                        // different anchor:
                        // pNodeFirst points to first node with same anchor now
                        break;
                }

                RepaintAnchor(ptxvd,
                              RGBCOL_RED);
            }
        }

        WinSetCapture(HWND_DESKTOP, hwndTextView);
        mrc = (MRESULT)TRUE;
    }

    return mrc;
}

/*
 *@@ ProcessButton1Up:
 *      implementation for WM_BUTTON1UP in fnwpTextView.
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 */

STATIC MRESULT ProcessButton1Up(HWND hwndTextView, MPARAM mp1)
{
    MRESULT             mrc = 0;
    PTEXTVIEWWINDATA    ptxvd;

    if (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
    {
        POINTL      ptlPos;
        HWND        hwndOwner = NULLHANDLE;

        ptlPos.x = SHORT1FROMMP(mp1) + ptxvd->scrw.ptlScrollOfs.x;
        ptlPos.y = SHORT2FROMMP(mp1) - ptxvd->scrw.ptlScrollOfs.y;
        WinSetCapture(HWND_DESKTOP, NULLHANDLE);

        if (ptxvd->pcszLastLinkClicked)
        {
            RepaintAnchor(ptxvd,
                          ptxvd->lForeColor);

            // nofify owner
            if (hwndOwner = WinQueryWindow(hwndTextView, QW_OWNER))
                WinPostMsg(hwndOwner,
                           WM_CONTROL,
                           MPFROM2SHORT(WinQueryWindowUShort(hwndTextView,
                                                             QWS_ID),
                                        TXVN_LINK),
                           (MPARAM)(ULONG)(ptxvd->pcszLastLinkClicked));
        }

        mrc = (MRESULT)TRUE;
    }

    return mrc;
}

/*
 *@@ ProcessChar:
 *      implementation for WM_CHAR in fnwpTextView.
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 *@@changed WarpIN V1.0.18 (2008-11-16) [pr]: added correct ID for horiz. scroll @@fixes 1086
 *@@changed WarpIN V1.0.18 (2008-11-16) [pr]: added Ctrl-Ins copy capability @@fixes 1116
 */

STATIC MRESULT ProcessChar(HWND hwndTextView, MPARAM mp1, MPARAM mp2)
{
    MRESULT             mrc = 0;
    PTEXTVIEWWINDATA    ptxvd;

    if (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
    {
        BOOL    fDefProc = TRUE;
        USHORT usFlags    = SHORT1FROMMP(mp1);
        // USHORT usch       = SHORT1FROMMP(mp2);
        USHORT usvk       = SHORT2FROMMP(mp2);

        if (usFlags & KC_VIRTUALKEY)
        {
            ULONG   ulMsg = 0;
            USHORT  usID = ID_VSCROLL;
            SHORT   sPos = 0;
            SHORT   usCmd = 0;
            fDefProc = FALSE;

            switch (usvk)
            {
                case VK_UP:
                    ulMsg = WM_VSCROLL;
                    usCmd = SB_LINEUP;
                break;

                case VK_DOWN:
                    ulMsg = WM_VSCROLL;
                    usCmd = SB_LINEDOWN;
                break;

                case VK_RIGHT:
                    ulMsg = WM_HSCROLL;
                    usCmd = SB_LINERIGHT;
                    usID = ID_HSCROLL;  // WarpIN V1.0.18
                break;

                case VK_LEFT:
                    ulMsg = WM_HSCROLL;
                    usCmd = SB_LINELEFT;
                    usID = ID_HSCROLL;  // WarpIN V1.0.18
                break;

                case VK_PAGEUP:
                    ulMsg = WM_VSCROLL;
                    if (usFlags & KC_CTRL)
                    {
                        sPos = 0;
                        usCmd = SB_SLIDERPOSITION;
                    }
                    else
                        usCmd = SB_PAGEUP;
                break;

                case VK_PAGEDOWN:
                    ulMsg = WM_VSCROLL;
                    if (usFlags & KC_CTRL)
                    {
                        sPos = ptxvd->xfd.szlWorkspace.cy;
                        usCmd = SB_SLIDERPOSITION;
                    }
                    else
                        usCmd = SB_PAGEDOWN;
                break;

                case VK_HOME:
                    if (usFlags & KC_CTRL)
                        // vertical:
                        ulMsg = WM_VSCROLL;
                    else
                    {
                        ulMsg = WM_HSCROLL;
                        usID = ID_HSCROLL;  // WarpIN V1.0.18
                    }

                    sPos = 0;
                    usCmd = SB_SLIDERPOSITION;
                break;

                case VK_END:
                    if (usFlags & KC_CTRL)
                    {
                        // vertical:
                        ulMsg = WM_VSCROLL;
                        sPos = ptxvd->xfd.szlWorkspace.cy;
                    }
                    else
                    {
                        ulMsg = WM_HSCROLL;
                        sPos = ptxvd->xfd.szlWorkspace.cx;
                        usID = ID_HSCROLL;  // WarpIN V1.0.18
                    }

                    usCmd = SB_SLIDERPOSITION;
                break;

                // WarpIN V1.0.18
                case VK_INSERT:
                    if (usFlags & KC_CTRL)
                    {
                        ulMsg = TXM_COPY;
                        usID = 0;
                    }
                break;

                default:
                    // other:
                    fDefProc = TRUE;
            }

            if (    ((usFlags & KC_KEYUP) == 0)
                 && (ulMsg)
               )
                WinSendMsg(hwndTextView,
                           ulMsg,
                           MPFROMSHORT(usID),
                           MPFROM2SHORT(sPos,
                                        usCmd));
        }

        if (fDefProc)
            mrc = WinDefWindowProc(hwndTextView, WM_CHAR, mp1, mp2);
                    // sends to owner
        else
            mrc = (MPARAM)TRUE;
    }

    return mrc;
}

/*
 *@@ ProcessJumpToAnchorName:
 *      implementation for TXM_JUMPTOANCHORNAME in fnwpTextView.
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 */

STATIC MRESULT ProcessJumpToAnchorName(HWND hwndTextView, MPARAM mp1)
{
    MRESULT             mrc = 0;
    PTEXTVIEWWINDATA    ptxvd;

    if (    (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
         && (mp1)
       )
    {
        PLISTNODE pWordNode;
        PTXVWORD pWord;
        if (    (pWordNode = txvFindWordFromAnchor(&ptxvd->xfd,
                                                   (const char*)mp1))
             && (pWord = (PTXVWORD)pWordNode->pItemData)
           )
        {
            // found:
            PTXVRECTANGLE pRect = pWord->pRectangle;
            ULONG ulWinCY = (ptxvd->rclViewText.yTop - ptxvd->rclViewText.yBottom);

            // now we need to scroll the window so that this rectangle is on top.
            // Since rectangles start out with the height of the window (e.g. +768)
            // and then have lower y coordinates down to way in the negatives,
            // to get the y offset, we must...
            ptxvd->scrw.ptlScrollOfs.y = (-pRect->rcl.yTop) - ulWinCY;

            if (ptxvd->scrw.ptlScrollOfs.y < 0)
                ptxvd->scrw.ptlScrollOfs.y = 0;
            if (ptxvd->scrw.ptlScrollOfs.y > ((LONG)ptxvd->xfd.szlWorkspace.cy - ulWinCY))
                ptxvd->scrw.ptlScrollOfs.y = (LONG)ptxvd->xfd.szlWorkspace.cy - ulWinCY;

            // vertical scroll bar enabled at all?
            if (ptxvd->flStyle & XS_VSCROLL)
            {
                /* BOOL fEnabled = */ winhUpdateScrollBar(ptxvd->scrw.hwndVScroll,
                                                    ulWinCY,
                                                    ptxvd->xfd.szlWorkspace.cy,
                                                    ptxvd->scrw.ptlScrollOfs.y,
                                                    (ptxvd->flStyle & XS_AUTOVHIDE));
                WinInvalidateRect(hwndTextView, NULL, FALSE);
            }

            mrc = (MRESULT)TRUE;
        }
    }

    return mrc;
}

/*
 *@@ ProcessDestroy:
 *      implementation for WM_DESTROY in fnwpTextView.
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 */

STATIC MRESULT ProcessDestroy(HWND hwndTextView, MPARAM mp1, MPARAM mp2)
{
    PTEXTVIEWWINDATA    ptxvd;

    if (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
    {
        xstrClear(&ptxvd->xfd.strViewText);
        xstrClear(&ptxvd->xfd.strOrigText);  // WarpIN V1.0.18
        lstClear(&ptxvd->xfd.llRectangles);
        lstClear(&ptxvd->xfd.llWords);
        GpiDestroyPS(ptxvd->hps);
        free(ptxvd);
        WinSetWindowPtr(hwndTextView, QWL_PRIVATE, NULL);
    }

    return WinDefWindowProc(hwndTextView, WM_DESTROY, mp1, mp2);
}

/*
 *@@ fnwpTextView:
 *      window procedure for the text view control. This is
 *      registered with the WC_XTEXTVIEW class in txvRegisterTextView.
 *
 *      The text view control is not a subclassed whatever control,
 *      but a control implemented from scratch. As a result, we
 *      had to implement all messages which are usually recognized
 *      by a control. In detail, we have:
 *
 *      -- WM_WINDOWPOSCHANGED: if the control is resized, the
 *         text is reformatted and the scroll bars are readjusted.
 *         See AdjustViewRects and txvFormatText.
 *
 *      -- WM_PRESPARAMCHANGED: if fonts or colors are dropped
 *         on the control, we reformat the text also.
 *
 *      -- WM_HSCROLL and WM_VSCROLL: this calls winhHandleScrollMsg
 *         to scroll the window contents.
 *
 *      -- WM_BUTTON1DOWN: this sets the focus to the control.
 *
 *      -- WM_SETFOCUS: if we receive the focus, we draw a fine
 *         dotted line in the "selection" color around the text
 *         window.
 *
 *      -- WM_CHAR: if we have the focus, the user can move the
 *         visible part within the workspace using the usual
 *         cursor and HOME/END keys.
 *
 *      -- WM_MOUSEMOVE: this sends WM_CONTROLPOINTER to the
 *         owner so the owner can change the mouse pointer.
 *
 *      <B>Painting</B>
 *
 *      The text view control creates a micro presentation space
 *      from the window's device context upon WM_CREATE, which is
 *      stored in TEXTVIEWWINDATA. We do not use WinBeginPaint in
 *      WM_PAINT, but only the PS we created ourselves. This saves
 *      us from resetting and researching all the fonts etc., which
 *      should be speedier.
 *
 *      The text view control uses a private window word for storing
 *      its own data. The client is free to use QWL_USER of the
 *      text view control.
 *
 *@@changed V0.9.3 (2000-05-05) [umoeller]: removed TXM_NEWTEXT; now supporting WinSetWindowText
 *@@changed V0.9.3 (2000-05-07) [umoeller]: crashed if create param was NULL; fixed
 *@@changed V0.9.20 (2002-08-10) [umoeller]: no longer using QWL_USER, which is free now
 *@@changed V0.9.20 (2002-08-10) [umoeller]: setting text on window creation never worked, fixed
 *@@changed V0.9.20 (2002-08-10) [umoeller]: added TXN_ANCHORCLICKED owner notify for anchors
 *@@changed V0.9.20 (2002-08-10) [umoeller]: converted private style flags to XS_* window style flags
 *@@changed V0.9.20 (2002-08-10) [umoeller]: added support for XS_STATIC
 *@@changed V0.9.20 (2002-08-10) [umoeller]: added support for formatting HTML and plain text automatically
 *@@changed V1.0.0 (2002-08-12) [umoeller]: optimized locality by moving big chunks into subfuncs
 *@@changed V1.0.1 (2003-01-25) [umoeller]: adjusted scroll msgs for new handler code
 *@@changed WarpIN V1.0.18 (2008-11-16) [pr]: added Ctrl-Ins copy capability @@fixes 1116
 *@@changed WarpIN V1.0.18 (2008-11-29) [pr]: added style set/query functions @@fixes 1116
 */

STATIC MRESULT EXPENTRY fnwpTextView(HWND hwndTextView, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT             mrc = 0;
    PTEXTVIEWWINDATA    ptxvd;

    switch (msg)
    {
        /*
         * WM_CREATE:
         *
         */

        case WM_CREATE:
            mrc = ProcessCreate(hwndTextView, mp1, mp2);
                    // extracted V1.0.0 (2002-08-12) [umoeller]
        break;

        /*
         * WM_SETWINDOWPARAMS:
         *      this message sets the window parameters,
         *      most importantly, the window text.
         *
         *      This updates the control.
         */

        case WM_SETWINDOWPARAMS:
            if (    (mp1)
                 && (((PWNDPARAMS)mp1)->fsStatus & WPM_TEXT)
                 && (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
               )
            {
                SetWindowText(hwndTextView,
                              ptxvd,
                              ((PWNDPARAMS)mp1)->pszText);
                mrc = (MRESULT)TRUE;     // was missing V0.9.20 (2002-08-10) [umoeller]
            }
        break;

        /*
         * WM_WINDOWPOSCHANGED:
         *
         */

        case WM_WINDOWPOSCHANGED:
            // resizing?
            if (    (mp1)
                 && (((PSWP)mp1)->fl & SWP_SIZE)
                 && (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
               )
            {
                WinQueryWindowRect(hwndTextView,
                                   &ptxvd->rclViewReal);
                AdjustViewRects(hwndTextView,
                                ptxvd);
                FormatText2Screen(hwndTextView,
                                  ptxvd,
                                  FALSE,
                                  FALSE);   // quick format
            }
        break;

        /*
         * WM_PAINT:
         *
         */

        case WM_PAINT:
            ProcessPaint(hwndTextView);
                    // extracted V1.0.0 (2002-08-12) [umoeller]
        break;

        /*
         * WM_PRESPARAMCHANGED:
         *
         *      Changing the color or font settings
         *      is equivalent to changing the default
         *      paragraph format. See TXM_SETFORMAT.
         */

        case WM_PRESPARAMCHANGED:
            ProcessPresParamChanged(hwndTextView, mp1);
        break;

        /*
         * WM_VSCROLL:
         *
         */

        case WM_VSCROLL:
        case WM_HSCROLL:
            ProcessScroll(hwndTextView, msg, mp2);
                    // V1.0.1 (2003-01-22) [umoeller]
        break;

        /*
         * WM_SETFOCUS:
         *
         */

        case WM_SETFOCUS:
            ProcessSetFocus(hwndTextView, mp2);
        break;

        /*
         * WM_MOUSEMOVE:
         *      send WM_CONTROLPOINTER to owner.
         */

        case WM_MOUSEMOVE:
        {
            HWND    hwndOwner;
            if (hwndOwner = WinQueryWindow(hwndTextView, QW_OWNER))
            {
                HPOINTER hptrSet
                    = (HPOINTER)WinSendMsg(hwndOwner,
                                           WM_CONTROLPOINTER,
                                           (MPARAM)(LONG)WinQueryWindowUShort(hwndTextView,
                                                                              QWS_ID),
                                           (MPARAM)WinQuerySysPointer(HWND_DESKTOP,
                                                                      SPTR_ARROW,
                                                                      FALSE));
                WinSetPointer(HWND_DESKTOP, hptrSet);
            }
        }
        break;

        /*
         * WM_BUTTON1DOWN:
         *
         */

        case WM_BUTTON1DOWN:
            mrc = ProcessButton1Down(hwndTextView, mp1);
        break;

        /*
         * WM_BUTTON1UP:
         *
         */

        case WM_BUTTON1UP:
            mrc = ProcessButton1Up(hwndTextView, mp1);
        break;

        /*
         * WM_CHAR:
         *
         */

        case WM_CHAR:
            mrc = ProcessChar(hwndTextView, mp1, mp2);
        break;

        /*
         *@@ TXM_QUERYPARFORMAT:
         *      this msg can be sent to the text view control
         *      to retrieve the paragraph format with the
         *      index specified in mp1.
         *
         *      This must be sent, not posted, to the control.
         *
         *      Parameters:
         *
         *      -- ULONG mp1: index of format to query.
         *              Must be 0 currently for the standard
         *              paragraph format.
         *
         *      -- PXFMTPARAGRAPH mp2: pointer to buffer
         *              which is to receive the formatting
         *              data.
         *
         *      Returns TRUE if copying was successful.
         *
         *@@added V0.9.3 (2000-05-06) [umoeller]
         */

        case TXM_QUERYPARFORMAT:
            if (    (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
                 && (!mp1)
                 && (mp2)
               )
            {
                memcpy(mp2,
                       &ptxvd->xfd.fmtpStandard,
                       sizeof(XFMTPARAGRAPH));
                mrc = (MPARAM)TRUE;
            }
        break;

        /*
         *@@ TXM_SETPARFORMAT:
         *      reverse to TXM_QUERYPARFORMAT, this sets a
         *      paragraph format (line spacings, margins
         *      and such).
         *
         *      This must be sent, not posted, to the control.
         *
         *      Parameters:
         *
         *      -- ULONG mp1: index of format to set.
         *              Must be 0 currently for the standard
         *              paragraph format.
         *
         *      -- PXFMTPARAGRAPH mp2: pointer to buffer
         *              from which to copy formatting data.
         *              If this pointer is NULL, the format
         *              is reset to the default.
         *
         *      This reformats the control.
         *
         *@@added V0.9.3 (2000-05-06) [umoeller]
         */

        case TXM_SETPARFORMAT:
            if (    (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
                 && (!mp1)
               )
            {
                if (mp2)
                    // copy:
                    memcpy(&ptxvd->xfd.fmtpStandard,
                           mp2,
                           sizeof(XFMTPARAGRAPH));
                else
                    // default:
                    memset(&ptxvd->xfd.fmtpStandard,
                           0,
                           sizeof(XFMTPARAGRAPH));

                FormatText2Screen(hwndTextView,
                                  ptxvd,
                                  FALSE,
                                  TRUE);    // full reformat

                mrc = (MPARAM)TRUE;
            }
        break;

        /*
         *@@ TXM_SETWORDWRAP:
         *      this text view control msg quickly changes
         *      the word-wrapping style of the default
         *      paragraph formatting.
         *
         *      This may be sent or posted.
         *
         *      (BOOL)mp1 determines whether word wrapping
         *      should be turned on or off.
         */

        case TXM_SETWORDWRAP:
            if (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
            {
                BOOL    ulOldFlFormat = ptxvd->xfd.fmtpStandard.fWordWrap;
                ptxvd->xfd.fmtpStandard.fWordWrap = (BOOL)mp1;
                if (ptxvd->xfd.fmtpStandard.fWordWrap != ulOldFlFormat)
                    FormatText2Screen(hwndTextView,
                                      ptxvd,
                                      FALSE,
                                      FALSE);       // quick format
            }
        break;

        /*
         *@@ TXM_QUERYCDATA:
         *      copies the current XTEXTVIEWCDATA
         *      into the specified buffer.
         *
         *      This must be sent, not posted, to the control.
         *
         *      Parameters:
         *
         *      --  PXTEXTVIEWCDATA mp1: target buffer.
         *          Before calling this, you MUST specify
         *          XTEXTVIEWCDATA.cbData.
         *
         *      Returns the bytes that were copied as
         *      a ULONG.
         */

        case TXM_QUERYCDATA:
            if (    (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
                 && (mp1)
               )
            {
                PXTEXTVIEWCDATA pTarget = (PXTEXTVIEWCDATA)mp1;
                mrc = (MRESULT)min(pTarget->cbData, sizeof(XTEXTVIEWCDATA));
                memcpy(pTarget,
                       &ptxvd->cdata,
                       (ULONG)mrc);
            }
        break;

        /*
         *@@ TXM_SETCDATA:
         *      updates the current XTEXTVIEWCDATA
         *      with the data from the specified buffer.
         *
         *      This must be sent, not posted, to the control.
         *
         *      Parameters:
         *
         *      -- PXTEXTVIEWCDATA mp1: source buffer.
         *         Before calling this, you MUST specify
         *         XTEXTVIEWCDATA.cbData.
         *
         *      Returns the bytes that were copied as
         *      a ULONG.
         *
         *@@changed V1.0.0 (2002-08-12) [umoeller]: now returning bytes
         */

        case TXM_SETCDATA:
            if (    (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
                 && (mp1)
               )
            {
                PXTEXTVIEWCDATA pSource = (PXTEXTVIEWCDATA)mp1;
                mrc = (MRESULT)min(pSource->cbData, sizeof(XTEXTVIEWCDATA));
                memcpy(&ptxvd->cdata,
                       pSource,
                       (ULONG)mrc);
            }
        break;

        /*
         *@@ TXM_JUMPTOANCHORNAME:
         *      scrolls the XTextView control contents so that
         *      the text marked with the specified anchor name
         *      (TXVESC_ANCHORNAME escape) appears at the top
         *      of the control.
         *
         *      This must be sent, not posted, to the control.
         *
         *      Parameters:
         *      -- PSZ mp1: anchor name (e.g. "anchor1").
         *
         *      Returns TRUE if the jump was successful.
         *
         *@@added V0.9.4 (2000-06-12) [umoeller]
         */

        case TXM_JUMPTOANCHORNAME:
            mrc = ProcessJumpToAnchorName(hwndTextView, mp1);
        break;

        /*
         *@@ TXM_QUERYTEXTEXTENT:
         *      returns the extents of the currently set text,
         *      that is, the width and height of the internal
         *      work area, of which the current view rectangle
         *      displays a subrectangle.
         *
         *      This must be sent, not posted, to the control.
         *
         *      Parameters:
         *
         *      --  PSIZEL mp1: pointer to a SIZEL buffer,
         *          which receives the extent in the cx and
         *          cy members. These will be set to null
         *          values if the control currently has no
         *          text.
         *
         *      Returns TRUE on success.
         *
         *@@added V0.9.20 (2002-08-10) [umoeller]
         */

        case TXM_QUERYTEXTEXTENT:
            if (    (mp1)
                 && (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
               )
            {
                memcpy((PSIZEL)mp1,
                       &ptxvd->xfd.szlWorkspace,
                       sizeof(SIZEL));
                mrc = (MRESULT)TRUE;
            }
        break;

        /*
         *@@ TXM_QUERYSTYLE:
         *      returns the current style flags.
         *
         *      This must be sent, not posted, to the control.
         *
         *      Parameters:
         *
         *      --  PULONG mp1: pointer to a ULONG buffer.
         *
         *      Returns TRUE on success.
         *
         *@@added WarpIN V1.0.18 (2008-11-29) [pr]
         */

        case TXM_QUERYSTYLE:
            if (    (mp1)
                 && (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
               )
            {
                *((ULONG *) mp1) = ptxvd->flStyle;
                mrc = (MRESULT)TRUE;
            }
        break;

        /*
         *@@ TXM_SETSTYLE:
         *      sets the current style flags.
         *
         *      This must be sent, not posted, to the control.
         *
         *      Parameters:
         *
         *      --  PULONG mp1: pointer to a ULONG buffer.
         *
         *      Returns TRUE on success.
         *
         *@@added WarpIN V1.0.18 (2008-11-29) [pr]
         */

        case TXM_SETSTYLE:
            if (    (mp1)
                 && (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
               )
            {
                ptxvd->flStyle = *((ULONG *) mp1);
                mrc = (MRESULT)TRUE;
            }
        break;

        /*
         *@@ TXM_COPY:
         *      copies the unprocessed window text to the clipboard.
         *
         *      Returns TRUE on success.
         *
         *@@added WarpIN V1.0.18 (2008-11-16) [pr]
         */

        case TXM_COPY:
            if (ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE))
            {
                mrc = (MRESULT) winhSetClipboardText(ptxvd->hab,
                                                     ptxvd->xfd.strOrigText.psz,
                                                     strlen(ptxvd->xfd.strOrigText.psz));
            }
        break;

        /*
         * WM_DESTROY:
         *      clean up.
         */

        case WM_DESTROY:
            mrc = ProcessDestroy(hwndTextView, mp1, mp2);
        break;

        default:
            mrc = WinDefWindowProc(hwndTextView, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ txvRegisterTextView:
 *      registers the Text View class with PM. Required
 *      before the text view control can be used.
 */

BOOL txvRegisterTextView(HAB hab)
{
    return WinRegisterClass(hab,
                            WC_XTEXTVIEW,
                            fnwpTextView,
                            0,
                            2 * sizeof(PVOID));     // QWL_USER and QWL_PRIVATE
}

/*
 *@@ txvReplaceWithTextView:
 *      replaces any window with a text view control.
 *      You must call txvRegisterTextView beforehand.
 *
 *@@added V0.9.1 (2000-02-13) [umoeller]
 */

HWND txvReplaceWithTextView(HWND hwndParentAndOwner,
                            USHORT usID,
                            ULONG flWinStyle,
                            USHORT usBorder)
{
    HWND hwndMLE = WinWindowFromID(hwndParentAndOwner, usID),
         hwndTextView = NULLHANDLE;
    if (hwndMLE)
    {
        ULONG           ul,
                        // attrFound,
                        abValue[32];
        SWP             swpMLE;
        XTEXTVIEWCDATA  xtxCData;
        PSZ             pszFont = winhQueryWindowFont(hwndMLE);
        LONG            lBackClr = -1,
                        lForeClr = -1;

        if ((ul = WinQueryPresParam(hwndMLE,
                                    PP_BACKGROUNDCOLOR,
                                    0,
                                    NULL,
                                    (ULONG)sizeof(abValue),
                                    (PVOID)&abValue,
                                    QPF_NOINHERIT)))
            lBackClr = abValue[0];

        if ((ul = WinQueryPresParam(hwndMLE,
                                    PP_FOREGROUNDCOLOR,
                                    0,
                                    NULL,
                                    (ULONG)sizeof(abValue),
                                    (PVOID)&abValue,
                                    QPF_NOINHERIT)))
            lForeClr = abValue[0];

        WinQueryWindowPos(hwndMLE, &swpMLE);

        WinDestroyWindow(hwndMLE);
        memset(&xtxCData, 0, sizeof(xtxCData));
        xtxCData.cbData = sizeof(xtxCData);
        xtxCData.ulXBorder = usBorder;
        xtxCData.ulYBorder = usBorder;
        hwndTextView = WinCreateWindow(hwndParentAndOwner,
                                       WC_XTEXTVIEW,
                                       "",
                                       flWinStyle,
                                       swpMLE.x,
                                       swpMLE.y,
                                       swpMLE.cx,
                                       swpMLE.cy,
                                       hwndParentAndOwner,
                                       HWND_TOP,
                                       usID,
                                       &xtxCData,
                                       0);
        if (pszFont)
        {
            winhSetWindowFont(hwndTextView, pszFont);
            free(pszFont);
        }

        if (lBackClr != -1)
            WinSetPresParam(hwndTextView,
                            PP_BACKGROUNDCOLOR,
                            sizeof(ULONG),
                            &lBackClr);
        if (lForeClr != -1)
            WinSetPresParam(hwndTextView,
                            PP_FOREGROUNDCOLOR,
                            sizeof(ULONG),
                            &lForeClr);
    }
    return hwndTextView;
}

/* ******************************************************************
 *
 *   Printer-dependent functions
 *
 ********************************************************************/

/*
 *@@ prthQueryQueues:
 *      returns a buffer containing all print queues
 *      on the system.
 *
 *      This is usually the first step before printing.
 *      After calling this function, show a dlg to the
 *      user, allow him to select the printer queue
 *      to be used. This can then be passed to
 *      prthCreatePrinterDC.
 *
 *      Use prthFreeBuf to free the returned buffer.
 */

STATIC PRQINFO3* prthEnumQueues(PULONG pulReturned)    // out: no. of queues found
{
    SPLERR  rc;
    ULONG   cTotal;
    ULONG   cbNeeded = 0;
    PRQINFO3 *pprq3 = NULL;

    // count queues & get number of bytes needed for buffer
    rc = SplEnumQueue(NULL,     // default computer
                      3,        // detail level
                      NULL,     // pbuf
                      0L,       // cbBuf
                      pulReturned, // out: entries returned
                      &cTotal,  // out: total entries available
                      &cbNeeded,
                      NULL);    // reserved

    if (!rc && cbNeeded)
    {
        pprq3 = (PRQINFO3*)malloc(cbNeeded);
        if (pprq3)
        {
            // enum the queues
            rc = SplEnumQueue(NULL,
                              3,
                              pprq3,
                              cbNeeded,
                              pulReturned,
                              &cTotal,
                              &cbNeeded,
                              NULL);
        }
    }

    return pprq3;
}

/*
 *@@ prthFreeBuf:
 *
 */

STATIC VOID prthFreeBuf(PVOID pprq3)
{
    if (pprq3)
        free(pprq3);
}

/*
 *@@ prthCreatePrinterDC:
 *      creates a device context for the printer
 *      specified by the given printer queue.
 *
 *      As a nifty feature, this returns printer
 *      device resolution automatically in the
 *      specified buffer.
 *
 *      Returns NULLHANDLE (== DEV_ERROR) on errors.
 *
 *      Use DevCloseDC to destroy the DC.
 *
 *      Based on print sample by Peter Fitzsimmons, Fri  95-09-29 02:47:16am.
 */

STATIC HDC prthCreatePrinterDC(HAB hab,
                               PRQINFO3 *pprq3,
                               PLONG palRes)  // out: 2 longs holding horizontal and vertical
                                              // printer resolution in pels per inch
{
    HDC     hdc = NULLHANDLE;
    DEVOPENSTRUC dos;
    PSZ     p;

    memset(&dos, 0, sizeof(dos));
    p = strrchr(pprq3->pszDriverName, '.');
    if (p)
        *p = 0;                         // del everything after '.'

    dos.pszLogAddress = pprq3->pszName;
    dos.pszDriverName = pprq3->pszDriverName;
    dos.pdriv = pprq3->pDriverData;
    dos.pszDataType = "PM_Q_STD";
    hdc = DevOpenDC(hab,
                    OD_QUEUED,
                    "*",
                    4L,     // count of items in next param
                    (PDEVOPENDATA)&dos,
                    0);     // compatible DC

    if (hdc)
        DevQueryCaps(hdc,
                     CAPS_HORIZONTAL_FONT_RES,
                     2,
                     palRes);   // buffer

    return hdc;
}

/*
 *@@ prthQueryForms:
 *      returns a buffer containing all forms
 *      supported by the specified printer DC.
 *
 *      Use prthFreeBuf to free the returned
 *      buffer.
 *
 *      HCINFO uses different model spaces for
 *      the returned info. See PMREF for details.
 */

STATIC HCINFO* prthQueryForms(HDC hdc,
                              PULONG pulCount)
{
    HCINFO  *pahci = NULL;

    LONG    cForms;

    // get form count
    cForms = DevQueryHardcopyCaps(hdc, 0L, 0L, NULL); // phci);
    if (cForms)
    {
        pahci = (HCINFO*)malloc(cForms * sizeof(HCINFO));
        if (pahci)
        {
            *pulCount = DevQueryHardcopyCaps(hdc, 0, cForms, pahci);
        }
    }

    return pahci;
}

/*
 *@@ prthCreatePS:
 *      creates a "normal" presentation space from the specified
 *      printer device context (which can be opened thru
 *      prthCreatePrinterDC).
 *
 *      Returns NULLHANDLE on errors.
 *
 *      Based on print sample by Peter Fitzsimmons, Fri  95-09-29 02:47:16am.
 */

STATIC HPS prthCreatePS(HAB hab,       // in: anchor block
                        HDC hdc,       // in: printer device context
                        ULONG ulUnits) // in: one of:
                                       // -- PU_PELS
                                       // -- PU_LOMETRIC
                                       // -- PU_HIMETRIC
                                       // -- PU_LOENGLISH
                                       // -- PU_HIENGLISH
                                       // -- PU_TWIPS
{
    SIZEL   sizel;

    sizel.cx = 0;
    sizel.cy = 0;
    return GpiCreatePS(hab,
                       hdc,
                       &sizel,
                       ulUnits | GPIA_ASSOC | GPIT_NORMAL);
}

/*
 *@@ prthStartDoc:
 *      calls DevEscape with DEVESC_STARTDOC.
 *      This must be called before any painting
 *      into the HDC's HPS. Any GPI calls made
 *      before this are ignored.
 *
 *      pszDocTitle appears in the spooler.
 */

STATIC VOID prthStartDoc(HDC hdc,
                         PSZ pszDocTitle)
{
    DevEscape(hdc,
              DEVESC_STARTDOC,
              strlen(pszDocTitle),
              pszDocTitle,
              0L,
              0L);
}

/*
 *@@ prthNextPage:
 *      calls DevEscape with DEVESC_NEWFRAME.
 *      Signals when an application has finished writing to a page and wants to
 *      start a new page. It is similar to GpiErase processing for a screen device
 *      context, and causes a reset of the attributes. This escape is used with a
 *      printer device to advance to a new page.
 */

STATIC VOID prthNextPage(HDC hdc)
{
    DevEscape(hdc,
              DEVESC_NEWFRAME,
              0,
              0,
              0,
              0);
}

/*
 *@@ prthEndDoc:
 *      calls DevEscape with DEVESC_ENDDOC
 *      and disassociates the HPS from the HDC.
 *      Call this right before doing
 +          GpiDestroyPS(hps);
 +          DevCloseDC(hdc);
 */

STATIC VOID prthEndDoc(HDC hdc,
                       HPS hps)
{
    DevEscape(hdc, DEVESC_ENDDOC, 0L, 0L, 0, NULL);
    GpiAssociate(hps, NULLHANDLE);
}

/*
 *@@ txvPrint:
 *      this does the actual printing.
 */

BOOL txvPrint(HAB hab,
              HDC hdc,          // in: printer device context
              HPS hps,          // in: printer presentation space (using PU_PELS)
              PSZ pszViewText,  // in: text to print
              ULONG ulSize,     // in: default font point size
              PSZ pszFaceName,  // in: default font face name
              HCINFO  *phci,    // in: hardcopy form to use
              PSZ pszDocTitle,  // in: document title (appears in spooler)
              FNPRINTCALLBACK *pfnCallback)
{
    RECTL       rclPageDevice,
                rclPageWorld;
    XFORMATDATA xfd;
    BOOL        fAnotherPage = FALSE;
    ULONG       ulCurrentLineIndex = 0,
                ulCurrentPage = 1;
    LONG        lCurrentYOfs = 0;

    /* MATRIXLF    matlf;
    POINTL      ptlCenter;
    FIXED       scalars[2]; */

    // important: we must do a STARTDOC before we use the printer HPS.
    prthStartDoc(hdc,
                 pszDocTitle);

    // the PS is in TWIPS, but our world coordinate
    // space is in pels, so we need to transform
    /* GpiQueryViewingTransformMatrix(hps,
                                   1L,
                                   &matlf);
    ptlCenter.x = 0;
    ptlCenter.y = 0;
    scalars[0] =  MAKEFIXED(2,0);
    scalars[1] =  MAKEFIXED(3,0);

    GpiScale (hps,
              &matlf,
              TRANSFORM_REPLACE,
              scalars,
              &ptlCenter); */

    // initialize format with font from window
    txvInitFormat(&xfd);

    /* SetFormatFont(hps,
                     &xfd,
                     ulSize,
                     pszFaceName); */

    // use text from window
    xstrcpy(&xfd.strViewText, pszViewText, 0);

    // setup page
    GpiQueryPageViewport(hps,
                         &rclPageDevice);
    // this is in device units; convert this
    // to the world coordinate space of the printer PS
    memcpy(&rclPageWorld, &rclPageDevice, sizeof(RECTL));
    GpiConvert(hps,
               CVTC_DEVICE,     // source
               CVTC_WORLD,
               2,               // 2 points, it's a rectangle
               (PPOINTL)&rclPageWorld);

    // left and bottom margins are in millimeters...
    /* rclPage.xLeft = 100;        // ###
    rclPage.yBottom = 100;
    rclPage.xRight = rclPage.xLeft + phci->xPels;
    rclPage.yTop = rclPage.yBottom + phci->yPels; */

    txvFormatText(hps,
                  &xfd,          // in: ptxvd->rclViewText
                  &rclPageWorld,
                  TRUE);

    do
    {
        _Pmpf(("---- printing page %d",
                ulCurrentPage));

        fAnotherPage = txvPaintText(hab,
                                    hps,
                                    &xfd,
                                    &rclPageWorld,
                                    0,
                                    &lCurrentYOfs,
                                    FALSE,      // draw only fully visible lines
                                    &ulCurrentLineIndex); // in/out: line to start with
        if (fAnotherPage)
        {
            prthNextPage(hdc);

            if (pfnCallback(ulCurrentPage++, 0) == FALSE)
                fAnotherPage = FALSE;
        }
    } while (fAnotherPage);

    prthEndDoc(hdc, hps);

    return TRUE;
}

/*
 *@@ txvPrintWindow:
 *      one-shot function which prints the contents
 *      of the specified XTextView control to the
 *      default printer, using the default form.
 *
 *      Returns a nonzero value upon errors.
 *
 *      Based on print sample by Peter Fitzsimmons, Fri  95-09-29 02:47:16am.
 */

int txvPrintWindow(HWND hwndTextView,
                   PSZ pszDocTitle,  // in: document title (appears in spooler)
                   FNPRINTCALLBACK *pfnCallback)
{
    int     irc = 0;

    PTEXTVIEWWINDATA ptxvd = (PTEXTVIEWWINDATA)WinQueryWindowPtr(hwndTextView, QWL_PRIVATE);

    if (!ptxvd)
        irc = 1;
    else
    {
        ULONG       cReturned = 0;
        PRQINFO3    *pprq3 = prthEnumQueues(&cReturned);
        HDC         hdc = NULLHANDLE;
        LONG        caps[2];

        // find default queue
        if (pprq3)
        {
            ULONG   i;
            // search for default queue;
            for (i = 0; i < cReturned; i++)
                if (pprq3[i].fsType & PRQ3_TYPE_APPDEFAULT)
                {
                    hdc = prthCreatePrinterDC(ptxvd->hab,
                                              &pprq3[i],
                                              caps);

                    break;
                }
            prthFreeBuf(pprq3);
        }

        if (!hdc)
            irc = 2;
        else
        {
            // OK, we got a printer DC:
            HPS     hps;
            ULONG   cForms = 0;
            HCINFO  *pahci,
                    *phciSelected = 0;

            // find default form
            pahci = prthQueryForms(hdc,
                                   &cForms);
            if (pahci)
            {
                HCINFO  *phciThis = pahci;
                ULONG   i;
                for (i = 0;
                     i < cForms;
                     i++, phciThis++)
                {
                    if (phciThis->flAttributes & HCAPS_CURRENT)
                    {
                        phciSelected = phciThis;
                    }
                }
            }

            if (!phciSelected)
                irc = 3;
            else
            {
                // create printer PS
                hps = prthCreatePS(ptxvd->hab,
                                   hdc,
                                   PU_PELS);

                if (hps == GPI_ERROR)
                    irc = 4;
                else
                {
                    PSZ pszFont;
                    ULONG ulSize = 0;
                    PSZ pszFaceName = 0;

                    if ((pszFont = winhQueryWindowFont(hwndTextView)))
                        gpihSplitPresFont(pszFont,
                                          &ulSize,
                                          &pszFaceName);
                    txvPrint(ptxvd->hab,
                             hdc,
                             hps,
                             ptxvd->xfd.strViewText.psz,
                             ulSize,
                             pszFaceName,
                             phciSelected,
                             pszDocTitle,
                             pfnCallback);

                    if (pszFont)
                        free(pszFont);

                    GpiDestroyPS(hps);
                }
            }
            DevCloseDC(hdc);
        }
    }

    return irc;
}


