/*
	@(#)Meta.h	2.9
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
 
#ifndef Meta__h
#define Meta__h


#define WIN_VERSION		0x300

#define HANDLE_TABLE_SIZE	128

#define	MEMORY_METAFILE		0
#define	DISK_METAFILE		1
#define EXT_HEADER_KEY		0x9ac6cdd7
#define	NOT_FOUND		-1
#define	NEED_TO_DELETE		-2

typedef	 LONG (* METACREATEOBJPROC)(HGDIOBJ, LPVOID);

/* Functions of ControlObjTable() */
#define	INIT_OBJ_TBL		0
#define	DELETE_OBJ_TBL		1
#define	GET_FREE_SLOT		2
#define	FIND_OBJ		3
#define	FREE_OBJ		4
#define	STORE_OBJ		5
#define	CHECK_HANDLE		6

/* Number of WORDs(indice) in METAHEADER structure */
#define	MF_TYPE			0	/* mtType		*/
#define	MF_HEADERSIZE		1	/* mtHeaderSize		*/
#define	MF_VERSION		2	/* mtVersion		*/
#define	MF_SIZE			3	/* mtSize		*/
#define	MF_NOOBJECTS		5	/* mtNoObjects		*/
#define	MF_MAXRECORD		6	/* mtMaxRecord		*/
#define	MF_NOPARAMS		8	/* mtNoParameters	*/
#define	MF_RECORD		9	/* metafile records	*/

/* Some indices in METARECORD structure */
#define	RDSIZE			0
#define	RDFUNC			2
#define	RDPARAM			3

typedef struct tagMETAOBJTABLE {
    HGDIOBJ	hObj;
    union {
	LOGPEN		pen;
	LOGBRUSH	brush;
	LOGFONT		font;
    } obj;
} METAOBJTABLE, *LPMETAOBJTABLE;

typedef struct tagINTERNALMETAFILE {
	BOOL		(*lpfRecorder)(struct tagINTERNALMETAFILE *,LPWORD);
	LPMETAOBJTABLE	lpObjTable;
	int		fd;
	HGLOBAL		hMetaMem;
	METAHEADER	header;		/* Native METAHEADER */
	char		szFullPath[_MAX_PATH];
} INTERNALMETAFILE;
typedef INTERNALMETAFILE*	LPINTERNALMETAFILE;

typedef struct tagDISKMFINTERNAL {
	METAHEADER	header;
	BYTE		bPlaceable;
	char		szFullPath[_MAX_PATH];
} DISKMFINTERNAL;
typedef DISKMFINTERNAL*	LPDISKMFINTERNAL;


/* MFRECORD superseeds binary METARECORD structure, */
/* and contains binary metafile record. (We cannot  */
/* use METARECORD, because of UINT.)   Actually,    */
/* sizeof(MFRECORD) = sizeof(METARECORD).           */
typedef struct tagMFRECORD {
	DWORD	rdSize;
	WORD	rdFunction;
	WORD	rdParm[1];
} MFRECORD;
typedef	MFRECORD*	LPMFRECORD;

/* Metafile structure is created by CloseMetaFile() */
/* GetMetaFile functions. (METAHEADER)              */
typedef struct tagMETAFILE {
	WORD	mtType;
	WORD	mtHeaderSize;
	WORD	mtVersion;
	DWORD	mtSize;
	WORD	mtNoObjects;
	DWORD	mtMaxRecord;
	WORD	mtNoParameters;
	WORD	mf[1];		/* Binary metafile itself starts here */
} METAFILE;
typedef	METAFILE*	LPMETAFILE;

#define	METAFILEHEADER_SIZE	22
#define	METAHEADER_SIZE		18

BOOL MemoryMetaFileRecorder(LPINTERNALMETAFILE, LPWORD);
BOOL DiskMetaFileRecorder(LPINTERNALMETAFILE, LPWORD);
LONG TWIN_mf_CreateBrushIndirect(HDC, LPVOID);
LONG TWIN_mf_CreateFontIndirect(HDC, LPVOID);
LONG TWIN_mf_CreatePenIndirect(HDC, LPVOID);
LONG TWIN_mf_CreatePalette(HDC, LPVOID);
LONG mf_DeleteObject(HDC, HGDIOBJ);
int TWIN_GetObjInfo(HANDLE, METACREATEOBJPROC *, LPVOID*);
int TWIN_ControlObjTable(LPINTERNALMETAFILE,int, HGDIOBJ);
void TWINInitHandleTable(LPHANDLETABLE, int);
int TWIN_MetaFindFreeSlot(LPHANDLETABLE, int);

#endif /* Meta__h */


