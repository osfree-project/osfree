/*
 * Copyright (c) Chris Wohlgemuth 2002
 * All rights reserved.
 *
 * http://www.geocities.com/SiliconValley/Sector/5785/
 * http://www.os2world.com/cdwriting
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The authors name may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/************************************************************************/
/* Put all #defines here                                                */
/************************************************************************/

#define INCL_32                         /* force 32 bit compile */
#define INCL_GPIBITMAPS
#define INCL_DOSFILEMGR
#define INCL_WIN
#define INCL_GPI
#define INCL_PM

#define MEMCHECK

/************************************************************************/
/* Put all #includes here                                               */
/************************************************************************/

#include "jpgproc.h"


ULONG readImageData( PJPGFILESTATUS pJPGInfo)
{
  GBM_ERR rcGBM;
  /********************************************************
   * Determine total bytes in image
   ********************************************************/

  pJPGInfo->ulRGBTotalBytes =  ( ((pJPGInfo->gbm.w * pJPGInfo->gbm.bpp + 31)/32) * 4 )
    * pJPGInfo->gbm.h;

  pJPGInfo->ulImgTotalBytes = pJPGInfo->ulRGBTotalBytes;

  /********************************************************
   * Get space for full image buffer.
   * This will be retained until the file is closed.
   ********************************************************/
  if (DosAllocMem ((PPVOID) &(pJPGInfo->lpRGBBuf),
                   pJPGInfo->ulRGBTotalBytes,
                   fALLOC))
    {
#ifdef DEBUG
      writeLog("readImagedata(): no memory for image data.\n");
#endif
      return (MMIO_ERROR);
    }

#ifdef DEBUG
      writeLog("readImagedata(): allocated %d bytes for image data.\n", pJPGInfo->ulRGBTotalBytes);
#endif

  if ( (rcGBM = gbm_read_data(pJPGInfo->fHandleGBM, pJPGInfo->ft,
                              &pJPGInfo->gbm, pJPGInfo->lpRGBBuf)) != GBM_ERR_OK )
    {
      DosFreeMem ((PVOID) pJPGInfo->lpRGBBuf);
#ifdef DEBUG
      writeLog("readImagedata(): can't read image data.\n");
#endif
      return (MMIO_ERROR);
    }

  /********************************************************
   * RGB Buffer now full, set position pointers to the
   * beginning of the buffer.
   ********************************************************/
  pJPGInfo->lImgBytePos =  0;

  return MMIO_SUCCESS;
}


/************************************************************************/
/* MMOT IOProc                                                          */
/*                                                                      */
/* ARGUMENTS:                                                           */
/*                                                                      */
/*     PSZ pmmioStr - pointer to MMIOINFO block                         */
/*     USHORT usMsg - MMIO message being sent                           */
/*     LONG lParam1 - filename or other parameter depending on message  */
/*     LONG lParam2 - used with some messages as values                 */
/*                                                                      */
/*                                                                      */
/*  RETURN:                                                             */
/*                                                                      */
/*      MMIOM_OPEN                                                      */
/*          Success           - MMIO_SUCCESS     (0)                    */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*                                                                      */
/*      MMIOM_READ                                                      */
/*          Success           - Returns the number of bytes actually    */
/*                              read.  Return 0L if no more bytes can   */
/*                              be read.                                */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*                                                                      */
/*      MMIOM_WRITE                                                     */
/*          Success           - Returns the number of bytes actually    */
/*                              written.                                */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*                                                                      */
/*      MMIOM_SEEK                                                      */
/*          Success           - Returns the new file position           */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*                                                                      */
/*      MMIOM_CLOSE                                                     */
/*          Success           - MMIO_SUCCESS     (0)                    */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*          Other             - MMIO_WARNING, file was closed but the   */
/*                              IOProc expected additional data         */
/*                                                                      */
/*      MMIOM_GETFORMATNAME                                             */
/*          Success           - Returns the number of bytes read into   */
/*                              the buffer (size of format name)        */
/*          Failure           - Return 0                                */
/*                                                                      */
/*      MMIOM_GETFORMATINFO                                             */
/*          Success           - MMIO_SUCCESS     (0)                    */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*                                                                      */
/*      MMIOM_QUERYHEADERLENGTH                                         */
/*          Success           - Returns the size of the header in bytes */
/*          Failure           - Return 0                                */
/*                                                                      */
/*      MMIOM_IDENTIFYFILE                                              */
/*          Success           - MMIO_SUCCESS     (0)                    */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*                                                                      */
/*      MMIOM_GETHEADER                                                 */
/*          Success           - Returns number of bytes copied into     */
/*                              the header structure.                   */
/*          Failure           - Return 0                                */
/*          Other             - If length passed in was not large       */
/*                              enough to hold header then,             */
/*                              MMIOERR_INVALID_BUFFER_LENGTH is set    */
/*                              in ulErrorRet.                          */
/*                            - If header is bad,                       */
/*                              MMIOERR_INVALID_STRUCTURE is set in     */
/*                              ulErrorRet                              */
/*                                                                      */
/*      MMIOM_SETHEADER                                                 */
/*          Success           - Returns number of bytes written         */
/*          Failure           - Return 0                                */
/*          Other             - If header is bad,                       */
/*                              MMIOERR_INVALID_STRUCTURE is set in     */
/*                              ulErrorRet                              */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      This routine will translate IBM M-Motion YUV Video data into    */
/*      OS/2 2.0 memory bitmap data and back again.  The IOProc is part */
/*      of OS/2 MultiMedia Extentions File Format Conversion Utility.   */
/*                                                                      */
/*  GLOBAL VARS REFERENCED:                                             */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  GLOBAL VARS MODIFIED:                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  NOTES:                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  SIDE EFFECTS:                                                       */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/

LONG EXPENTRY IOProc_Entry (PVOID  pmmioStr,
                             USHORT usMsg,
                             LONG   lParam1,
                             LONG   lParam2)

    {
    PMMIOINFO   pmmioinfo;                      /* MMIOINFO block */

    pmmioinfo = (PMMIOINFO) pmmioStr;

#ifdef DEBUG
    writeLog("MSG: %d %x\n", usMsg,usMsg);
#endif

    switch (usMsg)
        {
        /*#############################################################*
         * When Closing the file, perform the following:
         * 1) Setup Variables
         * 2) Process the Image buffer
         * 3) Compress the Image to appropriate format
         *#############################################################*/
        case MMIOM_CLOSE:
            {
            /************************************************************
             * Declare local variables.
             ************************************************************/
            PJPGFILESTATUS   pJPGInfo;         /* MMotionIOProc instance data */

            LONG            lRetCode;
            USHORT          rc;

#ifdef DEBUG
            writeLog("MMIO_CLOSE\n");
#endif

            /***********************************************************
             * Check for valid MMIOINFO block.
             ***********************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /***********************************************************
             * Set up our working file status variable.
             ***********************************************************/
            pJPGInfo = (PJPGFILESTATUS)pmmioinfo->pExtraInfoStruct;

            /***********************************************************
             * Assume success for the moment....
             ***********************************************************/
            lRetCode = MMIO_SUCCESS;


            /************************************************************
             * see if we are in Write mode and have a buffer to write out.
             *    We have no image buffer in UNTRANSLATED mode.
             ************************************************************/
            if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pJPGInfo->lpRGBBuf))
                {
                  int n_ft, ft;
                  BOOL bValidJPG;

                  /* Write the buffer to disk */
#ifdef DEBUG
                  writeLog("MMIO_CLOSE: pJPGInfo->gbm.w: %d, pJPGInfo->gbm.h: %d, pJPGInfo->gbm.bpp; %d\n",
                           pJPGInfo->gbm.w, pJPGInfo->gbm.h, pJPGInfo->gbm.bpp);
                  writeLog("MMIO_CLOSE: pJPGInfo->fHandleGBM: %d, pJPGInfo->ft: %d\n",
                           pJPGInfo->fHandleGBM, pJPGInfo->ft);

#endif
                  gbm_query_n_filetypes(&n_ft);

                  for ( ft = 0; ft < n_ft; ft++ )
                    {
                      GBMFT gbmft;
                      gbm_query_filetype(ft, &gbmft);
                      if(!stricmp(gbmft.short_name, "JPEG")) {
                        bValidJPG=TRUE;
                        break;
                      }
                    }

                  if(bValidJPG) {
                    if ( (rc = gbm_write("", pJPGInfo->fHandleGBM, ft, &pJPGInfo->gbm,
                                         NULL/*gbmrgb*/, pJPGInfo->lpRGBBuf, "")) != GBM_ERR_OK )
                      {
#ifdef DEBUG
                        writeLog("MMIO_CLOSE: can't write image data. rc=%d. %s\n", rc, gbm_err(rc));
#endif
                      }
                  }

                }  /* end IF WRITE & IMAGE BUFFER block */

            /***********************************************************
             * Free the RGB buffer, if it exists, that was created
             * for the translated READ operations.
             ***********************************************************/
            if (pJPGInfo->lpRGBBuf)
              {
                DosFreeMem ((PVOID) pJPGInfo->lpRGBBuf);
              }
            /***********************************************************
             * Close the file
             ***********************************************************/
            gbm_io_close(pJPGInfo->fHandleGBM);

            DosFreeMem ((PVOID) pJPGInfo);

            return (lRetCode);
            }  /* end case of MMIOM_CLOSE */

        /*#############################################################*
         * Get the NLS format Information.
         *#############################################################*/
        case MMIOM_GETFORMATINFO:
            {
            /***********************************************************
             * Declare local variables.
             ***********************************************************/
            PMMFORMATINFO       pmmformatinfo;

#ifdef DEBUG
            writeLog("MMIO_GETFORMATINFO\n");
#endif

            /************************************************************
             * Set pointer to MMFORMATINFO structure.
             ************************************************************/
            pmmformatinfo = (PMMFORMATINFO) lParam1;

            /************************************************************
             * Fill in the values for the MMFORMATINFO structure.
             ************************************************************/
            pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
            pmmformatinfo->fccIOProc    = FOURCC_JPG;
            pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
            pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;

            pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED |
              MMIO_CANSEEKTRANSLATED        |
              MMIO_CANWRITETRANSLATED
              /* MMIO_CANREADUNTRANSLATED      |
                 MMIO_CANWRITETRANSLATED       |
                 MMIO_CANWRITEUNTRANSLATED     |
                 MMIO_CANREADWRITEUNTRANSLATED |

                 MMIO_CANSEEKUNTRANSLATED */;

            strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, pszJPGExt);

  pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
  pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;

  pmmformatinfo->lNameLength=9;


            /************************************************************
             * Return success back to the application.
             ************************************************************/
            return (MMIO_SUCCESS);
            } /* end case of MMIOM_GETFORMATINFO */

        /*#############################################################*
         * Get the NLS format name.
         *#############################################################*/
        case MMIOM_GETFORMATNAME:
            {
              memcpy((char *)lParam1, "JPG Image", 10);
              return(10);
            } /* end case of MMIOM_GETFORMATNAME */
        /*#############################################################*
         * Get the file header.
         *#############################################################*/
        case MMIOM_GETHEADER:
            {
            /************************************************************
             * Declare local variables.
             ************************************************************/
              //  PMMFILESTATUS       pVidInfo;
            PJPGFILESTATUS       pJPGInfo;
#ifdef DEBUG
            writeLog("MMIO_GETHEADER\n");
#endif

            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (0);

            /************************************************************
             * Set up our working file status variable.
             ************************************************************/
            pJPGInfo = (PJPGFILESTATUS)pmmioinfo->pExtraInfoStruct;

            /**************************************************
             * Getheader only valid in READ or READ/WRITE mode.
             * There is no header to get in WRITE mode.  We
             * must also have a valid file handle to read from
             **************************************************/
            if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
                (!(pJPGInfo->fHandleGBM)))
               return (0);

            /************************************************************
             * Check for Translation mode.
             ************************************************************/
            if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
              {
                /********************************************************
                 * Translation is off.
                 ********************************************************/
                /* Unstranslatd headers are not supported !!! */
                return 0;
              }   /* end IF NOT TRANSLATED block */

            /******************
             * TRANSLATION IS ON
             ******************/
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
                    (PVOID)&pJPGInfo->mmImgHdr,
                    sizeof (MMIMAGEHEADER));

            return (sizeof (MMIMAGEHEADER));
            } /* end case of MMIOM_GETHEADER */

        /*#############################################################*
         * Identify whether this file can be processed.
         *#############################################################*/
        case MMIOM_IDENTIFYFILE:
            {

            /************************************************************
             * Declare local variables.
             ************************************************************/
            GBMFT gbmft;
            int ft, fd, n_ft;
            BOOL bValidJPG= FALSE;

            ULONG           ulTempFlags = MMIO_READ | MMIO_DENYWRITE |
                                          MMIO_NOIDENTIFY;
                                           /* flags used for temp open  */
                                           /* and close                 */
#ifdef DEBUG
            writeLog("MMIO_IDENTIFYFILE\n");
#endif

            /************************************************************
             * We need either a file name (lParam1) or file handle (lParam2)
             ************************************************************/
            if (!lParam1 && !lParam2)
                return (MMIO_ERROR);

            if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
              {
                return (MMIO_ERROR);
              }

            if ( gbm_init() != GBM_ERR_OK )
              return MMIO_ERROR;

            gbm_query_n_filetypes(&n_ft);

            for ( ft = 0; ft < n_ft; ft++ )
              {
                GBM gbm;

                if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
                  {
                    gbm_query_filetype(ft, &gbmft);
                    if(!stricmp(gbmft.short_name, "JPEG"))
                      bValidJPG=TRUE;
                  }
              }

            gbm_io_close(fd);
            gbm_deinit();

            if(bValidJPG) {
#ifdef DEBUG
            writeLog("File seems to be a JPEG\n");
#endif
              return (MMIO_SUCCESS);
            }

            return (MMIO_ERROR);

            } /* end case of MMIOM_IDENTIFYFILE */

        /*#############################################################*/
        /*#############################################################*/
        case MMIOM_OPEN:
            {

            /************************************************************
             * Declare local variables
             ************************************************************/


            PJPGFILESTATUS   pJPGInfo;   /* pointer to a JPG file       */
                                        /* status structure that we will*/
                                        /* use for this file instance   */

            MMIMAGEHEADER   MMImgHdr;
            ULONG           ulWidth;
            ULONG           ulHeight;
            PBYTE           lpRGBBufPtr;

            PSZ pszFileName = (CHAR *)lParam1;  /* get the filename from    */
                                                /* parameter                */

            GBMFT gbmft;
            int ft, fd, n_ft, stride, bytes;
            BOOL bValidJPG= FALSE;
            int fOpenFlags;
            GBM_ERR     rc;

#ifdef DEBUG
            writeLog("MMIO_OPEN\n");
#endif
            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /************************************************************
             * If flags show read and write then send back an error.  We
             * only support reading or writing but not both at the same
             * time on the same file.
             ************************************************************/
            if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
                ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
                 (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
                {
#ifdef DEBUG
                writeLog("MMIO_OPEN, read/write not supported.\n");
#endif
                return (MMIO_ERROR);
                }

            /* We can't read/write untranslated */
            if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
                   !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
              {
#ifdef DEBUG
                writeLog("MMIO_OPEN, untranslated not supported.\n");
#endif
                return (MMIO_ERROR);
              }


            /*!!!!!!!!!!!!!!!!!!!!!! FIXME !!!!!!!!!!!!!!!!!!!!!!!*/
            /* To be honest we can almost nothing ;-) */
            if (pmmioinfo->ulFlags &
                (MMIO_APPEND|
                 MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
              {
#ifdef DEBUG
                writeLog("MMIO_OPEN, unsupported flag in %x\n", pmmioinfo->ulFlags);
#endif
//                return (MMIO_ERROR);
              }

            /* Caller requested a delete */
            if(pmmioinfo->ulFlags & MMIO_DELETE) {
#ifdef DEBUG
            writeLog("MMIO_OPEN: MMIO_DELETE set\n");
#endif
              if(remove((PSZ) lParam1)==-1)
                return (MMIO_ERROR);
            }
            /************************************************************
              Allocate our private data structure
             ************************************************************/
            if(NO_ERROR!=DosAllocMem ((PPVOID) &pJPGInfo,
                                  sizeof (JPGFILESTATUS),
                                  fALLOC))
              return  MMIO_ERROR;

            memset((PVOID)pJPGInfo,0, sizeof(JPGFILESTATUS));


            if ( gbm_init() != GBM_ERR_OK ) {
              DosFreeMem(pJPGInfo);
              return MMIO_ERROR;
            }

            /************************************************************
             * Store pointer to our JPGFILESTATUS structure in
             * pExtraInfoStruct field that is provided for our use.
             ************************************************************/
            pmmioinfo->pExtraInfoStruct = (PVOID)pJPGInfo;

            /************************************************************
              MMIO_WRITE
             ************************************************************/
            if (pmmioinfo->ulFlags & MMIO_WRITE) {
              /* It's a write so open the file */
#ifdef DEBUG
            writeLog("MMIO_OPEN: flag MMIO_WRITE set\n");
#endif

              fOpenFlags=O_WRONLY|O_BINARY;
              if(pmmioinfo->ulFlags & MMIO_CREATE) {
                fOpenFlags|=O_CREAT|O_TRUNC;
                if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
                  {
#ifdef DEBUG
                    writeLog("MMIO_OPEN: create failed, flags: %x, file %s\n",
                             fOpenFlags, (PSZ) lParam1);
                    writeLog("MMIO_OPEN: errno: 0x%x, \n",
                             errno);
#endif
                    gbm_deinit();
                    DosFreeMem(pJPGInfo);
                    return (MMIO_ERROR);
                  }
              }
              else {
                if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
                  {
#ifdef DEBUG
                    writeLog("MMIO_OPEN: \n");
                    writeLog("MMIO_OPEN: open failed, flags: %x, file %s\n",
                             fOpenFlags, (PSZ) lParam1);
                    writeLog("MMIO_OPEN: errno: 0x%x, \n",
                             errno);
#endif
                    gbm_deinit();
                    DosFreeMem(pJPGInfo);
                    return (MMIO_ERROR);
                  }
              }/* else */

              pJPGInfo->fHandleGBM=fd;
              return (MMIO_SUCCESS);
            }

            /* Since we can't write and we are here, this is a read. The read flag isn't
               necessarily set...
               */

            /*
              First get some infos from GBM.
              */
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
                DosFreeMem(pJPGInfo);
                return (MMIO_ERROR);
              }

            gbm_query_n_filetypes(&n_ft);

            for ( ft = 0; ft < n_ft; ft++ )
              {
                if ( gbm_read_header((PSZ) lParam1, fd, ft, &pJPGInfo->gbm, "") == GBM_ERR_OK )
                  {
                    gbm_query_filetype(ft, &gbmft);
                    if(!stricmp(gbmft.short_name, "JPEG")) {
                      bValidJPG=TRUE;
                      pJPGInfo->ft=ft;
#ifdef DEBUG
              writeLog("MMIO_OPEN, JPEG proc found ft: %d\n", ft);
#endif
                      break;
                    }
                  }
              }

            if(!bValidJPG) {
#ifdef DEBUG
              writeLog("MMIO_OPEN, file isn't a valid JPEG file\n");
#endif
              DosFreeMem(pJPGInfo);
              gbm_io_close(fd);
              gbm_deinit();
              return (MMIO_ERROR);
            }

            pJPGInfo->fHandleGBM=fd;

            if(pJPGInfo->gbm.bpp==8) {
              GBMRGB gbmrgb[0x100];
              RGB2 *rgb2;
              int a;

#ifdef DEBUG
              writeLog("MMIO_OPEN, reading palette for 8bpp JPG.\n");
#endif

              /* Get palette for 8Bit JPGs (grey) */
              if ( (rc = gbm_read_palette(fd, pJPGInfo->ft, &pJPGInfo->gbm, gbmrgb)) != GBM_ERR_OK )
                {
#ifdef DEBUG
              writeLog("MMIO_OPEN, can't get palette for 8bpp JPG.\n");
#endif
                  DosFreeMem(pJPGInfo);
                  gbm_io_close(fd);
                  gbm_deinit();
                  return (MMIO_ERROR);
                }
              rgb2=MMImgHdr.bmiColors;
              for(a=0;a<256;a++){
                rgb2[a].bBlue=gbmrgb[a].b;
                rgb2[a].bGreen=gbmrgb[a].g;
                rgb2[a].bRed=gbmrgb[a].r;
              }


            }
            /************************************************************
             * If the app intends to read in translation mode, we must
             * allocate and set-up the buffer that will contain the RGB data
             *
             * We must also read in the data to insure that the first
             * read, seek, or get-header operation will have data
             * to use.  This is ONLY NECESSARY FOR TRANSLATED MODE
             * operations, since we must process reads/writes pretending
             * the image is stored from the Bottom-up.
             *
             ************************************************************
             ************************************************************
             * Fill out the MMIMAGEHEADER structure.
             ************************************************************/
#if 0
               MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
               MMImgHdr.ulContentType  = MMIO_IMAGE_PHOTO;
               MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                           sizeof (BITMAPINFOHEADER2);
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pJPGInfo->gbm.w;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pJPGInfo->gbm.h;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pJPGInfo->gbm.bpp;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   =
                   BCA_UNCOMP;
               if(pJPGInfo->gbm.bpp==8) {
                   MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                     ulWidth * ulHeight;
                   MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
                   MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
               }
               else
                 MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight * 3;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     =
                   BRA_BOTTOMUP;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     =
                   BRH_NOTHALFTONED;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
#endif

               MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
               MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
               MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                           sizeof (BITMAPINFOHEADER2);
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pJPGInfo->gbm.w;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pJPGInfo->gbm.h;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pJPGInfo->gbm.bpp;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   =
                   BCA_UNCOMP;
               if(pJPGInfo->gbm.bpp==8) {
                   MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                     ulWidth * ulHeight;
                   MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
                   MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
               }
               else
                 MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight * 3;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     =
                   BRA_BOTTOMUP;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     =
                   BRH_NOTHALFTONED;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;

#ifdef DEBUG
               writeLog("MMIO_OPEN, procname is: %s\n", gbmft.short_name);
               writeLog("MMIO_OPEN: pJPGInfo->gbm.w: %d, pJPGInfo->gbm.h: %d, pJPGInfo->gbm.bpp; %d\n",
                        pJPGInfo->gbm.w, pJPGInfo->gbm.h, pJPGInfo->gbm.bpp);
               writeLog("MMIO_OPEN: pJPGInfo->fHandleGBM: %d, pJPGInfo->ft: %d\n",
                        pJPGInfo->fHandleGBM, pJPGInfo->ft);
#endif
               /********************************************************
                * Determine total bytes in image
                ********************************************************/

               stride = ( ((pJPGInfo->gbm.w * pJPGInfo->gbm.bpp + 31)/32) * 4 );
               bytes = stride * pJPGInfo->gbm.h;

               pJPGInfo->ulRGBTotalBytes = bytes;
               pJPGInfo->ulImgTotalBytes = pJPGInfo->ulRGBTotalBytes;

               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pJPGInfo->ulRGBTotalBytes;
               //               MMImgHdr.mmXDIBHeader.XDIBHeaderPrefix.ulMemSize=pJPGInfo->ulRGBTotalBytes;
               //               MMImgHdr.mmXDIBHeader.XDIBHeaderPrefix.ulPelFormat=mmioFOURCC('p','a','l','b');

               /************************************************************
                * Copy the image header into private area for later use.
                * This will be returned on a mmioGetHeader () call
                ************************************************************/
               pJPGInfo->mmImgHdr = MMImgHdr;

               return MMIO_SUCCESS;

            } /* end case of MMIOM_OPEN */

        /*#############################################################*/
        /*#############################################################*/
        case MMIOM_QUERYHEADERLENGTH:
            {
#ifdef DEBUG
            writeLog("MMIO_QUERYHEADERLENGTH\n");
#endif
            /************************************************************
             * If there is no MMIOINFO block then return an error.
             ************************************************************/
            if (!pmmioinfo)
                return (0);

            /************************************************************
             * If header is in translated mode then return the media
             * type specific structure size.
             ************************************************************/
            if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
                return (sizeof (MMIMAGEHEADER));
            else
              /********************************************************
               * Header is not in translated mode so return the size
               * of the MMotion header.
               ********************************************************/
              /* Unstranslated headers not supported */
              /*!!!!!!!!!!!!!! FIXME !!!!!!!!!!!!!!!*/
              return 0;
            } /* end case of MMIOM_QUERYHEADERLENGTH */

        /*#############################################################*/
        /*#############################################################*/
        case MMIOM_READ:
            {

            /************************************************************
             * Declare Local Variables
             ************************************************************/
            PJPGFILESTATUS   pJPGInfo;
            LONG            rc;
            LONG            lBytesToRead;
            GBM_ERR     rcGBM;

#ifdef DEBUG
            writeLog("MMIO_READ\n");
#endif

            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /************************************************************
             * Set up our working file status variable.
             ************************************************************/
            pJPGInfo = (PJPGFILESTATUS)pmmioinfo->pExtraInfoStruct;

            /************************************************************
             * Is Translate Data off?
             ************************************************************/
            if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
              {
                int rc;
                /********************************************************
                 * Since no translation, provide exact number of bytes req
                 ********************************************************/
                if (!lParam1)
                  return (MMIO_ERROR);

                /* I don't know the header size of JPEG files so I can't skip
                   to the image data in MMIO_SEEK. If I can't find the image data
                   I can't read untranslated... */
                return (MMIO_ERROR);
#if 0
                rc=gbm_io_read(pJPGInfo->fHandleGBM,(PVOID) lParam1, (ULONG) lParam2);

                if(rc==-1)
                  return (MMIO_ERROR);

                return (rc);
#endif
              }

            /************************************************************
             * Otherwise, Translate Data is on...
             ************************************************************/

            /* Check if we already have read in the image data */
            if(!pJPGInfo->lpRGBBuf) {
              /* No, so read the image now */

#ifdef DEBUG
              writeLog("MMIO_READ: reading image data.\n");
#endif
              if(readImageData(pJPGInfo)==MMIO_ERROR)
                return MMIO_ERROR;
            }


            /************************************************************
             * Ensure we do NOT write more data out than is remaining
             *    in the buffer.  The length of read was requested in
             *    image bytes, so confirm that there are that many of
             *    virtual bytes remaining.
             ************************************************************/
            if ((ULONG)(pJPGInfo->lImgBytePos + lParam2) >
                pJPGInfo->ulImgTotalBytes)
              /*  pVidInfo->ulImgTotalBytes)*/
               lBytesToRead =
                   pJPGInfo->ulImgTotalBytes - pJPGInfo->lImgBytePos;
            /*    pVidInfo->ulImgTotalBytes - pJPGInfo->lImgBytePos;*/
            else
               lBytesToRead = (ULONG)lParam2;

            /************************************************************
             * Perform this block on ALL reads.  The image data should
             * be in the RGB buffer at this point, and can be handed
             * to the application.
             *
             * Conveniently, the virtual image position is the same
             *    as the RGB buffer position, since both are 24 bit-RGB
             ************************************************************/
            memcpy ((PVOID)lParam1,
                    &(pJPGInfo->lpRGBBuf[pJPGInfo->lImgBytePos]),
                    lBytesToRead);

            /************************************************************
             * Move RGB Buffer pointer forward by number of bytes read.
             * The Img buffer pos is identical, since both are 24 bits
             ************************************************************/
            pJPGInfo->lImgBytePos += lBytesToRead;

#ifdef DEBUG
            writeLog("MMIO_READ: read %d bytes.\n", lBytesToRead);
#endif

            return (lBytesToRead);
            }   /* end case  of MMIOM_READ */

        /*#############################################################*/
        /*#############################################################*/
        case MMIOM_SEEK:
            {

            /************************************************************
             * Set up locals.
             ************************************************************/
            PJPGFILESTATUS   pJPGInfo;
            LONG            lNewFilePosition;
            LONG            lPosDesired;
            SHORT           sSeekMode;

#ifdef DEBUG
            writeLog("MMIO_SEEK\n");
#endif

            /************************************************************
             * Check to make sure MMIOINFO block is valid.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);


            /************************************************************
             * Set up our working file status variable.
             ************************************************************/
            pJPGInfo = (PJPGFILESTATUS)pmmioinfo->pExtraInfoStruct;

            lPosDesired = lParam1;
            sSeekMode = (SHORT)lParam2;

            /************************************************************
             * Is Translate Data on?
             ************************************************************/
            if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
                {
                  /* Image data already read?? */
                  if(!pJPGInfo->lpRGBBuf) {
                    /* No, so read the image now */

#ifdef DEBUG
                    writeLog("MMIO_SEEK: reading image data.\n");
#endif

                    if(readImageData(pJPGInfo)==MMIO_ERROR)
                      return MMIO_ERROR;
                  }

                  /********************************************************
                   * Attempt to move the Image buffer pointer to the
                   *    desired location.  App sends SEEK requests in
                   *    positions relative to the image planes & bits/pel
                   * We must also convert this to RGB positions
                   ********************************************************/
                  switch (sSeekMode)
                    {
                    case SEEK_SET:
                      {
                        lNewFilePosition = lPosDesired;
                        break;
                      }
                    case SEEK_CUR:
                      {
                        lNewFilePosition = pJPGInfo->lImgBytePos + lPosDesired;
                        break;
                      }
                    case SEEK_END:
                      {

                        lNewFilePosition =
                          pJPGInfo->ulImgTotalBytes += lPosDesired;
                        break;
                      }
                    default :
                      return (MMIO_ERROR);
                    }

                  /********************************************************
                   * Make sure seek did not go before start of file.
                   * If so, then don't change anything, just return an error
                   ********************************************************/
                  if (lNewFilePosition < 0)
                    {
                      return (MMIO_ERROR);
                    }

                  /********************************************************
                   * Make sure seek did not go past the end of file.
                   ********************************************************/
                  if (lNewFilePosition > (LONG)pJPGInfo->ulImgTotalBytes)
                    lNewFilePosition = pJPGInfo->ulImgTotalBytes;

                  pJPGInfo->lImgBytePos = lNewFilePosition;

                  return (pJPGInfo->lImgBytePos);
                }

            return (MMIO_ERROR);
            }  /* end case of MMIOM_SEEK */

        /*#############################################################*/
        /*#############################################################*/
        case MMIOM_SETHEADER:
            {
              /************************************************************
               * Declare local variables.
               ************************************************************/
              PMMIMAGEHEADER          pMMImgHdr;
              PJPGFILESTATUS           pJPGInfo;
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


#ifdef DEBUG
            writeLog("MMIO_SETHEADER\n");
#endif

            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /************************************************************
             * Set up our working variable MMFILESTATUS.
             ************************************************************/
            pJPGInfo = (PJPGFILESTATUS) pmmioinfo->pExtraInfoStruct;


            /************************************************************
             * Only allow this function if we are in WRITE mode
             * And only if we have not already set the header
             ************************************************************/
            if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
                (pJPGInfo->bSetHeader))
              return (0);

            /********************************************************
             * Make sure lParam1 is a valid pointer
             ********************************************************/
            if (!lParam1)
              {
                pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
                return (0);
              }

            /************************************************************
             * Header is not in translated mode.
             ************************************************************/
            if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
                {

                  /* We don't do untranslated writes */
                  return (0);   /* 0 indicates error */

                }  /* end IF NOT TRANSLATED block */

            /************************************************************
             * Header is translated.
             ************************************************************/

            /************************************************************
             * Create local pointer media specific structure.
             ************************************************************/
            pMMImgHdr = (PMMIMAGEHEADER) lParam1;

            /************************************************************
             * Check for validity of header contents supplied
             ************************************************************
             *  --  Length must be that of the standard header
             *  --  NO Compression
             *      1 plane
             *      24, 8, 4 or 1 bpp
             ************************************************************/
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

#if 0
            /************************************************************
             * Complete MMOTIONHEADER.
             ************************************************************/
            memcpy ((PVOID)&pVidInfo->mmotHeader.mmID, "YUV12C", 6);
            pVidInfo->mmotHeader.mmXorg  = 0;
            pVidInfo->mmotHeader.mmYorg  = 0;
#endif


            ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
            ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;

            /************************************************************
             * Complete GBM structure.
             ************************************************************/
            pJPGInfo->gbm.w=ulWidth;
            pJPGInfo->gbm.h=ulHeight;
            pJPGInfo->gbm.bpp=usBitCount;

            /* Discard any image buffer already allocated. But..., where should it
               come from?!?? */
            if(pJPGInfo->lpRGBBuf) {
              DosFreeMem(pJPGInfo->lpRGBBuf);
              pJPGInfo->lpRGBBuf=NULLHANDLE;
            }

            pJPGInfo->ulRGBTotalBytes = ( ((pJPGInfo->gbm.w * pJPGInfo->gbm.bpp + 31)/32) * 4 )
              * pJPGInfo->gbm.h;
            pJPGInfo->ulImgTotalBytes = pJPGInfo->ulRGBTotalBytes;

            /********************************************************
             * Get space for full image buffer.
             * This will be retained until the file is closed.
             ********************************************************/
            if (DosAllocMem ((PPVOID) &(pJPGInfo->lpRGBBuf),
                             pJPGInfo->ulRGBTotalBytes,
                             fALLOC))
              {
#ifdef DEBUG
                writeLog("MMIO_SETHEADER: no memory for image data.\n");
#endif
                return (MMIO_ERROR);
              }

            /************************************************************
             * Flag that MMIOM_SETHEADER has been done.  It can only
             *    be done ONCE for a file.  All future attempts will
             *    be flagged as errors.
             ************************************************************/
            pJPGInfo->bSetHeader = TRUE;

            /************************************************************
             * Create copy of MMIMAGEHEADER for future use.
             ************************************************************/
            pJPGInfo->mmImgHdr = *pMMImgHdr;

            /********************************************************
             * Set up initial pointer value within RGB buffer & image
             ********************************************************/
            pJPGInfo->lImgBytePos = 0;

#ifdef DEBUG
            writeLog("MMIO_SETHEADER done\n");
#endif

            return (sizeof (MMIMAGEHEADER));
            }  /* end case of MMIOM_SETHEADER */

            /*#############################################################*/
            /*#############################################################*/
        case MMIOM_WRITE:
            {

            /************************************************************
             * Declare Local Variables.
             ************************************************************/
            PJPGFILESTATUS       pJPGInfo;
            USHORT              usBitCount;
            LONG                lBytesWritten;
            ULONG               ulImgBytesToWrite;
#ifdef DEBUG
            writeLog("MMIO_WRITE\n");
#endif

            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /************************************************************
             * Set up our working variable JPGFILESTATUS.
             ************************************************************/
            pJPGInfo = (PJPGFILESTATUS) pmmioinfo->pExtraInfoStruct;

            /************************************************************
             * See if a SetHeader has been done on this file.
             ************************************************************/
            if ((!pJPGInfo) || (!pJPGInfo->bSetHeader))
              {
                return (MMIO_ERROR);
              }

            if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
              {
                /********************************************************
                 * Translation is off, take amount of bytes sent and
                 * write to the file.
                 ********************************************************
                 * Ensure that there is a data buffer to write from
                 ********************************************************/
                if (!lParam1)
                  return (MMIO_ERROR);

                /* We don't do untranslated writes. We shouldn't come here because
                   the MMIO_OPEN call must fail because of the TRANSLATE flag */
                return (MMIO_ERROR);
#if 0
                lBytesWritten = mmioWrite (pVidInfo->hmmioSS,
                                           (PVOID) lParam1,
                                           (ULONG) lParam2);

                return (lBytesWritten);
#endif
                }

            /************************************************************
             * Translation is on.
             *************************************************************/

#ifdef DEBUG
            writeLog("MMIO_WRITE: pJPGInfo->lImgBytePos %d lParam2 %d pJPGInfo->ulImgTotalBytes: %d bytes\n",
                     pJPGInfo->lImgBytePos, lParam2, pJPGInfo->ulImgTotalBytes);
#endif

            /************************************************************
             * Ensure we do not attempt to write past the end of the
             *    buffer...
             ************************************************************/
            if ((ULONG)(pJPGInfo->lImgBytePos + lParam2) >
               pJPGInfo->ulImgTotalBytes)
               ulImgBytesToWrite =
                   pJPGInfo->ulImgTotalBytes - pJPGInfo->lImgBytePos;
            else {
               ulImgBytesToWrite = (ULONG)lParam2;
            }

#ifdef DEBUG
            writeLog("MMIO_WRITE: ulImgBytesToWrite %d \n", ulImgBytesToWrite);
#endif

            /************************************************************
             * Write the data into the image buffer.  It will be converted
             *  when the file is closed.  This allows the
             *   application to seek to arbitrary  positions within the
             *   image in terms of the bits/pel, etc they are writing.
             ************************************************************/
            memcpy (&(pJPGInfo->lpRGBBuf[pJPGInfo->lImgBytePos]),
                    (PVOID)lParam1,
                    ulImgBytesToWrite);

            /* Update current position in the image buffer */
            pJPGInfo->lImgBytePos += ulImgBytesToWrite;
#ifdef DEBUG
            writeLog("MMIO_WRITE: written %d bytes\n", ulImgBytesToWrite);
#endif
            return (ulImgBytesToWrite);
            }   /* end case of MMIOM_WRITE */

        /*
         * If the IOProc has a child IOProc, then pass the message on to the Child, otherwise
         * return Unsupported Message
         */
        default:
            {
             /*
              * Declare Local Variables.
              */
              //             PMMFILESTATUS       pVidInfo;
             LONG                lRC;

             /************************************************************
              * Check for valid MMIOINFO block.
              ************************************************************/
             if (!pmmioinfo)
                 return (MMIO_ERROR);

             /* !!!!!!!!!!!! FIXME !!!!!!!!!!!!!*/
             return (MMIOERR_UNSUPPORTED_MESSAGE);
#if 0
             /************************************************************
              * Set up our working variable MMFILESTATUS.
              ************************************************************/
             pVidInfo = (PMMFILESTATUS) pmmioinfo->pExtraInfoStruct;

             if (pVidInfo != NULL && pVidInfo->hmmioSS)
                {
                 lRC = mmioSendMessage (pVidInfo->hmmioSS,
                                        usMsg,
                                        lParam1,
                                        lParam2);
                 if (!lRC)
                    pmmioinfo->ulErrorRet = mmioGetLastError (pVidInfo->hmmioSS);
                 return (lRC);
                }
            else
               {
                if (pmmioinfo != NULL)
                   pmmioinfo->ulErrorRet = MMIOERR_UNSUPPORTED_MESSAGE;
                return (MMIOERR_UNSUPPORTED_MESSAGE);
               }
#endif
            }   /* end case of Default */

        } /* end SWITCH statement for MMIO messages */

    return (0);
    }      /* end of window procedure */
