
/*
 *@@sourcefile xmlview.cpp:
 *      XML viewer, which displays the elements,
 *      attributes, and DTD of any XML document
 *      in a PM container tree view.
 *
 *@@header "xmlview.h"
 *@@added V0.9.5 (2000-08-13) [umoeller]
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
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

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#include <os2.h>

// C library headers
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>

// generic headers
#include "setup.h"

#include "xwpapi.h"

#include "expat\expat.h"

#include "helpers\cnrh.h"
#include "helpers\datetime.h"           // date/time helper routines
#include "helpers\dialog.h"
#include "helpers\dosh.h"
#include "helpers\except.h"
#include "helpers\linklist.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\threads.h"
#include "helpers\tree.h"
#include "helpers\winh.h"
#include "helpers\xstring.h"
#include "helpers\xml.h"

#include "encodings\base.h"

#include "xmlview.h"

#pragma hdrstop

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

HWND        G_hwndMain = NULLHANDLE;
HWND        G_hwndStatusBar = NULLHANDLE;
HWND        G_hwndCnr = NULLHANDLE;

PFNWP       G_pfnwpCnrOrig = NULL,
            G_fnwpFrameOrig = NULL;

const char  *INIAPP              = "XWorkplace:XMLView";
const char  *INIKEY_MAINWINPOS   = "HandleFixWinPos";
const char  *INIKEY_OPENDLG      = "FileOpenDlgPath";

const char  *APPTITLE            = "xmlview";

/*
 *@@ NODERECORD:
 *
 */

typedef struct _NODERECORD
{
    RECORDCORE recc;

} NODERECORD, *PNODERECORD;

/*
 *@@ XMLFILE:
 *
 *@@added V0.9.9 (2001-02-10) [umoeller]
 */

typedef struct _XMLFILE
{
    XSTRING         strContents;
                        // contents of the XML document

    PXMLDOM         pDom;

} XMLFILE, *PXMLFILE;

PXMLFILE     G_pLoadedFile = NULL;

/* ******************************************************************
 *
 *   XML load
 *
 ********************************************************************/

/*
 *@@ winhSetWindowText:
 *      sets a window's text to a variable text. This is
 *      just like WinSetWindowText, but supports printf()
 *      formatting.
 *
 *      This is limited to 2000 characters.
 *
 *@@added V0.9.9 (2001-02-10) [umoeller]
 */

VOID winhSetVarWindowText(HWND hwnd,
                          const char *pcszFormat,     // in: format string (as with printf)
                          ...)                        // in: additional stuff (as with printf)
{
    va_list     args;
    CHAR        sz[2000];

    va_start(args, pcszFormat);
    vsprintf(sz, pcszFormat, args);
    va_end(args);

    WinSetWindowText(hwnd, sz);
}

/*
 *@@ SuperFileDlg:
 *
 */

BOOL SuperFileDlg(HWND hwndOwner,    // in: owner for file dlg
                  PSZ pszFile,       // in: file mask; out: fully q'd filename
                                     //    (should be CCHMAXPATH in size)
                  ULONG flFlags,     // in: any combination of the following:
                                     // -- WINH_FOD_SAVEDLG: save dlg; else open dlg
                                     // -- WINH_FOD_INILOADDIR: load FOD path from INI
                                     // -- WINH_FOD_INISAVEDIR: store FOD path to INI on OK
                  HINI hini,         // in: INI file to load/store last path from (can be HINI_USER)
                  const char *pcszApplication, // in: INI application to load/store last path from
                  const char *pcszKey)        // in: INI key to load/store last path from
{
    FILEDLG fd;
    memset(&fd, 0, sizeof(FILEDLG));
    fd.cbSize = sizeof(FILEDLG);
    fd.fl = FDS_CENTER;

    if (flFlags & WINH_FOD_SAVEDLG)
        fd.fl |= FDS_SAVEAS_DIALOG;
    else
        fd.fl |= FDS_OPEN_DIALOG;

    // default: copy pszFile
    strcpy(fd.szFullFile, pszFile);

    if ( (hini) && (flFlags & WINH_FOD_INILOADDIR) )
    {
        // overwrite with initial directory for FOD from OS2.INI
        if (PrfQueryProfileString(hini,
                                  pcszApplication,
                                  pcszKey,
                                  "",      // default string V0.9.9 (2001-02-10) [umoeller]
                                  fd.szFullFile,
                                  sizeof(fd.szFullFile)-10)
                    >= 2)
        {
            // found: append "\*"
            strcat(fd.szFullFile, "\\");
            strcat(fd.szFullFile, pszFile);
        }
    }

    if (    WinFileDlg(HWND_DESKTOP,    // parent
                       hwndOwner, // owner
                       &fd)
        && (fd.lReturn == DID_OK)
       )
    {
        // save path back?
        if (    (hini)
             && (flFlags & WINH_FOD_INISAVEDIR)
           )
        {
            // get the directory that was used
            PSZ p = strrchr(fd.szFullFile, '\\');
            if (p)
            {
                // contains directory:
                // copy to OS2.INI
                PSZ pszDir = strhSubstr(fd.szFullFile, p);
                if (pszDir)
                {
                    PrfWriteProfileString(hini,
                                          pcszApplication,
                                          pcszKey,
                                          pszDir);
                    free(pszDir);
                }
            }
        }

        strcpy(pszFile, fd.szFullFile);

        return (TRUE);
    }

    return (FALSE);
}

/*
 *@@ GetCPData:
 *      DOM callback for filling in codepage data
 *      for unicode conversion.
 *
 *@@added V0.9.14 (2001-08-09) [umoeller]
 */

int APIENTRY GetCPData(PXMLDOM pDom,
                       ULONG ulCP,
                       int *piMap)
{
    ULONG ul;

    ENCID id;

    XWPENCODINGMAP *pMap = NULL;
    ULONG cEntries = 0;

    id = encFindIdForCodepage(ulCP,
                              NULL,
                              NULL);

    if (    (id != UNSUPPORTED)

         && (encGetTable(id,
                         &pMap,
                         &cEntries))
       )
    {
        for (ul = 0;
             ul < cEntries;
             ul++)
        {
            USHORT usCP = pMap[ul].usCP;        // cp value
            if (usCP < 256)
                piMap[usCP] = (ULONG)pMap[ul].usUni;     // unicode value
        }

        // success
        return (1);
    }

    return (0);
}

/*
 *@@ ParseExternal:
 *
 *@@added V0.9.14 (2001-08-09) [umoeller]
 */

APIRET APIENTRY ParseExternal(PXMLDOM pDom,
                              XML_Parser pSubParser,
                              const char *pcszSystemID,
                              const char *pcszPublicID)
{
    APIRET arc = ERROR_FILE_NOT_FOUND;

    if (pcszSystemID)
    {
        PSZ pszContents = NULL;
        if (!(arc = doshLoadTextFile(pcszSystemID,
                                     &pszContents,
                                     NULL)))
        {
            if (!XML_Parse(pSubParser,
                           pszContents,
                           strlen(pszContents),
                           TRUE))
                arc = -1;

            free(pszContents);
        }
    }

    return (arc);
}

/*
 *@@ LoadXMLFile:
 *      loads and parses an XML file into an XMLFILE
 *      structure, which is created.
 *
 *      If this returns ERROR_DOM_PARSING or
 *      ERROR_DOM_VALIDITY, ppszError
 *      receives an error description, which the
 *      caller must free.
 *
 *@@added V0.9.9 (2001-02-10) [umoeller]
 */

APIRET LoadXMLFile(const char *pcszFilename,    // in: file:/K:\...
                   PXMLFILE *ppXMLFile,         // out: loaded file
                   PSZ *ppszError)     // out: error string
{
    APIRET  arc = NO_ERROR;

    PSZ     pszContents = NULL;

    arc = doshLoadTextFile(pcszFilename,
                           &pszContents,
                           NULL);

    if (arc == NO_ERROR)
    {
        PXMLFILE pFile = (PXMLFILE)malloc(sizeof(XMLFILE));
        if (!pFile)
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            memset(pFile, 0, sizeof(*pFile));

            xstrInitSet(&pFile->strContents, pszContents);

            xstrConvertLineFormat(&pFile->strContents,
                                  CRLF2LF);

            arc = xmlCreateDOM(DF_PARSEDTD | DF_DROP_WHITESPACE,
                               NULL,  // V0.9.20 (2002-07-17) [pr]
                               0,
                               GetCPData,
                               ParseExternal,
                               NULL,
                               &pFile->pDom);
            if (arc == NO_ERROR)
            {
                arc = xmlParse(pFile->pDom,
                               pFile->strContents.psz,
                               pFile->strContents.ulLength,
                               TRUE);       // last

                if (arc == ERROR_DOM_PARSING)
                {
                    CHAR    sz[1000];
                    sprintf(sz,
                            "Parsing error: %s (line %d, column %d)",
                            pFile->pDom->pcszErrorDescription,
                            pFile->pDom->ulErrorLine,
                            pFile->pDom->ulErrorColumn);

                    if (pFile->pDom->pxstrFailingNode)
                        sprintf(sz + strlen(sz),
                                " (%s)",
                                pFile->pDom->pxstrFailingNode->psz);

                    *ppszError = strdup(sz);
                }
                else if (arc == ERROR_DOM_VALIDITY)
                {
                    CHAR    sz[1000];
                    sprintf(sz,
                            "Validation error: %s (line %d, column %d)",
                            pFile->pDom->pcszErrorDescription,
                            pFile->pDom->ulErrorLine,
                            pFile->pDom->ulErrorColumn);

                    if (pFile->pDom->pxstrFailingNode)
                        sprintf(sz + strlen(sz),
                                " (%s)",
                                pFile->pDom->pxstrFailingNode->psz);

                    *ppszError = strdup(sz);
                }
            }

            if (arc == NO_ERROR)
                *ppXMLFile = pFile;
            else
                // error:
                free(pFile);
        }
    }

    return (arc);
}

/*
 *@@ FreeXMLFile:
 *
 */

VOID FreeXMLFile(PXMLFILE pFile)
{
    xmlFreeDOM(pFile->pDom);
}

/* ******************************************************************
 *
 *   XML view
 *
 ********************************************************************/

/*
 *@@ INSERTSTACK:
 *
 */

typedef struct _INSERTSTACK
{
    PNODERECORD     pParentRecord;
    PXMLDOM         pDom;
} INSERTSTACK, *PINSERTSTACK;

/*
 *@@ AppendUTF8:
 *
 *@@added V0.9.14 (2001-08-09) [umoeller]
 */

VOID AppendUTF8(PCONVERSION pCodec,
                PXSTRING pstrTarget,
                PXSTRING pstrUTF8)
{
    const char *p;
    if (p = pstrUTF8->psz)
    {
        ULONG ulChar;
        while (ulChar = encDecodeUTF8(&p))
            xstrcatc(pstrTarget,
                     (CHAR)encUni2Char(pCodec, ulChar));
    }
}

/*
 *@@ InsertAttribDecls:
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

VOID XWPENTRY InsertAttribDecls(PCONVERSION pCodec,
                                TREE *t,        // in: PCMATTRIBUTEDECL really
                                PVOID pUser)    // in: PINSERTSTACK user param
{
    PCMATTRIBUTEDECL pAttribDecl = (PCMATTRIBUTEDECL)t;
    PINSERTSTACK pInsertStack = (PINSERTSTACK)pUser;

    PNODERECORD prec = (PNODERECORD)cnrhAllocRecords(G_hwndCnr,
                                                     sizeof(NODERECORD),
                                                     1);

    XSTRING str;
    xstrInitCopy(&str, "attrib: \"", 0);
    xstrcats(&str, &pAttribDecl->NodeBase.strNodeName);
    xstrcat(&str, "\", type: ", 0);
    switch (pAttribDecl->ulAttrType)
    {
        case CMAT_ENUM:
        {
            xstrcat(&str, "ENUM(", 0);
            PNODEBASE pNode = (PNODEBASE)treeFirst(pAttribDecl->ValuesTree);
            while (pNode)
            {
                AppendUTF8(pCodec, &str, &pNode->strNodeName);
                // xstrcats(&str, &pNode->strNodeName);
                xstrcatc(&str, '|');

                pNode = (PNODEBASE)treeNext(&pNode->Tree);
            }
            /* treeTraverse(pAttribDecl->ValuesTree,
                         CatAttribEnums,
                         &str,
                         0); */
            *(str.psz + str.ulLength - 1) = ')';
        break; }

        case CMAT_CDATA:
            xstrcat(&str, "CDATA", 0);
        break;
        case CMAT_ID:
            xstrcat(&str, "ID", 0);
        break;
        case CMAT_IDREF:
            xstrcat(&str, "IDREF", 0);
        break;
        case CMAT_IDREFS:
            xstrcat(&str, "IDREFS", 0);
        break;
        case CMAT_ENTITY:
            xstrcat(&str, "ENTITY", 0);
        break;
        case CMAT_ENTITIES:
            xstrcat(&str, "ENTITIES", 0);
        break;
        case CMAT_NMTOKEN:
            xstrcat(&str, "NMTOKEN", 0);
        break;
        case CMAT_NMTOKENS:
            xstrcat(&str, "NMTOKENS", 0);
        break;
    }

    cnrhInsertRecords(G_hwndCnr,
                      (PRECORDCORE)pInsertStack->pParentRecord,
                      (PRECORDCORE)prec,
                      TRUE,
                      str.psz,
                      CRA_RECORDREADONLY,
                      1);
}

/*
 *@@ InsertEDParticle:
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

PNODERECORD InsertEDParticle(PCMELEMENTPARTICLE pParticle,
                             PNODERECORD pParentRecord)
{
    PNODERECORD prec = (PNODERECORD)cnrhAllocRecords(G_hwndCnr,
                                                     sizeof(NODERECORD),
                                                     1);

    XSTRING str;
    xstrInitCopy(&str, "\"", 0);
    xstrcats(&str, &pParticle->NodeBase.strNodeName);

    switch (pParticle->ulRepeater)
    {
        // XML_CQUANT_NONE

        case XML_CQUANT_OPT:
            xstrcatc(&str, '?');
        break;

        case XML_CQUANT_REP:
            xstrcatc(&str, '*');
        break;

        case XML_CQUANT_PLUS:
            xstrcatc(&str, '+');
        break;
    }

    const char *pcsz = "unknown";
    switch (pParticle->NodeBase.ulNodeType)
    {
        case ELEMENTPARTICLE_EMPTY:
            pcsz = "\" EMPTY";
        break;

        case ELEMENTPARTICLE_ANY:
            pcsz = "\" ANY";
        break;

        case ELEMENTPARTICLE_MIXED:
            pcsz = "\" MIXED";
        break;

        case ELEMENTPARTICLE_CHOICE:
            pcsz = "\" CHOICE";
        break;

        case ELEMENTPARTICLE_SEQ:
            pcsz = "\" SEQ";
        break;

        case ELEMENTPARTICLE_NAME:        // subtypes
            pcsz = "\" NAME";
        break;
    }

    xstrcat(&str, pcsz, 0);

    cnrhInsertRecords(G_hwndCnr,
                      (PRECORDCORE)pParentRecord,
                      (PRECORDCORE)prec,
                      TRUE,
                      str.psz,
                      CRA_RECORDREADONLY,
                      1);

    if (pParticle->pllSubNodes)
    {
        PLISTNODE pNode = lstQueryFirstNode(pParticle->pllSubNodes);
        while (pNode)
        {
            PCMELEMENTPARTICLE pSub = (PCMELEMENTPARTICLE)pNode->pItemData;
            InsertEDParticle(pSub,
                             prec);

            pNode = pNode->pNext;
        }
    }

    return (prec);
}

/*
 *@@ InsertElementDecls:
 *      tree traversal function initiated from InsertDocType.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

VOID XWPENTRY InsertElementDecls(TREE *t,           // in: an CMELEMENTDECLNODE really
                                 PVOID pUser)       // in: PINSERTSTACK user param
{
}

/*
 *@@ InsertDocType:
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

VOID InsertDocType(PCONVERSION pCodec,
                   PXMLDOM pDom,
                   PDOMDOCTYPENODE pDocTypeNode,
                   PNODERECORD pParentRecord)
{
    // INSERTSTACK InsertStack = {pParentRecord, pDom};

    TREE *t = treeFirst(pDocTypeNode->ElementDeclsTree);
    while (t)
    {
        PCMELEMENTDECLNODE pElementDecl = (PCMELEMENTDECLNODE)t;

        // insert the element declaration root particle and subparticles
        // (this recurses)
        PNODERECORD pElementRec = InsertEDParticle(&pElementDecl->Particle,
                                                   pParentRecord);

        // now insert attribute decls, if any
        PCMATTRIBUTEDECLBASE pAttribDeclBase
            = xmlFindAttribDeclBase(pDom,
                                    &pElementDecl->Particle.NodeBase.strNodeName);
        if (pAttribDeclBase)
        {
            // traverse the attributes tree as well
            INSERTSTACK Stack2 = {pElementRec, pDom};

            TREE *t = treeFirst(pAttribDeclBase->AttribDeclsTree);
            while (t)
            {
                InsertAttribDecls(pCodec,
                                  t,
                                  &Stack2);
                t = treeNext(t);
            }
            /* treeTraverse(pAttribDeclBase->AttribDeclsTree,
                         InsertAttribDecls,
                         &Stack2,       // user param
                         0); */
        }

        /* InsertAttribDecls(t,
                          &InsertStack); */
        t = treeNext(t);
    }

    /* treeTraverse(pDocTypeNode->ElementDeclsTree,
                 InsertElementDecls,
                 &InsertStack,         // user param
                 0); */
}

VOID InsertDom(PCONVERSION pCodec,
               PXMLDOM pDom,
               PDOMNODE pDomNode,
               PNODERECORD pParentRecord);

/*
 *@@ InsertDom:
 *
 *      Note: We get the document node first, which we must
 *      not insert, but only its children.
 */

VOID InsertDom(PCONVERSION pCodec,
               PXMLDOM pDom,
               PDOMNODE pDomNode,
               PNODERECORD pParentRecord)       // initially NULL
{
    BOOL        fInsertChildren = FALSE;
    PNODERECORD prec = NULL;
    PDOMDOCTYPENODE pDocType = NULL;

    XSTRING     str;
    xstrInit(&str, 100);

    switch (pDomNode->NodeBase.ulNodeType)
    {
        case DOMNODE_ELEMENT:
            // xstrcpys(&str, &pDomNode->NodeBase.strNodeName);
            AppendUTF8(pCodec, &str, &pDomNode->NodeBase.strNodeName);
        break;

        case DOMNODE_ATTRIBUTE:
            AppendUTF8(pCodec, &str, &pDomNode->NodeBase.strNodeName);
            // xstrcpys(&str, &pDomNode->NodeBase.strNodeName);
            xstrcat(&str, "=\"", 2);
            // xstrcats(&str, pDomNode->pstrNodeValue);
            AppendUTF8(pCodec, &str, pDomNode->pstrNodeValue);
            xstrcatc(&str, '\"');
        break;

        case DOMNODE_TEXT:
            xstrcpy(&str, "\"", 1);
            // xstrcats(&str, pDomNode->pstrNodeValue);
            AppendUTF8(pCodec, &str, pDomNode->pstrNodeValue);
            xstrcatc(&str, '\"');
        break;

        case DOMNODE_DOCUMENT:
            xstrcpy(&str, "Document", 0);
            xstrcat(&str, " \"", 0);
            // xstrcats(&str, &pDomNode->NodeBase.strNodeName);
            AppendUTF8(pCodec, &str, &pDomNode->NodeBase.strNodeName);
            xstrcat(&str, "\"", 0);
        break;

        case DOMNODE_DOCUMENT_TYPE:
        {
            pDocType = (PDOMDOCTYPENODE)pDomNode;
            xstrcpy(&str, "DOCTYPE system: \"", 0);
            // xstrcats(&str, &pDocType->strSystemID);
            AppendUTF8(pCodec, &str, &pDocType->strSystemID);
            xstrcat(&str, "\", public: \"", 0);
            // xstrcats(&str, &pDocType->strPublicID);
            AppendUTF8(pCodec, &str, &pDocType->strPublicID);
            xstrcatc(&str, '\"');
        break; }

        default:
        {
            CHAR sz[1000];
            sprintf(sz, "Unknown node type %d", pDomNode->NodeBase.ulNodeType);
            xstrcpy(&str, sz, 0);

        break; }
    }

    if (str.ulLength)
    {
        prec = (PNODERECORD)cnrhAllocRecords(G_hwndCnr,
                                             sizeof(NODERECORD),
                                             1);

        cnrhInsertRecords(G_hwndCnr,
                          (PRECORDCORE)pParentRecord,
                          (PRECORDCORE)prec,
                          TRUE,
                          str.psz,
                          CRA_RECORDREADONLY,
                          1);
        fInsertChildren = TRUE;
    }

    if (fInsertChildren)
    {
        // insert attributes
        INSERTSTACK Stack2 = {prec, pDom};
        PDOMNODE pAttrNode = (PDOMNODE)treeFirst(pDomNode->AttributesMap);
        while (pAttrNode)
        {
            InsertDom(pCodec,
                      pDom,
                      pAttrNode,
                      prec);
            pAttrNode = (PDOMNODE)treeNext((TREE*)pAttrNode);
        }

        // and children
        PLISTNODE pSubNode = lstQueryFirstNode(&pDomNode->llChildren);
        while (pSubNode)
        {
            PDOMNODE pDomSubnode = (PDOMNODE)pSubNode->pItemData;
            InsertDom(pCodec,
                      pDom,
                      pDomSubnode,
                      prec);

            pSubNode = pSubNode->pNext;
        }
    }
    else
        xstrClear(&str);

    if (prec && pDocType)
        InsertDocType(pCodec, pDom, pDocType, prec);
}

/* ******************************************************************
 *
 *   Container window proc
 *
 ********************************************************************/

/*
 *@@ LoadAndInsert:
 *
 */

VOID LoadAndInsert(const char *pcszFile)
{
    PSZ pszError;
    APIRET  arc = LoadXMLFile(pcszFile,
                              &G_pLoadedFile,
                              &pszError);
    if (    (arc == ERROR_DOM_PARSING)
         || (arc == ERROR_DOM_VALIDITY)
       )
    {
        winhSetVarWindowText(G_hwndStatusBar,
                             pszError);
        free(pszError);
    }
    else if (arc != NO_ERROR)
    {
        winhSetVarWindowText(G_hwndStatusBar,
                             "Error %d loading \"%s\".",
                             arc,
                             pcszFile);
    }
    else
    {
        winhSetVarWindowText(G_hwndStatusBar,
                          "File \"%s\" loaded, %d bytes",
                          pcszFile,
                          G_pLoadedFile->strContents.ulLength);

        cnrhRemoveAll(G_hwndCnr);

        // create codec for process codepage
        ULONG acp[8];       // fixed V0.9.19 (2002-04-14) [umoeller], this needs an array
        ULONG cb = 0;
        APIRET arcCP;
        ENCID id;
        PCONVERSION pCodec;
        if (    (!(arcCP = DosQueryCp(sizeof(acp),
                                      acp,
                                      &cb)))
             && (id = encFindIdForCodepage(acp[0], NULL, NULL))
             && (pCodec = encCreateCodec(id))
           )
        {
            InsertDom(pCodec,
                      G_pLoadedFile->pDom,
                      (PDOMNODE)G_pLoadedFile->pDom->pDocumentNode,
                      NULL);
            encFreeCodec(&pCodec);
        }
    }
}

/*
 *@@ fnwpSubclassedCnr:
 *
 */

MRESULT EXPENTRY fnwpSubclassedCnr(HWND hwndCnr, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_CLOSE:
            winhSaveWindowPos(G_hwndMain,
                              HINI_USER,
                              INIAPP,
                              INIKEY_MAINWINPOS);
            mrc = G_pfnwpCnrOrig(hwndCnr, msg, mp1, mp2);
        break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case IDMI_OPEN:
                {
                    CHAR    szFile[CCHMAXPATH] = "*.xml";
                    if (SuperFileDlg(G_hwndMain,
                                     szFile,
                                     WINH_FOD_INILOADDIR | WINH_FOD_INISAVEDIR,
                                     HINI_USER,
                                     INIAPP,
                                     INIKEY_OPENDLG))
                        LoadAndInsert(szFile);
                break; }

                case IDMI_EXIT:
                    WinPostMsg(G_hwndMain,
                               WM_SYSCOMMAND,
                               (MPARAM)SC_CLOSE,
                               0);
                break;
            }
        break;

        default:
            mrc = G_pfnwpCnrOrig(hwndCnr, msg, mp1, mp2);
    }

    return (mrc);
}

/*
 *@@ SetupCnr:
 *
 */

VOID SetupCnr(HWND hwndCnr)
{
    BEGIN_CNRINFO()
    {
        // switch view
        cnrhSetView(CV_TREE | CA_TREELINE | CV_TEXT);
        cnrhSetTreeIndent(30);
    } END_CNRINFO(hwndCnr)

    winhSetWindowFont(hwndCnr, NULL);
}

/* ******************************************************************
 *
 *   Frame window proc
 *
 ********************************************************************/

/*
 *@@ winhCreateStatusBar:
 *
 *@@added V0.9.5 (2000-08-13) [umoeller]
 */

HWND winhCreateStatusBar(HWND hwndFrame,
                         HWND hwndOwner,
                         USHORT usID,
                         PSZ pszFont,
                         LONG lColor)
{
    // create status bar
    HWND        hwndReturn = NULLHANDLE;
    PPRESPARAMS ppp = NULL;
    winhStorePresParam(&ppp, PP_FONTNAMESIZE, strlen(pszFont)+1, pszFont);
    lColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0);
    winhStorePresParam(&ppp, PP_BACKGROUNDCOLOR, sizeof(lColor), &lColor);
    lColor = CLR_BLACK;
    winhStorePresParam(&ppp, PP_FOREGROUNDCOLOR, sizeof(lColor), &lColor);
    hwndReturn = WinCreateWindow(G_hwndMain,
                                  WC_STATIC,
                                  "Welcome to xmlview, built " __DATE__,
                                  SS_TEXT | DT_VCENTER | WS_VISIBLE,
                                  0, 0, 0, 0,
                                  hwndOwner,
                                  HWND_TOP,
                                  usID,
                                  NULL,
                                  ppp);
    free(ppp);
    return (hwndReturn);
}

static PRECORDCORE s_preccExpanded = NULL;

/*
 *@@ winh_fnwpFrameWithStatusBar:
 *      subclassed frame window proc.
 *
 *@@added V0.9.5 (2000-08-13) [umoeller]
 */

MRESULT EXPENTRY winh_fnwpFrameWithStatusBar(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_QUERYFRAMECTLCOUNT:
        {
            // query the standard frame controls count
            ULONG ulrc = (ULONG)(G_fnwpFrameOrig(hwndFrame, msg, mp1, mp2));

            // if we have a status bar, increment the count
            ulrc++;

            mrc = (MPARAM)ulrc;
        break; }

        case WM_FORMATFRAME:
        {
            //  query the number of standard frame controls
            ULONG ulCount = (ULONG)(G_fnwpFrameOrig(hwndFrame, msg, mp1, mp2));

            // we have a status bar:
            // format the frame
            ULONG       ul;
            PSWP        swpArr = (PSWP)mp1;

            for (ul = 0; ul < ulCount; ul++)
            {
                if (WinQueryWindowUShort( swpArr[ul].hwnd, QWS_ID ) == 0x8008 )
                                                                 // FID_CLIENT
                {
                    POINTL      ptlBorderSizes;
                    ULONG       ulStatusBarHeight = 20;
                    WinSendMsg(hwndFrame,
                               WM_QUERYBORDERSIZE,
                               (MPARAM)&ptlBorderSizes,
                               0);

                    // first initialize the _new_ SWP for the status bar.
                    // Since the SWP array for the std frame controls is
                    // zero-based, and the standard frame controls occupy
                    // indices 0 thru ulCount-1 (where ulCount is the total
                    // count), we use ulCount for our static text control.
                    swpArr[ulCount].fl = SWP_MOVE | SWP_SIZE | SWP_NOADJUST | SWP_ZORDER;
                    swpArr[ulCount].x  = ptlBorderSizes.x;
                    swpArr[ulCount].y  = ptlBorderSizes.y;
                    swpArr[ulCount].cx = swpArr[ul].cx;  // same as cnr's width
                    swpArr[ulCount].cy = ulStatusBarHeight;
                    swpArr[ulCount].hwndInsertBehind = HWND_BOTTOM; // HWND_TOP;
                    swpArr[ulCount].hwnd = G_hwndStatusBar;

                    // adjust the origin and height of the container to
                    // accomodate our static text control
                    swpArr[ul].y  += swpArr[ulCount].cy;
                    swpArr[ul].cy -= swpArr[ulCount].cy;
                }
            }

            // increment the number of frame controls
            // to include our status bar
            mrc = (MRESULT)(ulCount + 1);
        break; }

        case WM_CALCFRAMERECT:
        {
            mrc = G_fnwpFrameOrig(hwndFrame, msg, mp1, mp2);

            // we have a status bar: calculate its rectangle
            // CalcFrameRect(mp1, mp2);
        break; }

        case WM_CONTROL:
            if (SHORT1FROMMP(mp1) == FID_CLIENT)
            {
                switch (SHORT2FROMMP(mp1))
                {
                    case CN_EXPANDTREE:
                        s_preccExpanded = (PRECORDCORE)mp2;
                        WinStartTimer(WinQueryAnchorBlock(hwndFrame),
                                      hwndFrame,
                                      1,
                                      100);
                    break;
                }
            }
        break;

        case WM_TIMER:
            // stop timer (it's just for one shot)
            WinStopTimer(WinQueryAnchorBlock(hwndFrame),
                         hwndFrame,
                         (ULONG)mp1);       // timer ID

            switch ((ULONG)mp1)        // timer ID
            {
                case 1:
                {
                    if (s_preccExpanded->flRecordAttr & CRA_EXPANDED)
                    {
                        HWND hwndCnr = WinWindowFromID(hwndFrame, FID_CLIENT);
                        PRECORDCORE     preccLastChild;
                        // scroll the tree view properly
                        preccLastChild = (PRECORDCORE)WinSendMsg(hwndCnr,
                                                    CM_QUERYRECORD,
                                                    s_preccExpanded,
                                                            // expanded PRECORDCORE from CN_EXPANDTREE
                                                    MPFROM2SHORT(CMA_LASTCHILD,
                                                                 CMA_ITEMORDER));
                        if (preccLastChild)
                        {
                            // ULONG ulrc;
                            cnrhScrollToRecord(hwndCnr,
                                    (PRECORDCORE)preccLastChild,
                                    CMA_TEXT,   // record text rectangle only
                                    TRUE);      // keep parent visible
                        }
                    }
                break; }

            }
        break;

        default:
            mrc = G_fnwpFrameOrig(hwndFrame, msg, mp1, mp2);
    }

    return (mrc);
}

/* ******************************************************************
 *
 *   main
 *
 ********************************************************************/

/* typedef struct _CONTROLDEF
{
    const char  *pcszClass;         // registered PM window class
    const char  *pcszText;          // window text (class-specific);
                                    // NULL for tables

    ULONG       flStyle;

    ULONG       ulID;

    USHORT      usAdjustPosition;
            // flags for winhAdjustControls; any combination of
            // XAC_MOVEX, XAC_MOVEY, XAC_SIZEX, XAC_SIZEY

    SIZEL       szlControl;         // proposed size
    ULONG       ulSpacing;          // spacing around control

} CONTROLDEF, *PCONTROLDEF; */

static CONTROLDEF
            Static1 =
                    {
                        WC_STATIC,
                        "Test text row 1 column 1",
                        WS_VISIBLE | SS_TEXT | DT_LEFT | DT_TOP,
                        0,
                        CTL_COMMON_FONT,
                        { 300, 30 },
                        5,
                        NULL
                    },
            Static2 =
                    {
                        WC_STATIC,
                        "Test text row 1 column 2",
                        WS_VISIBLE | SS_TEXT | DT_LEFT | DT_TOP,
                        0,
                        CTL_COMMON_FONT,
                        { 300, 30 },
                        5,
                        NULL
                    },
            Cnr =
                    {
                        WC_CONTAINER,
                        NULL,
                        WS_VISIBLE | WS_TABSTOP,
                        0,
                        CTL_COMMON_FONT,
                        { 100, 100 },
                        5,
                        NULL
                    },
            CnrGroup =
                    {
                        WC_STATIC,
                        "Container1",
                        WS_VISIBLE | SS_GROUPBOX | DT_MNEMONIC,
                        0,
                        CTL_COMMON_FONT,
                        { 100, 100 },       // ignored
                        5,
                        NULL
                    },
            OKButton =
                    {
                        WC_BUTTON,
                        "~OK",
                        WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_DEFAULT,
                        0,
                        CTL_COMMON_FONT,
                        { SZL_AUTOSIZE, 30 },
                        5,
                        NULL
                    },
            CancelButton =
                    {
                        WC_BUTTON,
                        "~Cancel",
                        WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                        0,
                        CTL_COMMON_FONT,
                        { SZL_AUTOSIZE, 30 },
                        5,
                        NULL
                    };

static _DLGHITEM aDlgItems[] =
                {
                    START_TABLE,
                        START_ROW(0),
                            CONTROL_DEF(&Static1),
                        START_ROW(0),
                            START_GROUP_TABLE(&CnrGroup),
                                START_ROW(0),
                                    CONTROL_DEF(&Cnr),
                            END_TABLE,
                            START_GROUP_TABLE(&CnrGroup),
                                START_ROW(0),
                                    CONTROL_DEF(&Cnr),
                                START_ROW(0),
                                    CONTROL_DEF(&OKButton),
                                START_ROW(0),
                                    CONTROL_DEF(&CancelButton),
                            END_TABLE,
                        START_ROW(0),
                            CONTROL_DEF(&Static2),

                        START_ROW(0),
                            CONTROL_DEF(&OKButton),
                            CONTROL_DEF(&CancelButton),
                    END_TABLE
                };

/*
 *@@ main:
 *
 */

int main(int argc, char* argv[])
{
    HAB         hab;
    HMQ         hmq;
    QMSG        qmsg;

    if (!(hab = WinInitialize(0)))
        return FALSE;

    if (!(hmq = WinCreateMsgQueue(hab, 0)))
        return FALSE;

    winhInitGlobals();      // V1.0.1 (2002-11-30) [umoeller]

    DosError(FERR_DISABLEHARDERR | FERR_ENABLEEXCEPTION);

    /* HWND hwndDlg = NULLHANDLE;
    APIRET arc = dlghCreateDlg(&hwndDlg,
                               NULLHANDLE,      // owner
                               FCF_TITLEBAR | FCF_SYSMENU | FCF_DLGBORDER | FCF_NOBYTEALIGN,
                               WinDefDlgProc,
                               "DemoDlg",
                               aDlgItems,
                               ARRAYITEMCOUNT(aDlgItems),
                               NULL,
                               "9.WarpSans");
    if (arc == NO_ERROR)
    {
        WinProcessDlg(hwndDlg);
        WinDestroyWindow(hwndDlg);
    }
    else
    {
        CHAR szErr[100];
        sprintf(szErr, "Error %d", arc);
        winhDebugBox(NULLHANDLE, "Error", szErr);
    } */

    // create frame and container
    G_hwndMain = winhCreateStdWindow(HWND_DESKTOP,
                                     0,
                                     FCF_TITLEBAR
                                        | FCF_SYSMENU
                                        | FCF_MINMAX
                                        | FCF_SIZEBORDER
                                        | FCF_ICON
                                        | FCF_MENU
                                        | FCF_TASKLIST,
                                     0,
                                     APPTITLE,      // xmlview
                                     1,     // icon resource
                                     WC_CONTAINER,
                                     CCS_MINIICONS | CCS_READONLY | CCS_EXTENDSEL
                                        | WS_VISIBLE,
                                     0,
                                     NULL,
                                     &G_hwndCnr);

    if ((G_hwndMain) && (G_hwndCnr))
    {
        // subclass cnr (it's our client)
        G_pfnwpCnrOrig = WinSubclassWindow(G_hwndCnr, fnwpSubclassedCnr);

        // create status bar as child of the frame
        G_hwndStatusBar = winhCreateStatusBar(G_hwndMain,
                                              G_hwndCnr,
                                              0,
                                              "9.WarpSans",
                                              CLR_BLACK);
        // subclass frame for supporting status bar and msgs
        G_fnwpFrameOrig = WinSubclassWindow(G_hwndMain,
                                            winh_fnwpFrameWithStatusBar);

        SetupCnr(G_hwndCnr);

        if (!winhRestoreWindowPos(G_hwndMain,
                                  HINI_USER,
                                  INIAPP,
                                  INIKEY_MAINWINPOS,
                                  SWP_SHOW | SWP_ACTIVATE | SWP_MOVE | SWP_SIZE))
            WinSetWindowPos(G_hwndMain,
                            HWND_TOP,
                            10, 10, 500, 500,
                            SWP_SHOW | SWP_ACTIVATE | SWP_MOVE | SWP_SIZE);

        if (argc > 1)
            LoadAndInsert(argv[1]);

        //  standard PM message loop
        while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
        {
            WinDispatchMsg(hab, &qmsg);
        }
    }

    // clean up on the way out
    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

    return (0);
}

