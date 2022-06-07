
/*
 *@@sourcefile xml.c:
 *      XML document handling.
 *
 *      XML support in the XWorkplace Helpers is broken into two
 *      layers:
 *
 *      --  The bottom layer is implemented by the @expat parser,
 *          which I have ported and hacked to the xwphelpers.
 *
 *          See xmlparse.c for an introduction.
 *
 *      --  Because expat requires so many callbacks and is non-validating,
 *          I have added a top layer above the expat library
 *          which is vaguely modelled after the Document Object Model
 *          (DOM) standardized by the W3C. That's this file.
 *
 *          This top layer allows you to do two things VERY EASILY:
 *
 *          1)  Parse an XML document (which uses expat internally)
 *              and build a DOM tree from that. See xmlCreateDOM.
 *
 *          2)  Create a DOM tree in memory and write an XML
 *              document from that. See xmlCreateDocument.
 *
 *      <B>XML</B>
 *
 *      In order to understand XML myself, I have written a couple of
 *      glossary entries for the complex XML terminology. See @XML
 *      for a start.
 *
 *      <B>Document Object Model (DOM)</B>
 *
 *      See @DOM for a general introduction.
 *
 *      DOM really calls for object oriented programming so the various
 *      structs can inherit from each other. Since this implementation
 *      was supposed to be a C-only interface, we cannot implement
 *      inheritance at the language level. Instead, each XML document
 *      is broken up into a tree of node structures only (see DOMNODE),
 *      each of which has a special type. The W3C DOM allows this
 *      (and calls this the "flattened" view, as opposed to the
 *      "inheritance view").
 *
 *      The W3C DOM specification prescribes tons of methods, which I
 *      really had no use for, so I didn't implement them. This implementation
 *      is only a DOM insofar as it uses nodes which represent @documents,
 *      @elements, @attributes, @comments, and @processing_instructions.
 *
 *      Most notably, there are the following differences:
 *
 *      --  External entities don't work yet. As a result, DOCTYPE's
 *          only make sense if the entire DTD is in the same document
 *          (internal subset).
 *
 *      --  Not all node types are implemented. See DOMNODE for
 *          the supported types.
 *
 *      --  Only a subset of the standardized methods is implemented,
 *          and they are called differently to adhere to the xwphelpers
 *          conventions.
 *
 *      --  DOM uses UTF-16 for its DOMString type. @expat gives UTF-8
 *          strings to all the handlers though, so all data in the DOM nodes
 *          is UTF-8 encoded. This still needs to be fixed.
 *
 *      --  DOM defines the DOMException class. This isn't supported in C.
 *          Instead, we use special error codes which add to the standard
 *          OS/2 error codes (APIRET). All our error codes are >= 40000
 *          to avoid conflicts.
 *
 *      It shouldn't be too difficult to write a C++ encapsulation
 *      of this though which fully implements all the DOM methods.
 *
 *      However, we do implement node management as in the standard.
 *      See xmlCreateDomNode and xmlDeleteNode.
 *
 *      The main entry point into this is xmlCreateDOM. See remarks
 *      there for how this will be typically used.
 *
 *      <B>Validation</B>
 *
 *      @expat doesn't check XML documents for whether they are @valid.
 *      In other words, expat is a non-validating XML processor.
 *
 *      By contrast, this pseudo-DOM implementation can validate to
 *      a certain extent.
 *
 *      -- If you pass DF_PARSEDTD to xmlCreateDOM, the DTD will be
 *         parsed and the document will be validated against it.
 *         Validation is working as far as elements and attributes
 *         are checked for proper nesting. However, we cannot fully
 *         check for proper ordering etc. in (children) mode of
 *         @element_declarations. This will only check for whether
 *         elements may appear in another element at all -- not for
 *         the correct order.
 *
 *      -- Otherwise the @DTD entries will not be stored in the DOM
 *         nodes, and no validation occurs. Still, if a DTD exists,
 *         @expat will insert attributes that have a default value
 *         in their @attribute declaraion and have not been specified.
 *
 *@@header "helpers\xml.h"
 *@@added V0.9.6 (2000-10-29) [umoeller]
 */

/*
 *      Copyright (C) 2000-2002 Ulrich M”ller.
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

#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "setup.h"                      // code generation and debugging options

#include "expat\expat.h"

#include "helpers\linklist.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\tree.h"
#include "helpers\xstring.h"
#include "helpers\xml.h"

#pragma hdrstop

/*
 *@@category: Helpers\XML
 *      see xml.c.
 */

/*
 *@@category: Helpers\XML\Document Object Model (DOM)
 *      see xml.c.
 */

/* ******************************************************************
 *
 *   Error handling
 *
 ********************************************************************/

/*
 *@@ xmlDescribeError:
 *      returns a string describing the error corresponding to code.
 *      The code should be one of the enums that can be returned from
 *      XML_GetErrorCode.
 *
 *@@changed V0.9.9 (2001-02-14) [umoeller]: adjusted for new error codes
 *@@changed V0.9.9 (2001-02-16) [umoeller]: moved this here from xmlparse.c
 */

const char* xmlDescribeError(int code)
{
    switch (code)
    {
        // start of expat (parser) errors
        case ERROR_EXPAT_NO_MEMORY:
            return "Out of memory";

        case ERROR_EXPAT_SYNTAX:
            return "Syntax error";
        case ERROR_EXPAT_NO_ELEMENTS:
            return "No element found";
        case ERROR_EXPAT_INVALID_TOKEN:
            return "Not well-formed (invalid token)";
        case ERROR_EXPAT_UNCLOSED_TOKEN:
            return "Unclosed token";
        case ERROR_EXPAT_PARTIAL_CHAR:
            return "Unclosed token";
        case ERROR_EXPAT_TAG_MISMATCH:
            return "Mismatched tag";
        case ERROR_EXPAT_DUPLICATE_ATTRIBUTE:
            return "Duplicate attribute";
        case ERROR_EXPAT_JUNK_AFTER_DOC_ELEMENT:
            return "Junk after root element";
        case ERROR_EXPAT_PARAM_ENTITY_REF:
            return "Illegal parameter entity reference";
        case ERROR_EXPAT_UNDEFINED_ENTITY:
            return "Undefined entity";
        case ERROR_EXPAT_RECURSIVE_ENTITY_REF:
            return "Recursive entity reference";
        case ERROR_EXPAT_ASYNC_ENTITY:
            return "Asynchronous entity";
        case ERROR_EXPAT_BAD_CHAR_REF:
            return "Reference to invalid character number";
        case ERROR_EXPAT_BINARY_ENTITY_REF:
            return "Reference to binary entity";
        case ERROR_EXPAT_ATTRIBUTE_EXTERNAL_ENTITY_REF:
            return "Reference to external entity in attribute";
        case ERROR_EXPAT_MISPLACED_XML_PI:
            return "XML processing instruction not at start of external entity";
        case ERROR_EXPAT_UNKNOWN_ENCODING:
            return "Unknown encoding";
        case ERROR_EXPAT_INCORRECT_ENCODING:
            return "Encoding specified in XML declaration is incorrect";
        case ERROR_EXPAT_UNCLOSED_CDATA_SECTION:
            return "Unclosed CDATA section";
        case ERROR_EXPAT_EXTERNAL_ENTITY_HANDLING:
            return "Error in processing external entity reference";
        case ERROR_EXPAT_NOT_STANDALONE:
            return "Document is not standalone";
        case ERROR_EXPAT_UNEXPECTED_STATE:
            return "Unexpected parser state - please send a bug report";
        // end of expat (parser) errors

        // start of validation errors

        case ERROR_DOM_UNDECLARED_ELEMENT:
            return "Element has not been declared";
        case ERROR_DOM_ROOT_ELEMENT_MISNAMED:
            return "Root element name does not match DOCTYPE name";
        case ERROR_DOM_INVALID_ROOT_ELEMENT:
            return "Invalid or duplicate root element";

        case ERROR_DOM_INVALID_SUBELEMENT:
            return "Invalid sub-element in parent element";
        case ERROR_DOM_DUPLICATE_ELEMENT_DECL:
            return "Duplicate element declaration";
        case ERROR_DOM_DUPLICATE_ATTRIBUTE_DECL:
            return "Duplicate attribute declaration";
        case ERROR_DOM_UNDECLARED_ATTRIBUTE:
            return "Undeclared attribute in element";
        case ERROR_ELEMENT_CANNOT_HAVE_CONTENT:
            return "Element cannot have content";
        case ERROR_DOM_INVALID_ATTRIB_VALUE:
            return "Invalid attribute value";
        case ERROR_DOM_REQUIRED_ATTRIBUTE_MISSING:
            return "Required attribute is missing";
        case ERROR_DOM_SUBELEMENT_IN_EMPTY_ELEMENT:
            return "Subelement in empty element";

        case ERROR_DOM_PARSING:
            return "Parsing error";
        case ERROR_DOM_VALIDITY:
            return "Validity error";

        case ERROR_DOM_NODETYPE_NOT_SUPPORTED:
            return "DOM node type not supported";
        case ERROR_DOM_NO_DOCUMENT:
            return "No DOM document";
        case ERROR_DOM_NO_ELEMENT:
            return "No DOM element";
        case ERROR_DOM_DUPLICATE_DOCTYPE:
            return "Duplicate doctype";
        case ERROR_DOM_DOCTYPE_ROOT_NAMES_MISMATCH:
            return "Root element doesn't match doctype name";
        case ERROR_DOM_INTEGRITY:
            return "DOM integrity error";
        case ERROR_DOM_DUPLICATE_ATTRIBUTE:
            return "Duplicate attribute";

        case ERROR_DOM_VALIDATE_INVALID_ELEMENT:
            return "Validation error: Undeclared element name";
        case ERROR_DOM_ELEMENT_DECL_OUTSIDE_DOCTYPE:
            return "Element declaration outside doctype";
        case ERROR_DOM_ATTLIST_DECL_OUTSIDE_DOCTYPE:
            return "Attlist declaration outside doctype";

        case ERROR_DOM_INCOMPLETE_ENCODING_MAP:
            return "Incomplete encoding map specified";

        case ERROR_DOM_INVALID_EXTERNAL_HANDLER:
            return "Invalid 'external' handler specified";
    }

    return NULL;
}

/*
 *@@ xmlSetError:
 *      sets the DOM's error state and stores error information
 *      and parser position.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

VOID xmlSetError(PXMLDOM pDom,
                 APIRET arc,
                 const char *pcszFailing,
                 BOOL fValidityError)   // in: if TRUE, this is a validation error;
                                        // if FALSE, this is a parser error
{
    pDom->arcDOM = arc;
    pDom->pcszErrorDescription = xmlDescribeError(pDom->arcDOM);
    pDom->ulErrorLine = XML_GetCurrentLineNumber(pDom->pParser);
    pDom->ulErrorColumn = XML_GetCurrentColumnNumber(pDom->pParser);

    if (pcszFailing)
    {
        if (!pDom->pxstrFailingNode)
            pDom->pxstrFailingNode = xstrCreate(0);

        xstrcpy(pDom->pxstrFailingNode, pcszFailing, 0);
    }

    if (fValidityError)
        pDom->fInvalid = TRUE;
}

/* ******************************************************************
 *
 *   Most basic node management
 *
 ********************************************************************/

/*
 *@@ CompareXStrings:
 *      tree comparison func for NodeBases.
 *      This works for all trees which contain structures
 *      whose first item is a _NODEBASE because NODEBASE's first
 *      member is a TREE.
 *
 *      Used in two places:
 *
 *      --  to insert _CMELEMENTDECLNODE nodes into
 *          _DOMDOCTYPENODE.ElementDeclsTree;
 *
 *      --  to insert _CMELEMENTPARTICLE nodes into
 *          _CMELEMENTDECLNODE.ElementNamesTree.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 *@@changed V0.9.14 (2001-08-09) [umoeller]: fixed map bug which caused the whole XML stuff to fail
 */

STATIC int TREEENTRY CompareXStrings(ULONG ul1,
                                     ULONG ul2)
{
    return strhcmp(((PXSTRING)ul1)->psz,
                   ((PXSTRING)ul2)->psz);
}

/*
 *@@ xmlCreateNodeBase:
 *      creates a new NODEBASE node.
 *
 *      Gets called from xmlCreateDomNode also to create
 *      a DOMNODE, since that in turn has a NODEBASE.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

APIRET xmlCreateNodeBase(NODEBASETYPE ulNodeType,     // in: node type
                         ULONG cb,                    // in: size of struct
                         const char *pcszNodeName,    // in: node name or NULL
                         ULONG ulNodeNameLength,      // in: node name length
                                                      // or 0 to run strlen(pcszNodeName)
                         PNODEBASE *ppNew)            // out: new node
{
    APIRET      arc = NO_ERROR;
    PNODEBASE   pNewNode = (PNODEBASE)malloc(cb);

    if (!pNewNode)
        arc = ERROR_NOT_ENOUGH_MEMORY;
    else
    {
        memset(pNewNode, 0, cb);
        pNewNode->ulNodeType = ulNodeType;

        pNewNode->Tree.ulKey = (ULONG)&pNewNode->strNodeName;

        xstrInit(&pNewNode->strNodeName, 0);
        if (pcszNodeName)
        {
            xstrcpy(&pNewNode->strNodeName,
                    pcszNodeName,
                    ulNodeNameLength);
        }

        *ppNew = pNewNode;
    }

    return arc;
}

/*
 *@@ xmlDeleteNode:
 *      deletes a NODEBASE and frees memory that was
 *      associated with its members.
 *
 *      After calling this, pNode is no longer valid.
 *
 *      If you invoke this on a DOCUMENT node, the
 *      entire DOM tree will get deleted recursively.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 *@@changed V0.9.14 (2001-08-09) [umoeller]: fixed crash on string delete
 */

VOID xmlDeleteNode(PNODEBASE pNode)
{
    if (pNode)
    {
        PLISTNODE   pNodeThis;
        PDOMNODE    pDomNode = NULL;

        LINKLIST    llDeleteNodes;          // list that nodes to be deleted
                                            // can be appended to
        PLISTNODE   pDelNode;
        lstInit(&llDeleteNodes, FALSE);

        // now handle special types and their allocations
        switch (pNode->ulNodeType)
        {
            case DOMNODE_ELEMENT:
            {
                PDOMNODE pAttrib;

                pDomNode = (PDOMNODE)pNode;

                // delete all attribute nodes
                pAttrib = (PDOMNODE)treeFirst(pDomNode->AttributesMap);
                while (pAttrib)
                {
                    // call delete recursively
                    PDOMNODE pNext = (PDOMNODE)treeNext((TREE*)pAttrib);
                    xmlDeleteNode((PNODEBASE)pAttrib);
                            // this will remove pAttrib from pNode's attrib
                            // tree and rebalance the tree
                    pAttrib = pNext;
                }
            break; }

            case DOMNODE_ATTRIBUTE:
            case DOMNODE_TEXT:
            case DOMNODE_PROCESSING_INSTRUCTION:
            case DOMNODE_COMMENT:
                pDomNode = (PDOMNODE)pNode;
            break;

            case DOMNODE_DOCUMENT:
                if (((PDOMDOCUMENTNODE)pNode)->pDocType)
                    xmlDeleteNode((PNODEBASE)((PDOMDOCUMENTNODE)pNode)->pDocType);
                pDomNode = (PDOMNODE)pNode;
            break;

            case DOMNODE_DOCUMENT_TYPE:
            {
                PDOMDOCTYPENODE pDocType = (PDOMDOCTYPENODE)pNode;
                PCMELEMENTDECLNODE pElDecl;
                PCMATTRIBUTEDECLBASE pAttrDeclBase;

                pDomNode = (PDOMNODE)pNode;

                pElDecl = (PCMELEMENTDECLNODE)treeFirst(pDocType->ElementDeclsTree);
                while (pElDecl)
                {
                    lstAppendItem(&llDeleteNodes, pElDecl);
                    pElDecl = (PCMELEMENTDECLNODE)treeNext((TREE*)pElDecl);
                }

                pAttrDeclBase = (PCMATTRIBUTEDECLBASE)treeFirst(pDocType->AttribDeclBasesTree);
                while (pAttrDeclBase)
                {
                    lstAppendItem(&llDeleteNodes, pAttrDeclBase);
                    pAttrDeclBase = (PCMATTRIBUTEDECLBASE)treeNext((TREE*)pAttrDeclBase);
                }

                xstrClear(&pDocType->strPublicID);
                xstrClear(&pDocType->strSystemID);
            break; }

            case ELEMENTPARTICLE_EMPTY:
            case ELEMENTPARTICLE_ANY:
            case ELEMENTPARTICLE_MIXED:
            case ELEMENTPARTICLE_CHOICE:
            case ELEMENTPARTICLE_SEQ:
            case ELEMENTPARTICLE_NAME:
            {
                PCMELEMENTPARTICLE pp = (PCMELEMENTPARTICLE)pNode;
                if (pp->pllSubNodes)
                {
                    pDelNode = lstQueryFirstNode(pp->pllSubNodes);
                    while (pDelNode)
                    {
                        PCMELEMENTPARTICLE
                                pParticle = (PCMELEMENTPARTICLE)pDelNode->pItemData;
                        xmlDeleteNode((PNODEBASE)pParticle);
                        //  treeDelete(pp->         // @@todo
                        pDelNode = pDelNode->pNext;
                    }
                }
            break; }

            // case ATTRIBUTE_DECLARATION_ENUM:     // this is a plain NODEBASE

            case ATTRIBUTE_DECLARATION:
            {
                // PCMATTRIBUTEDECL pDecl = (PCMATTRIBUTEDECL)pNode;
            break; }

            case ATTRIBUTE_DECLARATION_BASE:
            break;
        }

        if (pDomNode)
        {
            // recurse into child nodes
            while (pNodeThis = lstQueryFirstNode(&pDomNode->llChildren))
                // recurse!!
                xmlDeleteNode((PNODEBASE)(pNodeThis->pItemData));
                            // this updates llChildren

            if (pDomNode->pParentNode)
            {
                // node has a parent:
                if (pNode->ulNodeType == DOMNODE_ATTRIBUTE)
                    // this is an attribute:
                    // remove from parent's attributes map
                    treeDelete(&pDomNode->pParentNode->AttributesMap,
                               NULL,
                               (TREE*)pNode);
                else
                    // remove this node from the parent's list
                    // of child nodes before deleting this node
                    lstRemoveItem(&pDomNode->pParentNode->llChildren,
                                  pNode);

                pDomNode->pParentNode = NULL;
            }

            xstrFree(&pDomNode->pstrNodeValue);
            lstClear(&pDomNode->llChildren);
        }

        pDelNode = lstQueryFirstNode(&llDeleteNodes);
        while (pDelNode)
        {
            PNODEBASE pNodeBase = (PNODEBASE)pDelNode->pItemData;
            xmlDeleteNode(pNodeBase);
            pDelNode = pDelNode->pNext;
        }

        lstClear(&llDeleteNodes);

        xstrClear(&pNode->strNodeName);
        free(pNode);
    }
}

/*
 *@@ xmlCreateDomNode:
 *      creates a new DOMNODE with the specified
 *      type and parent. Other than that, the
 *      node fields are zeroed.
 *
 *      If pParentNode is specified (which is required,
 *      unless you are creating a document node),
 *      its children list is automatically updated
 *      (unless this is an attribute node, which updates
 *      the attributes map).
 *
 *      This returns the following errors:
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  ERROR_DOM_NOT_SUPPORTED: invalid ulNodeType
 *          specified.
 *
 *      --  ERROR_DOM_WRONG_DOCUMENT: cannot find the
 *          document for this node. This happens if you do
 *          not have a document node at the root of your tree.
 */

APIRET xmlCreateDomNode(PDOMNODE pParentNode,        // in: parent node or NULL if root
                        NODEBASETYPE ulNodeType,     // in: DOMNODE_* type
                        const char *pcszNodeName,    // in: node name or NULL
                        ULONG ulNodeNameLength,      // in: node name length
                                                     // or 0 to run strlen(pcszNodeName)
                        PDOMNODE *ppNew)             // out: new node
{
    PDOMNODE pNewNode = NULL;
    APIRET  arc = NO_ERROR;

    ULONG   cb = 0;

    switch (ulNodeType)
    {
        case DOMNODE_DOCUMENT:
            cb = sizeof(DOMDOCUMENTNODE);
        break;

        case DOMNODE_DOCUMENT_TYPE:
            cb = sizeof(DOMDOCTYPENODE);
        break;

        default:
            cb = sizeof(DOMNODE);
        break;
    }

    if (!(arc = xmlCreateNodeBase(ulNodeType,
                                  cb,
                                  pcszNodeName,
                                  ulNodeNameLength,
                                  (PNODEBASE*)&pNewNode)))
    {
        pNewNode->pParentNode = pParentNode;

        if (pParentNode)
        {
            // parent specified:
            // check if this is an attribute
            if (ulNodeType == DOMNODE_ATTRIBUTE)
            {
                // attribute:
                // add to parent's attributes list
                if (treeInsert(&pParentNode->AttributesMap,
                               NULL,
                               &pNewNode->NodeBase.Tree,
                               CompareXStrings))
                    arc = ERROR_DOM_DUPLICATE_ATTRIBUTE;
                                // shouldn't happen, because expat takes care of this
            }
            else
                // append this new node to the parent's
                // list of child nodes
                lstAppendItem(&pParentNode->llChildren,
                              pNewNode);

            if (!arc)
            {
                // set document pointer...
                // if the parent node has a document pointer,
                // we can copy that
                if (pParentNode->pDocumentNode)
                    pNewNode->pDocumentNode = pParentNode->pDocumentNode;
                else
                    // parent has no document pointer: then it is probably
                    // the document itself... check
                    if (pParentNode->NodeBase.ulNodeType == DOMNODE_DOCUMENT)
                        pNewNode->pDocumentNode = pParentNode;
                    else
                        arc = ERROR_DOM_NO_DOCUMENT;
            }
        }

        lstInit(&pNewNode->llChildren, FALSE);
        treeInit(&pNewNode->AttributesMap, NULL);
    }

    if (!arc)
        *ppNew = pNewNode;
    else
        if (pNewNode)
            free(pNewNode);

    return arc;
}

/* ******************************************************************
 *
 *   Specific DOM node constructors
 *
 ********************************************************************/

/*
 *@@ xmlCreateElementNode:
 *      creates a new element node with the specified name.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

APIRET xmlCreateElementNode(PDOMNODE pParent,         // in: parent node (either document or element)
                            const char *pcszElement,  // in: element name (null-terminated)
                            PDOMNODE *ppNew)
{
    PDOMNODE pNew = NULL;
    APIRET arc;

    if (!(arc = xmlCreateDomNode(pParent,
                                 DOMNODE_ELEMENT,
                                 pcszElement,
                                 0,
                                 &pNew)))
        *ppNew = pNew;

    return arc;
}

/*
 *@@ xmlCreateAttributeNode:
 *      creates a new attribute node with the specified data.
 *
 *      NOTE: Attributes have no "parent" node, technically.
 *      They are added to a special, separate list in @DOM_ELEMENT
 *      nodes.
 *
 *      This returns the following errors:
 *
 *      --  Error codes from xmlCreateDomNode.
 *
 *      --  ERROR_DOM_NO_ELEMENT: pElement is invalid or does
 *          not point to an @DOM_ELEMENT node.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 *@@changed V1.0.2 (2003-02-07) [umoeller]: added lenValue param
 */

APIRET xmlCreateAttributeNode(PDOMNODE pElement,        // in: element node
                              const char *pcszName,     // in: attribute name (null-terminated)
                              const char *pcszValue,    // in: attribute value (null-terminated)
                              ULONG lenValue,           // in: length of value (must be specified)
                              PDOMNODE *ppNew)
{
    APIRET      arc;
    PDOMNODE    pNew = NULL;

    if (    (!pElement)
         || (pElement->NodeBase.ulNodeType != DOMNODE_ELEMENT)
       )
        return ERROR_DOM_NO_ELEMENT;

    if (!(arc = xmlCreateDomNode(pElement,          // this takes care of adding to the list
                                 DOMNODE_ATTRIBUTE,
                                 pcszName,
                                 0,
                                 &pNew)))
    {
        pNew->pstrNodeValue = xstrCreate(lenValue + 1);
        xstrcpy(pNew->pstrNodeValue, pcszValue, lenValue);

        *ppNew = pNew;
    }

    return arc;
}

/*
 *@@ xmlCreateTextNode:
 *      creates a new text node with the specified content.
 *
 *      Note: This differs from the createText method
 *      as specified by DOM, which has no ulLength parameter.
 *      We need this for speed with @expat though.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

APIRET xmlCreateTextNode(PDOMNODE pParent,         // in: parent element node
                         const char *pcszText,     // in: ptr to start of text
                         ULONG lenText,            // in: length of *pcszText
                         PDOMNODE *ppNew)
{
    PDOMNODE pNew = NULL;
    APIRET arc;

    if (!(arc = xmlCreateDomNode(pParent,
                                 DOMNODE_TEXT,
                                 NULL,
                                 0,
                                 &pNew)))
    {
        PSZ pszNodeValue;
        if (pszNodeValue = (PSZ)malloc(lenText + 1))
        {
            memcpy(pszNodeValue, pcszText, lenText);
            pszNodeValue[lenText] = '\0';
            pNew->pstrNodeValue = xstrCreate(0);
            xstrset(pNew->pstrNodeValue, pszNodeValue);

            *ppNew = pNew;
        }
        else
        {
            arc = ERROR_NOT_ENOUGH_MEMORY;
            xmlDeleteNode((PNODEBASE)pNew);
        }
    }

    return arc;
}

/*
 *@@ xmlCreateCommentNode:
 *      creates a new comment node with the specified
 *      content.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

APIRET xmlCreateCommentNode(PDOMNODE pParent,         // in: parent element node
                            const char *pcszText,     // in: comment (null-terminated)
                            PDOMNODE *ppNew)
{
    PDOMNODE pNew = NULL;
    APIRET arc;
    if (!(arc = xmlCreateDomNode(pParent,
                                 DOMNODE_COMMENT,
                                 NULL,
                                 0,
                                 &pNew)))
    {
        pNew->pstrNodeValue = xstrCreate(0);
        xstrcpy(pNew->pstrNodeValue, pcszText, 0);
        *ppNew = pNew;
    }

    return arc;
}

/*
 *@@ xmlCreatePINode:
 *      creates a new processing instruction node with the
 *      specified data.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

APIRET xmlCreatePINode(PDOMNODE pParent,         // in: parent element node
                       const char *pcszTarget,   // in: PI target (null-terminated)
                       const char *pcszData,     // in: PI data (null-terminated)
                       PDOMNODE *ppNew)
{
    PDOMNODE pNew = NULL;
    APIRET arc;

    if (!(arc = xmlCreateDomNode(pParent,
                                 DOMNODE_PROCESSING_INSTRUCTION,
                                 pcszTarget,
                                 0,
                                 &pNew)))
    {
        pNew->pstrNodeValue = xstrCreate(0);
        xstrcpy(pNew->pstrNodeValue, pcszData, 0);

        *ppNew = pNew;
    }

    return arc;
}

/*
 *@@ xmlCreateDocumentTypeNode:
 *      creates a new document type node with the
 *      specified data.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

APIRET xmlCreateDocumentTypeNode(PDOMDOCUMENTNODE pDocumentNode,            // in: document node
                                 const char *pcszDoctypeName,
                                 const char *pcszSysid,
                                 const char *pcszPubid,
                                 int fHasInternalSubset,
                                 PDOMDOCTYPENODE *ppNew)
{
    APIRET arc = NO_ERROR;

    if (pDocumentNode->pDocType)
        // we already have a doctype:
        arc = ERROR_DOM_DUPLICATE_DOCTYPE;
    else
    {
        // create doctype node
        PDOMDOCTYPENODE pNew = NULL;
        if (!(arc = xmlCreateDomNode((PDOMNODE)pDocumentNode,
                                     DOMNODE_DOCUMENT_TYPE,
                                     pcszDoctypeName,
                                     0,
                                     (PDOMNODE*)&pNew)))
        {
            // the node has already been added to the children
            // list of the document node... in addition, set
            // the doctype field in the document
            pDocumentNode->pDocType = pNew;

            // initialize the extra fields
            xstrcpy(&pNew->strPublicID, pcszPubid, 0);
            xstrcpy(&pNew->strSystemID, pcszSysid, 0);
            pNew->fHasInternalSubset = fHasInternalSubset;

            treeInit(&pNew->ElementDeclsTree, NULL);
            treeInit(&pNew->AttribDeclBasesTree, NULL);

            *ppNew = pNew;
        }
    }
    return arc;
}

/* ******************************************************************
 *
 *   DOM level 3 content models
 *
 ********************************************************************/

/*
 *@@ SetupParticleAndSubs:
 *
 *      This creates sub-particles and recurses to set them up,
 *      if necessary.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

STATIC APIRET SetupParticleAndSubs(PCMELEMENTPARTICLE pParticle,
                                   PXMLCONTENT pModel,
                                   TREE **ppElementNamesTree) // in: ptr to _CMELEMENTDECLNODE.ElementNamesTree
                                                              // (passed to all recursions)
{
    APIRET arc = NO_ERROR;

    // set up member NODEBASE
    switch (pModel->type)
    {
        case XML_CTYPE_EMPTY: // that's easy
            pParticle->NodeBase.ulNodeType = ELEMENTPARTICLE_EMPTY;
        break;

        case XML_CTYPE_ANY:   // that's easy
            pParticle->NodeBase.ulNodeType = ELEMENTPARTICLE_ANY;
        break;

        case XML_CTYPE_NAME:   // that's easy
            pParticle->NodeBase.ulNodeType = ELEMENTPARTICLE_NAME;
            xstrcpy(&pParticle->NodeBase.strNodeName, pModel->name, 0);
            treeInsert(ppElementNamesTree,
                       NULL,
                       &pParticle->NodeBase.Tree,
                       CompareXStrings);
        break;

        case XML_CTYPE_MIXED:
            pParticle->NodeBase.ulNodeType = ELEMENTPARTICLE_MIXED;
        break;

        case XML_CTYPE_CHOICE:
            pParticle->NodeBase.ulNodeType = ELEMENTPARTICLE_CHOICE;
        break;

        case XML_CTYPE_SEQ:
            pParticle->NodeBase.ulNodeType = ELEMENTPARTICLE_SEQ;
        break;
    }

    pParticle->ulRepeater = pModel->quant;

    if (pModel->numchildren)
    {
        // these are the three cases where we have subnodes
        // in the XMLCONTENT... go for these and recurse
        ULONG ul;
        pParticle->pllSubNodes = lstCreate(FALSE);
        for (ul = 0;
             ul < pModel->numchildren;
             ul++)
        {
            PXMLCONTENT pSubModel = &pModel->children[ul];
            PCMELEMENTPARTICLE pSubNew = NULL;
            if (!(arc = xmlCreateNodeBase(TYPE_UNKNOWN, //       node type... for now
                                          sizeof(CMELEMENTPARTICLE),
                                          0,
                                          0,
                                          (PNODEBASE*)&pSubNew)))
            {
                if (!(arc = SetupParticleAndSubs(pSubNew,
                                                 pSubModel,
                                                 ppElementNamesTree)))
                {
                    // no error: append sub-particle to this particle's
                    // children list
                    lstAppendItem(pParticle->pllSubNodes,
                                  pSubNew);
                    // and store this particle as the parent in the
                    // sub-particle
                    pSubNew->pParentParticle = pParticle;
                }
            }

            if (arc)
                break;
        }
    }

    return arc;
}

/*
 *@@ xmlCreateElementDecl:
 *      creates a new _CMELEMENTDECLNODE for the specified
 *      _XMLCONTENT content model (which is the @expat structure).
 *      This recurses, if necessary.
 *
 *      NOTE: As opposed to the other "create" functions,
 *      this does not take a parent node as input. If this
 *      returns NO_ERROR, it is the caller's responsibility
 *      to add the produced node to the document's DOCTYPE node.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

APIRET xmlCreateElementDecl(const char *pcszName,
                            PXMLCONTENT pModel,
                            PCMELEMENTDECLNODE *ppNew)
{
    APIRET arc = NO_ERROR;
    PCMELEMENTDECLNODE pNew = NULL;

    if (!(arc = xmlCreateNodeBase(TYPE_UNKNOWN,      // for now
                                  sizeof(CMELEMENTDECLNODE),
                                  pcszName,
                                  0,
                                  (PNODEBASE*)&pNew)))
    {
        treeInit(&pNew->ParticleNamesTree, NULL);

        // set up the "particle" member and recurse into sub-particles
        if (!(arc = SetupParticleAndSubs(&pNew->Particle,
                                         pModel,
                                         &pNew->ParticleNamesTree)))
            *ppNew = pNew;
        else
            free(pNew);
    }

    return arc;
}

/*
 *@@ ValidateElementChildren
 *      validates the specified element against the document's @DTD,
 *      more specifically, against the element declaration of the
 *      new element's parent.
 *
 *      This sets arcDOM in XMLDOM on errors.
 *
 *      According to the XML spec, an element is valid if there
 *      is a declaration matching the element declaration where the
 *      element's name matches the element type, and _one_ of the
 *      following holds:
 *
 *      (1) The declaration matches EMPTY and the element has no @content. (done)
 *
 *      (2) The declaration matches (children) (see @element_declaration)
 *          and the sequence of child elements belongs to the language
 *          generated by the regular expression in the content model, with
 *          optional @white_space between the start-tag and the first child
 *          element, between child elements, or between the last
 *          child element and the end-tag. Note that a CDATA section
 *          is never considered "whitespace", even if it contains
 *          white space only. @@todo
 *
 *      (3) The declaration matches (mixed) (see @element_declaration)
 *          and the content consists of @content and child elements
 *          whose types match names in the content model. (done)
 *
 *      (4) The declaration matches ANY, and the types of any child
 *          elements have been declared. (done)
 *
 *      Preconditions: The element must already have been inserted
 *      into the parent element's list, or we cannot validate sequences.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

STATIC VOID ValidateElement(PXMLDOM pDom,
                            PDOMNODE pNewElement,     // in: new element
                            PCMELEMENTDECLNODE pParentElementDecl)
                                                      // in: element decl of element's parent
{
    if (pDom && pNewElement)
    {
        if (!pParentElementDecl)
        {
            // this is always missing for the root element, of course,
            // because the parent is the document
            if (pNewElement->pParentNode == (PDOMNODE)pDom->pDocumentNode)
                return;     // that's OK
            else
                xmlSetError(pDom,
                            ERROR_DOM_VALIDATE_INVALID_ELEMENT,
                            pNewElement->NodeBase.strNodeName.psz,
                            TRUE);
        }
        else
        {
            ULONG ulDeclType = pParentElementDecl->Particle.NodeBase.ulNodeType;

            switch (ulDeclType)
            {
                case ELEMENTPARTICLE_EMPTY:
                    // this is an error for sure
                    xmlSetError(pDom,
                                ERROR_DOM_SUBELEMENT_IN_EMPTY_ELEMENT,
                                pNewElement->NodeBase.strNodeName.psz,
                                TRUE);
                break;

                case ELEMENTPARTICLE_ANY:
                    // that's always OK
                break;

                case ELEMENTPARTICLE_MIXED:
                case ELEMENTPARTICLE_CHOICE:
                case ELEMENTPARTICLE_SEQ:
                {
                    PXSTRING pstrNewElementName
                        = &pNewElement->NodeBase.strNodeName;

                    // for all these, we first need to check if
                    // the element is allowed at all
                    PCMELEMENTPARTICLE pParticle
                        = (PCMELEMENTPARTICLE)treeFind(
                                         pParentElementDecl->ParticleNamesTree,
                                         (ULONG)pstrNewElementName,
                                         CompareXStrings);
                    if (!pParticle)
                        // not found: then this element is not allowed within this
                        // parent
                        xmlSetError(pDom,
                                    ERROR_DOM_INVALID_SUBELEMENT,
                                    pstrNewElementName->psz,
                                    TRUE);
                    else
                    {
                        // the element is allowed at all: now check for the
                        // lists case... @@todo
                        switch (ulDeclType)
                        {
                            case ELEMENTPARTICLE_CHOICE:
                            break;

                            case ELEMENTPARTICLE_SEQ:
                            break;
                        }
                    }

                break; }
            }
        }
    }
    else
        pDom->arcDOM = ERROR_INVALID_PARAMETER;

    // yes: get the element decl from the tree
    /* PCMELEMENTDECLNODE pElementDecl = xmlFindElementDecl(pDom,
                                                         &pElement->NodeBase.strNodeName);
    if (!pElementDecl)
    {
        xmlSetError(pDom,
                    ERROR_DOM_UNDECLARED_ELEMENT,
                    pElement->NodeBase.strNodeName.psz,
                    TRUE);
    }
    else
    {
        // element has been declared:
        // check if it may appear in this element's parent...
        PDOMNODE pParentElement = pElement->pParentNode;

        if (!pParentElement)
            pDom->arcDOM = ERROR_DOM_INTEGRITY;
        else switch (pParentElement->NodeBase.ulNodeType)
        {
            case DOMNODE_DOCUMENT:
            {
                // if this is the root element, compare its name
                // to the DOCTYPE name
                if (pParentElement != (PDOMNODE)pDom->pDocumentNode)
                    xmlSetError(pDom,
                                ERROR_DOM_INVALID_ROOT_ELEMENT,
                                pElement->NodeBase.strNodeName.psz,
                                TRUE);
                else if (strcmp(pDom->pDocumentNode->DomNode.NodeBase.strNodeName.psz,
                                pElement->NodeBase.strNodeName.psz))
                    // no match:
                    xmlSetError(pDom,
                                ERROR_DOM_ROOT_ELEMENT_MISNAMED,
                                pElement->NodeBase.strNodeName.psz,
                                TRUE);
            break; }

            case DOMNODE_ELEMENT:
            {
                // parent of element is another element:
                // check the parent in the DTD and find out if
                // this element may appear in the parent element
                PCMELEMENTDECLNODE pParentElementDecl
                        = xmlFindElementDecl(pDom,
                                             &pParentElement->NodeBase.strNodeName);
                if (!pParentElementDecl)
                    pDom->arcDOM = ERROR_DOM_INTEGRITY;
                else
                {
                    // now check the element names tree of the parent element decl
                    // for whether this element is allowed as a sub-element at all
                    PCMELEMENTPARTICLE pParticle
                        = treeFindEQData(&pParentElementDecl->ParticleNamesTree,
                                         (void*)pElement->NodeBase.strNodeName.psz,
                                         CompareNodeBaseData);
                    if (!pParticle)
                        // not found: then this element is not allowed within this
                        // parent
                        xmlSetError(pDom,
                                    ERROR_DOM_INVALID_SUBELEMENT,
                                    pElement->NodeBase.strNodeName.psz,
                                    TRUE);
                }
            break; }
        }
    }
    */
}

/*
 *@@ ValidateAttributeType:
 *      validates the specified attribute's type against the
 *      document's @DTD.
 *
 *      This sets arcDOM in XMLDOM on errors.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

STATIC VOID ValidateAttributeType(PXMLDOM pDom,
                                  PDOMNODE pAttrib,
                                  PCMATTRIBUTEDECLBASE *ppAttribDeclBase)
{
    PDOMNODE pElement = pAttrib->pParentNode;

    PCMATTRIBUTEDECL pAttribDecl = xmlFindAttribDecl(pDom,
                                                     &pElement->NodeBase.strNodeName,
                                                     &pAttrib->NodeBase.strNodeName,
                                                     ppAttribDeclBase);
    if (!pAttribDecl)
        xmlSetError(pDom,
                    ERROR_DOM_UNDECLARED_ATTRIBUTE,
                    pAttrib->NodeBase.strNodeName.psz,
                    TRUE);
    else
    {
        // check if the attribute value is allowed
        switch (pAttribDecl->ulAttrType)
        {
            case CMAT_CDATA:
            case CMAT_ID:
            case CMAT_IDREF:
            case CMAT_IDREFS:       // ###
            case CMAT_ENTITY:
            case CMAT_ENTITIES:
            case CMAT_NMTOKEN:
            case CMAT_NMTOKENS:
            break;

            case CMAT_ENUM:
            {
                // enumeration: then check if it has one of the
                // allowed values
                PNODEBASE pValue = (PNODEBASE)treeFind(
                                                pAttribDecl->ValuesTree,
                                                (ULONG)pAttrib->pstrNodeValue,
                                                CompareXStrings);
                if (!pValue)
                    xmlSetError(pDom,
                                ERROR_DOM_INVALID_ATTRIB_VALUE,
                                pAttrib->NodeBase.strNodeName.psz,
                                TRUE);
            }
        }

        if (pAttribDecl->ulConstraint == CMAT_FIXED_VALUE)
            if (strcmp(pAttrib->pstrNodeValue->psz, pAttribDecl->pstrDefaultValue->psz))
                // fixed value doesn't match:
                xmlSetError(pDom,
                            ERROR_DOM_INVALID_ATTRIB_VALUE,
                            pAttrib->NodeBase.strNodeName.psz,
                            TRUE);
    }
}

/*
 *@@ ValidateAllAttributes:
 *      validates the constraints of all attributes of the specified
 *      element against the document's @DTD.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

STATIC VOID ValidateAllAttributes(PXMLDOM pDom,
                                  PCMATTRIBUTEDECLBASE pAttribDeclBase,
                                  PDOMNODE pNewElement)
{
    PCMATTRIBUTEDECL pDeclThis
        = (PCMATTRIBUTEDECL)treeFirst(pAttribDeclBase->AttribDeclsTree);

    while ((pDeclThis) && (!pDom->arcDOM))
    {
        // if attribute is all optional: then we don't need
        // to check for whether it's here
        if (    (pDeclThis->ulConstraint != CMAT_IMPLIED)
             && (pDeclThis->ulConstraint != CMAT_DEFAULT_VALUE)
                    // we don't have to check this case because expat
                    // already adds default attributes for us
           )
        {
            // for all others , we need to find the attribute
            PXSTRING pstrAttrNameThis = &pDeclThis->NodeBase.strNodeName;
            PDOMNODE pAttrNode = (PDOMNODE)treeFind(
                                            pNewElement->AttributesMap,
                                            (ULONG)pstrAttrNameThis,
                                            CompareXStrings);

            // now switch again
            switch (pDeclThis->ulConstraint)
            {
                case CMAT_REQUIRED:
                    if (!pAttrNode)
                        // required, but no attribute with this name exists:
                        xmlSetError(pDom,
                                    ERROR_DOM_REQUIRED_ATTRIBUTE_MISSING,
                                    pstrAttrNameThis->psz,
                                    TRUE);
                break;
            }
        }

        pDeclThis = (PCMATTRIBUTEDECL)treeNext((TREE*)pDeclThis);
    }
}

/* ******************************************************************
 *
 *   Expat stack
 *
 ********************************************************************/

/*
 *@@ DOMSTACKITEM:
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

typedef struct _DOMSTACKITEM
{
    PDOMNODE                pDomNode;
    PCMELEMENTDECLNODE      pElementDecl;

} DOMSTACKITEM, *PDOMSTACKITEM;

/*
 *@@ PopElementStack:
 *
 *      NOTE:
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

STATIC PDOMSTACKITEM PopElementStack(PXMLDOM pDom,
                                     PLISTNODE *ppListNode)
{
    PDOMSTACKITEM   pStackItem = NULL;
    PLISTNODE       pParentLN = lstPop(&pDom->llElementStack);

    if (!pParentLN)
        pDom->arcDOM = ERROR_DOM_NO_ELEMENT;
    else
    {
        // we have at least one node:
        pStackItem = (PDOMSTACKITEM)pParentLN->pItemData;

        if (ppListNode)
            *ppListNode = pParentLN;
    }

    return pStackItem;
}

/*
 *@@ PushElementStack:
 *
 *      NOTE: pDomNode will most frequently be an element
 *      node, but will also be the document for root and
 *      a DOCTYPE node while parsing the DTD.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

STATIC VOID PushElementStack(PXMLDOM pDom,
                             PDOMNODE pDomNode)
{
    PDOMSTACKITEM pNew = (PDOMSTACKITEM)malloc(sizeof(*pNew));
    if (!pNew)
        pDom->arcDOM = ERROR_NOT_ENOUGH_MEMORY;
    else
    {
        memset(pNew, 0, sizeof(*pNew));
        pNew->pDomNode = pDomNode;

        // shall we validate?
        if (    (pDom->pDocTypeNode)
             && (pDomNode->NodeBase.ulNodeType == DOMNODE_ELEMENT)
           )
            pNew->pElementDecl = xmlFindElementDecl(pDom,
                                                    &pDomNode->NodeBase.strNodeName);

        lstPush(&pDom->llElementStack,
                pNew);
    }
}

/* ******************************************************************
 *
 *   Expat handlers
 *
 ********************************************************************/

/*
 *@@ UnknownEncodingHandler:
 *      @expat handler called when the xml
 *      @text_declaration has an @encoding that is not
 *      one of the four encodings built into expat.
 *
 *      See XML_SetUnknownEncodingHandler.
 *
 *@@added V0.9.14 (2001-08-09) [umoeller]
 */

STATIC int EXPATENTRY UnknownEncodingHandler(void *pUserData,   // in: out PXMLDOM really
                                             const XML_Char *pcszName,
                                             XML_Encoding *pEncoding)
{
    PXMLDOM     pDom = (PXMLDOM)pUserData;

    ULONG ulCP;
    if (    (pDom->pfnGetCPData)            // callback exists?
         && (!strncmp(pcszName, "cp", 2))
         && (strlen(pcszName) > 4)              // at least five chars (e.g. "cp850")
         && (ulCP = atoi(pcszName + 2))
       )
    {
        // this is a PC codepage:
/* typedef struct _XML_Encoding
{
  int           map[256];
  void          *data;
  int           (* EXPATENTRY convert)(void *data, const char *s);
  void          (* EXPATENTRY release)(void *data);
} XML_Encoding; */

        // ZERO(pEncoding);

        pEncoding->convert = NULL;
        pEncoding->release = NULL;

        memset(&pEncoding->map, -1, sizeof(pEncoding->map));

        if (pDom->pfnGetCPData(pDom,
                               ulCP,
                               pEncoding->map))
        {
            // go check if there's any -1 chars left
            ULONG ul;
            for (ul = 0;
                 ul < 256;
                 ul++)
            {
                if (pEncoding->map[ul] < 0)
                    xmlSetError(pDom,
                                ERROR_DOM_INCOMPLETE_ENCODING_MAP,
                                NULL,
                                FALSE);
            }
            // return success
            return 1;
        }
    }

    // error
    return 0;
}

/*
 *@@ StartElementHandler:
 *      @expat handler called when a new element is
 *      found.
 *
 *      We create a new record in the container and
 *      push it onto our stack so we can insert
 *      children into it. We first start with the
 *      attributes.
 */

STATIC void EXPATENTRY StartElementHandler(void *pUserData,      // in: our PXMLDOM really
                                           const char *pcszElement,
                                           const char **papcszAttribs)
{
    PXMLDOM     pDom = (PXMLDOM)pUserData;

    // continue parsing only if we had no errors so far
    if (!pDom->arcDOM)
    {
        ULONG       i;

        PDOMSTACKITEM pSI = PopElementStack(pDom,
                                            NULL);     // no free
        if (!pDom->arcDOM)
        {
            PDOMNODE    pParent = pSI->pDomNode,
                        pNew = NULL;

            if (!(pDom->arcDOM = xmlCreateElementNode(pParent,
                                                      pcszElement,
                                                      &pNew)))
                // OK, node is valid:
                // push this on the stack so we can add child elements
                PushElementStack(pDom,
                                 pNew);

            // shall we validate?
            if (    (!pDom->arcDOM)
                 && (pDom->pDocTypeNode)
               )
                ValidateElement(pDom,
                                pNew,                // new element
                                pSI->pElementDecl);  // parent's elem decl

            if (!pDom->arcDOM)
            {
                PCMATTRIBUTEDECLBASE pAttribDeclBase = NULL;

                // shall we validate?
                if (pDom->pDocTypeNode)
                    // yes: get attrib decl base for speed
                    pAttribDeclBase
                        = xmlFindAttribDeclBase(pDom,
                                                &pNew->NodeBase.strNodeName);

                // now for the attribs
                for (i = 0;
                     (papcszAttribs[i]) && (!pDom->arcDOM);
                     i += 2)
                {
                    PDOMNODE pAttrib;
                    PCSZ    pcszValue = papcszAttribs[i + 1];  // attr value
                    if (!(pDom->arcDOM = xmlCreateAttributeNode(pNew,                  // element,
                                                                papcszAttribs[i],      // attr name
                                                                pcszValue,
                                                                strlen(pcszValue),
                                                                &pAttrib)))
                    {
                        // shall we validate?
                        if (pDom->pDocTypeNode)
                            ValidateAttributeType(pDom,
                                                  pAttrib,
                                                  &pAttribDeclBase);
                    }
                    else
                    {
                        xmlSetError(pDom,
                                    pDom->arcDOM,
                                    papcszAttribs[i],
                                    FALSE);      // validation
                        break;
                    }
                }

                // OK, now we got all attributes:
                // now look for defaults in the DTD,
                // if we shall validate
                if (    (pDom->pDocTypeNode)
                     && (!pDom->arcDOM)
                     && (pAttribDeclBase)
                   )
                    ValidateAllAttributes(pDom,
                                          pAttribDeclBase,
                                          pNew);
            }
        }

        pDom->pLastWasTextNode = NULL;
    }
}

/*
 *@@ EndElementHandler:
 *      @expat handler for when parsing an element is done.
 *      We pop the element off of our stack then.
 */

STATIC void EXPATENTRY EndElementHandler(void *pUserData,      // in: our PXMLDOM really
                                         const XML_Char *name)
{
    PXMLDOM     pDom = (PXMLDOM)pUserData;
    // continue parsing only if we had no errors so far
    if (!pDom->arcDOM)
    {
        PLISTNODE pStackLN = NULL;
        PDOMSTACKITEM pSI = PopElementStack(pDom,
                                            &pStackLN);

        if (!pDom->arcDOM)
        {
            // shall we validate?
            /* if (pDom->pDocTypeNode)
                // yes:
                ValidateElementChildren(pDom,
                                        pSI->pDomNode); */

            lstRemoveNode(&pDom->llElementStack, pStackLN); // auto-free
        }
        else
            pDom->arcDOM = ERROR_DOM_INTEGRITY;

        pDom->pLastWasTextNode = NULL;
    }
}

/*
 *@@ CharacterDataHandler:
 *      @expat handler for character data (@content).
 *
 *      Note: expat passes chunks of content without zero-terminating
 *      them. We must concatenate the chunks to a full text node.
 */

STATIC void EXPATENTRY CharacterDataHandler(void *pUserData,      // in: our PXMLDOM really
                                            const XML_Char *s,
                                            int len)
{
    PXMLDOM     pDom = (PXMLDOM)pUserData;

    // continue parsing only if we had no errors so far
    if (    (!pDom->arcDOM)
         && (len)
       )
    {
        // we need a new text node:
        PDOMSTACKITEM pSI = PopElementStack(pDom,
                                            NULL);     // no free
        if (!pDom->arcDOM)
        {
            PDOMNODE    pParent = pSI->pDomNode;
                        // pNew = NULL;

            BOOL fIsWhitespace = FALSE;

            // shall we validate?
            if (pDom->pDocTypeNode)
            {
                // yes: check if the parent element allows
                // for content at all (must be "mixed" model)

                // get the element decl from the tree
                PCMELEMENTDECLNODE pElementDecl;
                if (pElementDecl = pSI->pElementDecl)
                {
                    switch (pElementDecl->Particle.NodeBase.ulNodeType)
                    {
                        case ELEMENTPARTICLE_ANY:
                        case ELEMENTPARTICLE_MIXED:
                            // those two are okay
                        break;

                        case ELEMENTPARTICLE_EMPTY:
                            // that's an error for sure
                            pDom->arcDOM = ERROR_ELEMENT_CANNOT_HAVE_CONTENT;
                        break;

                        default:
                        {
                            // ELEMENTPARTICLE_CHOICE:
                            // ELEMENTPARTICLE_SEQ:
                            // with these two, we accept whitespace, but nothing
                            // else... so if we have characters other than
                            // whitespace, terminate
                            ULONG ul;
                            const char *p = s;

                            if (pDom->flParserFlags & DF_DROP_WHITESPACE)
                                fIsWhitespace = TRUE;

                            for (ul = 0;
                                 ul < len;
                                 ul++, p++)
                                if (!strchr("\r\n\t ", *p))
                                {
                                    // other character:
                                    xmlSetError(pDom,
                                                ERROR_ELEMENT_CANNOT_HAVE_CONTENT,
                                                pParent->NodeBase.strNodeName.psz,
                                                TRUE);
                                    fIsWhitespace = FALSE;
                                    break;
                                }
                        }
                    }
                }

            } // end if (pDom->pDocTypeNode)

            if (!fIsWhitespace)
                // this is false if any of the following
                // is true:
                // --  we are not validating at all
                // --  we are validating, but the the element
                //     can have mixed content
                // --  we are validating and the element does
                //     _not_ have mixed content and DF_DROP_WHITESPACE
                //     is set, but the string is whitespace only
                //     --> drop it then

            if (pDom->pLastWasTextNode)
                // we had a text node, and no elements or other
                // stuff in between:
                xstrcat(pDom->pLastWasTextNode->pstrNodeValue,
                        s,
                        len);
            else
                pDom->arcDOM = xmlCreateTextNode(pParent,
                                                 s,
                                                 len,
                                                 &pDom->pLastWasTextNode);
        }
    }
}

/*
 *@@ CommentHandler:
 *      @expat handler for @comments.
 *
 *      Note: This is only set if DF_PARSECOMMENTS is
 *      flagged with xmlCreateDOM.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

STATIC void EXPATENTRY CommentHandler(void *pUserData,      // in: our PXMLDOM really
                                      const XML_Char *data)
{
    PXMLDOM     pDom = (PXMLDOM)pUserData;

    // continue parsing only if we had no errors so far
    if (!pDom->arcDOM)
    {
        // we need a new text node:
        PDOMSTACKITEM pSI = PopElementStack(pDom,
                                            NULL);     // no free
        if (!pDom->arcDOM)
        {
            PDOMNODE    pParent = pSI->pDomNode,
                        pNew = NULL;

            pDom->arcDOM = xmlCreateCommentNode(pParent,
                                                data,
                                                &pNew);
        }
    }
}

/*
 *@@ StartDoctypeDeclHandler:
 *      @expat handler that is called at the start of a DOCTYPE
 *      declaration, before any external or internal subset is
 *      parsed.
 *
 *      Both pcszSysid and pcszPubid may be NULL. "fHasInternalSubset"
 *      will be non-zero if the DOCTYPE declaration has an internal subset.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

STATIC void EXPATENTRY StartDoctypeDeclHandler(void *pUserData,
                                               const XML_Char *pcszDoctypeName,
                                               const XML_Char *pcszSysid,
                                               const XML_Char *pcszPubid,
                                               int fHasInternalSubset)
{
    PXMLDOM     pDom = (PXMLDOM)pUserData;

    // continue parsing only if we had no errors so far
    if (!pDom->arcDOM)
    {
        // get the document node
        PDOMDOCUMENTNODE pDocumentNode = pDom->pDocumentNode;
        if (!pDocumentNode)
            pDom->arcDOM = ERROR_DOM_NO_DOCUMENT;
        else
        {
            // doctype must be null
            if (pDom->pDocTypeNode)
                pDom->arcDOM = ERROR_DOM_DUPLICATE_DOCTYPE;
            else
                pDom->arcDOM = xmlCreateDocumentTypeNode(pDocumentNode,
                                                         pcszDoctypeName,
                                                         pcszSysid,
                                                         pcszPubid,
                                                         fHasInternalSubset,
                                                         &pDom->pDocTypeNode);
        }
    }
}

/*
 *@@ EndDoctypeDeclHandler:
 *      @expat handler that is called at the end of a DOCTYPE
 *      declaration, after parsing any external subset.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

STATIC void EXPATENTRY EndDoctypeDeclHandler(void *pUserData)      // in: our PXMLDOM really
{
    PXMLDOM     pDom = (PXMLDOM)pUserData;

    // continue parsing only if we had no errors so far
    if (!pDom->arcDOM)
    {
    }
}

/*
 *@@ NotationDeclHandler:
 *      @expat handler for @notation_declarations.
 *
 *      @@todo
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

STATIC void EXPATENTRY NotationDeclHandler(void *pUserData,      // in: our PXMLDOM really
                                           const XML_Char *pcszNotationName,
                                           const XML_Char *pcszBase,
                                           const XML_Char *pcszSystemId,
                                           const XML_Char *pcszPublicId)
{
    PXMLDOM     pDom = (PXMLDOM)pUserData;

    // continue parsing only if we had no errors so far
    if (!pDom->arcDOM)
    {
    }
}

/*
 *@@ ExternalEntityRefHandler:
 *      @expat handler for references to @external_entities.
 *
 *      This handler is also called for processing an external DTD
 *      subset if parameter entity parsing is in effect.
 *      (See XML_SetParamEntityParsing.)
 *
 *      The pcszContext argument specifies the parsing context in the
 *      format expected by the context argument to
 *      XML_ExternalEntityParserCreate; pcszContext is valid only until
 *      the handler returns, so if the referenced entity is to be
 *      parsed later, it must be copied.
 *
 *      The pcszBase parameter is the base to use for relative system
 *      identifiers. It is set by XML_SetBase and may be null.
 *
 *      The pcszPublicId parameter is the public id given in the entity
 *      declaration and may be null (since XML doesn't require public
 *      identifiers).
 *
 *      The pcszSystemId is the system identifier specified in the
 *      entity declaration and is never null. This is an exact copy
 *      of what was specified in the reference.
 *
 *      There are a couple of ways in which this handler differs
 *      from others. First, this handler returns an integer. A
 *      non-zero value should be returned for successful handling
 *      of the external entity reference. Returning a zero indicates
 *      failure, and causes the calling parser to return an
 *      ERROR_EXPAT_EXTERNAL_ENTITY_HANDLING error.
 *
 *      Second, instead of having pUserData as its first argument,
 *      it receives the parser that encountered the entity reference.
 *      This, along with the context parameter, may be used as
 *      arguments to a call to XML_ExternalEntityParserCreate.
 *      Using the returned parser, the body of the external entity
 *      can be recursively parsed.
 *
 *      Since this handler may be called recursively, it should not
 *      be saving information into global or static variables.
 *
 *      Your handler isn't actually responsible for parsing the entity,
 *      but it is responsible for creating a subsidiary parser with
 *      XML_ExternalEntityParserCreate that will do the job. That returns
 *      an instance of XML_Parser that has handlers and other data
 *      structures initialized from the parent parser. You may then use
 *      XML_Parse or XML_ParseBuffer calls against that parser. Since
 *      external entities may refer to other external entities, your
 *      handler should be prepared to be called recursively.
 *
 *@@added V0.9.14 (2001-08-09) [umoeller]
 *@@changed V0.9.20 (2002-07-06) [umoeller]: added automatic doctype support
 */

STATIC int EXPATENTRY ExternalEntityRefHandler(void *pUserData,      // in: our PXMLDOM really
                                               XML_Parser parser,
                                               const XML_Char *pcszContext,
                                               const XML_Char *pcszBase,
                                               const XML_Char *pcszSystemId,
                                               const XML_Char *pcszPublicId)
{
    PXMLDOM     pDom = (PXMLDOM)pUserData;

    int i = 0;          // return error per default

    // store the previous parser because
    // all the callbacks use the parser pointer
    XML_Parser pOldParser = pDom->pParser;
    pDom->pParser = NULL;

    if (    (    (pDom->pfnExternalHandler)
              || (pDom->cSystemIds)      // V0.9.20 (2002-07-06) [umoeller]
            )
            // create sub-parser and replace the one
            // in the DOM with it
         && (pDom->pParser = XML_ExternalEntityParserCreate(parser,
                                                            pcszContext,
                                                            "US-ASCII"))
       )
    {
        // run through the predefined doctypes given to us
        // in xmlCreateDOM, if any
        // V0.9.20 (2002-07-06) [umoeller]
        BOOL fCallExternal = TRUE;
        ULONG ul;

        for (ul = 0;
             ul < pDom->cSystemIds;
             ++ul)
        {
            const STATICSYSTEMID *pThis = &pDom->paSystemIds[ul];
            if (!strcmp(pThis->pcszSystemId, pcszSystemId))
            {
                // this one matches:
                // then parse the corresponding entry given
                // to us
                if (XML_Parse(pDom->pParser,
                              pThis->pcszContent,
                              strlen(pThis->pcszContent),
                              TRUE))
                    i = 1;      // success

                fCallExternal = FALSE;

                break;
            }
        }

        if (    (fCallExternal)     // not handled above
             && (pDom->pfnExternalHandler) // user handler set
           )
        {
            APIRET  arc;

            if (!(arc = pDom->pfnExternalHandler(pDom,
                                                 pDom->pParser,
                                                 pcszSystemId,
                                                 pcszPublicId)))
                i = 1;      // success
            else
            {
                // error:
                // now this needs special handling, since we're
                // dealing with a sub-handler here...

                if (arc == -1)
                    // parser error: well, then xmlSetError has been
                    // called from somewhere in the callbacks already,
                    // and we can safely ignore this
                    ;
                else
                {
                    pDom->arcDOM = arc;
                    if (pcszSystemId)
                    {
                        if (!pDom->pxstrFailingNode)
                            pDom->pxstrFailingNode = xstrCreate(0);
                        xstrcpy(pDom->pxstrFailingNode, pcszSystemId, 0);
                    }
                    pDom->pcszErrorDescription = xmlDescribeError(arc);
                    pDom->ulErrorLine = XML_GetCurrentLineNumber(pDom->pParser);
                    pDom->ulErrorColumn = XML_GetCurrentColumnNumber(pDom->pParser);
                }
            }
        }
    }
    else
        xmlSetError(pDom,
                    ERROR_DOM_INVALID_EXTERNAL_HANDLER,
                    NULL,
                    FALSE);

    if (pDom->pParser)
        XML_ParserFree(pDom->pParser);

    pDom->pParser = pOldParser;

    return i;
}

/*
 *@@ ElementDeclHandler:
 *      @expat handler for element declarations in a DTD. The
 *      handler gets called with the name of the element in
 *      the declaration and a pointer to a structure that contains
 *      the element model.
 *
 *      It is the application's responsibility to free this data
 *      structure. @@todo
 *
 *      The XML spec defines that no element may be declared more
 *      than once.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

STATIC void EXPATENTRY ElementDeclHandler(void *pUserData,      // in: our PXMLDOM really
                                          const XML_Char *pcszName,
                                          XMLCONTENT *pModel)
{
    PXMLDOM     pDom = (PXMLDOM)pUserData;

    // continue parsing only if we had no errors so far
    if (!pDom->arcDOM)
    {
        // OK, we're in a DOCTYPE node:
        PDOMDOCTYPENODE pDocType = pDom->pDocTypeNode;
        if (!pDocType)
            xmlSetError(pDom,
                        ERROR_DOM_ELEMENT_DECL_OUTSIDE_DOCTYPE,
                        pcszName,
                        TRUE);
        else
        {
            // create an element declaration and push it unto the
            // declarations tree
            PCMELEMENTDECLNODE pNew = NULL;
            if (!(pDom->arcDOM = xmlCreateElementDecl(pcszName,
                                                      pModel,
                                                      &pNew)))
                                    // this recurses!!
                                    // after this, pModel is invalid
            {
                // add this to the doctype's declarations tree
                if (treeInsert(&pDocType->ElementDeclsTree,
                               NULL,
                               (TREE*)pNew,
                               CompareXStrings))
                    // element already declared:
                    // according to the XML specs, this is a validity
                    // constraint, so we report a validation error
                    xmlSetError(pDom,
                                ERROR_DOM_DUPLICATE_ELEMENT_DECL,
                                pNew->Particle.NodeBase.strNodeName.psz,
                                TRUE);
            }
        }
    }
}

/*
 *@@ AddEnum:
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

STATIC APIRET AddEnum(PCMATTRIBUTEDECL pDecl,
                      const char *p,           // in: start of name
                      const char *pNext)       // in: end of name (not included)
{
    // PSZ pszType = strhSubstr(p, pNext);
    PNODEBASE pNew = NULL;
    APIRET arc;

    if (!(arc = xmlCreateNodeBase(ATTRIBUTE_DECLARATION_ENUM,
                                  sizeof(NODEBASE),
                                  p,
                                  (pNext - p),
                                  &pNew)))
        treeInsert(&pDecl->ValuesTree,
                   NULL,
                   (TREE*)pNew,
                   CompareXStrings);

    return arc;
}

/*
 *@@ AttlistDeclHandler:
 *      @expat handler for attlist declarations in the DTD.
 *
 *      This handler is called for each attribute. So a single attlist
 *      declaration with multiple attributes declared will generate
 *      multiple calls to this handler.
 *
 *      --  pcszElementName is the name of the  element for which the
 *          attribute is being declared.
 *
 *      --  pcszAttribName has the attribute name being declared.
 *
 *      --  pcszAttribType is the attribute type.
 *          It is the string representing the type in the declaration
 *          with whitespace removed.
 *
 *      --  pcszDefault holds the default value. It will be
 *          NULL in the case of "#IMPLIED" or "#REQUIRED" attributes.
 *          You can distinguish these two cases by checking the
 *          fIsRequired parameter, which will be true in the case of
 *          "#REQUIRED" attributes. Attributes which are "#FIXED"
 *          will have also have a TRUE fIsRequired, but they will have
 *          the non-NULL fixed value in the pcszDefault parameter.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

STATIC void EXPATENTRY AttlistDeclHandler(void *pUserData,      // in: our PXMLDOM really
                                          const XML_Char *pcszElementName,
                                          const XML_Char *pcszAttribName,
                                          const XML_Char *pcszAttribType,
                                          const XML_Char *pcszDefault,
                                          int fIsRequired)
{
    PXMLDOM     pDom = (PXMLDOM)pUserData;

    // continue parsing only if we had no errors so far
    if (!pDom->arcDOM)
    {
        // OK, we're in a DOCTYPE node:
        PDOMDOCTYPENODE pDocType = pDom->pDocTypeNode;
        if (!pDocType)
            xmlSetError(pDom,
                        ERROR_DOM_ATTLIST_DECL_OUTSIDE_DOCTYPE,
                        pcszElementName,
                        TRUE);
        else
        {
            PCMATTRIBUTEDECLBASE    pThis = NULL,
                                    pCache = pDom->pAttListDeclCache;

            // check if this is for the same attlist as the previous
            // call (we cache the pointer for speed)
            if (    (pCache)
                 && (!strhcmp(pCache->NodeBase.strNodeName.psz,
                              pcszElementName))
               )
                // this attdecl is for the same element:
                // use that (we won't have to search the tree)
                pThis = pDom->pAttListDeclCache;

            if (!pThis)
            {
                // cache didn't match: look up attributes tree then...
                // note: cheap trick, we need an XSTRING for treeFind
                // but don't want malloc, so we use xstrInitSet
                XSTRING strElementName;
                xstrInitSet(&strElementName, (PSZ)pcszElementName);
                if (!(pThis = (PCMATTRIBUTEDECLBASE)treeFind(
                                    pDocType->AttribDeclBasesTree,
                                    (ULONG)&strElementName,
                                    CompareXStrings)))
                {
                    // still not found:
                    // we need a new node then
                    if (!(pDom->arcDOM = xmlCreateNodeBase(ATTRIBUTE_DECLARATION_BASE,
                                                     sizeof(CMATTRIBUTEDECLBASE),
                                                     strElementName.psz,
                                                     strElementName.ulLength,
                                                     (PNODEBASE*)&pThis)))
                    {
                        // initialize the subtree
                        treeInit(&pThis->AttribDeclsTree, NULL);

                        treeInsert(&pDocType->AttribDeclBasesTree,
                                   NULL,
                                   (TREE*)pThis,
                                   CompareXStrings);
                    }
                }

                pDom->pAttListDeclCache = pThis;
            }

            if (pThis)
            {
                // pThis now has either an existing or a new CMATTRIBUTEDECLBASE;
                // add a new attribute def (CMATTRIBUTEDEDECL) to that
                PCMATTRIBUTEDECL  pNew = NULL;
                if (!(pDom->arcDOM = xmlCreateNodeBase(ATTRIBUTE_DECLARATION,
                                                       sizeof(CMATTRIBUTEDECL),
                                                       pcszAttribName,
                                                       0,
                                                       (PNODEBASE*)&pNew)))
                {
                    treeInit(&pNew->ValuesTree, NULL);

                    // check the type... expat is too lazy to parse this for
                    // us, so we must check manually. Expat only normalizes
                    // the "type" string to kick out whitespace, so we get:
                    // (TYPE1|TYPE2|TYPE3)
                    if (*pcszAttribType == '(')
                    {
                        // enumeration:
                        const char *p = pcszAttribType + 1,
                                   *pNext;
                        while (    (pNext = strchr(p, '|'))
                                && (!pDom->arcDOM)
                              )
                        {
                            pDom->arcDOM = AddEnum(pNew, p, pNext);
                            p = pNext + 1;
                        }

                        if (!pDom->arcDOM)
                        {
                            pNext = strchr(p, ')');
                            AddEnum(pNew, p, pNext);

                            pNew->ulAttrType = CMAT_ENUM;
                        }
                    }
                    else if (!strcmp(pcszAttribType, "CDATA"))
                        pNew->ulAttrType = CMAT_CDATA;
                    else if (!strcmp(pcszAttribType, "ID"))
                        pNew->ulAttrType = CMAT_ID;
                    else if (!strcmp(pcszAttribType, "IDREF"))
                        pNew->ulAttrType = CMAT_IDREF;
                    else if (!strcmp(pcszAttribType, "IDREFS"))
                        pNew->ulAttrType = CMAT_IDREFS;
                    else if (!strcmp(pcszAttribType, "ENTITY"))
                        pNew->ulAttrType = CMAT_ENTITY;
                    else if (!strcmp(pcszAttribType, "ENTITIES"))
                        pNew->ulAttrType = CMAT_ENTITIES;
                    else if (!strcmp(pcszAttribType, "NMTOKEN"))
                        pNew->ulAttrType = CMAT_NMTOKEN;
                    else if (!strcmp(pcszAttribType, "NMTOKENS"))
                        pNew->ulAttrType = CMAT_NMTOKENS;

                    if (!pDom->arcDOM)
                    {
                        if (pcszDefault)
                        {
                            // fixed or default:
                            if (fIsRequired)
                                // fixed:
                                pNew->ulConstraint = CMAT_FIXED_VALUE;
                            else
                                pNew->ulConstraint = CMAT_DEFAULT_VALUE;

                            pNew->pstrDefaultValue = xstrCreate(0);
                            xstrcpy(pNew->pstrDefaultValue, pcszDefault, 0);
                        }
                        else
                            // implied or required:
                            if (fIsRequired)
                                pNew->ulConstraint = CMAT_REQUIRED;
                            else
                                pNew->ulConstraint = CMAT_IMPLIED;

                        if (treeInsert(&pThis->AttribDeclsTree,
                                       NULL,
                                       (TREE*)pNew,
                                       CompareXStrings))
                            xmlSetError(pDom,
                                        ERROR_DOM_DUPLICATE_ATTRIBUTE_DECL,
                                        pcszAttribName,
                                        TRUE);
                    }
                }
            }
        }
    }
}

/*
 *@@ EntityDeclHandler:
 *      @expat handler that will be called for all entity declarations.
 *
 *      The fIsParameterEntity argument will be non-zero in the case
 *      of parameter entities and zero otherwise.
 *
 *      For internal entities (<!ENTITY foo "bar">), pcszValue will be
 *      non-NULL and pcszSystemId, pcszPublicId, and pcszNotationName
 *      will all be NULL. The value string is not NULL terminated; the
 *      length is provided in the iValueLength parameter. Do not use
 *      iValueLength to test for internal entities, since it is legal
 *      to have zero-length values. Instead check for whether or not
 *      pcszValue is NULL.
 *
 *      The pcszNotationName argument will have a non-NULL value only
 *      for unparsed entity declarations.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

STATIC void EXPATENTRY EntityDeclHandler(void *pUserData,      // in: our PXMLDOM really
                                         const XML_Char *pcszEntityName,
                                         int fIsParameterEntity,
                                         const XML_Char *pcszValue,
                                         int iValueLength,
                                         const XML_Char *pcszBase,
                                         const XML_Char *pcszSystemId,
                                         const XML_Char *pcszPublicId,
                                         const XML_Char *pcszNotationName)
{
    PXMLDOM     pDom = (PXMLDOM)pUserData;

    // continue parsing only if we had no errors so far
    if (!pDom->arcDOM)
    {
    }
}

/* ******************************************************************
 *
 *   DOM parser APIs
 *
 ********************************************************************/

/*
 *@@ xmlCreateDOM:
 *      creates an XMLDOM instance, which can be used for parsing
 *      an XML document and building a @DOM tree from it at the
 *      same time.
 *
 *      Pass the XMLDOM returned here to xmlParse afterwards.
 *
 *      Simplest possible usage:
 *
 *      1) Create a DOM instance.
 *
 +          PXMLDOM pDom = NULL;
 +          APIRET arc = xmlCreateDOM(flags, NULL, NULL, NULL, &pDom);
 +
 *      2) Give chunks of data (or an entire buffer)
 *         to the DOM instance for parsing.
 *
 +          arc = xmlParse(pDom,
 +                         pBuf,
 +                         TRUE); // if last, this will clean up the parser
 *
 *      3) Process the data in the DOM tree.
 *
 *         Look at the DOMNODE definition to see how you
 *         can traverse the data. Essentially, everything
 *         is based on linked lists and string maps.
 *
 *         A few helper functions have been added for
 *         quick lookup. See xmlGetRootElement,
 *         xmlGetFirstChild, xmlGetLastChild, xmlGetFirstText,
 *         xmlGetElementsByTagName, xmlGetAttribute.
 *
 *      4) When done, call xmlFreeDOM, which will free all memory.
 *
 *      The above code has limitations: only a few character
 *      @encodings are supported, and @external_entities are
 *      silently ignored.
 *
 *      This function supports a number of callbacks and flags
 *      to allow for maximum flexibility. Note however that
 *      not all @expat features are supported yet.
 *
 *      flParserFlags is any combination of the following:
 *
 *      --  DF_PARSECOMMENTS: XML @comments are to be returned in
 *          the DOM tree. Otherwise they are discarded.
 *
 *      --  DF_PARSEDTD: add the @DTD of the document into the DOM tree
 *          as well and validate the document, if a DTD was found.
 *          Otherwise just parse and do not validate.
 *
 *          DF_PARSEDTD is required for external entities to work
 *          also.
 *
 *      --  DF_FAIL_IF_NO_DTD: fail if no @DTD was found. Useful
 *          if you want to enforce validation. @@todo
 *
 *      --  DF_DROP_WHITESPACE: discard all @whitespace for those
 *          elements that can only have element content. Whitespace
 *          will be preserved only for elements that can have
 *          mixed content. -- If this flag is not set, all whitespace
 *          is preserved.
 *
 *      The following callbacks can be specified (any of these
 *      can be NULL):
 *
 *      --  pfnGetCPData should be specified if you want to
 *          support character @encodings other than the
 *          four that built into expat itself (see
 *          XML_SetUnknownEncodingHandler). This is probably
 *          a good idea to do under OS/2 since most OS/2
 *          documents are in a PC-specific codepage such as
 *          CP 850.
 *
 *          This callback must have the following prototype:
 *
 +              int APIENTRY FNGETCPDATA(PXMLDOM pDom, ULONG ulCP, int *piMap)
 *
 *          The callback will only be called once for each
 *          document if the "encoding" attribute of the
 *          XML @text_declaration starts with "cp" (e.g.
 *          "cp850") and will then receive the following
 *          parameters:
 *
 *          --  "pDom" will be the XMLDOM created by this function.
 *
 *          --  ulCP has the IBM code page number, such as "850".
 *
 *          --  piMap is an array of 256 integer values which must
 *              be filled with the callback. Each array item index
 *              is the codepage value, and the value of each field
 *              is the corresponding Unicode value, or -1 if the
 *              character is invalid (shouldn't happen with codepages).
 *
 *              For example, the German o-umlaut character is
 *              0x94 in CP850 and 0x00f6 in Unicode. So set
 *              the int at index 0x94 to 0x00f6.
 *
 *      --  pfnExternalHandler should be specified if you want the
 *          parser to be able to handle @external_entities. Since
 *          the parser has no concept of storage whatsoever, it is
 *          the responsibility of this callback to supply the parser
 *          with additional XML data when an external entity reference
 *          is encountered.
 *
 *          This callback must have the following prototype:
 *
 +              APIRET APIENTRY ParseExternal(PXMLDOM pDom,
 +                                            XML_Parser pSubParser,
 +                                            const char *pcszSystemID,
 +                                            const char *pcszPublicID)
 +
 *
 *          The callback will be called for each reference that refers
 *          to an external entity. pSubParser is a sub-parser created
 *          by the DOM engine, and pcszSystemID and pcszPublicID
 *          reference the external entity by means of a URI. As always
 *          with XML, the system ID is required, while the public ID is
 *          optional.
 *
 *          In the simplest case, this code could look as follows:
 *
 +              APIRET arc = ERROR_FILE_NOT_FOUND;
 +
 +              if (pcszSystemID)
 +              {
 +                  PSZ pszContents = NULL;
 +                  if (!(arc = doshLoadTextFile(pcszSystemID,
 +                                               &pszContents,
 +                                               NULL)))
 +                  {
 +                      if (!XML_Parse(pSubParser,
 +                                     pszContents,
 +                                     strlen(pszContents),
 +                                     TRUE))
 +                          arc = -1;
 +
 +                      free(pszContents);
 +                  }
 +              }
 +
 +              return arc;
 *
 *      --  pvCallbackUser is a user parameter which is simply stored
 *          in the XMLDOM struct which is returned. Since the XMLDOM
 *          is passed to all the callbacks, you can access that pointer
 *          from them.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 *@@changed V0.9.14 (2001-08-09) [umoeller]: added DF_DROP_WHITESPACE support
 *@@changed V0.9.20 (2002-07-06) [umoeller]: added static system IDs
 */

APIRET xmlCreateDOM(ULONG flParserFlags,            // in: DF_* parser flags
                    const STATICSYSTEMID *paSystemIds, // in: array of STATICSYSTEMID's or NULL
                    ULONG cSystemIds,                // in: array item count
                    PFNGETCPDATA pfnGetCPData,      // in: codepage callback or NULL
                    PFNEXTERNALHANDLER pfnExternalHandler, // in: external entity callback or NULL
                    PVOID pvCallbackUser,           // in: user param for callbacks
                    PXMLDOM *ppDom)                 // out: XMLDOM struct created
{
    APIRET      arc = NO_ERROR;
    PXMLDOM     pDom;
    PDOMNODE    pDocument = NULL;

    if (!(pDom = (PXMLDOM)malloc(sizeof(*pDom))))
        return ERROR_NOT_ENOUGH_MEMORY;

    memset(pDom, 0, sizeof(XMLDOM));

    pDom->flParserFlags = flParserFlags;
    pDom->pfnGetCPData = pfnGetCPData;
    pDom->pfnExternalHandler = pfnExternalHandler;
    pDom->pvCallbackUser = pvCallbackUser;

    // these added with V0.9.20 (2002-07-06) [umoeller]
    pDom->paSystemIds = paSystemIds;
    pDom->cSystemIds = cSystemIds;

    lstInit(&pDom->llElementStack,
            TRUE);                 // auto-free

    // create the document node
    if (!(arc = xmlCreateDomNode(NULL, // no parent
                                 DOMNODE_DOCUMENT,
                                 NULL,
                                 0,
                                 &pDocument)))
    {
        // store the document in the DOM
        pDom->pDocumentNode = (PDOMDOCUMENTNODE)pDocument;

        // push the document on the stack so the handlers
        // will append to that
        PushElementStack(pDom,
                         pDocument);

        pDom->pParser = XML_ParserCreate(NULL);

        if (!pDom->pParser)
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            if (pfnGetCPData)
                XML_SetUnknownEncodingHandler(pDom->pParser,
                                              UnknownEncodingHandler,
                                              pDom);        // user data

            XML_SetParamEntityParsing(pDom->pParser,
                                      XML_PARAM_ENTITY_PARSING_ALWAYS);

            XML_SetElementHandler(pDom->pParser,
                                  StartElementHandler,
                                  EndElementHandler);

            XML_SetCharacterDataHandler(pDom->pParser,
                                        CharacterDataHandler);

            // XML_SetProcessingInstructionHandler(XML_Parser parser,
            //                          XML_ProcessingInstructionHandler handler);


            if (flParserFlags & DF_PARSECOMMENTS)
                XML_SetCommentHandler(pDom->pParser,
                                      CommentHandler);

            if (    (pfnExternalHandler)
                 || (cSystemIds)     // V0.9.20 (2002-07-06) [umoeller]
               )
                XML_SetExternalEntityRefHandler(pDom->pParser,
                                                ExternalEntityRefHandler);

            if (flParserFlags & DF_PARSEDTD)
            {
                XML_SetDoctypeDeclHandler(pDom->pParser,
                                          StartDoctypeDeclHandler,
                                          EndDoctypeDeclHandler);

                XML_SetNotationDeclHandler(pDom->pParser,
                                           NotationDeclHandler);

                XML_SetElementDeclHandler(pDom->pParser,
                                          ElementDeclHandler);

                XML_SetAttlistDeclHandler(pDom->pParser,
                                          AttlistDeclHandler);

                XML_SetEntityDeclHandler(pDom->pParser,
                                         EntityDeclHandler);

                XML_SetParamEntityParsing(pDom->pParser,
                                          XML_PARAM_ENTITY_PARSING_ALWAYS);
            }

            // XML_SetXmlDeclHandler ... do we care for this? I guess not

            // pass the XMLDOM as user data to the handlers
            XML_SetUserData(pDom->pParser,
                            pDom);
        }
    }

    if (arc == NO_ERROR)
        *ppDom = pDom;
    else
        xmlFreeDOM(pDom);

    return arc;
}

/*
 *@@ xmlParse:
 *      parses another chunk of XML data.
 *
 *      If (fIsLast == TRUE), the internal @expat parser
 *      will be freed, but not the DOM itself.
 *
 *      You can pass an XML document to this function
 *      in one flush. Set fIsLast = TRUE on the first
 *      and only call then.
 *
 *      This returns NO_ERROR if the chunk was successfully
 *      parsed. Otherwise one of the following errors is
 *      returned:
 *
 *      -- ERROR_INVALID_PARAMETER
 *
 *      -- ERROR_DOM_PARSING: an @expat parsing error occurred.
 *         This might also be memory problems.
 *         With this error code, you will find specific
 *         error information in the XMLDOM fields.
 *
 *      -- ERROR_DOM_VALIDITY: the document is not @valid.
 *         This can only happen if @DTD parsing was enabled
 *         with xmlCreateDOM.
 *         With this error code, you will find specific
 *         error information in the XMLDOM fields.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

APIRET xmlParse(PXMLDOM pDom,               // in: DOM created by xmlCreateDOM
                const char *pcszBuf,        // in: chunk of XML document data (or full document)
                ULONG cb,                   // in: size of that chunk (required)
                BOOL fIsLast)               // in: set to TRUE if this is the last chunk
{
    APIRET arc = NO_ERROR;

    if (!pDom)
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        // go parse then
        if (!XML_Parse(pDom->pParser,
                       pcszBuf,
                       cb,
                       fIsLast))
        {
            // expat parsing error:
            xmlSetError(pDom,
                        XML_GetErrorCode(pDom->pParser),
                        NULL,
                        FALSE);

            if (pDom->pDocumentNode)
            {
                xmlDeleteNode((PNODEBASE)pDom->pDocumentNode);
                pDom->pDocumentNode = NULL;
            }

            arc = ERROR_DOM_PARSING;
        }
        else if (pDom->fInvalid)
        {
            // expat was doing OK, but the handlers' validation failed:
            arc = ERROR_DOM_VALIDITY;
                        // error info has already been set
        }
        else
            // expat was doing OK, but maybe we have integrity errors
            // from our DOM callbacks:
            if (pDom->arcDOM)
                arc = pDom->arcDOM;

        if (arc != NO_ERROR || fIsLast)
        {
            // last call or error: clean up
            XML_ParserFree(pDom->pParser);
            pDom->pParser = NULL;

            // clean up the stack (but not the DOM itself)
            lstClear(&pDom->llElementStack);
        }
    }

    return arc;
}

#ifdef __DEBUG__

/*
 *@@ Dump:
 *
 *@@added V0.9.20 (2002-07-03) [umoeller]
 */

STATIC VOID Dump(int iIndent,
                 PDOMNODE pDomNode)
{
    PLISTNODE pChildNode;
    int i;
    for (i = 0;
         i < iIndent;
         ++i)
    {
        printf(" ");
    }

    switch (pDomNode->NodeBase.ulNodeType)
    {
        #define DUMPTYPE(t) case t: printf(#t); break;
        DUMPTYPE(DOMNODE_ELEMENT)
        DUMPTYPE(DOMNODE_ATTRIBUTE)
        DUMPTYPE(DOMNODE_TEXT)
        DUMPTYPE(DOMNODE_PROCESSING_INSTRUCTION)
        DUMPTYPE(DOMNODE_COMMENT)
        DUMPTYPE(DOMNODE_DOCUMENT)
        DUMPTYPE(DOMNODE_DOCUMENT_TYPE)
        DUMPTYPE(ELEMENTPARTICLE_EMPTY)
        DUMPTYPE(ELEMENTPARTICLE_ANY)
        DUMPTYPE(ELEMENTPARTICLE_MIXED)
        DUMPTYPE(ELEMENTPARTICLE_CHOICE)
        DUMPTYPE(ELEMENTPARTICLE_SEQ)
        DUMPTYPE(ELEMENTPARTICLE_NAME)
        DUMPTYPE(ATTRIBUTE_DECLARATION_BASE)
        DUMPTYPE(ATTRIBUTE_DECLARATION)
        DUMPTYPE(ATTRIBUTE_DECLARATION_ENUM)
    }

    printf(" \"%s\"\n", STRINGORNULL(pDomNode->NodeBase.strNodeName.psz));

    ++iIndent;
    for (pChildNode = lstQueryFirstNode(&pDomNode->llChildren);
         pChildNode;
         pChildNode = pChildNode->pNext)
    {
        Dump(iIndent, (PDOMNODE)pChildNode->pItemData);
    }
    --iIndent;
}

#endif

/*
 *@@ xmlDump:
 *      debug function which dumps the DOM to stdout.
 *
 *@@added V0.9.20 (2002-07-03) [umoeller]
 */

VOID xmlDump(PXMLDOM pDom)
{
#ifdef __DEBUG__
    if (!pDom)
    {
        printf(__FUNCTION__ ": pDom is NULL\n");
        return;
    }

    printf(__FUNCTION__ ": dumping document node ");

    Dump(0, (PDOMNODE)pDom->pDocumentNode);
#endif
}

/*
 *@@ xmlFreeDOM:
 *      cleans up all resources allocated by
 *      xmlCreateDOM and xmlParse, including
 *      the entire DOM tree.
 *
 *      If you wish to keep any data, make
 *      a copy of the respective pointers in pDom
 *      or subitems and set them to NULL before
 *      calling this function.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

APIRET xmlFreeDOM(PXMLDOM pDom)
{
    APIRET arc = NO_ERROR;
    if (pDom)
    {
        // if the parser is still alive for some reason, close it.
        if (pDom->pParser)
        {
            XML_ParserFree(pDom->pParser);
            pDom->pParser = NULL;
        }

        xmlDeleteNode((PNODEBASE)pDom->pDocumentNode);

        if (pDom->pxstrSystemID)
            xstrFree(&pDom->pxstrSystemID);
        if (pDom->pxstrFailingNode)
            xstrFree(&pDom->pxstrFailingNode);

        lstClear(&pDom->llElementStack);

        free(pDom);
    }

    return arc;
}

/* ******************************************************************
 *
 *   DOM lookup
 *
 ********************************************************************/

/*
 *@@ xmlFindElementDecl:
 *      returns the CMELEMENTDECLNODE for the element
 *      with the specified name or NULL if there's none.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

PCMELEMENTDECLNODE xmlFindElementDecl(PXMLDOM pDom,
                                      const XSTRING *pcstrElementName)
{
    PCMELEMENTDECLNODE pElementDecl = NULL;

    PDOMDOCTYPENODE pDocTypeNode = pDom->pDocTypeNode;
    if (    (pDocTypeNode)
         && (pcstrElementName)
         && (pcstrElementName->ulLength)
       )
    {
        pElementDecl = (PCMELEMENTDECLNODE)treeFind(
                                      pDocTypeNode->ElementDeclsTree,
                                      (ULONG)pcstrElementName,
                                      CompareXStrings);
    }

    return pElementDecl;
}

/*
 *@@ xmlFindAttribDeclBase:
 *      returns the CMATTRIBUTEDECLBASE for the specified
 *      element name, or NULL if none exists.
 *
 *      To find a specific attribute declaration from both
 *      an element and an attribute name, use xmlFindAttribDecl
 *      instead.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

PCMATTRIBUTEDECLBASE xmlFindAttribDeclBase(PXMLDOM pDom,
                                           const XSTRING *pstrElementName)
{
    PDOMDOCTYPENODE pDocTypeNode = pDom->pDocTypeNode;
    if (    (pDocTypeNode)
         && (pstrElementName)
         && (pstrElementName->ulLength)
       )
    {
        return (PCMATTRIBUTEDECLBASE)treeFind(pDocTypeNode->AttribDeclBasesTree,
                                              (ULONG)pstrElementName,
                                              CompareXStrings);
    }

    return NULL;
}

/*
 *@@ xmlFindAttribDecl:
 *      returns the CMATTRIBUTEDEDECL for the specified
 *      element and attribute name, or NULL if none exists.
 *
 *@@added V0.9.9 (2001-02-16) [umoeller]
 */

PCMATTRIBUTEDECL xmlFindAttribDecl(PXMLDOM pDom,
                                   const XSTRING *pstrElementName,
                                   const XSTRING *pstrAttribName,
                                   PCMATTRIBUTEDECLBASE *ppAttribDeclBase)
                                            // in/out: attr decl base cache;
                                            // the pointer pointed to by this
                                            // must be NULL on the first call
{
    if (pstrElementName && pstrAttribName)
    {
        if (!*ppAttribDeclBase)
            // first call for this:
            *ppAttribDeclBase = xmlFindAttribDeclBase(pDom,
                                                      pstrElementName);
        if (*ppAttribDeclBase)
        {
            return (PCMATTRIBUTEDECL)treeFind(((**ppAttribDeclBase).AttribDeclsTree),
                                              (ULONG)pstrAttribName,
                                              CompareXStrings);
        }
    }

    return NULL;
}

/*
 *@@ xmlGetRootElement:
 *      returns the root element node from the specified
 *      DOM. Useful helper to start enumerating elements.
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 *@@changed V0.9.20 (2002-07-03) [umoeller]: this never worked with DTDs, fixed
 */

PDOMNODE xmlGetRootElement(PXMLDOM pDom)
{
    PDOMDOCUMENTNODE    pDocumentNode;
    PLISTNODE           pListNode;
    if (    (pDom)
         && (pDocumentNode = pDom->pDocumentNode)
         && (pListNode = lstQueryFirstNode(&pDocumentNode->DomNode.llChildren))
       )
    {
        // V0.9.20 (2002-07-03) [umoeller]:
        // we can't just return the first node on the
        // list, because if we have DTD, this might
        // be the doctype... so loop until we find
        // an element, which must be the root element
        while (pListNode)
        {
            PDOMNODE pDomNode = (PDOMNODE)pListNode->pItemData;
            if (pDomNode->NodeBase.ulNodeType == DOMNODE_ELEMENT)
                return pDomNode;

            pListNode = pListNode->pNext;
        }
    }

    return NULL;
}

/*
 *@@ xmlGetFirstChild:
 *      returns the first child node of pDomNode.
 *      See DOMNODE for what a "child" can be for the
 *      various node types.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

PDOMNODE xmlGetFirstChild(PDOMNODE pDomNode)
{
    PLISTNODE pListNode = lstQueryFirstNode(&pDomNode->llChildren);
    if (pListNode)
        return (PDOMNODE)pListNode->pItemData;

    return 0;
}

/*
 *@@ xmlGetLastChild:
 *      returns the last child node of pDomNode.
 *      See DOMNODE for what a "child" can be for the
 *      various node types.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

PDOMNODE xmlGetLastChild(PDOMNODE pDomNode)
{
    PLISTNODE pListNode = lstQueryLastNode(&pDomNode->llChildren);
    if (pListNode)
        return (PDOMNODE)pListNode->pItemData;

    return 0;
}

/*
 *@@ xmlGetFirstText:
 *      returns the first text (character data) node
 *      of pElement or NULL if there's none.
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 */

PDOMNODE xmlGetFirstText(PDOMNODE pElement)
{
    PLISTNODE   pNode;
    PDOMNODE    pDomNodeThis;

    for (pNode = lstQueryFirstNode(&pElement->llChildren);
         pNode;
         pNode = pNode->pNext)
    {
        if (    (pDomNodeThis = (PDOMNODE)pNode->pItemData)
             && (pDomNodeThis->NodeBase.ulNodeType == DOMNODE_TEXT)
           )
            return pDomNodeThis;
    }

    return NULL;
}

/*
 *@@ xmlGetElementsByTagName:
 *      returns a linked list of DOMNODE nodes which
 *      match the specified element name. The special name
 *      "*" matches all elements.
 *
 *      pParent must be the parent element DOMNODE...
 *      the only allowed exception is
 *
 *      The caller must free the list by calling lstFree.
 *      Returns NULL if no such elements could be found.
 *
 *@@added V0.9.9 (2001-02-14) [umoeller]
 */

PLINKLIST xmlGetElementsByTagName(PDOMNODE pParent,
                                  const char *pcszName)
{
    PLINKLIST pll = lstCreate(FALSE);       // no free
    if (pll)
    {
        ULONG   cItems = 0;
        BOOL    fFindAll = !strcmp(pcszName, "*");

        PLISTNODE   pNode;
        PDOMNODE    pDomNodeThis;

        for (pNode = lstQueryFirstNode(&pParent->llChildren);
             pNode;
             pNode = pNode->pNext)
        {
            if (    (pDomNodeThis = (PDOMNODE)pNode->pItemData)
                 && (pDomNodeThis->NodeBase.ulNodeType == DOMNODE_ELEMENT)
                 && (    (fFindAll)
                      || (!strcmp(pcszName, pDomNodeThis->NodeBase.strNodeName.psz))
                    )
               )
            {
                // element matches:
                lstAppendItem(pll, pDomNodeThis);
                cItems++;
            }
        }

        if (cItems)
            return pll;

        lstFree(&pll);
    }

    return 0;
}

/*
 *@@ xmlGetAttribute:
 *      returns the value of pElement's attribute
 *      with the given name or NULL.
 *
 *      This is a const pointer into the element's
 *      attribute list.
 *
 *@@added V0.9.11 (2001-04-22) [umoeller]
 */

const XSTRING* xmlGetAttribute(PDOMNODE pElement,
                               const char *pcszAttribName)
{
    XSTRING str;
    PDOMNODE pAttrNode;
    xstrInitSet(&str, (PSZ)pcszAttribName);
    // note, cheap trick: no malloc here, but we need
    // an XSTRING for treeFind
    if (pAttrNode = (PDOMNODE)treeFind(pElement->AttributesMap,
                                       (ULONG)&str,
                                       CompareXStrings))
        return pAttrNode->pstrNodeValue;

    return NULL;
}

/* ******************************************************************
 *
 *   DOM build
 *
 ********************************************************************/

/*
 *@@ xmlCreateDocument:
 *      creates a new XML document.
 *
 *      This is the first step in creating a DOM
 *      tree in memory.
 *
 *      This function creates a DOCUMENT node
 *      (which is returned) and a root ELEMENT node
 *      within the document. For convenience, the
 *      root ELEMENT node is returned as well.
 *
 *      This does not create a DOCTYPE node in
 *      the document.
 *
 *      After this, you can add sub-elements and
 *      attributes to the root element using
 *      xmlCreateElementNode and xmlCreateAttributeNode.
 *
 *      Use xmlDeleteNode on the DOCUMENT node
 *      to delete the entire DOM tree.
 *
 *@@added V0.9.12 (2001-05-21) [umoeller]
 */

APIRET xmlCreateDocument(const char *pcszRootElementName,   // in: root element name
                         PDOMDOCUMENTNODE *ppDocument,      // out: DOCUMENT node
                         PDOMNODE *ppRootElement)           // out: root ELEMENT node within DOCUMENT
{
    APIRET arc = NO_ERROR;
    PDOMDOCUMENTNODE pDocument = NULL;
    PDOMNODE pRootElement = NULL;

    if ( (!pcszRootElementName) || (!ppDocument) || (!ppRootElement) )
        arc = ERROR_INVALID_PARAMETER;
    else
        // create the document node
        if (!(arc = xmlCreateDomNode(NULL, // no parent
                                     DOMNODE_DOCUMENT,
                                     NULL,
                                     0,
                                     (PDOMNODE*)&pDocument)))
            if (!(arc = xmlCreateDomNode((PDOMNODE)pDocument,     // parent
                                         DOMNODE_ELEMENT,
                                         pcszRootElementName,
                                         0,
                                         &pRootElement)))
            {
                *ppDocument = pDocument;
                *ppRootElement = pRootElement;
            }

    return arc;
}

/*
 *@@ ESCAPES:
 *
 *@@added V1.0.0 (2002-08-21) [umoeller]
 */

typedef struct _ESCAPES
{
    XSTRING strQuot1,       // "
            strQuot2,       // &quot;
            strAmp1,        // &
            strAmp2,        // &amp;
            strLT1,         // <
            strLT2,         // &lt;
            strGT1,         // >
            strGT2;         // &gt;

    XSTRING strTemp;        // temp buffer

} ESCAPES, *PESCAPES;

/*
 *@@ DoEscapes:
 *
 *@@added V1.0.0 (2002-08-21) [umoeller]
 */

VOID DoEscapes(PESCAPES pEscapes,
               BOOL fQuotesToo)
{
    ULONG   ulOfs;
    size_t  ShiftTable[256];
    BOOL    fRepeat;

    if (fQuotesToo)
    {
        ulOfs = 0;
        fRepeat = FALSE;
        while (xstrFindReplace(&pEscapes->strTemp,
                               &ulOfs,
                               &pEscapes->strQuot1,
                               &pEscapes->strQuot2,
                               ShiftTable,
                               &fRepeat))
            ;
    }

    ulOfs = 0;
    fRepeat = FALSE;
    while (xstrFindReplace(&pEscapes->strTemp,
                           &ulOfs,
                           &pEscapes->strLT1,
                           &pEscapes->strLT2,
                           ShiftTable,
                           &fRepeat))
        ;

    ulOfs = 0;
    fRepeat = FALSE;
    while (xstrFindReplace(&pEscapes->strTemp,
                           &ulOfs,
                           &pEscapes->strGT1,
                           &pEscapes->strGT2,
                           ShiftTable,
                           &fRepeat))
        ;

    // replace ampersands last
    ulOfs = 0;
    fRepeat = FALSE;
    while (xstrFindReplace(&pEscapes->strTemp,
                           &ulOfs,
                           &pEscapes->strAmp1,
                           &pEscapes->strAmp2,
                           ShiftTable,
                           &fRepeat))
        ;
}

/*
 *@@ WriteNodes:
 *      internal helper for writing out the nodes.
 *      This recurses.
 *
 *@@added V0.9.12 (2001-05-21) [umoeller]
 *@@changed V1.0.0 (2002-08-21) [umoeller]: changed prototype, fixed unescaped characters in attributes and content
 */

STATIC VOID WriteNodes(PXSTRING pxstr,
                       PESCAPES pEscapes,
                       PDOMNODE pDomNode)       // in: node whose children are to be written (initially DOCUMENT)
{
    PLISTNODE pListNode;

    BOOL fMixedContent = (xmlGetFirstText(pDomNode) != NULL);

    for (pListNode = lstQueryFirstNode(&pDomNode->llChildren);
         (pListNode);
         pListNode = pListNode->pNext)
    {
        PDOMNODE pChildNode = (PDOMNODE)pListNode->pItemData;

        switch (pChildNode->NodeBase.ulNodeType)
        {
            case DOMNODE_ELEMENT:
            {
                PDOMNODE pAttribNode;
                // write out opening ELEMENT tag
                // add a line break if this does NOT have mixed
                // content
                if (!fMixedContent)
                    xstrcatc(pxstr, '\n');

                xstrcatc(pxstr, '<');
                xstrcats(pxstr, &pChildNode->NodeBase.strNodeName);

                // go through attributes
                for (pAttribNode = (PDOMNODE)treeFirst(pChildNode->AttributesMap);
                     (pAttribNode);
                     pAttribNode = (PDOMNODE)treeNext((TREE*)pAttribNode))
                {
                    xstrcat(pxstr, "\n    ", 0);
                    xstrcats(pxstr, &pAttribNode->NodeBase.strNodeName);
                    xstrcat(pxstr, "=\"", 0);

                    // copy attribute value to temp buffer first
                    // so we can escape quotes and ampersands
                    // V1.0.0 (2002-08-21) [umoeller]
                    xstrcpys(&pEscapes->strTemp, pAttribNode->pstrNodeValue);

                    DoEscapes(pEscapes,
                              TRUE);        // quotes too

                    // alright, use that
                    xstrcats(pxstr, &pEscapes->strTemp);
                    xstrcatc(pxstr, '\"');
                }

                // now check... do we have child nodes?
                if (lstCountItems(&pChildNode->llChildren))
                {
                    // yes:
                    xstrcatc(pxstr, '>');

                    // recurse into this child element
                    WriteNodes(pxstr, pEscapes, pChildNode);

                    if (!fMixedContent)
                        xstrcatc(pxstr, '\n');

                    // write closing tag
                    xstrcat(pxstr, "</", 0);
                    xstrcats(pxstr, &pChildNode->NodeBase.strNodeName);
                    xstrcatc(pxstr, '>');
                }
                else
                {
                    // no child nodes:
                    // mark this tag as "empty"
                    xstrcat(pxstr, "/>", 0);
                }
            }
            break;

            case DOMNODE_TEXT:
            case DOMNODE_COMMENT:
                // that's simple
                xstrcpys(&pEscapes->strTemp,
                         pChildNode->pstrNodeValue);

                DoEscapes(pEscapes,         // V1.0.0 (2002-08-21) [umoeller]
                          FALSE);           // quotes not

                xstrcats(pxstr, &pEscapes->strTemp);
            break;

            case DOMNODE_DOCUMENT_TYPE:
                // @@todo
            break;

        }
    }
}

/*
 *@@ xmlWriteDocument:
 *      creates a complete XML document in the specified
 *      string buffer from the specified DOMDOCUMENTNODE.
 *
 *      This creates a full XML document, starting with
 *      the <?xml...?> header, the DTD (if present),
 *      and the elements and attributes.
 *
 *      The input XSTRING must be initialized. Its
 *      contents will be overwritten, if any exists.
 *
 *      Sooo... to write a full XML document to disk,
 *      do the following:
 *
 *      1)  Call xmlCreateDocument to have an empty
 *          document with a root element created.
 *
 *      2)  Add elements, subelements, and attributes
 *          using xmlCreateElementNode and
 *          xmlCreateAttributeNode.
 *
 *      3)  Call xmlWriteDocument to have the XML
 *          document written into an XSTRING.
 *
 *      4)  Write the XSTRING to disk, e.g. using
 *          fwrite().
 *
 *          Note: You can also use doshWriteTextFile,
 *          but you should then first convert the
 *          line format using xstrConvertLineFormat.
 *
 *      Example:
 *
 +          APIRET arc = NO_ERROR;
 +          PDOMDOCUMENTNODE pDocument = NULL;
 +          PDOMNODE pRootElement = NULL;
 +
 +          // create a DOM
 +          if (!(arc = xmlCreateDocument("MYROOTNODE",
 +                                        &pDocument,
 +                                        &pRootElement)))
 +          {
 +              // add subelements to the root element
 +              PDOMNODE pSubelement;
 +              if (!(arc = xmlCreateElementNode(pRootElement,
 +                                               "MYSUBELEMENT",
 +                                               &pSubelement)))
 +              {
 +                  // add an attribute
 +                  PDOMNODE pAttribute;
 +                  if (!(arc = xmlCreateAttributeNode(pSubElement,
 +                                                     "MYATTRIB",
 +                                                     "VALUE",
 +                                                     &pAttribute)))
 +                  {
 +                      // alright, turn this into a string
 +                      XSTRING str;
 +                      xstrInit(&str, 1000);
 +                      if (!(arc = xmlWriteDocument(pDocument,
 +                                                   "ISO-8859-1",
 +                                                   NULL,      // or DOCTYPE
 +                                                   &str)))
 +                      {
 +                          FILE *file = fopen("myfile.xml", "w");
 +                          fwrite(str.psz,
 +                                 1,
 +                                 str.ulLength,
 +                                 file);
 +                          fclose(file);
 +                      }
 +                  }
 +              }
 +
 +              // this kills the entire tree
 +              xmlDeleteNode((PNODEBASE)pDocument);
 +
 +          }
 +
 *
 *      A note about whitespace handling. Presently, this
 *      adds line breaks after the opening tag of an
 *      element if the element has element content only.
 *      However, if the element has mixed content, this
 *      line break is NOT automatically added because
 *      white space may then be significant.
 *
 *@@added V0.9.12 (2001-05-21) [umoeller]
 */

APIRET xmlWriteDocument(PDOMDOCUMENTNODE pDocument,     // in: document node
                        const char *pcszEncoding,       // in: encoding string (e.g. "ISO-8859-1")
                        const char *pcszDoctype,        // in: entire DOCTYPE statement or NULL
                        PXSTRING pxstr)                 // out: document
{
    APIRET arc = NO_ERROR;

    if ( (!pDocument) || (!pcszEncoding) || (!pxstr) )
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        ESCAPES esc;

        // <?xml version="1.0" encoding="ISO-8859-1"?>
        xstrcpy(pxstr, "<?xml version=\"1.0\" encoding=\"", 0);
        xstrcat(pxstr, pcszEncoding, 0);
        xstrcat(pxstr, "\"?>\n", 0);

        // write entire DOCTYPE statement
        if (pcszDoctype)
        {
            xstrcatc(pxstr, '\n');
            xstrcat(pxstr, pcszDoctype, 0);
            xstrcatc(pxstr, '\n');
        }

        xstrInitCopy(&esc.strQuot1, "\"", 0);
        xstrInitCopy(&esc.strQuot2, "&quot;", 0);
        xstrInitCopy(&esc.strAmp1, "&", 0);
        xstrInitCopy(&esc.strAmp2, "&amp;", 0);
        xstrInitCopy(&esc.strLT1, "<", 0);
        xstrInitCopy(&esc.strLT2, "&lt;", 0);
        xstrInitCopy(&esc.strGT1, ">", 0);
        xstrInitCopy(&esc.strGT2, "&gt;", 0);

        xstrInit(&esc.strTemp, 0);       // temp buffer

        // write out children
        WriteNodes(pxstr, &esc, (PDOMNODE)pDocument);

        xstrClear(&esc.strQuot1);
        xstrClear(&esc.strQuot2);
        xstrClear(&esc.strAmp1);
        xstrClear(&esc.strAmp2);
        xstrClear(&esc.strLT1);
        xstrClear(&esc.strLT2);
        xstrClear(&esc.strGT1);
        xstrClear(&esc.strGT2);

        xstrClear(&esc.strTemp);       // temp buffer

        xstrcatc(pxstr, '\n');
    }

    return arc;
}


