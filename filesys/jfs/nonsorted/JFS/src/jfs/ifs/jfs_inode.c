/* $Id: jfs_inode.c,v 1.2 2003/09/21 08:42:57 pasha Exp $ */

static char *SCCSID = "@(#)1.39  11/1/99 13:37:43 src/jfs/ifs/jfs_inode.c, sysjfs, w45.fs32, fixbld";
/*
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
 */

/*
 * Change History :
 * PS 2001-2003 apply IBM fixes
 *
 */

/*
 *  jfs_inode.c: in-memory inode/vnode cache manager
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif  /* _JFS_OS2 */

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_cachemgr.h"
#include "jfs_txnmgr.h"
#include "jfs_imap.h"
#include "jfs_proto.h"
#include "jfs_debug.h"
#include "mmph.h"

/*
 * combined vnode/inode objects
 */
struct xnode {
    struct vnode    x_vnode;
    struct inode    x_inode;
};

/*
 *  inode cache
 */
extern uint32   openMax;
int32   maxiCache;      /* maximum # of inodes in cache */
pool_t  *iCache;        /* inode cache pool allocation structure */

/*
 *  inode lru freelist
 *
 * offsets of i_flnext and i_flprev must be same as in the inode
 */
int32   minifreelist;       /* minimum # of inodes in freelist */
int32   nifreelist = 0;     /* current number of inodes in freelist */
struct {
    struct inode *i_hlnext;     /* hashlist links, not used */
    struct inode *i_hlprev;

    struct inode *i_flnext;     /* freelist links */
    struct inode *i_flprev;
} ifreelist;

#define FREELIST_INSERT_HEAD(fip, ip)\
{\
    (ip)->i_flnext = (fip)->i_flnext;   \
    (ip)->i_flprev = (struct inode *)(fip); \
    (fip)->i_flnext->i_flprev = ip;     \
    (fip)->i_flnext = ip;           \
}

#define FREELIST_INSERT_TAIL(fip, ip)\
{\
    (ip)->i_flnext = (struct inode *)(fip); \
    (ip)->i_flprev = (fip)->i_flprev;   \
    (fip)->i_flprev->i_flnext = ip;     \
    (fip)->i_flprev = ip;           \
}

#define FREELIST_REMOVE(ip)\
{\
    (ip)->i_flprev->i_flnext = (ip)->i_flnext;  \
    (ip)->i_flnext->i_flprev = (ip)->i_flprev;  \
}

#define FREELIST_SELF(ip)\
    (ip)->i_flnext = (ip)->i_flprev = (ip)

/*
 *  inode hashlist anchor table
 */
int32   nhinode;    /* number of hashlist */
struct hinode *ihashtbl;

/* inode cache list management macros */
#define HASHLIST_INSERT_HEAD(hip, ip)\
{\
    (ip)->i_hlnext = (hip)->i_hlnext;   \
    (ip)->i_hlprev = (struct inode *)(hip); \
    (hip)->i_hlnext->i_hlprev = ip;     \
    (hip)->i_hlnext = ip;           \
}

#define HASHLIST_REMOVE(ip)\
{\
    (ip)->i_hlprev->i_hlnext = (ip)->i_hlnext;\
    (ip)->i_hlnext->i_hlprev = (ip)->i_hlprev;\
}

#define HASHLIST_SELF(ip)\
    (ip)->i_hlnext = (ip)->i_hlprev = (ip)


/*
 *  inode cache synchronization
 */
int32   ilock_cnt = 0;      /* inode lock occurrence */

TSPINLOCK_T jfsICacheLock;

#define ICACHE_LOCK_ALLOC()\
    TSPINLOCK_ALLOC(&jfsICacheLock,LOCK_ALLOC_PAGED,JFS_ICACHE_LOCK_CLASS,\
            -1)
#define ICACHE_LOCK_INIT()  TSPINLOCK_INIT(&jfsICacheLock)
/* ICACHE_LOCK()/ICACHE_UNLOCK(): ref. jfs_lock.h */
#define ICACHE_XBLOCK()     TSPINLOCK_XBLOCK(&jfsICacheLock)
#define ICACHE_XSPIN()      TSPINLOCK_XSPIN(&jfsICacheLock)
#define ICACHE_SLEEP(ip)\
    TEVENT_SLEEP(&ip->i_event, &jfsICacheLock, T_TSPINLOCK)
#define ICACHE_WAKEUP(ip)   TEVENT_WAKEUP(&ip->i_event)

/*
 *  inode readers/writer lock
 */
#define IRDWRLOCK_ALLOC(ip,index)\
    RDWRLOCK_ALLOC(&ip->i_rdwrlock,LOCK_ALLOC_PAGED,JFS_IRDWR_LOCK_CLASS,index)
#define IRDWRLOCK_INIT(ip)  RDWRLOCK_INIT(&ip->i_rdwrlock)
/* IREAD_LOCK()/IREAD_UNLOCK(): ref. jfs_Lock.h */
/* IWRITE_LOCK()/IWRITE_UNLOCK()/IWRITE_LOCK_TRY(): ref. jfs_Lock.h */

/*
 *  inode field access lock
 */
#define INODE_LOCK_ALLOC(ip,index)\
    SPINLOCK_ALLOC(&ip->i_nodelock,LOCK_ALLOC_PAGED,JFS_INODE_LOCK_CLASS,index)
#define INODE_LOCK_INIT(ip) SPINLOCK_INIT(&ip->i_nodelock)
/* INODE_LOCK()/INODE_UNLOCK(): ref. jfs_lock.h */

/*
 *  sync() control
 */
/*
 * sync() time control:
 *
 * LazyOff: Perform all writes synchronously;
 * SyncTime: sync daemon interval;
 * maxSyncTime: max time from first modification: SyncTime << 2;
 * minSyncTime: min time from last modification: SyncTime >> 3;
 */
int32   LazyOff = 0;
int32   SyncTime = 64;
int32   maxSyncTime = 256;
int32   minSyncTime = 8;

MUTEXLOCK_T     jfsSyncLock;
#define SYNC_LOCK()     MUTEXLOCK_LOCK(&jfsSyncLock)
#define SYNC_UNLOCK()       MUTEXLOCK_UNLOCK(&jfsSyncLock)

/*
 * external references
 */
int freeZeroLink(inode_t *ip);

#ifdef  _JFS_OS2
extern struct InfoSegGDT *GIS;

#define curtime(t)          \
{                   \
    (t)->tv_sec = GIS->SIS_BigTime; \
    (t)->tv_nsec = 0;       \
}
#endif  /* _JFS_OS2 */

/*
 * forward references
 */
static int32 getip(struct inode **ipp);
int32 iuncache(struct inode *ip, uint32 flag);
static void iClose(struct inode *ip);
static void vGet(struct inode *ip, struct vfs *vfsp);
void vPut(struct inode *ip);


#ifdef  _JFS_STATISTICS
/*
 *  statistics
 */
struct statICache {
    uint32  ninode;
    uint32  reclaim;
    uint32  recycle;
    uint32  release;
} statICache = {0};
#endif  /* _JFS_STATISTICS */


/*
 * NAME:    initicache()
 *
 * FUNCTION:    Initialize xnode (vnode+inode) table
 *
 * PARAMETERS:  None
 *
 * RETURN : None
 */
initicache()
{
    extern struct fsvar fsv;

    struct hinode *hip;
    struct inode *ip;
    int n, rc, i;
    uint memsize;
    uint k;

        /* Initialize the inode cache lock */
    ICACHE_LOCK_ALLOC();
    ICACHE_LOCK_INIT();

    /*
     * allocate inode cache pool
     */
    /*
     * minifreelist - min number of inodes in lru freelist
     * before to recycle from the list:
     * scale to real memory size: 1% of real memory size;
     */
    /* round up memsize to the nearest Meg */
    memsize = (MemorySize + ((1024*1024)-1) & ~(1024*1024));
    minifreelist = (memsize/100)/sizeof(struct xnode);

    /*
     * maxicache - max number of inodes available, i.e., max number
     * of files open at any one time (inode pool size):
     * scale to real memory size: 2% of real memory size;
     */
    maxiCache = minifreelist * 2;
    openMax = maxiCache;

    rc = initpool(maxiCache,    /* max number of inodes/pool size */
             512,       /* init number of inodes to allocate */
             sizeof(struct xnode),
             0,
             "ICACHE",
             &iCache,
             0);
    if (rc)
        return rc;

    /*
     * allocate inode hashlist anchor table.
     *
     * scale the number of hash classes s.t. hashlist size is less
     * than 4 for maciCache;
     */
    nhinode = maxiCache / 4;
    /* round up to next smallest power of 2 */
    nhinode = roundup2power(nhinode);
    if ((ihashtbl =
         (struct hinode *)xmalloc(nhinode * sizeof(struct hinode),
                                      0, kernel_heap|XMALLOC_HIGH)) == NULL)
        return ENOMEM;

    /*
     * initialize the hashlist anchor table
     */
    for (hip = ihashtbl; hip < &ihashtbl[nhinode]; hip++)
    {
        hip->i_hlnext = (struct inode *)hip;
        hip->i_hlprev = (struct inode *)hip;
        hip->i_timestamp = 0;
        hip->i_vget = 0;
    }

    /*
     * initialize lru freelist
     */
    ifreelist.i_flnext = (struct inode *) &ifreelist;
    ifreelist.i_flprev = (struct inode *) &ifreelist;

jFYI(1,("iCache: minFreelist:%d maxCache:%d\n", minifreelist, maxiCache));
    return 0;
}


/*
 * NAME:    imark()
 *
 * FUNCTION:    update i_flag and accessed, changed, or updated times
 *      in an inode.
 *
 * PARAMETERS:  ip  - pointer to inode
 *      flag    - inode times to set
 *
 * RETURN : None
 *
 * NOTE:    readers hold INODE_LOCK() to serialize update of i_atime
 *      on entry/exit
 */
void imark(
    struct inode *ip,
    int32   flag)
{
    struct timestruc_t t;

    if (isReadOnly(ip))
            return;

    curtime(&t);    /* fetch current time */

    if ((ip->i_flag & (IACC|ICHG|IUPD)) == 0)
        ip->i_synctime = t.tv_sec;

    ip->i_flag |= flag;

    if (flag & IACC)
        ip->i_atime = t;

    if (flag & IUPD)
        ip->i_mtime = t;

    if (flag & ICHG)
        ip->i_ctime = t;

}


#ifdef  _JFS_OS2
/*
 * NAME:    iAccess(ip, mode, crp)
 *
 * FUNCTION:    Check mode permission on inode.
 *      The only interesting bit is READONLY.  Write
 *      permission is denied if the bit is set.
 *
 * PARAMETER:   ip  - inode to check
 *      mode    - mode to check(rwx)
 *
 * RETURN:  EACCES  - if permission requested is denied
 *
 * SERIALIZATION: read/write inode lock held on entry/exit.
 */
iAccess(
    inode_t     *ip,
    int32       m,
    struct ucred    *crp)
{
    if (m & IWRITE)
    {
        if (ip->i_ipmnt->i_iplog == NULL)
            return EROFS;   /* no such beast, but maybe someday */

                if (ip->i_mode & IREADONLY)
            return EACCES;
    }

    /* check for existence, read access, and exec access
     * always return success
     */
    return 0;
}
#endif  /* _JFS_OS2 */

// BEGIN F226941
/*
 * NAME:    sort_and_lock()
 *
 * FUNCTION:    Lock variable number of inodes in descending i_number
 *
 * PARAMETERS:  ip  - optional inode not in list
 *      n   - number of inodes in list
 *      ilist   - array of inodes
 *
 * RETURN : None
 */
void    sort_and_lock(
    inode_t *ip,
    int32   n,
    inode_t **ilist)
{
    int32   k, m;
    int32   free_it = 0;
    inode_t *ipl_stack[64];
    inode_t **ipl = ipl_stack;

    if ((n + (ip != 0)) > 64)
    {
        ipl = (inode_t **)xmalloc((n+1)*sizeof(inode_t **), 0,
                      kernel_heap);
        free_it = 1;
    }

    for (k = 0; k < n; k++)
        ipl[k] = *ilist++;
    if (ip)
        ipl[n++] = ip;

    /* bubble sort inodes in descending i_number */
    do
    {
        m = 0;
        for (k = 0; k < n; k++)
            if ((k+1) < n && ipl[k+1]->i_number > ipl[k]->i_number)
            {
                ip = ipl[k];
                ipl[k] = ipl[k+1];
                ipl[k+1] = ip;
                m++;
            }
    } while (m);

    /*
     * lock each object in the list
     */
retry:
    for (k = 0; k < n; k++)
    {
        ip = ipl[k];
        MMPHPreIWRITE_LOCK(ip);             // D230860
        WRITE_LOCK(&ip->i_rdwrlock);
        MMPHPostIWRITE_LOCK();              // D230860

        if (ip->i_xlock & FSXLOCK)
        {
            /* release back locks */
            for (m = 0; m < k; m++)
                IWRITE_UNLOCK(ipl[m]);

            ip->i_xlock |= FSXWANT;
            /* release lock */
            EVENT_SLEEP(&ip->i_fsevent, &ip->i_rdwrlock, T_WRITELOCK|T_NORELOCK);

            goto retry;
        }
    }
    if (free_it)
        xmfree((void *)ipl, kernel_heap);
}

/*
 * NAME:    iwritelocklist()
 *
 * FUNCTION:    Lock variable number of inodes in descending i_number
 *
 * PARAMETERS:  n    - number of inodes
 *      va_ilist - varags inode list
 *
 * RETURN : None
 */
void __cdecl iwritelocklist(n, va_ilist)
int n;              /* Number of elements   */
va_list va_ilist;       /* Varargs list     */
{
    sort_and_lock(0, n, (inode_t **) &va_ilist);
}
// END F226941

/*
 * NAME:    ireadlockx()
 *
 * FUNCTION:    wrapper to lock inode
 *
 * PARAMETERS:  ip  - inode we want to lock
 *
 * RETURN : None
 */
void ireadlockx(struct inode *ip)
{
    IREAD_LOCK(ip);
}


/*
 * NAME:    ireadunlockx()
 *
 * FUNCTION:    wrapper to unlock inode
 *
 * PARAMETERS:  ip  - inode we want to unlock
 *
 * RETURN : None
 */
void ireadunlockx(struct inode *ip)
{
    IREAD_UNLOCK(ip);
}


/*
 * NAME:    iwritelockx()
 *
 * FUNCTION:    wrapper to lock inode
 *
 * PARAMETERS:  ip  - inode we want to lock
 *
 * RETURN : None
 */
void iwritelockx(struct inode *ip)
{
    IWRITE_LOCK(ip);
}


/*
 * NAME:    iwriteunlockx()
 *
 * FUNCTION:    wrapper to unlock inode
 *
 * PARAMETERS:  ip  - inode we want to unlock
 *
 * RETURN : None
 */
void iwriteunlockx(struct inode *ip)
{
    IWRITE_UNLOCK(ip);
}


/*
 * NAME:    iSearchHashlist()
 *
 * FUNCTION:    get the specified inode from the inode hash
 *
 *      reference of vnode and/or inode is acquired but not locked.
 *
 * PARAMETERS:  dev - device of wanted inode
 *      fileset - fileset number
 *      ino - inode number
 *      ipp - returned inode
 *
 * RETURN : 0   - a match was found
 *      != 0    - no match was found
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */
static iSearchHashlist(
    struct hinode *hip, /* pointer into inode hash table */
    dev_t   dev,
    int32   fileset,
    ino_t   ino,
    struct inode **ipp)
{
    struct inode *ip;   /* cursor into inode hash list */
    int32   xlocked;

retry:
    /*
     * search the hash list for the specified inode:
     */
    for (ip = hip->i_hlnext; ip != (struct inode *)hip; ip = ip->i_hlnext)
    {
        if (ip->i_number == ino &&
            ip->i_dev == dev &&
            ip->i_fileset == fileset)
        {
            /*
             *  cache hit:
             *
             * acquire a reference of the inode.
             */
            ip->i_count++;

            /*
             * if the inode is currently being in transition
             * by another thread (e.g., being initialized,
             * recycled, deleted), wait for the completion of
             * transition.
             */
            xlocked = 0;
            while (ip->i_xlock & IXLOCK)
            {
                xlocked = 1;
                ip->i_xlock |= IXWANT;
                ICACHE_SLEEP(ip);

                /*
                 * if the inode has been invalidated,
                 * release its reference (the inode will be
                 * freed at last reference release), and
                 * try again.
                 */
                if (ip->i_mode == 0)
                {
                    iput(ip, NULL);
                    goto retry;
                }
            }

            /*
             * remove from freelist if it is on freelist;
             */
            if (ip->i_count == 1 && !xlocked)
            {
                nifreelist--;
                FREELIST_REMOVE(ip);
                FREELIST_SELF(ip);
            }

            *ipp = ip;
#ifdef FreeJFS
            INCREMENT(statICache.reclaim);
#endif
            return 0;
        }

    }

    /*
     *  cache miss:
     */
    *ipp = 0;

    return -1;
}


/*
 * NAME:    iget()
 *
 * FUNCTION:    get the specified inode;
 *      reference of inode is acquired but not locked.
 *
 * PARAMETERS:  vfsp != NULL, this is an object in a filesystem.
 *           == NULL, this is a filesystem or log device.
 *      ino - inode number;
 *      ipp - returned inode (out);
 *      flags == 0, we need to scan the hash table for the inode.
 *               IGET_IALLOC, this is a new inode and we need not
 *           scan hash.
 *
 * RETURN : ENFILE  - If out of inodes
 *      errors from subroutines
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */
iget(
    struct vfs *vfsp,   /* vfs for inode */
    ino_t ino,      /* requested inode number */
    struct inode **ipp, /* pointer to requested inode pointer */
    uint32 flag)        /* control flag */
{
    int32   rc;         /* return codes from functions */
    dev_t devno = 0;        /* device ID for VFS */
    int32 fsno = 0;         /* fileset number for inode */
    struct hinode *hip;     /* pointer into inode hash table */
    struct inode *newip = 0;    /* inode pointer returned by getip() */
    struct inode *ipimap = NULL;    /* inode map in VFS structure */
    struct gnode *gp;       /* gnode pointer for new inode */
    uint32 timestamp;       /* hash chain update timestamp */
    extern struct vnodeops jfs_vnops; /* vnode ops for JFS */

    ipimap = (struct inode *)vfsp->vfs_data;
    if (ipimap)
    {
        /* standard inode from standard vfs */
        devno = ipimap->i_dev;
        fsno = ipimap->i_number;
    }
    else
    {
        /*
         * special case of inode access without inode allocation
         * map during bring up of a file system/log device
         * from pseudo/temporary vfs;
         */
        devno = ((union mntvfs *)vfsp)->dummyvfs.dev;
        fsno = 0;
    }

    IHASH(ino, devno ^ fsno, hip);

    /* skip 'initial' hash table scan for new inode allocation */
    if (flag & IGET_IALLOC)
        goto tryRecycle;

tryReclaim:
    /*
     * search hashlist whether the specified inode can be reclaimed;
     */
    if (iSearchHashlist(hip, devno, fsno, ino, ipp) == 0)
        goto out;

    /*
     *  cache miss:
     *
     * allocate/recycle a inode to activate the specified inode;
     */
tryRecycle:
    timestamp = hip->i_timestamp;

    /*
     *  allocate/recycle a inode
     */
    switch (rc = getip(&newip))
    {
            /*
             * a fresh inode from freelist: take it.
             */
        case 0:
            break;

            /*
             * recycled inode: ICACHE_LOCK has NOT been
             * released during recycle; take it.
             */
        case 1:
            assert (newip->i_count == 0);

            if (newip->i_mode != 0)
            {
                /*
                 * unbind from previous identity;
                 */
                HASHLIST_REMOVE(newip);
                vPut(newip);
            }
            break;

            /*
             * recycled inode: ICACHE_LOCK has been
             * released during recycle; ensure against
             * race for duplicate inode activation
             * while ICACHE_LOCK has been released.
             */
        case 2:
            /*
             * hashlist has NOT been modified:
             */
            if (hip->i_timestamp == timestamp)
            {
                /* if other threads acquired reference
                 * of the inode, defer the recycled inode
                 * to the thread reclaiming the inode;
                 */
                if (newip->i_count > 0)
                    goto tryRecycle;

                assert(newip->i_mode != 0);

                /*
                 * unbind from previous identity;
                 */
                HASHLIST_REMOVE(newip);
                vPut(newip);

                break;
            }

            /*
             * hashlist has been modified:
             */
            if (newip->i_count == 0)
            {
                /*
                 * return inode back at head of freelist;
                 */
                FREELIST_INSERT_HEAD(&ifreelist, newip);
                nifreelist++;
            }

            goto tryReclaim;

            /*
             * There were no available free inodes to allocate
             */
        case ENFILE:
            return rc;

        default: /* should not happen */
            assert(0);
            return EINVAL;
    }

    /*
     *  initialize inode;
     */
    /*
     * insert at head of hashlist:
     */
    newip->i_dev = devno;
    newip->i_fileset = fsno;
    newip->i_number = ino;

    newip->i_count = 1;

    newip->i_xlock = IXLOCK;

    hip->i_timestamp++;

    HASHLIST_INSERT_HEAD(hip, newip);

    FREELIST_SELF(newip);

    /*
     * bind inode to its mount inode and inode allocation map inode
     */
    newip->i_ipimap = ipimap;
    if (ipimap)
        newip->i_ipmnt = ipimap->i_ipmnt;
    else
    {
        /* initialize i_ipmnt of all inode except i_number = 0 */
        newip->i_ipmnt = ((union mntvfs *)vfsp)->dummyvfs.ipmnt;
    }

    gp = IP2GP(newip);

    /*
     *  new object inode allocation:
     *
     * caller will further initialize on-disk inode fields;
     */
    if (flag & IGET_IALLOC)
    {
        /* There is no on-disk part, so clear it out */
        bzero((void *)&newip->i_dinode, sizeof(struct dinode));

        /* refill on-disk inode fields zeroed out above */
        newip->i_dev = devno;
        newip->i_fileset = fsno;
        newip->i_number = ino;

        /*
         * Set the generation number if this a new object (not
         * a filesystem or log), otherwise set to zero.
         */
        if (ipimap)
        {
            newip->i_inostamp = ipimap->i_inostamp;
            newip->i_gen = ipimap->i_gengen++;
        }
        else
        {
            newip->i_inostamp = 0;
            newip->i_gen = 0;
        }

        /*
         * Make sure the transaction manager knows we are adding an
         * inode to the filesystem.
         */
        newip->i_cflag = COMMIT_NEW;
    }
    /*
     *  existing object inode activation:
     *
     * initialize on-disk inode fields with on-diks inode;
     */
    else
    {
        /*
         * copy on-disk inode into in-memory inode:
         */
        ICACHE_UNLOCK();
        rc = diRead(newip);
        ICACHE_LOCK();

        if (rc)
        {
            /* mark inode invalidated */
            newip->i_mode = 0;

            HASHLIST_REMOVE(newip);
            HASHLIST_SELF(newip);

            newip->i_xlock &= ~IXLOCK;

            if (newip->i_xlock & IXWANT)
            {
                newip->i_xlock &= ~IXWANT;
                ICACHE_WAKEUP(newip);
            }

            iput(newip, vfsp);

            return rc;
        }

        /*
         * Initialize the device gnode fields from the on-disk inode.
         */
#ifdef  _JFS_OS2
        switch(newip->i_mode & IFMT)
        {
            case (IFREG):
            gp->gn_type = VREG;
            break;
            case (IFDIR):
            gp->gn_type = VDIR;
            break;
        }
        gp->gn_rdev = newip->i_dev;
#endif  /* _JFS_OS2 */

        newip->i_cflag = 0;
    }

    /*
     * initializing in-memory fields common to both new and
     * existing inodes.
     */
    newip->i_flag = 0;

    newip->i_bxflag = 0;
    newip->i_blid = 0;
    newip->i_atlhead = 0;

    newip->i_rdcnt = newip->i_wrcnt = 0;
#ifdef  _JFS_OS2
    newip->i_drcnt = newip->i_dwcnt = newip->i_lgcnt = 0;
#endif  /* _JFS_OS2 */

    /* btree search heuristics */
    newip->i_btorder = BT_RANDOM;
    newip->i_btindex = 0;

    /* read-ahead/write-behind control */
    newip->i_cacheid = NULL;
    newip->i_stepsize = 0;
    newip->i_rasize = 0;
    newip->i_rapage = 0;
    newip->i_wbpage = 0;

    /* DASD limits - parent inode list */
    newip->i_dasdlim = NULL;                // F226941

        /* initialize lock pointers for OS/2 files */
#ifdef  _JFS_OS2
        gp->gn_filelocks = NULL;
        gp->gn_pendinglocks = NULL;
#endif  /* _JFS_OS2 */

    gp->gn_flags = 0;
    gp->gn_data = (caddr_t) newip;

    /*
     * bind with VFS
     */
    /* inherit fs state */
    if (newip->i_ipmnt)
        newip->i_xlock |= newip->i_ipmnt->i_xlock;

    vGet(newip, vfsp);

    /*
     * Pass back the pointer to the new/recycled inode.
     */
    *ipp = newip;

out:
    /*
     * The inode is done transitioning, so we just go ahead and return
     * it back to our caller.  She'll have to finish adding parts if
     * IGET_IALLOC was set, or else she'll be able to use the info we
     * filled in from disk via diRead().
     */
    (*ipp)->i_xlock &= ~IXLOCK;
    if ((*ipp)->i_xlock & IXWANT)
    {
        (*ipp)->i_xlock &= ~IXWANT;
        ICACHE_WAKEUP((*ipp));
    }

    return 0;
}


/*
 * NAME:    getip()
 *
 * FUNCTION:    allocate a in-memory inode.
 *
 * PARAMETERS:  ipp - returned inode
 *
 * RETURN : 0   - success without ICACHE_LOCK() has been released
 *      1   - success with ICACHE_LOCK() has been temporarily
 *            released
 *      ENFILE  - inode table overflow
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */
static int32 getip(
    struct inode **ipp)
{
    struct inode *ip;
    struct gnode *gp;
    struct vnode *vp;
    struct xnode *xp;

    *ipp = NULL;

    /*
     *  try to allocate from the freepool:
     *
     * if number of free inodes is under the min freelist size
     */
    if (nifreelist < minifreelist)
    {
#ifdef  _JFS_OS2
        /*
         * transform spinlock into blocking lock
         * while calling services that might block
         * (e.g., xmalloc or KernAllocSpinLock);
         */
        ICACHE_XBLOCK();
#endif  /* _JFS_OS2 */
        if ((xp = (struct xnode *) allocpool(iCache, 0))
            != NULL)
        {
            /* bind inode and vnode */
            ip = (struct inode *)&xp->x_inode;

            /* allocate inode lock(s) */
            ilock_cnt++;
            IRDWRLOCK_ALLOC(ip,ilock_cnt);
            IRDWRLOCK_INIT(ip);

            INODE_LOCK_ALLOC(ip,ilock_cnt);
            INODE_LOCK_INIT(ip);

            /* initialize events */
            event_init(&ip->i_event);
            event_init(&ip->i_openevent);
            gp = IP2GP(ip);

#ifdef  _JFS_OS2
            TSPINLOCK_ALLOC(&gp->gn_byte_lock,LOCK_ALLOC_PAGED,
                RECLK_LOCK_CLASS, gn_reclk_count++);
            TSPINLOCK_INIT(&gp->gn_byte_lock);
            gp->gn_filelocks = NULL;
            gp->gn_pendinglocks = NULL;

            /* transform blocking lock back to spinlock */
            ICACHE_XSPIN();
#endif  /* _JFS_OS2 */
            vp = (struct vnode *)&xp->x_vnode;
            vp->v_vfsp = NULL;
            vp->v_next = NULL;
            /* link from inode to vnode */
            ip->i_gnode.gn_vnode = (struct vnode *)vp;

            /* link from vnode to inode */
            vp->v_gnode = (struct gnode*)&ip->i_gnode;

            *ipp = ip;
#ifdef FreeJFS
            INCREMENT(statICache.ninode);
#endif

            return 0; /* fresh new inode */
        }
#ifdef  _JFS_OS2
        else
            /* transform blocking lock back to spinlock */
            ICACHE_XSPIN();
#endif  /* _JFS_OS2 */
    }

    /*
     *  try to recycle from freelist.
     *
     * if either nifreelist reached min freelist size or
     * free pool is empty:
     */
    if ((ip = ifreelist.i_flnext) == (struct inode *) &ifreelist)
        return ENFILE;

    *ipp = ip;

    /* if ICACHE_LOCK() had been temporarily released by iuncache(),
     * notify the caller to rescan hashlist whether
     * the specified inode had been entered by another thread.
     */
    if (iuncache(ip, 0))
        return 2; /* recycled inode: lock has been released */

    return 1; /* recycled inode: lock has NOT been released */
}


/*
 * NAME:    iuncache()
 *
 * FUNCTION:    deactivate the specified inode to recycle:
 *      remove inode from freelist; inode remains linked to
 *      previous vfs and hashlist (except for i_mode == 0);
 *      commit changes of inode and/or file, and/or
 *      release its virtual memory resources.
 *
 * PARAMETERS:  ip  - inode to blast
 *      flag    - invalidate inode
 *
 * RETURN : 0 - if ICACHE_LOCK() has NOT been temporarily released.
 *      1 - if ICACHE_LOCK() has been temporarily released.
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */
iuncache(
    struct inode    *ip,
    uint32      flag)
{
    int32   rc = 0;
    cmctl_t *ccp;
    int32   tid;
    inode_t *iplist[1];
    int64   xaddr;
    int32   xlen;

#ifdef FreeJFS
    INCREMENT(statICache.recycle);
#endif

    /*
     *  remove from freelist
     *
     * ip is on the freelist except at forced deactivation,
     * (e.g., unmount of root inode)
     */
    if (ip->i_count == 0)
    {
        FREELIST_REMOVE(ip);
        FREELIST_SELF(ip);
        nifreelist--;
    }

    /* invalid inode ? */
    if (ip->i_mode == 0)
        goto out;

    ccp = ip->i_cacheid;

    /* is the inode is ready to be recycled ? */
    if (ccp == NULL &&
        (!(ip->i_flag & (IACC|ICHG|IUPD)) || isReadOnly(ip)))
        goto out;

    /* mark the inode as in transition so that iget() of this inode
     * will wait for the completion of transition.
     */
    ip->i_xlock |= IXLOCK;

    ip->i_count++;

    ICACHE_UNLOCK();

    /* if the inode has a cache object, check if the inode has
     * been modified and should be mark as IFSYNC.
     */
    if (ccp)
    {
        /* mark to be synced if file have been modified */
        if (ip->i_flag & IUPD)
            ip->i_flag |= IFSYNC;
    }

    /*
     * if inode and/or file has changed then commit it:
     * txCommit() forces pageout of modified data pages at start
     * of commit;
     * note: the inode is NOT FSXLOCKed;
     */
    if (ip->i_flag & (IACC|ICHG|IFSYNC))
    {
        IWRITE_LOCK(ip);
        iplist[0] = ip;
        txBegin(ip->i_ipmnt, &tid, 0);
        rc = txCommit(tid, 1, &iplist[0], COMMIT_FORCE);  // D233382
        txEnd(tid);
        IWRITE_UNLOCK(ip);
    }

    /* if inode has cache object, free cached pages */
    if (ccp)
    {
        /* release external EA extent pages from jcache */
                if (ip->i_ea.flag & DXD_EXTENT)
                {
                        /* free EA pages */
                        xaddr = addressDXD(&ip->i_ea);
                        xlen = lengthDXD(&ip->i_ea);
                        bmExtentInvalidate(ip, xaddr, xlen);
        }

        /* release external ACL extent pages from jcache */
                if (ip->i_acl.flag & DXD_EXTENT)
                {
                        /* free ACL pages */
                        xaddr = addressDXD(&ip->i_acl);
                        xlen = lengthDXD(&ip->i_acl);
                        bmExtentInvalidate(ip, xaddr, xlen);
        }

        /* release data pages from ccache */
        if ((ip->i_mode & IFMT) == IFREG)
            cmRelease(ccp);

        /* flush meta-data pages from jcache */
        bmInodeFlush(ip);

        /*
         * release memory object:
         *
         * actual delete will occur when the all the pages
         * of the cache object is released;
         */
        iUnbindCache(ip);
    }

    ICACHE_LOCK();

    ip->i_count--;

    /* wakeup sleepers (iget()s for this inode who found it).
     * yield the deactivated inode to the iget() for this inode.
     */
    ip->i_xlock &= ~IXLOCK;
    if (ip->i_xlock & IXWANT)
    {
        ip->i_xlock &= ~IXWANT;
        ICACHE_WAKEUP(ip);
    }

    rc = 1;

out:
    if (flag)
        iunhash(ip);

    return rc;
}


/*
 * NAME:    iput()
 *
 * FUNCTION:    put the specified inode where
 *      if vfsp != NULL, put also the vnode of the inode,
 *      if vfsp == NULL, put only the inode.
 *
 *      reference of vnode and/or inode is released
 *      on the last reference release, insert into cachelist
 *      (valid inode) or return to freelist (invalid inode).
 *
 * PARAMETERS:  ip  - unwanted inode
 *      vfsp    - vfs
 *
 * RETURN : Errors from subroutines.
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */
iput(
    struct inode    *ip,
    struct vfs  *vfsp)
{
    struct vnode    *vp;

jEVENT(0,("iPut: ip:0x%08x(%d)\n", ip, ip->i_count));

    assert(ip->i_count > 0);

#ifdef FreeJFS
    INCREMENT(statICache.release);
#endif

    /*
     * release inode reference
     */
    if (--ip->i_count > 0)
        return 0;

    /*
     *  last reference release (ip->i_count == 0);
     */

    /*
     * free the invalid (i_mode == 0) inode:
     * (the invalid inode does not have resources)
     */
    if (ip->i_mode == 0)
    {
        iunhash(ip);
        return 0;
    }

    /*
     * release resources of removed (i_nlink == 0) inode;
     */
    if (ip->i_nlink == 0)
    {
        /* ICACHE_LOCK() will be released during close:
         * mark the inode as in transition so that iget() of this inode
         * will wait for the completion of transition.
         */
        ip->i_xlock |= IXLOCK;

        /*
         * release resources as required.
         */
        iClose(ip);

        ip->i_xlock &= ~IXLOCK;

        /* wakeup sleepers (iget()s for this inode who found it).
         * yield the inode to the iget() for this inode.
         */
        if (ip->i_xlock & IXWANT)
        {
            /* the inode is being deleted: mark the inode
             * to be discarded by previous iget()s
             * who found the inode,
             */
            ip->i_mode = 0;

            /* remove it from hashlist to prevent later iget()
             * to find the inode (e.g., VNOP_VGET())
             */
            HASHLIST_REMOVE(ip);
            HASHLIST_SELF(ip);

            ip->i_xlock &= ~IXWANT;
            ICACHE_WAKEUP(ip);

            return 0;
        }

        /* remove from hashlist and
         * insert at head of freelist
         */
        iunhash(ip);

        return 0;
    }

    /*
     * cache the valid inode
     */
    /* insert at tail of freelist */
    /* ip->i_flag &= (IACC|ICHG|IUPD|IFSYNC); */

    FREELIST_INSERT_TAIL(&ifreelist, ip);
    nifreelist++;

    return 0;
}


/*
 * NAME:    iClose()
 *
 * FUNCTION:    Close inode being removed (i_nlink = 0)
 *      on last reference release (i_count = 0);
 *
 *      the resources associated with the inode are freed,
 *      for directory, directory must consists of inode only:
 *      (all its directory pages have been freed as each directory
 *      page has become empty);
 *      for regular file, ...
 *
 * PARAMETERS:  ip  - pointer to inode to close
 *
 * RETURN : none
 *
 * SERIALIZATION: called only by iput() with ICACHE_LOCK() and
 *        IXLOCK on the inode
 */
static void iClose(struct inode *ip)
{
    ialloc_t iad;

jEVENT(0,("iClose: ip:0x%08x(%d)\n", ip, ip->i_count));

    /*
     * last reference release of file being deleted.
     *
     * free up working resources for inode:
     * persistent block map and inode map should already have
     * been updated by commit() with i_nlink = 0.
     */
    /* mark the inode to be skipped in logsync() */
    ip->i_cflag |= ICLOSE;

    ICACHE_UNLOCK();

    /* for non-directory:
     * free ea pages from buffer cache;
     * free ea blocks from working block map;
     * free data and index pages from buffer cache;
     * free data and index blocks from working block map;
     */
    if (ip->i_cflag & COMMIT_FREEWMAP)
        freeZeroLink(ip);

    /* free cache object */
    if (ip->i_cacheid)
       {
        bmInodeInvalidate(ip);                              //PS 28092001 jbuf must free too
        iUnbindCache(ip);
       }

    /*
     * free the disk inode from the working inode map.
     */
    iad.ino = ip->i_number;
    iad.ixpxd = ip->i_ixpxd;
    iad.agstart = ip->i_agstart;
    diFree(ip->i_ipimap, &iad);

    ICACHE_LOCK();

    return;
}


/*
 * NAME:    iunhash()
 *
 * FUNCTION:    remove inode from inode cache
 *
 * PARAMETERS:  ip  - inode to blast
 *
 * RETURN : 0 - if no sleeps
 *      1 - if sleeps
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit.
 */
iunhash (struct inode *ip)
{
    /* unbind from vfs */
    vPut(ip);

    /* remove from hashlist */
    HASHLIST_REMOVE(ip);
    HASHLIST_SELF(ip);

    /* it's a deadbeef */
    ip->i_dev = 0xdeadbeef;
    ip->i_fileset = 0;
    ip->i_number = 0;

    ip->i_mode = 0;
    ip->i_nlink = 0;

    ip->i_count = 0;

    /* insert at head of freelist */
    FREELIST_INSERT_HEAD(&ifreelist, ip);
    nifreelist++;

    return 0;
}


/*
 * NAME:    iSyncFS()
 *
 * FUNCTION:    commits all regular files which have not been committed
 *      since the last time jfs_sync() was invoked.
 *
 * PARAMETERS:  vfsp - specific vfs to sync inodes for
 *
 * SERIALIZATION: guarantee to not to interfere by deferring the inode
 *      with work in progress
 *
 *      if unmount in progress on the vfs, skip the vfs
 */
void iSyncFS(struct vfs *vfsp)
{
    inode_t *ip, *ipnext, *ipmnt;
    struct vnode *vp;
    int32   tid;
    inode_t *iplist[1];
    struct timestruc_t t;
    int32   rc;                     // D230860

    ipmnt = ((inode_t *)vfsp->vfs_data)->i_ipmnt;

    SYNC_LOCK();

    curtime(&t);

    ICACHE_LOCK();

    /*
     * scan inode on vfs vnode list
     */
    vp = vfsp->vfs_vnodes;
    while (vp != NULL)
    {
        ip = VP2IP(vp);

        /* skip uninteresting inodes:
         *   deactivation in progress (iuncache()/iClose())
         *   unlinked inodes
         *   unmodified inodes
         *   not aged enough
         *   special inodes
         */
        if ( ((ip->i_flag & (IACC|ICHG|IUPD)) == 0)
            || (ip->i_xlock & IXLOCK)
            || (ip->i_nlink == 0)
            || (ip->i_mode == 0)
            || (t.tv_sec - ip->i_synctime < maxSyncTime &&
                t.tv_sec - ip->i_mtime.tv_sec < minSyncTime)
            || isSpecial(ip) )
        {
            vp = vp->v_vfsnext;
            continue;
        }

        /*
         * acquire reference of the inode
         * to guarantee to stay put in vfs vnode list
         * while cache lock is released
         */
        ip->i_count++;
        if (ip->i_count == 1)
        {
            nifreelist--;
            FREELIST_REMOVE(ip);
            FREELIST_SELF(ip);
        }

        ICACHE_UNLOCK();

        /* commit if not work in progress */
// BEGIN D230860
        MMPHPreIWRITE_LOCK_TRY();
        rc = IWRITE_LOCK_TRY(ip);
        MMPHPostIWRITE_LOCK_TRY();
        if (rc)
// END D230860
        {
            if (ip->i_flag & (IACC|ICHG|IUPD))
            {
                if (ip->i_cacheid && ip->i_flag & IUPD)
                    ip->i_flag |= IFSYNC;

                txBegin(ipmnt, &tid, COMMIT_SYNC);  //PS21092003
                iplist[0] = ip;
                txCommit(tid, 1, &iplist[0], 0);
                txEnd(tid);
            }

            IWRITE_UNLOCK(ip);
        }

        ICACHE_LOCK();

        /*
         * get next vnode on list before
         * releasing reference of current inode;
         * note: inode to be iput() will not
         * release ICACHE_LOCK;
         */
        vp = vp->v_vfsnext;
        iput(ip, NULL);
    }

    ICACHE_UNLOCK();

    SYNC_UNLOCK();
}


/*
 * NAME:    iQuiesce()
 *
 * FUNCTION:    commits all regular files which have not been committed
 *      since the last time jfs_sync() was invoked.
 *
 * PARAMETERS:  vfsp - specific vfs to sync inodes for
 *
 * SERIALIZATION: ipmnt->i_rdwrlock held on entry/exit;
 */
iQuiesce(struct vfs *vfsp)
{
    inode_t *ip, *ipnext, *ipmnt;
    struct vnode *vp;
    int32   tid;
    inode_t *iplist[1];
    int32   nlink;

    ipmnt = ((inode_t *)vfsp->vfs_data)->i_ipmnt;

    /* block rename() on this fs */
    RENAME_LOCK(ipmnt);

    ICACHE_LOCK();

    ipmnt->i_xlock |= FSXLOCK;

    /*
     * scan inode on vfs vnode list
     */
repeat:
    vp = vfsp->vfs_vnodes;
    while (vp != NULL)
    {
        ip = VP2IP(vp);

        /* skip uninteresting inodes: */
        if ((ip->i_xlock & FSXLOCK)
            || (ip->i_mode == 0)
            || isSpecial(ip))
        {
            vp = vp->v_vfsnext;
            continue;
        }

        /*
         * acquire reference of the inode
         * to guarantee to stay put in vfs vnode list
         * while cache lock is released
         */
        ip->i_count++;

        /*
         * wait for completion of deactivation in progress
         * (iuncache()/iClose())
         */
        if (ip->i_xlock & IXLOCK)
        {
            do
            {
                ip->i_xlock |= IXWANT;
                ICACHE_SLEEP(ip);
                if (ip->i_mode == 0)
                {
                    iput(ip, NULL);
                    goto repeat;
                }
            }
            while (ip->i_xlock & IXLOCK);
        }

        assert(ip->i_mode != 0);

        if (ip->i_count == 1)
        {
            nifreelist--;
            FREELIST_REMOVE(ip);
            FREELIST_SELF(ip);
        }

        ICACHE_UNLOCK();

        nlink = ip->i_nlink;

        /*
         * commit and set FS in transition lock
         */
        IWRITE_LOCK(ip);

        ip->i_xlock |= FSXLOCK;
/*
printf("iQuiesce: inumber:%d\n", ip->i_number);
*/

        if ((nlink != 0)
           && (ip->i_flag & (IACC|ICHG|IUPD)))
        {
            if (ip->i_cacheid && ip->i_flag & IUPD)
                ip->i_flag |= IFSYNC;

            txBegin(ipmnt, &tid, 0);
            iplist[0] = ip;
            txCommit(tid, 1, &iplist[0], 0);
            txEnd(tid);
        }

        IWRITE_UNLOCK(ip);

        ICACHE_LOCK();

        /*
         * get next vnode on list before
         * releasing reference of current inode;
         */
        vp = vp->v_vfsnext;
        iput(ip, NULL);
        if (nlink == 0)
            goto repeat;
    }

    ICACHE_UNLOCK();
}


/*
 * NAME:    iResume()
 *
 * FUNCTION:    resume FS transactions;
 *
 * SERIALIZATION: ipmnt->i_rdwrlock held on entry/exit;
 */
iResume(struct vfs *vfsp)
{
    inode_t *ip, *ipnext, *ipmnt;
    struct vnode *vp;
    int32   tid;
    inode_t *iplist[1];
    int32   nlink;

    ipmnt = ((inode_t *)vfsp->vfs_data)->i_ipmnt;

    ICACHE_LOCK();

    ipmnt->i_xlock &= ~FSXLOCK;

    /*
     * scan inode on vfs vnode list
     */
repeat:
    vp = vfsp->vfs_vnodes;
    while (vp != NULL)
    {
        ip = VP2IP(vp);

        /* skip uninteresting inodes: */
        if (((ip->i_xlock & FSXLOCK) == 0)
            || (ip->i_mode == 0)
            || isSpecial(ip))
        {
            vp = vp->v_vfsnext;
            continue;
        }

        /*
         * acquire reference of the inode
         * to guarantee to stay put in vfs vnode list
         * while cache lock is released
         */
        ip->i_count++;

        /*
         * wait for completion of deactivation in progress
         * (iuncache()/iClose())
         */
        if (ip->i_xlock & IXLOCK)
        {
            do
            {
                ip->i_xlock |= IXWANT;
                ICACHE_SLEEP(ip);
                if (ip->i_mode == 0)
                {
                    iput(ip, NULL);
                    continue;
                }
            }
            while (ip->i_xlock & IXLOCK);
        }

        assert(ip->i_mode != 0);

        if (ip->i_count == 1)
        {
            nifreelist--;
            FREELIST_REMOVE(ip);
            FREELIST_SELF(ip);
        }

        ICACHE_UNLOCK();

        nlink = ip->i_nlink;

        /*
         * reset FS in transition lock
         */
        WRITE_LOCK(&ip->i_rdwrlock);

        ip->i_xlock &= ~FSXLOCK;
/*
printf("iResume: inumber:%d\n", ip->i_number);
*/

        if (ip->i_xlock & FSXWANT)
        {
            ip->i_xlock &= ~FSXWANT;
            EVENT_WAKEUP(&ip->i_fsevent);
        }

        WRITE_UNLOCK(&ip->i_rdwrlock);

        ICACHE_LOCK();

        /*
         * get next vnode on list before
         * releasing reference of current inode;
         */
        vp = vp->v_vfsnext;
        iput(ip, NULL);
        if (nlink == 0)
            goto repeat;
    }

    ICACHE_UNLOCK();

    RENAME_UNLOCK(ipmnt);
}


/*
 * NAME:    iUnmount()
 *
 * FUNCTION:    check if file system is quiescent to unmount, and
 *      clean up inodes of file system being unmounted.
 *
 * PARAMETERS:  dev - device to check for activity
 *      forced  - forced unmount
 *
 * RETURN : 0   - success
 *      EBUSY   - busy
 *
 * SERIALIZATION: guarantee to remove the inode from inode cache
 *      by waiting on the inode if work in progress
 *
 *      higher layer serializes mount/umount/path name
 *      translation/file handle translation (i.e., no
 *      race with iget()).
 */
iUnmount (
    struct vfs  *vfsp,      /* device being unmounted */
    int32   forced)         /* boolean (!0=>shutting down) */
{
    int32 rc = 0;
    struct inode    *ipimap, *ip;
    struct vnode    *vp, *vpnext;

    SYNC_LOCK();

    ICACHE_LOCK();

    /*
     * scan vfs vnode list and close each vnode/inode
     */
    ipimap = (inode_t *)vfsp->vfs_data;
    ipimap->i_cflag |= IUNMOUNT;

    /*
     * scan inode on vfs vnode list
     */
loop:
        for (vp = vfsp->vfs_vnodes; vp; vp = vp->v_vfsnext)
        {
        ip = VP2IP(vp);

        /* meta-file inodes acquire single/only reference
         * at mount time, and remains active to be released
         * at end of umount time.
         */
        if (isSpecial(ip) && ip->i_count == 1)
            continue;

        /* root inode of quiescent fs should have a single reference
         * by the vfs to be released at end of umount time.
         */
        if (ip->i_number == ROOT_I)
        {
            if (ip->i_count != 1)
            {
jALERT(1,("iUnmount: active root ip:0x%08x(%d)\n", ip, ip->i_count));
                rc = EBUSY;
                if (!forced)
                    goto out;
            }
            continue;
        }

        /*
         * acquire a reference of the inode.
         */
        ip->i_count++;

        /*
         * wait for completion of deactivation in progress
         * (iuncache()/iClose())
         */
        if (ip->i_xlock & IXLOCK)
        {
            do
            {
                ip->i_xlock |= IXWANT;
                ICACHE_SLEEP(ip);
                if (ip->i_mode == 0)
                {
                    iput(ip, NULL);
                    goto loop;
                }
            }
            while (ip->i_xlock & IXLOCK);
        }

        assert(ip->i_mode != 0);

        /*
         * release a reference of the inode.
         */
        ip->i_count--;

        /*
         * vnode/inode should not be active
         * unless forced unmount
         */
        if (ip->i_count > 0)
        {
jALERT(1,("iUnmount: active ip:0x%08x(%d)\n", ip, ip->i_count));
            rc = EBUSY;
            if (!forced)
                goto out;
            continue;
        }

        /*
         * close the current inode.
         */
        iuncache(ip, 1);
        }

out:
    ICACHE_UNLOCK();

    SYNC_UNLOCK();

    return rc;
}

#ifdef Drova
char FuckIBMCompiler[100];
#endif
/*
 * NAME:    iAlloc()
 *
 * FUNCTION:    allocate new inode for file in directory
 *
 * PARAMETER:   vfsp    - vfs pointer
 *      pip - parent inode pointer
 *      mode    - rwx and IFMT mode
 *      ipp - returned inode (out)
 *      crp - credential
 *
 * RETURN:  0   - new, locked inode
 *      EIO - I/O error on dquot (from subroutine)
 */
iAlloc(
    struct vfs  *vfsp,
    inode_t     *pip,
    mode_t      mode,
    inode_t     **ipp,
    struct ucred    *crp)
{
    int32   rc;
    inode_t *ipimap = (inode_t *)vfsp->vfs_data;
    inode_t *ip;
    struct gnode *gp;
    struct timestruc_t t;
    struct hinode *hip;
    boolean_t isdir;
    ialloc_t iad;

    *ipp = NULL;

    /*
     *  allocate on-disk inode
     */
    isdir = ((mode & IFMT) == IFDIR);
    if (rc = diAlloc(pip, isdir, &iad))
        return rc;

    /*
     *  allocate in-memory inode.
     *
     * if VFS_VGET() is in progress on the hashlist,
     * scan hashlist in iget() for newly allocated i_number
     * since VFS_VGET() may insert stale inode (with the same
     * i_number and i_nlink = 0) into the hashlist;
     * otherwise, skip hashlist scan in iget().
     * (the i_vget count is incremented and decremented
     * in jfs_vget()).
     */
    IHASH(iad.ino, pip->i_dev, hip);

    ICACHE_LOCK();

    if (hip->i_vget == 0)
        rc = iget(vfsp, iad.ino, &ip, IGET_IALLOC);
    else
        rc = iget(vfsp, iad.ino, &ip, 0);

    ICACHE_UNLOCK();

    if (rc)
    {
        diFree(ipimap, &iad);
        return rc;
    }

    ASSERT(ip->i_nlink == 0 && ip->i_mode == 0);

    /*
     * initialize on-disk inode
     *
     * (in-memory inode initialization performed by iget())
     */
    ip->i_nlink = 1;
    ip->i_mode = mode;
    if ((mode & IFMT) == IFDIR)
    {
        /* inherit ISGID from parent directory */
        if (pip->i_mode & ISGID)
            ip->i_mode |= ISGID;

        ip->i_mode |= IFJOURNAL;
    }
    else
    {
        if (ip->i_ipmnt->i_mntflag & JFS_SPARSE)
            ip->i_mode |= ISPARSE;
        ip->i_mode |= INLINEEA;
    }

    ip->i_ixpxd = iad.ixpxd;
    ip->i_agstart = iad.agstart;

    /* assign uid/gid */
    ip->i_uid = crp->cr_uid;
    ip->i_gid = (pip->i_mode & ISGID) ? pip->i_gid : crp->cr_gid;

    /* set update times directly to avoid extra commit in iput() */
    curtime(&t);
    ip->i_atime = t;
    ip->i_mtime = t;
    ip->i_ctime = t;
#ifdef _JFS_OS2
        ip->i_otime = t;                /* [OS/2] create time */
#endif
    ip->i_synctime = t.tv_sec;

    /* clear EA */
    ip->i_ea.flag = 0;
        ip->i_ea.rsrvd = 0;
    ip->i_ea.size = 0;

    /* clear ACL */
    ip->i_acl.flag = 0;
        ip->i_acl.rsrvd = 0;
    ip->i_acl.size = 0;

    ip->i_compress = 0;

    /* mark inode attribute change */
    ip->i_flag = ICHG;

    gp = IP2GP(ip);
#ifdef  _JFS_OS2
    switch(ip->i_mode & IFMT)
    {
        case (IFREG):
             gp->gn_type = VREG;
             break;
        case (IFDIR):
             gp->gn_type = VDIR;
             break;
        default:
            panic("Invalid object type");
    }
    gp->gn_rdev = ip->i_dev;
#endif  /* _JFS_OS2 */

    /* lock the inode to return (caller will test for FSXLOCK) */
    WRITE_LOCK(&ip->i_rdwrlock);

    *ipp = ip;

    return 0;
}


/*
 * NAME:    vGet()
 *
 * FUNMCTION:   initialize vnode of the inode at first iget().
 *
 *  vfsp != NULL:
 *  initialize v_count = 1 and link vnode to vfs (v_vfsp = vfsp);
 *
 *  vfsp == NULL:
 *  . inode does not have vnode (meta-inode);
 *  initialize v_count = 0 and leave vnode unlinked (v_vfsp = NULL).
 *
 * SERIALIZATION: ICACHE_LOCK held on entry/exit
 */
static void vGet(
    struct inode    *ip,
    struct vfs  *vfsp)
{
    struct vnode    *vp = ip->i_gnode.gn_vnode;

    vp->v_count = 1;

    vp->v_flag = 0;
    vp->v_vfsp = vfsp;

    /* insert at head of vfs vnodelist
     * (singly linked header, doubly linked list)
     */
    vp->v_vfsnext = vfsp->vfs_vnodes;
    vfsp->vfs_vnodes = vp;
    vp->v_vfsprev = NULL;
    if (vp->v_vfsnext != NULL)
        vp->v_vfsnext->v_vfsprev = vp;
}


/*
 * NAME:    vPut()
 *
 * FUNCTION:    finalize vnode of the inode at iunhash()
 *
 *  vfsp != NULL: unlink vnode from vfs (v_vfsp = vfsp);
 *
 *  vfsp == NULL:
 *  . inode does not have vnode (meta-inode);
 *  . vnode of inode not initialized yet (inode initialization failure);
 *   vnode is already freed
 *      (iuncache/iunhash sequence),
 *
 * SERIALIZATION: ICACHE_LOCK held on entry/exit
 */
void vPut(struct inode *ip)
{
    struct vnode    *vp = IP2VP(ip);
    struct vfs  *vfsp;

    vfsp = vp->v_vfsp;
    if (vfsp == NULL)
        return;

    vp->v_count = 0;

    /* remove from vfs vnode list
     * (singly linked header, doubly linked list)
     */
    if (vp == vfsp->vfs_vnodes)
        vfsp->vfs_vnodes = vp->v_vfsnext;
    if (vp->v_vfsnext != NULL)
        vp->v_vfsnext->v_vfsprev = vp->v_vfsprev;
    if (vp->v_vfsprev != NULL)
        vp->v_vfsprev->v_vfsnext = vp->v_vfsnext;

    vp->v_vfsp = NULL;
}


/*
 * NAME:    iBindCache()
 *
 * FUNCTION:    create a cache control object and associate it with
 *      the persistent object specified by ip.
 *
 * PARAMETER:
 *  ip  - pointer to inode to bind to cache
 *
 * RETURN:
 *  0   - success
 *  ENOMEM  - insuffcient virtual memory (return from cmCreate()).
 */
iBindCache(inode_t *ip)
{
    struct cache_ctl    *cacheid;
    int32   rc;
    inode_t *ipmnt;

    /* already bound to a cache control object ? */
    if (ip->i_cacheid)
        return(0);

    /* We can't hold the INODE_LOCK while calling cmCreate since it
     * may block.  We will check ip->i_cacheid afterwards and return
     * the cache control object if another thread beat us to it.
     */

    /* create a cache control object */
    ipmnt = ip->i_ipmnt;
    if (rc = cmCreate(ipmnt->i_cachedev,&cacheid))
        return rc;

    INODE_LOCK(ip);
    if (ip->i_cacheid)
    {
        /* Another thread must have jumped in ahead of us. */
        INODE_UNLOCK(ip);
        cmDelete(cacheid);
        return 0;
    }
    ip->i_cacheid = cacheid;
    /* if file system supports compression, compression applies
     * for non-journalled non-special files.
     */
    if (ipmnt->i_compress)
        ip->i_compress = ipmnt->i_compress;

    INODE_UNLOCK(ip);

    return  0;
}


/*
 * NAME:    iUnbindCache()
 *
 * FUNCTION:    disassociate an inode for a cache control object and
 *      free the cache control object.
 *
 * PARAMETER:
 *  ip  - pointer to inode to be unbound from a cache control
 *        object.
 *
 * RETURN:  none.
 */
iUnbindCache(struct inode *ip)
{
    /* is the inode associated with a cache control object ? */
    if (ip->i_cacheid == NULL)
        return  0;

    /*
     * for iUncache(): data pages have been synchronously written
     * by commit, and released (cmRelease());
     * meta-data pages have been flushed (bmInodeFlush());
     *
     * for iClose(): all data and meta-data pages have been freed;
     */

    /*
     * free the cache control object:
     *
     * cache control object will be released after all data and
     * meta-data pages in i/o, if any, are completetd;
     */
    cmDelete(ip->i_cacheid);
    ip->i_cacheid = NULL;

    return 0;
}


/*
 * NAME:    iFlushCache()
 *
 * FUNCTION:    synchronously flush a inode cache blocks to disk.
 *
 * PARAMETER:
 *  ip  - pointer to inode whose pages we want to write out
 *
 * RETURN:
 *  0   - success
 *  EIO - i/o error
 */
iFlushCache(inode_t *ip)
{
    int32 rc;

    /* is the inode associated with a cache control object ? */
    if (ip->i_cacheid == NULL)
        return 0;

    /* write the inode's cache blocks to disk */
    rc = cmFlush(ip->i_cacheid);

    return(rc);
}


#ifdef  _JFS_DEBUG
/*
 *  iSanity()
 */
void iSanity()
{
    struct hinode   *hip;
    inode_t     *ip, *pip;
    cmctl_t     *ccp;
    struct vnode    *vp;
    int32       nhash = 0, nfree = 0;

    printf("\n - icache sanity check: -\n");

    /*
     *  hashlist
     */
    printf("\nhashlist display:\n");
    for (hip = ihashtbl; hip < &ihashtbl[nhinode]; hip++)
    {
        for (ip = hip->i_hlnext;
                 ip != (inode_t *)hip;
                 ip = ip->i_hlnext)
        {
            printf("ip:0x%08x inumber:%d imode:0x%08x\n",
                ip, ip->i_number, ip->i_mode);

            ASSERT(ip->i_hlnext != ip);

            if (ip->i_count > 0)
            {
                nhash++;

                ASSERT(ip->i_mode != 0);
                ASSERT(ip->i_flnext == ip);
            }
        }
    }

    /*
     *  freelist
     */
    printf("\nfreelist display: nifreelist:%d\n", nifreelist);
    for (ip = ifreelist.i_flnext;
         ip != (inode_t *)&ifreelist;
         ip = ip->i_flnext)
    {
        nfree++;

        ASSERT(ip->i_flnext != ip);

        ASSERT(ip->i_count == 0);
        vp = IP2VP(ip);
        ASSERT(vp->v_count == 0);

        if (ip->i_mode == 0)
        {
            if (ip->i_hlnext != ip)
                ASSERT(ip->i_hlnext == ip);

            if (ccp = ip->i_cacheid)
            {
                if (ccp->cc_jbuflist.head != NULL)
                {
                    ASSERT(ccp->cc_jbuflist.head == NULL);
                    printf("ip:0x%08x jbuflist:0x%08x\n",
                        ip, ccp->cc_jbuflist.head);
                }
            }

            ASSERT(ip->i_blid == 0);
        }
        else
        {
            printf("ip:0x%08x imode:0x%08x\n",
                ip, ip->i_mode);
        }
    }
}
#endif  /* _JFS_DEBUG */
