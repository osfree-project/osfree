/* $Id: jfs_init_os2.c,v 1.1 2000/04/21 10:58:11 ktk Exp $ */

static char *SCCSID = "@(#)1.35  4/9/99 14:01:59 src/jfs/ifs/jfs_init_os2.c, sysjfs, w45.fs32, currbld";
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
 *      jfs_init.c: JFS installable file system bringup
 */
#define INCL_DOSMISC       /* DOS Miscellaneous values */
#define INCL_DOSERRORS     /* DOS Error values         */
#define INCL_NOPMAPI
#include <os2.h>
#include "jfs_os2.h"
#include "jfs_lock.h"
#include "jfs_filsys.h"
#include "jfs_debug.h"
#include "jfs_inode.h"
#include "jfs_dnlc.h"
#include "jfs_dirent.h"
#include "jfs_cachemgr.h"
#include "jfs_dio.h"
#include "jfs_proto.h"


/*
 *      JFS global lock
 */
MUTEXLOCK_T     jfsLock;

/*
 *      external references: JFS subsystem initialization
 */
extern MUTEXLOCK_T      jfsSyncLock;
#ifdef  _JFS_WIP
extern MUTEXLOCK_T  jfsEALock;
#endif  /* _JFS_WIP */
extern MUTEXLOCK_T  dasd_lock;

extern event_t  pg_event;
extern XSPINLOCK_T  pg_spinlock;

extern SPINLOCK_T   enum_lock;

extern int32    initicache(void);
extern int32    txInit(void);
extern int32    strat2Init(void);
void            ComInit(int32);
unsigned int _Optlink   strtoul(char *, char **, int);

extern int32    _autocheck_mask; /* mask of drives to be autochecked       */
extern int32    _force_mask;     /* mask of drives to be force autochecked */

extern int32    LazyOff;
extern int32    SyncTime;
extern int32    maxSyncTime;
extern int32    minSyncTime;

/*
 *      global data
 */
uint32  MemorySize;     /* physical memory size in bytes */
uint32  CacheSize = 0;  /* buffer cache size in bytes */
uint32  openMax = 0;    /* max number of objects to be open at any one time */

uint32  jfs_init_done = 0;  /* real initialization has been done */

pool_t  *vfspool;               /* pool of vfs structures */
struct  vfs *vfs_anchor = NULL; /* list of active vfs's */

pool_t  *dirent_pool;   /* pool of dirents used by readdir() */

pool_t  *lock_pool; /* pool of lock structures for file locks */

pool_t  *cache_list_pool; /* pool of cachelists (FS32_READFILEATCACHE) */

#ifdef  _JFS_UNICODE
pool_t  *unipool;   /* pool of Unicode pathnames */
pool_t  *string_pool;   /* pool of character strings */
#endif  /* _JFS_UNICODE */

/* instance of cred for global use */
struct ucred global_cred = { 0, 0};
struct ucred *pcred;

/* the following array is global to avoid putting it on the stack */
uint32  SysInfo[QSV_MAX];     /* System Information Data Buffer */

/*
 * NAME:        jfs_init()
 */

/*
 * NAME:        jfs_init2()
 *
 * FUNCTION:    bring up the JFS file system
 *
 *      called by jfs_mount the first time the IFS calls it.
 *
 * PARAMETER:   none
 *
 * RETURN:      value returned from called routine
 */
int32
jfs_init2()
{
    int32 rc;
    int32 minCache;
    int32 trythis;

    jFYI(1,("       JFS Bring Up:\n"));
    jFYI(1,("System Memory Size:%d Byte\n", MemorySize));

    /*
     * compute buffer cache size =
     * MAX(ifs cachesize, 1 Meg), if specified, OR
     * MIN(memory size/8, 64 Meg);
     */
    minCache = 1*1024*1024;
    if (CacheSize == 0)     /* Not set in parse_args */
        CacheSize = MIN(MemorySize >> 3, 64*1024*1024);
/* Enforce minimum of 1 Meg */
    else if (CacheSize < minCache)
        CacheSize = minCache;

    jFYI(1,("Buffer Cache Size:%d Byte\n", CacheSize));

#ifdef  _JFS_UNICODE
    /*    Set up pool of unicode pathnames */
    if (rc = initpool(4096, 63, (MAXPATHLEN+1)*sizeof(UniChar), 0, "UNIPATH",
                      &unipool, POOL_TAKE_LOCK))
        return rc;

    /*    Set up pool of character pathnames */
    if (rc = initpool(512, 31, (MAXPATHLEN+1)*sizeof(char), 0, "STRING",
                      &string_pool, POOL_TAKE_LOCK))
        return rc;
#endif  /* _JFS_UNICODE */

    /*
     *      initialize global data structures
     */
    /* set up pool of vfs structures */
    if (rc = initpool(26, 26, sizeof(struct vfs), 0, "VFS", &vfspool,
                      POOL_TAKE_LOCK))
        return rc;

    /* set up pool of dirents */
    if (rc = initpool(1024, 16, PSIZE, 0, "DIRENT", &dirent_pool, POOL_TAKE_LOCK))
        return rc;

    /* init pool of lock structures for file locking */
    if (rc = initpool(0xffffffff, 85, sizeof(struct file_lock), 0, "LOCKS",
                      &lock_pool, POOL_TAKE_LOCK))
        return rc;

    /* Initialize pool of 16-bit strat2 request lists */
    if (rc = strat2Init())
        return rc;

    /* initialize vnode/inode cache */
    if (rc = initicache())
        return rc;

    /* initialize directory name lookup cache:
     * ncInit() must be after initicache()
     */
    if (rc = ncInit())
        return rc;

    /* initialize data cache manager */
    if (rc = cmInit())
    {
        /* Try a smaller cache size */
        trythis = MIN(64*1024*1024, MemorySize >> 2);
        if (CacheSize > trythis)
        {
            CacheSize = trythis;
            rc = cmInit();
        }
        if (rc)
        {
            trythis = MIN(32*1024*1024, MemorySize >> 3);
            if (CacheSize > trythis)
            {
                CacheSize = trythis;
                rc = cmInit();
            }
        }
        if (rc)
        {
            trythis = MIN(2*1024*1024, MemorySize >> 4);
            if (CacheSize > trythis)
            {
                CacheSize = trythis;
                rc = cmInit();
            }
        }
        if (rc)
            return(rc);

        jFYI(1,("Specified cache too big.  New cache size is %d.\n", CacheSize));
    }

    /* initialize direct I/O buffer manager */
    if (rc = dioInit(64))
        return(rc);

    /* initialize meta-data cache manager */
    if (rc = bmInit())
        return rc;

    /* initialize log manager */
    if (rc = lmInit())
        return rc;

    /* initialize transaction manager */
    if (rc = txInit())
        return rc;

    /* Initialize pool of cachelists */
    if (rc = initpool(0xffffffff, 22, sizeof(cache_list_t), 0, "CACHLIST",
                      &cache_list_pool, POOL_TAKE_LOCK))
        return rc;


    /*
     *      initialize global locks
     */
    /* initialize the jfs lock */
    MUTEXLOCK_ALLOC(&jfsLock, LOCK_ALLOC_PAGED, JFS_LOCK_CLASS, -1);
    MUTEXLOCK_INIT(&jfsLock);

    /* initialize the sync lock */
    MUTEXLOCK_ALLOC(&jfsSyncLock, LOCK_ALLOC_PAGED, JFS_SYNC_LOCK_CLASS, -1);
    MUTEXLOCK_INIT(&jfsSyncLock);

    /* initialize the dasd lock */
    MUTEXLOCK_ALLOC(&dasd_lock, LOCK_ALLOC_PAGED, JFS_DASD_LOCK_CLASS, -1);
    MUTEXLOCK_INIT(&dasd_lock);

    /* initialize paging event & spinlock */
    event_init(&pg_event);
    XSPINLOCK_ALLOC(&pg_spinlock, LOCK_ALLOC_PIN, JFS_PAGING_LOCK_CLASS, -1);
    XSPINLOCK_INIT(&pg_spinlock);

    /* initialize ACL enumeration lock */
    SPINLOCK_ALLOC(&enum_lock, 0, 0, 0);
    SPINLOCK_INIT(&enum_lock);

#ifdef  _JFS_COMPRESSION
    /* initialize the compression lock */
    lock_alloc(&jfsCompressionLock, LOCK_ALLOC_PAGED, JFS_COMPRESSION_LOCK_CLASS, -1);
    simple_lock_init(&jfsCompressionLock);
#endif

    /* Initialize the credential structure pointer.  It is used
     * everywhere to indicate "superuser" authority to the JFS,
     * which doesn't really understand that anyone can do anything
     * in this system.
     */
    pcred = &global_cred;

    /* indicate that we don't have to do this again */
    jfs_init_done = 1;

    jFYI(1,("       JFS Bring Up Complete.\n"));
    return 0;
}


/*
 * NAME:        parse_args()
 *
 * FUNCTION:    parse command line arguments (from Config.Sys)
 *
 *      called from first call to jfs_mount().
 *
 * PARAMETER:   char * args - argument string
 *
 * RETURNS:     nothing
 *
 * SIDE EFFECTS:        Sets global variables
 *
 * NOTES:       No error checking yet.  Improper syntax may give unpredictable
 *              results
 */
void  parse_args(char *args)
{
    uint32  csize;
    int32   digit;
    UCHAR   force = FALSE;
    UCHAR   drive;

    /* initialize the masks for autocheck */
    _autocheck_mask = 0;
    _force_mask = 0;

    while (*args != 0)
    {
        /* look for - or / */
        while (*args && *args != '-' && *args != '/')
            args++;

        if (*args == 0)
        {
            break;
        }

        switch (*(++args))
        {

            case 'p':             /* set the port for debug messages */
            case 'P':

                if (*(++args) == ':')
                    args++;

                if (*(args++) == '1')
                    ComInit(1);
                else
                    ComInit(2);
                break;
                /* set the cache size */
            case 'c':
            case 'C':
                if (*(++args) == 'R' || *args == 'r')   /* HPFS CRECL */
                    break;
                while (*args != ':')
                {
                    if (*args == 0)
                        break;
                    args++;
                }
                csize = strtoul(++args, 0, 10);
                if (csize)
                    CacheSize = csize * 1024;
                break;

                /* autocheck */
            case 'A':
            case 'a':
                while (*args != ':' && *args != 0)  /* suck up letters until : */
                {
                    if (*args == 0)
                        break;
                    args++;
                }

                args++;                 /* skip over the : */

                while ((*args != ' ') && (*args != 0))
                {
                    drive = *args;        /* get drive letter  */

                    if (drive == '+')     /* force requested ? */
                    {
                        force = TRUE;      /* flag as force for next drive */
                        args++;
                        continue;          /* go to top of loop */
                    }
                    if (drive == '*')     /* autocheck all drives requested ? */
                    {
                        _autocheck_mask = 0xFFFFFFFC;  /* check c-z */
                        if (force)
                        {
                            _force_mask = 0xFFFFFFFC;    /* force all */
                        }
                        break;              /* we are done with autocheck parms */
                    }
                    /* ingnore a or b since they can't be JFS */
                    if (drive >= 'c' && drive <= 'z')
                    {
                        drive -= 'a';        /* change to drive index of letter */
                    }
                    else
                    {
                        if (drive >= 'C' && drive <= 'Z')
                        {
                            drive -= 'A';        /* change to drive index of letter */
                        }
                        else  /* not a valid drive letter, just quit here */
                        {
                            break;
                        }
                    }

                    /* if we get here then we have a valid single drive index */
                    _autocheck_mask |= 1 << drive;
                    if (force)
                    {
                        _force_mask |= 1 << drive;
                    }

                    force = FALSE;           /* reset force flag for next drive */
                    args++;                  /* go to next letter */
                }   /* end while */

                break;

                /* LazyWrite */
            case 'L':
            case 'l':
                while (*args != ':' && *args != 0)  /* suck up letters until : */
                {
                    if (*args == 0)
                        break;
                    args++;
                }

                if (*(args++) == 0)                 /* skip over the : */
                    break;

                if (*args == 'o' || *args == 'O')
                {
                    args++;
                    if (*args == 'f' || *args == 'F')
                    {
                        LazyOff = TRUE;
                        SyncTime = 1;
                        minSyncTime = 0;
                        maxSyncTime = 0;
                    }
                    break;
                }

                /* Parse SyncTime */

                digit = *args;
                if (digit < '0' || digit > '9')
                    break;

                SyncTime = digit - '0';
                digit = *(++args);
                while (digit >= '0' && digit <= '9')
                {
                    SyncTime = SyncTime*10 + (digit - '0');
                    digit = *(++args);
                }
                if (SyncTime == 0)
                    SyncTime = 1;

                /* Parse maxSyncTime */

                if ((*args != ',') || ((digit = *(++args)) < '0' || digit > '9'))
                {
                    maxSyncTime = SyncTime << 2;
                    minSyncTime = MIN(1, SyncTime >> 3);
                    break;
                }

                maxSyncTime = digit - '0';
                digit = *(++args);
                while (digit >= '0' && digit <= '9')
                {
                    maxSyncTime = maxSyncTime*10 + (digit - '0');
                    digit = *(++args);
                }

                /* Parse minSyncTime */

                if ((*args != ',') || ((digit = *(++args)) < '0' || digit > '9'))
                {
                    minSyncTime = MIN(1, SyncTime >> 3);
                    break;
                }

                minSyncTime = digit - '0';
                digit = *(++args);
                while (digit >= '0' && digit <= '9')
                {
                    minSyncTime = minSyncTime*10 + (digit - '0');
                    digit = *(++args);
                }

                break;

            default:
                args++;
                break;
        }
    }
    return;
}

/*
 *      strtoul() - Version in Visual Age library does not work on 16-bit
 *      stack.
 */
unsigned int _Optlink strtoul(
                             char    *string,
                             char    **next,
                             int     base)
{
    unsigned int    result = 0;

    if (base == 0)
    {
        base = 10;
        if (*string == '0')
        {
            if (*(++string) == 'x' || *string == 'X')
            {
                string++;
                base = 16;
            }
            else
                base = 8;
        }
    }
    while (*string)
    {
        if (base == 10)
        {
            if (*string < '0' || *string > '9')
                break;
            result = result*10 + (*string - '0');
        }
        else if (base == 8)
        {
            if (*string < '0' || *string > '7')
                break;
            result = result*8 + (*string - '0');
        }
        else if (base == 16)
        {
            if (*string >= '0' && *string <= '9')
                result = result*16 + (*string - '0');
            else if (*string >= 'a' && *string <= 'f')
                result = result*16 + (*string - 'a' + 10);
            else if (*string >= 'A' && *string <= 'F')
                result = result*16 + (*string - 'A' + 10);
            else
                break;
        }
        else    /* invalid base */
            return 0;
        string++;
    }
    if (next)
        *next = string;

    return result;
}
