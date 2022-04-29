#define INCL_32
#define INCL_GPIBITMAPS
#define INCL_DOSFILEMGR
#define INCL_WIN
#define INCL_GPI
#define INCL_PM
#include <os2.h>
//#define INCL_MMIOOS2
//#include <os2me.h>
#include <mmioos2.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gbm.h>
#pragma pack(2)
typedef struct _GBMFILESTATUS
{
  GBM gbm;
  int ft;  /* GBM index for the proc to use */
  int fHandleGBM;
  PBYTE   lpRGBBuf;               /* 24-bit RGB Buf for translated data */
  ULONG   ulRGBTotalBytes;        /* Length of 24-bit RGBBuf       */
  LONG    lImgBytePos;            /* Current pos in RGB buf        */
  ULONG   ulImgTotalBytes;
  BOOL    bSetHeader;             /* TRUE if header set in WRITE mode*/
  MMIMAGEHEADER   mmImgHdr;       /* Standard image header         */
} GBMFILESTATUS;
typedef GBMFILESTATUS FAR *PGBMFILESTATUS;
ULONG readImageData(PGBMFILESTATUS pGBMInfo)
{
  GBM_ERR rcGBM;
  pGBMInfo->ulRGBTotalBytes =  ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
    * pGBMInfo->gbm.h;
  pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
  if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                   pGBMInfo->ulRGBTotalBytes,
                   fALLOC))
    {
      return (MMIO_ERROR);
    }
  if ( (rcGBM = gbm_read_data(pGBMInfo->fHandleGBM, pGBMInfo->ft,
                              &pGBMInfo->gbm, pGBMInfo->lpRGBBuf)) != GBM_ERR_OK )
    {
      DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      return (MMIO_ERROR);
    }
  pGBMInfo->lImgBytePos =  0;
  return MMIO_SUCCESS;
}
LONG EXPENTRY BMPproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "OS/2 / Windows bitmap (BMP)", 28);
      return(28);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20504d42;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "BMP");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=28;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY VGAproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "OS/2 / Windows bitmap (VGA)", 28);
      return(28);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20414756;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "VGA");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=28;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY BGAproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "OS/2 / Windows bitmap (BGA)", 28);
      return(28);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20414742;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "BGA");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=28;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY RLEproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "OS/2 / Windows bitmap (RLE)", 28);
      return(28);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20454c52;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "RLE");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=28;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY DIBproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "OS/2 / Windows bitmap (DIB)", 28);
      return(28);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20424944;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "DIB");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=28;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY RL4proc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "OS/2 / Windows bitmap (RL4)", 28);
      return(28);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20344c52;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "RL4");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=28;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY RL8proc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bitmap")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "OS/2 / Windows bitmap (RL8)", 28);
      return(28);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20384c52;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "RL8");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=28;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY GIFproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "GIF"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "GIF")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "CompuServe Graphics Interchange Format (GIF)", 45);
      return(45);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20464947;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "GIF");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=45;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY PCXproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PCX"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PCX")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "ZSoft PC Paintbrush Image format (PCX)", 39);
      return(39);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20584350;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "PCX");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=39;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY PCCproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PCX"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PCX")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "ZSoft PC Paintbrush Image format (PCC)", 39);
      return(39);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20434350;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "PCC");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=39;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY TIFproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "TIFF"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "TIFF")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Tagged Image File Format support (TIFF 6.0) (TIF)", 50);
      return(50);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20464954;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "TIF");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=50;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY TIFFproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "TIFF"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "TIFF")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Tagged Image File Format support (TIFF 6.0) (TIFF)", 51);
      return(51);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x46464954;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "TIFF");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=51;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY TGAproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Targa"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Targa")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Truevision Targa/Vista bitmap (TGA)", 36);
      return(36);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20414754;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "TGA");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=36;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY VSTproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Targa"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Targa")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Truevision Targa/Vista bitmap (VST)", 36);
      return(36);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20545356;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "VST");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=36;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY AFIproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Targa"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Targa")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Truevision Targa/Vista bitmap (AFI)", 36);
      return(36);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20494641;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "AFI");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=36;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY IFFproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "ILBM"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "ILBM")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Amiga IFF / ILBM Interleaved bitmap (IFF)", 42);
      return(42);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20464649;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "IFF");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=42;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY LBMproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "ILBM"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "ILBM")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Amiga IFF / ILBM Interleaved bitmap (LBM)", 42);
      return(42);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x204d424c;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "LBM");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=42;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY VIDproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "YUV12C"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "YUV12C")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "YUV12C M-Motion Video Frame Buffer (VID)", 41);
      return(41);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20444956;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "VID");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=41;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY PBMproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bit-map"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Bit-map")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Portable Bit-map (PBM)", 23);
      return(23);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x204d4250;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "PBM");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=23;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY PGMproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Greymap"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Greymap")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Portable Greyscale-map (PGM)", 29);
      return(29);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x204d4750;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "PGM");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=29;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY PPMproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Pixmap"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Pixmap")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Portable Pixel-map (PPM)", 25);
      return(25);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x204d5050;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "PPM");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=25;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY PNMproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Anymap"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Anymap")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Portable Any-map (PNM)", 23);
      return(23);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x204d4e50;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "PNM");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=23;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY KPSproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "KIPS"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "KIPS")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "IBM KIPS (KPS)", 15);
      return(15);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x2053504b;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "KPS");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=15;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY IAXproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "IAX"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "IAX")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "IBM Image Access eXecutive (IAX)", 33);
      return(33);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20584149;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "IAX");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=33;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY XBMproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "XBitmap"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "XBitmap")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "X Windows Bitmap (XBM)", 23);
      return(23);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x204d4258;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "XBM");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=23;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY XPMproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "XPixMap"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "XPixMap")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "X Windows PixMap (XPM)", 23);
      return(23);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x204d5058;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "XPM");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=23;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY SPRproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Sprite"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Sprite")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Archimedes Sprite from RiscOS (SPR)", 36);
      return(36);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20525053;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "SPR");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=36;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY SPRITEproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Sprite"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Sprite")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Archimedes Sprite from RiscOS (SPRITE)", 39);
      return(39);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x49525053;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "SPRITE");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=39;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY PSEproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PSEG"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PSEG")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "IBM Printer Page Segment (PSE)", 31);
      return(31);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20455350;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "PSE");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=31;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY PSEGproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PSEG"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PSEG")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "IBM Printer Page Segment (PSEG)", 32);
      return(32);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x47455350;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "PSEG");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=32;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY PSEG38PPproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PSEG"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PSEG")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "IBM Printer Page Segment (PSEG38PP)", 36);
      return(36);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x47455350;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "PSEG38PP");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=36;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY PSEG3820proc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PSEG"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PSEG")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "IBM Printer Page Segment (PSEG3820)", 36);
      return(36);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x47455350;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "PSEG3820");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=36;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY IMGproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "GemRas"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "GemRas")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "GEM Raster (IMG)", 17);
      return(17);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20474d49;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "IMG");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=17;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY XIMGproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "GemRas"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "GemRas")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "GEM Raster (XIMG)", 18);
      return(18);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x474d4958;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "XIMG");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=18;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY CVPproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Portrait"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "Portrait")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Portrait (CVP)", 15);
      return(15);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20505643;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "CVP");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=15;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY PNGproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PNG"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "PNG")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "Portable Network Graphics Format (PNG)", 39);
      return(39);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x20474e50;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "PNG");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=39;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY JPGproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "JPEG"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "JPEG")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "JPEG Interchange File Format (JPG)", 35);
      return(35);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x2047504a;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "JPG");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=35;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY JPEGproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "JPEG"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "JPEG")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "JPEG Interchange File Format (JPEG)", 36);
      return(36);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x4745504a;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "JPEG");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=36;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY JPEproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "JPEG"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "JPEG")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "JPEG Interchange File Format (JPE)", 35);
      return(35);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x2045504a;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "JPE");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=35;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY JP2proc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "JP2"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "JP2")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "JPEG2000 Graphics File Format (JP2)", 36);
      return(36);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x2032504a;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "JP2");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=36;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY J2Cproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "JP2"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "JP2")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "JPEG2000 Graphics File Format (J2C)", 36);
      return(36);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x2043324a;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "J2C");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=36;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY J2Kproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "J2K"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "J2K")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "JPEG2000 Codestream (J2K)", 26);
      return(26);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x204b324a;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "J2K");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=26;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY JPCproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "J2K"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "J2K")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "JPEG2000 Codestream (JPC)", 26);
      return(26);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x2043504a;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED|MMIO_CANWRITETRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "JPC");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=26;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
LONG EXPENTRY JPTproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2){
  PMMIOINFO pmmioinfo;
  pmmioinfo = (PMMIOINFO) pmmioStr;
  switch (usMsg)
  {
    case MMIOM_CLOSE:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lRetCode;
      USHORT          rc;
      GBMRGB  gbmrgb[0x100];
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lRetCode = MMIO_SUCCESS;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
      {
        switch (pGBMInfo->gbm.bpp)
        {
          case 24:
          {
            gbm_trunc_pal_7R8G4B(gbmrgb);
            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);
            pGBMInfo->gbm.bpp=8;
            break;
          }
          case 1:
          case 4:
          case 8:
          default: break;
        };
        if ( (rc = gbm_write("", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,
                                     gbmrgb, pGBMInfo->lpRGBBuf, "")) != GBM_ERR_OK )
        {
        }
      }
      if (pGBMInfo->lpRGBBuf)
      {
        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
      }
      gbm_io_close(pGBMInfo->fHandleGBM);
      DosFreeMem ((PVOID) pGBMInfo);
      return (lRetCode);
    }
    case MMIOM_OPEN:
    {
      PGBMFILESTATUS   pGBMInfo;
      MMIMAGEHEADER   MMImgHdr;
      ULONG           ulWidth;
      ULONG           ulHeight;
      PBYTE           lpRGBBufPtr;
      PSZ pszFileName = (CHAR *)lParam1;
      GBMFT gbmft;
      int ft, fd, n_ft, stride, bytes;
      BOOL bValidGBM= FALSE;
      int fOpenFlags;
      GBM_ERR     rc;
      if (!pmmioinfo) return (MMIO_ERROR);
      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
      {
        return (MMIO_ERROR);
      }
      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
      {
        return (MMIO_ERROR);
      }
      if (pmmioinfo->ulFlags &
            (MMIO_APPEND|
             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
      {
            return (MMIO_ERROR);
      }

      if(pmmioinfo->ulFlags & MMIO_DELETE)
      {
        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);
      }
      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                              sizeof (GBMFILESTATUS),
                              fALLOC)) return  MMIO_ERROR;
      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));
      if ( gbm_init() != GBM_ERR_OK )
      {
        DosFreeMem(pGBMInfo);
        return MMIO_ERROR;
      }
      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;
      if (pmmioinfo->ulFlags & MMIO_WRITE)
      {
        fOpenFlags=O_WRONLY|O_BINARY;
        if(pmmioinfo->ulFlags & MMIO_CREATE)
        {
          fOpenFlags|=O_CREAT|O_TRUNC;
          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        else
        {
          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
          {
            gbm_deinit();
            DosFreeMem(pGBMInfo);
            return (MMIO_ERROR);
          }
        }
        pGBMInfo->fHandleGBM=fd;
        return (MMIO_SUCCESS);
      }

      if(pmmioinfo->ulFlags & MMIO_WRITE)
          fOpenFlags=GBM_O_WRONLY;
      else if(pmmioinfo->ulFlags & MMIO_READWRITE)
          fOpenFlags=GBM_O_RDWR;
      else
          fOpenFlags=GBM_O_RDONLY;

      fOpenFlags|=GBM_O_BINARY;

      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
      {
        gbm_deinit();
        DosFreeMem(pGBMInfo);
        return (MMIO_ERROR);
      }
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "JPT"))
          {
            bValidGBM=TRUE;
            pGBMInfo->ft=ft;
            break;
          }
        }
      }

      if(!bValidGBM)
      {
        DosFreeMem(pGBMInfo);
        gbm_io_close(fd);
        gbm_deinit();
        return (MMIO_ERROR);
      }

      pGBMInfo->fHandleGBM=fd;

      if(pGBMInfo->gbm.bpp==8)
      {
        GBMRGB gbmrgb[0x100];
        RGB2 *rgb2;
        int a;

        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
        {
          DosFreeMem(pGBMInfo);
          gbm_io_close(fd);
          gbm_deinit();
          return (MMIO_ERROR);
        }
        rgb2=MMImgHdr.bmiColors;
        for(a=0;a<256;a++)
        {
          rgb2[a].bBlue=gbmrgb[a].b;
          rgb2[a].bGreen=gbmrgb[a].g;
          rgb2[a].bRed=gbmrgb[a].r;
        }
      }
      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                         sizeof (BITMAPINFOHEADER2);
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;
      if(pGBMInfo->gbm.bpp==8)
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
      }
      else
      {
        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                 ulWidth * ulHeight * 3;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
      }
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
      bytes = stride * pGBMInfo->gbm.h;
      pGBMInfo->ulRGBTotalBytes = bytes;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
      pGBMInfo->mmImgHdr = MMImgHdr;
      return MMIO_SUCCESS;
    }
    case MMIOM_READ:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            rc;
      LONG            lBytesToRead;
      GBM_ERR     rcGBM;

      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);
      if(!pGBMInfo->lpRGBBuf)
      {
        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
            pGBMInfo->ulImgTotalBytes)
           lBytesToRead =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else
           lBytesToRead = (ULONG)lParam2;
      memcpy ((PVOID)lParam1,
                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                lBytesToRead);
      pGBMInfo->lImgBytePos += lBytesToRead;

      return (lBytesToRead);
    }
    case MMIOM_WRITE:
    {
      PGBMFILESTATUS       pGBMInfo;
      USHORT              usBitCount;
      LONG                lBytesWritten;
      ULONG               ulImgBytesToWrite;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
      {
        return (MMIO_ERROR);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
      {
        return (MMIO_ERROR);
      }
      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
           pGBMInfo->ulImgTotalBytes)
           ulImgBytesToWrite =
               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
      else {
           ulImgBytesToWrite = (ULONG)lParam2;
      }
      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                (PVOID)lParam1,
                ulImgBytesToWrite);

      pGBMInfo->lImgBytePos += ulImgBytesToWrite;
      return (ulImgBytesToWrite);
    }
    case MMIOM_SEEK:
    {
      PGBMFILESTATUS   pGBMInfo;
      LONG            lNewFilePosition;
      LONG            lPosDesired;
      SHORT           sSeekMode;
      if (!pmmioinfo) return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      lPosDesired = lParam1;
      sSeekMode = (SHORT)lParam2;
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
      {
        if(!pGBMInfo->lpRGBBuf)
        {
          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;
        }
        switch (sSeekMode)
        {
          case SEEK_SET:
          {
            lNewFilePosition = lPosDesired;
            break;
          }
          case SEEK_CUR:
          {
            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
            break;
          }
          case SEEK_END:
          {
            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;
            break;
          }
          default :
            return (MMIO_ERROR);
        }
        if (lNewFilePosition < 0) return (MMIO_ERROR);
        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                lNewFilePosition = pGBMInfo->ulImgTotalBytes;
        pGBMInfo->lImgBytePos = lNewFilePosition;
        return (pGBMInfo->lImgBytePos);
      }
      return (MMIO_ERROR);
    }
    case MMIOM_SETHEADER:
    {
      PMMIMAGEHEADER          pMMImgHdr;
      PGBMFILESTATUS          pGBMInfo;
      USHORT                  usNumColors;
      ULONG                   ulImgBitsPerLine;
      ULONG                   ulImgBytesPerLine;
      ULONG                   ulBytesWritten;
      ULONG                   ulWidth;
      ULONG                   ul4PelWidth;
      ULONG                   ulHeight;
      USHORT                  usPlanes;
      USHORT                  usBitCount;
      USHORT                  usPadBytes;
      if (!pmmioinfo)
            return (MMIO_ERROR);
      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;
      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
            (pGBMInfo->bSetHeader))
        return (0);
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
            return (0);
      }
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return (0);
      }
      pMMImgHdr = (PMMIMAGEHEADER) lParam1;
      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
             BCA_UNCOMP) ||
            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
            (! ((usBitCount == 24) || (usBitCount == 8) ||
                (usBitCount == 4) || (usBitCount == 1)))
            )
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      if (lParam2 != sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;
      pGBMInfo->gbm.w=ulWidth;
      pGBMInfo->gbm.h=ulHeight;
      pGBMInfo->gbm.bpp=usBitCount;
      if(pGBMInfo->lpRGBBuf)
      {
        DosFreeMem(pGBMInfo->lpRGBBuf);
        pGBMInfo->lpRGBBuf=NULLHANDLE;
      }
      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
          * pGBMInfo->gbm.h;
      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;
      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                         pGBMInfo->ulRGBTotalBytes,
                         fALLOC))
      {
        return (MMIO_ERROR);
      }
      pGBMInfo->bSetHeader = TRUE;
      pGBMInfo->mmImgHdr = *pMMImgHdr;
      pGBMInfo->lImgBytePos = 0;
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_QUERYHEADERLENGTH:
    {
      if (!pmmioinfo)
          return (0);
      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
          return (sizeof (MMIMAGEHEADER));
      else
        return 0;
    }
    case MMIOM_GETHEADER:
    {
      PGBMFILESTATUS       pGBMInfo;
      if (!pmmioinfo) return (0);
      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;
      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
          (!(pGBMInfo->fHandleGBM)))
         return (0);
      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
      {
        return 0;
      }
      if (lParam2 < sizeof (MMIMAGEHEADER))
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
        return (0);
      }
      if (!lParam1)
      {
        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
        return (0);
      }
      memcpy ((PVOID)lParam1,
                (PVOID)&pGBMInfo->mmImgHdr,
                sizeof (MMIMAGEHEADER));
      return (sizeof (MMIMAGEHEADER));
    }
    case MMIOM_IDENTIFYFILE:
    {
      GBMFT gbmft;
      int ft, fd, n_ft;
      BOOL bValidGBM=FALSE;
      if (!lParam1 && !lParam2) return (MMIO_ERROR);
      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
      {
        return (MMIO_ERROR);
      }
      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;
      gbm_query_n_filetypes(&n_ft);
      for ( ft = 0; ft < n_ft; ft++ )
      {
        GBM gbm;
        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
        {
          gbm_query_filetype(ft, &gbmft);
          if(!stricmp(gbmft.short_name, "JPT")) bValidGBM=TRUE;
        }
      }
      gbm_io_close(fd);
      gbm_deinit();
      if(bValidGBM) return (MMIO_SUCCESS);
      return (MMIO_ERROR);
    }
    case MMIOM_GETFORMATNAME:
    {
      memcpy((PSZ)lParam1, "JPT Stream (JPEG2000, JPIP) (JPT)", 34);
      return(34);
    }
    case MMIOM_GETFORMATINFO:
    {
      PMMFORMATINFO pmmformatinfo;
      pmmformatinfo = (PMMFORMATINFO) lParam1;
      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
      pmmformatinfo->fccIOProc    = 0x2054504a;
      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;
      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED;
      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, "JPT");
      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
      pmmformatinfo->lNameLength=34;
      return (MMIO_SUCCESS);
    }
    default:
    {
      return (MMIOERR_UNSUPPORTED_MESSAGE);
    }
  }
  return (0);
}
