/*
 $Id: F_hab.cpp,v 1.2 2002/11/10 15:20:04 evgen2 Exp $
*/
/* F_hab.cpp */
/* functions for HAB management */
/* ver 0.02 6.09.2002       */
/* DEBUG: section 4     client HAB manager */

#include <stdio.h>
#include <malloc.h>
#include <builtin.h>
#include <time.h>
#include "FreePM.hpp"
#include "F_hab.hpp"
#include "F_globals.hpp"
#include "debug.h"
#include <pmclient.h>

/*+---------------------------------+*/
/*| External function prototypes.   |*/
/*+---------------------------------+*/
extern "C" int QueryThreadOrdinal(int &tid);


/***********************************/

/*
 HAB - "Handle Ancor Block"
 HAB handles thread-specific info:
- last PM error per thread
- whats else ?

*/

/* return:
-1 - not found,
else iHAB if mode = 0(Q_ORDINAL_HAB), and index if mode = 1(Q_ORDINAL_INDEX);
*/
int _FreePM_HAB::QueryOrdinalUsed(int ordinal, int mode)
{  int i,rc = -1;
    for(i = 0; i < n; i++)
    {  if(hab[i].used)
       {  if(hab[i].ordinal == ordinal)
          {  if(mode) rc = i;
             else     rc = hab[i].iHAB;
             break;
          }
       }
    }
    return rc;
}


/*  return:
  -1 - iHAB not found
   0 - free
   else index+1
*/

int _FreePM_HAB::QueryHABexist(int iHAB)
{  int i,rc = -1;
   for(i=0;i<n;i++)
   {  if(iHAB == hab[i].iHAB)
      { if(hab[i].used)   rc = i+1;
        else              rc = 0;
        break;
      }
   }
   return rc;
}

/* GetCurrentHAB
Return:
-2 - thread not initialized to FreePM (PMERR_INVALID_HAB)
 else iHAB
*/
int _FreePM_HAB::GetCurrentHAB(void)
{   int ordinal,tid,rc, iHAB;

    ordinal = QueryThreadOrdinal(tid);
    rc = QueryOrdinalUsed(ordinal,Q_ORDINAL_HAB);
    if(rc == -1)
       return -2;
    return rc;
}

int _FreePM_HAB::GetCurrentHABindex(void)
{   int ordinal,tid,rc, iHAB;

    ordinal = QueryThreadOrdinal(tid);
    rc = QueryOrdinalUsed(ordinal,Q_ORDINAL_INDEX);
    if(rc == -1)
       return -2;
    return rc;
}

/*
  return:
-1 = no space for HAB info, increase  FREEPMS_MAX_NUM_THREADS
 0 = Ok
*/
int _FreePM_HAB::AddHAB(int ordinal, int iHAB,  int clientId)
{   int i,isfree=0;


    for(i = 0; i < n; i++)
    {  if(!hab[i].used)
       {  isfree = 1;
          break;
       }
    }
    if(!isfree)
    { if(n < FREEPMS_MAX_NUM_THREADS)
      { isfree = 1;
        i = n++;
      } else
          return -1;
    }
    debug(4, 1)("AddHAB  ordinal=%x, iHAB=%x, clientId=%x, index=%i\n", ordinal, iHAB,  clientId, i );
    hab[i].used = 1;
    hab[i].clientId = clientId;
    hab[i].iHAB     = iHAB;
    hab[i].ordinal  = ordinal;
    hab[i].Access   = UNLOCKED;
    hab[i].lastError = 0;
    hab[i].pQueue = NULL;
    hab[i].msgSendTo = 0;
    hab[i].nHwnd     = 0;
    hab[i].lAllocpHwnd = 0;
    hab[i].pHwnd  = NULL;

    return 0;
}

/*  return:
 -1 - ordinal not found
  0 - Ok
*/
int _FreePM_HAB::DelHAB(int iHAB)
{  int i,is=0, rc;

   rc = QueryHABexist(iHAB);
   if(rc <= 0)  /* already free(0) or not found(-1) */
       return rc;
   i = rc -1;

/* test for semaphor ?? hab[i].Access */
    debug(4, 1)("DelHAB  iHAB=%x, index=%i\n",  iHAB,  i );
    hab[i].used = 0;
    hab[i].ordinal = 0;
    hab[i].Access  = UNLOCKED;
    if(hab[i].pQueue)
    {  delete hab[i].pQueue;
       hab[i].pQueue = NULL;
    }
    return 0;
}

/* returns:
 -1 - iHAB not found
 -2 - ordinal not used
  0 - Ok
*/
int _FreePM_HAB::SetError(int iHAB, int err)
{  int rc=0, rc1,i;
   rc1 = QueryHABexist(iHAB);
   if(rc1 == -1)      rc = -1;
   else if(rc1 == 0)  rc = -2;
   else
   {  i = rc1 - 1;
      hab[i].lastError = err;
   }

   return rc;
}

int _FreePM_HAB::SetError(int err)
{  int  rc=0, rc1,i;
   rc1 = GetCurrentHABindex();
   if(rc1 == -2) rc = -1;
   else  hab[rc1].lastError = err;
   return rc;
}


/* returns:
 -1 - bad iHAB
 -2 - ordinal not used
else
 lastError
*/
int _FreePM_HAB::QueryError(int iHAB)
{  int rc,rc1;

   rc1 = QueryHABexist(iHAB);
   if(rc1 == -1)      rc = -1;
   else if(rc1 == 0)  rc = -2;
   else rc = hab[rc1-1].lastError;
   return rc;
}

/* добавить хендл окна к списку окон клиентской задачи */
/* return:
    -1 - PMERR_INVALID_HWND
    -2 - thread not initialized to FreePM (PMERR_INVALID_HAB)
*/
int _FreePM_HAB::AddHwnd(HWND hwnd, FPM_Window *pw)
{   int ordinal,tid,rc, iHAB;
    int i,n;
/* сначала проверим, нет ли уже этого окна в списке */
    rc = QueryHwnd(hwnd);
    if(rc > 0)
       return -1;
    ordinal = QueryThreadOrdinal(tid);
    rc = QueryOrdinalUsed(ordinal,Q_ORDINAL_HAB);
    if(rc == -1)
       return -2;
    iHAB = rc;
    rc = AddHwnd(hwnd, iHAB, pw);

    return rc;
}

/* Add hwnd to local hab's hwnd list
   return:
    -1 - PMERR_INVALID_HWND (already in list)
    -2 - bad iHAB
   else hwnd index
*/

int _FreePM_HAB::AddHwnd(HWND hwnd, int iHAB, FPM_Window *pw)
{   int i,rc,n,rcS;

    rc = QueryHABexist(iHAB);
    if(rc <= 0)  /* already free(0) or not found(-1) */
       return -2;
    i = rc - 1;
   do
   {  rcS =  __lxchg(&hab[i].Access,LOCKED);
      if(rcS) DosSleep(0);
   } while(rcS);     // доступ закрыт
/* проверим, нет ли уже этого окна в списке */
    rc = QueryHwnd(hwnd);
    if(rc <= 0)
    {
       if(hab[i].nHwnd+1 >= hab[i].lAllocpHwnd)
       {   hab[i].lAllocpHwnd += 256;
           if(hab[i].pHwnd == NULL)
                       hab[i].pHwnd = (_FreePM_hwnd *) calloc(hab[i].lAllocpHwnd, sizeof(_FreePM_hwnd));
           else
                       hab[i].pHwnd = (_FreePM_hwnd *) realloc((void *)hab[i].pHwnd, hab[i].lAllocpHwnd * sizeof(_FreePM_hwnd));
       }
       hab[i].pHwnd[hab[i].nHwnd].hwnd = hwnd;
       hab[i].pHwnd[hab[i].nHwnd].pw = pw;
       rc = hab[i].nHwnd++;

    } else {
       rc = -1;
    }
    __lxchg(&hab[i].Access,UNLOCKED);
    return rc;
}

/* Return:
   iHab+1 if hwnd is in _FreePM_hwnd hab[].pHwnd[]  list, else
   0
 */
int _FreePM_HAB::QueryHwnd(HWND hwnd)
{   int rc, indwind;
    rc = QueryIndexesHwnd(hwnd,indwind);
    if(rc > 0)
         return   hab[rc-1].iHAB + 1;
   return 0;
}

int _FreePM_HAB::QueryIndexesHwnd(HWND hwnd, int &indwind)
{   int i,j,is,rc;
    is = 0;
    for(i=0;i<n;i++)
    {  if(hab[i].pHwnd == NULL) continue;
       for(j=0;j < hab[i].nHwnd;j++)
       { if(hab[i].pHwnd[j].hwnd == hwnd)
         {  indwind = j;
            return   i + 1;
         }
       }
    }
   return 0;
}

/* Return:
   -1 - PMERR_INVALID_HWND
   -2  - bad iHAB
   i(nd)+1 in _FreePM_hwnd list pHwnd[] - if hwnd is in HAB, else
   0
 */
int _FreePM_HAB::QueryHwnd(HWND hwnd, int iHAB)
{  int rc,i, ind;
   rc = QueryHABexist(iHAB);
   if(rc != 1)
        return -2;
   ind = rc - 1;
   if(hab[ind].pHwnd == NULL)
           return -1;
    for(i=0;i < hab[ind].nHwnd;i++)
    { if(hab[ind].pHwnd[i].hwnd == hwnd)
                   return i+1;
    }
    return 0;
}

/*  удалить окно из списка окон клиентской задачи */
/* Return:
   -1 - PMERR_INVALID_HWND
   -2 - thread not initialized to FreePM (PMERR_INVALID_HAB)
    0 - Ok
 */
int _FreePM_HAB::DelHwnd(HWND hwnd)
{   int i,rc;
    int ordinal,tid,iHAB;
    rc = QueryHwnd(hwnd);
    if(rc <= 0)
       return -1;

    ordinal = QueryThreadOrdinal(tid);
    rc = QueryOrdinalUsed(ordinal,Q_ORDINAL_HAB);
    if(rc == -1)
       return -2;
    iHAB = rc;
    rc = DelHwnd(hwnd, iHAB);

    return rc;
}

int _FreePM_HAB::DelHwnd(HWND hwnd, int iHAB)
{   int i,rc,rc1,ind, rcS;

     rc1 = QueryHABexist(iHAB);
     if(rc1 != 1)
        return -2;
     ind = rc1 - 1;
   do
   {  rcS =  __lxchg(&hab[ind].Access,LOCKED);
      if(rcS) DosSleep(0);
   } while(rcS);     // доступ закрыт

    rc = QueryHwnd(hwnd,iHAB);
    if(rc > 0)
    {
        for(i=rc-1;i<hab[ind].nHwnd-1;i++)
        {   hab[ind].pHwnd[i] = hab[ind].pHwnd[i+1];
        }
        hab[ind].nHwnd--;
    }
    __lxchg(&hab[ind].Access,UNLOCKED);
    return 0;
}

