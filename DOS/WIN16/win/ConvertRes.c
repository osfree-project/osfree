/*    
	ConvertRes.c	2.12
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

#include "windows.h"

#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "Resources.h"
#include "Module.h"
#include "Kernel.h"
#include "DPMI.h"
#include "PrinterBin.h"

#include <stdio.h>
#include <string.h>


extern TYPEINFO hsmt_resource_templ[];

/* external routines */
HMODULE GetModuleFromInstance(HINSTANCE);

TYPEINFO * ReadResourceTable(HINSTANCE,USERDLL *);
void LoadResourceEx(HANDLE,NAMEINFO *,LPMEMORYINFO);
LPBYTE ConvertResourceToBin(LPBYTE,WORD);
LPBYTE ConvertResourceToNat(LPBYTE,WORD);
LPBYTE ExtractDialog(HINSTANCE,LPBYTE);
LPBYTE ExtractMenuTemplate(LPBYTE *);
void CleanupDialog(LPBYTE);
void CleanupMenuTemplate(LPBYTE);
LPBYTE MenuBinToNat(LPBYTE *);

LPBYTE hswr_cursor_bin_to_nat(LPBYTE);
LPBYTE hswr_bitmap_bin_to_nat(LPBYTE);
LPBYTE hswr_icon_bin_to_nat(LPBYTE);
LPBYTE hswr_menu_bin_to_nat(LPBYTE);
LPBYTE hswr_dialog_bin_to_nat(LPBYTE);
LPBYTE hswr_string_bin_to_nat(LPBYTE);

LPBYTE hswr_fontdir_bin_to_nat(LPBYTE);
LPBYTE hswr_font_bin_to_nat(LPBYTE);

LPBYTE hswr_accel_bin_to_nat(LPBYTE);
LPBYTE hswr_rcdata_bin_to_nat(LPBYTE);
LPBYTE hswr_grcursor_bin_to_nat(LPBYTE);
LPBYTE hswr_gricon_bin_to_nat(LPBYTE);
LPBYTE hswr_nametbl_bin_to_nat(LPBYTE);

LPBYTE hswr_cursor_nat_to_bin(LPBYTE);
LPBYTE hswr_bitmap_nat_to_bin(LPBYTE);
LPBYTE hswr_icon_nat_to_bin(LPBYTE);
LPBYTE hswr_menu_nat_to_bin(LPBYTE);
LPBYTE hswr_dialog_nat_to_bin(LPBYTE);
LPBYTE hswr_string_nat_to_bin(LPBYTE);
LPBYTE hswr_fontdir_nat_to_bin(LPBYTE);
LPBYTE hswr_font_nat_to_bin(LPBYTE);
LPBYTE hswr_accel_nat_to_bin(LPBYTE);
LPBYTE hswr_rcdata_nat_to_bin(LPBYTE);
LPBYTE hswr_grcursor_nat_to_bin(LPBYTE);
LPBYTE hswr_gricon_nat_to_bin(LPBYTE);

static LPSTR StringFromOrdinal(TYPEINFO *,WORD,WORD);
static LPSTR ReadPascalString(LPBYTE);
static int   GetString(LPSTR *,LPSTR);

void GetBIHeader(BITMAPINFOHEADER *,LPBYTE);
void PutBIHeader(LPBYTE,BITMAPINFOHEADER *);

static LPBYTE DialogBinToNat(TYPEINFO *,LPBYTE);

typedef LPBYTE (*CONVERTRESPROC)(LPBYTE);

typedef struct TYPEFUNC {
	CONVERTRESPROC lpfnBinToNat;
	CONVERTRESPROC lpfnNatToBin;
} TYPEFUNC;

static TYPEFUNC ConvRoutineTable[] = {
	{ NULL, NULL },
	{ hswr_cursor_bin_to_nat, hswr_cursor_nat_to_bin },
	{ hswr_bitmap_bin_to_nat, hswr_bitmap_nat_to_bin },
	{ hswr_icon_bin_to_nat, hswr_icon_nat_to_bin },
	{ hswr_menu_bin_to_nat, hswr_menu_nat_to_bin },
	{ hswr_dialog_bin_to_nat, hswr_dialog_nat_to_bin },
	{ hswr_string_bin_to_nat, hswr_string_nat_to_bin },
	{ hswr_fontdir_bin_to_nat, hswr_fontdir_nat_to_bin },
	{ hswr_font_bin_to_nat, hswr_font_nat_to_bin },
	{ hswr_accel_bin_to_nat, hswr_accel_nat_to_bin },
	{ hswr_rcdata_bin_to_nat, hswr_rcdata_nat_to_bin },
	{ hswr_rcdata_bin_to_nat, hswr_rcdata_nat_to_bin },
	{ hswr_grcursor_bin_to_nat, hswr_grcursor_nat_to_bin},
	{ hswr_rcdata_bin_to_nat, hswr_rcdata_nat_to_bin },
	{ hswr_gricon_bin_to_nat, hswr_gricon_nat_to_bin },
	{ hswr_nametbl_bin_to_nat, NULL }
};

static LPSTR std_controls[] = {
	"BUTTON",
	"EDIT",
	"STATIC",
	"LISTBOX",
	"SCROLLBAR",
	"COMBOBOX"
};

TYPEINFO *
ReadResourceTable(HINSTANCE hInst,USERDLL *lpDLL)
{
    HFILE hfFile;
    DWORD dwOffset;
    WORD wResourceTableSize;
    LPBYTE lpRscTblImage;
    LPBYTE lpResPtr;
    LPBYTE lpData;
    WORD wAlignShift, wType;
    TYPEINFO *rt;
    TYPEINFO *lpResTypeInfo;
    NAMEINFO *rcInfo;
    WORD rnID;
    WORD wAddSize = 0;
    int i,n,len;

    if ((hfFile = lpDLL->hfExeFile) < 0)
	return (TYPEINFO *)NULL;
    dwOffset = lpDLL->dwNEOffset + lpDLL->wResourceTblOffset;
    _llseek(hfFile,dwOffset,0);
    if (!(wResourceTableSize = (lpDLL->wResTblOffset - 
		lpDLL->wResourceTblOffset + 3) & ~0x3))
	return NULL;
    if (!(lpRscTblImage = (LPBYTE)WinMalloc(wResourceTableSize)))
	return (TYPEINFO *)NULL;
    if ((n = _lread(hfFile,lpRscTblImage,
			wResourceTableSize)) < (int)wResourceTableSize) 
	return (TYPEINFO *)NULL;
    lpResPtr = lpRscTblImage;
    lpResTypeInfo = (TYPEINFO *)WinMalloc(sizeof(TYPEINFO)*TEMPL_SIZE);
    memcpy((LPSTR)lpResTypeInfo,(LPSTR)&hsmt_resource_templ,
		sizeof(TYPEINFO)*TEMPL_SIZE);
    wAlignShift = GETWORD(lpResPtr);
    lpResPtr += 2;

/* Scan for NAME_TABLE type -- this guy has to be processed first */

    while ((wType = GETWORD(lpResPtr))) {
	if (wType == 0x800f) { 		/* RT_NAME_TABLE	*/
	    rt = &lpResTypeInfo[0xf];
	    rt->rcscount = GETWORD(lpResPtr+2);
	    lpResPtr += 8;
	    rcInfo = (NAMEINFO *)WinMalloc(rt->rcscount * sizeof(NAMEINFO));
	    rt->rcsinfo = rcInfo;
	    for (i=0; i<(int)rt->rcscount; i++) {
		rcInfo[i].wType = (WORD)0xf;
		rcInfo[i].rcsoffset = GETWORD(lpResPtr) << wAlignShift;
		rcInfo[i].rcslength = GETWORD(lpResPtr+2) << wAlignShift;
		rcInfo[i].rcsflags = GETWORD(lpResPtr+4);
		rcInfo[i].hObject = 0;
		rnID = GETWORD(lpResPtr+6);
		if (rnID & 0x8000)
		    rcInfo[i].rcsitemname = (LPSTR)(DWORD)rnID;
		else
		    rcInfo[i].rcsitemname =
		    		ReadPascalString(lpRscTblImage+rnID);
		lpResPtr += 12;
		lpData = (LPBYTE)WinMalloc(rcInfo[i].rcslength);
		_llseek(hfFile,rcInfo[i].rcsoffset,0);
		if ((len = _lread(hfFile,lpData,rcInfo[i].rcslength)) <
				(int)rcInfo[i].rcslength) {
		}
		rcInfo[i].rcsdata = (LPBYTE)hswr_nametbl_bin_to_nat(lpData);
	    }
	}
	else  
    	    lpResPtr += 8 + 12 * GETWORD(lpResPtr+2);
    }

/* Now we really process the resource table */

    lpResPtr = lpRscTblImage + 2;
    while ((wType = GETWORD(lpResPtr))) {
	if ((wType & 0x8000) && ((n = wType & ~0x8000)<(int)TEMPL_SIZE)) 
	    if (wType == 0x800f) {
		lpResPtr += 8 + 12 * GETWORD(lpResPtr+2);
		continue;
	    }
	    else {
		rt = &lpResTypeInfo[n];
		rt->rcstypename = (LPSTR)(wType & ~0x8000);
	    }
	else { 		/* user-defined resource */
	    for (n=0; n<(int)(TEMPL_SIZE+wAddSize); n++)
		if (lpResTypeInfo[n].rcstypename == RT_USER_DEF)
		    break;
	    if (n == (int)(TEMPL_SIZE+wAddSize)) {
		n--;
		wAddSize += 4;
		lpResTypeInfo = (TYPEINFO *)WinRealloc((LPSTR)lpResTypeInfo,
			sizeof(TYPEINFO)*(TEMPL_SIZE+wAddSize+1));
		for(i=n;i<n+4;i++) 
		    lpResTypeInfo[i].rcstypename = (LPSTR)RT_USER_DEF;
		lpResTypeInfo[i].rcstypename = (LPSTR)NULL; /* last entry */
	    }
	    rt = &lpResTypeInfo[n];
		if (wType & 0x8000)
		    rt->rcstypename = StringFromOrdinal(lpResTypeInfo,wType,0);
		else 
	            rt->rcstypename = ReadPascalString(lpRscTblImage+wType);
	}
	rt->rcscount = GETWORD(lpResPtr+2);
	lpResPtr += 8;
	rcInfo = (NAMEINFO *)WinMalloc(rt->rcscount * sizeof(NAMEINFO));
	rt->rcsinfo = rcInfo;
	for (i=0; i<(int)rt->rcscount; i++) {
	    rcInfo[i].wType = (wType & 0x8000)?
			(wType & ~0x8000):(WORD)(DWORD)RT_USER_DEF;
	    rcInfo[i].rcsoffset = GETWORD(lpResPtr) << wAlignShift;
	    rcInfo[i].rcslength = GETWORD(lpResPtr+2) << wAlignShift;
	    rcInfo[i].rcsflags = GETWORD(lpResPtr+4);
	    rcInfo[i].hRsrc = 0;
	    rcInfo[i].hGlobal = 0;
	    rcInfo[i].hObject = 0;
	    rnID = GETWORD(lpResPtr+6);
	    if (rnID & 0x8000)
		rcInfo[i].rcsitemname = StringFromOrdinal
				(lpResTypeInfo,wType,rnID);
	    else 
		rcInfo[i].rcsitemname = ReadPascalString(lpRscTblImage+rnID);
	    rcInfo[i].rcsdata = NULL;
	    lpResPtr += 12;
	}
    }
    return lpResTypeInfo;
}

void 
LoadResourceEx(HANDLE hInst,NAMEINFO *lpResInfo,LPMEMORYINFO lpMemory)
{
    LPBYTE lpData;
    MODULEINFO *modinfo;
    HFILE hf;
    int ret;
    CONVERTRESPROC lpfnConvert;
    HANDLE hModule;

    if (!(modinfo = GETMODULEINFO(hInst))) { /* hModule can be passed */
	hModule = GetModuleFromInstance(hInst);
	if (!(modinfo = (MODULEINFO *)GETMODULEINFO(hModule)))
	    return;
    }
    if ((hf = _lopen(modinfo->lpFileName,READ)) < 0) {
	RELEASEMODULEINFO(modinfo);
	return;
    }
    _llseek(hf,lpResInfo->rcsoffset,0);
    lpData = (LPBYTE)WinMalloc(lpResInfo->rcslength + 4);
    *((LPDWORD)lpData) = (DWORD)modinfo->ResourceTable;
    lpMemory->lpData = (LPSTR)lpData;
    lpData += 4;

    if((ret = _lread(hf,lpData,lpResInfo->rcslength)) <
    			(int)lpResInfo->rcslength) {
    }
    _lclose(hf);
    if (lpResInfo->wType < MAXSYSTYPE)
        lpfnConvert = ConvRoutineTable[lpResInfo->wType].lpfnBinToNat;
    else
	lpfnConvert = hswr_rcdata_bin_to_nat;
    if (!lpfnConvert) {
	RELEASEMODULEINFO(modinfo);
	FatalAppExit(0,"Resource conversion routine not found.");
	return;
    }
    lpResInfo->rcsdata = (LPBYTE)((lpfnConvert)(lpData));
    lpMemory->lpCore = (LPSTR)lpResInfo->rcsdata;
    RELEASEMODULEINFO(modinfo);
}

LPBYTE
hswr_cursor_bin_to_nat(LPBYTE lpData)
{
    WORD wHotX, wHotY;
    BITMAPINFOHEADER *lpbih;
    WORD wRGBCount, wMaskBytes;
    RGBQUAD *rgb_quad = (RGBQUAD *)NULL;
    LPBYTE XOR_mask;
    LPBYTE AND_mask;
    CURSORIMAGE *cur_image;

    wHotX = GETWORD(lpData);
    wHotY = GETWORD(lpData+2);
    lpData += 4;
    if (!(lpbih = (BITMAPINFOHEADER *)WinMalloc(sizeof(BITMAPINFOHEADER))))
	return NULL;
    GetBIHeader(lpbih,lpData);
    lpData += lpbih->biSize;
    wRGBCount = lpbih->biBitCount;
    wRGBCount = (wRGBCount==1)?2:(wRGBCount==4)?16:(wRGBCount==8)?256:0;
    if (wRGBCount) {
	rgb_quad = (RGBQUAD *)WinMalloc(sizeof(RGBQUAD) * wRGBCount);
	if (!rgb_quad)
	    return NULL;
	memcpy((LPSTR)rgb_quad,(LPSTR)lpData,sizeof(RGBQUAD) * wRGBCount);
	lpData += sizeof(RGBQUAD) * wRGBCount;
    }
    wMaskBytes = ((int)lpbih->biWidth * (int)lpbih->biHeight
			* (int)lpbih->biBitCount)/16;
    if (!(XOR_mask = (LPBYTE)WinMalloc(wMaskBytes)))
	return NULL;
    memcpy((LPSTR)XOR_mask,(LPSTR)lpData,(int)wMaskBytes);
    lpData += wMaskBytes;
    wMaskBytes = ((int)lpbih->biWidth * (int)lpbih->biHeight)/16;
    if (!(AND_mask = (LPBYTE)WinMalloc(wMaskBytes)))
	return NULL;
    memcpy((LPSTR)AND_mask,(LPSTR)lpData,(int)wMaskBytes);
    cur_image = (CURSORIMAGE *)WinMalloc(sizeof(CURSORIMAGE));
    cur_image->wHotSpotX = wHotX;
    cur_image->wHotSpotY = wHotY;
    cur_image->bmp_hdr = lpbih;
    cur_image->rgb_quad = rgb_quad;
    cur_image->xor_mask = XOR_mask;
    cur_image->and_mask = AND_mask;
    return (LPBYTE)cur_image;
}

LPBYTE
hswr_bitmap_bin_to_nat(LPBYTE lpData)
{
    DWORD dwSize;
    BITMAPINFOHEADER *lpbih;
    WORD wRGBCount;
    RGBQUAD *rgb_quad = (RGBQUAD *)NULL;
    LPBYTE bmp_bits;
    BITMAPIMAGE *bmp_image;
    int i;

    if (!(lpbih = (BITMAPINFOHEADER *)WinMalloc(sizeof(BITMAPINFOHEADER))))
	return NULL;
    GetBIHeader(lpbih,lpData);
    lpData += lpbih->biSize;
    wRGBCount = lpbih->biBitCount;
    wRGBCount = (wRGBCount==1)?2:(wRGBCount==4)?16:(wRGBCount==8)?256:0;
    if (wRGBCount) {
	rgb_quad = (RGBQUAD *)WinMalloc(sizeof(RGBQUAD) * wRGBCount);
	for (i=0; i < (int)wRGBCount; i++) {
	    rgb_quad[i].rgbBlue = *lpData++;
	    rgb_quad[i].rgbGreen = *lpData++;
	    rgb_quad[i].rgbRed = *lpData++;
	    if (lpbih->biSize == 40)
		rgb_quad[i].rgbReserved = *lpData++;
	    else
		rgb_quad[i].rgbReserved = 0;
	}
    }
    dwSize = lpbih->biSizeImage;
    bmp_bits = (LPBYTE)WinMalloc(dwSize);
    memcpy((LPSTR)bmp_bits,(LPSTR)lpData,(int)dwSize);
    lpData += dwSize;
    bmp_image = (BITMAPIMAGE *)WinMalloc(sizeof(BITMAPIMAGE));
    bmp_image->bmp_hdr = lpbih;
    bmp_image->rgb_quad = rgb_quad;
    bmp_image->bitmap_bits = (BYTE *)bmp_bits;
    return (LPBYTE)bmp_image;
}

LPBYTE
hswr_icon_bin_to_nat(LPBYTE lpData)
{
    BITMAPINFOHEADER *lpbih;
    WORD wRGBCount, wMaskBytes;
    RGBQUAD *rgb_quad = (RGBQUAD *)NULL;
    LPBYTE XOR_mask;
    LPBYTE AND_mask;
    ICONIMAGE *icon_image;

    if (!(lpbih = (BITMAPINFOHEADER *)WinMalloc(sizeof(BITMAPINFOHEADER))))
	return NULL;
    GetBIHeader(lpbih,lpData);
    lpData += lpbih->biSize;
    wRGBCount = lpbih->biBitCount;
    wRGBCount = (wRGBCount==1)?2:(wRGBCount==4)?16:(wRGBCount==8)?256:0;
    if (wRGBCount) {
	rgb_quad = (RGBQUAD *)WinMalloc(sizeof(RGBQUAD) * wRGBCount);
	if (!rgb_quad)
	    return NULL;
	memcpy((LPSTR)rgb_quad,(LPSTR)lpData,sizeof(RGBQUAD) * wRGBCount);
	lpData += sizeof(RGBQUAD) * wRGBCount;
    }
    wMaskBytes = ((int)lpbih->biWidth * (int)lpbih->biHeight
			* (int)lpbih->biBitCount)/16;
    if (!(XOR_mask = (LPBYTE)WinMalloc(wMaskBytes)))
	return NULL;
    memcpy((LPSTR)XOR_mask,(LPSTR)lpData,(int)wMaskBytes);
    lpData += wMaskBytes;
    wMaskBytes = ((int)lpbih->biWidth * (int)lpbih->biHeight)/16;
    if (!(AND_mask = (LPBYTE)WinMalloc(wMaskBytes)))
	return NULL;
    memcpy((LPSTR)AND_mask,(LPSTR)lpData,(int)wMaskBytes);
    icon_image = (ICONIMAGE *)WinMalloc(sizeof(ICONIMAGE));
    icon_image->icon_bmp_hdr = lpbih;
    icon_image->icon_rgb_quad = rgb_quad;
    icon_image->icon_xor_mask = XOR_mask;
    icon_image->icon_and_mask = AND_mask;
    return (LPBYTE)icon_image;
}

LPBYTE
hswr_menu_bin_to_nat(LPBYTE lpData)
{
    lpData += 4;	/* skip menu header */
    return MenuBinToNat(&lpData);
}

LPBYTE 
hswr_dialog_bin_to_nat(LPBYTE lpData)
{
    TYPEINFO *ResourceTable;

    ResourceTable = (TYPEINFO *)*((LPDWORD)(lpData-4));
	return DialogBinToNat(ResourceTable,lpData);
}

LPBYTE 
hswr_string_bin_to_nat(LPBYTE lpData)
{
    int i;
    BYTE len;
    WORD wSize = 0;
    WORD wOffset = 0;
    LPSTR *lpStringTable;
    LPSTR lpStringSpace;
#ifdef LATER
    BINADDR *lpStringTableBin;
    WORD wSel;
#endif

    lpStringTable = (LPSTR *)WinMalloc(32*sizeof(LPSTR));
    if (!lpStringTable) {
	return NULL;
    }
    for (i=0; i<16; i++) {
	len = *(lpData + wOffset) + 1;
	wSize += len;
	wOffset += len;
    }
    lpStringSpace = WinMalloc(wSize);

#ifdef LATER
/*
    Here we create a table of FAR * pointers to be returned by LockResource.
    Issue: is that what is really returned???
    For the time being let's return just a pointer to the pascal strings array.
    wSel = AssignSel(lpData);
    lpStringTableBin = (BINADDR *)(lpStringTable + 16);
*/
#endif
    for (i=0, wOffset=0; i<16; i++) {
	len = *(lpData + wOffset);
	memcpy(lpStringSpace,(LPSTR)lpData+wOffset+1,(int)len);
	lpStringTable[i] = lpStringSpace;
#ifdef LATER
	lpStringTableBin[i] = (BINADDR)MAKELONG(wOffset,wSel);
#endif
	lpStringSpace += len;
	*lpStringSpace++ = '\0';
	wOffset += len + 1;
    }
    return (LPBYTE)lpStringTable;
}

LPBYTE 
hswr_fontdir_bin_to_nat(LPBYTE lpData)
{
    return lpData;
}

LPBYTE 
hswr_font_bin_to_nat(LPBYTE lpData)
{
    return lpData;
}

LPBYTE 
hswr_accel_bin_to_nat(LPBYTE lpData)
{
    ACCELENTRY *ac;
    int nCount = 1,i;
    LPBYTE ptr;

    ptr = lpData;
    while (!(*ptr & 0x80)) {	/* last entry in accel. table */
	nCount++;
	ptr += 5;
    }
    ac = (ACCELENTRY *)WinMalloc(sizeof(ACCELENTRY) * nCount);
    for (i=0; i<nCount; i++) {
	ac[i].fFlags = *lpData++;
	ac[i].wEvent = GETWORD(lpData);
	ac[i].wId = GETWORD(lpData+2);
	lpData += 4;
    }
    return (LPBYTE)ac;
}

LPBYTE 
hswr_rcdata_bin_to_nat(LPBYTE lpData)
{
    return lpData;
}

LPBYTE 
hswr_grcursor_bin_to_nat(LPBYTE lpData)
{
    CURSORDIRENTRY *cd;

    if (!(cd = (CURSORDIRENTRY *)WinMalloc(sizeof(CURSORDIRENTRY)))) {
	return NULL;
    }
    cd->wWidth = GETWORD(lpData+6);
    cd->wHeight = GETWORD(lpData+8);
    cd->wPlanes = GETWORD(lpData+10);
    cd->wBitsPerPel = GETWORD(lpData+12);
    cd->dwBytesInRes = GETDWORD(lpData+14);
    cd->wOrdinalNumber = GETWORD(lpData+18);
    return (LPBYTE)cd;
}

LPBYTE 
hswr_gricon_bin_to_nat(LPBYTE lpData)
{
    ICONDIRENTRY *id;
    WORD idCount;
    int i;

    lpData += 2;	/* skip "reserved" field */
    if (GETWORD(lpData) != 1)	/* sanity check */ {
	return NULL;
    }
    idCount = GETWORD(lpData+2);
    lpData += 4;
    if (!(id = (ICONDIRENTRY *)WinMalloc(sizeof(ICONDIRENTRY)*(idCount+1)))) {
	return NULL;
    }
    for (i=0; i<(int)idCount; i++) {
	id[i].bWidth = *lpData;
	id[i].bHeight = *(lpData+1);
	id[i].bColorCount = *(lpData+2);
	id[i].wPlanes = GETWORD(lpData+4);
	id[i].wBitsPerPel = GETWORD(lpData+6);
	id[i].dwBytesInRes = GETDWORD(lpData+8);
	id[i].wOrdinalNumber = GETWORD(lpData+12);
	lpData += 14;
    }
    id[i].wOrdinalNumber = 0;
    return (LPBYTE)id;
}

LPBYTE
hswr_nametbl_bin_to_nat(LPBYTE lpData)
{
    WORD wByteCount;
    WORD wEntryCount = 0;
    NAMEENTRY *lpNameTbl;
    LPBYTE ptr;
    int i;

	/* Determine size of NAMETABLE */
    ptr = lpData;
    while ((wByteCount = GETWORD(ptr))) {
	wEntryCount++;
	ptr += wByteCount;
    }
    lpNameTbl = (NAMEENTRY *)WinMalloc((wEntryCount+1) * sizeof(NAMEENTRY));
    for (i=0; i<(int)wEntryCount; i++) {
	lpNameTbl[i].wTypeOrdinal = GETWORD(lpData+2);
	lpNameTbl[i].wIdOrdinal = GETWORD(lpData+4);
	lpData += 6;
	lpData += GetString(&lpNameTbl[i].szType,(LPSTR)lpData);
	lpData += GetString(&lpNameTbl[i].szId,(LPSTR)lpData);
    }
    lpNameTbl[i].wTypeOrdinal = 0;
    return (LPBYTE)lpNameTbl;
}

LPBYTE 
hswr_dialog_nat_to_bin(LPBYTE lpData)
{
    LPBYTE lpDialogPtr, lpDialog;
    DIALOGDATA *lpdd;
    CONTROLDATA *lpcd;
    WORD wSize, wNumControls;
    WORD i,j;
    WORD wOffset = 0;

    lpdd = (DIALOGDATA *)lpData;
    wSize = DWORD_86 + 4*WORD_86 + 1 + 4; /* 4 - for ResourceTable pointer */
    if (HIWORD(lpdd->szMenuName))
	wSize += strlen(lpdd->szMenuName) + 1;
    if (HIWORD(lpdd->szClassName))
	wSize += strlen(lpdd->szClassName) + 1;
    if (HIWORD(lpdd->szCaption))
	wSize += strlen(lpdd->szCaption) + 1;
    if (lpdd->lStyle & DS_SETFONT) 
	if (HIWORD(lpdd->szFaceName))
	    wSize += strlen(lpdd->szFaceName) + 1 + WORD_86;
	else
	    wSize += WORD_86;
    lpDialogPtr = (LPBYTE)WinMalloc(wSize);
	PUTDWORD(lpDialogPtr,0L);
    lpDialog = lpDialogPtr + 4;
    PUTDWORD(lpDialog,lpdd->lStyle);
    *(lpDialog+4) = lpdd->bNumberOfItems;
    PUTWORD(lpDialog+5,lpdd->x);
    PUTWORD(lpDialog+7,lpdd->y);
    PUTWORD(lpDialog+9,lpdd->cx);
    PUTWORD(lpDialog+11,lpdd->cy);
    lpDialog += 13;
    if (HIWORD(lpdd->szMenuName)) {
	strcpy((LPSTR)lpDialog,lpdd->szMenuName);
	lpDialog += strlen((LPSTR)lpDialog) + 1;
    }
    else *lpDialog++ = '\0';
    if (HIWORD(lpdd->szClassName)) {
	strcpy((LPSTR)lpDialog,lpdd->szClassName);
	lpDialog += strlen((LPSTR)lpDialog) + 1;
    }
    else *lpDialog++ = '\0';
    if (HIWORD(lpdd->szCaption)) {
	strcpy((LPSTR)lpDialog,lpdd->szCaption);
	lpDialog += strlen((LPSTR)lpDialog) + 1;
    }
    else *lpDialog++ = '\0';
    if (lpdd->lStyle & DS_SETFONT) {
	PUTWORD(lpDialog,lpdd->wPointSize);
	lpDialog += 2;
	if (HIWORD(lpdd->szFaceName)) {
	    strcpy((LPSTR)lpDialog,lpdd->szFaceName);
	    lpDialog += strlen((LPSTR)lpDialog) + 1;
	}
	else *lpDialog++ = '\0';
    }
    wOffset = lpDialog - lpDialogPtr;
    wNumControls = (WORD)lpdd->bNumberOfItems;
    wSize += wNumControls * (DWORD_86 + 5*WORD_86 + 1);
    lpcd = lpdd->controlinfo;
    for (i=0; i<wNumControls; i++) {
	if (HIWORD(lpcd[i].szText))
	    wSize += strlen(lpcd[i].szText) + 1;
	else wSize++;
	if (HIWORD(lpcd[i].szClass))
	    wSize += strlen(lpcd[i].szClass) + 1;
	else wSize++;
    }
    lpDialogPtr = (LPBYTE)WinRealloc((LPSTR)lpDialogPtr,wSize);
    lpDialog = lpDialogPtr + wOffset;
    for (i=0; i<wNumControls; i++) {
	PUTWORD(lpDialog,lpcd[i].x);
	PUTWORD(lpDialog+2,lpcd[i].y);
	PUTWORD(lpDialog+4,lpcd[i].cx);
	PUTWORD(lpDialog+6,lpcd[i].cy);
	PUTWORD(lpDialog+8,lpcd[i].wID);
	PUTDWORD(lpDialog+10,lpcd[i].lStyle);
	lpDialog += 14;
	if (HIWORD(lpcd[i].szClass)) {
	    for (j=0; j<6; j++) 
		if (!strcmp(std_controls[j],lpcd[i].szClass)) {
		    *lpDialog++ = (BYTE)(j | 0x80);
		    break;
		}
	    if (j == 6) {	/* Control name string not found */
		strcpy((LPSTR)lpDialog,lpcd[i].szClass);
		lpDialog += strlen((LPSTR)lpDialog) + 1;
	    }
	}
	else *lpDialog++ = '\0';
	if (HIWORD(lpcd[i].szText)) {
	    strcpy((LPSTR)lpDialog,lpcd[i].szText);
	    lpDialog += strlen((LPSTR)lpDialog) + 1;
	}
	else *lpDialog++ = '\0';
	*lpDialog++ = '\0'; /* don't ask me why -- LK	*/
    }
    return lpDialogPtr;
}

LPBYTE 
hswr_cursor_nat_to_bin(LPBYTE lpData)
{
    return lpData;
}

LPBYTE 
hswr_bitmap_nat_to_bin(LPBYTE lpData)
{
    BITMAPIMAGE *lpbi;
    LPBYTE lpBinData;
    LPBYTE ptr;
    WORD wRGBCount;
    DWORD dwSize;
    int i;

    lpbi = (BITMAPIMAGE *)lpData;
    wRGBCount = lpbi->bmp_hdr->biBitCount;
    wRGBCount = (wRGBCount==1)?2:(wRGBCount==4)?16:(wRGBCount==8)?256:0;
    dwSize = lpbi->bmp_hdr->biSize + wRGBCount*4 + lpbi->bmp_hdr->biSizeImage;
    lpBinData = (LPBYTE)WinMalloc(dwSize);
    ptr = lpBinData;
    PutBIHeader(ptr,lpbi->bmp_hdr);
    ptr += lpbi->bmp_hdr->biSize;
    if (lpbi->bmp_hdr->biSize == 40) {
	memcpy((LPSTR)ptr,(LPSTR)&lpbi->rgb_quad[0],(int)wRGBCount*4);
	ptr += wRGBCount*4;
    }
    else 
	for (i=0; i < (int)wRGBCount; i++) {
	    *ptr++ = lpbi->rgb_quad[i].rgbBlue;
	    *ptr++ = lpbi->rgb_quad[i].rgbGreen;
	    *ptr++ = lpbi->rgb_quad[i].rgbRed;
	}
    memcpy((LPSTR)ptr,(LPSTR)lpbi->bitmap_bits,lpbi->bmp_hdr->biSizeImage);
    return lpBinData;
}

LPBYTE 
hswr_icon_nat_to_bin(LPBYTE lpData)
{
    return lpData;
}

LPBYTE 
hswr_menu_nat_to_bin(LPBYTE lpData)
{
    return lpData;
}

LPBYTE 
hswr_string_nat_to_bin(LPBYTE lpData)
{
    return lpData;
}

LPBYTE 
hswr_fontdir_nat_to_bin(LPBYTE lpData)
{
    return lpData;
}

LPBYTE 
hswr_font_nat_to_bin(LPBYTE lpData)
{
    return lpData;
}

LPBYTE 
hswr_accel_nat_to_bin(LPBYTE lpData)
{
    return lpData;
}

LPBYTE 
hswr_rcdata_nat_to_bin(LPBYTE lpData)
{
    return lpData;
}

LPBYTE 
hswr_grcursor_nat_to_bin(LPBYTE lpData)
{
    return lpData;
}

LPBYTE 
hswr_gricon_nat_to_bin(LPBYTE lpData)
{
    return lpData;
}

LPBYTE
ExtractMenuTemplate(LPBYTE *lplpData)
{
    APISTR((LF_APISTUB,"ExtractMenuTemplate(LPBYTE=%p)\n",lplpData));
    return NULL;
}

void
CleanupMenuTemplate(LPBYTE lpData)
{
}

LPBYTE
MenuBinToNat(LPBYTE *lplpData)
{
    LPBYTE lpData;
    int index = 0;
    MENUDATA *md = NULL;
    int len;

    lpData = *lplpData;
    while(TRUE) {
	if (!md)
	    md = (MENUDATA *)WinMalloc(sizeof(MENUDATA));
	else
	    md = (MENUDATA *)WinRealloc((LPSTR)md,sizeof(MENUDATA)*(++index+1));
	if (!md)
	    return NULL;
	md[index].menuflag = GETWORD(lpData);
	lpData += 2;
	if (md[index].menuflag & MF_POPUP) 
	    md[index].menuid = 0;
	else {
	    md[index].menuid = GETWORD(lpData);
	    lpData += 2;
	}
	len = strlen((LPSTR)lpData) + 1;
	md[index].menuname = WinMalloc(len);
	strcpy(md[index].menuname,(LPSTR)lpData);
	lpData += len;
	if (md[index].menuflag & MF_POPUP) 
	    md[index].menupopup = (MENUDATA *)MenuBinToNat(&lpData);
    	if (md[index].menuflag & MF_END)
	    break;
    }
    *lplpData = lpData;
    return (LPBYTE)md;
}

static LPBYTE 
DialogBinToNat(TYPEINFO *ResourceTable,LPBYTE lpData)
{
    DIALOGDATA *lpdd;
    CONTROLDATA *lpcd;
    int len,i;
    WORD wNumControls;

    if (!(lpdd = (DIALOGDATA *)WinMalloc(sizeof(DIALOGDATA))))
	return NULL;
	memset((LPSTR)lpdd,'\0',sizeof(DIALOGDATA));
    lpdd->lStyle = GETDWORD(lpData);
    lpdd->bNumberOfItems = *(lpData+4);
    wNumControls = (WORD)lpdd->bNumberOfItems;
    lpdd->x = GETWORD(lpData+5);
    lpdd->y = GETWORD(lpData+7);
    lpdd->cx = GETWORD(lpData+9);
    lpdd->cy = GETWORD(lpData+11);
    lpData += 13;
    if (*lpData) {
	if (*lpData == 0xff) {
	    lpdd->szMenuName = (LPSTR)StringFromOrdinal
			(ResourceTable,
			 (WORD)(DWORD)RT_MENU|0x8000,
			 GETWORD(lpData+1));
	    lpData += 3;
	}
	else {
	    len = strlen((LPSTR)lpData)+1;
	    lpdd->szMenuName = WinMalloc(len);
	    strcpy(lpdd->szMenuName,(LPSTR)lpData);
	    lpData += len;
	}
    }
    else lpData++;
    if (*lpData) {
	len = strlen((LPSTR)lpData) + 1;
	lpdd->szClassName = WinMalloc(len);
	strcpy(lpdd->szClassName,(LPSTR)lpData);
	lpData += len;
    }
    else lpData++;
    if (*lpData) {
	len = strlen((LPSTR)lpData) + 1;
	lpdd->szCaption = WinMalloc(len);
	strcpy(lpdd->szCaption,(LPSTR)lpData);
	lpData += len;
    }
    else lpData++;
    if (lpdd->lStyle & DS_SETFONT) {
	lpdd->wPointSize = GETWORD(lpData);
	lpData += 2;
	len = strlen((LPSTR)lpData) + 1;
	lpdd->szFaceName = WinMalloc(len);
	strcpy(lpdd->szFaceName,(LPSTR)lpData);
	lpData += len;
    }
    if (wNumControls == 0) {
	lpdd->controlinfo = NULL;
	return (LPBYTE)lpdd;
    }

    lpcd = (CONTROLDATA *)WinMalloc(sizeof(CONTROLDATA)*wNumControls);
    if (!lpcd)
	return NULL;
    memset((LPSTR)lpcd,'\0',sizeof(CONTROLDATA)*wNumControls);
    for (i=0; i < (int)wNumControls; i++) {
	lpcd[i].x = GETWORD(lpData);
	lpcd[i].y = GETWORD(lpData+2);
	lpcd[i].cx = GETWORD(lpData+4);
	lpcd[i].cy = GETWORD(lpData+6);
	lpcd[i].wID = GETWORD(lpData+8);
	lpcd[i].lStyle = GETDWORD(lpData+10);
	lpData += 14;
	if (*lpData >= 0x80) 
	    lpcd[i].szClass = (LPSTR)WinStrdup(std_controls[*lpData++ & ~0x80]);
	else {
	    len = strlen((LPSTR)lpData) + 1;
	    lpcd[i].szClass = WinMalloc(len);
	    strcpy(lpcd[i].szClass,(LPSTR)lpData);
	    lpData += len;
	}
	if (*lpData != 0xff) {
	    len = strlen((LPSTR)lpData) + 1;
	    lpcd[i].szText = WinMalloc(len);
	    strcpy(lpcd[i].szText,(LPSTR)lpData);
	    lpData += len;
	}
	else {
	    lpcd[i].szText = StringFromOrdinal
			(ResourceTable,
			 (WORD)(DWORD)RT_DIALOG,
			 GETWORD(lpData+1));
	    lpData += 3;
	}
	lpData++;	/* undocumented extra byte */
    }
    lpdd->controlinfo = lpcd;
    return (LPBYTE)lpdd;
}

void
GetBIHeader(BITMAPINFOHEADER *lpbih,LPBYTE lpData)
{
    lpbih->biSize = GETDWORD(lpData);
    if (lpbih->biSize == 40) {
	lpbih->biWidth = GETDWORD(lpData+4);
	lpbih->biHeight = GETDWORD(lpData+8);
	lpbih->biPlanes = GETWORD(lpData+12);
	lpbih->biBitCount = GETWORD(lpData+14);
	lpbih->biCompression = GETDWORD(lpData+16);
	lpbih->biSizeImage = GETDWORD(lpData+20);
	lpbih->biXPelsPerMeter = GETDWORD(lpData+24);
	lpbih->biYPelsPerMeter = GETDWORD(lpData+28);
	lpbih->biClrUsed = GETDWORD(lpData+32);
	lpbih->biClrImportant = GETDWORD(lpData+36);
    } else if (lpbih->biSize == 12) {
	lpbih->biWidth = GETWORD(lpData+4);
	lpbih->biHeight = GETWORD(lpData+6);
	lpbih->biPlanes = GETWORD(lpData+8);
	lpbih->biBitCount = GETWORD(lpData+10);
	lpbih->biCompression = 0;
	lpbih->biSizeImage = 0;
	lpbih->biXPelsPerMeter = 0;
	lpbih->biYPelsPerMeter = 0;
	if (lpbih->biBitCount == 1) {
		lpbih->biClrUsed = 2;
		lpbih->biClrImportant = 2;
	} else if (lpbih->biBitCount == 4) {
		lpbih->biClrUsed = 0;
		lpbih->biClrImportant = 16;
	} else if (lpbih->biBitCount == 8) {
		lpbih->biClrUsed = 0;
		lpbih->biClrImportant = 256;
	}
    } else {
	return;
    }
    if (!lpbih->biCompression) {
	if (lpbih->biBitCount == 1) {
	    lpbih->biSizeImage = (lpbih->biWidth + 31)/32;
	    lpbih->biSizeImage <<= 2;
	    lpbih->biSizeImage *= lpbih->biHeight;
	} else if (lpbih->biBitCount == 4) {
	    lpbih->biSizeImage = (lpbih->biWidth + 7)/8;
	    lpbih->biSizeImage <<= 2;
	    lpbih->biSizeImage *= lpbih->biHeight;
	} else if (lpbih->biBitCount == 8) {
	    lpbih->biSizeImage = (lpbih->biWidth + 3)/4;
	    lpbih->biSizeImage <<= 2;
	    lpbih->biSizeImage *= lpbih->biHeight;
	}
    }
}

void 
PutBIHeader(LPBYTE lpData, BITMAPINFOHEADER *lpbih)
{
    PUTDWORD(lpData,lpbih->biSize);
    if (lpbih->biSize == 40) {
	PUTDWORD(lpData+4,lpbih->biWidth);
	PUTDWORD(lpData+8,lpbih->biHeight);
	PUTWORD(lpData+12,lpbih->biPlanes);
	PUTWORD(lpData+14,lpbih->biBitCount);
	PUTDWORD(lpData+16,lpbih->biCompression);
	PUTDWORD(lpData+20,lpbih->biSizeImage);
	PUTDWORD(lpData+24,lpbih->biXPelsPerMeter);
	PUTDWORD(lpData+28,lpbih->biYPelsPerMeter);
	PUTDWORD(lpData+32,lpbih->biClrUsed);
	PUTDWORD(lpData+36,lpbih->biClrImportant);
    }
    else if (lpbih->biSize == 12) {
	PUTWORD(lpData+4,lpbih->biWidth);
	PUTWORD(lpData+6,lpbih->biHeight);
	PUTWORD(lpData+8,lpbih->biPlanes);
	PUTWORD(lpData+10,lpbih->biBitCount);
    }
}

LPBYTE
PutBITMAPINFO(LPBITMAPINFO lpbmi)
{
    WORD wRGBCount;
    LPBYTE lpData;
    LPBYTE ptr;
    int i;

    wRGBCount = 1 << lpbmi->bmiHeader.biBitCount;
    lpData = (LPBYTE)WinMalloc(lpbmi->bmiHeader.biSize + wRGBCount*4);
    ptr = lpData;
    PutBIHeader(ptr,&lpbmi->bmiHeader);
    ptr += lpbmi->bmiHeader.biSize;
    if (wRGBCount) 
	for (i=0; i<(int)wRGBCount; i++) {
	    *ptr++ = lpbmi->bmiColors[i].rgbBlue;
	    *ptr++ = lpbmi->bmiColors[i].rgbGreen;
	    *ptr++ = lpbmi->bmiColors[i].rgbRed;
	    if (lpbmi->bmiHeader.biSize == 40)
		*ptr++ = lpbmi->bmiColors[i].rgbReserved;
	}
    return lpData;
}


void
GetBITMAPINFO(BITMAPINFO *lpbi, LPBYTE lpBin)
{
	BITMAPINFOHEADER	*lpbih;
	int			nColors, i;

	lpbih = (BITMAPINFOHEADER*)lpbi;
	GetBIHeader(lpbih, lpBin);
	nColors = 1 << lpbih->biBitCount;

	lpBin += SIZEOF_BITMAPINFOHEADER_BIN;
	for ( i = 0; i < nColors; i++ )
		memcpy((LPSTR)&lpbi->bmiColors[i], (LPSTR)lpBin+i*4, 4);
}

void
GetLOGFONT(LPLOGFONT lplf, LPBYTE lpStruct)
{
	lplf->lfHeight      = GETSHORT(lpStruct);
	lplf->lfWidth       = GETSHORT(lpStruct+2);
	lplf->lfEscapement  = GETSHORT(lpStruct+4);
	lplf->lfOrientation = GETSHORT(lpStruct+6);
	lplf->lfWeight      = GETSHORT(lpStruct+8);
	memcpy((LPSTR)&lplf->lfItalic,(LPSTR)lpStruct+10,LF_FACESIZE+8);
}

void
PutLOGFONT(LPBYTE lpStruct,LPLOGFONT lplf,UINT cbSize)
{
    if (cbSize >= 2) {
	PUTWORD(lpStruct,(WORD)lplf->lfHeight);
	if (cbSize >= 4) {
	    PUTWORD(lpStruct+2,(WORD)lplf->lfWidth);
	    if (cbSize >= 6) {
		PUTWORD(lpStruct+4,(WORD)lplf->lfEscapement);
		if (cbSize >= 8) {
		    PUTWORD(lpStruct+6,(WORD)lplf->lfOrientation);
		    if (cbSize >= 10) {
			PUTWORD(lpStruct+8,(WORD)lplf->lfWeight);
			memcpy((LPSTR)lpStruct+10,(LPSTR)&lplf->lfItalic,
					min(LF_FACESIZE+8,cbSize-10));
		    }
		}
	    }
	}
    }
}

static LPSTR
ReadPascalString(LPBYTE lp)
{
    int len;
    LPSTR lpString;

    if (!(len = *lp))
	return NULL;
    lpString = WinMalloc(len+1);
    memcpy(lpString,(LPSTR)lp+1,len);
    lpString[len] = '\0';
    return lpString;
}

static int
GetString(LPSTR *lpDest, LPSTR lpSource)
{
    LPSTR lpString;
    int len;

    if (!*lpSource) {
	*lpDest = NULL;
	return 1;
    }
    len = strlen(lpSource) + 1;
    lpString = WinMalloc(len);
    if (lpString) 
	strcpy(lpString,lpSource);
    *lpDest = lpString;
    return len;
}

static LPSTR 
StringFromOrdinal(TYPEINFO *ResourceTable, WORD wType, WORD wID)
{
    NAMEENTRY *lpNameTable = NULL;
    NAMEINFO *lpNameInfo;
    int i = 0;
    BOOL bByName;

    bByName = (wType & 0x8000)?TRUE:FALSE;
    wType &= 0x7fff;
    if (ResourceTable[0xf].rcscount > 0) {
	lpNameInfo = ResourceTable[0xf].rcsinfo;
	if (!lpNameInfo)
	    return (LPSTR)(wID?wID:wType);
	lpNameTable = (NAMEENTRY *)lpNameInfo->rcsdata;
    }
    else 
	return (LPSTR)(wID?wID:wType);
    if (!lpNameTable)
	return (LPSTR)(wID?wID:wType);
    while (lpNameTable[i].wTypeOrdinal) {
	if ((lpNameTable[i].wTypeOrdinal & 0x7fff) == wType) {
	    if (!wID)
		return (bByName && lpNameTable[i].szType)?
			lpNameTable[i].szType:(LPSTR)(DWORD)wType;
	    if (lpNameTable[i].wIdOrdinal == wID)
		return lpNameTable[i].szId;
	}
	i++;
    }
    return (LPSTR)(wID?wID:wType);
}

LPBYTE
ExtractDialog(HINSTANCE hInst, LPBYTE lpData)
{
    TYPEINFO *ResourceTable;
    MODULEINFO *modinfo;
    HANDLE hModule;

    if (hInst == 0) {
	hModule = GetModuleHandle("USER");
	modinfo = GETMODULEINFO(hModule);
    }
    else {
	if (!(modinfo = GETMODULEINFO(hInst))) {
	    hModule = GetModuleFromInstance(hInst);
	    if (!(modinfo = GETMODULEINFO(hModule))) {
	        return NULL;
	    }
	}
    }
    if (!(ResourceTable = modinfo->ResourceTable)) {
	RELEASEMODULEINFO(modinfo);
	return NULL;
    }
    RELEASEMODULEINFO(modinfo);
    return DialogBinToNat(ResourceTable,lpData);
}

void
CleanupDialog(LPBYTE lpData)
{
    DIALOGDATA *lpdd;
    BYTE i;

    lpdd = (DIALOGDATA *)lpData;
    if (HIWORD(lpdd->szMenuName))
	WinFree(lpdd->szMenuName);
    if (HIWORD(lpdd->szClassName))
	WinFree(lpdd->szClassName);
    if (HIWORD(lpdd->szCaption))
	WinFree(lpdd->szCaption);
    if (HIWORD(lpdd->szFaceName))
	WinFree(lpdd->szFaceName);
    for (i=0; i<lpdd->bNumberOfItems; i++) {
	if (HIWORD(lpdd->controlinfo[i].szClass))
	    WinFree(lpdd->controlinfo[i].szClass);
	if (HIWORD(lpdd->controlinfo[i].szText))
	    WinFree(lpdd->controlinfo[i].szText);
    }
    WinFree((LPSTR)lpdd->controlinfo);
#ifdef LATER
    WinFree((LPSTR)lpdd);
#endif
}

LPBYTE
ConvertResourceToBin(LPBYTE lpData, WORD wType)
{
    if (wType < MAXSYSTYPE)
	return (LPBYTE)((ConvRoutineTable[wType].lpfnNatToBin)(lpData));
    else
	return lpData;
}

static LPBYTE
ConvertDIALOGDATA(DIALOGDATA *lpdd)
{
        LPWORD lpcur;
        LPDLGTEMPLATE lpdt;
        LPDLGITEMTEMPLATE lpdit;
        CONTROLDATA *lpcd;
        int i,len;
        WORD wNumControls;

        /* Begin to parse DIALOGDATA data into DLGTEMPLATE structure */
        wNumControls = lpdd->bNumberOfItems;

        /* Calculate space for DLGTEMPLATE allowing extra space for
	   all of the WORD and DWORD alignment requirements (extra is okay)
	*/
	len = sizeof(DLGTEMPLATE);
	if (lpdd->szMenuName)
		len += strlen(lpdd->szMenuName);
	len += sizeof(WORD);
	if (lpdd->szClassName)
		len += strlen(lpdd->szClassName);
	len += sizeof(WORD);
	if (lpdd->szCaption)
		len += strlen(lpdd->szCaption);
	len += sizeof(WORD);
        if (lpdd->lStyle & DS_SETFONT && lpdd->szFaceName)
		len += strlen(lpdd->szFaceName);
	len += 2 * sizeof(WORD);

	for (i = 0; i < wNumControls; i++ ) {
		len += sizeof(DLGITEMTEMPLATE) + sizeof(DWORD);
		if (lpdd->controlinfo[i].szClass)
			len += strlen(lpdd->controlinfo[i].szClass);
		len += 2 * sizeof(WORD);
		if (lpdd->controlinfo[i].szText) {
			if ( HIWORD(lpdd->controlinfo[i].szText) ) 
				len += strlen(lpdd->controlinfo[i].szText);
			else
				len += 2 * sizeof(WORD);
		}
		len += 2 * sizeof(WORD) + sizeof(DWORD);
	}

        if (!(lpdt = (LPDLGTEMPLATE)WinMalloc(len)))
                return NULL;
        memset((LPSTR)lpdt,'\0',len);

        lpdt->style = lpdd->lStyle;
        lpdt->dwExtendedStyle = lpdd->dwExtStyle;
        lpdt->cdit = lpdd->bNumberOfItems;

        lpdt->x = lpdd->x;
        lpdt->y = lpdd->y;
        lpdt->cx = lpdd->cx;
	lpdt->cy = lpdd->cy;
	
        lpcur = (LPWORD)((LPBYTE)lpdt + sizeof(DLGTEMPLATE));
        /*  3 variable length strings follow DLGTEMPLATEs...position
            lpcur to the beginning of the first string */

	if (lpdd->szMenuName) {
		strcpy((LPSTR)lpcur, lpdd->szMenuName);
		lpcur += ALIGN_16BIT(strlen(lpdd->szMenuName) + 1) / 2;
	}
	else
		*lpcur++ = 0;

        /* Second array is a class array that identifies the window class
           of the control, if it is 0x00 then it uses the predefined
           dialog box */
        if (lpdd->szClassName) {
		strcpy((LPSTR)lpcur, lpdd->szClassName);
		lpcur += ALIGN_16BIT(strlen(lpdd->szClassName) + 1) / 2;
	}
	else
		*lpcur++ = 0;

        /* Third array is the title array, if it is 0x00 then there is no
           title.  */
        if (lpdd->szCaption) {
		strcpy((LPSTR)lpcur, lpdd->szCaption);
		lpcur += ALIGN_16BIT(strlen(lpdd->szCaption) + 1) / 2;
	}
	else
		*lpcur++ = 0;

        /*  The 16 bit point size value and typeface array follow the title
            array, but only if the style member specifies DS_SETFONT */
        if (lpdd->lStyle & DS_SETFONT) {
		*lpcur++ = lpdd->wPointSize;
		strcpy((LPSTR)lpcur, lpdd->szFaceName);
		lpcur += ALIGN_16BIT(strlen(lpdd->szFaceName) + 1) / 2;
        }

        for (i = 0; i < (int)wNumControls; i++)
        {
		lpcd = &lpdd->controlinfo[i];

	        lpdit = (LPDLGITEMTEMPLATE)ALIGN_32BIT(lpcur);

                /* create a CONTROLDATA structure for each DLGITEMTEMPLATE */
                lpdit->x = lpcd->x;
                lpdit->y = lpcd->y;
                lpdit->cx = lpcd->cx;
                lpdit->cy = lpcd->cy;
                lpdit->id = lpcd->wID;
                lpdit->style = lpcd->lStyle;
                lpdit->dwExtendedStyle = lpcd->dwExtStyle;

                lpcur = (LPWORD)((LPBYTE)lpdit + sizeof(DLGITEMTEMPLATE));

                /* this is the class name, it must exist */
		strcpy((LPSTR)lpcur, lpcd->szClass);
		lpcur += ALIGN_16BIT(strlen((LPSTR)lpcur) + 1) / 2;

                /* this is the title name, it must exist but may be an id */
		if (lpcd->szText)
			if ( HIWORD(lpcd->szText) ) {
				strcpy((LPSTR)lpcur, lpcd->szText);
				lpcur += ALIGN_16BIT(strlen((LPSTR)lpcur)+1)/2;
			}
			else {
				*lpcur++ = 0xFFFF;
				*lpcur++ = LOWORD(lpcd->szText);
			}
		else {
			strcpy((LPSTR)lpcur, "");
			lpcur++;
		}

                lpcur = (LPWORD)ALIGN_32BIT(lpcur);

        /* There needs to be code to handle the creation data array here
	   Currently the creation data is not stored in the CONTROLINFO
	   For now, just set the length to zero (no creation data)
	*/

		*lpcur++ = 0;

	} /* endfor() */

	return (LPBYTE)lpdt;
}

LPBYTE
ConvertResourceToNat(LPBYTE lpData, WORD wType)
{
    LPBYTE lpMemory;
    LPBITMAPIMAGE lpbmi;
    UINT uRGBByteCount;

    switch (wType) {
	case (WORD)(DWORD)RT_BITMAP:
	    lpbmi = (LPBITMAPIMAGE)lpData;
	    uRGBByteCount = (1 << lpbmi->bmp_hdr->biBitCount)*sizeof(RGBQUAD);
	    lpMemory = (LPBYTE)WinMalloc(lpbmi->bmp_hdr->biSize +
			uRGBByteCount + lpbmi->bmp_hdr->biSizeImage);
	    *((LPBITMAPINFOHEADER)lpMemory) = *lpbmi->bmp_hdr;
	    memcpy((LPSTR)((LPBITMAPINFOHEADER)lpMemory+1),
			(LPSTR)lpbmi->rgb_quad, uRGBByteCount);
	    memcpy((LPSTR)((LPBITMAPINFOHEADER)lpMemory+1)+uRGBByteCount,
			(LPSTR)lpbmi->bitmap_bits,lpbmi->bmp_hdr->biSizeImage);
	    return lpMemory;

        case (WORD)(DWORD)RT_DIALOG:
            lpMemory = ConvertDIALOGDATA((DIALOGDATA *)lpData);
            return lpMemory;

	default:
	    return lpData;
    }
}
