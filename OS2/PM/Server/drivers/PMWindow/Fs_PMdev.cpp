/*
 $Id: Fs_PMdev.cpp,v 1.5 2002/11/18 13:24:53 evgen2 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//Things marked as 'temporary' need to resolve. For now no messages from
//hardware.

#define POKA 0

/* Includes to use OS/2 PM. */
 #define INCL_DOSPROCESS
 #define INCL_WIN
 #define INCL_GPI
 #define INCL_DOS

 #include "os2.h"

//#define F_INCL_DOS
//#include "F_base.hpp"
//#include "F_GPI.hpp"
//#include "F_win.hpp"
//#include "F_pmdev.hpp"

#define CLS_CLIENT               "WindowClass"

/*+---------------------------------+*/
/*| External function prototypes.   |*/
/*+---------------------------------+*/
//temporary extern "C" int _DeskTopSendQueue(void *pDClass, QMSG  *qmMsg);
//temporary extern "C" int  QueryThreadOrdinal(int &tid);

/*+---------------------------------+*/
/*| Internal function prototypes.   |*/
/*+---------------------------------+*/
MRESULT EXPENTRY ClientWndProc ( HWND hwndWnd,
   ULONG ulMsg,
   MPARAM mpParm1,
   MPARAM mpParm2 ) ;


extern "C" void APIENTRY FPM_DeviceStart(void *param);

LONG *GetVideoConfig(HDC hdc);
int ErrInfoMsg2(char *str);
int open_Vbuff(RECTL rclRect);
int InitBuffer(void);
void DisplayVbuf(HWND hwnd, HPS  hpsBuffer,RECTL rect, int isChange);

#define LONGFromRGB(R,G,B) (LONG)(((LONG)R<<16)+((LONG)G<<8)+(LONG)B)
#define MYM_SHOWMESSAGE 3333

static HAB     habAnchor;
extern HPS     hpsDrawBMPBuffer = NULLHANDLE;
static HDC     hdcBMP =  NULLHANDLE;
static HWND    hwndFrame;
static HWND    hwndClient;
//temporary static int     MainThreadOrdinal;
static int     *pVBuffmem = NULL;
//extern class VideoPowerPresentation videopres;
//extern const char *const _FreePM_Application_Name;

void APIENTRY FPM_DeviceStart(void *param)
{
   //HMQ         hmqQueue;
   //ULONG       ulFlags;
   //BOOL        bLoop;
   //QMSG        qmMsg;
   void       *pDesktop;
   int        *pParams;
/**********************************************/
//{
  PPIB pib;
  PTIB tib;
  int  tid;
  printf("In console\n");
  DosGetInfoBlocks(&tib, &pib);

//   if(pib->pib_ultype == 2)          /* VIO */

      pib->pib_ultype = 3;
//для справки  MainThreadOrdinal = ptib->tib_ordinal;
//temporary   MainThreadOrdinal = QueryThreadOrdinal(tid);
//}
   pParams = (int *)param;
   pDesktop  = (void *)pParams[0];
   pVBuffmem = (int *) pParams[1];
   if(pDesktop == NULL || pVBuffmem == NULL)
   {    printf("FS_PMdev.cpp:85 pDesktop=%p, pVBuffmem=%p\n", pDesktop, pVBuffmem);
                printf(" Error, not initialized resources! \n");
       //DosBeep(500,500);
       exit(2);
   }
   habAnchor = WinInitialize ( 0 ) ;

   HMQ         hmqQueue;
   ULONG       ulFlags;
   BOOL        bLoop;
   QMSG        qmMsg;



   hmqQueue = WinCreateMsgQueue ( habAnchor, 0 ) ;

   WinRegisterClass ( habAnchor,
                      CLS_CLIENT,
                      ClientWndProc,
                      CS_SIZEREDRAW,
                      sizeof(int *)
                      ) ;

   ulFlags = FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER |
             FCF_MINMAX | FCF_SHELLPOSITION | FCF_TASKLIST |FCF_VERTSCROLL|FCF_HORZSCROLL;

   hwndFrame = WinCreateStdWindow ( HWND_DESKTOP,
                                    WS_VISIBLE,
                                    &ulFlags,
                                    CLS_CLIENT,
                                    "FreePM Server Window",
                                    CS_SIZEREDRAW|CS_MOVENOTIFY,
                                    NULLHANDLE,
                                    0,
                                    &hwndClient ) ;

   if ( hwndFrame != NULLHANDLE ) {
      bLoop = WinGetMsg ( habAnchor,
                          &qmMsg,
                          NULLHANDLE,
                          0,
                          0 ) ;
//temporary        if(qmMsg.hwnd == hwndClient)  /* translate only client messages */
//temporary                _DeskTopSendQueue(pDesktop, &qmMsg);
      while ( bLoop ) {
         WinDispatchMsg ( habAnchor, &qmMsg ) ;
         bLoop = WinGetMsg ( habAnchor,
                             &qmMsg,
                             NULLHANDLE,
                             0,
                             0 ) ;
//temporary        if(qmMsg.hwnd == hwndClient)  /* translate only client messages */
//temporary                _DeskTopSendQueue(pDesktop, &qmMsg);
      } /* endwhile */

      WinDestroyWindow ( hwndFrame ) ;
   } /* endif */

   WinDestroyMsgQueue ( hmqQueue ) ;
   WinTerminate ( habAnchor ) ;
//   return 0 ;
}

MRESULT EXPENTRY ClientWndProc ( HWND hwndWnd,
                                 ULONG ulMsg,
                                 MPARAM mpParm1,
                                 MPARAM mpParm2 )
{
   switch ( ulMsg ) {

   case WM_CREATE:
         {
             SIZEL sizl = { 0L, 0L };
             LONG  *alCaps;
             int rc;
             hdcBMP = WinOpenWindowDC(hwndWnd);
             alCaps = GetVideoConfig(hdcBMP);

      hpsDrawBMPBuffer = GpiCreatePS( habAnchor,
                  hdcBMP,
                  &sizl,
                  PU_PELS | GPIF_DEFAULT |
                  GPIT_NORMAL | GPIA_ASSOC );
         rc = GpiCreateLogColorTable(hpsDrawBMPBuffer,0,LCOLF_RGB,0,0,NULL);
         }

      break;

   case WM_ERASEBACKGROUND:
      return MRFROMSHORT ( TRUE ) ;

      case WM_PAINT:
      {   RECTL rclClient;
          RECTL RectField;
          HPS hps;
          static int col = 0;


      hps = WinBeginPaint(hwndWnd,NULLHANDLE,&rclClient);
//      WinFillRect(hps, &rclClient, col);
      col = (col+1) % 16;
//      printf("Rect= %i %i % i %i\n",rclClient.xLeft,rclClient.yBottom, rclClient.xRight,rclClient.yTop);
          WinQueryWindowRect(hwndWnd,&RectField);
          open_Vbuff( RectField);
          InitBuffer();
          DisplayVbuf(hwndWnd,hpsDrawBMPBuffer,RectField,1);

      WinEndPaint(hps);

      }
    break;

      case MYM_SHOWMESSAGE: /* показать сообщение из бэкгpаунда */
   WinAlarm( HWND_DESKTOP,WA_ERROR ); /* type of alarm */
   WinMessageBox( HWND_DESKTOP,    /* parent window handle    */
         HWND_DESKTOP,             /* owner window handle     */
         (PCSZ) mpParm1,           /* pointer to message text */
         "FreePM"/*_FreePM_Application_Name*/, /* pointer to title text   */
         0,                        /* message box identifier  */
         MB_OK | MB_ERROR |        /* message box style       */
         MB_SYSTEMMODAL );
    break;

   default:
      return WinDefWindowProc ( hwndWnd,
                                ulMsg,
                                mpParm1,
                                mpParm2 ) ;
   } /* endswitch */

//#endif //POKA
   return MRFROMSHORT ( FALSE ) ;
}

int ErrInfoMsg(char *str)
{
   WinAlarm( HWND_DESKTOP,    /* desktop window handle  */
        WA_ERROR );            /* type of alarm          */

   WinMessageBox( HWND_DESKTOP, /* parent window handle           */
         HWND_DESKTOP,          /* owner window handle            */
         str,                   /* pointer to message text        */
         "FreePM"/*_FreePM_Application_Name*/,             /* pointer to title text          */
         0,                     /* message box identifier for help*/
         MB_OK | MB_ERROR |/* message box style              */
         MB_SYSTEMMODAL );

//   SetStatusStr(NULL,str,1);
   return 0;
}

int ErrInfoMsg2(char *str)
{  int ordinal, tid;
static char LastErrmsg[512];

//temporary   ordinal =  QueryThreadOrdinal(tid);
//temporary   if(ordinal == MainThreadOrdinal)
//temporary        return  ErrInfoMsg(str);
   strcpy(LastErrmsg,str);
   WinAlarm( HWND_DESKTOP,    /* desktop window handle  */
        WA_ERROR );            /* type of alarm          */
   WinPostMsg( hwndClient, MYM_SHOWMESSAGE, LastErrmsg, NULL );
//   SetStatusStr(NULL,str,1);
   return 0;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#if POKA

/*+---------------------------------+*/
/*| External function prototypes.   |*/
/*+---------------------------------+*/
int ErrInfoMsg2(char *str);

/*+---------------------------------+*/
/*| Internal function prototypes.   |*/
/*+---------------------------------+*/
LONG *GetVideoConfig(HDC hdc);
int CalcColorForPower( double LaserPower, double MaxPower);
int open_Vbuff(RECTL rclRect);
int InitBuffer(void);
void DisplayVbuf(HWND hwnd, HPS  hpsBuffer,RECTL rect, int isChange);


#endif //POKA
/**************************/
/* Статические переменные */
/**************************/
static int BytesPerBmpPixel=1;
PBYTE pBmpBuffer= NULL;
int ZbNx,ZbNy,y_ZbNx[1284]; /* пpоизведение y*ZbNx */

 HBITMAP hbm = 0;
 BITMAPINFOHEADER2 bmp;
 BITMAPINFO2 _bmi;
 PBITMAPINFO2 pbmi = NULL; //&_bmi;
 HDC hdcMem;
 HPS hpsMem;

struct VideoDevConfigCaps
{ int  sts;
  LONG Caps[CAPS_DEVICE_POLYSET_POINTS];
};

struct VideoDevConfigCaps VideoDevConfig = { 0 };

#if POKA

int VideoPowerPresentation::Draw(HWND hwnd, RECTL rect, HPS hps, int x,int y)
{  int ix,iy;
   POINTL ptl;
   double dx,dy;
   RECTL rect0;
   rect0.xLeft = 1;
   rect0.xRight=nx;
   rect0.yBottom =1;
   rect0.yTop = ny;
   open_Vbuff( rect0);
//   InitBuffer();
   CopyBufferToBmp();
   DisplayVbuf(hwnd,hps,rect,1);

   dx = rect.xRight - rect.xLeft;
   dy = rect.yTop - rect.yBottom;
   if(dx < 1 || dy < 1)
      return 1;
   ptl.x = 0;
   ptl.y =  (y + ny/2.) * dy/ny ;
   if(ptl.y > rect.yTop) ptl.y = rect.yTop;
   if(ptl.y < 0) ptl.y = 0;

   GpiSetColor(hps,0);
   GpiMove(hps,&ptl);
   ptl.x = (nx + 0.5) * dx / nx;
   GpiLine(hps,&ptl);
   ptl.x = (x + nx/2.) * dx/nx;
   if(ptl.x > rect.xRight) ptl.x = rect.xRight;
   if(ptl.x < 0) ptl.x = 0;


   ptl.y = 0;
   GpiMove(hps,&ptl);
   ptl.y = (ny+0.5) * dy / ny;
   GpiLine(hps,&ptl);


   return 0;
}

#endif

int InitBuffer(void)
{
   int x,y,iR,iG,iB, *pBuf,col;

   for(y=0;y<ZbNy;y++)
   {
      for(x=0;x<ZbNx;x++)
      {  pBuf = (int *) (pBmpBuffer + (y*ZbNx+x) * BytesPerBmpPixel);
         iR = y%256;
         iG = x%256;
         iB = 0; //((x*y)/256)%256;

         *pBuf = LONGFromRGB(iR,iG,iB);
        }
   }

   return 0;
}


int open_Vbuff(RECTL rclRect)
{
static int LbmpBuffOld=0;
   int LbmpBuff;
   PSZ pszData[4] = { "Display", NULL, NULL, NULL };
   SIZEL sizlPage = {0, 0};
   LONG alData[2];
//   RGB2 rgb, *prgb;
   int Numcolors,ii,s;
extern ULONG ColorTable[256];

/*
   if(UseDive)
   { return  open_VbuffDive(rclRect, &BytesPerBmpPixel);
   }
*/

//    else if(pVC_Caps[CAPS_COLOR_BITCOUNT] < 8) BytesPerBmpPixel = 3;
//BytesPerBmpPixel = 3;
/*********** !!!!!!!!!!!!! **********/
//BytesPerBmpPixel = 1;

    ZbNx = rclRect.xRight - rclRect.xLeft+1;
    ZbNy = rclRect.yTop - rclRect.yBottom + 1;
    ZbNx = 640;
    ZbNy = 480;
    ZbNx = ((ZbNx-1)/32+1)*32;
    ZbNy = ((ZbNy-1)/32+1)*32;
    if(ZbNy > 1280) ZbNy = 1280;
/* пеpесчет y_ZbNx */
    for(ii=0,s=0;ii<=ZbNy;ii++,s+= ZbNx) y_ZbNx[ii] = s;

    if(pBmpBuffer)
    {
       LbmpBuff = BytesPerBmpPixel * ZbNx * (ZbNy+2)+4;
       if(LbmpBuff != LbmpBuffOld)
            pBmpBuffer = (BYTE *) realloc(pBmpBuffer,LbmpBuff);
    } else {

       LONG *pVC_Caps;
       pVC_Caps = GetVideoConfig(NULLHANDLE);
       if(pVC_Caps[CAPS_COLOR_BITCOUNT] > 8)
                 BytesPerBmpPixel = pVC_Caps[CAPS_COLOR_BITCOUNT]/8;

BytesPerBmpPixel = 3;
//BytesPerBmpPixel = 2;

       LbmpBuff = BytesPerBmpPixel * ZbNx * (ZbNy+2)+4;
       pBmpBuffer = (BYTE *)malloc(LbmpBuff);
    }
    if(pBmpBuffer == NULL)
       {
          ErrInfoMsg2("Критическая ошибка: нет памяти");
          DosSleep(4000);
          exit(1);
       }
   LbmpBuffOld = LbmpBuff;
/**************/
/*
{ char str[128];
  sprintf(str,"Dx=%i Dy=%i",ZbNx,ZbNy);
  SetStatusStr(NULL,str,2);
  sprintf(str,"Rect=%i,%i %i,%i",rclRect.xLeft,rclRect.yBottom,rclRect.xRight,rclRect.yTop);
  SetStatusStr(NULL,str,3);

}
*/
/**************/

/* Create the memory device context and presentation space so they
 * are compatible with the screen device context and presentation space.
 */
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

      hdcMem = DevOpenDC(habAnchor, OD_MEMORY, "*", 4,
          (PDEVOPENDATA) pszData, NULLHANDLE);
      hpsMem = GpiCreatePS(habAnchor, hdcMem, &sizlPage,
          PU_PELS | GPIA_ASSOC | GPIT_MICRO);
      /* Determine the device's plane/bit-count format. */
      GpiQueryDeviceBitmapFormats(hpsMem, 2, alData);


      bmp.cbFix = (ULONG) sizeof(BITMAPINFOHEADER2);
      bmp.cx = 16;
      bmp.cy = 16;
      bmp.cPlanes = alData[0];
      bmp.cBitCount = alData[1];

      if(bmp.cBitCount < 8)
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
      bmp.cclrUsed = Numcolors;
      bmp.cclrImportant = Numcolors-30;
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
/*  pbmi->usRendering = BRH_ERRORDIFFUSION; */
/*  pbmi->usRendering = BRH_PANDA; */
/*  pbmi->usRendering = BRH_SUPERCIRCLE; */
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


void close_Vbuff(void)
{  if(pBmpBuffer)
    free(pBmpBuffer);
    pBmpBuffer = NULL;

  if(hbm) GpiDeleteBitmap(hbm);
  hbm = 0;
  if(pbmi)
      free( pbmi);
  pbmi = NULL;
  GpiDestroyPS(hpsMem);       /* destroys presentation space */
  DevCloseDC(hdcMem);         /* closes device context       */
}


void DisplayVbuf( HWND hwnd, HPS  hpsBuffer,RECTL rect, int isChange)
{
     POINTL aptl[4];
     LONG lhits;
/*
   if(UseDive)
   {   DisplayVbufDive(hwnd, hpsBuffer,isChange);
       return;
   }
*/
  /* Create a bit map that is compatible with the display.            */
      if(isChange)
      {  if(hbm) GpiDeleteBitmap(hbm);
         hbm = GpiCreateBitmap(hpsMem, &bmp, CBM_INIT, (PBYTE) pBmpBuffer, pbmi);
         GpiSetBitmap(hpsMem,hbm);
  GpiSetBitmapBits(hpsMem, 0,bmp.cy,(PBYTE) pBmpBuffer, pbmi); //?????
      }
      else
      {  // GpiSetBitmap(hpsBuffer,hbm);
         // ii = GpiSetBitmapBits(hpsBuffer, 0,bmp.cy,(PBYTE) pBmpBuffer, pbmi);
           GpiSetBitmap(hpsMem,hbm);
           GpiSetBitmapBits(hpsMem, 0,bmp.cy,(PBYTE) pBmpBuffer, pbmi);
      }


      aptl[0].x = rect.xLeft;       /* Lower-left corner of destination rectangle  */
      aptl[0].y = rect.yBottom;       /* Lower-left corner of destination rectangle  */
      aptl[1].x = rect.xRight;  /* Upper-right corner of destination rectangle */
      aptl[1].y = rect.yTop; /* Upper-right corner of destination rectangle */
     /* Source-rectangle dimensions (in device coordinates)              */
      aptl[2].x = 0;      /* Lower-left corner of source rectangle       */
      aptl[2].y = 0;    /* Lower-left corner of source rectangle       */
      aptl[3].x = bmp.cx;  //  bmp.cx;
      aptl[3].y = bmp.cy;    // bmp.cy;

     pbmi->cy = bmp.cy;

/* >>>>>>>>>>>>>>>>>>>>>>>>>>> */
    lhits = GpiBitBlt(hpsBuffer , hpsMem,
         3,   /* 3-source rect=dest rect 4 Number of points in aptl */
         aptl, ROP_SRCCOPY,  BBO_IGNORE/* | BBO_PAL_COLORS*/ );

//       if(ii != GPI_OK)
//       {    ERRORID errid;
//            errid = WinGetLastError(hab);
//            ierr++;
//       }

      GpiSetBitmap(hpsMem,0);

//      rc = GpiDeleteBitmap(hbm);


}

/* Получить видеоконфигуpацию по полной пpогpамме */
LONG *GetVideoConfig(HDC hdc)
{
   LONG lCount = CAPS_PHYS_COLORS;
   LONG lStart = CAPS_FAMILY; /* 0 */
   BOOL rc;

   if( hdc )
   {  rc = DevQueryCaps( hdc, lStart, lCount, VideoDevConfig.Caps );
      if(rc)
      {   VideoDevConfig.sts = 1;
          return VideoDevConfig.Caps;
      }
   } else if(VideoDevConfig.sts == 1) {
          return VideoDevConfig.Caps;
   }
   return NULL;
}


