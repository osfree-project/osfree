/* F_DeskTop.cpp */
/* functions of class FreePM_DeskTop */
/* DEBUG: section 8 server class FreePM_DeskTop */
/* ver 0.01 12.09.2002       */

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <builtin.h>
#include <time.h>

#include "FreePM.hpp"
#include "FreePMs.hpp"
#include "F_hab.hpp"
#include "F_utils.hpp"
#include "F_globals.hpp"
#include "Fs_globals.hpp"
#include "FreePM_err.hpp"
#include "Fs_HPS.hpp"
//#include "F_GPI.hpp"
#include "F_def.hpp"
#include "debug.h"
#include <pmclient.h>

time_t getCurrentTime(void);

#define LONGFromRGB(R,G,B) (LONG)(((LONG)R<<16)+((LONG)G<<8)+(LONG)B)
#define LOCK_DESKTOP                            \
    {   int ilps_raz = 0, ilps_rc;              \
        do                                      \
        {  ilps_rc =  __lxchg(&Access,LOCKED);  \
           if(ilps_rc)                          \
           { if(++ilps_raz  < 3)  DosSleep(0);  \
             else            DosSleep(1);       \
           }                                    \
        } while(ilps_rc);          \
    }

#define UNLOCK_DESKTOP  {__lxchg(&Access,UNLOCKED);}

extern "C" int  _DeskTopSendQueue(void *pDClass, QMSG  *qmMsg);

static int BytesPerBmpPixel=1;
PBYTE pBmpBuffer = NULL;
int ZbNx, ZbNy, y_ZbNx[1284]; /* пpоизведение y*ZbNx */

HBITMAP hbm = 0;
BITMAPINFOHEADER2 bmp;
BITMAPINFO2 _bmi;
PBITMAPINFO2 pbmi = NULL; //&_bmi;
HPS hpsDesktop;

int DeskTopWindow::CreateDeskTopWindow( ULONG _flStyle,    /*  Window style. */
                   LONG _nx,          /*  Width of window, in window coordinates. */
                   LONG _ny,          /*  Height of window, in window coordinates. */
                   PVOID pCtlData,    /*  Pointer to control data. */
                   PVOID pPresParams) /*  Presentation parameters. */
{
  int rc;
  FPM_Window *dw = new FPM_Window(); /* Viking: Is this really right? Added it because
                                       CreateFPM_Window was called without a valid instance! */
  rc = dw->CreateFPM_Window(NULL,"DeskTop","Desktop text",
                    _flStyle,0L,0L,_nx,_ny,NULL,NULL,0, pCtlData, pPresParams);

  return rc;
}

int InitBuffer(void *pBmpBuffer, int nx, int ny, int bpp)
{
   int x, y, iR, iG, iB, *pBuf, col;

   for (y = 0; y < ny; y++)
   {
      for (x = 0; x < nx; x++)
      {
         pBuf = (int *) ((char *)pBmpBuffer + (y*nx+x) * bpp);
         iR = y%256;
         iG = x%256;
         iB = ((x*y)*30)%256;

         *pBuf = LONGFromRGB(iR,iG,iB);
      }
   }

   return 0;
}

/***********************************************************************/
/* Init desktop window: get iHAB, create queue, create  desktop window */
/***********************************************************************/
int FreePM_DeskTop::Init(int nx, int ny, int bytesPerPixel)
{
    int i, ordinal,tid,rc, len,inf[2];

/* desktop is a usual application with its HAB, queue and procedure */
/* initialize hab */

    ordinal = QueryThreadOrdinal(tid);
    rc = _shab.QueryOrdinalUsed(ordinal, Q_ORDINAL_HAB);
    if(rc != -1)
    {  _shab.SetError(rc, PMERR_ALREADY_INITIALIZED);
       fatalf("FreePM_DeskTop::Init WARNING: Tread Ordinal already used\n");
       return NULLHANDLE;
    }

    iHab = pSession->hab_list.Add(iClientId,-1);
/* Create queue */
    _shab.hab[iHab].pQueue = new FreePM_Queue(iHab);
    pQueue =  _shab.hab[iHab].pQueue;
    debug(8, 0) ("FreePM_DeskTop::Init iHab = %i\n", iHab);

/* создание окна HWND_DESKTOP  */
/*  the creation of window HWND_DESKTOP */
   pWindow = new DeskTopWindow(); /* bytesPerPixel */
   pWindow->CreateDeskTopWindow(0L,nx, ny,  NULL, NULL); /* bytesPerPixel */
   hwnd = AddWindow(pWindow);
   pWindow->SetHandle(hwnd);

/* Init device i.e. memory, PM, gradd or else */
   rc = pSession->InitDevice(FPM_DEV_PMWIN, this);
   /*rc = pSession->InitDevice(FPM_DEV_SERVERMEM, this);*/
   /* init videomode */

   /* Draw a picture in a buffer */
   //pBmpBuffer = (PBYTE)this->pVBuffmem;
   InitBuffer(this->pVBuffmem, nx, ny, bytesPerPixel);
   debug(8, 0) ("FreePM_DeskTop::Init OK\n");

/* создание остальных окон     */
/* the creation of the remaining windows */
//todo

  // prokushev: I think here we must create separate thread which will
  // process desktop messages loop.

   return 0;
}

/* добавить окно к десктопу */
/* to add window to [desktop] */
/* */
HWND FreePM_DeskTop::AddWindow(FPM_Window *pNewWindow)
{  HWND hwnd;
   int ind;
//  FPM_Window *pNewWindow;

/* get window handle */
   hwnd = _WndList.Add(iHab);
LOCK_DESKTOP;
       if(numWindows+1 >= nWAlloced)
       {  nWAlloced += 256;
          if(pWindows == NULL)
          {       pWindows = (FPM_Window * *) calloc(nWAlloced, sizeof(FPM_Window *));
           /*  pHabSvrInf[0] is formal, not realy used,  may will use it for "master" hab/thread ? */
                  memset((void *)&pWindows[0],0,sizeof(_FreePM_HABserverinf));
                  numWindows = 1;
          }
          else
                  pWindows = (FPM_Window**) realloc((void *)pWindows, nWAlloced * sizeof(FPM_Window *));
       }
       ind = numWindows++;

//   pNewWindow = new
//   FPM_Window(NULL,   /*  Parent-window handle. */
//              NULL,   /*  Registered-class name. */
//              NULL,   /*  Window text. */
//              0L,     /*  Window style. */
//              x,      /*  x-coordinate of window position. */
//              y,      /*  y-coordinate of window position. */
//              nx,     /*  Width of window, in window coordinates. */
//              ny,     /*  Height of window, in window coordinates. */
//              NULL,   /*  Owner-window handle. */
//              NULL,   /*  Sibling-window handle. */
//              0L,     /*  Window resource identifier. */
//              NULL,   /*  Pointer to control data. */
//              NULL);  /*  Presentation parameters. */

   pWindows[ind] =  pNewWindow;

UNLOCK_DESKTOP;
   return hwnd;
}

/* удалить окно с десктопа */
/* to remove window from [desktop] */
int FreePM_DeskTop::DelWindow(int ind)
{    int i,rc;
     if(ind < 0 || ind >=  numWindows)
             return -1;
LOCK_DESKTOP;
     if(pWindows[ind])
        delete pWindows[ind];
     numWindows--;
     for(i=ind; i < numWindows; i++)
     {  pWindows[i] = pWindows[i+1];
     }
UNLOCK_DESKTOP;

   return 0;
}

/* Add Presentation space */
HPS  FreePM_DeskTop::AddPS(HWND hwnd)
{
//todo
  return 0;
}

/* Delete Presentation space */
int FreePM_DeskTop::DelPS(HPS hps)
{
//todo
  return 0;
}

/*  Get Desctop size and bytes per pixel */
int FreePM_DeskTop::GetPar(int &Dx, int &Dy, int &bytes_PerPixel)
{  Dx = nx;
   Dy = ny;
   bytes_PerPixel = bytesPerPixel;
   return 0;
}

/* special function for the call from the processor of device (as the minimum, PM)
   and the transfer of communication in the turn of desktop
*/
extern "C" int  _DeskTopSendQueue(void *pDClass, QMSG  *pqmMsg)
{
    FreePM_DeskTop *pDesktop;
    QMSG  qMsg;
    SQMSG sqMsg;
    int debl=1;
    pDesktop = (FreePM_DeskTop *) pDClass;
    if(pDesktop == NULL || pqmMsg == NULL)
                 return -1;
    if(pqmMsg->msg == WM_MOUSEMOVE)
               debl = 9;
    debug(8, debl) ("Hw %x Msg %x m1 %x mp2 %x (%i,%i)\n",
              pqmMsg->hwnd,pqmMsg->msg,pqmMsg->mp1,pqmMsg->mp2,pqmMsg->ptl.x,pqmMsg->ptl.y/*,pqmMsg->time,pqmMsg->uid*/);
/* pqmMsg - pointer to old PM QMSG structure! */
    qMsg.hwnd = pqmMsg->hwnd;
    qMsg.msg  = pqmMsg->msg;
    qMsg.mp1  = pqmMsg->mp1;
    qMsg.mp2  = pqmMsg->mp2;
    qMsg.ptl  = pqmMsg->ptl;
//    qMsg.time = pqmMsg->time;
//todo: get current user id
//    qMsg.uid  = 0;
//todo: get current remoute id
//    qMsg.remoteId = 0;

    getCurrentTime();
//    qMsg.dtime = _FreePM_current_dtime;
 if(qMsg.msg == WM_PAINT)
     DosBeep(6000,1);

     pDesktop->pWindow->proc(&qMsg);
    if(qMsg.msg == WM_PAINT)
    {  int iwn;
       int ihab,rc;
/* Send msg to all child windows */
       for(iwn=0; iwn < _WndList.n; iwn++)
       {  if( _WndList.pWND[iwn].iParent == pDesktop->hwnd)
          {  DosBeep(3000,1);
//todo
             sqMsg.qmsg = qMsg;
             sqMsg.qmsg.hwnd = _WndList.pWND[iwn].hwnd;
             sqMsg.ihto   = _WndList.pWND[iwn].iHab;
             rc = _WndList.QueryHab(pDesktop->hwnd,ihab);
             sqMsg.ihfrom = ihab;
             debug(8, 2) ("Add Msg WM_PAINT to Hwnd %x, hab=%i; habfrom=%i\n",sqMsg.qmsg.hwnd, sqMsg.ihto,ihab);
             pDesktop->pSession->hab_list.Queue.Add(&sqMsg);
          }
       }
    }


    return 0;
}

int DeskTopWindow::proc( PQMSG pqmsg)
{
 //todo
  switch(pqmsg->msg)
  {
     case WM_PAINT:
    debug(8, 0) ("DeskTopWindow::proc  WM_PAINT\n");
/* the conclusion of the window of desktop */
     {  HPS hps;
        hps = GetPS();
        Draw(hps);
/* the conclusion of all daughterly windows of desktop */
     }
       break;
  }
  return NULL;
}

int open_Vbuff(RECTL rclRect)
{
    static int LbmpBuffOld = 0;
    int LbmpBuff;
    PSZ pszData[4] = { "Display", NULL, NULL, NULL };
    SIZEL sizlPage = {0, 0};
    LONG alData[2];
//   RGB2 rgb, *prgb;
    int Numcolors,ii,s;
    extern ULONG ColorTable[256];

    ZbNx = rclRect.xRight - rclRect.xLeft   + 1;
    ZbNy = rclRect.yTop   - rclRect.yBottom + 1;
    //ZbNx = 640;
    //ZbNy = 480;
    //ZbNx = ((ZbNx-1)/32+1)*32;
    //ZbNy = ((ZbNy-1)/32+1)*32;
    if (ZbNx > 960)  ZbNx = 960;
    if (ZbNy > 1280) ZbNy = 1280;

    /* пеpесчет y_ZbNx */
    for(ii=0,s=0;ii<=ZbNy;ii++,s+= ZbNx) y_ZbNx[ii] = s;

    if(pBmpBuffer == NULL)
    {
      printf("Критическая ошибка: нет памяти");
      DosSleep(4000);
      exit(1);
    }

/* Create the memory device context and presentation space so they
 * are compatible with the screen device context and presentation space.
 */

     printf("pBmpBuffer=%lx\n", pBmpBuffer);

     if(hbm)
     {
        bmp.cx = ZbNx;
        bmp.cy = ZbNy;
        pbmi->cx = bmp.cx;
        pbmi->cy = bmp.cy;
        pbmi->cbImage = bmp.cbImage;
        return 0;
     }
          /* PSZ pszData[4] = { "Display", NULL, NULL, NULL }; */
      /* SIZEL sizlPage = {0, 0}; */

      /* Determine the device's plane/bit-count format. */
      GpiQueryDeviceBitmapFormats(hpsDesktop, 2, alData);

      bmp.cbFix = (ULONG) sizeof(BITMAPINFOHEADER2);
      bmp.cx = 16;
      bmp.cy = 16;
      bmp.cPlanes = alData[0];
      bmp.cBitCount = alData[1];

      if (bmp.cBitCount < 8)
      {   bmp.cPlanes = 1;
          bmp.cBitCount = 8;
      }
      bmp.cPlanes = 1;
      bmp.cBitCount = 8;

//!!
bmp.cBitCount = 24;

      bmp.ulCompression = BCA_UNCOMP;
      bmp.cbImage = 0;
      bmp.cxResolution = 70;
      bmp.cyResolution = 70;
      Numcolors = 230;
      //bmp.cclrUsed = Numcolors;
      //bmp.cclrImportant = Numcolors-30;
//!!
bmp.cclrUsed = 0;
bmp.cclrImportant = 0;

      bmp.usUnits = BRU_METRIC;
      bmp.usReserved = 0;
      bmp.usRecording = BRA_BOTTOMUP;
      bmp.usRendering = BRH_NOTHALFTONED;//BRH_ERRORDIFFUSION;
      bmp.cSize1 = 0;
      bmp.cSize2 = 0;
      bmp.ulColorEncoding = BCE_RGB;
      bmp.ulIdentifier = 1;

      ii = (sizeof(RGB2) * (1 << bmp.cPlanes) * (1 << bmp.cBitCount));
      if(bmp.cBitCount  > 8)
                           ii = 16;
      ii += sizeof(BITMAPINFO2);
      pbmi = (BITMAPINFO2 *) malloc(ii);

      pbmi->cbFix = bmp.cbFix;
      pbmi->cx = bmp.cx;
      pbmi->cy = bmp.cy;
      pbmi->cPlanes = bmp.cPlanes;
      pbmi->cBitCount = bmp.cBitCount;
      pbmi->ulCompression = BCA_UNCOMP;
      pbmi->cbImage = 1;
      pbmi->cxResolution = 70;
      pbmi->cyResolution = 70;

      pbmi->cclrUsed = Numcolors;
      pbmi->cclrImportant = Numcolors-20;
//!!
pbmi->cclrUsed = 0;
pbmi->cclrImportant = 0;
      pbmi->usUnits = BRU_METRIC;
      pbmi->usReserved = 0;
      pbmi->usRecording = BRA_BOTTOMUP;
      pbmi->usRendering = BRH_NOTHALFTONED;
//  pbmi->usRendering = BRH_ERRORDIFFUSION;
//  pbmi->usRendering = BRH_PANDA;
//  pbmi->usRendering = BRH_SUPERCIRCLE;
      pbmi->cSize1 = 0;
      pbmi->cSize2 = 0;
      pbmi->ulColorEncoding = BCE_RGB;
      pbmi->ulIdentifier = 1;

    bmp.cbImage = 0;//((ZbNx+31)/32) * ZbNy;
    bmp.cx = ZbNx;
    bmp.cy = ZbNy;
    pbmi->cx = bmp.cx;
    pbmi->cy = bmp.cy;
    pbmi->cbImage = bmp.cbImage;

//      prgb = (RGB2 *) (((PBYTE)pbmi)+bmp.cbFix);

   return 0;
}


void DisplayVbuf(HPS  hpsBuffer, RECTL rect, int isChange)
{
    POINTL aptl[4];
    LONG   lhits;

    /* Create a bit map that is compatible with the display.            */
    if (isChange)
    {
        if (hbm) GpiDeleteBitmap(hbm);
        hbm = GpiCreateBitmap(hpsDesktop, &bmp, CBM_INIT, (PBYTE) pBmpBuffer, pbmi);
        GpiSetBitmap(hpsDesktop,hbm);
        GpiSetBitmapBits(hpsDesktop, 0,bmp.cy,(PBYTE) pBmpBuffer, pbmi); //?????
    }
    else
    {
        // GpiSetBitmap(hpsBuffer,hbm);
        // ii = GpiSetBitmapBits(hpsBuffer, 0,bmp.cy,(PBYTE) pBmpBuffer, pbmi);
        GpiSetBitmap(hpsDesktop,hbm);
        GpiSetBitmapBits(hpsDesktop, 0,bmp.cy,(PBYTE) pBmpBuffer, pbmi);
    }

    aptl[0].x = rect.xLeft;       /* Lower-left corner of destination rectangle  */
    aptl[0].y = rect.yBottom;     /* Lower-left corner of destination rectangle  */
    aptl[1].x = rect.xRight;      /* Upper-right corner of destination rectangle */
    aptl[1].y = rect.yTop;        /* Upper-right corner of destination rectangle */
    /* Source-rectangle dimensions (in device coordinates)                       */
    aptl[2].x = 0;                /* Lower-left corner of source rectangle       */
    aptl[2].y = 0;                /* Lower-left corner of source rectangle       */
    aptl[3].x = bmp.cx;           //  bmp.cx;
    aptl[3].y = bmp.cy;           // bmp.cy;

    pbmi->cy = bmp.cy;

    /* >>>>>>>>>>>>>>>>>>>>>>>>>>> */
    //lhits = GpiBitBlt(hpsBuffer , hpsMem,
    //     3,   /* 3-source rect=dest rect 4 Number of points in aptl */
    //     aptl, ROP_SRCCOPY,  BBO_IGNORE/* | BBO_PAL_COLORS*/ );

//       if(ii != GPI_OK)
//       {    ERRORID errid;
//            errid = WinGetLastError(hab);
//            ierr++;
//       }

    GpiSetBitmap(hpsDesktop,0);

//  rc = GpiDeleteBitmap(hbm);
}

extern "C" BOOL APIENTRY GpiQueryBoundaryData(HPS hps, PRECTL prclBoundary);

int DeskTopWindow::Draw(HPS hps)
{
   RECTL rect;
   //POINTL ptlPoint;/*  Corner point. */
   //int lControl;
   /* background */
   /*todo: if background == picture */
   /* rectangle 0,0, nx-1,ny-1, bytesPerPixel color pp.BackgroungColor */
   //GpiBox(hps, lControl, &ptlPoint, 0, 0);
   //printf("hpsDesktop=%lx\n", hps);
   //hpsDesktop = hps;
   //GpiQueryBoundaryData(hpsDesktop, &rect);
   //open_Vbuff(rect);
   //DisplayVbuf(hpsDesktop, rect, 1);

   return 0; /* Just to make OW compile it. */
}
