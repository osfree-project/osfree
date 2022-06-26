
/*
 *@@sourcefile h2i.cpp:
 *      the one and only source file for h2i.exe.
 *
 *      h2i is a replacement for HTML2IPF.CMD and now used
 *      by the XWorkplace and WarpIN makefiles to convert
 *      the various HTML sources in 001\ to IPF code, which
 *      can then be fed into IPFC.
 *
 *      Compared to HTML2IPF.CMD, h2i has the following
 *      advantages:
 *
 *      --  Even though it's fairly sloppy code, it's
 *          MAGNITUDES faster.
 *
 *      --  It can process C include files to allow for
 *          character entity references. In those include
 *          files, only #define statements are evaluated.
 *          This can be used for string replacements.
 *
 *      --  It supports IFDEF and IFNDEF blocks like this:
 *
 +              <IFDEF some_c_definition>
 +                  Lots of text here, which can optionally
 +                  be ignored.
 +              </IFDEF>
 *
 *      --  It supports a RESID attribute to the HTML
 *          tag to allow for setting a resid explicitly.
 *          Together with the include facility, this
 *          is very useful for the XWorkplace help file.
 *
 *      --  This also fixes some minor formatting bugs
 *          that HTML2IPF.CMD exhibited, mostly with
 *          lists and PRE sections.
 *
 *      Current limitations:
 *
 *      --  This does not automatically convert GIFs or
 *          JPEGs to OS/2 1.3 BMP files, like HTML2IPF.CMD
 *          does.
 *
 *      --  <HTML SUBLINKS> and <HTML NOSUBLINKS> are not
 *          yet supported.
 *
 *@@header "h2i.h"
 *@@added V0.9.13 (2001-06-23) [umoeller]
 */

/*
 *      Copyright (C) 2001-2003 Ulrich M”ller.
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

#define OS2EMX_PLAIN_CHAR

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <io.h>

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include "setup.h"
#include "bldlevel.h"

#include "helpers\dosh.h"
#include "helpers\except.h"
#include "helpers\linklist.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\tree.h"
#include "helpers\xstring.h"

#include "h2i.h"

#pragma info (nocnv)

/* ******************************************************************
 *
 *   NLS strings
 *
 ********************************************************************/

PCSZ    G_pcszResourcesOnInternetTitle = "Resources on the Internet",
        G_pcszResourcesOnInternetBody =
            "This chapter contains all external links referenced in this book. "
            "Each link contained herein is an Unified Resource Locator (URL) "
            "to a certain location on the Internet. Simply double-click on one "
            "of them to launch Netscape with the respective URL.",
        G_pcszClickBelow =
            "Click below to launch Netscape with this URL&colon.";

/* ******************************************************************
 *
 *   Private declarations
 *
 ********************************************************************/

/*
 *@@ DEFINENODE:
 *      represents a #define from a C header.
 */

typedef struct _DEFINENODE
{
    TREE        Tree;
                        // Tree.ulKey has the (PSZ) identifier
    PSZ         pszValue;
                        // value without quotes
    ULONG       ulValueLength;
                        // strlen(pszValue)
} DEFINENODE, *PDEFINENODE;

/*
 *@@ ARTICLETREENODE:
 *      represents one article to be worked on.
 *
 *      This comes from one HTML file each and
 *      will turn into an IPF article.
 */

typedef struct _ARTICLETREENODE
{
    TREE        Tree;               // Tree.ulKey has the (PSZ) filename

    ULONG       ulHeaderLevel;      // IPF header level (1 if root file;
                                    // -1 if link for "Internet" section)

    ULONG       ulResID;            // the resid of this link

    BOOL        fProcessed;         // TRUE after this has been parsed;
                                    // if FALSE, everything below is
                                    // still undefined

    // after parsing, the following are valid:

    XSTRING     strIPF;             // mostly translated IPF source

    XSTRING     strTitle;

    struct _ARTICLETREENODE *pFirstReferencedFrom;

    // misc data from HTML source

    LONG        lGroup;
    PSZ         pszWidth,
                pszXPos;
    BOOL        fHidden;

    // sublinks, nosublinks

    BOOL        fWritten;          // TRUE after this article has been
                                   // written to the main buffer; this
                                   // is used when we sort out parent
                                   // articles so we won't write the
                                   // article twice

} ARTICLETREENODE, *PARTICLETREENODE;

#define LIST_UL             1
#define LIST_OL             2
#define LIST_DL             3

/*
 *@@ STATUS:
 *      parser status.
 */

typedef struct _STATUS
{
    // temp data for handlers
    PSZ         pSource;          // current source pointer;
                                  // with HandleTag:
                                  //    in: points to '<' char,
                                  //    out: should point to '>' char

    PSZ         pNextClose;       // ptr to next closing tag while in HandleTag

    // accumulated line length (for inserting line breaks automatically)
    ULONG       ulLineLength;

    // current state of formatting
    BOOL        fInHead,
                fItalics,
                fBold,
                fCode,
                fUnderlined,
                fInPre,
                fInTable,

                fJustHadSpace,
                fNeedsP;

    ULONG       ulInLink;           // 0: not in <A block
                                    // 1: in regular <A HREF= block, must be closed
                                    // 2: in special <A AUTO= block, must not be closed

    XSTRING     strLinkTag;         // if ulInLink > 0, current link tag (for
                                    // image link support); this has something
                                    // like
                                    // :link reftype=hd res=58.
                                    // V0.9.20 (2002-07-12) [umoeller]

    LINKLIST    llListStack;        // linklist abused as a stack for list tags;
                                    // the list item is simply a ULONG with a
                                    // LISTFL_* value

    ULONG       ulDefinition;       // if inside a <DL>, this is 1 if last item was
                                    // a DD

    BOOL        fFatal;             // if error is returned and this is TRUE,
                                    // processing is stopped; otherwise the
                                    // error is considered a warning only

    ULONG       ulNestingIFDEFs,
                ulNestingIFNDEFs;
} STATUS, *PSTATUS;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

TREE        *G_DefinesTreeRoot;
ULONG       G_cDefines = 0;
ULONG       G_ulReplacements = 0;
TREE        *G_LinkIDsTreeRoot;

LINKLIST    G_llFiles2Process;

ULONG       G_ulVerbosity = 1;

BOOL        G_fNoMoveToRoot = FALSE;

XSTRING     G_strError,               // string buffer for error msgs
            G_strCrashContext;

/* ******************************************************************
 *
 *   misc
 *
 ********************************************************************/

/*
 *@@ Error:
 *
 *      ulCategory is:
 *
 *      --  0: command line syntax error.
 *
 *      --  1: HTML syntax warning.
 *
 *      --  2: fatal error.
 */

VOID Error(ULONG ulCategory,
           const char *pcszFile,
           ULONG ulLine,
           const char *pcszFunction,
           const char *pcszFormat,     // in: format string (like with printf)
           ...)                        // in: additional stuff (like with printf)
{
    if (G_ulVerbosity <= 1)
        printf("\n");
    if (G_ulVerbosity)
        printf("h2i (%s line %u, %s)",
               pcszFile,
               ulLine,
               pcszFunction);
    else
        printf("h2i");

    if (pcszFormat)
    {
        va_list     args;
        switch (ulCategory)
        {
            case 0: printf(": Error in command line syntax. "); break;
            case 1: printf(" HTML warning: "); break;
            case 2: printf(" error: "); break;
        }
        va_start(args, pcszFormat);
        vprintf(pcszFormat, args);
        va_end(args);
    }

    if (ulCategory == 0)
        printf("\nType 'h2i -h' for help.\n");
    else
        printf("\n");
}

#define H2I_HEADER "h2i V"BLDLEVEL_VERSION" ("__DATE__") (C) 2001-2003 Ulrich M”ller"

/*
 * PrintHeader:
 *
 *
 */

VOID PrintHeader(VOID)
{
    printf(H2I_HEADER "\n");
    printf("  Part of the XWorkplace package.\n");
    printf("  This is free software under the GNU General Public Licence (GPL).\n");
    printf("  Refer to the COPYING file in the XWorkplace installation dir for details.\n");
}

/*
 *@@ Explain:
 *
 */

VOID Explain(const char *pcszFormat,     // in: format string (like with printf)
             ...)                        // in: additional stuff (like with printf)
{
    if (pcszFormat)
    {
        va_list     args;
        printf("h2i error: ");
        va_start(args, pcszFormat);
        vprintf(pcszFormat, args);
        va_end(args);
        printf("\n");
    }

    PrintHeader();
    printf("h2i (for 'html to ipf') translates a large bunch of HTML files into a\n");
    printf("single IPF file, which can then be fed into IBM's ipfc compiler.\n");
    printf("Usage: h2i [-i<include>]... [-v[0|1|2|3]] [-s] [-r] <root.htm>\n");
    printf("with:\n");
    printf("   <root.htm>  the HTML file to start with. This better link to other files.\n");
    printf("   -i<include> include a C header, whose #define statements are parsed\n");
    printf("               to allow for entity references (&ent;).\n");
    printf("   -v[0|1|2|3] specify verbosity level. If not specified, defaults to 1;\n");
    printf("               if -v only is specified, uses 2.\n");
    printf("   -s          show some statistics when done.\n");
    printf("   -r          show only <root.htm> at level 1 in the TOC; otherwise all\n");
    printf("               second-level files are moved to level 1 as well.\n");
}

/*
 *@@ fnCompareStrings:
 *      tree comparison func (src\helpers\tree.c).
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

int TREEENTRY fnCompareStrings(ULONG ul1, ULONG ul2)
{
    return (strhicmp((const char *)ul1,
                     (const char *)ul2));
}

/* ******************************************************************
 *
 *   Parser helpers
 *
 ********************************************************************/

/*
 *@@ AddDefinition:
 *
 */

BOOL AddDefinition(PDEFINENODE p)
{
    return (!treeInsert(&G_DefinesTreeRoot,
                        NULL,
                        (TREE*)p,
                        fnCompareStrings));
}

/*
 *@@ FindDefinition:
 *
 */

PDEFINENODE FindDefinition(const char *pcszIdentifier)
{
    return ((PDEFINENODE)treeFind(G_DefinesTreeRoot,
                                  (ULONG)pcszIdentifier,
                                  fnCompareStrings));
}

/*
 *@@ ResolveEntities:
 *
 */

APIRET ResolveEntities(PARTICLETREENODE pFile2Process,
                       PXSTRING pstrSource)
{
    if (G_cDefines)
    {
        // we have include files:
        PSZ p = pstrSource->psz;

        // extract the entity (between '&' and ';')
        while (p = strchr(p, '&'))
        {
            PSZ p2;
            if (!(p2 = strchr(p, ';')))
                p++;
            else
            {
                // find the definition for the entity in
                // the global list; so make this null-terminated
                // first
                *p2 = '\0';

                PDEFINENODE pDef = 0;
                if (pDef = FindDefinition(p + 1))
                {
                    if (G_ulVerbosity > 2)
                        printf("\n   found entity \"%s\" --> \"%s\"",
                               p + 1, pDef->pszValue);
                    *p2 = ';';

                    // replace the entity string with the
                    // definition string
                    ULONG ulPos = (p - pstrSource->psz),
                          ulReplLen = pDef->ulValueLength;
                    xstrrpl(pstrSource,
                            // first ofs to replace:
                            ulPos,
                            // char count to replace:
                            (p2 + 1) - p,       // include ';'
                            // replacement string:
                            pDef->pszValue,
                            ulReplLen);

                    // string buffer might have changed, so
                    // re-adjust source position
                    p = pstrSource->psz + ulPos;

                    G_ulReplacements++;
                }
                else
                {
                    // not found:
                    // if it's not one of the standards we'll
                    // replace later, complain
                    if (    (strcmp(p + 1, "gt"))
                         && (strcmp(p + 1, "lt"))
                         && (strcmp(p + 1, "amp"))
                       )
                    {
                        Error(1,
                              __FILE__, __LINE__, __FUNCTION__,
                              "Unknown entity \"%s;\" at line %d in file \"%s\".",
                              p,
                              strhCount(pstrSource->psz, '\n') + 1,
                              (PSZ)pFile2Process->Tree.ulKey);      // filename
                    }

                    *p2 = ';';

                    p = p2 + 1;
                }
            }
        }
    }
}

/*
 *@@ ConvertEscapes:
 *
 */

const char *ConvertEscapes(PXSTRING pstr)
{
    const char *papszSources[] =
        {
            "&amp;",
            "&lt;",
            "&gt;"
        };
    const char *papszTargets[] =
        {
            "&amp.",
            "<",
            ">"
        };

    ULONG ul;
    for (ul = 0;
         ul < ARRAYITEMCOUNT(papszSources);
         ul++)
    {
        ULONG ulOfs = 0;
        while (xstrFindReplaceC(pstr,
                                &ulOfs,
                                papszSources[ul],
                                papszTargets[ul]))
            ;
    }
}

/*
 *@@ GetOrCreateArticle:
 *      returns a link ID (resid) for the
 *      specified filename.
 *
 *      If this is called for the first time for
 *      this file name, a new ID is created and
 *      returned. Otherwise the existing one
 *      is returned.
 *
 *      Special flags for ulCurrentLevel:
 *
 *      --  0: create root article (will receive 1 then)
 *
 *      -- -1: do not create if it doesn't exist.
 *
 *      If a file is created, it will intially
 *      carry a resid of -1. If the file has a
 *      RESID attribute to the HTML tag, ParseFile
 *      will reset the resid then. Later we go
 *      through all files and check which ones
 *      are still -1 and assign random values then.
 */

PARTICLETREENODE GetOrCreateArticle(const char *pcszFilename,
                                    ULONG ulCurrentLevel,
                                    PARTICLETREENODE pParent)
{
    PARTICLETREENODE pMapping;

    // check if we have an article for this file name already
    if (pMapping = (PARTICLETREENODE)treeFind(G_LinkIDsTreeRoot,
                                              (ULONG)pcszFilename,
                                              fnCompareStrings))
        // exists:
        return (pMapping);
    else
        if (ulCurrentLevel != -1)
        {
            // create new one
            if (pMapping = NEW(ARTICLETREENODE))
            {
                ZERO(pMapping);

                pMapping->Tree.ulKey = (ULONG)strhdup(pcszFilename, NULL);
                pMapping->ulHeaderLevel = ulCurrentLevel + 1;
                pMapping->ulResID = -1;         // for now
                pMapping->pFirstReferencedFrom = pParent;

                xstrInit(&pMapping->strIPF, 0);
                xstrInit(&pMapping->strTitle, 0);

                if (!treeInsert(&G_LinkIDsTreeRoot,
                                NULL,
                                (TREE*)pMapping,
                                fnCompareStrings))
                {
                    // this is a new file...
                    // if it's not an HTML link, check if this exists
                    if (access(pcszFilename, 0))
                    {
                        // file does not exist:
                        if (    (strncmp(pcszFilename, "http://", 7))
                             && (strncmp(pcszFilename, "ftp://", 6))
                             && (strncmp(pcszFilename, "mailto:", 7))
                           )
                        {
                            if (G_ulVerbosity)
                                printf("\n  Warning: Link from \"%s\" to \"%s\" was not found.",
                                       (pParent)
                                           ? (PSZ)pParent->Tree.ulKey // pszFilename,
                                           : "none",
                                       (PSZ)pMapping->Tree.ulKey); // pszFilename,
                        }

                        // do not try to load this file!
                        pMapping->ulHeaderLevel = -1; // special flag
                    }

                    // since this is new, add it to the list of
                    // things to be processed too
                    lstAppendItem(&G_llFiles2Process,
                                  pMapping);

                    return (pMapping);
                }
            }
        }

    return (0);
}

/* ******************************************************************
 *
 *   Tag handlers
 *
 ********************************************************************/

/*
 *@@ CheckP:
 *      adds a :p. if this was markes as due
 *      in STATUS. When we encounter <P> in HTML,
 *      we never insert :p. directly but only
 *      set the flag in STATUS so we can do
 *      additional checks, mostly for better
 *      <LI> formatting. This gets called from
 *      various locations before appending
 *      other character data.
 */

VOID CheckP(PXSTRING pxstrIPF,
            PSTATUS pstat)    // in/out: parser status
{
    if (pstat->fNeedsP)
    {
        // we just had a <P> tag previously:
        if (pstat->fInPre)
            xstrcatc(pxstrIPF, '\n');
        else
            xstrcat(pxstrIPF, "\n:p.\n", 0);
        pstat->ulLineLength = 0;
        pstat->fJustHadSpace = TRUE;
        pstat->fNeedsP = FALSE;
    }
}

/*
 *@@ PushList:
 *      pushes the specified list on the list stack.
 */

VOID PushList(PSTATUS pstat,    // in/out: parser status
              ULONG ulList)     // in: LIST_* flag
{
    lstAppendItem(&pstat->llListStack,
                  (PVOID)ulList);
}

/*
 *@@ CheckListTop:
 *      returns TRUE if the top item on the list
 *      stack matches the given LIST_* flag.
 */

BOOL CheckListTop(PSTATUS pstat,    // in: status
                  ULONG ulList)     // in: LIST_* flag
{
    PLISTNODE pNode;
    if (pNode = lstQueryLastNode(&pstat->llListStack))
        if ((ULONG)pNode->pItemData == ulList)
            return (TRUE);
    return (FALSE);
}

/*
 *@@ PopList:
 *      removes the top item from the list stack.
 */

VOID PopList(PSTATUS pstat) // in/out: parser status
{
    lstRemoveNode(&pstat->llListStack,
                  lstQueryLastNode(&pstat->llListStack));
}

typedef PCSZ TAGHANDLER(PARTICLETREENODE pFile2Process, // in: file to process
                        PXSTRING pxstrIPF,  // out: IPF string
                        PSTATUS pstat,      // in/out: parser status
                        BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
                        PSZ pszAttrs);      // in: attributes
typedef TAGHANDLER *PTAGHANDLER;

/*
 *@@ HandleHTML:
 *
 */

PCSZ HandleHTML(PARTICLETREENODE pFile2Process,
                PXSTRING pxstrIPF,  // out: IPF string
                PSTATUS pstat,      // in/out: parser status
                BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
                PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        if (pszAttrs)
        {
            PSZ pszAttrib;
            if (pszAttrib = strhGetTextAttr(pszAttrs,
                                            "GROUP",
                                            NULL))
            {
                pFile2Process->lGroup = atoi(pszAttrib);
                free(pszAttrib);
            }
            if (pszAttrib = strhGetTextAttr(pszAttrs,
                                            "WIDTH",
                                            NULL))
            {
                pFile2Process->pszWidth = pszAttrib;
            }
            if (pszAttrib = strhGetTextAttr(pszAttrs,
                                            "XPOS",
                                            NULL))
            {
                pFile2Process->pszXPos = pszAttrib;
            }
            if (pszAttrib = strhGetTextAttr(pszAttrs,
                                            "HIDDEN",
                                            NULL))
            {
                pFile2Process->fHidden = TRUE;
                free(pszAttrib);
            }
            if (pszAttrib = strhGetTextAttr(pszAttrs,
                                            "RESID",
                                            NULL))
            {
                pFile2Process->ulResID = atoi(pszAttrib);
                if (!pFile2Process->ulResID)
                    return ("Invalid RESID in <HTML> tag.");
                free(pszAttrib);
            }
            if (pszAttrib = strhGetTextAttr(pszAttrs,
                                            "SUBLINKS",
                                            NULL))
            {
                // @@todo
            }
            if (pszAttrib = strhGetTextAttr(pszAttrs,
                                            "NOSUBLINKS",
                                            NULL))
            {
                // @@todo
            }
        }
    }

    return (0);
}

/*
 *@@ HandleHEAD:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleHEAD(PARTICLETREENODE pFile2Process,
                PXSTRING pxstrIPF,  // out: IPF string
                PSTATUS pstat,      // in/out: parser status
                BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
                PSZ pszAttrs)
{
    if (!fClosingTag)
        pstat->fInHead = TRUE;
                // this flag starts skipping all characters
    else
        pstat->fInHead = FALSE;
                // stop skipping all characters

    return 0;
}

/*
 *@@ HandleTITLE:
 *
 */

PCSZ HandleTITLE(PARTICLETREENODE pFile2Process,
                 PXSTRING pxstrIPF,  // out: IPF string
                 PSTATUS pstat,      // in/out: parser status
                 BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
                 PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        // TITLE tag...
        // we need to remove whitespace from the
        // front and the tail, so play a bit here
        PSZ p3 = pstat->pNextClose + 1;
        while (    *p3
                && (    (*p3 == ' ')
                     || (*p3 == '\n')
                     || (*p3 == '\t')
                   )
              )
            p3++;
        if (!*p3)
            return ("Incomplete <TITLE>.");
        // find closing title tag
        PSZ p4 = strstr(p3, "</TITLE>");
        if (!p4)
            return ("<TITLE> has no closing tag.");
        else
        {
            // from beginning of title, go backwards
            // until the last non-whitespace
            PSZ p5 = p4 - 1;
            while (    p5 > p3
                    && (    (*p5 == ' ')
                         || (*p5 == '\n')
                         || (*p5 == '\t')
                       )
                  )
                p5--;

            if (p5 > p3)
            {
                xstrcpy(&pFile2Process->strTitle,
                        p3,
                        p5 - p3 + 1);
                ResolveEntities(pFile2Process,
                                &pFile2Process->strTitle);
                // search on after </TITLE>
                pstat->pNextClose = p4 + 7;
            }
            else
                return ("Empty <TITLE> block.");
        }
    }

    return (0);
}

/*
 *@@ HandleIFDEF:
 *
 */

PCSZ HandleIFDEF(PARTICLETREENODE pFile2Process,
                 PXSTRING pxstrIPF,  // out: IPF string
                 PSTATUS pstat,      // in/out: parser status
                 BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
                 PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        // IFDEF tag...
        // get the define from the attributes
        PCSZ pDef = pszAttrs;
        while (    (*pDef)
                && (*pDef == ' ')
              )
            pDef++;

        if (*pDef)
        {
            PDEFINENODE pNode;
            if (!(pNode = FindDefinition(pDef)))
            {
                // definition _not_ found: then we must _skip_ everything
                PSZ pEndOfSkip;
                if (pEndOfSkip = strstr(pstat->pNextClose + 1,
                                        "</IFDEF>"))
                {
                    // search on after </IFDEF>
                    if (G_ulVerbosity > 1)
                        printf("\n<IFDEF '%s'> is FALSE, skipping %d chars",
                               pDef,
                               pEndOfSkip + 8 - pstat->pSource);
                    pstat->pNextClose = pEndOfSkip + 7;
                }
                else
                    return ("Cannot find closing </IFDEF> tag");
            }
            else
                (pstat->ulNestingIFDEFs)++;
        }
    }
    else
        if (pstat->ulNestingIFDEFs)
            (pstat->ulNestingIFDEFs)--;
        else
            return ("Unrelated closing </IFDEF> tag");

    return (0);
}

/*
 *@@ HandleIFNDEF:
 *
 */

PCSZ HandleIFNDEF(PARTICLETREENODE pFile2Process,
                  PXSTRING pxstrIPF,  // out: IPF string
                  PSTATUS pstat,      // in/out: parser status
                  BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
                  PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        // IFNDEF tag...
        // get the define from the attributes
        PCSZ pDef = pszAttrs;
        while (    (*pDef)
                && (*pDef == ' ')
              )
            pDef++;

        if (*pDef)
        {
            PDEFINENODE pNode;
            if (pNode = FindDefinition(pDef))
            {
                // definition _found_: then we must _skip_ everything
                PSZ pEndOfSkip;
                if (pEndOfSkip = strstr(pstat->pNextClose + 1,
                                        "</IFNDEF>"))
                {
                    // search on after </IFNDEF>
                    if (G_ulVerbosity > 1)
                        printf("\n<IFNDEF '%s'> is FALSE, skipping %d chars",
                               pDef,
                               pEndOfSkip + 9 - pstat->pSource);
                    pstat->pNextClose = pEndOfSkip + 8;
                }
                else
                    return ("Cannot find closing </IFNDEF> tag");
            }
            else
                (pstat->ulNestingIFNDEFs)++;
        }
    }
    else
        if (pstat->ulNestingIFNDEFs)
            (pstat->ulNestingIFNDEFs)--;
        else
            return ("Unrelated closing </IFNDEF> tag");

    return (0);
}

/*
 *@@ HandleP:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleP(PARTICLETREENODE pFile2Process,
             PXSTRING pxstrIPF,
             PSTATUS pstat,    // in/out: parser status
             BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
             PSZ pszAttrs)
{
    if (!fClosingTag)
        // mark this only for now since we don't
        // know yet whether a <LI> comes next...
        // if we do <P><LI>, we get huge spaces
        pstat->fNeedsP = TRUE;

    return 0;
}

/*
 *@@ HandleBR:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleBR(PARTICLETREENODE pFile2Process,
              PXSTRING pxstrIPF,
              PSTATUS pstat,    // in/out: parser status
              BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
              PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        if (!pstat->fInTable)
        {
            // IPF cannot handle .br in tables
            xstrcat(pxstrIPF, "\n.br\n", 0);
            pstat->ulLineLength = 0;
        }
    }

    return 0;
}

/*
 *@@ HandleHR:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleHR(PARTICLETREENODE pFile2Process,
              PXSTRING pxstrIPF,
              PSTATUS pstat,    // in/out: parser status
              BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
              PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        ULONG ul;
        xstrcat(pxstrIPF, ":cgraphic.", 0);
        for (ul = 0; ul < 80; ul++)
            xstrcatc(pxstrIPF, '_');
        xstrcat(pxstrIPF, ":ecgraphic.", 0);
        pstat->fNeedsP = TRUE;
    }
    return 0;
}

/*
 *@@ HandleI:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleI(PARTICLETREENODE pFile2Process,
             PXSTRING pxstrIPF,
             PSTATUS pstat,    // in/out: parser status
             BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
             PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        if (pstat->fItalics)
            return ("Nested <I> or <EM> tag.");
        else
        {
            xstrcat(pxstrIPF, ":hp1.", 0);
            pstat->fItalics = TRUE;
        }
    }
    else
    {
        if (!pstat->fItalics)
            return ("Unrelated closing </I> or </EM> tag.");
        else
        {
            xstrcat(pxstrIPF, ":ehp1.", 0);
            pstat->fItalics = FALSE;
        }
    }

    return 0;
}

/*
 *@@ HandleB:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleB(PARTICLETREENODE pFile2Process,
             PXSTRING pxstrIPF,
             PSTATUS pstat,    // in/out: parser status
             BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
             PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        if (pstat->fBold)
            return ("Nested <B> or <STRONG> tag.");
        else
        {
            xstrcat(pxstrIPF, ":hp2.", 0);
            pstat->fBold = TRUE;
        }
    }
    else
    {
        if (!pstat->fBold)
            return ("Unrelated closing </B> or </STRONG> tag.");
        else
        {
            xstrcat(pxstrIPF, ":ehp2.", 0);
            pstat->fBold = FALSE;
        }
    }

    return 0;
}

/*
 *@@ HandleU:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleU(PARTICLETREENODE pFile2Process,
             PXSTRING pxstrIPF,
             PSTATUS pstat,    // in/out: parser status
             BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
             PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        if (pstat->fUnderlined)
            return ("Nested <U> tag.");
        else
        {
            xstrcat(pxstrIPF, ":hp5.", 0);
            pstat->fUnderlined = TRUE;
        }
    }
    else
    {
        if (!pstat->fUnderlined)
            return ("Unrelated closing </U> tag.");
        else
        {
            xstrcat(pxstrIPF, ":ehp5.", 0);
            pstat->fUnderlined = FALSE;
        }
    }

    return 0;
}

/*
 *@@ HandleCODE:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleCODE(PARTICLETREENODE pFile2Process,
                PXSTRING pxstrIPF,
                PSTATUS pstat,    // in/out: parser status
                BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
                PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        if (pstat->fCode)
            return ("Nested <CODE>, <CITE>, or <TT> tag.");
        else
        {
            xstrcat(pxstrIPF, ":font facename='Courier' size=18x12.", 0);
            pstat->fCode = TRUE;
        }
    }
    else
    {
        if (!pstat->fCode)
            return ("Unrelated closing </CODE>, </CITE>, or </TT> tag.");
        else
        {
            xstrcat(pxstrIPF, ":font facename=default size=0x0.", 0);
            pstat->fCode = FALSE;
        }
    }

    return 0;
}

/*
 *@@ HandlePRE:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandlePRE(PARTICLETREENODE pFile2Process,
               PXSTRING pxstrIPF,
               PSTATUS pstat,    // in/out: parser status
               BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
               PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        if (pstat->fInPre)
            return ("Nested <PRE> tag.");
        else
        {
            xstrcat(pxstrIPF, "\n:cgraphic.", 0);
            pstat->fInPre = TRUE;
        }
    }
    else
    {
        if (!pstat->fInPre)
            return ("Unrelated closing </PRE> tag.");
        else
        {
            xstrcat(pxstrIPF, "\n:ecgraphic.", 0);
            pstat->fInPre = FALSE;
            pstat->fNeedsP = TRUE;
        }
    }

    return 0;
}

/*
 *@@ HandleUL:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleUL(PARTICLETREENODE pFile2Process,
              PXSTRING pxstrIPF,
              PSTATUS pstat,    // in/out: parser status
              BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
              PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        xstrcat(pxstrIPF, "\n:ul compact.", 0);
        PushList(pstat, LIST_UL);
        pstat->ulLineLength = 0;
    }
    else
    {
        if (CheckListTop(pstat, LIST_UL))
        {
            xstrcat(pxstrIPF, "\n:eul.", 0);
            PopList(pstat);
            pstat->fNeedsP = TRUE;
        }
        else
            return ("Invalid </UL> nesting.");
    }

    return 0;
}

/*
 *@@ HandleOL:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleOL(PARTICLETREENODE pFile2Process,
              PXSTRING pxstrIPF,
              PSTATUS pstat,    // in/out: parser status
              BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
              PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        xstrcat(pxstrIPF, "\n:ol compact.", 0);
        PushList(pstat, LIST_OL);
        pstat->ulLineLength = 0;
    }
    else
    {
        if (CheckListTop(pstat, LIST_OL))
        {
            xstrcat(pxstrIPF, "\n:eol.", 0);
            PopList(pstat);
            pstat->fNeedsP = TRUE;
        }
        else
            return ("Invalid </OL> nesting.");
    }

    return 0;
}

/*
 *@@ HandleLI:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleLI(PARTICLETREENODE pFile2Process,
              PXSTRING pxstrIPF,
              PSTATUS pstat,    // in/out: parser status
              BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
              PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        if (    (CheckListTop(pstat, LIST_UL))
             || (CheckListTop(pstat, LIST_OL))
           )
        {
            if (pstat->fNeedsP)
            {
                // we just had a <P> previously:
                // add a .br instead or we'll get huge
                // whitespace
                xstrcat(pxstrIPF, "\n.br\n", 0);
                pstat->fNeedsP = FALSE;
            }
            xstrcat(pxstrIPF, ":li.\n", 0);
            pstat->ulLineLength = 0;
        }
        else
            return ("<LI> outside list.");
    }

    return 0;
}

/*
 *@@ HandleDL:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleDL(PARTICLETREENODE pFile2Process,
              PXSTRING pxstrIPF,
              PSTATUS pstat,    // in/out: parser status
              BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
              PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        xstrcat(pxstrIPF, "\n:dl compact break=all.", 0);
        PushList(pstat, LIST_DL);
        pstat->ulLineLength = 0;
    }
    else
    {
        if (CheckListTop(pstat, LIST_DL))
        {
            if (!pstat->ulDefinition)
                // if the list didn't stop with a DD last,
                // we must add one or IPFC will choke
                xstrcat(pxstrIPF, "\n:dd.", 0);

            xstrcat(pxstrIPF, "\n:edl.", 0);
            PopList(pstat);
            pstat->fNeedsP = TRUE;
            pstat->ulDefinition = 0;
        }
        else
            return ("Invalid </DL> nesting.");
    }

    return 0;
}

/*
 *@@ HandleDD:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleDD(PARTICLETREENODE pFile2Process,
              PXSTRING pxstrIPF,
              PSTATUS pstat,    // in/out: parser status
              BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
              PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        if (!CheckListTop(pstat, LIST_DL))
            return ("<DD> outside of <DL>.");
        else
        {
            xstrcat(pxstrIPF, "\n:dd.", 0);
            pstat->ulLineLength = 0;
            pstat->ulDefinition = 1;
        }
    }

    return 0;
}

/*
 *@@ HandleDT:
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

PCSZ HandleDT(PARTICLETREENODE pFile2Process,
              PXSTRING pxstrIPF,
              PSTATUS pstat,    // in/out: parser status
              BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
              PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        if (!CheckListTop(pstat, LIST_DL))
            return ("<DT> outside of <DL>.");
        else
        {
            xstrcat(pxstrIPF, "\n:dt.", 0);
            pstat->ulLineLength = 0;
            pstat->ulDefinition = 0;
        }
    }

    return 0;
}

#define START_KEY  "@#!LINK1@#!"
#define END_KEY    "@#!LINK2@#!"

/*
 *@@ HandleA:
 *
 *@@changed V0.9.20 (2002-07-06) [umoeller]: added support for cpref etc. toolkit INF variables
 +@@changed V0.9.20 (2002-07-12) [umoeller]: added support for A HREF="helpfile.hlp#panel"
 */

PCSZ HandleA(PARTICLETREENODE pFile2Process,
             PXSTRING pxstrIPF,
             PSTATUS pstat,    // in/out: parser status
             BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
             PSZ pszAttrs)
{
    if (!fClosingTag)
    {
        if (pstat->ulInLink)
            return ("Nested <A ...> blocks.");
        else
        {
            // check the attribute:
            // is it HREF?
            if (pszAttrs)
            {
                PSZ pszAttrib;
                if (pszAttrib = strhGetTextAttr(pszAttrs,
                                                "AUTO",
                                                NULL))
                {
                    PARTICLETREENODE patn
                        = GetOrCreateArticle(pszAttrib,
                                             pFile2Process->ulHeaderLevel,
                                             pFile2Process);

                    // V0.9.20 (2002-07-12) [umoeller]
                    xstrPrintf(&pstat->strLinkTag,
                               // hack in the @#!LINK@#! string for now;
                               // this is later replaced with the
                               // resid in ParseFiles
                               ":link reftype=hd res=" START_KEY "%s" END_KEY " auto dependent.",
                               pszAttrib);

                    xstrcats(pxstrIPF,
                             &pstat->strLinkTag);

                    pstat->ulInLink = 2;        // special, do not close
                }
                else if (pszAttrib = strhGetTextAttr(pszAttrs,
                                                     "HREF",
                                                     NULL))
                {
                    PSZ pExt;
                    if (pExt = strhistr(pszAttrib, ".INF"))
                    {
                        // special INF cross-link support:
                        // V0.9.20 (2002-07-06) [umoeller]
                        // remove .INF and the following # if any;
                        // this allows us to specify "cpref.inf#func"
                        // in the sources even though cpref is really
                        // an environment variable defined by the
                        // toolkit install
                        PSZ p3 = pExt + 4;
                        if (*p3 == '#')
                        {
                            *p3 = ' ';
                        }
                        strcpy(pExt, p3);

                        // V0.9.20 (2002-07-12) [umoeller]
                        xstrPrintf(&pstat->strLinkTag,
                                   ":link reftype=launch object='view.exe' data='%s'.",
                                   pszAttrib);

                        xstrcats(pxstrIPF,
                                 &pstat->strLinkTag);
                    }
                    else if (pExt = strhistr(pszAttrib, ".HLP"))
                    {
                        // V0.9.20 (2002-07-06) [umoeller]
                        // special HLP cross-link support:
                        // syntax is "helpfile.hlp#panel" here
                        PSZ p3 = pExt + 4;

                        if (*p3 == '#')
                        {
                            *p3 = '\0';

                            // V0.9.20 (2002-07-12) [umoeller]
                            xstrPrintf(&pstat->strLinkTag,
                                       ":link reftype=hd refid=%d database='%s'.",
                                       atoi(p3 + 1),
                                       pszAttrib);

                            xstrcats(pxstrIPF,
                                     &pstat->strLinkTag);
                        }
                        else
                            return ("Invalid syntax in <A HREF=\"file.hlp#panel\".");
                    }
                    else
                    {
                        // no INF:
                        PARTICLETREENODE patn
                        = GetOrCreateArticle(pszAttrib,
                                             pFile2Process->ulHeaderLevel,
                                             pFile2Process);

                        // V0.9.20 (2002-07-12) [umoeller]
                        xstrPrintf(&pstat->strLinkTag,
                                   // hack in the @#!LINK@#! string for now;
                                   // this is later replaced with the
                                   // resid in ParseFiles
                                   ":link reftype=hd res=" START_KEY "%s" END_KEY ".",
                                   pszAttrib);

                        xstrcats(pxstrIPF,
                                 &pstat->strLinkTag);
                    }

                    pstat->ulInLink = 1;        // regular, do close

                    free(pszAttrib);
                }
                else
                    return ("Unknown attribute to <A> tag.");
            }
            else
                return ("<A> tag has no attributes.");
        }
    } // end if (!fClosingTag)
    else
    {
        if (pstat->ulInLink)
        {
            if (pstat->ulInLink == 1)
                xstrcat(pxstrIPF,
                        ":elink.",
                        0);
            // otherwise 2: that's from A AUTO, do not close

            pstat->ulInLink = 0;
        }
        else
            return ("Unrelated closing </A> tag.");
    }

    return (0);
}

/*
 *@@ HandleIMG:
 *
 */

PCSZ HandleIMG(PARTICLETREENODE pFile2Process,
               PXSTRING pxstrIPF,
               PSTATUS pstat,    // in/out: parser status
               BOOL fClosingTag,   // in: TRUE == closing tag, FALSE == opening tag
               PSZ pszAttrs)
{
    PSZ pszAttrib;
    if (pszAttrib = strhGetTextAttr(pszAttrs,
                                    "SRC",
                                    NULL))
    {
        XSTRING str;
        xstrInitSet(&str, pszAttrib);
        PSZ p = strrchr(pszAttrib, '.');
        if (    (!p)
             || (strlen(p) < 3)
           )
            xstrReserve(&str, CCHMAXPATH);
        strcpy(p, ".bmp");

        if (access(str.psz, 0))
            // doesn't exist:
            Error(1,
                  __FILE__, __LINE__, __FUNCTION__,
                  "The bitmap file \"%s\" was not found.",
                  str.psz);

        if (pstat->ulInLink == 1)
            // if we are inside a link, close it first
            // V0.9.20 (2002-07-12) [umoeller]
            xstrcat(pxstrIPF,
                    ":elink.",
                    0);

        if (pstat->fNeedsP)
            // if we had a <P> just before, do not use runin
            xstrCatf(pxstrIPF,
                     "\n:artwork name='%s' align=left.",
                     str.psz);
        else
            xstrCatf(pxstrIPF,
                     "\n:artwork name='%s' runin align=left.",
                     str.psz);

        // if we are inside a link, add special tag
        // :artwork name='img/xfwps_mini.bmp' runin align=left.
        // :artlink.:link reftype=hd res=58.:eartlink.
        // V0.9.20 (2002-07-12) [umoeller]
        if (pstat->ulInLink)
        {
            xstrCatf(pxstrIPF,
                     "\n:artlink.\n%s\n:eartlink.\n",
                     pstat->strLinkTag.psz);
                        // e.g. ":link reftype=hd res=58."

            // do not add another :elink.
            pstat->ulInLink = 2;
        }

        xstrClear(&str);

        pstat->fNeedsP = FALSE;
    }
    else
        return ("No SRC attribute to <IMG> tag.");

    return (0);
}

typedef struct _HANDLERDEF
{
    PCSZ        pcszTag;
    PTAGHANDLER pHandler;
} HANDLERDEF, *PHANDLERDEF;

typedef struct _HANDLERTREENODE
{
    TREE        Tree;           // ulkey is used for (ULONG)pcszTag
    PTAGHANDLER pHandler;
} HANDLERTREENODE, *PHANDLERTREENODE;

/*
 *@@ HandlersList:
 *      array of all tags we understand with the
 *      handler func that handles the tag, or NULL
 *      if the tag is to be silently ignored.
 *
 *      We build a string map from this list for
 *      fast lookup in main().
 *
 *@@added V0.9.16 (2001-11-22) [umoeller]
 */

HANDLERDEF HandlersList[] =
    {
        "P", HandleP,
        "BR", HandleBR,
        "LI", HandleLI,
        "IMG", HandleIMG,
        "HR", HandleHR,
        "I", HandleI,
        "EM", HandleI,
        "B", HandleB,
        "STRONG", HandleB,
        "U", HandleU,
        "CODE", HandleCODE,
        "CITE", HandleCODE,
        "TT", HandleCODE,
        "A", HandleA,
        "UL", HandleUL,
        "OL", HandleOL,
        "DL", HandleDL,
        "DT", HandleDT,
        "DD", HandleDD,
        "HEAD", HandleHEAD,
        "PRE", HandlePRE,
        "HTML", HandleHTML,
        "TITLE", HandleTITLE,
        "IFDEF", HandleIFDEF,
        "IFNDEF", HandleIFNDEF,
        "BODY", NULL,           // ignore
        "H1", NULL,             // ignore
        "H2", NULL,             // ignore
        "H3", NULL,             // ignore
        "H4", NULL,             // ignore
        "H5", NULL,             // ignore
        "H6", NULL              // ignore
    };

TREE *G_HandlersTreeRoot;

/*
 *@@ HandleTag:
 *      HTML tag handler. Gets called from ParseFile
 *      whenever a '<' character is found in the HTML
 *      sources. We then look up the tag name in the
 *      string map we built in main() and call the
 *      respective handler, if one exists.
 *
 *@@changed V0.9.16 (2001-11-22) [umoeller]: major speedup with tag handlers string map
 */

const char* HandleTag(PARTICLETREENODE pFile2Process,
                      PSTATUS pstat)    // in/out: parser status
{
    const char *pcszError = NULL;
    PXSTRING pxstrIPF = &pFile2Process->strIPF;

    PSZ pStartOfTagName = pstat->pSource + 1;

    // is this a comment?
    if (!strncmp(pStartOfTagName, "!--", 3))
    {
        // start of comment:
        // find end of comment
        PSZ pEnd;
        if (pEnd = strstr(pStartOfTagName + 3, "-->"))
        {
            // found:
            // search on after end of comment
            pstat->pSource = pEnd + 2;
        }
        else
        {
            // end of comment not found:
            // stop formatting...
            pcszError = "Terminating comment not found.";
            pstat->fFatal = TRUE;
        }
    }
    else
    {
        // no comment:
        // find end of tag
        PSZ     p2 = pStartOfTagName,
                // pNextClose = 0,     // receives first '>' after '<'
                pNextSpace = 0;     // receives first ' ' after '<'
        BOOL    fCont = TRUE;

        pstat->pNextClose = 0;

        while (fCont)
        {
            switch (*p2)
            {
                case ' ':
                case '\r':
                case '\n':
                case '\t':
                    // store first space after '<'
                    if (!pNextSpace)
                        pNextSpace = p2;
                    *p2 = ' ';
                break;

                case '>':   // end of tag found:
                    pstat->pNextClose = p2;
                    fCont = FALSE;
                break;

                case '<':
                    // another opening tag:
                    // that's an HTML error
                    return ("Opening < within another tag found.");
                break;

                case 0:
                    fCont = FALSE;
                break;
            }
            p2++;
        }

        if (pstat->pNextClose)
        {
            // end of tag found:
            ULONG cbTag;
            PSZ pszAttrs = NULL;

            if ((pNextSpace) && (pNextSpace < pstat->pNextClose))
            {
                // we have attributes:
                cbTag = pNextSpace - pStartOfTagName;
                pszAttrs = strhSubstr(pNextSpace, pstat->pNextClose);
            }
            else
                cbTag = pstat->pNextClose - pStartOfTagName;

            if (!cbTag)
            {
                // happens if we have a "<>" in the text:
                // just insert the '<>' and go on, we have no tag here
                xstrcatc(pxstrIPF, *(pstat->pSource)++);
                xstrcatc(pxstrIPF, *(pstat->pSource)++);
                pstat->ulLineLength += 2;
            }
            else
            {
                PHANDLERTREENODE pHandlerTreeNode;
                PTAGHANDLER pHandler = NULL;
                BOOL fClosingTag = FALSE;

                // attributes come after this
                // and go up to pNextClose

                // add a null terminator so we can
                // use strcmp for testing for tags
                CHAR cSaved = *(pStartOfTagName + cbTag);
                *(pStartOfTagName + cbTag) = '\0';

                // printf("'%s' ", pStartOfTagName);

                // tags which can be open or closed
                // (e.g. B or /B):
                PSZ pStart2 = pStartOfTagName;

                if (*pStart2 == '/')
                {
                    // closing tag:
                    fClosingTag = TRUE;
                    // use next char for strcmp
                    pStart2++;
                }

                // now handle tags
                if (pHandlerTreeNode = (PHANDLERTREENODE)treeFind(G_HandlersTreeRoot,
                                                                  (ULONG)pStart2,
                                                                  fnCompareStrings))
                {
                    // handler tree node found:
                    if (pHandlerTreeNode->pHandler)
                        // tag needs handling code:
                        pcszError = pHandlerTreeNode->pHandler(pFile2Process,
                                                               pxstrIPF,
                                                               pstat,
                                                               fClosingTag,
                                                               pszAttrs);
                }
                else
                {
                    xstrcpy(&G_strError, "", 0);
                    xstrCatf(&G_strError,
                               "Unknown tag %s (%s)",
                               pStartOfTagName,
                               pStart2);
                    pcszError = G_strError.psz;
                }

                // restore char under null terminator
                *(pStartOfTagName + cbTag) = cSaved;

                // skip the tag, even if unknown
                pstat->pSource = pstat->pNextClose;

                if (pszAttrs)
                    free(pszAttrs);
            }
        } // end if (pNextClose)
        else
        {
            xstrcatc(pxstrIPF,
                     *(pstat->pSource));
            // no closing tag found:
            // just return, caller will try next char
            // (probably no tag anyway)
        }
    }

    return (pcszError);
}

/*
 *@@ ParseFile:
 *      converts the HTML code in the buffer to IPF.
 *
 *      This is a complete conversion except:
 *
 *      --  automatic resids will be assigned later,
 *          unless this file has <HTML RESID=xxx> set;
 *
 *      --  the :hX. tag will not be added here
 *          (because it depends on sublinks).
 *
 *      This adds files to G_llFiles2Process if they
 *      are linked to from this file by calling
 *      GetOrCreateArticle.
 */

APIRET ParseFile(PARTICLETREENODE pFile2Process,
                 PSZ pszBuf)
{
    PXSTRING pxstrIPF = &pFile2Process->strIPF;

    STATUS stat = {0};
    lstInit(&stat.llListStack, FALSE);
    xstrInit(&stat.strLinkTag, 0);       // V0.9.20 (2002-07-12) [umoeller]
    stat.fJustHadSpace = TRUE;

    // start at beginning of buffer
    stat.pSource = pszBuf;

    while (TRUE)
    {
        const char *pcszError = NULL;
        CHAR    c = *(stat.pSource);
        PSZ     pSaved = stat.pSource;

        switch (c)
        {
            case '<':
                pcszError = HandleTag(pFile2Process,
                                      &stat);
            break;

            case ':':
                if (!stat.fInHead)
                {
                    CheckP(pxstrIPF, &stat);
                    xstrcat(pxstrIPF, "&colon.", 7);
                    stat.ulLineLength += 7;
                }
            break;

            case '.':
                if (!stat.fInHead)
                {
                    CheckP(pxstrIPF, &stat);
                    xstrcat(pxstrIPF, "&per.", 5);
                    stat.ulLineLength += 5;
                }
            break;

            case '\r':
                // just skip this
            break;

            case '\n':
            case ' ':
                // throw out all line breaks and spaces
                // unless we're in PRE mode
                if (stat.fInPre)
                    xstrcatc(pxstrIPF,
                             c);
                else
                {
                    CheckP(pxstrIPF, &stat);
                    // add a space instead, if we didn't just have one
                    // dump duplicate spaces
                    if (!stat.fJustHadSpace)
                    {
                        // on spaces, check how long the line is already...
                        if (stat.ulLineLength > 50)
                        {
                            xstrcatc(pxstrIPF, '\n');
                            stat.ulLineLength = 0;
                        }
                        else
                        {
                            xstrcatc(pxstrIPF, ' ');
                            (stat.ulLineLength)++;
                        }
                        stat.fJustHadSpace = TRUE;
                    }
                }

            break;

            case '\0':
                // null terminator reached:
                // some safety checks
                if (stat.fInHead)
                    pcszError = "No closing </HEAD> tag found.";
            break;

            default:
                if (!stat.fInHead)
                {
                    CheckP(pxstrIPF, &stat);
                    xstrcatc(pxstrIPF,
                             c);
                    (stat.ulLineLength)++;
                    stat.fJustHadSpace = FALSE;
                }
        }

        if (pcszError)
        {
            // get the line count
            CHAR c = *pSaved;
            *pSaved = 0;
            Error(1,
                  __FILE__, __LINE__, __FUNCTION__,
                  "file \"%s\", line %d:\n   %s",
                  (PSZ)pFile2Process->Tree.ulKey,       // file name
                  strhCount(pszBuf, '\n') + 1,
                  pcszError);
            *pSaved = c;
            if (stat.fFatal)
                return (1);
            // otherwise continue
        }

        if (*(stat.pSource))
            (stat.pSource)++;
        else
            break;
    }

    ConvertEscapes(&pFile2Process->strTitle);
    ConvertEscapes(pxstrIPF);

    return (0);
}

/* ******************************************************************
 *
 *   File processor
 *
 ********************************************************************/

/*
 *@@ AppendToMainBuffer:
 *
 */

VOID AppendToMainBuffer(PXSTRING pxstrIPF,
                        PARTICLETREENODE pFile2Process,
                        PXSTRING pstrExtras)       // temp buffer for speed
{
    if (pFile2Process->ulHeaderLevel == 1)
    {
        // root file:
        xstrCatf(pxstrIPF,
                 ":title.%s\n",
                 pFile2Process->strTitle.psz);
    }

    xstrCatf(pxstrIPF,
             "\n.* Source file: \"%s\"\n",
             (PSZ)pFile2Process->Tree.ulKey); // pszFilename);

    xstrcpy(pstrExtras, NULL, 0);
    if (pFile2Process->lGroup)
        xstrCatf(pstrExtras,
                 " group=%d",
                 pFile2Process->lGroup);

    BOOL fFull = TRUE;
    if (pFile2Process->pszXPos)
    {
        xstrCatf(pstrExtras,
                 " x=%s",
                 pFile2Process->pszXPos);
        fFull = FALSE;
    }
    if (pFile2Process->pszWidth)
    {
        xstrCatf(pstrExtras,
                 " width=%s",
                 pFile2Process->pszWidth);
        fFull = FALSE;
    }

    // If neither x nor width are given, make this full size
    // so that help panels will resize properly with the
    // main helpmgr frame. Thanks to Alex Taylor for the hint.
    // V1.0.0 (2002-08-21) [umoeller]
    if (fFull)
    {
        xstrcat(pstrExtras,
                " x=left y=bottom width=100% height=100%",
                0);
    }

    if (pFile2Process->fHidden)
        xstrcat(pstrExtras, " hide", 0);

    ULONG ulHeaderLevel = pFile2Process->ulHeaderLevel;
    // if this is not at root level already,
    // move down one level (unless -r is specified)
    if (!G_fNoMoveToRoot)
        if (ulHeaderLevel > 1)
            ulHeaderLevel--;

    xstrCatf(pxstrIPF,
             ":h%d res=%d%s.%s\n",
             ulHeaderLevel,
             pFile2Process->ulResID,
             (pstrExtras->ulLength)
                  ? pstrExtras->psz
                  : "",
             pFile2Process->strTitle.psz);
    xstrcat(pxstrIPF,
            ":p.\n",
            0);

    // and the rest of the converted file
    xstrcats(pxstrIPF,
             &pFile2Process->strIPF);

    pFile2Process->fWritten = TRUE;
}

/*
 *@@ DumpArticlesWithParent:
 *
 */

VOID DumpArticlesWithParent(PXSTRING pxstrIPF,
                            PARTICLETREENODE pParent,
                            PXSTRING pstrExtras)
{
    PLISTNODE pNode = lstQueryFirstNode(&G_llFiles2Process);
    while (pNode)
    {
        PARTICLETREENODE pFile2Process = (PARTICLETREENODE)pNode->pItemData;

        // rule out special links for now
        if (    (pFile2Process->ulHeaderLevel != -1)
             && (!pFile2Process->fWritten)
           )
        {
            if (pFile2Process->pFirstReferencedFrom == pParent)
            {
                if (G_ulVerbosity > 1)
                    printf("File \"%s\" has resid %d, %d bytes IPF\n",
                           (PSZ)pFile2Process->Tree.ulKey, // pszFilename,
                           pFile2Process->ulResID,
                           pFile2Process->strIPF.ulLength);

                AppendToMainBuffer(pxstrIPF,
                                   pFile2Process,
                                   pstrExtras);

                // recurse with this node
                DumpArticlesWithParent(pxstrIPF,
                                       pFile2Process,
                                       pstrExtras);
            }

            // build a linked list of articles which belong
            // under this article

            /* PLISTNODE pSubnode = lstQueryFirstNode(&G_llFiles2Process);
            while (pSubnode)
            {
                PARTICLETREENODE pSub = (PARTICLETREENODE)pSubnode->pItemData;

                if (    (pSub->pFirstReferencedFrom == pFile2Process)
                     && (pSub->ulHeaderLevel != -1)
                   )
                {
                    if (G_ulVerbosity > 1)
                        printf("   loop 2: Writing out %s\n",
                               pSub->pszFilename);
                    AppendToMainBuffer(pxstrIPF,
                                       pSub,
                                       &strExtras);
                        // marks the file as "written"
                }

                pSubnode = pSubnode->pNext;
            } */
        }

        // now for the next main node
        pNode = pNode->pNext;
    }
}

/*
 *@@ ProcessFiles:
 *      loops through all files.
 *
 *      When this is called (from main()), G_llFiles2Process
 *      contains only the one file that was specified on
 *      the command line. This then calls ParseFile() on
 *      that file, which will add to the list for every
 *      A tag that links to another file.
 *
 *      After ParseFile() has been called for each such
 *      file then, we run several additional loops here
 *      for resid resolution and such.
 *
 *      Finally, in the last loop, the IPF code from all
 *      files is added to the one given XSTRING buffer,
 *      which will then be written out to disk by the
 *      caller (main()).
 */

APIRET ProcessFiles(PXSTRING pxstrIPF)           // out: one huge IPF file
{
    PLISTNODE pNode;
    APIRET arc = NO_ERROR;

    /*
     * loop 1:
     *
     */

    if (G_ulVerbosity > 1)
        printf("Reading source files...\n");

    // go thru the list of files to process and
    // translate them from HTML to IPF...
    // when this func gets called, this list
    // contains only the root file, but ParseFile
    // will add new files to the list, so we just
    // keep going
    XSTRING strSource;
    xstrInit(&strSource, 0);

    pNode = lstQueryFirstNode(&G_llFiles2Process);
    while (pNode)
    {
        PARTICLETREENODE pFile2Process = (PARTICLETREENODE)pNode->pItemData;

        // rule out special links for now
        if (pFile2Process->ulHeaderLevel != -1)
        {
            PSZ pszContents = NULL;

            if (G_ulVerbosity > 1)
            {
                printf("processing %s... ",
                       (PSZ)pFile2Process->Tree.ulKey); // pszFilename);
                fflush(stdout);
            }
            else if (G_ulVerbosity)
            {
                printf(".");
                fflush(stdout);
            }

            xstrcpy(&G_strCrashContext, "", 0);
            xstrCatf(&G_strCrashContext,
                       "Processing file %s",
                       (PSZ)pFile2Process->Tree.ulKey);

            ULONG cbRead = 0;
            if (!(arc = doshLoadTextFile((PSZ)pFile2Process->Tree.ulKey, // pszFilename,
                                         &pszContents,
                                         &cbRead)))
            {
                xstrset2(&strSource, pszContents, cbRead - 1);
                xstrConvertLineFormat(&strSource, CRLF2LF);

                ResolveEntities(pFile2Process,
                                &strSource);

                // now go convert this buffer to IPF
                if (!(arc = ParseFile(pFile2Process,
                                      strSource.psz)))
                {
                }

                pFile2Process->fProcessed = TRUE;

                if (G_ulVerbosity > 1)
                    printf("\n");
            }

            if (arc)
            {
                Error(2,
                      __FILE__, __LINE__, __FUNCTION__,
                      "Error %d occurred reading file \"%s\".",
                      arc,
                      (PSZ)pFile2Process->Tree.ulKey); // pszFilename);
                arc = 1;
            }
        }

        if (arc)
            break;

        // next file (ParseFile has added files to list)
        pNode = pNode->pNext;
    }

    if (!arc)
    {
        /*
         * loop 2:
         *
         */

        // go through the entire file list again
        // and assign an automatic resid if this
        // has not been set yet

        if (G_ulVerbosity > 1)
            printf("Assigning automatic resids...\n");

        ULONG ulNextResID = 10000;

        pNode = lstQueryFirstNode(&G_llFiles2Process);
        while (pNode)
        {
            PARTICLETREENODE pFile2Process = (PARTICLETREENODE)pNode->pItemData;

            // process special links too
            if (pFile2Process->ulResID == -1)
                pFile2Process->ulResID = ulNextResID++;

            pNode = pNode->pNext;
        }

        if (G_ulVerbosity > 1)
            printf("  Done, %d resids assigned (%d files had explicit resids)\n",
                   (ulNextResID - 10000),
                   lstCountItems(&G_llFiles2Process) - (ulNextResID - 10000));

        /*
         * loop 3:
         *
         */

        if (G_ulVerbosity > 1)
            printf("Replacing link strings...\n");

        // go through the entire file list again
        // and in each IPF buffer, replace the
        // special link strings with the good resids,
        // which are known by now

        ULONG ulStartKeyLen = strlen(START_KEY),
              ulEndKeyLen = strlen(END_KEY);

        pNode = lstQueryFirstNode(&G_llFiles2Process);
        while (pNode && !arc)
        {
            PARTICLETREENODE pFile2Process = (PARTICLETREENODE)pNode->pItemData;

            // rule out special links for now
            if (pFile2Process->ulHeaderLevel != -1)
            {
                PSZ pStart;

                if (pStart = pFile2Process->strIPF.psz)
                        // could be empty V0.9.16 (2001-11-22) [umoeller]
                {
                    PSZ p;
                    // now, find the special ugly link keys
                    // we hacked in before; for each string
                    // target filename now, find the resid

                    while (p = (strstr(pStart,
                                       START_KEY)))
                    {
                        PSZ pFirst = pFile2Process->strIPF.psz;

                        PSZ p2 = strstr(p + ulStartKeyLen,
                                        END_KEY);
                        if (!p2)
                        {
                            Error(2,
                                  __FILE__, __LINE__, __FUNCTION__,
                                  "Cannot find second LINK string for \"%s\"\nFile: \"%s\"\n",
                                  p,
                                  (PSZ)pFile2Process->Tree.ulKey); // pszFilename);
                            arc = 1;
                            break;
                        }

                        CHAR cSaved = *p2;
                        *p2 = '\0';

                        if (G_ulVerbosity > 2)
                            printf("   encountered link \"%s\" at %d (len %d), searching resid\n",
                                   p + ulStartKeyLen,
                                   p - pFirst,
                                   p2 - pFirst);

                        PARTICLETREENODE pTarget = GetOrCreateArticle(p + ulStartKeyLen, // filename
                                                                      -1,   // do not create
                                                                      NULL);

                        if (!pTarget)
                        {
                            Error(2,
                                  __FILE__, __LINE__, __FUNCTION__,
                                  "Cannot resolve cross reference for \"%s\"\nFile: \"%s\"\n",
                                  p + ulStartKeyLen,
                                  (PSZ)pFile2Process->Tree.ulKey); // pszFilename);
                            arc = 1;
                            break;
                        }

                        *p2 = cSaved;

                        CHAR szResID[30];
                        sprintf(szResID, "%d", pTarget->ulResID);
                        ULONG ulResIDLen = strlen(szResID);
                        ULONG   ulFirst = p - pFirst,
                                cReplace = (p2 + ulStartKeyLen) - p;
                        if (G_ulVerbosity > 2)
                            printf("     ofs %d, cReplace %d, replacing with \"%s\"\n",
                                   ulFirst,
                                   cReplace,
                                   szResID);
                        xstrrpl(&pFile2Process->strIPF,
                                // ofs of first char to replace:
                                ulFirst,
                                // count of chars to replace:
                                cReplace, // (p2 + KEYLEN) - p,
                                // replace this with the resid
                                szResID,
                                ulResIDLen);

                        pStart = pFile2Process->strIPF.psz + ulFirst;
                    }
                }
            }

            pNode = pNode->pNext;
        }

        if (!arc)
        {
            XSTRING strExtras;
            xstrInit(&strExtras, 0);

            /*
             * loop 4:
             *
             */

            if (G_ulVerbosity > 1)
                printf("Composing IPF buffer...\n");

            LINKLIST llSubarticles;
            lstInit(&llSubarticles, FALSE);

            // now dump out IPF into one huge buffer...
            // note, this loop will go thru all files again,
            // but since we have a second loop within which
            // marks files as written, this will effectively
            // only process the root items
            DumpArticlesWithParent(pxstrIPF,
                                   NULL,       // root documents only
                                   &strExtras);
                    // this will recurse

            /*
             * loop 5:
             *
             */

            // add all the special links
            // to the bottom; these should be the only
            // unprocessed files, or something went wrong here
            pNode = lstQueryFirstNode(&G_llFiles2Process);
            BOOL fFirst = TRUE;
            XSTRING strEncode;
            xstrInit(&strEncode, 0);
            while (pNode)
            {
                PARTICLETREENODE pFile2Process = (PARTICLETREENODE)pNode->pItemData;

                // process only the special files now
                if (pFile2Process->ulHeaderLevel == -1)
                {
                    if (fFirst)
                    {
                        xstrcat(pxstrIPF,
                                ":h1 group=99 x=right width=30%.",
                                0);
                        xstrcat(pxstrIPF,
                                G_pcszResourcesOnInternetTitle,
                                0);
                        xstrcatc(pxstrIPF, '\n');
                        xstrcat(pxstrIPF,
                                G_pcszResourcesOnInternetBody,
                                0);
                        xstrcatc(pxstrIPF, '\n');
                        fFirst = FALSE;
                    }

                    // encode the strings again because STUPID ipf
                    // gets confused otherwise
                    xstrcpy(&strEncode,
                            (PSZ)pFile2Process->Tree.ulKey, // pszFilename,
                            0);
                    ConvertEscapes(&strEncode);
                    ULONG ulOfs = 0;
                    while (xstrFindReplaceC(&strEncode,
                                            &ulOfs,
                                            ":",
                                            "&colon."))
                        ;

                    xstrCatf(pxstrIPF,
                               ":h2 res=%d group=98 x=right y=bottom width=60%% height=40%%.%s\n",
                               pFile2Process->ulResID,
                               strEncode.psz);
                    xstrCatf(pxstrIPF,
                               ":p.:lines align=center."
                                    "\n%s\n"
                                    ":p.:link reftype=launch object='netscape.exe' data='%s'.\n"
                                    "%s\n"
                                    ":elink.:elines.\n",
                               G_pcszClickBelow,
                               (PSZ)pFile2Process->Tree.ulKey, // pszFilename,
                               strEncode.psz);
                }
                else
                    if (!pFile2Process->fProcessed)
                        Error(2,
                              __FILE__, __LINE__, __FUNCTION__,
                              "Strange, file \"%s\" wasn't processed.",
                              (PSZ)pFile2Process->Tree.ulKey); // pszFilename);

                pNode = pNode->pNext;
            }

            if (G_ulVerbosity > 1)
                printf("\nDone processing files.\n");
            else
                printf("\n");
        }
    }

    return (arc);
}

/* ******************************************************************
 *
 *   main
 *
 ********************************************************************/

/*
 *@@ AddDefine:
 *
 */

BOOL AddDefine(const char *pcszDefine)      // in: first char after #define
{
    PCSZ    p = pcszDefine;
    PSZ     pszIdentifier = NULL,
            pszValue = NULL;

    /*
                    printf("testing %c%c%c%c%c%c%c%c%c%c\n",
                            *p,     *(p+1), *(p+2), *(p+3), *(p+4),
                            *(p+5), *(p+6), *(p+7), *(p+8), *(p+9));
    */

    while ((*p) && (!pszValue))
    {
        switch (*p)
        {
            // skip comments
            case '/':
            case '\r':
            case '\n':
                // forget it
                    // printf("      quitting\n");
                return (FALSE);

            case ' ':
            case '\t':
            break;

            case '"':
            {
                if (pszIdentifier)
                {
                    /* printf("testing %c%c%c%c%c%c%c%c%c%c\n",
                            *p,     *(p+1), *(p+2), *(p+3), *(p+4),
                            *(p+5), *(p+6), *(p+7), *(p+8), *(p+9)); */

                    // looks like a string constant...
                    PCSZ pEnd = p + 1;
                    while (*pEnd && !pszValue)
                    {
                        switch (*pEnd)
                        {
                            case '\r':
                            case '\n':
                                // forget it
                                return (FALSE);

                            case '"':
                                // allow for \" escape
                                // V0.9.20 (2002-07-12) [umoeller]
                                if (pEnd[-1] != '\\')
                                    // looks goood!
                                    pszValue = strhSubstr(p + 1,    // after quote
                                                          pEnd);
                                    // printf("-->found \"%s\"\n", pszValue);
                            break;
                        }

                        if ((pEnd) && (*pEnd))
                            pEnd++;
                        else
                            break;
                    }
                }
                else
                    // quote before identifier: don't think so
                    return (FALSE);
            }
            break;

            default:
            {
                // regular character:
                // go to the next space
                // this must be the identifier then
    /*
    printf("testing %c%c%c%c%c\n",
            *p, *(p+1), *(p+2), *(p+3), *(p+4));
            */
                PCSZ pEnd = p;          // first non-space character
                while (    (*pEnd)
                        && (*pEnd != ' ')
                        && (*pEnd != '\t')
                        && (*pEnd != '\n')
                      )
                    pEnd++;

                if (    (*pEnd == ' ')
                     || (*pEnd == '\t')
                     || ((pszIdentifier) && (*pEnd == '\n'))
                   )
                {
                    // got it
                    if (!pszIdentifier)
                    {
                        pszIdentifier = strhSubstr(p, pEnd);
                                // printf("  got identifier \"%s\"\n", pszIdentifier);
                    }
                    else
                        pszValue = strhSubstr(p, pEnd);

                    // go on after this
                    p = pEnd;
                }
                else
                    // bull
                    return (FALSE);
            }
            break;
        }

        if ((p) && (*p))
            p++;
        else
            return (FALSE);
    }

    if (pszValue)
    {
        if (!strcmp(pszIdentifier, "H2I_INTERNET_TITLE"))
        {
            G_pcszResourcesOnInternetTitle = pszValue;
        }
        else if (!strcmp(pszIdentifier, "H2I_INTERNET_BODY"))
        {
            G_pcszResourcesOnInternetBody = pszValue;
        }
        else if (!strcmp(pszIdentifier, "H2I_INTERNET_CLICK"))
        {
            G_pcszClickBelow = pszValue;
        }
        else
        {
            PDEFINENODE pMapping = NEW(DEFINENODE);
            pMapping->Tree.ulKey = (ULONG)pszIdentifier;

            // handle escapes V0.9.20 (2002-07-12) [umoeller]
            XSTRING strValue;
            xstrInitSet(&strValue, pszValue);

            ULONG ulOfs;
            ulOfs = 0;
            while (xstrFindReplaceC(&strValue,
                                    &ulOfs,
                                    // replace \" with "
                                    "\\\"",
                                    "\""))
                ;
            ulOfs = 0;
            while (xstrFindReplaceC(&strValue,
                                    &ulOfs,
                                    // replace \r with carriage return
                                    "\\r",
                                    "\r"))
                ;
            ulOfs = 0;
            while (xstrFindReplaceC(&strValue,
                                    &ulOfs,
                                    // replace \n with newline
                                    "\\n",
                                    "\n"))
                ;
            ulOfs = 0;
            while (xstrFindReplaceC(&strValue,
                                    &ulOfs,
                                    // replace \\ with backslash
                                    "\\\\",
                                    "\\"))
                ;

            pszValue = strValue.psz;
                // do not free strValue

            pMapping->pszValue = pszValue;
            pMapping->ulValueLength = strlen(pszValue);

            if (AddDefinition(pMapping))
            {
                if (G_ulVerbosity > 2)
                    printf("  found #define \"%s\" \"%s\"\n",
                           pszIdentifier,
                           pszValue);
                G_cDefines++;
                return (TRUE);
            }
        }
    }

    return (FALSE);
}

/*
 *@@ ParseCHeader:
 *
 */

APIRET ParseCHeader(const char *pcszHeaderFile,
                    PULONG pcDefines)
{
    PSZ pszContents;
    APIRET arc;
    ULONG cDefines = 0;
    ULONG cbRead = 0;
    if (!(arc = doshLoadTextFile(pcszHeaderFile,
                                 &pszContents,
                                 &cbRead)))
    {
        XSTRING strSource;
        xstrInitSet2(&strSource, pszContents, cbRead - 1);
        xstrConvertLineFormat(&strSource, CRLF2LF);

        PSZ p = strSource.psz;

        while (TRUE)
        {
            switch (*p)
            {
                // skip comments
                case '/':
                    switch (*(p+1))
                    {
                        case '/':       // C++ comment
                            // go to the end of the line, below
                            p = strhFindEOL(p + 1, NULL);
                        break;

                        case '*':       // C comment
                            p = strstr(p+2, "*/");
                        break;

                    }
                break;

                case '#':
                    if (!strncmp(p + 1,
                                 "define",
                                 6))
                        if (AddDefine(p + 7))
                            cDefines++;

                    // go to the end of the line,
                    // there can't be anything else
                    p = strhFindEOL(p + 1, NULL);
                break;

                case ' ':
                case '\t':
                    // ignore
                    p++;
                    continue;
                break;
            }

            if (p && (*p))
                p++;
            else
                break;
        }

        xstrClear(&strSource);
    }

    if (!arc)
        *pcDefines = cDefines;

    return (arc);
}

/*
 *@@ main:
 *      program entry point.
 *
 */

int main(int argc, char* argv[])
{
    int     rc = 0;

    /*
     * 1) initialization
     *
     */

    xstrInit(&G_strError, 0);
    xstrInit(&G_strCrashContext, 0);

    xstrcpy(&G_strCrashContext, "Startup", 0);

    TRY_LOUD(excpt1)
    {
        CHAR    szRootFile[CCHMAXPATH] = "";

        BOOL    fShowStatistics = FALSE;

        treeInit(&G_LinkIDsTreeRoot, NULL);
        treeInit(&G_DefinesTreeRoot, NULL);
        lstInit(&G_llFiles2Process, FALSE);

        LINKLIST llIncludes;
        lstInit(&llIncludes, TRUE);         // will hold plain -i filenames from strdup

        /*
         * 2) parse command line
         *
         */

        xstrcpy(&G_strCrashContext, "Parsing command line", 0);

        if (argc < 2)
        {
            Explain(NULL);
            rc = 999;
        }
        else
        {
            SHORT i = 0;
            while (    (i++ < argc - 1)
                    && (!rc)
                  )
            {
                if (argv[i][0] == '-')
                {
                    // option found:
                    SHORT i2;
                    for (i2 = 1;
                         i2 < strlen(argv[i]) && (!rc);
                         i2++)
                    {
                        CHAR cOption = argv[i][i2];
                        switch (cOption)
                        {
                            case 'h':
                            case '?':
                                Explain(NULL);
                                rc = 1;
                            break;

                            case 's':
                                fShowStatistics = TRUE;
                            break;

                            case 'v':
                                i2++;
                                switch (argv[i][i2])
                                {
                                    case '0': G_ulVerbosity = 0; break;
                                    case '1': G_ulVerbosity = 1; break;     // default also
                                    case '3': G_ulVerbosity = 3; break;
                                    case '4': G_ulVerbosity = 4; break;
                                    default: G_ulVerbosity = 2;
                                }
                            break;

                            case 'i':
                                if (strlen(&argv[i][i2+1]))
                                {
                                    lstAppendItem(&llIncludes,
                                                  strdup(&argv[i][i2+1]));
                                }
                                else
                                {
                                    Error(2,
                                          __FILE__, __LINE__, __FUNCTION__,
                                          "No filename specified with -i option.");
                                    rc = 99;
                                }
                                i2 = 999999999;
                            break;

                            case 'r':
                                G_fNoMoveToRoot = TRUE;
                            break;

                            default:  // unknown option
                                Explain("Unknown option '%c'.",
                                        cOption);
                                rc = 999;
                            break;
                        }
                    }
                } // end if (argv[i][0] == '-')
                else
                {
                    /*
                     * collect file names:
                     *
                     */

                    if (!szRootFile[0])
                        strcpy(szRootFile, argv[i]);
                    else
                    {
                        Explain("More than one root file specified.");
                        rc = 999;
                    }
                } // end elseif (argv[i][0] == '-')
            }
        }

        if (!szRootFile[0] && !rc)
        {
            Explain("You have not specified any input file.");
            rc = 999;
        }

        if (!rc)
        {
            if (G_ulVerbosity)
                PrintHeader();

            /*
             * build handlers tree
             *
             */

            treeInit(&G_HandlersTreeRoot, NULL);

            ULONG ul;
            for (ul = 0;
                 ul < ARRAYITEMCOUNT(HandlersList);
                 ul++)
            {
                PHANDLERTREENODE pNode = NEW(HANDLERTREENODE);
                pNode->Tree.ulKey = (ULONG)HandlersList[ul].pcszTag;
                pNode->pHandler = HandlersList[ul].pHandler;
                treeInsert(&G_HandlersTreeRoot,
                           NULL,
                           (TREE*)pNode,
                           fnCompareStrings);
            }

            /*
             * parse includes
             *
             */

            xstrcpy(&G_strCrashContext, "Parsing include files", 0);

            PLISTNODE pNode = lstQueryFirstNode(&llIncludes);
            while (pNode)
            {
                PSZ pszInclude = (PSZ)pNode->pItemData;
                ULONG cDefines = 0;
                if (!(rc = ParseCHeader(pszInclude,
                                        &cDefines)))
                {
                    if (G_ulVerbosity)
                        printf("Found %d valid #define's in \"%s\".\n",
                               cDefines,
                               pszInclude);
                }
                else
                     Error(2,
                           __FILE__, __LINE__, __FUNCTION__,
                           "Error %d opening include file \"%s\".",
                           rc,
                           pszInclude);

                pNode = pNode->pNext;
            }

            /*
             * finally, convert HTML to IPF
             *
             */

            // main buffer:
            XSTRING str;
            xstrInit(&str, 100*1000);

            xstrcpy(&str,
                    ":userdoc.\n",
                    0);
            xstrcat(&str,
                    ":docprof toc=12345.\n",   // let heading levels 1-5 appear in TOC
                    0);
            xstrcat(&str,
                    ".* Converted from HTML to IPF by " H2I_HEADER "\n",
                    0);

            xstrcpy(&G_strCrashContext, "Reading root article", 0);

            GetOrCreateArticle(szRootFile,
                               0,             // current nesting level will be 1 then
                               NULL);

            rc = ProcessFiles(&str);

            xstrcat(&str,
                    ":euserdoc.",
                    0);

            xstrcpy(&G_strCrashContext, "Writing IPF", 0);

            if (!rc)
            {
                CHAR szOutputFile[CCHMAXPATH];
                strcpy(szOutputFile, szRootFile);
                PSZ p = strrchr(szOutputFile, '.');
                if (p)
                    strcpy(p, ".ipf");
                else
                    strcat(szOutputFile, ".ipf");

                ULONG cbWritten = 0;

                XSTRING str2;
                xstrInit(&str2, str.ulLength * 2 / 3);

                p = str.psz;
                CHAR c;
                while (c = *p++)
                {
                    if (c == '\n')
                        xstrcatc(&str2, '\r');
                    xstrcatc(&str2, c);
                }
                xstrcatc(&str2, '\0');

                if (rc = doshWriteTextFile(szOutputFile,
                                           str2.psz,
                                           &cbWritten,
                                           NULL))
                    Error(2,
                          __FILE__, __LINE__, __FUNCTION__,
                          "Error %d writing output file \"%s\".",
                          rc,
                          szOutputFile);
                else
                    if ((fShowStatistics) || (G_ulVerbosity > 1))
                    {
                        printf("%d HTML files were processed\n",
                               lstCountItems(&G_llFiles2Process));
                        printf("%d #defines were active\n",
                               G_cDefines);
                        printf("%d character entities were replaced\n",
                               G_ulReplacements);
                        printf("Output file \"%s\" successfully written, %d bytes\n",
                                szOutputFile,
                                cbWritten);
                    }
            }
        }
    }
    CATCH(excpt1)
    {
        Error(2,
              __FILE__, __LINE__, __FUNCTION__,
              "Exception caught in main(), context: %s",
              G_strCrashContext.psz);
        rc = 1;
    } END_CATCH();

    return (rc);
}

