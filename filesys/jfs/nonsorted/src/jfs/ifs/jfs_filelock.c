/* $Id: jfs_filelock.c,v 1.1 2000/04/21 10:58:08 ktk Exp $ */

static char *SCCSID = "@(#)1.12  9/13/99 15:04:04 src/jfs/ifs/jfs_filelock.c, sysjfs, w45.fs32, fixbld";
/*****************************************************************************
 *
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 *
 *
 * Module: jfs_filelock.c
 *
 * External Functions:
 *                     uint32      jfs_lock
 *                     void        free_pending_locks
 *		       int32	   check_read_locks
 *
 * Description:  This module implements File level locking for JFS.
 *
 * Notes:
 *
 * Change History
 * Date      Name          Description
 * ---------------------------------------------------------------------------
 *
 *****************************************************************************/

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#include "jfs_inode.h"
#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_txnmgr.h"
#include "jfs_proto.h"
#include "jfs_debug.h"

extern struct InfoSegGDT *GIS;

/* Global vars  ***********************************************/

extern pool_t * lock_pool;         /* pool of lock structures        */

#define LOCK_EXPIRED   0x0L
#define LOCK_WAKEUP    0x1L
#define LOCK_CANCELED  0x2L


/****************************************************************************
 * NAME:        wake_pending_locks(start, length)
 *
 * FUNCTION:    searches the list of pending lock and wakes up those that are
 *              enabled
 *
 * PARAMETER:   start      - starting offset in file of new free range
 *              length     - length of range freed
 *              search_list - pointer to first item in list to be searched
 *
 * RETURN:      none
 *
 *****************************************************************************/

void wake_pending_locks(
	uint32	start,
	uint32	length,
	struct	file_lock * search_list)

{
  struct file_lock	*curr_lock;

  for (curr_lock = search_list; curr_lock != NULL;
       curr_lock = curr_lock->next_lock)
  {
    if (start > curr_lock->lock_end)
	break;

    /* do the lock ranges overlap ?? */
    if ((start + length - 1) >= curr_lock->lock_start)
    {
jEVENT(0,("Wake up pending_locks: start:%x, end:%x,anchor%x\n",
	curr_lock->lock_start,curr_lock->lock_end,curr_lock->anchor));

      KernWakeup((ULONG)curr_lock, /* addr of lock is eventid */
		WAKEUP_DATA, /* no flags */
		NULL,
		LOCK_WAKEUP); /* tell thread may get lock */
    }
  }

  return;
}


/****************************************************************************
 * NAME:        jfs_filelock
 *
 * FUNCTION:    Performs file locks and unlocks
 *
 * PARAMETER:   vp  - ptr to vnode for which lock is to be set
 *              lock_count - count of locks to perform
 *              lock_list  - ptr to array of locks to perform
 *              timeout    - timeout for locks; 0 if no wait, ffffffff if infinite
 *              unlock_count - count of unlocks to perform
 *              unlock_list  - ptr to array of unlocks to perform
 *              flags
 *
 *
 * RETURN:      ERROR_INTERUPT
 *              ERROR_LOCK_VIOLATION
 *              0
 ***************************************************************************/

uint32 jfs_filelock( struct vnode *vp,     /* object vnode          */
          struct file_lock * * lock_anchor,
          uint32 lock_count,                  /* lock count            */
          struct lock_info * lock_list,       /* list of locks         */
          uint32 timeout,                     /* timeout in ms         */
          uint32 unlock_count,                /* unlock count          */
          struct unlock_info * unlock_list,   /* list of unlocks       */
          uint16 pid,                         /* pid                   */
	  uint32 longlock)		      /* TRUE if longlong      */
{
  int32   rc = 0;
  struct inode	*ip;                   /* pointer to inode for file   */
  struct gnode	*gp;                   /* pointer to inode for file   */
  struct file_lock * curr_lock;               /* pointer to current lock     */
  struct file_lock * new_lock;                /* pointer to new     lock     */
  UCHAR  locks_allowed = TRUE;                /* true if no lock conflict    */
  uint32 i;                                   /* loop counter                */
  uint32 lock_to_block;                       /* index of lock to block on   */
  struct lock_infol *lock_listl;	      /* longlong lock list	     */
  struct unlock_infol *unlock_listl;	      /* longlong unlock list	     */
  int64  start;
  int64  length;
  int32  share;
  uint32 done;

  if (vp == 0)		/* DASD open */
	return ERROR_INVALID_HANDLE;

  gp = VP2GP(vp);                 /* get the gnode ptr         */
  ip = GP2IP(gp);                 /* get the inode ptr         */

  lock_listl = (struct lock_infol *)lock_list;
  unlock_listl = (struct unlock_infol *)unlock_list;

  IWRITE_LOCK(ip); // FILELOCK_LOCK(ip);

  /*
   *	1. unlock first
   */
  for (i = 0; i < unlock_count; i++)
  {
     rc = ERROR_LOCK_VIOLATION;  /* init error code */

     if (longlock)
     {
	start = unlock_listl[i].start;
	length = unlock_listl[i].length;
     }
     else
     {
	start = unlock_list[i].start;
	length = unlock_list[i].length;
     }
     /* Loop through locks on this handle looking for match */
     for (curr_lock = *lock_anchor; curr_lock != NULL;
          curr_lock = curr_lock->vnode_next_lock)
     {
       /*
	*  range to be unlocked must match exactly with existing lock:
        */
       if ((curr_lock->lock_start == start) &&
          (curr_lock->lock_end == (start + length - 1)) &&
           (curr_lock->pid == pid))
       {
         /* FOUND matching Lock */

         /*
          * delete from the per file locklist
          */
         if (curr_lock->prev_lock != NULL)
         {
           curr_lock->prev_lock->next_lock = curr_lock->next_lock;
         }
         else  /* fix head of list for 1st item */
         {
           gp->gn_filelocks = curr_lock->next_lock;
         }
         if (curr_lock->next_lock != NULL)
         {
           curr_lock->next_lock->prev_lock = curr_lock->prev_lock;
         }

         /*
          * delete from the per open locklist
          */
         if (curr_lock->vnode_prev_lock != NULL)
         {
           curr_lock->vnode_prev_lock->vnode_next_lock = curr_lock->vnode_next_lock;
         }
         else  /* fix head of list for 1st item */
         {
           *lock_anchor = curr_lock->vnode_next_lock;
         }
         if (curr_lock->vnode_next_lock != NULL)
         {
           curr_lock->vnode_next_lock->vnode_prev_lock = curr_lock->vnode_prev_lock;
         }

         /*
          * return the lock to the pool
          */
         freepool(lock_pool, (VOID *)curr_lock);
         rc = 0;
jEVENT(0,("Unlock success: start:%x len%x, anchor:%x,pid:%x\n",
	start, length, lock_anchor,pid));

         /*
          * wakeup threads sleeping on the lock if any
          */
         if (gp->gn_pendinglocks)
         {
           wake_pending_locks(start, length, gp->gn_pendinglocks); // D228334
         }

         break;
       }
     } /* end for each lock in list */

     if (rc != 0)
     {
jEVENT(0,("##Unlock FAIL: start:%x len:%x, anchor:%x,pid%x\n",
	start, length, lock_anchor,pid));

       goto out;
     }
  } /* end for each unlock requested */

  /*
   *	2. try to lock
   */
retryLock:
  locks_allowed = TRUE;

  /* test if all the locks in the list of lock can be acquired
   * atomically (FILEIO calls with a list of locks that must be 
   * set atomically);
   */
  for (i = 0; (locks_allowed == TRUE) && (i < lock_count); i++)
  {
     if (longlock)
     {
	start = lock_listl[i].start;
	length = lock_listl[i].length;
	share = lock_listl[i].share;
     }
     else
     {
	start = lock_list[i].start;
	length = lock_list[i].length;
	share = lock_list[i].share;
     }
    for (curr_lock = gp->gn_filelocks; curr_lock != NULL;
         curr_lock = curr_lock->next_lock)
     {
      /* do the lock ranges overlap ?? */
      if ((start <= curr_lock->lock_end)  &&
           ((start + length - 1)
           >= curr_lock->lock_start))
      {
        /* if either lock is exclusive, fail */
        if (!share || !curr_lock->share)
        {
          /* this lock would violate an existing lock */
          locks_allowed = FALSE;
          lock_to_block = i;
          break;                   /* end for loop */
        }
      }
    } /* end for */
  } /* end for each lock requested */

  /*
   * 2.1 all requested locks can be acquired:
   */
  if (locks_allowed)
  {
    for (i = 0; i < lock_count; i++)
    {
      if (longlock)
      {
 	start = lock_listl[i].start;
	length = lock_listl[i].length;
	share = lock_listl[i].share;
      }
      else
      {
	start = lock_list[i].start;
	length = lock_list[i].length;
	share = lock_list[i].share;
      }
      /*
       * alloc/init a lock for range to be locked;
       */
      /* get a lock structure from the pool */
      new_lock = (struct file_lock *)allocpool(lock_pool, 0);
      if (!new_lock)  /* OUT OF MEMORY */
      {
        rc = ERROR_SHARING_BUFFER_EXCEEDED;
	goto out;
      }

      /* fill in lock info */
      new_lock->lock_start = start;
      new_lock->lock_end = start + length - 1;
      new_lock->share = share;
      new_lock->anchor = lock_anchor;
      new_lock->pid = pid;
jEVENT(0,("Lock Created: start:%x len:%x,anchor %x, share:%x, pid:%x\n",
	start, length, lock_anchor, share, pid));
      /* time fields do not need to be filled in for active locks */

      /*
       * insert the lock to the locklists 
       * in ascending order of start offset of the range;
       */
      /* is this the first lock on the per file locklist ? */
      if (gp->gn_filelocks == NULL)
      {
        /* insert the lock to the per file locklist */
        gp->gn_filelocks = new_lock;
        new_lock->next_lock = NULL;
        new_lock->prev_lock = NULL;

        /* insert the lock to the per open locklist */
        *lock_anchor = new_lock;
        new_lock->vnode_next_lock = NULL;
        new_lock->vnode_prev_lock = NULL;
      }
      /* there are locks on the per file locklist */
      else
      {
        /*
         * insert the lock to the per file locklist
         */
        for (curr_lock = gp->gn_filelocks; curr_lock != NULL;
             curr_lock = curr_lock->next_lock)
        {
          if (curr_lock->lock_start >= start)
          {
            /* is curr_lock at head ? */
            if (gp->gn_filelocks == curr_lock)
            {
              /* insert as the first item */
              gp->gn_filelocks = new_lock;
              curr_lock->prev_lock = new_lock;
              new_lock->next_lock = curr_lock;
              new_lock->prev_lock = NULL;
              break;
            }
            else
            {
              /* add to middle or end of list */
              new_lock->prev_lock = curr_lock->prev_lock;
              curr_lock->prev_lock = new_lock;
              new_lock->next_lock = curr_lock;
              new_lock->prev_lock->next_lock = new_lock;
              break;
            }
          }
          else
          {
            /* is curr_lock at tail ? */
            if (curr_lock->next_lock == NULL)  
            {
              new_lock->prev_lock = curr_lock;
              curr_lock->next_lock = new_lock;
              new_lock->next_lock = NULL;
              break;
            }
          }
        } /* end for each lock structure in gnode */

        /*
         * insert the lock to the per open locklist
         * for fast path checking during reads and writes;
         */
        if (*lock_anchor == NULL)
        {
          *lock_anchor = new_lock;
          new_lock->vnode_next_lock = NULL;
          new_lock->vnode_prev_lock = NULL;
        }
        else
        {
	  /*
           * WIP: the insertion point in per open locklist may be found 
           * while scanning the per file locklist above since all locks
           * of each per open locklist are also on the per file locklis; 
           */
          for (curr_lock = *lock_anchor; curr_lock != NULL;
               curr_lock = curr_lock->vnode_next_lock)
          {
            if (curr_lock->lock_start >= start)
            {
              /* insert berfore this item */
              if (*lock_anchor == curr_lock)
              {
                /* insert as the first item */
                *lock_anchor = new_lock;
                curr_lock->vnode_prev_lock = new_lock;
                new_lock->vnode_next_lock = curr_lock;
                new_lock->vnode_prev_lock = NULL;
                break;
              }
              else
              {
                /* add to middle  */
                new_lock->vnode_prev_lock = curr_lock->vnode_prev_lock;
                curr_lock->vnode_prev_lock = new_lock;
                new_lock->vnode_next_lock = curr_lock;
                new_lock->vnode_prev_lock->vnode_next_lock = new_lock;
                break;
              }
            }
            else
            {
              if (curr_lock->vnode_next_lock == NULL)  /* if last in list */
              {                                  /* add after curr item */
                new_lock->vnode_prev_lock = curr_lock;
                curr_lock->vnode_next_lock = new_lock;
                new_lock->vnode_next_lock = NULL;
                break;
              }
            }
          } /* end for each vnode lock */
        }
      } /* end else gnode lock list not null */
    } /* end for each lock in list */
  } /* end if locks_allowed */
  /*
   * 2.2 one or more locks could NOT be acquired:
   */
  else
  {
    /*
     * 2.2.1 caller wants to wait to acquire lock:
     */
    if (timeout)
    {
      /*
       * insert first locked-out range to the per file pending locklist
       * in ascending order of start offset of the range;
       */
      /* get a lock structure from the pool */
      new_lock = (struct file_lock *)allocpool(lock_pool,0);
      if (!new_lock)  /* OUT OF MEMORY */
      {
        rc = ERROR_SHARING_BUFFER_EXCEEDED;
	goto out;
      }

      /* fill in lock info */
      new_lock->pid = pid;
      if (longlock)
      {
	new_lock->lock_start = lock_listl[lock_to_block].start;
	new_lock->lock_end = lock_listl[lock_to_block].start + lock_listl[lock_to_block].length - 1;
	new_lock->share = lock_listl->share;
      }
      else
      {
	new_lock->lock_start = lock_list[lock_to_block].start;
	new_lock->lock_end = lock_list[lock_to_block].start + lock_list[lock_to_block].length - 1;
	new_lock->share = lock_list->share;
      }
      new_lock->anchor = lock_anchor;
      new_lock->time_remaining = timeout;
        new_lock->time_start = GIS->SIS_MsCount;
jEVENT(0,("Time: infoseg:%x ms:%x\n",GIS, new_lock->time_start));
jEVENT(0,("Lock Pending: start:%x len%x,anchor %x,timeout:%x.Eid:%x\n",
	lock_list[lock_to_block].start,lock_list[lock_to_block].length,
	lock_anchor,timeout,new_lock));

      /*
       * insert to the per file pending locklist
       */
      if (gp->gn_pendinglocks == NULL)
      {
        gp->gn_pendinglocks = new_lock;
        new_lock->next_lock = NULL;
        new_lock->prev_lock = NULL;
        new_lock->vnode_next_lock = NULL;
        new_lock->vnode_prev_lock = NULL;
      }
      else
      {
        for (curr_lock = gp->gn_pendinglocks; curr_lock != NULL;
             curr_lock = curr_lock->next_lock)
        {
          if (curr_lock->lock_start >= new_lock->lock_start)	// D228334
          {
            /* insert berfore this item */
            if (gp->gn_pendinglocks == curr_lock)
            {
              /* insert as the first item */
              gp->gn_filelocks = new_lock;
              curr_lock->prev_lock = new_lock;
              new_lock->next_lock = curr_lock;
              new_lock->prev_lock = NULL;
              break;
            }
            else
            {
              /* add to middle or end of list */
              new_lock->prev_lock = curr_lock->prev_lock;
              curr_lock->prev_lock = new_lock;
              new_lock->next_lock = curr_lock;
              new_lock->prev_lock->next_lock = new_lock;
              break;
            }
          }
          else
          {
            /* if last in list, add after curr item */
            if (curr_lock->next_lock == NULL)
            {
              new_lock->prev_lock = curr_lock;
              curr_lock->next_lock = new_lock;
              new_lock->next_lock = NULL;
              break;
            }
          }
        } /* end for each lock structure in gnode */
      }

      /*
       * wait for timout to see if lock frees
       */
      done = LOCK_EXPIRED;
      rc = KernBlock((ULONG)new_lock, /* eventid */
                 new_lock->time_remaining, /* timeout */
                 T_WRITELOCK,
                 &ip->i_rdwrlock,
                 (ULONG*)&done); /* wakeup reason */

jEVENT(0,("Return KernBlock: done:%x rc:%x\n",done,rc));

      /*
       * remove lock from the pending list
       */
      if (new_lock->prev_lock != NULL)
      {
        new_lock->prev_lock->next_lock = new_lock->next_lock;
      }
      else  /* fix head of list for 1st item */
      {
        gp->gn_pendinglocks = new_lock->next_lock;
      }
      if (new_lock->next_lock != NULL)
      {
        new_lock->next_lock->prev_lock = new_lock->prev_lock;
      }

      /*
       * return the lock struct to the pool
       */
      freepool(lock_pool, (VOID *)new_lock);      

      /* fixup timeout before returning lock structure */
      if (timeout != 0xffffffff)
      {
        if (new_lock->time_start < GIS->SIS_MsCount)
        {
          timeout = timeout - (GIS->SIS_MsCount - new_lock->time_start);
        }
        else  /* ms counter wrapped */
        {
          timeout = timeout -
          (0xffffffff - new_lock->time_start + GIS->SIS_MsCount);
        }
      }

      switch (done)
      {
      case LOCK_WAKEUP:
        goto retryLock;
      case LOCK_EXPIRED:
         rc = ERROR_LOCK_VIOLATION;
	 break;
      case LOCK_CANCELED:
         rc =ERROR_INTERRUPT;
	 break;
      default:
         rc = ERROR_LOCK_VIOLATION;
      }
    }
    /*
     * 2.2.2 caller does NOT want to wait to acquire lock:
     */
    else
    {
      rc = ERROR_LOCK_VIOLATION;
    }
  } /* end else lock not obtained */

out:
  IWRITE_UNLOCK(ip); // FILELOCK_UNLOCK(ip);

  return (rc);
}

/****************************************************************************
 * NAME:        check_read_locks
 *
 * FUNCTION:    searches the locks on an inode to verify whether or not a
 *              read is allowed
 *
 * PARAMETER:   gp - gnode pointer
 *		offset - offset of read
 *		length - length of read
 *		pid - Process's ID
 *		anchor_ptr - pointer to lock_anchor
 *
 * RETURN:      0 - Read allowed
 *		ERROR_LOCK_VIOLATION - Read disallowed
 *
 *****************************************************************************/

int32 check_read_locks(
	struct gnode		*gp,
	int64			offset,
	int64			length,
	unsigned short		pid,
	struct file_lock	**anchor_ptr)
{
	struct file_lock	*curr_lock;

	/* search per open locklist if the thread locked range */
	for (curr_lock = *anchor_ptr; curr_lock != NULL;
	     curr_lock = curr_lock->vnode_next_lock)
	{
		if (offset < curr_lock->lock_start)
			break;
		if ((offset <= curr_lock->lock_end) && (curr_lock->pid == pid))
		{
			if (offset + length <= curr_lock->lock_end)
				return 0;
			length = curr_lock->lock_end - offset;
			offset = curr_lock->lock_end + 1;
		}
	}

	/* search the per file locklist if the range is locked */
	for (curr_lock = gp->gn_filelocks; curr_lock != NULL;
	     curr_lock = curr_lock->next_lock)
	{
		/* do the lock ranges overlap ?? */
		if ((offset <= curr_lock->lock_end)  &&
		    ((offset + length) >= curr_lock->lock_start))
		{
			/* all shared are OK, */
			/* if lock type exclusive, and not right anchor or pid*/
			if (( !curr_lock->share) &&
			    ((curr_lock->anchor != anchor_ptr) ||
			     (curr_lock->pid != pid)))
				return ERROR_LOCK_VIOLATION;
		}
	}
	return 0;
}
