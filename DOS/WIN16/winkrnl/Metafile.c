/*    
	Metafile.c	2.20
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

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "windows.h"

#include "kerndef.h"
#include "Endian.h"
#include "GdiDC.h"
#include "GdiObjects.h"
#include "Log.h"
#include "Meta.h"
#include "PrinterBin.h"

#define MAX_RECORD_SIZE 	(4*1024)
#define BUF_SIZE		(MAX_RECORD_SIZE * 2)

static BOOL TWIN_DiskMetaFileRecorder(LPINTERNALMETAFILE, LPWORD);
static BOOL TWIN_MemoryMetaFileRecorder(LPINTERNALMETAFILE, LPWORD);

void META_PUTWORD(LPVOID , WORD );
void META_PUTDWORD(LPVOID , DWORD );

HDC WINAPI
CreateMetaFile(LPCSTR filename)
{
	LPINTERNALMETAFILE	lpIntMeta;
	HDC			hDC;
	HDC32			hDC32;

	APISTR((LF_APICALL, "CreateMetaFile(%s)\n",(filename)?filename:"NULL"));

	lpIntMeta = (LPINTERNALMETAFILE)WinMalloc(sizeof(INTERNALMETAFILE));

	if ( !(hDC = CreateDC("META", 0, 0, 0)) ) {
		WinFree((void*)lpIntMeta);
		APISTR((LF_APIFAIL, "CrateMetaFile: return HDC 0\n"));
		return (HDC)NULL;
	}

	memset(lpIntMeta, 0x00, sizeof(INTERNALMETAFILE));
	if ( filename  &&  filename[0] ) {
		if ( (lpIntMeta->fd = _lcreat(filename, 0)) == HFILE_ERROR ) {
			APISTR((LF_APICALL,"CreateMetaFile: return HDC 0\n"));
			WinFree((void*)lpIntMeta);
			return (HDC)NULL;
		}
		/* Advance file pointer and leave the space for METAHEADER */
		_llseek(lpIntMeta->fd, METAHEADER_SIZE, 0);
		lpIntMeta->lpfRecorder = TWIN_DiskMetaFileRecorder;
		strcpy(lpIntMeta->szFullPath, filename);
		lpIntMeta->header.mtType = DISK_METAFILE;
	}
	else {
		/* Allocate initial block for memory metafile */
		lpIntMeta->hMetaMem = GlobalAlloc(GHND, METAHEADER_SIZE);
		lpIntMeta->lpfRecorder = TWIN_MemoryMetaFileRecorder;
		lpIntMeta->header.mtType = MEMORY_METAFILE;
	}

	lpIntMeta->header.mtVersion    = WIN_VERSION;
	lpIntMeta->header.mtHeaderSize = METAHEADER_SIZE / sizeof(WORD);
	lpIntMeta->header.mtSize       = METAHEADER_SIZE / sizeof(WORD);

	lpIntMeta->lpObjTable = (LPMETAOBJTABLE) 
			WinMalloc(HANDLE_TABLE_SIZE * sizeof(METAOBJTABLE));
	TWIN_ControlObjTable(lpIntMeta,INIT_OBJ_TBL, 0);

	/*  Get address of DC structure to store  */
	hDC32 = GETHDC32(hDC);
	hDC32->lpDrvData = (LPVOID)lpIntMeta;
	
	APISTR((LF_APIRET, "CreateMetaFile: return HDC %x\n", hDC));
	RELEASEDCINFO(hDC32);
	return hDC;
}

HMETAFILE WINAPI
CloseMetaFile(HDC hDC)
{
	LPINTERNALMETAFILE	lpIntMeta;
	LPDISKMFINTERNAL	lpDiskMF;
	LPBYTE			lpMeta;
	HGLOBAL			hMF;
	WORD			MFheader[METAHEADER_SIZE / sizeof(WORD)];
	WORD			rec[3];
	HDC32			hDC32;

	APISTR((LF_APICALL,"CloseMetaFile(HDC=%x0\n", hDC));

	ASSERT_HDC(hDC32,hDC,0);

	/* Last record in metafile */
	META_PUTDWORD(&(rec[RDSIZE]), sizeof(rec) / sizeof(WORD));
	META_PUTWORD(&(rec[RDFUNC]), 0);
	lpIntMeta = (LPINTERNALMETAFILE)hDC32->lpDrvData;
	lpIntMeta->lpfRecorder(lpIntMeta, (LPWORD)&rec);


	if ( lpIntMeta->header.mtType == MEMORY_METAFILE ) {
		lpMeta = (LPBYTE)GlobalLock(lpIntMeta->hMetaMem);
		memcpy(lpMeta, &(lpIntMeta->header), sizeof(METAHEADER));
		GlobalUnlock(lpIntMeta->hMetaMem);
		hMF = lpIntMeta->hMetaMem;
		if (lpIntMeta->lpObjTable)
		    WinFree((LPSTR)lpIntMeta->lpObjTable);
		WinFree((void*)lpIntMeta);
	}
	else {
		hMF = GlobalAlloc(GHND, sizeof(DISKMFINTERNAL));
		lpDiskMF = (LPDISKMFINTERNAL)GlobalLock(hMF);
		memcpy(&(lpDiskMF->header), &(lpIntMeta->header),
				sizeof(METAHEADER));
		strcpy(lpDiskMF->szFullPath, lpIntMeta->szFullPath);

		/* Convert METAHEADER from native to binary format */
		/* and write it into metafile                      */
		META_PUTWORD(&MFheader[MF_TYPE], lpIntMeta->header.mtType);
		META_PUTWORD(&MFheader[MF_HEADERSIZE],
			lpIntMeta->header.mtHeaderSize);
		META_PUTWORD(&MFheader[MF_VERSION], lpIntMeta->header.mtVersion);
		META_PUTDWORD(&MFheader[MF_SIZE], lpIntMeta->header.mtSize);
		META_PUTWORD(&MFheader[MF_NOOBJECTS], lpIntMeta->header.mtNoObjects);
		META_PUTDWORD(&MFheader[MF_MAXRECORD], 
			lpIntMeta->header.mtMaxRecord);
		META_PUTWORD(&MFheader[MF_NOPARAMS], 
			lpIntMeta->header.mtNoParameters);
		_llseek(lpIntMeta->fd, 0, 0);
		_lwrite(lpIntMeta->fd, MFheader, sizeof(MFheader));
		_lclose(lpIntMeta->fd);

		if (lpIntMeta->lpObjTable)
		    WinFree((LPSTR)lpIntMeta->lpObjTable);
		WinFree((void*)lpIntMeta);
		GlobalUnlock(hMF);
	}
	
	RELEASEDCINFO(hDC32);
	DeleteDC(hDC);

	APISTR((LF_APIRET,"CloseMetaFile: returns HMETAFILE %x\n", hMF));
	return (HMETAFILE)hMF;
}

BOOL WINAPI
DeleteMetaFile(HMETAFILE hMF)
{
	BOOL rc;
	APISTR((LF_APICALL, "DeleteMetaFile(0x%x)\n", hMF));
	if ( GlobalFree(hMF) == 0 )
		rc = TRUE;
	else
		rc = FALSE;
	APISTR((LF_APIRET, "DeleteMetaFile: returns BOOL %d\n", rc));
	
	return rc;
}

/* -------------------------------------------------------------------- */
/* Check metafile validity (undocumented API)				*/
/* -------------------------------------------------------------------- */
BOOL
IsValidMetaFile(HMETAFILE hmf)
{
	LPWORD		lpMF;
	WORD		mtType, mtHeaderSize, mtVersion;
	int		rc = TRUE;

	lpMF = (LPWORD)GlobalLock((HGLOBAL)hmf);
	mtType       = GETWORD(&(lpMF[MF_TYPE]));
	mtHeaderSize = GETWORD(&(lpMF[MF_HEADERSIZE]));
	mtVersion    = GETWORD(&(lpMF[MF_VERSION]));
	if (((mtType != MEMORY_METAFILE)  &&  (mtType != DISK_METAFILE))  ||
		(mtHeaderSize != METAHEADER_SIZE/sizeof(WORD))  ||
		(mtVersion < WIN_VERSION )) {
			rc = FALSE;	/* Invalid metafile */
	}
	GlobalUnlock((HGLOBAL)hmf);
	return rc;
}

HMETAFILE WINAPI
GetMetaFile(LPCSTR filename)
{
	LPWORD		lpMF;
	LPBYTE		lpBuff;
	DWORD		dwPlaceableKey, mtSize;
	DWORD		dwSize, dwSizeMF, dwFileLen;
	HGLOBAL		hMF;
	int		fd, nRead, nBytes;
	WORD		sHeader[METAHEADER_SIZE / sizeof(WORD)];

	APISTR((LF_APICALL, "GetMetaFile(%s)\n", (filename) ? filename : "NULL"));
	if ( (fd = _lopen(filename, READ)) == HFILE_ERROR ) {
		APISTR((LF_APIFAIL, "GetMetaFile: return HMETAFILE %d\n",0));
		return (HMETAFILE) 0;
	}
	dwFileLen = _llseek(fd, 0, 2);
	_llseek(fd, 0, 0);
	if ( _lread(fd, (LPBYTE)sHeader, METAHEADER_SIZE) != METAHEADER_SIZE ) {
		_lclose(fd);
		APISTR((LF_APIFAIL, "GetMetaFile: return HMETAFILE %d\n",0));
		return (HMETAFILE) 0;
	}
	dwPlaceableKey = GETDWORD(sHeader);
	if ( dwPlaceableKey == EXT_HEADER_KEY ) {
		_llseek(fd, METAFILEHEADER_SIZE, 0);
        	if (_lread(fd,(LPBYTE)sHeader,METAHEADER_SIZE)!=METAHEADER_SIZE) {
		       _lclose(fd);
		       APISTR((LF_APIFAIL,"GetMetaFile: return HMETAFILE 0\n"));
		       return (HMETAFILE) 0;
		}
		dwFileLen -= METAFILEHEADER_SIZE;
	}
        mtSize = GETDWORD(sHeader+3) - METAHEADER_SIZE / sizeof(WORD);
	/* Calculate the actual size of METAFILE structure           */
	/* It's a length of file minus the length of METAFILEHEADER, */
	/* if any.                                                   */
	dwSizeMF = dwFileLen;		/* = length of metafile itself */
	if ( (hMF = GlobalAlloc(GHND, dwSizeMF)) == (HMETAFILE) 0 ) {
		APISTR((LF_APIFAIL,"GetMetaFile: return HMETAFILE 0\n"));
		return (HMETAFILE) 0;
	}

	lpMF = (LPWORD)GlobalLock(hMF);
	memcpy((LPBYTE)lpMF, (LPBYTE)sHeader, METAHEADER_SIZE);

	lpBuff = (LPBYTE)lpMF + METAHEADER_SIZE;	/* Ptr to records */
	dwSize = dwSizeMF - METAHEADER_SIZE;
	nRead = min((USHRT_MAX-1) & 0xfff0, dwSize);
	do {
		if ( (nBytes = _lread(fd, lpBuff, nRead)) == -1 ) {
			_lclose(fd);
			GlobalUnlock(hMF);
			GlobalFree(hMF);
			APISTR((LF_APICALL, "GetMetaFile: return HMETAFILE 0\n"));
			return (HMETAFILE) 0;
		}
		lpBuff += nBytes;
	} while ( nBytes <= nRead  &&  nBytes );

	_lclose(fd);
	GlobalUnlock(hMF);
	if ( !IsValidMetaFile(hMF) ) {
		/* Invalid metafile */
		GlobalFree(hMF);
		APISTR((LF_APICALL, "GetMetaFile: return HMETAFILE 0\n"));
		return (HMETAFILE)NULL;
	}

	APISTR((LF_APIRET, "GetMetaFile: returns HMETAFILE %d\n", hMF));
	return (HMETAFILE)hMF;
}


HGLOBAL WINAPI
GetMetaFileBits(HMETAFILE hMF)
{
	APISTR((LF_APICALL,"GetMetaFileBits(0x%x)\n", hMF));
	APISTR((LF_APIRET, "GetMetaFileBits: returns HGLOBAL %x\n",hMF));
	return (HGLOBAL)hMF;
}

UINT WINAPI
GetMetaFileBitsEx(HMETAFILE hMF, UINT nSize, LPVOID lpData)
{
    APISTR((LF_APISTUB, "GetMetaFileBitsEx(HMETAFILE=%x, UINT=%x, LPVOID=%x)\n",
		hMF, nSize, lpData));

    return 0;
}

HMETAFILE WINAPI
CopyMetaFile(HMETAFILE hMFSrc, LPCSTR lpszFile)
{
	HDC		hDC;
	HMETAFILE	hMF;

	APISTR((LF_APICALL,"CopyMetaFile(0x%x, %s)\n", hMFSrc, 
		(lpszFile) ? lpszFile : "NULL"));

	if ( !IsValidMetaFile(hMFSrc)  ||  !(hDC = CreateMetaFile(lpszFile)) ) {
		APISTR((LF_APIFAIL,"CopyMetaFile: return HMETAFILE 0\n"));
		return (HMETAFILE)NULL;
	}

	if ( !PlayMetaFile(hDC, hMFSrc) ) {
		/* Cleanup */
		if ((hMF = CloseMetaFile(hDC)))
			DeleteMetaFile(hMF);

		if ( lpszFile )
			unlink(lpszFile);

		APISTR((LF_APIFAIL,"CopyMetaFile: return HMETAFILE 0\n"));
		return (HMETAFILE)NULL;
	}

	hMF = CloseMetaFile(hDC);
	APISTR((LF_APIRET, "CopyMetaFile: returns HMETAFILE %x\n", hMF));
	return hMF;
}

HMETAFILE WINAPI
SetMetaFileBits(HGLOBAL hGlobal)
{
    APISTR((LF_APICALL, "SetMetaFileBits(0x%x)\n", hGlobal));
    APISTR((LF_APIRET, "SetMetaFileBits: returns HMETAFILE %x\n", hGlobal));
    return (HMETAFILE)hGlobal;
}

HMETAFILE WINAPI
SetMetaFileBitsBetter(HGLOBAL hGlobal)
{
    APISTR((LF_APICALL, "SetMetaFileBitsBetter(HGLOBAL=%x)\n", hGlobal));
    APISTR((LF_APIRET, "SetMetaFileBitsBetter: returns HMETAFILE %x\n",
	hGlobal));

    return (HMETAFILE)hGlobal;
}

HMETAFILE WINAPI
SetMetaFileBitsEx(UINT nSize, LPBYTE lpData)
{
    APISTR((LF_APISTUB, "SetMetaFileBitsEx(0x%x, 0x%x)\n", nSize, lpData));

    return (HMETAFILE)NULL;
}

static BOOL
TWIN_MemoryMetaFileRecorder(LPINTERNALMETAFILE lpIntMeta, LPWORD lpRec)
{
	DWORD	nWords, dwMFLen;
	LPBYTE	lpMeta;

	nWords = GETDWORD(&(lpRec[RDSIZE]));
	dwMFLen = (lpIntMeta->header.mtSize + nWords) * sizeof(WORD);

	lpIntMeta->hMetaMem = GlobalReAlloc(lpIntMeta->hMetaMem, dwMFLen,
				GMEM_ZEROINIT|GMEM_MOVEABLE);
	lpMeta = (LPBYTE)GlobalLock(lpIntMeta->hMetaMem);

	memcpy((LPVOID)(lpMeta + lpIntMeta->header.mtSize*sizeof(WORD)),
		(LPVOID)lpRec, nWords*sizeof(WORD));
	GlobalUnlock(lpIntMeta->hMetaMem);
	lpIntMeta->header.mtSize += nWords;		/* New size */
	lpIntMeta->header.mtMaxRecord = max(lpIntMeta->header.mtMaxRecord,
						nWords);
	return TRUE;
}

static BOOL
TWIN_DiskMetaFileRecorder(LPINTERNALMETAFILE lpIntMeta, LPWORD lpRec)
{
	DWORD	nWords;

	nWords = GETDWORD(&(lpRec[RDSIZE]));
	lpIntMeta->header.mtSize += nWords;
	if ( _lwrite(lpIntMeta->fd, lpRec, nWords*sizeof(WORD)) == -1 )
		return FALSE;
	else {
		lpIntMeta->header.mtMaxRecord = 
				max(lpIntMeta->header.mtMaxRecord, nWords);
		return TRUE;
	}
}

LONG
TWIN_mf_CreateBrushIndirect(HDC hDC, void *lpLogObj)
{
    HDC32 hDC32;
    HBITMAP hBitmap;
    LPINTERNALMETAFILE	lpIntMeta;
    LPLOGBRUSH		lpLogBrush = (LPLOGBRUSH)lpLogObj;
    BITMAP		bm;
    LPBITMAPINFO	lpbmi;
    LPVOID		lpBits;
    BITMAPINFO		bmi;
    LPSTR		lpDIBInfo;
    WORD		rec[7];
    DWORD		dwRecSize;
    LPWORD		lpRec;
    int			nNumColors;

    ASSERT_HDC(hDC32,hDC,0L);

    if (lpLogBrush->lbStyle != BS_PATTERN &&
	lpLogBrush->lbStyle != BS_DIBPATTERN) {
	META_PUTDWORD(&(rec[RDSIZE]), sizeof(rec) / sizeof(WORD));
	META_PUTWORD(&(rec[RDFUNC]), META_CREATEBRUSHINDIRECT);
	META_PUTWORD(&(rec[3]), lpLogBrush->lbStyle);
	META_PUTDWORD(&(rec[4]), lpLogBrush->lbColor);
	META_PUTWORD(&(rec[6]), lpLogBrush->lbHatch);
	lpIntMeta = (LPINTERNALMETAFILE)hDC32->lpDrvData;
	lpIntMeta->lpfRecorder(lpIntMeta, (LPWORD)&rec);
	return (LONG)TRUE;
    }

    hBitmap = (HBITMAP)lpLogBrush->lbHatch;
    if (GetObject(hBitmap,sizeof(BITMAP),(LPSTR)&bm) == 0) {
        RELEASEDCINFO(hDC32);
	return (LONG)0;
    }

    memset((LPVOID)&bmi,'\0', sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth  = bm.bmWidth;
    bmi.bmiHeader.biHeight = bm.bmHeight;
    bmi.bmiHeader.biPlanes = bm.bmPlanes;
    bmi.bmiHeader.biBitCount = bm.bmBitsPixel;
    bmi.bmiHeader.biCompression = BI_RGB;
    GetDIBits(hDC,hBitmap,0,bm.bmHeight,(LPVOID)NULL,&bmi,DIB_RGB_COLORS);
    if (bmi.bmiHeader.biSizeImage == 0)
	return (LONG)FALSE;
    nNumColors = 1 << bm.bmBitsPixel;

    lpDIBInfo = WinMalloc(sizeof(BITMAPINFO) +
		nNumColors*sizeof(RGBQUAD)+
		bmi.bmiHeader.biSizeImage);
    lpbmi = (LPBITMAPINFO)lpDIBInfo;
    lpBits = (LPVOID)(lpDIBInfo +
		sizeof(BITMAPINFO) + nNumColors*sizeof(RGBQUAD));
    *lpbmi = bmi;
    GetDIBits(hDC,hBitmap,0,bm.bmHeight,lpBits,lpbmi,DIB_RGB_COLORS);
    dwRecSize = DWORD_86 + WORD_86 + WORD_86 + WORD_86 +
		SIZEOF_BITMAPINFOHEADER_BIN +
		SIZEOF_RGBQUAD_BIN * nNumColors +
		bmi.bmiHeader.biSizeImage;
    lpRec = (LPWORD)WinMalloc(dwRecSize);
    META_PUTDWORD(&(lpRec[RDSIZE]), dwRecSize / sizeof(WORD));
    META_PUTWORD(&(lpRec[RDFUNC]), META_DIBCREATEPATTERNBRUSH);
    META_PUTWORD(&(lpRec[3]), lpLogBrush->lbStyle);
    META_PUTWORD(&(lpRec[4]), DIB_RGB_COLORS);
    META_PUTDWORD(&(lpRec[5]), bmi.bmiHeader.biSize);
    META_PUTDWORD(&(lpRec[7]), bmi.bmiHeader.biWidth);
    META_PUTDWORD(&(lpRec[9]), bmi.bmiHeader.biHeight);
    META_PUTWORD(&(lpRec[11]), bmi.bmiHeader.biPlanes);
    META_PUTWORD(&(lpRec[12]), bmi.bmiHeader.biBitCount);
    META_PUTDWORD(&(lpRec[13]), bmi.bmiHeader.biCompression);
    META_PUTDWORD(&(lpRec[15]), bmi.bmiHeader.biSizeImage);
    META_PUTDWORD(&(lpRec[17]), bmi.bmiHeader.biXPelsPerMeter);
    META_PUTDWORD(&(lpRec[19]), bmi.bmiHeader.biYPelsPerMeter);
    META_PUTDWORD(&(lpRec[21]), bmi.bmiHeader.biClrUsed);
    META_PUTDWORD(&(lpRec[23]), bmi.bmiHeader.biClrImportant);
    memcpy((LPBYTE)&(lpRec[25]), &lpbmi->bmiColors[0],
			SIZEOF_RGBQUAD_BIN * nNumColors);
    memcpy((LPBYTE)&(lpRec[25+
			(SIZEOF_RGBQUAD_BIN * nNumColors/sizeof(WORD))]),
		lpBits, bmi.bmiHeader.biSizeImage);
    lpIntMeta = (LPINTERNALMETAFILE)hDC32->lpDrvData;
    lpIntMeta->lpfRecorder(lpIntMeta, lpRec);
    WinFree((LPSTR)lpDIBInfo);
    WinFree((LPSTR)lpRec);
    RELEASEDCINFO(hDC32);
    return TRUE;
}

LONG
TWIN_mf_CreateFontIndirect(HDC hDC, void *lpLogObj)
{
	HDC32 hDC32;
	LPINTERNALMETAFILE	lpIntMeta;
	LPLOGFONT		lpLogFont = (LPLOGFONT)lpLogObj;
	LPWORD			lpRec;
	int			nMemLen;

	ASSERT_HDC(hDC32,hDC,0L);

	nMemLen = (8*INT_86 + 8 + strlen(lpLogFont->lfFaceName) + 2) & ~1;
	lpRec = (LPWORD)WinMalloc(nMemLen);
	META_PUTDWORD(&(lpRec[RDSIZE]), nMemLen/sizeof(WORD));
	META_PUTWORD(&(lpRec[RDFUNC]), META_CREATEFONTINDIRECT);
	META_PUTWORD(&(lpRec[3]), lpLogFont->lfHeight);
	META_PUTWORD(&(lpRec[4]), lpLogFont->lfWidth);
	META_PUTWORD(&(lpRec[5]), lpLogFont->lfEscapement);
	META_PUTWORD(&(lpRec[6]), lpLogFont->lfOrientation);
	META_PUTWORD(&(lpRec[7]), lpLogFont->lfWeight);
	memcpy((LPSTR)&(lpRec[8]), &lpLogFont->lfItalic, 8);
	strcpy((LPSTR)&(lpRec[12]), lpLogFont->lfFaceName);

	lpIntMeta = (LPINTERNALMETAFILE)hDC32->lpDrvData;
	lpIntMeta->lpfRecorder(lpIntMeta, lpRec);
	WinFree((LPSTR)lpRec);
	RELEASEDCINFO(hDC32);
	return TRUE;
}

LONG
TWIN_mf_CreatePenIndirect(HDC hDC, void *lpLogObj)
{
	HDC32 hDC32;
	LPINTERNALMETAFILE	lpIntMeta;
	LPLOGPEN		lpLogPen = (LPLOGPEN)lpLogObj;
	WORD			rec[8];

	ASSERT_HDC(hDC32,hDC,0L);

	META_PUTDWORD(&(rec[RDSIZE]), sizeof(rec)/sizeof(WORD));
	META_PUTWORD(&(rec[RDFUNC]), META_CREATEPENINDIRECT);
	META_PUTWORD(&(rec[3]), lpLogPen->lopnStyle);
	META_PUTWORD(&(rec[4]), lpLogPen->lopnWidth.x);
	META_PUTWORD(&(rec[5]), lpLogPen->lopnWidth.y);
	META_PUTDWORD(&(rec[6]), lpLogPen->lopnColor);
	lpIntMeta = (LPINTERNALMETAFILE)hDC32->lpDrvData;
	lpIntMeta->lpfRecorder(lpIntMeta, (LPWORD)&rec);
	RELEASEDCINFO(hDC32);
	return TRUE;
}

LONG
TWIN_mf_CreatePalette(HDC hDC, void *lpLogObj)
{
    HDC32 hDC32;
    LPINTERNALMETAFILE      lpIntMeta;
    LPWORD lpRec;
    HPALETTE32 hPalette32 = (HPALETTE32)lpLogObj;
    int nMemLen,nCount;
    int RDWord = RDPARAM;
    LPPALETTEENTRY lppe;

    ASSERT_HDC(hDC32,hDC,0L);

    nMemLen = 5*WORD_86 + hPalette32->lpLogPalette->palNumEntries*sizeof(PALETTEENTRY);
    lpRec = (LPWORD)WinMalloc(nMemLen);

    META_PUTDWORD(&lpRec[RDSIZE], nMemLen / sizeof(WORD));
    META_PUTWORD(&lpRec[RDFUNC], META_CREATEPALETTE);
    META_PUTWORD(&lpRec[RDWord++], hPalette32->lpLogPalette->palVersion);
    META_PUTWORD(&lpRec[RDWord++], hPalette32->lpLogPalette->palNumEntries);
    lppe = (LPPALETTEENTRY)&lpRec[RDWord];
    for (nCount = 0; nCount < hPalette32->lpLogPalette->palNumEntries; lppe++,nCount++) {
	*lppe = hPalette32->lpLogPalette->palPalEntry[nCount];
    }
    lpIntMeta = (LPINTERNALMETAFILE)hDC32->lpDrvData;
    lpIntMeta->lpfRecorder(lpIntMeta, lpRec);
    WinFree((LPSTR)lpRec);
    RELEASEDCINFO(hDC32);
    return (LONG)TRUE;
}
