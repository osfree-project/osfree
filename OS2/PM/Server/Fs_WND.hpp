/*
 $Id: Fs_WND.hpp,v 1.9 2002/11/21 13:19:04 evgen2 Exp $
*/
/* Fs_WND.hpp */
/* ver 0.00 4.09.2002       */
#ifndef FREEPMS_WND
  #define FREEPMS_WND

/* The Window Structure (WND) */
/* This structure represents a window. It is created by WinCreateWindow.
    The WND has two main functions:
         It acts as the link between the MQ and the thread's window procedure
         It establishes the WND hierarchy.
*/

struct WND
{  int used;
   int iNextSibling;   /* index of Next sibling WND */
   int iParent;        /* index of Parent WND       */
   int iFirstChild;    /* index of First Child WND  */
   int iOwner;         /* index of Owner WND        */
   int iUserId;        /* user specified id         */

   int pMQ;     /* ? The pointer to the MQ that will queue messages sent and posted to this window  */
   int hwnd;    /* The windows's HWND  */
   int is16bit; /* ? A Boolean, which if non-zero, indicates that the window procedure address is a 16-bit far pointer  */
   int pWproc;  /* The address of the window procedure  */
   int iHab;    /* Window HAB  */
   int iHDC;    /* Window DC   */
   int x,y,z; /* координаты начала - левого нижнего угла */
              /* the coordinate of beginning - the left lower angle */
   int nx;    /* размер по X в pixel'ах */
              /* size on the X in pixel'[akh] */
   int ny;    /* размер по Y в pixel'ах */
              /* size on the Y in pixel'[akh] */

   int word;    /* Window word */
};

/* class F_WND_List control global WND List (array) */
class F_WND_List
{
   volatile int wndAccess; /* fast access semaphor, use when pWND is realloced  */
   volatile int hpsAccess; /* fast access semaphor, use when pPS  is realloced  */
   volatile int dcAccess;  /* fast access semaphor, use when pDC  is realloced  */
   int nAllocated;     /* Number of allocated array elements in pWND;*/
   int nPSAllocated;   /* allocated array elements in pPS  */
   int nDCAllocated;   /* allocated array elements in pDC  */
public:
   int n;            /* Number of array elements in pWND;*/
   struct WND *pWND;
   int iHwndCurrentDeskTop;  /* index of current desktop window */

   int numPS;        /* number of PS */
   F_PS  *pPS;       /* array of PS structures */

   int numDC;        /* number of F_DC */
   F_DC  *pDC;       /* array of FDC structures */

   F_WND_List(void)
   {  n = nAllocated = 0;
      pWND = NULL;
      pDC  = NULL;
      pPS  = NULL;
      wndAccess = UNLOCKED;
      hpsAccess = UNLOCKED;
      dcAccess = UNLOCKED;
      iHwndCurrentDeskTop = 0;
      numPS = nPSAllocated = 0;
      numDC = nDCAllocated = 0;

   }
   ~F_WND_List()
   {  if(pDC)
      { free(pDC);
      }
      if(pPS)
      { free(pPS);
      }
      if(pWND)
      { free(pWND);
      }
   }

   int Add(int _ihab); /* добавить окно */
   int Del(int iHWND); /* удалить окно  */
   int Query(int iHWND);
   int QueryHab(int iWND, int &ihab);
   int SetParent(int iHWND, int iHWNDparent);

   HPS GetHPS(int iHWND);     /* добавить HPS */
   int ReleaseHPS(HPS ihps);  /* удалить  HPS */
   HDC OpenDC(int _ihab, HWND ihwnd); /* добавить DC */
   int CloseDC(int iDC);      /* удалить  DC */
   /* установить положение */
   int WinSetWindowPos(int iHWND, int hwndInsertBehind, int x, int y, int nx, int ny, int flStyle);

};

#endif
       /* FREEPMS_WND */
