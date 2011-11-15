/*
 $Id: F_session.cpp,v 1.8 2002/11/18 13:24:52 evgen2 Exp $
*/
/* FreePM_session.cpp */
/* DEBUG: section 7     Server session class */
/* functions of class FreePM_session */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>

#include <process.h>

#include "FreePM.hpp"
#include "FreePMs.hpp"

#include "F_globals.hpp"
#include "Fs_globals.hpp"
#include "Fs_driver.h"
#include <gd.h>
/* Creates a palette-based image (up to 256 colors). */
/*gdImagePtr gdImageCreate(int sx, int sy);*/

extern "C" int  _DeskTopSendQueue(void *pDClass, QMSG  *pqmMsg);

/*int initGD(int DimX, int DimY);*/
int initGD(int DimX, int DimY) {
        return (int)gdImageCreate(DimX, DimY);
}


void _FPM_PMWinStart(void *param)
{
 debug(7, 0) ("Call FPM_DeviceStart\n");
  FPM_DeviceStart(param);
}


int FreePM_session::AddDesktop(int _dev_type, int nx, int ny, int bytesPerPixel,  struct WinPresParam * pp)
{
  int i;

  FreePM_DeskTop *pDeskTop;
/* create Desktop */
  pDeskTop = new FreePM_DeskTop(_dev_type,nx,ny, bytesPerPixel, pp, this);

  if(numDeskTops == 0 && pDeskTops == NULL)
   {  numpDeskTopsAllocated = 16;
      pDeskTops = (FreePM_DeskTop * *) calloc(sizeof(FreePM_DeskTop *),numpDeskTopsAllocated);
      pDeskTops[numDeskTops++] = pDeskTop;
   } else {
      if(numDeskTops >= numpDeskTopsAllocated)
      {  numpDeskTopsAllocated += 16;
         pDeskTops = (FreePM_DeskTop * *) realloc((void *)pDeskTops, sizeof(FreePM_DeskTop *) * numpDeskTopsAllocated);
         for(i = 0; i < 16; i++) pDeskTops[numpDeskTopsAllocated-i-1] = NULL;
      }
      pDeskTops[numDeskTops++] = pDeskTop;
   }

/* Init desktop window: get iHAB, create queue, create  desktop window */
    pDeskTop->Init(nx,ny,bytesPerPixel);
    if(numDeskTops == 1)
            SetCurrentDesktop(0);
   return numDeskTops-1;
}

int FreePM_session::DelDesktop(int ind)
{
    if(ind < 0 || ind >= numDeskTops)
           return 1;
    if(pDeskTops[ind] == NULL)
           return 2;
    delete pDeskTops[ind];
    pDeskTops[ind] = NULL;
    return 0;
}

int FreePM_session::InitDevice(int _dev_type, FreePM_DeskTop *pDesktop)
{
  int rc,id,idd,nx,ny, bytes_PerPixel;
  static int params[4];
  PBYTE pBmpBuffer = NULL;
  static int LbmpBuffOld = 0;
  int LbmpBuff;

   switch(_dev_type)
   {
      case FPM_DEV_PMWIN: /* */
        debug(7, 0) ("FreePM_session::InitDevice PMWIN: %d \n", _dev_type);

        pDesktop->GetPar(nx, ny, bytes_PerPixel);
        printf ("nx=%u, ny=%u, bpp=%u\n", nx, ny, bytes_PerPixel);

        if(pBmpBuffer)
        {
           LbmpBuff = bytes_PerPixel * nx * (ny+2)+4;
           if(LbmpBuff != LbmpBuffOld)
             pBmpBuffer = (BYTE *) realloc(pBmpBuffer,LbmpBuff);
        } else {
          LbmpBuff = bytes_PerPixel * nx * (ny+2)+4;
          pBmpBuffer = (BYTE *)malloc(LbmpBuff);
        }
        LbmpBuffOld = LbmpBuff;

        //pDesktop->pVBuffmem = (int *) initGD(nx,  ny);
        pDesktop->pVBuffmem = pBmpBuffer;

        /* start separate thread for PM window */
        params[0] = (int)pDesktop;
        params[1] = (int)pDesktop->pVBuffmem;
        params[2] = (int)(&_DeskTopSendQueue);
        params[3] = 0;
        id = _beginthread(_FPM_PMWinStart,NULL, THREAD_STACK_SIZE*2,(void *)params);
        break;
      case  FPM_DEV_PMWIN_DIR:
 debug(7, 0) ("WARNING:FreePM_session::InitDevice device PMWIN_DIR not yet supported\n");
         break;
      case FPM_DEV_SERVERMEM:
 debug(7, 0) ("FreePM_session::InitDevice SERVERMEM\n", _dev_type);
      pDesktop->GetPar(nx, ny, bytes_PerPixel);
      /*rc = initGD( nx,  ny);*/
          pDesktop->pVBuffmem = (int *) initGD(nx,  ny);
 debug(7, 0) ("WARNING:FreePM_session::InitDevice device SERVERMEM not yet supported\n");
         break;
      default:
 debug(7, 0) ("WARNING:FreePM_session::InitDevice device %i not supported\n", _dev_type);
        fatal("Device not supported");

         break;
   }
   return rc;
}

int FreePM_session::SetCurrentDesktop(int ind)
{
    if(ind < 0 || ind >= numDeskTops)
           return 1;
    if(pDeskTops[ind] == NULL)
           return 2;

    indCurrentDesktop = ind;
//todo: regen DeskTop
    _WndList.iHwndCurrentDeskTop = pDeskTops[ind]->hwnd;
    return 0;
}

