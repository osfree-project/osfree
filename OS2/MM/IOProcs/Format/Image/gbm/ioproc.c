#include "ioproc.h"


ULONG readImageData( PGBMFILESTATUS pGBMInfo)
{
  GBM_ERR rcGBM;
  /********************************************************
   * Determine total bytes in image
   ********************************************************/

  pGBMInfo->ulRGBTotalBytes =  ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
    * pGBMInfo->gbm.h;

  pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;

  /********************************************************
   * Get space for full image buffer.
   * This will be retained until the file is closed.
   ********************************************************/
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

  /********************************************************
   * RGB Buffer now full, set position pointers to the
   * beginning of the buffer.
   ********************************************************/
  pGBMInfo->lImgBytePos =  0;

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
            PGBMFILESTATUS   pGBMInfo;         /* MMotionIOProc instance data */

            LONG            lRetCode;
            USHORT          rc;
            GBMRGB  gbmrgb[0x100];

            /***********************************************************
             * Check for valid MMIOINFO block.
             ***********************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /***********************************************************
             * Set up our working file status variable.
             ***********************************************************/
            pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

            /***********************************************************
             * Assume success for the moment....
             ***********************************************************/
            lRetCode = MMIO_SUCCESS;


            /************************************************************
             * see if we are in Write mode and have a buffer to write out.
             *    We have no image buffer in UNTRANSLATED mode.
             ************************************************************/
            if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGBMInfo->lpRGBBuf))
                {
                    /* Проверяем, поддерживается ли такая глубина цвета? */
                    switch (pGBMInfo->gbm.bpp)
                    {
                      case 24: {
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

                }  /* end IF WRITE & IMAGE BUFFER block */

            /***********************************************************
             * Free the RGB buffer, if it exists, that was created
             * for the translated READ operations.
             ***********************************************************/
            if (pGBMInfo->lpRGBBuf)
              {
                DosFreeMem ((PVOID) pGBMInfo->lpRGBBuf);
              }
            /***********************************************************
             * Close the file
             ***********************************************************/
            gbm_io_close(pGBMInfo->fHandleGBM);

            DosFreeMem ((PVOID) pGBMInfo);

            return (lRetCode);
            }  /* end case of MMIOM_CLOSE */

        /*#############################################################*
         * Get the file header.
         *#############################################################*/
        case MMIOM_GETHEADER:
            {
            /************************************************************
             * Declare local variables.
             ************************************************************/
              //  PMMFILESTATUS       pVidInfo;
            PGBMFILESTATUS       pGBMInfo;

            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (0);

            /************************************************************
             * Set up our working file status variable.
             ************************************************************/
            pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

            /**************************************************
             * Getheader only valid in READ or READ/WRITE mode.
             * There is no header to get in WRITE mode.  We
             * must also have a valid file handle to read from
             **************************************************/
            if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
                (!(pGBMInfo->fHandleGBM)))
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
                    (PVOID)&pGBMInfo->mmImgHdr,
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
            BOOL bValidGBM= FALSE;

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
                    if(!stricmp(gbmft.short_name, "GBM"))
                      bValidGBM=TRUE;
                  }
              }

            gbm_io_close(fd);
            gbm_deinit();

            if(bValidGBM) {
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


            PGBMFILESTATUS   pGBMInfo;   /* pointer to a GBM file       */
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
            BOOL bValidGBM= FALSE;
            int fOpenFlags;
            GBM_ERR     rc;

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
                return (MMIO_ERROR);
                }

            /* We can't read/write untranslated */
            if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
                   !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
              {
                return (MMIO_ERROR);
              }


            /*!!!!!!!!!!!!!!!!!!!!!! FIXME !!!!!!!!!!!!!!!!!!!!!!!*/
            /* To be honest we can almost nothing ;-) */
            if (pmmioinfo->ulFlags &
                (MMIO_APPEND|
                 MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
              {
//                return (MMIO_ERROR);
              }

            /* Caller requested a delete */
            if(pmmioinfo->ulFlags & MMIO_DELETE) {
              if(remove((PSZ) lParam1)==-1)
                return (MMIO_ERROR);
            }
            /************************************************************
              Allocate our private data structure
             ************************************************************/
            if(NO_ERROR!=DosAllocMem ((PPVOID) &pGBMInfo,
                                  sizeof (GBMFILESTATUS),
                                  fALLOC))
              return  MMIO_ERROR;

            memset((PVOID)pGBMInfo,0, sizeof(GBMFILESTATUS));


            if ( gbm_init() != GBM_ERR_OK ) {
              DosFreeMem(pGBMInfo);
              return MMIO_ERROR;
            }

            /************************************************************
             * Store pointer to our GBMFILESTATUS structure in
             * pExtraInfoStruct field that is provided for our use.
             ************************************************************/
            pmmioinfo->pExtraInfoStruct = (PVOID)pGBMInfo;

            /************************************************************
              MMIO_WRITE
             ************************************************************/
            if (pmmioinfo->ulFlags & MMIO_WRITE) {
              /* It's a write so open the file */

              fOpenFlags=O_WRONLY|O_BINARY;
              if(pmmioinfo->ulFlags & MMIO_CREATE) {
                fOpenFlags|=O_CREAT|O_TRUNC;
                if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
                  {
                    gbm_deinit();
                    DosFreeMem(pGBMInfo);
                    return (MMIO_ERROR);
                  }
              }
              else {
                if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
                  {
                    gbm_deinit();
                    DosFreeMem(pGBMInfo);
                    return (MMIO_ERROR);
                  }
              }/* else */

              pGBMInfo->fHandleGBM=fd;
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
                DosFreeMem(pGBMInfo);
                return (MMIO_ERROR);
              }

            gbm_query_n_filetypes(&n_ft);

            for ( ft = 0; ft < n_ft; ft++ )
              {
                if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGBMInfo->gbm, "") == GBM_ERR_OK )
                  {
                    gbm_query_filetype(ft, &gbmft);
                    if(!stricmp(gbmft.short_name, "GBM")) {
                      bValidGBM=TRUE;
                      pGBMInfo->ft=ft;
                      break;
                    }
                  }
              }

            if(!bValidGBM) {
              DosFreeMem(pGBMInfo);
              gbm_io_close(fd);
              gbm_deinit();
              return (MMIO_ERROR);
            }

            pGBMInfo->fHandleGBM=fd;

            if(pGBMInfo->gbm.bpp==8) {
              GBMRGB gbmrgb[0x100];
              RGB2 *rgb2;
              int a;


              /* Get palette for 8Bit GBMs (grey) */
              if ( (rc = gbm_read_palette(fd, pGBMInfo->ft, &pGBMInfo->gbm, gbmrgb)) != GBM_ERR_OK )
                {
                  DosFreeMem(pGBMInfo);
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
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   =
                   BCA_UNCOMP;
               if(pGBMInfo->gbm.bpp==8) {
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
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGBMInfo->gbm.w;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGBMInfo->gbm.h;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGBMInfo->gbm.bpp;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   =
                   BCA_UNCOMP;
               if(pGBMInfo->gbm.bpp==8) {
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

               /********************************************************
                * Determine total bytes in image
                ********************************************************/

               stride = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 );
               bytes = stride * pGBMInfo->gbm.h;

               pGBMInfo->ulRGBTotalBytes = bytes;
               pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;

               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGBMInfo->ulRGBTotalBytes;
               //               MMImgHdr.mmXDIBHeader.XDIBHeaderPrefix.ulMemSize=pGBMInfo->ulRGBTotalBytes;
               //               MMImgHdr.mmXDIBHeader.XDIBHeaderPrefix.ulPelFormat=mmioFOURCC('p','a','l','b');

               /************************************************************
                * Copy the image header into private area for later use.
                * This will be returned on a mmioGetHeader () call
                ************************************************************/
               pGBMInfo->mmImgHdr = MMImgHdr;

               return MMIO_SUCCESS;

            } /* end case of MMIOM_OPEN */

        /*#############################################################*/
        /*#############################################################*/
        case MMIOM_QUERYHEADERLENGTH:
            {
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
            PGBMFILESTATUS   pGBMInfo;
            LONG            rc;
            LONG            lBytesToRead;
            GBM_ERR     rcGBM;


            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /************************************************************
             * Set up our working file status variable.
             ************************************************************/
            pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

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

                /* I don't know the header size of GBM files so I can't skip
                   to the image data in MMIO_SEEK. If I can't find the image data
                   I can't read untranslated... */
                return (MMIO_ERROR);
              }

            /************************************************************
             * Otherwise, Translate Data is on...
             ************************************************************/

            /* Check if we already have read in the image data */
            if(!pGBMInfo->lpRGBBuf) {
              /* No, so read the image now */
              if(readImageData(pGBMInfo)==MMIO_ERROR)
                return MMIO_ERROR;
            }


            /************************************************************
             * Ensure we do NOT write more data out than is remaining
             *    in the buffer.  The length of read was requested in
             *    image bytes, so confirm that there are that many of
             *    virtual bytes remaining.
             ************************************************************/
            if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
                pGBMInfo->ulImgTotalBytes)
              /*  pVidInfo->ulImgTotalBytes)*/
               lBytesToRead =
                   pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
            /*    pVidInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;*/
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
                    &(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                    lBytesToRead);

            /************************************************************
             * Move RGB Buffer pointer forward by number of bytes read.
             * The Img buffer pos is identical, since both are 24 bits
             ************************************************************/
            pGBMInfo->lImgBytePos += lBytesToRead;

            return (lBytesToRead);
            }   /* end case  of MMIOM_READ */

        /*#############################################################*/
        /*#############################################################*/
        case MMIOM_SEEK:
            {

            /************************************************************
             * Set up locals.
             ************************************************************/
            PGBMFILESTATUS   pGBMInfo;
            LONG            lNewFilePosition;
            LONG            lPosDesired;
            SHORT           sSeekMode;

            /************************************************************
             * Check to make sure MMIOINFO block is valid.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);


            /************************************************************
             * Set up our working file status variable.
             ************************************************************/
            pGBMInfo = (PGBMFILESTATUS)pmmioinfo->pExtraInfoStruct;

            lPosDesired = lParam1;
            sSeekMode = (SHORT)lParam2;

            /************************************************************
             * Is Translate Data on?
             ************************************************************/
            if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
                {
                  /* Image data already read?? */
                  if(!pGBMInfo->lpRGBBuf) {
                    /* No, so read the image now */

                    if(readImageData(pGBMInfo)==MMIO_ERROR)
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
                        lNewFilePosition = pGBMInfo->lImgBytePos + lPosDesired;
                        break;
                      }
                    case SEEK_END:
                      {

                        lNewFilePosition =
                          pGBMInfo->ulImgTotalBytes += lPosDesired;
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
                  if (lNewFilePosition > (LONG)pGBMInfo->ulImgTotalBytes)
                    lNewFilePosition = pGBMInfo->ulImgTotalBytes;

                  pGBMInfo->lImgBytePos = lNewFilePosition;

                  return (pGBMInfo->lImgBytePos);
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
              PGBMFILESTATUS           pGBMInfo;
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


            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /************************************************************
             * Set up our working variable MMFILESTATUS.
             ************************************************************/
            pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;


            /************************************************************
             * Only allow this function if we are in WRITE mode
             * And only if we have not already set the header
             ************************************************************/
            if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
                (pGBMInfo->bSetHeader))
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

            ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
            ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;

            /************************************************************
             * Complete GBM structure.
             ************************************************************/
            pGBMInfo->gbm.w=ulWidth;
            pGBMInfo->gbm.h=ulHeight;
            pGBMInfo->gbm.bpp=usBitCount;

            /* Discard any image buffer already allocated. But..., where should it
               come from?!?? */
            if(pGBMInfo->lpRGBBuf) {
              DosFreeMem(pGBMInfo->lpRGBBuf);
              pGBMInfo->lpRGBBuf=NULLHANDLE;
            }

            pGBMInfo->ulRGBTotalBytes = ( ((pGBMInfo->gbm.w * pGBMInfo->gbm.bpp + 31)/32) * 4 )
              * pGBMInfo->gbm.h;
            pGBMInfo->ulImgTotalBytes = pGBMInfo->ulRGBTotalBytes;

            /********************************************************
             * Get space for full image buffer.
             * This will be retained until the file is closed.
             ********************************************************/
            if (DosAllocMem ((PPVOID) &(pGBMInfo->lpRGBBuf),
                             pGBMInfo->ulRGBTotalBytes,
                             fALLOC))
              {
                return (MMIO_ERROR);
              }

            /************************************************************
             * Flag that MMIOM_SETHEADER has been done.  It can only
             *    be done ONCE for a file.  All future attempts will
             *    be flagged as errors.
             ************************************************************/
            pGBMInfo->bSetHeader = TRUE;

            /************************************************************
             * Create copy of MMIMAGEHEADER for future use.
             ************************************************************/
            pGBMInfo->mmImgHdr = *pMMImgHdr;

            /********************************************************
             * Set up initial pointer value within RGB buffer & image
             ********************************************************/
            pGBMInfo->lImgBytePos = 0;

            return (sizeof (MMIMAGEHEADER));
            }  /* end case of MMIOM_SETHEADER */

            /*#############################################################*/
            /*#############################################################*/
        case MMIOM_WRITE:
            {

            /************************************************************
             * Declare Local Variables.
             ************************************************************/
            PGBMFILESTATUS       pGBMInfo;
            USHORT              usBitCount;
            LONG                lBytesWritten;
            ULONG               ulImgBytesToWrite;

            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /************************************************************
             * Set up our working variable GBMFILESTATUS.
             ************************************************************/
            pGBMInfo = (PGBMFILESTATUS) pmmioinfo->pExtraInfoStruct;

            /************************************************************
             * See if a SetHeader has been done on this file.
             ************************************************************/
            if ((!pGBMInfo) || (!pGBMInfo->bSetHeader))
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

            /************************************************************
             * Ensure we do not attempt to write past the end of the
             *    buffer...
             ************************************************************/
            if ((ULONG)(pGBMInfo->lImgBytePos + lParam2) >
               pGBMInfo->ulImgTotalBytes)
               ulImgBytesToWrite =
                   pGBMInfo->ulImgTotalBytes - pGBMInfo->lImgBytePos;
            else {
               ulImgBytesToWrite = (ULONG)lParam2;
            }

            /************************************************************
             * Write the data into the image buffer.  It will be converted
             *  when the file is closed.  This allows the
             *   application to seek to arbitrary  positions within the
             *   image in terms of the bits/pel, etc they are writing.
             ************************************************************/
            memcpy (&(pGBMInfo->lpRGBBuf[pGBMInfo->lImgBytePos]),
                    (PVOID)lParam1,
                    ulImgBytesToWrite);

            /* Update current position in the image buffer */
            pGBMInfo->lImgBytePos += ulImgBytesToWrite;
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
