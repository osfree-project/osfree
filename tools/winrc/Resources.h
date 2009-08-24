/*  Resources.h	2.14
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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/
 
#ifndef Resources__h
#define Resources__h

#ifndef RESOURCE_H
#define RESOURCE_H

#define ALIGN_16BIT(x)		((int)((int)(x)+1) & ~1)
#define ALIGN_32BIT(x)		((int)((int)(x)+3) & ~3)

typedef struct tagNAMEENTRY {
	WORD	wTypeOrdinal;
	WORD	wIdOrdinal;
	LPSTR	szType;
	LPSTR	szId;
} NAMEENTRY;	

typedef struct tagCURSORDIRENTRY {
	WORD	wWidth;
	WORD	wHeight;
	WORD	wPlanes;
	WORD	wBitsPerPel;
	DWORD	dwBytesInRes;
	WORD	wOrdinalNumber;
} CURSORDIRENTRY;

typedef struct tagICONDIRENTRY {
	char	bWidth;
	char	bHeight;
	char	bColorCount;
	WORD	wPlanes;
	WORD	wBitsPerPel;
	DWORD	dwBytesInRes;
	WORD	wOrdinalNumber;
} ICONDIRENTRY;

typedef struct iconitem {
	/* the first three elements are equivalent to MENUITEMTEMPLATE,
         * we convert them to this form for internal use...
	 */ 
	WORD  iconflag;
	WORD  iconid;
	LPSTR iconname;

	/* this element points to a popup if menuflag & MF_POPUP */
	struct iconitem *icondata;
} ICONDATA;

typedef struct menuitem {
	/* the first three elements are equivalent to MENUITEMTEMPLATE,
         * we convert them to this form for internal use...
	 */ 
	WORD  menuflag;
	WORD  menuid;
	LPSTR menuname;

	/* this element points to a popup if menuflag & MF_POPUP */
	struct menuitem *menupopup;
} MENUDATA;

typedef struct {
	WORD 	x,y;
	WORD	cx,cy;
	WORD	wID;
	DWORD   lStyle;
	DWORD   dwExtStyle;
	LPSTR   szClass;
	LPSTR   szText;	
} CONTROLDATA;

typedef struct {
	DWORD	lStyle;
	DWORD	dwExtStyle;
	WORD    bNumberOfItems;
	WORD	x,y;
	WORD    cx,cy;
	LPSTR   szMenuName;
	LPSTR   szClassName;
	LPSTR 	szCaption;
	WORD	wPointSize;
	LPSTR   szFaceName;
	CONTROLDATA *controlinfo;
} DIALOGDATA;

typedef struct {		/* MFC 4.x internal resource */
	WORD	wDataSize;
	BYTE	bData[1024];
} DLGINITDATA;

typedef struct {		/* MFC 4.x internal resource */
	WORD	wVersion;
	WORD	wWidth;
	WORD	wHeight;
	WORD	wItemCount;
	WORD	aItems[32];
} TOOLBARDATA;

typedef struct {
	LPBYTE	rcsdata;	/* INTEL data */
	WORD	wType;		/* index into conversion routines table	*/
	HGLOBAL	hGlobal;	/* memory handle of resource	   */
	DWORD	rcsoffset;	/* location in resource file       */
	DWORD	rcslength;	/* length of data in resource file */
	WORD	rcsflags;	/* pre-loaded, discardable...      */
	HRSRC	hRsrc;		/* handle to 'found' resource      */
	HGDIOBJ	hObject;	/* GDI object associated with resource */
	LPSTR	rcsitemname;	/* name/ordinal of resource        */
} NAMEINFO;
typedef NAMEINFO *LPNAMEINFO;

typedef char	RCDATAENTRY;

typedef struct	tagCURSORIMAGE {
	short	wHotSpotX;
	short	wHotSpotY;
	BITMAPINFOHEADER *bmp_hdr;
	RGBQUAD	*rgb_quad;
	LPBYTE	xor_mask;
	LPBYTE	and_mask;
} CURSORIMAGE;

typedef struct	tagBITMAPIMAGE {
	BITMAPINFOHEADER *bmp_hdr;
	RGBQUAD	*rgb_quad;
	LPBYTE	bitmap_bits;
} BITMAPIMAGE;
typedef BITMAPIMAGE *LPBITMAPIMAGE;

typedef struct	tagICONIMAGE {
	BITMAPINFOHEADER *icon_bmp_hdr;
	RGBQUAD		 *icon_rgb_quad;
	LPBYTE		 icon_xor_mask;
	LPBYTE		 icon_and_mask;
} ICONIMAGE;
typedef ICONIMAGE *LPICONIMAGE;

typedef struct {
	LPSTR     rcstypename;
	WORD  rcscount;
	NAMEINFO *rcsinfo;
} TYPEINFO;

typedef struct	tagACCELENTRY {
	BYTE	fFlags;
	WORD	wEvent;
	WORD	wId;
	WORD	wPadding;
} ACCELENTRY;

typedef LPCSTR STRINGENTRY;

#define CONTROLITEM(x,y,cx,cy,id,style,exstyle,class,text) { \
	x,y, \
	cx,cy, \
	id, \
	style, \
	exstyle, \
	class, \
	text,  \
}
#define ACCELITEM(f,event,wid) { f,event,wid }

#define MENUITEM(type,id,text,submenu) { \
	type,	id,	text,	submenu }

#define RT_USER_DEF	MAKEINTRESOURCE(11)
#define RT_NAME_TABLE	MAKEINTRESOURCE(15)

#define TEMPL_SIZE 0x14
#define MAXSYSTYPE (WORD)(DWORD)RT_NAME_TABLE

LPBYTE hswr_dialog_nat_to_bin(LPBYTE);

typedef struct {
        char *owner;
        char *company;
        char *keyword;
	char *pgpkey;
} REGISTRATION;

#endif
#endif /* Resources__h */
