
/*
 *@@sourcefile filetype.c:
 *      extended file types implementation code. This has
 *      the method implementations for XFldDataFile.
 *
 *      This has the complete engine for the extended
 *      file type associations, i.e. associating filters
 *      with types, and types with program objects.
 *
 *      This file is ALL new with V0.9.0.
 *
 *      There are several entry points into this mess:
 *
 *      --  ftypQueryAssociatedProgram gets called from
 *          XFldDataFile::wpQueryAssociatedProgram and also
 *          from XFldDataFile::wpOpen. This must return a
 *          single association according to a given view ID.
 *
 *      --  ftypModifyDataFileOpenSubmenu gets called from
 *          the XFldDataFile menu methods to hack the
 *          "Open" submenu.
 *
 *      --  BuildAssocsList could be called separately
 *          to build a complete associations list for an
 *          object.
 *
 *      Function prefix for this file:
 *      --  ftyp*
 *
 *@@added V0.9.0 [umoeller]
 *@@header "filesys\filetype.h"
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
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

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSNLS
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINDIALOGS
#define INCL_WININPUT           // WM_CHAR
#define INCL_WINPOINTERS
#define INCL_WINPROGRAMLIST     // needed for wppgm.h
#define INCL_WINSHELLDATA       // Prf* functions
#define INCL_WINMENUS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINLISTBOXES
#define INCL_WINSTDCNR
#define INCL_WINSTDDRAG
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <io.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\tree.h"               // red-black binary trees
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles
#include "helpers\xstring.h"            // extended string helpers

#include "expat\expat.h"                // XWPHelpers expat XML parser
#include "helpers\xml.h"                // XWPHelpers XML engine

// SOM headers which don't crash with prec. header files
// #include "xfobj.ih"
#include "xfwps.ih"
#include "xfdataf.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\cnrsort.h"             // container sort comparison functions
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#define FILETYPE_PRIVATE
#include "filesys\filetype.h"           // extended file types implementation
#include "filesys\object.h"             // XFldObject implementation

// other SOM headers
#pragma hdrstop                 // VAC++ keeps crashing otherwise
#include <wppgm.h>                      // WPProgram
#include <wppgmf.h>                     // WPProgramFile
#include "filesys\program.h"            // program implementation; WARNING: this redefines macros

/* ******************************************************************
 *
 *   Additional declarations
 *
 ********************************************************************/

/*
 *@@ INSTANCETYPE:
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

typedef struct _INSTANCETYPE
{
    TREE        Tree;               // ulKey is a PSZ to szUpperType
    SOMClass    *pClassObject;
    CHAR        szUpperType[1];     // upper-cased type string; struct
                                    // is dynamic in size
} INSTANCETYPE, *PINSTANCETYPE;

/*
 *@@ INSTANCEFILTER:
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

typedef struct _INSTANCEFILTER
{
    SOMClass    *pClassObject;
    CHAR        szFilter[1];        // filter string; struct is dynamic in size
} INSTANCEFILTER, *PINSTANCEFILTER;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

static HMTX     G_hmtxInstances = NULLHANDLE;
static TREE     *G_InstanceTypesTreeRoot = NULL;
static LONG     G_cInstanceTypes;
static LINKLIST G_llInstanceFilters;

static PCSZ     G_pcszCODEPAGE = "CODEPAGE";

#define CID_SYSTEMID "xwpftyp.dtd"

static PCSZ G_pcszDoctype =
"<!DOCTYPE XWPFILETYPES SYSTEM \"" CID_SYSTEMID "\">";

static const STATICSYSTEMID G_aSysIds[] =
    {
        CID_SYSTEMID,
        "<!ELEMENT XWPFILETYPES (TYPE*)>\n"
        "    <!ATTLIST XWPFILETYPES\n"
        "            CODEPAGE NMTOKEN '850' >\n"
        "<!ELEMENT TYPE (TYPE* | FILTER*)>\n"
        "    <!ATTLIST TYPE\n"
        "            NAME CDATA #REQUIRED >\n"
        "<!ELEMENT FILTER EMPTY>\n"
        "    <!ATTLIST FILTER\n"
        "            VALUE CDATA #IMPLIED>\n"
    };

/* ******************************************************************
 *
 *   Class types and filters
 *
 ********************************************************************/

/*
 *@@ LockInstances:
 *      locks the association caches.
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 */

STATIC BOOL LockInstances(VOID)
{
    if (G_hmtxInstances)
        return !DosRequestMutexSem(G_hmtxInstances, SEM_INDEFINITE_WAIT);

    // first call:
    if (!DosCreateMutexSem(NULL,
                           &G_hmtxInstances,
                           0,
                           TRUE))     // lock!
    {
        treeInit(&G_InstanceTypesTreeRoot,
                 &G_cInstanceTypes);
        lstInit(&G_llInstanceFilters,
                TRUE);         // auto-free
        return TRUE;
    }

    return FALSE;
}

/*
 *@@ UnlockInstances:
 *      unlocks the association caches.
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 */

STATIC VOID UnlockInstances(VOID)
{
    DosReleaseMutexSem(G_hmtxInstances);
}

/*
 *@@ ftypRegisterInstanceTypesAndFilters:
 *      called by M_XWPFileSystem::wpclsInitData
 *      to register the instance type and filters
 *      of a file-system class. These are then
 *      later used by turbo populate to make files
 *      instances of those classes.
 *
 *      Returns the total no. of classes and filters
 *      found or 0 if none.
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

ULONG ftypRegisterInstanceTypesAndFilters(M_WPFileSystem *pClassObject)
{
    BOOL fLocked = FALSE;
    ULONG ulrc = 0;

    TRY_LOUD(excpt1)
    {
        PSZ pszTypes = NULL,
            pszFilters = NULL;

        PMPF_TURBOFOLDERS(("entering for class %s", _somGetName(pClassObject)));

        // go for the types
        if (    (pszTypes = _wpclsQueryInstanceType(pClassObject))
             && (*pszTypes)     // many classes return ""
           )
        {
            // this is separated by commas
            PCSZ pStart = pszTypes;
            BOOL fQuit;

            do
            {
                PSZ pEnd;
                ULONG ulLength;
                PINSTANCETYPE pNew;

                fQuit = TRUE;

                // skip spaces
                while (    (*pStart)
                        && (*pStart == ' ')
                      )
                    pStart++;

                if (pEnd = strchr(pStart, ','))
                    ulLength = pEnd - pStart;
                else
                    ulLength = strlen(pStart);

                if (    (ulLength)
                     && (pNew = (PINSTANCETYPE)malloc(   sizeof(INSTANCETYPE)
                                                                // has one byte for
                                                                // null termn. already
                                                       + ulLength))
                   )
                {
                    memcpy(pNew->szUpperType,
                           pStart,
                           ulLength);
                    pNew->szUpperType[ulLength] = '\0';
                    nlsUpper(pNew->szUpperType);

                    // store pointer to string in ulKey
                    pNew->Tree.ulKey = (ULONG)pNew->szUpperType;

                    pNew->pClassObject = pClassObject;

                    PMPF_TURBOFOLDERS(("    found type %s", pNew->szUpperType));

                    if (!fLocked)
                        fLocked = LockInstances();

                    if (fLocked)
                    {
                        if (treeInsert(&G_InstanceTypesTreeRoot,
                                       &G_cInstanceTypes,
                                       (TREE*)pNew,
                                       treeCompareStrings))
                        {
                            // already registered:
                            free(pNew);
                        }
                        else
                        {
                            // got something:
                            ulrc++;
                        }

                        if (pEnd)       // points to comma
                        {
                            pStart = pEnd + 1;
                            fQuit = FALSE;
                        }
                    }
                }
            } while (!fQuit);

        } // end if (    (pszTypes = _wpclsQueryInstanceType(pClassObject))

        // go for the filters
        if (    (pszFilters = _wpclsQueryInstanceFilter(pClassObject))
             && (*pszFilters)     // many classes return ""
           )
        {
            // this is separated by commas
            PCSZ pStart = pszFilters;
            BOOL fQuit;

            do
            {
                PSZ pEnd;
                ULONG ulLength;
                PINSTANCEFILTER pNew;

                fQuit = TRUE;

                // skip spaces
                while (    (*pStart)
                        && (*pStart == ' ')
                      )
                    pStart++;

                if (pEnd = strchr(pStart, ','))
                    ulLength = pEnd - pStart;
                else
                    ulLength = strlen(pStart);

                if (    (ulLength)
                     && (pNew = (PINSTANCEFILTER)malloc(   sizeof(INSTANCEFILTER)
                                                                // has one byte for
                                                                // null termn. already
                                                         + ulLength))
                   )
                {
                    memcpy(pNew->szFilter, pStart, ulLength);
                    pNew->szFilter[ulLength] = '\0';
                    nlsUpper(pNew->szFilter);

                    PMPF_TURBOFOLDERS(("    found filter %s", pNew->szFilter));

                    pNew->pClassObject = pClassObject;

                    if (!fLocked)
                        fLocked = LockInstances();

                    if (fLocked)
                    {
                        if (lstAppendItem(&G_llInstanceFilters,
                                          pNew))
                            // got something:
                            ulrc++;

                        if (pEnd)       // points to comma
                        {
                            pStart = pEnd + 1;
                            fQuit = FALSE;
                        }
                    }
                }
            } while (!fQuit);

        } // end if (    (pszFilters = _wpclsQueryInstanceFilter(pClassObject))
    }
    CATCH(excpt1)
    {
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "wpclsQueryInstanceType or *Filter failed for %s",
               _somGetName(pClassObject));
    } END_CATCH();

    if (fLocked)
        UnlockInstances();

    return ulrc;
}

/*
 *@@ ftypFindClassFromInstanceType:
 *      returns the name of the class which claims ownership
 *      of the specified file type or NULL if there's
 *      none.
 *
 *      This searches without respect to case.
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

PCSZ ftypFindClassFromInstanceType(PCSZ pcszType)     // in: type string (case ignored)
{
    PCSZ pcszClassName = NULL;
    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        ULONG ulTypeLength;
        PSZ pszUpperType;
        if (    (pcszType)
             && (ulTypeLength = strlen(pcszType))
             && (pszUpperType = _alloca(ulTypeLength + 1))
             && (fLocked = LockInstances())
           )
        {
            PINSTANCETYPE p;

            // upper-case this, or the tree won't work
            nlsUpper(pszUpperType);

            if (p = (PINSTANCETYPE)treeFind(G_InstanceTypesTreeRoot,
                                            (ULONG)pszUpperType,
                                            treeCompareStrings))
                pcszClassName = _somGetName(p->pClassObject);
        }
    }
    CATCH(excpt1)
    {
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "crash for %s",
               pcszType);
        pcszClassName = NULL;
    } END_CATCH();

    if (fLocked)
        UnlockInstances();

    return pcszClassName;
}

/*
 *@@ ftypFindClassFromInstanceFilter:
 *      returns the name of the first class whose instance
 *      filter matches pcszObjectTitle or NULL if there's none.
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: optimized
 */

PCSZ ftypFindClassFromInstanceFilter(PCSZ pcszObjectTitle,
                                     ULONG ulTitleLen)      // in: length of title string (req.)
{
    PCSZ pcszClassName = NULL;
    BOOL fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (    (pcszObjectTitle)
             // && (*pcszObjectTitle)
             && (ulTitleLen)
             && (fLocked = LockInstances())
           )
        {
            PLISTNODE pNode = lstQueryFirstNode(&G_llInstanceFilters);
            PSZ pszUpperTitle = _alloca(ulTitleLen + 1);
            memcpy(pszUpperTitle, pcszObjectTitle, ulTitleLen + 1);
            nlsUpper(pszUpperTitle);

            while (pNode)
            {
                PINSTANCEFILTER p = (PINSTANCEFILTER)pNode->pItemData;
                if (doshMatchCaseNoPath(p->szFilter, pszUpperTitle))
                        // V0.9.20 (2002-07-25) [umoeller]
                {
                    pcszClassName = _somGetName(p->pClassObject);
                    // and stop, we're done
                    break;
                }
                pNode = pNode->pNext;
            }
        }
    }
    CATCH(excpt1)
    {
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "crash for %s",
               pcszObjectTitle);
        pcszClassName = NULL;
    } END_CATCH();

    if (fLocked)
        UnlockInstances();

    return pcszClassName;
}

/* ******************************************************************
 *
 *   Associations cleanup
 *
 ********************************************************************/

/*
 *@@ RemoveAssocReferences:
 *      called twice from ftypAssocObjectDeleted,
 *      with the PMWP_ASSOC_TYPES and PMWP_ASSOC_FILTERS
 *      strings, respectively.
 *
 *@@added V0.9.12 (2001-05-22) [umoeller]
 */

STATIC ULONG RemoveAssocReferences(PCSZ pcszHandle,     // in: decimal object handle
                                   PCSZ pcszIniApp)     // in: OS2.INI app to search
{
    APIRET arc;
    ULONG ulrc = 0;
    PSZ pszKeys = NULL;

    // loop 1: go thru all types/filters
    if (!(arc = prfhQueryKeysForApp(HINI_USER,
                                    pcszIniApp,
                                    &pszKeys)))
    {
        PCSZ pKey = pszKeys;
        while (*pKey != 0)
        {
            // loop 2: go thru all assocs for this type/filter
            ULONG cbAssocData;
            PSZ pszAssocData;
            if (pszAssocData = prfhQueryProfileData(HINI_USER,
                                                    pcszIniApp, // "PMWP_ASSOC_TYPE" or "PMWP_ASSOC_FILTER"
                                                    pKey,       // current type or filter
                                                    &cbAssocData))
            {
                PSZ     pAssoc = pszAssocData;
                ULONG   ulOfsAssoc = 0;
                LONG    cbCopy = cbAssocData;
                while (*pAssoc)
                {
                    // pAssoc now has the decimal handle of
                    // the associated object
                    ULONG cbAssocThis = strlen(pAssoc) + 1; // include null byte
                    cbCopy -= cbAssocThis;

                    // check if this assoc is to be removed
                    if (!strcmp(pAssoc, pcszHandle))
                    {
                        PMPF_ASSOCS(("removing handle %s from %s",
                                        pcszHandle,
                                        pKey));

                        // yes: well then...
                        // is this the last entry?
                        if (cbCopy > 0)
                        {
                            // no: move other entries up front
                            memmove(pAssoc,
                                    pAssoc + cbAssocThis,
                                    // remaining bytes:
                                    cbCopy);
                        }
                        // else: just truncate the chunk

                        cbAssocData -= cbAssocThis;

                        // now rewrite the assocs list...
                        // note, we do not remove the key,
                        // this is the types list of the WPS.
                        // If no assocs are left, we write a
                        // single null byte.
                        PrfWriteProfileData(HINI_USER,
                                            (PSZ)pcszIniApp,
                                            (PSZ)pKey,
                                            (cbAssocData)
                                                ? pszAssocData
                                                : "\0",
                                            (cbAssocData)
                                                ? cbAssocData
                                                : 1);           // null byte only
                        ulrc++;
                        break;
                    }

                    // go for next object handle (after the 0 byte)
                    pAssoc += cbAssocThis;
                    ulOfsAssoc += cbAssocThis;
                    if (pAssoc >= pszAssocData + cbAssocData)
                        break; // while (*pAssoc)
                } // end while (*pAssoc)

                free(pszAssocData);
            }

            // go for next key
            pKey += strlen(pKey) + 1;
        }

        free(pszKeys);
    }

    return ulrc;
}

/*
 *@@ ftypAssocObjectDeleted:
 *      runs through all association entries and
 *      removes somSelf from all associations, if
 *      present.
 *
 *      Gets called from XWPProgram::wpDestroyObject,
 *      i.e. when a WPProgram is physically destroyed.
 *
 *      Returns the no. of associations removed.
 *
 *@@added V0.9.12 (2001-05-22) [umoeller]
 *@@changed V0.9.20 (2002-07-31) [umoeller]: changed prototype to have WPObject* instead of HOBJECT
 */

ULONG ftypAssocObjectDeleted(WPObject *somSelf)
{
    ULONG ulrc = 0;

    TRY_LOUD(excpt1)
    {
        HOBJECT hobj;

        if (hobj = _xwpQueryHandle(somSelf,
                                   FALSE))      // do not create handle if there's none
                                                // V0.9.20 (2002-08-04) [umoeller]
        {
            CHAR szHandle[20];

            // run through OS2.INI assocs...
            // NOTE: we run through both the WPS types and
            // WPS filters sections here, even though XWP
            // extended assocs don't use the WPS filters.
            // But the object got deleted, so we shouldn't
            // leave the old entries in there.
            sprintf(szHandle, "%d", hobj);

            PMPF_ASSOCS(("running with %s", szHandle));

            ulrc += RemoveAssocReferences(szHandle,
                                          WPINIAPP_ASSOCTYPE); // "PMWP_ASSOC_TYPE"
            ulrc += RemoveAssocReferences(szHandle,
                                          WPINIAPP_ASSOCFILTER); // "PMWP_ASSOC_FILTER"
        }
    }
    CATCH(excpt1) {} END_CATCH();

    return ulrc;
}

#ifndef __NOTURBOFOLDERS__

/* ******************************************************************
 *
 *   Extended associations helper funcs
 *
 ********************************************************************/

/*
 *@@ ftypAppendSingleTypeUnique:
 *      appends the given type to the given list, if
 *      it's not on the list yet. Returns TRUE if the
 *      item either existed or was appended.
 *
 *      This assumes that pszNewType is free()'able. If
 *      pszNewType is already on the list, the string
 *      is freed!
 *
 *@@changed V0.9.6 (2000-11-12) [umoeller]: fixed memory leak
 *@@changed V0.9.20 (2002-07-25) [umoeller]: changed prototype, no longer expecting freeable string
 */

BOOL ftypAppendSingleTypeUnique(PLINKLIST pll,      // in: list to append to; list gets created on first call
                                PCSZ pcszNewType,   // in: new type to append
                                ULONG ulNewTypeLen) // in: length of pcszNewType; if 0, we run strlen() here
{
    BOOL    brc = FALSE;
    PLISTNODE pNode = lstQueryFirstNode(pll);
    PSZ     pszTypeCopy = NULL;

    if (ulNewTypeLen)
    {
        // if the length is specified, create a copy if
        // the string is not null-terminated
        // V0.9.20 (2002-07-25) [umoeller]
        if (pcszNewType[ulNewTypeLen])
        {
            pszTypeCopy = malloc(ulNewTypeLen + 1);
            memcpy(pszTypeCopy,
                   pcszNewType,
                   ulNewTypeLen);
            pszTypeCopy[ulNewTypeLen] = '\0';
            // use that for the comparisons
            pcszNewType = pszTypeCopy;
        }
        // else: string is null-terminated, so we can use it
    }
    else
        ulNewTypeLen = strlen(pcszNewType);

    while (pNode)
    {
        PSZ psz;
        if (    (psz = (PSZ)pNode->pItemData)
             && (!strcmp(psz, pcszNewType))
           )
        {
            // matches: it's already on the list,
            // so stop
            brc = TRUE;

            // and free the string (the caller has always created a copy)
            // free(pszNewType);        nope V0.9.20 (2002-07-25) [umoeller]

            break;
        }

        pNode = pNode->pNext;
    }

    if (!brc)
    {
        // not found:
        // store a copy on the list
        if (pszTypeCopy)
            // we made a null-terminated copy above:
            brc = (lstAppendItem(pll, pszTypeCopy) != NULL);
        else
        {
            // copy now
            if (pszTypeCopy = malloc(ulNewTypeLen + 1))
            {
                memcpy(pszTypeCopy,
                       pcszNewType,
                       ulNewTypeLen + 1);
                brc = (lstAppendItem(pll, pszTypeCopy) != NULL);
            }
        }
    }
    else
        // already on list:
        // free copy, if we made one V0.9.20 (2002-07-25) [umoeller]
        if (pszTypeCopy)
            free(pszTypeCopy);

    return brc;
}

/*
 *@@ ftypAppendTypesFromString:
 *      this splits a standard WPS file types
 *      string (where several file types are
 *      separated by a separator char) into
 *      a linked list of newly allocated PSZ's.
 *
 *      For some reason, WPDataFile uses \n as
 *      the types separator (wpQueryType), while
 *      WPProgram(File) uses a comma (',',
 *      wpQueryAssociationType).
 *
 *      The list is not cleared, but simply appended to.
 *      The type is only added if it doesn't exist in
 *      the list yet.
 *
 *      The list should be in auto-free mode
 *      so that the strings are automatically
 *      freed upon lstClear. See lstInit for details.
 *
 *@@added V0.9.0 (99-11-27) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: optimized
 */

ULONG ftypAppendTypesFromString(PCSZ pcszTypes, // in: types string (e.g. "C Code\nPlain text")
                                CHAR cSeparator, // in: separator (\n for data files, ',' for programs)
                                PLINKLIST pllTypes) // in/out: list of newly allocated PSZ's
                                                   // with file types (e.g. "C Code", "Plain text")
{
    ULONG   ulrc = 0;
    // if we have several file types (which are then separated
    // by line feeds (\n), get the one according to ulView.
    // For example, if pszType has "C Code\nPlain text" and
    // ulView is 0x1001, get "Plain text".
    const char  *pTypeThis = pcszTypes,
                *pLF = 0;

    // loop thru types list
    while (pTypeThis)
    {
        // get next line feed
        if (pLF = strchr(pTypeThis, cSeparator))
        {
            // line feed found:
            // extract type and store in list
            ftypAppendSingleTypeUnique(pllTypes,
                                       pTypeThis,
                                       // length: V0.9.20 (2002-07-25) [umoeller]
                                       pLF - pTypeThis);
            ulrc++;
            // next type (after LF)
            pTypeThis = pLF + 1;
        }
        else
        {
            // no next line feed found:
            // store last item
            if (*pTypeThis)
            {
                ftypAppendSingleTypeUnique(pllTypes,
                                           pTypeThis,
                                           // length: V0.9.20 (2002-07-25) [umoeller]
                                           0);
                ulrc++;
            }
            break;
        }
    }

    return ulrc;
}

/*
 *@@ AppendTypesForFile:
 *      this calls the given callback for all extended
 *      file types which have a file filter assigned to
 *      them which matches the given object title.
 *
 *      For example, if you pass "filetype.c" to this
 *      function and "C Code" has the "*.C" filter
 *      assigned, this would call the callback with
 *      the "C Code" type string.
 *
 *      In order to query all associations for a given
 *      object, pass the object title to this function
 *      first (to get the associated types). Then, for
 *      all types on the list which was filled for this
 *      function, call ListAssocsForType with the same
 *      objects list for each call.
 *
 *      This returns the no. of items which were added
 *      (0 if none).
 *
 *@@added V0.9.20 (2002-07-25) [umoeller]
 */

ULONG ftypForEachAutoType(PCSZ pcszObjectTitle,
                          PFNFOREACHAUTOMATICTYPE pfnftypForEachAutoType,
                          PVOID pvUser)
{
    ULONG   ulrc = 0;
    ULONG   ulObjectTitleLen;
    PSZ     pszUpperTitle;

    // loop thru all extended file types which have
    // filters assigned to them to check whether the
    // filter matches the object title

    if (    (pcszObjectTitle)
         && (pszUpperTitle = strhdup(pcszObjectTitle, &ulObjectTitleLen))
       )
    {
        PSZ     pszTypesWithFiltersList;

        PMPF_ASSOCS(("[%s] entering", pcszObjectTitle));

        nlsUpper(pszUpperTitle);

        if (!prfhQueryKeysForApp(HINI_USER,
                                 INIAPP_XWPFILEFILTERS, // "XWorkplace:FileFilters"
                                 &pszTypesWithFiltersList))
        {
            PSZ     pTypeThis = pszTypesWithFiltersList;
            BOOL    fQuit = FALSE;

            while ((*pTypeThis) && (!fQuit))
            {
                // pTypeWithFilterThis has the current type now
                // (e.g. "C Code");
                // get filters for that (e.g. "*.c");
                // this is another list of null-terminated strings
                ULONG   ulTypeLength;

                // no longer using a malloc'd buffer here
                CHAR    szFiltersForTypeList[300];      // should be enough?
                ULONG   cbFiltersForTypeList = sizeof(szFiltersForTypeList);

                PMPF_ASSOCS(("   checking filters of type %s", pTypeThis));

                if (    (ulTypeLength = strlen(pTypeThis))
                     && (PrfQueryProfileData(HINI_USER,
                                             (PSZ)INIAPP_XWPFILEFILTERS, // "XWorkplace:FileFilters"
                                             pTypeThis,
                                             szFiltersForTypeList,
                                             &cbFiltersForTypeList))
                   )
                {
                    // second loop: thru all filters for this file type
                    PSZ pFilterThis = szFiltersForTypeList;

                    // just in case the buffer is too small, null-terminate it
                    szFiltersForTypeList[sizeof(szFiltersForTypeList) - 1] = '\0';

                    while (*pFilterThis)
                    {
                        // check if this matches the data file name
                        ULONG ulFilterThisLen = nlsUpper(pFilterThis);

                        PMPF_ASSOCS(("      checking filters %s", pFilterThis));

                        if (doshMatchCaseNoPath(pFilterThis, pszUpperTitle))
                        {
                            PMPF_ASSOCS(("  type %s matches filter %s on %s",
                                        pTypeThis,
                                        pFilterThis,
                                        pszUpperTitle));

                            // matches:
                            // now we have:
                            // --  pFilterThis e.g. "*.c"
                            // --  pTypeWithFilterThis e.g. "C Code"

                            ulrc++;     // found something

                            // call user callback
                            if (!pfnftypForEachAutoType(pTypeThis,
                                                        ulTypeLength,
                                                        pvUser))
                                fQuit = TRUE;

                            break;      // this file type is done, so go for next type
                        }

                        pFilterThis += ulFilterThisLen + 1;   // next type/filter

                        if (pFilterThis >= szFiltersForTypeList + cbFiltersForTypeList)
                            break; // while (*pFilter))

                    } // end while (*pFilterThis)
                }

                pTypeThis += ulTypeLength + 1;   // next type/filter

            } // end while (*pTypeWithFilterThis != 0)

            free(pszTypesWithFiltersList);
                        // we created copies of each string here
        }

        free(pszUpperTitle);
    }

    return ulrc;
}

/* ******************************************************************
 *
 *   XFldDataFile extended associations
 *
 ********************************************************************/

/*
 *@@ ftypQueryAssociatedProgram:
 *      implementation for XFldDataFile::wpQueryAssociatedProgram.
 *
 *      This gets called _instead_ of the WPDataFile version if
 *      extended associations have been enabled.
 *
 *      This also gets called from XFldDataFile::wpOpen to find
 *      out which of the new associations needs to be opened.
 *
 *      It is the responsibility of this method to return the
 *      associated WPProgram or WPProgramFile object for the
 *      given data file according to ulView.
 *
 *      Normally, ulView is the menu item ID in the "Open"
 *      submenu, which is >= 0x1000. HOWEVER, for some reason,
 *      the WPS also uses OPEN_RUNNING as the default view.
 *      We can also get OPEN_DEFAULT.
 *
 *      The object returned has been locked by this function.
 *      Use _wpUnlockObject to unlock it again.
 *
 *@@added V0.9.0 (99-11-27) [umoeller]
 *@@changed V0.9.6 (2000-10-16) [umoeller]: lists are temporary only now
 *@@changed V0.9.6 (2000-11-12) [umoeller]: added pulView output
 *@@changed V0.9.16 (2002-01-26) [umoeller]: performance tweaking
 *@@changed V0.9.19 (2002-05-23) [umoeller]: fixed wrong default icons for WPUrl
 *@@changed V0.9.20 (2002-07-25) [umoeller]: got rid of linked list
 *@@changed V1.0.5 (2006-05-14) [pr]: @@fixes 14
 */

WPObject* ftypQueryAssociatedProgram(WPDataFile *somSelf,       // in: data file
                                     PULONG pulView,            // in: default view (normally 0x1000,
                                                                // can be > 0x1000 if the default view
                                                                // has been manually changed on the
                                                                // "Menu" page);
                                                                // out: "real" default view if this
                                                                // was OPEN_RUNNING or something
                                     BOOL fUsePlainTextAsDefault)
                                            // in: use "plain text" as standard if no other type was found?
{
    WPObject    *pObjReturn = 0;
    WPObject    *apObjects[MAX_ASSOCS_PER_OBJECT];
    ULONG       cAssocObjects = 0;
    ULONG       ulIndex = 0;

    // V1.0.5 (2006-05-14) [pr]: @@fixes 14
    if (*pulView == OPEN_DEFAULT)
        *pulView = _wpQueryDefaultView(somSelf);
                // returns 0x1000, unless the user has changed
                // the default association on the "Menu" page.
                // This can return OPEN_RUNNING, so moved this up now.

    if (*pulView == OPEN_RUNNING)
        *pulView = 0x1000;

    // calc index to search...
    if (    (*pulView >= 0x1000)
         // delimit this!! Return a null icon if this is way too large.
         // V0.9.19 (2002-05-23) [umoeller]
         && (*pulView <= 0x1000 + MAX_ASSOCS_PER_OBJECT)
       )
    {
        ulIndex = *pulView - 0x1000;
    // else
        // ulIndex = 0;
        // wrooong: WPUrl objects have OPEN_CONTENTS (1)
        // as their default view and the WPS does not give them an associated
        // file icon... instead, they always get the class default icon.
        // This was broken with XWP, which always associated the first
        // program even if the default view was < 0x1000. In that case,
        // we must rather return a null icon so that the class icon gets
        // used instead. So ONLY run thru the list below if we actually
        // have a default view >= 0x1000.
        // V0.9.19 (2002-05-23) [umoeller]

        if (cAssocObjects = _xwpQueryAssociations(somSelf,
                                                  apObjects,
                                                  ulIndex + 1,
                                                  fUsePlainTextAsDefault))
        {
            // any items found:
            ULONG       ul;

            if (ulIndex >= cAssocObjects)
                ulIndex = 0;

            pObjReturn = apObjects[ulIndex];

            // unlock all objs except the one that we return
            for (ul = 0;
                 ul < cAssocObjects;
                 ++ul)
            {
                if (apObjects[ul] != pObjReturn)
                    _wpUnlockObject(apObjects[ul]);
            }
        }
    }

    return pObjReturn;
}

/*
 *@@ ftypModifyDataFileOpenSubmenu:
 *      this adds associations to an "Open" submenu, after
 *      clearing old the existing items in that menu.
 *
 *      This gets called only if extended associations are
 *      enabled.
 *
 *@@added V0.9.0 (99-11-27) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: got rid of linked list
 *@@changed V1.0.0 (2002-08-31) [umoeller]: fixed deletion of WPMENUID_PROPERTIES
 *@@changed V1.0.1 (2002-12-14) [umoeller]: removed fDeleteExisting param
 */

BOOL ftypModifyDataFileOpenSubmenu(WPDataFile *somSelf, // in: data file in question
                                   HWND hwndOpenSubmenu)
                                            // in: "Open" submenu window
{
    BOOL            brc = FALSE;

    if (hwndOpenSubmenu)
    {
        ULONG       ulItemID = 0;

        // 1) remove existing (default WPS) association
        // items from "Open" submenu

        // find first item
        SHORT sPos = 0;

        do
        {
            ulItemID = (ULONG)WinSendMsg(hwndOpenSubmenu,
                                         MM_ITEMIDFROMPOSITION,
                                         (MPARAM)sPos,      // V1.0.0 (2002-08-31) [umoeller]
                                         0);      // reserved
            if (    (ulItemID)
                 && (ulItemID != MIT_ERROR)
               )
            {
                // only remove items >= 0x1000 because at this
                // point, the "properties" menu item is still
                // in the menu (dammit, IBM)
                // V1.0.0 (2002-08-31) [umoeller]
                if (ulItemID < 0x1000)
                {
                    ++sPos;
                }
                else
                {
                    #ifdef __DEBUG__
                        PSZ pszItemText = winhQueryMenuItemText(hwndOpenSubmenu, ulItemID);
                        PMPF_ASSOCS((" removing 0x%lX (%s)",
                                    ulItemID,
                                    pszItemText));
                        free(pszItemText);
                    #endif

                    winhDeleteMenuItem(hwndOpenSubmenu, ulItemID);
                }

                brc = TRUE;
            }
            else
                break;

        } while (TRUE);

        // 2) add the new extended associations

        if (brc)
        {
            WPObject    *apObjects[MAX_ASSOCS_PER_OBJECT];
            ULONG       cAssocObjects = 0;
            if (cAssocObjects = _xwpQueryAssociations(somSelf,
                                                      apObjects,
                                                      // get all:
                                                      -1,
                                                      // use "plain text" as default:
                                                      TRUE))
            {
                // now add all the associations; this list has
                // instances of WPProgram and WPProgramFile

                // get data file default associations; this should
                // return something >= 0x1000 also
                ULONG   ulDefaultView = _wpQueryDefaultView(somSelf),
                        ul;

                // initial menu item ID; all associations must have
                // IDs >= 0x1000
                ulItemID = 0x1000;

                for (ul = 0;
                     ul < cAssocObjects;
                     ++ul)
                {
                    WPObject *pAssocThis;
                    if (pAssocThis = apObjects[ul])
                    {
                        PSZ pszAssocTitle;
                        if (pszAssocTitle = _wpQueryTitle(pAssocThis))
                        {
                            winhInsertMenuItem(hwndOpenSubmenu,  // still has "Open" submenu
                                               MIT_END,
                                               ulItemID,
                                               pszAssocTitle,
                                               MIS_TEXT,
                                               // if this is the default view,
                                               // mark as default
                                               (ulItemID == ulDefaultView)
                                                    ? MIA_CHECKED
                                                    : 0);
                        }

                        _wpUnlockObject(pAssocThis);
                    }

                    ulItemID++;     // raise item ID even if object was invalid;
                                    // this must be the same in wpMenuItemSelected
                }
            }
        }
    }

    return brc;
}

/*
 *@@ ftypRenameFileType:
 *      renames a file type and updates all associated
 *      INI entries.
 *
 *      This returns:
 *
 *      -- ERROR_FILE_NOT_FOUND: pcszOld is not a valid file type.
 *
 *      -- ERROR_FILE_EXISTS: pcszNew is already occupied.
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 *@@changed V0.9.12 (2001-05-31) [umoeller]: mutex was missing, fixed
 */

APIRET ftypRenameFileType(PCSZ pcszOld,      // in: existing file type
                          PCSZ pcszNew)      // in: new name for pcszOld
{
    APIRET  arc = FALSE;
    ULONG   ulLength;
    // check WPS file types... this better exist, or we'll stop
    // right away
    if (!PrfQueryProfileSize(HINI_USER,
                             (PSZ)WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE"
                             (PSZ)pcszOld,
                             &ulLength))
        arc = ERROR_FILE_NOT_FOUND;
    else
    {
        // pcszNew must not be used yet.
        if (PrfQueryProfileSize(HINI_USER,
                                (PSZ)WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE"
                                (PSZ)pcszNew,
                                &ulLength))
            arc = ERROR_FILE_EXISTS;
        else
        {
            // OK... first of all, we must write a new entry
            // into "PMWP_ASSOC_TYPE" with the old handles
            if (!(arc = prfhRenameKey(HINI_USER,
                                      // old app:
                                      WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE"
                                      // old key:
                                      pcszOld,
                                      // new app:
                                      NULL,     // same app
                                      // new key:
                                      pcszNew)))
            {
                PSZ pszXWPParentTypes;

                // now update the the XWP entries, if any exist

                // 1) associations linked to this file type:
                prfhRenameKey(HINI_USER,
                              INIAPP_XWPFILEFILTERS, // "XWorkplace:FileFilters"
                              pcszOld,
                              NULL,         // same app
                              pcszNew);
                // 2) parent types for this:
                prfhRenameKey(HINI_USER,
                              INIAPP_XWPFILETYPES, // "XWorkplace:FileTypes"
                              pcszOld,
                              NULL,         // same app
                              pcszNew);

                // 3) now... go thru ALL of "XWorkplace:FileTypes"
                // and check if any of the types in there has specified
                // pcszOld as its parent type. If so, update that entry.
                if (!(arc = prfhQueryKeysForApp(HINI_USER,
                                                INIAPP_XWPFILETYPES, // "XWorkplace:FileTypes"
                                                &pszXWPParentTypes)))
                {
                    PSZ pParentThis = pszXWPParentTypes;
                    while (*pParentThis)
                    {
                        PSZ pszThisParent = prfhQueryProfileData(HINI_USER,
                                                                 INIAPP_XWPFILETYPES, // "XWorkplace:FileTypes"
                                                                 pParentThis,
                                                                 NULL);
                        if (pszThisParent)
                        {
                            if (!strcmp(pszThisParent, pcszOld))
                                // replace this entry
                                PrfWriteProfileString(HINI_USER,
                                                      (PSZ)INIAPP_XWPFILETYPES, // "XWorkplace:FileTypes"
                                                      pParentThis,
                                                      (PSZ)pcszNew);

                            free(pszThisParent);
                        }

                        pParentThis += strlen(pParentThis) + 1;
                    }

                    free(pszXWPParentTypes);
                }
            }
        }
    }

    return arc;
}

/* ******************************************************************
 *
 *   Import facility
 *
 ********************************************************************/

/*
 *@@ ImportFilters:
 *      imports the filters for the given TYPE
 *      element and merges them with the existing
 *      filters.
 *
 *@@added V0.9.12 (2001-05-21) [umoeller]
 *@@changed V1.0.2 (2003-02-07) [umoeller]: added codepage support
 */

STATIC APIRET ImportFilters(PDOMNODE pTypeElementThis,
                            PCSZ pcszTypeNameThis)          // in: current type (already CP-decoded)
{
    APIRET arc = NO_ERROR;

    PLINKLIST pllElementFilters;
    if (pllElementFilters = xmlGetElementsByTagName(pTypeElementThis,
                                                    "FILTER"))
    {
        // alright, this is tricky...
        // we don't just want to overwrite the existing
        // filters, we need to merge them with the new
        // filters, if any exist.

        // CHAR    szFilters[2000] = "";   // should suffice        bah! V1.0.2 (2003-02-07) [umoeller]
        ULONG   lenFiltersTotal = 0;
        PSZ     pszFilters = NULL;
        ULONG   cbFilters = 0;

        // 1) get the XWorkplace-defined filters for this file type
        ULONG cbFiltersData = 0;
        PSZ pszFiltersData = prfhQueryProfileData(HINI_USER,
                                                  INIAPP_XWPFILEFILTERS, // "XWorkplace:FileFilters"
                                                  (PSZ)pcszTypeNameThis,
                                                  &cbFiltersData);

        LINKLIST llAllFilters;
        PLISTNODE pNode;
        lstInit(&llAllFilters, TRUE);      // will hold all PSZ's, auto-free    V1.0.2 (2003-02-07) [umoeller]

        PMPF_ASSOCS(("     got %d new filters for %s, merging",
                    lstCountItems(pllElementFilters),
                    pcszTypeNameThis));

        if (pszFiltersData)
        {
            // pszFiltersData now has a string array of
            // defined filters, each null-terminated
            PSZ     pFilter = pszFiltersData;

            PMPF_ASSOCS(("       got %d bytes of existing filters", cbFiltersData));

            // now parse the filters string
            while ((*pFilter) && (!arc))
            {
                ULONG   lenFilter = strlen(pFilter);

                PMPF_ASSOCS(("           appending existing %s", pFilter));

                lstAppendItem(&llAllFilters,
                              memcpy(malloc(lenFilter + 1), pFilter, lenFilter + 1));
                lenFiltersTotal += lenFilter + 1;

                // go for next object filter (after the 0 byte)
                pFilter += lenFilter + 1;
                if (pFilter >= pszFiltersData + cbFiltersData)
                    break; // while (*pFilter))
            } // end while (*pFilter)
        }
        else
            PMPF_ASSOCS(("       no existing filters"));

        // 2) add the new filters from the elements, if they are not on the list yet
        for (pNode = lstQueryFirstNode(pllElementFilters);
             pNode;
             pNode = pNode->pNext)
        {
            PDOMNODE pFilterNode = (PDOMNODE)pNode->pItemData;

            // filter is in attribute VALUE="*.psd"
            const XSTRING *pstrFilterEnc = xmlGetAttribute(pFilterNode,
                                                           "VALUE");

            PMPF_ASSOCS(("           adding new %s",
                        (pstrFilterEnc) ? pstrFilterEnc->psz : "NULL"));

            if (pstrFilterEnc)
            {
                // check if filter is on list already
                PLISTNODE pNode2;

                BOOL fExists = FALSE;

                // decode non-ASCII stuff V1.0.2 (2003-02-07) [umoeller]
                XSTRING strFilter;
                xstrInit(&strFilter, 0);
                xstrcpys(&strFilter, pstrFilterEnc);
                xstrDecode2(&strFilter, '%');

                for (pNode2 = lstQueryFirstNode(&llAllFilters);
                     pNode2;
                     pNode2 = pNode2->pNext)
                {
                    if (!strcmp((PSZ)pNode2->pItemData,
                                strFilter.psz))
                    {
                        fExists = TRUE;
                        break;
                    }
                }

                if (!fExists)
                {
                    lstAppendItem(&llAllFilters,
                                  strFilter.psz);       // will be freed with auto-free list
                    lenFiltersTotal += strFilter.ulLength + 1;
                }
                else
                    xstrClear(&strFilter);
            }
        }

        // 3) compose new filters string from the list with
        //    the old and new filters, each filter null-terminated
        if (lenFiltersTotal)
        {
            if (!(pszFilters = malloc(lenFiltersTotal + 1)))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                for (pNode = lstQueryFirstNode(&llAllFilters);
                     pNode;
                     pNode = pNode->pNext)
                {
                    PSZ pszFilterThis = (PSZ)pNode->pItemData;
                    PMPF_ASSOCS(("       appending filter %s",
                                     pszFilterThis));

                    cbFilters += sprintf(pszFilters + cbFilters,
                                         "%s",
                                         pszFilterThis    // filter string
                                        ) + 1;
                }
            }
        }

        if (pszFiltersData)
            free(pszFiltersData);

        if (!arc)
            // 4) write out the merged filters list now
            PrfWriteProfileData(HINI_USER,
                                (PSZ)INIAPP_XWPFILEFILTERS, // "XWorkplace:FileFilters"
                                (PSZ)pcszTypeNameThis,
                                pszFilters,                 // can be NULL if no filters found
                                cbFilters);

        lstClear(&llAllFilters);
        lstFree(&pllElementFilters);
    }
    // else no filters: no problem,
    // we leave the existing intact, if any

    return arc;
}

/*
 *@@ ImportTypes:
 *      adds the child nodes of pParentElement
 *      to the types table.
 *
 *      This recurses, if necessary.
 *
 *      Initially called with the XWPFILETYPES
 *      (root) element.
 *
 *@@added V0.9.12 (2001-05-21) [umoeller]
 *@@changed V1.0.2 (2003-02-07) [umoeller]: added codepage support
 */

STATIC APIRET ImportTypes(PDOMNODE pParentElement,
                          PCSZ pcszParentType)          // in: parent type name or NULL
{
    APIRET      arc = NO_ERROR;
    PLINKLIST   pllTypes;

    if (pllTypes = xmlGetElementsByTagName(pParentElement,
                                           "TYPE"))
    {
        PLISTNODE pTypeNode;
        for (pTypeNode = lstQueryFirstNode(pllTypes);
             (pTypeNode) && (!arc);
             pTypeNode = pTypeNode->pNext)
        {
            PDOMNODE pTypeElementThis = (PDOMNODE)pTypeNode->pItemData;

            // get the type name
            const XSTRING *pstrTypeNameEnc;
            if (!(pstrTypeNameEnc = xmlGetAttribute(pTypeElementThis,
                                                    "NAME")))
                arc = ERROR_DOM_VALIDITY;
            else
            {
                // alright, we got a type...
                // check if it's in OS2.INI already
                ULONG cb = 0;

                // decode the type name
                XSTRING strTypeName;
                xstrInit(&strTypeName, 0);
                xstrcpys(&strTypeName, pstrTypeNameEnc);
                xstrDecode2(&strTypeName, '%');

                PMPF_ASSOCS(("importing %s, parent is %s",
                             strTypeName.psz,
                             (pcszParentType) ? pcszParentType : "NULL"));

                if (    (!PrfQueryProfileSize(HINI_USER,
                                              (PSZ)WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE"
                                              strTypeName.psz,
                                              &cb))
                     || (cb == 0)
                   )
                {
                    // alright, add a new type, with a single null byte
                    // (no associations yet)
                    static const CHAR NullByte = '\0';

                    PMPF_ASSOCS(("   type %s doesn't exist, adding to PMWP_ASSOC_TYPE",
                                 strTypeName.psz));

                    PrfWriteProfileData(HINI_USER,
                                        (PSZ)WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE"
                                        strTypeName.psz,
                                        (PSZ)&NullByte,
                                        1);
                }
                else
                    PMPF_ASSOCS(("   type %s exists",  strTypeName.psz));

                // now update parent type
                // in any case, write the parent type
                // to the XWP types list (overwrite existing
                // parent type, if it exists);
                // -- if pcszParentType is NULL, the existing
                //    parent type is reset to root (delete the entry)
                // -- if pcszParentType != NULL, the existing
                //    parent type is replaced
                PrfWriteProfileString(HINI_USER,
                                      (PSZ)INIAPP_XWPFILETYPES, // "XWorkplace:FileTypes"
                                      // key name == type name
                                      strTypeName.psz,
                                      // data == parent type
                                      (PSZ)pcszParentType);

                // get the filters, if any
                ImportFilters(pTypeElementThis,
                              strTypeName.psz);

                // recurse for this file type, it may have subtypes
                arc = ImportTypes(pTypeElementThis,
                                  strTypeName.psz);

                xstrClear(&strTypeName);
            }
        } // end for (pTypeNode = lstQueryFirstNode(pllTypes);

        lstFree(&pllTypes);
    }

    return arc;
}

/*
 *@@ ftypImportTypes:
 *      loads new types and filters from the specified
 *      XML file, which should have been created with
 *      ftypExportTypes and have an extension of ".XTP".
 *
 *      Returns either a DOS or XML error code (see xml.h).
 *
 *@@added V0.9.12 (2001-05-21) [umoeller]
 */

APIRET ftypImportTypes(PCSZ pcszFilename,        // in: XML file name
                       PXSTRING pstrError)       // out: error description, if rc != NO_ERROR
{
    APIRET arc;

    PSZ pszContent = NULL;

    if (!(arc = doshLoadTextFile(pcszFilename,
                                 &pszContent,
                                 NULL)))
    {
        // now go parse
        // create the DOM
        PXMLDOM pDom = NULL;
        if (!(arc = xmlCreateDOM(DF_PARSEDTD | DF_FAIL_IF_NO_DTD | DF_DROP_WHITESPACE,
                                 G_aSysIds,
                                 ARRAYITEMCOUNT(G_aSysIds),
                                 NULL,
                                 NULL,
                                 NULL,
                                 &pDom)))
        {
            if (!(arc = xmlParse(pDom,
                                 pszContent,
                                 strlen(pszContent),
                                 TRUE)))    // last chunk (we only have one)
            {
                TRY_LOUD(excpt1)
                {
                    PDOMNODE pRootElement;
                    if (pRootElement = xmlGetRootElement(pDom))
                    {
                        // decode codepage attribute V1.0.2 (2003-02-07) [umoeller]
                        const XSTRING *pstrCPAttr;
                        ULONG   ulcpSys = nlsQueryCodepage(),
                                ulcpDoc;

                        if (!(pstrCPAttr = xmlGetAttribute(pRootElement,
                                                           G_pcszCODEPAGE)))
                            ulcpDoc = ulcpSys;      // old format: assume process codepage
                        else
                            ulcpDoc = atoi(pstrCPAttr->psz);

                        arc = ImportTypes(pRootElement,
                                          NULL);        // parent type == none
                                // this recurses into subtypes
                    }
                    else
                        arc = ERROR_DOM_NO_ELEMENT;
                }
                CATCH(excpt1)
                {
                    arc = ERROR_PROTECTION_VIOLATION;
                } END_CATCH();
            }

            switch (arc)
            {
                case NO_ERROR:
                break;

                case ERROR_DOM_PARSING:
                case ERROR_DOM_VALIDITY:
                {
                    CHAR szError2[100];
                    PCSZ pcszError;
                    if (!(pcszError = pDom->pcszErrorDescription))
                    {
                        sprintf(szError2, "Code %u", pDom->arcDOM);
                        pcszError = szError2;
                    }

                    if (arc == ERROR_DOM_PARSING)
                    {
                        xstrPrintf(pstrError,
                                   "Parsing error: %s (line %d, column %d)",
                                   pcszError,
                                   pDom->ulErrorLine,
                                   pDom->ulErrorColumn);

                        if (pDom->pxstrFailingNode)
                        {
                            xstrcat(pstrError, " (", 0);
                            xstrcats(pstrError, pDom->pxstrFailingNode);
                            xstrcat(pstrError, ")", 0);
                        }
                    }
                    else if (arc == ERROR_DOM_VALIDITY)
                    {
                        xstrPrintf(pstrError,
                                "Validation error: %s (line %d, column %d)",
                                pcszError,
                                pDom->ulErrorLine,
                                pDom->ulErrorColumn);

                        if (pDom->pxstrFailingNode)
                        {
                            xstrcat(pstrError, " (", 0);
                            xstrcats(pstrError, pDom->pxstrFailingNode);
                            xstrcat(pstrError, ")", 0);
                        }
                    }
                }
                break;

                default:
                    cmnDescribeError(pstrError,
                                     arc,
                                     NULL,
                                     TRUE);
            }

            xmlFreeDOM(pDom);
        }

        free(pszContent);
    }

    return arc;
}

/* ******************************************************************
 *
 *   Export facility
 *
 ********************************************************************/

/*
 *@@ ExportAddType:
 *      stores one type and all its filters
 *      as a TYPE element with FILTER subelements
 *      and the respective attributes.
 *
 *@@added V0.9.12 (2001-05-21) [umoeller]
 *@@changed V1.0.2 (2003-02-07) [umoeller]: added pstrBuf param
 *@@changed V1.0.2 (2003-02-07) [umoeller]: now encoding non-ASCII characters to fix XML encoding problems
 */

STATIC APIRET ExportAddType(PDOMNODE pParentNode,          // in: type's parent node (document root node if none)
                            PFILETYPELISTITEM pliAssoc,    // in: type description
                            PXSTRING pstrBuf,              // in/out: working buffer (must be initialized)
                            PDOMNODE *ppNewNode)           // out: new element
{
    PDOMNODE pNodeReturn;
    APIRET arc;
    if (!(arc = xmlCreateElementNode(pParentNode,
                                     // parent record; this might be pRootElement
                                     "TYPE",
                                     &pNodeReturn)))
    {
        PDOMNODE    pAttribute;
        pliAssoc->precc = (PFILETYPERECORD)pNodeReturn;
        pliAssoc->fProcessed = TRUE;

        xstrcpy(pstrBuf, pliAssoc->pszFileType, 0);
        xstrEncodeASCII(pstrBuf);           // V1.0.2 (2003-02-07) [umoeller]

        // create NAME attribute
        if (!(arc = xmlCreateAttributeNode(pNodeReturn,
                                           "NAME",
                                           pstrBuf->psz,
                                           pstrBuf->ulLength,
                                           &pAttribute)))
        {
            // create child ELEMENTs for each
            // XWorkplace-defined filter for this file type

            ULONG cbFiltersData;
            PSZ pszFiltersData;
            if (pszFiltersData = prfhQueryProfileData(HINI_USER,
                                                      INIAPP_XWPFILEFILTERS, // "XWorkplace:FileFilters"
                                                      pliAssoc->pszFileType,
                                                      &cbFiltersData))
            {
                // pszFiltersData now has a string array of
                // defined filters, each null-terminated
                PSZ     pFilter = pszFiltersData;

                // now parse the filters string
                while ((*pFilter) && (!arc))
                {
                    // add the filter to the "Filters" container
                    PDOMNODE pFilterNode;
                    ULONG lenFilter = strlen(pFilter);
                    xstrcpy(pstrBuf, pFilter, lenFilter);
                    xstrEncodeASCII(pstrBuf);               // V1.0.2 (2003-02-07) [umoeller]

                    if (!(arc = xmlCreateElementNode(pNodeReturn,
                                                     // parent record; this might be pRootElement
                                                     "FILTER",
                                                     &pFilterNode)))
                        arc = xmlCreateAttributeNode(pFilterNode,
                                                     "VALUE",
                                                     pstrBuf->psz,
                                                     pstrBuf->ulLength,
                                                     &pAttribute);

                    // go for next object filter (after the 0 byte)
                    pFilter += lenFilter + 1;
                    if (pFilter >= pszFiltersData + cbFiltersData)
                        break; // while (*pFilter))
                } // end while (*pFilter)

                free(pszFiltersData);
            }
        }

        *ppNewNode = pNodeReturn;
    }
    else
        PMPF_ASSOCS(("xmlCreateElementNode returned %d for %s",
                    arc,
                    pliAssoc->pszFileType));

    return arc;
}

/*
 *@@ ExportAddFileTypeAndAllParents:
 *      adds the specified file type to the DOM
 *      tree; also adds all the parent file types
 *      if they haven't been added yet.
 *
 *      This code is very similar to that in
 *      AddFileTypeAndAllParents (for the cnr page)
 *      but works on the DOM tree instead.
 *
 *@@added V0.9.12 (2001-05-21) [umoeller]
 *@@changed V1.0.5 (2006-05-21) [pr]
 */

STATIC APIRET ExportAddFileTypeAndAllParents(PDOMNODE pRootElement,
                                             PLINKLIST pllFileTypes,    // in: list of all file types
                                             PSZ pszKey,
                                             PXSTRING pstrBuf,          // in/out: working buffer (must be initialized)
                                             PDOMNODE *ppNewElement)    // out: element node for this key
{
    APIRET              arc = NO_ERROR;
    PDOMNODE            pParentNode = pRootElement,
                        pNodeReturn = NULL;
    PLISTNODE           pAssocNode;

    // query the parent for pszKey
    PSZ pszParentForKey;

    if (pszParentForKey = prfhQueryProfileData(HINI_USER,
                                               INIAPP_XWPFILETYPES, // "XWorkplace:FileTypes"
                                               pszKey,
                                               NULL))
    {
        // key has a parent: recurse first! we need the
        // parent records before we insert the actual file
        // type as a child of this
        arc = ExportAddFileTypeAndAllParents(pRootElement,
                                             pllFileTypes,
                                             // recurse with parent
                                             pszParentForKey,
                                             pstrBuf,
                                             &pParentNode);
        free(pszParentForKey);
    }

    if (!arc)
    {
        // we arrive here after the all the parents
        // of pszKey have been added;
        // if we have no parent, pParentNode is NULL
        if (!pParentNode)
            pParentNode = pRootElement;	// V1.0.5 (2006-05-21) [pr]: @@fixes 702

        // now find the file type list item
        // which corresponds to pKey
        pAssocNode = lstQueryFirstNode(pllFileTypes);
        while ((pAssocNode) && (!arc))
        {
            PFILETYPELISTITEM pliAssoc = (PFILETYPELISTITEM)pAssocNode->pItemData;

            if (!strcmp(pliAssoc->pszFileType, pszKey))
            {
                if (!pliAssoc->fProcessed)
                {
                    if (!pliAssoc->fCircular)
                    {
                        arc = ExportAddType(pParentNode,
                                            pliAssoc,
                                            pstrBuf,
                                            &pNodeReturn);
                    }

                    pliAssoc->fCircular = TRUE;
                }
                else
                    // record core already created:
                    // return that one
                    pNodeReturn = (PDOMNODE)pliAssoc->precc;

                // in any case, stop
                break;
            }

            pAssocNode = pAssocNode->pNext;
        }

        // return the DOMNODE which we created;
        // if this is a recursive call, this will
        // be used as a parent by the parent call
        *ppNewElement = pNodeReturn;
    }

    return arc;
}

/*
 *@@ ExportAddTypesTree:
 *      writes all current types into the root
 *      element in the DOM tree.
 *
 *      Called from ftypExportTypes.
 *
 *@@added V0.9.12 (2001-05-21) [umoeller]
 *@@changed V1.0.5 (2006-05-14) [pr]: Fix error 42005 if no XWP FileTypes exist
 */

STATIC APIRET ExportAddTypesTree(PDOMNODE pRootElement,
                                 PXSTRING pstrBuf)          // in/out: working buffer (must be initialized)
{
    APIRET arc = NO_ERROR;
    PSZ pszAssocTypeList;
    LINKLIST llFileTypes;
    lstInit(&llFileTypes, TRUE);

    // step 1: load WPS file types list
    if (!(arc = prfhQueryKeysForApp(HINI_USER,
                                    WPINIAPP_ASSOCTYPE, // "PMWP_ASSOC_TYPE"
                                    &pszAssocTypeList)))
    {
        PSZ         pKey = pszAssocTypeList;
        PSZ         pszFileTypeHierarchyList;
        PLISTNODE   pAssocNode;

        while (*pKey != 0)
        {
            // for each WPS file type,
            // create a list item
            PFILETYPELISTITEM pliAssoc = malloc(sizeof(FILETYPELISTITEM));
            memset(pliAssoc, 0, sizeof(*pliAssoc));
            // mark as "not processed"
            // pliAssoc->fProcessed = FALSE;
            // set anti-recursion flag
            // pliAssoc->fCircular = FALSE;
            // store file type
            pliAssoc->pszFileType = strdup(pKey);
            // add item to list
            lstAppendItem(&llFileTypes, pliAssoc);

            // go for next key
            pKey += strlen(pKey)+1;
        }

        // step 2: load XWorkplace file types hierarchy
        if (!(arc = prfhQueryKeysForApp(HINI_USER,
                                        INIAPP_XWPFILETYPES, // "XWorkplace:FileTypes"
                                        &pszFileTypeHierarchyList)))
        {
            // step 3: go thru the file type hierarchy
            // and add parents;
            // AddFileTypeAndAllParents will mark the
            // inserted items as processed (for step 4)

            pKey = pszFileTypeHierarchyList;
            while ((*pKey != 0) && (!arc))
            {
                PDOMNODE pNewElement;
                PMPF_ASSOCS(("processing %s", pKey));

                arc = ExportAddFileTypeAndAllParents(pRootElement,
                                                     &llFileTypes,
                                                     pKey,
                                                     pstrBuf,
                                                     &pNewElement);
                                                // this will recurse
                #ifdef __DEBUG__
                if (arc)
                {
                    PMPF_ASSOCS(("ExportAddFileTypeAndAllParents returned %d for %s",
                                arc, pKey));
                }
                else
                    PMPF_ASSOCS(("%s processed OK", pKey));
                #endif

                // go for next key
                pKey += strlen(pKey)+1;
            }

            free(pszFileTypeHierarchyList); // was missing V0.9.12 (2001-05-12) [umoeller]
        }
        else
            if (arc == PRFERR_KEYSLIST)	// V1.0.5 (2006-05-14) [pr]
                arc = NO_ERROR;

        // step 4: add all remaining file types
        // to root level
        if (!arc)
        {
            pAssocNode = lstQueryFirstNode(&llFileTypes);

            while ((pAssocNode) && (!arc))
            {
                PFILETYPELISTITEM pliAssoc = (PFILETYPELISTITEM)pAssocNode->pItemData;

                if (!pliAssoc->fProcessed)
                {
                    // add to root element node
                    PDOMNODE pNewElement;
                    arc = ExportAddType(pRootElement,
                                        pliAssoc,
                                        pstrBuf,
                                        &pNewElement);
                    #ifdef __DEBUG__
                    if (arc)
                        PMPF_ASSOCS(("xmlCreateElementNode returned %d", arc));
                    #endif
                }

                pAssocNode = pAssocNode->pNext;
            }
        }

        free(pszAssocTypeList);
    }

    // clean up the list
    ftypClearTypesList(NULLHANDLE,     // no cnr here
                       &llFileTypes);

    return arc;
}

/*
 *@@ ftypExportTypes:
 *      writes the current types and filters setup into
 *      the specified XML file, which should have an
 *      extension of ".XTP".
 *
 *      Returns either a DOS or XML error code (see xml.h).
 *
 *@@added V0.9.12 (2001-05-21) [umoeller]
 *@@changed V1.0.2 (2003-02-07) [umoeller]: format changed; encoding non-ASCII values; DTD no longer exposed
 */

APIRET ftypExportTypes(PCSZ pcszFilename)        // in: XML file name
{
    APIRET arc = NO_ERROR;

    XSTRING     strBuf;     // working buffer V1.0.2 (2003-02-07) [umoeller]
    xstrInit(&strBuf, 0);

    TRY_LOUD(excpt1)
    {
        PDOMDOCUMENTNODE pDocument = NULL;
        PDOMNODE    pRootElement = NULL,
                    pCPAttr;

        // create a DOM
        if (!(arc = xmlCreateDocument("XWPFILETYPES",
                                      &pDocument,
                                      &pRootElement)))
        {
            // add the codepage attribute V1.0.2 (2003-02-07) [umoeller]
            CHAR    szEncoding[30];
            ULONG   lenEncoding;

            lenEncoding = sprintf(szEncoding,
                                  "%d",
                                  nlsQueryCodepage());

            // add the types tree
            if (    (!(arc = xmlCreateAttributeNode(pRootElement,
                                                    G_pcszCODEPAGE,
                                                    szEncoding,
                                                    lenEncoding,
                                                    &pCPAttr)))
                 && (!(arc = ExportAddTypesTree(pRootElement,
                                                &strBuf)))
               )
            {
                // create a text XML document from all this
                XSTRING strDocument;

                xstrInit(&strDocument, 1000);

                if (!(arc = xmlWriteDocument(pDocument,
                                             "US-ASCII",
                                             G_pcszDoctype,
                                             &strDocument)))
                {
                    xstrConvertLineFormat(&strDocument,
                                          LF2CRLF);
                    arc = doshWriteTextFile(pcszFilename,
                                            strDocument.psz,
                                            NULL,
                                            NULL);
                }

                xstrClear(&strDocument);
            }

            // kill the DOM document
            xmlDeleteNode((PNODEBASE)pDocument);
        }
        else
            PMPF_ASSOCS(("xmlCreateDocument returned %d", arc));

    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    xstrClear(&strBuf);

    return arc;
}

#endif // #ifndef __NOTURBOFOLDERS__
