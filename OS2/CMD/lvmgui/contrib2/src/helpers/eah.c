
/*
 *@@sourcefile eah.c:
 *      contains helper functions for handling Extended Attributes.
 *      See explanations below.
 *
 *      Function prefixes (new with V0.81):
 *      --  ea*         EA helper functions
 *
 *      Usage: All OS/2 programs.
 *
 *      This file is new with V0.81 and contains all the EA functions
 *      that were in helpers.c previously.
 *
 *      Most of the functions in here come in pairs: one for working
 *      on EAs based on a path (eaPath*), one for open HFILEs (eaHFile*).
 *      Most functions return structures called EABINDING which should
 *      be freed again later.
 *
 *      <B>Example:</B>
 +          PEABINDING peab = eaPathReadOneByName("C:\Desktop", ".LONGNAME");
 +          if (peab)
 +              PSZ pszLongName = peab->pszValue;
 +          eaFreeBinding(peab);
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\eah.h"
 */

/*
 *      Most of the code in this file dealing with Extended Attributes is based
 *      on code (w) by Chris Hanson (cph@zurich.ai.mit.edu).
 *      Copyright (c) 1995 Massachusetts Institute of Technology.
 *
 *      The original code is available as EALIB.ZIP at Hobbes.
 *
 *      This material was developed by the Scheme project at the Massachusetts
 *      Institute of Technology, Department of Electrical Engineering and
 *      Computer Science.  Permission to copy this software, to redistribute
 *      it, and to use it for any purpose is granted, subject to the following
 *      restrictions and understandings.
 *
 *      1. Any copy made of this software must include this copyright notice
 *      in full.
 *
 *      2. Users of this software agree to make their best efforts (a) to
 *      return to the MIT Scheme project any improvements or extensions that
 *      they make, so that these may be included in future releases; and (b)
 *      to inform MIT of noteworthy uses of this software.
 *
 *      3. All materials developed as a consequence of the use of this
 *      software shall duly acknowledge such use, in accordance with the usual
 *      standards of acknowledging credit in academic research.
 *
 *      4. MIT has made no warrantee or representation that the operation of
 *      this software will be error-free, and MIT is under no obligation to
 *      provide any services, by way of maintenance, update, or otherwise.
 *
 *      5. In conjunction with products arising from the use of this material,
 *      there shall be no use of the name of the Massachusetts Institute of
 *      Technology nor of any adaptation thereof in any advertising,
 *      promotional, or sales literature without prior written consent from
 *      MIT in each case.
 *
 *      Copyright (C) 1995 Massachusetts Institute of Technology.
 *      Copyright (C) 1997-2002 Ulrich M”ller.
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

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/eah.h"

#pragma hdrstop

/*
 *@@category: Helpers\Control program helpers\Extended attributes
 *      See eah.c.
 */

/********************************************************************
 *
 *   Extended Attribute handling
 *
 ********************************************************************/

#define EA_BINDING_FLAGS(binding) ((binding)->bFlags)
#define EA_BINDING_NAME_LENGTH(binding) ((binding)->bNameLength)
#define EA_BINDING_VALUE_LENGTH(binding) ((binding)->usValueLength)
#define EA_BINDING_NAME(binding) ((binding)->pszName)
#define EA_BINDING_VALUE(binding) ((binding)->pszValue)

#define EA_LIST_BINDING(list) ((list)->peab)
#define EA_LIST_NEXT(list) ((list)->next)

// forward declarations to helper funcs at bottom
STATIC PEALIST      ReadEAList(ULONG, PVOID);
STATIC EABINDING *  ReadEAByIndex(ULONG, PVOID, ULONG);
STATIC EABINDING *  ReadEAByName(ULONG, PVOID, PSZ);
STATIC PDENA2       ReadDenaByIndex(ULONG, PVOID, ULONG);
STATIC PEABINDING   GetEAValue(ULONG, PVOID, PDENA2);
STATIC void         SetupQueryEAInfo(PDENA2, PEAOP2);
STATIC PEABINDING   ConvertFeal2Binding(PFEA2LIST);
STATIC APIRET       WriteEAList(ULONG, PVOID, PEALIST);
STATIC APIRET       WriteEA(ULONG, PVOID, PEABINDING);
STATIC PFEA2LIST    ConvertBinding2Feal(PEABINDING);

/*
 *@@ eaFreeBinding:
 *      deallocate EA binding memory that was generated
 *      by the ea...Read... procedures below. These procs
 *      assume that "malloc" was used for allocation and
 *      that the "pszName" and "pszValue" fields of each binding
 *      were also allocated using "malloc". "pszValue" may also
 *      be NULL.
 *
 *@@changed V0.9.0 [umoeller]: added check for (binding != NULL)
 */

void eaFreeBinding(PEABINDING peab)
{
    if (peab)
    {
        free(EA_BINDING_NAME(peab));
        if ((EA_BINDING_VALUE(peab)) != 0)
            free(EA_BINDING_VALUE(peab));
        free(peab);
    }
}

/*
 *@@ eaFreeList:
 *      like eaFreeBinding, but for an EA binding list.
 *      This calls eaFreeBinding for each list item.
 *
 *@@changed V0.9.0 [umoeller]: added check for (list != NULL)
 */

void eaFreeList(PEALIST list)
{
    if (list)
    {
        while (list != 0)
        {
            PEALIST next = (EA_LIST_NEXT (list));
            eaFreeBinding(EA_LIST_BINDING (list));
            free(list);
            list = next;
        }
    }
}

/* ******************************************************************
 *
 *   Read-EA functions
 *
 ********************************************************************/

/*
 * All of the following functions come in two flavors:
 *
 *      eaHFile*    operate on an open file handle.
 *
 *      eaPath*     operate on any file specified by its
 *                  filename, which may be fully qualified.
 */

/*
 *@@ eaPathQueryTotalSize:
 *      returns the total size of all EAs for a given file.
 *      This does not use the other EA functions, but
 *      accesses the EAs directly, so this is a lot quicker
 *      if you only want the total EA size without accessing
 *      the EA data.
 */

ULONG eaPathQueryTotalSize(const char *pcszPath)
{
    APIRET  arc;
    ULONG   ulTotalEASize = 0;
    FILEFINDBUF4   ffb4;

    if (!(arc = DosQueryPathInfo((PSZ)pcszPath,
                                 FIL_QUERYEASIZE,
                                 &ffb4,
                                 sizeof(FILEFINDBUF4))))
    {
        BYTE abBuf[2000];
        LONG lCount = 0;
        PDENA2 pdena2;

        lCount = -1;

        if (!(arc = DosEnumAttribute(ENUMEA_REFTYPE_PATH,
                                     (PSZ)pcszPath,
                                     1,
                                     abBuf,
                                     sizeof(abBuf),
                                     (PULONG)&lCount,
                                     ENUMEA_LEVEL_NO_VALUE)))
        {
            // ulCount now contains the EA count

            pdena2 = (PDENA2)abBuf;

            if (lCount > 0)
            {
                ulTotalEASize = pdena2->cbName + 8;

                while (lCount > 0)
                {
                    ulTotalEASize += (pdena2->cbValue + sizeof(DENA2));
                    lCount--;
                    pdena2 = (PDENA2) (((PBYTE) pdena2) +
                                  pdena2->oNextEntryOffset);

                }
            }
        }
    }

    return ulTotalEASize;
}

/*
 *@@ eaPathReadAll:
 *      reads all of the extended attributes into an EALIST.
 *      Returns NULL if no EAs were found.
 *      The returned list should be freed using eaFreeList.
 */

PEALIST eaPathReadAll(const char *pcszPath)
{
    return ReadEAList(ENUMEA_REFTYPE_PATH, (PSZ)pcszPath);
}

/*
 *@@ eaHFileReadAll:
 *      like eaPathReadAll, but for an open file handle.
 */

PEALIST eaHFileReadAll(HFILE hfile)
{
    return ReadEAList(ENUMEA_REFTYPE_FHANDLE, (&hfile));
}

/*
 *@@ eaPathReadOneByIndex:
 *      returns one EA specified by a given index, counting
 *      from 1. Returns NULL if the specified index was not
 *      found, either because the file has no EAs at all or
 *      the index is too large.
 *      The returned binding should be freed using eaFreeBinding.
 */

PEABINDING eaPathReadOneByIndex(const char *pcszPath, ULONG index)
{
    return ReadEAByIndex(ENUMEA_REFTYPE_PATH, (PSZ)pcszPath, index);
}

/*
 *@@ eaHFileReadOneByIndex:
 *      like eaPathReadOneByIndex, but for an open file handle.
 */

PEABINDING eaHFileReadOneByIndex(HFILE hfile, ULONG index)
{
    return ReadEAByIndex(ENUMEA_REFTYPE_FHANDLE, (&hfile), index);
}

/*
 *@@ eaPathReadOneByName:
 *      returns one EA specified by the given EA name (e.g.
 *      ".LONGNAME"). Returns NULL if not found.
 *      The returned binding should be freed using eaFreeBinding.
 */

PEABINDING eaPathReadOneByName(const char *pcszPath, const char *pcszEAName)
{
    return ReadEAByName(ENUMEA_REFTYPE_PATH, (PSZ)pcszPath, (PSZ)pcszEAName);
}

/*
 *@@ eaHFileReadOneByName:
 *      like eaPathReadOneByName, but for an open file handle.
 */

PEABINDING eaHFileReadOneByName(HFILE hfile, const char *pcszEAName)
{
    return ReadEAByName(ENUMEA_REFTYPE_FHANDLE, (&hfile), (PSZ)pcszEAName);
}

/* ******************************************************************
 *
 *   Write-EA functions
 *
 ********************************************************************/

/*
 *@@ eaPathWriteAll:
 *      writes a list of EAs to a given file. These EAs
 *      are added to possibly existing EAs on the file;
 *      existing EAs will be overwritten.
 *
 *      A given EA is deleted if its EABINDING.usValueLength
 *      field is 0; only in that case, the EABINDING.value
 *      field may also be NULL.
 *
 *@@changed V0.9.7 (2000-11-30) [umoeller]: now returning APIRET
 */

APIRET eaPathWriteAll(const char *pcszPath, PEALIST list)
{
    return WriteEAList(ENUMEA_REFTYPE_PATH, (PSZ)pcszPath, list);
}

/*
 *@@ eaHFileWriteAll:
 *      like eaPathWriteAll, but for an open file handle.
 *
 *@@changed V0.9.7 (2000-11-30) [umoeller]: now returning APIRET
 */

APIRET eaHFileWriteAll(HFILE hfile, PEALIST list)
{
    return WriteEAList(ENUMEA_REFTYPE_FHANDLE, (&hfile), list);
}

/*
 *@@ eaPathWriteOne:
 *      adds one EA to a given file. If the EA exists
 *      alredy, it is overwritten.
 *
 *      A given EA is deleted if its EABINDING.usValueLength
 *      field is 0; only in that case, the EABINDING.pszValue
 *      field may also be NULL.
 *
 *      To delete an EA, you may also use eaPathDeleteOne.
 *
 *@@changed V0.9.7 (2000-11-30) [umoeller]: now returning APIRET
 */

APIRET eaPathWriteOne(const char *pcszPath, PEABINDING peab)
{
    return WriteEA(ENUMEA_REFTYPE_PATH, (PSZ)pcszPath, peab);
}

/*
 *@@ eaHFileWriteOne:
 *      like eaPathWriteOne, but for an open file handle.
 *
 *@@changed V0.9.7 (2000-11-30) [umoeller]: now returning APIRET
 */

APIRET eaHFileWriteOne(HFILE hfile, PEABINDING peab)
{
    return WriteEA(ENUMEA_REFTYPE_FHANDLE, (&hfile), peab);
}

/*
 *@@ eaPathDeleteOne:
 *      this deletes one EA by constructing a temporary
 *      empty EABINDING for pszEAName and calling
 *      eaPathWriteOne.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.7 (2000-11-30) [umoeller]: now returning APIRET
 */

APIRET eaPathDeleteOne(const char *pcszPath, const char *pcszEAName)
{
    EABINDING eab;
    eab.bFlags = 0;
    eab.bNameLength = strlen(pcszEAName);
    eab.pszName = (PSZ)pcszEAName;
    eab.usValueLength = 0;
    eab.pszValue = 0;
    return eaPathWriteOne(pcszPath, &eab);
}

/*
 *@@category: Helpers\Control program helpers\Extended attributes\Translation helpers
 *      these functions are built on top of the regular EA helpers
 *      (see eah.c) to convert standard types to EAs.
 */

/********************************************************************
 *
 *   Translation funcs
 *
 ********************************************************************/

/*
 *@@ eaQueryEAType:
 *      this returns the type of the EA stored
 *      in the given EA binding.
 *      See CPREF for the EAT_* codes which are
 *      returned here.
 *      Returns 0 upon errors.
 *
 *@@added V0.9.0 [umoeller]
 */

USHORT eaQueryEAType(PEABINDING peab)
{
    USHORT usReturn = 0;
    if (peab)
        if (peab->pszValue)
            // first USHORT always has EAT_* flag
            usReturn = *((PUSHORT)(peab->pszValue));
    return usReturn;
}

/*
 *@@ eaCreatePSZFromBinding:
 *      this returns a new PSZ for the given EABINDING,
 *      if this is a string EA (EAT_ASCII).
 *      This PSZ should be free()'d after use.
 *      Otherwise, NULL is returned.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.2 (2000-03-12) [umoeller]: added checks for NULl strings
 */

PSZ eaCreatePSZFromBinding(PEABINDING peab)
{
    PSZ pszReturn = NULL;

    if (peab)
    {
        if (eaQueryEAType(peab) == EAT_ASCII)
        {
            // OK: next USHORT has length of string
            PUSHORT pusLength = (PUSHORT)(peab->pszValue + 2);
            if (*pusLength)
            {
                pszReturn = (PSZ)malloc(*pusLength + 1);
                memcpy(pszReturn, peab->pszValue + 4, *pusLength);
                // add null terminator
                *(pszReturn + (*pusLength)) = 0;
            }
        }
    }

    return pszReturn;
}

/*
 *@@ eaCreateBindingFromPSZ:
 *      reverse to eaCreatePSZFromBinding, this creates
 *      a new EABINDING from the given PSZ, which
 *      can be used with the write-EA functions.
 *      This EA is of type EAT_ASCII and will be
 *      made non-critical (peab->bFlags = 0).
 *
 *      If the given string is NULL or empty,
 *      a binding is created anyway; however, usValueLength
 *      and pszValue will be set to NULL so that the string
 *      can be deleted.
 *
 *      Returns NULL only upon malloc() errors.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (2000-01-22) [umoeller]: fixed null-string behavior
 *@@changed V0.9.2 (2000-02-26) [umoeller]: fixed null-string behaviour AGAIN
 */

PEABINDING eaCreateBindingFromPSZ(const char *pcszEAName, // in: EA name (e.g. ".LONGNAME")
                                  const char *pcszInput)  // in: string for EAT_ASCII EA
{
    PEABINDING peab = (PEABINDING)malloc(sizeof(EABINDING));
    if (peab)
    {
        SHORT cbString = 0;
        if (pcszInput)
            cbString = strlen(pcszInput);

        peab->bFlags = 0;
        peab->bNameLength = strlen(pcszEAName);
        peab->pszName = strdup(pcszEAName);

        if (cbString)
        {
            // non-null string:
            peab->usValueLength = cbString + 4;
            peab->pszValue = (PSZ)malloc(peab->usValueLength);
            if (peab->pszValue)
            {
                // set first USHORT to EAT_ASCII
                *((PUSHORT)(peab->pszValue)) = EAT_ASCII;
                // set second USHORT to length of string
                *((PUSHORT)(peab->pszValue + 2)) = cbString;
                // copy string to byte 4 (no null-terminator)
                memcpy(peab->pszValue + 4, pcszInput, cbString);
            }
            else
            {
                // malloc error:
                if (peab->pszName)
                    free(peab->pszName);
                free(peab);
                peab = NULL;
            }
        }
        else
        {
            // null string:
            peab->usValueLength = 0;
            peab->pszValue = NULL;
        }
    }

    return peab;
}

/*
 *@@ eaQueryMVCount:
 *      this returns the number of subitems in a
 *      multi-value EA. This works for both EAT_MVMT
 *      and EAT_MVST (multi-type and single-type) EAs.
 *      Returns 0 upon errors.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.2 (2000-03-12) [umoeller]: added more error checks
 */

USHORT eaQueryMVCount(PEABINDING peab,      // in: EA binding to examine (must be EAT_MVMT or EAT_MVST)
                      PUSHORT pusCodepage,  // out: codepage found in binding (ptr can be NULL)
                      PUSHORT pusEAType)    // out: either EAT_MVST or EAT_MVMT (ptr can be NULL)
{
    USHORT usReturn = 0;

    USHORT usEAType = eaQueryEAType(peab);

    if (pusEAType)
        *pusEAType = usEAType;

    if (usEAType == EAT_MVST)
    {
        // multi-value single-type:
        // pszValue is as follows (all USHORTs)
        // EAT_MVST usCodepage usCount usDataType data ....

        // store codepage
        if (pusCodepage)
            *pusCodepage = *((PUSHORT)(peab->pszValue + 2));
        // find count of entries
        usReturn = *((PUSHORT)(peab->pszValue + 4));
    } // end if (*((PUSHORT)(peab->pszValue)) == EAT_MVST)
    else if (usEAType == EAT_MVMT)
    {
        // multi-value multi-type:
        // pszValue is as follows (all USHORTs)
        // EAT_MVMT usCodepage usCount (usDataType data... )...

        // store codepage
        if (pusCodepage)
            *pusCodepage = *((PUSHORT)(peab->pszValue + 2));
        // find count of entries
        usReturn = *((PUSHORT)(peab->pszValue + 4));
    }

    return usReturn;
}

/*
 *@@ eaQueryMVItem:
 *      this returns a pointer to the beginning
 *      of data of a subitem in a multi-value
 *      (EAT_MVST or EAT_MVMT) EA.
 *
 *      Note that this _only_ works if the data
 *      stored in the multi-value fields has
 *      length-data right after the EAT_* fields.
 *      This is true for EAT_ASCII, for example.
 *
 *      <B>Example:</B> If the EA value in the
 *      binding is like this:
 +          EAT_MVMT  codp  count [DataType   Data] ...
 +          EAT_MVMT  0000  0002  EAT_ASCII   000A Hello John
 +                                EAT_BINARY  0003 0x12 0x21 0x34
 *      calling this function with usindex==1 would return
 *      a pointer to the 0x12 byte in the EA data, set *pusCodepage
 *      to 0, and set *pusDataLength to 3.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.2 (2000-03-12) [umoeller]: added more error checks
 */

PSZ eaQueryMVItem(PEABINDING peab,            // in: binding to examing
                  USHORT usIndex,             // in: item to search (starting at 0)
                  PUSHORT pusCodepage,        // out: codepage found in binding (ptr can be NULL)
                  PUSHORT pusEAType,          // out: EAT_* data type of data that the return value points to (ptr can be NULL)
                  PUSHORT pusDataLength)      // out: length of data that the return value points to (ptr can be NULL)
{
    PSZ pszReturn = NULL;

    USHORT usEAType = eaQueryEAType(peab);

    if (usEAType == EAT_MVST)
    {
        // multi-value single-type:
        // pszValue is as follows (all USHORTs)
        // EAT_MVST usCodepage usCount usDataType data ....
        USHORT usCount,
               usDataType;
        PUSHORT pusLengthThis;
        PSZ    pData;
        // store codepage
        if (pusCodepage)
            *pusCodepage = *((PUSHORT)(peab->pszValue + 2));
        // find count of entries
        usCount = *((PUSHORT)(peab->pszValue + 4));

        if (usIndex < usCount)
        {
            // find data type
            usDataType = *((PUSHORT)(peab->pszValue + 6));
            if (    (usDataType == EAT_ASCII)
                 || (usDataType == EAT_BINARY)
               )
            {
                USHORT us = 0;
                // find beginning of data (after length word)
                pData = peab->pszValue + 10;

                while (us < usIndex)
                {
                    pusLengthThis = (PUSHORT)(pData - 2);
                    pData += *pusLengthThis + 2;
                    us++;
                }

                // pData now points to the actual data;
                // pData - 2 has the length of the subvalue

                // return values
                if (pusEAType)
                    *pusEAType = usDataType; // same for all MVST subvalues
                if (pusDataLength)
                    *pusDataLength = *((PUSHORT)(pData - 2));
                pszReturn = pData;
            }
        }
    } // end if (usEAType == EAT_MVST)
    else if (usEAType == EAT_MVMT)
    {
        // multi-value multi-type:
        // pszValue is as follows (all USHORTs)
        // EAT_MVMT usCodepage usCount (usDataType data... )...
        USHORT usCount;
               // usDataType;
        PUSHORT pusLengthThis;
        PSZ    pData;
        // store codepage
        if (pusCodepage)
            *pusCodepage = *((PUSHORT)(peab->pszValue + 2));
        // find count of entries
        usCount = *((PUSHORT)(peab->pszValue + 4));

        if (usIndex < usCount)
        {
            USHORT us = 0;
            pData = peab->pszValue + 6;

            while (us < usIndex)
            {
                PUSHORT pusDataType = (PUSHORT)(pData);

                if (    (*pusDataType == EAT_ASCII)
                     || (*pusDataType == EAT_BINARY)
                   )
                {
                    pusLengthThis = (PUSHORT)(pData + 2);
                    pData += *pusLengthThis + 4;
                    us++;
                }
                else
                {
                    pData = 0;
                    break;
                }
            }

            // pData now points to the usDataType field
            // of the subvalue;
            // pData + 2 is the length of the subvalue;
            // pData + 4 is the actual data

            // return values
            if (pData)
            {
                if (pusEAType)
                    *pusEAType = *((PUSHORT)(pData));  // different for each MVMT item
                if (pusDataLength)
                    *pusDataLength = *((PUSHORT)(pData + 2));
                pszReturn = pData + 4;
            }
        }
    } // end if (usEAType == EAT_MVMT)

    return pszReturn;
}

/*
 *@@ eaCreatePSZFromMVBinding:
 *      this returns a new PSZ for the given EABINDING,
 *      if this is a multi-value, multi-type EA (EAT_MVMT),
 *      all of whose subitems are of EAT_ASCII though.
 *      Note that this does _not_ work for EAT_MVST EAs.
 *
 *      This format is used by the .COMMENTS and .KEYPHRASES
 *      system EAs, for example.
 *
 *      The different subitems will be separated by what
 *      you specify in pszSeparator. You can, for example,
 *      specify \n\r\0 to have CR-LF separators.
 *
 *      This PSZ should be free()'d after use.
 *      On errors, NULL is returned.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.2 (2000-03-12) [umoeller]: added more error checks
 */

PSZ eaCreatePSZFromMVBinding(PEABINDING peab,       // in: EAT_MVMT binding
                             const char *pcszSeparator,  // in: null-terminated string used as separator
                             PUSHORT pusCodepage)   // out: codepage found in binding (ptr can be NULL)
{
    PSZ     pszTotal = NULL;     // this will hold the whole string

    if (peab)
    {
        USHORT  usEAType = 0;

        USHORT usMVCount = eaQueryMVCount(peab,
                                          pusCodepage,
                                          &usEAType);
        if (    (usMVCount)
                    // items found?
             && (usEAType == EAT_MVMT)
                    // rule out EAT_MVST, which we cannot handle here
           )
        {
            // EAT_MVMT items found:
            // go thru all of them
            USHORT  us = 0;
            USHORT  cbComment = 0;
            USHORT  cbSeparator = strlen(pcszSeparator);
            while (us < usMVCount)
            {
                USHORT  usEATypeThis = 0;
                USHORT  usEALenThis = 0;
                PSZ pszSubItem = eaQueryMVItem(peab,
                                               us,    // index
                                               NULL,  // codepage
                                               &usEATypeThis,
                                               &usEALenThis);
                if (!pszSubItem)
                    break;

                if (usEATypeThis == EAT_ASCII)
                {
                    if (usEALenThis)
                    {
                        PSZ pszTemp = pszTotal;
                        PSZ pTarget;
                        cbComment += usEALenThis + cbSeparator;
                        pszTotal = (PSZ)malloc(cbComment);
                        if (pszTemp)
                        {
                            // not first loop: append EA value
                            USHORT cbCommentOld = strlen(pszTemp);
                            // copy previous compiled string
                            strcpy(pszTotal, pszTemp);
                            // append separator
                            memcpy(pszTotal + cbCommentOld,
                                   pcszSeparator,
                                   cbSeparator);
                            // copy the rest after the separator (below)
                            pTarget = pszTotal + cbCommentOld + cbSeparator;
                            free(pszTemp);
                        }
                        else
                            // first loop: copy to beginning
                            pTarget = pszTotal;

                        // copy EA value
                        memcpy(pTarget, pszSubItem, usEALenThis);
                        // append null terminator
                        *(pTarget + usEALenThis) = 0;
                    }
                } // if (usEATypeThis == EAT_ASCII)
                us++;
            } // end while (us < usMVCount)
        } // end if (usMVCount)
    }

    return pszTotal;
}

/*
 *@@ eaCreateMVBindingFromPSZ:
 *      reverse to eaCreatePSZFromMVBinding, this
 *      parses pszInput and creates an EAT_MVMT
 *      EABINDING, all of whose subvalues are of
 *      EAT_ASCII. The EABINDING is non-critical
 *      (peab->bFlags = 0).
 *
 *      pszSeparator is used in the analysis of pszInput
 *      as the separator string (e.g. "\r\n").
 *
 *      Returns NULL if the given string is NULL.
 *
 *      <B>Example:</B>
 +          PSZ pszInput = "Line 1\r\nLine2\r\nLine3\r\n";
 +          PEABINDING peab = eaCreateMVBindingFromPSZ(".KEYPHRASES",
 +                                                     pszInput,
 +                                                     "\r\n",
 +                                                     0);      // codepage
 *      will create an EABINDING with the following pszValue:
 +          EAT_MVMT codp usCount (usDataType usLen data...)
 +          EAT_MVMT   0     3    EAT_ASCII     6   Line 1
 +                                EAT_ASCII     5   Line2
 +                                EAT_ASCII     5   Line3
 *
 *@@added V0.9.0 [umoeller]
 */

PEABINDING eaCreateMVBindingFromPSZ(const char *pcszEAName,      // in: EA name (e.g. ".KEYPHRASES")
                                    const char *pcszInput,       // in: string to parse
                                    const char *pcszSeparator,   // in: separator used in pszInput
                                    USHORT usCodepage)  // in: codepage to set in EAT_MVMT
{
    PEABINDING peab = NULL;
    if (pcszInput)
    {
        if ((peab = (PEABINDING)malloc(sizeof(EABINDING))))
        {
            const char *p = pcszInput,
                    *pSource;
            PSZ     pTarget;
            USHORT  cbInput = strlen(pcszInput),
                    cbSep = strlen(pcszSeparator),
                    usSepCount = 0,
                    cbToAlloc = 0,
                    cbThis,
                    us;

            peab->bFlags = 0;
            peab->bNameLength = strlen(pcszEAName);
            peab->pszName = strdup(pcszEAName);

            // now count the number of pszSeparators in pszInput
            while ((p = strstr(p, pcszSeparator)))
            {
                usSepCount++;
                p += cbSep;
            }
            // usSepCount now has the separator count; that means
            // we have (usSepCount + 1) data fields

            // allocate the memory we need for the total EA:
            // EAT_MVMT usCodepage usCount (usDataType data... )...

            cbToAlloc =     6       // six fixed bytes for (EAT_MVMT usCodepage usCount)
                         +  cbInput - (usSepCount * cbSep)
                                    // length of input string minus length of all separators
                         +  ((usSepCount + 1) * 4);
                                    // for each data field (of which we'll have (usSepCount + 1)),
                                    // we need an extra four bytes for EAT_ASCII and the length
            peab->usValueLength = cbToAlloc;
            peab->pszValue = (PSZ)malloc(cbToAlloc + 1);

            // now initialize the first three fields:
            *((PUSHORT)(peab->pszValue)) = EAT_MVMT;
            *((PUSHORT)(peab->pszValue + 2)) = usCodepage;
            *((PUSHORT)(peab->pszValue + 4)) = (usSepCount + 1);

            // set pointer to first field
            pSource = pcszInput;
            pTarget = peab->pszValue + 6;

            // now go thru all fields except the last
            for (us = 0;
                 us < usSepCount;       // exclude the last field
                 us++)
            {
                // find the next separator
                PSZ     pNextSep = strstr(pSource, pcszSeparator);
                // calculate the length of the substring
                cbThis = pNextSep - pSource;
                // set data type in field
                *((PUSHORT)(pTarget)) = EAT_ASCII;
                // set length of this field
                *((PUSHORT)(pTarget + 2)) = cbThis;
                // copy data
                memcpy(pTarget + 4, pSource, cbThis);
                // advance source to the next (after substring and separator)
                pSource += cbThis + cbSep;
                // advance target to the next (after substring and (EAT_ASCII usLen)
                pTarget += cbThis + 4;
            }

            // now handle the last field
            cbThis = strlen(pSource);
            *((PUSHORT)(pTarget)) = EAT_ASCII;
            *((PUSHORT)(pTarget + 2)) = cbThis;
            memcpy(pTarget + 4, pSource, cbThis);
        } // end if (peab)
    } // end if (pszInput)

    return peab;
}

/********************************************************************
 *
 *   EA helper funcs
 *
 ********************************************************************/

/*
 *  These get called by the functions above.
 *
 */

/*
 *@@ ReadEAList:
 *
 */

STATIC PEALIST ReadEAList(ULONG type, // in: ENUMEA_REFTYPE_FHANDLE or  ENUMEA_REFTYPE_PATH
                          PVOID pfile)
{
    ULONG index = 1;
    PEALIST head = 0;
    PEALIST tail = 0;

    while (1)
    {
        PEABINDING binding = (ReadEAByIndex(type, pfile, index));
        if (binding == 0)
            break;
        {
            PEALIST list = (PEALIST)malloc(sizeof(EALIST));
            (EA_LIST_BINDING (list)) = binding;
            (EA_LIST_NEXT (list)) = 0;
            if (head == 0)
                head = list;
            else
                (EA_LIST_NEXT (tail)) = list;
            tail = list;
        }
        index += 1;
    }
    return head;
}

/*
 *@@ ReadEAByIndex:
 *
 */

STATIC PEABINDING ReadEAByIndex(ULONG type, // in: ENUMEA_REFTYPE_FHANDLE or  ENUMEA_REFTYPE_PATH
                                PVOID pfile,
                                ULONG index)
{
    PDENA2 dena = (ReadDenaByIndex(type, pfile, index));
    return ((dena == 0)
                    ? 0
                    : (GetEAValue(type, pfile, dena)));
}

/*
 *@@ ReadEAByName:
 *
 */

STATIC PEABINDING ReadEAByName(ULONG type, // in: ENUMEA_REFTYPE_FHANDLE or  ENUMEA_REFTYPE_PATH
                               PVOID pfile,
                               PSZ name)
{
    ULONG index = 1;
    while (1)
    {
        PDENA2 dena = ReadDenaByIndex(type, pfile, index);
        if (dena == 0)
            return NULL;
        if ((strcmp(name, (dena->szName))) == 0)
            return GetEAValue(type, pfile, dena);
        free(dena);
        index += 1;
    }
}

/*
 *@@ ReadDenaByIndex:
 *      fills a DENA2 structure by calling DosEnumAttribute.
 *
 *      DENA2 is typedef'd to FEA2 in the OS/2 headers.
 *      FEA2 defines the format for setting the full extended
 *      attributes in the file.
 *
 +         typedef struct _FEA2 {
 +           ULONG      oNextEntryOffset;  //  Offset to next entry.
 +           BYTE       fEA;               //  Extended attributes flag.
 +           BYTE       cbName;            //  Length of szName, not including NULL.
 +           USHORT     cbValue;           //  Value length.
 +           CHAR       szName[1];         //  Extended attribute name.
 +         } FEA2;
 +         typedef FEA2 *PFEA2;
 *
 *      Returns 0 on errors.
 *
 *@@changed V0.9.2 (2000-03-12) [umoeller]: added more error checking
 *@@changed V0.9.4 (2000-08-03) [umoeller]: added even more error checking; this fixed problems with swapper.dat and such
 */

STATIC PDENA2 ReadDenaByIndex(ULONG type, // in: ENUMEA_REFTYPE_FHANDLE or  ENUMEA_REFTYPE_PATH
                              PVOID pfile, // in: file handle or name
                              ULONG index) // in: EA index (>= 1)
{
    ULONG count = 1;
    PDENA2 dena = (PDENA2)malloc(500); // 500 is magic -- IBM doesn't explain.
    APIRET arc = DosEnumAttribute(type,
                                  pfile,
                                  index,
                                  dena,
                                  500,
                                  (&count),
                                  ENUMEA_LEVEL_NO_VALUE);
    if (   (arc != NO_ERROR)
        || (count == 0)     // check error code?
       )
    {
        free(dena);
        return 0;
    }

    return dena;
}

/*
 *@@ GetEAValue:
 *
 *@@changed V0.9.1 (2000-01-30) [umoeller]: now returning NULL upon errors
 */

STATIC PEABINDING GetEAValue(ULONG type,  // in: ENUMEA_REFTYPE_FHANDLE or  ENUMEA_REFTYPE_PATH
                             PVOID pfile, // in: file handle or path
                             PDENA2 dena) // in: buffer allocated by ReadDenaByIndex
{
    ULONG level = FIL_QUERYEASFROMLIST;
    EAOP2 eaop;
    ULONG size = (sizeof(eaop));
    APIRET arc = NO_ERROR;
    SetupQueryEAInfo(dena, (&eaop));
            // this freeds dena
    if (type == ENUMEA_REFTYPE_FHANDLE)
        arc = DosQueryFileInfo((* ((PHFILE) pfile)), level, (&eaop), size);
    else
        arc = DosQueryPathInfo((PSZ)pfile, level, (&eaop), size);
                    // changed V0.9.0 (99-11-28) [umoeller]; this now works with C
    free(eaop.fpGEA2List);
    if (arc == NO_ERROR)
        return ConvertFeal2Binding(eaop.fpFEA2List);

    return NULL;
}

/*
 *@@ SetupQueryEAInfo:
 *      prepares a EAOP2 structure for querying EAs.
 *      This also frees dena, since it's no longer used.
 */

STATIC void SetupQueryEAInfo(PDENA2 dena,  // in: buffer allocated by ReadDenaByIndex
                             PEAOP2 eaop)  // out: EAOP2 to fill
{
    unsigned int geal_size = ((sizeof (GEA2LIST)) + (dena->cbName));
    unsigned int feal_size
                 = ((sizeof (FEA2LIST)) + (dena->cbName) + (dena->cbValue));
    (eaop->fpGEA2List) = (GEA2LIST*)(malloc(geal_size));
    ((eaop->fpGEA2List)->cbList) = geal_size;
    (eaop->fpFEA2List) = (FEA2LIST*)(malloc(feal_size));
    ((eaop->fpFEA2List)->cbList) = feal_size;
    (eaop->oError) = 0;
    {
        PGEA2 gea = (&(((eaop->fpGEA2List)->list) [0]));
        (gea->oNextEntryOffset) = 0;
        (gea->cbName) = (dena->cbName);
        strcpy ((gea->szName), (dena->szName));
    }
    free(dena);
}

/*
 *@@ ConvertFeal2Binding:
 *      creates a new EABINDING from the given FEA2LIST
 *      and frees feal. Returns NULL upon errors.
 *
 *@@changed V0.9.1 (2000-01-30) [umoeller]: now returning NULL upon errors
 */

STATIC PEABINDING ConvertFeal2Binding(PFEA2LIST feal)
{
    PFEA2 fea = (&((feal->list) [0]));
    PEABINDING binding = (PEABINDING)(malloc(sizeof (EABINDING)));
    if (binding)
    {
        (EA_BINDING_FLAGS (binding)) = (fea->fEA);
        (EA_BINDING_NAME_LENGTH (binding)) = (fea->cbName);
        (EA_BINDING_VALUE_LENGTH (binding)) = (fea->cbValue);
        (EA_BINDING_NAME (binding)) = (PSZ)(malloc((fea->cbName) + 1));
        strcpy ((EA_BINDING_NAME (binding)), (fea->szName));
        (EA_BINDING_VALUE (binding)) = (PSZ)(malloc(fea->cbValue));
        memcpy ((EA_BINDING_VALUE (binding)),
                (&((fea->szName) [(fea->cbName) + 1])),
                (fea->cbValue));
        free(feal);
    }

    return binding;
}

/*
 *@@ WriteEAList:
 *
 *@@changed V0.9.1 (2000-01-30) [umoeller]: now returning APIRET
 */

STATIC APIRET WriteEAList(ULONG type, // in: ENUMEA_REFTYPE_FHANDLE or  ENUMEA_REFTYPE_PATH
                          PVOID pfile,
                          PEALIST list)
{
    APIRET arc = NO_ERROR;
    while (    (list != 0)
            && (arc == NO_ERROR)
          )
    {
        arc = WriteEA(type, pfile, (EA_LIST_BINDING (list)));
        list = (EA_LIST_NEXT (list));
    }

    return arc;
}

/*
 *@@ WriteEA:
 *
 *@@changed V0.9.1 (2000-01-30) [umoeller]: now returning APIRET
 */

STATIC APIRET WriteEA(ULONG type,          // in: ENUMEA_REFTYPE_FHANDLE or  ENUMEA_REFTYPE_PATH
                      PVOID pfile,
                      PEABINDING binding)
{
    ULONG level = FIL_QUERYEASIZE;
    EAOP2 eaop;
    ULONG size = (sizeof (eaop));
    APIRET arc = NO_ERROR;

    (eaop.fpGEA2List) = 0;
    (eaop.fpFEA2List) = (ConvertBinding2Feal(binding));
    (eaop.oError) = 0;
    if (type == ENUMEA_REFTYPE_FHANDLE)
        arc = DosSetFileInfo((* ((PHFILE) pfile)), level, (&eaop), size);
    else
        arc = DosSetPathInfo((PSZ)pfile, level, (&eaop), size, DSPI_WRTTHRU);
    free(eaop.fpFEA2List);
    return arc;
}

/*
 *@@ ConvertBinding2Feal:
 *
 */

STATIC PFEA2LIST ConvertBinding2Feal(PEABINDING binding)
{
    unsigned int feal_size
              = ((sizeof (FEA2LIST))
                 + (EA_BINDING_NAME_LENGTH (binding))
                 + (EA_BINDING_VALUE_LENGTH (binding)));
    PFEA2LIST feal = (PFEA2LIST)(malloc(feal_size));
    PFEA2 fea = (&((feal->list) [0]));
    (feal->cbList) = feal_size;
    (fea->oNextEntryOffset) = 0;
    (fea->fEA) = (EA_BINDING_FLAGS (binding));
    (fea->cbName) = (EA_BINDING_NAME_LENGTH (binding));
    (fea->cbValue) = (EA_BINDING_VALUE_LENGTH (binding));
    strcpy ((fea->szName), (EA_BINDING_NAME (binding)));
    if ((EA_BINDING_VALUE (binding)) != 0)
        memcpy ((&((fea->szName) [(fea->cbName) + 1])),
                (EA_BINDING_VALUE (binding)),
                (fea->cbValue));
    return feal;
}

/* ******************************************************************
 *
 *   Direct plain-string EA handling
 *
 ********************************************************************/

#define USE_EAMVMT 1

/*
 * eahWriteStringEA:
 *
 *      (C) Christian Langanke. Moved this here from tmsgfile.c.
 *
 *@@added V0.9.3 (2000-05-21) [umoeller]
 */

APIRET eahWriteStringEA(HFILE hfile,
                        PSZ pszEaName,
                        PSZ pszEaValue)
{

    APIRET          rc = NO_ERROR;
    PFEA2LIST       pfea2l = NULL;

#ifdef USE_EAMVMT
    PEAMVMT         peamvmt;
#else
    PEASVST         peasvst;
#endif
    ULONG           ulEAListLen;
    ULONG           ulValueLen;
    EAOP2           eaop2;

    // PSZ             pszValue;

    do
    {
        // check parameters
        if (    (hfile == NULLHANDLE)
             || (pszEaName == NULL)
             || (*pszEaName == 0)
             || (pszEaValue == NULL)
           )
        {
            rc = ERROR_INVALID_PARAMETER;
            break;
        }

        // write EAs
        ulValueLen = strlen(pszEaValue) +
#ifdef USE_EAMVMT
            sizeof(EAMVMT);
#else
            sizeof(EASVST);
#endif

        ulEAListLen = strlen(pszEaName) +
                      sizeof(FEA2LIST) +
                      ulValueLen;

        // get memory for FEA2LIST
        pfea2l = (FEA2LIST*)malloc(ulEAListLen);
        if (pfea2l == 0)
        {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        // init FEA2LIST
        eaop2.fpGEA2List = NULL;
        eaop2.fpFEA2List = pfea2l;
        memset(pfea2l, 0, ulEAListLen);

        // write timeframe EA
        pfea2l->cbList = ulEAListLen;
        pfea2l->list[0].cbName = strlen(pszEaName);
        strcpy(pfea2l->list[0].szName, pszEaName);

        // delete attribute if value empty
        if (strlen(pszEaValue) == 0)
            pfea2l->list[0].cbValue = 0;
        else
        {
            pfea2l->list[0].cbValue = ulValueLen;

#ifdef USE_EAMVMT
            // multi value multi type
            peamvmt = (PEAMVMT) NEXTSTR(pfea2l->list[0].szName);
            peamvmt->usType = EAT_MVMT;
            peamvmt->usCodepage = 0;
            peamvmt->usEntries = 1;
            peamvmt->usEntryType = EAT_ASCII;
            peamvmt->usEntryLen = strlen(pszEaValue);
            memcpy(&peamvmt->chEntry[0], pszEaValue, peamvmt->usEntryLen);
#else
            // single value single type
            peasvst = NEXTSTR(pfea2l->list[0].szName);
            peasvst->usType = EAT_ASCII;
            peasvst->usEntryLen = strlen(pszEaValue);
            memcpy(&peasvst->chEntry[0], pszEaValue, peasvst->usEntryLen);
#endif
        }

        // set the new EA value
        rc = DosSetFileInfo(hfile,
                            FIL_QUERYEASIZE,
                            &eaop2,
                            sizeof(eaop2));

    }
    while (FALSE);

// cleanup
    if (pfea2l)
        free(pfea2l);
    return rc;
}

/*
 * eahReadStringEA:
 *
 *      (C) Christian Langanke. Moved this here from tmsgfile.c.
 *
 *@@added V0.9.3 (2000-05-21) [umoeller]
 */

APIRET eahReadStringEA(PSZ pszFileName,
                       PSZ pszEaName,
                       PSZ pszBuffer,
                       PULONG pulBuflen)
{

    APIRET          rc = NO_ERROR;
    FILESTATUS4     fs4;

    EAOP2           eaop2;
    PGEA2LIST       pgea2l = NULL;
    PFEA2LIST       pfea2l = NULL;

    PGEA2           pgea2;
    PFEA2           pfea2;

    ULONG           ulGea2Len = 0;
    ULONG           ulFea2Len = 0;

    PEASVST         peasvst;
    PEAMVMT         peamvmt;

    ULONG           ulRequiredLen;

    do
    {
        // check parameters
        if ((pszFileName == NULL) ||
            (pszEaName == NULL) ||
            (*pszEaName == 0) ||
            (pulBuflen == NULL))
        {
            rc = ERROR_INVALID_PARAMETER;
            break;
        }

        // initialize target buffer
        if (pszBuffer)
            memset(pszBuffer, 0, *pulBuflen);

        // get EA size
        rc = DosQueryPathInfo(pszFileName,
                              FIL_QUERYEASIZE,
                              &fs4,
                              sizeof(fs4));
        if (rc != NO_ERROR)
            break;

        // no eas here ?
        if (fs4.cbList == 0)
        {
            pulBuflen = 0;
            break;
        }

        // determine required space
        // - for ulFea2Len use at least 2 * Gea2Len because
        //   buffer needs at least to be Geal2Len even for an empty
        //   attribute, otherwise rc == ERROR_BUFFER_OVERFLOW !
        ulGea2Len = sizeof(GEA2LIST) + strlen(pszEaName);
        ulFea2Len = 2 * MAX(fs4.cbList, ulGea2Len);

        // get memory for GEA2LIST
        if ((pgea2l = (GEA2LIST*)malloc(ulGea2Len)) == 0)
        {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        memset(pgea2l, 0, ulGea2Len);

        // get memory for FEA2LIST
        if ((pfea2l = (FEA2LIST*)malloc(ulFea2Len)) == 0)
        {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        memset(pfea2l, 0, ulFea2Len);

        // init ptrs and do the query
        memset(&eaop2, 0, sizeof(EAOP2));
        eaop2.fpGEA2List = pgea2l;
        eaop2.fpFEA2List = pfea2l;
        pfea2l->cbList = ulFea2Len;
        pgea2l->cbList = ulGea2Len;

        pgea2 = &pgea2l->list[0];
        pfea2 = &pfea2l->list[0];


        pgea2->oNextEntryOffset = 0;
        pgea2->cbName = strlen(pszEaName);
        strcpy(pgea2->szName, pszEaName);

        rc = DosQueryPathInfo(pszFileName,
                              FIL_QUERYEASFROMLIST,
                              &eaop2,
                              sizeof(eaop2));
        if (rc != NO_ERROR)
            break;

        // check first entry only
        peamvmt = (PEAMVMT) ((PBYTE) pfea2->szName + pfea2->cbName + 1);

        // is it MVMT ? then adress single EA !
        if (peamvmt->usType == EAT_MVMT)
        {
            peasvst = (PEASVST) & peamvmt->usEntryType;
        }
        else
            peasvst = (PEASVST) peamvmt;


        // is entry empty ?
        if (peasvst->usEntryLen == 0)
        {
            rc = ERROR_INVALID_EA_NAME;
            break;
        }

        // is it ASCII ?
        if (peasvst->usType != EAT_ASCII)
        {
            rc = ERROR_INVALID_DATA;
            break;
        }

        // check buffer and hand over value
        ulRequiredLen = peasvst->usEntryLen + 1;
        if (*pulBuflen < ulRequiredLen)
        {
            *pulBuflen = ulRequiredLen;
            rc = ERROR_BUFFER_OVERFLOW;
            break;
        }

        // hand over len
        *pulBuflen = ulRequiredLen;

        // hand over value
        if (pszBuffer)
            memcpy(pszBuffer, peasvst->chEntry, peasvst->usEntryLen);

    }
    while (FALSE);

// cleanup
    if (pgea2l)
        free(pgea2l);
    if (pfea2l)
        free(pfea2l);
    return rc;
}


