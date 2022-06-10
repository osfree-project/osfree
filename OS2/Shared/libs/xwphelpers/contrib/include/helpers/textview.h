
/*
 *@@sourcefile textview.h:
 *      header file for textview.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 */

/*      Copyright (C) 2000-2008 Ulrich M”ller.
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
 *@@include #define INCL_SPL        // for printing functions
 *@@include #include <os2.h>
 *@@include #include "helpers\linklist.h"   // for device-independent functions
 *@@include #include "helpers\xstring.h"    // for device-independent functions
 *@@include #include "helpers\textview.h"
 */

#if __cplusplus
extern "C" {
#endif

#ifndef TEXTVIEW_HEADER_INCLUDED
    #define TEXTVIEW_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Escape sequences
     *
     ********************************************************************/

    /*
     *@@ TXVESC_CHAR:
     *      general escape character which introduces
     *      all escape sequences.
     */

    #define TXVESC_CHAR             '\xFF'
    #define TXVESC_STRING           "\xFF"

    // fixed formatting tags: PRE, B, I U, STRIKE
    // (no parameters)
    /* #define TXVESC_PRE_BEGIN        TXVESC_STRING "\x01\x01"
    #define TXVESC_PRE_END          TXVESC_STRING "\x01\x02" */

    #define TXVESC_SET_FONT         TXVESC_STRING "\x01"
                // here follow three characters with the
                // font index:
                // e.g. "000" == default font
                //      "001" == monospaced font
                // Fonts 0 and 1 always exist in the XTextView control.
                // Other fonts are presently not supported.
                // --> total size: 5

    #define TXVESC_BOLD_BEGIN       TXVESC_STRING "\x02\x01"
    #define TXVESC_BOLD_END         TXVESC_STRING "\x02\x02"

    #define TXVESC_ITALICS_BEGIN    TXVESC_STRING "\x03\x01"
    #define TXVESC_ITALICS_END      TXVESC_STRING "\x03\x02"

    #define TXVESC_UNDERLINE_BEGIN  TXVESC_STRING "\x04\x01"
    #define TXVESC_UNDERLINE_END    TXVESC_STRING "\x04\x02"

    #define TXVESC_STRIKE_BEGIN     TXVESC_STRING "\x05\x01"
    #define TXVESC_STRIKE_END       TXVESC_STRING "\x05\x02"

    // links
    #define TXVESC_LINK_BEGIN       TXVESC_STRING "\x06"
                // here follows the link reference, which
                // is variable in length and must be terminated
                // with another 0xFF escape code (NOT null byte);
                // --> total size: 2 plus link name length
                //      plus 1 for 0xFF terminator

    #define TXVESC_LINK_END         TXVESC_STRING "\x07"
                // end of the link
                // --> total size: 2

    #define TXVESC_ANCHORNAME       TXVESC_STRING "\x08"
                // here follows the anchor name, which is
                // variable in length and must be terminated
                // with another 0xFF escape code (NOT null byte);
                // --> total size: 2 plus anchor name length
                //      plus 1 for 0xFF terminator

    // character formatting with parameters:
    #define TXVESC_POINTSIZE_REL    TXVESC_STRING "\x10"
                // here follow three characters with a percentage
                // of the default point size
                // e.g. "050" == half the size
                //      "200" == double the size
                // --> total size: 5

    /*
     * paragraph formatting
     *      -- left and right margins:
     */

    #define TXVESC_LEFTMARGIN               TXVESC_STRING "\x20"
                // here follow four characters to specify the
                // new first line left margin (XFMTPARAGRAPH.lLeftMargin),
                // which is based on the average char width of
                // the current font:
                // e.g. "0020" == set margin to 20 average char widths
                // --> total size: 6

    #define TXVESC_FIRSTLINEMARGIN_LEFT     TXVESC_STRING "\x21-"
    #define TXVESC_FIRSTLINEMARGIN_RIGHT    TXVESC_STRING "\x21+"
                // here follow three characters to specify the
                // new absolute left margin (XFMTPARAGRAPH.lFirstLineMargin)
                // which is based on the average char width of
                // the current font:
                // e.g. "+020" == set first line margin to 20 average char widths
                // e.g. "-020" == set first line margin to 20 average char widths
                // --> total size: 6

    #define TXVESC_TAB                      TXVESC_STRING "\x22"
                // the tab can be used to move from the first line margin
                // to the left margin, if the first line margin is negative;
                // used with list items (UL LI and such)
                // --> total size: 2

    #define TXVESC_MARKER                   TXVESC_STRING "\x23"
                // draw marker at the current position: the next byte
                // determines the type of marker:
                // -- 1: disc (filled circle)
                // -- 2: square (filled box)
                // -- 3: circle (hollow circle)
                // --> total size: 3

    /*
     * paragraph formatting
     *      -- paragraph spacing:
     */

    #define TXVESC_SPACEBEFORE              TXVESC_STRING "\x30"
    #define TXVESC_SPACEAFTER               TXVESC_STRING "\x31"
                // here follow four characters to specify the
                // new spacing before or after a paragraph:
                // e.g. "0020" == twenty
                // !! special case: "####" means reset to default
                // --> total size: 6

    #define TXVESC_WORDWRAP                 TXVESC_STRING "\x32"
                // here follows a single character being "0" or "1"
                // --> total size: 3

    /* ******************************************************************
     *
     *   Device-independent functions
     *
     ********************************************************************/

    #define TXVWORDF_GLUEWITHNEXT       1       // escape
    #define TXVWORDF_LINEBREAK          2       // \n
    #define TXVWORDF_LINEFEED           4       // \r

    typedef struct _TXVRECTANGLE *PTXVRECTANGLE;

    /*
     *@@ TXVWORD:
     *      this defines a "word" to be drawn. A word is
     *      normally a sequence of characters between
     *      space, \n, or \r characters, unless escape
     *      sequences come up. See txvFormatText.
     *
     *@@added V0.9.3 (2000-05-14) [umoeller]
     */

    typedef struct _TXVWORD
    {
        const char  *pStart;        // ptr into TEXTVIEWWINDATA.pszViewText
        ULONG       cChars;         // characters in line to draw, starting
                                    // at pStartOfLine;
                                    // this might be less than the next \n
                                    // char if word-wrapping has been enabled
                                    // or an escape character was found

        CHAR        cEscapeCode;
                    // if 0, this word represents a substring to be painted
                    // ("normal word").
                    // If != 0; this "word" holds an escape sequence in *pStart
                    // which must be considered for paragraph formatting, but
                    // NOT be painted, except if fPaintEscapeWord is TRUE also.
                    // cEscapeCode then holds a copy of the escape character
                    // (which is pStart + 1).
        BOOL        fPaintEscapeWord;
                    // if this is TRUE, this escape is some special graphics
                    // to be painted also (e.g. list markers). txvPaint then
                    // evaluates the escape code (in pStart) to see what needs
                    // to be done.

        // all the following are only set if (fIsEscapeSequence == FALSE):

        ULONG       ulFlags;
                    // one of the following:
                    // -- TXVWORDF_GLUEWITHNEXT: this "word" was cut because
                    //          an escape sequence was found; this word must
                    //          therefore be in the same line with the next word
                    // -- TXVWORDF_LINEBREAK: this word ends with a \n char;
                    //          start new paragraph after this
                    // -- TXVWORDF_LINEFEED: this words ends with a \r char;
                    //          start new line after this

        ULONG       ulCXWithSpaces, // width of word (in pels), including trailing spaces
                    ulCXWord,       // width of word (in pels), without trailing spaces
                    ulCY;           // height of word (in pels)

        LONG        lX;             // X position to paint this word at;
                                    // this is (re)set during word-to-rectangle correlation!
        PTXVRECTANGLE pRectangle;   // reverse pointer to the TXVRECTANGLE structure to
                                    // which this word belongs; useful for repainting;
                                    // this is (re)set during word-to-rectangle correlation!
        ULONG       ulBaseLineOfs;  // base line offset; add this to rcl.yBottom
                                    // to get the the start point to be used for
                                    // GpiCharStringPosAt

        LONG        lcid;           // logical font ID to use for this rectangle
        LONG        lPointSize;     // point size to use for this rectangle;
                                    // this is set to 0 for bitmap fonts
        ULONG       flChar;         // flOptions parameter for GpiCharStringPosAt;
                                    // this will only have the CHS_UNDERSCORE or
                                    // CHS_STRIKEOUT flags set

        PCSZ        pcszLinkTarget; // if != NULL, pointer to a string in XFORMATDATA.llLinks;
                                    // the word is then part of a link
                                    // V0.9.20 (2002-08-10) [umoeller]

    } TXVWORD, *PTXVWORD;

    #ifdef LINKLIST_HEADER_INCLUDED
        /*
         *@@ TXVRECTANGLE:
         *      this defines one line to be drawn.
         *      This is device-independent.
         *      See txvFormatText.
         */

        typedef struct _TXVRECTANGLE
        {
            RECTL       rcl;            // rectangle of this line, into which
                                        // the words in llWords should be painted;
                                        // the bottom of this rectangle specifies the
                                        // bottommost point used (neccessary for
                                        // characters with underlengths, such as
                                        // "g" or "y"); ulMaxBaseLineOfs must be
                                        // added to the starting point
                                        // when using GpiCharStringPosAt
            ULONG       ulMaxBaseLineOfs; // highest ulBaseLineOfs of all words
                                        // belonging to this rectangle.
            LINKLIST    llWords;        // list of TXVWORD's which belong to this
                                        // rectangle. Note that these are duplicate
                                        // pointers to be able to correlate rectangles
                                        // to words; there is a second list of TXVWORD's
                                        // in XFORMATDATA which holds really all the
                                        // words which were created (for cleanup).
                                        // So one allocated TXVWORD structure can
                                        // have two pointers pointing to it.
        } TXVRECTANGLE;
    #endif

    /*
     *@@ XFMTFONT:
     *      specifies a font to be used in
     *      XFMTCHARACTER.
     *
     *@@added V0.9.3 (2000-05-06) [umoeller]
     */

    typedef struct _XFMTFONT
    {
        LONG        lcid;
                        // default font to use for this paragraph
        FONTMETRICS FontMetrics;
        LONG        alCX[256]; // width of each ASCII character for this
                               // string; computed by txvSetFormatFont
    } XFMTFONT, *PXFMTFONT;

    /*
     *@@ XFMTCHARACTER:
     *      character formatting parameters.
     *      This is used by the formatting engine
     *      (txvFormat)
     *      to describe character formatting.
     *      Character formatting can only be
     *      changed between words (TXVWORD).
     *
     *@@added V0.9.3 (2000-05-06) [umoeller]
     */

    typedef struct _XFMTCHARACTER
    {
        LONG        lPointSize;
                        // default font size to use for this paragraph
        XFMTFONT    fntRegular,
                    fntBold,
                    fntItalics,
                    fntBoldItalics;
    } XFMTCHARACTER, *PXFMTCHARACTER;

    /*
     *@@ XFMTPARAGRAPH:
     *      paragraph formatting parameters.
     *      This is used by the formatting engine
     *      (txvFormat)
     *      for every paragraph which is started.
     *
     *@@added V0.9.3 (2000-05-06) [umoeller]
     */

    typedef struct _XFMTPARAGRAPH
    {
        BOOL        fWordWrap;
        LONG        lLeftMargin,
                        // left indentation for all lines
                        // (in pixels)
                    lRightMargin,
                        // right indentation for all lines
                        // (in pixels)
                    lFirstLineMargin;
                        // additional indentation for first line only;
                        // this is added to lLeftMargin (can be negative)
                        // (in pixels)
        LONG        lSpaceBefore,
                        // space before each paragraph
                        // (in pixels)
                    lSpaceAfter;
                        // space after each paragraph
                        // (in pixels)
    } XFMTPARAGRAPH, *PXFMTPARAGRAPH;

    #ifdef LINKLIST_HEADER_INCLUDED
    #ifdef XSTRING_HEADER_INCLUDED

        /*
         *@@ XFORMATDATA:
         *      text formatting data. Passed to
         *      txvPaintText, stored in TEXTVIEWWINDATA.
         *      This is device-independent.
         */

        typedef struct _XFORMATDATA
        {
            XSTRING     strOrigText;    // original text from WinSetWindowText

            // input to txvFormatText
            XSTRING     strViewText;    // view text after conversion

            XFMTPARAGRAPH
                        fmtpStandard;   // standard paragraph format

            XFMTCHARACTER
                        fmtcStandard,   // format for standard text
                        fmtcCode;       // format for code text (monospaced; PRE, CODE etc.)

            // output from txvFormatText
            LINKLIST    llRectangles;
                                // list of TXVRECTANGLE, from top to bottom;
                                // text pointers point into pszViewText
            LINKLIST    llWords;
                                // list of TXVWORD's, in order of creation;
                                // this is just for proper cleanup. The items
                                // in the TXVRECTANGLE.llWords list also point
                                // to the words stored in this list.

            SIZEL       szlWorkspace;   // width and height of viewport (total text space)

            LINKLIST    llLinks;
                                // list of malloc'd PSZ's (auto-free) with all
                                // the links encountered while parsing the text.
                                // These are needed for passing the correct link
                                // target names when the user clicks on one.
                                // V0.9.20 (2002-08-10) [umoeller]

        } XFORMATDATA, *PXFORMATDATA;

        VOID txvInitFormat(PXFORMATDATA pxfd);

        VOID txvFormatText(HPS hps,
                           PXFORMATDATA pxfd,
                           PRECTL prclView,
                           BOOL fFullRecalc);
    #endif
    #endif

    VOID txvStripLinefeeds(char **ppszText,
                           ULONG ulTabSize);

    /* ******************************************************************
     *
     *   Window-dependent functions
     *
     ********************************************************************/

    /*
     * XTextView messages:
     *
     */

//  #define TXM_QUERYCHARFORMAT             (WM_USER + 1022)
//  #define TXM_SETCHARFORMAT               (WM_USER + 1023)
    #define TXM_QUERYPARFORMAT              (WM_USER + 1022)
    #define TXM_SETPARFORMAT                (WM_USER + 1023)
    #define TXM_SETWORDWRAP                 (WM_USER + 1024)
    #define TXM_QUERYCDATA                  (WM_USER + 1025)
    #define TXM_SETCDATA                    (WM_USER + 1026)
    #define TXM_JUMPTOANCHORNAME            (WM_USER + 1027)
    #define TXM_QUERYTEXTEXTENT             (WM_USER + 1028)
    #define TXM_QUERYSTYLE                  (WM_USER + 1029)
    #define TXM_SETSTYLE                    (WM_USER + 1030)
    #define TXM_COPY                        (WM_USER + 1031)

    #define WC_XTEXTVIEW     "XTextViewClass"

    /*
     * XTextView nofication codes:
     *
     */

    /*
     *@@ TXVN_LINK:
     *      WM_CONTROL notification code posted (!)
     *      to the XTextView control's owner.
     *
     *      Parameters:
     *
     *      --  USHORT SHORT1FROMMP(mp1): id of the control.
     *
     *      --  USHORT SHORT2FROMMP(mp1): nofify code (TXVN_LINK).
     *
     *      --  const char *mp2: target of the link that the
     *          user clicked on.
     *
     *@@added V0.9.3 (2000-05-18) [umoeller]
     */

    #define TXVN_LINK                       1

    /*
     * XTextView window style flags:
     *      all renamed, all turned into window style flags
     */

    #define XS_VSCROLL          0x0001      // show vertical scroll bar
    #define XS_HSCROLL          0x0002      // show horizontal scroll bar
    #define XS_AUTOVHIDE        0x0004      // with XS_VSCROLL: automatically hide scrollbar
    #define XS_AUTOHHIDE        0x0008      // with XS_HSCROLL: automatically hide scrollbar

    // handy macro V0.9.20 (2002-08-10) [umoeller]
    #define XS_FULLSCROLL       (XS_VSCROLL | XS_HSCROLL | XS_AUTOVHIDE | XS_AUTOHHIDE)

    #define XS_WORDWRAP         0x0010
                // enable word-wrapping in the default paragraph
                // format from the start

    #define XS_STATIC           0x0020
                // behave like static control: no focus, be skipped
                // over with tabbing in dialogs
                // V0.9.20 (2002-08-10) [umoeller]

    #define XS_FORMAT_MASK      0x0700
    #define XS_PREFORMATTED     0x0000      // plain text with \n only plus \xFF escape codes
                                            // (no conversion performed)
    #define XS_PLAINTEXT        0x0100      // plain text with \r and \xFF chars that need conversion
    #define XS_HTML             0x0200      // HTML

    /*
     *@@ XTEXTVIEWCDATA:
     *      control data structure for text view
     *      control. This can be passed to
     *      WinCreateWindow with the pCtlData
     *      parameter. You then MUST set the
     *      "cbData" field to sizeof(XTEXTVIEWCDATA).
     */

    typedef struct _XTEXTVIEWCDATA
    {
        USHORT      cbData;
        ULONG       ulXBorder,
                // space to leave on the left and right of text view;
                // defaults to 0
                    ulYBorder;
                // space to leave on the top and bottom of text view;
                // defaults to 0
        ULONG       ulVScrollLineUnit,
                // pixels to scroll if scroll bar "up" or "down" button
                // is pressed; defaults to 15
                    ulHScrollLineUnit;
                // pixels to scroll if scroll bar "left" or "right" button
                // is pressed; defaults to 15
    } XTEXTVIEWCDATA, *PXTEXTVIEWCDATA;


    BOOL txvRegisterTextView(HAB hab);

    HWND txvReplaceWithTextView(HWND hwndParentAndOwner,
                                USHORT usID,
                                ULONG flWinStyle,
                                USHORT usBorder);

    /* ******************************************************************
     *
     *   Printer-dependent functions
     *
     ********************************************************************/

    typedef BOOL APIENTRY FNPRINTCALLBACK(ULONG ulPage,
                                          ULONG ulUser);

    #ifdef INCL_SPL
        BOOL txvPrint(HAB hab,
                      HDC hdc,
                      HPS hps,
                      PSZ pszViewText,
                      ULONG ulSize,
                      PSZ pszFaceName,
                      HCINFO  *phci,
                      PSZ pszDocTitle,
                      FNPRINTCALLBACK *pfnCallback);
    #endif

    int txvPrintWindow(HWND hwndTextView,
                       PSZ pszDocTitle,
                       FNPRINTCALLBACK *pfnCallback);

#endif

#if __cplusplus
}
#endif

