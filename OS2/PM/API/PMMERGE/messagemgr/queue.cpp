/*
 $Id: F_messages.cpp,v 1.3 2002/09/06 14:59:33 evgen2 Exp $
*/
/* F_messages.cpp  */
/* class  FreePM_Queue stuff */
/* ver 0.00 16.08.2002       */


#define INCL_DOSPROCESS
#include "osfree.h"

#include <queue.hpp>

#include <builtin.h>
/* Add message to queue */
/* rc = 0 - Ok
   rc = 1 - No space left
   rc = 2 - pmsg is NULL
   rc = 3 - First message was overwrited for circle queue
*/
int FreePM_Queue::Add(PQMSG pmsg)
{   int ind,rc=0,rcS;
    if(pmsg == NULL)
           return 2;
#if USE_CIRCLE_QUEUES
   do
   {  rcS =  __lxchg(&Access,LOCKED);
      if(rcS) DosSleep(0);
   } while(rcS);     // доступ закрыт

    ind = (start + length) % lQueuesize;
    queue[ind] = *pmsg;
    if(length < lQueuesize)
              length++;
    else
              rc = 3;
#else
    if(length >= lQueuesize)
              return 1;  /* No space left */
   do
   {  rcS =  __lxchg(&Access,LOCKED);
      if(rcS) DosSleep(0);
   } while(rcS);     // доступ закрыт

    ind = (start + length) % lQueuesize;
    queue[ind] = *pmsg;
    length++;
#endif

    __lxchg(&Access,UNLOCKED);

    return rc;
}
/* Delete message index ind from queue */
/* rc = 0 - Ok
   rc = 1 - no message (bad ind)
*/
int FreePM_Queue::Del(int ind)
{   int i,l, i_ind,i_ind1,rcS;
    if(length == 0)
             return 1;
   do
   {  rcS =  __lxchg(&Access,LOCKED);
      if(rcS) DosSleep(0);
   } while(rcS);     // доступ закрыт

   if(ind == start)
   {  start++; length--;
      start = (start + 1) % lQueuesize;
   } else {
      if(ind < start)
           l = ind + lQueuesize - start;
      else
           l = ind - start;
      for(i=0;i < l; i++)
      {  i_ind  = (ind + i)   % lQueuesize;
         i_ind1 = (ind + i+1) % lQueuesize;
         queue[i_ind] = queue[i_ind1];
      }
      length--;
   }
    __lxchg(&Access,UNLOCKED);
   return 0;
}

/* Delete first message ind from queue */
/* rc = 0 - Ok
   rc = 1 - no messages
*/
int FreePM_Queue::DelFirst(void)
{  int rcS;
   if(length == 0)
             return 1;
   do
   {  rcS =  __lxchg(&Access,LOCKED);
      if(rcS) DosSleep(0);
   } while(rcS);     // доступ закрыт

    start++; length--;
    start = (start + 1) % lQueuesize;

    __lxchg(&Access,UNLOCKED);

    return 0;
}

/* Get and Delete first message ind from queue */
/* rc = 0 - Ok
   rc = 1 - no messages
   rc = 2 - pmsg is NULL
*/

int FreePM_Queue::Get(PQMSG pmsg)
{  int rcS;
   if(length == 0)
             return 1;
    if(pmsg == NULL)
           return 2;
   do
   {  rcS =  __lxchg(&Access,LOCKED);
      if(rcS) DosSleep(0);
   } while(rcS);     // доступ закрыт

    *pmsg = queue[start];

    start++; length--;
    start = (start + 1) % lQueuesize;

    __lxchg(&Access,UNLOCKED);

    return 0;
}

