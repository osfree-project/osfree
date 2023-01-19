// EA.C - reads & writes ASCII EA values


#include "product.h"

#include <string.h>
#include <stdlib.h>

#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#include "4all.h"


#pragma pack (1)
typedef struct _STRUC_EAT_SV        // structure for EAT_ASCII
{
    USHORT usEAType;
    USHORT uscValue;
    CHAR   cValue[1];
} STRUC_EAT_SV;

typedef STRUC_EAT_SV *PSTRUC_EAT_SV;


#pragma pack ()

/*****************************************************************************
 * --- Internal function only ---
 * Create a Get-EA-List (GEAList)
 * The GEAList contains only 1 GEA-Entry
 * Entry:  pszName: Name of EA
 * return: Pointer to GEAList
 *****************************************************************************/
PGEA2LIST CreateGEAList( PCHAR pszName )
{
    int nLength;
    PGEA2LIST pGEAl;

    nLength = strlen( pszName );
    DosAllocMem ((PPVOID)&pGEAl, sizeof (GEA2LIST) + nLength, PAG_COMMIT | PAG_READ | PAG_WRITE);
    pGEAl->cbList = sizeof (GEA2LIST) + nLength;
    pGEAl->list->oNextEntryOffset = 0;        // last entry
    pGEAl->list->cbName = (CHAR)nLength;
    strcpy( pGEAl->list->szName, pszName );

    return pGEAl;
}


/*****************************************************************************
 * --- Internal function only ---
 * Create a Full-EA-List (FEAList)
 * The FEAList contains only 1 FEA-Entry
 * Entry:  pszName : Name of EA
 *         pValue  : Value of EA
 *         uscValue: Length of EA value
 * return: Pointer to FEAList
 *****************************************************************************/
PFEA2LIST CreateFEAList( PCHAR pszName, PBYTE pValue, USHORT uscValue )
{
    int nLength;
    PFEA2LIST pFEAl;

    nLength = strlen( pszName );
    DosAllocMem( (PPVOID)&pFEAl, sizeof (FEA2LIST) + nLength + uscValue, PAG_COMMIT | PAG_READ | PAG_WRITE );
    pFEAl->cbList = sizeof(FEA2LIST) + nLength + uscValue;
    pFEAl->list->oNextEntryOffset = 0;  // last entry
    pFEAl->list->fEA = 0;           // no flags
    pFEAl->list->cbName = (CHAR)nLength;
    pFEAl->list->cbValue = uscValue;
    strcpy( pFEAl->list->szName, pszName );
    memcpy( (PBYTE)pFEAl->list->szName + nLength + 1, pValue, uscValue );

    return pFEAl;
}


/*****************************************************************************
 * --- Internal function only ---
 * Create an EAOP-Structure with FEA-buffer at the end. This buffer may be used
 * for DosFind*, DosGetFileInfo or DosGetPathInfo-calls.
 * Entry:  ulcBuffer: Size of buffer (EAOP2 + FEAList)
 *         pGEAl:     Pointer to GEAList
 * return: Pointer to EAOP-structure
 *****************************************************************************/
PEAOP2 CreateEAOPRd( ULONG ulcBuffer, PGEA2LIST pGEAl )
{
    PEAOP2 pEAOP;

    DosAllocMem( (PPVOID)&pEAOP, ulcBuffer, PAG_COMMIT | PAG_READ | PAG_WRITE );
    pEAOP->fpGEA2List = pGEAl;
    pEAOP->fpFEA2List = (FEA2LIST *)(pEAOP + 1);
    pEAOP->fpFEA2List->cbList = ulcBuffer - sizeof(EAOP2);

    return pEAOP;
}


/*****************************************************************************
 * --- Internal function only ---
 * Create an EAOP-Structure with FEA-buffer at the end. This buffer may be
 *   used for DosSetFileInfo or DosSetPathInfo-calls.
 * Entry:  pFEAl:     Pointer to FEAList
 * return: Pointer to EAOP-structure
 *****************************************************************************/
PEAOP2 CreateEAOPWr( PFEA2LIST pFEAl )
{
    PEAOP2 pEAOP;

    DosAllocMem( (PPVOID)&pEAOP, sizeof (PEAOP2), PAG_COMMIT | PAG_READ | PAG_WRITE );
    pEAOP->fpGEA2List = NULL;
    pEAOP->fpFEA2List = pFEAl;

    return pEAOP;
}


/*****************************************************************************
 * Write EAT_ASCII-EAs (value of EA is ASCII-string).
 * Entry:  pszPathName: Filename
 *         pszEAName:   Name of EA
 *         pszString:   Value of EA (ASCIIZ-String)
 * return: TRUE:  EA was written
 *         FALSE: Error occured
 *****************************************************************************/
BOOL EAWriteASCII( PCHAR pszPathName, PCHAR pszEAName, PCHAR pszString )
{
    int nLength;
    BOOL          bRC;
    PSTRUC_EAT_SV peaASCII;
    PFEA2LIST     pFEAl;
    PEAOP2        pEAOP;

    // Fill EA structure
    nLength = strlen( pszString );
    DosAllocMem ((PPVOID)&peaASCII, sizeof (STRUC_EAT_SV) + nLength - 1, PAG_COMMIT | PAG_READ | PAG_WRITE );
    peaASCII->usEAType = EAT_ASCII;
    peaASCII->uscValue = (USHORT)nLength;
    memcpy( peaASCII->cValue, pszString, nLength );

    // Create FEA-list
    pFEAl = CreateFEAList( pszEAName, (PBYTE)peaASCII,
                          (nLength == 0) ? 0 : sizeof(STRUC_EAT_SV) + (USHORT)nLength - 1 );

    // Create EAOP-structure
    pEAOP = CreateEAOPWr( pFEAl );

    // Write EA
    bRC = ( DosSetPathInfo( pszPathName, FIL_QUERYEASIZE, pEAOP, sizeof (EAOP2), DSPI_WRTTHRU )) ? FALSE : TRUE;

    DosFreeMem( pEAOP );
    DosFreeMem( pFEAl );
    DosFreeMem( peaASCII );

    return bRC;
}


/*****************************************************************************
 * Read EAT_ASCII-EAs (value of EA is ASCII-string).
 * Entry:  pszPathName: Filename
 *         pszEAName:   Name of EA
 *         pcValue:     Size of buffer 'pszString'
 * Exit:   pszString:   Value of EA (ASCIIZ)
 *         pcValue:     Length of value of EA (strlen (pszString))
 * return: TRUE:  EA was read
 *         FALSE: Error occured
 *****************************************************************************/
BOOL EAReadASCII( PCHAR pszPathName, PCHAR pszEAName, PCHAR pszString, PINT pcValue )
{
    BOOL bRC;
    LONG lcBytes;
    FILESTATUS4 ffb4;
    PGEA2LIST pGEAl;
    PEAOP2 pEAOP;
    union _pEA {
        PFEA2   pFEA;
        PUSHORT pWord;
        PBYTE   pByte;
    } pEA;

    *pszString = '\0';
    bRC = FALSE;

    if ( DosQueryPathInfo( pszPathName, FIL_QUERYEASIZE, &ffb4, sizeof(FILESTATUS4)) == 0 ) {

        pGEAl = CreateGEAList( pszEAName );
        pEAOP = CreateEAOPRd( sizeof(EAOP2) + ffb4.cbList, pGEAl );
        if ( DosQueryPathInfo( pszPathName, FIL_QUERYEASFROMLIST, pEAOP, sizeof(EAOP2) ) == 0 ) {

            pEA.pFEA = pEAOP->fpFEA2List->list;
            if ( pEA.pFEA->cbValue != 0 ) {

                pEA.pByte = (PBYTE)&(pEA.pFEA->szName) + pEA.pFEA->cbName + 1;
                if ( *pEA.pWord++ == EAT_ASCII ) {

                    lcBytes = min( *pEA.pWord, (LONG)*pcValue - 1 );
                    if ( lcBytes > 0 ) {
                        memcpy( pszString, pEA.pWord + 1, lcBytes );
                        pszString[ lcBytes ] = '\0';
                    }

                    *pcValue = *pEA.pWord;
                    bRC = TRUE;
                }

            } else {
                // EA not present
                *pcValue = 0;
                bRC = TRUE;
            }
        }

        DosFreeMem( pEAOP );
        DosFreeMem( pGEAl );
    }

    // In case of an error return 0 as length of string
    if ( bRC == 0 )
        *pcValue = 0;

    return bRC;
}
