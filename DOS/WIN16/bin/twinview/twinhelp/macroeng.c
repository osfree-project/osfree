/***********************************************************************

    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

***********************************************************************/
/*******************************************************************************************
**
**                                 MACRO ROUTINES
**
********************************************************************************************/

/***********************************
**
**  System Includes
**
***********************************/
#include <stdlib.h>


/***********************************
**
**  App. Includes
**
***********************************/
#include "macroeng.h"
#include "winmem.h"
#include "string.h"
#include "msgbox.h"
#include "twhlprc.h"
#include "globals.h"


/***********************************
**
**  Private Defines
**
***********************************/

/* How many default macros are there. */
#define NUMDEFAULTMACROS ( sizeof DefaultMacros / sizeof DefaultMacros[0] )

/* 
** Information for each instance of a macro engine.
*/
typedef struct tagMACROENGINE
{
  HGLOBAL hAppData;          /* Data specific to app that owns the macro engine instance. */
  HGLOBAL hRegTableList;     /* Registered tables (Linked list of REGTABLEREC structures). */
  HGLOBAL hLoadTableStack;   /* Loaded tables stack (Linked list of REGTABLEREC structures). */
  HGLOBAL hExecutionStack;   /* Macro execution stack. */
} 
MACROENGINE;
typedef MACROENGINE __far * MACROENGINEPTR;


/* 
** Record in the list of registered tables. 
*/
typedef HGLOBAL HREGTABLEREC;
typedef struct tagREGTABLEREC
{
  /* Linked list data. */
  HREGTABLEREC hPrev;
  HREGTABLEREC hNext;
  
  /* Record data. */
  char    szTableName[ MAX_TABLE_NAME ];  /* Name of the table. */
  WORD    wNumMacros;                     /* Number of macros in the table. */
  HGLOBAL hAtomList;                      /* Handle to the first ATOM in the list. */
  HGLOBAL hMacroTable;                    /* Handle to the first REGTABLEENTRY in the table. */
} 
REGTABLEREC;
typedef REGTABLEREC __far * REGTABLERECPTR;

/* 
** A reg. table entry contains information
** for one macro that is in a registered
** macro table.
*/
typedef struct tagREGTABLEENTRY
{
  MACROFUNCPTR MacroFuncPtr;
  HGLOBAL      hMacroData;
} 
REGTABLEENTRY;
typedef REGTABLEENTRY __far * REGTABLEENTRYPTR;


/* 
** Record in the loaded tables stack.
*/
typedef HGLOBAL HLOADEDTABLEREC;
typedef struct tagLOADEDTABLEREC
{
  /* Linked list data. */
  HLOADEDTABLEREC hPrev;
  HLOADEDTABLEREC hNext;
  
  /* Record data. */
  HGLOBAL hRegTableRec;    /* Handle to the registered table. */
} 
LOADEDTABLEREC;
typedef LOADEDTABLEREC __far * LOADEDTABLERECPTR;


/* 
** Record in the execution stack. 
*/
typedef HGLOBAL HEXECREC;
typedef struct tagEXECREC
{
  /* Linked list data. */
  HEXECREC hPrev;
  HEXECREC hNext;
  
  /* Record data. */
  WORD      wPosition;          /* Current position in the macro string being executed. */
  HGLOBAL   hMacroString;       /* Macro string being executed. */
  short int nOpenParen;         /* Number of parenthesis still left open. */
} 
EXECREC;
typedef EXECREC __far * EXECRECPTR;


/* Enumeration callback function for . */
typedef BOOL ( CALLBACK * ENUMREGTABLERECPROC )( HREGTABLEREC hRegTableRec, LPARAM lParam );
typedef BOOL ( CALLBACK * ENUMLOADEDTABLERECPROC )( HLOADEDTABLEREC hLoadedTableRec, LPARAM lParam );

/*
** Data sent to FindRegTable() enumeration call-back function.
*/
typedef struct tagFINDREGTABLEDATA
{
  char __far * szTableName;
  HREGTABLEREC hRegTableRec;
}
FINDREGTABLEDATA;
typedef FINDREGTABLEDATA __far * FINDREGTABLEDATAPTR;

/* Token used in macro command expressions. */
#define CMDSEP_CHAR_TOKEN ';'


/*
** Data for GetCommandInfo() function and the enumeration functions that
** it uses.
*/
typedef struct tagGETCOMMANDINFODATA
{
  BOOL bFound;                      /* Did we find data for the command. */
  char __far * szMacroCommand;      /* Command. */
  MACROFUNCPTR MacroFuncPtr;        /* Command's function. */
  HGLOBAL      hMacroData;          /* Command's data. */
}
GETCOMMANDINFODATA;
typedef GETCOMMANDINFODATA __far * GETCOMMANDINFODATAPTR;


/***********************************
**
**  Private Function Prototypes
**
***********************************/

static BOOL __far __pascal CreateRegMacroTable
( 
  HWND hErrorWnd,
  HGLOBAL __far * hRegTableListPtr,
  char __far * szTableName, 
  WORD wNumMacros,
  MACROTABLEENTRYPTR MacroTableEntryPtr
);
static HREGTABLEREC __far __pascal DestroyRegMacroTable
( 
  HGLOBAL __far * hRegTableListPtr,
  char __far * szTableName 
);
static void __far __pascal DestroyRegTableList
( 
  HGLOBAL __far * hRegTableListPtr
);
static BOOL __far __pascal FindRegTable( HREGTABLEREC hRegTableRec, LPARAM lParam );
static void __far __pascal EnumRegTable( HGLOBAL hRegTableList, ENUMREGTABLERECPROC EnumRegTableRecProc, LPARAM lParam );
static BOOL __far __pascal FreeRegTable( HREGTABLEREC hRegTableRec, LPARAM lParam );
static BOOL __far __pascal FindRegTableMacro
( 
  HREGTABLEREC hRegTableRec, 
  char __far * szMacroCommand, 
  MACROFUNCPTR __far * MacroFuncDblPtr, 
  HGLOBAL __far * hMacroDataPtr 
);


static BOOL __far __pascal UnloadAllOneTable
( 
  HGLOBAL __far * hLoadTableStackPtr, 
  HREGTABLEREC hRegTableRec 
);
static void __far __pascal DestroyLoadTableStack
( 
  HGLOBAL __far * hLoadTableStackPtr 
);
static void __far __pascal EnumLoadTableStack( HGLOBAL hLoadTableStack, ENUMLOADEDTABLERECPROC EnumLoadedTableRecProc, LPARAM lParam );


static BOOL __far __pascal PushMacroString
( 
  HWND hErrorWnd,
  HGLOBAL __far * hExecutionStackPtr,
  char __far * MacroStringPtr
);
static BOOL __far __pascal PopMacroString
( 
  HGLOBAL __far * hExecutionStackPtr
);
static BOOL __far __pascal GetNextMacroCommand
( 
  HWND hErrorWnd,
  MACROENGINEPTR MacroEnginePtr, 
  MACROFUNCPTR __far * MacroFuncDblPtr,
  HGLOBAL __far * hMacroDataPtr
);
static void __far __pascal DestroyExecutionStack
( 
  HGLOBAL __far * hExecutionStackPtr 
);
static BOOL __far __pascal GetCommandInfo
( 
  char __far * szMacroCommand,
  HGLOBAL hLoadTableStack, 
  MACROFUNCPTR __far * MacroFuncDblPtr, 
  HGLOBAL __far * hMacroDataPtr 
);
static BOOL __far __pascal GetCommandInfoEx( HLOADEDTABLEREC hLoadedTableRec, LPARAM lParam );


static BOOL __far __pascal MacroLoadMacroTable
( 
  HMACROENGINE hMacroEngine,
  HGLOBAL hAppData,
  HGLOBAL hMacroData,
  MACRORETVAL __far * MacroRetValPtr
);
static BOOL __far __pascal MacroUnloadMacroTable
( 
  HMACROENGINE hMacroEngine,
  HGLOBAL hAppData,
  HGLOBAL hMacroData,
  MACRORETVAL __far * MacroRetValPtr
);
static BOOL __far __pascal MacroExecuteMacroFile
( 
  HMACROENGINE hMacroEngine,
  HGLOBAL hAppData,
  HGLOBAL hMacroData,
  MACRORETVAL __far * MacroRetValPtr
);

static BOOL __far __pascal HexConvert( char __far * ArgPtr, long unsigned int __far * nHexConvert );


/***********************************
**
**  Global Static Data
**
***********************************/

/* Default macros. */
static MACROTABLEENTRY DefaultMacros[] =
{
  "LoadMacroTable",       MacroLoadMacroTable,       NULL,  
  "UnloadMacroTable",     MacroUnloadMacroTable,     NULL,  
  "ExecuteMacroFile",     MacroExecuteMacroFile,     NULL  
};



/*********************************************************************************************
**
**                                   Shared Functions
**
**********************************************************************************************/


/************************************ ENGINE FUNCTIONS *****************************/

/***********************************
**
**  Start macro engine. 
**
**  Returns:
**
**     Success - Handle to macro engine.
**     Failure - NULL.
**
***********************************/
HMACROENGINE __far __pascal __export StartMacroEngine( HWND hErrorWnd, HGLOBAL hAppData )
{
  HMACROENGINE   hMacroEngine;
  MACROENGINEPTR MacroEnginePtr;
  
  
  /* Alloc. instance data. */
  if( ! GlobalAllocMem( hErrorWnd, &hMacroEngine, sizeof(MACROENGINE) ) )
  {
    /* Failure. */
    return( NULL );
  }

  /* Lock engine data. */
  MacroEnginePtr = (MACROENGINEPTR) GlobalLock( hMacroEngine );
  
  /* Initialize structure. */
  MacroEnginePtr->hRegTableList   = NULL;
  MacroEnginePtr->hLoadTableStack = NULL;
  MacroEnginePtr->hExecutionStack = NULL;
  MacroEnginePtr->hAppData        = hAppData;
  
  /* Register default macro table. */
  if( !RegisterMacroTable( hErrorWnd, hMacroEngine, RESERVED_TABLE_NAME, 
                           NUMDEFAULTMACROS, DefaultMacros ) )
  {
    /* Free engine data. */
    GlobalUnlock( hMacroEngine );
    GlobalFree( hMacroEngine );
  
    /* Failure. */
    return( NULL );
  }
  
  /* Load default macro table. */
  if( ! LoadMacroTable( hErrorWnd, hMacroEngine, RESERVED_TABLE_NAME ) )
  {
    /* Unregister default macro table. */
    UnregisterMacroTable( hMacroEngine, RESERVED_TABLE_NAME );
    
    /* Free engine data. */
    GlobalUnlock( hMacroEngine );
    GlobalFree( hMacroEngine );
  
    /* Failure. */
    return( NULL );
  }

  /* Unlock engine data. */
  GlobalUnlock( hMacroEngine );
  
  /* Success. */
  return( hMacroEngine );
}



/***********************************
**
**  Stop macro system and frees
**  resource associated with the 
**  instance of the engine. 
**
***********************************/
void __far __pascal __export StopMacroEngine( HMACROENGINE __far * hMacroEnginePtr )
{
  MACROENGINEPTR MacroEnginePtr;
  
  
  /* Lock engine data. */
  MacroEnginePtr = (MACROENGINEPTR) GlobalLock( *hMacroEnginePtr );
  
  /* Initialize structure. */
  MacroEnginePtr->hRegTableList   = NULL;
  MacroEnginePtr->hLoadTableStack = NULL;
  MacroEnginePtr->hExecutionStack = NULL;

  /* Destroy macro table registration list. */
  DestroyRegTableList( &(MacroEnginePtr)->hRegTableList );

  /* Destroy loaded table stack. */
  DestroyLoadTableStack( &(MacroEnginePtr)->hLoadTableStack );

  /* Destroy macro execution stack. */
  DestroyExecutionStack( &(MacroEnginePtr)->hExecutionStack );

  /* Free engine data. */
  GlobalUnlock( *hMacroEnginePtr );
  GlobalFree( *hMacroEnginePtr );
  *hMacroEnginePtr = NULL;
}


/***********************************
**
**  Retrieves the application-specific
**  data originally given to the 
**  application. 
**
**  Returns:
**
**     Success - Handle to macro engine.
**     Failure - NULL.
**
***********************************/
HGLOBAL __far __pascal __export GetMacroEngineAppData( HMACROENGINE hMacroEngine )
{
  MACROENGINEPTR MacroEnginePtr;
  HGLOBAL hAppData;
  
  /* Lock engine data. */
  MacroEnginePtr = (MACROENGINEPTR) GlobalLock( hMacroEngine );
  
  /* Get the handle to the app's data. */
  hAppData = MacroEnginePtr->hAppData;
  
  /* Unlock engine data. */
  GlobalUnlock( hMacroEngine );
  
  return( hAppData );
}



/******************************** TABLE REGISTRATION FUNCTIONS *****************************/
  
/***********************************
**
**  Registers a table of macros.  The
**  table can then be loaded and unloaded. 
**
***********************************/
BOOL __far __pascal __export RegisterMacroTable
( 
  HWND hErrorWnd,
  HMACROENGINE hMacroEngine,
  char __far * szTableName, 
  WORD wNumMacros,
  MACROTABLEENTRYPTR MacroTableEntryPtr
)
{
  MACROENGINEPTR MacroEnginePtr;
  BOOL bReturn;
    
                                  
  /* Lock engine data. */
  MacroEnginePtr = (MACROENGINEPTR) GlobalLock( hMacroEngine );
  
  /*  
  ** Create and insert a new macro table into the registered macro
  ** table list.
  */
  bReturn = CreateRegMacroTable( hErrorWnd, &(MacroEnginePtr)->hRegTableList, 
                                 szTableName, wNumMacros, MacroTableEntryPtr );

  /* Unlock engine data. */
  GlobalUnlock( hMacroEngine );
  
  /* Success/Failure. */
  return( bReturn );
}


/***********************************
**
**  Unregisters a table of macros.  The
**  table can not be loaded and unloaded.
**
**  All loaded instances of the table are
**  unloaded. 
**
***********************************/
BOOL __far __pascal __export UnregisterMacroTable
( 
  HMACROENGINE hMacroEngine,
  char __far * szTableName
)
{
  MACROENGINEPTR MacroEnginePtr;
  HREGTABLEREC   hDestroyTableRec;

                                  
  /* Lock engine data. */
  MacroEnginePtr = (MACROENGINEPTR) GlobalLock( hMacroEngine );
  
  /*  
  ** Destroy information associated with the registered macro
  ** table list.
  */
  hDestroyTableRec = DestroyRegMacroTable( &(MacroEnginePtr)->hRegTableList, szTableName );

  /* If we found something to delete, update the loaded table stack. */
  if( hDestroyTableRec != NULL )
  {
    /* Remove all instances of the table from the loaded table stack. */
    UnloadAllOneTable( &(MacroEnginePtr)->hLoadTableStack, hDestroyTableRec );
  }
  
  /* Unlock engine data. */
  GlobalUnlock( hMacroEngine );
  
  /* Success/Failure. */
  return( hDestroyTableRec != NULL );
}



/***********************************
**
**  Create and insert a new macro
**  table into the registered macro
**  table list.
**
***********************************/
static BOOL __far __pascal CreateRegMacroTable
( 
  HWND hErrorWnd,
  HGLOBAL __far * hRegTableListPtr,
  char __far * szTableName, 
  WORD wNumMacros,
  MACROTABLEENTRYPTR MacroTableEntryPtr
)
{
  REGTABLERECPTR RegTableRecPtr;
  
  WORD               wCounter;
  
  HREGTABLEREC   hNewRegTableRec;
  REGTABLERECPTR NewRegTableRecPtr;
  
  ATOM __far * CurrAtomPtr;
  
  REGTABLEENTRYPTR RegTableEntryPtr;
  
  FINDREGTABLEDATA FindRegTableData;
  
  
  /* Not empty list. */                                  
  if( *hRegTableListPtr != NULL )
  {
    /* Set search criteria. */
    FindRegTableData.hRegTableRec = NULL;
    FindRegTableData.szTableName  = szTableName;
    
    /* Verify that no other table with that name exists. */
    EnumRegTable( *hRegTableListPtr, FindRegTable, (LPARAM) (FINDREGTABLEDATAPTR) &FindRegTableData );
  
    /* Duplicate found? */
    if( FindRegTableData.hRegTableRec != NULL )
    {
      /* Failure. */
      return( FALSE );
    }
  }


  /* Allocate space for the reg. macro table record. */
  if( ! GlobalAllocMem( hErrorWnd, &hNewRegTableRec, sizeof(REGTABLEREC) ) )
  {
    /* Failure. */
    return( FALSE );
  }
  
  /* Lock the new record. */
  NewRegTableRecPtr = (REGTABLERECPTR) GlobalLock( hNewRegTableRec );
  
  /* Initialize the record's data. */
  _fstrcpy( NewRegTableRecPtr->szTableName, szTableName );
  NewRegTableRecPtr->wNumMacros = wNumMacros;
  NewRegTableRecPtr->hAtomList = NULL;
  NewRegTableRecPtr->hMacroTable = NULL;
  

  /* Allocate the atom list. */
  if( ! GlobalAllocMem( hErrorWnd, &(NewRegTableRecPtr)->hAtomList, wNumMacros * sizeof(ATOM) ) )
  {
    /* Free the new record. */
    GlobalUnlock( hNewRegTableRec );
    GlobalFree( hNewRegTableRec );

    /* Failure. */
    return( FALSE );
  }
  
  /* Lock the atom list. */
  CurrAtomPtr = (ATOM __far *) GlobalLock( NewRegTableRecPtr->hAtomList );


  /* Allocate the macro table. */
  if( ! GlobalAllocMem( hErrorWnd, &(NewRegTableRecPtr)->hMacroTable, wNumMacros * sizeof(REGTABLEENTRY) ) )
  {
    /* Free the atom table. */
    GlobalUnlock( NewRegTableRecPtr->hAtomList );
    GlobalFree( NewRegTableRecPtr->hAtomList );

    /* Free the new record. */
    GlobalUnlock( hNewRegTableRec );
    GlobalFree( hNewRegTableRec );

    /* Failure. */
    return( FALSE );
  }
  
  /* Lock the first entry in the macro table. */
  RegTableEntryPtr = (REGTABLEENTRYPTR) GlobalLock( NewRegTableRecPtr->hMacroTable );
  
  
  /* For each macro table entry. */
  for( wCounter = 0; wCounter < wNumMacros; wCounter++ )
  {
    /* Make atom for the macro's name. */
    *CurrAtomPtr = AddAtom( MacroTableEntryPtr->MacroNamePtr );
    
    /* Save function name and data. */
    RegTableEntryPtr->MacroFuncPtr = MacroTableEntryPtr->MacroFuncPtr;
    RegTableEntryPtr->hMacroData   = MacroTableEntryPtr->hMacroData;

    /* Move to next macro table entry. */
    CurrAtomPtr++;
    RegTableEntryPtr++;
    MacroTableEntryPtr++;
  }
  
  /* Unlock the atom list. */
  GlobalUnlock( NewRegTableRecPtr->hAtomList );

  /* Unlock the macro table. */
  GlobalUnlock( NewRegTableRecPtr->hMacroTable );

  /* Unlock the new record. */
  GlobalUnlock( hNewRegTableRec );
  
  /* Not empty list. */                                  
  if( *hRegTableListPtr != NULL )
  {
    /* Lock the reg. table list. */
    RegTableRecPtr = (REGTABLERECPTR) GlobalLock( *hRegTableListPtr );
  
    /* Set previous record. */
    RegTableRecPtr->hPrev = hNewRegTableRec;

    /* Unlock the reg. table list. */
    GlobalUnlock( *hRegTableListPtr );
  }
  
  /* Lock the new reg. table record. */
  RegTableRecPtr = (REGTABLERECPTR) GlobalLock( hNewRegTableRec );

  /* Set previous record. */
  RegTableRecPtr->hPrev = NULL;
  RegTableRecPtr->hNext = *hRegTableListPtr;
  
  /* Unlock the new reg. table record. */
  GlobalUnlock( hNewRegTableRec );

  /* New top. */
  *hRegTableListPtr = hNewRegTableRec;
  
  /* Success. */
  return( TRUE );
}


/***********************************
**
**  Destroy information associated 
**  with the registered macro table
**  list.
**
***********************************/
static HREGTABLEREC __far __pascal DestroyRegMacroTable
( 
  HGLOBAL __far * hRegTableListPtr,
  char __far * szTableName 
)
{
  HREGTABLEREC   hCurrTableRec;
  HREGTABLEREC   hNextTableRec;
  REGTABLERECPTR CurrTableRecPtr;
  REGTABLERECPTR TempTableRecPtr;

  ATOM __far * CurrAtomPtr;
  
  REGTABLEENTRYPTR RegTableEntryPtr;
  
  WORD               wCounter;

  HREGTABLEREC   hDestroyTableRec;
    
  
  /* Empty list. */                                  
  if( *hRegTableListPtr == NULL ) return NULL;

  /* Nothing destroyed yet. */
  hDestroyTableRec = NULL;
 
  /* Get handle to first record. */
  hCurrTableRec = *hRegTableListPtr;
  
  while( hCurrTableRec != NULL )
  {
    /* Lock the current reg. table record. */
    CurrTableRecPtr = (REGTABLERECPTR) GlobalLock( hCurrTableRec );
  
    /* Found name? */
    if( _fstrcmp( CurrTableRecPtr->szTableName, szTableName ) == 0 )
    {
      /* Table destroyed. */
      hDestroyTableRec = hCurrTableRec;
      
      /* Adjust prev. record to point to next. */
      if( CurrTableRecPtr->hPrev != NULL )
      {
        /* Lock the prev. reg. table record. */
        TempTableRecPtr = (REGTABLERECPTR) GlobalLock( CurrTableRecPtr->hPrev );
        
        /* Prev.'s next points top current's next. */
        TempTableRecPtr->hNext = CurrTableRecPtr->hNext;

        /* Unlock the prev. reg. table record. */
        GlobalUnlock( CurrTableRecPtr->hPrev );
      }
    
      /* Adjust next record to point to prev. */
      if( CurrTableRecPtr->hNext != NULL )
      {
        /* Lock the next reg. table record. */
        TempTableRecPtr = (REGTABLERECPTR) GlobalLock( CurrTableRecPtr->hNext );
        
        /* Prev.'s next points top current's next. */
        TempTableRecPtr->hPrev = CurrTableRecPtr->hPrev;

        /* Unlock the next reg. table record. */
        GlobalUnlock( CurrTableRecPtr->hNext );
      }
    
      /* If we have a new top. */
      if( hCurrTableRec == *hRegTableListPtr )
      {
        *hRegTableListPtr = CurrTableRecPtr->hNext;
      }
      
      /* Lock the first entry in the macro table. */
      RegTableEntryPtr = (REGTABLEENTRYPTR) GlobalLock( CurrTableRecPtr->hMacroTable );
      
      /* Lock first entry in the atom list. */
      CurrAtomPtr = (ATOM __far *) GlobalLock( CurrTableRecPtr->hAtomList );

      /* For each macro table entry. */
      for( wCounter = 0; wCounter < CurrTableRecPtr->wNumMacros; wCounter++ )
      {
        /* Delete the atom. */
        DeleteAtom( *CurrAtomPtr );
        
        /* Free the macro's associated data. */
        GlobalFree( RegTableEntryPtr->hMacroData );
    
        /* Move to next macro table entry. */
        CurrAtomPtr++;
        RegTableEntryPtr++;
      }

      /* Free the the macro table. */
      GlobalUnlock( CurrTableRecPtr->hMacroTable );
      GlobalFree( CurrTableRecPtr->hMacroTable );

      /* Free the the ATOM list. */
      GlobalUnlock( CurrTableRecPtr->hAtomList );
      GlobalFree( CurrTableRecPtr->hAtomList );

      /* Free the current reg. table record. */
      GlobalUnlock( hCurrTableRec );
      GlobalFree( hCurrTableRec);

      /* Stop looking. */
      hCurrTableRec = NULL;
    }
     
    /* Keep looking. */
    else
    {
      /* Get next record's handle. */
      hNextTableRec = CurrTableRecPtr->hNext;
    
      /* Unlock the current reg. table record. */
      GlobalUnlock( hCurrTableRec );

      /* New current. */
      hCurrTableRec = hNextTableRec;
    }
  } 
  
  /* Return handle of destroyed record. */
  return( hDestroyTableRec );
}


static void __far __pascal DestroyRegTableList
( 
  HGLOBAL __far * hRegTableListPtr
)
{
  HREGTABLEREC   hCurrTableRec;
  HREGTABLEREC   hNextTableRec;
  REGTABLERECPTR CurrTableRecPtr;

  ATOM __far * CurrAtomPtr;
  
  REGTABLEENTRYPTR RegTableEntryPtr;
  
  WORD               wCounter;

  
  /* Get handle to first record. */
  hCurrTableRec = *hRegTableListPtr;
  
  while( hCurrTableRec != NULL )
  {
    /* Lock the current reg. table record. */
    CurrTableRecPtr = (REGTABLERECPTR) GlobalLock( hCurrTableRec );
  
    /* Lock the first entry in the macro table. */
    RegTableEntryPtr = (REGTABLEENTRYPTR) GlobalLock( CurrTableRecPtr->hMacroTable );
      
    /* Lock first entry in the atom list. */
    CurrAtomPtr = (ATOM __far *) GlobalLock( CurrTableRecPtr->hAtomList );

    /* For each macro table entry. */
    for( wCounter = 0; wCounter < CurrTableRecPtr->wNumMacros; wCounter++ )
    {
      /* Delete the atom. */
      DeleteAtom( *CurrAtomPtr );
        
      /* Free the macro's associated data. */
      GlobalFree( RegTableEntryPtr->hMacroData );
    
      /* Move to next macro table entry. */
      CurrAtomPtr++;
      RegTableEntryPtr++;
    }

    /* Free the the macro table. */
    GlobalUnlock( CurrTableRecPtr->hMacroTable );
    GlobalFree( CurrTableRecPtr->hMacroTable );

    /* Free the the ATOM list. */
    GlobalUnlock( CurrTableRecPtr->hAtomList );
    GlobalFree( CurrTableRecPtr->hAtomList );

    /* Get next record's handle. */
    hNextTableRec = CurrTableRecPtr->hNext;
    
    /* Free the current reg. table record. */
    GlobalUnlock( hCurrTableRec );
    GlobalFree( hCurrTableRec);

    /* New current. */
    hCurrTableRec = hNextTableRec;
  }

  /* Empty list. */
  *hRegTableListPtr = NULL;
}


/***********************************
**
**  Search for a registered macro
**  table list with the specified
**  name.
**
***********************************/
static BOOL __far __pascal FindRegTable( HREGTABLEREC hRegTableRec, LPARAM lParam )
{
  REGTABLERECPTR RegTableRecPtr;
  FINDREGTABLEDATAPTR FindRegTableDataPtr;
  
  /* Get search criteria. */
  FindRegTableDataPtr = (FINDREGTABLEDATAPTR) lParam;

  /* Lock the reg. table record. */
  RegTableRecPtr = (REGTABLERECPTR) GlobalLock( hRegTableRec );
    
  /* Found name? */
  if( _fstrcmp( RegTableRecPtr->szTableName, FindRegTableDataPtr->szTableName ) == 0 )
  {
    FindRegTableDataPtr->hRegTableRec = hRegTableRec;
  }
  
  /* Unlock the reg. table record. */
  GlobalUnlock( hRegTableRec );
  
  /* If we didn't found it keep enumerating. */
  return( FindRegTableDataPtr->hRegTableRec == NULL );
}  


/***********************************
**
**  Pass each record in the 
**  hRegTableList to the call-back
**  function.
**
***********************************/
static void __far __pascal EnumRegTable
( 
  HGLOBAL hRegTableList, 
  ENUMREGTABLERECPROC EnumRegTableRecProc, 
  LPARAM lParam 
)
{
  
  HREGTABLEREC hCurr;
  HREGTABLEREC hNext;

  REGTABLERECPTR RegTableRecPtr;
  
  BOOL bContinue;
  
  
  /* Get handle of first record. */
  hCurr = hRegTableList;
  
  /* Not empty list. */                                  
  while( hCurr != NULL )
  {
    /* Lock the reg. table record. */
    RegTableRecPtr = (REGTABLERECPTR) GlobalLock( hCurr );
    
    /* Get next. */
    hNext = RegTableRecPtr->hNext;

    /* Unlock the reg. table record. */
    GlobalUnlock( hCurr );

    /* Call call-back function. */
    bContinue = (*EnumRegTableRecProc)( hCurr, lParam );

    /* If keep enumerating. */
    if( bContinue )
    {
      /* New current record. */
      hCurr = hNext;
    }
    
    /* If not keep enumerating. */
    else 
    { 
      /* Stop. */
      hCurr = NULL;
    }
  } 
}



/***********************************
**
**  Find a macro's data in a
**  registered table record.
**
***********************************/
static BOOL __far __pascal FindRegTableMacro
( 
  HREGTABLEREC hRegTableRec, 
  char __far * szMacroCommand, 
  MACROFUNCPTR __far * MacroFuncDblPtr, 
  HGLOBAL __far * hMacroDataPtr 
)
{
  REGTABLERECPTR RegTableRecPtr;
  REGTABLEENTRYPTR RegTableEntryPtr;
  
  ATOM SearchAtom;
  ATOM __far * CurrAtomPtr;

  WORD wCounter;
  BOOL bFound; 
  
  
  
  /* Create an atom for the target macro's name. */
  SearchAtom = AddAtom( szMacroCommand );
  
  /* Lock the record. */
  RegTableRecPtr = (REGTABLERECPTR) GlobalLock( hRegTableRec );
  
  /* Lock the atom list. */
  CurrAtomPtr = (ATOM __far *) GlobalLock( RegTableRecPtr->hAtomList );

  /* Haven't found atom. */
  bFound = FALSE;
  
  /* For each atom in the table entry. */
  for( wCounter = 0; wCounter < RegTableRecPtr->wNumMacros; wCounter++ )
  {
    /* Found the atom for the command. */
    if( *CurrAtomPtr == SearchAtom )
    {
      bFound = TRUE;
      break;
    }

    /* Move to next atom. */
    CurrAtomPtr++;
  }
  
  /* Unlock the atom list. */
  GlobalUnlock( RegTableRecPtr->hAtomList );

  /* Delete the atom for the target macro's name. */
  DeleteAtom( SearchAtom );
        
  /* If we didn't find the atom. */
  if( bFound == FALSE )
  {
    /* Unlock the record. */
    GlobalLock( hRegTableRec );
  
    return( FALSE );
  }
  
  /* Lock the first entry in the macro table. */
  RegTableEntryPtr = (REGTABLEENTRYPTR) GlobalLock( RegTableRecPtr->hMacroTable );
  
  /* Go to correct entry. */
  RegTableEntryPtr = RegTableEntryPtr + wCounter;
  
  /* Get macro command's data. */
  *MacroFuncDblPtr = RegTableEntryPtr->MacroFuncPtr; 
  *hMacroDataPtr   = RegTableEntryPtr->hMacroData;

  /* Unlock the macro table. */
  GlobalUnlock( RegTableRecPtr->hMacroTable );

  /* Unlock the record. */
  GlobalLock( hRegTableRec );
  
  /* Success. */
  return( TRUE );
}

/********************************** LOADED TABLES STACK FUNCTIONS *****************************/

/***********************************
**
**  Adds the table to the top of the
**  macro table stack.
**
**  It will be the first table that is
**  considered when executing a macro.
**
***********************************/
BOOL __far __pascal __export LoadMacroTable
( 
  HWND hErrorWnd,
  HMACROENGINE hMacroEngine,
  char __far * szTableName 
)
{
  MACROENGINEPTR MacroEnginePtr;
  FINDREGTABLEDATA FindRegTableData;
  
  HLOADEDTABLEREC hNewLoadTableRec;
  LOADEDTABLERECPTR LoadTableRecPtr;
  
                                  
  /* Lock engine data. */
  MacroEnginePtr = (MACROENGINEPTR) GlobalLock( hMacroEngine );
  
  /* Set search criteria. */
  FindRegTableData.hRegTableRec = NULL;
  FindRegTableData.szTableName  = szTableName;
    
  /* Find the registered table's handle. */
  EnumRegTable( MacroEnginePtr->hRegTableList, FindRegTable, (LPARAM) (FINDREGTABLEDATAPTR) &FindRegTableData );
  
  /* Not found? */
  if( FindRegTableData.hRegTableRec == NULL )
  {
    /* Unlock engine data. */
    GlobalUnlock( hMacroEngine );
    
    /* Failure. */
    return( FALSE );
  }

  /* Allocate space for the loaded table record. */
  if( ! GlobalAllocMem( hErrorWnd, &hNewLoadTableRec, sizeof(LOADEDTABLEREC) ) )
  {
    /* Unlock engine data. */
    GlobalUnlock( hMacroEngine );
    
    /* Failure. */
    return( FALSE );
  }
  
  /* Not empty stack. */                                  
  if( MacroEnginePtr->hLoadTableStack != NULL )
  {
    /* Lock the top of stack. */
    LoadTableRecPtr = (LOADEDTABLERECPTR) GlobalLock( MacroEnginePtr->hLoadTableStack );
  
    /* Set previous record. */
    LoadTableRecPtr->hPrev = hNewLoadTableRec;

    /* Unlock the top of stack. */
    GlobalUnlock( MacroEnginePtr->hLoadTableStack );
  }
  
  /* Lock the new record. */
  LoadTableRecPtr = (LOADEDTABLERECPTR) GlobalLock( hNewLoadTableRec );
  
  /* Set new record's data. */
  LoadTableRecPtr->hPrev = NULL;
  LoadTableRecPtr->hNext = MacroEnginePtr->hLoadTableStack;
  LoadTableRecPtr->hRegTableRec = FindRegTableData.hRegTableRec;
  
  /* Unlock the new reg. table record. */
  GlobalUnlock( hNewLoadTableRec );

  /* New top. */
  MacroEnginePtr->hLoadTableStack = hNewLoadTableRec;
  
  /* Unlock engine data. */
  GlobalUnlock( hMacroEngine );
  
  /* Success. */
  return( TRUE );
}


/***********************************
**
**  Remove the first instance of the
**  table from the macro table list.
**
**  Other instances of the table will
**  not be removed.
**
**  If no instance of the table exists,
**  no action is performed.
**
***********************************/
BOOL __far __pascal __export UnloadMacroTable
( 
  HMACROENGINE hMacroEngine,
  char __far * szTableName 
)
{
  MACROENGINEPTR MacroEnginePtr;
  FINDREGTABLEDATA FindRegTableData;
  
  HLOADEDTABLEREC hCurrLoadTableRec;
  HLOADEDTABLEREC hNextLoadTableRec;
  
  LOADEDTABLERECPTR CurrLoadTableRecPtr;
  LOADEDTABLERECPTR TempTableRecPtr;
  
  BOOL bUnloaded;
  
                                  
  /* Lock engine data. */
  MacroEnginePtr = (MACROENGINEPTR) GlobalLock( hMacroEngine );
  
  /* Empty stack? */
  if( MacroEnginePtr->hLoadTableStack == NULL ) 
  {
    /* Unlock engine data. */
    GlobalUnlock( hMacroEngine );
  
    /* Failure. */
    return FALSE;
  }
  
  /* Set search criteria. */
  FindRegTableData.hRegTableRec = NULL;
  FindRegTableData.szTableName  = szTableName;
    
  /* Find the registered table's handle. */
  EnumRegTable( MacroEnginePtr->hRegTableList, FindRegTable, (LPARAM) (FINDREGTABLEDATAPTR) &FindRegTableData );
  
  /* Not found? */
  if( FindRegTableData.hRegTableRec == NULL )
  {
    /* Unlock engine data. */
    GlobalUnlock( hMacroEngine );
  
    /* Failure. */
    return( FALSE );
  }

  /* No unloaded yet. */
  bUnloaded = FALSE;

  /* Get first record. */
  hCurrLoadTableRec = MacroEnginePtr->hLoadTableStack;
   
  /* For each record. */
  while( hCurrLoadTableRec != NULL )
  {
    /* Lock the current record. */
    CurrLoadTableRecPtr = (LOADEDTABLERECPTR) GlobalLock( hCurrLoadTableRec );
  
    /* Found name? */
    if( CurrLoadTableRecPtr->hRegTableRec == FindRegTableData.hRegTableRec )
    {
      /* Something getting unloaded. */
      bUnloaded = TRUE;
      
      /* Adjust prev. record to point to next. */
      if( CurrLoadTableRecPtr->hPrev != NULL )
      {
        /* Lock the prev. record. */
        TempTableRecPtr = (LOADEDTABLERECPTR) GlobalLock( CurrLoadTableRecPtr->hPrev );
        
        /* Prev.'s next points top current's next. */
        TempTableRecPtr->hNext = CurrLoadTableRecPtr->hNext;

        /* Unlock the prev. reg. table record. */
        GlobalUnlock( CurrLoadTableRecPtr->hPrev );
      }
    
      /* Adjust next record to point to prev. */
      if( CurrLoadTableRecPtr->hNext != NULL )
      {
        /* Lock the next reg. table record. */
        TempTableRecPtr = (LOADEDTABLERECPTR) GlobalLock( CurrLoadTableRecPtr->hNext );
        
        /* Prev.'s next points top current's next. */
        TempTableRecPtr->hPrev = CurrLoadTableRecPtr->hPrev;

        /* Unlock the next reg. table record. */
        GlobalUnlock( CurrLoadTableRecPtr->hNext );
      }
    
      /* If we have a new top. */
      if( hCurrLoadTableRec == MacroEnginePtr->hLoadTableStack )
      {
        MacroEnginePtr->hLoadTableStack = CurrLoadTableRecPtr->hNext;
      }
      
      /* Free the current record. */
      GlobalUnlock( hCurrLoadTableRec );
      GlobalFree( hCurrLoadTableRec);

      /* Stop looking. */
      hCurrLoadTableRec = NULL;
    }
     
    /* Keep looking. */
    else
    {
      /* Get next record's handle. */
      hNextLoadTableRec = CurrLoadTableRecPtr->hNext;
    
      /* Unlock the current record. */
      GlobalUnlock( hCurrLoadTableRec );

      /* New current record. */
      hCurrLoadTableRec = hNextLoadTableRec;
    }
  } 

  /* Unlock engine data. */
  GlobalUnlock( hMacroEngine );
  
  /* Success/Failure. */
  return( bUnloaded );
}



/***********************************
**
**  Remove all instances of a 
**  table from the loaded table stack.
**
***********************************/
static BOOL __far __pascal UnloadAllOneTable
( 
  HGLOBAL __far * hLoadTableStackPtr, 
  HREGTABLEREC hRegTableRec 
)
{
  HLOADEDTABLEREC hCurrLoadTableRec;
  HLOADEDTABLEREC hNextLoadTableRec;
  
  LOADEDTABLERECPTR CurrLoadTableRecPtr;
  LOADEDTABLERECPTR TempTableRecPtr;
  
  BOOL bUnloaded;
  
                                  
  /* Empty stack? */
  if( *hLoadTableStackPtr == NULL ) 
  {
    /* Failure. */
    return FALSE;
  }
  
  /* No unloaded yet. */
  bUnloaded = FALSE;

  /* Get first record. */
  hCurrLoadTableRec = *hLoadTableStackPtr;
   
  /* For each record. */
  while( hCurrLoadTableRec != NULL )
  {
    /* Lock the current record. */
    CurrLoadTableRecPtr = (LOADEDTABLERECPTR) GlobalLock( hCurrLoadTableRec );
  
    /* Found name? */
    if( CurrLoadTableRecPtr->hRegTableRec == hRegTableRec )
    {
      /* Something getting unloaded. */
      bUnloaded = TRUE;
      
      /* Adjust prev. record to point to next. */
      if( CurrLoadTableRecPtr->hPrev != NULL )
      {
        /* Lock the prev. record. */
        TempTableRecPtr = (LOADEDTABLERECPTR) GlobalLock( CurrLoadTableRecPtr->hPrev );
        
        /* Prev.'s next points top current's next. */
        TempTableRecPtr->hNext = CurrLoadTableRecPtr->hNext;

        /* Unlock the prev. reg. table record. */
        GlobalUnlock( CurrLoadTableRecPtr->hPrev );
      }
    
      /* Adjust next record to point to prev. */
      if( CurrLoadTableRecPtr->hNext != NULL )
      {
        /* Lock the next reg. table record. */
        TempTableRecPtr = (LOADEDTABLERECPTR) GlobalLock( CurrLoadTableRecPtr->hNext );
        
        /* Prev.'s next points top current's next. */
        TempTableRecPtr->hPrev = CurrLoadTableRecPtr->hPrev;

        /* Unlock the next reg. table record. */
        GlobalUnlock( CurrLoadTableRecPtr->hNext );
      }
    
      /* If we have a new top. */
      if( hCurrLoadTableRec == *hLoadTableStackPtr )
      {
        *hLoadTableStackPtr = CurrLoadTableRecPtr->hNext;
      }
      
      /* Get next record's handle. */
      hNextLoadTableRec = CurrLoadTableRecPtr->hNext;
    
      /* Free the current record. */
      GlobalUnlock( hCurrLoadTableRec );
      GlobalFree( hCurrLoadTableRec);

      /* New current record. */
      hCurrLoadTableRec = hNextLoadTableRec;
    }
     
    /* Keep looking. */
    else
    {
      /* Get next record's handle. */
      hNextLoadTableRec = CurrLoadTableRecPtr->hNext;
    
      /* Unlock the current record. */
      GlobalUnlock( hCurrLoadTableRec );

      /* New current record. */
      hCurrLoadTableRec = hNextLoadTableRec;
    }
  } 

  /* Success/Failure. */
  return( bUnloaded );
}

  

/***********************************
**
**  Remove all tables from the 
**  loaded table stack.
**
***********************************/
static void __far __pascal DestroyLoadTableStack
( 
  HGLOBAL __far * hLoadTableStackPtr 
)
{
  HLOADEDTABLEREC hCurrLoadTableRec;
  HLOADEDTABLEREC hNextLoadTableRec;
  
  LOADEDTABLERECPTR CurrLoadTableRecPtr;
  
  /* Get first record. */
  hCurrLoadTableRec = *hLoadTableStackPtr;
   
  /* For each record. */
  while( hCurrLoadTableRec != NULL )
  {
    /* Lock the current record. */
    CurrLoadTableRecPtr = (LOADEDTABLERECPTR) GlobalLock( hCurrLoadTableRec );
      
    /* Get next record's handle. */
    hNextLoadTableRec = CurrLoadTableRecPtr->hNext;
    
    /* Free the current record. */
    GlobalUnlock( hCurrLoadTableRec );
    GlobalFree( hCurrLoadTableRec);

    /* New current record. */
    hCurrLoadTableRec = hNextLoadTableRec;
  }
  
  /* Empty stack. */
  *hLoadTableStackPtr = NULL;
}



/***********************************
**
**  Pass each record in the 
**  hLoadedTableStack to the call-back
**  function.
**
***********************************/
static void __far __pascal EnumLoadTableStack
( 
  HGLOBAL hLoadTableStack, 
  ENUMLOADEDTABLERECPROC EnumLoadedTableRecProc, 
  LPARAM lParam 
)
{
  
  HLOADEDTABLEREC hCurr;
  HLOADEDTABLEREC hNext;

  LOADEDTABLERECPTR LoadedTableRecPtr;
  
  BOOL bContinue;
  
  
  /* Get handle of first record. */
  hCurr = hLoadTableStack;
  
  /* Not empty list. */                                  
  while( hCurr != NULL )
  {
    /* Lock the reg. table record. */
    LoadedTableRecPtr = (LOADEDTABLERECPTR) GlobalLock( hCurr );
    
    /* Get next. */
    hNext = LoadedTableRecPtr->hNext;

    /* Unlock the reg. table record. */
    GlobalUnlock( hCurr );

    /* Call call-back function. */
    bContinue = (*EnumLoadedTableRecProc)( hCurr, lParam );

    /* If keep enumerating. */
    if( bContinue )
    {
      /* New current record. */
      hCurr = hNext;
    }
    
    /* If not keep enumerating. */
    else 
    { 
      /* Stop. */
      hCurr = NULL;
    }
  } 
}



/********************************** EXECUTION STACK FUNCTIONS *****************************/

/***********************************
**
**  Execute a macro string. 
**
**  Returns:
**
**      TRUE  - No error.
**      FALSE - Error occured.
**
***********************************/
BOOL __far __pascal __export ExecuteMacro
( 
  HWND hErrorWnd,
  HMACROENGINE hMacroEngine,
  char __far * MacroStringPtr,
  MACRORETVAL __far * MacroRetValPtr
)
{
  BOOL           bReturn;
  MACROENGINEPTR MacroEnginePtr;
  
  MACROFUNCPTR   MacroFuncPtr;
  HGLOBAL        hMacroData;

                                  
  /* Lock engine data. */
  MacroEnginePtr = (MACROENGINEPTR) GlobalLock( hMacroEngine );
  
  /* 
  ** Push the new macro string onto the top of 
  ** the execution stack. 
  */
  if( ! PushMacroString( hErrorWnd, &(MacroEnginePtr)->hExecutionStack, MacroStringPtr ) )
  {
    /* Unlock engine data. */
    GlobalUnlock( hMacroEngine );
    
    /* Failure */
    return( FALSE );
  }


  /* Get first command - We should always have at least one command. */
  bReturn = GetNextMacroCommand( hErrorWnd, MacroEnginePtr, &MacroFuncPtr, &hMacroData );

  /* No macro command found or no matching function for the command. */
  if( bReturn == FALSE || MacroFuncPtr == NULL )
  {
    /* Pop unprocessed macro string off top of execution stack. */
    PopMacroString( &(MacroEnginePtr)->hExecutionStack );
 
    /* Unlock engine data. */
    GlobalUnlock( hMacroEngine );
    
    /* Failure */
    return( FALSE );
  }

  /* Call the function associated with the macro command. */
  bReturn = (*MacroFuncPtr)( hMacroEngine, MacroEnginePtr->hAppData, hMacroData, MacroRetValPtr ); 
    
  /* Function failed. */
  if( bReturn == FALSE )
  {
    /* Pop unprocessed macro string off top of execution stack. */
    PopMacroString( &(MacroEnginePtr)->hExecutionStack );
 
    /* Unlock engine data. */
    GlobalUnlock( hMacroEngine );
    
    /* Failure */
    return( FALSE );
  }


  /* Process any other commands in the line. */
  while( TRUE )
  {
    /* Get next command. */
    bReturn = GetNextMacroCommand( hErrorWnd, MacroEnginePtr, &MacroFuncPtr, &hMacroData );
  
    /* Error. */
    if( bReturn == FALSE )
    {
      /* Pop unprocessed macro string off top of execution stack. */
      PopMacroString( &(MacroEnginePtr)->hExecutionStack );
   
      /* Unlock engine data. */
      GlobalUnlock( hMacroEngine );
      
      /* Failure */
      return( FALSE );
    }

  
    /* No error, but no more commands. */
    else if( bReturn == TRUE && MacroFuncPtr == NULL )
    {
      /* Stop. */
      break;
    }

  
    /* Call the function associated with the macro command. */
    else 
    {
      bReturn = (*MacroFuncPtr)( hMacroEngine, MacroEnginePtr->hAppData, hMacroData, MacroRetValPtr ); 
    }
  }

  /* 
  ** Pop the processed macro string from the top of the 
  ** execution stack. 
  */
  if( ! PopMacroString( &(MacroEnginePtr)->hExecutionStack ) )
  {
    /* Unlock engine data. */
    GlobalUnlock( hMacroEngine );
    
    /* Failure */
    return( FALSE );
  }

  /* Unlock engine data. */
  GlobalUnlock( hMacroEngine );
  
  /* Success. */
  return( TRUE );
}



/***********************************
**
** Push a macro string onto the
** the top of the execution stack. 
**
***********************************/
static BOOL __far __pascal PushMacroString
( 
  HWND hErrorWnd,
  HGLOBAL __far * hExecutionStackPtr,
  char __far * MacroStringPtr
)
{
  HGLOBAL      hMacroString;
  char __far * DstMacroStringPtr;
  WORD         wStringSize;
  
  HEXECREC   hNewExecRec;
  EXECRECPTR ExecRecPtr;
  
                                  
  /* Allocate space for the loaded table record. */
  if( ! GlobalAllocMem( hErrorWnd, &hNewExecRec, sizeof(EXECREC) ) )
  {
    /* Failure. */
    return( FALSE );
  }
  
  /* 
  ** Size of macro string. 
  **
  ** One extra char. added for teminating null char.
  ** One extra char. added for ';' added as fist char.
  */
  wStringSize = _fstrlen( MacroStringPtr ) + ( 2 * sizeof(char) );
  
  /* Allocate space for the loaded table record. */
  if( ! GlobalAllocMem( hErrorWnd, &hMacroString, wStringSize ) )
  {
    /* Free new record. */
    GlobalFree( hNewExecRec );
    
    /* Failure. */
    return( FALSE );
  }

  /* Copy macro string into buffer. */
  DstMacroStringPtr = (char __far *) GlobalLock( hMacroString );
  *DstMacroStringPtr = CMDSEP_CHAR_TOKEN;
  _fstrcat( DstMacroStringPtr, MacroStringPtr );
  GlobalUnlock( hMacroString );

  /* Not empty stack. */                                  
  if( *hExecutionStackPtr != NULL )
  {
    /* Lock the top of stack. */
    ExecRecPtr = (EXECRECPTR) GlobalLock( *hExecutionStackPtr );
  
    /* Set previous record. */
    ExecRecPtr->hPrev = hNewExecRec;

    /* Unlock the top of stack. */
    GlobalUnlock( *hExecutionStackPtr );
  }
  
  /* Lock the new record. */
  ExecRecPtr = (EXECRECPTR) GlobalLock( hNewExecRec );
  
  /* Set new record's data. */
  ExecRecPtr->hPrev          = NULL;
  ExecRecPtr->hNext          = *hExecutionStackPtr;
  ExecRecPtr->wPosition      = 0;
  ExecRecPtr->hMacroString   = hMacroString;
  ExecRecPtr->nOpenParen     = 0;

  /* Unlock the new reg. table record. */
  GlobalUnlock( hNewExecRec );

  /* New top. */
  *hExecutionStackPtr = hNewExecRec;
  
  /* Success. */
  return( TRUE );
}


 
/***********************************
**
** Pop a macro string from the
** the top of the execution stack. 
**
***********************************/
static BOOL __far __pascal PopMacroString
( 
  HGLOBAL __far * hExecutionStackPtr
)
{
  HEXECREC   hOldExecRec;
  EXECRECPTR OldExecRecPtr;
  EXECRECPTR NextExecRecPtr;
  
  /* Not empty stack. */                                  
  if( *hExecutionStackPtr == NULL ) return( FALSE );

  /* Save handle of record being freed. */
  hOldExecRec = *hExecutionStackPtr;
  
  /* Lock the old record. */
  OldExecRecPtr = (EXECRECPTR) GlobalLock( hOldExecRec );
  
  /* Next record is new top. */
  *hExecutionStackPtr = OldExecRecPtr->hNext;
  
  /* If there is a next record. */
  if( OldExecRecPtr->hNext != NULL )
  {
    /* Lock the next record. */
    NextExecRecPtr = (EXECRECPTR) GlobalLock( OldExecRecPtr->hNext );
  
    /* Set prev. to nothing. */
    NextExecRecPtr->hPrev = NULL;

    /* Unlock the next record. */
    GlobalUnlock( OldExecRecPtr->hNext );
  }

  /* Free the old record. */
  GlobalFree( OldExecRecPtr->hMacroString );
  GlobalUnlock( hOldExecRec );
  GlobalFree( hOldExecRec );
  
  /* Success. */
  return( TRUE );
}



/***********************************
**
**  Removes all records
**  from the execution stack.
**
***********************************/
static void __far __pascal DestroyExecutionStack
( 
  HGLOBAL __far * hExecutionStackPtr 
)
{
  HEXECREC hCurrExecRec;
  HEXECREC hNextExecRec;
  EXECRECPTR CurrExecRecPtr;

  
  /* Get first record. */
  hCurrExecRec = *hExecutionStackPtr;
   
  /* For each record. */
  while( hCurrExecRec != NULL )
  {
    /* Lock the current record. */
    CurrExecRecPtr = (EXECRECPTR) GlobalLock( hCurrExecRec );
      
    /* Free the current record's macro string. */
    GlobalFree( CurrExecRecPtr->hMacroString );

    /* Get next record's handle. */
    hNextExecRec = CurrExecRecPtr->hNext;
    
    /* Free the current record. */
    GlobalUnlock( hCurrExecRec );
    GlobalFree( hCurrExecRec);

    /* New current record. */
    hCurrExecRec = hNextExecRec;
  }
  
  /* Empty stack. */
  *hExecutionStackPtr = NULL;
}



/********************************* MACRO PARSING FUNCTIONS ***************************/

/***********************************
**
**  Get next command from the current
**  position in the macro string and
**  find its associated call-back
**  function and private data.
**  
**  Returns:
**      
**    TRUE  - If no more commands and no error occurred. 
** 
**    FALSE - Error occurred such as parsing error or could not
**            find the function associated with the next macro command.
**
***********************************/
static BOOL __far __pascal GetNextMacroCommand
( 
  HWND hErrorWnd,
  MACROENGINEPTR MacroEnginePtr, 
  MACROFUNCPTR __far * MacroFuncDblPtr,
  HGLOBAL __far * hMacroDataPtr
)
{
  EXECRECPTR ExecRecPtr;      /* Pointer to the macro string currently being executed. */

  char __far * CurrPosPtr;    /* Pointer to the current position in the macro string. */

  char __far * CmdEndPtr;     /* Pointer to the position in the macro string after a command. */
  WORD wCmdLen;               /* Length of the command. */

  BOOL bFoundNextCmnd;

  BOOL bReturn;
  
  
  /* Lock the execution stack's top record. */
  ExecRecPtr = (EXECRECPTR) GlobalLock( MacroEnginePtr->hExecutionStack );
      
  /* Lock the macro string. */
  CurrPosPtr = (char __far *) GlobalLock( ExecRecPtr->hMacroString );

  /* Move to current position in macro string. */
  CurrPosPtr = CurrPosPtr + ExecRecPtr->wPosition;
  
  /* Haven't found a command yet. */
  bFoundNextCmnd = FALSE;
  
  /* Skip leading white space. */
  while( *CurrPosPtr == ' ' ) 
  { 
    ExecRecPtr->wPosition = ExecRecPtr->wPosition + sizeof( char );
    CurrPosPtr++;
  }
                              
  /* Go to next command. */
  while( *CurrPosPtr != '\0' && bFoundNextCmnd == FALSE )
  {
    switch( *CurrPosPtr )
    {
      /* Found an open parenthesis. */
      case '(':
      {
        /* Increment position. */
        ExecRecPtr->wPosition = ExecRecPtr->wPosition + sizeof( char );
        CurrPosPtr++; 
        
        /* Increment open parenthesis counter. */
        ExecRecPtr->nOpenParen = ExecRecPtr->nOpenParen + 1;
      
        break;
      }
    

      /* Found a closing parenthesis. */
      case ')':
      {
        /* Increment position. */
        ExecRecPtr->wPosition = ExecRecPtr->wPosition + sizeof( char );
        CurrPosPtr++; 
        
        /* Decrement open parenthesis counter. */
        ExecRecPtr->nOpenParen = ExecRecPtr->nOpenParen - 1;
      
        break;
      }
    

      /* Found character that separates commands. */
      case CMDSEP_CHAR_TOKEN:
      {
        /* Thru all parenthesis groups? */
        if( ExecRecPtr->nOpenParen == 0 )
        {
          /* At end of current command. */
          bFoundNextCmnd = TRUE;
        }
          
        /* Increment position. */
        ExecRecPtr->wPosition = ExecRecPtr->wPosition + sizeof( char );
        CurrPosPtr++; 
        
        break;
      }


      /* Found an unimportant character. */
      default:
      {
        /* Increment position. */
        ExecRecPtr->wPosition = ExecRecPtr->wPosition + sizeof( char );
        CurrPosPtr++; 
      
        break;
      }

    }
  }

  /* No more commands? */
  if( bFoundNextCmnd == FALSE || *CurrPosPtr == '\0' )
  {
    /* Unlock the macro string. */
    GlobalUnlock( ExecRecPtr->hMacroString );
  
    /* Failure. */
    return( FALSE );
  }
  
  /* Find end of command string. */  
  CmdEndPtr = _fstrchr( CurrPosPtr, '(' );

  /* Parsing error. */  
  if( CmdEndPtr == NULL )
  {
    /* Unlock the macro string. */
    GlobalUnlock( ExecRecPtr->hMacroString );
  
    /* Parsing error. */ 
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_MACROPARSINGERROR, MB_ICONHAND | MB_OK );

    /* Failure. */
    return( FALSE );
  }

  /* Terminate end of the command found. */
  *CmdEndPtr = '\0';
  
  /* Have one open parenthesis. */
  ExecRecPtr->nOpenParen = 1;

  /* Get the call-back function associated with the command. */
  bReturn = GetCommandInfo( CurrPosPtr, MacroEnginePtr->hLoadTableStack, MacroFuncDblPtr, 
                            hMacroDataPtr );
  
  /* Failed to find command's data. */
  if( bReturn == FALSE )
  {
    /* Unlock the macro string. */
    GlobalUnlock( ExecRecPtr->hMacroString );
  
    /* Parsing error. */ 
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_MACROPARSINGERROR, MB_ICONHAND | MB_OK );

    /* Failure. */
    return( FALSE );
  }

  /* Set the next read position. */
  wCmdLen = _fstrlen( CurrPosPtr ) + sizeof( char );
  ExecRecPtr->wPosition = ExecRecPtr->wPosition + wCmdLen;
  
  /* Unlock the macro string. */
  GlobalUnlock( ExecRecPtr->hMacroString );

  /* Success. */
  return( TRUE );
}



/***********************************
**
**  Find a command in one of the
**  loaded tables and get the
**  data asssociated with it.
**
***********************************/
static BOOL __far __pascal GetCommandInfo
( 
  char __far * szMacroCommand,
  HGLOBAL hLoadTableStack, 
  MACROFUNCPTR __far * MacroFuncDblPtr, 
  HGLOBAL __far * hMacroDataPtr 
)
{
  GETCOMMANDINFODATA Data;
  
  /* Init. search data. */
  Data.bFound         = FALSE;
  Data.szMacroCommand = szMacroCommand;
  
  /* Use GetCommandInfoEx1() to look through the loaded macro tables. */
  EnumLoadTableStack( hLoadTableStack, GetCommandInfoEx, (LPARAM) (GETCOMMANDINFODATAPTR) &Data );

  /* If we found the command's info. */
  if( Data.bFound == TRUE )
  {
    *MacroFuncDblPtr = Data.MacroFuncPtr;
    *hMacroDataPtr   = Data.hMacroData;
  }
  
  /* Success or Failure */
  return( Data.bFound );
}


/***********************************
**
**  Search for a registered macro
**  table list with the specified
**  name.
**
***********************************/
static BOOL __far __pascal GetCommandInfoEx( HLOADEDTABLEREC hLoadedTableRec, LPARAM lParam )
{
  LOADEDTABLERECPTR LoadedTableRecPtr;
  GETCOMMANDINFODATAPTR DataPtr;
  
  /* Get search criteria. */
  DataPtr = (GETCOMMANDINFODATAPTR) lParam;

  /* Lock the loaded table record. */
  LoadedTableRecPtr = (LOADEDTABLERECPTR) GlobalLock( hLoadedTableRec );
    
  DataPtr->bFound = FindRegTableMacro( LoadedTableRecPtr->hRegTableRec, DataPtr->szMacroCommand,
                                       &(DataPtr)->MacroFuncPtr, &(DataPtr)->hMacroData );
  /* Unlock the loaded table record. */
  GlobalUnlock( hLoadedTableRec );
  
  /* If we didn't found it keep enumerating. */
  return( ! DataPtr->bFound );
}  



/***********************************
**
**  Get the next argument from 
**  the top macro string. 
**
***********************************/
BOOL __far __pascal __export GetNextMacroArg 
(
  HWND hErrorWnd,                /* Parent window for error message box. */
  HMACROENGINE hMacroEngine,     /* Handle to engine. */
  void __far * DataPtr,          /* Place in which to save data. */
  MACROARGTYPE MacroArgType      /* Type of data that it is. */
)
{
  MACROENGINEPTR MacroEnginePtr; /* Pointer to the macro engine's instance data. */

  EXECRECPTR ExecRecPtr;      /* Pointer to the macro string currently being executed. */

  char __far * ArgStartPtr;   /* Pointer to the current position in the macro string. */
  char __far * CurrPosPtr;    /* Pointer to the current position in the macro string. */

  WORD wCmdLen;               /* Length of the command. */

  BOOL bFoundArgEnd;

  short int nEmbeddedOpenParen;
  
  /* In case of hex conversion. */
  unsigned long int nHexConvert;
  
  /* Pointers to the data saving area. */
  char __far *      CharPtr;
  WORD __far *      WordPtr;
  DWORD __far *     DWordPtr;
  short int __far * IntPtr; 
  long int __far *  LongPtr; 
  BOOL __far *      BOOLPtr; 
  
   
  /* Lock engine data. */
  MacroEnginePtr = (MACROENGINEPTR) GlobalLock( hMacroEngine );
  
  /* Lock the execution stack's top record. */
  ExecRecPtr = (EXECRECPTR) GlobalLock( MacroEnginePtr->hExecutionStack );
      
  /* Lock the macro string. */
  CurrPosPtr = (char __far *) GlobalLock( ExecRecPtr->hMacroString );

  /* Move to current position in macro string. */
  CurrPosPtr = CurrPosPtr + ExecRecPtr->wPosition;
  
  /* Haven't found the end of argument yet. */
  bFoundArgEnd = FALSE;
  
  /* Skip leading white space. */
  while( *CurrPosPtr == ' ' ) 
  { 
    ExecRecPtr->wPosition = ExecRecPtr->wPosition + sizeof( char );
    CurrPosPtr++;
  }
  
  /* Save starting position of argument. */
  ArgStartPtr = CurrPosPtr;
  
  /* No open parenthesis. */                            
  nEmbeddedOpenParen = 0;

  /* Go to end of argument. */
  while( *CurrPosPtr != '\0' && bFoundArgEnd == FALSE )
  {
    switch( *CurrPosPtr )
    {
      /* Found an open parenthesis. */
      case '(':
      {
        /* Increment position. */
        CurrPosPtr++; 
        
        /* Increment open parenthesis counters. */
        ExecRecPtr->nOpenParen = ExecRecPtr->nOpenParen + 1;
        nEmbeddedOpenParen = nEmbeddedOpenParen + 1;
        break;
      }
    

      /* Found a closing parenthesis - may be end of argument. */
      case ')':
      {
         /* Decrement open parenthesis counter. */
        ExecRecPtr->nOpenParen = ExecRecPtr->nOpenParen - 1;
        nEmbeddedOpenParen = nEmbeddedOpenParen - 1;
      
       /* Thru all parenthesis groups at this level? */
        if( ExecRecPtr->nOpenParen == 0 )
        {
          /* At end of current argument. */
          bFoundArgEnd = TRUE;
          break;
        }
          
        /* Increment position. */
        CurrPosPtr++; 

        break;
      }
    

      /* Found character that separate arguments. */
      case ',':
      {
        /* Thru all embedded parenthesis in the current argument? */
        if( nEmbeddedOpenParen == 0 )
        {
          /* At end of current argument. */
          bFoundArgEnd = TRUE;
          break;
        }
          
        /* Increment position. */
        CurrPosPtr++; 

        break;
      }


      /* Found an unimportant character. */
      default:
      {
        /* Increment position. */
        CurrPosPtr++; 
      
        break;
      }

    }
  }

  /* Did not find end of argument? */
  if( bFoundArgEnd == FALSE || *CurrPosPtr == '\0' )
  {
    /* Unlock the macro string. */
    GlobalUnlock( ExecRecPtr->hMacroString );
  
    /* Parsing error. */ 
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_MACROPARSINGERROR, MB_ICONHAND | MB_OK );

    /* Failure. */
    return( FALSE );
  }
  
  /* Terminate end of the argument found. */
  *CurrPosPtr = '\0';
  
  /* Set the next read position. */
  wCmdLen = _fstrlen( ArgStartPtr ) + sizeof( char );
  ExecRecPtr->wPosition = ExecRecPtr->wPosition + wCmdLen;
  
  
  /* Save argument into the buffer. */
  switch( MacroArgType )
  {
    /* Argument is a boolean value. */
    case MACRO_BOOL:
    {
      /* Cast void data pointer. */
      BOOLPtr = (BOOL __far * ) DataPtr;

      /* If its hex, convert it first. */
      if( HexConvert( ArgStartPtr, &nHexConvert ) )
      {
        /* Save to data pointer. */
        *BOOLPtr = (BOOL) nHexConvert;
      }
      
      else
      {
        /* Convert the string. */
        *BOOLPtr = (BOOL) atoi( ArgStartPtr );
      }
      
      break;
    }


    /* Argument is an signed long int. */
    case MACRO_SIGNEDLONGINT:
    {
      /* Cast void data pointer. */
      LongPtr = (long int __far * ) DataPtr;

      /* If its hex, convert it first. */
      if( HexConvert( ArgStartPtr, &nHexConvert ) )
      {
        /* Save to data pointer. */
        *LongPtr = (long int) nHexConvert;
      }
      
      else
      {
        /* Convert the string. */
        *LongPtr = atol( ArgStartPtr );
      }

      break;
    }


    /* Argument is an signed short int. */
    case MACRO_SIGNEDSHORTINT:
    {
      /* Cast void data pointer. */
      IntPtr = (short int __far * ) DataPtr;

      /* If its hex, convert it first. */
      if( HexConvert( ArgStartPtr, &nHexConvert ) )
      {
        /* Save to data pointer. */
        *IntPtr = (short int) nHexConvert;
      }
      
      else
      {
        /* Convert the string. */
        *IntPtr = atoi( ArgStartPtr );
      }

      break;
    }


    /* Argument is an unsigned long int. */
    case MACRO_UNSIGNEDLONGINT:
    {
      /* Cast void data pointer. */
      DWordPtr = (DWORD __far * ) DataPtr;

      /* If its hex, convert it first. */
      if( HexConvert( ArgStartPtr, &nHexConvert ) )
      {
        /* Save to data pointer. */
        *DWordPtr = (DWORD) nHexConvert;
      }
      
      else
      {
        /* Convert the string. */
        *DWordPtr = (DWORD) atol( ArgStartPtr );
      }

      break;
    }


    /* Argument is an unsigned short int. */
    case MACRO_UNSIGNEDSHORTINT:
    {
      /* Cast void data pointer. */
      WordPtr = (WORD __far * ) DataPtr;

      /* If its hex, convert it first. */
      if( HexConvert( ArgStartPtr, &nHexConvert ) )
      {
        /* Save to data pointer. */
        *WordPtr = (WORD) nHexConvert;
      }
      
      else
      {
        /* Convert the string. */
        *WordPtr = (WORD) atoi( ArgStartPtr );
      }

      break;
    }


    /* Argument is a quoted string - quotes will be removed. */
    case MACRO_STRING:
    {
      /* First char should be a quote or tilde char. */
      if( *ArgStartPtr != '\"' && *ArgStartPtr != '`')
      {
        /* Unlock the macro string. */
        GlobalUnlock( ExecRecPtr->hMacroString );
      
        /* Parsing error. */ 
        MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_MACROPARSINGERROR, MB_ICONHAND | MB_OK );
    
        /* Failure. */
        return( FALSE );
      }
      else
      {
        /* Skip first quote char or tilde char. */
        ArgStartPtr =  ArgStartPtr + sizeof( char );
      }
      
      /* Point to last char in the string - skipping blank space. */
      CurrPosPtr = CurrPosPtr - sizeof( char );
      while( *CurrPosPtr == ' ' )
      {
        CurrPosPtr - sizeof( char );
      }

      /* Last char should be a quote or apostrophe char. */
      if( *CurrPosPtr != '\"' && *CurrPosPtr != '\'')
      {
        /* Unlock the macro string. */
        GlobalUnlock( ExecRecPtr->hMacroString );
      
        /* Parsing error. */ 
        MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_MACROPARSINGERROR, MB_ICONHAND | MB_OK );
    
        /* Failure. */
        return( FALSE );
      }
      else
      {
        /* Trim last quote char. */
        *CurrPosPtr = '\0';
      }

      /* Copy argument into string buffer. */
      CharPtr = (char __far *) DataPtr;
      _fstrcpy( CharPtr, ArgStartPtr );

      break;
    }
  
    /* Return argument as a string. */
    default:
    {
      /* Copy argument into string buffer. */
      CharPtr = (char __far *) DataPtr;
      _fstrcpy( CharPtr, ArgStartPtr );
    }
  }


  /* Unlock the macro string. */
  GlobalUnlock( ExecRecPtr->hMacroString );

  /* Unlock engine data. */
  GlobalUnlock( hMacroEngine );
  
  /* Success. */
  return( TRUE );
}


/***********************************
**
**  Checks to see if the argument
**  is in hex format and if it
**  is translates the number to
**  an unsigned long int.
**
**  Returns:
**
**      TRUE:  Number was hex and converted.
**      FALSE: Number was not in hex.
**
***********************************/
static BOOL __far __pascal HexConvert( char __far * ArgPtr, long unsigned int __far * nHexConvert )
{
  if( *ArgPtr == '0' && ( *(ArgPtr + sizeof(char) ) == 'x' ||  *(ArgPtr + sizeof(char) ) == 'X' ) )
  {
    /* If its hex, convert it first. */
    *nHexConvert = strtoul( ArgPtr, NULL, 0 );
  
    /* Was in hex. */
    return( TRUE );
  }

  /* Was not in hex. */
  return( FALSE );
}      



/**************************** DEFAULT MACROS IMPLEMENTATION FUNCTIONS ***********************/


/***********************************
**
**  Macro that adds the specified
**  table to the top of the
**  macro table list.
**
**  It will be the first table that is
**  considered when executing a macro.
**
***********************************/
static BOOL __far __pascal MacroLoadMacroTable
( 
  HMACROENGINE hMacroEngine,
  HGLOBAL hAppData,
  HGLOBAL hMacroData,
  MACRORETVAL __far * MacroRetValPtr
)
{
  /* Success. */
  return( TRUE );
}


/***********************************
**
**  Macro that removes the first instance of 
**  the table from the macro table list.
**
**  Other instances of the table will
**  not be removed.
**
**  If no instance of the table exists,
**  no action is performed.
**
***********************************/
static BOOL __far __pascal MacroUnloadMacroTable
( 
  HMACROENGINE hMacroEngine,
  HGLOBAL hAppData,
  HGLOBAL hMacroData,
  MACRORETVAL __far * MacroRetValPtr
)
{
  /* Success. */
  return( TRUE );
}


/***********************************
**
**  Remove the first instance of the
**  table from the macro table list.
**
**  Other instances of the table will
**  not be removed.
**
**  If no instance of the table exists,
**  no action is performed.
**
***********************************/
static BOOL __far __pascal MacroExecuteMacroFile
( 
  HMACROENGINE hMacroEngine,
  HGLOBAL hAppData,
  HGLOBAL hMacroData,
  MACRORETVAL __far * MacroRetValPtr
)
{
  /* Success. */
  return( TRUE );
}
