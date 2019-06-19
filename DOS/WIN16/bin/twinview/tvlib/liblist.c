/***************************************************************************************
**
**
**                              Library List Functions
**
**
***************************************************************************************/

/*************************************
*
*  System Includes
*
**************************************/
#include <windows.h>


/*************************************
*
*  Custom Includes
*
**************************************/
#include "tvlibprv.h"


/*************************************
*
*  Global Variables
*
**************************************/
HTVLIBLIST  hTVLibList = 0;


/*************************************
*
*  Create a library list.
*
**************************************/
BOOL __far __pascal __export CreateTVLibList
( 
  char __far * szLibWildName,        /* File specification for a library file. */
  BOOL Yield                         /* Yeild while looking for library files. */
)
{
  FPTVLIBLIST fpTVLibList;          /* Pointer to the allocated library list. */

  if( ! GlobalAllocMem( NULL, &hTVLibList, sizeof(TVLIBLIST) ) ) return FALSE;
  
  /* Lock it. */
  fpTVLibList = (FPTVLIBLIST) GlobalLock( hTVLibList ) ;

  /* Initialize structure. */
  fpTVLibList->nLibCount   = 0;
  fpTVLibList->hTVLibInfo = 0;
  
  /* Unlock it. */
  GlobalUnlock( hTVLibList ) ;

  /* Find all library file names and pass them to the callback function InsertLibInfo(). */
  if( !EnumTVLibFiles( NULL, szLibWildName, InsertLibInfo, NULL, Yield ) ) return FALSE;

  /* Return it. */
  return TRUE;
}




/*************************************
*
*  Destroy a library list.
*
**************************************/
void __far __pascal __export DestroyTVLibList( void )
{
  FPTVLIBLIST fpTVLibList;         /* Pointer to the library list. */
  FPTVLIBINFO fpTVLibInfo;         /* Pointer to the top of the lib. info list. */
  int Counter;                     /* Incrementer. */
 

  /* Error - Bad list handle. */
  if( hTVLibList == 0 ) return;

  /* Lock the TVLIBLIST handle. */
  fpTVLibList = (FPTVLIBLIST) GlobalLock( hTVLibList );

  /* Error - Bad list handle. */
  if( fpTVLibList == NULL ) return;

  /* No lib info in the list. */
  if( fpTVLibList->nLibCount == 0 ) 
  {
    GlobalUnlock( hTVLibList );
    return;
  }
  
  /* Lock the lib info list. */ 
  fpTVLibInfo = (FPTVLIBINFO) GlobalLock( fpTVLibList->hTVLibInfo );

  /* For each TVLIBINFO structure in the list. */
  for( Counter = 0; Counter < fpTVLibList->nLibCount; Counter++ )
  {
    /* Delete data inside of the TVLIBINFO. */
    DeleteLibInfoData( fpTVLibInfo );
    
    /* Move to the next TVLIBINFO in the list. */
    fpTVLibInfo = fpTVLibInfo + 1;
  }

  /* Free the TVLIBINFO handle. */
  GlobalUnlock( fpTVLibList->hTVLibInfo );
  GlobalFree( fpTVLibList->hTVLibInfo );

  /* Free the TVLIBLIST handle. */
  GlobalUnlock( hTVLibList );
  GlobalFree( hTVLibList );
}




  
