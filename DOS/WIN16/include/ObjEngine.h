/*
	@(#)ObjEngine.h	2.21
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
 
#ifndef ObjEngine__h
#define ObjEngine__h

/* General object header structure */
typedef struct tagOBJHEAD
{
    HANDLE	hObj;			/* object handle */
    WORD	wObjSignature;		/* object signature */
    WORD	wRefCount;		/* reference count */
    struct tagOBJHEAD	*lpObjNext;	/* pointer to next */
} OBJHEAD;
typedef OBJHEAD *LPOBJHEAD;

/* General object tag structure */
typedef struct tagOBJTAG
{
    LPSTR       lpObjInfo;
    HANDLE      hObj;
    WORD	wFlags;
} OBJTAG;
typedef OBJTAG *LPOBJTAG;

/* Handle-objects engine entry prototype */
LPOBJHEAD HandleObj(int, WORD, LPVOID);

DWORD TWIN_CheckObject(int);

/* Handle-objects engine function definitions */
#define HM_INIT         0
#define HM_GETHND       1
#define HM_GETOBJ       2
#define HM_FREEHND      3
#define HM_LOCKHND	4
#define HM_UNLOCKHND	5
#define HM_CHECKHND	7
#define HM_DUMPTABLE	6

#define	OE_BUSY		0x8000

/* Handle-objects engine object categories */
#define	CT_GDI		'G'
#define	CT_KERNEL	'K'
#define	CT_USER		'U'

/* Handle-objects engine object types */
#define	OT_MODULE	'M'
#define	OT_TASK		'T'
#define OT_HANDLE	'H'

#define	OT_BRUSH	'B'
#define	OT_PEN		'P'
#define	OT_FONT		'F'
#define	OT_BITMAP	'T'
#define	OT_ICON		'I'
#define	OT_REGION	'R'
#define	OT_PALETTE	'L'
#define OT_METAFILE	'M'
#define OT_CURSOR	'C'
#define	OT_DC		'D'

#define	OT_WINDOW	'W'
#define	OT_WINPOS	'P'
#define	OT_LBINFO	'L'
#define	OT_MENU		'M'
#define	OT_SBINFO	'S'
#define OT_ATOM		'A'

#define	OM_MASK		0x4000

#define	MAKESIGNATURE(c,t)	(WORD)(((c) << 8) + (t))

#define	GET_OBJECT_SIGNATURE(lpObjHead)	(lpObjHead->wObjSignature)
#define	GET_OBJECT_TYPE(lpObjHead)	(LOBYTE((lpObjHead->wObjSignature)))
#define	GET_OBJECT_CATEGORY(lpObjHead)	(HIBYTE((lpObjHead->wObjSignature)))

/* General check object handle macro */
#define	CHECKOBJ(h,c,t)	HandleObj(HM_GETOBJ,MAKESIGNATURE(c,t),(LPVOID)((DWORD)h))

#define MLTASK		MAKESIGNATURE(CT_KERNEL,OT_TASK)
#define MLMODULE	MAKESIGNATURE(CT_KERNEL,OT_MODULE)
#define MLHANDLE	MAKESIGNATURE(CT_KERNEL,OT_HANDLE)
#define MLATOM		MAKESIGNATURE(CT_KERNEL,OT_ATOM)

#define MLPEN		MAKESIGNATURE(CT_GDI,OT_PEN)
#define MLBRUSH		MAKESIGNATURE(CT_GDI,OT_BRUSH)
#define MLPALETTE	MAKESIGNATURE(CT_GDI,OT_PALETTE)
#define	MLGDI		MAKESIGNATURE(CT_GDI,0L)
#define MLFONT		MAKESIGNATURE(CT_GDI,OT_FONT)
#define MLREGION	MAKESIGNATURE(CT_GDI,OT_REGION)
#define MLICON		MAKESIGNATURE(CT_GDI,OT_ICON)
#define MLBITMAP	MAKESIGNATURE(CT_GDI,OT_BITMAP)
#define MLMETAFILE	MAKESIGNATURE(CT_GDI,OT_METAFILE)
#define MLCURSOR	MAKESIGNATURE(CT_GDI,OT_CURSOR)
#define MLDC		MAKESIGNATURE(CT_GDI,OT_DC)

#define	MLWIN		MAKESIGNATURE(CT_USER,OT_WINDOW)
#define	MLPOS		MAKESIGNATURE(CT_USER,OT_WINPOS)
#define	MLSBAR		MAKESIGNATURE(CT_USER,OT_SBINFO)
#define MLLBOX		MAKESIGNATURE(CT_USER,OT_LBINFO)

#ifdef TWIN_THREAD_SAFE
#define RELEASEINFO(h32) 						\
			{ 						\
			    if (((LPOBJHEAD) (h32))->wRefCount > 0) 	\
				((LPOBJHEAD) (h32))->wRefCount--;	\
			    (h32) = NULL;                               \
			}
#else
#define RELEASEINFO(h32) { }
#endif

#define RELEASECLASSINFO(h32) RELEASEINFO(h32)

#define	CREATEHWIN(h) (LPWININFO) \
			HandleObj(HM_GETHND,MLWIN,(LPVOID)(&(h)))
#define	GETWININFO(h) (LPWININFO) \
			HandleObj(HM_GETOBJ,MLWIN,(LPVOID)((DWORD)h))
#define	CHECKWININFO(h) (LPWININFO) \
			HandleObj(HM_CHECKHND,MLWIN,(LPVOID)((DWORD)h))
#define RELEASEWININFO(h32) RELEASEINFO(h32)
#define	RELEASEHWND(h) \
			HandleObj(HM_UNLOCKHND,MLWIN,(LPVOID)((DWORD)h))
#define	FREEHWND(h) HandleObj(HM_FREEHND,MLWIN,(LPVOID)((DWORD)h))

#define	CREATEHPOS(h) (LPMULTIWINPOS) \
			HandleObj(HM_GETHND,MLPOS,(LPVOID)(&(h)))
#define	GETPOSINFO(h) (LPMULTIWINPOS) \
			HandleObj(HM_GETOBJ,MLPOS,(LPVOID)((DWORD)h))
#define RELEASEPOSINFO(h32) RELEASEINFO(h32)
#define	RELEASEHPOS(h) \
			HandleObj(HM_UNLOCKHND,MLPOS,(LPVOID)((DWORD)h))
#define	FREEHPOS(h) HandleObj(HM_FREEHND,MLPOS,(LPVOID)((DWORD)h))

#define	CREATESBAR(h) (LPTWINSCROLLINFO)HandleObj(HM_GETHND,MLSBAR,(LPVOID)(&(h)))
#define	GETSBARINFO(h) (LPTWINSCROLLINFO)HandleObj(HM_GETOBJ,MLSBAR,(LPVOID)((DWORD)h))
#define RELEASESBARINFO(h32) RELEASEINFO(h32)
#define	RELEASESBAR(h) \
			HandleObj(HM_UNLOCKHND,MLSBAR,(LPVOID)((DWORD)h))
#define	FREESBAR(h) HandleObj(HM_FREEHND,MLSBAR,(LPVOID)((DWORD)h))

#define CREATELBOX(h) (LPLISTBOXINFO)HandleObj(HM_GETHND,MLLBOX,(LPVOID)(&(h)))
#define GETLBOXINFO(h) (LPLISTBOXINFO)HandleObj(HM_GETOBJ,MLLBOX,(LPVOID)((DWORD)h))
#define RELEASELBOXINFO(h32) RELEASEINFO(h32)
#define RELEASELBOX(h) \
			HandleObj(HM_UNLOCKHND,MLLBOX,(LPVOID)((DWORD)h))
#define CHECKLBOXINFO(h) (LPLISTBOXINFO)HandleObj(HM_CHECKHND,MLLBOX,(LPVOID)((DWORD)h))
#define FREELBOX(h) HandleObj(HM_FREEHND,MLLBOX,(LPVOID)((DWORD)h))

#define CREATEHTASK(h) (LPTASKINFO) \
			HandleObj(HM_GETHND,MLTASK,(LPVOID)(&(h)))
#define GETTASKINFO(h) (LPTASKINFO) \
			HandleObj(HM_GETOBJ,MLTASK,(LPVOID)((DWORD)h))
#define RELEASETASKINFO(h32) RELEASEINFO(h32)
#define RELEASEHTASK(h) \
			HandleObj(HM_UNLOCKHND,MLTASK,(LPVOID)((DWORD)h))
#define FREEHTASK(h) HandleObj(HM_FREEHND,MLTASK,(LPVOID)((DWORD)h))

#define CREATEHMODULE(h) (LPMODULEINFO) \
			HandleObj(HM_GETHND,MLMODULE,(LPVOID)(&(h)))
#define GETMODULEINFO(h) (LPMODULEINFO) \
			HandleObj(HM_CHECKHND,MLMODULE,(LPVOID)((DWORD)h))
#define RELEASEMODULEINFO(h32) RELEASEINFO(h32)
#define RELEASEHMODULE(h) \
			HandleObj(HM_UNLOCKHND,MLMODULE,(LPVOID)((DWORD)h))
#define	CHECKMODULEINFO(h) (LPMODULEINFO) \
			HandleObj(HM_CHECKHND,MLMODULE,(LPVOID)((DWORD)h))
#define FREEHMODULE(h) HandleObj(HM_FREEHND,MLMODULE,(LPVOID)((DWORD)h))

#define CREATEHANDLE(h) (LPHANDLEINFO) \
			HandleObj(HM_GETHND,MLHANDLE,(LPVOID)(&(h)))
#define GETHANDLEINFO(h) (LPHANDLEINFO) \
			HandleObj(HM_GETOBJ,MLHANDLE,(LPVOID)((DWORD)h))
#define RELEASEHANDLEINFO(h32) RELEASEINFO(h32)
#define RELEASEHANDLE(h) \
			HandleObj(HM_UNLOCKHND,MLHANDLE,(LPVOID)((DWORD)h))
#define FREEHANDLE(h) HandleObj(HM_FREEHND,MLHANDLE,(LPVOID)((DWORD)h))

#define CREATEATOM(h) (LPHATOMINFO) \
			HandleObj(HM_GETHND,MLATOM,(LPVOID)(&(h)))
#define GETATOMINFO(h) (LPHATOMINFO) \
			HandleObj(HM_GETOBJ,MLATOM,(LPVOID)((DWORD)h))
#define RELEASEATOMINFO(h32) RELEASEINFO(h32)
#define RELEASEATOM(h) \
			HandleObj(HM_UNLOCKHND,MLATOM,(LPVOID)((DWORD)h))
#define FREEATOM(h) HandleObj(HM_FREEHND,MLATOM,(LPVOID)((DWORD)h))

#define	GETGDIINFO(h)	((LPOBJHEAD) \
			HandleObj(HM_GETOBJ,MLGDI,(LPVOID)((DWORD)h)))
#define RELEASEGDIINFO(h32) RELEASEINFO(h32)
#define	RELEASEGDI(h) \
			HandleObj(HM_UNLOCKHND,MLGDI,(LPVOID)((DWORD)h)))
#define	CHECKGDIINFO(h)	((LPOBJHEAD) \
			HandleObj(HM_CHECKHND,MLGDI,(LPVOID)((DWORD)h)))
#define	FREEGDI(h)	HandleObj(HM_FREEHND,MLGDI,(LPVOID)((DWORD)h))

#define	LOCKGDI(h)	((VOID) \
			HandleObj(HM_LOCKHND,MLGDI,(LPVOID)((DWORD)h)))
#define	UNLOCKGDI(h)	((VOID) \
			HandleObj(HM_UNLOCKHND,MLGDI,(LPVOID)((DWORD)h)))

#define	CREATEHCURSOR(h)	((LPCURSORINFO) \
			HandleObj(HM_GETHND,MLCURSOR,(LPVOID)(&(h))))
#define	GETCURSORINFO(h)	((LPCURSORINFO) \
			HandleObj(HM_GETOBJ,MLCURSOR,(LPVOID)((DWORD)h)))
#define RELEASECURSORINFO(h32) RELEASEINFO(h32)
#define	RELEASEHCURSOR(h) \
			HandleObj(HM_UNLOCKHND,MLCURSOR,(LPVOID)((DWORD)h)))

#define	CREATEHPEN(h)	((LPPENINFO) \
			HandleObj(HM_GETHND,MLPEN,(LPVOID)(&(h))))
#define	GETPENINFO(h)	((LPPENINFO) \
			HandleObj(HM_GETOBJ,MLPEN,(LPVOID)((DWORD)h)))
#define RELEASEPENINFO(h32) RELEASEINFO(h32)
#define	RELEASEHPEN(h) \
			HandleObj(HM_UNLOCKHND,MLPEN,(LPVOID)((DWORD)h)))

#define	CREATEHBRUSH(h)	((LPBRUSHINFO) \
			HandleObj(HM_GETHND,MLBRUSH,(LPVOID)(&(h))))
#define	GETBRUSHINFO(h)	((LPBRUSHINFO) \
			HandleObj(HM_GETOBJ,MLBRUSH,(LPVOID)((DWORD)h)))
#define RELEASEBRUSHINFO(h32) RELEASEINFO(h32)
#define	RELEASEHBRUSH(h) \
			HandleObj(HM_UNLOCKHND,MLBRUSH,(LPVOID)((DWORD)h)))

#define	CREATEHPALETTE(h) ((LPPALETTEINFO) \
			HandleObj(HM_GETHND,MLPALETTE,(LPVOID)(&(h))))
#define	GETPALETTEINFO(h) ((LPPALETTEINFO) \
			HandleObj(HM_GETOBJ,MLPALETTE,(LPVOID)((DWORD)h)))
#define RELEASEPALETTEINFO(h32) RELEASEINFO(h32)
#define	RELEASEHPALETTE(h) \
			HandleObj(HM_UNLOCKHND,MLPALETTE,(LPVOID)((DWORD)h)))

#define	CREATEHFONT(h)	((LPFONTINFO) \
			HandleObj(HM_GETHND,MLFONT,(LPVOID)(&(h))))
#define	GETFONTINFO(h)	((LPFONTINFO) \
			HandleObj(HM_GETOBJ,MLFONT,(LPVOID)((DWORD)h)))
#define RELEASEFONTINFO(h32) RELEASEINFO(h32)
#define	RELEASEHFONT(h) \
			HandleObj(HM_UNLOCKHND,MLFONT,(LPVOID)((DWORD)h)))

#define	CREATEHICON(h)	((LPTWIN_ICONINFO) \
			HandleObj(HM_GETHND,MLICON,(LPVOID)(&(h))))
#define	GETICONINFO(h)	((LPTWIN_ICONINFO) \
			HandleObj(HM_GETOBJ,MLICON,(LPVOID)((DWORD)h)))
#define RELEASEICONINFO(h32) RELEASEINFO(h32)
#define	RELEASEHICON(h) \
			HandleObj(HM_UNLOCKHND,MLICON,(LPVOID)((DWORD)h)))

#define	CREATEHBITMAP(h) ((LPIMAGEINFO) \
			HandleObj(HM_GETHND,MLBITMAP,(LPVOID)(&(h))))
#define	GETBITMAPINFO(h) ((LPIMAGEINFO) \
			HandleObj(HM_GETOBJ,MLBITMAP,(LPVOID)((DWORD)h)))
#define RELEASEBITMAPINFO(h32) RELEASEINFO(h32)
#define	RELEASEHBITMAP(h) \
			HandleObj(HM_UNLOCKHND,MLBITMAP,(LPVOID)((DWORD)h))
#define	FREEBITMAP(h)	HandleObj(HM_FREEHND,MLBITMAP,(LPVOID)((DWORD)h))

#define CREATEHRGN(h)	((LPREGIONINFO) \
			HandleObj(HM_GETHND,MLREGION,(LPVOID)(&(h))))
#define GETREGIONINFO(h) ((LPREGIONINFO)	\
			HandleObj(HM_GETOBJ,MLREGION,(LPVOID)((DWORD)h)))
#define RELEASEREGIONINFO(h32) RELEASEINFO(h32)
#define RELEASEHRGN(h) \
			HandleObj(HM_UNLOCKHND,MLREGION,(LPVOID)((DWORD)h)))

#define	CREATEHMETAFILE(h) ((LPMETAFILEINFO) \
			HandleObj(HM_GETHND,MLMETAFILE,(LPVOID)(&(h))))
#define	GETMETAFILEINFO(h) ((LPMETAFILEINFO) \
			HandleObj(HM_GETOBJ,MLMETAFILE,(LPVOID)((DWORD)h)))
#define RELEASEMETAFILEINFO(h32) RELEASEINFO(h32)
#define	RELEASEHMETAFILE(h) \
			HandleObj(HM_UNLOCKHND,MLMETAFILE,(LPVOID)((DWORD)h)))

#define	CREATEHDC(h) ((LPDCINFO) \
			HandleObj(HM_GETHND,MLDC,(LPVOID)(&(h))))
#define	GETDCINFO(h) ((LPDCINFO) \
			HandleObj(HM_GETOBJ,MLDC,(LPVOID)((DWORD)h)))
#define RELEASEDCINFO(h32) RELEASEINFO(h32)
#define	RELEASEHDC(h) \
			HandleObj(HM_UNLOCKHND,MLDC,(LPVOID)((DWORD)h)))

#define	DUMPHTAB(x,y)	(VOID)HandleObj(HM_DUMPTABLE,0,0);

#endif

