/*
 $Id: F_hab.hpp,v 1.5 2002/11/06 12:37:23 evgen2 Exp $
*/
/* F_hab.hpp */
/* functions for HAB management */
/* ver 0.00 25.08.2002       */

#ifndef FREEPM_HAB
  #define FREEPM_HAB

#include "FreePM.hpp"
#include <malloc.h>

#define Q_ORDINAL_HAB   0
#define Q_ORDINAL_INDEX 1

struct _FreePM_hwnd
{  HWND hwnd;       /* window handle */
   FPM_Window *pw;  /* pointer to window class */
};

struct _FreePM_HABinf
{
   volatile int Access; /* fast access semaphore  */
   int ordinal;         /* Thread ordinal         */
   int lastError;       /* last error for thread  */
   int used;            /* 0-free, 1-used (??)    */
   int clientId;        /* magic number, assigned to client */
   int iHAB;            /* HAB index from server  */

   class  FreePM_Queue   *pQueue;
   volatile int msgSendTo;/* (iHAB+1 ?? ) to msg is sent, semaphore for msg send state */
   QMSG msgSend;        /* буфер для одного сообщения, посылаемого из одной нитки в другую */

   int nHwnd;      /* число окон клиентской задачи */
   int lAllocpHwnd;/* размер буфера pHwnd */
   _FreePM_hwnd *pHwnd;
/* буффер длиной lAllocpHwnd*sizeof(_FreePM_hwnd) со списком hwnd окон, принадлежащих HABу */
};

class _FreePM_HAB
{  int n;  /* number of HABinf */
   int nTaskHwnd; /* число окон клиентской задачи */
//   int lAllocpHwnd;/* */
public:
   struct _FreePM_HABinf hab[FREEPMS_MAX_NUM_THREADS];
//   HWND *pHwnd;

   _FreePM_HAB(void)
   {  n = 0;
      nTaskHwnd = 0;
//      pHwnd = NULL;
//      lAllocpHwnd = 0;
   }

   ~_FreePM_HAB()
   {  int i;
      for(i=0; i < n; i++)
      { if(hab[i].pHwnd)
        {  free(hab[i].pHwnd);
           hab[i].pHwnd = NULL;
           hab[i].nHwnd = hab[i].lAllocpHwnd = 0;
        }
      }
   }
   int QueryOrdinalUsed(int ordinal, int mode);
   int QueryHABexist(int iHAB);
   int AddHAB(int ordinal, int iHAB, int clientId);
   int DelHAB(int iHAB);
   int GetCurrentHAB(void);
   int GetCurrentHABindex(void);

   int SetError(int iHAB, int err);
   int SetError(int err);
   int QueryError(int iHAB);
   int AddHwnd(HWND hwnd, FPM_Window *pw);
   int AddHwnd(HWND hwnd, int iHAB, FPM_Window *pw);
   int DelHwnd(HWND hwnd);
   int DelHwnd(HWND hwnd, int iHAB);
   int QueryIndexesHwnd(HWND hwnd, int &indwind);
   int QueryHwnd(HWND hwnd);
   int QueryHwnd(HWND hwnd, int iHAB);

};


#endif
   /*  FREEPM_HAB  */
