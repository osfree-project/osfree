/* File produces I/O Procedures for all GBM formats */

#include <os2.h>

//#define INCL_MMIOOS2
//#include <os2me.h>
#include <mmioos2.h>

#include <string.h>
#include <stdio.h>

#include <gbm.h>

struct STR_SAVED_TOKENS_ {
        char *str_next_saved_tokens;
        char str_ch_saved_tokens;
};

typedef struct STR_SAVED_TOKENS_ STR_SAVED_TOKENS;

static char *nxtToken = 0;              /* pointer to previous scanned string */
static char ch;                                         /* previous token delimiter */

char *StrTokenize(char *str, const char * const token)
{

        if(!str) {
                if((str = nxtToken) == 0                /* nothing to do */
                 || (*str++ = ch) == 0)               /* end of string reached */
                        return( nxtToken = 0);
        }

        if(!token || !*token)                   /* assume all the string */
                nxtToken = 0;
        else {
                nxtToken = str - 1;
                while(!strchr(token, *++nxtToken));
                ch = *nxtToken;
                *nxtToken = 0;
        }

        return( str);
}

void StrTokSave(STR_SAVED_TOKENS *st)
{

        if(st) {
                st->str_next_saved_tokens = nxtToken;
                st->str_ch_saved_tokens = ch;
        }

}

void StrTokRestore(STR_SAVED_TOKENS *st)
{

        if(st) {
                nxtToken = st->str_next_saved_tokens;
                ch = st->str_ch_saved_tokens;
        }

}


#define StrTokStop() (void)StrTokenize(0, 0)


void main(void)
{
  int n_ft, ft;
  GBMFT gbmft;
  char *p;
  STR_SAVED_TOKENS st;

  printf("#define INCL_32\n");
  printf("#define INCL_GPIBITMAPS\n");
  printf("#define INCL_DOSFILEMGR\n");
  printf("#define INCL_WIN\n");
  printf("#define INCL_GPI\n");
  printf("#define INCL_PM\n");
  printf("#include <os2.h>\n");
  printf("//#define INCL_MMIOOS2\n");
  printf("//#include <os2me.h>\n");
  printf("#include <mmioos2.h>\n");
  printf("#include <stdio.h>\n");
  printf("#include <stddef.h>\n");
  printf("#include <stdlib.h>\n");
  printf("#include <string.h>\n");
  printf("#include <ctype.h>\n");
  printf("#include <stdarg.h>\n");
  printf("#include <io.h>\n");
  printf("#include <fcntl.h>\n");
  printf("#include <sys/types.h>\n");
  printf("#include <sys/stat.h>\n");
  printf("#include <gbm.h>\n");
  printf("#include <gbmtrunc.h>\n");
  printf("#pragma pack(2)\n");
  printf("typedef struct _GBMFILESTATUS\n");
  printf("{\n");
  printf("  GBM gbm;\n");
  printf("  int ft;  /* GBM index for the proc to use */\n");
  printf("  int fHandleGBM;\n");
  printf("  PBYTE   lpRGBBuf;               /* 24-bit RGB Buf for translated data */\n");
  printf("  ULONG   ulRGBTotalBytes;        /* Length of 24-bit RGBBuf       */\n");
  printf("  LONG    lImgBytePos;            /* Current pos in RGB buf        */\n");
  printf("  ULONG   ulImgTotalBytes;\n");
  printf("  BOOL    bSetHeader;             /* TRUE if header set in WRITE mode*/\n");
  printf("  MMIMAGEHEADER   mmImgHdr;       /* Standard image header         */\n");
  printf("} GBMFILESTATUS;\n");
  printf("typedef GBMFILESTATUS FAR *PGBMFILESTATUS;\n");
  printf("ULONG readImageData(PGBMFILESTATUS pGBMInfo)\n");
  printf("{\n");
  printf("  GBM_ERR rcGBM;\n");
  printf("  pGBMInfo->ulRGBTotalBytes =  ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )\n");
  printf("    * pGBMInfo->gbm.h;\n");
  printf("  pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;\n");
  printf("  if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),\n");
  printf("                   pGBMInfo->ulRGBTotalBytes,\n");
  printf("                   fALLOC))\n");
  printf("    {\n");
  printf("      return (MMIO_ERROR);\n");
  printf("    }\n");
  printf("  if ( (rcGBM = gbm_read_data(pGBMInfo->fHandleGBM, pGBMInfo->ft,\n");
  printf("                              &pGBMInfo->gbm, pGBMInfo->lpRGBBuf)) != GBM_ERR_OK )\n");
  printf("    {\n");
  printf("      DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);\n");
  printf("      return (MMIO_ERROR);\n");
  printf("    }\n");
  printf("  pGBMInfo->lImgBytePos =  0;\n");
  printf("  return MMIO_SUCCESS;\n");
  printf("}\n");


  gbm_query_n_filetypes(&n_ft);

  for ( ft = 0; ft < n_ft; ft++ )
  {
    gbm_query_filetype(ft, &gbmft);

    StrTokSave(&st);
    if((p = StrTokenize((char*)gbmft.extensions, " ")) != 0) do if(*p)
    {
      printf("LONG EXPENTRY %sproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2)", p);
      printf("{\n");
      printf("  PMMIOINFO pmmioinfo;\n");
      printf("  pmmioinfo = (PMMIOINFO) pmmioStr;\n");
      printf("  switch (usMsg)\n");
      printf("  {\n");
      printf("    case MMIOM_CLOSE:\n");
      printf("    {\n");
      printf("      PGBMFILESTATUS   pGBMInfo;\n");
      printf("      LONG            lRetCode;\n");
      printf("      USHORT          rc;\n");
      printf("      GBMRGB  gbmrgb[0x100];\n");
      printf("      if (!pmmioinfo) return (MMIO_ERROR);\n");
      printf("      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;\n");
      printf("      lRetCode = MMIO_SUCCESS;\n");
      printf("      if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))\n");
      printf("      {\n");
      printf("        switch (pGBMInfo->gbm.bpp)\n");
      printf("        {\n");
      printf("          case 24:\n");
      printf("          {\n");
      printf("            gbm_trunc_pal_7R8G4B(gbmrgb);\n");
      printf("            gbm_trunc_7R8G4B(&pGBMInfo->gbm, pGBMInfo->lpRGBBuf, pGBMInfo->lpRGBBuf);\n");
      printf("            pGBMInfo->gbm.bpp=8;\n");
      printf("            break;\n");
      printf("          }\n");
      printf("          case 1:\n");
      printf("          case 4:\n");
      printf("          case 8:\n");
      printf("          default: break;\n");
      printf("        };\n");
      printf("        if ( (rc = gbm_write(\"\", pGBMInfo->fHandleGBM, pGBMInfo->ft, &pGBMInfo->gbm,\n");
      printf("                                     gbmrgb, pGBMInfo->lpRGBBuf, \"\")) != GBM_ERR_OK )\n");
      printf("        {\n");
      printf("        }\n");
      printf("      }\n");
      printf("      if (pGBMInfo->lpRGBBuf)\n");
      printf("      {\n");
      printf("        DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);\n");
      printf("      }\n");
      printf("      gbm_io_close(pGBMInfo->fHandleGBM);\n");
      printf("      DosFreeMem ((PVOID) pGBMInfo);\n");
      printf("      return (lRetCode);\n");
      printf("    }\n");
      printf("    case MMIOM_OPEN:\n");
      printf("    {\n");
      printf("      PGBMFILESTATUS   pGBMInfo;\n");
      printf("      MMIMAGEHEADER   MMImgHdr;\n");
      printf("      ULONG           ulWidth;\n");
      printf("      ULONG           ulHeight;\n");
      printf("      PBYTE           lpRGBBufPtr;\n");
      printf("      PSZ pszFileName = (CHAR *)lParam1;\n");
      printf("      GBMFT gbmft;\n");
      printf("      int ft, fd, n_ft, stride, bytes;\n");
      printf("      BOOL bValidGBM= FALSE;\n");
      printf("      int fOpenFlags;\n");
      printf("      GBM_ERR     rc;\n");
      printf("      if (!pmmioinfo) return (MMIO_ERROR);\n");
      printf("      if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&\n");
      printf("          ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||\n");
      printf("          (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))\n");
      printf("      {\n");
      printf("        return (MMIO_ERROR);\n");
      printf("      }\n");
      printf("      if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||\n");
      printf("               !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )\n");
      printf("      {\n");
      printf("        return (MMIO_ERROR);\n");
      printf("      }\n");
      printf("      if (pmmioinfo->ulFlags &\n");
      printf("            (MMIO_APPEND|\n");
      printf("             MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))\n");
      printf("      {\n");
      printf("            return (MMIO_ERROR);\n");
      printf("      }\n");
      printf("\n");
      printf("      if(pmmioinfo->ulFlags & MMIO_DELETE)\n");
      printf("      {\n");
      printf("        if(remove((PSZ) lParam1)==-1) return (MMIO_ERROR);\n");
      printf("      }\n");
      printf("      if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,\n");
      printf("                              sizeof (GBMFILESTATUS),\n");
      printf("                              fALLOC)) return  MMIO_ERROR;\n");
      printf("      memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));\n");
      printf("      if ( gbm_init() != GBM_ERR_OK )\n");
      printf("      {\n");
      printf("        DosFreeMem(pGBMInfo);\n");
      printf("        return MMIO_ERROR;\n");
      printf("      }\n");
      printf("      pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;\n");
      printf("      if (pmmioinfo->ulFlags & MMIO_WRITE)\n");
      printf("      {\n");
      printf("        fOpenFlags=O_WRONLY|O_BINARY;\n");
      printf("        if(pmmioinfo->ulFlags & MMIO_CREATE)\n");
      printf("        {\n");
      printf("          fOpenFlags|=O_CREAT|O_TRUNC;\n");
      printf("          if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )\n");
      printf("          {\n");
      printf("            gbm_deinit();\n");
      printf("            DosFreeMem(pGBMInfo);\n");
      printf("            return (MMIO_ERROR);\n");
      printf("          }\n");
      printf("        }\n");
      printf("        else\n");
      printf("        {\n");
      printf("          if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )\n");
      printf("          {\n");
      printf("            gbm_deinit();\n");
      printf("            DosFreeMem(pGBMInfo);\n");
      printf("            return (MMIO_ERROR);\n");
      printf("          }\n");
      printf("        }\n");
      printf("        pGBMInfo->fHandleGBM=fd;\n");
      printf("        return (MMIO_SUCCESS);\n");
      printf("      }\n");
      printf("\n");
      printf("      if(pmmioinfo->ulFlags & MMIO_WRITE)\n");
      printf("          fOpenFlags=GBM_O_WRONLY;\n");
      printf("      else if(pmmioinfo->ulFlags & MMIO_READWRITE)\n");
      printf("          fOpenFlags=GBM_O_RDWR;\n");
      printf("      else\n");
      printf("          fOpenFlags=GBM_O_RDONLY;\n");
      printf("\n");
      printf("      fOpenFlags|=GBM_O_BINARY;\n");
      printf("\n");
      printf("      if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )\n");
      printf("      {\n");
      printf("        gbm_deinit();\n");
      printf("        DosFreeMem(pGBMInfo);\n");
      printf("        return (MMIO_ERROR);\n");
      printf("      }\n");
      printf("      gbm_query_n_filetypes(&n_ft);\n");
      printf("      for ( ft = 0; ft < n_ft; ft++ )\n");
      printf("      {\n");
      printf("        if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, \"\") == GBM_ERR_OK )\n");
      printf("        {\n");
      printf("          gbm_query_filetype(ft, &gbmft);\n");
      printf("          if(!stricmp(gbmft.short_name, \"%s\"))\n",gbmft.short_name);
      printf("          {\n");
      printf("            bValidGBM=TRUE;\n");
      printf("            pGBMInfo->ft=ft;\n");
      printf("            break;\n");
      printf("          }\n");
      printf("        }\n");
      printf("      }\n");
      printf("\n");
      printf("      if(!bValidGBM)\n");
      printf("      {\n");
      printf("        DosFreeMem(pGBMInfo);\n");
      printf("        gbm_io_close(fd);\n");
      printf("        gbm_deinit();\n");
      printf("        return (MMIO_ERROR);\n");
      printf("      }\n");
      printf("\n");
      printf("      pGBMInfo->fHandleGBM=fd;\n");
      printf("\n");
      printf("      if(pGBMInfo->gbm.bpp==8)\n");
      printf("      {\n");
      printf("        GBMRGB gbmrgb[0x100];\n");
      printf("        RGB2 *rgb2;\n");
      printf("        int a;\n");
      printf("\n");
      printf("        if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )\n");
      printf("        {\n");
      printf("          DosFreeMem(pGBMInfo);\n");
      printf("          gbm_io_close(fd);\n");
      printf("          gbm_deinit();\n");
      printf("          return (MMIO_ERROR);\n");
      printf("        }\n");
      printf("        rgb2=MMImgHdr.bmiColors;\n");
      printf("        for(a=0;a<256;a++)\n");
      printf("        {\n");
      printf("          rgb2[a].bBlue=gbmrgb[a].b;\n");
      printf("          rgb2[a].bGreen=gbmrgb[a].g;\n");
      printf("          rgb2[a].bRed=gbmrgb[a].r;\n");
      printf("        }\n");
      printf("      }\n");
      printf("      MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);\n");
      printf("      MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;\n");
      printf("      MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =\n");
      printf("                         sizeof (BITMAPINFOHEADER2);\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   = BCA_UNCOMP;\n");
      printf("      if(pGBMInfo->gbm.bpp==8)\n");
      printf("      {\n");
      printf("        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =\n");
      printf("                   ulWidth * ulHeight;\n");
      printf("        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;\n");
      printf("        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;\n");
      printf("      }\n");
      printf("      else\n");
      printf("      {\n");
      printf("        MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =\n");
      printf("                 ulWidth * ulHeight * 3;\n");
      printf("            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;\n");
      printf("            MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;\n");
      printf("      }\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     = BRA_BOTTOMUP;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     = BRH_NOTHALFTONED;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;\n");
      printf("      stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );\n");
      printf("      bytes = stride * pGBMInfo->gbm.h;\n");
      printf("      pGBMInfo->ulRGBTotalBytes = bytes;\n");
      printf("      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;\n");
      printf("      MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;\n");
      printf("      pGBMInfo->mmImgHdr = MMImgHdr;\n");
      printf("      return MMIO_SUCCESS;\n");
      printf("    }\n");
      printf("    case MMIOM_READ:\n");
      printf("    {\n");
      printf("      PGBMFILESTATUS   pGBMInfo;\n");
      printf("      LONG            rc;\n");
      printf("      LONG            lBytesToRead;\n");
      printf("      GBM_ERR     rcGBM;\n");
      printf("\n");
      printf("      if (!pmmioinfo) return (MMIO_ERROR);\n");
      printf("      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;\n");
      printf("\n");
      printf("      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)) return (MMIO_ERROR);\n");
      printf("      if(!pGBMInfo->lpRGBBuf)\n");
      printf("      {\n");
      printf("        if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;\n");
      printf("      }\n");
      printf("      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >\n");
      printf("            pGBMInfo->ulImgTotalBytes)\n");
      printf("           lBytesToRead =\n");
      printf("               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;\n");
      printf("      else\n");
      printf("           lBytesToRead = (ULONG)lParam2;\n");
      printf("      memcpy ((PVOID)lParam1,\n");
      printf("                &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),\n");
      printf("                lBytesToRead);\n");
      printf("      pGBMInfo->lImgBytePos += lBytesToRead;\n");
      printf("\n");
      printf("      return (lBytesToRead);\n");
      printf("    }\n");
      printf("    case MMIOM_WRITE:\n");
      printf("    {\n");
      printf("      PGBMFILESTATUS       pGBMInfo;\n");
      printf("      USHORT              usBitCount;\n");
      printf("      LONG                lBytesWritten;\n");
      printf("      ULONG               ulImgBytesToWrite;\n");
      printf("      if (!pmmioinfo) return (MMIO_ERROR);\n");
      printf("      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;\n");
      printf("      if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))\n");
      printf("      {\n");
      printf("        return (MMIO_ERROR);\n");
      printf("      }\n");
      printf("      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))\n");
      printf("      {\n");
      printf("        return (MMIO_ERROR);\n");
      printf("      }\n");
      printf("      if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >\n");
      printf("           pGBMInfo->ulImgTotalBytes)\n");
      printf("           ulImgBytesToWrite =\n");
      printf("               pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;\n");
      printf("      else {\n");
      printf("           ulImgBytesToWrite = (ULONG)lParam2;\n");
      printf("      }\n");
      printf("      memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),\n");
      printf("                (PVOID)lParam1,\n");
      printf("                ulImgBytesToWrite);\n");
      printf("\n");
      printf("      pGBMInfo->lImgBytePos += ulImgBytesToWrite;\n");
      printf("      return (ulImgBytesToWrite);\n");
      printf("    }\n");
      printf("    case MMIOM_SEEK:\n");
      printf("    {\n");
      printf("      PGBMFILESTATUS   pGBMInfo;\n");
      printf("      LONG            lNewFilePosition;\n");
      printf("      LONG            lPosDesired;\n");
      printf("      SHORT           sSeekMode;\n");
      printf("      if (!pmmioinfo) return (MMIO_ERROR);\n");
      printf("      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;\n");
      printf("      lPosDesired = lParam1;\n");
      printf("      sSeekMode = (SHORT)lParam2;\n");
      printf("      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)\n");
      printf("      {\n");
      printf("        if(!pGBMInfo->lpRGBBuf)\n");
      printf("        {\n");
      printf("          if(readImageData(pGBMInfo)==MMIO_ERROR) return MMIO_ERROR;\n");
      printf("        }\n");
      printf("        switch (sSeekMode)\n");
      printf("        {\n");
      printf("          case SEEK_SET:\n");
      printf("          {\n");
      printf("            lNewFilePosition = lPosDesired;\n");
      printf("            break;\n");
      printf("          }\n");
      printf("          case SEEK_CUR:\n");
      printf("          {\n");
      printf("            lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;\n");
      printf("            break;\n");
      printf("          }\n");
      printf("          case SEEK_END:\n");
      printf("          {\n");
      printf("            lNewFilePosition = pGBMInfo->ulImgTotalBytes += lPosDesired;\n");
      printf("            break;\n");
      printf("          }\n");
      printf("          default :\n");
      printf("            return (MMIO_ERROR);\n");
      printf("        }\n");
      printf("        if (lNewFilePosition < 0) return (MMIO_ERROR);\n");
      printf("        if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)\n");
      printf("                lNewFilePosition = pGBMInfo->ulImgTotalBytes;\n");
      printf("        pGBMInfo->lImgBytePos = lNewFilePosition;\n");
      printf("        return (pGBMInfo->lImgBytePos);\n");
      printf("      }\n");
      printf("      return (MMIO_ERROR);\n");
      printf("    }\n");
      printf("    case MMIOM_SETHEADER:\n");
      printf("    {\n");
      printf("      PMMIMAGEHEADER          pMMImgHdr;\n");
      printf("      PGBMFILESTATUS          pGBMInfo;\n");
      printf("      USHORT                  usNumColors;\n");
      printf("      ULONG                   ulImgBitsPerLine;\n");
      printf("      ULONG                   ulImgBytesPerLine;\n");
      printf("      ULONG                   ulBytesWritten;\n");
      printf("      ULONG                   ulWidth;\n");
      printf("      ULONG                   ul4PelWidth;\n");
      printf("      ULONG                   ulHeight;\n");
      printf("      USHORT                  usPlanes;\n");
      printf("      USHORT                  usBitCount;\n");
      printf("      USHORT                  usPadBytes;\n");
      printf("      if (!pmmioinfo)\n");
      printf("            return (MMIO_ERROR);\n");
      printf("      pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;\n");
      printf("      if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||\n");
      printf("            (pGBMInfo->bSetHeader))\n");
      printf("        return (0);\n");
      printf("      if (!lParam1)\n");
      printf("      {\n");
      printf("        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;\n");
      printf("            return (0);\n");
      printf("      }\n");
      printf("      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))\n");
      printf("      {\n");
      printf("        return (0);\n");
      printf("      }\n");
      printf("      pMMImgHdr = (PMMIMAGEHEADER) lParam1;\n");
      printf("      usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;\n");
      printf("      if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=\n");
      printf("             BCA_UNCOMP) ||\n");
      printf("            (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||\n");
      printf("            (! ((usBitCount == 24) || (usBitCount == 8) ||\n");
      printf("                (usBitCount == 4) || (usBitCount == 1)))\n");
      printf("            )\n");
      printf("      {\n");
      printf("        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;\n");
      printf("        return (0);\n");
      printf("      }\n");
      printf("      if (lParam2 != sizeof (MMIMAGEHEADER))\n");
      printf("      {\n");
      printf("        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;\n");
      printf("        return (0);\n");
      printf("      }\n");
      printf("      ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;\n");
      printf("      ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;\n");
      printf("      pGBMInfo->gbm.w=ulWidth;\n");
      printf("      pGBMInfo->gbm.h=ulHeight;\n");
      printf("      pGBMInfo->gbm.bpp=usBitCount;\n");
      printf("      if(pGBMInfo->lpRGBBuf)\n");
      printf("      {\n");
      printf("        DosFreeMem(pGBMInfo->lpRGBBuf);\n");
      printf("        pGBMInfo->lpRGBBuf=NULLHANDLE;\n");
      printf("      }\n");
      printf("      pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )\n");
      printf("          * pGBMInfo->gbm.h;\n");
      printf("      pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;\n");
      printf("      if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),\n");
      printf("                         pGBMInfo->ulRGBTotalBytes,\n");
      printf("                         fALLOC))\n");
      printf("      {\n");
      printf("        return (MMIO_ERROR);\n");
      printf("      }\n");
      printf("      pGBMInfo->bSetHeader = TRUE;\n");
      printf("      pGBMInfo->mmImgHdr = *pMMImgHdr;\n");
      printf("      pGBMInfo->lImgBytePos = 0;\n");
      printf("      return (sizeof (MMIMAGEHEADER));\n");
      printf("    }\n");
      printf("    case MMIOM_QUERYHEADERLENGTH:\n");
      printf("    {\n");
      printf("      if (!pmmioinfo)\n");
      printf("          return (0);\n");
      printf("      if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)\n");
      printf("          return (sizeof (MMIMAGEHEADER));\n");
      printf("      else\n");
      printf("        return 0;\n");
      printf("    }\n");
      printf("    case MMIOM_GETHEADER:\n");
      printf("    {\n");
      printf("      PGBMFILESTATUS       pGBMInfo;\n");
      printf("      if (!pmmioinfo) return (0);\n");
      printf("      pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;\n");
      printf("      if ((pmmioinfo->ulFlags & MMIO_WRITE) ||\n");
      printf("          (!(pGBMInfo->fHandleGBM)))\n");
      printf("         return (0);\n");
      printf("      if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))\n");
      printf("      {\n");
      printf("        return 0;\n");
      printf("      }\n");
      printf("      if (lParam2 < sizeof (MMIMAGEHEADER))\n");
      printf("      {\n");
      printf("        pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;\n");
      printf("        return (0);\n");
      printf("      }\n");
      printf("      if (!lParam1)\n");
      printf("      {\n");
      printf("        pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;\n");
      printf("        return (0);\n");
      printf("      }\n");
      printf("      memcpy ((PVOID)lParam1,\n");
      printf("                (PVOID)&pGBMInfo->mmImgHdr,\n");
      printf("                sizeof (MMIMAGEHEADER));\n");
      printf("      return (sizeof (MMIMAGEHEADER));\n");
      printf("    }\n");
      printf("    case MMIOM_IDENTIFYFILE:\n");
      printf("    {\n");
      printf("      GBMFT gbmft;\n");
      printf("      int ft, fd, n_ft;\n");
      printf("      BOOL bValidGBM=FALSE;\n");
      printf("      if (!lParam1 && !lParam2) return (MMIO_ERROR);\n");
      printf("      if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )\n");
      printf("      {\n");
      printf("        return (MMIO_ERROR);\n");
      printf("      }\n");
      printf("      if ( gbm_init() != GBM_ERR_OK ) return MMIO_ERROR;\n");
      printf("      gbm_query_n_filetypes(&n_ft);\n");
      printf("      for ( ft = 0; ft < n_ft; ft++ )\n");
      printf("      {\n");
      printf("        GBM gbm;\n");
      printf("        if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, \"\") == GBM_ERR_OK )\n");
      printf("        {\n");
      printf("          gbm_query_filetype(ft, &gbmft);\n");
      printf("          if(!stricmp(gbmft.short_name, \"%s\")) bValidGBM=TRUE;\n", gbmft.short_name);
      printf("        }\n");
      printf("      }\n");
      printf("      gbm_io_close(fd);\n");
      printf("      gbm_deinit();\n");
      printf("      if(bValidGBM) return (MMIO_SUCCESS);\n");
      printf("      return (MMIO_ERROR);\n");
      printf("    }\n");
      printf("    case MMIOM_GETFORMATNAME:\n");
      printf("    {\n");
      printf("      memcpy((PSZ)lParam1, \"%s (%s)\", %d);\n", gbmft.long_name, p, strlen(gbmft.long_name)+strlen(p)+4);
      printf("      return(%d);\n", strlen(gbmft.long_name)+strlen(p)+4);
      printf("    }\n");
      printf("    case MMIOM_GETFORMATINFO:\n");
      printf("    {\n");
      printf("      PMMFORMATINFO pmmformatinfo;\n");
      printf("      pmmformatinfo = (PMMFORMATINFO) lParam1;\n");
      printf("      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);\n");
      printf("      pmmformatinfo->fccIOProc    = 0x%x;\n", mmioStringToFOURCC(p, MMIO_TOUPPER ));
      printf("      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;\n");
      printf("      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;\n");
      printf("      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED");
      if (gbmft.flags & (GBM_FT_W1|GBM_FT_W4|GBM_FT_W8|GBM_FT_W24|GBM_FT_W32|GBM_FT_W48|GBM_FT_W64))
        printf("|MMIO_CANWRITETRANSLATED");
      printf(";\n      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, \"%s\");\n",p);
      printf("      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;\n");
      printf("      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;\n");
      printf("      pmmformatinfo->lNameLength=%d;\n", strlen(gbmft.long_name)+strlen(p)+4);
      printf("      return (MMIO_SUCCESS);\n");
      printf("    }\n");
      printf("    default:\n");
      printf("    {\n");
      printf("      return (MMIOERR_UNSUPPORTED_MESSAGE);\n");
      printf("    }\n");
      printf("  }\n");
      printf("  return (0);\n");
      printf("}\n");
    } while((p = StrTokenize(0, " ")) != 0);
    StrTokRestore(&st);
  }
}
