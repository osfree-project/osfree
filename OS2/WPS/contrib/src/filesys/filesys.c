
/*
 *@@sourcefile filesys.c:
 *      various implementation code related to file-system objects.
 *      This is mostly for code which is shared between folders
 *      and data files.
 *      So this code gets interfaced from XFolder, XFldDataFile,
 *      and XWPProgramFile.
 *
 *      This file is ALL new with V0.9.0.
 *
 *      Function prefix for this file:
 *      --  fsys*
 *
 *@@added V0.9.0 [umoeller]
 *@@header "filesys\filesys.h"
 */

/*
 *      Copyright (C) 1997-2010 Ulrich M”ller.
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

#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSRESOURCES
#define INCL_DOSERRORS

#define INCL_WINPROGRAMLIST     // needed for PROGDETAILS, wppgm.h
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\eah.h"                // extended attributes helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\standards.h"          // some standard macros
#include "helpers\threads.h"            // thread helpers

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"
#include "xwppgmf.ih"

// XWorkplace implementation headers
#include "shared\classtest.h"           // some cheap funcs for WPS class checks
#include "shared\common.h"              // the majestic XWorkplace include file

#include "filesys\icons.h"              // icons handling
#include "filesys\filesys.h"            // various file-system object implementation code
#include "filesys\filetype.h"           // extended file types implementation
#include "filesys\folder.h"             // XFolder implementation
#include "filesys\object.h"             // XFldObject implementation

// other SOM headers
#pragma hdrstop                 // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

static PGEA2LIST    G_StandardGEA2List = NULL;
static ULONG        G_cbStandardGEA2List = 0;

/* ******************************************************************
 *
 *   File system information implementation
 *
 ********************************************************************/

/*
 *@@ fsysQueryEASubject:
 *      returns the contents of the .SUBJECT extended
 *      attribute in a new buffer, which must be free()'d
 *      by the caller.
 *
 *      Returns NULL on errors or if the EA doesn't exist.
 *
 *      The .SUBJECT extended attribute is a plain PSZ
 *      without line breaks.
 *
 *@@added V0.9.7 (2000-11-30) [umoeller]
 */

PSZ fsysQueryEASubject(WPFileSystem *somSelf)
{
    PSZ     psz = 0;
    CHAR    szFilename[CCHMAXPATH];
    if (_wpQueryFilename(somSelf, szFilename, TRUE))
    {
        PEABINDING  peab;
        if (peab = eaPathReadOneByName(szFilename, ".SUBJECT"))
        {
            psz = eaCreatePSZFromBinding(peab);
            eaFreeBinding(peab);
        }
    }

    return psz;
}

/*
 *@@ fsysQueryEAComments:
 *      returns the contents of the .COMMENTS extended
 *      attribute in a new buffer, which must be free()'d
 *      by the caller.
 *
 *      Returns NULL on errors or if the EA doesn't exist.
 *
 *      The .COMMENTS EA is multi-value multi-type, but all
 *      of the sub-types are EAT_ASCII. We convert all the
 *      sub-items into one string and separate the items
 *      with CR/LF.
 *
 *@@added V0.9.7 (2000-11-30) [umoeller]
 */

PSZ fsysQueryEAComments(WPFileSystem *somSelf)
{
    PSZ     psz = 0;
    CHAR    szFilename[CCHMAXPATH];
    if (_wpQueryFilename(somSelf, szFilename, TRUE))
    {
        PEABINDING  peab;
        if (peab = eaPathReadOneByName(szFilename, ".COMMENTS"))
        {
            psz = eaCreatePSZFromMVBinding(peab,
                                           "\r\n", // separator string
                                           NULL);  // codepage (not needed)
            eaFreeBinding(peab);
        }
    }

    return psz;
}

/*
 *@@ fsysQueryEAKeyphrases:
 *      returns the contents of the .KEYPHRASES extended
 *      attribute in a new buffer, which must be free()'d
 *      by the caller.
 *
 *      Returns NULL on errors or if the EA doesn't exist.
 *
 *      The .KEYPHRASES EA is multi-value multi-type, but all
 *      of the sub-types are EAT_ASCII. We convert all the
 *      sub-items into one string and separate the items
 *      with CR/LF.
 *
 *@@added V0.9.7 (2000-11-30) [umoeller]
 */

PSZ fsysQueryEAKeyphrases(WPFileSystem *somSelf)
{
    PSZ     psz = 0;
    CHAR    szFilename[CCHMAXPATH];
    if (_wpQueryFilename(somSelf, szFilename, TRUE))
    {
        PEABINDING  peab;
        if (peab = eaPathReadOneByName(szFilename, ".KEYPHRASES"))
        {
            psz = eaCreatePSZFromMVBinding(peab,
                                           "\r\n", // separator string
                                           NULL);  // codepage (not needed)

            eaFreeBinding(peab);
        }
    }

    return psz;
}

/*
 *@@ fsysSetEASubject:
 *      sets a new value for the .SUBJECT extended
 *      attribute.
 *
 *      If (psz == NULL), the EA is deleted.
 *
 *      This EA expects a plain PSZ string without
 *      line breaks.
 *
 *@@added V0.9.7 (2000-11-30) [umoeller]
 */

BOOL fsysSetEASubject(WPFileSystem *somSelf, PCSZ psz)
{
    BOOL brc = FALSE;
    CHAR    szFilename[CCHMAXPATH];

    if (_wpQueryFilename(somSelf, szFilename, TRUE))
    {
        PCSZ pcszEA = ".SUBJECT";
        if (psz)
        {
            PEABINDING  peab;
            if (peab = eaCreateBindingFromPSZ(pcszEA, psz))
            {
                brc = (NO_ERROR == eaPathWriteOne(szFilename, peab));
                eaFreeBinding(peab);
            }
        }
        else
            brc = (NO_ERROR == eaPathDeleteOne(szFilename, pcszEA));
    }

    return brc;
}

/*
 *@@ fsysSetEAComments:
 *      sets a new value for the .COMMENTS extended
 *      attribute.
 *
 *      If (psz == NULL), the EA is deleted.
 *
 *      This EA is multi-value multi-type, but all of
 *      the sub-types are EAT_ASCII. This function
 *      expects a string where several lines are
 *      separated with CR/LF, which is then converted
 *      into the multi-value EA.
 *
 *@@added V0.9.7 (2000-11-30) [umoeller]
 */

BOOL fsysSetEAComments(WPFileSystem *somSelf, PCSZ psz)
{
    BOOL brc = FALSE;
    CHAR    szFilename[CCHMAXPATH];

    if (_wpQueryFilename(somSelf, szFilename, TRUE))
    {
        PCSZ pcszEA = ".COMMENTS";
        if (psz)
        {
            PEABINDING  peab;
            if (peab = eaCreateMVBindingFromPSZ(pcszEA,
                                                psz,
                                                "\r\n",     // separator
                                                0))         // codepage
            {
                brc = (NO_ERROR == eaPathWriteOne(szFilename, peab));
                eaFreeBinding(peab);
            }
        }
        else
            brc = (NO_ERROR == eaPathDeleteOne(szFilename, pcszEA));
    }

    return brc;
}

/*
 *@@ fsysSetEAKeyphrases:
 *      sets a new value for the .KEYPHRASES extended
 *      attribute.
 *
 *      If (psz == NULL), the EA is deleted.
 *
 *      This EA is multi-value multi-type, but all of
 *      the sub-types are EAT_ASCII. This function
 *      expects a string where several lines are
 *      separated with CR/LF, which is then converted
 *      into the multi-value EA.
 *
 *@@added V0.9.7 (2000-11-30) [umoeller]
 */

BOOL fsysSetEAKeyphrases(WPFileSystem *somSelf, PCSZ psz)
{
    BOOL brc = FALSE;
    CHAR    szFilename[CCHMAXPATH];

    if (_wpQueryFilename(somSelf, szFilename, TRUE))
    {
        PCSZ pcszEA = ".KEYPHRASES";
        if (psz)
        {
            PEABINDING  peab;
            if (peab = eaCreateMVBindingFromPSZ(pcszEA,
                                                psz,
                                                "\r\n",     // separator
                                                0))         // codepage
            {
                brc = (NO_ERROR == eaPathWriteOne(szFilename, peab));
                eaFreeBinding(peab);
            }
        }
        else
            brc = (NO_ERROR == eaPathDeleteOne(szFilename, pcszEA));
    }

    return brc;
}

/* ******************************************************************
 *
 *   Populate / refresh
 *
 ********************************************************************/

/*
 *@@ fsysCreateStandardGEAList:
 *      sets up a pointer to the GEA2LIST which
 *      describes the EA names to look for during
 *      file-system populate. Since we always use
 *      the same list, we create this on
 *      M_XFolder::wpclsInitData and reuse that list
 *      forever.
 *
 *      GetGEAList then makes a copy of that for
 *      fdrPopulate. See remarks there.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 */

VOID fsysCreateStandardGEAList(VOID)
{
    /*
        Entries in the GEA2 list
        must be aligned on a doubleword boundary. Each oNextEntryOffset
        field must contain the number of bytes from the beginning of the
        current entry to the beginning of the next entry.

        typedef struct _GEA2LIST {
            ULONG     cbList;   // Total bytes of structure including full list.
            GEA2      list[1];  // Variable-length GEA2 structures.
        } GEA2LIST;

        typedef struct _GEA2 {
            ULONG     oNextEntryOffset;  // Offset to next entry.
            BYTE      cbName;            // Name length not including NULL.
            CHAR      szName[1];         // Attribute name.
        } GEA2;
    */

    if (!G_StandardGEA2List)
    {
        // first call:

        static PCSZ apcszEANames[] =
            {
                ".CLASSINFO",
                ".LONGNAME",
                ".TYPE",
                ".ICON"
            };

        // check how much memory we need:
        ULONG   ul;

        G_cbStandardGEA2List = sizeof(ULONG);       // GEA2LIST.cbList

        for (ul = 0;
             ul < ARRAYITEMCOUNT(apcszEANames);
             ul++)
        {
            G_cbStandardGEA2List +=   sizeof(ULONG)         // GEA2.oNextEntryOffset
                                    + sizeof(BYTE)          // GEA2.cbName
                                    + strlen(apcszEANames[ul])
                                    + 1;                    // null terminator

            // add padding, each entry must be dword-aligned
            G_cbStandardGEA2List += 4;
        }

        if (G_StandardGEA2List = (PGEA2LIST)malloc(G_cbStandardGEA2List))
        {
            PGEA2 pThis, pLast;

            G_StandardGEA2List->cbList = G_cbStandardGEA2List;
            pThis = G_StandardGEA2List->list;

            for (ul = 0;
                 ul < ARRAYITEMCOUNT(apcszEANames);
                 ul++)
            {
                pThis->cbName = strlen(apcszEANames[ul]);
                memcpy(pThis->szName,
                       apcszEANames[ul],
                       pThis->cbName + 1);

                pThis->oNextEntryOffset =   sizeof(ULONG)
                                          + sizeof(BYTE)
                                          + pThis->cbName
                                          + 1;

                // add padding, each entry must be dword-aligned
                pThis->oNextEntryOffset += 3 - ((pThis->oNextEntryOffset + 3) & 0x03);
                            // 1:   1 + 3 = 4  = 0      should be 3
                            // 2:   2 + 3 = 5  = 1      should be 2
                            // 3:   3 + 3 = 6  = 2      should be 1
                            // 4:   4 + 3 = 7  = 3      should be 0

                pLast = pThis;
                pThis = (PGEA2)(((PBYTE)pThis) + pThis->oNextEntryOffset);
            }

            pLast->oNextEntryOffset = 0;
        }
    }
}

/*
 *@@ GetGEA2List:
 *      returns a copy of the GEA2 list created by
 *      fsysCreateStandardGEAList. CPREF says that
 *      this buffer is modified internally by
 *      DosFindFirst/Next, so we need a separate
 *      buffer for each populate.
 *
 *      The caller is responsible for free()ing
 *      the buffer.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

STATIC PGEA2LIST GetGEA2List(VOID)
{
    PGEA2LIST pList;
    if (pList = (PGEA2LIST)malloc(G_cbStandardGEA2List))
        memcpy(pList, G_StandardGEA2List, G_cbStandardGEA2List);

    return pList;
}

/*
 *@@ fsysCreateFindBuffer:
 *      called from PopulateWithFileSystems to
 *      allocate and set up a buffer for DosFindFirst/Next
 *      or DosQueryPathInfo.
 *
 *      Use fsysFreeFindBuffer to free the buf again.
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

APIRET fsysCreateFindBuffer(PEAOP2 *pp)
{
    APIRET arc;

    if (!(arc = DosAllocMem((PVOID*)pp,
                            FINDBUFSIZE, // 64K
                            OBJ_TILE | PAG_COMMIT | PAG_READ | PAG_WRITE)))
    {
        // set up the EAs list... sigh

        /*  CPREF: On input, pfindbuf contains an EAOP2 data structure. */
        PEAOP2      peaop2 = *pp;
                    /*  typedef struct _EAOP2 {
                          PGEA2LIST     fpGEA2List;
                          PFEA2LIST     fpFEA2List;
                          ULONG         oError;
                        } EAOP2; */

        /*  CPREF: fpGEA2List contains a pointer to a GEA2 list, which
            defines the EA names whose values are to be returned. */

        // since we try the same EAs for the objects, we
        // create a GEA2LIST only once and reuse that forever:
        peaop2->fpGEA2List = GetGEA2List();     // freed by fsysFreeFindBuffer

        // set up FEA2LIST output buffer: right after the leading EAOP2
        peaop2->fpFEA2List          = (PFEA2LIST)(peaop2 + 1);
        peaop2->fpFEA2List->cbList  =    FINDBUFSIZE   // 64K
                                       - sizeof(EAOP2);
        peaop2->oError              = 0;
    }

    return arc;
}

/*
 *@@ fsysFillFindBuffer:
 *      fills a FILEFINDBUF3L for the given file system
 *      object, including all the EAs.
 *
 *      This calls fsysCreateFindBuffer internally,
 *      so the caller is responsible for calling
 *      fsysFreeFindBuffer(peaop) when done.
 *
 *      For convenience, *ppfb3 is set to the address
 *      of the FILEFINDBUF3L within the EAOP2 buffer.
 *
 *@@added V0.9.18 (2002-03-19) [umoeller]
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 */

APIRET fsysFillFindBuffer(PCSZ pcszFilename,       // in: fully q'fied filename to check
                          PFILEFINDBUF3L *ppfb3,   // out: ptr into *ppeaop
                          PEAOP2 *ppeaop)          // out: buffer to be freed
{
    APIRET arc;
    if (!(arc = fsysCreateFindBuffer(ppeaop)))
                // freed at bottom
    {
        HDIR        hdirFindHandle = HDIR_CREATE;
        ULONG       ulFindCount = 1;
        if (!(arc = DosFindFirst((PSZ)pcszFilename,
                                 &hdirFindHandle,
                                 FILE_DIRECTORY
                                    | FILE_ARCHIVED | FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
                                 *ppeaop,     // buffer
                                 FINDBUFSIZE, // 64K
                                 &ulFindCount,
                                 FIL_QUERYEASFROMLISTL)))
        {
            *ppfb3 = (PFILEFINDBUF3L)(((PBYTE)*ppeaop) + sizeof(EAOP2));
        }

        DosFindClose(hdirFindHandle);
    }

    return arc;
}

/*
 *@@ fsysFreeFindBuffer:
 *
 *@@added V0.9.16 (2002-01-01) [umoeller]
 */

VOID fsysFreeFindBuffer(PEAOP2 *pp)
{
    PEAOP2 peaop;
    if (    (pp)
         && (peaop = *pp)
       )
    {
        FREE(peaop->fpGEA2List);
        DosFreeMem(peaop);

        *pp = NULL;
    }
}

// find this many files at a time
#define FIND_COUNT              300
            // doesn't make a whole lot of difference whether
            // I set this to 1 or 300 on my system, but maybe
            // on SMP systems this helps?!?

/*
 *@@ CLASSINFO:
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

typedef struct _FSCLASSINFO
{
    ULONG   whatever;
    ULONG   cbObjData;          // if != 0, length of object data after szClassName
    CHAR    szClassName[1];
} FSCLASSINFO, *PFSCLASSINFO;

/*
 *@@ DecodeClassInfo:
 *      decodes the .CLASSINFO EA, if present.
 *      If so, it returns the name of the class
 *      to be used for the folder or file.
 *      Otherwise NULL is returned.
 *
 *      Note that this also sets the given POBJDATA
 *      pointer to the OBJDATA structure found in
 *      the classinfo. This ptr will always be set,
 *      either to NULL or the OBJDATA structure.
 *
 *      If this returns NULL, the caller is responsible
 *      for finding out the real folder or data file
 *      class to be used.
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

STATIC PCSZ DecodeClassInfo(PFEA2LIST pFEA2List2,
                            PULONG pulClassNameLen,    // out: strlen of the return value
                            POBJDATA *ppObjData)       // out: OBJDATA for _wpclsMakeAwake
{
    PCSZ        pcszClassName = NULL;
    ULONG       ulClassNameLen = 0;

    PBYTE pbValue;

    *ppObjData = NULL;

    if (pbValue = doshFindEAValue(pFEA2List2,
                                  ".CLASSINFO",
                                  NULL))
    {
        PUSHORT pusType = (PUSHORT)pbValue;
        if (*pusType == EAT_BINARY)
        {
            // CPREF: first word after EAT_BINARY specifies length
            PUSHORT pusDataLength = pusType + 1;      // pbValue + 2

            PFSCLASSINFO pInfo = (PFSCLASSINFO)(pusDataLength + 1); // pbValue + 4

            if (ulClassNameLen = strlen(pInfo->szClassName))
            {
                if (pInfo->cbObjData)
                {
                    // we have OBJDATA after szClassName:
                    *ppObjData = (POBJDATA)(   pInfo->szClassName
                                             + ulClassNameLen
                                             + 1);              // null terminator
                }

                pcszClassName = pInfo->szClassName;
            }
        }
    }

    *pulClassNameLen = ulClassNameLen;

    return pcszClassName;
}

/*
 *@@ FindBestDataFileClass:
 *      gets called for all instances of WPDataFile
 *      to find the real data file class to be used
 *      for instantiating the object.
 *
 *      Returns either the name of WPDataFile subclass,
 *      if wpclsQueryInstanceType/Filter of a class
 *      requested ownership of this object, or NULL
 *      for the default "WPDataFile".
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

STATIC PCSZ FindBestDataFileClass(PFEA2LIST pFEA2List2,
                                  PCSZ pcszObjectTitle,
                                  ULONG ulTitleLen)      // in: length of title string (req.)
{
    PCSZ pcszClassName = NULL;

    PBYTE pbValue;

    if (pbValue = doshFindEAValue(pFEA2List2,
                                  ".TYPE",
                                  NULL))
    {
        // file has .TYPE EA:
        PUSHORT pusType = (PUSHORT)pbValue;
        if (*pusType == EAT_MVMT)
        {
            // layout of EAT_MVMT:
            // 0    WORD     EAT_MVMT        (pusType)
            // 2    WORD     usCodepage      (pusType + 1, pbValue + 2)
            //               if 0, system default codepage
            // 4    WORD     cEntries        (pusType + 2, pbValue + 4)
            // 6    type 0   WORD    EAT_ASCII
            //               WORD    length
            //               CHAR[]  achType   (not null-terminated)
            //      type 1   WORD    EAT_ASCII
            //               WORD    length
            //               CHAR[]  achType   (not null-terminated)

            PUSHORT pusCodepage = pusType + 1;      // pbValue + 2
            PUSHORT pcEntries = pusCodepage + 1;    // pbValue + 4

            PBYTE   pbEAThis = (PBYTE)(pcEntries + 1);  // pbValue + 6

            ULONG ul;
            for (ul = 0;
                 ul < *pcEntries;
                 ul++)
            {
                PUSHORT pusTypeThis = (PUSHORT)pbEAThis;
                if (*pusTypeThis == EAT_ASCII)
                {
                    PUSHORT pusLengthThis = pusTypeThis + 1;
                    // next sub-EA:
                    PSZ pszType  =   pbEAThis
                                   + sizeof(USHORT)      // EAT_ASCII
                                   + sizeof(USHORT);     // usLength
                    PBYTE pbNext =   pszType
                                   + (*pusLengthThis);   // skip string

                    // null-terminate the type string
                    CHAR c = *pbNext;
                    *pbNext = '\0';
                    // pszType now has the null-terminated type string:
                    // try to find the class
                    if (pcszClassName = ftypFindClassFromInstanceType(pszType))
                        // we can stop here
                        break;

                    *pbNext = c;
                    pbEAThis = pbNext;
                }
                else
                    // non-ASCII: we cannot handle this!
                    break;
            }
        }
    }

    if (!pcszClassName)
        // instance types didn't help: then go for the
        // instance filters
        pcszClassName = ftypFindClassFromInstanceFilter(pcszObjectTitle,
                                                        ulTitleLen);

    return pcszClassName;     // can be NULL
}

/*
 *@@ RefreshOrAwake:
 *      called by PopulateWithFileSystems for each file
 *      or directory returned by DosFindFirst/Next.
 *
 *      On input, we get the sick FILEFINDBUF3L returned
 *      from DosFindFirst/Next, which contains both
 *      the EAs for the object and its real name.
 *
 *      This checks if the object is already awake in
 *      the folder. If so, it is refreshed if necessary.
 *
 *      If the object is not awake, it is awakened by
 *      a call to wpclsMakeAwake with the correct class
 *      object.
 *
 *      In any case (refresh or awakening), the object
 *      is locked once in this call.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 *@@changed V0.9.18 (2002-02-06) [umoeller]: fixed duplicate awakes and "treeInsert failed"
 *@@changed V0.9.19 (2002-04-14) [umoeller]: fixed missing FOUNDBIT after awake
 *@@changed V1.0.1 (2002-12-08) [umoeller]: added detection of .LONGNAME-changed case @@fixes 238
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 */

STATIC WPFileSystem* RefreshOrAwake(WPFolder *pFolder,
                                    PFILEFINDBUF3L pfb3)
{
    WPFileSystem *pAwake = NULL;

    BOOL fFolderLocked = FALSE;

    // Alright, the caller has given us a pointer
    // into the return buffer from DosFindFirst;
    // we have declared this to be a FILEFINDBUF3L
    // here, but according to CPREF, the struct is
    // _without_ the cchName and achName fields...

    // My lord, who came up with this garbage?!?

    // the FEA2LIST with the EAs comes after the
    // truncated FILEFINDBUF3L, that is, at the
    // address where FILEFINDBUF3L.cchName would
    // normally be
    PFEA2LIST pFEA2List2 = (PFEA2LIST)(   ((PBYTE)pfb3)
                                        + FIELDOFFSET(FILEFINDBUF3L,
                                                      cchName));

    // next comes a UCHAR with the name length
    PUCHAR puchNameLen = (PUCHAR)(((PBYTE)pFEA2List2) + pFEA2List2->cbList);

    // finally, the (real) name of the object
    PSZ pszRealName = ((PBYTE)puchNameLen) + sizeof(UCHAR);

    static ULONG s_ulWPDataFileLen = 0;

    // _PmpfF(("processing %s", pszRealName));

    // now, ignore "." and ".." which we don't want to see
    // in the folder, of course
    if (    (pszRealName[0] == '.')
         && (    (*puchNameLen == 1)
              || (    (*puchNameLen == 2)
                   && (pszRealName[1] == '.')
                 )
            )
       )
        return NULL;

    if (!s_ulWPDataFileLen)
        // on first call, cache length of "WPDataFile" string
        s_ulWPDataFileLen = strlen(G_pcszWPDataFile);

    TRY_LOUD(excpt1)
    {
        // sem was missing, this produced "treeInsertFailed",
        // and duplicate awakes for the same object sometimes
        // V0.9.18 (2002-02-06) [umoeller]
        if (fFolderLocked = !_wpRequestFolderMutexSem(pFolder, SEM_INDEFINITE_WAIT))
        {
            CHAR            szLongname[CCHMAXPATH];
            ULONG           ulTitleLen;
            PSZ             pszTitle;

            // for the title of the new object, use the real
            // name, unless we also find a .LONGNAME attribute,
            // so decode the EA buffer

            // moved this up: we need to check the title for
            // refreshing awake objects too (bug 238)
            // V1.0.1 (2002-12-08) [umoeller]
            if (doshQueryLongname(pFEA2List2,
                                  szLongname,
                                  &ulTitleLen))
                // got .LONGNAME:
                pszTitle = szLongname;
            else
            {
                // no .LONGNAME:
                pszTitle = pszRealName;
                ulTitleLen = *puchNameLen;
            }

            // alright, apparently we got something:
            // check if it is already awake (using the
            // fast content tree functions)
            if (pAwake = fdrFastFindFSFromName(pFolder,
                                               pszRealName))
            {
                FILEFINDBUF4        ffb4;

                _wpLockObject(pAwake);

                // now set the refresh flags... since wpPopulate gets in turn
                // called by wpRefresh, we are responsible for setting the
                // "dirty" and "found" bits here, or the object will disappear
                // from the folder on refresh.
                // For about how this works, the Warp 4 WPSREF says:

                //    1. Loop through all of the objects in the folder and turn on the DIRTYBIT
                //       and turn off the FOUNDBIT for all of your objects.
                //    2. Loop through the database. For every entry in the database, find the
                //       corresponding object.
                //         a. If the object exists, turn on the FOUNDBIT for the object.
                //         b. If the object does not exist, create a new object with the
                //            FOUNDBIT turned on and the DIRTYBIT turned off.
                //    3. Loop through the objects in the folder again. For any object that has
                //       the FOUNDBIT turned off, delete the object (since there is no longer a
                //       corresponding entry in the database). For any object that has the
                //       DIRTYBIT turned on, update the view with the current contents of the
                //       object and turn its DIRTYBIT off.

                // Note, these two wpSet/QueryRefreshFlags methods have always been in
                // the toolkit headers, but are only documented with the Warp 4 toolkit.
                // We used to have wrappers around them, but this wasn't necessary and
                // has thus been removed V0.9.20 (2002-07-25) [umoeller].

                // Now, since the objects disappear on refresh, I assume
                // we need to set the FOUNDBIT to on; since we are refreshing
                // here already, we can set DIRTYBIT to off as well.
                _wpSetRefreshFlags(pAwake,
                                   (_wpQueryRefreshFlags(pAwake)
                                        & ~DIRTYBIT)
                                        | FOUNDBIT);

                // this is way faster, I believe V0.9.16 (2001-12-18) [umoeller]
                _wpQueryDateInfo(pAwake, &ffb4);

                // in both ffb3 and ffb4, fdateCreation is the first date/time field;
                // FDATE and FTIME are a USHORT each, and the decl in the toolkit
                // has #pragma pack(2), so this should work
                if (    (memcmp(&pfb3->fdateCreation,
                                &ffb4.fdateCreation,
                                3 * (sizeof(FDATE) + sizeof(FTIME)) ))
                     // refresh also if .LONGNAME changed V1.0.1 (2002-12-08) [umoeller]
                     || (strcmp(pszTitle, _wpQueryTitle(pAwake)))
                   )
                {
                    // object changed: go refresh it
                    _wpRefreshFSInfo(pAwake, NULLHANDLE, NULL, TRUE);
                            // safe to call this method now since we have managed
                            // to override it V0.9.20 (2002-07-25) [umoeller]
                            // now using NULL instead of pfb3 - guess it wants a
                            // FILEFINDBUF3 not a FILEFINDBUF3L but who knows
                            // as it's completely undocumented? V1.0.9 [pr]
                }
            }
            else
            {
                // no: wake it up then... this is terribly complicated:
                POBJDATA        pObjData = NULL;

                PCSZ            pcszClassName = NULL;
                ULONG           ulClassNameLen;
                somId           somidClassName;
                SOMClass        *pClassObject;

                // NOTE about the class management:
                // At this point, we operate on class _names_
                // only and do not mess with class objects yet. This
                // is because we must take class replacements into
                // account; that is, if the object says "i wanna be
                // WPDataFile", it should really be XFldDataFile
                // or whatever other class replacements are installed.
                // While the _WPDataFile macro will not always correctly
                // resolve (since apparently this code gets called
                // too early to properly initialize the static variables
                // hidden in the macro code), somFindClass _will_
                // return the proper replacement classes.

                // _PmpfF(("checking %s", pszTitle));

                // decode the .CLASSINFO EA, which may give us a
                // class name and the OBJDATA buffer
                if (!(pcszClassName = DecodeClassInfo(pFEA2List2,
                                                      &ulClassNameLen,
                                                      &pObjData)))
                {
                    // no .CLASSINFO: use default class...
                    // if this is a directory, use _WPFolder
                    if (pfb3->attrFile & FILE_DIRECTORY)
                        pcszClassName = G_pcszWPFolder;
                    // else for WPDataFile, keep NULL so we
                    // can determine the proper class name below
                }
                else
                {
                    // we found a class name:
                    // if this is "WPDataFile", return NULL instead so we
                    // can still check for the default data file subclasses

                    // _Pmpf(("  got .CLASSINFO %s", pcszClassName));

                    if (    (s_ulWPDataFileLen == ulClassNameLen)
                         && (!memcmp(G_pcszWPDataFile, pcszClassName, s_ulWPDataFileLen))
                       )
                        pcszClassName = NULL;
                }

                if (!pcszClassName)
                {
                    // still NULL: this means we have no .CLASSINFO,
                    // or the .CLASSINFO specified "WPDataFile"
                    // (folders were ruled out before, so we do have
                    // a data file now)...
                    // for WPDataFile, we must run through the
                    // wpclsQueryInstanceType/Filter methods to
                    // find if any WPDataFile subclass wants this
                    // object to be its own (for example, .EXE files
                    // should be WPProgramFile instead)
                    pcszClassName = FindBestDataFileClass(pFEA2List2,
                                                          // title (.LONGNAME or realname)
                                                          pszTitle,
                                                          ulTitleLen);
                            // this returns either NULL or the
                            // class object of a subclass

                    // _Pmpf(("  FindBestDataFileClass = %s", pcszClassName));
                }

                if (!pcszClassName)
                    // still nothing:
                    pcszClassName = G_pcszWPDataFile;

                // now go load the class
                if (somidClassName = somIdFromString((PSZ)pcszClassName))
                {
                    if (!(pClassObject = _somFindClass(SOMClassMgrObject,
                                                       somidClassName,
                                                       0,
                                                       0)))
                    {
                        // this class is not installed:
                        // this can easily happen with multiple OS/2
                        // installations accessing the same partitions...
                        // to be on the safe side, use either
                        // WPDataFile or WPFolder then
                        if (pfb3->attrFile & FILE_DIRECTORY)
                            pcszClassName = G_pcszWPFolder;
                        else
                            pcszClassName = G_pcszWPDataFile;

                        SOMFree(somidClassName);
                        if (somidClassName = somIdFromString((PSZ)pcszClassName))
                            pClassObject = _somFindClass(SOMClassMgrObject,
                                                         somidClassName,
                                                         0,
                                                         0);
                    }
                }

                if (pClassObject)
                {
                    MAKEAWAKEFS   awfs;

                    // alright, now go make the thing AWAKE
                    awfs.pszRealName        = pszRealName;

                    memcpy(&awfs.Creation, &pfb3->fdateCreation, sizeof(FDATETIME));
                    memcpy(&awfs.LastAccess, &pfb3->fdateLastAccess, sizeof(FDATETIME));
                    memcpy(&awfs.LastWrite, &pfb3->fdateLastWrite, sizeof(FDATETIME));

                    awfs.attrFile           = pfb3->attrFile;
                    // No idea how the high DWORD is supposed to get set here
                    // You'd have thought IBM would've changed it, but it seems
                    // that's not the case V1.0.9 [pr]
                    awfs.cbFile             = pfb3->cbFile.ulLo;
                    awfs.cbList             = pFEA2List2->cbList;
                    awfs.pFea2List          = pFEA2List2;

                    if (pAwake = _wpclsMakeAwake(pClassObject,
                                                 pszTitle,
                                                 0,                 // style
                                                 NULLHANDLE,        // icon
                                                 pObjData,          // null if no .CLASSINFO found
                                                 pFolder,           // folder
                                                 (ULONG)&awfs))
                    {
                        #ifdef __DEBUG__
                            ULONG fl = _wpQueryRefreshFlags(pAwake);
                            PMPF_TURBOFOLDERS(("refresh flags for new \"%s\": 0x%lX (%s%s)",
                                pszRealName,
                                fl,
                                (fl & FOUNDBIT) ? "FOUNDBIT" : "",
                                (fl & DIRTYBIT) ? "DIRTYBIT" : ""));
                        #endif

                        // refresh flags are 0 always after creation,
                        // so turn on the FOUNDBIT but leave DIRTYBIT
                        // off
                        // V0.9.19 (2002-04-14) [umoeller]
                        _wpSetRefreshFlags(pAwake, FOUNDBIT);
                    }
                }

                if (somidClassName)
                    SOMFree(somidClassName);
            }
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    if (fFolderLocked)
        _wpReleaseFolderMutexSem(pFolder);

    return pAwake;
}

/*
 *@@ SYNCHPOPULATETHREADS:
 *      structure for communication between
 *      PopulateWithFileSystems and fntFindFiles.
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 */

typedef struct _SYNCHPOPULATETHREADS
{
    // input parameters copied from PopulateWithFileSystems
    // so fntFindFiles knows what to look for
    PCSZ            pcszFolderFullPath;     // wpQueryFilename(somSelf, TRUE)
    PCSZ            pcszFileMask;           // NULL or file mask to look for
    BOOL            fFoldersOnly;           // TRUE if folders only

    // two 64K buffers allocated by PopulateWithFileSystems
    // for use with DosFindFirst/Next;
    // after DosFindFirst has found something in fntFindFiles,
    // PopulateWithFileSystems() can process that buffer,
    // while fntFindFiles can already run DosFindNext on the
    // second buffer.
    PEAOP2          pBuf1,      // fpGEA2List has GetGEA2List buffer to be freed
                    pBuf2;

    // current buffer to work on for PopulateWithFileSystems;
    // set by fntFindFiles after each DosFindFirst/Next.
    // This points into either pBuf1 or pBuf2 (after the
    // EAOP2 structure).
    // This must only be read or set by the owner of hmtxBuffer.
    // As a special rule, if fntFindFiles sets this to NULL,
    // it is done with DosFindFirst/Next.
    PFILEFINDBUF3L  pfb3;
    ULONG           ulFindCount;        // find count from DosFindFirst/Next

    // synchronization semaphores:
    // 1) current owner of the buffer
    //    RULE: only the owner of this mutex may post or
    //    reset any of the event semaphores
    HMTX            hmtxBuffer;
    // 2) "buffer taken" event sem; posted by PopulateWithFileSystems
    //    after it has copied the pfb3 pointer (before it starts
    //    processing the buffer); fntFindFiles blocks on this before
    //    switching the buffer again
    HEV             hevBufTaken;
    // 3) "buffer changed" event sem; posted by fntFindFiles
    //    after it has switched the buffer so that
    //    PopulateWithFileSystems knows new data is available
    //    (or DosFindFirst/Next is done);
    //    PopulateWithFileSystems blocks on this before processing
    //    the buffer
    HEV             hevBufPtrChanged;

    // return code from fntFindFiles, valid only after exit
    APIRET          arcReturn;

} SYNCHPOPULATETHREADS, *PSYNCHPOPULATETHREADS;

/*
 *@@ fntFindFiles:
 *      find-files thread started by PopulateWithFileSystems.
 *      This actually does the DosFindFirst/Next loop and
 *      fills a result buffer for PopulateWithFileSystems
 *      to create objects from.
 *
 *      This allows us to get better CPU utilization since
 *      DosFindFirst/Next produce a lot of idle time (waiting
 *      for a disk transaction to complete), especially with
 *      the tons of EAs we are trying to read here.
 *      We can use this idle time to do all the CPU-intensive
 *      object creation instead of doing "find file" and
 *      "create object" synchronously.
 *
 *      Note that this requires a lot of evil synchronization
 *      between the two threads. A SYNCHPOPULATETHREADS structure
 *      is created on   PopulateWithFileSystems's stack to organize
 *      this. See remarks there for details.
 *
 *      This thread does _not_ have a message queue.
 *
 *@@added V0.9.16 (2001-10-28) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: lowered priority
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 */

STATIC void _Optlink fntFindFiles(PTHREADINFO ptiMyself)
{
    PSYNCHPOPULATETHREADS pspt = (PSYNCHPOPULATETHREADS)ptiMyself->ulData;
    HDIR            hdirFindHandle = HDIR_CREATE;

    BOOL            fSemBuffer = FALSE;
    APIRET          arc;

    TRY_LOUD(excpt1)
    {
        if (    (!(arc = DosCreateEventSem(NULL,    // unnamed
                                           &pspt->hevBufTaken,
                                           0,       // unshared
                                           FALSE))) // not posted
             && (!(arc = DosCreateEventSem(NULL,    // unnamed
                                           &pspt->hevBufPtrChanged,
                                           0,       // unshared
                                           FALSE))) // not posted
             && (!(arc = DosCreateMutexSem(NULL,
                                           &pspt->hmtxBuffer,
                                           0,
                                           TRUE)))      // request! this blocks out the
                                                        // second thread
             && (fSemBuffer = TRUE)
           )
        {
            CHAR            szFullMask[2*CCHMAXPATH];
            ULONG           attrFind;
            LONG            cb;

            ULONG           ulFindCount;

            PBYTE           pbCurrentBuffer;

            PCSZ            pcszFileMask = pspt->pcszFileMask;

            // crank up the priority of this thread so
            // that we get the CPU as soon as there's new
            // data from DosFindFirst/Next; since we are
            // blocked most of the time, this ensures
            // that the CPU is used most optimally
            DosSetPriority(PRTYS_THREAD,
                           PRTYC_TIMECRITICAL,
                           +31,
                           0);      // current thread

            // post thread semaphore so that thrCreate returns
            DosPostEventSem(ptiMyself->hevRunning);

            if (!pcszFileMask)
                pcszFileMask = "*";

            sprintf(szFullMask,
                    "%s\\%s",
                    pspt->pcszFolderFullPath,
                    pcszFileMask);

            if (pspt->fFoldersOnly)
                attrFind =   MUST_HAVE_DIRECTORY
                           | FILE_ARCHIVED | FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY;
            else
                attrFind =   FILE_DIRECTORY
                           | FILE_ARCHIVED | FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY;

            // on the first call, use buffer 1
            pbCurrentBuffer = (PBYTE)pspt->pBuf1;

            ulFindCount = FIND_COUNT;
            arc = DosFindFirst(szFullMask,
                               &hdirFindHandle,
                               attrFind,
                               pbCurrentBuffer,     // buffer
                               FINDBUFSIZE,
                               &ulFindCount,
                               FIL_QUERYEASFROMLISTL);

            // start looping...
            while (    (arc == NO_ERROR)
                    || (arc == ERROR_BUFFER_OVERFLOW)
                  )
            {
                // go process this file or directory
                ULONG ulPosted;

                // On output from DosFindFirst/Next, the buffer
                // has the EAOP2 struct first, which we no
                // longer care about... after that comes
                // a truncated FILEFINDBUF3L with all the
                // data we need, so give this to the populate
                // thread, which calls RefreshOrAwake.

                // Note that at this point, we _always_ own
                // hmtxBuffer; on the first loop because it
                // was initially requested, and later on
                // because of the explicit request below.

                // 1) set buffer pointer for populate thread
                pspt->pfb3 = (PFILEFINDBUF3L)(pbCurrentBuffer + sizeof(EAOP2));
                pspt->ulFindCount = ulFindCount;        // items found
                // 2) unset "buffer taken" event sem
                DosResetEventSem(pspt->hevBufTaken, &ulPosted);
                // 3) tell second thread we're going for DosFindNext
                // now, which will block on this
                // _PmpfF(("posting hevBufPtrChanged"));
                DosPostEventSem(pspt->hevBufPtrChanged);

                if (!ptiMyself->fExit)
                {
                    // 4) release buffer mutex; the second thread
                    //    is blocked on this and will then run off
                    // _PmpfF(("releasing hmtxBuffer"));
                    DosReleaseMutexSem(pspt->hmtxBuffer);
                    fSemBuffer = FALSE;

                    // _PmpfF(("blocking on hevBufTaken"));
                    if (    (!ptiMyself->fExit)
                         && (!(arc = DosWaitEventSem(pspt->hevBufTaken,
                                                     SEM_INDEFINITE_WAIT)))
                            // check again, second thread might be exiting now
                         && (!ptiMyself->fExit)
                       )
                    {
                        // alright, we got something else:
                        // request the buffer mutex again
                        // and re-loop; above, we will block
                        // again until the second thread has
                        // taken the new buffer
                        // _PmpfF(("blocking on hmtxBuffer"));
                        if (!(arc = DosRequestMutexSem(pspt->hmtxBuffer,
                                                       SEM_INDEFINITE_WAIT)))
                        {
                            fSemBuffer = TRUE;
                            // switch the buffer so we can load next file
                            // while second thread is working on the
                            // previous one
                            if (pbCurrentBuffer == (PBYTE)pspt->pBuf1)
                                pbCurrentBuffer = (PBYTE)pspt->pBuf2;
                            else
                                pbCurrentBuffer = (PBYTE)pspt->pBuf1;

                            // find next:
                            // _PmpfF(("DosFindNext"));
                            ulFindCount = FIND_COUNT;
                            arc = DosFindNext(hdirFindHandle,
                                              pbCurrentBuffer,
                                              FINDBUFSIZE,
                                              &ulFindCount);
                        }
                    } // end if !DosWaitEventSem(pspt->hevBufTaken)

                } // if (!ptiMyself->fExit)

                if (ptiMyself->fExit)
                    // we must exit for some reason:
                    break;

            } // while (arc == NO_ERROR)

            if (arc == ERROR_NO_MORE_FILES)
            {
                // nothing found is not an error
                arc = NO_ERROR;
            }
        }
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    // post thread semaphore so that thrCreate returns,
    // in case we haven't even gotten to the above call
    DosPostEventSem(ptiMyself->hevRunning);

    // cleanup:

    DosFindClose(hdirFindHandle);

    if (!fSemBuffer)
        if (!DosRequestMutexSem(pspt->hmtxBuffer,
                                SEM_INDEFINITE_WAIT))
            fSemBuffer = TRUE;

    // tell populate thread we're done
    if (fSemBuffer)
    {
        // buffer == NULL means no more data
        pspt->pfb3 = NULL;

        // post "buf changed" because populate
        // blocks on this
        DosPostEventSem(pspt->hevBufPtrChanged);
        DosReleaseMutexSem(pspt->hmtxBuffer);
    }

    // return what we have
    pspt->arcReturn = arc;

    // _PmpfF(("exiting"));
}

/*
 *@@ fsysPopulateWithFSObjects:
 *      called from fdrPopulate to get the file-system
 *      objects.
 *
 *      This starts off a second thread which does
 *      the DosFindFirst/Next loop. See fntFindFiles.
 *
 *      --  For file-system objects,
 *          fsysPopulateWithFSObjects() starts a second
 *          thread which does the actual DosFindFirst/Next
 *          processing (see fntFindFiles). This allows
 *          us to use the idle time produced by
 *          DosFindFirst/Next for the SOM processing
 *          which is quite expensive.
 *
 *      --  Since we can use our fast folder content
 *          trees (see fdrSafeFindFSFromName), this
 *          is a _lot_ faster for folders with many
 *          file system objects because we can check
 *          much more quickly if an object is already
 *          awake.
 *
 *      Benchmarks (pure populate with the
 *      QUICKOPEN=IMMEDIATE setup string, so no container
 *      management involved):
 *
 +      +--------------------+-------------+-------------+-------------+
 +      |                    | turbo on    | turbo on    |  turbo off  |
 +      |                    | findcnt 1   | findcnt 300 |             |
 +      +--------------------+-------------+-------------+-------------+
 +      |   JFS folder with  |     53 s    |             |      160 s  |
 +      |   10000 files      |             |             |             |
 +      +--------------------+-------------+-------------+-------------+
 +      |   JFS folder with  |     60 s    |     60 s    |      211 s  |
 +      |   13000 files      |             |             |             |
 +      +--------------------+-------------+-------------+-------------+
 +      |   HPFS folder with |     56 s    |             |             |
 +      |   10.000 files     |             |             |             |
 +      +--------------------+-------------+-------------+-------------+
 *
 *      Obviously, the time that the default WPS populate
 *      takes increases exponentially with the no. of objects
 *      in the folder. As a result, the fuller a folder is,
 *      the better this replacement becomes in comparison.
 *
 *      Two bottlenecks remain for folder populating...
 *      one is DosFindFirst/Next, which is terminally
 *      slow (and which I cannot fix), the other is the
 *      record management in the containers.
 *
 *@@added V0.9.16 (2001-10-25) [umoeller]
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 */

BOOL fsysPopulateWithFSObjects(WPFolder *somSelf,
                               HWND hwndReserved,
                               PMINIRECORDCORE pMyRecord,
                               PCSZ pcszFolderFullPath,  // in: wpQueryFilename(somSelf, TRUE)
                               BOOL fFoldersOnly,
                               PCSZ pcszFileMask,     // in: file mask or NULL for "*" (ignored if fFoldersOnly)
                               PBOOL pfExit)          // in: exit flag
{
    APIRET      arc;

    THREADINFO  tiFindFiles;
    volatile TID tidFindFiles = 0;

    // structure on stack to synchronize our two threads
    SYNCHPOPULATETHREADS spt;

    BOOL        fBufSem = FALSE;

    memset(&spt, 0, sizeof(spt));
    spt.pcszFileMask = pcszFileMask;
    spt.pcszFolderFullPath = pcszFolderFullPath;
    spt.fFoldersOnly = fFoldersOnly;

            // allocate two 64K buffers
    if (    (!(arc = fsysCreateFindBuffer(&spt.pBuf1)))
         && (!(arc = fsysCreateFindBuffer(&spt.pBuf2)))
            // create the find-files thread
         && (thrCreate(&tiFindFiles,
                       fntFindFiles,
                       &tidFindFiles,
                       "FindFiles",
                       THRF_WAIT_EXPLICIT,      // no PM msg queue!
                       (ULONG)&spt))
       )
    {
        TRY_LOUD(excpt1)
        {
            while (!arc)
            {
                // go block until find-files has set the buf ptr
                // _PmpfF(("blocking on hevBufPtrChanged"));
                if (!(arc = DosWaitEventSem(spt.hevBufPtrChanged,
                                            SEM_INDEFINITE_WAIT)))
                {
                    // _PmpfF(("blocking on hmtxBuffer"));
                    if (!(arc = DosRequestMutexSem(spt.hmtxBuffer,
                                                   SEM_INDEFINITE_WAIT)))
                    {
                        // OK, find-files released that sem:
                        // we either have data now or we're done
                        PFILEFINDBUF3L  pfb3;
                        ULONG           ulFindCount;
                        ULONG           ulPosted;

                        fBufSem = TRUE;

                        DosResetEventSem(spt.hevBufPtrChanged, &ulPosted);

                        // take the buffer pointer and the find count
                        pfb3 = spt.pfb3;
                        ulFindCount = spt.ulFindCount;

                        // tell find-files we've taken that buffer
                        DosPostEventSem(spt.hevBufTaken);
                        // release buffer mutex, on which
                        // find-files may have blocked
                        DosReleaseMutexSem(spt.hmtxBuffer);
                        fBufSem = FALSE;

                        if (pfb3)
                        {
                            // we have more data:
                            // run thru the buffer array
                            ULONG ul;
                            for (ul = 0;
                                 ul < ulFindCount;
                                 ul++)
                            {
                                // process this item
                                RefreshOrAwake(somSelf,     // folder
                                               pfb3);       // file
                                // _PmpfF(("done with RefreshOrAwake"));

                                // next item in buffer
                                if (pfb3->oNextEntryOffset)
                                    pfb3 = (PFILEFINDBUF3L)(   (PBYTE)pfb3
                                                            + pfb3->oNextEntryOffset
                                                           );
                            }

                            if (hwndReserved)
                            {
                                WinPostMsg(hwndReserved,
                                           0x0405,
                                           (MPARAM)-1,
                                           (MPARAM)pMyRecord);
                                // do this only once, or the folder
                                // chokes on the number of objects inserted
                                hwndReserved = NULLHANDLE;
                            }
                        }
                        else
                            // no more data, exit now!
                            break;
                    }
                }

                if (*pfExit)
                    arc = -1;

            } // end while (!arc)
        }
        CATCH(excpt1)
        {
            arc = ERROR_PROTECTION_VIOLATION;
        } END_CATCH();

        // tell find-files to exit too
        tiFindFiles.fExit = TRUE;

        // in case find-files is still blocked on this
        DosPostEventSem(spt.hevBufTaken);

        if (fBufSem)
            DosReleaseMutexSem(spt.hmtxBuffer);

        // wait for thread to terminate
        // before freeing the buffers!!
        while (tidFindFiles)
        {
            // _PmpfF(("tidFindFiles %lX is %d",
               //  &tidFindFiles,
                // tidFindFiles));
            DosSleep(0);
        }
    }

    fsysFreeFindBuffer(&spt.pBuf1);
    fsysFreeFindBuffer(&spt.pBuf2);

    if (spt.hevBufTaken)
        DosCloseEventSem(spt.hevBufTaken);
    if (spt.hevBufPtrChanged)
        DosCloseEventSem(spt.hevBufPtrChanged);
    if (spt.hmtxBuffer)
        DosCloseMutexSem(spt.hmtxBuffer);

    if (!arc)
        arc = spt.arcReturn;

    // return TRUE if no error
    return !arc;
}

/*
 *@@ fsysRefresh:
 *      implementation for our replacement XWPFileSystem::wpRefreshFSInfo.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 *@@changed V1.0.1 (2002-12-08) [umoeller]: added detection of .LONGNAME-changed case @@fixes 238
 *@@changed V1.0.2 (2004-01-08) [umoeller]: fixed infinite recursion during mozilla URL d&d @@fixes 531
 *@@changed V1.0.9 (2010-01-17) [pr]: added large file support @@fixes 586
 */

APIRET fsysRefresh(WPFileSystem *somSelf,
                   PVOID pvReserved)            // in: ptr to FILEFINDBUF3 or NULL
{
    APIRET          arc = NO_ERROR;
    PFILEFINDBUF3L  pfb3;
    PEAOP2          peaop = NULL;
    CHAR            szFilename[CCHMAXPATH];

    // V1.0.9 Ignore pvReserved which only points to a PFILEFINDBUF3 seemingly
    if (!_wpQueryFilename(somSelf, szFilename, TRUE))
        arc = ERROR_FILE_NOT_FOUND;
    else
        arc = fsysFillFindBuffer(szFilename, &pfb3, &peaop);

    if (!arc)
    {
        WPObject *pobjLock = NULL;

        TRY_LOUD(excpt1)
        {
            if (pobjLock = cmnLockObject(somSelf))
            {
                // XWPFileSystemData *somThis = XWPFileSystemGetData(somSelf);

                ULONG flState = _wpQueryState(somSelf); // V1.0.2 (2004-01-08) [umoeller]

                ULONG flRefresh = _wpQueryRefreshFlags(somSelf);

                PFEA2LIST pFEA2List2 = (PFEA2LIST)(   ((PBYTE)pfb3)
                                                    + FIELDOFFSET(FILEFINDBUF3L,
                                                                  cchName));

                PMINIRECORDCORE prec = _wpQueryCoreRecord(somSelf);
                HPOINTER hptrNew = NULLHANDLE;

                CHAR            szLongname[CCHMAXPATH];
                ULONG           ulTitleLen;
                PSZ             pszTitle;

                // fixing these flags is necessary to avoid endless recursion
                // V1.0.2 (2004-01-08) [umoeller]
                _wpSetState(somSelf,
                            (flState & ~STATEFL_INITIALIZED)
                                | STATEFL_REFRESHING);

                // the WPS clears this weird flag, whatever it is
                if (flRefresh & 0x20000000)
                    _wpSetRefreshFlags(somSelf, flRefresh & ~0x20000000);

                // check if the title has changed (.LONGNAME might
                // have been deleted or something)
                // V1.0.1 (2002-12-08) [umoeller]
                // for the title of the new object, use the real
                // name, unless we also find a .LONGNAME attribute,
                // so decode the EA buffer
                if (doshQueryLongname(pFEA2List2,
                                      szLongname,
                                      &ulTitleLen))
                    // got .LONGNAME:
                    pszTitle = szLongname;
                else
                {
                    // no .LONGNAME:
                    PUCHAR puchNameLen = (PUCHAR)(((PBYTE)pFEA2List2) + pFEA2List2->cbList);
                    pszTitle = ((PBYTE)puchNameLen) + sizeof(UCHAR);
                    ulTitleLen = *puchNameLen;
                }

                if (strcmp(pszTitle, _wpQueryTitle(somSelf)))
                    // whoa, .LONGNAME changed:
                    _xwpSetTitleOnly(somSelf, pszTitle);

                // set the instance variable for wpCnrRefreshDetails to
                // 0 so that we can count pending changes... see
                // XWPFileSystem::wpCnrRefreshDetails
                // _ulCnrRefresh = 0;

                // refresh various file-system info
                _wpSetAttr(somSelf, pfb3->attrFile);
                        // this calls _wpCnrRefreshDetails

                // this is funny: we can pass in pFEA2List2, but only
                // if we also set pszTypes to -1... so much for lucid APIs
                _wpSetType(somSelf,
                           (PSZ)-1,
                           pFEA2List2);
                        // this does not call _wpCnrRefreshDetails
                _wpSetDateInfo(somSelf, (PFILEFINDBUF4)pfb3);
                        // this does not call _wpCnrRefreshDetails
                _wpSetFileSizeInfo(somSelf,
                                   pfb3->cbFile.ulLo,         // file size
                                   pFEA2List2->cbList);       // EA size
                        // this calls _wpCnrRefreshDetails
                _wpSetFileSizeL(somSelf, &pfb3->cbFile);  // V1.0.9

                // refresh the icon... if prec->hptrIcon is still
                // NULLHANDLE, the WPS hasn't loaded the icon yet,
                // so there's no need to refresh
                if (prec->hptrIcon)
                {
                    // alright, we had an icon previously:
                    ULONG flNewStyle = 0;

                    // check if we have an .ICON EA... in that case,
                    // always set the icon data
                    if (!(arc = icoBuildPtrFromFEA2List(pFEA2List2,
                                                        &hptrNew,
                                                        NULL,
                                                        NULL)))
                    {
                        _wpSetIcon(somSelf, hptrNew);
                        flNewStyle = OBJSTYLE_NOTDEFAULTICON;
                    }

                    // if we didn't get an icon, do some default stuff
                    if (!hptrNew)
                    {
                        if (_somIsA(somSelf, _WPProgramFile))
                            // try to refresh the program icon,
                            // in case somebody just ran the resource
                            // compiler or something
                            _wpSetProgIcon(somSelf, NULL);
                        else if (    (ctsIsIcon(somSelf))
                                  || (ctsIsPointer(somSelf))
                                )
                        {
                            _wpQueryFilename(somSelf, szFilename, TRUE);
                            if (!icoLoadICOFile(szFilename,
                                                &hptrNew,
                                                NULL,
                                                NULL))
                            {
                                _wpSetIcon(somSelf, hptrNew);
                                flNewStyle = OBJSTYLE_NOTDEFAULTICON;
                            }
                        }
                        else if (_somIsA(somSelf, _WPDataFile))
                            // data file other than program file:
                            // set the association icon
                            _wpSetAssociatedFileIcon(somSelf);
                    }

                    _wpModifyStyle(somSelf,
                                   OBJSTYLE_NOTDEFAULTICON,
                                   flNewStyle);
                }

                // V1.0.2 (2004-01-08) [umoeller]
                _wpSetState(somSelf,
                            (flState & ~STATEFL_REFRESHING)
                                | STATEFL_INITIALIZED);

                _wpCnrRefreshDetails(somSelf);
            }
        }
        CATCH(excpt1)
        {
            arc = ERROR_PROTECTION_VIOLATION;
        } END_CATCH();

        if (pobjLock)
            _wpReleaseObjectMutexSem(pobjLock);
    }

    fsysFreeFindBuffer(&peaop);

    return arc;
}

