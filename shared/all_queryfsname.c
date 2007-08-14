/*!
   $Id: all_queryfsname.c,v 1.1.1.1 2003/10/04 08:36:18 prokushev Exp $ 
  
   @file all_queryfsname.c
 
   @brief utility functions for querying File System name for specified disk
   shared along all code

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <osfree.h>

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
APIRET all_QueryFSName(PSZ szGivenDriveName,PSZ szFSName)
{
  PSZ szDriveName="C:\0     "; /* drive name, must be 8 bytes long */
  APIRET rc;
  ULONG cbBufSize=sizeof(FSQBUFFER2) + (3 * CCHMAXPATH);
  BYTE *pfsqBuffer=(BYTE *)calloc(cbBufSize,1);

  /* we're only interested in drive letter */
  szDriveName[0]=szGivenDriveName[0];

  rc = DosQueryFSAttach(
              szDriveName,    /* Logical drive of attached FS      */
              0L,       /* ignored for FSAIL_QUERYNAME       */
              FSAIL_QUERYNAME, /* Return data for a Drive or Device */
              (PFSQBUFFER2) pfsqBuffer,       /* returned data                     */
              &cbBufSize);      /* returned data length              */

  if (rc!=NO_ERROR) 
  { 
    free(pfsqBuffer);
    return rc;
  };

  /* copy FSDName (located right after szName) to pszName, to return it */
  strcpy(
    szFSName, 
    ((PFSQBUFFER2)pfsqBuffer)->szName + ((PFSQBUFFER2)pfsqBuffer)->cbName + 1
     );            

  /* free no-longer-needed buffer */
  free(pfsqBuffer);

  return NO_ERROR;
};
