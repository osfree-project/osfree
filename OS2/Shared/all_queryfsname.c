/*!
   $Id: all_queryfsname.c,v 1.1.1.1 2003/10/04 08:36:18 prokushev Exp $

   @file all_queryfsname.c

   @brief utility functions for querying File System name for specified disk
   shared along all code

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

/* C standard library headers */
#include <malloc.h>
#include <string.h>

#include <all_shared.h> /*  shared functions and defines */

/*!
  Query file system name for specified disk

  @param szGivenDriveName    disk name (eg.: "d:")
  @param szFSName            buffer for file system name (must be allocated by
                             user, and be enough big)

  @return
        - 0 - if completed successully
        - rc from DosQueryFSAttach, when error
*/
#ifndef __386__
typedef struct _FSQBUFFER {
    USHORT iType;
    USHORT cbName;
    UCHAR  szName[1];
    USHORT cbFSDName;
    UCHAR  szFSDName[1];
    USHORT cbFSAData;
    UCHAR  rgFSAData[1];
} FSQBUFFER, *PFSQBUFFER;
#endif

APIRET all_QueryFSName(PSZ szGivenDriveName,PSZ szFSName)
{
  PSZ szDriveName="C:\0     "; /* drive name, must be 8 bytes long */
  APIRET rc;
#ifdef __386__
  ULONG cbBufSize=sizeof(FSQBUFFER2) + (3 * CCHMAXPATH);
  PBYTE pfsqBuffer=(PBYTE)calloc(cbBufSize,1);
#else
  USHORT cbBufSize=sizeof(FSQBUFFER) + (3 * CCHMAXPATH);
  PBYTE pfsqBuffer=(PBYTE)_fcalloc(cbBufSize,1);
#endif

  /* we're only interested in drive letter */
  szDriveName[0]=szGivenDriveName[0];

#ifdef __386__
  rc = DosQueryFSAttach(
              szDriveName,    /* Logical drive of attached FS      */
              0L,       /* ignored for FSAIL_QUERYNAME       */
              FSAIL_QUERYNAME, /* Return data for a Drive or Device */
              (PFSQBUFFER2) pfsqBuffer,       /* returned data                     */
              &cbBufSize);      /* returned data length              */
#else
  rc = DosQFSAttach(
              szDriveName,    /* Logical drive of attached FS      */
              0L,       /* ignored for FSAIL_QUERYNAME       */
              FSAIL_QUERYNAME, /* Return data for a Drive or Device */
              (PBYTE) pfsqBuffer,       /* returned data                     */
              &cbBufSize,      /* returned data length              */
              0);            /* reserved                           */
#endif

  if (rc!=NO_ERROR)
  {
#ifdef __386__
    free(pfsqBuffer);
#else
    _ffree(pfsqBuffer);
#endif
    return rc;
  };

  /* copy FSDName (located right after szName) to pszName, to return it */
#ifdef __386__
  strcpy(
    szFSName,
    ((PFSQBUFFER2)pfsqBuffer)->szName + ((PFSQBUFFER2)pfsqBuffer)->cbName + 1
     );
#else
  _fstrcpy(
    szFSName,
    ((PFSQBUFFER)pfsqBuffer)->szName + ((PFSQBUFFER)pfsqBuffer)->cbName + 1
     );
#endif

  /* free no-longer-needed buffer */
#ifdef __386__
    free(pfsqBuffer);
#else
    _ffree(pfsqBuffer);
#endif

  return NO_ERROR;
};
