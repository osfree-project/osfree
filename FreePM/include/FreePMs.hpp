/*
 $Id: FreePMs.hpp,v 1.8 2002/11/18 13:21:22 evgen2 Exp $
*/
/* FreePMs.hpp */
/* ver 0.00 10.09.2002 */
#ifndef FREEPM_FREEPMS
  #define FREEPM_FREEPMS
#include "Fs_queue.hpp"
#include "Fs_hab.hpp"

#define FPM_DEV_CLIENTMEM      0   /* In client memory only */
#define FPM_DEV_SERVERMEM      1   /* In server memory only */
#define FPM_DEV_PMWIN          2   /* server write to memory and then bitbit memory as bitmap to window */
#define FPM_DEV_PMWIN_DIR      3   /* server write directly to PM window  */
#define FPM_DEV_GENGRADD       4  
#define FPM_DEV_SDDNUCLEUS     5

class FreePM_session;

class DeskTopWindow:public FPM_Window
{
 public:
     DeskTopWindow(void):FPM_Window()
     {
     }
     CreateDeskTopWindow( ULONG _flStyle,    /*  Window style. */
                   LONG _nx,          /*  Width of window, in window coordinates. */
                   LONG _ny,          /*  Height of window, in window coordinates. */
                   PVOID pCtlData,    /*  Pointer to control data. */
                   PVOID pPresParams); /*  Presentation parameters. */

   int proc(PQMSG pqmsg);
   int Draw(HPS hps);
};



class FreePM_DeskTop
{  int id; /* идентификатор */
   int iHab;       /* formal iHab      */
   int iClientId;  /* formal client id */
   int numWindows; /* number of Desktop's windows */
   int nWAlloced;  /* alloced windows  */
   FPM_Window **pWindows; /* указатель на массив указателей на окна */
   volatile int Access; /* fast access semaphor, use when pWND is realloced  */


   int nx; /* размер по X в pixel'ах */
   int ny; /* размер по Y в pixel'ах */
   int bytesPerPixel;  /* байт на пиксел */
   int bitsPerPixel;   /* бит на пиксел */
   int nColors;        /* число цветов на пиксел: 2,16,256, 32k, 64k ... */
   int dev_type;    /* тип устройства: 0-память, 1-PM Window, 2 - GENGRADD, 3 - SDDNUCLEUS */
public:
   HWND hwnd;       /* desktop window handle */
private:
   struct WinPresParam pp;
public:
   FreePM_session *pSession;  /* pointer to parent session */
   FreePM_Queue *pQueue;        /* master Queue */
   DeskTopWindow  *pWindow; /* desktop window */
   int *pVBuffmem;          /* pointer to videobuffer in memory */ 
public:
   FreePM_DeskTop(void)
   { numWindows = 0;
     nWAlloced = 0;
     pWindows = NULL;
     id =_FreePM_id_index++;
     dev_type = FPM_DEV_SERVERMEM;
     iHab = NULL;
     pSession = NULL;
     pQueue = NULL;
     Access = UNLOCKED;
     pWindow = NULL;
     pVBuffmem = NULL;
   }

   FreePM_DeskTop(int _dev_type, int _nx, int _ny, int _bytesPerPixel,
                  struct WinPresParam *_pp,  FreePM_session *_pSession)
   { numWindows = 0;
     nWAlloced  = 0;
     pWindows = NULL;
     nx = _nx;
     ny = _ny;
     bytesPerPixel = _bytesPerPixel;
     id =_FreePM_id_index++;
     dev_type = _dev_type;
     pp = *_pp;
     pSession = _pSession;
     Access = UNLOCKED;
     pWindow = NULL;
   }
   int GetPar(int &Dx, int &Dy, int &bytes_PerPixel);
   HWND AddWindow(FPM_Window *NewWindow);
   int DelWindow(int ind);
   int Init(int nx, int ny, int bytes_PerPixel);
   HPS AddPS(HWND hwnd);
   int DelPS(HPS hps);

};


class FreePM_session
{  int id; /* идентификатор */
public:
   int numDeskTops;          /* число рабочих столов */
   int numpDeskTopsAllocated; /* выделено памяти под массив указателей на рабочие столы */
   FreePM_DeskTop **pDeskTops; /* указатель на массив указателей на рабочие столы */
   int indCurrentDesktop;      /* индекс текущего десктопа */
   class _FreePM_HAB_serverlist  hab_list;

public:
   FreePM_session(void)
   { numDeskTops = 0;
     numpDeskTopsAllocated = 0;
     indCurrentDesktop     = 0;
     pDeskTops = NULL;
     id =_FreePM_id_index++;
   }
   int AddDesktop(int _dev_type, int nx, int ny, int bytesPerPixel, struct WinPresParam *pp);
   int DelDesktop(int ind);
   int InitDevice(int _dev_type, FreePM_DeskTop *pDesktop);
   int SetCurrentDesktop(int ind);

};

#endif
   /* FREEPM_FREEPMS  */
