/* $Id: jfs_ifs.c,v 1.2 2003/09/21 08:42:56 pasha Exp $ */

static char *SCCSID = "@(#)1.120.1.2  12/2/99 11:13:28 src/jfs/ifs/jfs_ifs.c, sysjfs, w45.fs32, fixbld";
/*
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
 *   MODULE_NAME:       jfs_ifs.c
 *
 *   COMPONENT_NAME:    sysjfs
 *
 */

/*
 * Change History :
 * PS 2001-2003 apply IBM fixes
 *
 */

#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_NOPMAPI
#define INCL_LONGLONG

#define _cdecl __cdecl

#include <os2.h>
/* FIL_NAMEISVALID is not in bsedos.h */
#define FIL_NAMEISVALID 6
#define FIL_LVL7    7

#include <fsd.h>
#include <sysbloks.h>
#include <extboot.h>
#include <stdlib.h>

#include "jfs_os2.h"
#include "mmph.h"
#include "jfs_inode.h"
#include "jfs_dirent.h"
#include "jfs_cntl.h"
#include "jfs_lock.h"
#include "jfs_dmap.h"
#include "jfs_ea.h"
#include "jfs_superblock.h"
#include "jfs_proto.h"
#include "jfs_chkdsk.h"
#include "jfs_clrbblks.h"
#include "jfs_defragfs.h"
#include "jfs_extendfs.h"
#include "jfs_debug.h"
#include <priv/jfs_acl.h>
#include "jfs_dasdlim.h"                    // F226941
#include <uni_inln.h>

#define FIL_QUERYALLEAS 4   /* also defined in bsedosp.h */
//PS21092003 Begin
#define FILE_SWAP 0x100
extern struct dpbd * _Optlink _DosSelToFlat(struct dpbd *);
//PS End

#pragma pack(1)
struct CmdLock
{
    USHORT Cmd;           /* 0 for Lock */
    USHORT LockCnt;       /* number of locks that follow */
    ULONG TimeOut;        /* timeout in ms               */
};

struct fileio_lock
{
    USHORT Share;         /* 0 is exclusive 1 is shared */
    ULONG  Start;         /* byte offset of lock start  */
    ULONG  Length;        /* length of lock             */
};

struct CmdUnLock
{
    USHORT Cmd;           /* 1 for UnLock */
    USHORT LockCnt;       /* number of locks that follow */
};

struct fileio_unlock
{
    ULONG  Start;         /* byte offset of unlock start  */
    ULONG  Length;        /* length of unlock             */
};

struct CmdSeek
{
    USHORT Cmd;           /* 2 for seek                 */
    USHORT Method;        /* 0 for absolute             */
                          /* 1 for relative to current  */
                          /* 2 for relative to EOF      */
    LONG  Position;       /* position or delta          */
    LONG  Actual;         /* actual position seeked to  */
};

struct CmdIO
{
    USHORT Cmd;           /* 3 for read 4 for write     */
    void * _Seg16 Buffer; /* data buffer                */
    USHORT BufferLen;     /* buffer length              */
    USHORT Actual;        /* actual bytes transferred   */

};
#pragma pack()


struct jfs_sffsd
{
    struct vnode * vnode_ptr;
    struct file_lock * lock_anchor;
    uint32 reserved[4];
};

/*****
 *
 * Externs
 *
 */
unsigned long   nomounts = 0;   /* Init error occurred.  Do not mount anything*/
unsigned long   SyncOnClose = 0;/* set to 1 to cause synchronous write on close */
unsigned long   SyncRunning = 0;    /* Insures only one jfssync thread */
event_t     SyncEvent;  /* Event jfssync thread sleeps on */

extern struct ucred *pcred;     /* for calls to vfs/vnode ops */
extern struct vfs *vfs_anchor;     /* used to find log VPB      */

extern int32 LazyOff;
extern int32 SyncTime;
extern int32 maxSyncTime;
extern int32 minSyncTime;
extern uint32 CacheSize;
extern pool_t * lock_pool;         /* pool of lock structures        */
extern pool_t *cache_list_pool;
extern struct cCache cachemgr;                  // D230860
extern int32    TlocksLow;                      //PS21092003

extern XSPINLOCK_T  jfs_IOCacheLock;
/*****
 *
 * Prototypes for calls to top-level JFS routines.
 *
 */
int32   jfs_init2(void);
int     jfs_mount(struct vfs *, struct ucred *);
int     jfs_umount(struct vfs *, int, struct ucred *);
int     jfs_sync(struct vfs *);

int     jfs_cntl(struct vfs *, int, caddr_t, size_t, struct ucred *);
int     jfs_link(struct vnode *, struct vnode *, char *, struct ucred *);
int     jfs_mkdir(struct vnode *, UniChar *, int, EAOP *, struct ucred *);
int     jfs_remove(struct vnode *, struct vnode *, UniChar *, boolean_t,
                   struct ucred *);
int     jfs_rename(struct vnode *, struct vnode *, UniChar *,
                   struct vnode *, struct vnode *, UniChar *,
                   struct dasd_usage *, struct dasd_usage *, uint32); //F226941
int     jfs_rmdir(struct vnode *, struct vnode *, UniChar *, struct ucred *);
int     jfs_open(struct vnode *, int, int, EAOP *, struct ucred *);
int     jfs_create(struct vnode *, struct vnode **, int, int, UniChar *,
                   EAOP *, struct ucred *);
int     jfs_hold(struct vnode *);
int     jfs_rele(struct vnode *);
int     jfs_close(struct vnode *, int, caddr_t, struct ucred *);
int     jfs_read(struct vnode *, int64, caddr_t, int64 *, int32);
int     jfs_write(struct vnode *, int64, caddr_t, int64 *, int32);
int     jfs_access(struct vnode *, int, int, struct ucred *);
int     jfs_getattr(struct vnode *, struct vattr *, struct ucred *);
int     jfs_setattr(struct vnode *, int, int, int, int, struct ucred *);
int     jfs_fsync(struct vnode *, int, int, struct ucred *);
int     jfs_ftruncate(struct vnode *, int, offset_t, caddr_t, struct ucred *);
int     jfs_pageio(struct vnode *, struct PageCmdHeader *);
int32   jfs_SetLevel1Info (inode_t *, FILESTATUS *, int32);
int32   jfs_SetLevel11Info (inode_t *, FILESTATUS3L *, int32);
int32   jfs_GetLevel1Info (inode_t *, FILESTATUS *, int32);
int32   jfs_GetLevel11Info (inode_t *, FILESTATUS3L *, int32);
int32   jfs_SetLevel2Info (inode_t *, EAOP *, int32);
int32   jfs_GetLevel2Info (inode_t *, FILESTATUS2 *, int32);
int32   jfs_GetLevel12Info (inode_t *, FILESTATUS4L *, int32);
int32   jfs_GetLevel3Info (inode_t *ip, GEALIST *pgeal, FEALIST *pfeal,
                           ULONG *oError);
void    jfs_EABufFree (char *, int);

int     pathlookup(struct vfs *, pathlook_t *, struct ucred *);
int     pathlookup_pc(struct vfs *, UniChar *, UniChar *);
int     readdir(struct vnode *, struct fsfd *, char *, uint32, uint32 *, uint32,
                uint32, EAOP *, struct ucred *,struct dpdb *);   //PS21092003

int32   get_fileinfo(struct inode *, char **, uint32, UniChar *,
                     uint32, uint32, uint32, EAOP *, unsigned short);
void    get_vattr(struct inode *, struct vattr *);
void    Net_To_Dos_Date(uint32, uint16 *, uint16 *);
int32   dasd_read(int64, caddr_t, int64 *, struct vfs *);
int32   dasd_write(int64, caddr_t, int64 *, struct vfs *);
int32   rawRead(inode_t *, int64, cbuf_t **);
int32   rawWrite(inode_t *, cbuf_t *, uint32 flag);
int32   readSuper(inode_t *, cbuf_t **);
int32   jfs_readFAC(inode_t *, int64, int64 *, cache_list_t *);
int32   jfs_getacc0(inode_t *, struct jfs_access_parms *);
int32   jfs_getacc1(inode_t *, struct jfs_access_parms *, char *, int32 *);
int32   jfs_SetACL(inode_t *, void *, int32);
int32   jfs_enumACL(struct vnode *,struct jfs_access_parms *, char *, int32 *);
enum_instance_t *alloc_enum(void);
int32   verify_enum(enum_instance_t *);
int32   free_enum(enum_instance_t *);
void    enum_exit(int32 pid);
int32   hard_quiesce(struct vfs *);
int32   hard_resume(struct vfs *);
void    iwritelocklist(int, ...);               // F226941
#ifdef _JFS_LAZYCOMMIT
int32   jfs_lazycommit(void);                   // D230860
#endif
#ifdef _JFS_CACHESTATS
void    getCacheStats(struct cachestats *);         // D230860
#endif

int32 jfs_filelock( struct vnode *vnode_ptr,      /* object vnode          */
                    struct file_lock * * lock_anchor,   /* lock anchor           */
                    uint32 lock_count,                  /* lock count            */
                    struct lock_info * lock_list,       /* list of locks         */
                    uint32 timeout,                     /* timeout in ms         */
                    uint32 unlock_count,                /* unlock count          */
                    struct unlock_info * unlock_list,   /* list of unlocks       */
                    uint16 pid,                         /* owner pid             */
                    int32  longlock);           /* True if longlong locks */

void wake_pending_locks(uint32 start,
                        uint32 length,
                        struct file_lock * search_list);

int32 check_read_locks(struct gnode *, int64, int64, unsigned short,
                       struct file_lock **);

// BEGIN F226941

APIRET APIENTRY DASD_FSCTL (
                           union argdat    *pArgDat,
                           unsigned long   iArgType,
                           unsigned long   func,
                           char            *pParm,
                           unsigned long   lenParm,
                           unsigned short  *plenParmIO,
                           char            *pData,
                           unsigned long   lenData,
                           unsigned short  *plenDataIO);

APIRET APIENTRY DASD_FSCTL2 (
                            union argdat    *pArgDat,
                            unsigned long   iArgType,
                            unsigned long   func,
                            char            *pParm,
                            unsigned long   lenParm,
                            unsigned short  *plenParmIO,
                            char            *pData,
                            unsigned long   lenData,
                            unsigned short  *plenDataIO);

// END F226941

APIRET APIENTRY FS32_READ(
                         struct sffsi    *psffsi,
                         struct sffsd    *psffsd,
                         void            *pData,
                         unsigned long   *pLen,
                         unsigned long   IOflag);

APIRET APIENTRY FS32_WRITE(
                          struct sffsi    *psffsi,
                          struct sffsd    *psffsd,
                          void            *pData,
                          unsigned long   *pLen,
                          unsigned long   IOflag);

APIRET APIENTRY FS32_CHGFILEPTRL (
                                 struct sffsi    *psffsi,
                                 struct sffsd    *psffsd,
                                 long long       offset,
                                 unsigned long   type,
                                 unsigned long   IOflag);

struct  vfs *VPBtoVFS(SHANDLE hVPB);
struct  vfs *PATHtoVFS(char *);

#ifdef  _JFS_STATISTICS
int     jfsStatistics(char *, size_t);
int     ncStatistics(caddr_t, int32);
int     bmStatistics(caddr_t, int32);
int     lmStatistics(caddr_t, int32);
#endif  /* _JFS_STATISTICS */

int32   jfs_strtoUCS(UniChar *, char *, int32);
int32   jfs_strfromUCS(char *, UniChar *, int32, int32);

/*****
 *
 * Global Data
 *
 */

#define LONESTAR L"*"

extern pool_t   *unipool;

extern  struct InfoSegLDT * _Seg16 pLIS;


/* log volume info */
uint32 logVPB = 0;
struct vpfsi *logvpfs = 0;
void * _Seg16 logStrat2;
void (* APIENTRY logStrat3)();

/* page volume */
unsigned short page_hVPB = 0;

/* lock defines */
#define LOCK_EXPIRED   0x0L
#define LOCK_WAKEUP    0x1L
#define LOCK_CANCELED  0x2L

/* FILEIO DEFINES */
#define FILEIO_LOCK    0
#define FILEIO_UNLOCK  1
#define FILEIO_SEEK    2
#define FILEIO_READ    3
#define FILEIO_WRITE   4

#define SEEK_ABSOLUTE   0
#define SEEK_REL_CURR   1
#define SEEK_REL_EOF    2


/*
 * FS Operation Trace Data
 *
 * define _FS_OP_TRACE to turn on operation tracing
 */
#ifdef  _FS_OP_TRACE

/* Operation IDs
 */

#define FINDFIRST       0x10    /* directory operations */
#define FINDNEXT        0x11
#define FINDFROMNAME    0x12
#define FINDCLOSE       0x13

#define OPENCREATE      0x20    /* open file operations */
#define CLOSE           0x21
#define READ            0x22
#define WRITE           0x23
#define COMMIT          0x24
#define NEWSIZE         0x25
#define CHGFILEPTR      0x26
#define IOCTL           0x27
#define FILEINFO        0x28
#define FILEIO          0x2A
#define FILELOCKS       0x2B
#define CANCELLOCKREQ   0x2C
#define READFAC     0x2D

#define FILEATTRIBUTE   0x30    /* name based operations */
#define PATHINFO        0x31
#define CHDIR           0x32
#define COPY            0x33
#define MOVE            0x34
#define DELETE          0x35
#define MKDIR           0x36
#define RMDIR           0x37

#define MOUNT           0x40    /* volume based operations */
#define FSINFORM        0x41
#define FSCTL           0x42
#define FLUSHBUF        0x43
#define SHUTDOWN        0x44

#define OPENPAGEFILE    0x50    /* page space operations */
#define ALLOCATEPGSP    0x51
#define DOPAGEIO        0x52
#define SETSWAP         0x53

#define EXIT            0x60    /* others (not supported) */
#define ATTACH          0x61
#define NMPIPE          0x62
#define PROCESSNAME     0x63
#define FINDNOTIFYCLOSE 0x6A
#define FINDNOTIFYFIRST 0x6B
#define FINDNOTIFYNEXT  0x6C


/* Trace Structure
 *
 * The structure is ordered to make it easier to look at in the debugger --
 * byte swapping causes the op to be displayed in the first position.
 * The sequence number must be first so that the rest of the structure can
 * be compared easily using memcmp.
 */
struct fs_optrc
{
    uint16          fot_seq;        /* sequence number (must be first) */
    uint8           fot_subop;      /* sub-operation; flags or type */
    uint8           fot_op;         /* operation; see defines above */
    caddr_t         fot_other;      /* other operation dependent data */
    caddr_t         fot_nmnode;     /* file name or vnode pointer */
    uint32          fot_ptid;       /* process and thread id */
};

#define TRC_EVENTS      256

struct  fs_optrc trc_lst[TRC_EVENTS];
struct  fs_optrc *trc_cur = 0;
struct  fs_optrc *trc_prev = 0;
uint16  trc_sequence = 0;

void    fsoptrace(uint32, uint32, caddr_t, caddr_t);

SPINLOCK_T  optrace_lock;

#define FSoptrace(op, subop, other, nmnode) \
        fsoptrace((op), (uint32)(subop), (caddr_t)(other), (caddr_t)(nmnode))

#else  /* _FS_OP_TRACE */
#define FSoptrace(op, subop, other, nmnode)
#endif /* _FS_OP_TRACE */

#pragma pack(1)
/* Operation counts
 */
struct _opstats
{
    int allocatepagespace;  /* 00 */
    int attach;             /* 01 */
    int chdir;              /* 02 */
    int chgfileptr;         /* 03 */
    int close;              /* 04 */
    int commit;             /* 05 */
    int copy;               /* 06 */
    int delete;             /* 07 */
    int dopageio;           /* 08 */
    int exit;               /* 09 */
    int fileattribute;      /* 0a */
    int fileinfo;           /* 0b */
    int fileio;             /* 0c */
    int findclose;          /* 0d */
    int findfirst;          /* 0e */
    int findfromname;       /* 0f */
    int findnext;           /* 10 */
    int findnotifyclose;    /* 11 */
    int findnotifyfirst;    /* 12 */
    int findnotifynext;     /* 13 */
    int flushbuf;           /* 14 */
    int fsctl;              /* 15 */
    int fsinfo;             /* 16 */
    int ioctl;              /* 17 */
    int mkdir;              /* 18 */
    int mount;              /* 19 */
    int move;               /* 1a */
    int newsize;            /* 1b */
    int nmpipe;             /* 1c */
    int opencreate;         /* 1d */
    int openpagefile;       /* 1e */
    int pathinfo;           /* 1f */
    int processname;        /* 20 */
    int read;               /* 21 */
    int rmdir;              /* 22 */
    int setswap;            /* 23 */
    int shutdown;           /* 24 */
    int write;              /* 25 */
    int cancellockreq;      /* 26 */
    int filelocks;          /* 27 */
    int cacheread;          /* 28 */
    int returnfilecache;    /* 29 */
    int LastCalled;         /* PS */
} opstats;
#pragma pack()

/*
 * Message stuff - messages defined in misc.asm in 16-bit data segment
 */
#define LT16M_MSGNO 0
extern  uint8   LT16DriveLtr;   /* position within message for drive letter */
int32 FSH_CRITERROR(int32, int32, char *);  /* misc.asm */

/*****
 *
 * Declarations for the FSD entry points.
 *
 */

APIRET APIENTRY FS32_ALLOCATEPAGESPACE (
                                       struct sffsi    *psffsi,
                                       struct sffsd    *psffsd,
                                       unsigned long   size,
                                       unsigned long   contig)
{
    int rc;
    struct vnode *vp = *(struct vnode **)psffsd;

    FSoptrace(ALLOCATEPGSP, contig, size, vp);
    opstats.allocatepagespace++;
#ifdef FreeJFS
    opstats.LastCalled=0;             // PS
#endif

    /* The ftruncate operation will extend or reduce the size
     * of the file.  There is no option for meeting the "contig"
     * requirement, but extend will give the best contiguity
     * that is possible anyway.
     */
    rc = jfs_ftruncate(vp, 0, size, 0, pcred);
    if (rc == 0)
        psffsi->sfi_sizel = size;

    return rc;
}

APIRET APIENTRY FS32_CANCELLOCKREQUESTL (
                                        struct sffsi    *psffsi,
                                        struct sffsd    *psffsd,
                                        FILELOCKL   *pLockRange)
{
    struct jfs_sffsd * fsd_data_ptr;
    struct file_lock * curr_lock;               /* pointer to current lock     */
    struct file_lock * tmp_lock;                /* pointer to         lock     */
    inode_t * ip;
    struct gnode * gnode_ptr;                   /* pointer to gnode for file   */
    struct vnode *vp;
    uint32 rc = ERROR_CANCEL_VIOLATION;
    uint32 rc2;

    fsd_data_ptr = (struct jfs_sffsd *) psffsd;
    vp = fsd_data_ptr->vnode_ptr;

    if (vp == 0)        /* DASD open */
        return ERROR_INVALID_HANDLE;

    FSoptrace(CANCELLOCKREQ, 0, psffsi, pLockRange);
    opstats.cancellockreq++;
#ifdef FreeJFS
    opstats.LastCalled=26;             // PS
#endif

    ip = VP2IP(vp);  /* get the ip pointer for locking */
    gnode_ptr = VP2GP(vp);  /* get the gp */

    IWRITE_LOCK(ip); 
    /* search the pending lock list and release the specified lock */
    for (curr_lock = gnode_ptr->gn_pendinglocks; curr_lock != NULL;
        curr_lock = curr_lock->next_lock)
    {
        /* lock pending ?? */
        if ((pLockRange->lOffset == curr_lock->lock_start)  &&
            ((pLockRange->lOffset + pLockRange->lRange - 1) ==
             curr_lock->lock_end)  &&
            (&fsd_data_ptr->lock_anchor == curr_lock->anchor))
        {
            jEVENT(0,("Lock Cancel Req.: start:%x len:%x,anchor:%x,Eid:%x\n",
                      pLockRange->lOffset,pLockRange->lRange,
                      &fsd_data_ptr->lock_anchor,curr_lock));

            /* wake up the thread that is waiting on the lock */
            KernWakeup((ULONG)curr_lock, /* addr of lock is eventid */
                       WAKEUP_DATA, /* pass data to thread      */
                       0, /* return of num threads woken, don't care */
                       LOCK_CANCELED); /* tell thread he was cancelled  */

            rc = 0;

        }
    }

    IWRITE_UNLOCK(ip);

    return(rc);
}

APIRET APIENTRY FS32_CANCELLOCKREQUEST (
                                       struct sffsi    *psffsi,
                                       struct sffsd    *psffsd,
                                       FILELOCK    *pLockRange)
{
    FILELOCKL   LockRangeL;

    LockRangeL.lOffset = pLockRange->lOffset;
    LockRangeL.lRange = pLockRange->lRange;

    return FS32_CANCELLOCKREQUESTL(psffsi, psffsd, &LockRangeL);
}

APIRET APIENTRY FS32_CHDIR (
                           unsigned short  flag,
                           struct cdfsi    *pcdfsi,
                           struct cdfsd    *pcdfsd,
                           char        *pDir,
                           unsigned short  iCurDirEnd)
{
    pathlook_t      pls;    /* pathname lookup structure */
    struct vnode    *vp;    /* vnode of current working directory */
    int             rc;
    struct vfs  *vfsp;
    UniChar         *Dir;

    FSoptrace(CHDIR, flag, pcdfsi, pDir);
    opstats.chdir++;
#ifdef FreeJFS
    opstats.LastCalled=02;             // PS
#endif

    if (flag == CD_FREE)
        return NO_ERROR;

    Dir = (UniChar *)allocpool(unipool, 0);
    if (Dir == 0)
        return ENOMEM;

    if (flag == CD_EXPLICIT)
    {
        if (jfs_strtoUCS(Dir, pDir, MAXPATHLEN) == -1)
        {
            rc = ERROR_BUFFER_OVERFLOW;
            goto out;
        }
    }
    else if (flag == CD_VERIFY)
    {
        if (jfs_strtoUCS(Dir, pcdfsi->cdi_curdir, MAXPATHLEN) == -1)
        {
            rc = ERROR_BUFFER_OVERFLOW;
            goto out;
        }
    }
    else
    {
        rc = EINVAL;
        goto out;
    }

    pls.path = Dir;
    vfsp = VPBtoVFS(pcdfsi->cdi_hVPB);

    DLIM_init(&pls.dlim, DLIM_READONLY);            // F226941 //!!! в исходной вообще ее нет
    rc = pathlookup(vfsp, &pls, pcred);
    if (rc)
        goto out;

    jfs_rele(pls.dvp);
    switch (pls.statlast)
    {
        case PL_NOENT:
            rc= ENOTDIR;
            break;
        case PL_WCARD:
            rc = ENOENT;
            break;
        case PL_EXIST:
            jfs_rele(pls.vp);
            rc = ENOTDIR;
            break;

        case PL_ISDIR:
            /* We don't need to hold a reference on the vnode.  OS/2
             * guarantees that we won't be asked to remove a process's
             * current directory.  Additionally, we may not get a
             * CD_FREE request prior to an unmount.
             */
            jfs_rele(pls.vp);
            rc = NO_ERROR;
            break;
    }
    out:
    if (Dir)
        freepool(unipool, (caddr_t *)Dir);

    return rc;
}

APIRET APIENTRY FS32_CHGFILEPTRL (
                                 struct sffsi    *psffsi,
                                 struct sffsd    *psffsd,
                                 long long   offset,
                                 unsigned long   type,
                                 unsigned long   IOflag)
{
    struct vnode    *vp = *(struct vnode **)psffsd;
    ULONG rc = 0;

    FSoptrace(CHGFILEPTR, type, (long)offset, psffsi);
    opstats.chgfileptr++;
#ifdef FreeJFS
    opstats.LastCalled=03;             // PS
#endif

    /* Set a new current position in the file based on
     * type and offset specifications.
     */
    switch (type)
    {
        case CFP_RELBEGIN:
            /* Offset is absolute. */
            psffsi->sfi_positionl = offset;
            break;

        case CFP_RELCUR:
            /* Offset is relative to current offset. */
            psffsi->sfi_positionl += offset;
            break;

        case CFP_RELEND:
            /* Offset is relative to end of file.
             * Current end of file must be retrieved from the inode.
             */
            if (vp == 0)
                /* Dasd open */
                psffsi->sfi_positionl = psffsi->sfi_sizel + offset;
            else
                psffsi->sfi_positionl = VP2IP(vp)->i_size + offset;
            break;
        default:
            rc = ERROR_INVALID_FUNCTION;
    }
    return(rc);
}

APIRET APIENTRY FS32_CHGFILEPTR (
                                struct sffsi    *psffsi,
                                struct sffsd    *psffsd,
                                long            offset,
                                unsigned long   type,
                                unsigned long   IOflag)
{
    return FS32_CHGFILEPTRL(psffsi, psffsd, (long long)offset, type, IOflag);
}

APIRET APIENTRY FS32_CLOSE (
                           unsigned short  type,
                           unsigned short  IOflag,
                           struct sffsi    *psffsi,
                           struct sffsd    *psffsd)
{
    int rc;
    struct jfs_sffsd * fsd_data_ptr;
    struct file_lock * curr_lock;   /* pointer to current lock  */
    struct file_lock * tmp_lock;    /* pointer to lock      */
    inode_t * ip;
    struct gnode * gnode_ptr;   /* pointer to gnode for file    */
    struct vnode *vp;

    FSoptrace(CLOSE, type, psffsi, vp);
    opstats.close++;

#ifdef FreeJFS
    opstats.LastCalled=04;             // PS
#endif

    fsd_data_ptr = (struct jfs_sffsd *) psffsd;
    vp = fsd_data_ptr->vnode_ptr;

    if (vp == 0)            /* Opened with OPEN_FLAGS_DASD */
        return NO_ERROR;

    /* Only operate on last closes.
     */
    if (type != FS_CL_FORSYS)
        return NO_ERROR;

    // Until we have a sync daemon (or something of the sort),
    // it seems like a good idea to get the data written out sometime
    // before shutdown.  Set SyncOnClose to 0 to avoid this.
    // Update: we now have a sync daemon.  SyncOnClose is now set
    // to zero by default.
    if ((SyncOnClose) &&
        (psffsi->sfi_mode & (OPEN_ACCESS_WRITEONLY | OPEN_ACCESS_READWRITE)))
        jfs_fsync(vp, IOflag, 0, pcred);


    ip = VP2IP(vp);  /* get the ip pointer for locking */
    gnode_ptr = VP2GP(vp);  /* get the gp */

    IWRITE_LOCK(ip);

    /* release range locks from per open locklist */

    curr_lock = fsd_data_ptr->lock_anchor;
    while (curr_lock != NULL)
    {
        /* first delete from the per file locklist */
        if (curr_lock->prev_lock != NULL)
        {
            curr_lock->prev_lock->next_lock = curr_lock->next_lock;
        }
        else  /* fix head of list for 1st item */
        {
            gnode_ptr->gn_filelocks = curr_lock->next_lock;
        }
        if (curr_lock->next_lock != NULL)
        {
            curr_lock->next_lock->prev_lock = curr_lock->prev_lock;
        }

        /* now delete from the per open locklist */
        if (curr_lock->vnode_prev_lock != NULL)
        {
            curr_lock->vnode_prev_lock->vnode_next_lock = curr_lock->vnode_next_lock;
            if (curr_lock->vnode_next_lock != NULL)
            {
                curr_lock->vnode_next_lock->vnode_prev_lock = curr_lock->vnode_prev_lock;
            }
        }
        else  /* fix head of list for 1st item */
        {
            fsd_data_ptr->lock_anchor = curr_lock->vnode_next_lock;
            if (curr_lock->vnode_next_lock != NULL)
            {
                curr_lock->vnode_next_lock->vnode_prev_lock = NULL;
            }
        }
        /* if any locks are pending, see if they can be freed */
        if (gnode_ptr->gn_pendinglocks)
        {
            wake_pending_locks(curr_lock->lock_start,
                               curr_lock->lock_end - curr_lock->lock_start,
                               gnode_ptr->gn_pendinglocks);
        }

        tmp_lock = curr_lock;
        curr_lock = curr_lock->vnode_next_lock;

        /* return the lock struct to the pool */
        freepool(lock_pool, (VOID *)tmp_lock);
    } /* end while */

    rc = jfs_close(vp, psffsi->sfi_mode, NULL, pcred);

    dlim_release(ip);                   // F226941

    IWRITE_UNLOCK(ip);

    jfs_rele(vp);
    return rc;
}

APIRET APIENTRY FS32_COMMIT (
                            unsigned short  type,
                            unsigned short  IOflag,
                            struct sffsi    *psffsi,
                            struct sffsd    *psffsd)
{
    int rc;
    struct vnode *vp = *(struct vnode **)psffsd;

    FSoptrace(COMMIT, type, psffsi, vp);
    opstats.commit++;

#ifdef FreeJFS
    opstats.LastCalled=05;             // PS
#endif

    if (vp == 0)            /* opened with OPEN_FLAGS_DASD */
        return NO_ERROR;

    rc = jfs_fsync(vp, IOflag, 0, pcred);

    return rc;
}

APIRET APIENTRY FS32_DELETE (
                            struct cdfsi    *pcdfsi,
                            struct cdfsd    *pcdfsd,
                            char        *pFile,
                            unsigned short  iCurDirEnd)
{
    inode_t     *dip;                   // F226941
    int32       have_locks;             // F226941
    inode_t     *ip;                    // F226941
    pathlook_t      pls;
    int             rc;
    struct vfs  *vfsp;
    UniChar         *File;
    struct dpbd  *dlock; //PS21092003
    inode_t      *ipmnt; //PS21092003

    FSoptrace(DELETE, 0, pcdfsi, pFile);
    opstats.delete++;

#ifdef FreeJFS
    opstats.LastCalled=07;             // PS
#endif

    DLIM_init(&pls.dlim, DLIM_RELEASE);         // F226941

    File = (UniChar *)allocpool(unipool, 0);
    if (File == 0)
        return ENOMEM;

    if (jfs_strtoUCS(File, pFile, MAXPATHLEN) == -1)
    {
        rc = ERROR_BUFFER_OVERFLOW;
        goto out;
    }

    pls.path = File;
    vfsp = VPBtoVFS(pcdfsi->cdi_hVPB);
    rc = pathlookup(vfsp, &pls, pcred);
    if (rc)
        goto out;

    if (pls.statlast == PL_NOENT)
    {
        rc = ENOENT;
        goto fs_delete2;
    }
    if (pls.statlast == PL_ISDIR)
    {
        rc = EISDIR;
        goto fs_delete1;
    }

// BEGIN F226941
    ip = VP2IP(pls.vp);
    dip = VP2IP(pls.dvp);
    if (pls.dlim.num_limits)
    {
        DLIM_WRITE_LOCK(ip, &pls.dlim);

        ip->i_dasdlim = &pls.dlim;
        dip->i_dasdlim = ip->i_dasdlim;
        have_locks = TRUE;
    }
    else
        have_locks = FALSE;
// END F226941

    rc = jfs_remove(pls.vp, pls.dvp, pls.pathlast, have_locks, pcred);

// BEGIN F226941
    if (have_locks)
    {
        ip->i_dasdlim = 0;         //PS21092003
        dip->i_dasdlim = 0;
        dasd_write_unlock(ip, &pls.dlim);
    }
// END F226941

    fs_delete1:
    jfs_rele(pls.vp);
    fs_delete2:
    if (pls.dlim.num_limits == 0)               // F226941
        jfs_rele(pls.dvp);
    out:
    DLIM_free(&pls.dlim);                   // F226941
    freepool(unipool, (caddr_t *)File);

    return rc;
}

APIRET APIENTRY FS32_DOPAGEIO(
                             struct sffsi     *psffsi,
                             struct sffsd     *psffsd,
                             struct PageCmdHeader *pList)
{
    int rc;
    struct vnode *vp = *(struct vnode **)psffsd;

    FSoptrace(DOPAGEIO, 0, pList, vp);
    opstats.dopageio++;
#ifdef FreeJFS
    opstats.LastCalled=8;             // PS
#endif

    /* call the pager i/o routine
     */
    rc = jfs_pageio(vp, pList);

    return rc;
}

APIRET APIENTRY FS32_EXIT (
                          uint32  uid,
                          uint32  pid,
                          uint32  pdb)
{
#ifdef FreeJFS
    opstats.exit++;
    opstats.LastCalled=9;             // PS
#endif
    enum_exit(pid);
    return NO_ERROR;
}

APIRET APIENTRY FS32_FILEATTRIBUTE (
                                   unsigned short  flag,
                                   struct cdfsi    *pcdfsi,
                                   struct cdfsd    *pcdfsd,
                                   char        *pName,
                                   unsigned short  iCurDirEnd,
                                   unsigned short  *pAttr)
{
    int32           rc;
    struct inode    *ip;
    pathlook_t      pls;
    struct vfs  *vfsp;
    UniChar         *Name;

    FSoptrace(FILEATTRIBUTE, flag, pAttr, pName);
    opstats.fileattribute++;

#ifdef FreeJFS
    opstats.LastCalled=0xa;             // PS
#endif
    Name = (UniChar *)allocpool(unipool, 0);
    if (Name == 0)
        return ENOMEM;

    if (jfs_strtoUCS(Name, pName, MAXPATHLEN) == -1)
    {
        rc = ERROR_BUFFER_OVERFLOW;
        goto out;
    }

    /* look up the vnode for the path
     */
    pls.path = Name;
    vfsp = VPBtoVFS(pcdfsi->cdi_hVPB);
    DLIM_init(&pls.dlim, DLIM_READONLY);            // F226941
    rc = pathlookup(vfsp, &pls, pcred);
    if (rc)
        goto out;
    jfs_rele(pls.dvp);

    /* get the vnode for the requested file or directory.
     */
    switch (pls.statlast)
    {
        case PL_NOENT:
            rc = ENOENT;
            break;

        case PL_EXIST:
        case PL_ISDIR:
            ip = VP2IP(pls.vp);
            if (flag == FA_RETRIEVE)
            {
                *pAttr = (ip->i_mode & ~INEWNAME) >> ATTRSHIFT;
                rc = NO_ERROR;
            }
            else if (isReadOnly(ip))
                rc = ERROR_WRITE_PROTECT;
            else
            {
                assert (flag == FA_SET);

                IWRITE_LOCK(ip);

                /* set the attributes.
                 */
                ip->i_mode = (ip->i_mode & ~IRASH)
                             | ((*pAttr << ATTRSHIFT) & IRASH);

                /* set flag here instad of calling imark() so dates
                 * aren't changed
                 */
                ip->i_flag |= ICHG;

                IWRITE_UNLOCK(ip);
            }

            jfs_rele(pls.vp);

            break;
    }
    out:
    freepool(unipool, (caddr_t *)Name);

    return rc;
}

APIRET APIENTRY FS32_FILEINFO (
                              unsigned short  flag,
                              struct sffsi    *psffsi,
                              struct sffsd    *psffsd,
                              unsigned short  level,
                              char            *pData,
                              unsigned short  cbData,
                              unsigned short  IOflag)
{
    struct inode *ip;
    struct vnode *vp = *(struct vnode **)psffsd;
    char data[sizeof(FILESTATUS4L)];
    int rc, rc2;
    struct vattr    vattr;

    FSoptrace(FILEINFO, ((flag << 4) | level), pData, vp);
    opstats.fileinfo++;
#ifdef FreeJFS
    opstats.LastCalled=0xb;             // PS
#endif

    if (vp == 0)
    {
        jEVENT(1,("fs_fileinfo called for DASD open\n"));
        return ERROR_INVALID_FUNCTION;
    }

    /* Protect our buffer */
    if (cbData > sizeof(FILESTATUS4L))
        cbData = sizeof(FILESTATUS4L);

    if (flag == FI_SET || level == FIL_QUERYEASFROMLIST ||
        level == FIL_QUERYALLEAS)
    {
        rc = KernCopyIn(data, pData, cbData);
        if (rc != NO_ERROR)
            return rc;
    }

    /* get the inode pointer */
    ip = VP2IP(vp);

    if (flag == FI_RETRIEVE)
    {
        EAOP    eaop;

        IREAD_LOCK (ip);

        switch (level)
        {
            case FIL_STANDARD:
                rc = jfs_GetLevel1Info (ip,
                                        (FILESTATUS *) data, cbData);
                break;
            case FIL_STANDARDL:
                rc = jfs_GetLevel11Info (ip,
                                         (FILESTATUS3L *) data, cbData);
                break;
            case FIL_QUERYEASIZE:
                rc = jfs_GetLevel2Info (ip,
                                        (FILESTATUS2 *) data, cbData);
                break;
            case FIL_QUERYEASIZEL:
                rc = jfs_GetLevel12Info (ip,
                                         (FILESTATUS4L *) data, cbData);
                break;
            case FIL_QUERYALLEAS:
                ((EAOP *)data)->fpGEAList = 0;
            case FIL_QUERYEASFROMLIST:
                /* validate the EAOP structure to ensure all of
                 * the fields exist.
                 */
                if (cbData < sizeof(EAOP))
                {
                    rc =  ERROR_BUFFER_OVERFLOW;
                    break;
                }
                rc = jfs_GetLevel3Info(ip,
                                       ((EAOP *)data)->fpGEAList,
                                       ((EAOP *)data)->fpFEAList,
                                       &(((EAOP *)pData)->oError));
                break;
            default:
                rc = ERROR_INVALID_LEVEL;
                break;
        }

        if (level == FIL_STANDARD || level == FIL_QUERYEASIZE ||
            level == FIL_STANDARDL || level == FIL_QUERYEASIZEL )
        {
            /* get information from the inode */
            get_vattr(ip, &vattr);

            /* Set time stamps */
            Net_To_Dos_Date(vattr.va_otime.tv_sec,
                            &psffsi->sfi_cdate, &psffsi->sfi_ctime);
            Net_To_Dos_Date(vattr.va_atime.tv_sec,
                            &psffsi->sfi_adate, &psffsi->sfi_atime);
            Net_To_Dos_Date(vattr.va_mtime.tv_sec,
                            &psffsi->sfi_mdate, &psffsi->sfi_mtime);

            rc2 = KernCopyOut(pData, data, cbData);
            if (rc == 0)
                rc = rc2;
        }


        IREAD_UNLOCK (ip);
    }
    else if (isReadOnly(ip))
        rc = ERROR_WRITE_PROTECT;
    else
    {
        assert(flag == FI_SET);

        switch (level)
        {
            case FIL_STANDARD:
                rc = jfs_SetLevel1Info (ip,
                                        (FILESTATUS *) data, cbData);
                break;
            case FIL_STANDARDL:
                rc = jfs_SetLevel11Info (ip,
                                         (FILESTATUS3L *) data, cbData);
                break;
            case FIL_QUERYEASIZE:
                IWRITE_LOCK(ip);        // F226941
                rc = jfs_SetLevel2Info (ip,
                                        (EAOP *) data, cbData);
                DLIM_WRITE_UNLOCK(ip, ip->i_dasdlim); // F226941
                break;
            default:
                rc = ERROR_INVALID_LEVEL;
                break;
        }
        if (! rc)
        {
            /* Set timestamp flags */
            get_vattr(ip,&vattr);                                //PS21092003
            psffsi->sfi_tstamp |= ST_PCREAT|ST_PWRITE|ST_PREAD;
            psffsi->sfi_tstamp &= ~(ST_SCREAT|ST_SWRITE|ST_SREAD);
            psffsi->sfi_DOSattr = vattr.va_mode >> ATTRSHIFT;    //PS21092003

            /* Set time stamps */
            Net_To_Dos_Date(vattr.va_otime.tv_sec,
                            &psffsi->sfi_cdate, &psffsi->sfi_ctime);
            Net_To_Dos_Date(vattr.va_atime.tv_sec,
                            &psffsi->sfi_adate, &psffsi->sfi_atime);
            Net_To_Dos_Date(vattr.va_mtime.tv_sec,
                            &psffsi->sfi_mdate, &psffsi->sfi_mtime);
        }
    }
    return(rc);
}

APIRET APIENTRY FS32_FILEIO (
                            struct sffsi    *psffsi,
                            struct sffsd    *psffsd,
                            char            *pCmdList,
                            unsigned short  cbCmdList,
                            unsigned short  *poError,
                            unsigned short  IOflag)
{
    struct CmdIO * io_cmd;                   /* ptr to IO cmd structure   */
    struct CmdSeek * seek_cmd;               /* ptr to seek cmd struct    */
    struct CmdLock * lock_cmd;               /* ptr to lock cmd struct    */
    struct CmdUnLock * unlock_cmd;            /* ptr to unlock cmd struct  */
    uint32 rc = 0;
    uint32 rc2;
    KernVMLock_t lockHandle;         /* lock handle       */
    KernPageList_t pagelist[16];         /* Dummy pagelist        */
    ULONG num_pages;
    UCHAR cmd_buffer[256];                   /* local cmd buffer          */
    void * user_buffer;                      /* flat ptr to user buffer   */
    UCHAR * cmd_list;                        /* ptr to local command list */
    UCHAR * curr_cmd;                        /* ptr to current command    */
    ULONG buffer_len;
    struct jfs_sffsd * fsd_data_ptr;
    short error;                 /* value to be copied to poError */

    FSoptrace(FILEIO, cbCmdList, pCmdList, psffsi);
    opstats.fileio++;

#ifdef FreeJFS
    opstats.LastCalled=0xc;             // PS
#endif

    jEVENT(0,("fs_fileio\n"));

    fsd_data_ptr = (struct jfs_sffsd *) psffsd;

    if (cbCmdList > 256)
    {
        /* Expensive system call, so only lock if its a large list */
        rc = KernVMLock(VMDHL_WRITE, pCmdList, cbCmdList, &lockHandle, pagelist,
                        &num_pages);
        ASSERT(num_pages < 17);
        if (rc != NO_ERROR)
        {
            error = 0;
            (void) KernCopyOut(poError, &error, 2);
            return rc;
        }
        cmd_list = pCmdList;
    }
    else
    {
        /* copy entire command list to kernel mem so we don't have to keep doing */
        /* verify access calls                                                   */
        rc = KernCopyIn(cmd_buffer, pCmdList, cbCmdList);
        cmd_list = cmd_buffer;
    }

    curr_cmd = cmd_list;

    while ((rc == 0) && ((ULONG)curr_cmd < ((ULONG)cmd_list + cbCmdList)))
    {
        switch ((USHORT)*curr_cmd)
        {
            case FILEIO_LOCK:
                lock_cmd = (struct CmdLock *) curr_cmd;
                rc = jfs_filelock(fsd_data_ptr->vnode_ptr,     /* object vnode          */
                                  &fsd_data_ptr->lock_anchor,            /* lock anchor           */
                                  lock_cmd->LockCnt,                     /* lock count            */
                                  (struct lock_info*)(curr_cmd + sizeof(struct CmdLock)),
                                  /* list of locks         */
                                  lock_cmd->TimeOut,                     /* timeout in ms         */
                                  0,                                     /* unlock count          */
                                  NULL,                                  /* list of unlocks       */
                                  psffsi->sfi_PID,                       /* owning PID            */
                                  FALSE);                                /* Not longlong          */
                curr_cmd += (sizeof(struct CmdLock) +
                             (lock_cmd->LockCnt * sizeof(struct fileio_lock)));
                break;
            case FILEIO_UNLOCK:
                unlock_cmd = (struct CmdUnLock *) curr_cmd;
                rc = jfs_filelock(fsd_data_ptr->vnode_ptr,     /* object vnode          */
                                  &fsd_data_ptr->lock_anchor,            /* lock anchor           */
                                  0,                                     /* lock count            */
                                  NULL,                                  /* list of   locks       */
                                  0,                                     /* timeout in ms         */
                                  unlock_cmd->LockCnt,                   /* unlock count          */
                                  (struct unlock_info*)(curr_cmd + sizeof(struct CmdUnLock)),
                                  /* list of unlocks         */
                                  psffsi->sfi_PID,                       /* owning PID            */
                                  FALSE);                                /* Not longlong          */
                curr_cmd += (sizeof(struct CmdUnLock) +
                             (unlock_cmd->LockCnt * sizeof(struct fileio_unlock)));
                break;
            case FILEIO_READ:
                io_cmd = (struct CmdIO *) curr_cmd;
                /* fs32_read take and adress of ulong for buffer size so */
                /* we need to copy the ushort from params to ulong       */
                buffer_len = io_cmd->BufferLen;
                user_buffer = io_cmd->Buffer ;
                rc = FS32_READ(psffsi, psffsd, user_buffer, &buffer_len,(ULONG)IOflag);
                /* put the actual byte return field */
                io_cmd->Actual = (USHORT) buffer_len;
                /* move to next cmd */
                curr_cmd += sizeof(struct CmdIO);
                break;
            case FILEIO_WRITE:
                io_cmd = (struct CmdIO *) curr_cmd;
                /* fs32_write take and adress of ulong for buffer size so */
                /* we need to copy the ushort from params to ulong       */
                buffer_len = io_cmd->BufferLen;
                user_buffer = io_cmd->Buffer ;
                rc = FS32_WRITE(psffsi, psffsd, user_buffer, &buffer_len,(ULONG)IOflag);
                /* put the actual byte return field */
                io_cmd->Actual = (USHORT) buffer_len;
                /* move to next cmd */
                curr_cmd += sizeof(struct CmdIO);
                break;
            case FILEIO_SEEK:
                seek_cmd = (struct CmdSeek *) curr_cmd;
                rc = FS32_CHGFILEPTRL(psffsi, psffsd, (long long)seek_cmd->Position,
                                      (ULONG)seek_cmd->Method, (ULONG)IOflag);
                if (!rc)
                {
                    seek_cmd->Actual = seek_cmd->Position;
                }
                curr_cmd += sizeof(struct CmdSeek);
                break;
            default:
                rc = ERROR_INVALID_PARAMETER;
                break;

        } /* end switch */
    } /* end while */

    if (rc)  /* if error occurred, update error pointer */
    {
        error = curr_cmd - cmd_list;
        (void) KernCopyOut(poError, &error, 2);
    }

    if (cbCmdList > 256)
    {
        rc2 = KernVMUnlock(&lockHandle);
        ASSERT(rc2 == 0);
    }
    else
    {
        /* Some fields (Actual) may have changed, so copy back to user's buffer */
        (void) KernCopyOut(pCmdList, cmd_list, cbCmdList);
    }
    return(rc);
}

APIRET APIENTRY FS32_FILELOCKSL (
                                struct sffsi    *psffsi,
                                struct sffsd    *psffsd,
                                FILELOCKL   *pUnlockRange,
                                FILELOCKL   *pLockRange,
                                unsigned long   timeout,
                                unsigned long   flags)
{

    uint32 rc = 0;
    struct vnode * vnode_ptr = *(struct vnode **)psffsd;
    struct jfs_sffsd * fsd_data_ptr;
    struct lock_infol lock_list;
    struct unlock_infol unlock_list;
    uint32 lock_count = 0;
    uint32 unlock_count = 0;

    FSoptrace(FILELOCKS, flags, psffsi, timeout);
    opstats.filelocks++;

#ifdef FreeJFS
    opstats.LastCalled=0x27;             // PS
#endif

    fsd_data_ptr = (struct jfs_sffsd *) psffsd;

    if ((pUnlockRange != NULL) && (pUnlockRange->lRange != 0))
    {
        unlock_list.start  = pUnlockRange->lOffset;
        unlock_list.length = pUnlockRange->lRange;
        unlock_count = 1;
    }

    if ((pLockRange != NULL) && (pLockRange->lRange != 0))
    {
        lock_list.start  = pLockRange->lOffset;
        lock_list.length = pLockRange->lRange;
        lock_count = 1;
        if (flags & 1L)
        {
            lock_list.share = TRUE;
        }
        else
        {
            lock_list.share = FALSE;
        }
    }

    if (lock_count == 0 && unlock_count == 0)
    {
        rc = ERROR_LOCK_VIOLATION;
    }
    else
    {
        rc =  jfs_filelock(vnode_ptr,                      /* object vnode          */
                           &fsd_data_ptr->lock_anchor,            /* lock anchor           */
                           lock_count,                            /* lock count            */
                           (struct lock_info *)&lock_list,        /* list of locks         */
                           timeout,                               /* timeout in ms         */
                           unlock_count,                          /* unlock count          */
                           (struct unlock_info *)&unlock_list,    /* list of unlocks       */
                           psffsi->sfi_PID,                       /* owning PID            */
                           TRUE);                 /* longlong          */

    }
    return(rc);
}

APIRET APIENTRY FS32_FILELOCKS (
                               struct sffsi    *psffsi,
                               struct sffsd    *psffsd,
                               FILELOCK    *pUnlockRange,
                               FILELOCK    *pLockRange,
                               unsigned long   timeout,
                               unsigned long   flags)
{

    uint32 rc = 0;
    struct vnode * vnode_ptr = *(struct vnode **)psffsd;
    struct jfs_sffsd * fsd_data_ptr;
    struct lock_info lock_list;
    struct unlock_info unlock_list;
    uint32 lock_count = 0;
    uint32 unlock_count = 0;

    FSoptrace(FILELOCKS, flags, psffsi, timeout);
    opstats.filelocks++;
#ifdef FreeJFS
    opstats.LastCalled=0x27;             // PS
#endif

    fsd_data_ptr = (struct jfs_sffsd *) psffsd;

    if ((pUnlockRange != NULL) && (pUnlockRange->lRange != 0))
    {
        unlock_list.start  = pUnlockRange->lOffset;
        unlock_list.length = pUnlockRange->lRange;
        unlock_count = 1;
    }

    if ((pLockRange != NULL) && (pLockRange->lRange != 0))
    {
        lock_list.start  = pLockRange->lOffset;
        lock_list.length = pLockRange->lRange;
        lock_count = 1;
        if (flags & 1L)
        {
            lock_list.share = TRUE;
        }
        else
        {
            lock_list.share = FALSE;
        }
    }

    if (lock_count == 0 && unlock_count == 0)
    {
        rc = ERROR_LOCK_VIOLATION;
    }
    else
    {
        rc =  jfs_filelock(vnode_ptr,                      /* object vnode          */
                           &fsd_data_ptr->lock_anchor,            /* lock anchor           */
                           lock_count,                            /* lock count            */
                           &lock_list,                            /* list of locks         */
                           timeout,                               /* timeout in ms         */
                           unlock_count,                          /* unlock count          */
                           &unlock_list,                          /* list of unlocks       */
                           psffsi->sfi_PID,                       /* owning PID            */
                           FALSE);                /* Not longlong      */

    }
    return(rc);
}

APIRET APIENTRY FS32_FINDCLOSE (
                               struct fsfsi    *pfsfsi,
                               struct fsfsd    *pfsfsd)
{
    struct fsfd     *fsfp = (struct fsfd *)pfsfsd;

    FSoptrace(FINDCLOSE, 0, pfsfsi, fsfp);
    opstats.findclose++;
#ifdef FreeJFS
    opstats.LastCalled=0xd;             // PS
#endif

    /* if there is not an open search struct then FS32_FINDFIRST
     * cleaned it up.  just say ok.
     */
    if (fsfp->fsd_inum != 0)
    {
        /* free the pattern buffers if a wild card search was done
         */
        if (fsfp->fsd_pattern.name != LONESTAR)
            freepool(unipool, (caddr_t *)fsfp->fsd_pattern.name);
        if (fsfp->fsd_lastmatch != NULL)
            freepool(unipool, (caddr_t *)fsfp->fsd_lastmatch);
    }

    return NO_ERROR ;
}

APIRET APIENTRY FS32_FINDFIRST (
                               struct cdfsi    *pcdfsi,
                               struct cdfsd    *pcdfsd,
                               char        *pName,
                               unsigned short  iCurDirEnd,
                               unsigned short  attr,
                               struct fsfsi    *pfsfsi,
                               struct fsfsd    *pfsfsd,
                               char        *pData,
                               unsigned short  cbData,
                               unsigned short  *pcMatch,
                               unsigned short  level,
                               unsigned short  flags)
{
    uint32          rc, len;
    uint32          matchcnt = *pcMatch;
    struct vnode    *dvp;
    struct fsfd     *fsfp = (struct fsfd *)pfsfsd;
    char            *ubuf;
    char            *posbuf;
    uint32          namlen;
    pathlook_t      pls;
    EAOP            eaop, *eaopp;
    uint32          ffhdsize;               /* size of ffbuf header */
    struct vfs  *vfsp;
    UniChar         *Name = 0;
    struct dpbd     *dlock;                 //PS21092003
    inode_t         *ipmnt;                 //PS21092003

    FSoptrace(FINDFIRST, ((flags << 4) | level), pName, fsfp);
    opstats.findfirst++;

#ifdef FreeJFS
    opstats.LastCalled=0xe;             // PS
#endif

    /* validate level requested
     */
    if (((level < FIL_STANDARD) || (level > FIL_QUERYEASFROMLIST)) &&
        ((level < FIL_STANDARDL) || (level > FIL_QUERYEASFROMLISTL)))
    {
        *pcMatch = 0;
        return ERROR_INVALID_FUNCTION;
    }

    Name = (UniChar *)allocpool(unipool, 0);
    if (Name == 0)
    {
        rc =  ENOMEM;
        goto cleanup;
    }

    if (jfs_strtoUCS(Name, pName, MAXPATHLEN) == -1)
    {
        rc = ERROR_BUFFER_OVERFLOW;
        goto cleanup;
    }

    /* initialize directory search information
     */
    fsfp->fsd_inum = 0;
    fsfp->fsd_attr = attr;
    fsfp->fsd_pattern.name = LONESTAR;
    fsfp->fsd_lastmatch = NULL;
    fsfp->fsd_offset = 0x0000ffff;

    /* lookup the vnode for the path
     */
    pls.path = Name;
    vfsp = VPBtoVFS(pcdfsi->cdi_hVPB);
//PS21092003 Begin
    if(vfsp == 0)
       return ERROR_INVALID_PARAMETER;
    ipmnt=((inode_t *)vfsp->vfs_data) ? (((inode_t *)vfsp->vfs_data)->i_ipmnt):0;
    if( !ipmnt)     
        return ENFILE;
     dlock = (struct dpbd *)(ipmnt->i_devfp);
     dlock = _DosSelToFlat(dlock);
     if( dlock->dpb_drive_lock)
        return ENFILE;
//PS21092003 End
    DLIM_init(&pls.dlim, DLIM_READONLY);            // F226941
    rc = pathlookup(vfsp, &pls, pcred);
    if (rc)
        goto cleanup;

    /* set the directory to be read (either the last component
     * returned by pathlookup, or the directory preceding it).
     */
    switch (pls.statlast)
    {
        case PL_NOENT:
            /* no match in last component
             */
            jfs_rele(pls.dvp);
            *pcMatch = 0;
            rc = ERROR_NO_MORE_FILES;
            goto cleanup;

        case PL_EXIST:
        case PL_ISDIR:
            /* last component matches a file or directory
             */
            /* Release the inode for the file and drop through to wildcard
     * case.
     */
            jfs_rele(pls.vp);

        case PL_WCARD:
            /* wild card in last component
             */
            /* save the directory vnode for the search */
            dvp = pls.dvp;

            /* special case compatibility check:
             * "*.*" should work the same as "*".
             */
            if (UniStrcmp(pls.pathlast, L"*.*") == 0)
                pls.pathlast[1] = 0; //+++PS более правильней

            /* If level == 3, there will be an EAOP structure at beginning
             * which needs to be passed down.  Need to return user data in
             * the buffer after the EAOP structure.
             */
            if ((level == FIL_QUERYEASFROMLIST) ||
                (level == FIL_QUERYEASFROMLISTL))
            {
                if ((cbData < sizeof(EAOP)) ||
                    KernCopyIn(&eaop, pData, sizeof(eaop)))
                {
                    rc =  ERROR_BUFFER_OVERFLOW;
                    goto out;
                }
                eaopp = &eaop;

                /* Setup to point to space after EAOP for return data */
                ubuf = pData + sizeof(EAOP);
                cbData -= sizeof(EAOP);
            }
            else
            {
                eaopp = NULL;
                ubuf = pData;
            }
            break;

        default:
            /* failure to set a valid return code in statlast
             * is a programming error in pathlookup().
             */
            assert(0);
    }

    /* allocate the pattern buffer and save it for subsequent calls
     */
    fsfp->fsd_pattern.namlen = len = UniStrlen(pls.pathlast);
    if ((len == 1) && (pls.pathlast[0] == '*'))
        fsfp->fsd_pattern.name = LONESTAR;
    else
    {
        fsfp->fsd_pattern.name = (UniChar *)allocpool(unipool, 0);
        if (fsfp->fsd_pattern.name == NULL)
        {
            /* Set fsd_pattern to LONESTAR so we don't try to
             * free null pointer
             */
            fsfp->fsd_pattern.name = LONESTAR;
            rc = ENOMEM;
            goto out;
        }

        /* copy the last component to the pattern buffer and convert
         * to upper case.
         */
        UniStrcpy(fsfp->fsd_pattern.name, pls.pathlast);
        UniStrupr(fsfp->fsd_pattern.name);
    }

    /* allocate the buffer for the last name matched
     */
    fsfp->fsd_lastmatch = (UniChar *)allocpool(unipool, 0);
    if (fsfp->fsd_lastmatch == NULL)
    {
        rc = ENOMEM;
        goto out;
    }

    /* read entries from the directory until we have matches
     */
    fsfp->fsd_inum = VP2IP(dvp)->i_number;
    matchcnt = *pcMatch;
    rc = readdir(dvp, fsfp, ubuf, cbData, &matchcnt, level, flags, eaopp,
                 pcred,dlock);

    /* return match count
     */
    *pcMatch = matchcnt;
    out:
    /* release the vnode
     */
    jfs_rele(dvp);

    if ((rc != 0) && (rc != ERROR_EAS_DIDNT_FIT))
    {
        /* free the pattern buffer
         */
        if (fsfp->fsd_pattern.name != LONESTAR)
            freepool(unipool, (caddr_t *)fsfp->fsd_pattern.name);
        if (fsfp->fsd_lastmatch != NULL)
            freepool(unipool, (caddr_t *)fsfp->fsd_lastmatch);
    }
    cleanup:
    if (Name)
        freepool(unipool, (caddr_t *)Name);

    if ((rc != 0) && (rc != ERROR_EAS_DIDNT_FIT))
        *pcMatch = 0;   /* clear match count */

    return rc;
}

APIRET APIENTRY FS32_FINDFROMNAME (
                                  struct fsfsi    *pfsfsi,
                                  struct fsfsd    *pfsfsd,
                                  char        *pData,
                                  unsigned short  cbData,
                                  unsigned short  *pcMatch,
                                  unsigned short  level,
                                  unsigned long   position,
                                  char        *pName,
                                  unsigned short  flags)
{
    uint32          rc;
    uint32          matchcnt = 0;
    struct fsfd     *fsfp = (struct fsfd *)pfsfsd;
    char            *ubuf;
    EAOP            eaop, *eaopp;
    UniChar         *Name;
    inode_t     *ip;
    struct vnode    *vp;
    struct vfs  *vfsp;
    inode_t     *ipmnt;
    struct dpbd *dlock;

    FSoptrace(FINDFROMNAME, ((flags << 4) | level), position,
              fsfp->fsd_inum);

    vfsp = VPBtoVFS(pfsfsi->fsi_hVPB);
    if (vfsp == 0)
        return ERROR_INVALID_PARAMETER;

//PS21092003 Begin
    ipmnt=((inode_t *)vfsp->vfs_data) ? (((inode_t *)vfsp->vfs_data)->i_ipmnt):0;
    if( !ipmnt)     
        return ENFILE;
     dlock = (struct dpbd *)(ipmnt->i_devfp);
     dlock = _DosSelToFlat(dlock);
     if( dlock->dpb_drive_lock)
        return ENFILE;
//PS21092003 End

    /* if there is not an open search struct then fs_findfirst
     * cleaned it up.  just say no.
     */
    if (fsfp->fsd_inum == 0)
    {
        *pcMatch = 0;
        return ERROR_NO_MORE_FILES;
    }

    /* validate level requested
     */
    if (((level < FIL_STANDARD) || (level > FIL_QUERYEASFROMLIST)) &&
        ((level < FIL_STANDARDL) || (level > FIL_QUERYEASFROMLISTL)))
    {
        *pcMatch = 0;
        return ERROR_INVALID_FUNCTION;
    }

    /* Get the inode */
    ICACHE_LOCK();
    rc = iget(vfsp, fsfp->fsd_inum, &ip, 0);
    ICACHE_UNLOCK();
    if (rc != 0)
    {
        rc = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }
    vp = IP2VP(ip);

    /* If level == 3, there will be an EAOP structure at beginning which
     * needs to be passed down.  Need to return user data in the buffer
     * after the EAOP structure.
     */
    if ((level == FIL_QUERYEASFROMLIST) || (level == FIL_QUERYEASFROMLISTL))
    {
        if ((cbData < sizeof(EAOP)) ||
            KernCopyIn(&eaop, pData, sizeof(eaop)) != NO_ERROR)
        {
            rc =  ERROR_BUFFER_OVERFLOW;
            goto out;
        }
        eaopp = &eaop;

        /* Setup to point to space after EAOP for return data */
        ubuf = pData + sizeof(EAOP);
        cbData -= sizeof(EAOP);
    }
    else
    {
        eaopp = NULL;
        ubuf = pData;
    }

    Name = (UniChar *)allocpool(unipool, 0);
    if (Name == 0)
    {
        rc = ENOMEM;
        goto out;
    }

    if (jfs_strtoUCS(Name, pName, MAXPATHLEN) == -1)
    {
        rc = ERROR_BUFFER_OVERFLOW;
        goto freeName;
    }

    /* Set the directory offset and findfromname to the values
     * specified by the caller.
     */
    fsfp->fsd_offset = position;
    UniStrcpy(fsfp->fsd_lastmatch, Name);

    /* read the directory
     */
    matchcnt = *pcMatch;
    rc = readdir(vp, fsfp, ubuf, cbData, &matchcnt, level, flags, eaopp,
                 pcred,dlock);                       //PS21092003
    freeName:
    freepool(unipool, (caddr_t *)Name);

    out:
    jfs_rele(vp);

    /* set match count
     */
    *pcMatch = matchcnt;

    cleanup:
    if ((rc != 0) && (rc != ERROR_EAS_DIDNT_FIT))
        *pcMatch = 0;

    return rc;
}

APIRET APIENTRY FS32_FINDNEXT (
                              struct fsfsi    *pfsfsi,
                              struct fsfsd    *pfsfsd,
                              char        *pData,
                              unsigned short  cbData,
                              unsigned short  *pcMatch,
                              unsigned short  level,
                              unsigned short  flags)
{
    uint32          rc;
    uint32          matchcnt = 0;
    struct fsfd     *fsfp = (struct fsfd *)pfsfsd;
    char            *ubuf;
    EAOP            eaop, *eaopp;
    inode_t     *ip;
    struct vnode    *vp;
    struct vfs  *vfsp;
    struct dpbd *dlock;       //PS21092003
    inode_t *ipmnt;           //PS21092003


    FSoptrace(FINDNEXT, ((flags << 4) | level), pfsfsi, fsfp->fsd_inum);
    opstats.findnext++;

#ifdef FreeJFS
    opstats.LastCalled=0x10;             // PS
#endif

    vfsp = VPBtoVFS(pfsfsi->fsi_hVPB);

    if (vfsp == 0)
        return ERROR_INVALID_PARAMETER;

//PS21092003 Begin
    ipmnt=((inode_t *)vfsp->vfs_data) ? (((inode_t *)vfsp->vfs_data)->i_ipmnt):0;
    if( !ipmnt)     
        return ENFILE;
     dlock = (struct dpbd *)(ipmnt->i_devfp);
     dlock = _DosSelToFlat(dlock);
     if( dlock->dpb_drive_lock)  //!!!! Проверить смещения!!! должно быть 16h
        return ENFILE;
//PS21092003 End
    
    /* if there is not an open search struct then fs_findfirst
     * cleaned it up.  just say no.
     */
    if (fsfp->fsd_inum == 0)
    {
        *pcMatch = 0;
        return ERROR_NO_MORE_FILES;
    }

    /* validate level requested
     */
    if (((level < FIL_STANDARD) || (level > FIL_QUERYEASFROMLIST)) &&
        ((level < FIL_STANDARDL) || (level > FIL_QUERYEASFROMLISTL)))
    {
        *pcMatch = 0;
        return ERROR_INVALID_FUNCTION;
    }

    /* Get the inode */
    ICACHE_LOCK();
    rc = iget(vfsp, fsfp->fsd_inum, &ip, 0);
    ICACHE_UNLOCK();
    if (rc != 0)
    {
        *pcMatch = 0;
        return ERROR_INVALID_PARAMETER;
    }
    vp = IP2VP(ip);

    /* If level == 3, there will be an EAOP structure at beginning which
     * needs to be passed down.  Need to return user data in the buffer
     * after the EAOP structure.
     */
    if ((level == FIL_QUERYEASFROMLIST) || (level == FIL_QUERYEASFROMLISTL))
    {
        if ((cbData < sizeof(EAOP)) ||
            KernCopyIn(&eaop, pData, sizeof(eaop)) != NO_ERROR)
        {
            rc =  ERROR_BUFFER_OVERFLOW;
            goto out;
        }
        eaopp = &eaop;

        /* Setup to point to space after EAOP for return data */
        ubuf = pData + sizeof(EAOP);
        cbData -= sizeof(EAOP);
    }
    else
    {
        eaopp = NULL;
        ubuf = pData;
    }

    /* read the directory
     */
    matchcnt = *pcMatch;
    rc = readdir(vp, fsfp, ubuf, cbData, &matchcnt, level, flags, eaopp,
                 pcred,dlock);
    out:
    jfs_rele(vp);

    /* set match count
     */
    if ((rc == 0) || (rc == ERROR_EAS_DIDNT_FIT))
        *pcMatch = matchcnt;
    else
        *pcMatch = 0;

    return rc;
}

APIRET APIENTRY FS32_FLUSHBUF (
                              unsigned short  hVPB,
                              unsigned short  flag)
{
    int rc;
    struct vfs *vfsp;

    FSoptrace(FLUSHBUF, flag, 0, hVPB);
    opstats.flushbuf++;

#ifdef FreeJFS
    opstats.LastCalled=0x14;             // PS
#endif

    vfsp = VPBtoVFS(hVPB);
    if (vfsp)
    {
        if (vfsp->vfs_flag & VFS_ACCEPT)
            rc = NO_ERROR;
        else
            rc = jfs_sync(vfsp);
    }
    else
        rc = EINVAL;

    return rc;
}

APIRET APIENTRY FS32_FSCTL (
                           union argdat    *pArgDat,
                           unsigned long   iArgType,
                           unsigned long   func,
                           char            *pParm,
                           unsigned long   lenParm,
                           unsigned short  *plenParmIO,
                           char            *pData,
                           unsigned long   lenData,
                           unsigned short  *plenDataIO)
{
    int     rc;
    extern struct InfoSegGDT * GIS;

    FSoptrace(FSCTL, func, GIS->SIS_BigTime, pArgDat);
    opstats.fsctl++;

// BEGIN F226941
    /*
     * This function is getting too long, offload to DASDlimit
     * specific function.
     */
    if ((func >= PBDASD_ADD) && (func <= PBDASD_CTLVOL))
        return DASD_FSCTL(pArgDat, iArgType, func, pParm, lenParm,
                          plenParmIO, pData, lenData, plenDataIO);
    if ((func >= JFS_DLIM_INIT) && (func <= JFS_DLIM_FC))
        return DASD_FSCTL2(pArgDat, iArgType, func, pParm, lenParm,
                           plenParmIO, pData, lenData, plenDataIO);
// END F226941

    switch (func)
    {
        /* IFSM required support */
        case FSCTL_ERROR_INFO:
            /* Return information about new error codes.  The IFSM
             * document says that it is not used, and HPFS doesn't
             * implement it.
             */
            rc = ERROR_INVALID_FUNCTION;
            break;

        case FSCTL_MAX_EASIZE:
            {
                EASIZEBUF easz = {MAXEASIZE, MAXEALISTSIZE};

                /* Return maximum Extended Attribute sizes if the
                 * arguments passed in are okay.
                 */
                if (lenData < sizeof (EASIZEBUF))
                    rc = ERROR_BUFFER_OVERFLOW;
                else
                    rc = KernCopyOut(pData, &easz, sizeof (EASIZEBUF));
                break;
            }
        case FSCTL_DAEMON_QUERY:
            {
                FSDDAEMON   fsddaem;
                FSDTHREAD   *fsdthread;

                /* Return information on worker threads we want started */

                if (lenData < sizeof (FSDDAEMON))
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                fsddaem.usMoreFlag = 0;
                if (SyncRunning)
                    fsddaem.usNumThreads = 0;
                else
                {
#ifdef _JFS_LAZYCOMMIT
                    fsddaem.usNumThreads = 2;       // D230860
#else
                    fsddaem.usNumThreads = 1;
#endif

                    fsdthread = &fsddaem.tdThrds[0];
                    fsdthread->usFunc = JFSCTL_SYNC;
                    fsdthread->usStackSize = 2048;
                    fsdthread->ulPriorityClass = PRTYC_REGULAR; // D230860
                    fsdthread->lPriorityLevel = 0;
// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
                    fsdthread++;
                    fsdthread->usFunc = JFSCTL_LAZYCOMMIT;
                    fsdthread->usStackSize = 2048;
                    fsdthread->ulPriorityClass = PRTYC_FOREGROUNDSERVER;
                    fsdthread->lPriorityLevel = 0;
#endif /* _JFS_LAZYCOMMIT */
// END D230860
                }
                rc = KernCopyOut(pData, &fsddaem, sizeof(FSDDAEMON));
                break;
            }
            /*
         * JFS specific support
         */

        case JFSCTL_SYNC:
            {
                struct vfs *vfsp;

                if (SyncRunning)
                    return ERROR_INVALID_FUNCTION;
                SyncRunning = 1;
                event_init(&SyncEvent);

                while (1)
                {
                    MMPHPresyncDaemon();  /* MMPH Performance Hook */
                    for (vfsp = vfs_anchor; vfsp != NULL;
                        vfsp = vfsp->vfs_next)
                        if (!(vfsp->vfs_flag & VFS_ACCEPT))
                            jfs_sync(vfsp); /* sync each vfs  */
                    MMPHPostsyncDaemon();  /* MMPH Performance Hook */
                    if(TlocksLow) continue;          //PS21092003
                    KernBlock((ULONG)&SyncEvent, SyncTime*1000, 0, 0, 0);
                }
                /* Never return */
            }
// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
        case JFSCTL_LAZYCOMMIT: /* lazy commit thread */
            rc = jfs_lazycommit();  /* Normally doesn't return */
            break;
#endif
// END D230860
        case JFSCTL_CHKDSK: /* for chkdsk utility */
            rc = jfs_chkdsk(pData, lenData, plenDataIO);
            break;

        case JFSCTL_CLRBBLKS: /* for clrbblks utility */
            rc = jfs_clrbblks(pData, lenData, plenDataIO);
            break;

        case JFSCTL_DEFRAGFS: /* for defragfs utility */
            rc = jfs_defragfs(pData, lenData, plenDataIO);
            break;

        case JFSCTL_EXTENDFS: /* for extendfs utility */
            rc = jfs_extendfs(pData, lenData, plenDataIO);
            break;

        case JFSCTL_QUIESCE:    /* LVM is extending partition */
        case JFSCTL_RESUME: /* LVM is done extending partition */
            {
                struct vfs  *vfsp;

                if (iArgType != FSCTL_ARG_CURDIR)
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                if (pArgDat->cd.pcdfsi->cdi_hVPB == page_hVPB)
                {
                    rc = ERROR_DEVICE_IN_USE;
                    break;
                }

                vfsp = VPBtoVFS(pArgDat->cd.pcdfsi->cdi_hVPB);
                if ((vfsp == NULL) || (vfsp->vfs_flag & VFS_ACCEPT))
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                if (vfsp->vfs_flag & VFS_READONLY)
                {
                    rc = ERROR_WRITE_PROTECT;
                    break;
                }
                if (func == JFSCTL_QUIESCE)
                    rc = hard_quiesce(vfsp);
                else
                    rc = hard_resume(vfsp);

                plenDataIO = 0;
                plenParmIO = 0;

                break;
            }

        case JFSCTL_LW_GET: /* Query lazywrite parameters */
            {
                struct LWPARMS  lwparms;
                int32       datalen = MIN(lenData, sizeof(lwparms)); // D230860

                if (datalen < OLD_LWPARMS_LEN)          // D230860
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }

                lwparms.LazyOff = LazyOff;
                lwparms.SyncTime = SyncTime;
                lwparms.MaxAge = maxSyncTime;
                lwparms.BufferIdle = minSyncTime;
                lwparms.CacheSize = CacheSize;
                lwparms.MinFree = cachemgr.minfree;     // D230860
                lwparms.MaxFree = cachemgr.maxfree;     // D230860

                rc = KernCopyOut(pData, &lwparms, datalen); // D230860
                if (rc)
                    *plenDataIO = 0;
                else
                    *plenDataIO = datalen;          // D230860

                break;
            }

        case JFSCTL_LW_SET: /* Set lazywrite parameters */
            {
                struct LWPARMS  lwparms;
                int32       datalen = MIN(*plenDataIO, sizeof(lwparms)); // D230860

                if (datalen < sizeof(lwparms))          // D230860
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                rc = KernCopyIn(&lwparms, pData, datalen);  // D230860
                if (rc)
                    break;

                LazyOff = lwparms.LazyOff;
                SyncTime = lwparms.SyncTime;
                maxSyncTime = lwparms.MaxAge;
                minSyncTime = lwparms.BufferIdle;

// BEGIN D230860
                if (datalen == sizeof(lwparms))
                {
                    /*
                     * Zero indicates no change
                     */
                    if (lwparms.MinFree)
                        cachemgr.minfree = lwparms.MinFree;
                    if (lwparms.MaxFree)
                        cachemgr.maxfree = lwparms.MaxFree;
                }
// END D230860

                KernWakeup((ULONG)&SyncEvent, 0, 0, 0);

                break;
            }

        case JFS_GETACC0:
        case JFS_GETACC1:
        case JFS_SETACC:
        case JFS_ADDACC:
        case JFS_DELACC:
            {
                int32           ACLparms_header[6];
                struct jfs_access_parms *ACLparms =
                (struct jfs_access_parms *)&ACLparms_header;
                int32           dlim_flag;      // F226941
                inode_t         *ip;
                int32           length;
                int32           oldtype;
                UniChar         *Path = 0;
                pathlook_t      pls;
                struct vfs      *vfsp;

                rc = NO_ERROR;

                if (iArgType != FSCTL_FSDNAME)
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                if (lenParm < sizeof(struct jfs_access_parms))
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }

                *plenParmIO = sizeof(ACLparms_header);
                rc = KernCopyIn(ACLparms, pParm, sizeof(ACLparms_header));
                if (rc)
                    break;

                /* Note: Pinned copy of ACLparms contains first 4 characters
                 * of achPath.  That is enough to determine if this path is
                 * on one of our mounted volumes.
                 */
                vfsp = PATHtoVFS(ACLparms->achPath);

                if (vfsp == 0)
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }

// BEGIN F226941
                if ((func == JFS_GETACC0) || (func == JFS_GETACC1))
                    dlim_flag = DLIM_READONLY;
                else
                    dlim_flag = DLIM_RELEASE;
                DLIM_init(&pls.dlim, dlim_flag);
// END F226941

                /*
                 *  Resolve the path to a vnode
                 */
                Path = (UniChar *)allocpool(unipool, 0);
                if (Path == 0)
                {
                    rc = ENOMEM;
                    break;
                }
                if (jfs_strtoUCS(Path,
                                 ((struct jfs_access_parms *)pParm)->achPath,
                                 MAXPATHLEN) == -1)
                {
                    rc = ERROR_BUFFER_OVERFLOW;
                    goto acl_out0;
                }
                pls.path = Path;
                rc = pathlookup(vfsp, &pls, pcred);
                if (rc)
                {
                    ACLparms->ulReturnCode = rc;
                    goto acl_out1;
                }
                if ((pls.statlast == PL_NOENT) || (pls.statlast == PL_WCARD))
                {
                    if (((func == JFS_GETACC0) || (func = JFS_GETACC1)) &&
                        (ACLparms->ulFlags & JFS_ACCESS_FLAGS_INHERIT))
                    {
                        /* Get ACL of parent directory */
                        ip = VP2IP(pls.dvp);
                        IREAD_LOCK(ip);
                        if (func == JFS_GETACC0)
                            jfs_getacc0(ip, ACLparms);
                        else
                        {
                            length = lenData;
                            jfs_getacc1(ip, ACLparms, pData,
                                        &length);
                            *plenDataIO = length;
                        }
                        IREAD_UNLOCK(ip);
                    }
                    else
                        ACLparms->ulReturnCode = ERROR_FILE_NOT_FOUND;

                    jfs_rele(pls.dvp);
                    goto acl_out1;
                }
                ip = VP2IP(pls.vp);

// BEGIN F226941
                if (pls.dlim.num_limits && (pls.statlast == PL_ISDIR) &&
                    (ip->i_number != 2))
                {
                    DLIM_add(&pls.dlim, ip, rc);
                    if (rc)
                        goto acl_out2;
                    pls.dlim.flag |= DLIM_IP_IN_LIST;
                }
// END F226941

                if (func == JFS_GETACC0)
                {
                    IREAD_LOCK(ip);
                    if ((ACLparms->ulFlags & JFS_ACCESS_FLAGS_INHERIT) &&
                        ((ip->i_mode & IFMT) == IFREG) &&
                        ((ip->i_acl.flag == 0) || ip->i_acl.size == 0))
                    {
                        /* Get ACL from parent directory */
                        IREAD_UNLOCK(ip);
                        ip = VP2IP(pls.dvp);
                        IREAD_LOCK(ip);
                        jfs_getacc0(ip, ACLparms);
                    }
                    else
                    {
                        jfs_getacc0(ip, ACLparms);
                        if ((ip->i_mode & IFMT) == IFDIR)
                            ACLparms->ulFlags |=
                            JFS_ACCESS_FLAGS_DIRECTORY;
                    }
                    IREAD_UNLOCK(ip);

                    goto acl_out2;
                }
                else if (func == JFS_GETACC1)
                {
                    IREAD_LOCK(ip);
                    if ((ACLparms->ulFlags & JFS_ACCESS_FLAGS_INHERIT) &&
                        ((ip->i_mode & IFMT) == IFREG) &&
                        ((ip->i_acl.flag == 0) || ip->i_acl.size == 0))
                    {
                        /* Get ACL from parent directory */
                        IREAD_UNLOCK(ip);
                        ip = VP2IP(pls.dvp);
                        IREAD_LOCK(ip);
                        length = lenData;
                        jfs_getacc1(ip, ACLparms, pData, &length);
                        *plenDataIO = length;
                    }
                    else
                    {
                        length = lenData;
                        jfs_getacc1(ip, ACLparms, pData, &length);
                        *plenDataIO = length;
                        if ((ip->i_mode & IFMT) == IFDIR)
                            ACLparms->ulFlags |=
                            JFS_ACCESS_FLAGS_DIRECTORY;
                    }
                    IREAD_UNLOCK(ip);

                    goto acl_out2;
                }
                else if (func == JFS_ADDACC)
                {
                    if ((ACLparms->ulFlags & JFS_ACCESS_FLAGS_TARGETACL_MASK)
                        != JFS_ACCESS_FLAGS_TARGETACL_ALL)
                    {
                        ACLparms->ulReturnCode = ERROR_INVALID_PARAMETER;
                        goto acl_out2;
                    }
                    DLIM_WRITE_LOCK_ATTACH(ip, &pls.dlim);  // F226941

                    if (ip->i_acl.flag && (ip->i_acl.size > 0))
                    {
                        ACLparms->ulReturnCode = ERROR_ALREADY_EXISTS;
                        ACLparms->cchData = ip->i_acl.size;
                        ACLparms->ulFlags = ip->i_acltype;
                        DLIM_WRITE_UNLOCK_DETACH(ip, &pls.dlim); // F226941
                        goto acl_out2;
                    }
                    ip->i_acltype = ACLparms->ulFlags &
                                    JFS_ACCESS_FLAGS_ACLTYPE_MASK;
                    ACLparms->ulReturnCode =
                    jfs_SetACL(ip, pData, (int32)*plenDataIO);
                    if (ACLparms->ulReturnCode != 0)
                        ip->i_acltype = 0;
                    DLIM_WRITE_UNLOCK_DETACH(ip, &pls.dlim);  // F226941
                }
                else if (func == JFS_SETACC)
                {
                    if ((ACLparms->ulFlags & JFS_ACCESS_FLAGS_TARGETACL_MASK)
                        != JFS_ACCESS_FLAGS_TARGETACL_ALL)
                    {
                        ACLparms->ulReturnCode = ERROR_INVALID_PARAMETER;
                        goto acl_out2;
                    }
                    DLIM_WRITE_LOCK_ATTACH(ip, &pls.dlim);  // F226941
                    if (ip->i_acl.flag == 0)
                    {
                        DLIM_WRITE_UNLOCK_DETACH(ip, &pls.dlim); // F226941
                        ACLparms->ulFlags = 0;
                        ACLparms->cchData = 0;
                        ACLparms->ulReturnCode = ERROR_FILE_EXISTS;
                        goto acl_out2;
                    }
                    if ((ip->i_acltype & JFS_ACCESS_FLAGS_ACLTYPE_MASK) !=
                        (ACLparms->ulFlags & JFS_ACCESS_FLAGS_ACLTYPE_MASK))
                    {
                        jEVENT(1,("SETACC, acltype mismatch\n"));
                        ACLparms->cchData = ip->i_acl.size;
                        ACLparms->ulFlags = ip->i_acltype;
                        DLIM_WRITE_UNLOCK_DETACH(ip, &pls.dlim); // F226941
                        ACLparms->ulReturnCode = ERROR_INVALID_DATA;
                        goto acl_out2;
                    }
                    oldtype = ip->i_acltype;
                    ip->i_acltype = ACLparms->ulFlags &
                                    JFS_ACCESS_FLAGS_ACLTYPE_MASK;
                    ACLparms->ulReturnCode =
                    jfs_SetACL(ip, pData, (int32)*plenDataIO);
                    if (ACLparms->ulReturnCode != 0)
                        ip->i_acltype = oldtype;
                    DLIM_WRITE_UNLOCK_DETACH(ip, &pls.dlim);  // F226941
                }
                else if (func == JFS_DELACC)
                {
                    if ((ACLparms->ulFlags & JFS_ACCESS_FLAGS_TARGETACL_MASK)
                        != JFS_ACCESS_FLAGS_TARGETACL_ALL)
                    {
                        ACLparms->ulReturnCode = ERROR_INVALID_PARAMETER;
                        goto acl_out2;
                    }
                    DLIM_WRITE_LOCK_ATTACH(ip, &pls.dlim);  // F226941
                    if (ip->i_acl.flag == 0)
                    {
                        DLIM_WRITE_UNLOCK_DETACH(ip, &pls.dlim); // F226941
                        ACLparms->ulFlags = 0;
                        ACLparms->cchData = 0;
                        ACLparms->ulReturnCode = ERROR_FILE_EXISTS;
                        goto acl_out2;
                    }
                    if ((ip->i_acltype & JFS_ACCESS_FLAGS_ACLTYPE_MASK) !=
                        (ACLparms->ulFlags & JFS_ACCESS_FLAGS_ACLTYPE_MASK))
                    {
                        jEVENT(1,("DELACC, acltype mismatch\n"));
                        ACLparms->cchData = ip->i_acl.size;
                        ACLparms->ulFlags = ip->i_acltype;
                        DLIM_WRITE_UNLOCK_DETACH(ip, &pls.dlim); // F226941
                        ACLparms->ulReturnCode = ERROR_INVALID_DATA;
                        goto acl_out2;
                    }
                    ACLparms->ulReturnCode = jfs_SetACL(ip, 0, 0);
                    DLIM_WRITE_UNLOCK_DETACH(ip, &pls.dlim);  // F226941
                }
                acl_out2:
                jfs_rele(pls.vp);
                if (pls.dlim.num_limits == 0)           // F226941
                    jfs_rele(pls.dvp);
                acl_out1:
                rc = KernCopyOut(pParm, ACLparms, sizeof(ACLparms_header));
                acl_out0:
                DLIM_free(&pls.dlim);               // F226941
                freepool(unipool, (caddr_t *)Path);
                break;
            }
        case JFS_FFACC:
            {
                int32           ACLparms_header[6];
                struct jfs_access_parms *ACLparms =
                (struct jfs_access_parms *)&ACLparms_header;
                struct enum_instance    *enumInst;
                unsigned long       hEnum_save;
                int32           length;
                UniChar         *Dir = 0;
                pathlook_t      pls;
                struct vfs      *vfsp;

                if (iArgType != FSCTL_FSDNAME)
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                if (lenParm < sizeof(struct jfs_access_parms))
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                *plenParmIO = sizeof(ACLparms_header);
                rc = KernCopyIn(ACLparms, pParm, sizeof(ACLparms_header));
                if (rc)
                    break;

                vfsp = PATHtoVFS(ACLparms->achPath);
                if (vfsp == 0)
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }

                Dir = (UniChar *)allocpool(unipool, 0);
                if (Dir == 0)
                {
                    rc = ENOMEM;
                    break;
                }
                if (jfs_strtoUCS(Dir,
                                 ((struct jfs_access_parms *)pParm)->achPath,
                                 MAXPATHLEN) == -1)
                {
                    rc = ERROR_BUFFER_OVERFLOW;
                    goto ffacc_out0;
                }

                pls.path = Dir;
                DLIM_init(&pls.dlim, DLIM_READONLY);        // F226941
                rc = pathlookup(vfsp, &pls, pcred);
                if (rc)
                {
                    ACLparms->ulReturnCode = rc;
                    goto ffacc_out1;
                }
                jfs_rele(pls.dvp);
                if ((pls.statlast == PL_NOENT) || (pls.statlast == PL_WCARD))
                {
                    ACLparms->ulReturnCode = ENOTDIR;
                    goto ffacc_out1;
                }
                if (pls.statlast == PL_EXIST)   /* File, not directory */
                {
                    jfs_rele(pls.vp);
                    ACLparms->ulReturnCode = ENOTDIR;
                    goto ffacc_out1;
                }

                if ((ACLparms->ulFlags & JFS_ACCESS_FLAGS_TARGETACL_MASK)
                    != JFS_ACCESS_FLAGS_TARGETACL_ALL)
                {
                    jfs_rele(pls.vp);
                    jEVENT(1,("FFACC, acltype mismatch\n"));
                    ACLparms->ulReturnCode = ERROR_INVALID_PARAMETER;
                    goto ffacc_out1;
                }

                enumInst = alloc_enum();
                if (enumInst == 0)
                {
                    jfs_rele(pls.vp);
                    ACLparms->ulReturnCode = ENOMEM;
                    goto ffacc_out1;
                }
                enumInst->enum_vfsp = pls.vp->v_vfsp;
                enumInst->enum_inum = VP2IP(pls.vp)->i_number;
                enumInst->enum_pid = pLIS->LIS_CurProcID;

                hEnum_save = ACLparms->hEnum;
                ACLparms->hEnum = (unsigned long)enumInst;
                length = lenData;

                rc = jfs_enumACL(pls.vp, ACLparms, pData, &length);

                jfs_rele(pls.vp);

                if (rc == 0)
                {
                    *plenDataIO = length;
                }
                else
                {
                    ACLparms->hEnum = hEnum_save;
                    free_enum(enumInst);
                }
                ACLparms->ulReturnCode = rc;
                ffacc_out1:
                rc = KernCopyOut(pParm, ACLparms, sizeof(ACLparms_header));
                ffacc_out0:
                freepool(unipool, (caddr_t *)Dir);

                break;
            }
        case JFS_FNACC:
            {
                int32           ACLparms_header[5];
                struct jfs_access_parms *ACLparms =
                (struct jfs_access_parms *)&ACLparms_header;
                struct enum_instance    *enumInst;
                inode_t         *ip;
                int32           length;
                struct vnode        *vp;

                if (iArgType != FSCTL_FSDNAME)
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                if (lenParm < sizeof(ACLparms_header))
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                *plenParmIO = sizeof(ACLparms_header);
                rc = KernCopyIn(ACLparms, pParm, sizeof(ACLparms_header));
                if (rc)
                    break;

                enumInst = (struct enum_instance *)ACLparms->hEnum;

                /* Verify that this is a valid enumInst */
                rc = verify_enum(enumInst);
                if (rc)
                    break;

                if ((ACLparms->ulFlags & JFS_ACCESS_FLAGS_TARGETACL_MASK)
                    != JFS_ACCESS_FLAGS_TARGETACL_ALL)
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }

                ICACHE_LOCK();
                rc = iget(enumInst->enum_vfsp, enumInst->enum_inum, &ip, 0);
                ICACHE_UNLOCK();
                if (rc != 0)
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                vp = IP2VP(ip);

                length = lenData;
                rc = jfs_enumACL(vp, ACLparms, pData, &length);

                jfs_rele(vp);

                if (rc == 0)
                    *plenDataIO = length;
                else
                    *plenDataIO = 0;

                ACLparms->ulReturnCode = rc;

                rc = KernCopyOut(pParm, ACLparms, sizeof(ACLparms_header));

                break;
            }
        case JFS_FCACC:
            {
                int32           ACLparms_header[5];
                struct jfs_access_parms *ACLparms =
                (struct jfs_access_parms *)&ACLparms_header;
                struct enum_instance    *enumInst;

                if (iArgType != FSCTL_FSDNAME)
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                if (lenParm < sizeof(ACLparms_header))
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                *plenParmIO = sizeof(ACLparms_header);
                rc = KernCopyIn(ACLparms, pParm, sizeof(ACLparms_header));
                if (rc)
                    break;

                enumInst = (struct enum_instance *)ACLparms->hEnum;

                ACLparms->ulReturnCode = free_enum(enumInst);
                ACLparms->hEnum = JFS_ACCESS_HANDLE_UNALLOCATED;

                rc = KernCopyOut(pParm, ACLparms, sizeof(ACLparms_header));

                break;
            }
// BEGIN F226941
        case JFS_VOLENUM:
            {
                char                drives[25];
                int32               i = 0;
                struct JFS_volenum_request  vereq;
                struct vfs          *vfsp;

                if (iArgType != FSCTL_FSDNAME)
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                if (*plenParmIO < sizeof(vereq))
                {
                    rc = ERROR_INVALID_PARAMETER;
                    break;
                }
                rc = KernCopyIn(&vereq, pParm, sizeof(vereq));
                if (rc)
                    break;

                if (vereq.ulLevel != 0)
                {
                    rc = ERROR_INVALID_LEVEL;
                    break;
                }

                for (vfsp = vfs_anchor; vfsp; vfsp = vfsp->vfs_next)
                    drives[i++] = vfsp->vfs_vpfsi->vpi_drive + 'A';

                drives[i] = 0;

                *plenDataIO = i+1;

                if (lenData < *plenDataIO)
                {
                    rc = ERROR_BUFFER_OVERFLOW;
                    break;
                }

                rc = KernCopyOut(pData, drives, *plenDataIO);

                break;
            }
// END F226941
// BEGIN D230860
#ifdef _JFS_CACHESTATS
        case JFSCTL_CACHESTATS:
            {
                struct cachestats cs;

                getCacheStats(&cs);

                rc = KernCopyOut(pData, &cs, sizeof(cs));

                break;
            }
#endif /* _JFS_CACHESTATS */
// END D230860
#ifdef FreeJFS
//+++PS
        case JFSCTL_CALLSTATS:
                if( lenData < sizeof(struct _opstats))
                    rc = ERROR_BUFFER_OVERFLOW;
                else
                    rc = KernCopyOut(pData, &opstats, sizeof(struct _opstats));
                break;
//+++PS
#endif
        default:
            jALERT(0,("fs_fsctl, function code %d\n", func));
            rc = ERROR_INVALID_FUNCTION;
    }
    return rc;
}

APIRET APIENTRY FS32_FSINFO (
                            unsigned short  flag,
                            unsigned short  hVPB,
                            char            *pData,
                            unsigned short  cbData,
                            unsigned short  level)
{
    cbuf_t          *bp;
    struct Extended_Boot    *extboot;
    FSALLOCATE      fsalloc;
    FSINFO          fsinfo;
    unsigned int             i,j;
    struct inode    *ipmnt;
    int             rc, rc_save = 0;
    struct superblock *sb;
    struct vfs      *vfsp;
    VOLUMELABEL     vol_label;
    struct  vpfsi   *vpfsip;
unsigned long a;

    FSoptrace(FSINFORM, flag, level, hVPB);
    opstats.fsinfo++;
#ifdef FreeJFS
    opstats.LastCalled=0x16;             // PS
#endif

    vfsp = VPBtoVFS(hVPB);

    if (flag == INFO_SET)
    {
        if ((vfsp == 0) || (vfsp->vfs_flag & VFS_ACCEPT))
            return EINVAL;

        if (level != FSIL_VOLSER)
            return EINVAL;

        if (cbData < 1)
            return ERROR_INSUFFICIENT_BUFFER;

        rc = KernCopyIn(&vol_label, pData,
                        MIN(cbData, sizeof(VOLUMELABEL)));
        if (rc)
            return rc;

        if (vol_label.cch > 11)
        {
            vol_label.cch = 11;
            rc_save = ERROR_LABEL_TOO_LONG;
        }

        if (vol_label.cch + 2 > cbData)
            return ERROR_INSUFFICIENT_BUFFER;

        if (vfsp->vfs_flag & VFS_READONLY)
            return ERROR_WRITE_PROTECT;

        /* Modify volume label in master boot record
         */
        ipmnt = ((struct inode *)(vfsp->vfs_data))->i_ipmnt;
        rc = rawRead(ipmnt, 0, &bp);
        if (rc)
            return rc;
        extboot = (struct Extended_Boot *)bp->cm_cdata;
        memcpy(extboot->Boot_Vol_Label, vol_label.szVolLabel,
               vol_label.cch);

        /* Fill the rest of the volume label with spaces
         */
        for (i = vol_label.cch; i < 11; i++)
            extboot->Boot_Vol_Label[i] = ' ';

        rc = rawWrite(ipmnt, bp, 1);
        if (rc)
            return rc;

        /* Modify volume label in superblock
         */
        rc = readSuper(ipmnt, &bp);
        if (rc)
            return rc;
        sb = (struct superblock *)bp->b_bdata;
        memcpy(sb->s_fpack, vol_label.szVolLabel, vol_label.cch);
        for (i = vol_label.cch; i < 11; i++)
            sb->s_fpack[i] = ' ';
        rc = rawWrite(ipmnt, bp, 1);
        if (rc)
            return rc;

        /* Modify volume label in volume parameters data structure
         */
        memcpy(vfsp->vfs_vpfsi->vpi_text, vol_label.szVolLabel,
               vol_label.cch);
        for (i = vol_label.cch; i < VPBTEXTLEN; i++)
            vfsp->vfs_vpfsi->vpi_text[i] = 0;

        return rc_save;
    }
    else if (flag == INFO_RETREIVE)
    {
        if (vfsp == 0)
        {
            if (level != FSIL_VOLSER)
                return EDIRTY;
            for (vfsp = vfs_anchor; vfsp != NULL;
                vfsp = vfsp->vfs_next)
                if (vfsp->vfs_logVPB == hVPB)
                {
                    vpfsip = vfsp->vfs_logvpfs;
                    goto out;
                }
            return EDIRTY;
        }

        if (vfsp->vfs_flag & VFS_ACCEPT)
            return EDIRTY;

        vpfsip = vfsp->vfs_vpfsi;

        out:
        if (level == FSIL_ALLOC)
        {
            if (cbData <  18)       /* sizeof(FSALLOCATE) */
                return ERROR_BUFFER_OVERFLOW;

            ipmnt = ((struct inode *)(vfsp->vfs_data))->i_ipmnt;

            fsalloc.cSectorUnit = 1 << ipmnt->i_l2bfactor;
            fsalloc.cUnit = ipmnt->i_ipbmap->i_bmap->db_mapsize;
            fsalloc.cUnitAvail = ipmnt->i_ipbmap->i_bmap->db_nfree;
            fsalloc.cbSector = 512;        //  ipmnt->i_pbsize

            rc = KernCopyOut(pData, &fsalloc, 18);
        }
        else if (level == FSIL_VOLSER)
        {
            if (cbData < 17)        /* sizeof(FSINFO) */
                return ERROR_BUFFER_OVERFLOW;

//                      fsinfo.ulVSN = vpfsip->vpi_vid;
            *(ULONG *)&fsinfo.fdateCreation = vpfsip->vpi_vid;
            strncpy(fsinfo.vol.szVolLabel, vpfsip->vpi_text, 12);
            /* Strip off trailing blanks
             */
            for (i = j = 0; (i < 11) && (vpfsip->vpi_text[i]); i++)
                if (vpfsip->vpi_text[i] != ' ')
                    j = i+1;
            fsinfo.vol.szVolLabel[j] = 0;
            fsinfo.vol.cch = j;

            rc = KernCopyOut(pData, &fsinfo, 17);
        }
        else
            rc = EINVAL;

        return(rc);
    }
    else
        return(EINVAL);
}

APIRET APIENTRY FS32_MKDIR (
                           struct cdfsi    *pcdfsi,
                           struct cdfsd    *pcdfsd,
                           char        *pName,
                           unsigned short  iCurDirEnd,
                           char        *pEABuf,
                           unsigned short  flags)
{
    int rc;
    pathlook_t      pls;    /* pathname lookup struct */
    FEALIST         *pfealist = NULL;
    inode_t         *dip;                   // F226941
    int32           tid;
    struct vfs  *vfsp;
    UniChar         *Name;

    FSoptrace(MKDIR, flags, pEABuf, pName);
    opstats.mkdir++;
#ifdef FreeJFS
    opstats.LastCalled=0x18;             // PS
#endif

    DLIM_init(&pls.dlim, DLIM_RELEASE);         // F226941

    Name = (UniChar *)allocpool(unipool, 0);
    if (Name == 0)
        return ENOMEM;

    if (jfs_strtoUCS(Name, pName, MAXPATHLEN) == -1)
    {
        rc = ERROR_BUFFER_OVERFLOW;
        goto out;
    }

    /* convert path name to parent directory vnode pointer
     */
    pls.path = Name;
    vfsp = VPBtoVFS(pcdfsi->cdi_hVPB);
    rc = pathlookup(vfsp, &pls, pcred);
    if (rc)
        goto out;

    if (pls.statlast == PL_NOENT)
    {
// BEGIN F226941
        dip = VP2IP(pls.dvp);
        DLIM_LOCK_DIRS(dip, &pls.dlim);        
        if (OVER_LIMIT(dip, 0))
            rc = ERROR_DISK_FULL;
        else
// END F226941
            /* Create the named directory.
             * The mode is zero because read, write, and search
             * permission are granted by default.
             */
            rc = jfs_mkdir(pls.dvp, pls.pathlast,
                           (flags & FILE_NEWNAME) << ATTRSHIFT,
                           (EAOP *)pEABuf, pcred);
        DLIM_UNLOCK_DIRS(dip, &pls.dlim);       // F226941
    }
    else
    {
        /* There is already an object with this name;
         * release the object and report the error.
         */
        jfs_rele(pls.vp);
        rc = EISDIR;
    }
    /*
     * If pathlookup created a list of ancestor inodes, the parent will
     * be released with the others
     */
    if (pls.dlim.num_limits == 0)               // F226941
        jfs_rele(pls.dvp);
    out:
    DLIM_free(&pls.dlim);                   // F226941
    freepool(unipool, (caddr_t *)Name);

    return(rc);
}

APIRET APIENTRY FS32_MOUNT (
                           unsigned long   flag,
                           struct vpfsi    *pvpfsi,
                           struct vpfsd    *pvpfsd,
                           unsigned long   hVPB,
                           char            *pBoot)
{
    int                     rc;
    struct vfs              *vfsp;
    struct vfs              *last_vfs;
    DriverCaps              *capsp;
    struct Extended_Boot    *extboot = (struct Extended_Boot *)pBoot;
    extern pool_t           *vfspool;
    extern struct vfs       *vfs_anchor;
    extern uint32           jfs_init_done;

    FSoptrace(MOUNT, flag, hVPB, pvpfsi);
    opstats.mount++;
#ifdef FreeJFS
    opstats.LastCalled=0x19;             // PS
#endif

    jEVENT(0,("Mount: flag = %d\n", flag));

    /*
 * nomounts is also set if phase 2 initialization fails.
     */
    if (nomounts)
        return(ERROR_INVALID_FUNCTION);

    /* complete the initialization of the jfs
     */
    if (!jfs_init_done)
    {
        rc = jfs_init2();
        if (rc != NO_ERROR)
        {
            /* How do I write to the console? */
            jERROR(1,("Phase 2 JFS Initialization failed.  Please specify a smaller cachesize.\n"));
            nomounts = 1;
            return rc;
        }
    }

    switch (flag)
    {
        case MOUNT_MOUNT:

#ifdef _JFS_DEBUG
            {
                char system_id[9];

                strncpy(system_id, extboot->Boot_System_ID, 8);
                system_id[8] = 0;
                jEVENT(0,("Mount: ID = %s, disk = %d\n",
                          system_id, pvpfsi->vpi_unit));
            }
#endif /* _JFS_DEBUG */
#ifdef _JFS_EXTERNAL_LOG
            /* is this a jfs log device?
             */
            if (memcmp(extboot->Boot_System_ID, "JFSLOG  ",8) == 0)
            {
                if (logvpfs != NULL)
                {
                    /* Check to see if this is a duplicate
                     */
                    if ((logvpfs->vpi_vid == pvpfsi->vpi_vid) &&  // 229335
                        (logvpfs->vpi_hDEV == pvpfsi->vpi_hDEV))  // 229335
                    {
                        strncpy(pvpfsi->vpi_text,
                                extboot->Boot_Vol_Label,
                                VPBTEXTLEN-1);
                        return NO_ERROR;
                    }
                    jALERT(1,("multiple logs not supported\n"));
                    return(NO_ERROR);
                }

                logVPB = hVPB;
                logvpfs = pvpfsi;
                capsp = pvpfsi->vpi_pDCS;
                logStrat2 = *(void * _Seg16 *)&capsp->Strategy2;

                if (capsp->Capabilities & GDC_DD_STRAT3)
                    logStrat3 = (void(* _System)())capsp->Strategy3;
                else
                    logStrat3 = 0;

                /* set the volume label */
                strncpy(pvpfsi->vpi_text, extboot->Boot_Vol_Label,
                        VPBTEXTLEN-1);
                return NO_ERROR;
            }
#endif /* _JFS_EXTERNAL_LOG */

            /* is this a jfs file system?
             */
            if (memcmp(extboot->Boot_System_ID, "JFS     ",8) == 0)
            {
                /* Check for obsolete adapter that cannot support
                 * high real memory.
                 */
                if (!(pvpfsi->vpi_flags & VPB_ABOVE16M_ALLOWED))
                {
                    /* Couldn't get parameter substitution working
                     */
                    LT16DriveLtr = pvpfsi->vpi_drive + 'A';
                    FSH_CRITERROR(LT16M_MSGNO, 0, "");
                    return NO_ERROR;
                }

                /* Check to see if this is a duplicate
                 *   A 16-bit IFS would call FSH_FINDDUPHVPB here.
                 */
                for (vfsp = vfs_anchor; vfsp; vfsp = vfsp->vfs_next)
                {
                    if ((vfsp->vfs_vpfsi->vpi_vid == pvpfsi->vpi_vid) &&  // 229335
                        (vfsp->vfs_vpfsi->vpi_hDEV == pvpfsi->vpi_hDEV))  // 229335
                    {
                        strncpy(pvpfsi->vpi_text,
                                extboot->Boot_Vol_Label,
                                VPBTEXTLEN-1);
                        return NO_ERROR;
                    }
                }

                /* get a vfs structure and initialize it
                 */
                vfsp = (struct vfs *)allocpool(vfspool, 0);
                bzero((caddr_t)vfsp, sizeof(struct vfs));
                vfsp->vfs_hVPB = hVPB;
                vfsp->vfs_vpfsi = pvpfsi;
                vfsp->vfs_logVPB = logVPB;
                vfsp->vfs_logvpfs = logvpfs;
                vfsp->vfs_bsize = extboot->Boot_BPB.bytes_per_sector;

                /* get the address of the strategy routine for
                 * this device and save it in the vfs.
                 */
                capsp = pvpfsi->vpi_pDCS;
                vfsp->vfs_strat2p = *(void * _Seg16 *)&capsp->Strategy2;

                if (capsp->Capabilities & GDC_DD_STRAT3)
                    vfsp->vfs_strat3p =
                    (void(* _System)())capsp->Strategy3;
                else
                    vfsp->vfs_strat3p = 0;

                /* call the real mount routine
                 */
                rc = jfs_mount(vfsp, pcred);

                if (rc != NO_ERROR)
                    /* free the vfs on failure */
                    freepool(vfspool, (caddr_t *)vfsp);
                else
                {
                    /* store pointer to the vfs on success */
                    *(struct vfs **)pvpfsd = vfsp;

                    /* set the volume label */
                    strncpy(pvpfsi->vpi_text,
                            extboot->Boot_Vol_Label,
                            VPBTEXTLEN-1);

                    /* put the vfs on the chain
                     */
                    vfsp->vfs_next = vfs_anchor;
                    vfs_anchor = vfsp;

// BEGIN F226941
                    if (vfsp->vfs_flag & VFS_DASDPRIME)
                        rc = dasd_prime(vfsp);
                    if (vfsp->vfs_flag & VFS_DASDLIM)
                        dasd_init(vfsp);
// END F226941
                }
                return NO_ERROR;
            }

            /* we don't recognize the file system   */
//PS 28092001            return ERROR_NOT_SUPPORTED;
            break;
        case MOUNT_RELEASE:
            if (hVPB == logVPB)
            {
                for (vfsp = vfs_anchor; vfsp; vfsp = vfsp->vfs_next)
                    if (vfsp->vfs_logVPB)
                        return ERROR_BUSY_DRIVE;
                logVPB = 0;
                logvpfs = 0;
                return NO_ERROR;
            }
            last_vfs = 0;
            rc = NO_ERROR;
            for (vfsp = vfs_anchor; vfsp; vfsp = vfsp->vfs_next)
            {
                if (hVPB == vfsp->vfs_hVPB)
                {
                    if (last_vfs)
                        last_vfs->vfs_next = vfsp->vfs_next;
                    else
                        vfs_anchor = vfsp->vfs_next;

                    if ((vfsp->vfs_flag & VFS_ACCEPT) == 0)
                    {

                        /* OS/2 will not let the unmount fail,
                         * so we will set VFS_SHUTDOWN to force
                         * it
                         */
                        vfsp->vfs_flag |= VFS_SHUTDOWN;
                        rc = jfs_umount(vfsp, 0, pcred);
                    }
                    freepool(vfspool, (caddr_t *)vfsp);
                    return rc;
                }
                last_vfs = vfsp;
            }
            return NO_ERROR;

        case MOUNT_ACCEPT:
            /* get a vfs structure and initialize it
             */
            vfsp = (struct vfs *)allocpool(vfspool, 0);
            bzero((caddr_t)vfsp, sizeof(struct vfs));
            vfsp->vfs_hVPB = hVPB;
            vfsp->vfs_vpfsi = pvpfsi;
/*
 * pBoot is not set in this case.
 */
//  vfsp->vfs_bsize = extboot->Boot_BPB.bytes_per_sector;
            vfsp->vfs_bsize = 512;

            capsp = pvpfsi->vpi_pDCS;
            vfsp->vfs_strat2p = *(void * _Seg16 *)&capsp->Strategy2;

            if (capsp->Capabilities & GDC_DD_STRAT3)
                vfsp->vfs_strat3p = (void(* _System)())capsp->Strategy3;
            else
                vfsp->vfs_strat3p = 0;

            vfsp->vfs_flag = VFS_ACCEPT;

            /* Store pointer to the vfs
             */
            *(struct vfs **)pvpfsd = vfsp;

            /* put the vfs on the chain
             */
            vfsp->vfs_next = vfs_anchor;
            vfs_anchor = vfsp;

            return NO_ERROR;
        case MOUNT_VOL_REMOVED:
            return NO_ERROR;

        default:
            return ERROR_INVALID_FUNCTION;
    }
  return ERROR_NOT_SUPPORTED;
}

APIRET APIENTRY FS32_MOVE (
                          struct cdfsi    *pcdfsi,
                          struct cdfsd    *pcdfsd,
                          char        *pSrc,
                          unsigned short  iSrcCurDirEnd,
                          char        *pDst,
                          unsigned short  iDstCurDirEnd,
                          unsigned short  flags)
{
    int             rc;
    pathlook_t      spls;
    pathlook_t      dpls;
    struct vfs  *vfsp;
    UniChar         *Src;
    UniChar         *Dst;

    FSoptrace(MOVE, flags, pSrc, pDst);
    opstats.move++;

#ifdef FreeJFS
    opstats.LastCalled=0x1a;             // PS
#endif

    Src = (UniChar *)allocpool(unipool, 0);
    if (Src == 0)
        return ENOMEM;
    Dst = (UniChar *)allocpool(unipool, 0);
    if (Dst == 0)
    {
        rc = ENOMEM;
        goto freeSrc;
    }

    DLIM_init(&spls.dlim, DLIM_RELEASE|DLIM_RENAME);  // F226941, D233382
    DLIM_init(&dpls.dlim, DLIM_RELEASE|DLIM_RENAME);  // F226941, D233382

    if (jfs_strtoUCS(Src, pSrc, MAXPATHLEN) == -1)
    {
        rc = ERROR_BUFFER_OVERFLOW;
        goto freeDst;
    }
    if (jfs_strtoUCS(Dst, pDst, MAXPATHLEN) == -1)
    {
        rc = ERROR_BUFFER_OVERFLOW;
        goto freeDst;
    }

    /* Lookup the vnodes for the source.
     */
    spls.path = Src;
    vfsp = VPBtoVFS(pcdfsi->cdi_hVPB);
    rc = pathlookup(vfsp, &spls, pcred);
    if (rc)
        goto freeDst;


    /* Source must be an existing file or directory.
     */
    if ((spls.statlast != PL_EXIST)
        && (spls.statlast != PL_ISDIR))
    {
        spls.vp = 0;
        rc = ENOENT;
        goto freeSpls;
    }

    /* Lookup vnodes for the destination.
     */
    dpls.path = Dst;
    rc = pathlookup(vfsp, &dpls, pcred);
    if (rc)
        /* release source vnodes */
        goto freeSpls;

    /* Destination must either not exist or be the same vnode as source.
     * If source & destination are the same vnode, FS_MOVE may be changing
     * the case of the name.
     */
    if (((dpls.statlast == PL_EXIST) || (dpls.statlast == PL_ISDIR))
        && (dpls.vp != spls.vp))
    {
        rc = ERROR_ACCESS_DENIED;
        goto out;
    }

    if (dpls.statlast == PL_WCARD)
    {
        /* no wild cards allowed */
        rc = ENOENT;
        goto out;
    }

    /* Call the vnode op to do the work.
     */
    rc = jfs_rename(spls.vp, spls.dvp, spls.pathlast,
                    dpls.vp, dpls.dvp, dpls.pathlast,
                    &spls.dlim, &dpls.dlim,         // F226941
                    (flags & FILE_NEWNAME)<<ATTRSHIFT);

    out:
    /* release all the vnodes.
     */
    if (dpls.vp)
        jfs_rele(dpls.vp);
    if (dpls.dlim.num_limits == 0)              // F226941
        jfs_rele(dpls.dvp);
    freeSpls:
    if (spls.vp)
        jfs_rele(spls.vp);
    if (spls.dlim.num_limits == 0)              // F226941
        jfs_rele(spls.dvp);

    freeDst:
    freepool(unipool, (caddr_t *)Dst);
    DLIM_free(&dpls.dlim);                  // F226941
    DLIM_free(&spls.dlim);                  // F226941
    freeSrc:
    freepool(unipool, (caddr_t *)Src);

    return rc;
}

APIRET APIENTRY FS32_NEWSIZEL (
                              struct sffsi    *psffsi,
                              struct sffsd    *psffsd,
                              long long   len,
                              unsigned short  IOflag)
{
    int rc = 0;
    struct vnode *vp = *(struct vnode **)psffsd;
    inode_t *ip;

    FSoptrace(NEWSIZE, 0, len, vp);
    opstats.newsize++;
#ifdef FreeJFS
    opstats.LastCalled=0x1b;             // PS
#endif

    if (vp == 0)
    {
        jEVENT(1,("fs_newsize called for DASD open\n"));
        return ERROR_INVALID_FUNCTION;
    }

    if (len < 0)
        return ERROR_INVALID_PARAMETER;

    ip = VP2IP(vp);
    /* call the jfs routine to change the file size.
     * if it is successful then reflect that size in the sft.
     */
    if (ip->i_size != len)
    {
        if (isReadOnly(ip))
            return ERROR_WRITE_PROTECT;

        rc = jfs_ftruncate(vp, 0, len, 0, pcred);
        if (rc == 0)
            psffsi->sfi_sizel = len;
    }
    else
        psffsi->sfi_sizel = len;

    return rc;
}

APIRET APIENTRY FS32_OPENCREATE (
                                struct cdfsi    *pcdfsi,
                                struct cdfsd    *pcdfsd,
                                char        *pName,
                                unsigned short  iCurDirEnd,
                                struct sffsi    *psffsi,
                                struct sffsd    *psffsd,
                                unsigned long   OpenMode,
                                unsigned short  OpenFlag,
                                unsigned short  *Action,
                                unsigned short  Attr,
                                char        *pcEABuf,
                                unsigned short  *pfgenflag)
{
    int             rc;
    pathlook_t      pls;    /* pathname lookup structure */
    int32       dasd_flag;              // F226941
    struct inode    *dip;                   // F226941
    struct inode    *ip;
    struct vattr    vattr;
//PS 28092001    ULONG           cbList;
//PS 28092001    FEALIST         *pfealist = NULL;
//PS 28092001    int32           tid;
    struct vfs      *vfsp;
    int64           partition_size;
    struct jfs_sffsd * fsd_data_ptr;
    boolean_t      have_locks;       //PS 28092001
    boolean_t      IsSWAP=FALSE;           //PS 28092001
    UniChar         *Name;

    MMPHPreFS32_OPENCREATE();  /* MMPH Performance Hook */

//PS 28092001 Begin    
    if( Attr & FILE_SWAP )
      {
      IsSWAP = TRUE; 
      Attr &= ~FILE_SWAP;
      }
//PS 28092001 End
    FSoptrace(OPENCREATE, OpenFlag, OpenMode, pName);
    opstats.opencreate++;
#ifdef FreeJFS
    opstats.LastCalled=0x1d;             // PS
#endif

    fsd_data_ptr = (struct jfs_sffsd *) psffsd;

    /* requests to open devices
     */
    if (OpenMode & OPEN_FLAGS_DASD)
    {
        /*
         * We set *psffsd (which is normally a pointer to the vnode)
         * to zero to indicate that the file was open with
         * OPEN_FLAGS_DASD
         */

        fsd_data_ptr->vnode_ptr = NULL;
        fsd_data_ptr->lock_anchor = NULL;
        vfsp = VPBtoVFS(psffsi->sfi_hVPB);
        if (vfsp == 0)
        {
            rc = NO_ERROR;
            goto fs32_OpenCreate_Exit;
        }

        partition_size = (int64)vfsp->vfs_vpfsi->vpi_totsec *
                         (int64)vfsp->vfs_bsize;
        psffsi->sfi_sizel = partition_size;
        rc = NO_ERROR;
        goto fs32_OpenCreate_Exit;
    }

    Name = (UniChar *)allocpool(unipool, 0);
    if (Name == 0)
    {
        rc = ENOMEM;
        goto fs32_OpenCreate_Exit;
    }
    if (jfs_strtoUCS(Name, pName, MAXPATHLEN) == -1)
    {
        rc = ERROR_FILENAME_EXCED_RANGE;
        goto out;
    }

    /* convert path name to vnode pointer
     */
    pls.path = Name;
    vfsp = VPBtoVFS(pcdfsi->cdi_hVPB);

// BEGIN F226941
    if ((OpenMode & (OPEN_ACCESS_WRITEONLY|OPEN_ACCESS_READWRITE))  ||
        (OpenFlag & (OPEN_ACTION_REPLACE_IF_EXISTS|OPEN_ACTION_CREATE_IF_NEW)) )
        dasd_flag = DLIM_RELEASE;
    else
        dasd_flag = DLIM_READONLY;
    DLIM_init(&pls.dlim, dasd_flag);
// END F226941

    rc = pathlookup(vfsp, &pls, pcred);
    if (rc)
        goto out2;

    switch (pls.statlast)
    {
        case(PL_EXIST):     /* file already exists */
            if (OpenFlag & (OPEN_ACTION_REPLACE_IF_EXISTS |
                            OPEN_ACTION_OPEN_IF_EXISTS))
            {
                ip = VP2IP(pls.vp);

                IWRITE_LOCK(ip)             // F226941

                /* Cannot replace hidden or system file unless
                 * identical attributes are set in Attr paramter
                 */
                if ((OpenFlag & OPEN_ACTION_REPLACE_IF_EXISTS) &&
                    (((ip->i_mode & IHIDDEN) && !(Attr & FILE_HIDDEN))||
                     ((ip->i_mode & ISYSTEM) && !(Attr & FILE_SYSTEM))))
                {
                    IWRITE_UNLOCK(ip);        // F226941
                    jfs_rele(pls.vp);
                    rc = ERROR_ACCESS_DENIED;
                    break;
                }

// BEGIN F226941
                /*
                 * Save the list of ancestors in the inode if a
                 * previous open hadn't already done so.
                 */
                if (pls.dlim.num_limits && (ip->i_dasdlim == 0))
                {
                    ip->i_dasdlim = dlim_copy(&pls.dlim);
                    if (ip->i_dasdlim == 0)
                    {
                        IWRITE_UNLOCK(ip);
                        jfs_rele(pls.vp);
                        rc = ENOMEM;
                        break;
                    }
                }
// END F226941

                rc = jfs_open(pls.vp, OpenMode, OpenFlag,
                              (EAOP *)pcEABuf, pcred);

                if (rc)
                {
// BEGIN F226941
                    if (ip->i_dasdlim)
                    {
                        dasd_write_unlock(0, ip->i_dasdlim);
                        dlim_release(ip);
                    }
                    IWRITE_UNLOCK(ip);
// END F226941
                    /* free vnode returned by pathlookup */
                    jfs_rele(pls.vp);
                    break;
                }

                /* indicate the action taken */
                if (OpenFlag & OPEN_ACTION_REPLACE_IF_EXISTS)
                {
                    /* set file attributes */
                    ip->i_mode = (ip->i_mode & ~(IRASH|INEWNAME))
                                 | ((Attr << ATTRSHIFT) & (IRASH|INEWNAME));
                    *Action = FILE_TRUNCATED;
                }
                else
                    *Action = FILE_EXISTED;
                DLIM_WRITE_UNLOCK(ip, ip->i_dasdlim)    // F226941
            }
            else
            {
                jfs_rele(pls.vp);
                rc = EOPENFAILED;
            }
            break;

        case(PL_NOENT):     /* file does not exist */
            if (OpenFlag & OPEN_ACTION_CREATE_IF_NEW)
            {
// BEGIN F226941
                /* Attach dasd limits structure to parent inode */
                dip = VP2IP(pls.dvp);
                DLIM_LOCK_DIRS(dip, &pls.dlim);
                if (OVER_LIMIT(dip, 0))
                    rc = ERROR_DISK_FULL;
                else
// END F226941
                    rc = jfs_create(pls.dvp, &pls.vp, OpenMode,
                                    OpenFlag, pls.pathlast,
                                    (EAOP *)pcEABuf, pcred);
                DLIM_UNLOCK_DIRS(dip, &pls.dlim);   // F226941

                if (rc != 0)
                    break;

                /* indicate action taken */
                *Action = FILE_CREATED;

                /* set file attributes */
                ip = VP2IP(pls.vp);

                ip->i_mode = (ip->i_mode & ~(IRASH|INEWNAME)) |
                             ((Attr << ATTRSHIFT) & (IRASH|INEWNAME));

                /* Lock on inode was taken when it was created */
                IWRITE_UNLOCK(ip);

                /* set sft timestamp flags */
                psffsi->sfi_tstamp |= ST_PCREAT|ST_SCREAT|ST_PWRITE|
                                      ST_SWRITE|ST_PREAD|ST_SREAD;
            }
            else
                rc = EOPENFAILED;
            break;

        case(PL_ISDIR):     /* attempt to open directory */
            jfs_rele(pls.vp);
            rc = EISDIR;
            break;
    }

    /* set file size */
//PS 28092001  Begin
  if( (IsSWAP == TRUE ) && (ISSPARSE(ip)) ) 
    {
    ip->i_mode &= (~JFS_SPARSE);
    }
  else
   {
   IsSWAP=FALSE;
   }
//PS 28092001 end
  if ((rc == 0) && (psffsi->sfi_sizel > 0) ) 
    {
    if ( (*Action == FILE_CREATED) || (*Action == FILE_TRUNCATED) )
      {
        rc = jfs_ftruncate(pls.vp, 0, psffsi->sfi_sizel, 0, pcred);
        if (rc)
        {
// BEGIN D230860
            if (ip->i_dasdlim)
               {
                DLIM_WRITE_LOCK(ip, ip->i_dasdlim);
                dip = VP2IP(pls.dvp);
                dip->i_dasdlim = ip->i_dasdlim;
                have_locks = TRUE;
                }
            else
                have_locks = FALSE;
            jfs_remove(pls.vp, pls.dvp, pls.pathlast, have_locks,
                       pcred);
            if (have_locks)
               {
                dip->i_dasdlim = 0;
                dasd_write_unlock(ip, ip->i_dasdlim);
                }
// END D230860
            jfs_rele(pls.vp);
           }
      }
//   }
//PS 28092001 Begin 
  else if( (*Action == FILE_EXISTED) && ( IsSWAP == TRUE) )
      {
      rc = jfs_ftruncate(pls.vp, 0, 0, 0, pcred);   // SwapFile must be truncate
      if(rc)
        return rc;
      rc = jfs_ftruncate(pls.vp, 0, psffsi->sfi_sizel, 0, pcred);
      if(rc) 
        {
        if(ip->i_dasdlim)
          {
          DLIM_WRITE_LOCK(ip, ip->i_dasdlim);
          dip = VP2IP(pls.dvp);
          dip->i_dasdlim = ip->i_dasdlim;
          have_locks = TRUE;
          }
        else
          have_locks = FALSE; 
        jfs_remove(pls.vp, pls.dvp, pls.pathlast, have_locks, pcred);
        if (have_locks)
          {
          dip->i_dasdlim = 0;
          dasd_write_unlock(ip, ip->i_dasdlim);
          }
        jfs_rele(pls.vp);
        }
      }
    }
//PS 28092001 End
    
    /*
     * If pathlookup() has collected a list of ancestor inodes, the
     * reference on the parent will be released with the others.
     */
    if (pls.dlim.num_limits == 0)               // F226941
        jfs_rele(pls.dvp);

    /* Save file information in the SFT.
     */
    if (rc == 0)
    {
        /* save JFS specific data (the vnode pointer) to
         * use on subsequent calls.
         */
        fsd_data_ptr->vnode_ptr = pls.vp;
        fsd_data_ptr->lock_anchor = NULL;

        /* get information from the inode */
        get_vattr(VP2IP(pls.vp), &vattr);

        /* set time stamps */
        Net_To_Dos_Date(vattr.va_otime.tv_sec,
                        &psffsi->sfi_cdate,
                        &psffsi->sfi_ctime);
        Net_To_Dos_Date(vattr.va_atime.tv_sec,
                        &psffsi->sfi_adate,
                        &psffsi->sfi_atime);
        Net_To_Dos_Date(vattr.va_mtime.tv_sec,
                        &psffsi->sfi_mdate,
                        &psffsi->sfi_mtime);

        /* set file type */
        psffsi->sfi_type = STYPE_FILE;

        /* set file size */
        psffsi->sfi_sizel = vattr.va_size;

        /* set file attributes (R/A/S/H) */
        psffsi->sfi_DOSattr = vattr.va_mode >> ATTRSHIFT;
    }
    out2:
    DLIM_free(&pls.dlim);                   // F226941
    out:
    freepool(unipool, (caddr_t *)Name);

fs32_OpenCreate_Exit:

    MMPHPostFS32_OPENCREATE(); /* MMPH Performance Hook */

    return rc;
}

APIRET APIENTRY FS32_OPENPAGEFILE (
                                  unsigned long   *pFlag,
                                  unsigned long   *pcMaxReq,
                                  char        *pName,
                                  struct sffsi    *psffsi,
                                  struct sffsd    *psffsd,
                                  unsigned short  OpenMode,
                                  unsigned short  OpenFlag,
                                  unsigned short  Attr,
                                  unsigned long   Reserved)
{
    int             rc;
#if 0
    unsigned long   lockc;          /* count of pages locked */
    SG_Descriptor   pgdesc;         /* physical address and size */
    struct LockHandle_s lockh;      /* pinned memory handle */
#endif  /* 0 */
    uint16          dummyAction, dummyFlag;
    struct cdfsi    dummyCds;

    FSoptrace(OPENPAGEFILE, Attr, OpenMode, pName);
    opstats.openpagefile++;

#ifdef FreeJFS
    opstats.LastCalled=0x1e;             // PS
#endif

    /* Keep track of volume with swap-space.  We can't allow this volume
     * to be quiesced.
     */
    page_hVPB = psffsi->sfi_hVPB;

    /* pathlookup needs the hVPB in the current directory structure
     * to figure out where to start.  conjure up a cds with just the
     * needed information.
     */
    dummyCds.cdi_hVPB = psffsi->sfi_hVPB;

    /* do a regular open or create
     */
     Attr |= FILE_SWAP;                            //PS 28092001
    rc = FS32_OPENCREATE(&dummyCds, NULL, pName, -1,
                         psffsi, psffsd, OpenMode , OpenFlag,  
                         &dummyAction, 
                         Attr,
                         NULL, &dummyFlag); 

    if (rc == 0)
    {
        struct vnode *vp = *(struct vnode **)psffsd;
        struct inode *ip = VP2IP(vp);

        /* set indication in inode that this is a swap file.
         */
        ip->i_mode |= ISWAPFILE;

        /* set return information:
         *   pageio requests require physical addresses;
         *   maximum request is 16 pages;
         */
        *pFlag = PGIO_PADDR;
        *pcMaxReq = MAXPGREQ;

    }

    return rc;
}

APIRET  APIENTRY FS32_PATHINFO (
                               unsigned short  flag,
                               struct cdfsi    *pcdfsi,
                               struct cdfsd    *pcdfsd,
                               char        *pName,
                               unsigned short  iCurDirEnd,
                               unsigned short  level,
                               char        *pData,
                               unsigned short  cbData)
{
    int             rc, rc2;
    int32       dlim_flag;              // F226941
    struct inode    *ip;
    pathlook_t      pls;
    EAOP            eaop;
    int32           tid;
    char        data[sizeof(FILESTATUS4L)];
    UniChar     *path_pc;
    struct vfs  *vfsp;
    UniChar         *Name = 0;

    FSoptrace(PATHINFO, (flag << 4) | level, pData, pName);
    opstats.pathinfo++;

#ifdef FreeJFS
    opstats.LastCalled=0x1f;             // PS
#endif

    /* Protect our buffer */
    if ((cbData > sizeof(FILESTATUS4L)) && (level != FIL_LVL7))
        cbData = sizeof(FILESTATUS4L);

    if ((flag & PI_SET) || (level == FIL_QUERYEASFROMLIST) ||
        (level == FIL_QUERYALLEAS))
    {
        rc = KernCopyIn(data, pData, cbData);
        if (rc)
            return rc;
    }

    Name = (UniChar *)allocpool(unipool, 0);
    if (Name == 0)
    {
        rc = ENOMEM;
        goto out1;
    }

    if (jfs_strtoUCS(Name, pName, MAXPATHLEN) == -1)
    {
        rc = ERROR_BUFFER_OVERFLOW;
        goto out1;
    }

    if (level == FIL_NAMEISVALID)
    {
        int32   count = 0;
        UniChar *ptr = Name;

        if (flag & PI_SET)
        {
            rc = ERROR_INVALID_LEVEL;
            goto out1;
        }

        /* We need to verify the length of the path (done by
           jfs_strtoUCS) and each component */
        while (*ptr)
        {
            if (*(ptr++) == '\\')
                count = 0;
            else if (++count == JFS_NAME_MAX)
            {
                rc = ERROR_FILENAME_EXCED_RANGE;
                goto out1;
            }
        }
        rc =  NO_ERROR;
        goto out1;
    }

    if (level == FIL_LVL7)
    {
        if (flag & PI_SET)
        {
            rc = ERROR_INVALID_LEVEL;
            goto out1;
        }
        path_pc = (UniChar *)allocpool(unipool, 0);
        if (path_pc == 0)
        {
            rc = ENOMEM;
            goto out1;
        }
        vfsp = VPBtoVFS(pcdfsi->cdi_hVPB);
        rc = pathlookup_pc(vfsp, Name, path_pc);
        if (rc)
        {
            freepool(unipool, (caddr_t *)path_pc);
            goto out1;
        }

        if (jfs_strfromUCS(pData, path_pc, cbData, UniStrlen(path_pc))
            == -1)
            rc = ERROR_INSUFFICIENT_BUFFER;
        else
            rc = NO_ERROR;

        freepool(unipool, (caddr_t *)path_pc);

        goto out1;
    }

    pls.path = Name;
    vfsp = VPBtoVFS(pcdfsi->cdi_hVPB);

// BEGIN F226941
    if ((flag & PI_SET) && (level == FIL_QUERYEASIZE))
        dlim_flag = DLIM_RELEASE;
    else
        dlim_flag = DLIM_READONLY;
    DLIM_init(&pls.dlim, dlim_flag);
// END F226941


    rc = pathlookup(vfsp, &pls, pcred);
    if (rc)
        goto out2;

    if (pls.dlim.num_limits == 0)               // F226941
        jfs_rele(pls.dvp);
// BEGIN F226941
    else if ((pls.statlast == PL_ISDIR) && (VP2IP(pls.vp)->i_number != 2))
    {
        DLIM_add(&pls.dlim, VP2IP(pls.vp), rc);
        if (rc)
        {
            jfs_rele(pls.vp);
            goto out2;
        }
        pls.dlim.flag |= DLIM_IP_IN_LIST;
    }
// END F226941

    if (pls.statlast == PL_NOENT)
    {
        rc = ENOENT;
        goto out2;
    }

    /* get the inode pointer */
    ip = VP2IP(pls.vp);

    if (flag == PI_RETRIEVE)
    {
        IREAD_LOCK (ip);

        switch (level)
        {
            case FIL_STANDARD:
                rc = jfs_GetLevel1Info (ip,
                                        (FILESTATUS *) data, cbData);
                break;
            case FIL_STANDARDL:
                rc = jfs_GetLevel11Info (ip,
                                         (FILESTATUS3L *) data, cbData);
                break;
            case FIL_QUERYEASIZE:
                rc = jfs_GetLevel2Info (ip,
                                        (FILESTATUS2 *) data, cbData);
                break;
            case FIL_QUERYEASIZEL:
                rc = jfs_GetLevel12Info (ip,
                                         (FILESTATUS4L *) data, cbData);
                break;
            case FIL_QUERYALLEAS:
                ((EAOP *)data)->fpGEAList = 0;
            case FIL_QUERYEASFROMLIST:
                /* validate the EAOP structure to ensure all of
                 * the fields exist.
                 */
                if (cbData < sizeof(EAOP))
                {
                    rc =  ERROR_INSUFFICIENT_BUFFER;
                    break;
                }
                rc = jfs_GetLevel3Info(ip,
                                       ((EAOP *)data)->fpGEAList,
                                       ((EAOP *)data)->fpFEAList,
                                       &(((EAOP *)pData)->oError));

                break;
            default:
                rc = ERROR_INVALID_LEVEL;
                break;
        }

        IREAD_UNLOCK (ip);

        if (level == FIL_STANDARD || level == FIL_QUERYEASIZE ||
            level == FIL_STANDARDL || level == FIL_QUERYEASIZEL)
        {
            rc2 = KernCopyOut(pData, data, cbData);
            if (rc == 0)
                rc = rc2;
        }
    }
    else if (isReadOnly(ip))
        rc = ERROR_WRITE_PROTECT;
    else
    {
        switch (level)
        {
            case FIL_STANDARD:
                rc = jfs_SetLevel1Info (ip,
                                        (FILESTATUS *) data, cbData);
                break;
            case FIL_STANDARDL:
                rc = jfs_SetLevel11Info (ip,
                                         (FILESTATUS3L *) data, cbData);
                break;
            case FIL_QUERYEASIZE:
                DLIM_WRITE_LOCK_ATTACH(ip, &pls.dlim); //F226941

                rc = jfs_SetLevel2Info (ip,
                                        (EAOP *) data, cbData);

                DLIM_WRITE_UNLOCK_DETACH(ip, &pls.dlim); // F226941
                break;
            default:
                rc = ERROR_INVALID_LEVEL;
                break;
        }
    }

    jfs_rele(pls.vp);

    out2:
    DLIM_free(&pls.dlim);                   // F226941
    out1:
    if (Name)
        freepool(unipool, (caddr_t *)Name);

    return(rc);
}

APIRET APIENTRY FS32_READ(
                         struct sffsi    *psffsi,
                         struct sffsd    *psffsd,
                         void            *pData,
                         unsigned long   *pLen,
                         unsigned long   IOflag)
{
    int32           rc;
    struct vnode    *vp;
    int64           offset;
    int64           length;
    struct vfs      *vfsp;
    struct jfs_sffsd * fsd_data_ptr;
    inode_t * ip;
    struct gnode * gp;                          /* pointer to gnode for file   */

    FSoptrace(READ, 0, *pLen, psffsi);
    opstats.read++;

#ifdef FreeJFS
    opstats.LastCalled=0x21;             // PS
#endif
    /* get vnode pointer from SFT */
    fsd_data_ptr = (struct jfs_sffsd *) psffsd;
    vp = fsd_data_ptr->vnode_ptr;

    MMPHPreFS32_READ();        /* MMPH Performance Hook */

    /* set up local (64 bit) variables for offset and length */
    offset = psffsi->sfi_positionl;
    length = *pLen;
    *pLen = 0;

    if (vp == 0)
    {
        if (offset + length > psffsi->sfi_sizel)
        {
            length = psffsi->sfi_sizel - offset;
            if (length <= 0)
            {
                //return NO_ERROR;
                rc = 0;
                goto fs32_Read_Exit;
            }
        }
        vfsp = VPBtoVFS(psffsi->sfi_hVPB);
        if (vfsp == 0)
        {
            //return EINVAL;
            rc = EINVAL;
            goto fs32_Read_Exit;
        }

        rc = dasd_read(offset, pData, &length, vfsp);
        goto out;
    }

    ip = VP2IP(vp);  /* get the ip pointer for locking */
    gp = VP2GP(vp);  /* get the gp */

    IREAD_LOCK(ip);

    /* before calling read worker, check for lock conflict */

    if (gp->gn_filelocks != NULL)
    {
        rc = check_read_locks(gp, (int64)offset, (int64)length,
                              psffsi->sfi_PID, &fsd_data_ptr->lock_anchor);
        if (rc)
        {
            IREAD_UNLOCK(ip);
            goto fs32_Read_Exit;
        }
    }

    rc = jfs_read(vp, offset, pData, &length, IOflag);

    IREAD_UNLOCK(ip);

    out:
    if (rc == 0)
    {
        /* set the number of bytes read */
        *pLen = (uint32)length;

        /* update the offset in the SFT */
        psffsi->sfi_positionl += length;

        /* set SFT timestamp flags */
        psffsi->sfi_tstamp |= ST_SREAD|ST_PREAD;
    }

    fs32_Read_Exit:
    MMPHPostFS32_READ();       /* MMPH Performance Hook */

    return rc;
}

APIRET APIENTRY FS32_READFILEATCACHE(
                                    struct sffsi    *psffsi,
                                    struct sffsd    *psffsd,
                                    ULONG       IOflag,
                                    int64       offset,
                                    ULONG       *pLen,
                                    KernCacheList_t **ppCacheList)
{
    cache_list_t        *cacheList = 0;
    struct jfs_sffsd    *fsd_data_ptr;
    struct gnode        *gp;
    inode_t         *ip;
    int64           nbytes;
    APIRET          rc;
    struct vnode        *vp;

#ifdef FreeJFS
    opstats.cacheread++;
    opstats.LastCalled=0x28;             // PS
#endif

    FSoptrace(READFAC, 0, offset, psffsi);

    *ppCacheList = 0;
    nbytes = *pLen;
    *pLen = 0;

    /* get vnode pointer from SFT */
    fsd_data_ptr = (struct jfs_sffsd *) psffsd;
    vp = fsd_data_ptr->vnode_ptr;

    if (vp == 0)
    {
        rc = EINVAL;        /* Not supported for OPEN_FLAGS_DASD */
        goto fs32_ReadFAC_Exit;
    }

    MMPHPreFS32_READFAC();      /* MMPH Performance Hook */

    ip = VP2IP(vp);
    gp = VP2GP(vp);

    IREAD_LOCK(ip);

    /* Don't allow read past end of file */

    if (nbytes > ip->i_size - offset)
    {
        nbytes = ip->i_size - offset;
        if (nbytes <=0)
        {
            IREAD_UNLOCK(ip);
            return 0;
        }
    }

    /* before calling read worker, check for lock conflict */

    if (gp->gn_filelocks != NULL)
    {
        rc = check_read_locks(gp, offset, nbytes, psffsi->sfi_PID,
                              &fsd_data_ptr->lock_anchor);
        if (rc)
        {
            IREAD_UNLOCK(ip);
            goto fs32_ReadFAC_Exit;
        }
    }

    cacheList = (cache_list_t *)allocpool(cache_list_pool, 0);
    if (cacheList == 0)
    {
        rc = ENOMEM;
        IREAD_UNLOCK(ip);
        goto fs32_ReadFAC_Exit;
    }

    rc = jfs_readFAC(ip, offset, &nbytes, cacheList);
    *pLen = (ULONG)nbytes;
    *ppCacheList = &cacheList->kCacheList;

    IREAD_UNLOCK(ip);

    fs32_ReadFAC_Exit:
    if ((rc != 0) && (cacheList != 0))
        freepool(cache_list_pool, (caddr_t *)cacheList);

    MMPHPostFS32_READFAC();     /* MMPH Performance Hook */

    return rc;
}

APIRET APIENTRY FS32_RETURNFILECACHE(
                                    KernCacheList_t *pCacheList)
{
    cache_list_t    *cacheList;
    cbuf_t      *cp;
    int32       i;
    int32       ipl;

    MMPHPreFS32_RETURNFILECACHE();      /* MMPH Performance Hook */

#ifdef FreeJFS
    opstats.returnfilecache++;
    opstats.LastCalled=0x29;             // PS
#endif

    cacheList = (cache_list_t *)pCacheList;

    /*
     * The IOCache Lock prevents another sendfile thread from getting
     * the buffer while we are checking cm_usecount and releasing it.
     * Since we are releasing the lock before calling cmPut, setting
     * cm_sendfile to FALSE insures that another thread cannot increment
     * cm_usecount before cmPut gets the lock again.
     */

    ipl = IOCACHE_LOCK();

    for (i = 0; i < cacheList->nCbufs; i++)
    {
        cp = cacheList->cbuf[i];
        if (--cp->cm_usecount == 0)
        {
            cp->cm_sendfile = FALSE;
            IOCACHE_UNLOCK(ipl);
            cmPut(cp, FALSE);
            ipl = IOCACHE_LOCK();
        }
    }

    IOCACHE_UNLOCK(ipl);

    freepool(cache_list_pool, (caddr_t *)cacheList);

    MMPHPostFS32_RETURNFILECACHE();     /* MMPH Performance Hook */

    return 0;
}

APIRET APIENTRY FS32_RMDIR (
                           struct cdfsi    *pcdfsi,
                           struct cdfsd    *pcdfsd,
                           char        *pName,
                           unsigned short  iCurDirEnd)
{
    inode_t     *dip;                   // F226941
    inode_t     *ip;                    // F226941
    pathlook_t      pls;
    int             rc;
    struct vfs  *vfsp;
    UniChar         *Name;

    FSoptrace(RMDIR, 0, pcdfsi, pName);
    opstats.rmdir++;
#ifdef FreeJFS
    opstats.LastCalled=0x22;             // PS
#endif

    DLIM_init(&pls.dlim, DLIM_RELEASE);         // F226941

    Name = (UniChar *)allocpool(unipool, 0);
    if (Name == 0)
        return ENOMEM;

    if (jfs_strtoUCS(Name, pName, MAXPATHLEN) == -1)
    {
        rc = ERROR_BUFFER_OVERFLOW;
        goto freeName;
    }

    pls.path = Name;

    if (pls.path[3] == 0)           /* Must be "X:\" */
    {
        rc = ENOTDIR;
        goto freeName;
    }

    vfsp = VPBtoVFS(pcdfsi->cdi_hVPB);
    rc = pathlookup(vfsp, &pls, pcred);
    if (rc)
        goto freeName;

    if (pls.statlast == PL_NOENT)
    {
        rc = ENOTDIR;
        goto fs_rmdir2;
    }
    if (pls.statlast == PL_EXIST)
    {
        rc = ENOTDIR;
        goto fs_rmdir1;
    }

// BEGIN F226941
    dip = VP2IP(pls.dvp);
    ip = VP2IP(pls.vp);
    if (pls.dlim.num_limits)
    {
        DLIM_WRITE_LOCK(ip, &pls.dlim);
        ip->i_dasdlim = &pls.dlim;
        dip->i_dasdlim = ip->i_dasdlim;
    }
    else
        IWRITE_LOCK_LIST(2, dip, ip);
// END F226941

    rc = jfs_rmdir(pls.vp, pls.dvp, pls.pathlast, pcred);

// BEGIN F226941
    if (pls.dlim.num_limits)
    {
        ip->i_dasdlim = 0;
        dip->i_dasdlim = 0;
        DLIM_WRITE_UNLOCK(ip, &pls.dlim);
    }
    else
    {
        IWRITE_UNLOCK(ip);
        IWRITE_UNLOCK(dip);
    }
// END F226941

    fs_rmdir1:
    jfs_rele(pls.vp);
    fs_rmdir2:
    if (pls.dlim.num_limits == 0)               // F226941
        jfs_rele(pls.dvp);
    freeName:
    DLIM_free(&pls.dlim);                   // F226941
    freepool(unipool, (caddr_t *)Name);

    return rc;
}

APIRET APIENTRY FS32_SETSWAP (
                             struct sffsi    *psffsi,
                             struct sffsd    *psffsd)
{
    int32           rc = NO_ERROR;
    struct vnode    *vp = *(struct vnode **)psffsd;
    struct inode    *ip = VP2IP(vp);

    FSoptrace(SETSWAP, 0, psffsi, vp);
    opstats.setswap++;
#ifdef FreeJFS
    opstats.LastCalled=0x23;             // PS
#endif

    assert(ip->i_mode & ISWAPFILE);

    return rc;
}

APIRET APIENTRY FS32_SHUTDOWN (
                              unsigned short          type,
                              unsigned long           reserved)
{
    struct vfs              *vfsp;
    extern struct vfs       *vfs_anchor;

    FSoptrace(SHUTDOWN, type, 0, 0);
    opstats.shutdown++;
#ifdef FreeJFS
    opstats.LastCalled=0x24;             // PS
#endif

    switch (type)
    {
        case(SD_BEGIN):
            jEVENT(1,("shutdown begin\n"));
            break;
        case(SD_COMPLETE):
            /* We set vfs_anchor to NULL to stop the sync daemon */
            for (vfsp = vfs_anchor, vfs_anchor = NULL;
                vfsp != NULL;
                vfsp = vfsp->vfs_next)
            {
                if (!(vfsp->vfs_flag & VFS_ACCEPT))
                {
                    vfsp->vfs_flag |= VFS_SHUTDOWN;
                    jfs_umount(vfsp, 0, pcred);
                }
            }
            jEVENT(1,("shutdown end\n"));
            break;
        default:
            panic("bad type on shutdown");
    }
    return(NO_ERROR);
}

APIRET APIENTRY FS32_WRITE(
                          struct sffsi    *psffsi,
                          struct sffsd    *psffsd,
                          void            *pData,
                          unsigned long   *pLen,
                          unsigned long   IOflag)
{
    int32           rc;
    struct vnode    *vp;
    int64           offset;
    int64           length;
    int64           lock_offset;
    int64           lock_length;
    UCHAR own;
    struct vfs      *vfsp;
    struct jfs_sffsd * fsd_data_ptr;
    struct file_lock * curr_lock;               /* pointer to current lock     */
    inode_t * ip;
    struct gnode * gp;                          /* pointer to gnode for file   */

    FSoptrace(WRITE, 0, *pLen, (caddr_t)psffsi);
    opstats.write++;
#ifdef FreeJFS
    opstats.LastCalled=0x25;             // PS
#endif

    /* get vnode pointer from SFT */
    fsd_data_ptr = (struct jfs_sffsd *) psffsd;
    vp = fsd_data_ptr->vnode_ptr;

    MMPHPreFS32_WRITE();       /* MMPH Performance Hook */

    /* set up local (64 bit) variables for offset and length */
    offset = psffsi->sfi_positionl;
    length = *pLen;
    *pLen = 0;

    if (vp == 0)
    {
        if (offset + length > psffsi->sfi_sizel)
        {
            //return ERROR_WRITE_FAULT;
            rc = ERROR_WRITE_FAULT;
            goto fs32_Write_Exit;
        }
        vfsp = VPBtoVFS(psffsi->sfi_hVPB);
        if (vfsp == 0)
        {
            //return EINVAL;
            rc = EINVAL;
            goto fs32_Write_Exit;
        }

        rc = dasd_write(offset, pData, &length, vfsp);
        goto out;
    }
//PS21092003 Begin
    if( vp->v_vfsp == 0)
      {
      rc = EINVAL;
      goto fs32_Write_Exit;
      }
//PS End      

    ip = VP2IP(vp);  /* get the ip pointer for locking */
    gp = VP2GP(vp);  /* get the gp */

    IWRITE_LOCK(ip);

    /* before calling write worker, check for lock conflict */

    if (gp->gn_filelocks != NULL)
    {
        own = FALSE;
        lock_offset = offset;
        lock_length = length;

        /* search per open locklist if the thread locked range */
        for (curr_lock = fsd_data_ptr->lock_anchor; curr_lock != NULL;
            curr_lock = curr_lock->vnode_next_lock)
        {
            if (lock_offset < curr_lock->lock_start)
            {
                own = FALSE;
                break;
            }
            else
            {
                if ((lock_offset <= curr_lock->lock_end) && (curr_lock->share == 0))
                {
                    if ((lock_offset + lock_length <= curr_lock->lock_end) &&
                        (curr_lock->pid == psffsi->sfi_PID))
                    {
                        own = TRUE;
                        break;
                    }
                    else
                    {
                        lock_length = curr_lock->lock_end - lock_offset;
                        lock_offset = curr_lock->lock_end + 1;
                    }
                }
            }
        }

        if (!own)
        {
            /* search the per file locklist if the range is locked */
            for (curr_lock = gp->gn_filelocks; curr_lock != NULL;
                curr_lock = curr_lock->next_lock)
            {
                /* do the lock ranges overlap ?? */
                if ((offset <= curr_lock->lock_end)  &&
                    ((offset + length) >= curr_lock->lock_start))
                {
                    /* if we own shared lock or other handle own any lock  */
                    if ((( curr_lock->share) &&
                         (curr_lock->anchor == &fsd_data_ptr->lock_anchor)) ||
                        (curr_lock->anchor != &fsd_data_ptr->lock_anchor)  ||
                        (curr_lock->pid != psffsi->sfi_PID))
                    {
                        jEVENT(0,("##WRITE FAILED: start:%x len%x,anchor %x,pid %x\n",
                                  offset,length, &fsd_data_ptr->lock_anchor,psffsi->sfi_PID));
                        IWRITE_UNLOCK(ip);
                        rc = ERROR_LOCK_VIOLATION;
                        goto fs32_Write_Exit;
                    }
                }
            }
        }
    }

    rc = jfs_write(vp, offset, pData, &length, IOflag);

    psffsi->sfi_sizel = ip->i_size;

    DLIM_WRITE_UNLOCK(ip, ip->i_dasdlim);           // F226941

    out:
    if (rc == 0)
    {
        /* set number of bytes written */
        *pLen = (uint32)length;

        /* update the offset in the SFT */
        psffsi->sfi_positionl += length;

        /* set SFT timestamp flags */
        psffsi->sfi_tstamp |= ST_SWRITE|ST_PWRITE;
        ip->i_mode |= IARCHIVE;
        psffsi->sfi_DOSattr |= IARCHIVE >> ATTRSHIFT;     //PS21092003
    }
    else 
     if (rc == ERROR_DISK_FULL)
        {
        rc = NO_ERROR;      /* *pLen is zero */
        /* set number of bytes written */
        *pLen = (uint32)length;

        /* update the offset in the SFT */
        psffsi->sfi_positionl += length;

        /* set SFT timestamp flags */
        psffsi->sfi_tstamp |= ST_SWRITE|ST_PWRITE;

        ip->i_mode |= IARCHIVE;
        psffsi->sfi_DOSattr |= IARCHIVE >> ATTRSHIFT;     //PS21092003
      }

    fs32_Write_Exit:
    MMPHPostFS32_WRITE();      /* MMPH Performance Hook */

    return rc;
}


#ifdef drova  //  _JFS_STATISTICS
/*
 *      jfsStatistics()
 */
int jfsStatistics(
                 char    *buf,
                 size_t  bufsize)
{
    int32   cmd;
    int32   rc;
    int32   type;

    /* get the type of statistics from the buffer */
    cmd = *(int32 *)buf;

    switch (cmd)
    {
        case FSCNTL_NCACHE: /* ncache statistics */
            rc = ncStatistics(buf, 1);
            break;

        case FSCNTL_ICACHE: /* icache statistics */
            rc = ERROR_INVALID_FUNCTION;
            break;

        case FSCNTL_JCACHE: /* jcache statistics */
            rc = bmStatistics(buf, 1);
            break;

        case FSCNTL_DCACHE: /* dcache statistics */
            rc = ERROR_INVALID_FUNCTION;
            break;

        case FSCNTL_LCACHE: /* log statistics */
            rc = lmStatistics(buf, 1);
            break;

        default:
            rc = ERROR_INVALID_FUNCTION;
            break;
    }
    return rc;
}
#endif  /* _JFS_STATISTICS */


#ifdef _FS_OP_TRACE
void
fsoptrace(uint32        operation,
          uint32        subop,
          caddr_t       other,
          caddr_t       nmnode)
{
    if (trc_cur == 0)
    {
        XSPINLOCK_ALLOC(&optrace_lock, 0, 0, 0);
        trc_cur = &trc_lst[TRC_EVENTS-1];
        trc_prev = &trc_lst[0];
    }

    SPINLOCK_LOCK(&optrace_lock);
    /* save data for the current operation in the next available
     * slot in the trace list.
     */
    trc_cur->fot_op = operation;
    trc_cur->fot_subop = subop;
    trc_cur->fot_seq = ++trc_sequence;
    trc_cur->fot_other = other;
    trc_cur->fot_nmnode = nmnode;
    trc_cur->fot_ptid = (pLIS->LIS_CurProcID << 16) | pLIS->LIS_CurThrdID;

    /* set the index to the next slot.  However,
     * if the operation and arguments exactly match the
     * last one, then don't move the index.  (this reduces
     * the number of entries in the trace list.  the list
     * fills up with findnexts, reads, and writes, and it
     * wraps very quickly without this optimization.)
     */
    if (memcmp(&trc_cur->fot_subop, &trc_prev->fot_subop,
               sizeof(struct fs_optrc) - sizeof(trc_sequence)) != 0)
    {
        trc_prev = trc_cur;
        if (trc_cur != trc_lst)
            trc_cur--;
        else
            trc_cur = &trc_lst[TRC_EVENTS-1];
    }
    SPINLOCK_UNLOCK(&optrace_lock);
}
#endif /* _FS_OP_TRACE */
