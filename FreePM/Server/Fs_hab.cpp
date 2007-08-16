/*
 $Id: Fs_hab.cpp,v 1.2 2002/09/10 10:22:53 evgen2 Exp $
*/
/* Fs_hab.cpp */
/* server hab manager  */
/* ver 0.00 7.09.2002       */
/* DEBUG: section 5   hab manager */

#include <malloc.h>
#include <memory.h>
#include <builtin.h>
#include <time.h>
#include "FreePM.hpp"
#include "Fs_queue.hpp"
#include "Fs_hab.hpp"
#define F_INCL_DOSPROCESS
   #include "F_OS2.hpp"

#include "Fs_WND.hpp"

/* for debug()*/
#include <stdio.h>
#include "F_globals.hpp"

#include "Fs_globals.hpp"

#define LOCK_HABSERVER                          \
    {   int ilps_raz = 0, ilps_rc;              \
        do                                      \
        {  ilps_rc =  __lxchg(&Access,LOCKED);  \
           if(ilps_rc)                          \
           { if(++ilps_raz  < 3)  DosSleep(0);  \
             else            DosSleep(1);       \
           }                                    \
        } while(ilps_rc);          \
    }

#define UNLOCK_HABSERVER  {__lxchg(&Access,UNLOCKED);}

int _FreePM_HAB_serverlist::Add(int &iClientId, int threadNum)
{  int rc,ind;
   double dt;
static int clientCount=0;
LOCK_HABSERVER;
   rc = GetFreeOldest(dt);
   if(rc != -1)
   {  if(dt < _FreePMconfig.Timeout_connection) rc = -1;
   }
   if(rc == -1)  /* не нашли пустое место */
   { //todo   GetFree(1);
       if(n+1 >= nAlloced)
       {  nAlloced += 256;
          if(pHabSvrInf == NULL)
          {       pHabSvrInf = (_FreePM_HABserverinf *) calloc(nAlloced, sizeof(_FreePM_HABserverinf));
           /*  pHabSvrInf[0] is formal, not realy used,  may will use it for "master" hab/thread ? */
                  memset((void *)&pHabSvrInf[0],0,sizeof(_FreePM_HABserverinf));
                  n = 1;
          }
          else
                  pHabSvrInf = (_FreePM_HABserverinf *) realloc((void *)pHabSvrInf, nAlloced* sizeof(_FreePM_HABserverinf));
       }
       ind = n++;
   } else {
       ind = rc;
   }
   pHabSvrInf[ind].used = 1;
   pHabSvrInf[ind].nsrvthread = threadNum;
   pHabSvrInf[ind].clientId = iClientId = clientCount++;
   pHabSvrInf[ind].disconnect = time(NULL);

UNLOCK_HABSERVER;
   return ind;
}

int _FreePM_HAB_serverlist::Free(int iHab)
{
   if(iHab < 0 || iHab >= n)
              return -1;
LOCK_HABSERVER;
   pHabSvrInf[iHab].used = 0;
   pHabSvrInf[iHab].disconnect = time(NULL);
UNLOCK_HABSERVER;
   return 0;
}


/* найти свободный элемент в списке */
/* mode = 0 don't use time;
   mode = 1 - use the most old
   return
   -1  - no free
*/
//todo mode=1
int _FreePM_HAB_serverlist::GetFree(void)
{  int i,isfree=0,rc=-1;
   for(i=0;i < n; i++)
   {  if(!pHabSvrInf[i].used)
      { isfree = 1;
        rc = i;
        break;
      }
   }
   return rc;
}

/* search for the oldest element in list
   return
   -1  - no free
*/
int _FreePM_HAB_serverlist::GetFreeOldest(double &dtmax)
{  int i,isfree=0,rc=-1;
   double d;
   time_t t;
   t = time(NULL);
/* index 0 not used. (or can't be free ?) */
   for(i=1;i < n; i++)
   {  if(!pHabSvrInf[i].used)
      { d = difftime(t, pHabSvrInf[i].disconnect);
        if(!isfree || d > dtmax)
        {  dtmax = d;
           rc = i;
           isfree++;
        }
      }
   }

   return rc;
}
