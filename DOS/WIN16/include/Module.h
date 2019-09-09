/*
	@(#)Module.h	2.10
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

*/
 
#ifndef Module__h
#define Module__h

#include "ModTable.h"
#include "ObjEngine.h"

/* **** InternalLoadLibrary flags **** */

#define ILL_APPL	0x0001
#define ILL_DSCR	0x0002
#define ILL_BINARY	0x0004
#define ILL_FILENAME	0x0008
#define ILL_NOERROR	0x0010
#define ILL_NOLOAD	0x0020
#define ILL_EXEC	0x0040
#define ILL_RESOURCE	0x0080
#define ILL_INTERMED	0x0100
#define ILL_FINAL	0x0200
#define ILL_NOFORK	0x0400
#define ILL_NEWINST	0x0800
#define ILL_SHELL  	0x1000

#define BUFFER 0x200

typedef struct tagUSERDLL
  {
	HFILE	hfExeFile;
	WORD	wSelectorBase;
	LPBYTE	lpHeaderImage;
	LPBYTE	lpEntryTable;
	LPBYTE	lpImportTable;
	LPBYTE	lpSegmentTable;
	LPWORD	lpModuleTable;
	LPBYTE	lpNonResTable;
	DWORD	dwNEOffset;
	DWORD	dwResPart;
	WORD	wProgramFlags;
	BYTE	bAdditFlags;
	WORD	wFLOffset;
	WORD	wFLLength;
	WORD	wAutoData;
	WORD	wInitHeap;
	WORD	wInitStack;
	WORD	wInitCS;
	WORD	wInitIP;
	WORD	wInitSS;
	WORD	wInitSP;
	WORD	wStackBottom;
	WORD	wSegCount;
	WORD	wSegOffset;
	WORD	wModCount;
	WORD	wModOffset;
	WORD	wEntryTblSize;
	WORD	wEntryTblOffset;
	WORD	wResourceTblOffset;
	WORD	wNResTblSize;
	DWORD	dwNResTblOffset;
	WORD	wResTblOffset;
	WORD	wImpTblOffset;
	WORD    wMovCount;
	WORD	wSectorShift;
	WORD	wTargetOS;
	WORD	wWinVer;
	LPHANDLE lpModIndex;
	WORD	wNumEntries;
	LPSTR	lpDLLName;
	LPSTR	lpDLLDescription;
  } USERDLL;

typedef	USERDLL *LPUSERDLL;

#define START_PARA      0x0010
#define	__ROMBIOS	0x00ad
#define __WINFLAGS      0x00b2
#define __AHSHIFT       0x0071
#define __AHINCR        0x0072
#define __C000H		0x00c3

#define	__C000H_BY_NAME	"__C000H"

#define LEXE_exit(x) {LEXE_perror(x);_lclose(lpDLL->hfExeFile); return(-x);}

/* Module information structure */
typedef struct tagMODULEINFO
  {
    OBJHEAD	ObjHead;		/* generic object header */
    LPSTR	lpModuleName;		/* ptr to the program name */
    LPSTR	lpFileName;
    LPSTR	lpModuleDescr;		/* module description string */
    TYPEINFO	*ResourceTable;
    ENTRYTAB	*lpEntryTable;
    SEGTAB	*lpSegTable;		/* segment table for LOADONCALL */
    LPHANDLE	lpModIndex;
    USERDLL	*lpDLL;
    ATOM	atmModuleName;
    WORD	wDGROUP;		/* selector assigned to DGROUP */
    HANDLE	hInst;			/* global handle of DGROUP */
    WORD	wSegCount;		/* number of segments in module */
    WORD	wSelBase;		/* first sel assigned to module */
    WORD	wProgramFlags;
    WORD	wSectorShift;
    BYTE	bModIndex;		/* index into module table */
    BOOL	fMakeInstance;		/* when "reloading", make a new inst */
  } MODULEINFO;
 
typedef MODULEINFO *LPMODULEINFO;

typedef struct tagPARAMBLOCK {
    WORD wSegEnv;
    LPSTR lpszCmdLine;
    UINT *lpShow;
    UINT *lpReserved;
 } PARAMBLOCK;

/* SEGMENTINFO struct for utility function GetSegmentInfo() */
typedef struct tagSEGMENTINFO {
    LPSTR lpModuleName;
    LPSTR lpFileName;
    int nSegmentOrd;
    WORD wSegType;
} SEGMENTINFO;
    
#endif /* Module__h */
