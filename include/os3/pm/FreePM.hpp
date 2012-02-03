/* FreePM.hpp */
/* ver 0.03 10.09.2002 */
/* first version 0.01 03.07.2002 */
#ifndef FREEPM_FREEPM
  #define FREEPM_FREEPM

// OS/2 Headers
#define INCL_OS2
#define INCL_BSE
#define INCL_BSEDOS
#define INCL_PM
#define INCL_WIN
#define INCL_ERRORS
#define INCL_SHLERRORS
#define INCL_WINERRORS
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_DOSNMPIPES
#define INCL_DOSSEMAPHORES
#define INCL_DOSMODULEMGR
#define INCL_WINFRAMEMGR
#include <osfree.h>
#include <pmerr.h>

// FreePM specific headers
#include "F_config.hpp"
#include "F_def.hpp"
#include "F_pipe.hpp"
#include "F_globals.hpp"
#include "F_utils.hpp"
#include "f_Err.h"


/*+---------------------------------+*/
/*| Global variables                |*/
/*+---------------------------------+*/
extern int _FreePM_id_index;

#define MAX_QMSG_SIZE  64
#define MAX_SQMSG_SIZE 256

#include "Fs_queue.hpp"

/* message queue. */
class  FreePM_Queue
{
 HAB     hab;         /* Anchor-block handle.   */
 int     length;      /* queue length, messages */
 int     start;       /* start index of circular queue */
 LONG    lQueuesize;  /* Maximum queue size.    */
 HMQ     hmq;         /* Message-queue handle.  */
 volatile int Access; /* Fast semaphore         */

 QMSG queue[MAX_QMSG_SIZE];

public:
 FreePM_Queue(void)
 {  lQueuesize   = MAX_QMSG_SIZE;
    length       = 0;
    hmq          = NULLHANDLE;
    start        = 0;
    Access       = 0;
    hab          = NULL;
 }
 FreePM_Queue(int iHab)
 {  lQueuesize   = MAX_QMSG_SIZE;
    length       = 0;
    hmq          = NULLHANDLE;
    start        = 0;
    Access       = 0;
    hab          = iHab;
 }
 int Add(PQMSG pmsg);
 int Del(int id);
 int DelFirst(void);
 int Get(PQMSG pmsg);

 int GetLength(void)
 {  return length;
 }

};

struct F_DC /* Device context */
{  int used;
   int dev_type;       /* тип устройства: 0-память, 1-PM Window, 2 - GENGRADD, 3 - SDDNUCLEUS */
   HWND hwnd;          /* хендл окна */
   int iHab;
   int units;          /* единицы измерения pels, 0.1 mm, etc.*/
};

/* Presentation Space, handle to F_PS is HPS*/
struct  F_PS /* micro PS ?? */
{
public:
   int used;
   int iDC; /* индекс F_DC */
        /* Absolute screan coordinates in pixels */
   int x0,y0,z0;/* left lower corner */
   int nx;   /* X size in pixels  */
   int ny;   /* Y size in pixels  */
   int bytesPerPixel;  /* байт на пиксел */
/* текущие значения */
   int color;
   int x,y,z;
//todo
};

/* Window Class */
class FPM_Window
{  int id; /* идентификатор */
protected:
   HWND handle;     /*  Window handle. */
   HWND hwndOwner;  /*  Owner-window handle. */
   HWND hwndParent; /*  Parent-window handle. */
   int dev_class;   /* класс устройства: 0-память, 1-дисплей, 2 - принтер */
   int dev_type;    /* тип устройства: 0-память, 1-PM Window, 2 - GENGRADD, 3 - SDDNUCLEUS */
   int x,y,z; /* координаты начала - левого нижнего угла */
   int nx; /* размер по X в pixel'ах */
   int ny; /* размер по Y в pixel'ах */
   int bytesPerPixel;  /* байт на пиксел */
   int bitsPerPixel;   /* бит на пиксел */
   int nColors;        /* число цветов на пиксел: 2,16,256, 32k, 64k ... */
   int class_type; /* класс окна     */
   PCSZ  pszClass;   /*  Registered-class name. */
   PCSZ  pszName;    /*  Window text. */
   ULONG flStyle;    /*  Window style. */
   ULONG rid;        /*  Window resource identifier. */
   struct WinPresParam pp;

public:
   FPM_Window
                  (HWND hwndParent,  /*  Parent-window handle. */
                   PCSZ  _pszClass,   /*  Registered-class name. */
                   PCSZ  _pszName,    /*  Window text. */
                   ULONG _flStyle,    /*  Window style. */
                   LONG _x,          /*  x-coordinate of window position. */
                   LONG _y,          /*  y-coordinate of window position. */
                   LONG _nx,          /*  Width of window, in window coordinates. */
                   LONG _ny,          /*  Height of window, in window coordinates. */
                   HWND hwndOwner,   /*  Owner-window handle. */
                   HWND hwndInsertBehind, /*  Sibling-window handle. */
                   ULONG _rid,       /*  Window resource identifier. */
                   PVOID pCtlData,   /*  Pointer to control data. */
                   PVOID pPresParams);/*  Presentation parameters. */
   FPM_Window(void)
   {
      dev_type  = FPM_W_DEVT_MEMORY;
      dev_class = FPM_W_DEV_MEMORY;
      nx = ny = 0;
      bytesPerPixel=bitsPerPixel=0;
      nColors = 0;
      id =_FreePM_id_index++;
      flStyle = 0;
      class_type = 0;
      pszClass = NULL;
      pszName  = NULL;
      rid = 0;
      hwndOwner = hwndParent = handle = 0;
   };

   int CreateFPM_Window(HWND hwndParent,  /*  Parent-window handle. */
                   PCSZ  _pszClass,   /*  Registered-class name. */
                   PCSZ  _pszName,    /*  Window text. */
                   ULONG _flStyle,    /*  Window style. */
                   LONG _x,          /*  x-coordinate of window position. */
                   LONG _y,          /*  y-coordinate of window position. */
                   LONG _nx,          /*  Width of window, in window coordinates. */
                   LONG _ny,          /*  Height of window, in window coordinates. */
                   HWND hwndOwner,   /*  Owner-window handle. */
                   HWND hwndInsertBehind, /*  Sibling-window handle. */
                   ULONG _rid,       /*  Window resource identifier. */
                   PVOID pCtlData,   /*  Pointer to control data. */
                   PVOID pPresParams);/*  Presentation parameters. */
   void SetHandle(int _hwnd)
   { handle = _hwnd;
   }
   HWND GetHandle(void)
   { return handle;
   }

//   int proc( HWND hwndWnd, ULONG ulMsg, MPARAM mpParm1,MPARAM mpParm2 );  /*  Window procedure pointer  (?) */
 virtual int proc(PQMSG pqmsg);
 int proc( HWND hwndWnd, ULONG ulMsg, MPARAM mpParm1,MPARAM mpParm2 ); /* From F_Window.cpp */
 //int GetPS(F_PS *pps);
 HPS GetPS(void);
 BOOL ReleasePS(HPS hps);

 virtual int Draw(HPS hps);
 int SendMsg_to_proc(ULONG umsg, MPARAM mp1, MPARAM mp2);
 int WinSetWindowPos(HWND hwndInsertBehind, LONG x, LONG y, LONG cx, LONG cy, ULONG fl);

};

/* Frame Window Class                */
class FRAME_Window:public FPM_Window
{
 public:
/* child windows */
//todo: those should be other window classes
   FPM_Window *pClient;
   FPM_Window *pTitleBar;
   FPM_Window *pSysMenu;
   FPM_Window *pMinMaxMenu;
   FPM_Window *pMenu;
   FPM_Window *pVertScroll;
   FPM_Window *pHorizScroll;
   FRAME_Window(void):FPM_Window()
   { pClient   = NULL;
     pTitleBar = NULL;
     pSysMenu  = NULL;
     pMinMaxMenu = NULL;
     pMenu       = NULL;
     pVertScroll  = NULL;
     pHorizScroll = NULL;
   }
   ~FRAME_Window(void)
   { if(pClient)
         delete(pClient);
     if(pTitleBar)
         delete(pTitleBar);
     if(pSysMenu)
         delete(pSysMenu);
     if(pMinMaxMenu)
         delete(pMinMaxMenu);
     if(pMenu)
         delete(pMenu);
     if(pVertScroll)
         delete(pVertScroll);
     if(pHorizScroll)
         delete(pHorizScroll);
   }
   int proc(PQMSG pqmsg);
};

/* Standard Window Class                */
/*  handle is FRAME Window handle;      */
/*   maybe it should be FRAME window ?? */
/* tothink: what is the diff between Standard Window and FRAME ? */
class STD_Window:public FRAME_Window
{
 public:
/* child windows */
//   FPM_Window Client;
//   FPM_Window TitleBar;
//   FPM_Window SysMenu;
//   FPM_Window Menu;
//   FPM_Window VertScroll;
//   FPM_Window HorizScroll;

//   STD_Window(void):FPM_Window(void)
//   {
//   }

   int CreateSTD_Window(HWND hwndParent,
               ULONG flStyle,
               PULONG pflCreateFlags,
               PSZ pszClientClass,
               PSZ pszTitle,
               ULONG styleClient,
               HMODULE hmod,
               ULONG idResources,
               PHWND phwndClient);

};

#endif
   /* FREEPM_FREEPM  */
