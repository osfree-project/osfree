/*

gbmdlg.c - File Open / File Save as dialogs for GBM
           (with bitmap preview)

*/

#define    INCL_BASE
#define    INCL_DOS
#define    INCL_WIN
#define    INCL_GPI
#define    INCL_DEV
#define    INCL_BITMAPFILEFORMAT
#include <os2.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <fcntl.h>

#include "gbm.h"
#include "gbmscale.h"
#define    _GBMDLG_
#include "gbmdlg.h"
#include "gbmdlgrc.h"


static CHAR szAllSupportedFiles [] = "<Alle untersttzten Formate>";
static CHAR szSaveAs []            = "Sichern unter...";
static CHAR szOpen   []            = "™ffnen...";


/* bitmap header incl. color table */
/* for 8 bit and lower images */
#pragma pack(2)
typedef struct
{
   BITMAPINFOHEADER2 bmp2;
   RGB2 argb2Color[0x100];
} BMP8HEADER;

/* for above 8 bit images */
typedef struct
{
   BITMAPINFOHEADER2 bmp2;
} BMP24HEADER;
#pragma pack()

/********************************/

static void UpdateBitmapPreview(const HWND dlgHwnd, const char * fileName, const char * options);
static BOOL CreateScaledSystemBitmap(const HAB   hab,
                                     const int   fd     , const int ft,
                                     const GBM * gbm_src, const int dst_w, const int dst_h,
                                     HBITMAP   * bmpHandle);
static BOOL SupportsNumberOfPagesQuery(void);
static BOOL GetNumberOfPages(const char * fileName, const int fd, const int ft, int * numPages);
static BOOL Expand1To24bit(GBM *gbm, const GBMRGB *gbmrgb, byte **data);

/********************************/

MRESULT _System GbmDefFileDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   #define L_FN  500

   switch ( (int) msg )
   {
      case WM_INITDLG:
      {
         MRESULT mr = WinDefFileDlgProc(hwnd, msg, mp1, mp2);
         GBMFILEDLG *pgbmfild = (GBMFILEDLG *) WinQueryWindowULong(hwnd, QWL_USER);
         FILEDLG *pfild = &(pgbmfild -> fild);
         SHORT sInx = SHORT1FROMMR(WinSendDlgItemMsg(hwnd, DID_FILTER_CB, LM_SEARCHSTRING, MPFROM2SHORT(LSS_CASESENSITIVE, 0), MPFROMP(szAllSupportedFiles)));
         WinSendDlgItemMsg(hwnd, DID_FILTER_CB, LM_SELECTITEM, MPFROMSHORT(sInx), MPFROMSHORT(TRUE));

         WinSendDlgItemMsg(hwnd, DID_GBM_OPTIONS_ED, EM_SETTEXTLIMIT, MPFROMSHORT(L_GBM_OPTIONS), NULL);
         WinSetDlgItemText(hwnd, DID_GBM_OPTIONS_ED, pgbmfild -> szOptions);
         /* Init query changed message for options entry field (focus change handling) */
         WinSendDlgItemMsg(hwnd, DID_GBM_OPTIONS_ED, EM_QUERYCHANGED, (MPARAM)0, (MPARAM)0);

         WinSetDlgItemText(hwnd, DID_PREVIEW_FORMAT_ED, "");
         WinSetDlgItemText(hwnd, DID_PREVIEW_WIDTH_ED , "");
         WinSetDlgItemText(hwnd, DID_PREVIEW_HEIGHT_ED, "");
         WinSetDlgItemText(hwnd, DID_PREVIEW_DEPTH_ED , "");
         WinSetDlgItemText(hwnd, DID_PREVIEW_PAGES_ED , "");

         WinSendDlgItemMsg(hwnd, DID_PREVIEW_RECT, VM_SETITEM,
                           MPFROM2SHORT(1,1), MPFROMLONG(NULLHANDLE));

         /* hide pages text if this cannot be provided due to old GBM version */
         if (! SupportsNumberOfPagesQuery())
         {
            WinSetDlgItemText(hwnd, DID_PREVIEW_PAGES_TXT , "");
         }

         if ( pfild -> pszTitle != NULL )
            WinSetWindowText(hwnd, pfild -> pszTitle);
         else if ( pfild -> fl & FDS_OPEN_DIALOG )
            WinSetWindowText(hwnd, szOpen);
         else if ( pfild -> fl & FDS_SAVEAS_DIALOG )
            WinSetWindowText(hwnd, szSaveAs);

         return ( mr );
      }

      case WM_COMMAND:
         switch (SHORT1FROMMP(mp1))
         {
            case DID_OK:
            {
               MRESULT mr;
               GBMFILEDLG *pgbmfild = (GBMFILEDLG *) WinQueryWindowULong(hwnd, QWL_USER);
               WinQueryDlgItemText(hwnd, DID_GBM_OPTIONS_ED, L_GBM_OPTIONS, pgbmfild->szOptions);

               mr = WinDefFileDlgProc(hwnd, msg, mp1, mp2);

               /* delete bitmap set */
               {
                  HBITMAP oldBitmap = (HBITMAP) WinSendDlgItemMsg(hwnd, DID_PREVIEW_RECT, VM_QUERYITEM,
                                                                  MPFROM2SHORT(1,1), MPFROMLONG(NULL));
                  if (oldBitmap)
                  {
                     WinSendDlgItemMsg(hwnd, DID_PREVIEW_RECT, VM_SETITEM,
                                       MPFROM2SHORT(1,1), MPFROMLONG(NULLHANDLE));
                     GpiDeleteBitmap(oldBitmap);
                     oldBitmap = NULLHANDLE;
                  }
               }
               return ( mr );
            }

            case DID_CANCEL:
            {
               MRESULT mr;
               GBMFILEDLG *pgbmfild = (GBMFILEDLG *) WinQueryWindowULong(hwnd, QWL_USER);
               WinQueryDlgItemText(hwnd, DID_GBM_OPTIONS_ED, L_GBM_OPTIONS, pgbmfild->szOptions);

               mr = WinDefFileDlgProc(hwnd, msg, mp1, mp2);

               /* delete bitmap set */
               {
                  HBITMAP oldBitmap = (HBITMAP) WinSendDlgItemMsg(hwnd, DID_PREVIEW_RECT, VM_QUERYITEM,
                                                                  MPFROM2SHORT(1,1), MPFROMLONG(NULL));
                  if (oldBitmap)
                  {
                     WinSendDlgItemMsg(hwnd, DID_PREVIEW_RECT, VM_SETITEM,
                                       MPFROM2SHORT(1,1), MPFROMLONG(NULLHANDLE));
                     GpiDeleteBitmap(oldBitmap);
                     oldBitmap = NULLHANDLE;
                  }
               }
               return ( mr );
            }
         }
         break;

      case WM_FOCUSCHANGE: /* update preview when options have been changed */
      {
        const HWND hwndSender = HWNDFROMMP(mp1);
        const HWND hwndField  = WinWindowFromID(hwnd, DID_GBM_OPTIONS_ED);

        if (hwndSender == hwndField)
        {
           const MRESULT mr = WinDefFileDlgProc(hwnd, msg, mp1, mp2);

           /* update preview if window changed looses focus and options field has changed */
           if (LONGFROMMR(WinSendMsg(hwndField, EM_QUERYCHANGED, (MPARAM)0, (MPARAM)0)))
           {
              const HWND hwndListbox = WinWindowFromID(hwnd, DID_FILES_LB);
              const LONG index = LONGFROMMR(WinSendMsg(hwndListbox, LM_QUERYSELECTION,
                                                                    MPFROMLONG(LIT_FIRST),
                                                                    (MPARAM)0));
              if (index >= 0)
              {
                 /* deselecting and reselecting the file entry triggers preview update */
                 WinSendMsg(hwndListbox, LM_SELECTITEM, MPFROMSHORT(index), MPFROMSHORT(FALSE));
                 WinSendMsg(hwndListbox, LM_SELECTITEM, MPFROMSHORT(index), MPFROMSHORT(TRUE));
              }
           }
           return ( mr );
        }
      }
      break;

      case WM_CONTROL:
        switch (SHORT1FROMMP(mp1))
        {
           case DID_FILES_LB:
              if (SHORT2FROMMP(mp1) == LN_SELECT)
              {
                 const MRESULT mr = WinDefFileDlgProc(hwnd, msg, mp1, mp2);

                 const HWND lbHwnd = HWNDFROMMP(mp2);
                 const LONG index  = WinQueryLboxSelectedItem(lbHwnd);

                 char itemText[L_FN+1] = { 0 };

                 WinSetDlgItemText(hwnd, DID_PREVIEW_FORMAT_ED, "");
                 WinSetDlgItemText(hwnd, DID_PREVIEW_WIDTH_ED , "");
                 WinSetDlgItemText(hwnd, DID_PREVIEW_HEIGHT_ED, "");
                 WinSetDlgItemText(hwnd, DID_PREVIEW_DEPTH_ED , "");
                 WinSetDlgItemText(hwnd, DID_PREVIEW_PAGES_ED , "");

                 /* delete bitmap set */
                 {
                    HBITMAP oldBitmap = (HBITMAP) WinSendDlgItemMsg(hwnd, DID_PREVIEW_RECT, VM_QUERYITEM,
                                                                    MPFROM2SHORT(1,1), MPFROMLONG(NULL));
                    if (oldBitmap)
                    {
                       WinSendDlgItemMsg(hwnd, DID_PREVIEW_RECT, VM_SETITEM,
                                         MPFROM2SHORT(1,1), MPFROMLONG(NULLHANDLE));
                       GpiDeleteBitmap(oldBitmap);
                       oldBitmap = NULLHANDLE;
                    }
                 }

                 if (index >= 0)
                 {
                    int ft;
                    GBMFILEDLG *pgbmfild = (GBMFILEDLG *) WinQueryWindowULong(hwnd, QWL_USER);

                    WinQueryDlgItemText(hwnd, DID_GBM_OPTIONS_ED, L_GBM_OPTIONS, pgbmfild->szOptions);
                    WinQueryLboxItemText(lbHwnd, (SHORT) index, itemText, L_FN);
                    WinSetDlgItemText(hwnd, DID_FILENAME_ED, itemText);

                    /* check file type */
                    if (gbm_guess_filetype(itemText, &ft) == GBM_ERR_OK)
                    {
                       char options[L_GBM_OPTIONS+1] = { 0 };

                       WinQueryDlgItemText(hwnd, DID_GBM_OPTIONS_ED, L_GBM_OPTIONS, options);
                       UpdateBitmapPreview(hwnd, itemText, options);
                    }
                 }
                 return mr;
              }
              break;
        }
        break;


      case WM_HELP:
      {
         /* Parent is HWND_DESKTOP */
         /* WinDefDlgProc() will pass this up to the parent */
         /* So redirect to the owner */
         /* PM Bug: (USHORT) SHORT1FROMMP(mp1) is not usCmd as it should be */
         /* So fix it up to always be the same for this dialog */

         return ( WinSendMsg(WinQueryWindow(hwnd, QW_OWNER), msg, MPFROMSHORT(DID_GBM_FILEDLG), mp2) );
      }

      /*
       * To be listed, the file must match the user specified filename (if present)
       * and the filetype specification.
       */
      case FDM_FILTER:
      {
         CHAR szFn [L_FN+1], szFnOut [L_FN+1];
         WinQueryDlgItemText(hwnd, DID_FILENAME_ED, sizeof(szFn), szFn);

         /* user has specified a filter himself */
         if ( strlen(szFn) != 0 )
            if ( DosEditName(1, (PCH) mp1, szFn, szFnOut, sizeof(szFnOut)) == 0 )
               if ( stricmp(szFn, szFnOut) && stricmp((char *) mp1, szFnOut) )
                  return ( (MRESULT) FALSE );

         /* filter based on file type */
         {
            HWND hwndFt = WinWindowFromID(hwnd, DID_FILTER_CB);
            CHAR szFt [100+1];
            int ft, n_ft, guess_ft;
            SHORT sInx;

            if ( (sInx = SHORT1FROMMR(WinSendMsg(hwndFt, LM_QUERYSELECTION, NULL, NULL))) != -1 )
               WinSendMsg(hwndFt, LM_QUERYITEMTEXT, MPFROM2SHORT(sInx, sizeof(szFt)), MPFROMP(szFt));
            else
               WinQueryWindowText(hwndFt, sizeof(szFt), szFt);

            /* Look up type name in GBM supported file types */
            gbm_query_n_filetypes(&n_ft);
            for ( ft = 0; ft < n_ft; ft++ )
            {
               GBMFT gbmft;

               gbm_query_filetype(ft, &gbmft);
               if ( !strcmp(szFt, gbmft.long_name) )
                  break;
            }

            if ( ft < n_ft )
            {
               /* Must not be <All Files> or <All GBM supported files> */
               if ( gbm_guess_filetype((char *) mp1, &guess_ft) != GBM_ERR_OK ||
                    guess_ft != ft )
                  return ( (MRESULT) FALSE );
            }
            else if ( !strcmp(szFt, szAllSupportedFiles) )
            {
               if ( gbm_guess_filetype((char *) mp1, &guess_ft) != GBM_ERR_OK ||
                    guess_ft == -1 )
                  return ( (MRESULT) FALSE );
            }
         }

         return ( (MRESULT) TRUE );
      }
   }
   return ( WinDefFileDlgProc(hwnd, msg, mp1, mp2) );
}

/********************************/

HWND _System GbmFileDlg(HWND hwndP, HWND hwndO, GBMFILEDLG *pgbmfild)
{
   FILEDLG *pfild = &(pgbmfild -> fild);
   HMODULE hmod;
   int ft, n_ft;
   CHAR **apsz;
   HWND hwndRet;

   DosQueryModuleHandle("GBMDLG", &hmod);

   pfild -> fl |= FDS_CUSTOM;

   if ( pfild -> pfnDlgProc == (PFNWP) NULL )
      pfild -> pfnDlgProc = GbmDefFileDlgProc;

   if ( pfild -> hMod == (HMODULE) NULL )
   {
      pfild -> hMod    = hmod;
      pfild -> usDlgId = RID_GBM_FILEDLG;
   }

   gbm_query_n_filetypes(&n_ft);

   if ( (apsz = malloc((n_ft + 2) * sizeof(CHAR *))) == NULL )
   {
      return ( (HWND) NULL );
   }

   for ( ft = 0; ft < n_ft; ft++ )
   {
      GBMFT gbmft;

      gbm_query_filetype(ft, &gbmft);
      apsz [ft] = gbmft.long_name;
   }
   apsz [n_ft++] = szAllSupportedFiles;
   apsz [n_ft  ] = NULL;

   pfild -> papszITypeList = (PAPSZ) apsz;

   hwndRet = WinFileDlg(hwndP, hwndO, (FILEDLG *) pgbmfild);

   free(apsz);

   return ( hwndRet );
}

/********************************/

static void UpdateBitmapPreview(const HWND dlgHwnd, const char * fileName, const char * options)
{
   int ft;

   /* check file type */
   if (gbm_guess_filetype(fileName, &ft) == GBM_ERR_OK)
   {
      int   fd;
      GBMFT gbmft;
      GBM   gbm;
      char  fullFileName[2*CCHMAXPATH+1] = "";
      char  buffer[50] = "";

      const GBMFILEDLG *pgbmfild = (GBMFILEDLG *) WinQueryWindowULong(dlgHwnd, QWL_USER);
      const FILEDLG    *pfild    = &(pgbmfild -> fild);
      const CHAR       *pPath    = pfild->szFullFile;

      /* create fully qualified filename */
      strcpy(fullFileName, pPath);
      strcat(fullFileName, fileName);

      gbm_query_filetype(ft, &gbmft);

      /* get bitmap header */
      fd = gbm_io_open(fullFileName, GBM_O_RDONLY);

      if (fd != -1)
      {
         int numPages = 1;
         char ext_options[L_GBM_OPTIONS+10+1] = { 0 };

         if (GetNumberOfPages(fileName, fd, ft, &numPages))
         {
            sprintf(buffer, "%d", numPages);
            WinSetDlgItemText(dlgHwnd, DID_PREVIEW_PAGES_ED, buffer);
         }

         sprintf(ext_options, "ext_bpp %s", options);
         if (gbm_read_header(fullFileName, fd, ft, &gbm, ext_options) == GBM_ERR_OK)
         {
            HBITMAP bmpHandle = NULLHANDLE;
            MRESULT mr;
            USHORT  rect_w  = 0;
            USHORT  rect_h  = 0;
            const int ext_bpp = gbm.bpp;

            WinSetDlgItemText(dlgHwnd, DID_PREVIEW_FORMAT_ED, gbmft.short_name);

            sprintf(buffer, "%d", gbm.w);
            WinSetDlgItemText(dlgHwnd, DID_PREVIEW_WIDTH_ED, buffer);

            sprintf(buffer, "%d", gbm.h);
            WinSetDlgItemText(dlgHwnd, DID_PREVIEW_HEIGHT_ED, buffer);

            /* We asked for extended color depth for info field, but OS/2 only supports up to 24 bpp.
             * So we must reopen the file and use the backward compatible mode which automatically
             * scales down to 24 bpp.
             */
            if (ext_bpp > 24)
            {
               if (gbm_read_header(fullFileName, fd, ft, &gbm, options) == GBM_ERR_OK)
               {
                  /* if user entered "ext_bpp" */
                  if (gbm.bpp > 24)
                  {
                     const char * extbpp = "ext_bpp";
                     char * bufptr;
                     strcpy(ext_options, options);

                     /* strip "ext_bpp" if specified by the user, so that the preview can still be shown */
                     bufptr = strstr(ext_options, extbpp);
                     while((bufptr != NULL) && (bufptr != extbpp))
                     {
                        /* override "ext_bpp" with white spaces */
                        memset(bufptr, ' ', 7);
                        bufptr = strstr(ext_options, extbpp);
                     }

                     if (gbm_read_header(fullFileName, fd, ft, &gbm, ext_options) != GBM_ERR_OK)
                     {
                        gbm_io_close(fd);
                        WinSetDlgItemText(dlgHwnd, DID_PREVIEW_PAGES_ED, "");
                        return;
                     }

                     sprintf(buffer, "%d", ext_bpp);
                     WinSetDlgItemText(dlgHwnd, DID_PREVIEW_DEPTH_ED, buffer);
                  }
                  else
                  {
                     sprintf(buffer, "%d (%d)", gbm.bpp, ext_bpp);
                     WinSetDlgItemText(dlgHwnd, DID_PREVIEW_DEPTH_ED, buffer);
                  }
               }
               else
               {
                  gbm_io_close(fd);
                  WinSetDlgItemText(dlgHwnd, DID_PREVIEW_PAGES_ED, "");
                  return;
               }
            }
            else
            {
               sprintf(buffer, "%d", gbm.bpp);
               WinSetDlgItemText(dlgHwnd, DID_PREVIEW_DEPTH_ED, buffer);
            }

            /* load bitmap for preview and scale down */

            /* set to preview */
            mr = WinSendDlgItemMsg(dlgHwnd, DID_PREVIEW_RECT,
                                   VM_QUERYMETRICS, (MPARAM) VMA_ITEMSIZE, (MPARAM) 0);

            rect_w = SHORT1FROMMR(mr);
            rect_h = SHORT2FROMMR(mr);

            /* create system bitmap */
            {
               int dst_w = gbm.w;
               int dst_h = gbm.h;

               if (rect_h < gbm.h)
               {
                  dst_w = (gbm.w * rect_h) / gbm.h;
                  dst_h = rect_h;
               }

               /* check if we need to scale down based on the width */
               if (dst_w > rect_w)
               {
                  dst_w = rect_w;
                  dst_h = (gbm.h * rect_w) / gbm.w;
               }

               if ((dst_w > 0 && dst_w <= rect_w) && (dst_h > 0 && dst_h <= rect_h))
               {
                  if (CreateScaledSystemBitmap(WinQueryAnchorBlock(dlgHwnd),
                                               fd, ft, &gbm,
                                               dst_w, dst_h,
                                               &bmpHandle))
                  {
                      /* set to preview */
                      WinSendDlgItemMsg(dlgHwnd, DID_PREVIEW_RECT, VM_SETITEM,
                                        MPFROM2SHORT(1,1), MPFROMLONG(bmpHandle));
                  }
               }
            }
         }
         else
         {
            WinSetDlgItemText(dlgHwnd, DID_PREVIEW_PAGES_ED, "");
         }

         gbm_io_close(fd);
      }
   }
}

/********************************/

static BOOL CreateScaledSystemBitmap(const HAB   hab,
                                     const int   fd     , const int ft,
                                     const GBM * gbm_src, const int dst_w, const int dst_h,
                                     HBITMAP   * bmpHandle)
{
   GBM gbm = *gbm_src;

   const int dataDstLength = (((dst_w  * gbm.bpp + 31) / 32) * 4) * dst_h;
   byte * dataDst = NULL;

   BMP8HEADER  header8BMP;
   BMP24HEADER header24BMP;

   /* init returned handle */
   *bmpHandle = NULLHANDLE;

   /* read bitmap data */
   {
      const int dataSrcLength = (((gbm.w * gbm.bpp + 31) / 32) * 4) * gbm.h;

      byte * dataSrc = (byte *) malloc(dataSrcLength);
      if (dataSrc == NULL)
      {
         return FALSE;
      }

      if (gbm_read_data(fd, ft, &gbm, dataSrc) != GBM_ERR_OK)
      {
         free(dataSrc);
         return FALSE;
      }

      /* scale down to target size */
      dataDst = (byte *) malloc(dataDstLength);
      if (dataDst == NULL)
      {
         free(dataSrc);
         return FALSE;
      }

      if (gbm_simple_scale(dataSrc, gbm.w, gbm.h,
                           dataDst, dst_w , dst_h,
                           gbm.bpp) != GBM_ERR_OK)
      {
         free(dataSrc);
         free(dataDst);
         return FALSE;
      }

      free(dataSrc);
      gbm.w = dst_w;
      gbm.h = dst_h;
   }

   /* create bitmap header */
   if (gbm.bpp <= 8)
   {
      int    i;
      GBMRGB gbmrgb[0x100];

      /* read color palette */
      if (gbm_read_palette(fd, ft, &gbm, gbmrgb) != GBM_ERR_OK)
      {
         return FALSE;
      }
      gbm.w = dst_w;
      gbm.h = dst_h;

      /* expand 1bpp to true color bitmap */
      if (gbm.bpp == 1)
      {
        const int dataLength = (((gbm.w  * 24 + 31) / 32) * 4) * gbm.h;

        if (! Expand1To24bit(&gbm, gbmrgb, &dataDst))
        {
          free(dataDst);
          return FALSE;
        }

        memset( &header24BMP, 0, sizeof(BMP24HEADER));
        header24BMP.bmp2.cbFix         = sizeof(BITMAPINFOHEADER2);
        header24BMP.bmp2.cx            = gbm.w;
        header24BMP.bmp2.cy            = gbm.h;
        header24BMP.bmp2.cPlanes       = 1;          /*  Number of bit planes. */
        header24BMP.bmp2.cBitCount     = 24;         /*  Number of bits per pel within a plane. */
        header24BMP.bmp2.ulCompression = BCA_UNCOMP; /*  Compression scheme used to store the bit map. */
        header24BMP.bmp2.cbImage       = dataLength; /*  Length of bit-map storage data, in bytes. */
      }
      else
      {
        memset( &header8BMP, 0, sizeof(BMP8HEADER));
        header8BMP.bmp2.cbFix         = sizeof(BITMAPINFOHEADER2);
        header8BMP.bmp2.cx            = gbm.w;
        header8BMP.bmp2.cy            = gbm.h;
        header8BMP.bmp2.cPlanes       = 1;                /*  Number of bit planes. */
        header8BMP.bmp2.cBitCount     = gbm.bpp;          /*  Number of bits per pel within a plane. */
        header8BMP.bmp2.ulCompression = BCA_UNCOMP;       /*  Compression scheme used to store the bit map. */
        header8BMP.bmp2.cbImage       = dataDstLength;    /*  Length of bit-map storage data, in bytes. */

        /* copy color table */
        for (i = 0; i < 256; i++)
        {
            header8BMP.argb2Color[i].bBlue     = gbmrgb[i].b;
            header8BMP.argb2Color[i].bRed      = gbmrgb[i].r;
            header8BMP.argb2Color[i].bGreen    = gbmrgb[i].g;
            header8BMP.argb2Color[i].fcOptions = 0;
        }
      }
   }
   else
   {
      memset( &header24BMP, 0, sizeof(BMP24HEADER));
      header24BMP.bmp2.cbFix         = sizeof(BITMAPINFOHEADER2);
      header24BMP.bmp2.cx            = gbm.w;
      header24BMP.bmp2.cy            = gbm.h;
      header24BMP.bmp2.cPlanes       = 1;             /*  Number of bit planes. */
      header24BMP.bmp2.cBitCount     = gbm.bpp;       /*  Number of bits per pel within a plane. */
      header24BMP.bmp2.ulCompression = BCA_UNCOMP;    /*  Compression scheme used to store the bit map. */
      header24BMP.bmp2.cbImage       = dataDstLength; /*  Length of bit-map storage data, in bytes. */
   }

   /* create system bitmap */
   {
      HDC hdc;
      HPS hps;
      SIZEL sizl;

      BITMAPINFO2 *bmpInfo;

      if (gbm.bpp <= 8)
         bmpInfo = (PBITMAPINFO2) &header8BMP;
      else
         bmpInfo = (PBITMAPINFO2) &header24BMP;

      /* create a memory context */
      if ( (hdc = DevOpenDC(hab, OD_MEMORY, "*", 0L, (PDEVOPENDATA) NULL, (HDC) NULL)) == (HDC) NULL )
      {
         free(dataDst);
         return FALSE;
      }

      sizl.cx = bmpInfo->cx;
      sizl.cy = bmpInfo->cy;

      /* create presentation space */
      if ( (hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC)) == (HPS) NULL )
      {
         DevCloseDC(hdc);
         free(dataDst);
         return FALSE;
      }

      /* create the bitmap */
      *bmpHandle = GpiCreateBitmap( hps
                                    , (PBITMAPINFOHEADER2) bmpInfo
                                    , CBM_INIT
                                    , (PBYTE) dataDst
                                    , bmpInfo );

      /* destroy memory context */
      GpiSetBitmap(hps, (HBITMAP) NULL);
      GpiDestroyPS(hps);
      DevCloseDC(hdc);
   }
   free(dataDst);

   return TRUE;
}

/********************************/

/* Depending on GBM.DLL version the number of pages can be retrieved (versions > 1.35)
 * or the functionality does not yet exist.
 *
 * Dynamically link the specific function to support also older versions of GBM.DLL.
 */
static BOOL SupportsNumberOfPagesQuery(void)
{
   HMODULE hmod;
   PFN     functionAddr = NULL;
   APIRET  rc = 0;

   /* check version first */
   if (gbm_version() < 135)
   {
      return FALSE;
   }

   /* now dynamically link GBM.DLL */
   rc = DosLoadModule("", 0, "GBM", &hmod);
   if (rc)
   {
      return FALSE;
   }

   /* lookup gbm_read_imgcount() */
   rc = DosQueryProcAddr(hmod, 0L, "gbm_read_imgcount", &functionAddr);

   DosFreeModule(hmod);

   return rc ? FALSE : TRUE;
}

/********************************/

/* Depending on GBM.DLL version the number of pages can be retrieved (versions > 1.35)
 * or the functionality does not yet exist.
 *
 * Dynamically link the specific function to support also older versions of GBM.DLL.
 */
static BOOL GetNumberOfPages(const char * fileName, const int fd, const int ft, int * numPages)
{
   HMODULE hmod;
   PFN     functionAddr = NULL;
   APIRET  rc = 0;

   /* check version first */
   if (gbm_version() < 135)
   {
      return FALSE;
   }

   /* now dynamically link GBM.DLL */
   rc = DosLoadModule("", 0, "GBM", &hmod);
   if (rc)
   {
      return FALSE;
   }

   /* lookup gbm_read_imgcount() */
   rc = DosQueryProcAddr(hmod, 0L, "gbm_read_imgcount", &functionAddr);
   if (rc)
   {
      DosFreeModule(hmod);
      return FALSE;
   }

   /* call gbm_read_imgcount(const char *fn, int fd, int ft, int *pimgcnt) */
   if (functionAddr(fileName, fd, ft, numPages) != GBM_ERR_OK)
   {
      DosFreeModule(hmod);
      return FALSE;
   }

   DosFreeModule(hmod);

   return TRUE;
}

/********************************/

/** Expand 1bpp to 24bpp */
static BOOL Expand1To24bit(GBM *gbm, const GBMRGB *gbmrgb, byte **data)
{
  int stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
  int new_stride = ((gbm->w * 3 + 3) & ~3);
  int bytes, y;
  byte *new_data;

  if (gbm->bpp == 24)
  {
    return TRUE;
  }
  if (gbm->bpp != 1)
  {
    return FALSE;
  }

  bytes = new_stride * gbm->h;
  if ( (new_data = malloc((size_t) bytes)) == NULL )
  {
    return FALSE;
  }

  for (y = 0; y < gbm->h; y++)
  {
    byte  *src = *data + y * stride;
    byte  *dest = new_data + y * new_stride;
    byte  c;
    int   x;

    for (x = 0; x < gbm->w; x++)
    {
      if ((x & 7) == 0)
      {
        c = *src++;
      }
      else
      {
        c <<= 1;
      }

      *dest++ = gbmrgb[c >> 7].b;
      *dest++ = gbmrgb[c >> 7].g;
      *dest++ = gbmrgb[c >> 7].r;
    }
  }
  free(*data);
  *data = new_data;
  gbm->bpp = 24;

  return TRUE;
}


