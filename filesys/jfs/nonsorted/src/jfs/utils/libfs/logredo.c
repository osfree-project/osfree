/* $Id: logredo.c,v 1.3 2004/03/21 02:43:19 pasha Exp $ */

static char *SCCSID = "@(#)1.34  9/7/99 07:53:48 src/jfs/utils/libfs/logredo.c, jfslib, w45.fs32, currbld";
/*
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
 *   MODULE_NAME:		logredo.c
 *
 *   COMPONENT_NAME: 	jfslib
 *
 *   FUNCTIONS:  jfs_logredo.c: recovery manager
 *
 *              logRedo()  -- The main routine
 *              doMount
 *              openVol
 *              updateSuper
 *              rdwrSuper
 *              bflush
 *              isLogging
 *              isFilesystem
 *              logOpen
 *              lockLog
 *              unlockLog
 *              makeOpen
 *              makeDevice
 *              fsError
 *              logError
 *              recoverExtendFS
 *              alloc_dmap_bitrec
 *              alloc_storage
 *              nfsisloaded
 *
 *
*/
/* History
 * PS21032004 - insert IBM fixes
 */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#include <os2.h>
#include "jfs_types.h"
#include <jfs_aixisms.h>
#include "sysbloks.h"
#include <extboot.h>
#include "bootsec.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "jfs_filsys.h"
#include "jfs_superblock.h"
#include "jfs_inode.h"
#include "jfs_dtree.h"
#include "jfs_xtree.h"
#include "jfs_logmgr.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_debug.h"
#include "jfs_cntl.h"
#include "logredo.h"
#include "logform.h"
#include "devices.h"
#include "debug.h"

#include "fsckmsgc.h"		/* for chkdsk message logging facility */

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For the chkdsk message logging facility
  *
  *      defined in xchkdsk.c
  */
extern char message_parm_0[];
extern char message_parm_1[];
extern char message_parm_2[];
extern char message_parm_3[];
extern char message_parm_4[];
extern char message_parm_5[];
extern char message_parm_6[];
extern char message_parm_7[];
extern char message_parm_8[];
extern char message_parm_9[];

extern char *msgprms[];
extern int16 msgprmidx[];

extern char *terse_msg_ptr;
extern char *verbose_msg_ptr;

extern char *MsgText[];


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *   L O C A L   M A C R O    D E F I N I T I O N S
  *
  */
#define MAKEDEV(__x,__y)        (dev_t)(((__x)<<16) | (__y))

#define LOGPNTOB(x)  ((x)<<L2LOGPSIZE)

#define LOG2NUM(NUM, L2NUM)\
{\
        if ((NUM) <= 0)\
                L2NUM = -1;\
        else\
        if ((NUM) == 1)\
                L2NUM = 0;\
        else\
        {\
                L2NUM = 0;\
                while ( (NUM) > 1 )\
                {\
                        L2NUM++;\
                        (NUM) >>= 1;\
                }\
        }\
}


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *    R E M E M B E R    M E M O R Y    A L L O C    F A I L U R E
  *
  */
int32  Insuff_memory_for_maps = 0;
char  *available_stg_addr = NULL;
int32  available_stg_bytes = 0;
char  *bmap_stg_addr = NULL;
int32  bmap_stg_bytes = 0;


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *    S T U F F    F O R    T H E    L O G 
  *
  */
int16   loglocation = 0;        /* 1 = inlinelog, 2 = outlinelog */
int32    logmajor = 0;          /* major number of log device */
int32    logminor = 0;          /* minor number of log device */
int32    logserial;             /* log serial number in super block */
int32    logend;                /* address of the end of last log record */
int32    logfd;                 /* file descriptor for log */
int32    logsize;               /* size of log in pages */
logsuper_t logsup;              /* log super block */


int32 numdoblk;                 /* number of do blocks used     */
int32 numnodofile;              /* number of nodo file blocks used  */
int32 numExtDtPg = 0;         /* number of extended dtpage blocks used  */

/*
 *      open file system aggregate/lv array
 *
 * logredo() processes a single log.
 *
 * In the first release, logredo will process a single log which relates
 * to the single fileset in a single aggregate.  In some future release, 
 * a single log may be used for multiple filesets which may or may not all
 * reside in the same aggregate.  
 *
 * The log and any aggregates (logical volumes) containing filesets for
 * which the log is used must all be in the same volume group.  Thus while
 * a logical volume is uniquely identified by a (minor number,major number)
 * pair, the major number for the log and any aggregates containing filesets
 * for which the log used will be the same.
 *
 * The maximum number of logical volumes in a volume group is NUMMINOR(256)
 */
struct vopen vopen[NUMMINOR];              /* (88) */
 	/*
	 * if this flag is set then the primary superblock is 
	 * corrupt.  The secondary superblock is good, but chkdsk
	 * wasn't able to fix the primary version.  logredo can
	 * run, but must use the secondary version of the 
	 * aggregate superblock
	 */
int32 use_2ndary_agg_superblock;
/*
 *      file system page buffer cache
 *
 * for k > 0, bufhdr[k] describes contents of buffer[k-1].
 * bufhdr[0] is reserved as anchor for free/lru list:
 * bufhdr[0].next points to the MRU buffer (head),
 * bufhdr[0].prev points to the LRU buffer (tail);
 */

/* buffer header table */
struct bufhdr
{
        int16           next;           /* 2: next on free/lru list */
        int16           prev;           /* 2: previous on free/lru list */
        int16           hnext;          /* 2: next on hash chain */
        int16           hprev;          /* 2: previous on hash chain */
        char            modify;         /* 1: buffer was modified */
        char            inuse;          /* 1: buffer on hash chain */
        int16           reserve;        /* 2 */
        int32           vol;            /* 4: minor of agrregate/lv number */
        pxd_t           pxd;            /* 8: on-disk page pxd */
} bufhdr[NBUFPOOL];                     /* (24) */

/* buffer table */
struct bufpool
{
        char    bytes[PSIZE];
} buffer[NBUFPOOL - 1];


/*
 *      log page buffer cache
 *
 * log has its own 4 page buffer pool.
 */
uint8    afterdata[LOGPSIZE];   /* buffer to read in redopage data */

/*
 * Miscellaneous
 */
caddr_t prog;                           /* Program name */
int32 mntcnt, bufsize;
char *mntinfo;
int32 retcode;                          /* return code from logredo    */
//PS21032004 int  end_of_transaction = 0;                                              // @D2

char    loglockpath[MAXPATHLEN + 1];    /* log lock file                */

/*
 * external references
 */
extern  char *optarg;
extern  int optind;
extern errno;
extern int initMaps( int32 );
extern int updateMaps(int);
extern int32 findEndOfLog(void);
extern int logRead( int32 , struct lrd *, char* );
extern int32 logredoInit( void );
extern int doCommit( struct lrd * );
extern int doExtDtPg( void );
extern int doNoRedoFile( struct lrd * ,  ino_t);
extern int doNoRedoPage( struct lrd * );
extern int doNoRedoInoExt( struct lrd * );
extern int doAfter( struct lrd * , int32 );
extern int doUpdateMap( struct lrd * );


extern void fsck_send_msg( int, int, int );		/* defined in fsckmsg.c */
extern int alloc_wrksp( uint32, int, int, void ** );	/* defined in fsckwsp.c */

/*
 * forward references
 */
int doMount( struct lrd * );
int32 openVol( int32  );
int32 updateSuper( int vol);
int32 rdwrSuper( int32 ,   struct superblock *, int32 );
int bflush( int32  ,   struct bufpool * );
int isLogging( caddr_t , int32 , char *, int32 );
int32 isFilesystem( caddr_t , int32 );
int32 logOpen(void);
int fsError( int , int , int64 );
int logError( int , int );
static int32 recoverExtendFS(int32 fd);
int32 alloc_storage( int32, void **, int32 * );
int32 alloc_dmap_bitrec( dmap_bitmaps_t ** );

/*
 * debug control
 */
#ifdef _JFS_DEBUG
int32   dflag = 1;
time_t  *Tp;
uint32  tp_start, tp_end;
int xdump(char*, int);
int x_scmp(char*, char*);
void x_scpy(char*, char*);
int prtdesc(struct lrd *);
#else
int32   dflag = 0;
#endif


/*
 * NAME:        jfs_logredo()
 *
 * FUNCTION:	Replay all transactions committed since the most
 *		recent synch point.
 *
 * NOTES:	
 *	>>>>>> 	The log replay is accomplished in one pass over the
 *		log, reading backwards from logend to the first synch
 *		point record encountered.  This means that the log 
 *		entries are read and processed in LIFO (Last-In-First-Out)
 *		order.  In other words, the records logged latest in
 *		time are the first records processed during log replay.
 *
 *	>>>>>> 	Inodes, index trees, and directory trees
 *
 *		Inodes, index tree structures, and directory tree 
 *		structures are handled by processing committed redopage
 *		records which have not been superceded by noredo records.
 *		This processing copies data from the log record into the
 *		appropriate disk extent page(s).
 *
 *		To ensure that only the last (in time) updates to any
 *		given disk page are applied during log replay, logredo
 *		maintains a record (union structure summary1/summary2),
 *		for each disk page which it has processed, of which 
 *		portions have been updated by log records encountered.  
 *
 *	>>>>>> 	Inode Allocation Map processing

 *		The xtree for the Inode Allocation Map is journaled, and
 *		a careful write is used to update it during commit
 *		processing.
 * The imap index tree is also duplicated at the known location. (TBD)
 * So at logredo time, the xtree for imap is always readable and correct.
 * This is the basic requirement from logredo.
 *
 * the inode map control page (dinomap_t) is only flushed to disk at
 * the umount time. For iag_t, pmap will go to disk at commit time.
 * iagnum will not change in run-time.
 * agstart field will stable without extendfs utility. It is TBD for
 * how to handle agstart when extendfs utility is available.
 * Other fields ( wmap. inosmap, extsmap ino free list pointers,
 * ino ext free list pointers ) are at working status ( i.e they are
 * updated in run-time. So the following
 * meta-data of the imap need to be reconstructed at the logredo time:
 *  1) IAGs, the pmap of imap and inoext array are contained in IAGs.
 *  2) AG Free inode list
 *  3) AG Free Inode Extent list
 *  4) IAG Free list
 *
 * There are two imaps need to take care of :
 *   1) aggregate imap
 *   2) fileset imap
 * For the first release, the aggregate imap is stable and we only
 * need to deal with the fileset imap.
 *
 * Block Allocation Map (bmap file) is for an aggregate/lv. There are
 * three fields related to the size of bmap file.
 *  1) superblock.s_size: This field indicates aggregate size. It
 *                        tells number of sector-size blocks for this
 *                        aggregate. The size of aggregate determines
 *                        the size of its bmap file.
 *                        Since the aggregate's superblock is updated
 *                        using sync-write, superblock.s_size is trustable
 *                        at logredo time.
 *               note1:   mkfs reserves the fsck space. So s_size really
 *                        inidcate (size_of_aggregate - fsck_reserve_space)
 *               note2:   At the mkfs time, "-s" parameter could be used
 *                        to indicate how large the aggregate/filesystem is.
 *                        One lv contains at most one aggregate/filesystem.
 *                        If "-s" gives the value is smaller than the size
 *                        of lv, it is ok. The space is just wasted.
 *
 *                        Without "-s" parameter, mkfs wil use the whole
 *                        size of lv to make an aggregate/filesystem.
 *                        That is usually the case. So we can also say
 *                        an aggregate/lv. "-s" is often used for test.
 *
 *  2) dbmap_t.dn_mapsize: This field also indicates aggregate/lv size.
 *                        It tells number of aggre. blocks in the
 *                        aggregate/lv. Without extendfs, this field should
 *                        be equivalent to superblock.s_size.
 *                        With extendfs, this field may not be updated
 *                        before a system crash happens. So logredo
 *                        need to update it.
 *  3) dinode_t.di_size:  For an inode of bmap file, this field indicates
 *                        the logical size of the file. I.e. it contains
 *                        the offset value of the last byte written
 *                        in the file plus one.
 *                        So di_size will include the bmap control page,
 *                        the dmap control pages and dmap pages.
 *                        In the JFS, if a file is a sparse file, the logical
 *                        size is different from its physical size.
 *                        The bmap file is a sparse file if the total of
 *                        dmap pages is  ( < 1024) or ( < 1024 * 1024).
 *                        In that case, physically L1.0, and/or L2 does
 *                        not exist, but di_size will include their page
 *                        size.
 *
 *              Note:     The di_size does NOT contain the logical
 *                        structure of the file, i.e. the space allocated
 *                        for the xtree stuff is not indicated in di_size.
 *                        It is indicated in di_nblocks.
 *
 *                        In addition, the mkfs always put one more dmap
 *                        page into the bmap file for preparing extendfs.
 *                        This hidden dmap page cannot be figured out from
 *                        superblock.s_size, but di_size includes it. Any
 *                        dmapctl_t pages caused by this hidden dmap page
 *                        are also included in di_size.
 *
 * The bmap control page, dmap control pages and dmap pages are all
 * needed to rebuild at logredo time.
 *
 * In overall, the following actions are taken at logredo time:
 *   1) apply log rec data to the specified page.
 *   2) initialize freelist for dtree page or root.
 *   3) rebuilt imap
 *   4) rebuilt bmap
 *   in addition, in order to ensure the log record only applying to a
 *   certain portion of page one time, logredo will start NoRedoFile,
 *   NoRedoExtent/NoRedoPage filter in the process for accuracy and
 *   efficiency.
 *
 *  The three log rec types: REDOPAGE, NOREDOPAGE, NOREDOINOEXT, and 
 *  UPDATEMAP, are the main force to initiate these actions.  See 
 *  comments on doAfter(), updatePage(), doNoRedoPage(), doNoRedoInoExt,
 *  and doUpdateMap() for detailed information.
 *
 * If the aggregate/lv has state of FM_DIRTY, then fsck will run
 * after the logredo process since logredo could not get 100%
 * recovery. Currently bmap rebuild is slow ( 1 min per 32 GB),
 * so logredo will NOT rebuild imap and bmap if fsck will do it
 * anyway. But logredo still read maps in and mark them for starting
 * NoRedoExtent/NoRedoPage filter.
 *
 * The maps are rebuilt in the following way:
 * at the init phase, storage is allocated for the whole map file for
 * both imap and bmap. Reading in the map files from the disk.
 * The wmap is inited to zero. At the logredo time, the wmap is used
 * to track the bits in pmap. In the beginning of the logredo process
 * the allocation status of every block is in doubt. As log records
 * are processed, the allocation state is determined and the bit of pmap
 * is updated. This fact is recorded in the corresponding bits in wmap.
 * So a pmap bit is only updated once at logredo time and only updated
 * by the latest in time log record.
 * At the end of logredo, the control information, the freelist, etc.
 * are built from the value of pmap; then pmap is copied to wmap and
 * the whole map is written back to disk.
 *
 * the status field s_state in the superblock of each file-system is
 * set to FM_CLEAN provided the initial status was either FM_CLEAN
 * or FM_MOUNT and logredo processing was successful. If an error
 * is detected in logredo the status is set to FM_LOGREDO. the status
 * is not changed if its initial value was FM_MDIRTY. fsck should be
 * run to clean-up the probable damage if the status after logredo
 * is either FM_LOGREDO or FM_MDIRTY.
 *
 *  The log record has the format:
 *   <LogRecordData><LogRecLRD>
 *  At logredo time, the log is read backward. So for every log rec,
 *  we read LogRecLRD, which tells how long the LogRecordData is.
 *  see comments on updatePage() for detailed info of log record format.
 *
 *.....................................................................
 * The logredo handles the log-within-file-system (aka inline log) issue:
 *.....................................................................
 * For AIX, we always deal with the outline log, i.e. the log resides
 * in a separate logical volume. A log is associated with one volume
 * group and can be shared by many file systems with this volume group.
 * In AIX, the logredo received a device name. It then determines if
 * this device is a log name  or a filesystem name. If it is a filesustem
 * name, get the log minor number for this filesystem. If it is a log name,
 * get its minor number.
 *
 * In OS/2 there is no mount/umount concept, the logical partition
 * name ( e.g. C:, D: ) are dynamically assigned at the system boot
 * time. At the boot time, OS/2 sequencially scan each partition and
 * make the assignment.
 * Since the logical partition name could change each time the system
 * boot, i.e. the logical partition name for the log could be changed
 * every system boot time. If the system crashed, it may be hard to
 * locate the log.
 * so XJFS decided to put log inside the file system until this
 * problem is solved. So in the first release for OS2, we always
 * assume that the inlinelog is used.
 *
 * For supporting the inline log, the above AIX logic should be changed.
 *
 * Here is the outline:
 *
 * When the logredo received a device name, it first read the SIZE_OF_SUPER
 * bytes from SUPER1_OFF  offset to see if it is a file system superblock.
 * If yes, check the s_flag to see if it has a inline log or outline log.
 * for an inline log the s_logdev should match the input device name's
 * major and minor number. If not, an error is returned and logredo exit.
 * If no error, the logredo read the log superblock according the log info
 * in the fs superblock.
 * If the device name does not represent a filesystem device, then logredo
 * read the LOGPSIZE bytes from the log page 1 location. If it indicates
 * a log device, then open the filesystems according to the log superblock's
 * active list. For each filesystem in the active list, read its superblock
 * if one of the superblock indicates that it uses an inline log, return
 * an error. It is a system code bug if some filesystems use inline log
 * and some use outline log.
 * If the superblock indicates it used an outline log, check the superblock's
 * s_logdev to match the input device name's major and minor numbers.
 * If one of them does not match, return error. -- It is a system code bug,
 * if some match and some not match; -- It should either match all or non of
 * them match. The AIX logredo never check s_logdev with the input log device.
 * We should check here.
 *
 * for outline log, logredo will be called once to cover all the file
 * systems in the log superblock's active list.
 * For inline log, logredo will be called many times. Each time is for
 * one file system. The log superblock's active list has nothing. The
 * logmajor and logminor contains file system's major and minor number.
 *
 *.....................................................................
 * logredo handles support EA:
 *.....................................................................
 * There is 16-byte EA descriptor which is located in the section I of
 * dinode.
 * The EA can be inline or outline. If it is inlineEA then the data will
 * occupy the section IV of the dinode. The dxd_t.flag will indicate so.
 * If it is outlineEA, dxd_t.flag will indicate so and the single extent
 * is described by EA descriptor.
 *
 * The section IV of dinode has 128 byte. It is shared by the xtroot and
 * inlineEA. The sharing is in FCFS style. If xtree gets the section IV,
 * xtree will never give it away even if xtree is shrink or split.
 * If inlineEA gets it, there is a chance that later inlineEA is freed and
 * so xtree still can get it.
 *
 * for outlineEA, the XJFS will syncly write the data portion out so there
 * is no log rec for the data, but there is still an INODE log rec for EA
 * descriptor changes and there is a UPDATEMAP log rec for the allocated
 * pxd. If an outlineEA is freed, there are also two log records for it:
 * one is INODE with EA descriptor zeroed out, another is the UPDATEMAP
 * log rec for the freed pxd.
 * For inlineEA, it has to be recorded in the log rec. It is not in a
 * separate log rec. Just one additional segment is added into the
 * INODE log rec. So an INODE log rec can have at most three segments:
 * when the parent and child inodes are in the same page, then there are
 * one segment for parent base inode; one segment for child base inode;
 * and maybe the third one for the child inlineEA data.
 *....................................................................
 * 32-bit vs 64-bit
 * At the first release. assume that a file system will not be larger
 * than 32-bit.
 * Also for OS/2, currently  DosSetFilePtr, DosRead and DosWrite
 * only support "unsigned long".  So the offset is casted as uint32 when
 * call DosSetFilePtr().
 *....................................................................
 * TBD:
 * the method for handling crashes in the middle of extending a file
 * system is as follows. the size of a filesystem is established from
 * the superblock.s_size field (i.e the sizes in the diskmap
 * and inodemaps are ignored). in extendfs (jfs_cntl.c) the superblock
 * is not updated before the maps have been extended and the new inodes
 * formatted to zeros. no allocations in the new part of the filesystem
 * occur prior to the change in map sizes. if a crash occurs just
 * before updating the superblock, the map sizes will be their old
 * values. in this case the maps as files may be bigger than necessary.
 * if the crash occurs just after writing the super block, the map sizes
 * are fixed up here.
 */
jfs_logredo(
        caddr_t pathname,
        int32   fd,
        int32   use_2nd_aggSuper
        )
{
        int32   rc;
        int32   k,logaddr,nextaddr,lastaddr, nlogrecords;
        unsigned long   actual;
        int  syncrecord, word, bit;
        struct lrd ld;
        int logformit;
        unsigned long dummyd = 0;
        unsigned long dummyp = 0;
        logsuper_t *plogsup = &logsup;
        int64  aggsb_numpages = 0;
        int32 lowest_lr_byte = 2 * LOGPSIZE + LOGPHDRSIZE;
        int32 highest_lr_byte = 0;
        int log_has_wrapped = 0;
	/*
	 * store away the indicator of which aggregate superblock
	 * to use
	 */
        use_2ndary_agg_superblock = use_2nd_aggSuper;

        /*
         * loop until we get enough memory to read vmount struct
         */
        mntinfo = (char *)&bufsize;
        bufsize = sizeof(int);


        /*
         * validate that the log is not currently in use;
         */
        rc = isLogging(pathname, fd, mntinfo, mntcnt);
        if (rc < 0 )
        {
                fsck_send_msg( lrdo_DEVOPNREADERROR, 0, 0 );
                return (rc);
        }

	/* recover from extendfs() ? */
	if (loglocation == INLINELOG &&
	    (vopen[logminor].status & FM_EXTENDFS))
	{
                     fsck_send_msg( lrdo_REXTNDBEGIN, 0, 0 );
		rc = recoverExtendFS(fd);
                     fsck_send_msg( lrdo_REXTNDDONE, 0, 0 );
		return rc;
	}

        /*
         *      open log
         */
        logfd = logOpen();

        /*
         * validate log superblock
         *
         * aggregate block size is for log file as well.
         */
        rc = ujfs_rw_diskblocks( logfd,
                (uint64)(vopen[logminor].logxaddr+LOGPNTOB(LOGSUPER_B)),
                (unsigned)sizeof(logsuper_t),
                (char *)&logsup,
                GET);
        if (rc != 0)
        {
                fsck_send_msg( lrdo_CANTREADLOGSUP, 0, 0 );
                rc = LOGSUPER_READ_ERROR;
                goto error_out;
        }

        if (logsup.magic != LOGMAGIC )
        {
                fsck_send_msg( lrdo_LOGSUPBADMGC, 0, 0 );
                rc = NOT_LOG_FILE_ERROR;
                goto error_out;
        }

        if (logsup.version != LOGVERSION)
        {
                fsck_send_msg( lrdo_LOGSUPBADVER, 0, 0 );
                rc = JFS_VERSION_ERROR;
                goto error_out;
        }

        if (logsup.state == LOGREDONE)  {
                fsck_send_msg( lrdo_ALREADYREDONE, 0, 0 );
                if ((rc = updateSuper(logminor)) !=0)  {
                        fsck_send_msg( lrdo_CANTUPDLOGSUP, 0, 0 );
                        return(rc); 
                        }
                return (0);
                }

        logsize = logsup.size;
        logserial = logsup.serial;

        /*
         * find the end of log
         */
        logend = findEndOfLog();

        if (logend  < 0)
        {
                sprintf( message_parm_0, "0x0%lx", logend );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_LOGEND, 0, 1 );

                fsck_send_msg( lrdo_LOGENDBAD1, 0, 0 );
                logError(LOGEND,0);
                rc = ujfs_rw_diskblocks(logfd,
                      (uint64)(vopen[logminor].logxaddr+LOGPNTOB(LOGSUPER_B)),
                                        (unsigned long)LOGPSIZE,
                                        (char *)&logsup,
                                        PUT);
                rc = logend;
                goto error_out;
        }

        /*
         * allocate/initialize logredo runtime data structures and
         * initialize each file system associated with the log based on
         * the contents of its superblock
         */
        if (( rc = logredoInit()) !=0)
        {
                sprintf( message_parm_0, "%ld", rc );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                sprintf( message_parm_1, "%ld", errno );
                msgprms[1] = message_parm_1;
                msgprmidx[1] = 0;
                fsck_send_msg( lrdo_INITFAILED, 0, 2 );
                goto error_out;
        }

        /*
         * Now that the aggregate superblock has been read, do some
         * more validation of the log superblock 
         */
        if( logsup.bsize != vopen[logminor].lblksize ) {
                fsck_send_msg( lrdo_LOGSUPBADBLKSZ, 0, 0 );
                rc = JFS_BLKSIZE_ERROR;
                goto error_out;
                }

        if( logsup.l2bsize != vopen[logminor].l2bsize ) {
                fsck_send_msg( lrdo_LOGSUPBADL2BLKSZ, 0, 0 );
                rc = JFS_L2BLKSIZE_ERROR;
                goto error_out;
                }

        aggsb_numpages = lengthPXD( &(vopen[logminor].log_pxd) ) *
                                       logsup.bsize / LOGPSIZE;
        if( logsup.size != aggsb_numpages ) {
                fsck_send_msg( lrdo_LOGSUPBADLOGSZ, 0, 0 );
                rc = JFS_LOGSIZE_ERROR;
                goto error_out;
                }

        highest_lr_byte = logsup.size * LOGPSIZE - LOGRDSIZE;

        if ( (logend  < lowest_lr_byte) || (logend > highest_lr_byte) ) {
                sprintf( message_parm_0, "0x0%lx", logend );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_LOGEND, 0, 1 );

                fsck_send_msg( lrdo_LOGENDBAD2, 0, 0 );
                rc = INVALID_LOGEND;
                goto error_out;
                }

        /*
         *      replay log
         *
         * read log backwards and process records as we go.
         * reading stops at place specified by first SYNCPT we
         * encounter.
         */
        nlogrecords = lastaddr = 0;
        nextaddr = logend;

        do
        {
                logaddr = nextaddr;
                nextaddr = logRead(logaddr, &ld, afterdata);
#ifdef _JFS_DEBUG
printf("logredo: logaddr=0x0%x (0d0%d)\tNextaddr=0x0%x (0d0%d)\n", 
		logaddr, logaddr, nextaddr, nextaddr  );
#endif
                DBG_TRACE(("Logaddr=%x\nNextaddr=%x\n",logaddr,nextaddr))
                nlogrecords += 1;
	/*
	 *
	 * Validate the nextaddr as much as possible 
	 *
	 */
                if (nextaddr < 0)
                {
                        fsck_send_msg( lrdo_NEXTADDRINVALID, 0, 0 );
                        rc = nextaddr;
                        goto error_out;                                         // @D1
                }

                if ( (nextaddr  < lowest_lr_byte) || (nextaddr > highest_lr_byte) ) {
                    sprintf( message_parm_0, "0x0%lx", nextaddr );
                    msgprms[0] = message_parm_0;
                    msgprmidx[0] = 0;
                    fsck_send_msg( lrdo_NEXTADDROUTRANGE, 0, 1 );
                    rc = INVALID_NEXTADDR;
                    goto error_out;
                    }

                if ( nextaddr  == logaddr ) {
                    sprintf( message_parm_0, "0x0%lx", nextaddr );
                    msgprms[0] = message_parm_0;
                    msgprmidx[0] = 0;
                    fsck_send_msg( lrdo_NEXTADDRSAME, 0, 1 );
                    rc = NEXTADDR_SAME;
                    goto error_out;
                    }

                if(  nextaddr > logaddr ) {
                    if( log_has_wrapped ) {
                        fsck_send_msg( lrdo_LOGWRAPPED, 0, 0 );
                        rc = LOG_WRAPPED_TWICE;
                        goto error_out;
                        }
                    else {
                        log_has_wrapped = -1;
                        }
                    }
	/*
	 *
	 * The addresses seem ok.  Process the current record.
	 *
	 */
                switch(ld.type) {

                case LOG_COMMIT:
                        rc = doCommit(&ld);
                        if (rc)
                        {
                                sprintf( message_parm_0, "0x0%lx", logaddr );
                                msgprms[0] = message_parm_0;
                                msgprmidx[0] = 0;
                                fsck_send_msg( lrdo_BADCOMMIT, 0, 1 );

                                goto error_out;
                        }
                        break;
                case LOG_MOUNT:
                        sprintf( message_parm_0, "0x0%lx", logaddr );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_MOUNTRECORD, 0, 1 );

                        rc = doMount(&ld);
                        if (rc)
                        {
                                sprintf( message_parm_0, "0x0%lx", logaddr );
                                msgprms[0] = message_parm_0;
                                msgprmidx[0] = 0;
                                fsck_send_msg( lrdo_BADMOUNT, 0, 1 );

                                goto error_out;
                        }
                        break;

                case LOG_SYNCPT:
                        sprintf( message_parm_0, "0x0%lx", logaddr );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_SYNCRECORD, 0, 1 );

                        rc = 0;
                        if (lastaddr == 0)
                        {
                                syncrecord = logaddr;
                                lastaddr = (ld.log.syncpt.sync == 0)
                                        ? logaddr
                                        : ld.log.syncpt.sync;
                        }
                        break;

                case LOG_REDOPAGE:
#ifdef _JFS_DEBUG
/*
printf("logredo: record type = REDOPAGE \n");
*/
/*
prtdesc(&ld);
printf(" logaddr: 0x%08x",logaddr);
printf(" ld->backchain: 0x%08x",ld.backchain);
printf(" ld->length: 0x%04x\n",ld.length);
printf("logtid:0x%08x",ld.logtid);
printf(" Fileset:0x%08x",ld.log.redopage.fileset);
printf(" inode:0x%08x\n",ld.log.redopage.inode);
printf(" pxd.len=0x%08x",lengthPXD(&ld.log.redopage.pxd));
printf(" pxd.addr=0x%llx\n", addressPXD(&ld.log.redopage.pxd));
*/
#endif
                                DBG_TRACE(("jfs_logredo:Case Log_redoPage"))
                        rc = doAfter(&ld, logaddr);
                        if (rc)
                        {
                                sprintf( message_parm_0, "0x0%lx", logaddr );
                                msgprms[0] = message_parm_0;
                                msgprmidx[0] = 0;
                                fsck_send_msg( lrdo_BADREDOPAGE, 0, 1 );
                                goto error_out;
                        }
                        break;

                case LOG_NOREDOPAGE:
#ifdef _JFS_DEBUG
/*
printf("logredo: record type = NOREDOPAGE \n");
*/
/*
printf(" logaddr: 0x%08x",logaddr);
printf(" ld->backchain: 0x%08x",ld.backchain);
printf(" ld->length: 0x%04x\n",ld.length);
printf("logtid:0x%08x",ld.logtid);
printf(" Fileset:0x%08x",ld.log.redopage.fileset);
printf(" inode:0x%08x\n",ld.log.redopage.inode);
printf(" pxd.len=0x%08x",lengthPXD(&ld.log.redopage.pxd));
printf(" pxd.addr=0x%llx\n", addressPXD(&ld.log.redopage.pxd));
*/
#endif
                        DBG_TRACE(("jfs_logredo:Case Log_noredopage"))
                        rc = doNoRedoPage(&ld);
                        if (rc)
                        {
                                sprintf( message_parm_0, "0x0%lx", logaddr );
                                msgprms[0] = message_parm_0;
                                msgprmidx[0] = 0;
                                fsck_send_msg( lrdo_BADNOREDOPAGE, 0, 1 );
                                goto error_out;
                        }
                        break;

                case LOG_NOREDOINOEXT:
#ifdef _JFS_DEBUG
/*
printf("logredo: record type = NOREDOINOEXT \n");
*/
/*
printf(" logaddr: 0x%08x",logaddr);
printf(" ld->backchain: 0x%08x",ld.backchain);
printf(" ld->length: 0x%04x\n",ld.length);
printf("logtid:0x%08x",ld.logtid);
printf(" Fileset:0x%08x",ld.log.redopage.fileset);
printf(" inode:0x%08x\n",ld.log.redopage.inode);
printf(" pxd.len=0x%08x",lengthPXD(&ld.log.redopage.pxd));
printf(" pxd.addr=0x%llx\n", addressPXD(&ld.log.redopage.pxd));
*/
#endif
                        DBG_TRACE(("jfs_logredo:Case Log_noredoinoext"))
                        rc = doNoRedoInoExt(&ld);
                        if (rc)
                        {
                                sprintf( message_parm_0, "0x0%lx", logaddr );
                                msgprms[0] = message_parm_0;
                                msgprmidx[0] = 0;
                                fsck_send_msg( lrdo_BADNOREDOINOEXT, 0, 1 );
                                goto error_out;
                        }
                        break;

                case LOG_UPDATEMAP:
                        rc = doUpdateMap(&ld);
                        if (rc)
                        {
                                sprintf( message_parm_0, "0x0%lx", logaddr );
                                msgprms[0] = message_parm_0;
                                msgprmidx[0] = 0;
                                fsck_send_msg( lrdo_BADUPDATEMAP, 0, 1 );
                                goto error_out;
                        }
                        break;

                default:
                        sprintf( message_parm_0, "0x0%lx", logaddr );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_UNKNOWNTYPE, 0, 1 );
                        rc = UNRECOG_LOGRECTYP;
                        goto error_out;
                        break;
                }

                if( rc < 0 )
                {
                        fsck_send_msg( lrdo_ERRORNEEDREFORMAT, 0, 0 );
                        goto error_out;
                }

                if( rc != 0 )
                {
                        fsck_send_msg( lrdo_ERRORCANTCONTIN, 0, 0 );
                        goto error_out;
                }

		       /*  
	        	* If the transaction just completed was the last
			* for the current transaction, then flush the
			* buffers.
		        */
#ifdef Drova   PS21032004
                if( end_of_transaction != 0 )                              // @D2
                {                                                          // @D2
		        for (k = 1; k < NBUFPOOL ; k ++)                   // @D2
		        {                                                  // @D2
                		if ((rc = bflush(k,  &buffer[k-1]))!= 0)   // @D2
		                        goto error_out;                    // @D2
		        }                                                  // @D2
		        end_of_transaction = 0;                            // @D2
                }                                                          // @D2
#endif

        } while (logaddr != lastaddr);
	/*
	 * If any 'dtpage extend' records were processed, then we need
	 * to go back and rebuild their freelists.  This cannot be done
	 * when the 'dtpage extend' record is processed, since there may
	 * be records processed later which affect the previous (shorter)
	 * version of the dtpage.  Only after all these records are processed
	 * can we safely and accurately rebuild the freelist.
	 */
        if( numExtDtPg != 0 ) {
            rc = doExtDtPg();
            }

loopexit:
        /*
         * run logform?
         */
        logformit = (rc < 0);

        /*
         * flush data page buffer cache
         */
        for (k = 1; k < NBUFPOOL ; k ++)
        {
                if ((rc = bflush(k,  &buffer[k-1]))!= 0)
                        break;
        }

        /*
         *      finalize file systems
         *
         * update allocation map and superblock of file systems
         * of volumes which are open if they were modified here.
         * i.e. if they were not previously unmounted cleanly.
         */
        for(k = 0; k < NUMMINOR; k++)
        {
                if (!vopen[k].fd)
                        continue;

                /* don't update the maps if the aggregate/lv is
                 * FM_DIRTY since fsck will rebuild maps anyway
                 */
                if ( !vopen[k].is_fsdirty )
                {
                        if ((rc = updateMaps(k))!= 0)
                        {
                                fsck_send_msg( lrdo_ERRORCANTUPDMAPS, 0, 0 );
                                goto error_out;
                        }
                }

                if ((rc = updateSuper(k)) !=0)
                {
                        fsck_send_msg( lrdo_ERRORCANTUPDFSSUPER, 0, 0 );
                        goto error_out;
                }
        }

        /*
         *      finalize log.
         *
         * clear active list.
         * If this is a fully replayed log then it can be moved to earlier
         * versions of the operating system.  Therefore switch the magic
         * number to the earliest level.
         */
        if (logsup.state != LOGREADERR)
        {
                for (k = 0; k < NUMMINOR/32; k++)
                        logsup.active[k] = 0;

                logsup.end = logend;
                logsup.state = LOGREDONE;
                logsup.magic = LOGMAGIC;
        }
        rc = ujfs_rw_diskblocks(logfd,
              (uint64)(vopen[logminor].logxaddr+LOGPNTOB(LOGSUPER_B)),
                                (unsigned long)LOGPSIZE,
                                (char *)&logsup,
                                PUT);
        if (actual != sizeof(logsup) && errno == EIO )
        {
              fsck_send_msg( lrdo_CANTWRITELOGSUPER, 0, 0 );
              rc = LOGSUPER_WRITEERROR1;
        }

	/*
	 * now log some info for the curious
	 */
        sprintf( message_parm_0, "0x0%lx", logend );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_LOGEND, 0, 1 );

        sprintf( message_parm_0, "0x0%lx", syncrecord );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_RPTSYNCNUM, 0, 1 );

        sprintf( message_parm_0, "0x0%lx", lastaddr );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_RPTSYNCADDR, 0, 1 );

        sprintf( message_parm_0, "(d) %d", nlogrecords );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_RPTNUMLOGREC, 0, 1 );

        sprintf( message_parm_0, "(d) %d", numdoblk );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_RPTNUMDOBLK, 0, 1 );

        sprintf( message_parm_0, "(d) %d", numnodofile );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_RPTNUMNODOBLK, 0, 1 );

error_out:

        if( rc > 0 ) {
                rc = rc * (-1);
                }

		/*
		 * If everything went ok except that we didn't have
		 * enough memory to deal with the block map, tell chkdsk
		 * to be sure to do a full check and repair, but that a log
		 * format is not necessary
		 */
        if( (rc == 0) && Insuff_memory_for_maps )  {  
                rc = ENOMEM25;
                }

        return (rc);
}


/*
 * NAME:        doMount(ld)
 *
 * FUNCTION:    a log mount record is the first-in-time record which is
 *              put in the log so it is the last we want to process in
 *              logredo. so we mark volume as cleanly unmounted in vopen
 *              array. the mount record is imperative when the volume
 *              is a newly made filesystem.
 */
int
doMount(
struct lrd * ld)        /* pointer to record descriptor */
{
        int vol, status;

        vol = 0;
        status = vopen[vol].status;
           DBG_TRACE(("Logredo:domount: status=%d\n", status))

        if (!(status & (FM_LOGREDO|FM_DIRTY)))
                vopen[vol].status = FM_CLEAN;

        return (0);
}

/*
 * NAME:        openVol(vol)
 *
 * FUNCTION:    open the aggregate/volume specified.
 *              check if it was cleanly unmounted. also check log
 *              serial number. initialize disk and inode mpas.
 */
int32 openVol(
        int32   vol)    /* device minor number of aggregate/lv */
{
        int32   k, fd, rc, l2agsize, agsize;
        int64   fssize; /* number of aggre. blks in the aggregate/lv   */
        struct superblock       sb;

        fd = vopen[logminor].fd;

        /* read superblock of the aggregate/volume */
        if ((rc = rdwrSuper(fd, &sb, PB_READ)) !=0)
        {
                fsck_send_msg( lrdo_CANTREADFSSUPER, 0, 0 );

                fsError(READERR,vol, SUPER1_B);
                vopen[vol].fd = 0;
                return(FSSUPER_READERROR1);
        }

        /* check magic number and initialize version specific
         * values in the vopen struct for this vol.
         */
        if (strncmp(sb.s_magic,JFS_MAGIC,(unsigned)strlen(JFS_MAGIC)) == 0)
        {
                if (sb.s_version != JFS_VERSION)
                {
                        fsck_send_msg( lrdo_FSSUPERBADMAGIC, 0, 0 );
                        vopen[vol].fd = 0;
                        return(LOGSUPER_BADVERSION);
                }

                if ( loglocation == OUTLINELOG &&
                     (sb.s_flag & JFS_INLINELOG == JFS_INLINELOG ))
                {
                        fsck_send_msg( lrdo_FSSUPERBADLOGLOC, 0, 0 );
                        vopen[vol].fd = 0;
                        return(LOGSUPER_BADLOGLOC);
                }
                if ( loglocation != INLINELOG)
                {
                        return(LOGSUPER_LOGNOTINLINE);
                }
                vopen[vol].lblksize = sb.s_bsize;
                vopen[vol].l2bsize = sb.s_l2bsize;
                vopen[vol].l2bfactor = sb.s_l2bfactor;
                fssize = sb.s_size >> sb.s_l2bfactor;
                vopen[vol].fssize = fssize;
                vopen[vol].agsize = sb.s_agsize;
                /* LOG2NUM will alter agsize, so use local var */
                agsize = vopen[vol].agsize;
                LOG2NUM(agsize, l2agsize);
                vopen[vol].numag = fssize >> l2agsize;
                if ( fssize & (vopen[vol].agsize - 1 ))
                        vopen[vol].numag += 1;
                vopen[vol].l2agsize = l2agsize;
        }
        else
        {
                fsck_send_msg( lrdo_FSSUPERBADMAGIC, 0, 0 );
                vopen[vol].fd = 0;
                return(LOGSUPER_BADMAGIC);
        }

        /* 
	 *set lbperpage in vopen.
         */
        vopen[vol].lbperpage = PSIZE >> vopen[vol].l2bsize;

        /* 
         * was it cleanly umounted ?
         */
        if (sb.s_state == FM_CLEAN)
        {
                vopen[vol].status  = FM_CLEAN;
                vopen[vol].fd = 0;
                return(0);
        }

        /* 
	 * else get status of volume
         */
        vopen[vol].status = sb.s_state;
        vopen[vol].is_fsdirty = ( sb.s_state & FM_DIRTY );

        /* 
	 *check log serial number
         */
        if (sb.s_logserial != logserial)
        {
                fsck_send_msg( lrdo_FSSUPERBADLOGSER, 0, 0 );
                vopen[vol].fd = 0;
                fsError(SERIALNO, vol, SUPER1_B);
                return(LOGSUPER_BADSERIAL);
        }

        /* initialize the disk and inode maps
         */
        if ((rc = initMaps(vol)) !=0)
        {
                fsck_send_msg( lrdo_INITMAPSFAIL, 0, 0 );
                fsError(MAPERR,vol,0);
        }
        return (rc);
}


/*
 * NAME:         updateSuper(vol)
 *
 * FUNCTION:     updates primary aggregate/lv's superblock status and
 *               writes it out.
 */
int32
updateSuper(
int vol)   /* device minor number of aggregate/lv */
{
        int rc, status;
        struct superblock sb;

        /* read in superblock of the volume */
        if ((rc = rdwrSuper(vopen[vol].fd, &sb, PB_READ)) !=0)
        {
                fsck_send_msg( lrdo_READFSSUPERFAIL, 0, 0 );
                return(FSSUPER_READERROR2);
        }

        /* mark superblock state. write it out */
        status = vopen[vol].status;
        if (!(status & (FM_DIRTY|FM_LOGREDO)))
                sb.s_state = FM_CLEAN;
        else
                sb.s_state = status & ~FM_EXTENDFS;

        if ((rc = rdwrSuper(vopen[vol].fd, &sb, PB_UPDATE)) !=0)
        {
                fsck_send_msg( lrdo_WRITEFSSUPERFAIL, 0, 0 );
        }

        return(rc);
}


/*
 * NAME:        rdwrSuper(fd, sb, rwflag)
 *
 * FUNCTION:    read or write the superblock for the file system described
 *              by the file descriptor of the opened aggregate/lv.
 *              for read, if a read of primary superblock is failed,
 *              try to read the secondary superblock. report error only
 *              when both reads failed.
 *              for write, any write failure should be reported.
 */
int32 rdwrSuper(
        int32   fd,             /* file descriptor */
        struct  superblock *sb, /* superblock of the opened aggregate/lv */
        int32   rwflag)         /* PB_READ, PB_UPDATE   */
{
        int32 rc;
        uint32 actual;
        uint64 super_offset;
        union {
                struct superblock super;
                char block[PSIZE];
        } super;

        if( use_2ndary_agg_superblock ) {
                super_offset = SUPER2_OFF;
                }
        else {
                super_offset = SUPER1_OFF;
                }
        /*
         * seek to the postion of the primary superblock.
         * since at this time we don't know the aggregate/lv
         * logical block size yet, we have to use the fixed
         * byte offset address super_offset to seek for.
         */

        /*
         * read super block
         */
        if (rwflag == PB_READ)
        {
                rc = ujfs_rw_diskblocks(fd, super_offset, 
                                        (unsigned)SIZE_OF_SUPER,
                                        super.block, GET);
                if ( rc != 0 )
                {
                        if( !use_2ndary_agg_superblock ) {
                                fsck_send_msg( lrdo_READFSPRIMSBFAIL, 0, 0 );
                                return(CANTREAD_PRIMFSSUPER);
                                }
                        else {
                                fsck_send_msg( lrdo_READFS2NDSBFAIL, 0, 0 );
                                return(CANTREAD_2NDFSSUPER);
                               }
                }

                *sb = super.super;
        }
        /*
         * write superblock
         */
        else /* PB_UPDATE */
        {
                /* ? memset(super.block, 0, SIZE_OF_SUPER); */
                super.super = *sb;

                /* 
                 * write whichever superblock we're working with.
                 * chkdsk will take care of replicating it.
                 */
                rc = ujfs_rw_diskblocks(fd, super_offset, 
                                        (unsigned)SIZE_OF_SUPER,
                                        super.block, PUT);
                if ( rc != 0 )
                {
                        if( !use_2ndary_agg_superblock ) {
                                fsck_send_msg( lrdo_WRITEFSPRIMSBFAIL, 0, 0 );
                                return(CANTWRITE_PRIMFSSUPER);
                                }
                        else {
                                fsck_send_msg( lrdo_WRITEFS2NDSBFAIL, 0, 0 );
                                return(CANTWRITE_2NDFSSUPER);
                                }
                }
        }

        return (0);
}


/*
 * NAME:        bflush()
 *
 * FUNCTION:    write out appropriate portion of buffer page if its modified.
 *              Note that a dtree page may not be 4k, depending on the length
 *              field specified in pxd. Write out only length that is needed.
 */
bflush(
        int32   k,      /*  The index in bufhdr that describes buf */
        struct bufpool *buf)  /* pointer to buffer pool page */
{
        int32   rc, fd, vol;
        int32   nbytes;
        int64   blkno;

        /* nothing to do ? */
        if (bufhdr[k].modify == 0)
                return (0);

        /* write it out */
        vol = bufhdr[k].vol;
        fd = vopen[vol].fd;
        blkno = addressPXD(&bufhdr[k].pxd);
        nbytes = lengthPXD(&bufhdr[k].pxd) << vopen[vol].l2bsize;
        rc = ujfs_rw_diskblocks(fd, (uint64)(blkno << vopen[vol].l2bsize),
                                 (unsigned)nbytes, (char *)buf, PUT);
        if (rc != 0)
        {
                fsck_send_msg( lrdo_BUFFLUSHFAIL, 0, 0 );
                return(BFLUSH_WRITEERROR);
        }

        return (0);
}


/*
 * NAME:        isLogging()
 *
 * FUNCTION:    Check to see if device is currently in use as a log
 *
 * NOTES: mntctl and stat are AIX system calls.  major and minor number are
 *        also AIX concept. They could be changed in OS/2 system.
 *
 * DATA STRUCTURES: global variables logmajor and logminor are altered.
 *
 * PARAMETERS:  logname - device name
 *              fd      - file handle for the logname
 *              vmt     - pointer to buffer that contains an array of
 *                        vmount sturctures, which are returned by the
 *                        mntctl system call.
 *              cnt     - number of vmount structures in buffer pointed by vmt
 *
 * RETURNS:     0       - device logname is not actively used as a log
 *              NOT_FSDEV_ERROR (-6) Not a valid fs device (from isFilesystem)
 *              NOT_INLINELOG_ERROR (-7) Log is not an inline log  (from isFilesystem)
 *              MAJOR_ERROR - returned from isFilesystem().
 *              MINOR_ERROR - devices already mounted that are using logname
 */
isLogging(
        caddr_t logname,
        int32   fd,
        char    *vmt_in,
        int32   cnt)
{
        int32 i, rc = 0;

        /*
         * determine if logname represents a file system device  or
         * a log device and set its major number
         */
        if ((rc = isFilesystem(logname, fd)) < 0)
                return(rc);

        return (rc);
}


/*
 * NAME:        isFilesystem()
 *
 * FUNCTION:     open the device to see if it's a valid filesystem.
 *               If open failed, then return MAJOR_ERROR.
 *               If open ok, and it is a valid file system,
 *               return the minor device number of the log for this
 *               filesystem. Otherwise, return MINOR_ERROR.
 *
 * PRE CONDITION: other process opened the device log should allow
 *                a O_RDONLY re-open.
 *
 * POST CONDITION: this O_RDONLY open is closed.
 *
 * PARAMETERS:  dev_name        - device name. This is the name passed to
 *                                logredo.
 *
 * RETURNS:
 *              NOT_FSDEV_ERROR (-6) Not a valid fs device
 *              NOT_INLINELOG_ERROR (-7) Log is not an inline log -
 *              MAJOR_ERROR(-2) -  open device  or stat device failure, or
 *                                 s_logdev number wrong.
 *                                 for OS/2, if it is not an inlinelog, it is
 *                                 an error.
 *              0               -  1)read fs superblock ok but dev_name does not
 *                                   represent a file system device name
 *                                 2)read fs fs superblock ok, dev_name
 *                                   represents a fs device name.
 *                                   loglocation is set up. ( INLINELOG or
 *                                   OUTLINELOG ).
 *                                 3)read fs superblock failed, but successful
 *                                   to read the device as log
 */
int32 isFilesystem(
        caddr_t dev_name,
        int32   fd)
{
        int32 rc = 0;
        struct superblock sb;
        int32 devmajor = 0;
        int32 devminor = 0;
        uint32 Action, actual;

        /*
         * for _JFS_OS2, each file system must maintain its own in-line log.
         */

        /*
         * try the LV as file system with in-line log
         */
        if ((rc = rdwrSuper(fd, &sb, PB_READ)) == 0)
        {
                /*
                 * is the LV a file system ?
                 */
                if (strncmp(sb.s_magic,JFS_MAGIC,(unsigned)strlen(JFS_MAGIC))
                                         == 0 )
                {
                        /*
                         * does file system contains its in-line log ?
                         */
                        if ( ( sb.s_flag & JFS_INLINELOG ) == JFS_INLINELOG )
                        {

                                if ( !( logmajor == devmajor &&
                                        logminor == devminor ) )
                                {
                                        fsck_send_msg( lrdo_NOTAFSDEVNUM, 0, 0 );
                                        return(NOT_FSDEV_ERROR);

                                }

                                loglocation = INLINELOG;
                                vopen[devminor].fd = fd;
                                vopen[devminor].status = sb.s_state;
                                vopen[devminor].log_pxd = sb.s_logpxd;
                                vopen[devminor].l2bsize = sb.s_l2bsize;
                                vopen[devminor].logxaddr =
                                      addressPXD(&sb.s_logpxd) << sb.s_l2bsize;
                        }
                        /*
                         * the FS is associated with external/out-of-line log
                         */
                        else
                        {
                                fsck_send_msg( lrdo_LOGNOTINLINE1, 0, 0 );
                                rc = NOT_INLINELOG_ERROR;
                        }
                }
                /*
                 * validation failure: the LV is not a file system:
                 */
                else
                {
                        fsck_send_msg( lrdo_NOTAFSDEV, 0, 0 );
                        rc = NOT_FSDEV_ERROR;
                }
        }
        /*
         * read failure: try the LV as out-of-line log
         */
        else
        {
                fsck_send_msg( lrdo_LOGNOTINLINE2, 0, 0 );
                rc = NOT_INLINELOG_ERROR;
        }
        return (rc);
}


/*
 * NAME:        logOpen()
 *
 * FUNCTION:    opens the log and returns its fd.
 *              sets logmajor to the major number of the device.
 *
 * PRE CONDITION: logmajor and logminor have been set up
 *
 * POST CONDITION: log is opened and locked by the loglock file
 *
 * PARAMETERS: NONE
 *
 * RETURNS:     >= 0    - file descriptor reported from makeOpen()
 *              < 0     - any error reported from makeOpen()
 */
int32 logOpen()
{
        int k;
        int fd;

        sprintf( message_parm_0, "0x0%x", logmajor );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "0x0%x", logminor );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        fsck_send_msg( lrdo_LOGOPEN, 0, 2 );

        fd = vopen[logminor].fd;

        return(fd);
}


extern void exit(int);


/*
 * NAME:        fsError(type,vol,bn)
 *
 * FUNCTION:    error handling code for the specified
 *              aggregate/lv (filesystem).
 */
fsError(
int type,       /* error types */
int vol,        /* the minor number of the aggregate/lv */
int64 bn)       /* aggregate block No.  */
{
        int status;

        sprintf( message_parm_0, "(d) %d", vol );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_ERRORONVOL, 0, 1 );

        retcode = -1;
        vopen[vol].status = FM_LOGREDO;

        switch(type) {
        case OPENERR:
                fsck_send_msg( lrdo_OPENFAILED, 0, 0 );
                break;
        case MAPERR:
                fsck_send_msg( lrdo_CANTINITMAPS, 0, 0 );
                break;
        case DBTYPE:
                sprintf( message_parm_0, "0x0%llx", bn );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_BADDISKBLKNUM, 0, 1 );
                break;
        case INOTYPE:
                sprintf( message_parm_0, "(d) %ld", bn );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_BADINODENUM, 0, 1 );
                break;
        case READERR:
                sprintf( message_parm_0, "0x0%llx", bn );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_CANTREADBLK, 0, 1 );
                break;
        case SERIALNO:
                fsck_send_msg( lrdo_BADLOGSER, 0, 0 );
                break;
        case IOERROR:
                sprintf( message_parm_0, "0x0%llx", bn );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_IOERRREADINGBLK, 0, 1 );
                break;
        case LOGRCERR:
                sprintf( message_parm_0, "(d) %d", bn );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_BADUPDMAPREC, 0, 1 );
                break;
        }
        return (0);
}

/*
 *      logError(type)
 *
 * error handling for log read errors.
 */
logError(
int type,
int logaddr)
{
        int k;
        retcode = -1;
        logsup.state = LOGREADERR;
        switch(type) {
        case LOGEND:
                fsck_send_msg( lrdo_FINDLOGENDFAIL, 0, 1 );
                break;
        case READERR:
                sprintf( message_parm_0, "0x0%llx", logaddr );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_LOGREADFAIL, 0, 1 );
                break;
        case UNKNOWNR:
                sprintf( message_parm_0, "0x0%llx", logaddr );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_UNRECOGTYPE, 0, 1 );
                break;
        case IOERROR:
                sprintf( message_parm_0, "0x0%llx", logaddr );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_IOERRONLOG, 0, 1 );
                break;
        case LOGWRAP:
                fsck_send_msg( lrdo_LOGWRAP, 0, 0 );
        }

        /* mark all open volumes in error
         */
        for(k = 0; k < NUMMINOR; k++)
        {
                if (vopen[k].fd && vopen[k].status != FM_CLEAN)
                        vopen[k].status = FM_LOGREDO;
        }
        return (0);
}


/*
 *	recoverExtendFS()
 *
 * function: recover crash while in extendfs() for inline log;
 *
 * note: fs superblock fields remains pre-extendfs state,
 * while that bmap file, fsck and inline log area may be in 
 * unknown state;
 *
 * at entry, only log type/lv has been validated;
 * for inline log: vopen[logminor = 0], fs fd = log fd;
 */
static int32 recoverExtendFS(int32	fd)
{
	int32	rc;
	struct vopen	*logLV;
	struct superblock	*sbp;
	dinode_t	*dip1, *dip2;
	dbmap_t	*bgcp;
	xtpage_t	*p;
	int64	lmchild, xaddr, xoff, barrier, t64, agsize;
	uint8	lmxflag;
	int32	i;
	char	*dip, *bp;
	pxd_t	temp_pxd;

/*
printf("recover from extendfs.\n");
*/

        logfd = fd;	/* fake logOpen() */
	logLV = &vopen[logminor];

	/*
	 * read bmap global control page
	 */
	/* read superblock yet again */
	sbp = (struct superblock *)&buffer[0];
	if (rc = rdwrSuper(fd, sbp, PB_READ))
		goto errout;

	/* read primary block allocation map inode */
	dip = (char *)&buffer[1];
	if (rc = ujfs_rw_diskblocks(fd, AITBL_OFF, PSIZE, dip, GET)) {
                     fsck_send_msg( lrdo_EXTFSREADFSSUPERFAIL, 0, 0 );
		goto errout;
                     }

        /* locate the inode in the buffer page */
	dip1 = (dinode_t *)dip;
        dip1 += BMAP_I;

	bp = (char *)&buffer[2];	/* utility buffer */

        /* start from root in dinode */
        p = (xtpage_t *)&dip1->di_btroot;
        /* is this page leaf ? */
        if (p->header.flag & BT_LEAF)
                goto rdbgcp;

        /* traverse down leftmost child node to leftmost leaf of xtree */
        do
        {
                /* read in the leftmost child page */
        	t64 = addressXAD(&p->xad[XTENTRYSTART]) << sbp->s_l2bsize;
                if (rc = ujfs_rw_diskblocks(fd, t64, PSIZE, bp, GET))  {
                        fsck_send_msg( lrdo_EXTFSREADBLKMAPINOFAIL, 0, 0 );
                        goto errout;
                        }

                p = (xtpage_t *)bp;
                /* is this page leaf ? */
                if (p->header.flag & BT_LEAF)
                        break;
        } while (TRUE);

rdbgcp:
	t64 = addressXAD(&p->xad[XTENTRYSTART]) << sbp->s_l2bsize;
	if (rc = ujfs_rw_diskblocks(fd, t64, PSIZE, bp, GET))  {
		sprintf( message_parm_0, "0x0%llx", t64 );
		msgprms[0] = message_parm_0;
		msgprmidx[0] = 0;
		fsck_send_msg( lrdo_EXTFSREADBLKFAIL1, 0, 1 );
		goto errout;
		}
	bgcp = (dbmap_t *)bp;

	/*
	 * recover to pre- or post-extendfs state ?:
	 */
	if (bgcp->dn_mapsize > (sbp->s_size >> sbp->s_l2bfactor))
	{
		agsize = bgcp->dn_agsize;
		goto postx;
	}
	
	/*
	 *	recover pre-extendfs state
	 */
	/*
	 * reset block allocation map inode (xtree root)
	 */
	/* read 2ndary block allocation map inode */
	t64 = addressPXD(&sbp->s_ait2) << sbp->s_l2bsize;
	if (rc = ujfs_rw_diskblocks(fd, t64, PSIZE, bp, GET))  {
		sprintf( message_parm_0, "0x0%llx", t64 );
		msgprms[0] = message_parm_0;
		msgprmidx[0] = 0;
		fsck_send_msg( lrdo_EXTFSREADBLKFAIL2, 0, 1 );
		goto errout;
		}
	dip2 = (dinode_t *)bp;
	dip2 += BMAP_I;

	/* 
	 * Reset primary bam inode with 2ndary bam inode 
	 *
	 * Not forgetting to reset di_ixpxd since they are in different
	 * inode extents.
	 */
	memcpy((void *) &temp_pxd, (void *)&(dip1->di_ixpxd), sizeof(pxd_t) );
	memcpy(dip1, dip2, DISIZE);
	memcpy((void *)&(dip1->di_ixpxd), (void *) &temp_pxd, sizeof(pxd_t) );

	if (rc = ujfs_rw_diskblocks(fd, AITBL_OFF, PSIZE, dip, PUT))  {
		sprintf( message_parm_0, "0x0%llx", AITBL_OFF );
		msgprms[0] = message_parm_0;
		msgprmidx[0] = 0;
		fsck_send_msg( lrdo_EXTFSWRITEBLKFAIL1, 0, 1 );
		goto errout;
		}

	/*
	 * backout bmap file to fs size:
	 *
	 * trim xtree to range specified by i_size:
	 * xtree has been grown in append mode and
	 * written from right to left, bottom-up;
	 */
	barrier = dip1->di_size >> sbp->s_l2bsize;

	/* start with root */
	xaddr = 0;
	p = (xtpage_t *)&dip1->di_btroot;
	lmxflag = p->header.flag;
	p->header.next = 0;
	if (lmxflag & BT_INTERNAL)
	{
		/* save leftmost child xtpage xaddr */
		lmchild = addressXAD(&p->xad[XTENTRYSTART]);
	}

	/*
	 * scan each level of xtree via leftmost descend 
	 */
	while(1)
	{
		/*
		 * scan each xtpage of current level of xtree
		 */
		while(1) 
		{
			/*
			 * scan each xad in current xtpage
			 */
			for (i = XTENTRYSTART; i < p->header.nextindex; i++) 
			{
				/* test if extent is of interest */
				xoff = offsetXAD(&p->xad[i]);
				if (xoff < barrier)
					continue;

				/*
				 * barrier met in current page
				 */
				ASSERT(i > XTENTRYSTART);
				/* update current page */
				p->header.nextindex = i;
				if (xaddr)
				{
					/* discard further right sibling pages */
					p->header.next = 0;
					if (rc = ujfs_rw_diskblocks(fd, t64, PSIZE, p, PUT)) {
						sprintf( message_parm_0, "0x0%llx", t64 );
						msgprms[0] = message_parm_0;
						msgprmidx[0] = 0;
						fsck_send_msg( lrdo_EXTFSWRITEBLKFAIL2, 0, 1 );
						goto errout;
						}
				}

				goto nextLevel;
			}  /* end for current xtpage scan */

			/* barrier was not met in current page */

			/* read in next/right sibling xtpage */
			xaddr = p->header.next;
			if (xaddr)
			{
				if (xaddr >= barrier)
				{
					p->header.next = 0;
					if (rc = ujfs_rw_diskblocks(fd, t64, PSIZE, p, PUT)) {
						sprintf( message_parm_0, "0x0%llx", t64 );
						msgprms[0] = message_parm_0;
						msgprmidx[0] = 0;
						fsck_send_msg( lrdo_EXTFSWRITEBLKFAIL3, 0, 1 );
						break; /* break out current level */
						}
				}

				t64 = xaddr << sbp->s_l2bsize;
				if (rc = ujfs_rw_diskblocks(fd, t64, PSIZE, bp, GET))  {
					sprintf( message_parm_0, "0x0%llx", t64 );
					msgprms[0] = message_parm_0;
					msgprmidx[0] = 0;
					fsck_send_msg( lrdo_EXTFSREADBLKFAIL3, 0, 1 );
					goto errout;
					}

				p = (xtpage_t *)bp;
			}
			else
				break; /* break out current level */
		} /* end while current level scan */

		/*
		 * descend: read leftmost xtpage of next lower level of xtree
		 */
nextLevel:
		if (lmxflag & BT_INTERNAL)
		{
			/* get the leftmost child page  */
			xaddr = lmchild;
			t64 = xaddr << sbp->s_l2bsize;
			if (rc = ujfs_rw_diskblocks(fd, t64, PSIZE, bp, GET))  {
				sprintf( message_parm_0, "0x0%llx", t64 );
				msgprms[0] = message_parm_0;
				msgprmidx[0] = 0;
				fsck_send_msg( lrdo_EXTFSREADBLKFAIL4, 0, 1 );
				goto errout;
				}

			p = (xtpage_t *)bp;

			lmxflag = p->header.flag;
			if (lmxflag & BT_INTERNAL)
			{
				/* save leftmost child xtpage xaddr */
				lmchild = addressXAD(&p->xad[XTENTRYSTART]);
			}
		}
		else
			break;
	} /* end while level scan */

	/*
	 * reconstruct map;
	 *
	 * readBmap() init blocks beyond fs size in the last 
	 * partial dmap page as allocated which might have been 
	 * marked as free by extendfs();
	 */
	/* fake log opend/validated */
	logserial = sbp->s_logserial;

        /*
	 *	reconstruct maps
         */
	/* open LV and initialize maps for logminor */
        if (rc = logredoInit())  {
		fsck_send_msg( lrdo_EXTFSINITLOGREDOFAIL, 0, 0 );
		goto errout;
		}

	/* bypass log replay */

	/* update/write maps */
	updateMaps(logminor);

	/*
	 * reformat log
	 *
	 * request reformat original log  (which might have been 
	 * overwritten by extendfs() and set superblock clean
	 */
	jfs_logform(
		fd,		/* file descriptor for fs/log LV */
		(int32)sbp->s_bsize,	/* aggr blk size in bytes */
		(int32)sbp->s_l2bsize,	/* log2 of aggr blk size in bytes */
		(uint32)sbp->s_flag,	/* fs superblock s_flag */
		(int64)addressPXD(&sbp->s_logpxd), /* inline log start address 
						    * in aggr blk
						    */
		(int32)lengthPXD(&sbp->s_logpxd), /* inline log length 
						   * in aggr blk
						   */ 
		NULL,		/* outline log LV */
		0);		/* size of the outline log */

	/* update superblock */
	updateSuper(logminor);

           fsck_send_msg( lrdo_REXTNDTOPRE, 0, 0 );

	return 0;

	/*
	 *	recover post-extendfs state
	 */
postx:
	/*
	 * update 2ndary bam inode
	 */
	/* read 2ndary block allocation map inode */
	t64 = addressPXD(&sbp->s_ait2) << sbp->s_l2bsize;
	if (rc = ujfs_rw_diskblocks(fd, t64, PSIZE, bp, GET))  {
		sprintf( message_parm_0, "0x0%llx", t64 );
		msgprms[0] = message_parm_0;
		msgprmidx[0] = 0;
		fsck_send_msg( lrdo_EXTFSREADBLKFAIL5, 0, 1 );
		goto errout;
		}
	dip2 = (dinode_t *)bp;
	dip2 += BMAP_I;

	/* 
	 * Reset 2ndary bam inode with primary bam inode 
	 * Not forgetting to reset di_ixpxd since they are in different
	 * inode extents.
	 */
	memcpy((void *) &temp_pxd, (void *)&(dip2->di_ixpxd), sizeof(pxd_t) );
	memcpy(dip2, dip1, DISIZE);
	memcpy((void *)&(dip2->di_ixpxd), (void *) &temp_pxd, sizeof(pxd_t) );

	if (rc = ujfs_rw_diskblocks(fd, t64, PSIZE, bp, PUT))  {
		sprintf( message_parm_0, "0x0%llx", t64 );
		msgprms[0] = message_parm_0;
		msgprmidx[0] = 0;
		fsck_send_msg( lrdo_EXTFSWRITEBLKFAIL4, 0, 1 );
		goto errout;
		}

	/*
	 * update superblock
	 */
	if (!(sbp->s_state & (FM_DIRTY|FM_LOGREDO)))
		sbp->s_state = FM_CLEAN;
	else
		sbp->s_state &= ~FM_EXTENDFS;
	sbp->s_size = sbp->s_xsize;
	sbp->s_agsize = agsize;
	sbp->s_fsckpxd = sbp->s_xfsckpxd;
	sbp->s_fscklog = 0;
	sbp->s_logpxd = sbp->s_xlogpxd;
	sbp->s_logserial = 1;

	if (rc = rdwrSuper(fd, sbp, PB_UPDATE))  {
		fsck_send_msg( lrdo_EXTFSWRITEFSSUPERFAIL, 0, 0 );
		goto errout;
		}

	/*
	 * finalize log
	 *
	 * note: new log is valid;
	 */
	/* read log superblock */
	t64 = (addressPXD(&sbp->s_logpxd) << sbp->s_l2bsize) + LOGPSIZE;
	if (rc = ujfs_rw_diskblocks(fd, t64, LOGPSIZE, &logsup, GET))  {
		fsck_send_msg( lrdo_EXTFSREADLOGSUPFAIL, 0, 0 );
		goto errout;
		}

	logsup.end = findEndOfLog();
	logsup.state = LOGREDONE;

	if (rc = ujfs_rw_diskblocks(fd, t64, LOGPSIZE, &logsup, PUT))  {
		fsck_send_msg( lrdo_EXTFSWRITELOGSUPFAIL, 0, 0 );
		goto errout;
		}

           fsck_send_msg( lrdo_REXTNDTOPOST, 0, 0 );

	return 0;

errout:
           sprintf( message_parm_0, "%ld", errno );
           msgprms[0] = message_parm_0;
           msgprmidx[0] = 0;
           fsck_send_msg( lrdo_REXTNDFAIL, 0, 1 );
	return(EXTENDFS_FAILRECOV);
}


/*
 *
 * NAME:        alloc_dmap_bitrec 
 *
 * FUNCTION:    This routine allocates memory by calling the chkdsk
 *		alloc_wrksp() routine (because that will allocate high
 *		memory during autocheck).  If that fails then logredo 
 *                   cannot continue bmap processing, so it will set a flag
 *                   and make the storage aleady allocated to the bmap
 *                   available for other uses. 
 *		was successfully allocated and there's enough of it left, 
 *		this routine will return a piece of it.
 */
int32 alloc_dmap_bitrec( dmap_bitmaps_t **dmap_bitrec )
{
    int32 adb_rc = 0;
    int32 intermed_rc = 0;
    
    *dmap_bitrec = NULL;

    intermed_rc = alloc_wrksp( (uint32) (sizeof(dmap_bitmaps_t)), 
                                 0,		/* not meaningful from logredo */
                                 -1,                 /* I am logredo */
                                dmap_bitrec
                                );

    if( (intermed_rc != 0) || ((*dmap_bitrec) == NULL) ) {

        Insuff_memory_for_maps = -1;
        available_stg_addr = bmap_stg_addr;
        available_stg_bytes = bmap_stg_bytes;
            /*
             * initialize the storage for its new use
             */
        memset( (void *) available_stg_addr, 0, available_stg_bytes );
        }  

    return( adb_rc );
}                                            /* end alloc_dmap_bitrec() */


/*
 *
 * NAME:        alloc_storage
 *
 * FUNCTION:    This routine allocates memory by calling the chkdsk
 *		alloc_wrksp() routine (because that will allocate high
 *		memory during autocheck).  If that fails and the bmap
 *		was successfully allocated and there's enough of it left, 
 *		this routine will return a piece of it.
 */
int32 alloc_storage(	int32	size_in_bytes,
			void	**addr_stg_ptr,
			int32	*bmap_stg_returned
			)
{
    int32 as_rc = 0;
    int32 intermed_rc = 0;
    
    *bmap_stg_returned = 0;	/* assume we'll get it the usual way */
    *addr_stg_ptr = NULL;

    intermed_rc = alloc_wrksp( (uint32) size_in_bytes, 
                                 0,		/* not meaningful from logredo */
                                 -1,                 /* I am logredo */
                                addr_stg_ptr
                                );

    if( (intermed_rc != 0) || ((*addr_stg_ptr) == NULL) ) {

        if( (!Insuff_memory_for_maps) && (bmap_stg_addr != NULL) ) {  /*
				* we did allocate storage for the bmap
				* and haven't started cannibalizing it yet
				*/
            Insuff_memory_for_maps = -1;
            available_stg_addr = bmap_stg_addr;
            available_stg_bytes = bmap_stg_bytes;
	        /*
	         * initialize the storage for its new use
	         */
            memset( (void *) available_stg_addr, 0, available_stg_bytes );
            }  /* end we did allocate storage for the bmap... */

        if( Insuff_memory_for_maps & (available_stg_bytes!=0) ) {  /*
				* we may be able to go on anyway
				*/
            if( available_stg_bytes < size_in_bytes ) {  /* 
				* not enough here
				*/
                return  (ENOMEM0);
                }  /* end not enough here */
            else {  /* we can scavenge the memory we need */
                *addr_stg_ptr = available_stg_addr;
                available_stg_bytes -= size_in_bytes;
                available_stg_addr = (char *) 	( 
				((int32)available_stg_addr) + size_in_bytes 
						);
                *bmap_stg_returned = -1;	
                }  /* end else we can scavenge the memory we need */
            }  /* end we may be able to go on anyway */
        else {  /* there's no going on from here */
            return  (ENOMEM1);
            }  /* end else there's no going on from here */
        }

    return( as_rc );
}                                            /* end alloc_storage() */


#ifdef  _JFS_WIP
/*
 *      nfsisloaded()
 *
 * check whether nfs is loaded
 */
static int
nfsisloaded()
{
        int sav_errno;
        int (*entry)();
        if (entry = load("/usr/sbin/probe", 0, 0))
                return (1);
        if (errno == ENOEXEC) {
                DBG_TRACE(("%s: nfs is not loaded\n", prog))
                return (0);
        }
        sav_errno = errno;
        DBG_TRACE(( "%s: ", prog))
        errno = sav_errno;
        perror("load");
        return (0);
}
#endif  /* _JFS_WIP */

#ifdef _JFS_DEBUG
/*
 *      xdump()
 *
 * hex dump
 */
xdump (
char *saddr,
int count)
{
#define LINESZ     60
#define ASCIISTRT    40
#define HEXEND       36
    int i, j, k, hexdigit;
    register int c;
    char *hexchar;
    char linebuf[LINESZ+1];
    char prevbuf[LINESZ+1];
    char *linestart;
    int asciistart;
    char asterisk = ' ';
    void x_scpy ();
    int x_scmp ();
    hexchar = "0123456789ABCDEF";
    prevbuf[0] = '\0';
    i = (int) saddr % 4;
    if (i != 0)
        saddr = saddr - i;
    for (i = 0; i < count;) {
        for (j = 0; j < LINESZ; j++)
            linebuf[j] = ' ';
        linestart = saddr;
        asciistart = ASCIISTRT;
        for (j = 0; j < HEXEND;) {
            for (k = 0; k < 4; k++) {
                c = *(saddr++) & 0xFF;
                if ((c >= 0x20) && (c <= 0x7e))
                    linebuf[asciistart++] = (char) c;
                else
                    linebuf[asciistart++] = '.';
                hexdigit = c >> 4;
                linebuf[j++] = hexchar[hexdigit];
                hexdigit = c & 0x0f;
                linebuf[j++] = hexchar[hexdigit];
                i++;
            }
            if (i >= count)
                break;
            linebuf[j++] = ' ';
        }
        linebuf[LINESZ] = '\0';
        if (((j = x_scmp (linebuf, prevbuf)) == 0) && (i < count)) {
            if (asterisk == ' ') {
                asterisk = '*';
                DBG_TRACE(("    *\n"))
            }
        }
        else {
            DBG_TRACE(("    %x  %s\n",linestart, linebuf))
            asterisk = ' ';
            x_scpy (prevbuf, linebuf);
        }
    }
    return (0);
}

int x_scmp(
register char *s1,
register char* s2)
{
    while ((*s1) && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    if (*s1 || *s2)
        return(-1);
    else
        return(0);
}

void x_scpy(
register char *s1,
register char *s2)
{
    while ((*s1 = *s2) != '\0') {
        s1++;
        s2++;
    }
}

prtdesc(struct lrd * ld)
{        switch ( ld->log.redopage.type)
        {
        case LOG_XTREE:
                DBG_TRACE((" REDOPAGE:XTREE\n  "))
                break;
        case (LOG_XTREE | LOG_NEW):
                DBG_TRACE((" REDOPAGE:XTREE_NEW\n  "))
                break;
        case (LOG_BTROOT | LOG_XTREE):
                DBG_TRACE((" REDOPAGE:BTROOT_XTREE\n  "))
                break;
        case LOG_DTREE:
                DBG_TRACE((" REDOPAGE:DTREE\n  "))
                break;
        case (LOG_DTREE | LOG_NEW):
                DBG_TRACE((" REDOPAGE:DTREE_NEW \n "))
                break;
        case (LOG_DTREE | LOG_EXTEND):
                DBG_TRACE((" REDOPAGE:DTREE_EXTEND\n  "))
                break;
        case (LOG_BTROOT | LOG_DTREE):
                DBG_TRACE((" REDOPAGE:BTROOT_DTREE\n  "))
                break;
        case (LOG_BTROOT | LOG_DTREE | LOG_NEW):
                DBG_TRACE((" REDOPAGE:BTROOT_DTREE.NEW\n  "))
                break;
        case LOG_INODE :
                /*
                 * logredo() updates imap for alloc of inode.
                 */
                DBG_TRACE((" REDOPAGE:INODE\n  "))
                break;
        case LOG_EA:
                DBG_TRACE((" REDOPAGE:EA\n  "))
                break;
        case LOG_DATA:
                DBG_TRACE((" REDOPAGE:DATA\n  "))
                break;
        }
        return (0);
}
#endif /* _JFS_DEBUG */
