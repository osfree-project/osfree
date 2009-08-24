/* $Id: log_work.c,v 1.3 2004/03/21 02:43:17 pasha Exp $ */

static char *SCCSID = "@(#)1.20  9/7/99 07:53:45 src/jfs/utils/libfs/log_work.c, jfslib, w45.fs32, currbld";
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
 *   MODULE_NAME:		log_work.c
 *
 *   COMPONENT_NAME: 	jfslib
 *
 *   FUNCTIONS:  log_work.c: recovery manager
 *
 *              deleteCommit
 *              doAfter
 *              doCommit
 *              doExtDtPg
 *              doNoRedoFile
 *              doNoRedoInoExt
 *              doNoRedoPage
 *              doUpdateMap
 *              dtpg_resetFreeList
 *              dtrt_resetFreeList
 *              findCommit
 *              findPageRedo
 *              logredoInit
 *              markBmap
 *              markImap
 *              saveExtDtPg
 *              updatePage
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
  *    R E M E M B E R    M E M O R Y    A L L O C    F A I L U R E
  *
  */
extern int32  Insuff_memory_for_maps;
extern char  *available_stg_addr;
extern int32  available_stg_bytes;
extern char  *bmap_stg_addr;
extern int32  bmap_stg_bytes;

//PS21032004 extern int  end_of_transaction;                                              // @D2


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *   L O C A L   M A C R O    D E F I N I T I O N S
  *
  */
#define UZBIT_8         ((uint8) (1 << 7))
#define UZBIT_16        ((uint16) (1 << 15 ))
#define UZBIT_32        ((uint32) (1 << 31 ))

#define DTPGWORD        32
#define L2DTPGWORD      5

/* convert disk block number to bmap file page number */
#define BLKTODMAPN(b)\
        (((b) >> 13) + ((b) >> 23) + ((b) >> 33) + 3 + 1)

/* The following MACRO update bmap for extents that have
 * XAD_NEW | XAD_EXTENDED flag in Xadlist. Then reset
 * the XAD_NEW | XAD_EXTENDED bits in the flag
 */
#define  MARKXADNEW(PXD, XADP, BMAPT, VOL)\
{\
        PXDlength((&PXD), (XADP->len));\
        PXDaddress((&PXD), addressXAD((XADP)));\
        markBmap((dmap_t*)(BMAPT), (PXD), 1, (VOL));\
        (XADP)->flag &= ~(XAD_NEW|XAD_EXTENDED);\
}

/*
 *
 *   T R A N S A C T I O N   H A S H   T A B L E
 *
 * each entry represents a committed transaction in recovery-in-progress
 *
 * An entry stays in the commit table until the first (in time, last seen
 * by logredo) record of a transaction is seen.  (It is recognized because
 * its backchain == 0)
 */
#define COMSIZE   512
int comfree;                    /* index of a free com structure */
int comhmask = 63;              /* hash mask for comhash */
int comhash[64];                  /* head of hash chains */
struct com
{
        int tid;                /* 4: committed tid.  */
        int next;               /* 4: next on free list or hash chain */
} com[COMSIZE];                 /* (8) */

/*
 *
 *   R E D O   P A G E   H A S H   T A B L E
 *
 * each entry represents a disk page that have been updated
 * by LOG_REDOPAGE records.
 *
 * When replay the LOG_REDOPAGE log rec, we should track which portion of the
 * disk page has been updated by the LOG_REDOPAGE log records.
 * When log is read in backwards direction, a LOG_REDOPAGE log record is
 * only applied to disk page image that has not been updated by any earlier
 * log records. So that only the last update to a portion is applied.
 *
 * There are many types of data for LOG_REDOPAGE log rec. Fields
 * ino_base, ino_ea, ino_data, summary1 and summary2 use different
 * way to do the track according to log.redopage.type:
 *
 *  1) Xtree page -- since the log record carries at most 2 segments,
 *                   the first one is header, if any, the second one is the
 *                   updated contigous xtree entries starting from offset.
 *                   The number of entries specified in the length.
 *                   If there is only one segment, it must be header.
 *
 *                   For xtree, the new entry either inserts or appends.
 *                   If it appends, then the log rec contains only the
 *                   new entry data. If it is inserted, all the entries
 *                   after the insertion need to shift. Then the log rec
 *                   contains all entries starting from the new insertion.
 *                   So only low water mark (lwm) offset needs to be recorded
 *                   in summary2. At logredo time, only the log rec having
 *                   offset lower than lwm will apply to page and only apply
 *                   the difference between offset and lwm.
 *                   Header of the xtree page should be only applied once
 *                  in logredo. So another field in summzry2 track header.
 *  2) Dtree page -- A dtree page has 128 slots including header. the slot size
 *                   is 32 byte. A 4 words(32 byte) array is used as  a bit
 *                   vector to track the 128 slots.
 *  3) inode page -- There are 3 slot sizes and 5 sub-types for inode page.
 *                    Each type is in a separate log record.
 *
 *      a) type == INODE
 *         Section I (128 byte) of the dinode is logged. Offset is
 *         counted from the beginning of the inode page.
 *         A 8-bit vector tracks 8 inodes in inode page.
 *         the slot size for this type is 128 byte.
 *         Note: Each inode has 512 bytes, the INODE type only refers
 *               to the first 128 section.  so the offset should
 *               be always a multiply of 4, i.e. 0, 4, 8, 12, etc..
 *      b) type == EA  ( extended attribute )
 *         it is the top 16 bytes of section II of the dinode.
 *         offset should be always ???.
 *         A 8-bit vector tracks 8 inodes in inode page.
 *         the slot size is 16 bytes.
 *      c) type ==  DATA ( in-line data for symlink )
 *         A real xt data file starts from the 16 bytes above
 *         section III of the dinode.
 *         offset should be always ???.
 *         A 8-bit vector tracks 8 inodes in inode page.
 *         the slot size is 16 bytes.
 *      d) type == BTROOT + DTREE
 *         It starts from the 32 bytes above section III of the dinode.
 *         Offset is counted from the beginning of BTROOT.
 *         An array of 8 uint16, each is used as a bit vector to track
 *         one dtroot.
 *         the slot size for this type is 32 byte.
 *      e) type == BTROOT + XTREE
 *         It starts from the 32 bytes above section III of the dinode.
 *         Offset is counted from the beginning of BTROOT.
 *         an array of 8 structrues, each tracks one xtroot for lwm and
 *         header of xtroot.
 *         the slot size for this type is 16 byte.
 *
 *  Note1: The slot size is stored in lrd.redopage.l2linesize field.
 *
 *  Note2: The hash key for doblk is aggregate+fileset+pxd.
 *         The same pxd can be used for an INODE log record,
 *         a BTROOT+DTREE log rec,  a BTROOT+XTREE log rec,
 *         a EA log rec, and a in-line DATA log rec. So for these
 *         five types, we cannot overlay them each other.
 *         But the same pxd can be used for either a BTROOT+DTREE log rec
 *         or a DTREE page log rec, not both. The same pxd can be used
 *         for either a BTROOT+XTREE log rec or a XTREE page log rec,
 *         not both.
 *  Note3: xtpage_lwm and i_xtroot[] have a different initialization value
 *         from rest of the types. They have to be inited to the
 *         highest value.
 */
#define BHASHSIZE 1024          /* must be a power of two       */
struct doblk
{
        int32 aggregate;        /* 4: file system aggregate/lv number */
        int32 fileset;          /* 4: fileset number.   */
        pxd_t  pxd;             /* 8: on-disk page pxd */
        uint8  ino_base;        /* 1: each bit tracks one dinode for section I
                                   of the dinode (128 bytes.) Each inode has 4
                                   128-byte slots, with one base slot.
                                   A total of 8 bits that need to be marked.
                                   for 8 dinodes in one inode page  */
        uint8  ino_ea;          /* 1: extended attribute of the dinode, 16 byte*/
        uint8  ino_data;        /* 1: in-line data, at most (256 + 16) byte for
                                   each dinode. */
        uint8  reserved;        /* 1: */
        union {
           uint16  ino_dtroot[8]; /* 16: dtree root. each dinode has 9
                                   dtree-root slots, including 1 slot header.
                                   each slot is 32-byte.
                                   Each element of i_dtroot[] monitors
                                   one dtree root of the dinode.
                                   For each 16-bit, only 9-bit is used.*/
           uint32  dtpage_word[4]; /* 16:dtree page. a total of 128 slots
                                      including header    */
        } summary1;             /* 16    */

        union {
           struct {
                uint8  xtrt_hd;   /* 1: xtroot header        */
                uint8  xtrt_lwm;  /* 1: xtroot lwm value.   */
           }i_xtroot[8];          /* 16: xtree root. each dinode has 10
                                   xtree-root slots, including 2 heade slots.
                                   each slot is 16-byte.
                                   Each element of i_xtroot[] monitors
                                   one xtree root of the dinode.  */
           struct {
                uint8  xtpage_hd;  /* 1: xtree page header         */
                uint8  xtpage_lwm; /* 1: xtree page. the lowest offset among
                                      non-header segments */
           } xtpg;
        } summary2;             /* 16    */

        struct doblk *next;     /* 4: next entry on hash chain  */
};                              /* (56) */

#define  db_ibase       ino_base
#define  db_iea         ino_ea
#define  db_idata       ino_data
#define  db_dtroot      summary1.ino_dtroot
#define  db_dtpagewd    summary1.dtpage_word
#define  db_xtpagelwm   summary2.xtpg.xtpage_lwm
#define  db_xtpghd      summary2.xtpg.xtpage_hd
#define  db_xtroot      summary2.i_xtroot

extern int32 numdoblk;      /* number of do blocks used     */
int32 blkhmask = (BHASHSIZE-1); /* hash mask for blkhash */
struct doblk *blkhash[BHASHSIZE]; /* head of doblk hash chains */
int32 Freedoblk;                /* number of unused doblk struct */
struct doblk  *Blkpage;      /* the beginning address of doblk hash table page */

/*
 *
 *   N O R E D O   F I L E   H A S H   T A B L E
 *
 * each entry represents a file system object which has been deleted 
 * (entry is added when the log record describing the delete is processed)
 */

#define NODOFILEHASHSIZE   512
struct nodofile
{
        int32 aggregate;        /* 4: file system aggregate/lv number */
        int32 fileset;             /* 4: fileset number this file is belonged */
        ino_t  inode;             /* 4: inode number                      */
        struct nodofile *next;  /* 4: next entry on nodo hash chain*/
};                                   /* (16) */
extern int32 numnodofile;               /* number of nodo file blocks used      */
int32 nodofilehmask = (NODOFILEHASHSIZE -1);  /* hash mask for nodohash  */
struct nodofile *nodofilehash[NODOFILEHASHSIZE]; /* head of nodo hash chains */
int32  Freenodofile;                    /* number of unused nodofile struct */
struct nodofile *Nodofilep;             /* the beginning address of nodo
                                           hash table page   */

struct  ExtDtPg
{
        int32 pg_vol;              /* 4: volume containing the dtpage */
        int64 pg_off;              /* 8: dtpage offset, in fsblocks, in the aggregate */
        struct ExtDtPg *next;  /* 4: next entry on list */
};                                      /* (16) */
extern int32 numExtDtPg;        /* number of extended dtpage blocks used  */
int32 FreeExtDtPg = 0;	           /* number of unused extended dtpage blocks */
struct ExtDtPg *DtPgPage = NULL;  /* storage available for new blocks */
struct ExtDtPg *DtPgList = NULL;  /*  list of extended dtpages */


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *    S T U F F    F O R    T H E    L O G 
  *
  *       externals defined in logredo.c
  */

/*
 *
 *   O P E N   F I L E   S Y S T E M   A G G R E G A T E / L V  A R R A Y
 *
 *     Defined in logredo.c
 */
extern struct vopen vopen[];              /* (88) */

/*
 *
 *   B U F F E R   H E A D E R   T A B L E
 *
 */
extern struct bufhdr
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
} bufhdr[];                             /* (24) */

/*
 *
 *   L O G   P A G E   B U F F E R   C A C H E
 *
 * log has its own 4 page buffer pool.
 * --> afterdata defined in logredo.c <--
 */
extern uint8    afterdata[LOGPSIZE];   /* buffer to read in redopage data */

extern int16    loglocation;       /* 1 = inlinelog, 2 = outlinelog */
extern int32    logminor;          /* minor number of log device */
extern logsuper_t logsup;          /* log super block */


/*
 *
 *   E X T E R N A L   A N D   F O R W A R D   R E F E R E N C E S
 *
 */
/*
 * external references
 */
extern int bread( int32,  pxd_t, void  **, int32 );
extern int fsError( int, int, int64 );
extern int32 openVol( int32 );
extern int32 alloc_storage( int32, void **, int32 * );
extern int32 alloc_dmap_bitrec( dmap_bitmaps_t ** );
extern int dMapGet( int, int );
extern int iagGet( int, int32 );

extern void fsck_send_msg( int, int, int );		/* defined in fsckmsg.c */

/*
 * forward references
 */
int deleteCommit( int32 );
int doAfter( struct lrd *, int32 );
int doCommit( struct lrd * );
int doExtDtPg( void );
int doNoRedoFile( struct lrd *,  ino_t );
int doNoRedoInoExt( struct lrd * );
int doNoRedoPage( struct lrd * );
int doUpdateMap( struct lrd * );
int dtpg_resetFreeList( int32, int * );
int dtrt_resetFreeList( int32, struct doblk *, struct lrd *, caddr_t );
int findCommit( int32 );
int findPageRedo( int32, int32,  pxd_t, struct doblk ** );
int32 logredoInit( void );
int markBmap( dmap_t *, pxd_t, int, int );
int markImap( struct fsimap_lst, ino_t, pxd_t, int, int );
int updatePage( struct lrd *, int32 );
int saveExtDtPg( int32, int64 );

/*
   =================================================================== 
*/
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        deleteCommit(tid)
 *
 * FUNCTION:    Search in the commit array for a commit record with 
 *              transaction id (tid) matching the given tid.  If a
 *              match is found, delete the array entry containing it.
 */
deleteCommit(
int32 tid)  /* transaction id to be deleted */
{
        int k,n,hash;

        hash = tid & comhmask;          /* hash class */
        n = 0;                          /* previous entry on hash chain  */

        for (k = comhash[hash]; com[k].tid != tid ; k = com[k].next)
                n = k;

        /* remove k from hash chain and put it on free list
         * Special case when 1st on the hash list
         */
        if ( n == 0)
                comhash[hash] = com[k].next;
        else
                com[n].next = com[k].next;

        com[k].next = comfree;
        comfree = k;

	/* note that the end of the transaction has been seen so
	 * that the log records it contains will be written to the
	 * device.
	 */
//PS21032004        end_of_transaction = -1;                                 // @D2

        return(0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        doAfter(ld)
 *
 * FUNCTION:    processing for LOG_REDOPAGE  record types.
 *
 *           IN GENERAL
 *           ----------
 *              The redopage log rec has many sub-types defined in
 *              ld->log.redopage.type, but in general, the data belonging
 *              to the REDOPAGE log record is applied to the disk pages
 *              described by the address in ld->log.redopage.pxd.
 *
 *              Also, depending on the redopage.type, 
 *                - the block map (bmap) may be updated
 *                - the applicable inode map (imap) may be updated
 *                - a NoRedoFile filter may be established
 *                - a NoRedoPage filter may be established
 *
 *           IN PARTICULAR
 *           -------------
 *              LOG_BTROOT | LOG_DTREE | LOG_NEW -- describes a dtree root
 *                                        which has been reset
 *
 *                 The log record data is applied to dtroot slots.  The
 *                 dtree root's freelist needs to be initialized.  The
 *                 LOG_NEW flag indicates that a dtree root which was a
 *                 leaf is now an internal node.  
 *                 Specifically:
 *                     - the dtroot was full and was copied into a new
 *                       dtree (non-root) node.  
 *                     - after the log record data is applied to this
 *                       dtree root, only 1 slot will be in use -- it
 *                       will point to the new dtree node.
 *
 *                 One log record of this type describes a single dtree
 *                 root.
 *
 *              LOG_BTROOT | LOG_DTREE --describes an updated dtree root
 *
 *                 The log record data is applied to dtroot slots.  This
 *                 covers the case of dtree root initialization, since 
 *                 dtInitRoot() writes all slots (including free slots)
 *                 for a dtree root node into the log record.
 *
 *                 One log record of this type describes a single dtree
 *                 root.
 *
 *              LOG_DTREE -- describes an updated dtree (non-root) node
 *
 *                 The log record data is applied to the storage defined
 *                 by the pxd.
 *
 *                 One log record of this type describes a single dtree
 *                 root.
 *
 *              LOG_DTREE | LOG_NEW --  describes a new dtree (non-root) node
 *
 *                 The log record data is applied to the storage defined
 *                 by the pxd.  The node's freelist is initialized.  The
 *                 block map is updated for the storage allocated for the
 *                 new node.
 *
 *                 One log record of this type describes a single dtree
 *                 (non-root) node.
 *
 *              LOG_DTREE | LOG_EXTEND -- describes a (non-root) dtree node
 *                                        which has been extended.  
 *
 *                 The log record data is applied to the storage defined
 *                 by the pxd.  The node's freelist is initialized.  The
 *                 block map is updated for the storage allocated for the
 *                 extended node.
 *
 *                 One log record of this type describes a single dtree
 *                 (non-root) node.
 *
 *              LOG_BTROOT | LOG_XTREE -- describes an updated xtree root
 *
 *                 The log record data is applied to the xtree root's 
 *                 slots.  The block map is updated to show blocks defined
 *                 by the pxd's in updated slots are now in use.
 *
 *                 One log record of this type describes a single xtree
 *                 root.
 *
 *              LOG_XTREE | LOG_NEW -- describes a new xtree (non-root) node
 *
 *                 The log record data is applied to the xtree node's 
 *                 slots.  The block map is updated to show blocks defined
 *                 by the pxd's in updated slots are now in use.  The block
 *                 map is also updated to show that the blocks occupied by
 *                 the xtree node itself are now in use.
 *
 *                 One log record of this type describes a single xtree
 *                 (non-root) node.
 *
 *              LOG_XTREE -- describes an xtree (non-root) node which
 *                           which has been changed.
 *
 *                 The log record data is applied to the xtree node's 
 *                 slots.  The block map is updated to show blocks defined
 *                 by the pxd's in updated slots are now in use.  
 *                 (Note that when a file is truncated, the slots describing
 *                 the now deleted blocks are not updated.  File truncation
 *                 affects the slot containing the xtree header nextindex
 *                 field.)
 *
 *                 One log record of this type describes a single xtree
 *                 (non-root) node.
 *
 *              LOG_INODE -- describes one (4096 byte) page of an inode 
 *                           extent (that is, a page actually containing
 *                           inodes) which has been updated. 
 *
 *                 The log record data is applied to the inode extent page,
 *                 updating from 1 to all of the inodes contained in the 
 *                 page.  (Note that this may include updates to inline 
 *                 EA data.)
 *
 *                 ** for this log record, log.redopage.inode contains the
 *                    inode number of the inode map which owns the page.
 *                    That is, not the inode number of any inode being 
 *                    changed by the contents of this log record.
 *                    The inode number of each inode being changed by this
 *                    log record can be found in the inode's after image 
 *                    in the log record data.
 *
 *                 For each inode affected by the log record:
 *                   -  If the portion of the inode affected by this
 *                      log data has not been updated by an earlier
 *                      log record in this session, the log data is 
 *                      copied over the appropriate portion of the 
 *                      inode.
 *                   -  If the nlink count == 0, 
 *                        o the inode map (imap) is updated to show the 
 *                          inode is free
 *                        o a NoRedoFile filter is started for the inode
 *                   -  Otherwise (nlink != 0),
 *                        o the inode map (imap) is updated to show the inode
 *                          is allocated.
 *
 *                 If at least 1 inode affected by the current log record
 *                 has nlink != 0, the block map is updated to show that 
 *                 the block(s) containing the inode extent (of which this
 *                 page is a part) are allocated.
 *
 *                 (It is obvious that from this log record we don't have 
 *                 enough information to consider marking the blocks 
 *                 containing the inode extent free.)
 *
 *                 One log record of this type describes from 1 to all 8
 *                 inodes contained in the page. 
 *
 *              LOG_DATA --  ** may be used in a future release **
 *
 *                 This is for in-line data (i.e. symlink.)
 *                 (TBD)
 *
 */
doAfter(
struct lrd * ld,        /* pointer to log record descriptor */
int32 logaddr)
{
        int vol, rc = 0;
        int32  hash;
        struct nodofile *nodoptr;

        /* 
         * If it's not part of a committed transaction then it
         * should be ignored, so just return.
         */
        if (!findCommit(ld->logtid))
                return(0);

        /* 
         * if it's the last entry for the current committed transaction,
         * remove the commit record from the commit list because we won't
         * be needing it any more.
         */
        if (ld->backchain == 0)
                deleteCommit(ld->logtid);


        /* 
         * if the filesystem was cleanly unmounted or if the last 
         * thing that happened was a logredo failure, skip  this
         * record.  (Necessary for the case of logredo a log shared
         * by multiple filesystems.  We want to process log records
         * for those filesystems which don't meet this criteria, but
         * skip log records for those which do.)
         */
        vol = 0;
        if (vopen[vol].status == FM_CLEAN || vopen[vol].status == FM_LOGREDO)
                return (0);

        /* 
         * If the redopage.type != LOG_INODE, then if there is a
         * NoRedoFile filter in effect for the inode we can skip this
         * log record.
         *
         * N.B. When redopage.type == LOG_INODE, log.redopage.inode is 
         *      the owning imap's inode number.  We'll check for NoRedoFile 
         *      filter(s) on the inode(s) actually affected by the log 
         *      record when we examine the log record data in updatePage().
         */
        if ( !(ld->log.redopage.type & LOG_INODE))
        {
           hash = (ld->aggregate + ld->log.redopage.fileset +
                        ld->log.redopage.inode ) & nodofilehmask;
           for (nodoptr = nodofilehash[hash]; nodoptr != NULL;
                                        nodoptr = nodoptr->next)
           {
                if (ld->aggregate  == nodoptr->aggregate &&
                        ld->log.redopage.fileset == nodoptr->fileset &&
                    ld->log.redopage.inode == nodoptr->inode )
                        return(0);
           }
        }
        /*
         * updatePage() takes care of applying the log data.
         * This includes:
         *    - applying log data to the affected page
         *    - updates to the inode map for inodes allocated/free
         *    - updates to the block map for an allocated inode extent
         *    - establishing NoRedoFile or NoRedoExtent filters
         *    - updates to the block map for extents described in an
         *      xtree root or node xadlist
         */
        if ((rc = updatePage(ld, logaddr)) !=0)
        {
                sprintf( message_parm_0, "0x0%llx", logaddr  );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                sprintf( message_parm_1, "(d) %d", rc  );
                msgprms[1] = message_parm_1;
                msgprmidx[1] = 0;
                fsck_send_msg( lrdo_DAFTUPDPGFAILED, 0, 2 );
                return(rc);
        }

        /* 
         * If this isn't a REDOPAGE log record, we're done
         */
        if (ld->type != LOG_REDOPAGE)
                return (0);

        /*
         * update the block map for a new or extended dtree page and
         * for a new xtree page
         */
        switch ( ld->log.redopage.type)
        {
        case (LOG_DTREE | LOG_NEW):
          /* 
           * the pxd describes the (non-root) dtree page 
           */
        case (LOG_DTREE | LOG_EXTEND):
          /* 
           * The pxd describes the entire (non-root) dtree page, not
           * just the new extension, but it's always ok to mark allocated
           * something which is already marked that way.
           *
           * (And we don't really know how many of the trailing blocks
           * were newly added here anyway.)
           */
        case ( LOG_XTREE | LOG_NEW ):
          /* 
           * the pxd describes the (non-root) xtree page (always 4096 bytes)
           */
                rc = markBmap((dmap_t *)vopen[vol].bmap_ctl,
                                        ld->log.redopage.pxd, 1, vol);
                if (rc)  {
                        sprintf( message_parm_0, "0x0%llx", logaddr  );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        sprintf( message_parm_1, "(d) %d", rc  );
                        msgprms[1] = message_parm_1;
                        msgprmidx[1] = 0;
                        fsck_send_msg( lrdo_DAFTMRKBMPFAILED, 0, 2 );
                        }
                break;
        }

        return (rc);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        doCommit(ld)
 *
 * FUNCTION:    Insert the transaction ID (tid) from the given commit 
 *              record into the commit array
 */
doCommit(
struct lrd * ld)  /* pointer to record descriptor */
{
        int  k,hash;

           DBG_TRACE(("logredo:Docommit\n"))
        if (comfree == 0)
                return(JLOG_NOCOMFREE);

        k = comfree;
        comfree = com[k].next;

        hash = ld->logtid & comhmask;
        com[k].next = comhash[hash];
        com[k].tid = ld->logtid;
        comhash[hash] = k;

        return(0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        doExtDtPg()
 *
 * FUNCTION:    Rebuild the freelist for each dtpage which has been
 *                   extended in the current logredo session.
 *
 */
int doExtDtPg(  )
{
    struct ExtDtPg *edpp;
    int dedp_rc = 0;
    int *buf;
    pxd_t a_pxd;

    while( (DtPgList != NULL) && (dedp_rc == 0) ) {
        edpp = DtPgList;
        PXDaddress( &a_pxd, edpp->pg_off );
        PXDlength( &a_pxd, vopen[edpp->pg_vol].lbperpage );

        dedp_rc = bread( edpp->pg_vol, a_pxd, (void **)&buf, PB_UPDATE );
        if( dedp_rc )  {
            sprintf( message_parm_0, "0x0%llx", edpp->pg_off  );
            msgprms[0] = message_parm_0;
            msgprmidx[0] = 0;
            sprintf( message_parm_1, "(d) %d", dedp_rc  );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            fsck_send_msg( lrdo_DEDPBREADFAILED, 0, 2 );
            dedp_rc = DTPAGE_READERROR1;
            }
        else {
            dedp_rc = dtpg_resetFreeList( edpp->pg_vol, buf );
            DtPgList = DtPgList->next;
            }
        }  /* end while */

    return( dedp_rc );
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        doNoRedoFile()
 *
 * FUNCTION:    Add a record for the specified inode to the 
 *              NoRedoFile array.
 */
doNoRedoFile(
struct lrd * ld,  /* pointer to record descriptor */
ino_t inum)       /* the inode number for noredofile */
{
        int rc = 0;
        struct nodofile *ndptr;
        int32  hash;
        int32 allocated_from_bmap = 0;

        /* 
         * If it's not part of a committed transaction then it
         * should be ignored, so just return.
         */
        if (!findCommit(ld->logtid))
                return(0);

        /* 
         * if it's the last entry for the current committed transaction,
         * remove the commit record from the commit list because we won't
         * be needing it any more.
         */
        if (ld->backchain == 0)
                deleteCommit(ld->logtid);

        /*
         * start NoRedoFile filter for the specified inode by
         * adding a record to the NoRedoFile hash list.
         *
         */
        hash = (ld->aggregate + ld->log.redopage.fileset
                                + inum) & nodofilehmask;
        if ( Freenodofile == 0 )
        {
#ifdef _JFS_DEBUG
printf("logredo:alloc (d)%d bytes for NoRedoFile filter\n", PSIZE);
#endif
           sprintf( message_parm_0, "(d) %d", PSIZE );
           msgprms[0] = message_parm_0;
           msgprmidx[0] = 0;
           fsck_send_msg( lrdo_ALLOC4NOREDOFL, 0, 1 );

           rc = alloc_storage( 	(uint32) PSIZE, 
				(void **) &Nodofilep,
				&allocated_from_bmap 
				);
           if( (rc != 0) || (Nodofilep == NULL) ) {  /* 
				* NoRedoFile filter allocation failed 
				*/
#ifdef _JFS_DEBUG
printf("logredo:alloc (d)%lld bytes for NoRedoFile filter failed\n",PSIZE);
/* DosSleep(10000); */
#endif
                      sprintf( message_parm_0, "(d) %lld", PSIZE );
                      msgprms[0] = message_parm_0;
                      msgprmidx[0] = 0;
                      fsck_send_msg( lrdo_ALLOC4NOREDOFLFAIL, 0, 1 );
                      return(ENOMEM3);
                      }  /* end NoRedoFile filter allocation failed */
           else if( Nodofilep != NULL ) {  /* NoRedoFile filter allocation successful */
                      if( allocated_from_bmap ) {
#ifdef _JFS_DEBUG
printf("logredo:alloc (d)%lld bytes for NoRedoFile filter out of bmap allocation\n",PSIZE);
/* DosSleep(10000); */
#endif
                                 fsck_send_msg( lrdo_USINGBMAPALLOC4NRFL, 0, 0 );
                                 }
                      }  /* end NoRedoFile filter allocation successful */

           Freenodofile = PSIZE / sizeof(struct nodofile);
           }

        ndptr = Nodofilep++;

        numnodofile++;
        Freenodofile--;

        ndptr->next = nodofilehash[hash];
        nodofilehash[hash] = ndptr;
        ndptr->aggregate = ld->aggregate;
        ndptr->fileset = ld->log.redopage.fileset;
        ndptr->inode = inum;

        return (0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        doNoRedoPage()
 *
 * FUNCTION:    Processing for LOG_NOREDOPAGE rec type.
 *          
 *              This routine starts a NoRedoPage filter for the
 *              specified xtree or dtree node (may be root).
 *
 *              This routine updates the bmap when the freed node
 *              is NOT the root node.
 *              (ld->log.noredopage.pxd = old page extent)
 *
 * NOTE:        This routine only updates the block map when the
 *              extent being freed describes a dtree page.
 *              Block map updates for a freed xtree page are handled
 *              when the UPDATEMAP log record is seen.
 *
 */
doNoRedoPage(
struct lrd * ld)        /* pointer to log record descriptor */
{
        int n,k,vol, rc = 0;
        int32  hash, xlen, xlength;
        struct nodofile *nodoptr;
        pxd_t pxd1;
        int16 nword, nslots,nbits, i;
        struct doblk *db;
        int32 inonum;

        /* 
         * If it's not part of a committed transaction then it
         * should be ignored, so just return.
         */
        if (!findCommit(ld->logtid))
                return(0);

        /* 
         * if it's the last entry for the current committed transaction,
         * remove the commit record from the commit list because we won't
         * be needing it any more.
         */
        if (ld->backchain == 0)
                deleteCommit(ld->logtid);

        /* 
         * if the filesystem was cleanly unmounted or if the last 
         * thing that happened was a logredo failure, skip  this
         * record.  (Necessary for the case of logredo a log shared
         * by multiple filesystems.  We want to process log records
         * for those filesystems which don't meet this criteria, but
         * skip log records for those which do.)
         */
        vol = 0;
        if (vopen[vol].status == FM_CLEAN || vopen[vol].status == FM_LOGREDO)
                return (0);

        /* 
         * We may already have a NoRedoPage filter in effect.
         * If one is found on the NoRedoFile hash chain, goto update
	 * map.
         *
         * N.B.  The log.noredopage.inode in the log record is the 
         *       inode number of the applicable imap.  We NEVER
         *       NoRedoFile for an imap inode.
         */
	hash = (ld->aggregate + ld->log.noredopage.fileset +
                        ld->log.noredopage.inode ) & nodofilehmask;
	for (nodoptr = nodofilehash[hash]; nodoptr != NULL;
	     nodoptr = nodoptr->next)
	{
		if (ld->aggregate  == nodoptr->aggregate &&
	            ld->log.noredopage.fileset  == nodoptr->fileset &&
		    ld->log.noredopage.inode == nodoptr->inode )
			goto updmap;
	}
        /*
         * start NoRedoPage filter for LOG_NOREDOPAGE log rec.
         *
         */
        xlen = lengthPXD(&ld->log.noredopage.pxd);
        pxd1 = ld->log.noredopage.pxd;

               /*
                * DTREE ROOT
                *
                * Do not process any further log records for
                * the root of the specified (directory) inode's dtree.
                */
        if( (ld->log.noredopage.type & (LOG_BTROOT|LOG_DTREE)) 
                                                == (LOG_BTROOT|LOG_DTREE) ) 
        {
                   /*
                    * get the noredopage record for this page
                    * (one is created if none exists)
                    */
                rc = findPageRedo(ld->aggregate,ld->log.noredopage.fileset,
                                  pxd1, &db);
                if (rc!=0)  {
                        sprintf( message_parm_0, "(d) %d", rc  );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_DNRPFNDDTRTPGREDOFAIL, 0, 1 );
                        return(rc);
                        }
                   /* 
                    * mark the appropriate slot in the noredopage for 
                    * no further updates to this (directory) inode's 
                    * dtree root.
                    */
                inonum = ld->log.redopage.inode & 0x07;
                db->db_dtroot[inonum] = 0x01ff;
                 /*
                  * This inode is identified as a dtroot.  So mark 
                  * the appropriate slot in the noredopage for 
                  * NO updates to this inode as an xtroot.
                  */
                db->db_xtroot[inonum].xtrt_lwm = XTENTRYSTART;
                db->db_xtroot[inonum].xtrt_hd = 1;
        }
               /*
                * DTREE NODE
                *
                * Do not process any further log records for
                * the specified dtree page.
                */
        else if( (ld->log.noredopage.type & LOG_DTREE) == LOG_DTREE ) 
        {
                   /*
                    * get the noredopage record for this page
                    * (one is created if none exists)
                    */
                rc = findPageRedo(ld->aggregate,ld->log.noredopage.fileset,
                                  pxd1, &db);
                if (rc!=0)  {
                        sprintf( message_parm_0, "(d) %d", rc  );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_DNRPFNDDTPGPGREDOFAIL, 0, 1 );
                        return(rc);
                        }

                 /*
                  * This page is identified as a dtpage.  So mark 
                  * the appropriate slots in the noredopage for 
                  * NO updates to this page as an xtpage.
                  */
                db->db_xtpagelwm = XTENTRYSTART;
                db->db_xtpghd = 1;

                /* 
                 * This extent may not be a full page (4096 bytes in length)
                 *
                 * Figure out its size, the number of slots it
                 * covers, the number words used to track those slots,
                 * and the number of bits in the last of those words.
                 */
                xlength = xlen << vopen[vol].l2bsize;
                nslots = xlength >> L2DTSLOTSIZE;  
                nword = nslots >> L2DBWORD;
                nbits = nslots & (DBWORD - 1);
                for ( i = 0; i < nword; i++)
                        db->db_dtpagewd[i] = ONES;
                if ( nbits )
                {
                   i = DBWORD - nbits;
                   db->db_dtpagewd[nword] |= ((uint32)( ONES >> i )) << i;
                }
        }

               /*
                * XTREE ROOT
                *
                * Do not process any further log records for
                * the root of the specified inode's xtree.
                */
        else if( (ld->log.redopage.type & (LOG_BTROOT|LOG_XTREE)) 
                                                 == (LOG_BTROOT|LOG_XTREE) ) 
        {
                   /*
                    * get the noredopage record for this page
                    * (one is created if none exists)
                    */
                rc = findPageRedo(ld->aggregate,ld->log.noredopage.fileset,
                                  pxd1, &db);
                if (rc!=0)  {
                        sprintf( message_parm_0, "(d) %d", rc  );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_DNRPFNDXTRTPGREDOFAIL, 0, 1 );
                        return(rc);
                        }

                   /* 
                    * mark the appropriate slot in the noredopage for no
                    * further updates to this inode's xtree root.
                    */
                inonum = ld->log.redopage.inode & 0x07;  /* the inode's
                                       * position in this page of inodes
                                       */
                db->db_xtroot[inonum].xtrt_lwm = XTENTRYSTART;
                db->db_xtroot[inonum].xtrt_hd = 1;
		/*
                       * This inode is identified as an xtree root.  So
                       * mark the appropriate slot in the noredopage for 
                       * NO updates to this inode's root as a dtree.
		 */
                db->db_dtroot[inonum] = 0x01ff;
        }
               /*
                * XTREE NODE
                *
                * Do not process any further log records for
                * the specified xtree page.
                */
        else if( (ld->log.redopage.type & LOG_XTREE) == LOG_XTREE ) 
        {
                   /*
                    * get the noredopage record for this page
                    * (one is created if none exists)
                    */
                rc = findPageRedo(ld->aggregate,ld->log.noredopage.fileset,
                                  pxd1, &db);
                if (rc!=0)  {
                        sprintf( message_parm_0, "(d) %d", rc  );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_DNRPFNDXTPGPGREDOFAIL, 0, 1 );
                        return(rc);
                        }

                   /* 
                    * mark the noredopage for no further updates
                    * to this page.
                    */
                db->db_xtpagelwm = XTENTRYSTART;
                db->db_xtpghd = 1;
		/*
                       * This page is identified as an xtpage.  So mark 
                       * the appropriate slots in the noredopage for 
                       * NO updates to this page as a dtpage. 
		 */
                db->db_dtpagewd[0] = ONES;
                db->db_dtpagewd[1] = ONES;
                db->db_dtpagewd[2] = ONES;
                db->db_dtpagewd[3] = ONES;
        }
               /*
                * UNRECOGNIZED TYPE
                *
                * We don't ever expect to get here!
                */
        else 
        {
                fsck_send_msg( lrdo_DNRPUNKNOWNTYPE, 0, 0 );
        }

        /*
         * If this is a (non-root) dtree page update the bmap.
         */
 updmap:
        if ( ld->log.noredopage.type == LOG_DTREE ) 
        {
                rc = markBmap((dmap_t *)vopen[vol].bmap_ctl,
                         ld->log.noredopage.pxd, 0, vol);
                if( rc != 0 ) {
                        return( rc );
                        }
        }

        return (0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        doNoRedoInoExt()
 *
 * FUNCTION:    Processing for LOG_NOREDOINOEXT rec type.
 *          
 *              This routine starts a NoRedoPage filter for each
 *		page in the inode extent being released.
 *
 *              This routine may updates the bmap and the imap.
 *
 * NOTE:        The noredoinoext.pxd describes a 4 (4096-byte) page
 *		inode extent.  
 *
 *
 * NOTE:	This log record was written when an inode extent was
 *		released.
 *		
 *		At this point, there are 3 possibilities:
 *		
 *		o the extent could have been reallocated for an inode
 *		  extent (possibly even for the same inode number range.
 *		  If this is true, then there is already a NoRedoExtent
 *		  filter in effect.
 *		
 *		o the extent could now be allocated for user data or for
 *		  JFS metadata.  If this is true, then further updates
 *		  to the blocks in the extent would violate data 
 *		  integrity.  Therefore, we establish a NoRedoExtent 
 *		  filter (i.e., a NoRedoPage filter for each of the
 *		  4 pages in the extent).
 *		
 *		o the extent could be unallocated.  That is, none of the
 *		  blocks in the extent pages are in use.  If this is
 *		  true, then further updates to the blocks in the extent
 *		  are a waste of processing time.  Therefore, we establish
 *		  a NoRedoExtent filter (i.e., a NoRedoPage filter for
 *		  each of the 4 pages in the extent).
 *
 */
doNoRedoInoExt(
struct lrd * ld)        /* pointer to log record descriptor */
{
        int pg_idx, vol, rc = 0;
        pxd_t pxd1;
        struct doblk *db;
        int32 iagnum, iagext_idx;
        iag_data_t *imp;
        int inonum;

        /* 
         * If it's not part of a committed transaction then it
         * should be ignored, so just return.
         */
        if (!findCommit(ld->logtid))
                return(0);

        /* 
         * if it's the last entry for the current committed transaction,
         * remove the commit record from the commit list because we won't
         * be needing it any more.
         */
        if (ld->backchain == 0)
                deleteCommit(ld->logtid);

        /* 
         * if the filesystem was cleanly unmounted or if the last 
         * thing that happened was a logredo failure, skip  this
         * record.  (Necessary for the case of logredo a log shared
         * by multiple filesystems.  We want to process log records
         * for those filesystems which don't meet this criteria, but
         * skip log records for those which do.)
         */
        vol = 0;
        if (vopen[vol].status == FM_CLEAN || vopen[vol].status == FM_LOGREDO)
                return (0);
	
	    /*
	     * Establish the 4 NoRedoPage filters which together 
	     * form the NoRedoExtent filter.
	     */
	pxd1 = ld->log.noredoinoext.pxd;
	for( pg_idx = 0; pg_idx < 4; pg_idx++ ) {
		/*
		 * find the noredo record for this page.
		 * (or create one if not found)
		 */
	    rc = findPageRedo( ld->aggregate,
				ld->log.noredoinoext.fileset,
				pxd1, &db );  
	    if( rc != 0 ) {
		sprintf( message_parm_0, "(d) %d", rc  );
		msgprms[0] = message_parm_0;
		msgprmidx[0] = 0;
		fsck_send_msg( lrdo_DNRIFNDNOREDORECFAIL, 0, 1 );
		return( rc );
		}
		/*
		 * mark for no inode updates to the page, no matter what
		 * format it might appear to have
		 */
	    db->db_ibase = 0xFF; 
	    db->db_iea = 0xFF;
	    db->db_idata = 0xFF;
               for( inonum=0; inonum<8; inonum++ ) {
                   db->db_xtroot[inonum].xtrt_lwm = XTENTRYSTART;
                   db->db_xtroot[inonum].xtrt_hd = 1;
                   db->db_dtroot[inonum] = 0x01ff;
                   }
               db->db_xtpagelwm = XTENTRYSTART;
               db->db_xtpghd = 1;
               db->db_dtpagewd[0] = ONES;
               db->db_dtpagewd[1] = ONES;
               db->db_dtpagewd[2] = ONES;
               db->db_dtpagewd[3] = ONES;
	        /*
	 	 * set up for the next page in the extent
		 */
	    PXDaddress( &pxd1,
			(addressPXD(&pxd1) + vopen[vol].lbperpage)
		      );
	    }  /* end for */
	    /*
	     * Now go update the block map if appropriate.
	     *
	     * Note:
	     *	If any of these blocks has been allocated LATER
	     *  (in time) than this extent release then we don't
	     *  want to mark them unallocated, but if not then
	     *  we must mark them unallocated.  Since we process
	     *  log records in LIFO order, we have already 
	     *  processed the log record(s) (if any) describing 
	     *  reallocation of the block(s).
	     *
	     *  The markBmap routine only marks the block map
	     *  for blocks whose status has not already been 
	     *  set by markBmap in the current logredo session.
	     *
	     */
	rc = markBmap( (dmap_t *)vopen[vol].bmap_ctl,
	          ld->log.noredoinoext.pxd, 0, vol );
	if( rc != 0 ) {
	        return( rc );
	        }
	    /*
	     * Now, if no extent has been reallocated for
	     * the inodes in the range which the extent being
	     * released describes, then the IAG needs to be
	     * updated to show that this extent is not 
	     * allocated.
	     *
	     * NOTE: It is not necessary to adjust the IAG 
	     *       Free Extent List, the IAG Free Inodes List,
	     *       or the Free IAG List because they will be
	     *       rebuilt from scratch before the filesystem
	     *       is remounted (either near the end of logredo
	     *	     processing or by chkdsk after logredo returns
	     *       control to it with rc != 0)
	     *
	     * NOTE: The wmap of the IAG tells whether logredo
	     *       has already encountered a log record for
	     *       one of these inodes.  This would mean 
	     *       activity for the inode(s) LATER IN TIME
	     *       than the current transaction.  If no such
	     *	     record has been seen, then the IAG[extno]
	     *       needs to be cleared.
	     */
	iagnum = ld->log.noredoinoext.iagnum;

           imp = vopen[vol].fsimap_lst.imap_wsp[(iagnum + 1)].imap_data;
           if( imp == NULL ) {  /* first touch to this IAG */
                rc = iagGet( vol, iagnum );
                if( rc != 0 ) {
                        return( rc );
                        }
                imp = vopen[vol].fsimap_lst.imap_wsp[(iagnum + 1)].imap_data;
                }  /* end first touch to this IAG */

	iagext_idx = ld->log.noredoinoext.inoext_idx;

	if( imp->wmap[iagext_idx] == 0 ) {  /* no later activity for 
				* the inodes in the range for this 
				* inode extent. 
				*/
	    imp->wmap[iagext_idx] = 0xFFFFFFFF;  /* all of them are
				* now in a determined state
				*/
	    imp->pmap[iagext_idx] = 0x00000000;  /* and that state is
				* 'not allocated'
				*/
	    PXDlength( &(imp->inoext[iagext_idx]), 0 );
	    PXDaddress( &(imp->inoext[iagext_idx]), 0 );
	    }  /* end no activity for the inodes in the range for ... */

        return (0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        doUpdateMap(ld)
 *
 * FUNCTION:    processing for LOG_UPDATEMAP record types.
 *
 *           IN GENERAL
 *           ----------
 *              The updatemap log record has many sub-types defined in
 *              ld->log.updatemap.type, but in general, the log record
 *              data describes file system block extent(s) for which 
 *              the block map (bmap) needs to be marked.
 *
 *           IN PARTICULAR
 *           -------------
 *              LOG_ALLOCPXD -- is written when an outline EA is allocated
 *
 *                 The log record data is a single PXD describing the
 *                 filesystem blocks to be marked allocated in the block map.
 *
 *              LOG_ALLOCPXDLIST -- is not used in release 1 of JFS
 *
 *              LOG_ALLOCXAD -- is not used in release 1 of JFS
 *
 *              LOG_ALLOCXADLIST -- is not used in release 1 of JFS
 *
 *              LOG_FREEPXD -- is written when a file is truncated and 
 *                             a portion of the extent described by a
 *                             PXD is released
 *                          -- is written when an outline EA is freed
 *
 *                 The log record data is a single PXD describing the
 *                 filesystem blocks to be marked free in the block map.
 *
 *              LOG_FREEPXDLIST -- is written when a file is compressed 
 *                              -- is written when a file grows and the tail
 *                                 cannot be extended in-place. **see note
 *
 *                 The log record data is a list of PXD's describing the
 *                 extents to be marked free in the block map.
 *
 *              LOG_FREEXAD -- is not used in release 1 of JFS
 *
 *              LOG_FREEXADLIST -- is written when a file is truncated
 *
 *                 The log record data is a list of XAD's describing the
 *                 extents to be marked free in the block map.
 *
 * **note:  (see reference in LOG_FREEPXDLIST above)
 *
 *         Each extent of a file MUST be an even multiple of 4096 byte 
 *         pages, except the last extent, which need only be an even 
 *         multiple of filesystem blocks.  This means that, if the last 
 *         extent is not a whole page, then it must be extended to a 
 *         whole page before another extent can be added.  If the 
 *         filesystem blocks immediately following the current last 
 *         extent are not available, then it is necessary to select 
 *         a larger storage area and copy the contents of the current
 *         last extent and the new data (the reason the file is being
 *         extended in the first place) into it.
 *
 *         To illustrate:  
 *            suppose the filesystem block size is 1024 and a particular
 *            files is 9216 bytes long, stored as 
 *                1 extent 8 fs blocks   (8192 bytes = 2 * 4096) and
 *                1 extent 1 fs block    (1024 bytes)
 *            now suppose another 5120 bytes are appended to the file.
 *            The 2nd extent must be extended because it must either
 *            become an even multiple of 4096 or it must remain the last
 *            extent in the file.
 *            Both:
 *                          1 extent 8 fs blocks  (8192 bytes)
 *                          1 extent 6 fs blocks  (6144 bytes)
 *            and:
 *                          1 extent 8 fs blocks  (8192 bytes)
 *                          1 extent 4 fs blocks  (4096 bytes)
 *                          1 extent 2 fs blocks  (2048 bytes) 
 *            are possible and correct outcomes. 
 *
 *
 * NOTE:  Since UPDATEMAP log records only affect the block map, 
 *        the noredofile hash chain is not checked and the transaction
 *        should not be skipped. 
 *
 *        When a file system object is deleted, UPDATEMAP log records
 *        are created (as appropriate) to release storage from (and 
 *        possibly for) metadata xtree pages.  No NoRedoPage log records
 *        are written for these pages.
 *
 */
doUpdateMap(
struct lrd * ld)        /* pointer to log record descriptor */
{
        int i, j, k,vol, rc = 0;
        int32  hash;
        struct nodofile *nodoptr;
	struct doblk *db;
        xad_t *l_xad;
        pxd_t *l_pxd;
        pxd_t pxd1;

        /* 
         * If it's not part of a committed transaction then it
         * should be ignored, so just return.
         */
        if (!findCommit(ld->logtid))
                return(0);

        /* 
         * if it's the last entry for the current committed transaction,
         * remove the commit record from the commit list because we won't
         * be needing it any more.
         */
        if (ld->backchain == 0)
                deleteCommit(ld->logtid);


        /* 
         * if the filesystem was cleanly unmounted or if the last 
         * thing that happened was a logredo failure, skip  this
         * record.  (Necessary for the case of logredo a log shared
         * by multiple filesystems.  We want to process log records
         * for those filesystems which don't meet this criteria, but
         * skip log records for those which do.)
         */
        vol = 0;
        if (vopen[vol].status == FM_CLEAN || vopen[vol].status == FM_LOGREDO)
                return (0);

        if ( ld->log.updatemap.type & LOG_FREEXADLIST )
        {
            /*
             * The data area contains an array of XAD's, with
             * updatemap.nxd elements.  
             */
                l_xad = (xad_t*)afterdata;
                for ( i = 0; i < ld->log.updatemap.nxd; i++)
                {
                        PXDaddress(&pxd1, addressXAD(l_xad));
                        PXDlength(&pxd1, l_xad->len);
                        rc = markBmap((dmap_t*)vopen[vol].bmap_ctl, pxd1, 0, vol);
                        if( rc != 0 ) {
                                return( rc );
                                }
                        l_xad += 1;
                }
        }
        else if (ld->log.updatemap.type & LOG_FREEPXDLIST )
        {
            /*
             * The data area contains an array of PXD's, with 
             * updatemap.nxd elements.  
             */
                l_pxd = (pxd_t*)afterdata;
                for ( i= 0; i < ld->log.updatemap.nxd; i++, l_pxd++)
                        rc = markBmap((dmap_t*)vopen[vol].bmap_ctl, 
                                 *l_pxd, 0, vol);
                        if( rc != 0 ) {
                                return( rc );
                                }
        }
        else if ( ld->log.updatemap.type & LOG_FREEPXD )
        {
            /*
             * The updatemap.nxd should be 1 in this case.
             */
                if ( ld->log.updatemap.nxd > 1 )
                        fsError(LOGRCERR, vol, ld->log.updatemap.nxd );
                        rc = markBmap((dmap_t *)vopen[vol].bmap_ctl,
                                                ld->log.updatemap.pxd, 0, vol);
                        if( rc != 0 ) {
                                return( rc );
                                }
        }
        else if ( ld->log.updatemap.type & LOG_ALLOCPXD )
        {
            /*
             * The updatemap.nxd should be 1 in this case.
             */
                if ( ld->log.updatemap.nxd > 1 )
                        fsError(LOGRCERR, vol, ld->log.updatemap.nxd);
                        rc = markBmap((dmap_t *)vopen[vol].bmap_ctl,
                                              ld->log.updatemap.pxd, 1, vol);
                        if( rc != 0 ) {
                                return( rc );
                                }
        }
        else
                fsck_send_msg( lrdo_DUMPUNKNOWNTYPE, 0, 0 );

        return (0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        dtpg_resetFreeList(ld)
 *
 * FUNCTION:    Reset the freelist in the given Directory Btree (nonroot)
 * 		node.
 *
 * NOTE:        none
 *
 */
dtpg_resetFreeList( int32 vol, int *buf)       /* buf contains on-disk page image */
{

        int32  i,j;
        uint32 wbitno;
        int16  pxd_len;
        int16  nslots;  /* number of slots in log.redopage.pxd */
        int16  nword;   /* number of words in the pxd  */
        dtslot_t        *f;
        dtpage_t        *dtpg;
        uint32 uzbit32, fword, fsi;

        int16		stbl_nslots;  /* number of slots occupied by
				* the stbl for the page
				*/
        int8		slot_map[DTPAGEMAXSLOT];
        int8		*slot_table;
        int16		sidx, slot_idx;
        dtslot_t	*this_slot, *last_slot;
        idtentry_t	*intern_hdr_slot;
        ldtentry_t	*leaf_hdr_slot;

        dtpg = (dtpage_t*)buf;

        if( dtpg->header.nextindex == -1 ) {  /*
                                  * the stbl is full, no slots 
                                  * can be available
                                  */
	    dtpg->header.freecnt = 0;
	    dtpg->header.freelist = -1;
	    }  /* end the stbl is full, no slots can be available */
        else {  /* the stbl isn't full. slots may be free. */
	    /* 
	     * The dtree page size is 512, 1024, 2048, or 4096 bytes.
	     * We need to know how many slots it contains and how many
	     * slots are occupied by its slot table (stbl).
	     */
	    pxd_len = 
		(lengthPXD(&dtpg->header.self)) << vopen[vol].l2bsize;
	    switch (pxd_len) {
		case DT8THPGNODEBYTES:		/* 512 bytes */
		    nslots = DT8THPGNODESLOTS;
		    stbl_nslots = DT8THPGNODETSLOTS;
		    break;
		case DTQTRPGNODEBYTES:		/* 1024 bytes */
		    nslots = DTQTRPGNODESLOTS;
		    stbl_nslots = DTQTRPGNODETSLOTS;
		    break;
		case DTHALFPGNODEBYTES:		/* 2048 bytes */
		    nslots = DTHALFPGNODESLOTS;
		    stbl_nslots = DTHALFPGNODETSLOTS;
		    break;
		default:			/* 4096 bytes */
		    nslots = DTFULLPGNODESLOTS;
		    stbl_nslots = DTFULLPGNODETSLOTS;
		    break;
		}  /* end switch */
		/*
		 * clear the slot map
		 */
	    for( sidx=0; sidx < nslots; sidx++ ) {
		slot_map[sidx] = 0;
		}
		/*
		 * account for the header and for the stbl slots
		 */
	    slot_map[0] = -1;  /* the header */
	    for( sidx=0; sidx < stbl_nslots; sidx++ ) {  
		slot_map[ dtpg->header.stblindex + sidx ] = -1;
		}  /* end for */

	    slot_table = (int8 *) &(dtpg->slot[ dtpg->header.stblindex ]);

 			/*
			 * figure out which slots are in use
			 */
	    for( sidx=0; sidx < dtpg->header.nextindex; sidx++ ) {
		/* 
		 * the dir entry header slot
		 */
		    /* 
		     * If the index is out of bounds or if we've 
		     * already seen it in use then something is
		     * seriously wrong and we need a full chkdsk.
		     * Since the problem could have been caused
		     * by something in this logredo session,
		     * signal chkdsk to reformat the log.
		     */
		if( (slot_table[sidx] >= nslots) || 
		    (slot_map[ slot_table[sidx] ] != 0)    ) { 
		    sprintf( message_parm_0, "0x0%llx", (addressPXD(&dtpg->header.self))  );
		    msgprms[0] = message_parm_0;
		    msgprmidx[0] = 0;
		    fsck_send_msg( lrdo_DPRFBADSTBLENTRY, 0, 1 );
		    return( DTPAGE_BADSTBLENTRY1 );
		    }  /* endif */

		slot_map[ slot_table[sidx] ] = -1;
		    /*
		     * any continuation slots for the dir entry
		     */
		if( (dtpg->header.flag & BT_LEAF) == BT_LEAF ) {
		    leaf_hdr_slot = (ldtentry_t *)
					&( dtpg->slot[ slot_table[sidx] ]);
		    slot_idx = leaf_hdr_slot->next;
		    }
		else {  /* internal page */
		    intern_hdr_slot = (idtentry_t *)
					&( dtpg->slot[ slot_table[sidx] ]);
		    slot_idx = intern_hdr_slot->next;
		    }  /* end else internal page */
		while( slot_idx != -1 ) {
			/* 
			 * if the index is out of bounds or
			 * if we've already seen it in use then 
			 * something is seriously wrong and we 
			 * need a full chkdsk.
			 * Since the problem could have been caused
			 * by something in this logredo session,
			 * signal chkdsk to reformat the log.
			 */
		    if( (slot_idx >= nslots) ||
		        (slot_map[ slot_idx ] != 0)    ) { 
			sprintf( message_parm_0, "0x0%llx", (addressPXD(&dtpg->header.self))  );
			msgprms[0] = message_parm_0;
			msgprmidx[0] = 0;
			fsck_send_msg( lrdo_DPRFBADSLOTNXTIDX, 0, 1 );
			return( DTPAGE_BADSLOTNEXTIDX1 );
			}  /* endif */

		    slot_map[ slot_idx ] = -1;
		    this_slot = &( dtpg->slot[slot_idx] );
		    slot_idx = this_slot->next;
		    }  /* end while slot_idx */
		}  /* end for sidx */

		/*
		 * find the first available slot
		 */
	    dtpg->header.freecnt = 0;      /* assume none free */
	    dtpg->header.freelist = -1;    /* assume none free */
	    for( sidx=0; 
	         ((sidx < nslots) && (dtpg->header.freecnt == 0));
	         sidx++ ) {
		if( slot_map[sidx] == 0 ) {
		    dtpg->header.freecnt = 1;
		    dtpg->header.freelist = sidx;
		    slot_idx = sidx;
		    last_slot = &( dtpg->slot[sidx] );
		    }  /* end if */
		}  /* end for */
		/*
		 * count and chain together all available slots
		 */
	    if( dtpg->header.freecnt != 0 ) {  /* found a free one */
		for( sidx=(slot_idx + 1); sidx < nslots; sidx++ ) {
		    if( slot_map[sidx] == 0 ) {
			last_slot->next = sidx;
			dtpg->header.freecnt += 1;
			last_slot = &( dtpg->slot[sidx] );
		        }  /* end if */
		    }  /* end for */
		last_slot->next = -1;  /* terminate the chain */
		}  /* end found a free one */
	    }  /* end else the stbl isn't full. slots may be free. */

	return(0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        dtrt_resetFreeList(ld)
 *
 * FUNCTION:    Reset the freelist in the given Directory inode Btree root.
 *
 * NOTE:        none
 */
dtrt_resetFreeList(
int32 vol,
struct doblk *db,
struct lrd *ld,
caddr_t buf_btroot)       /* the buffer contains on-disk page image */
{
        dtroot_t        *dtrt;
        int8		slot_map[DTROOTMAXSLOT];
        int16		sidx, slot_idx;
	dtslot_t	*this_slot, *last_slot;
	idtentry_t	*intern_hdr_slot;
	ldtentry_t	*leaf_hdr_slot;

	/*
         * The doblk.i_dtroot (or doblk.dtpage_word) tracks which slots
         * slots have been updated, but since some of those updates may
         * be deletions, we can't use it to create the freelist.
         */
	dtrt = (dtroot_t*)buf_btroot;
	if( dtrt->header.nextindex == -1 ) {  /*
                                  * the stbl is full, no slots 
                                  * can be available
                                  */
	    dtrt->header.freecnt = 0;
	    dtrt->header.freelist = -1;
	    }  /* end the stbl is full, no slots can be available */

	else {  /* the stbl isn't full. slots may be free. */
		/*
		 * clear the slot map
		 */
	    for( sidx=0; sidx < DTROOTMAXSLOT; sidx++ ) {
		slot_map[sidx] = 0;
		}
	    slot_map[0] = -1;  /* the header occupies this space */
		/*
		 * figure out which slots are in use
		 */
	    for( sidx=0; sidx < dtrt->header.nextindex; sidx++ ) {
		/* 
		 * the dir entry header slot
		 */
		    /* 
		     * If the index is out of bounds or if we've 
		     * already seen it in use then something is
		     * seriously wrong and we need a full chkdsk.
		     * Since the problem could have been caused
		     * by something in this logredo session,
		     * signal chkdsk to reformat the log.
		     */
		if( (dtrt->header.stbl[sidx] >= DTROOTMAXSLOT) ||
		    (slot_map[ dtrt->header.stbl[sidx] ] != 0)    ) { 
		    fsck_send_msg( lrdo_DRRFBADSTBLENTRY, 0, 0 );
		    return( DTPAGE_BADSTBLENTRY2 );
		    }  /* endif */

		slot_map[ dtrt->header.stbl[sidx] ] = -1;
		    /*
		     * any continuation slots for the dir entry
		     */
		if( (dtrt->header.flag & BT_LEAF) == BT_LEAF ) {
		    leaf_hdr_slot = (ldtentry_t *)
			    &( dtrt->slot[ dtrt->header.stbl[sidx] ]);
		    slot_idx = leaf_hdr_slot->next;
		    }
		else {  /* internal page */
		    intern_hdr_slot = (idtentry_t *)
			    &( dtrt->slot[ dtrt->header.stbl[sidx] ]);
		    slot_idx = intern_hdr_slot->next;
		    }  /* end else internal page */
		while( slot_idx != -1 ) {
			/* 
			 * if the index is out of bounds or if we've 
			 * already seen it in use then something is
			 * seriously wrong and we need a full chkdsk.
			 *
			 * Since the problem could have been caused by
			 * something in this logredo session, signal 
			 * chkdsk to reformat the log.
			 */
		    if( (slot_idx >= DTROOTMAXSLOT) ||
		        (slot_map[ slot_idx ] != 0)    ) { 
			fsck_send_msg( lrdo_DRRFBADSLOTNXTIDX, 0, 0 );
			return( DTPAGE_BADSLOTNEXTIDX2 );
			}  /* endif */

		    slot_map[ slot_idx ] = -1;
		    this_slot = &( dtrt->slot[slot_idx] );
		    slot_idx = this_slot->next;
		    }  /* end while slot_idx */
		}  /* end for sidx */
		/*
		 * find the first available slot
		 */
	    dtrt->header.freecnt = 0;      /* assume none free */
	    dtrt->header.freelist = -1;    /* assume none free */
	    for( sidx=0; 
	         ((sidx < DTROOTMAXSLOT) && (dtrt->header.freecnt == 0));
	         sidx++ ) {
		if( slot_map[sidx] == 0 ) {
		    dtrt->header.freecnt = 1;
		    dtrt->header.freelist = sidx;
		    slot_idx = sidx;
		    last_slot = &( dtrt->slot[sidx] );
		    }
		}
		/*
		 * count and chain together all available slots
		 */
	    if( dtrt->header.freecnt != 0 ) {  /* found a free one */
		for( sidx=(slot_idx + 1); sidx < DTROOTMAXSLOT; sidx++ ) {
		    if( slot_map[sidx] == 0 ) {
			last_slot->next = sidx;
			dtrt->header.freecnt += 1;
			last_slot = &( dtrt->slot[sidx] );
		        }  /* end if */
		    }  /* end for */
		last_slot->next = -1;  /* terminate the chain */
		}  /* end found a free one */
	    }  /* end else the stbl isn't full. slots may be free. */

	return(0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        findCommit(tid)
 *
 * FUNCTION:    Search in the commit array for a commit record with 
 *              transaction id (tid) matching the given tid.
 *              Return the commit array index where found, or 0
 *              if not found.
 */
findCommit(
int32 tid)  /* transaction id */
{
        int32 k,hash;

        hash = tid & comhmask;  /* hash class */
        for (k = comhash[hash]; k != 0; k = com[k].next)
                if (com[k].tid == tid)
                        return(k);

        return(0);   /* not found */
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        findPageRedo()
 *
 * FUNCTION:    Search in the RedoPage hash table for a record 
 *              containing the block address in the given pxd.
 *              If no match is found, such a record is created and 
 *              inserted into the table.
 *
 *              The address of the found (or created) doblk is
 *              returned.
 *
 */
findPageRedo(
int32 aggregate,    /* file system aggregate/lv number */
int32 fileset,      /* fileset number within this aggregate */
pxd_t  pxd,          /* on-disk page pxd  */
struct doblk **doptr)
{
        int rc = 0;
        int32 hash, i,j;
        struct doblk *dp;
        int32 allocated_from_bmap = 0;

           /*
            * Search for a record with matching aggregate, fileset,
            * and block offset.
            */
        hash = (aggregate + fileset + addressPXD(&pxd)) & blkhmask;
        for (dp = blkhash[hash];  dp != NULL; dp = dp->next)
                if (dp->aggregate == aggregate &&
                        dp->fileset == fileset &&
                       addressPXD(&dp->pxd) == addressPXD(&pxd) )
                {
                   /*
                    * match found.  return its address to caller
                    */
                        *doptr = dp;
                        return(0);
                }

           /*
            * No match was found.
            */
                /*
                 * if there are no available doblk records,
                 * allocate some storage
                 */
        if ( Freedoblk == 0)
        {
           sprintf( message_parm_0, "(d) %d", PSIZE );
           msgprms[0] = message_parm_0;
           msgprmidx[0] = 0;
           fsck_send_msg( lrdo_ALLOC4DOBLK, 0, 1 );

           rc = alloc_storage( 	(uint32) PSIZE, 
				(void **) &Blkpage,
				&allocated_from_bmap 
				);
           if( (rc != 0) || (Blkpage == NULL) ) {  /* doblk allocation failed */
                      sprintf( message_parm_0, "(d) %lld", PSIZE );
                      msgprms[0] = message_parm_0;
                      msgprmidx[0] = 0;
                      fsck_send_msg( lrdo_ALLOC4DOBLKFAIL, 0, 1 );
                      return(ENOMEM4);
                      }  /* end doblk allocation failed */
           else if( Blkpage != NULL ) {  /* doblk allocation successful */
                      if( allocated_from_bmap ) {
                                 fsck_send_msg( lrdo_USINGBMAPALLOC4DOBLK, 0, 0 );
                                 }
                      }  /* end doblk allocation successful */

           Freedoblk = PSIZE/sizeof(struct doblk);
           }
        dp = Blkpage;

           /*
            * Allocate a doblk record and initialize it
            * with the given aggregate, fileset, and block 
            * offset.  Insert the record into the RedoPage 
            * hash table.
            */
        numdoblk++;
        Blkpage++ ;
        Freedoblk--;
        dp->next = blkhash[hash];
        blkhash[hash] = dp;
        dp->aggregate = aggregate;
        dp->fileset = fileset;
        dp->pxd = pxd;
        dp->db_ibase = 0;
        dp->db_iea = 0;
        dp->db_idata = 0;
        for ( i = 0, j=0; i< 4; i++,j+=2)
        {
                dp->db_dtpagewd[i] = 0;
        }
        for ( j = 0; j < 8; j++)
        {
                dp->db_xtroot[j].xtrt_hd = 0;
                dp->db_xtroot[j].xtrt_lwm = 0xff;
        }

            /*
             * return the address of the created doblk to caller
             */
        *doptr = dp;

        return(0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:     logredoInit()
 *
 * FUNCTION:    allocate/initialize runtime data structures and
 *              initialize for file systems sharing the log.
 */
int32
logredoInit()
{
        int rc = 0; 
        int32  k, nword, nbits;
        int32 allocated_from_bmap = 0;

        /*
         * init free list for com. index 0 is not used.
         */
        comfree = 1;
        for (k = 1; k < COMSIZE; k++)
                com[k].next = k + 1;

        /*
         * init comhash chains
         */
        for (k = 0; k < 64; k++)
                comhash[k] = 0;

        /*
         * init block hash chains
         */
        numdoblk = 0;
        for (k = 0 ; k < BHASHSIZE ; k++ )
                blkhash[k] = NULL;

        /*
         * allocate one page space for redo page hash table
         */
        sprintf( message_parm_0, "(d) %d", PSIZE );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_ALLOC4REDOPG, 0, 1 );

        rc = alloc_storage( 	(uint32) PSIZE, 
				(void **) &Blkpage,
				&allocated_from_bmap 
				);
        if( (rc != 0) || (Blkpage == NULL) ) {  /* RedoPage record allocation failed */
                sprintf( message_parm_0, "(d) %lld", PSIZE );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_ALLOC4REDOPGFAIL, 0, 1 );
                return(ENOMEM5);
                }  /* end RedoPage record allocation failed */
        else if( Blkpage != NULL ) {  /* RedoPage record allocation successful */
                if( allocated_from_bmap ) {
                        fsck_send_msg( lrdo_USINGBMAPALLOC4RDPG, 0, 0 );
                        }
                }  /* end RedoPage record allocation successful */
        Freedoblk = PSIZE/sizeof(struct doblk);

        /*
         * init nodofile hash chains , and counts
         */

        numnodofile = 0;
        for ( k = 0; k < NODOFILEHASHSIZE; k++)
                nodofilehash[k] = NULL;

        /*
         * allocate one page space for nodo file hash table
         */
        sprintf( message_parm_0, "(d) %d", PSIZE );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_ALLOC4NODOFL, 0, 1 );

        rc = alloc_storage( 	(uint32) PSIZE, 
				(void **) &Nodofilep,
				&allocated_from_bmap 
				);
        if( (rc != 0) || (Nodofilep == NULL) ) {  /* RedoPage record allocation failed */
                sprintf( message_parm_0, "(d) %lld", PSIZE );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_ALLOC4NODOFLFAIL, 0, 1 );
                return(ENOMEM6);
                }  /* end NoDoFile record allocation failed */
        else if( Nodofilep != NULL ) {  /* NoDoFile record allocation successful */
                if( allocated_from_bmap ) {
                        fsck_send_msg( lrdo_USINGBMAPALLOC4NDFL, 0, 0 );
                        }
                }  /* end NoDoFile record allocation successful */

        Freenodofile = PSIZE/sizeof(struct nodofile);

        /* init buffer pool */
        for (k = 0; k < NBUFPOOL; k++)
        {
                bufhdr[k].next = k + 1;
                bufhdr[k].prev = k - 1;
        }

        bufhdr[0].prev = NBUFPOOL - 1;
        bufhdr[NBUFPOOL - 1]. next = 0;

        /*
         *      initialize file systems
         * For outlinelog,
         * open all file system lvs which were in the log active list;
         * validate superblock and allocation map of file systems;
         * For inlinelog, only one file system to be processed at one time,
         * so open just this file system. logmajor and logminor is the file
         * system's major and minor numbers.
         */
        if ( loglocation == INLINELOG )
        {
                if (openVol(logminor) != 0 )
                        return(CANTOPEN_INLINELOG);
        }
        else
                for (k = 0; k < NUMMINOR ; k++)
                {
                        nword = k >> L2DBWORD;
                        nbits = k & (DBWORD -1);
                        if (logsup.active[nword] & (UZBIT_32 >> nbits))
                        {
                                if (openVol(k) != 0 )
                                        return(CANTOPEN_OUTLINELOG);
                        }
                }

        return (0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        markBmap()
 *
 * FUNCTION:    This routine updates the Aggregate Block Map 
 *              for each block described by the given pxd unless
 *              it has already been done.  That is, on a block-by-block
 *              basis, if the Aggregate Block Map has not been updated
 *              for the block in the current session, then it is 
 *              updated now.
 *
 *              Specifically, if the Block Map persistent map (pmap)
 *              bit representing the block has not already been 
 *              updated by this routine, that bit is updated as
 *              requested by the caller 
 *
 * NOTES:	This routine uses the Block Map page working map (wmap) 
 *              to keep track of which IAG pmap bits have already been 
 *              updated by this routine.
 */
markBmap(
dmap_t *dmappg,         /* the beginning of bmap file */
pxd_t   pxd,            /* descriptor for the blocks of interest */
int     val,            /* 1 to allocate, 0 to free      */
int     vol)
{
        int rc = 0;
        int64 blkno, dmap_pg;
        dmap_bitmaps_t *dp;
        uint32 rem, nblocks,word,dbitno,nblks,rbits, nwords;
        uint16 wbitno, nbits,n,j;
        int32 dmap_number;

    if( ! Insuff_memory_for_maps ) {  /* we do have a local BMap image */
        nblocks = lengthPXD(&pxd); /* number of blocks described */
        blkno = addressPXD(&pxd);  /* the first block number */
        if ((blkno + nblocks) > vopen[vol].fssize)  {
                sprintf( message_parm_0, "0x0%llx", blkno  );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                sprintf( message_parm_1, "(d)%ld", nblocks  );
                msgprms[1] = message_parm_1;
                msgprmidx[1] = 0;
                fsck_send_msg( lrdo_MBMPBLKOUTRANGE, 0, 2 );
                fsError(DBTYPE, vol, blkno);
                return (BLOCK_OUTOFRANGE);
                }

        /*
         * nblocks may be large enough to span several dmap_t pages.
         * Update the block state one dmap_t page at a time.
         */
        for (rem = nblocks; rem > 0; rem -= nblks, blkno += nblks)
        {
            dmap_pg = BLKTODMAPN(blkno);
            dp = vopen[vol].bmap_wsp[dmap_pg].dmap_bitmaps;
            if( dp == NULL ) 
            {  /* first touch to this dmap */
                  dmap_number = blkno >> L2BPERDMAP;
                  rc = dMapGet( vol, dmap_number );
                  if( rc != 0 ) {
                          return( rc );
                          }
                  dp = vopen[vol].bmap_wsp[dmap_pg].dmap_bitmaps;
            }  /* end first touch to this dmap */

            if( dp != NULL ) 
            {  /* we have bit maps for this dmap */               
                dbitno = blkno & (BPERDMAP - 1);  /* 
                         * the bit position, within the current dmap
                         * page, representing the current aggregate block.
                         */
                word = dbitno >> L2DBWORD;  /*
                         * the word, within the current dmap page, which
                         * contains the bit for the block.
                         */
                nblks = MIN(rem, BPERDMAP - dbitno); /*
                         * number of blocks which are in the extent and
                         * are described by the current dmap.
                         */
                /* 
                 * Mark the dmap bitmap.
                 */
                for (rbits = nblks; rbits > 0; rbits -= nbits, dbitno += nbits)
                {
                        wbitno = dbitno & (DBWORD-1);  /* 
                              * bit position in the word
                              */
                        nbits = MIN(rbits, DBWORD - wbitno); /*
                              * number of bits in the word
                              */

                            /* 
                             * only part of the word is implicated
                             */
                        if (nbits < DBWORD)
                        {
                                for ( n = 0; n < nbits; n++, wbitno++)
                                {
                                        /* 
                                         * If bit already updated in this
                                         * logredo session, nothing to do.
                                         */
                                        if (dp->wmap[word] & (UZBIT_32 >> wbitno))
                                                continue;

                                        /* update pmap according to val.
                                         * set wmap to indicate state is determined.
                                         */
                                        dp->wmap[word] |= 
                                                  (UZBIT_32 >> wbitno); /*
                                            * note (in the wmap) that the 
                                            * bits have been updated in 
                                            * this session.
                                            */
                                        if (val)  /* request to turn on */
                                                dp->pmap[word] |= 
                                                   (UZBIT_32 >> wbitno);
                                        else      /* request to turn off */
                                                dp->pmap[word] &= 
                                                   ~(UZBIT_32 >> wbitno); 
                                }
                                word += 1;
                        }
                        else  /* nbits == DBWORD. One or more words
                               * are to have all their bits updated.
                               */
                        {
                                nwords = rbits >> L2DBWORD;  /*
                                    * number of words to update
                                    */
                                nbits = nwords << L2DBWORD; /*
                                    * total number of bits in those words
                                    */
                                for ( n= 0 ; n < nwords; n++, word++)
                                   for ( j = 0; j < DBWORD; j++)
                                   {
                                        /* 
                                         * If bit already updated in this
                                         * logredo session, nothing to do.
                                         */
                                        if (dp->wmap[word] & (UZBIT_32 >> j))
                                                continue;
                                        dp->wmap[word] |= 
                                                        (UZBIT_32 >> j);/*
                                            * note (in the wmap) that the 
                                            * bits have been updated in 
                                            * this session.
                                            */

                                        if (val)  /* turn on request */
                                                dp->pmap[word] |= 
                                                        (UZBIT_32 >> j);
                                        else      /* turn off request */
                                                dp->pmap[word] &= 
                                                        ~(UZBIT_32 >> j);
                                        } /* end for j */
                                }
                        }
                  }  /* end we have bit maps for this dmap */
            }  
        }  /* end we do have a local BMap image */
        return(0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        markImap()
 *
 * FUNCTION:    This routine updates the inode allocation map
 *              for the specified inode unless it has already 
 *              been done.
 *
 *              Specifically, if the IAG persistent map (pmap)
 *              bit representing the inode has not already been 
 *              updated by this routine,
 *
 *              - that bit is updated as requested by the caller 
 *
 *              - If the bit is being set to '1', the descriptor  
 *                for the extent containing the inode is refreshed.
 *
 * NOTES:	This routine uses the IAG working map (wmap) to keep
 *              track of which IAG pmap bits have already been updated
 *              by this routine.
 */
markImap(
struct fsimap_lst fsimap,        /* data for the inode table of the owning fileset */
ino_t   inum,           /* inode number */
pxd_t   inopxd,         /* inode extent descriptor this inode */
int     val,            /* 1 to allocate, 0 to free      */
int     vol )
{
        int rc;
        iag_data_t *imp;
        int32 iag_num,ino,extno,bitno;

        iag_num = INOTOIAG(inum);

        imp = fsimap.imap_wsp[(iag_num + 1)].imap_data;
        if( imp == NULL ) {  /* first touch to this IAG */
                rc = iagGet( vol, iag_num );
                if( rc != 0 ) {
                        return( rc );
                        }
                imp = fsimap.imap_wsp[(iag_num + 1)].imap_data;
                }  /* end first touch to this IAG */

        ino = inum &(INOSPERIAG - 1);
        extno = ino >> L2INOSPEREXT;
        bitno = ino & (INOSPEREXT - 1);

        /* process it only if the state is not determined */
        if ( !(imp->wmap[extno] & (UZBIT_32 >> bitno )))
        {
                /* update pmap according to val,
                 * set wmap to indicate state is determined.
                 */
                imp->wmap[extno] |= (UZBIT_32 >> bitno );
                if ( val )
                {
                        imp->pmap[extno] |= (UZBIT_32 >> bitno );
                        imp->inoext[extno] = inopxd;  /* ???? */
                }
                else
                {
                        imp->pmap[extno] &= ~(UZBIT_32 >> bitno );
                }

        }
        return (0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        saveExtDtPg()
 *
 * FUNCTION:    Add an entry to the list of extended dtpages
 *                   for the dtpage whose length and offset are given.
 *
 */
int saveExtDtPg( int32 pageVol, int64 pageOff )
{
        int rc = 0;
        struct ExtDtPg *edpp;
        int32 allocated_from_bmap = 0;
                /*
                 * if there are no available records,
                 * allocate some storage
                 */
        if( FreeExtDtPg == 0 ) {
                sprintf( message_parm_0, "(d) %d", PSIZE );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_ALLOC4EXTDTPG, 0, 1 );

                rc = alloc_storage( 	(uint32) PSIZE, 
					(void **) &DtPgPage,
					&allocated_from_bmap 
					);
                if( (rc != 0) || (DtPgPage == NULL) ) {  /*
				* ExtDtPg record allocation failed 
				*/
                        sprintf( message_parm_0, "(d) %lld", PSIZE );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_ALLOC4EXTDTPGFAIL, 0, 1 );
                        return(ENOMEM7);
                        }  /* end ExtDtPg record allocation failed */
                else if( DtPgPage != NULL ) {  /* ExtDtPg record allocation successful */
                        if( allocated_from_bmap ) {
                                fsck_send_msg( lrdo_USINGBMAPALLOC4EDPG, 0, 0 );
                                }
                        }  /* end ExtDtPg record allocation successful */
                FreeExtDtPg = PSIZE/sizeof(struct ExtDtPg);
                }
	/*
	 * allocate a block, assign the given values to it,
	 * and add it to the list
	 */
        edpp = DtPgPage;
        numExtDtPg++;
        DtPgPage++;
        FreeExtDtPg--;
        edpp->pg_vol = pageVol;
        edpp->pg_off = pageOff;
        edpp->next = DtPgList;
        DtPgList = edpp;

        return(0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *
 * NAME:        updatePage(ld)
 *
 * FUNCTION:     This routine performs the following functions:
 *
 *      1) APPLY LOGREC DATA (in buffer afterdata[]) 
 *
 *           This is copied into the disk page specified by redopage.pxd.
 *
 *           We apply only the portion(s) of the disk page not already
 *           updated by a log record in the current logredo session.
 *           logredo sees the disk page as a series of segments, and 
 *           handles the segments affected by the current logrec data
 *           individually.
 *
 *           It is possible that, if the data in a particular log record
 *           contains several segments, the segments may overlay each
 *           other.  In order to insure that the last (in time) image
 *           logged is the image applied by logredo, the segments in
 *           the log record are read in right-to-left order.  (That is,
 *           in last-in-first-out or LIFO order.)
 *
 *           The doblk fields summary1 and summary2 track updates to
 *           page segments.  These are marked as the log record data 
 *           is applied.  This ensures that data logged earlier (which
 *           is processed later by logredo) for a particular segment
 *           will be ignored.          
 *
 *      2) INITIALIZE/RESET DTREE FREELIST 
 *
 *      3) UPDATE THE AGGREGATE BLOCK MAP
 *
 *           We update the block map for extents described by the 
 *           xadlist in an xtree or xtree_root page.
 *
 *           We reset the XAD_NEW|XAD_EXTENDED flags in the xad in
 *           case they happen to be on.  (The block map has been updated
 *           regardless of their state.)
 *
 *      4) IF log.redopage.type == LOG_INODE
 *
 *           then all work necessary is finished with this log record 
 *           because all the information needed is contained in the
 *           log record data area.  
 *
 *           Specifically:
 *             -  update the inode base image. This is needed even for an
 *                inode which is being released in order to set nlink = 0.
 *             -  update the Inode Allocation Map (imap) if the inode
 *                is being allocated or released.
 *             -  update the Aggregate Block Map (bmap) if a new inode
 *                extent is allocated.
 *
 * NOTE:  o The field lrd.length describes the redopage data area only.
 *          That is, it does not include the length of the lrd itself.
 *
 *
 *        o Since the slot size differs, log.redopage.l2linesize
 *          contains log2 of the slot size for the current log record.
 *
 *
 *        o As transactions are processed by the JFS IFS, they are
 *          logged from left to right (physically) in the log file,
 *          but that because we process them in LIFO (last-in-first-out)
 *          order, logredo reads and processes them from right to left
 *          (physically)  This applies to both the transactions in the 
 *          log and to the records in each transaction.
 *             N.B. The exception to this is that the log is circular 
 *                  and may wrap during a particular session.
 *
 *          A log record consists of 3 parts:
 *                             type independent area (16 bytes)
 *                             type dependent area   (20 bytes)
 *                             data area             (variable length)
 *          and, although they are processed in the order listed, they
 *          occur physically (in the log file) in the opposite order.
 *          That is, if you were to examine the storage in a log page,
 *          reading from left to right, a general log record would
 *          appear 
 *               <data area><type dependent area><type independent area>
 *
 *
 *        o The FORMAT OF THE REDOPAGE DATA AREA depends on the 
 *          redopage type:
 *
 *          REDOPAGE:DIRECTORY data format
 *            a series of 1 or more <dtSegment>
 *
 *            each <dtSegment> is <segmentData><segmentDescriptor>
 *
 *            <segmentDescriptor> is <segmentOffset><segmentLength>
 *                          *** These are expressed in number of slots.
 *                              Stored as <int16><int16>
 *                          *** A directory slot is 32 bytes in length
 *                          *** slot[0] begins at beginning of the 
 *                              directory tree page specified by pxd.
 *            <segmentData> is the byte image of <segmentLength>
 *                          slots, starting with <segmentOffset> slot
 *                          in the directory node (or root) specified.
 *
 *            N.B. The <dtSegment>s may overlay slots.
 *           ======
 *                 FOR EXAMPLE:  Suppose we need to insert an entry
 *                 into the first half of a particular directory page
 *                 which does not have enough available slots to 
 *                 accomodate it.
 *                 - We split the existing directory page, moving 1/2 
 *                   the entries into a new page 
 *                 - We free the slots which the moved entries occupied
 *                   before the split.
 *                 - We insert the new entry, using the (newly) free
 *                   slot(s).
 *                 Thus some slots have been freed and then allocated
 *                 again, all in a single transaction.
 *
 *                 When a transaction is logged, the segments are written
 *                 as they occur, from left to right in the log.
 *                 (So in the example, the 'free slot(s) segment' would 
 *                 be written, then the 'allocate slot(s) segment' would
 *                 be written.) 
 * 
 *                 Since logredo processes the log from right to left
 *                 (i.e. even the records with a transaction are processed
 *                 from right to left),  it processes the 'allocate 
 *                 slot(s) segment', noting the slots which have been
 *                 updated.  Then logredo encounters the 'free slot(s)
 *                 segment' and ignores updates to slots which have
 *                 already been updated.
 *
 *          REDOPAGE:XTREE data format
 *            a series of 1 or more <xtSegment>
 *
 *            each <xtSegment> is <segmentData><segmentDescriptor>
 *
 *            <segmentDescriptor> is <segmentOffset><segmentLength>
 *                          *** These are expressed in number of slots.
 *                              Stored as <int16><int16>
 *                          *** An index tree slot is 16 bytes in length
 *                          *** slot[0] begins at beginning of the 
 *                              index tree page specified by pxd.
 *            <segmentData> is the byte image of <segmentLength>
 *                          slots, starting with <segmentOffset> slot
 *                          in the directory node (or root) specified.
 *
 *            N.B. The data area of this type of log record contains,
 *                 at most, two segments.  The header segment is
 *                 always included and logged as one segment (even if
 *                 the slot(s) described by the second segment are
 *                 contiguous to the header segment).  There may
 *                 also be a second segment containing the after-image
 *                 of non-header slot(s).
 *          
 *          REDOPAGE:INODE data format
 *            a series of 1 or more <iSegment>
 *
 *            each <iSegment> is <segmentData><segmentDescriptor>
 *
 *            <segmentDescriptor> is <segmentOffset><segmentLength>
 *                          *** These are expressed in number of slots.
 *                              Stored as <int16><int16>
 *                          *** The length of an inode slot in this 
 *                              record type is 128 bytes
 *                          *** slot[0] begins at beginning of the 
 *                              (4096 byte) inode extent page
 *                              specified by pxd.
 *            <segmentData> is the byte image of <segmentLength>
 *                          slots, starting with <segmentOffset> slot
 *                          in the inode extent page specified.
 *
 *            N.B. In this type of log record, redopage.inode is the 
 *                 inode number of the inode which owns the IAG which
 *                 describes the extent which contains the inode
 *                 actually being changed.  (For example, an update
 *                 to inode x in fileset 0 would show lrd.inode==16
 *                 because aggregate inode 16 is the Inode Allocation
 *                 Table inode for fileset 0.)  
 *
 *                 Also, lrd.pxd defines the (4096 byte) page of
 *                 storage which contains the inode being changed.
 *
 *                 Since each inode extent contains 4 pages, this may
 *                 or may not also be the first page of an inode extent.
 *
 *                 Since each page contains 8 inodes, some more work is
 *                 needed to determine the inode number of the inode
 *                 actually being changed.
 *
 *            N.B. In this type of log record data, there may be several
 *                 segments.  Each segment is either the base image of
 *                 one inode or the byte image of one inlineEA.
 *
 *                 If a particular segment describes an inode base image,
 *                 then   <segmentOffset> / 4 has no remainder
 *                 else if the segment describes an inlineEA
 *                 then   <segmentOffset> / 4 has remainder 3
 *
 *          REDOPAGE:BTROOT|XTREE data format
 *            a series of 1 or more <iSegment>
 *
 *            each <iSegment> is <segmentData><segmentDescriptor>
 *
 *            <segmentDescriptor> is <segmentOffset><segmentLength>
 *                              INDEX TREE slots.
 *                              Stored as <int16><int16>
 *                          *** The length of an index tree slot  
 *                              16 bytes
 *                          *** slot[0] begins at di_btroot (32 bytes
 *                              before Section III of the dinode)
 *            <segmentData> is the byte image of <segmentLength>
 *                          slots, starting with <segmentOffset> slot
 *                          in the inode extent page specified.
 *
 *            N.B. In this type of log record, redopage.inode is the 
 *                 inode number of the inode actually being changed.
 *
 *                 Also, lrd.pxd defines the (4096 byte) page of
 *                 storage which contains the inode being changed.
 *
 *                 Since each inode extent contains 4 pages, this may
 *                 or may not also be the first page of an inode extent.
 *
 *                 Since each page contains 8 inodes, some more work is
 *                 needed to determine the starting offset (within the
 *                 page specified by pxd) of the inode actually being
 *                 changed.
 *
 *          REDOPAGE:BTROOT|DTREE data format
 *            a series of 1 or more <iSegment>
 *
 *            each <iSegment> is <segmentData><segmentDescriptor>
 *
 *            <segmentDescriptor> is <segmentOffset><segmentLength>
 *                          *** These are expressed in number of 
 *                              DIRECTORY TREE slots.
 *                              Stored as <int16><int16>
 *                          *** The length of a directory tree slot  
 *                              is 32 bytes
 *                          *** slot[0] begins at di_btroot (32 bytes
 *                              before Section III of the dinode)
 *            <segmentData> is the byte image of <segmentLength>
 *                          slots, starting with <segmentOffset> slot
 *                          in the inode extent page specified.
 *
 *            N.B. In this type of log record, redopage.inode is the 
 *                 inode number of the inode actually being changed.
 *
 *                 Also, lrd.pxd defines the (4096 byte) page of
 *                 storage which contains the inode being changed.
 *
 *                 Since each inode extent contains 4 pages, this may
 *                 or may not also be the first page of an inode extent.
 *
 *                 Since each page contains 8 inodes, some more work is
 *                 needed to determine the starting offset (within the
 *                 page specified by pxd) of the inode actually being
 *                 changed.
 *
 *          REDOPAGE:DATA (i.e. inline data) data format
 *            a series of 1 or more <iSegment>
 *
 *            each <iSegment> is <segmentData><segmentDescriptor>
 *
 *            N.B. This type is used for inline data, currently only
 *                 defined for symbolic links (symlinks) which 
 *                 are ** NOT IMPLEMENTED IN RELEASE I OF JFS/WARP **
 *
 *
 *        o Since 
 *             - logredo only updates the first segment of an inode
 *               by applying a REDOPAGE:INODE log record
 *             - a REDOPAGE_INODE log record pxd describes a page
 *               in an inode extent (that is, 4096 bytes containing
 *               8 inodes)
 *             - a REDOPAGE_INODE log record contains the inode number
 *               of the Inode Alloc Table, not the affected inode
 *               (indeed there may be more than 1 affected inode in
 *               the data for the record)
 *             - segmentData contains only the after-image of the
 *               inode(s) being changed
 *          logredo cannot determine which, if any, inode(s) in the 
 *          page have just been allocated (nlink was 0, now is not)
 *          and/or have just been released (nlink was not 0, now is).
 *
 *          Therefore, for the purposes of updating the Inode Allocation
 *          Map, logredo assumes that all inodes in the page which
 *          are allocated, are newly allocated, and all inodes in the
 *          page which are not allocated have just been released.
 *                       
 */
updatePage(
struct lrd *ld,
int32  logaddr)
{
        int32 vol, rc;
        int *buf;
        struct doblk *db;
        uint8 mask_8, xtroot_lwm;
        uint16 mask_9, dtroot_9;
        uint32 mask_32,dtpg_32;
        int16 l2linesize;      /* log2 of the slot size */
        int32  i,j,k, seglen,linesize,segnum, inonum, flist;
        int32  iag_num, ino, extno;
        int16  off,ln, inoext_alloc, allocate, delta_len, s_index, ino_rem;
        int16 *segdata, wbitno, nbits, nslots, size_dinode;
        caddr_t data,buf_ptr, buf_btroot;
        xad_t *xad_p;
        pxd_t ino_pxd1, pxd1;
        iag_data_t *imp;
        dtslot_t        *f;
        dtroot_t        *dtrt;
        struct dinode *dip;
        uint16  free = 0;
        int32  xlen, xlength;
        int16 nword;
        int8 upd_possible = 0;

        if (ld->length <= 0)
                return (0);

        vol = 0;

        /* 
         * segdata points to the end of afterdata 
         */
        size_dinode = sizeof(dinode_t);
        segdata = (int16 *)((caddr_t)afterdata + ld->length);
        l2linesize = ld->log.redopage.l2linesize;
        linesize = 1 << l2linesize;
        j = 0;
        seglen = 0;
        segnum = 0;

        /*
         * find doblk for the specified pxd
         */
        if ((rc = findPageRedo(ld->aggregate, ld->log.redopage.fileset,
                                 ld->log.redopage.pxd, &db)) !=0)  {
                sprintf( message_parm_0, "(d) %d", rc );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_UPPGFNDPGREDOFAIL, 0, 1 );
                return(rc);
                }

        /*
         * check to see if there is anything to do for this page
         */
        if ( ld->log.redopage.type == LOG_INODE )
        {
                if ( db->db_ibase == 0xFF &&
                        db->db_iea == 0xFF )  /* for each inode
                            * in the page, both the base image and the
                            * EA have already been refreshed in this
                            * logredo session
                            */
                        return (0);
        }
        else   /*
                *
                * Dtree Root Node
                *
                */
        if ( (ld->log.redopage.type & (LOG_BTROOT|LOG_DTREE))
                        == (LOG_BTROOT|LOG_DTREE) )
        {
           /* 
            * log.redopage.inode is the inode number of the inode
            * whose tree will be refreshed.  
            * inonum is the position of that inode in the inode 
            * extent (4096 byte) page described by the pxd. 
            * (Each page has 8 inodes.)
            */
           inonum = ld->log.redopage.inode & 0x07;

             /*
              * This inode is identified as a dtroot.  So, first
              * mark the appropriate slot in the noredopage for 
              * NO updates to this inode's root as an xtree.
              */
           db->db_xtroot[inonum].xtrt_lwm = XTENTRYSTART;
           db->db_xtroot[inonum].xtrt_hd = 1;

           dtroot_9 = db->db_dtroot[inonum];
           if ( dtroot_9 == 0x01ff )  /* all slots in the dtree root
                            * have already been refreshed in this 
                            * logredo session
                            */
                return (0);
        }
        else   /*
                *
                * Xtree Root Node
                *
                */
        if ( (ld->log.redopage.type & (LOG_BTROOT|LOG_XTREE))
                        == (LOG_BTROOT|LOG_XTREE) )
        {
           /* 
            * log.redopage.inode is the inode number of the inode
            * whose tree will be refreshed.  
            * inonum is the position of that inode in the inode 
            * extent (4096 byte) page described by the pxd. 
            * (Each page has 8 inodes.)
            */
           inonum = ld->log.redopage.inode & 0x07;
           /*
            * This inode is identified as an xtroot.  So, first
            * mark the appropriate slot in the noredopage for 
            * NO updates to this inode's root as a dtree.
            */
           db->db_dtroot[inonum] = 0x01ff;

           xtroot_lwm = db->db_xtroot[inonum].xtrt_lwm;
           if ( (xtroot_lwm == XTENTRYSTART ) &&
                         db->db_xtroot[inonum].xtrt_hd )  /* 
                            * The header slot and at least the 1st 
                            * slot after the header have already
                            * been refreshed in this logredo session.
                            */
                return (0);
        }
        else   /*
                *
                * Dtree Non-Root Node
                *
                */
        if ((ld->log.redopage.type & LOG_DTREE ) == LOG_DTREE)
        {
             /*
              * This page is identified as a dtpage.  So, first
              * mark the appropriate slots in the noredopage for 
              * NO updates to this page as an xtpage.
              */
            db->db_xtpagelwm = XTENTRYSTART;
            db->db_xtpghd = 1;
                /* 
                 * This extent may not be a full page (4096 bytes in length)
                 *
                 * Figure out its size, the number of slots it
                 * covers, the number words used to track those slots,
                 * and the number of bits in the last of those words.
                 */
                xlen = lengthPXD((&ld->log.redopage.pxd));
                xlength = xlen << vopen[vol].l2bsize;
                nslots = xlength >> L2DTSLOTSIZE;  
                nword = nslots >> L2DBWORD;
                nbits = nslots & (DBWORD - 1);
                upd_possible = 0;
                for ( i = 0; i < nword; i++)  {
                        if( db->db_dtpagewd[i] != ONES ) {
                                upd_possible = -1;
                                }
                        }
                if ( nbits )
                {
                   i = DBWORD - nbits;
                   mask_32 = ((uint32)( ONES >> i )) << i;
                   if( db->db_dtpagewd[nword] != mask_32 )  {
                       upd_possible = -1;
                       }
                }

            if ( ! upd_possible )  {  /*  all slots in the dtree node have
                            * already been refreshed in this logredo session.
                            */
                   return (0);
                   }
        }
        else    /*
                *
                * Xtree Non-Root Node
                *
                */
        if ((ld->log.redopage.type & LOG_XTREE) == LOG_XTREE)
        {
                 /*
                  * This page is identified as an xtpage.  So, first
                  * mark the appropriate slots in the noredopage for 
                  * NO updates to this page as a dtpage.
                  */
                db->db_dtpagewd[0] = ONES;
                db->db_dtpagewd[1] = ONES;
                db->db_dtpagewd[2] = ONES;
                db->db_dtpagewd[3] = ONES;

                if ((db->db_xtpagelwm == XTENTRYSTART)  && 
                    db->db_xtpghd ) /* the header slot and at least
                            * the first slot after the header have been
                            * refreshed in this logredo session.
                            */
                        return (0);
        }

            /*
             *
             * There is actually some work to do
             *
             */
        buf = NULL;
        inoext_alloc = 0;   /* init ino extent allocation state */
        while ( j < ld->length )  /* while data segments unprocessed */
        {
                ln = *--segdata;   /* get length */
                off = *--segdata;  /* get offset */
                segnum++;
                seglen  = ln << ld->log.redopage.l2linesize;
                  /*
                   * segdata points to the beginning of the segment
                   */
                segdata = (int16 *)((caddr_t)segdata - seglen);
                data = (caddr_t)segdata;
                j +=  seglen + 4 ;

                   /*
                    *
                    * INODE
                    *
                    */
                if ( (ld->log.redopage.type & LOG_INODE) == LOG_INODE )
                {
                   /* check to see if the current inode base image has
                    * been updated by an earlier log rec
                    * Note: we don't need to check if the current inode
                    * base image is on noredofile hash chain since the
                    * doblk does the same job for us. An inode is on the
                    * noredofile hash chain only if it is processed by
                    * an earlier log rec
                    */
                   inonum = off >> 2;  /* inode seq. no in the inode page
                                             zero origin */
                   ino_rem = off & 3;
                   if ( !ino_rem) /* don't update dip if the segment is
                                   * for inlineEA.  When an update
                                   * involves EA, the base inode image
                                   * is always out too, 
                                   * so dip can always catch some
                                   * base inode image data
                                   */
                        dip = (struct dinode *)data;

                   mask_8 = UZBIT_8 >> inonum ;
                   if ( ino_rem == 3 ) /* it is inlineEA segment  */
                   {
                        if (db->db_iea & mask_8)  /* already updated */
                                continue;
                   }
                   else if ( ino_rem == 0 )  /* inode base segment  */
                   {
                        if ( db->db_ibase & mask_8 )  /* already updated */
                                 continue;
                   }
                   else  {
                        sprintf( message_parm_0, "(d) %d", off );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_UPPGBADINODESEGOFFSET, 0, 1 );
                        }
                   /* 
                    * read the inode extent page into a buffer
                    */
                   if (buf== NULL)
                   {
                        rc = bread(vol, ld->log.redopage.pxd, (void **)&buf,PB_UPDATE);
                        if (rc)
                        {
                           sprintf( message_parm_0, "(d) %d", rc );
                           msgprms[0] = message_parm_0;
                           msgprmidx[0] = 0;
                           fsck_send_msg( lrdo_UPPGBREADFAIL1, 0, 1 );
                           return(INOEXT_READERROR1);
                        }
                   }
                   if ( !ino_rem)   /* inode base image  */
                   {
                        /* 
                         * refresh the appropriate slot in the 
                         * inode extent page in the buffer with
                         * the data in the current segment.
                         */
                        buf_ptr = (caddr_t)buf+(off<<l2linesize);
                        memcpy(buf_ptr, data, linesize );
                        db->db_ibase |= mask_8;
                        allocate = (dip->di_nlink != 0 );  /* 
                                        * is this inode allocated?
                                        */
                        inoext_alloc |= allocate;  /* inoext_alloc
                                         * will be nonzero if at least
                                         * one inode in the page is
                                         * allocated.  In this case,
                                         * we will mark block map to 
                                         * show the whole inode extent
                                         * (all 4 pages) as allocated 
                                         */
                        /* 
                         * In RELEASE I OF JFS/WARP, there is only
                         * one fileset per aggregate, so the inode
                         * extent is always owned by inode FILESYSTEM_I
                         */
                        if ( ld->log.redopage.inode == FILESYSTEM_I )
                        {
                                rc = markImap(vopen[vol].fsimap_lst, dip->di_number,
                                        dip->di_ixpxd, allocate, vol );
                                if( rc ) {
                                        sprintf( message_parm_0, "(d) %d", rc );
                                        msgprms[0] = message_parm_0;
                                        msgprmidx[0] = 0;
                                        fsck_send_msg( lrdo_UPPGMIMPFAIL, 0, 1 );
                                        return( rc );
                                        }
                        }
                        /* if zero link count, start NoRedoFile filter
                         * Note: for the first release, we ensure that the
                         * nlink for the aggregate inode FILESYSTEM_I will
                         * be non-zero. If more filesets are processed in
                         * the later release, then nlink could be zero for
                         * the aggregate inodes.
                         * since NoRedoFile filter carries both fileset
                         * number and inode number, so it is able to handle
                         * both aggregate level and fileset level inode.
                         */

                        if (!allocate)
                                doNoRedoFile(ld, dip->di_number);
                   }
                   else  /* inlineEA data section  */
                   {
                        buf_ptr = (caddr_t)buf+(off<<l2linesize);
                        memcpy(buf_ptr, data, linesize );
                        db->db_iea |= mask_8;
                   }
                }
                else
                   /*
                    *
                    * Xtree Root Node
                    *
                    */
                if ((ld->log.redopage.type & (LOG_BTROOT|LOG_XTREE))
                                == (LOG_BTROOT|LOG_XTREE) )
                {
                    /*
                     * This inode is identified as an xtroot.  So, first
                     * mark the appropriate slot in the noredopage for 
                     * NO updates to this inode's root as a dtree.
                     */
                   db->db_dtroot[inonum] = 0x01ff;

                   if ( (off != 0 && xtroot_lwm <= (uint8)off ) ||
                        (off == 0 && db->db_xtroot[inonum].xtrt_hd ))
                        continue;

                   /* 
                    * read in the inode extent page 
                    */
                   if (buf== NULL)
                   {
                        rc = bread(vol, ld->log.redopage.pxd, 
                                        (void **)&buf,PB_UPDATE);
                        if (rc)
                        {
                           sprintf( message_parm_0, "(d) %d", rc );
                           msgprms[0] = message_parm_0;
                           msgprmidx[0] = 0;
                           fsck_send_msg( lrdo_UPPGBREADFAIL2, 0, 1 );
                           return(INOEXT_READERROR2);
                        }
                   }
                   /*
                    * for BTROOT ( either xtree or dtree ), off starts
                    * from di_btroot, not from the beginning of inode,
                    * not from beginning of page
                    * slot number is zero origin. di_btroot starts
                    * from slot #14 for xtroot
                    *
                    * First let buf_ptr points to the beginning of off
                    */
                   buf_btroot = (caddr_t)buf+ size_dinode * inonum
                                                 + (14 << l2linesize );
                   /* if xtroot has taken the inode section IV,
                    * indicate in db_iea so that later inlineEA data
                    * won't apply to it
                    */
                   if ( (off + ln) > 10)
                   {
                        mask_8 = UZBIT_8 >> inonum ;
                        db->db_iea |= mask_8;
                   }
                   if ( off == 0 )
                   {
                         /* header segment and the first time
                          */
                        memcpy((caddr_t)buf_btroot, data, ln << l2linesize );
                        db->db_xtroot[inonum].xtrt_hd = 1;
                   }
                   else
                   {
                        if ( xtroot_lwm == 0xFF ) /* the first time */
                           delta_len = ln;
                        else
                           delta_len = xtroot_lwm - off;

                        xad_p = (xad_t *)data ;
                        for ( i = 0; i < delta_len; i++)
                        {
                           MARKXADNEW(pxd1, xad_p, vopen[vol].bmap_ctl, vol);
                           xad_p += 1;
                        }
                        buf_ptr = buf_btroot + ( off<< l2linesize );
                        memcpy((caddr_t)buf_ptr, data,
                                        delta_len << l2linesize );
                        db->db_xtroot[inonum].xtrt_lwm = xtroot_lwm = off;
                    }
                }
                else
                   /*
                    *
                    * Xtree Non-Root Node
                    *
                    */
                if ( (ld->log.redopage.type & LOG_XTREE) == LOG_XTREE )
                {
                    /*
                     * This page is identified as an xtpage.  So, first
                     * mark the appropriate slots in the noredopage for 
                     * NO updates to this page as a dtpage.
                     */
                   db->db_dtpagewd[0] = ONES;
                   db->db_dtpagewd[1] = ONES;
                   db->db_dtpagewd[2] = ONES;
                   db->db_dtpagewd[3] = ONES;

                        /*
                         * log rec for xtree has two segments.
                         * the first is the header. The second
                         * segment for xtpage slots update.
                         *
                         * The log rec data area contains a list of
                         * xad's. Each XAD may has a flag of XAD_NEW
                         * or XAD_EXTENDED. If it shows up, then
                         * logredo needs to reset the flag and mark
                          on bmap, then update the on-disk xtree image.
                         */

                   /* if the lwm less than offset or it is a header
                    * segment but header had been updated previously,
                    * ignore this segment.
                    * Note: every log rec has the header data. But xtpage
                    * header should be only updated once at logredo.
                    */
                   if ( (off != 0 && db->db_xtpagelwm <= (uint8)off) ||
                        ( off == 0 && db->db_xtpghd ))
                         continue;

                   /* read the on-disk page into buffer pool.
                    */
                   if (buf== NULL)
                   {
                        rc = bread(vol, ld->log.redopage.pxd, (void **)&buf,PB_UPDATE);
                        if( rc )
                        {
                           sprintf( message_parm_0, "(d) %d", rc );
                           msgprms[0] = message_parm_0;
                           msgprmidx[0] = 0;
                           fsck_send_msg( lrdo_UPPGBREADFAIL3, 0, 1 );
                           return(XTPAGE_READERROR1);
                        }
                   }

                   if ( off == 0 )
                         /* it is header segment and the first time
                          */
                   {
                        memcpy((caddr_t)buf, data, ln << l2linesize );
                        db->db_xtpghd = 1;
                   }
                   else
                   {
                        if ( db->db_xtpagelwm == 0xff ) /* the first time */
                           delta_len = ln;
                        else
                           delta_len = db->db_xtpagelwm - off;
                        xad_p = (xad_t *)data ;
                        for ( i = 0; i < delta_len; i++)
                        {
                           MARKXADNEW(pxd1,xad_p,vopen[vol].bmap_ctl,vol);
                           xad_p += 1;
                        }
                        buf_ptr = (caddr_t)buf + (off << l2linesize);
                        memcpy((caddr_t)buf_ptr, data,
                                        delta_len << l2linesize );
                        db->db_xtpagelwm = (uint8)off;
                    }
                }
                else 
                   /*
                    *
                    * Dtree Node -- Root and Non-Root
                    *
                    */
                {
                   /* read the on-disk page into buffer pool.  */
                   if (buf== NULL)
                   {
                        rc = bread(vol, ld->log.redopage.pxd, (void **)&buf,PB_UPDATE);
                        if( rc )
                        {
                           sprintf( message_parm_0, "(d) %d", rc );
                           msgprms[0] = message_parm_0;
                           msgprmidx[0] = 0;
                           fsck_send_msg( lrdo_UPPGBREADFAIL4, 0, 1 );
                           return(DTREE_READERROR1);
                        }
                   }
                   if ((ld->log.redopage.type & (LOG_BTROOT|LOG_DTREE))
                                == (LOG_BTROOT|LOG_DTREE) )
                   {

                        /*
                         * This inode is identified as a dtroot.  So, first
                         * mark the appropriate slot in the noredopage for 
                         * NO updates to this inode's root as an xtree.
                         */
                      db->db_xtroot[inonum].xtrt_lwm = XTENTRYSTART;
                      db->db_xtroot[inonum].xtrt_hd = 1;
                         
                      /*
                       * If more than one slot in a segment, we apply
                       * slots one at a time.
                       * for BTROOT ( either xtree or dtree ), off starts
                       * from di_btroot, not from the beginning of inode,
                       * not from beginning of page
                       * slot number is zero origin. di_btroot starts
                       * from slot #8 for dtroot.
                       *
                       * First let buf_ptr points to the beginngin of off
                       */

                      buf_btroot = (caddr_t)buf+ size_dinode * inonum
                                + (7 << l2linesize );
                      buf_ptr = buf_btroot + ( off<< l2linesize );
                      for ( i = 0; i < ln; i++)
                      {
                           mask_9 = UZBIT_16 >> (7+(off & 0x000f));

                           if ( !(mask_9 & dtroot_9))
                           {
                                memcpy((caddr_t)buf_ptr, data, linesize );
                                dtroot_9 |= mask_9 ;
                           }
                           buf_ptr += linesize;
                           data += linesize;
                           off++;  /* off incremented for next slot if ln > 1
                                    * In the dtroot case, ln will never > 9
                                    */
                       }
                       db->db_dtroot[inonum] = dtroot_9;
                   }
                   else
                   if ((ld->log.redopage.type & LOG_DTREE) == LOG_DTREE )
                   {
                        /*
                         * This page is identified as a dtpage.  So, first
                         * mark the appropriate slots in the noredopage for 
                         * NO updates to this page as an xtpage.
                         */
                       db->db_xtpagelwm = XTENTRYSTART;
                       db->db_xtpghd = 1;

                       /* update bits of words in dtpage_word[]. starting
                        * from "off" and cover a range of "ln".
                        * Each word tracks 32 slots. The first and last words
                        * may only have a subset of their bits updated.
                        */

                       s_index = off >> L2DTPGWORD; /* start word in
                                                      dtpage_word[].*/
                       dtpg_32 = db->db_dtpagewd[s_index];

                       for (nslots =ln; nslots >0; nslots -=nbits,off +=nbits)
                       {
                          /* determine the start bit number within the word
                           * and the number of bits to be updated within
                           * the word
                           */
                           wbitno = off &(DTPGWORD - 1);
                           nbits = MIN(nslots, DTPGWORD - wbitno);

                           for (i = 0, k = wbitno; i < nbits ; i++,k++)
                           {
                              mask_32 = UZBIT_32 >> k;
                              if ( !(mask_32 & dtpg_32 ))
                              {
                                  buf_ptr = (caddr_t)buf +
                                        (off<<l2linesize) + (i<<l2linesize);
                                  memcpy((caddr_t)buf_ptr, data, linesize );
                                  dtpg_32 |= mask_32;
                              }
                              data += linesize;
                           }
                           db->db_dtpagewd[s_index] = dtpg_32;
                           dtpg_32 = db->db_dtpagewd[++s_index];
                       }
                       db->db_dtpagewd[s_index] = dtpg_32;
                   }
                   else
                   if ( (ld->log.redopage.type & LOG_DATA) == LOG_DATA)
                                ;
                            /* TBD */

           }
        }  /* end of while ( j < ld->length )  */


        /* for LOG_INODE, we need to process bmap because we need
	 * to be absolutely certain that the inode extent is allocated.
         *
         * This log record may have several segments, each for a
         * different inode (base image), however, these inodes MUST
         * all reside in the same inode extent page to be included
         * in the same log record.  Therefore, they MUST belong to the
         * same inode extent (4 pages, each 4096 bytes).  Therefore,
         * it doesn't matter which segment we use to determine the
         * particulars about the extent.
         *
         */
        if ( ld->log.redopage.type & LOG_INODE )
        {
		    /* 
		     * If at least 1 inode in the current log record is
		     * marked as allocated, then we must be sure that
		     * the inode extent containing it (them) is allocated
		     * and properly hooked into the IAG.
		     *
		     * Since logredo processes log records LIFO and
		     * processes the base image of any inode only once,
		     * we are assured that the inode status is final
		     * (for this logredo session).
                     */
		if ( inoext_alloc)
		{
			/*
			 * If it IS a fileset-owned inode extent
			 * (and not an aggregate-owned inode extent)
			 */
		    if( ld->log.redopage.inode == FILESYSTEM_I ) {  
			    /*
			     * figure out which IAG and which extent
			     */
			iag_num = INOTOIAG(dip->di_number);

			imp = vopen[vol].fsimap_lst.imap_wsp[ (iag_num + 1)
                                                                                    ].imap_data;
			if( imp == NULL ) {  /* first touch to this IAG */
			        rc = iagGet( vol, iag_num );
			        if( rc != 0 ) {
                                                return( rc );
			                }
			        imp = vopen[vol].fsimap_lst.imap_wsp[ (iag_num + 1)
                                                                                            ].imap_data;
			        }  /* end first touch to this IAG */

			ino = dip->di_number & (INOSPERIAG - 1);
			extno = ino >> L2INOSPEREXT;
			    /*
			     * make sure the IAG points to it correctly
			     */
			imp->inoext[extno] = dip->di_ixpxd;
			}  /* end fileset owned */
			/*
			 * make sure the block map shows it allocated
			 */
		    rc = markBmap((dmap_t *)vopen[vol].bmap_ctl,
                                                  dip->di_ixpxd, 1, vol);
		    if( rc ) {
			sprintf( message_parm_0, "(d) %d", rc );
			msgprms[0] = message_parm_0;
			msgprmidx[0] = 0;
			fsck_send_msg( lrdo_UPPGMBMPFAIL, 0, 1 );
			return( rc );
			}
		}
        }
        /*
         * init freelist for a new dtroot 
         */
        if(ld->log.redopage.type == (LOG_DTREE | LOG_BTROOT | LOG_NEW))
        {
                rc = dtrt_resetFreeList(vol, db, ld, buf_btroot );
                if( rc ) {
                        sprintf( message_parm_0, "(d) %d", rc );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_UPPGDTRTRFLFAIL, 0, 1 );
                        return( rc );
                        }
        }
        /*
         * init freelist for a new dtpage 
         */
        if( ld->log.redopage.type == (LOG_DTREE | LOG_NEW) )
        {
                rc = dtpg_resetFreeList( vol, buf);
                if( rc ) {
                        sprintf( message_parm_0, "(d) %d", rc );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_UPPGDTPGRFLFAIL, 0, 1 );
                        return( rc );
                        }
        }
        /*
         * make a note to rebuild freelist for an extended dtpage 
         */
        if( ld->log.redopage.type == (LOG_DTREE | LOG_EXTEND) )
        {
                rc = saveExtDtPg( vol, addressPXD( &(ld->log.redopage.pxd) ) );
                if( rc ) {
                        sprintf( message_parm_0, "(d) %d", rc );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_UPPGSEDPFAIL, 0, 1 );
                        return( rc );
                        }
        }

        return (0);
}
