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
//#include "F_GPI.hpp"

time_t getCurrentTime(void);

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

/***********************************************************************/
/* Init desktop window: get iHAB, create queue, create  desktop window */
/***********************************************************************/
int FreePM_DeskTop::Init(int nx, int ny, int bytesPerPixel)
{   int i, ordinal,tid,rc, len,inf[2];

/* desktop is a usual application with its HAB, queue and procedure */
/* initialize hab */

    ordinal = QueryThreadOrdinal(tid);
    rc = _shab.QueryOrdinalUsed(ordinal, Q_ORDINAL_HAB);
    if(rc != -1)
    {  _shab.SetError(rc, PMERR_ALREADY_INITIALIZED);
       fatal("FreePM_DeskTop::Init WARNING: Tread Ordinal already used\n");
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

int DeskTopWindow::Draw(HPS hps)
{
   POINTL ptlPoint;/*  Corner point. */
   int lControl;
/* background */
/*todo: if background == picture */
/* rectangle 0,0, nx-1,ny-1, bytesPerPixel color pp.BackgroungColor */
//   F_GpiBox(hps, lControl,&ptlPoint,0,0);
   return 0; /* Just to make OW compile it. */
}

