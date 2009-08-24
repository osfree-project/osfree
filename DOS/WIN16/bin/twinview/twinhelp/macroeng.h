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
#ifndef MACROENG_H
#define MACROENG_H

/***********************************
**
**  System Includes
**
***********************************/
#include <windows.h>


/***********************************
**
**  App. Includes
**
***********************************/


/***********************************
**
**  Shared Defines
**
***********************************/

/* Hnadle to an instance of the macro engine. */
typedef HGLOBAL HMACROENGINE;

/* Return value of a macro. */
typedef long int MACRORETVAL;
 
/* Macro implementation callback function. */
typedef BOOL ( CALLBACK * MACROFUNCPTR )( HMACROENGINE hMacroEngine, HGLOBAL hAppData, HGLOBAL hMacroData, MACRORETVAL __far * MacroRetValPtr );

/* 
** A macro table entry contains information
** for one macro that is in a macro table.
*/
typedef struct tagMACROTABLEENTRY
{
  char __far * MacroNamePtr;
  MACROFUNCPTR MacroFuncPtr;
  HGLOBAL      hMacroData;
} 
MACROTABLEENTRY;
typedef MACROTABLEENTRY __far * MACROTABLEENTRYPTR;

typedef enum 
{
  MACRO_NOCONVERT,            /* Argument is a string without quotes - return all. */
  MACRO_STRING,               /* Argument is a quoted string - quotes will be removed. */
  MACRO_UNSIGNEDSHORTINT,     /* UINT, WORD, WPARAM */
  MACRO_UNSIGNEDLONGINT,      /* DWORD */
  MACRO_SIGNEDSHORTINT,       /* BOOL */
  MACRO_SIGNEDLONGINT,        /* LONG, LPARAM, LRESULT */
  MACRO_BOOL                  /* Value of 1 or 0. */
}
MACROARGTYPE;


/* 
** Maximum length of a macro string 
** including the terminating NULL character.
*/
#define MAX_MACRO_STRING    513

/* 
** Maximum length of a macro table's name 
** including the terminating NULL character. 
*/
#define MAX_TABLE_NAME      30

/* This table name is reserved and cannot be used. */
#define RESERVED_TABLE_NAME  "RESERVED_TABLE_NAME"

/* 
** This table name can be registered and loaded and
** contains default system macros. 
*/
#define SYSTEM_TABLE_NAME  "SYSTEM_TABLE_NAME"


/***********************************
**
**  Shared Function Prototypes
**
***********************************/

HMACROENGINE __far __pascal __export StartMacroEngine( HWND hErrorWnd, HGLOBAL hAppData );
void __far __pascal __export StopMacroEngine( HMACROENGINE __far * hMacroEnginePtr );
HGLOBAL __far __pascal __export GetMacroEngineAppData( HMACROENGINE hMacroEngine );

BOOL __far __pascal __export RegisterMacroTable
( 
  HWND hErrorWnd,
  HMACROENGINE hMacroEngine,
  char __far * szTableName, 
  WORD wNumMacros,
  MACROTABLEENTRYPTR MacroTableEntryPtr
);

BOOL __far __pascal __export UnregisterMacroTable
( 
  HMACROENGINE hMacroEngine,
  char __far * szTableName
);


BOOL __far __pascal __export LoadMacroTable
( 
  HWND hErrorWnd,
  HMACROENGINE hMacroEngine,
  char __far * szTableName 
);

BOOL __far __pascal __export UnloadMacroTable
( 
  HMACROENGINE hMacroEngine,
  char __far * szTableName 
);

BOOL __far __pascal __export ExecuteMacro
( 
  HWND hErrorWnd,
  HMACROENGINE hMacroEngine,
  char __far * MacroStringPtr,
  MACRORETVAL __far * MacroRetValPtr
);

BOOL __far __pascal __export GetNextMacroArg 
(
  HWND hErrorWnd,                /* Parent window for error message box. */
  HMACROENGINE hMacroEngine,     /* Handle to engine. */
  void __far * DataPtr,          /* Place in which to save data. */
  MACROARGTYPE MacroArgType      /* Type of data that it is. */
);


#endif




