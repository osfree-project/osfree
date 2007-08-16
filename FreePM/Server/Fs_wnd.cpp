/*
 $Id: Fs_wnd.cpp,v 1.10 2003/06/15 17:45:47 evgen2 Exp $
*/
/* Fs_wnd.cpp */
/* server window manager (?) */
/* ver 0.00 3.03.2002       */
/* DEBUG: section 2   server WND manager */

#include <malloc.h>
#include <memory.h>
#include <builtin.h>
#include "FreePM.hpp"
#define F_INCL_DOSPROCESS
   #include "F_OS2.hpp"

#include "F_hab.hpp"
#include "Fs_WND.hpp"
#include "FreePM_err.hpp"
#include "Fs_globals.hpp"

/* for debug()*/
#include <stdio.h>
#include "F_globals.hpp"

/* добавить окно с HABом _ihab в список окон.
   return: индекс окна в этом списке.
           сей индекс будет называться потом хендлом окна (hwnd)
*/
int F_WND_List::Add(int _ihab)
{   int i,rc,is,ind;
    int ilps_raz = 0, ilps_rc;
    do
    {  ilps_rc =  __lxchg(&wndAccess,LOCKED);
       if(ilps_rc)
       { if(++ilps_raz  < 3)  DosSleep(0);
          else                DosSleep(1);
       }
    } while(ilps_rc);
/* поищем неиспользуемые элементы массива pWND */
    is = 0;
    for(i=1;i<n;i++)  /* 0-element doesn't use */
    { if(!pWND[i].used)
      {  is= 1;
         ind = i;
      }
    }
    if(!is) /* free element not found  */
    {  if(n+1 >= nAllocated)
       {  nAllocated += 256;
          if(pWND == NULL)
          {        pWND = (WND *) calloc(nAllocated, sizeof(WND));
           /*  pWND[0] is formal, not realy used */
                   memset((void *)&pWND[0],0,sizeof(WND));
                   n = 1;
          }
          else
                  pWND = (WND *) realloc((void *)pWND, nAllocated * sizeof(WND));
       }
       ind = n++;
    }
    __lxchg(&wndAccess,UNLOCKED);
    memset((void *)&pWND[ind],0,sizeof(WND));
    pWND[ind].used = 1;
    pWND[ind].iHab = _ihab;
    pWND[ind].iHDC = NULL;/* при роджении окна неизвестен его DC */
    pWND[ind].hwnd = ind; /* для контроля или будущих изменений  */
debug(2, 0) ("WND_List::Add %i, n=%i\n",ind,n);
    return ind;
}

int F_WND_List::Query(int iWND)
{ int rc = 0;
  if(iWND < 0 || iWND >= n) rc = -1;
  if(pWND[iWND].used)       rc = 1;
  return rc;
}

int F_WND_List::QueryHab(int iWND, int &ihab)
{ int rc = 0;
  if(iWND < 0 || iWND >= n) rc = -1;
  if(pWND[iWND].used)  {     rc = 1;  ihab = pWND[iWND].iHab; }
  return rc;
}

int F_WND_List::Del(int iWND)
{  int ilps_raz = 0, ilps_rc;

   if(iWND < 0 || iWND >= n)
                 return 1;
   if(!pWND[iWND].used)
                 return 2;
debug(2, 0) ("WND_List::Del %i\n",iWND);
    do
    {  ilps_rc =  __lxchg(&wndAccess,LOCKED);
       if(ilps_rc)
       { if(++ilps_raz  < 3)  DosSleep(0);
          else            DosSleep(1);
       }
    } while(ilps_rc);

/* if window use HPS - del HPS's ?? */
//todo

/* if window have DC - close DC */
    if(pWND[iWND].iHDC)  CloseDC(pWND[iWND].iHDC);
    pWND[iWND].iHDC = NULL;

    pWND[iWND].used = 0;
    if(iWND == n -1)
    {   while(n>0 && (pWND[n].used))
                     n--;
    }
    __lxchg(&wndAccess,UNLOCKED);
    return 0;
}


int F_WND_List::SetParent(int iHWND, int iHWNDparent)
{
   if(iHWND == HWND_DESKTOP)
                 iHWND = iHwndCurrentDeskTop;
   if(iHWNDparent == HWND_DESKTOP)
                 iHWNDparent = iHwndCurrentDeskTop;

   if(iHWND < 0 || iHWND >= n)
                 return 1;
   if(iHWNDparent < 0 || iHWNDparent >= n)
                 return 1;
   if(!pWND[iHWND].used)
                 return 2;
   if(!pWND[iHWNDparent].used)
                 return 2;
    pWND[iHWND].iParent = iHWNDparent;

    return 0;
}

/* Get "Cached presentation space" */
HPS F_WND_List::GetHPS(int iHWND)
{  int rc, ihab,is,i,ind;
   int ilps_raz = 0, ilps_rc;

   if(iHWND == HWND_DESKTOP)
                 iHWND = iHwndCurrentDeskTop;
/* Query ihab for iHWND */
   rc = QueryHab(iHWND, ihab);
   if(rc!=1)
   {   _shab.SetError(rc, PMERR_INVALID_HWND);
      return NULL;
   }
   if(pWND[iHWND].iHDC)  /* window has open DC */
   {
   } else {
      rc = OpenDC(ihab, iHWND);
   }

    do
    {  ilps_rc =  __lxchg(&hpsAccess,LOCKED);
       if(ilps_rc)
       { if(++ilps_raz  < 3)  DosSleep(0);
          else                DosSleep(1);
       }
    } while(ilps_rc);

/* поищем неиспользуемые элементы массива pPS */
    is = 0;
    for(i=1;i<numPS;i++)  /* 0-element doesn't use */
    { if(!pPS[i].used)
      {  is= 1;
         ind = i;
      }
    }
    if(!is) /* free element not found  */
    {  if(numPS+1 >= nPSAllocated)
       {  nPSAllocated += 64;
          if(numPS == NULL)
          {        pPS = (F_PS  *) calloc(nPSAllocated, sizeof(F_PS ));
           /*  pDC[0] is formal, not realy used */
                   memset((void *)&pPS[0],0,sizeof(F_PS ));
                   numPS = 1;
          }
          else
                  pPS = (F_PS  *) realloc((void *)pPS,  sizeof(F_PS ));
       }
       ind = numPS++;
    }
    __lxchg(&hpsAccess,UNLOCKED);
    memset((void *)&pDC[ind],0,sizeof(F_PS));
    pPS[ind].used = 1;
    pPS[ind].iDC = pWND[iHWND].iHDC;
    pPS[ind].x0 =  pWND[iHWND].x;
    pPS[ind].y0 =  pWND[iHWND].y;
    pPS[ind].z0 =  pWND[iHWND].z;
    pPS[ind].nx =  pWND[iHWND].nx;
    pPS[ind].ny =  pWND[iHWND].ny;

debug(2, 0) (__FUNCTION__" %i, n=%i\n",ind,numPS);

    return ind;
}

int F_WND_List::ReleaseHPS(HPS ihps)
{  int ilps_raz = 0, ilps_rc;

   if(ihps < 0 || ihps >= numPS)
                 return 1;
   if(!pPS[ihps].used)
                 return 2;
    debug(2, 0) (__FUNCTION__ " %i\n",ihps);
    do
    {  ilps_rc =  __lxchg(&hpsAccess,LOCKED);
       if(ilps_rc)
       { if(++ilps_raz  < 3)  DosSleep(0);
          else            DosSleep(1);
       }
    } while(ilps_rc);
    pPS[ihps].used = 0;
    if(ihps == numPS -1)
    {   while(numPS > 0 && (pPS[numPS].used))
                     numPS--;
    }
    __lxchg(&hpsAccess,UNLOCKED);


    return 0;
}

HDC F_WND_List::OpenDC(int _ihab, HWND ihwnd)
{   int i,rc,is,ind;
    int ilps_raz = 0, ilps_rc;
    do
    {  ilps_rc =  __lxchg(&dcAccess,LOCKED);
       if(ilps_rc)
       { if(++ilps_raz  < 3)  DosSleep(0);
          else                DosSleep(1);
       }
    } while(ilps_rc);
/* поищем неиспользуемые элементы массива pDC */
    is = 0;
    for(i=1;i<numDC;i++)  /* 0-element doesn't use */
    { if(!pDC[i].used)
      {  is= 1;
         ind = i;
      }
    }
    if(!is) /* free element not found  */
    {  if(numDC+1 >= nDCAllocated)
       {  nDCAllocated += 64;
          if(pDC == NULL)
          {        pDC = (F_DC *) calloc(nDCAllocated, sizeof(F_DC));
           /*  pDC[0] is formal, not realy used */
                   memset((void *)&pDC[0],0,sizeof(F_DC));
                   numDC = 1;
          }
          else
                  pDC = (F_DC *) realloc((void *)pDC, nDCAllocated * sizeof(F_DC));
       }
       ind = numDC++;
    }
    __lxchg(&dcAccess,UNLOCKED);
    memset((void *)&pDC[ind],0,sizeof(F_DC));
    pDC[ind].used = 1;
    pDC[ind].iHab = _ihab;
    pDC[ind].hwnd =  ihwnd;
debug(2, 0) (__FUNCTION__" %i, n=%i\n",ind,numDC);
    return ind;
}

int F_WND_List::CloseDC(int iDC)
{  int ilps_raz = 0, ilps_rc;

   if(iDC < 0 || iDC >= numDC)
                 return 1;
   if(!pDC[iDC].used)
                 return 2;
    debug(2, 0) (__FUNCTION__ " %i\n",iDC);
    do
    {  ilps_rc =  __lxchg(&dcAccess,LOCKED);
       if(ilps_rc)
       { if(++ilps_raz  < 3)  DosSleep(0);
          else            DosSleep(1);
       }
    } while(ilps_rc);
    pDC[iDC].used = 0;
    if(iDC == numDC -1)
    {   while(numDC>0 && (pDC[numDC].used))
                     numDC--;
    }
    __lxchg(&dcAccess,UNLOCKED);

    return 0;
}

int F_WND_List::WinSetWindowPos(int iHWND, int hwndInsertBehind, int x, int y, int nx, int ny, int flStyle)
{
//todo or toredo
return 0; /* Just to make OW compile it. */
}




