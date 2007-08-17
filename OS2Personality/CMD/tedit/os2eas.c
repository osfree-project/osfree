/***********************************************************************/
/* OS2EAS.C - OS/2 Extended Attribute Preservation Routines            */
/***********************************************************************/
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 *
 *======================================================================
 * This code based heavily on sample code provided as part of the IBM 
 * OS/2 CSet/2 1.0 compiler.
 *======================================================================
 *
 */

static char RCSid[] = "$Id: os2eas.c,v 1.2 2001/12/18 08:23:27 mark Exp $";

#ifdef __EMX__                                /* EMX has its own routines, which   */
                                      /* work in both read and protected   */
                                      /* mode.                             */
#include <the.h>
#include <sys/ead.h>

static _ead CurrentFileEAs = NULL;

/****************************************************************\
 *                                                            *
 *  Name:    ReadEAs(filename)                                        *
 *                                                            *
 *  Purpose: Call EMX to Query a file's EA names and values.    *
 *                                                            *
 *  Usage:                                                    *
 *                                                            *
 *  Method : NOTE:  This routine does NOT prevent other       *
 *         processes from accessing the file's EAs while it is*
 *         reading them in, or while the program is editing   *
 *         them.                                              *
 *                                                            *
 *  Returns: 0          if it successfully reads in the EAs. Upon exit*
 *         , CurrentFileEAs holds the EAs for the current     *
 *         file. Any EAs previously saved will be destroyed.  *
 *                                                            *
\****************************************************************/
bool ReadEAs(CHARTYPE *filename)
{
   if (CurrentFileEAs != NULL)                /* should not happen                 */
   {
      _ead_destroy(CurrentFileEAs);
      CurrentFileEAs = NULL;
   }

   if ((CurrentFileEAs = _ead_create()) == NULL)
    return (1);
   if (_ead_read(CurrentFileEAs, filename, 0, 0) != 0)
   {
      _ead_destroy(CurrentFileEAs);
      CurrentFileEAs = NULL;
      return (1);
   }
   return(0);
}

/****************************************************************\
 *                                                            *
 *  Name:    WriteEAs(filename)                               *
 *                                                            *
 *  Purpose: Writes all EAs of CurrentFileEAs to the file     *
 *         and destroys the contents of CurrentFileEAs.       *
 *         CurrentFileEAs may be empty.                       *
 *         You should never nest ReadEAs und WriteEAs.        *
 *                                                            *
 *  Returns: 0          if successful,  1     otherwise               *
 *                                                            *
\****************************************************************/
bool WriteEAs(CHARTYPE *filename)
{
   bool retval;

   if (CurrentFileEAs == NULL)
      return (1);
   if (_ead_write(CurrentFileEAs, filename, 0, 0) < 0)
      retval = 1;
     else
      retval = 0;

  _ead_destroy(CurrentFileEAs);
  CurrentFileEAs = NULL;
  return (retval);
}
#else                                 /* not EMX                           */
#define INCL_DOSPROCESS     
#include <the.h>

/* The HoldFEA is used to hold individual EAs.  The member names correspond
   directly to those of the FEA structure.  Note however, that both szName
   and aValue are pointers to the values.  An additional field, next, is
   used to link the HoldFEA's together to form a linked list. */

struct _HoldFEA
{
 ULONG oNextEntryOffset;
 BYTE fEA;       
 BYTE cbName;
 USHORT cbValue;
 CHAR *szName;
 CHAR *aValue;
 struct _HoldFEA *next;
};
typedef struct _HoldFEA HOLDFEA;

static VOID  Free_FEAList(HOLDFEA *);
static VOID  GetMem(PVOID *pvMessage, ULONG ulSize);

/*--------------------------------------------------------------*\
 *  Other constants                                             *
\*--------------------------------------------------------------*/
#define MAX_GEA            500L  /* Max size for a GEA List                 */
#define Ref_ASCIIZ            1  /* Reference type for DosEnumAttribute     */

/* definition of level specifiers. required for File Info */

#define GetInfoLevel1         1            /* Get info from SFT             */
#define GetInfoLevel2         2            /* Get size of FEAlist           */
#define GetInfoLevel3         3            /* Get FEAlist given the GEAlist */
#define GetInfoLevel4         4            /* Get whole FEAlist             */
#define GetInfoLevel5         5            /* Get FSDname                   */

#define SetInfoLevel1         1            /* Set info in SFT               */
#define SetInfoLevel2         2            /* Set FEAlist                   */


/*--------------------------------------------------------------*\
 *  Global variables                                            *
\*--------------------------------------------------------------*/
CHAR    *pAlloc,*szEditBuf;
HOLDFEA *pHoldFEA;                         /* Global EA linked-list pointer */

/****************************************************************\
 *                                                              *
 *  Name:    ReadEAs(filename)                                  *
 *                                                              *
 *  Purpose: Call DOS to Query a file's EA names and values.    *
 *                                                              *
 *  Usage:                                                      *
 *                                                              *
 *  Method : Routine does NOT do full memory error trapping.    *
 *           NOTE:  This routine does NOT prevent other         *
 *           processes from accessing the file's EAs while it is*
 *           reading them in, or while the program is editing   *
 *           them.                                              *
 *                                                              *
 *  Returns: 0    if it successfully reads in the EAs. Upon exit*
 *           , globalp HoldFEA points to a linked list of the   *
 *           EAs for the current file.                          *
 *                                                              *
\****************************************************************/
bool ReadEAs(CHARTYPE *filename)
{
 CHAR *pAllocc=NULL; /* Holds the FEA struct returned by DosEnumAttribute */
                     /*  used to create the GEA2LIST for DosQueryPathInfo */
 CHAR *pBigAlloc=NULL; /* Temp buffer to hold each EA as it is read in    */
 USHORT cbBigAlloc=0;  /* Size of buffer                                  */

 ULONG ulEntryNum = 1; /* count of current EA to read (1-relative)        */
 ULONG ulEnumCnt;      /* Number of EAs for Enum to return, always 1      */

 HOLDFEA *pLastIn=0;    /* Points to last EA added, so new EA can link    */
 HOLDFEA *pNewFEA=NULL; /* Struct to build the new EA in                  */

 FEA2 *pFEA;           /* Used to read from Enum's return buffer          */
 GEA2LIST *pGEAList;/*Ptr used to set up buffer for DosQueryPathInfo call */
 EAOP2  eaopGet;       /* Used to call DosQueryPathInfo                   */
                                 /* Allocate enough room for any GEA List */
 GetMem((PPVOID)&pAllocc, MAX_GEA);
 pFEA = (FEA2 *) pAllocc;               /* pFEA always uses pAlloc buffer */

 pHoldFEA = '\0';            /* Reset the pointer for the EA linked list  */

 while(1)                /* Loop continues until there are no more EAs */
   {
    ulEnumCnt = 1;                /* Only want to get one EA at a time */
    if(DosEnumAttribute(Ref_ASCIIZ,         /* Read into pAlloc Buffer */
                        (CHAR *)filename,   /* Note that this does not */
                        ulEntryNum,         /* get the aValue field,   */
                        pAllocc,            /* so DosQueryPathInfo must*/
                        MAX_GEA,            /* be called to get it.    */
                        &ulEnumCnt,
                        (LONG) GetInfoLevel1))
      break;                           /* There was some sort of error */

    if(ulEnumCnt != 1)                   /* All the EAs have been read */
       break;

    ulEntryNum++;

    GetMem((PPVOID)&pNewFEA, sizeof(HOLDFEA));

    if (pNewFEA == '\0')                              /* Out of memory */
      {
       DosFreeMem(pAllocc);
       Free_FEAList(pHoldFEA);
       return (1);
      }

    pNewFEA->cbName = pFEA->cbName;   /* Fill in the HoldFEA structure */
    pNewFEA->cbValue= pFEA->cbValue;
    pNewFEA->fEA    = pFEA->fEA;
    pNewFEA->next = '\0';

    GetMem((PPVOID)&pNewFEA->szName, pFEA->cbName +1); /* Allocate for 2 arrays */
    GetMem((PPVOID)&pNewFEA->aValue, pFEA->cbValue);

    if (!pNewFEA->szName || !pNewFEA->aValue)          /* Out of memory */
      {
       if(pNewFEA->szName)
          DosFreeMem(pNewFEA->szName);
       if(pNewFEA->aValue)
          DosFreeMem(pNewFEA->aValue);

       DosFreeMem(pAllocc);
       DosFreeMem(pNewFEA);

       Free_FEAList(pHoldFEA);
       return (1);
      }
    strcpy(pNewFEA->szName,pFEA->szName);           /* Copy in EA Name */

    cbBigAlloc = sizeof(FEA2LIST) + pNewFEA->cbName+1 +
                            pNewFEA->cbValue;
    GetMem((PPVOID)&pBigAlloc, cbBigAlloc);
    if (pBigAlloc == '\0')
      {
       DosFreeMem(pNewFEA->szName);
       DosFreeMem(pNewFEA->aValue);
       DosFreeMem(pAllocc);
       DosFreeMem(pNewFEA);
       Free_FEAList(pHoldFEA);
       return (1);
      }

    pGEAList = (GEA2LIST *) pAllocc;       /* Set up GEAList structure */

    pGEAList->cbList = sizeof(GEA2LIST) + pNewFEA->cbName; /* +1 for NULL */
    pGEAList->list[0].oNextEntryOffset = 0L;
    pGEAList->list[0].cbName = pNewFEA->cbName;
    strcpy(pGEAList->list[0].szName,pNewFEA->szName);

    eaopGet.fpGEA2List = (GEA2LIST FAR *) pAllocc;
    eaopGet.fpFEA2List = (FEA2LIST FAR *) pBigAlloc;

    eaopGet.fpFEA2List->cbList = cbBigAlloc;

    DosQueryPathInfo((CHAR *)filename,     /* Get the complete EA info */
                     FIL_QUERYEASFROMLIST,
                    (PVOID) &eaopGet,
                     sizeof(EAOP2));
    memcpy(pNewFEA->aValue,               /* Copy the value to HoldFEA */
           pBigAlloc+sizeof(FEA2LIST)+pNewFEA->cbName,
           pNewFEA->cbValue);

    DosFreeMem(pBigAlloc);                /* Release the temp Enum buffer */

#if 0
    if(!CheckEAIntegrity(pNewFEA->aValue,pNewFEA->cbValue))  /* Bad EA */
      {
       FreeMem(pNewFEA->szName);
       FreeMem(pNewFEA->aValue);
       FreeMem(pNewFEA);
       continue;                   /* Don't add this EA to linked list */
      }
#endif

    if(pHoldFEA == '\0')                  /* If first EA, set pHoldFEA */
       pHoldFEA = pNewFEA;
    else                              /* Otherwise, add to end of list */
       pLastIn->next = pNewFEA;

    pLastIn = pNewFEA;                   /* Update the end of the list */
   }
 DosFreeMem(pAllocc);               /* Free up the GEA buf for DosEnum */
 return (0);
}

/****************************************************************\
 *                                                              *
 *  Name:    WriteEAs(filename)                                 *
 *                                                              *
 *  Purpose: This routine updates the EAs on disk to reflect    *
 *           their current condition in memory.  First, all EAs *
 *           in the delete list are removed from the disk, then *
 *           all EAs in the pHoldFEA list are written to disk.  *
 *  Usage  :                                                    *
 *  Method : This routine is not bulletproof as it does not get *
 *           exclusive access to the file EAs, nor does it      *
 *           handle out of disk space sort of errors. Also,     *
 *           memory fetches are not fully error trapped.        *
 *                                                              *
 *  Returns: 0    if successful,  1     otherwise               *
 *                                                              *
\****************************************************************/
bool WriteEAs(CHARTYPE *filename)
{
 HOLDFEA    *pHFEA=pHoldFEA;
 EAOP2      eaopWrite;
 CHAR       aBuf[MAX_GEA],*aPtr=NULL;
 FEA2       *pFEA = (FEA2 *) &aBuf[sizeof(ULONG)];
 USHORT     usMemNeeded=0, usRet=0;
 ULONG      *pulPtr=(ULONG *)aBuf;/* Initally points to top of FEALIST */

 eaopWrite.fpFEA2List=(FEA2LIST FAR *)aBuf; /* Setup fields that won't */
 pFEA->fEA     = 0;                         /* change for the delete   */
 pFEA->cbValue = 0;                         /* calls to DosSetPathInfo */

 while(pHFEA)                               /* Go through each HoldFEA */
   {
    usMemNeeded = sizeof(FEA2LIST) + pHFEA->cbName+1 + pHFEA->cbValue;
    GetMem((PPVOID)&aPtr, usMemNeeded);
    if (aPtr == NULL) /* GetMem failed to allocate */
      {
       Free_FEAList(pHoldFEA);     
       return(1);
      }

    eaopWrite.fpFEA2List = (FEA2LIST FAR *) aPtr;/* Fill in eaop struct */
    eaopWrite.fpFEA2List->cbList = usMemNeeded;

    eaopWrite.fpFEA2List->list[0].fEA     = pHFEA->fEA;
    eaopWrite.fpFEA2List->list[0].cbName  = pHFEA->cbName;
    eaopWrite.fpFEA2List->list[0].cbValue = pHFEA->cbValue;

    strcpy(eaopWrite.fpFEA2List->list[0].szName, pHFEA->szName);
    memcpy(eaopWrite.fpFEA2List->list[0].szName + pHFEA->cbName+1,
           pHFEA->aValue, pHFEA->cbValue);

    usRet = DosSetPathInfo((CHAR *)filename,       /* Write out the EA */
                           FIL_QUERYEASIZE,
                           (PVOID) &eaopWrite,
                           sizeof(EAOP2),
                           DSPI_WRTTHRU);
    if (usRet)
      {
       DosFreeMem(aPtr);                 /* Free up the FEALIST struct */
       Free_FEAList(pHoldFEA);     
       return(1);
      }
    DosFreeMem(aPtr);                    /* Free up the FEALIST struct */

    pHFEA = pHFEA->next;                           /* point to next EA */
   }
 Free_FEAList(pHoldFEA);     /* free up all memory associated with EAs */
 return(0);
}

/****************************************************************\
 *                                                              *
 *  Name:    Free_FEAList(pFEA)                                 *
 *                                                              *
 *  Purpose: This routine frees up the current list of EAs by   *
 *           deallocating the space used by the szName and      *
 *           aValue fields, then deallocating the HoldFEA struct*
 *           Next, it deletes the EAName space.                 *
 *  Usage :                                                     *
 *  Method:  Note that EAS always passes in pHoldFEA.           *
 *                    which is unnecessary since they are global*
 *           pointers;however, this is done to make the routine *
 *           more flexible by allowing multiple linked lists to *
 *           exist.                                             *
 *                                                              *
 *  Returns: VOID.  The two linked lists passed in are cleaned  *
 *           out though.                                        *
 *                                                              *
\****************************************************************/
static VOID Free_FEAList(HOLDFEA *pFEA)
{
 HOLDFEA *next;    /* Holds the next field since we free the structure */
                   /* before reading the current next field            */

 while(pFEA)
   {
    next = pFEA->next;
    if(pFEA->szName)                         /* Free if non-NULL name  */
       DosFreeMem(pFEA->szName);
    if(pFEA->aValue)                         /* Free if non-NULL value */
       DosFreeMem(pFEA->aValue);

    DosFreeMem(pFEA);                           /* Free HoldFEA struct */
    pFEA = next;
   }
 return;
}

/****************************************************************\
 *                                                              *
 *  Name:    GetMem (ppv, cb)                                   *
 *                                                              *
 *  Purpose: This routine returns a pointer to a available*     *
 *           memory space.                                      *
 *                                                              *
 *  Usage  :                                                    *
 *  Method : Routine should be bullet proof as it does its own  *
 *           error checking. It assumes that hwnd points to the *
 *           correct window with the name listbox in it.        *
 *                                                              *
 *  Returns: VOID, and allocated memory.                        *
 *                                                              *
\****************************************************************/
static VOID GetMem (PVOID *ppv, ULONG cb)
{
 BOOL f;

 f =(BOOL) DosAllocMem(ppv, cb, fPERM|PAG_COMMIT);
 if (f)
   {
    *ppv = NULL;
    return;
   }
 return;
}
#endif
