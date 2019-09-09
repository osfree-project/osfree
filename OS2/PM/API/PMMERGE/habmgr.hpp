/* F_hab.hpp */
/* functions for HAB management */
/* ver 0.00 25.08.2002       */

#ifndef FREEPM_HAB
  #define FREEPM_HAB

#include <FreePM.hpp>
//#include <osfree.h>
#include <malloc.h>

//#include <queue.hpp>
#include <window.hpp>

#define LOCKED    1
#define UNLOCKED  0

//#define FREEPMS_MAX_NUM_THREADS 1024

#define Q_ORDINAL_HAB   0
#define Q_ORDINAL_INDEX 1

struct _FreePM_hwnd
{
  HWND hwnd;       /* window handle */
  FPM_Window *pw;  /* pointer to window class */
//  void *pw;  /* pointer to window class */
};

struct _FreePM_ClassInfo
{
  CLASSINFO ci;
  char      classname[128];
};

struct _FreePM_HABinf
{
   volatile int Access; /* fast access semaphore  */
   int ordinal;         /* Thread ordinal         */
   int lastError;       /* last error for thread  */
   int used;            /* 0-free, 1-used (??)    */
   int clientId;        /* magic number, assigned to client */
   int iHAB;            /* HAB index from server  */

   class  Fs_Queue   *pQueue;
   volatile int msgSendTo;/* (iHAB+1 ?? ) to msg is sent, semaphore for msg send state */
   QMSG msgSend;        /* Buffer for one message which sent from one thread to another */

   int nHwnd;           /* Number of client's windows */
   int lAllocpHwnd;     /* Size of pHwnd buffer */
   _FreePM_hwnd *pHwnd; /* Buffer with pointers to windows handles of current anchor block */
   int numWinClasses;
   int lAllocWinClasses;
   struct _FreePM_ClassInfo *pWinClassList; /* Private window classes list */
};
/* буффер длиной lAllocpHwnd*sizeof(_FreePM_hwnd) со списком hwnd окон, принадлежащих HABу */
  \
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
