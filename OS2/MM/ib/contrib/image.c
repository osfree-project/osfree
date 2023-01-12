/****************************************************************************

    image.c - main line control

    Image is an image viewer for OS/2 + PM + MMPM/2
    Copyright (c) 2001 Mark Kimes
    Copyright (c) 2001 Steven H. Levine

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Revisions	19 Aug 01 MK - Release
		25 Sep 01 SHL - #define PGM_VERSION
		27 Oct 01 SHL - ShowPicture: drop spurious free(wnd)

*****************************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_MMIOOS2

#include <os2.h>
#include <os2me.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <process.h>
#include <math.h>
#include "image.h"

#define PGM_VERSION "0.02"

#pragma data_seg(DATA1)

typedef struct {
  HAB           hab;
  HWND          hwndClient;
  HWND          hwndFrame;
  HBITMAP       hbm;
  char          filename[CCHMAXPATH];
  MMFORMATINFO  mmFormatInfo;
  FOURCC        saveas;
  BOOL          convert;
  BOOL          marking;
  BOOL          iconify;
  RECTL         marked;
  USHORT        bitcount;
  char          isline;
  char          stopflag;
  char          busy;
  char          killme;
} IMAGEDATA;

typedef struct {
  char   filenamel[CCHMAXPATH];
  char   filenames[CCHMAXPATH];
  FOURCC loadas;
  FOURCC saveas;
} CVTIMAGEDATA;

typedef struct {
  ULONG   xsize;
  ULONG   ysize;
  double  ratio;
} RESIZEDATA;

HWND      hwndMain;
HPOINTER  hptrArrow,hptrWait,hptrCross;
#ifdef USE_PALETTE
long      colors,palsup;
HPAL      hpal;
#endif
ULONG     compression,rcompression = BBO_IGNORE,lcompression = BBO_IGNORE;
HPS       hpsWnd;

#define alloc_text(RESIZE,ResizeDlgProc,ResizeProc)
#pragma alloc_text(SAVE,SaveBitmap,Iconify,SaveProc)
#pragma alloc_text(PASTE,PasteBitmap,PasteProc)
#pragma alloc_text(MISC,CopyBitmap,AdjustRectl,FixRectl)
#pragma alloc_text(MISC,SpecialRectl,ReverseRectl,saymsg,ImageInfoProc)
#pragma alloc_text(CVT,ConvertImageThread,ConvertImageDlgProc,ConvertProc)
#pragma alloc_text(DRAG,FullDrgName,DragProc,LoadPicture)
#pragma alloc_text(DRAG,OpenImageDlgProc,InvokeLoadImage)
#pragma alloc_text(LOAD,LoadImageThread,LoadBitmap,GetBitmap,LoadProc)
#pragma alloc_text(ONCE,main,save_dir,ShowPicture,ClipBitmap)


APIRET save_dir (char *curdir) {

  APIRET  ret;
  ULONG   curdirlen,curdrive,drivemap;

  *curdir = 0;
  ret = DosQCurDisk (&curdrive, &drivemap);
  curdirlen = CCHMAXPATH - 4;   /* NOTE!!!!!!!!! */
  ret += DosQCurDir (curdrive, &curdir[3], &curdirlen);
  *curdir = (char)('@' + (INT)curdrive);
  curdir[1] = ':';
  curdir[2] = '\\';
  return ret;
}


APIRET saymsg (APIRET type,HWND hwnd,char *title,char *string,...) {

  char        *buffer;
  va_list     ap;
  APIRET      ret;

  buffer = malloc(4096);
  if(!buffer) {
    WinMessageBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  "Out of memory.",
                  title,
                  0,
                  MB_ENTER);
    return -1;
  }
  va_start(ap,string);
  vsprintf(buffer,string,ap);
  va_end(ap);
  if(!hwnd)
    hwnd = HWND_DESKTOP;
  ret = WinMessageBox(HWND_DESKTOP,
                      hwnd,
                      buffer,
                      title,
                      0,
                      type | MB_MOVEABLE);
  free(buffer);
  return ret;
}


INT IsFile (char *filename) {

  /* returns:  -1 (error), 0 (is a directory), or 1 (is a file) */

  FILESTATUS3 fsa;
  APIRET      ret;

  if(filename) {
    DosError(FERR_DISABLEHARDERR);
    ret = DosQueryPathInfo(filename,FIL_STANDARD,&fsa,
                        (ULONG)sizeof(fsa));
    if(!ret)
      return ((fsa.attrFile & FILE_DIRECTORY) == 0);
  }
  return -1;  /* error; doesn't exist or can't read or null filename */
}


BOOL Iconify (HAB hab,HBITMAP hbmP,PSZ pszFileName) {

  HDC               hdcMemory = DEV_ERROR;
  HPS               hpsMemory = GPI_ERROR;
  SIZEL             ImageSize = {0,0};
  HBITMAP           hbm = HBM_ERROR;
  POINTL            aptl[4];
  BITMAPFILEHEADER2 bfh;
  ULONG             rc = 1,cBitCount;

  if(pszFileName) {
    hdcMemory = DevOpenDC(hab,
                          OD_MEMORY,
                          "*",
                          0L,
                          NULL,
                          0);
    if(hdcMemory != DEV_ERROR) {
      ImageSize.cx = WinQuerySysValue(HWND_DESKTOP,SV_CXICON);
      ImageSize.cy = WinQuerySysValue(HWND_DESKTOP,SV_CYICON);
      hpsMemory = GpiCreatePS(hab,
                          hdcMemory,
                          &ImageSize,
                          PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);
      if(hpsMemory != GPI_ERROR) {
        memset(&bfh,0,sizeof(bfh));
        bfh.bmp2.cbFix = sizeof(BITMAPINFOHEADER2);
        if(GpiQueryBitmapInfoHeader(hbmP,&bfh.bmp2)) {
          cBitCount = bfh.bmp2.cBitCount;
          memset(&bfh,0,sizeof(bfh));
          bfh.bmp2.cbFix = sizeof(BITMAPINFOHEADER2);
          bfh.bmp2.cx = WinQuerySysValue(HWND_DESKTOP,SV_CXICON);
          bfh.bmp2.cy = WinQuerySysValue(HWND_DESKTOP,SV_CYICON);
          bfh.bmp2.cBitCount = cBitCount;
          compression = (bfh.bmp2.cBitCount > 1) ? rcompression : lcompression;
          bfh.bmp2.cPlanes = 1;
          hbm = GpiCreateBitmap(hpsMemory,
                                &bfh.bmp2,
                                0L,
                                NULL,
                                NULL);
          if(hbm != GPI_ERROR) {
            if(GpiSetBitmap(hpsMemory,hbm) == HBM_ERROR) {
              GpiAssociate(hpsMemory,NULLHANDLE);
              GpiDestroyPS(hpsMemory);
              GpiDeleteBitmap(hbm);
              DevCloseDC(hdcMemory);
              return FALSE;
            }

            memset(&bfh,0,sizeof(bfh));
            bfh.bmp2.cbFix = sizeof(BITMAPINFOHEADER2);
            if(!GpiQueryBitmapInfoHeader(hbmP,&bfh.bmp2)) {
              GpiSetBitmap(hpsMemory,(HBITMAP)0);
              GpiAssociate(hpsMemory,NULLHANDLE);
              GpiDestroyPS(hpsMemory);
              GpiDeleteBitmap(hbm);
              DevCloseDC(hdcMemory);
              return FALSE;
            }
            {
              double ratio;
              POINTL ptl;

              ratio = (double)bfh.bmp2.cx / (double)bfh.bmp2.cy;
              /* make unused portion of icon white w/ diagonal stripes */
              GpiSetColor(hpsMemory,CLR_WHITE);
              GpiSetBackColor(hpsMemory,CLR_BLACK);
              GpiSetBackMix(hpsMemory,BM_OVERPAINT);
              GpiSetPattern(hpsMemory,PATSYM_DENSE3);
              ptl.x = ptl.y = -1;
              GpiMove(hpsMemory,&ptl);
              ptl.x = WinQuerySysValue(HWND_DESKTOP,SV_CXICON);
              ptl.y = WinQuerySysValue(HWND_DESKTOP,SV_CYICON);
              GpiBox(hpsMemory,DRO_FILL,&ptl,0,0);

              if(ratio > 1.0) {
                aptl[1].x = WinQuerySysValue(HWND_DESKTOP,SV_CXICON);
                aptl[1].y = (long)((double)(WinQuerySysValue(HWND_DESKTOP,SV_CYICON) - 1) / ratio);
                if(aptl[1].y < 4)
                  aptl[1].y = 4;
                aptl[0].x = 0;
                aptl[0].y = (WinQuerySysValue(HWND_DESKTOP,SV_CYICON) - aptl[1].y) / 2;
                aptl[1].y += aptl[0].y;
                if(aptl[1].y > WinQuerySysValue(HWND_DESKTOP,SV_CYICON))
                  aptl[1].y = WinQuerySysValue(HWND_DESKTOP,SV_CYICON);
              }
              else {
                aptl[1].x = (long)((double)(WinQuerySysValue(HWND_DESKTOP,SV_CXICON) - 1) * ratio);
                aptl[1].y = WinQuerySysValue(HWND_DESKTOP,SV_CYICON);
                if(aptl[1].x < 4)
                  aptl[1].x = 4;
                aptl[0].y = 0;
                aptl[0].x = (WinQuerySysValue(HWND_DESKTOP,SV_CXICON) - aptl[1].x) / 2;
                aptl[1].x += aptl[0].x;
                if(aptl[1].x > WinQuerySysValue(HWND_DESKTOP,SV_CXICON))
                  aptl[1].x = WinQuerySysValue(HWND_DESKTOP,SV_CXICON);
              }
            }

            aptl[2].x = 0;               /* source lower left */
            aptl[2].y = 0;

            aptl[3].x = bfh.bmp2.cx;     /* source upper right */
            aptl[3].y = bfh.bmp2.cy;

            if(GpiWCBitBlt(hpsMemory,
                           hbmP,
                           4L,
                           aptl,
                           ROP_SRCCOPY,
                           compression) ==
               GPI_ERROR) {
              GpiSetBitmap(hpsMemory,(HBITMAP)0);
              GpiAssociate(hpsMemory,NULLHANDLE);
              GpiDestroyPS(hpsMemory);
              GpiDeleteBitmap(hbm);
              DevCloseDC(hdcMemory);
              return FALSE;
            }

            memset(&bfh,0,sizeof(bfh));
            bfh.usType = BFT_COLORICON;
            bfh.cbSize = sizeof(BITMAPFILEHEADER2) - sizeof(BITMAPINFOHEADER2);
            bfh.xHotspot = 0;
            bfh.yHotspot = 0;
            bfh.offBits  = 0;
            bfh.bmp2.cbFix = sizeof(BITMAPINFOHEADER2);

            {
              PBITMAPINFO2 pbmi = NULL;
              PBYTE        pbBuffer = NULL;
              ULONG        cbBuffer,cbBitmapInfo;

              if(!GpiQueryBitmapInfoHeader(hbm,&bfh.bmp2)) {
                GpiSetBitmap(hpsMemory,(HBITMAP)0);
                GpiAssociate(hpsMemory,NULLHANDLE);
                GpiDestroyPS(hpsMemory);
                GpiDeleteBitmap(hbm);
                DevCloseDC(hdcMemory);
                return FALSE;
              }

              cbBuffer = (((((bfh.bmp2.cBitCount * bfh.bmp2.cx) + 31) / 32)
                             * 4) * bfh.bmp2.cy) * bfh.bmp2.cPlanes;
              if(bfh.bmp2.cBitCount == 24)
                cbBitmapInfo = sizeof(BITMAPINFOHEADER2);
              else
                cbBitmapInfo = sizeof(BITMAPINFOHEADER2) + (sizeof(RGB2) *
                                (1 << (bfh.bmp2.cPlanes * bfh.bmp2.cBitCount)));

              if(DosAllocMem((PPVOID)&pbBuffer,cbBuffer,
                             PAG_COMMIT | PAG_READ | PAG_WRITE))
              {
                GpiSetBitmap(hpsMemory,(HBITMAP)0);
                GpiAssociate(hpsMemory,NULLHANDLE);
                GpiDestroyPS(hpsMemory);
                GpiDeleteBitmap(hbm);
                DevCloseDC(hdcMemory);
                return FALSE;
              }
              if(DosAllocMem((PPVOID)&pbmi,cbBitmapInfo,
                             PAG_COMMIT | PAG_READ | PAG_WRITE))
              {
                DosFreeMem(pbBuffer);
                GpiSetBitmap(hpsMemory,(HBITMAP)0);
                GpiAssociate(hpsMemory,NULLHANDLE);
                GpiDestroyPS(hpsMemory);
                GpiDeleteBitmap(hbm);
                DevCloseDC(hdcMemory);
                return FALSE;
              }

              memset(pbmi,0,cbBitmapInfo);
              memcpy(pbmi,&bfh.bmp2,sizeof(BITMAPINFOHEADER2));
              pbmi->cbFix = sizeof(BITMAPINFOHEADER2);
              if(GpiQueryBitmapBits(hpsMemory,0,bfh.bmp2.cy,pbBuffer,pbmi) !=
                 GPI_ALTERROR) {

                ULONG            x,off1,off2;
                BITMAPFILEHEADER bfh1;
                PFEA2LIST        pfealist = NULL;
                EAOP2            eaop;
                ULONG            ealen,len;
                char            *eaval,*seek1,*seek2,*top;

                len = sizeof(bfh1) + 6 + sizeof(bfh1) +
                      (((cbBitmapInfo - sizeof(BITMAPINFOHEADER2)) / sizeof(RGB2)) * 3) +
                      ((((bfh.bmp2.cx + 31) / 32) * 4) * (bfh.bmp2.cy * 2)) +
                      cbBuffer;
                ealen = sizeof(FEA2LIST) + 128L + strlen(".ICON") + 1L +
                        len + 4L;
                if(!DosAllocMem((PPVOID)&pfealist,ealen,
                                OBJ_TILE | PAG_COMMIT | PAG_READ | PAG_WRITE))
                {
                  memset(pfealist,0,ealen);
                  pfealist->list[0].oNextEntryOffset = 0L;
                  pfealist->list[0].fEA = 0;
                  pfealist->list[0].cbName = strlen(".ICON");
                  memcpy(pfealist->list[0].szName,".ICON",pfealist->list[0].cbName + 1);
                  eaval = pfealist->list[0].szName + pfealist->list[0].cbName + 1;
                  *(USHORT *)eaval = (USHORT)EAT_ICON;
                  eaval += sizeof(USHORT);

                  *(USHORT *)eaval = (USHORT)len;
                  eaval += sizeof(USHORT);
                  top = eaval;
                  memset(&bfh1,0,sizeof(bfh1));
                  bfh1.usType = BFT_COLORICON;
                  bfh1.cbSize = sizeof(bfh1);
                  bfh1.xHotspot = WinQuerySysValue(HWND_DESKTOP,SV_CXICON) / 2;
                  bfh1.yHotspot = WinQuerySysValue(HWND_DESKTOP,SV_CYICON) / 2;
                  bfh1.offBits = 0;
                  bfh1.bmp.cbFix = sizeof(bfh1.bmp);
                  bfh1.bmp.cx = bfh.bmp2.cx;
                  bfh1.bmp.cy = bfh.bmp2.cy * 2;
                  bfh1.bmp.cPlanes = 1;
                  bfh1.bmp.cBitCount = 1;
                  memcpy(eaval,&bfh1,bfh1.cbSize);
                  eaval += bfh1.cbSize;
                  seek1 = eaval - sizeof(bfh1.bmp);
                  seek1 -= sizeof(bfh1.offBits);
                  memcpy(eaval,"\0\0\0\xff\xff\xff",6);
                  eaval += 6;
                  bfh1.bmp.cPlanes = bfh.bmp2.cPlanes;
                  bfh1.bmp.cBitCount = bfh.bmp2.cBitCount;
                  bfh1.bmp.cy = bfh.bmp2.cy;
                  memcpy(eaval,&bfh1,bfh1.cbSize);
                  eaval += bfh1.cbSize;
                  seek2 = eaval - sizeof(bfh1.bmp);
                  seek2 -= sizeof(bfh1.offBits);
                  for(x = 0;
                      x < (cbBitmapInfo - sizeof(BITMAPINFOHEADER2)) / sizeof(RGB2);
                      x++) {
                    *eaval = pbmi->argbColor[x].bBlue;
                    eaval++;
                    *eaval = pbmi->argbColor[x].bGreen;
                    eaval++;
                    *eaval = pbmi->argbColor[x].bRed;
                    eaval++;
                  }
                  off1 = eaval - top;
                  for(x = 0;
                      x < ((((bfh.bmp2.cx + 31) / 32) * 4) * (bfh.bmp2.cy * 2));
                      x++) {
                    *eaval = 0;
                    eaval++;
                  }
                  off2 = eaval - top;
                  memcpy(eaval,pbBuffer,cbBuffer);
                  eaval += cbBuffer;

                  *(ULONG *)seek1 = off1;
                  *(ULONG *)seek2 = off2;

                  pfealist->list[0].cbValue = (ULONG)(eaval -
                                              (pfealist->list[0].szName +
                                               pfealist->list[0].cbName + 1));
                  eaop.fpGEA2List = (PGEA2LIST)0;
                  eaop.fpFEA2List = pfealist;
                  eaop.oError = 0L;
                  pfealist->cbList = 13L + (ULONG)pfealist->list[0].cbName +
                                     (ULONG)pfealist->list[0].cbValue;

                  rc = DosSetPathInfo(pszFileName,FIL_QUERYEASIZE,(PVOID)&eaop,
                                      (ULONG)sizeof(EAOP2),DSPI_WRTTHRU);
                  DosFreeMem(pfealist);
                }
              }
              DosFreeMem(pbBuffer);
              DosFreeMem(pbmi);
            }
            GpiSetBitmap(hpsMemory,(HBITMAP)0);
            GpiDeleteBitmap(hbm);
            hbm = (HBITMAP)0;
          }
        }
        GpiAssociate(hpsMemory,NULLHANDLE);
        GpiDestroyPS(hpsMemory);
        hpsMemory = (HPS)0;
      }
      DevCloseDC(hdcMemory);
      hdcMemory = (HDC)0;
    }
  }
  return (rc == 0);
}


BOOL SaveBitmap (HAB hab,HBITMAP hbmIn,char *pszFileName,USHORT bitcount,
                 FOURCC saveas) {

  HDC               hdcMemory;
  HPS               hpsMemory;
  SIZEL             ImageSize;
  POINTL            aptl[4];
  BITMAPINFOHEADER2 bmp2;
  HBITMAP           hbm;
  ULONG             rc;
  BOOL              ret = FALSE;
  PBITMAPINFO2      pbmi = NULL;
  PBYTE             pbBuffer = NULL;
  ULONG             cbBuffer,cbBitmapInfo;
  MMIOINFO          mmioinfoTarget;
  HMMIO             hmmioTarget;
  ULONG             ulBytesRead;
  MMIMAGEHEADER     mmImgHdr;

  if(!pszFileName)
    return ret;

  rc = IsFile(pszFileName);
  if(rc == 1) {
    if(saymsg(MB_YESNOCANCEL | MB_ICONEXCLAMATION,
              hwndMain,
              "Confirm overwrite:",
              "File \"%s\" exists -- do you want to overwrite it?",
              pszFileName) != MBID_YES)
    return ret;
  }
  else if(rc == 0) {
    saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
           hwndMain,
           "Image save error",
           "\"%s\" exists and is a directory.",
           pszFileName);
    return ret;
  }

  hdcMemory = DevOpenDC(hab,OD_MEMORY,"*",0L,NULL,0);
  if(hdcMemory == DEV_ERROR)
    return ret;

  WinSetPointer(HWND_DESKTOP,hptrWait);

  memset(&bmp2,0,sizeof(bmp2));
  bmp2.cbFix = sizeof(bmp2);
  if(GpiQueryBitmapInfoHeader(hbmIn,&bmp2)) {
    ImageSize.cx = bmp2.cx;
    ImageSize.cy = bmp2.cy;
    if(bitcount &&
       bitcount < bmp2.cBitCount)
      bmp2.usRendering = BRH_ERRORDIFFUSION;
    if(bitcount)
      bmp2.cBitCount = bitcount;
    compression = (bmp2.cBitCount > 1) ? rcompression : lcompression;
    hpsMemory = GpiCreatePS(hab,
                            hdcMemory,
                            &ImageSize,
                            PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);
    if(hpsMemory) {
      hbm = GpiCreateBitmap(hpsMemory,
                            &bmp2,
                            0,
                            NULL,
                            NULL);
      if(hbm) {
        if(GpiSetBitmap(hpsMemory,hbm) != HBM_ERROR) {
          aptl[0].x = 0;              /* target lower left */
          aptl[0].y = 0;
          aptl[1].x = bmp2.cx - 1;    /* target upper right */
          aptl[1].y = bmp2.cy - 1;
          aptl[2].x = 0;              /* source lower left */
          aptl[2].y = 0;
          aptl[3].x = bmp2.cx;        /* source upper right */
          aptl[3].y = bmp2.cy;
          if(GpiWCBitBlt(hpsMemory,
                         hbmIn,
                         4L,
                         aptl,
                         ROP_SRCCOPY,
                         compression) !=  GPI_ERROR) {
            cbBuffer = (((((bmp2.cBitCount * bmp2.cx) + 31) / 32) *
                            4) * bmp2.cy) * bmp2.cPlanes;
            cbBitmapInfo = sizeof(BITMAPINFOHEADER2);
            if(bmp2.cBitCount < 24)
              cbBitmapInfo += (sizeof(RGB2) *
                               (1 << (bmp2.cPlanes * bmp2.cBitCount)));
            if(!DosAllocMem((PPVOID)&pbBuffer,cbBuffer,
                            PAG_COMMIT | PAG_READ | PAG_WRITE) &&
               !DosAllocMem((PPVOID)&pbmi,cbBitmapInfo,
                            PAG_COMMIT | PAG_READ | PAG_WRITE)) {
              memset(pbmi,0,cbBitmapInfo);
              memcpy(pbmi,&bmp2,sizeof(BITMAPINFOHEADER2));
              rc = GpiQueryBitmapBits(hpsMemory,0,bmp2.cy,pbBuffer,pbmi);
              if(rc != (ULONG)GPI_ALTERROR) {
                /*******************************/
                /* Set up/open the TARGET file */
                /*******************************/
                memset(&mmioinfoTarget, 0L, sizeof(MMIOINFO));
                mmioinfoTarget.fccIOProc = saveas;
                mmioinfoTarget.ulTranslate = MMIO_TRANSLATEHEADER |
                                             MMIO_TRANSLATEDATA;
                hmmioTarget = mmioOpen(pszFileName,
                                       &mmioinfoTarget,
                                       MMIO_CREATE | MMIO_WRITE |
                                       MMIO_DENYWRITE | MMIO_NOIDENTIFY |
                                       MMIO_ALLOCBUF);
                if(hmmioTarget) {
                  memset(&mmImgHdr,0,sizeof(MMIMAGEHEADER));
                  mmImgHdr.ulHeaderLength = sizeof(MMIMAGEHEADER);
                  mmImgHdr.ulContentType = MMIO_IMAGE_UNKNOWN;
                  mmImgHdr.ulMediaType = MMIO_MEDIATYPE_IMAGE;
                  mmImgHdr.mmXDIBHeader.XDIBHeaderPrefix.ulMemSize =
                    cbBuffer;
                  mmImgHdr.mmXDIBHeader.XDIBHeaderPrefix.ulPelFormat =
                    (bmp2.cBitCount < 24) ? mmioFOURCC('p','a','l','b') :
                      mmioFOURCC('r','g','b','b');
                  mmImgHdr.mmXDIBHeader.XDIBHeaderPrefix.usTransType = 0;
                  mmImgHdr.mmXDIBHeader.XDIBHeaderPrefix.ulTransVal = 0;
                  memcpy(&mmImgHdr.mmXDIBHeader.BMPInfoHeader2,
                         pbmi,cbBitmapInfo);
                  rc = (long)mmioSetHeader(hmmioTarget,
                                           &mmImgHdr,
                                           (long)sizeof(MMIMAGEHEADER),
                                           (PLONG)&ulBytesRead,
                                           0L,
                                           0L);
                  if(rc == MMIO_SUCCESS) {
                    ulBytesRead = (ULONG)mmioWrite(hmmioTarget,
                                                   pbBuffer,
                                                   cbBuffer);
                    if(ulBytesRead != MMIO_ERROR && ulBytesRead)
                      ret = TRUE;
                    else
                      saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                             hwndMain,
                             "Bitmap save error",
                             "Error returned on write to file%s.",
                             (ulBytesRead == 0) ?
                              " (MMPM/2 error, no image data written, "
                              "IMAGE crash possible)" : "");
                  }
                  else
                    saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                           hwndMain,
                           "Bitmap save error",
                           "Could not save header.");
                  rc = (USHORT)mmioClose(hmmioTarget,0L);
                  if(rc != MMIO_SUCCESS)
                    saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                           HWND_DESKTOP,
                           "Bitmap save error",
                           "Error returned when file closed (%lu)",
                           rc);
                }
                else
                  saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                         hwndMain,
                         "Bitmap save error",
                         "Cannot open \"%s\" for write.",
                         pszFileName);
              }
              else
                saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                       hwndMain,
                       "Bitmap save error",
                       "GpiQueryBitmapBits failed.");
            }
            else
              saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                     hwndMain,
                     "Bitmap save error",
                     "Out of memory.");
            if(pbBuffer)
              DosFreeMem(pbBuffer);
            if(pbmi)
              DosFreeMem(pbmi);
          }
          else
            saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                   hwndMain,
                   "Bitmap save error",
                   "GpiWCBitBlt failed.");
        }
        GpiDeleteBitmap(hbm);
      }
      GpiDestroyPS(hpsMemory);
    }
  }
  DevCloseDC(hdcMemory);
  WinSetPointer(HWND_DESKTOP,hptrArrow);
  return ret;
}


HBITMAP LoadBitmap (HAB    hab,
                    HDC    hdc,
                    HPS   *hps,
                    HWND   hwnd,
                    FOURCC fccIOProc,
                    PSZ    pszFileName) {

  HBITMAP       hbm = (HBITMAP)0;
  MMIOINFO      mmioinfo;
  HMMIO         hmmio;
  ULONG         ulImageHeaderLength = 0;
  MMIMAGEHEADER mmImgHdr;
  ULONG         ulBytesRead = 0;
  ULONG         dwNumRowBytes = 0;
  PBYTE         pRowBuffer = NULL;
  ULONG         dwRowCount = 0;
  SIZEL         ImageSize;
  ULONG         dwHeight = 0, dwWidth = 0,hund = 10;
  SHORT         wBitCount = 0;
  ULONG         dwPadBytes = 0;
  ULONG         dwRowBits = 0;
  ULONG         ulReturnCode = 0;
  HPS           hpsDraw = (HPS)0;
  POINTL        ptl;

  /* Clear out and initialize mminfo structure */

  memset(&mmioinfo,
         0,
         sizeof(MMIOINFO));
  mmioinfo.fccIOProc = fccIOProc;
  mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
  mmioinfo.cchBuffer = 65536;

  hmmio = mmioOpen((PSZ)pszFileName,
                   &mmioinfo,
                   MMIO_READ | MMIO_DENYWRITE | MMIO_NOIDENTIFY |
                   MMIO_ALLOCBUF);
  if(!hmmio)   /* If file could not be opened, return with error */
    return (HBITMAP)0;

  mmioQueryHeaderLength(hmmio,
                        (PLONG)&ulImageHeaderLength,
                        0L,
                        0L);

  if(ulImageHeaderLength != sizeof(MMIMAGEHEADER)) {
    /* We have a problem.....possibly incompatible versions */
    mmioClose(hmmio,0L);
    return (HBITMAP)0;
  }

  ulReturnCode = mmioGetHeader(hmmio,
                               &mmImgHdr,
                               (long)sizeof(MMIMAGEHEADER),
                               (PLONG)&ulBytesRead,
                               0L,
                               0L);

  if(ulReturnCode != MMIO_SUCCESS) {
    /* Header unavailable */
    mmioClose(hmmio,0L);
    return (HBITMAP)0;
  }

  /*
   *  Determine the number of bytes required, per row.
   *      PLANES MUST ALWAYS BE = 1
   */

  if(mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) {
    mmioClose(hmmio,0L);
    return (HBITMAP)0;
  }
  dwHeight = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cy;
  dwWidth = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cx;
  wBitCount = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount;
  dwRowBits = dwWidth * wBitCount;
  dwNumRowBytes = dwRowBits >> 3;
  rcompression = (wBitCount > 1) ? BBO_IGNORE : lcompression;

  /*
   *  Account for odd bits used in 1bpp or 4bpp images that are
   *  NOT on byte boundaries.
   */

  if(dwRowBits % 8)
    dwNumRowBytes++;

  /*
   *  Ensure the row length in bytes accounts for byte padding.
   *  All bitmap data rows must are aligned on long/4-BYTE boundaries.
   *  The data FROM an IOProc should always appear in this form.
   */

  dwPadBytes = (dwNumRowBytes % 4);
  if(dwPadBytes)
    dwNumRowBytes += 4 - dwPadBytes;

  /* Allocate space for ONE row of pels */

  if(DosAllocMem((PPVOID)&pRowBuffer,
                  (ULONG)dwNumRowBytes,
                  fALLOC)) {
    mmioClose(hmmio, 0L);
    return(HBITMAP)0;
  }

  /*****************************************************
   * Create a memory presentation space that includes  *
   * the memory device context obtained above.         *
   *****************************************************/

  ImageSize.cx = dwWidth;
  ImageSize.cy = dwHeight;

  *hps = GpiCreatePS(hab,
                     hdc,
                     &ImageSize,
                     PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);

  if(!*hps) {
    mmioClose(hmmio, 0L);
    return (HBITMAP)0;
  }

  /****************************************************
   * Create an uninitialized bitmap.  This is where we
   * will put all of the bits once we read them in.
   ****************************************************/

  hbm = GpiCreateBitmap(*hps,
                        &mmImgHdr.mmXDIBHeader.BMPInfoHeader2,
                        0L,
                        NULL,
                        NULL);

  if(!hbm) {
    mmioClose (hmmio, 0L);
    return (HBITMAP)0;
  }

  if(hwnd &&
     dwHeight > 100) {
    hpsDraw = WinGetPS(hwnd);
    if(hpsDraw) {
      GpiSetColor(hpsDraw,CLR_BLUE);
      hund = dwHeight / 100;
      hund *= 4;
      hund = max(hund,10);
    }
  }

  // ***************************************************
  // Select the bitmap into the memory device context.
  // ***************************************************

  GpiSetBitmap(*hps,hbm);

  //***************************************************************
  //  LOAD THE BITMAP DATA FROM THE FILE
  //      One line at a time, starting from the BOTTOM
  //*************************************************************** */

  for(dwRowCount = 0;dwRowCount < dwHeight;dwRowCount++) {
    ulBytesRead = (ULONG)mmioRead(hmmio,
                                  pRowBuffer,
                                  dwNumRowBytes);
    if(!ulBytesRead)
      break;

     /*
      *  Allow context switching while previewing.. Couldn't get
      *  it to work. Perhaps will get to it when time is available...
      */

     GpiSetBitmapBits(*hps,
                      (long)dwRowCount,
                      (long)1,
                      (PBYTE)pRowBuffer,
                      (PBITMAPINFO2)&mmImgHdr.mmXDIBHeader.BMPInfoHeader2);

    if(hpsDraw &&
       !(dwRowCount % hund)) {
      ptl.x = ptl.y = 2;
      GpiMove(hpsDraw,&ptl);
      ptl.x = (((dwRowCount * 100) / dwHeight)) + 2;
      ptl.y = 6;
      GpiBox(hpsDraw,DRO_OUTLINEFILL,&ptl,0,0);
      DosSleep(0L);
    }
  }

  if(hpsDraw)
    WinReleasePS(hpsDraw);
  mmioClose(hmmio,0L);
  DosFreeMem(pRowBuffer);

#ifdef USE_PALETTE
  if(hbm && colors == 256 && (wBitCount == 4 || wBitCount == 8)) {

/*
    int  x;
    BYTE b;

    for(x = 0;x < ((wBitCount == 4) ? 16 : 256);x++) {
      b = mmImgHdr.bmiColors[x].bBlue;
      mmImgHdr.bmiColors[x].bBlue = mmImgHdr.bmiColors[x].bGreen;
      mmImgHdr.bmiColors[x].bGreen = mmImgHdr.bmiColors[x].bRed;
      mmImgHdr.bmiColors[x].bRed = mmImgHdr.bmiColors[x].fcOptions;
      mmImgHdr.bmiColors[x].fcOptions = b;
    }
*/

    hpal = GpiCreatePalette(hab,
                            0L,
                            LCOLF_CONSECRGB,
                            ((wBitCount == 4) ? 16 : 256),
                            (PULONG)mmImgHdr.bmiColors);
    if(hpal)
      GpiSelectPalette(hpsWnd,hpal);
  }
#endif

  return(hbm);
}


/*
 *  GetBitMap
 *
 *  Get a bitmap for the specified file
 */

BOOL GetBitMap (IMAGEDATA *wnd) {

  /*
   *  Load Bitmap, which will then be drawn during WM_PAINT processing
   */

  HPS hpsMemory;
  HDC hdcMemory;

  if(!wnd ||
     !*(wnd->filename))
    return FALSE;

#ifdef USE_PALETTE
  if(hpal) {
    GpiSelectPalette(hpsWnd,(HPAL)0);
    GpiDeletePalette(hpal);
    hpal = (HPAL)0;
  }
#endif

  hdcMemory = DevOpenDC(wnd->hab,
                        OD_MEMORY,
                        "*",
                        0L,
                        NULL,
                        0);
  if(!hdcMemory)
    return FALSE;

  /*
   * Delete previously loaded bitmap if present
   */

  if(wnd->hbm &&
     wnd->hbm != HBM_ERROR)
    GpiDeleteBitmap(wnd->hbm);

  wnd->hbm = LoadBitmap(wnd->hab,
                        hdcMemory,
                        &hpsMemory,
                        wnd->hwndClient,
                        wnd->mmFormatInfo.fccIOProc,
                        wnd->filename);
  wnd->saveas = wnd->mmFormatInfo.fccIOProc;

  /* free hps and hdc -- no longer needed */
  if(hpsMemory) {
    GpiSetBitmap(hpsMemory,0);
    GpiAssociate(hpsMemory,NULLHANDLE);
    GpiDestroyPS(hpsMemory);
  }
  if(hdcMemory)
    DevCloseDC(hdcMemory);

  if(!wnd->hbm ||
     wnd->hbm == HBM_ERROR) /* Error loading bitmap */
    return FALSE;

  return TRUE;
}


/*
 *  DrawBitmap
 *
 *  Draw the previously loaded bitmap in the rectangle occupied by the
 *  frame ID_IMAGE.
 */

BOOL DrawBitmap (HPS hps,IMAGEDATA *wnd) {

  SWP               swp;
  POINTL            aptl[4];
  BITMAPINFOHEADER2 bmp2;

  memset(&bmp2,0,sizeof(bmp2));
  bmp2.cbFix = sizeof(bmp2);
  if(!GpiQueryBitmapInfoHeader(wnd->hbm,
                               &bmp2))
    return FALSE;

  /*
   *  Get position of image frame
   */

  WinQueryWindowPos(wnd->hwndClient,&swp);

  aptl[0].x = 0;              // target lower left
  aptl[0].y = 0;

  aptl[1].x = swp.cx - 1;     // target upper right
  aptl[1].y = swp.cy - 1;

  aptl[2].x = 0;              // source lower left
  aptl[2].y = 0;

  aptl[3].x = bmp2.cx;        // source upper right
  aptl[3].y = bmp2.cy;

  /*
   *  Call GpiWcBitBlt and supply 4 aptl structures.  This tells
   *  it to stretch or compress the bitmap depending on what is
   *  in the aptl structures.  See above lines for their current
   *  settings.
   */

  compression = (bmp2.cBitCount > 1) ? rcompression : lcompression;
  wnd->isline = (bmp2.cBitCount == 1);
  GpiWCBitBlt(hps,
              wnd->hbm,
              4L,
              aptl,
              ROP_SRCCOPY,
              compression);
  return TRUE;
}


void ReverseRectl (RECTL *rcl,BOOL xdir) {

  long temp;

  if(xdir) {
    temp = rcl->xLeft;
    rcl->xLeft = rcl->xRight;
    rcl->xRight = temp;
  }
  else {  /* ydir */
    temp = rcl->yBottom;
    rcl->yBottom = rcl->yTop;
    rcl->yTop = temp;
  }
}


void AdjustRectl (RECTL *rcl) {

  if(rcl->xLeft > rcl->xRight)
    ReverseRectl(rcl,TRUE);
  if(rcl->yBottom > rcl->yTop)
    ReverseRectl(rcl,FALSE);
}


void SpecialRectl (RECTL *rcl,BOOL swapx,BOOL swapy) {

  /* start from known condition */
  AdjustRectl(rcl);
  if(swapx)
    ReverseRectl(rcl,TRUE);
  if(swapy)
    ReverseRectl(rcl,FALSE);
}


BOOL FixRectl (HBITMAP hbm,HWND hwnd,RECTL *rcl) {

  BITMAPINFOHEADER2 bmp2;
  double            mx,my;
  ULONG             adjust = 0;
  RECTL             wrcl;

  memset(&bmp2,0,sizeof(bmp2));
  bmp2.cbFix = sizeof(bmp2);
  if(GpiQueryBitmapInfoHeader(hbm,&bmp2)) {
    WinQueryWindowRect(hwnd,&wrcl);
    if((rcl->xLeft == rcl->xRight ||
        rcl->yTop == rcl->yBottom) ||
       (rcl->xLeft == 0 &&
        rcl->xRight >= wrcl.xRight - 1 &&
        rcl->yBottom == 0 &&
        rcl->yTop >= wrcl.yTop - 1)) {
      rcl->xLeft = rcl->yBottom = 0;
      rcl->xRight = bmp2.cx;
      rcl->yTop = bmp2.cy;
      adjust = 1;
    }
    if(wrcl.xRight && wrcl.yTop) {
      mx = ((double)bmp2.cx / (double)wrcl.xRight);
      my = ((double)bmp2.cy / (double)wrcl.yTop);
    }
    else
      mx = my = 0;
    AdjustRectl(rcl);
    if(adjust) {
      rcl->xRight--;
      rcl->yTop--;
    }
    rcl->xLeft = ceil((double)rcl->xLeft * mx);
    rcl->xRight = ceil((double)rcl->xRight * mx);
    rcl->yBottom = ceil((double)rcl->yBottom * my);
    rcl->yTop = ceil((double)rcl->yTop * my);
    if(rcl->yTop > bmp2.cy - 1)
      rcl->yTop = bmp2.cy - 1;
    if(rcl->xRight > bmp2.cx - 1)
      rcl->xRight = bmp2.cx - 1;
    if(rcl->xLeft < 0)
      rcl->xLeft = 0;
    if(rcl->yBottom < 0)
      rcl->yBottom = 0;
    if(rcl->xLeft != rcl->xRight &&
       rcl->yTop != rcl->yBottom)
      return TRUE;
    else
      rcl->xLeft = rcl->xRight = rcl->yBottom = rcl->yTop = 0;
  }
  return FALSE;
}


HBITMAP PasteBitmap(HAB hab,HBITMAP hbmIn1,HBITMAP hbmIn2,RECTL *rcl) {

  /*
   *  Paste one bitmap inside another; return the HBITMAP of the pasted copy.
   */

  HPS               hpsMemory;
  HDC               hdcMemory;
  HBITMAP           hbm = (HBITMAP)0;
  SIZEL             ImageSize;
  POINTL            aptl[4];
  BITMAPINFOHEADER2 bmp2;

  if(!hbmIn1 || !hbmIn2)
    return hbm;
  if(rcl->xLeft == rcl->xRight ||
     rcl->yTop == rcl->yBottom)
    return hbm;

  hdcMemory = DevOpenDC(hab,
                        OD_MEMORY,
                        "*",
                        0L,
                        NULL,
                        0);
  if(hdcMemory) {
    memset(&bmp2,0,sizeof(bmp2));
    bmp2.cbFix = sizeof(bmp2);
    if(GpiQueryBitmapInfoHeader(hbmIn1,&bmp2)) {
      ImageSize.cx = bmp2.cx;
      ImageSize.cy = bmp2.cy;
      hpsMemory = GpiCreatePS(hab,
                              hdcMemory,
                              &ImageSize,
                              PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);
      if(hpsMemory) {
        hbm = GpiCreateBitmap(hpsMemory,
                              &bmp2,
                              0,
                              NULL,
                              NULL);
        if(hbm) {
          if(GpiSetBitmap(hpsMemory,hbm) != HBM_ERROR) {
            aptl[0].x = 0;              /* target lower left */
            aptl[0].y = 0;
            aptl[1].x = bmp2.cx - 1;    /* target upper right */
            aptl[1].y = bmp2.cy - 1;
            aptl[2].x = 0;              /* source lower left */
            aptl[2].y = 0;
            aptl[3].x = bmp2.cx;        /* source upper right */
            aptl[3].y = bmp2.cy;
            compression = (bmp2.cBitCount > 1) ? rcompression : lcompression;
            if(GpiWCBitBlt(hpsMemory,
                           hbmIn1,
                           4L,
                           aptl,
                           ROP_SRCCOPY,
                           compression) != GPI_ERROR) {
              if(GpiQueryBitmapInfoHeader(hbmIn2,&bmp2)) {
                aptl[0].x = rcl->xLeft;             /* target lower left */
                aptl[0].y = rcl->yBottom;
                aptl[1].x = rcl->xRight;            /* target upper right */
                aptl[1].y = rcl->yTop;
                aptl[2].x = 0;                      /* source lower left */
                aptl[2].y = 0;
                aptl[3].x = bmp2.cx;                /* source upper right */
                aptl[3].y = bmp2.cy;
                if(GpiWCBitBlt(hpsMemory,
                               hbmIn2,
                               4L,
                               aptl,
                               ROP_SRCCOPY,
                               compression) == GPI_ERROR) {
                  GpiDeleteBitmap(hbm);
                  hbm = (HBITMAP)0;
                }
              }
              else {
                GpiDeleteBitmap(hbm);
                hbm = (HBITMAP)0;
              }
            }
            else {
              GpiDeleteBitmap(hbm);
              hbm = (HBITMAP)0;
            }
          }
          else {
            GpiDeleteBitmap(hbm);
            hbm = (HBITMAP)0;
          }
          GpiSetBitmap(hpsMemory,0);
        }
        GpiAssociate(hpsMemory,NULLHANDLE);
        GpiDestroyPS(hpsMemory);
      }
    }
    DevCloseDC(hdcMemory);
  }
  return hbm;
}


HBITMAP CopyBitmap (HAB hab, HBITMAP hbmIn, RECTL *rcl) {

  /*
   *  Copy a bitmap; return the HBITMAP of the copy.
   */

  HPS               hpsMemory;
  HDC               hdcMemory;
  HBITMAP           hbm = (HBITMAP)0;
  SIZEL             ImageSize;
  POINTL            aptl[4];
  BITMAPINFOHEADER2 bmp2;

  if(!hbmIn)
    return hbm;
  if(rcl->xLeft == rcl->xRight ||
     rcl->yTop == rcl->yBottom)
    return hbm;

  hdcMemory = DevOpenDC(hab,
                        OD_MEMORY,
                        "*",
                        0L,
                        NULL,
                        0);
  if(hdcMemory) {
    memset(&bmp2,0,sizeof(bmp2));
    bmp2.cbFix = sizeof(bmp2);
    if(GpiQueryBitmapInfoHeader(hbmIn,&bmp2)) {
      bmp2.cx = abs(rcl->xRight - rcl->xLeft) + 1;
      bmp2.cy = abs(rcl->yTop - rcl->yBottom) + 1;
      bmp2.cbImage = 0;
      ImageSize.cx = bmp2.cx;
      ImageSize.cy = bmp2.cy;
      hpsMemory = GpiCreatePS(hab,
                              hdcMemory,
                              &ImageSize,
                              PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);
      if(hpsMemory) {
        hbm = GpiCreateBitmap(hpsMemory,
                              &bmp2,
                              0,
                              NULL,
                              NULL);
        if(hbm) {
          if(GpiSetBitmap(hpsMemory,hbm) != HBM_ERROR) {
            if(rcl->xRight > rcl->xLeft) {          /* target */
              aptl[0].x = 0;
              aptl[1].x = abs(rcl->xRight - rcl->xLeft);
            }
            else {
              aptl[0].x = abs(rcl->xRight - rcl->xLeft);
              aptl[1].x = 0;
            }
            if(rcl->yTop > rcl->yBottom) {
              aptl[0].y = 0;
              aptl[1].y = abs(rcl->yTop - rcl->yBottom);
            }
            else {
              aptl[0].y = abs(rcl->yTop - rcl->yBottom);
              aptl[1].y = 0;
            }
            if(rcl->xRight > rcl->xLeft) {
              aptl[2].x = rcl->xLeft;               /* source */
              aptl[3].x = rcl->xRight + 1;
            }
            else {
              aptl[2].x = rcl->xRight;
              aptl[3].x = rcl->xLeft + 1;
            }
            if(rcl->yTop > rcl->yBottom) {
              aptl[2].y = rcl->yBottom;
              aptl[3].y = rcl->yTop + 1;
            }
            else {
              aptl[3].y = rcl->yTop;
              aptl[2].y = rcl->yBottom + 1;
            }
            compression = (bmp2.cBitCount > 1) ? rcompression : lcompression;
            if(GpiWCBitBlt(hpsMemory,
                           hbmIn,
                           4L,
                           aptl,
                           ROP_SRCCOPY,
                           compression) == GPI_ERROR) {
              GpiDeleteBitmap(hbm);
              hbm = (HBITMAP)0;
            }
            GpiSetBitmap(hpsMemory,0);
          }
          else {
            GpiDeleteBitmap(hbm);
            hbm = (HBITMAP)0;
          }
        }
        GpiAssociate(hpsMemory,NULLHANDLE);
        GpiDestroyPS(hpsMemory);
      }
    }
    DevCloseDC(hdcMemory);
  }
  return hbm;
}


BOOL SetBestSize (HWND hwndFrame,HBITMAP hbm) {

  long              cxWindow,cyWindow,cxScreen,cyScreen;
  RECTL             rcl;
  BITMAPINFOHEADER2 bmp2;
  double            percent;
  SWP               swpO,swpN;

  memset(&bmp2,0,sizeof(bmp2));
  bmp2.cbFix = sizeof(bmp2);
  if(GpiQueryBitmapInfoHeader(hbm,&bmp2)) {
    rcl.xLeft   = 0;
    rcl.xRight  = max(bmp2.cx,4);
    rcl.yBottom = 0;
    rcl.yTop    = max(bmp2.cy,4);
    percent = (double)bmp2.cy / (double)bmp2.cx;
    WinCalcFrameRect(hwndFrame,&rcl,FALSE);
    cxWindow = (rcl.xRight - rcl.xLeft) -
                (WinQuerySysValue(HWND_DESKTOP,SV_CXSIZEBORDER) * 2);
    cyWindow = (long)((double)cxWindow * percent) +
                (WinQuerySysValue(HWND_DESKTOP,SV_CYSIZEBORDER) * 2) +
                (WinQuerySysValue(HWND_DESKTOP,SV_CYTITLEBAR));
    cxScreen = WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN);
    cyScreen = WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN);
    if(cxWindow > cxScreen) {
      percent = (double)cxScreen / (double)cxWindow;
      cxWindow = cxScreen;
      cyWindow = (SHORT)((double)cyWindow * percent);
    }
    if(cyWindow > cyScreen) {
      percent = (double)cyScreen / (double)cyWindow;
      cyWindow = cyScreen;
      cxWindow = (SHORT)((double)cxWindow * percent);
    }
    WinQueryWindowPos(hwndFrame,&swpO);
    WinSetWindowPos(hwndFrame,HWND_TOP,
                    (cxScreen - cxWindow) / 2,
                    (cyScreen - cyWindow) / 2,
                    cxWindow,
                    cyWindow,
                    SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ACTIVATE);
    WinQueryWindowPos(hwndFrame,&swpN);
    if(swpO.x == swpN.x &&
       swpO.y == swpN.y &&
       swpO.cx == swpN.cx &&
       swpO.cy == swpN.cy) {
      WinInvalidateRect(WinWindowFromID(hwndFrame,FID_CLIENT),NULL,TRUE);
      return TRUE;
    }
  }
  return FALSE;
}


void PasteProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  IMAGEDATA *wnd;

  wnd = WinQueryWindowPtr(hwnd,0);
  if(wnd && !wnd->busy && wnd->hbm) {

    HBITMAP hbm,hbmTemp;
    RECTL   rcl;

    if(WinOpenClipbrd(wnd->hab)) {
      hbm = (HBITMAP)WinQueryClipbrdData(wnd->hab,CF_BITMAP);
      if(hbm) {

        BOOL resize = TRUE;

        hbmTemp = wnd->hbm;
        rcl = wnd->marked;
        if(wnd->marked.xLeft == wnd->marked.xRight ||
           wnd->marked.yTop == wnd->marked.yBottom)
          FixRectl(wnd->hbm,hwnd,&rcl);
        else {
          if(wnd->marked.xLeft < wnd->marked.xRight)
            wnd->marked.xRight++;
          else
            wnd->marked.xLeft++;
          if(wnd->marked.yBottom < wnd->marked.yTop)
            wnd->marked.yTop++;
          else
            wnd->marked.yBottom++;
        }
        if(SHORT1FROMMP(mp1) != IDM_WEIRDPASTE &&
           ((!(WinGetKeyState(HWND_DESKTOP,VK_SHIFT) & 0x8000) &&
           ((!(WinGetKeyState(HWND_DESKTOP,VK_CTRL) & 0x8000) &&
            (wnd->marked.xLeft == wnd->marked.xRight ||
             wnd->marked.yTop == wnd->marked.yBottom)) ||
            !FixRectl(wnd->hbm,hwnd,&rcl)))) &&
            SHORT1FROMMP(mp1) != IDM_STRETCHPASTE) {

          BITMAPINFOHEADER2 bmp2;

          memset(&bmp2,0,sizeof(bmp2));
          bmp2.cbFix = sizeof(bmp2);
          if(GpiQueryBitmapInfoHeader(hbm,&bmp2)) {
            rcl.xLeft = rcl.yBottom = 0;
            rcl.xRight = bmp2.cx - 1;
            rcl.yTop = bmp2.cy - 1;
            wnd->hbm = CopyBitmap(wnd->hab,hbm,&rcl);
            resize = FALSE;
          }
        }
        else
          wnd->hbm = PasteBitmap(wnd->hab,wnd->hbm,hbm,&rcl);
        WinCloseClipbrd(wnd->hab);
        if(wnd->hbm) {
          GpiDeleteBitmap(hbmTemp);
          if(resize) {
            SetBestSize(WinQueryWindow(hwnd,QW_PARENT),wnd->hbm);
            WinPostMsg(hwnd,
                       WM_COMMAND,
                       MPFROM2SHORT(IDM_CANCEL,0),
                       MPVOID);
          }
          WinInvalidateRect(hwnd,NULL,TRUE);
        }
        else
          wnd->hbm = hbmTemp;
      }
    }
  }
}


void LoadImageThread (void *args) {

  IMAGEDATA *org = (IMAGEDATA *)args,wnd;
  HAB        hab2;
  HMQ        hmq2;

  if(org) {
    wnd = *org;
    hab2 = WinInitialize(0);
    if(hab2) {
      hmq2 = WinCreateMsgQueue(hab2,0);
      if(hmq2) {
        WinCancelShutdown(hmq2,
                          TRUE);
        if(WinIsWindow(hab2,
                       wnd.hwndClient)) {
          WinSendMsg(wnd.hwndClient,
                     UM_NOTIFY,
                     MPVOID,
                     MPVOID);
          WinInvalidateRect(wnd.hwndClient,
                            NULL,
                            TRUE);
          priority_normal();
          if(!GetBitMap(&wnd))
            saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                   hwndMain,
                   "Error:",
                   "Couldn't load image file \"%s\".",
                   wnd.filename);
          if(WinIsWindow(hab2,
                         wnd.hwndClient)) {
            if(wnd.hbm) {
              WinSendMsg(wnd.hwndClient,
                         UM_CONTAINER_FILLED,
                         MPFROMP(&wnd),
                         MPVOID);
              SetBestSize(wnd.hwndFrame,
                          wnd.hbm);
            }
            else
              WinSendMsg(wnd.hwndClient,
                         UM_CONTAINER_FILLED,
                         MPVOID,
                         MPVOID);
          }
        }
        WinDestroyMsgQueue(hmq2);
      }
      WinTerminate(hab2);
    }
  }
}


void ConvertImageThread (void *args) {

  CVTIMAGEDATA *cvt = (CVTIMAGEDATA *)args;
  MMIOINFO      mmioinfoSource;
  MMIOINFO      mmioinfoTarget;
  HMMIO         hmmioSource;
  HMMIO         hmmioTarget;
  ULONG         ulImageHeaderLength = 0;
  MMIMAGEHEADER mmImgHdr;
  ULONG         ulBytesRead;
  ULONG         dwNumRowBytes;
  PBYTE         pRowBuffer = NULL;
  ULONG         dwRowCount;
  ULONG         dwWidth;
  ULONG         dwHeight;
  ULONG         dwPadBytes;
  ULONG         dwRowBits;
  long          rc;
  long          rcSrcQueryCount = 0;
  long          rcTrgQueryCount = 0;
  long          rcTrgSetImage   = 0;
  ULONG         iIndex,iCount,iCount2;
  BOOL          okay = FALSE;
  HAB           hab2;
  HMQ           hmq2;

  if(cvt) {
    hab2 = WinInitialize(0);
    if(hab2) {
      hmq2 = WinCreateMsgQueue(hab2,0);
      if(hmq2) {
        WinCancelShutdown(hmq2,TRUE);
        priority_normal();
        rc = IsFile(cvt->filenames);
        if(rc == 1) {
          if(saymsg(MB_YESNOCANCEL | MB_ICONEXCLAMATION,
                    hwndMain,
                    "Confirm overwrite:",
                    "File \"%s\" exists -- do you want to overwrite it?",
                    cvt->filenames) != MBID_YES)
          goto Abort;
        }
        else if(rc == 0) {
          saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                 hwndMain,
                 "Image conversion error",
                 "\"%s\" exists and is a directory.",cvt->filenames);
          goto Abort;
        }
        /*******************************/
        /* Set up/open the SOURCE file */
        /*******************************/
        memset(&mmioinfoSource, 0L, sizeof(MMIOINFO));
        mmioinfoSource.fccIOProc = cvt->loadas;
        mmioinfoSource.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
        memset(&mmioinfoTarget, 0L, sizeof(MMIOINFO));
        mmioinfoTarget.fccIOProc = cvt->saveas;
        mmioinfoTarget.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;

        hmmioSource = mmioOpen(cvt->filenamel,
                               &mmioinfoSource,
                               MMIO_READ | MMIO_DENYWRITE |
                               MMIO_NOIDENTIFY | MMIO_ALLOCBUF);
        if(!hmmioSource) {
          saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                 hwndMain,
                 "Image conversion error",
                 "Cannot open \"%s\" for read.",
                 cvt->filenamel);
          goto Abort;
        }

        /*******************************/
        /* Set up/open the TARGET file */
        /*******************************/

        hmmioTarget = mmioOpen(cvt->filenames,
                               &mmioinfoTarget,
                               MMIO_CREATE | MMIO_WRITE |
                               MMIO_DENYWRITE | MMIO_NOIDENTIFY |
                               MMIO_ALLOCBUF);
        if(!hmmioTarget) {
          mmioClose(hmmioSource, 0L);
          saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                 hwndMain,
                 "Image conversion error",
                 "Cannot open \"%s\" for write.",
                 cvt->filenames);
          goto Abort;
        }

        /* find out if source has multiple image support */
        rcSrcQueryCount = mmioSendMessage(hmmioSource,
                                          MMIOM_QUERYIMAGECOUNT,
                                          (long)&iCount,
                                          (long)0);
        if(rcSrcQueryCount)
          iCount = 1;

        /* find out if the target has multiple image support */
        rcTrgQueryCount = mmioSendMessage(hmmioTarget,
                                          MMIOM_QUERYIMAGECOUNT,
                                          (long)&iCount2,
                                          (long)0);

        {
          char message[(CCHMAXPATH * 2) + 80];

          sprintf(message,"Saving image \"%s\" as \"%s\"",
                  cvt->filenamel,cvt->filenames);
        }

        /* loop through known images */
        for(iIndex = 0;iIndex < iCount;iIndex++) {
          if(!rcSrcQueryCount && !rcTrgQueryCount) {  /* Both support images */
            /* Determine if the target can write arrays, and if not */
            /* then write the the default image from the source     */
            if(rcTrgSetImage && iIndex > 0)
              break;  /* Target Can't Write array */
            /* Now, determine if the target can write arrays */
            rcTrgSetImage = mmioSendMessage(hmmioTarget,
                                            MMIOM_SETIMAGE,
                                            (long)iIndex,
                                            (long)0);
            if(!rcTrgSetImage)
              mmioSendMessage(hmmioSource,
                              MMIOM_SETIMAGE,
                              (long)iIndex,
                              (long)0);
          }
          else if(!rcSrcQueryCount) {      /* Source does but target doesn't */
            /* Use the default image from source to copy to target */
            /* so do set the index of the first, let it default    */
            /* get the base photo cd image (2 of 5)                */
            if(iIndex > 0)
              break;
          }
//          else if(!rcTrgQueryCount) {      /* Target does but source doesn't */
//            /* Use the only image to do a default write to target */
//          }
//          else {
//            /* neither do: just write one image from here to there */
//          }

          /****************************/
          /* Obtain the SOURCE HEADER */
          /****************************/
          mmioQueryHeaderLength(hmmioSource,
                                (PLONG)&ulImageHeaderLength,
                                0L,
                                0L);
          if(ulImageHeaderLength != sizeof(MMIMAGEHEADER)) {
            /* We have a problem.....possibly incompatible versions */
            mmioClose(hmmioSource, 0L);
            mmioClose(hmmioTarget, 0L);
            goto Abort;
          }

          rc = (long)mmioGetHeader(hmmioSource, &mmImgHdr,
                                   (long)sizeof(MMIMAGEHEADER),
                                   (PLONG)&ulBytesRead,
                                   0L,
                                   0L);

          if(rc != MMIO_SUCCESS) {
            /* Header unavailable */
            mmioClose(hmmioSource, 0L);
            mmioClose(hmmioTarget, 0L);
            goto Abort;
          }

          /*************************/
          /* Set the TARGET HEADER */
          /*************************/
          mmioQueryHeaderLength(hmmioTarget,
                                (PLONG)&ulImageHeaderLength,
                                0L,
                                0L);
          if(ulImageHeaderLength != sizeof(MMIMAGEHEADER)) {
            /* We have a problem.....possibly incompatible versions */
            mmioClose(hmmioSource, 0L);
            mmioClose(hmmioTarget, 0L);
            goto Abort;
          }

          /* Use the SAME data as came from the SOURCE FILE.  It must be
             compatible with the OS/2 bitmaps, etc. */

          rc = (long)mmioSetHeader(hmmioTarget,
                                   &mmImgHdr,
                                   (long)sizeof(MMIMAGEHEADER),
                                   (PLONG)&ulBytesRead,
                                   0L,0L);

          if(rc != MMIO_SUCCESS) {
            /* Header unavailable */
            mmioClose(hmmioSource, 0L);
            mmioClose(hmmioTarget, 0L);
            goto Abort;
          }

          /* Determine the number of bytes required, per row */
          /*      PLANES MUST ALWAYS BE = 1 */
          dwHeight = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cy;
          dwWidth  = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cx;
          dwRowBits = dwWidth * mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount;
          dwNumRowBytes = dwRowBits >> 3;

          /* Account for odd bits used in 1bpp or 4bpp images that are NOT on byte boundaries. */
          if(dwRowBits % 8)
            dwNumRowBytes++;

          /* Ensure the row length in bytes accounts for byte padding.  All bitmap data rows
             must are aligned on long/4-BYTE boundaries.   The data FROM an IOProc
             should always appear in this form. */
          dwPadBytes = (dwNumRowBytes % 4);
          if(dwPadBytes)
            dwNumRowBytes += 4 - dwPadBytes;

          /* Allocate space for one row */
          if(DosAllocMem((PVOID)&pRowBuffer,(ULONG)dwNumRowBytes,fALLOC)) {
            mmioClose(hmmioSource, 0L);
            mmioClose(hmmioTarget, 0L);
            goto Abort;
          }

          for(dwRowCount = 0;dwRowCount < dwHeight;dwRowCount++) {
            ulBytesRead = (ULONG)mmioRead(hmmioSource,
                                          pRowBuffer,
                                          dwNumRowBytes);
            if(ulBytesRead)
              mmioWrite(hmmioTarget,pRowBuffer,(ULONG)ulBytesRead);
            else
              break;
          }
        }
        okay = TRUE;
        mmioClose(hmmioSource, 0L);
        mmioClose(hmmioTarget, 0L);

        DosFreeMem(pRowBuffer);
Abort:
        if(!okay)
          saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                 hwndMain,
                 "FM/2",
                 "Image save failed.");
        WinDestroyMsgQueue(hmq2);
      }
      WinTerminate(hab2);
    }
    free(cvt);
  }
}


MRESULT EXPENTRY ConvertImageDlgProc (HWND hwnd,ULONG msg,
                                     MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      WinPostMsg(hwnd,
                 UM_SETUP,
                 MPVOID,
                 MPVOID);
      break;

    case UM_SETUP:
      {
        MMFORMATINFO mmf;
        long         nf = 0;
        FILEDLG     *fdlg;
        IMAGEDATA   *wnd;
        BOOL         atleastone = FALSE;

        fdlg = WinQueryWindowPtr(hwnd,0);
        wnd = (IMAGEDATA *)fdlg->ulUser;
        if(!wnd->convert) {

          BITMAPINFOHEADER2 bmp2;
          char              s[13];
          USHORT            bitarray[] = {1,4,8,24,0},x;

          for(x = 0;bitarray[x];x++) {
            sprintf(s,"%u",bitarray[x]);
            WinSendDlgItemMsg(hwnd,
                              IMGS_BITCOUNT,
                              LM_INSERTITEM,
                              MPFROM2SHORT(LIT_END,0),
                              MPFROMP(s));
          }
          WinCheckButton(hwnd,
                         IMGS_ICONIFY,
                         wnd->iconify);
          wnd->bitcount = 8;
          memset(&bmp2,0,sizeof(bmp2));
          bmp2.cbFix = sizeof(bmp2);
          if(GpiQueryBitmapInfoHeader(wnd->hbm,
                                      &bmp2))
            wnd->bitcount = bmp2.cBitCount;
          sprintf(s,
                  "%u",
                  wnd->bitcount);
          WinSetDlgItemText(hwnd,
                            IMGS_BITCOUNT,
                            s);
          WinSetWindowText(hwnd,"FM/2:  Save image as...");
        }
        else {
          WinShowWindow(WinWindowFromID(hwnd,IMGS_BITCOUNT),FALSE);
          WinShowWindow(WinWindowFromID(hwnd,IMGS_BITCOUNTHDR),FALSE);
          WinShowWindow(WinWindowFromID(hwnd,IMGS_ICONIFY),FALSE);
          WinSetWindowText(hwnd,"FM/2:  Convert original image to...");
        }
        memset(&mmf,0,sizeof(MMFORMATINFO));
        mmf.ulMediaType = MMIO_MEDIATYPE_IMAGE;
        mmf.ulStructLen = sizeof(MMFORMATINFO);
        if(!mmioQueryFormatCount(&mmf,
                                 &nf,
                                 0,
                                 0) &&
           nf) {

          MMFORMATINFO *pmmf;
          long          pnf = 0;

          pmmf = malloc(sizeof(MMFORMATINFO) * nf);
          if(pmmf) {
            memset(pmmf,0,sizeof(MMFORMATINFO) * nf);
            memset(&mmf,0,sizeof(MMFORMATINFO));
            mmf.ulMediaType = MMIO_MEDIATYPE_IMAGE;
            mmf.ulStructLen = sizeof(MMFORMATINFO);
            if(!mmioGetFormats(&mmf,
                               nf,
                               pmmf,
                               &pnf,
                               0,
                               0) &&
               pnf) {

              char         *iname;
              register long x;
              SHORT         sItem;
              long          nb;

              for(x = 0;x < pnf;x++) {
                if((pmmf[x].ulFlags & MMIO_CANWRITETRANSLATED) != 0) {
                  iname = malloc(pmmf[x].lNameLength + 1);
                  if(iname) {
                    nb = 0;
                    if(!mmioGetFormatName(pmmf + x,
                                          iname,
                                          &nb,
                                          0,
                                          0) &&
                       nb) {
                      sItem = (SHORT)WinSendDlgItemMsg(hwnd,
                                                       IMGS_LISTBOX,
                                                       LM_INSERTITEM,
                                                       MPFROM2SHORT(LIT_SORTASCENDING,
                                                                    0),
                                                       MPFROMP(iname));
                      if(sItem >= 0) {
                        atleastone = TRUE;
                        WinSendDlgItemMsg(hwnd,
                                          IMGS_LISTBOX,
                                          LM_SETITEMHANDLE,
                                          MPFROMSHORT(sItem),
                                          MPFROMLONG((ULONG)pmmf[x].fccIOProc));
                        if(wnd->saveas == pmmf[x].fccIOProc ||
                           (!wnd->saveas &&
                            strstr(iname,"OS/2 2.0")))
                          WinSendDlgItemMsg(hwnd,
                                            IMGS_LISTBOX,
                                            LM_SELECTITEM,
                                            MPFROMSHORT(sItem),
                                            MPFROMSHORT(TRUE));
                      }
                    }
                    free(iname);
                  }
                }
              }
            }
            else
              WinDismissDlg(hwnd,0);
            free(pmmf);
          }
          else
            WinDismissDlg(hwnd,0);
        }
        else
          WinDismissDlg(hwnd,0);
        if(!atleastone)
          WinDismissDlg(hwnd,0);
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case IMGS_LISTBOX:
          switch(SHORT2FROMMP(mp1)) {
            case CBN_LBSELECT:
            case CBN_ENTER:
              {
                MMFORMATINFO find,found;
                long         nf = 0;
                char         fname[CCHMAXPATH],*p;
                SHORT        x;

                x = (SHORT)WinSendDlgItemMsg(hwnd,
                                             IMGS_LISTBOX,
                                             LM_QUERYSELECTION,
                                             MPVOID,MPVOID);
                if(x >= 0) {
                  memset(&find,0,sizeof(MMFORMATINFO));
                  memset(&found,0,sizeof(MMFORMATINFO));
                  find.ulMediaType = MMIO_MEDIATYPE_IMAGE;
                  find.ulStructLen = sizeof(MMFORMATINFO);
                  find.fccIOProc = (FOURCC)WinSendDlgItemMsg(hwnd,IMGS_LISTBOX,
                                                             LM_QUERYITEMHANDLE,
                                                             MPFROMSHORT(x),
                                                             MPVOID);
                  if(!mmioGetFormats(&find,1,&found,&nf,0,0) && nf) {
                    *fname = 0;
                    WinQueryDlgItemText(hwnd,258,CCHMAXPATH,fname);
                    p = strrchr(fname,'.');
                    if(p)
                      *p = 0;
                    strcat(fname,".");
                    strcat(fname,found.szDefaultFormatExt);
                    WinSetDlgItemText(hwnd,258,fname);
                  }
                }
              }
              break;
          }
          break;
      }
      break;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          { /* get image type and save in wnd */
            IMAGEDATA *wnd;
            FILEDLG   *fdlg;
            char       imagetype[256],ctype[256];
            SHORT      x,numsels;

            fdlg = WinQueryWindowPtr(hwnd,0);
            wnd = (IMAGEDATA *)fdlg->ulUser;
            if(!wnd->convert) {
              WinQueryDlgItemText(hwnd,IMGS_BITCOUNT,3,imagetype);
              wnd->bitcount = (USHORT)atoi(imagetype);
              wnd->iconify = WinQueryButtonCheckstate(hwnd,IMGS_ICONIFY);
            }
            *imagetype = 0;
            WinQueryDlgItemText(hwnd,
                                IMGS_LISTBOX,
                                256,
                                imagetype);
            if(*imagetype) {
              if(!strcmp(imagetype,"Baseline JPEG")) {
                if(saymsg(MB_YESNOCANCEL | MB_ICONEXCLAMATION,
                          hwnd,
                          "Warning, Warning.  Danger, Will Robinson!",
                          "Some versions of OS/2 will fail or trap IMAGE "
                          "when attempting to save a JPG file (this behavior "
                          "observed in Warp up to at least version 4.0 with "
                          "fixpack 3)."
                          "\r\rDo you want to proceed anyway?") != MBID_YES)
                  return 0;
              }
              numsels = (SHORT)WinSendDlgItemMsg(hwnd,
                                                 IMGS_LISTBOX,
                                                 LM_QUERYITEMCOUNT,
                                                 MPVOID,MPVOID);
              if(numsels > 0) {
                for(x = 0;x < numsels;x++) {
                  *ctype = 0;
                  WinSendDlgItemMsg(hwnd,IMGS_LISTBOX,
                                    LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(x,256),
                                    MPFROMP(ctype));
                  if(!stricmp(ctype,imagetype)) {
                    wnd->saveas = (FOURCC)WinSendDlgItemMsg(hwnd,
                                                            IMGS_LISTBOX,
                                                            LM_QUERYITEMHANDLE,
                                                            MPFROMSHORT(x),
                                                            MPVOID);
                    break;
                  }
                }
                if(x >= numsels)
                  *imagetype = 0;
              }
            }
            if(!*imagetype) {
              saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                     hwnd,
                     "Image conversion error",
                     "Please select an image type.");
              WinSetFocus(HWND_DESKTOP,
                          WinWindowFromID(hwnd,
                                          IMGS_LISTBOX));
              return 0;
            }
          }
          break;
      }
      break;
  }
  return WinDefFileDlgProc(hwnd,msg,mp1,mp2);
}


void ConvertProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  IMAGEDATA *wnd;

  wnd = WinQueryWindowPtr(hwnd,0);
Again:
  if(wnd && !wnd->busy && wnd->hbm && *wnd->filename) {

    FILEDLG fdlg;
    char    drive[3],*pdrive = drive;

    memset(&fdlg,0,sizeof(FILEDLG));
    fdlg.cbSize =       sizeof(FILEDLG);
    fdlg.fl     =       FDS_HELPBUTTON | FDS_CENTER |
                        FDS_SAVEAS_DIALOG | FDS_CUSTOM;
    fdlg.pszTitle =     "FM/2:  Convert original image...";
    fdlg.pszOKButton =  "Convert";
    *drive = *wnd->filename;
    drive[1] = ':';
    drive[2] = 0;
    fdlg.pszIDrive = pdrive;
    strcpy(fdlg.szFullFile,wnd->filename);
    fdlg.pfnDlgProc = (PFNWP)ConvertImageDlgProc;
    fdlg.hMod = 0;
    fdlg.usDlgId = IMGS_FRAME;
    fdlg.ulUser = (ULONG)wnd;
    fdlg.lReturn = DID_CANCEL;
    wnd->convert = TRUE;
    wnd->saveas = wnd->mmFormatInfo.fccIOProc;
    WinFileDlg(HWND_DESKTOP,
               hwnd,
               &fdlg);
    if(fdlg.lReturn != DID_CANCEL &&
       *fdlg.szFullFile &&
       stricmp(wnd->filename,fdlg.szFullFile)) {

      CVTIMAGEDATA *cvt;

      cvt = malloc(sizeof(CVTIMAGEDATA));
      if(cvt) {
        memset(cvt,0,sizeof(CVTIMAGEDATA));
        strcpy(cvt->filenamel,wnd->filename);
        strcpy(cvt->filenames,fdlg.szFullFile);
        cvt->saveas = wnd->saveas;
        cvt->loadas = wnd->mmFormatInfo.fccIOProc;
        if(_beginthread(ConvertImageThread,NULL,65536 * 2,
                        (PVOID)cvt) == -1)
          free(cvt);
      }
    }
    else if(fdlg.lReturn != DID_CANCEL &&
            !stricmp(wnd->filename,fdlg.szFullFile)) {
      if(saymsg(MB_YESNOCANCEL | MB_ICONEXCLAMATION,hwnd,
                "Image conversion request error:",
                "Sorry, can't convert to the same filename.  "
                "Try again?") == MBID_YES)
      goto Again;
    }
    wnd->saveas = wnd->mmFormatInfo.fccIOProc;
  }
}


MRESULT EXPENTRY OpenImageDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static MMFORMATINFO *pmmf = NULL;
  static long          pnf = 0;

  switch(msg) {
    case WM_INITDLG:
      {
        MMFORMATINFO mmf;
        long         nf = 0;

        memset(&mmf,0,sizeof(MMFORMATINFO));
        mmf.ulMediaType = MMIO_MEDIATYPE_IMAGE;
        mmf.ulStructLen = sizeof(MMFORMATINFO);
        if(!mmioQueryFormatCount(&mmf,&nf,0,0) && nf) {
          pmmf = malloc(sizeof(MMFORMATINFO) * nf);
          if(pmmf) {
            memset(pmmf,0,sizeof(MMFORMATINFO) * nf);
            memset(&mmf,0,sizeof(MMFORMATINFO));
            mmf.ulMediaType = MMIO_MEDIATYPE_IMAGE;
            mmf.ulStructLen = sizeof(MMFORMATINFO);
            if(mmioGetFormats(&mmf,nf,pmmf,&pnf,0,0) || !pnf) {
              free(pmmf);
              pmmf = NULL;
            }
          }
        }
      }
      break;

    case FDM_FILTER:
      {
        MRESULT ret = (MRESULT)TRUE;
        ULONG   x;
        char   *p,mask[CCHMAXPATH];

        *mask = 0;
        WinQueryDlgItemText(hwnd,258,CCHMAXPATH,mask);
        if(strcmp(mask,"*"))
          break;
        if(pmmf && pnf) {
          ret = (MRESULT)FALSE;
          p = strrchr((char *)mp1,'.');
          if(p) {
            p++;
            for(x = 0;x < pnf;x++) {
              if((pmmf[x].ulFlags & MMIO_CANREADTRANSLATED) &&
                 !stricmp(p,pmmf[x].szDefaultFormatExt)) {
                ret = (MRESULT)TRUE;
                break;
              }
            }
          }
        }
        return ret;
      }

    case WM_DESTROY:
      if(pnf)
        pnf = 0;
      if(pmmf) {
        free(pmmf);
        pmmf = NULL;
      }
      break;
  }

  return WinDefFileDlgProc(hwnd,msg,mp1,mp2);
}


BOOL InvokeLoadImage (HWND hwnd,char *filename) {

  FILEDLG fdlg;
  char    drive[3],*pdrive = drive,temp[CCHMAXPATH];

  if(!filename)
    return FALSE;
  memset(&fdlg,0,sizeof(FILEDLG));
  fdlg.cbSize =       sizeof(FILEDLG);
  fdlg.fl     =       FDS_CENTER | FDS_OPEN_DIALOG | FDS_CUSTOM;
  fdlg.pszTitle =     "FM/2:  Open image file";
  fdlg.pszOKButton =  "Open";
  fdlg.pszIDrive = pdrive;
  if(!*filename) {
    save_dir(temp);
    *drive = toupper(*temp);
    strcpy(fdlg.szFullFile,temp);
    if(temp[strlen(temp) - 1] != '\\')
      strcat(fdlg.szFullFile,"\\");
    strcat(fdlg.szFullFile,"*");
  }
  else {
    *drive = toupper(*filename);
    strcpy(fdlg.szFullFile,filename);
    if(!strchr(filename,'*') && !strchr(filename,'?')) {
      if(!IsFile(filename)) {
        if(filename[strlen(filename) - 1] != '\\')
          strcat(fdlg.szFullFile,"\\");
      }
      strcat(fdlg.szFullFile,"*");
    }
  }
  drive[1] = ':';
  drive[2] = 0;
  fdlg.lReturn = DID_CANCEL;
  fdlg.pfnDlgProc = (PFNWP)OpenImageDlgProc;
  fdlg.usDlgId = LIMGS_FRAME;
  WinFileDlg(HWND_DESKTOP,
             hwnd,
             &fdlg);
  if(fdlg.lReturn == DID_CANCEL || !*fdlg.szFullFile ||
     IsFile(fdlg.szFullFile) != 1)
    return FALSE;
  strcpy(filename,fdlg.szFullFile);
  return TRUE;
}


void LoadPicture (HWND hwnd,char *szName) {

  if(szName && *szName) {

    FOURCC       fccStorageSystem = (FOURCC)0;
    APIRET       rc;
    MMFORMATINFO mmFormatInfo,mmFormatTemp;
    IMAGEDATA   *wnd = WinQueryWindowPtr(hwnd,0);
    char         tempname[CCHMAXPATH];

    memset(&mmFormatInfo,0,sizeof(MMFORMATINFO));
    mmFormatInfo.ulStructLen = sizeof(MMFORMATINFO);
    rc = mmioIdentifyFile(szName,
                          0L,
                          &mmFormatInfo,
                          &fccStorageSystem,
                          0L,
                          MMIO_FORCE_IDENTIFY_FF);
     if(rc != MMIO_ERROR &&
       mmFormatInfo.fccIOProc != FOURCC_DOS) {
      if(mmFormatInfo.ulMediaType == MMIO_MEDIATYPE_IMAGE &&
         (mmFormatInfo.ulFlags & MMIO_CANREADTRANSLATED) != 0) {
        mmFormatTemp = wnd->mmFormatInfo;
        wnd->mmFormatInfo = mmFormatInfo;
        strcpy(tempname,wnd->filename);
        {
          char s[CCHMAXPATH + 80];

          sprintf(s,"FM/2:  %s",szName);
          WinSetWindowText(WinQueryWindow(hwnd,QW_PARENT),s);
          WinSetWindowText(WinWindowFromID(WinQueryWindow(hwnd,
                           QW_PARENT),FID_TITLEBAR),s);
          strcpy(wnd->filename,szName);
        }
        wnd->busy++;
        if(_beginthread(LoadImageThread,NULL,65536 * 2,
                        (PVOID)wnd) == -1) {
          wnd->busy = 0;
          wnd->mmFormatInfo = mmFormatTemp;
          strcpy(wnd->filename,tempname);
          {
            char s[CCHMAXPATH + 80];

            sprintf(s,"FM/2:  %s",tempname);
            WinSetWindowText(WinQueryWindow(hwnd,QW_PARENT),s);
            WinSetWindowText(WinWindowFromID(WinQueryWindow(hwnd,
                             QW_PARENT),FID_TITLEBAR),s);
          }
        }
      }
    }
  }
}


void ClipBitmap (HAB hab,char *filename) {

  IMAGEDATA wnd;
  APIRET    rc;

  if(filename &&
     *filename) {

    FOURCC fccStorageSystem = (FOURCC)0;

    memset(&wnd,
           0,
           sizeof(IMAGEDATA));
    wnd.hab = hab;
    wnd.mmFormatInfo.ulStructLen = sizeof(MMFORMATINFO);
    rc = mmioIdentifyFile(filename,
                          0L,
                          &(wnd.mmFormatInfo),
                          &fccStorageSystem,
                          0L,
                          MMIO_FORCE_IDENTIFY_FF);
     if(rc != MMIO_ERROR &&
       wnd.mmFormatInfo.fccIOProc != FOURCC_DOS) {
      if(wnd.mmFormatInfo.ulMediaType == MMIO_MEDIATYPE_IMAGE &&
       (wnd.mmFormatInfo.ulFlags & MMIO_CANREADTRANSLATED) != 0) {
        strcpy(wnd.filename,filename);
        if(GetBitMap(&wnd) &&
           wnd.hbm) {
          if(WinOpenClipbrd(hab)) {
            if(!WinSetClipbrdData(hab,
                                  (ULONG)wnd.hbm,
                                  CF_BITMAP,
                                  CFI_HANDLE))
              GpiDeleteBitmap(wnd.hbm);
            WinCloseClipbrd(hab);
          }
          else
            GpiDeleteBitmap(wnd.hbm);
        }
      }
    }
  }
}


MRESULT EXPENTRY ResizeDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  RESIZEDATA  *rsz;
  static ULONG lastx,lasty;

  switch(msg) {
    case WM_INITDLG:
      rsz = (RESIZEDATA *)mp2;
      if(rsz &&
         rsz->xsize &&
         rsz->ysize) {

        char   s[34];

        rsz->ratio = (double)rsz->xsize / (double)rsz->ysize;
        if(!rsz->ratio) {
          WinCheckButton(hwnd,RSZ_RATIO,FALSE);
          WinEnableWindow(WinWindowFromID(hwnd,RSZ_RATIO),FALSE);
        }
        else
          WinCheckButton(hwnd,RSZ_RATIO,TRUE);
        WinSetWindowPtr(hwnd,0,rsz);
        sprintf(s,"%lu",rsz->xsize);
        lastx = rsz->xsize;
        WinSetDlgItemText(hwnd,RSZ_XSIZE,s);
        WinSendDlgItemMsg(hwnd,RSZ_XSIZE,EM_SETSEL,
                          MPFROM2SHORT(0,strlen(s)),MPVOID);
        sprintf(s,"%lu",rsz->ysize);
        lasty = rsz->ysize;
        WinSetDlgItemText(hwnd,RSZ_YSIZE,s);
        WinSendDlgItemMsg(hwnd,RSZ_YSIZE,EM_SETSEL,
                          MPFROM2SHORT(0,strlen(s)),MPVOID);
      }
      else
        WinDismissDlg(hwnd,0);
      break;

    case WM_CONTROL:
      rsz = WinQueryWindowPtr(hwnd,0);
      if(rsz) {
        if(WinQueryButtonCheckstate(hwnd,RSZ_RATIO)) {
          switch(SHORT1FROMMP(mp1)) {
            case RSZ_XSIZE:
            case RSZ_YSIZE:
              switch(SHORT2FROMMP(mp1)) {
                case EN_KILLFOCUS:
                  {
                    char  s[34];
                    ULONG size;

                    switch(SHORT1FROMMP(mp1)) {
                      case RSZ_XSIZE:
                        WinQueryDlgItemText(hwnd,RSZ_XSIZE,34,s);
                        size = abs(atol(s));
                        if(size != lastx) {
                          lastx = size;
                          sprintf(s,"%lu",size);
                          WinSetDlgItemText(hwnd,RSZ_XSIZE,s);
                          size = (ULONG)((double)size / rsz->ratio);
                          lasty = size;
                          sprintf(s,"%lu",size);
                          WinSetDlgItemText(hwnd,RSZ_YSIZE,s);
                        }
                        break;
                      case RSZ_YSIZE:
                        WinQueryDlgItemText(hwnd,RSZ_YSIZE,34,s);
                        size = abs(atol(s));
                        if(size != lasty) {
                          lasty = size;
                          sprintf(s,"%lu",size);
                          WinSetDlgItemText(hwnd,RSZ_YSIZE,s);
                          size = (ULONG)((double)size * rsz->ratio);
                          lastx = size;
                          sprintf(s,"%lu",size);
                          WinSetDlgItemText(hwnd,RSZ_XSIZE,s);
                        }
                        break;
                    }
                  }
                  break;
              }
              break;
          }
        }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;
        case DID_OK:
          rsz = WinQueryWindowPtr(hwnd,0);
          if(rsz) {

            char  s[34];
            ULONG xsize,ysize;

            WinQueryDlgItemText(hwnd,RSZ_XSIZE,34,s);
            xsize = abs(atol(s));
            WinQueryDlgItemText(hwnd,RSZ_YSIZE,34,s);
            ysize = abs(atol(s));
            if(xsize > 1 && ysize > 1) {
              rsz->xsize = xsize;
              rsz->ysize = ysize;
              WinDismissDlg(hwnd,1);
            }
            else
              DosBeep(50,100);
          }
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


void ResizeProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  IMAGEDATA *wnd;

  wnd = WinQueryWindowPtr(hwnd,0);
  if(wnd && !wnd->busy && wnd->hbm) {

    BITMAPINFOHEADER2 bmp2;
    RESIZEDATA        rsz;
    RECTL             rcl;
    HBITMAP           hbm,hbmTemp;

    memset(&bmp2,0,sizeof(bmp2));
    bmp2.cbFix = sizeof(bmp2);
    if(GpiQueryBitmapInfoHeader(wnd->hbm,&bmp2)) {
      memset(&rsz,0,sizeof(rsz));
      rsz.xsize = bmp2.cx;
      rsz.ysize = bmp2.cy;
      if(WinDlgBox(HWND_DESKTOP,
                   hwnd,
                   ResizeDlgProc,
                   0,
                   RSZ_FRAME,
                   &rsz) &&
         (rsz.xsize != bmp2.cx || rsz.ysize != bmp2.cy)) {
        rcl.xLeft = rcl.yBottom = 0;
        rcl.xRight = rsz.xsize - 1;
        rcl.yTop = rsz.ysize - 1;
        hbm = CopyBitmap(wnd->hab,wnd->hbm,&rcl);
        if(hbm) {
          hbmTemp = PasteBitmap(wnd->hab,hbm,wnd->hbm,&rcl);
          GpiDeleteBitmap(hbm);
          if(hbmTemp) {
            GpiDeleteBitmap(wnd->hbm);
            wnd->hbm = hbmTemp;
            if(!SetBestSize(WinQueryWindow(hwnd,QW_PARENT),wnd->hbm))
              WinInvalidateRect(hwnd,NULL,TRUE);
            WinPostMsg(hwnd,
                       WM_COMMAND,
                       MPFROM2SHORT(IDM_CANCEL,0),
                       MPVOID);
          }
        }
      }
    }
  }
}


char * FullDrgName (PDRAGITEM pDItem,char *buffer,ULONG buflen) {

  register ULONG len,blen;

  *buffer = 0;
  blen = DrgQueryStrName(pDItem->hstrContainerName,
                         buflen,buffer);
  if(blen) {
    if(*(buffer + (blen - 1L)) != '\\') {
      *(buffer + blen) = '\\';
      blen++;
    }
  }
  buffer[blen] = 0;
  len = DrgQueryStrName(pDItem->hstrSourceName,
                        buflen - blen,buffer + blen);
  buffer[blen + len] = 0;
  return buffer;
}


MRESULT EXPENTRY DragProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case DM_DRAGOVER:
      {
        PDRAGITEM pDItem;
        PDRAGINFO pDInfo;

        pDInfo = (PDRAGINFO)mp1;
        DrgAccessDraginfo(pDInfo);
        pDItem = DrgQueryDragitemPtr(pDInfo,0);
        if(DrgVerifyRMF(pDItem,"DRM_OS2FILE",NULL)) {
          DrgFreeDraginfo(pDInfo);
          return(MRFROM2SHORT(DOR_DROP,DO_COPY));
        }
        DrgFreeDraginfo(pDInfo);
      }
      return(MRFROM2SHORT(DOR_NEVERDROP,0));

    case DM_DROP:
      {
        PDRAGITEM   pDItem;
        PDRAGINFO   pDInfo;
        ULONG       numitems,x;
        static char szName[CCHMAXPATH + 2];

        pDInfo = (PDRAGINFO)mp1;
        DrgAccessDraginfo(pDInfo);
        numitems = DrgQueryDragitemCount(pDInfo);
        pDItem = DrgQueryDragitemPtr(pDInfo,0);
        if(DrgVerifyRMF(pDItem,"DRM_OS2FILE",NULL) &&
           !(pDItem->fsControl & DC_PREPARE)) {
          *szName = 0;
          FullDrgName(pDItem,szName,CCHMAXPATH + 1);
          if(*szName)
            LoadPicture(hwnd,szName);
          for(x = 0;x < numitems;x++) {
            pDItem = DrgQueryDragitemPtr(pDInfo,x);
            DrgSendTransferMsg(pDInfo->hwndSource,DM_ENDCONVERSATION,
                               MPFROMLONG(pDItem->ulItemID),
                               MPFROMLONG(DMFL_TARGETFAIL));
          }
        }
        DrgDeleteDraginfoStrHandles(pDInfo);
        DrgFreeDraginfo(pDInfo);
      }
      break;
  }
  return 0;
}


void SaveProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  IMAGEDATA *wnd;

  wnd = WinQueryWindowPtr(hwnd,0);
  if(wnd && !wnd->busy && wnd->hbm) {

    FILEDLG fdlg;
    char    drive[3],*pdrive = drive;
    ULONG   curdrive,drivemap;

    DosQCurDisk (&curdrive, &drivemap);
    memset(&fdlg,0,sizeof(FILEDLG));
    fdlg.cbSize =       sizeof(FILEDLG);
    fdlg.fl     =       FDS_CENTER | FDS_OPEN_DIALOG | FDS_CUSTOM;
    fdlg.pszTitle =     "FM/2:  Save new image file as...";
    fdlg.pszOKButton =  "Save";
    *drive = (*wnd->filename) ?
              *wnd->filename :
              (char)(curdrive + '@');
    drive[1] = ':';
    drive[2] = 0;
    fdlg.pszIDrive = pdrive;
    if(*wnd->filename)
      strcpy(fdlg.szFullFile,wnd->filename);
    else
      strcpy(fdlg.szFullFile,"CLIPBRD.BMP");
    fdlg.pfnDlgProc = (PFNWP)ConvertImageDlgProc;
    fdlg.hMod = 0;
    fdlg.usDlgId = IMGS_FRAME;
    fdlg.ulUser = (ULONG)wnd;
    fdlg.lReturn = DID_CANCEL;
    wnd->convert = FALSE;
    wnd->saveas = wnd->mmFormatInfo.fccIOProc;
    WinFileDlg(HWND_DESKTOP,
               hwnd,
               &fdlg);
    if(fdlg.lReturn != DID_CANCEL &&
       *fdlg.szFullFile) {
      if(SaveBitmap(wnd->hab,
                    wnd->hbm,
                    fdlg.szFullFile,
                    wnd->bitcount,
                    wnd->saveas) &&
         wnd->iconify) {
        if(!Iconify(wnd->hab,
                    wnd->hbm,
                    fdlg.szFullFile))
          saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                 hwnd,
                 "Bitmap save error",
                 "Image saved, but could not iconify.");
      }
    }
    wnd->saveas = wnd->mmFormatInfo.fccIOProc;
  }
}


void ImageInfoProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  IMAGEDATA *wnd;

  wnd = WinQueryWindowPtr(hwnd,0);
  if(wnd &&
     !wnd->busy &&
     wnd->hbm) {

    char             *name;
    long              ulBytesRead = 0;
    BITMAPINFOHEADER2 bmp2;
    FILESTATUS3       fs3;
    RECTL             rcl;

    memset(&bmp2,
           0,
           sizeof(bmp2));
    bmp2.cbFix = sizeof(bmp2);
    if(GpiQueryBitmapInfoHeader(wnd->hbm,
                                &bmp2)) {
      rcl = wnd->marked;
      FixRectl(wnd->hbm,
               hwnd,
               &rcl);
      if(*wnd->filename) {
        DosError(FERR_DISABLEHARDERR);
        if(DosQueryPathInfo(wnd->filename,
                            FIL_STANDARD,
                            &fs3,
                            (ULONG)sizeof(fs3)))
          fs3.cbFile = 0;
        name = malloc(wnd->mmFormatInfo.lNameLength + 1);
        if(name) {
          if(!mmioGetFormatName(&(wnd->mmFormatInfo),
                                name,
                                &ulBytesRead,
                                0,
                                0) &&
             ulBytesRead) {
            saymsg(MB_ENTER | MB_MOVEABLE | MB_ICONASTERISK,hwnd,
                   "                FM/2 Image information                ",
                   "Type:  \"%s\"\r\rSize:  %lu x %lu\r\r"
                   "Color bits:  %lu\r\rName:  \"%s\"\r\r"
                   "Disk size:  %lu bytes\r\r"
                   "Memory size:  %lu bytes\r\r"
                   "Selected area:  %lu x %lu",
                   name,
                   bmp2.cx,
                   bmp2.cy,
                   bmp2.cBitCount,
                   wnd->filename,
                   fs3.cbFile,
                   ((((((bmp2.cBitCount * bmp2.cx) + 31) / 32) *
                   4) * bmp2.cy) * bmp2.cPlanes) + sizeof(bmp2) +
                   ((bmp2.cBitCount == 24) ? 0 : (1 << bmp2.cBitCount)),
                   abs(rcl.xRight - rcl.xLeft) + 1,
                   abs(rcl.yTop - rcl.yBottom) + 1);
          }
        }
        free(name);
      }
      else
        saymsg(MB_ENTER | MB_MOVEABLE | MB_ICONASTERISK,hwnd,
               "                FM/2 Image information                ",
               "Type:  Clipboard bitmap\r\rSize:  %lu x %lu\r\r"
               "Color bits:  %lu\r\r"
               "Memory size:  %lu bytes\r\r"
               "Selected area:  %lu x %lu",
               bmp2.cx,
               bmp2.cy,
               bmp2.cBitCount,
               ((((((bmp2.cBitCount * bmp2.cx) + 31) / 32) *
               4) * bmp2.cy) * bmp2.cPlanes) + sizeof(bmp2) +
               ((bmp2.cBitCount == 24) ? 0 : (1 << bmp2.cBitCount)),
               abs(rcl.xRight - rcl.xLeft) + 1,
               abs(rcl.yTop - rcl.yBottom) + 1);
      return;
    }
  }
  DosBeep(50,100);
}


void LoadProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  IMAGEDATA *wnd;

  wnd = WinQueryWindowPtr(hwnd,0);
  if(wnd && !wnd->busy) {
    wnd->marking = FALSE;
    if(WinQueryCapture(HWND_DESKTOP) == hwnd)
      WinSetCapture(HWND_DESKTOP,(HWND)0);
    wnd->marked.xLeft = wnd->marked.xRight =
      wnd->marked.yTop = wnd->marked.yBottom = 0;
    if(SHORT1FROMMP(mp1) == IDM_IMAGERELOAD)
      LoadPicture(hwnd,wnd->filename);
    else {

      char filename[CCHMAXPATH],*p;

      strcpy(filename,wnd->filename);
      p = strrchr(filename,'\\');
      if(p) {
        if(p < filename + 3)
          p++;
        *p = 0;
      }
      if(InvokeLoadImage(hwnd,filename))
        LoadPicture(hwnd,filename);
    }
  }
}


MRESULT EXPENTRY ImageWndProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  IMAGEDATA      *wnd;
  static HWND     hwndMenu = (HWND)0;
  static HPOINTER hptrArt;

  switch(msg) {
    case WM_CREATE:
      hptrArt = WinLoadPointer(HWND_DESKTOP,0,ID_ARTICON);
      hpsWnd = WinGetPS(hwnd);
      if(!hpsWnd)
        return MRFROMSHORT(1);
hwndMain = hwnd;
      return 0;

    case DM_DRAGOVER:
      return DragProc(hwnd,msg,mp1,mp2);
    case DM_DROP:
      DragProc(hwnd,msg,mp1,mp2);
      return 0;

    case WM_CONTEXTMENU:
      wnd = WinQueryWindowPtr(hwnd,0);
      if(wnd && !wnd->busy) {
        wnd->marking = FALSE;
        if(WinQueryCapture(HWND_DESKTOP) == hwnd)
          WinSetCapture(HWND_DESKTOP,(HWND)0);
        if(wnd->marked.xLeft == wnd->marked.xRight ||
           wnd->marked.yTop == wnd->marked.yBottom)
          wnd->marked.xLeft = wnd->marked.xRight =
            wnd->marked.yTop = wnd->marked.yBottom = 0;
        WinSetFocus(HWND_DESKTOP,hwnd);
        if(!hwndMenu)
          hwndMenu = WinLoadMenu(HWND_DESKTOP,0,ID_IMAGE);
        if(hwndMenu) {

          POINTL ptl;
          SHORT  attr;
          SWP    swp;

          WinQueryWindowPos(hwnd,&swp);
          attr = (wnd->marked.xLeft != wnd->marked.xRight &&
                  wnd->marked.yBottom != wnd->marked.yTop &&
                  (wnd->marked.xLeft != 0 ||
                   wnd->marked.yBottom != 0 ||
                   wnd->marked.xRight != swp.cx ||
                   wnd->marked.yTop != swp.cy)) ?
                  0 : MIA_DISABLED;
          WinSendMsg(hwndMenu,MM_SETITEMATTR,
                     MPFROM2SHORT(IDM_CROP,TRUE),
                     MPFROM2SHORT(MIA_DISABLED,attr));
          if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
            attr = MIA_DISABLED;
            if(WinQueryClipbrdData(WinQueryAnchorBlock(hwnd),CF_BITMAP))
              attr = 0;
            WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
            WinSendMsg(hwndMenu,
                       MM_SETITEMATTR,
                       MPFROM2SHORT(IDM_PASTE,TRUE),
                       MPFROM2SHORT(MIA_DISABLED,attr));
          }
          attr = (*wnd->filename) ? 0 : MIA_DISABLED;
          WinSendMsg(hwndMenu,
                     MM_SETITEMATTR,
                     MPFROM2SHORT(IDM_IMAGERELOAD,TRUE),
                     MPFROM2SHORT(MIA_DISABLED,attr));
          WinSendMsg(hwndMenu,
                     MM_SETITEMATTR,
                     MPFROM2SHORT(IDM_CONVERTIMAGE,TRUE),
                     MPFROM2SHORT(MIA_DISABLED,attr));
          WinSendMsg(hwndMenu,
                     MM_SETITEMATTR,
                     MPFROM2SHORT(IDM_ICONIFY,TRUE),
                     MPFROM2SHORT(MIA_DISABLED,attr));
          WinQueryPointerPos(HWND_DESKTOP,&ptl);
          WinMapWindowPoints(HWND_DESKTOP,hwnd,&ptl,1L);
          if(!WinPopupMenu(hwnd,
                           hwnd,
                           hwndMenu,
                           ptl.x,
                           ptl.y,
                           0,
                           PU_HCONSTRAIN | PU_VCONSTRAIN |
                           PU_KEYBOARD   | PU_MOUSEBUTTON1)) {
            WinDestroyWindow(hwndMenu);
            hwndMenu = (HWND)0;
          }
        }
      }
      else
        DosBeep(150,100);
      return MRFROMSHORT(TRUE);

    case WM_MENUEND:
      if(hwndMenu == (HWND)mp2) {
        WinDestroyWindow(hwndMenu);
        hwndMenu = (HWND)0;
      }
      return 0;

    case WM_SIZE:
      wnd = WinQueryWindowPtr(hwnd,0);
      if(wnd) {
        wnd->marking = FALSE;
        wnd->marked.xLeft = wnd->marked.xRight =
          wnd->marked.yTop = wnd->marked.yBottom = 0;
      }
      return 0;

    case WM_BUTTON1DBLCLK:
      wnd = WinQueryWindowPtr(hwnd,0);
      if(wnd && !wnd->busy && wnd->hbm) {

        SWP    swp;
        HPS    hps;
        POINTL ptl;

        hps = WinGetPS(hwnd);
        if(hps) {
          WinQueryWindowPos(hwnd,&swp);
          wnd->marked.xLeft = wnd->marked.yBottom = 0;
          wnd->marked.xRight = swp.cx - 1;
          wnd->marked.yTop = swp.cy - 1;
          GpiSetColor(hps,CLR_PALEGRAY);
          GpiSetMix(hps,FM_XOR);
          ptl.x = wnd->marked.xLeft;
          ptl.y = wnd->marked.yBottom;
          GpiMove(hps,&ptl);
          ptl.x = wnd->marked.xRight;
          ptl.y = wnd->marked.yTop;
          GpiBox(hps,DRO_OUTLINE,&ptl,0,0);
          WinReleasePS(hps);
        }
      }
      break;

    case WM_BUTTON1DOWN:
    case WM_BUTTON1UP:
    case WM_MOUSEMOVE:
      if(msg == WM_MOUSEMOVE)
        WinSetPointer(HWND_DESKTOP,hptrCross);
      wnd = WinQueryWindowPtr(hwnd,0);
      if(wnd && !wnd->busy && wnd->hbm) {

        POINTS pts;
        POINTL ptl;
        HPS    hps;
        SWP    swp;

        if(msg != WM_BUTTON1DOWN && !wnd->marking)
          return MRFROMSHORT(TRUE);
        pts.x = SHORT1FROMMP(mp1);
        pts.y = SHORT2FROMMP(mp1);
        WinQueryWindowPos(hwnd,&swp);
        if(pts.x < 0 || pts.x > swp.cx ||
           pts.y < 0 || pts.y > swp.cy) {
          if(pts.x < 0)
            pts.x = 0;
          if(pts.x > swp.cx)
            pts.x = swp.cx;
          if(pts.y < 0)
            pts.y = 0;
          if(pts.y > swp.cy)
            pts.y = swp.cy;
          ptl.x = pts.x;
          ptl.y = pts.y;
          WinMapWindowPoints(hwnd,HWND_DESKTOP,&ptl,1L);
          WinSetPointerPos(HWND_DESKTOP,ptl.x,ptl.y);
        }
        hps = WinGetPS(hwnd);
        if(hps) {
          GpiSetColor(hps,CLR_PALEGRAY);
          GpiSetMix(hps,FM_XOR);
            if(msg == WM_BUTTON1DOWN || wnd->marking &&
               (wnd->marked.xLeft != wnd->marked.xRight &&
                wnd->marked.yTop != wnd->marked.yBottom)) {
            /* remove old box */
            ptl.x = wnd->marked.xLeft;
            ptl.y = wnd->marked.yBottom;
            GpiMove(hps,&ptl);
            ptl.x = wnd->marked.xRight;
            ptl.y = wnd->marked.yTop;
            GpiBox(hps,DRO_OUTLINE,&ptl,0,0);
          }
          if(msg != WM_BUTTON1DOWN && wnd->marking) {
            if(wnd->marked.xLeft != pts.x &&
                wnd->marked.yBottom != pts.y) {
              /* draw new box */
              ptl.x = wnd->marked.xLeft;
              ptl.y = wnd->marked.yBottom;
              GpiMove(hps,&ptl);
              ptl.x = pts.x;
              ptl.y = pts.y;
              GpiBox(hps,DRO_OUTLINE,&ptl,0,0);
            }
          }
          switch(msg) {
            case WM_BUTTON1DOWN:
              wnd->marking = TRUE;
              wnd->marked.xLeft = wnd->marked.xRight = pts.x;
              wnd->marked.yTop = wnd->marked.yBottom = pts.y;
              WinSetCapture(HWND_DESKTOP,hwnd);
              WinSetFocus(HWND_DESKTOP,hwnd);
              break;
            case WM_BUTTON1UP:
              wnd->marking = FALSE;
              wnd->marked.xRight = pts.x;
              wnd->marked.yTop = pts.y;
              if(wnd->marked.xLeft == wnd->marked.xRight ||
                 wnd->marked.yTop == wnd->marked.yBottom)
                wnd->marked.xLeft = wnd->marked.xRight =
                  wnd->marked.yTop = wnd->marked.yBottom = 0;
              if(WinQueryCapture(HWND_DESKTOP) == hwnd)
                WinSetCapture(HWND_DESKTOP,(HWND)0);
              break;
            case WM_MOUSEMOVE:
              wnd->marked.xRight = pts.x;
              wnd->marked.yTop = pts.y;
              break;
          }
          if(wnd->marking) {

            char s[80];

            sprintf(s,"%lu x %lu",
                    abs(wnd->marked.xRight - wnd->marked.xLeft),
                    abs(wnd->marked.yTop - wnd->marked.yBottom));
            WinSetWindowText(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                             FID_TITLEBAR),s);
          }
          else {

            char s[CCHMAXPATH + 80];

            sprintf(s,"FM/2:  %s",wnd->filename);
            WinSetWindowText(WinQueryWindow(hwnd,QW_PARENT),s);
            WinSetWindowText(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                             FID_TITLEBAR),s);
          }
          WinReleasePS(hps);
        }
      }
      return MRFROMSHORT(TRUE);

#ifdef USE_PALETTE
    case WM_REALIZEPALETTE:
      if(hpal) {

        ULONG changed;

        GpiSelectPalette(hpsWnd,hpal);
        if(WinRealizePalette(hwnd,hpsWnd,&changed) == PAL_ERROR)
          WinSetWindowText(WinQueryWindow(hwnd,QW_PARENT),"Palette error");
        else {

          char s[80];

          sprintf(s,"Palette: %lu",changed);
          WinSetWindowText(WinQueryWindow(hwnd,QW_PARENT),s);
        }
        return 0;
      }
      break;
#endif

    case WM_ERASEBACKGROUND:
      return MRFROMSHORT(TRUE);

    case WM_PAINT:
      wnd = WinQueryWindowPtr(hwnd,0);
      if(wnd &&
         !wnd->busy) {

        POINTL ptl;

        WinBeginPaint(hwnd,
                      hpsWnd,
                      NULL);
        GpiSetMix(hpsWnd,
                  FM_DEFAULT);
        DrawBitmap(hpsWnd,wnd);
        if(wnd->marked.xLeft != wnd->marked.xRight &&
           wnd->marked.yTop != wnd->marked.yBottom) {
          /* draw old box */
          GpiSetColor(hpsWnd,CLR_PALEGRAY);
          GpiSetMix(hpsWnd,
                    FM_XOR);
          ptl.x = wnd->marked.xLeft;
          ptl.y = wnd->marked.yBottom;
          GpiMove(hpsWnd,&ptl);
          ptl.x = wnd->marked.xRight;
          ptl.y = wnd->marked.yTop;
          GpiBox(hpsWnd,
                 DRO_OUTLINE,
                 &ptl,
                 0,
                 0);
        }
        WinEndPaint(hpsWnd);
      }
      else {

        HPS      hps;
        RECTL    rcl;

        hps = WinBeginPaint(hwnd,0,&rcl);
        WinEndPaint(hps);
        WinSendMsg(hwnd,
                   UM_PAINT,
                   MPVOID,
                   MPVOID);
      }
      return 0;

    case UM_PAINT:
      {
        HPS    hps;
        RECTL  rcl;
        POINTL ptl;

        hps = WinGetPS(hwnd);
        if(hps) {
          WinQueryWindowRect(hwnd,&rcl);
          WinFillRect(hps,
                      &rcl,
                      CLR_PALEGRAY);
          if(hptrArt)
            WinDrawPointer(hps,
                           4,
                           16,
                           hptrArt,
                           DP_NORMAL);
          WinDrawText(hps,
                      -1L,
                      "Loading image...",
                      &rcl,
                      CLR_BLACK,CLR_PALEGRAY,
                      DT_CENTER | DT_VCENTER);
          GpiSetColor(hps,CLR_BLACK);
          ptl.x = 1;
          ptl.y = 1;
          GpiMove(hps,&ptl);
          ptl.y = 7;
          GpiLine(hps,&ptl);
          ptl.x = 103;
          GpiLine(hps,&ptl);
          GpiSetColor(hps,CLR_WHITE);
          ptl.y = 1;
          GpiLine(hps,&ptl);
          ptl.x = 1;
          GpiLine(hps,&ptl);
          GpiSetColor(hps,CLR_DARKGRAY);
          GpiSetPattern(hps,PATSYM_HALFTONE);
          ptl.x = ptl.y = 2;
          GpiMove(hps,&ptl);
          ptl.x = 102;
          ptl.y = 6;
          GpiBox(hps,DRO_OUTLINEFILL,&ptl,0,0);
          WinReleasePS(hps);
        }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_COMPRESSION:
          wnd = WinQueryWindowPtr(hwnd,0);
          if(wnd &&
             !wnd->busy &&
             wnd->hbm) {
            if(wnd->isline)
              lcompression = (lcompression == BBO_IGNORE) ?
                              BBO_OR :
                              (lcompression == BBO_OR) ?
                               BBO_AND :
                               BBO_IGNORE;
            else
              rcompression = (rcompression == BBO_IGNORE) ?
                              BBO_OR :
                              (rcompression == BBO_OR) ?
                               BBO_AND :
                               BBO_IGNORE;
            WinInvalidateRect(hwnd,NULL,TRUE);
          }
          break;

        case IDM_ICONIFY:
          wnd = WinQueryWindowPtr(hwnd,0);
          if(wnd &&
             !wnd->busy &&
             wnd->hbm &&
             *wnd->filename) {
            if(!Iconify(wnd->hab,wnd->hbm,wnd->filename))
              saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                     hwnd,
                     "Iconify error",
                     "Couldn't iconify the image file.");
          }
          break;

        case IDM_CROP:
          wnd = WinQueryWindowPtr(hwnd,0);
          if(wnd &&
             !wnd->busy &&
             wnd->hbm) {

            HBITMAP hbm;
            RECTL   rcl;

            rcl = wnd->marked;
            if(FixRectl(wnd->hbm,hwnd,&rcl)) {
              hbm = CopyBitmap(wnd->hab,wnd->hbm,&rcl);
              if(hbm) {
                GpiDeleteBitmap(wnd->hbm);
                wnd->hbm = hbm;
                if(!SetBestSize(WinQueryWindow(hwnd,QW_PARENT),wnd->hbm))
                  WinInvalidateRect(hwnd,NULL,TRUE);
                WinPostMsg(hwnd,
                           WM_COMMAND,
                           MPFROM2SHORT(IDM_CANCEL,0),
                           MPVOID);
              }
            }
          }
          break;

        case IDM_VERTICAL:
        case IDM_HORIZONTAL:
          wnd = WinQueryWindowPtr(hwnd,0);
          if(wnd && !wnd->busy && wnd->hbm) {

            HBITMAP           hbm;
            RECTL             rcl;
            BITMAPINFOHEADER2 bmp2;

            memset(&bmp2,0,sizeof(bmp2));
            bmp2.cbFix = sizeof(bmp2);
            if(GpiQueryBitmapInfoHeader(wnd->hbm,&bmp2) &&
               bmp2.cx && bmp2.cy) {
              rcl.xLeft = rcl.yBottom = 0;
              rcl.xRight = bmp2.cx - 1;
              rcl.yTop = bmp2.cy - 1;
              switch(SHORT1FROMMP(mp1)) {
                case IDM_VERTICAL:
                  SpecialRectl(&rcl,FALSE,TRUE);
                  break;
                case IDM_HORIZONTAL:
                  SpecialRectl(&rcl,TRUE,FALSE);
                  break;
              }
              hbm = CopyBitmap(wnd->hab,wnd->hbm,&rcl);
              if(hbm) {
                GpiDeleteBitmap(wnd->hbm);
                wnd->hbm = hbm;
                WinInvalidateRect(hwnd,NULL,TRUE);
              }
            }
          }
          break;

        case IDM_ROTATE:
          saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                 hwnd,
                 "Sorry",
                 "Not implemented yet.");
          break;

        case IDM_RESIZE:
          ResizeProc(hwnd,msg,mp1,mp2);
          break;

        case IDM_COPY:
          wnd = WinQueryWindowPtr(hwnd,0);
          if(wnd && !wnd->busy && wnd->hbm) {

            HBITMAP hbm;
            RECTL   rcl;

            rcl = wnd->marked;
            if(FixRectl(wnd->hbm,hwnd,&rcl)) {
              hbm = CopyBitmap(wnd->hab,wnd->hbm,&rcl);
              if(hbm) {
                if(WinOpenClipbrd(wnd->hab)) {
                  if(!WinSetClipbrdData(wnd->hab,
                                        (ULONG)hbm,
                                        CF_BITMAP,CFI_HANDLE))
                    GpiDeleteBitmap(hbm);
                  WinCloseClipbrd(wnd->hab);
                }
                else
                  GpiDeleteBitmap(hbm);
              }
            }
          }
          break;

        case IDM_WEIRDPASTE:
        case IDM_STRETCHPASTE:
        case IDM_PASTE:
          PasteProc(hwnd,msg,mp1,mp2);
          break;

        case IDM_CANCEL:
          wnd = WinQueryWindowPtr(hwnd,0);
          if(wnd && !wnd->busy && wnd->hbm) {
            if(wnd->marked.xLeft != wnd->marked.xRight &&
               wnd->marked.yTop != wnd->marked.yBottom) {

              POINTL ptl;
              HPS    hps;

              hps = WinGetPS(hwnd);
              if(hps) {
                /* remove old box */
                GpiSetColor(hps,CLR_PALEGRAY);
                GpiSetMix(hps,FM_XOR);
                ptl.x = wnd->marked.xLeft;
                ptl.y = wnd->marked.yBottom;
                GpiMove(hps,&ptl);
                ptl.x = wnd->marked.xRight;
                ptl.y = wnd->marked.yTop;
                GpiBox(hps,DRO_OUTLINE,&ptl,0,0);
                WinReleasePS(hps);
              }
            }
            if(WinQueryCapture(HWND_DESKTOP) == hwnd)
              WinSetCapture(HWND_DESKTOP,(HWND)0);
            wnd->marking = FALSE;
            wnd->marked.xLeft = wnd->marked.xRight =
              wnd->marked.yTop = wnd->marked.yBottom = 0;
          }
          break;

        case IDM_CLOSE:
          WinPostMsg(hwnd,
                     WM_CLOSE,
                     MPVOID,
                     MPVOID);
          break;

        case IDM_HELP:
          WinMessageBox(HWND_DESKTOP,
                        hwnd,
                        "This program is a simple, fast image displayer "
                        "that works with OS/2's MMPM/2.  It is intended "
                        "for use with File Manager/2 (FM/2).\r"
                        "BTW, try IMAGE /C <imagefile> to copy an image "
                        "from its file to the clipboard...\r\r"
                        "Image is free software from Mark Kimes "
                        "and Steven H. Levine\r\r"
                        "Compiled "__DATE__ " " __TIME__ "\r"
                        "Version " PGM_VERSION,
                        "                             About Image                   ",
                        0,MB_ENTER | MB_ICONASTERISK | MB_MOVEABLE);
          break;

        case IDM_SAVEIMAGE:
          SaveProc(hwnd,
                   msg,
                   mp1,
                   mp2);
          break;

        case IDM_IMAGEINFO:
          ImageInfoProc(hwnd,
                        msg,
                        mp1,
                        mp2);
          break;

        case IDM_BESTSIZE:
          wnd = WinQueryWindowPtr(hwnd,0);
          if(wnd && !wnd->busy)
            SetBestSize(WinQueryWindow(hwnd,QW_PARENT),wnd->hbm);
          break;

        case IDM_IMAGERELOAD:
        case IDM_IMAGELOAD:
          LoadProc(hwnd,
                   msg,
                   mp1,
                   mp2);
          break;

        case IDM_CONVERTIMAGE:
          ConvertProc(hwnd,
                      msg,
                      mp1,
                      mp2);
          break;

        default:
          break;
      }
      return 0;

    case UM_NOTIFY:
      wnd = WinQueryWindowPtr(hwnd,0);
      if(wnd)
        wnd->busy++;
      return 0;

    case UM_CONTAINER_FILLED:
      wnd = WinQueryWindowPtr(hwnd,0);
      if(wnd) {
        if(wnd->killme)
          WinPostMsg(hwnd,
                     WM_CLOSE,
                     MPVOID,
                     MPVOID);
        if(mp1) {
          if(wnd != (IMAGEDATA *)mp1)
            *wnd = *(IMAGEDATA *)mp1;
          wnd->busy = 0;
          wnd->marking = FALSE;
          if(WinQueryCapture(HWND_DESKTOP) == hwnd)
            WinSetCapture(HWND_DESKTOP,(HWND)0);
          wnd->marked.xLeft = wnd->marked.xRight =
            wnd->marked.yTop = wnd->marked.yBottom = 0;
        }
        else {
          saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                 hwnd,
                 "Image file load error",
                 "Couldn't load image file \"%s\"",
                 wnd->filename);
          wnd->busy = 0;
          WinPostMsg(hwnd,
                     WM_CLOSE,
                     MPVOID,
                     MPVOID);
        }
      }
      else
        WinPostMsg(hwnd,
                   WM_CLOSE,
                   MPVOID,
                   MPVOID);
      return 0;

    case WM_CLOSE:
      wnd = WinQueryWindowPtr(hwnd,0);
      if(wnd && wnd->busy)
        wnd->killme = TRUE;
      else
        WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
      return 0;

    case WM_DESTROY:
      wnd = WinQueryWindowPtr(hwnd,0);
      if(wnd) {
        if(wnd->hbm &&
           wnd->hbm != HBM_ERROR)
          GpiDeleteBitmap(wnd->hbm);
        free(wnd);
        WinPostMsg((HWND)0,
                   WM_QUIT,
                   MPVOID,
                   MPVOID);
      }
      if(hptrArt)
        WinDestroyPointer(hptrArt);
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


HWND ShowPicture (HWND hwndParent,char *filename) {

  static ULONG flFrameFlags = FCF_TITLEBAR      | FCF_SYSMENU      |
                              FCF_MINMAX        | FCF_TASKLIST     |
                              FCF_SIZEBORDER    | FCF_ACCELTABLE   |
                              FCF_ICON          | FCF_NOBYTEALIGN;
  HWND         hwndFrame = (HWND)0,hwndClient;
  IMAGEDATA   *wnd;
  APIRET       rc;

  wnd = malloc(sizeof(IMAGEDATA));
  if(wnd) {
    memset(wnd,0,sizeof(IMAGEDATA));
    wnd->mmFormatInfo.ulStructLen = sizeof(MMFORMATINFO);
    wnd->iconify = TRUE;
    hwndFrame = WinCreateStdWindow(HWND_DESKTOP,
                                   0,
                                   &flFrameFlags,
                                   "FM/2 Image Viewer",
                                   "",
                                   0,
                                   0,
                                   ID_IMAGE,
                                   &hwndClient);
    if(hwndFrame) {
      wnd->hab = WinQueryAnchorBlock(hwndFrame);
      wnd->hwndFrame = hwndFrame;
      wnd->hwndClient = hwndClient;
      strcpy(wnd->filename,filename);
      {
        char s[CCHMAXPATH + 80];

        sprintf(s,
                "FM/2:  %s",
                (filename && *filename) ?
                 filename :
                 "<Clipboard>");
        WinSetWindowText(hwndFrame,s);
        WinSetWindowText(WinWindowFromID(hwndFrame,FID_TITLEBAR),s);
      }
      WinSetWindowPtr(hwndClient,0,wnd);
    }
  }
  if(filename &&
     *filename) {

    FOURCC fccStorageSystem = (FOURCC)0;

    rc = mmioIdentifyFile(filename,
                          0L,
                          &(wnd->mmFormatInfo),
                          &fccStorageSystem,
                          0L,
                          MMIO_FORCE_IDENTIFY_FF);
     if(rc != MMIO_ERROR &&
       wnd->mmFormatInfo.fccIOProc != FOURCC_DOS) {
      if(wnd->mmFormatInfo.ulMediaType == MMIO_MEDIATYPE_IMAGE &&
       (wnd->mmFormatInfo.ulFlags & MMIO_CANREADTRANSLATED) != 0) {
        wnd->busy++;
        {
          SWP swpM;

          if(IsFile(filename) == 1) {
            WinGetMaxPosition(hwndFrame,&swpM);
            WinSetWindowPos(hwndFrame,HWND_TOP,
                            (swpM.cx - 256) / 2,
                            (swpM.cy - 108) / 2,
                            256,
                            108,
                            SWP_MOVE | SWP_SIZE | SWP_SHOW);
          }
        }
        if(_beginthread(LoadImageThread,
                        NULL,
                        65536 * 2,
                        (PVOID)wnd) == -1) {
          wnd->busy = 0;
          WinDestroyWindow(hwndFrame);
          hwndFrame = (HWND)0;
        }
      }
      else {
        wnd->busy = 0;
        WinDestroyWindow(hwndFrame);
        hwndFrame = (HWND)0;
      }
    }
  }
  else {
    if(WinOpenClipbrd(wnd->hab)) {

      HBITMAP hbm;

      hbm = (HBITMAP)WinQueryClipbrdData(wnd->hab,CF_BITMAP);
      if(hbm) {

        BITMAPINFOHEADER2 bmp2;
        RECTL             rcl;

        memset(&bmp2,0,sizeof(bmp2));
        bmp2.cbFix = sizeof(bmp2);
        if(GpiQueryBitmapInfoHeader(hbm,&bmp2)) {
          rcl.xLeft = rcl.yBottom = 0;
          rcl.xRight = bmp2.cx - 1;
          rcl.yTop = bmp2.cy - 1;
          wnd->hbm = CopyBitmap(wnd->hab,hbm,&rcl);
          if(wnd->hbm) {
            WinSendMsg(wnd->hwndClient,
                       UM_CONTAINER_FILLED,
                       MPFROMP(wnd),
                       MPVOID);
            SetBestSize(hwndFrame,
                        wnd->hbm);
            WinInvalidateRect(wnd->hwndClient,
                              NULL,
                              TRUE);
          }
        }
      }
      WinCloseClipbrd(wnd->hab);
    }
    wnd->busy = 0;
    if(!wnd->hbm) {
      WinDestroyWindow(hwndFrame);
      hwndFrame = (HWND)0;
    }
  }
  return hwndFrame;
}


int main (int argc,char *argv[]) {

  HAB          hab;
  HMQ          hmq;
  QMSG         qmsg;
  HWND         hwndFrame;
#ifdef USE_PALETTE
  HDC          hdc;
  DEVOPENSTRUC dop = {NULL,"DISPLAY",NULL,NULL,NULL,NULL,NULL,NULL,NULL};
#endif
  static char  filename[CCHMAXPATH];

  hwndMain = HWND_DESKTOP;
  *filename = 0;
  DosError(FERR_DISABLEHARDERR);
  hab = WinInitialize(0);
  if(hab) {
    hmq = WinCreateMsgQueue(hab,256);
    if(hmq) {
      WinRegisterClass(hab,
                       "FM/2 Image Viewer",
                       ImageWndProc,
                       CS_SIZEREDRAW,
                       sizeof(PVOID));
      if(argc > 1) {
        if(!stricmp(argv[1],"/C")) {
          if(argc > 2) {
            if(!DosQueryPathInfo(argv[2],
                                 FIL_QUERYFULLNAME,
                                 filename,
                                 sizeof(filename)))
              ClipBitmap(hab,filename);
          }
          goto Abort;
        }
        if(DosQueryPathInfo(argv[1],
                            FIL_QUERYFULLNAME,
                            filename,
                            sizeof(filename)))
          *filename = 0;
      }
      if(IsFile(filename) != 1) {
        if(WinOpenClipbrd(hab)) {

          HBITMAP hbm;

          hbm = (HBITMAP)WinQueryClipbrdData(hab,CF_BITMAP);
          WinCloseClipbrd(hab);
          if(!hbm) {
            if(!InvokeLoadImage(HWND_DESKTOP,
                                filename))
              goto Abort;
          }
        }
        else if(!InvokeLoadImage(HWND_DESKTOP,
                                 filename))
          goto Abort;
      }
      hptrArrow = WinQuerySysPointer(HWND_DESKTOP,SPTR_ARROW,FALSE);
      hptrWait = WinQuerySysPointer(HWND_DESKTOP,SPTR_WAIT,FALSE);
      hptrCross = WinLoadPointer(HWND_DESKTOP,0,ID_CROSSICON);
#ifdef USE_PALETTE
      hdc = DevOpenDC(hab,
                      OD_MEMORY,
                      "*",
                      5L,
                      (PDEVOPENDATA)&dop,
                      0);
      if(hdc != DEV_ERROR) {
        DevQueryCaps(hdc,
                     CAPS_COLORS,
                     1,
                     &colors);
        DevQueryCaps(hdc,
                     CAPS_ADDITIONAL_GRAPHICS,
                     1,
                     &palsup);
        DevCloseDC(hdc);
        if(!(palsup & CAPS_PALETTE_MANAGER))
          colors = 0;
      }
      else
        colors = 0;
#endif
      if((hwndFrame = ShowPicture(HWND_DESKTOP,filename)) != (HWND)0) {
        while(WinGetMsg(hab,&qmsg,(HWND)0,0,0))
          WinDispatchMsg(hab,&qmsg);
        if(hpsWnd)
          WinReleasePS(hpsWnd);
        WinDestroyWindow(hwndFrame);
      }
Abort:
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}

