/* $Id: logview.c,v 1.2 2004/03/21 02:43:13 pasha Exp $ */

static char *SCCSID = "@(#)1.4  7/31/98 08:23:19 src/jfs/utils/logview/jfs_logview.c, jfsutil, w45.fs32, 990417.1";
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
 *
 *	jfs_logview: display log ( This is an internal tool )
 *
 * Note: The difference between logview and logredo:
 *	 the logredo get a fs from fsck. logview needs to open device 
 *	 by itself.
 *	
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>


#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#include <os2.h>

#include "jfs_types.h"
#include <jfs_aixisms.h>
#include "sysbloks.h"
#include <extboot.h>
#include "bootsec.h"
#include "jfs_filsys.h"
#include "jfs_superblock.h"
#include "jfs_inode.h"
#include "jfs_dtree.h"
#include "jfs_xtree.h"
#include "jfs_logmgr.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_debug.h"
#include "devices.h"
// #include <sys/stat.h>
typedef long off_t;
      #pragma pack(2)
      struct stat {
         dev_t st_dev;
         ino_t st_ino;
         unsigned short st_mode;
         short st_nlink;
         short st_uid;
         short st_gid;
         dev_t st_rdev;
         unsigned short __filler;
         off_t st_size;
         time_t st_atime;
         time_t st_mtime;
         time_t st_ctime;
         };
      #pragma pack()


#define	LINESIZE	128

#define DTPGWORD	32
#define L2DTPGWORD	5
#define UZBIT_8		((uint8) (1 << 7))	
#define UZBIT_16	((uint16) (1 << 15 ))
#define	UZBIT_32	((uint32) (1 << 31 ))
#define min(A,B) ((A) < (B) ? (A) : (B))
#define BTOLOGPN(x)  ((unsigned)(x) >> L2LOGPSIZE)
#define LOGPNTOB(x)  ((x)<<L2LOGPSIZE)
#define MAKEDEV(__x,__y)        (dev_t)(((__x)<<16) | (__y))

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

/* The following defines are for aggregate block allocation map */
#define BMAPPAGESIZE    4096
#define PAGESIZE    	4096
#define L2OFFSET(l2lblksize)    ((BMAPPAGESIZE) >> l2lblksize)
#define L10OFFSET(l2lblksize)   (((BMAPPAGESIZE)<<1 ) >> l2lblksize)
#define L20OFFSET(l2lblksize)   (((BMAPPAGESIZE) * 3 ) >> l2lblksize)
#define DMAP0OFFSET(l2lblksize) (((BMAPPAGESIZE)<<2 ) >> l2lblksize)
#define SIZEOFDMAP	sizeof(dmap_t)
#define SIZEOFDMAPCTL	sizeof(dmapctl_t)

/*
 * At the logredo time, the dmap read into memory to form an array
 * of file pages. The first page is always the aggregate disk allocation
 * map descriptor ( i.e. the bmap control page), the remaining pages are
 * either dmap control pages or dmap pages. 
 * tlvl tells the zero origin dmapctl level of the top dmapctl.
 * when tlvl == 2, L2 page exists; 
 * when tlvl == 1, L2 does not exist, but L1.n and L0.n pages exist (0 <= n <= 1023); 
 * when tlvl == 0, L2 and L1.n pages do not exist, only L0.n pages exist(0<= n <= 1023); 
 */

/*
 * convert disk block number to the dmap file page index number (zero origin)
 * of the dmap describing the disk block.
 * tlvl is the zero origin dmapctl level of the top dmapctl
 */

#define BLKTODMAPPG(b,tlvl) \
   ( ((b) >> 13) + ((b) >> 23) + ((b) >> 33) + 1 +     \
                ( ((tlvl) == 2) ? 3 : ((tlvl) == 1) ? 2 : 1) )

/*
 * convert disk block number to the dmap file page index block number 
 * of the LEVEL 0 dmapctl describing the disk block.
 */
#define BLKTOL0PG(b,tlvl)  \
   (((((b) >> 23) << 10) + ((b) >> 23) + ((b) >> 33) + 1 +    \
               (((tlvl) == 2) ? 2 : ((tlvl) == 1) ? 1 : 0))

/*
 * convert disk block number to the dmap file page index block number 
 * of the LEVEL 1 dmapctl describing the disk block.
 *
 * Note: tlvl must be 2 or 1, cannot be 0
 */
#define BLKTOL1PG(b,tlvl)  \
   ( (((b) >> 33) << 20) + (((b) >> 33) << 10) + ((b) >> 33) + 1 +  \
               (((tlvl) == 2) ? 1 : 0) )

/* convert disk block number to allocation group number.
 */
#define BLKNOTOAG(b,l2agsize)   ((b) >> l2agsize)


#define PB_READ    0
#define PB_UPDATE  1	

/* error types
 */
#define MAPERR 	  0	
#define DBTYPE    1	
#define INOTYPE   2
#define READERR   3
#define UNKNOWNR  4
#define LOGEND    5
#define SERIALNO  6
#define OPENERR	  7	
#define IOERROR	  8	
#define LOGWRAPPED        9

#define MINOR_ERROR -1		/* error that will not abort the program */
#define MAJOR_ERROR -2		/* return code to force abort of logredo */
#define REFORMAT_ERROR -3	/* return code to force logform of log	 */

#define INLINELOG   21
#define OUTLINELOG  22

#define	FATAL(str)	fatal(str, __LINE__)
#define numeric(c)	(c >= '0' && c <= '9')

/* things for the map */

int16 top_dmapctl_lvl; /* zero origin dmapctl level of the top dmapctl */

/*
 * 	things for the log. 
 */
int16  loglocation = 0;         /* 1 = inlinelog, 2 = outlinelog */
int32    logmajor = 0;		/* major number of log device */
int32    logminor = 0;		/* minor number of log device */
int32    logserial;		/* log serial number in super block */	
int32    logend;		/* address of the end of last  log record */
int32    logstart;		/* address of last  log record */
int32    loglastp;		/* last page of log read       */	
int32    lognumread; 		/* number of log pages read    */
int32    logfd;			/* file descriptor for log */
int32    logsize;		/* size of log in pages */
logsuper_t logsup;                      /* log super block */
int32    logaddr;
int32    syncrecord;
int32    nlogrecords;

/*
 *	transaction hash table
 *
 * each entry represents a committed transaction in recovery-in-progress
 *
 * entry stays in com table until first record of transaction is found 
 * (it has backchain = 0).
 */
#define COMSIZE   512
int comfree; 			/* index of a free com structure */
int comhmask = 63; 		/* hash mask for comhash */  
int comhash[64];  		/* head of hash chains */
struct com
{	
	int tid;  		/* committed tid.  */
	int next; 		/* next on free list or hash chain */
} com[COMSIZE];


/*
 *	redo page hash table
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
 *		     updated contigous xtree entries starting from offset. 
 *		     The number of entries specified in the length.  
 *		     If there is only one segment, it must be header.
 *
 *		     For xtree, the new entry either inserts or appends. 
 *		     If it appends, then the log rec contains only the 
 *		     new entry data. If it is inserted, all the entries
 *		     after the insertion need to shift. Then the log rec
 *		     contains all entries starting from the new insertion.
 *		     So only low water mark (lwm) offset needs to be recorded
 *		     in summary2. At logredo time, only the log rec having
 *		     offset lower than lwm will apply to page and only apply
 *		     the difference between offset and lwm.
 *		     Header of the xtree page should be only applied once 
 *		    in logredo. So another field in summzry2 track header.
 *  2) Dtree page -- A dtree page has 128 slots including header. the slot size
 *                   is 32 byte. A 4 words(32 byte) array is used as  a bit 
 *		     vector to track the 128 slots.
 *  3) inode page -- There are 3 slot sizes and 5 sub-types for inode page.
 *                    Each type is in a separate log record.
 *
 *      a) type == INODE
 *         Section I (128 byte) of the dinode is logged. Offset is
 *	   counted from the beginning of the inode page.
 *	   A 8-bit vector tracks 8 inodes in inode page.
 *         the slot size for this type is 128 byte.
 *         Note: Each inode has 512 bytes, the INODE type only refers 
 *		 to the first 128 section.  so the offset should
 *               be always a multiply of 4, i.e. 0, 4, 8, 12, etc..
 *      b) type == EA  ( extended attribute )
 *         it is the top 16 bytes of section II of the dinode.
 *	   offset should be always ???.
 *	   A 8-bit vector tracks 8 inodes in inode page.
 *         the slot size is 16 bytes.
 *      c) type ==  DATA ( in-line data for symlink )
 *         A real xt data file starts from the 16 bytes above 
 *	   section III of the dinode.
 *	   offset should be always ???.
 *	   A 8-bit vector tracks 8 inodes in inode page.
 *         the slot size is 16 bytes.
 *      d) type == BTROOT + DTREE
 *         It starts from the 32 bytes above section III of the dinode.
 *	   Offset is counted from the beginning of BTROOT.
 *	   An array of 8 uint16, each is used as a bit vector to track
 *	   one dtroot.
 *         the slot size for this type is 32 byte.
 *      e) type == BTROOT + XTREE
 *         It starts from the 32 bytes above section III of the dinode.
 *	   Offset is counted from the beginning of BTROOT.
 *	   an array of 8 structrues, each tracks one xtroot for lwm and 
 *	   header of xtroot.
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

#define	BHASHSIZE 1024		/* must be a power of two	*/
int32 numdoblk;			/* number of do blocks used	*/
struct doblk
{
	int32 aggregate;      /* 4: file system aggregate/lv number */
	int32 fileset;	      /* 4: fileset number.   */
	pxd_t  pxd;           /* 8: on-disk page pxd */
        uint8  ino_base;   /* 1: each bit tracks one dinode for section I 
				   of the dinode (128 bytes.) Each inode has 4
				   128-byte slots, with one base slot.
                                   A total of 8 bits that need to be marked.
				   for 8 dinodes in one inode page  */
        uint8  ino_ea;     /* 1: extended attribute of the dinode, 16 byte*/
        uint8  ino_data;   /* 1: in-line data, at most (256 + 16) byte for
				   each dinode. */
	uint8  reserved;
        union {
           uint16  ino_dtroot[8]; /* 16: dtree root. each dinode has 9
				   dtree-root slots, including 1 slot header.
				   each slot is 32-byte.
				   Each element of i_dtroot[] monitors 
				   one dtree root of the dinode. 
                                   For each 16-bit, only 9-bit is used.*/
           uint32  dtpage_word[4]; /* 16:dtree page. a total of 128 slots 
				      including header    */
        } summary1;                 /* 16    */
	
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
        } summary2;                 /* 16    */

	struct doblk *next;	/* 4: next entry on hash chain	*/
};

#define  db_ibase	ino_base
#define  db_iea		ino_ea
#define  db_idata	ino_data
#define  db_dtroot	summary1.ino_dtroot
#define  db_dtpagewd	summary1.dtpage_word
#define  db_xtpagelwm	summary2.xtpg.xtpage_lwm
#define  db_xtpghd      summary2.xtpg.xtpage_hd
#define  db_xtroot	summary2.i_xtroot

int32 blkhmask = (BHASHSIZE-1);	/* hash mask for blkhash */
struct doblk *blkhash[BHASHSIZE]; /* head of doblk hash chains */
int32 Freedoblk;		/* number of unused doblk struct */
struct doblk  *Blkpage;         /* the beginning address of doblk hash table
                                  page           */

/*
 *	 noredo file hash table.
 *
 * each entry represents a journalled objects deleted.
 */
#define NODOFILEHASHSIZE   512
int32 numnodofile;			/* number of nodo file blocks used	*/	
int32 nodofilehmask = (NODOFILEHASHSIZE -1);  /* hash mask for nodohash  */
struct nodofile
{
	int32 aggregate;      /* 4: file system aggregate/lv number */
	int32 fileset;        /* 4: fileset number this file is belonged */
	ino_t  inode;		/* inode number			*/
	struct nodofile *next;	/* next entry on nodo hash chain*/
}; 

struct nodofile *nodofilehash[NODOFILEHASHSIZE]; /* head of nodo hash chains */
int32  Freenodofile;			/* number of unused nodofile struct */
struct nodofile *Nodofilep;		/* the beginning address of nodo
					   hash table page   */

/*
 *	open file system aggregate/lv array
 *
 * logredo() processes a single log.
 * at the first release, logredo will process a single log
 * related to one aggregate. But the future release, logredo needs to
 * to process one single log related to multiple agreegates.
 * In both cases, the aggregate(logical volume) where the log stays
 * will be different from  the file system aggregate/lv.
 *
 * There will be one imap for the aggregate inode allocation map 
 * and a list of imap pointers to multiple fileset inode allocation maps.
 *
 * There is one block allocation map per aggregate and shared by all the
 * filesets within the aggregate.
 *
 * the log and related agreegates (logical volumes) are all in 
 * the same volume group, i.e., each logical volume is uniquely specified 
 * by their minor number with the same major number,
 * the maximum number of lvs in a volume group is NUMMINOR (256). 
 */
#define NUMMINOR  256
struct vopen
{
	int32  fd;     		/* 4: file descriptor for the opened aggregate */
	int32  status; 		/* 4: status of  aggregate/volume */
	int64  fssize;		/* 8: fs size, total number of aggre. blks */
	pxd_t  log_pxd; 	/* 8: log pxd for aggr./lv inline log     */
	int64  logxaddr;	/* 8: inline log address in bytes offset  */
	int32  lblksize; 	/* 4: aggre. blksize (ie fs blksize) in bytes*/
	int16  lbperpage; 	/* 2: aggregate blocks per page */
	int16  is_fsdirty;	/* 2: true if superblock.state is FM_DIRTY */
        int16  l2bsize;         /* 2: log2 of s_bsize */
        int16  l2bfactor;       /* 2: log2(s_bsize/physical block size) */
	int32  agsize; 		/* 4: num of aggre. blks per alloc group  */
	int32  l2agsize;	/* 4: Log2 of agsize   */
	int32  numag; 		/* 4: total num of ags in this aggregate. */
	dinomap_t  *aggimap_ctrl;    /* 4: pointer to the control page of 
                                           aggregate inode map */
        iag_t   *aggimap_iag;	/* 4: ptr to aggregate inode map iag pages */
	struct fsimap_lst {
	    int32 fileset;  	    /* 4: fileset number, which is the inode
				     * number of this fileset imap inode in
				     * the aggregate inode table.
				     */
	    dinomap_t  *fsimapctrl; /* 4: ptr to ctrl page of fileset imap */
            iag_t   *fsimapiag;     /* 4: ptr to 1st iag page of fileset imap */
            struct fsimap_lst  *next;  /* 4: ptr to next fileset imap */
	} fsimap_lst;               /* 12:  the list of multiple fileset imap */
		
	dbmap_t  *bmap_ctl; /* 4: ptr to control page of aggregate block 
			     *    allocate map (i.e. bmap) file 
			     */
	dmapctl_t *dmap_ctl; /* 4: ptr to the 1st dmapctl_t page in bmap file */
} vopen[NUMMINOR];

#define  fsimap_ctrl    fsimap_lst.fsimapctrl
#define  fsimap_iag     fsimap_lst.fsimapiag
#define  fs_next        fsimap_lst.next
#define  fs_fileset     fsimap_lst.fileset



/*
 *	file system page buffer cache
 *
 * for k > 0 bufhdr[k] describes contents of buffer[k-1].
 * bufhdr[0] is used as anchor for free/lru list. 
 * when a buffer is needed, bufhdr[0].prev is the buffer selected.
 * bread of a buffer causes bufhdr[0].next to be set to the
 * buffer's index.
 */
#define NBUFPOOL  128
int32 bhmask = (NBUFPOOL - 1); 	/* hash mask for bhash */
int16 bhash[NBUFPOOL]; 		/* head of buffer hash chains */
struct bufhdr
{
	int16   	next;		/* 2: next on free/lru list */
	int16   	prev;		/* 2: previous on free/lru list */
	int16		hnext;		/* 2: next on hash chain */
	int16   	hprev;		/* 2: previous on hash chain */
	char		modify;		/* 1: buffer was modified */
	char		inuse;		/* 1: buffer on hash chain */
	int16           reserve;        /* 2 */
	int32		vol;		/* 4: minor of agrregate/lv number */
	pxd_t		pxd;		/* 8: on-disk page pxd */
					/* 24 */
} bufhdr[NBUFPOOL];

struct bufpool
{
	char	bytes[PSIZE];
} buffer[NBUFPOOL - 1];

/*
 *      log page buffer array
 *
 * log has its own 4 page buffer pool.
 */
int32    afterdata[LOGPSIZE/4];	/* buffer to read in after data */

int    nextrep;                 /* next log buffer pool slot to replace */
int    logptr[4];               /* log pages currently held in logp */
logpage_t logp[4];		/* log page buffer pool  */ 


/* maptable. the kth bit of dmaptab[x] is a one if the byte x
 * contains a  sequence of k consecutive zeros, k = 1,2,..8.
 * leftmost bit of char is bit 1.
 */
unsigned char  dmaptab[256];


/* maptab is used for imap. It determines number of zeroes within
 * characters of imap bitmap words. The character values  serve
 * as indexes into the table
 * e.g. if char has value of "3", maptab[2] = 6 which indicates there
 * are 6 zeroes in "3".
 */

unsigned char maptab[256] = {
	8, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4,
	7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3,
	7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3,
	6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2,
	7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3,
	6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2,
	6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2,
	5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1,
	7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3,
	6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2,
	6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2,
	5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1,
	6, 5, 5, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 3, 3, 2,
	5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1,
	5, 4, 4, 3, 4, 3, 3, 2, 4, 3, 3, 2, 3, 2, 2, 1,
	4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0};

/*
 * budtab[]
 *
 * used to determine the maximum free string(i.e. buddy size)
 * in a character of a dmap bitmap word.  the values of the character
 * serve as the index into this array and the value of the budtab[]
 * array at that index is the max binary buddy of free bits within
 * the character. 
 * e.g. when char = "15" (i.e. 00001111), budtab[15] = 2 because
 * the max free bits is 2**2 (=4).
 *
 */

signed char budtab[256] = {
        3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1};


#define BUGPRT(prspec)\
{\
         printf prspec;\
}

int moveWords( int, 
		int*,
		int*,
		int*);



/* Misc
 */
caddr_t prog;			/* Program name				*/
int32 mntcnt;
char *mntinfo;


/* Global variables
 */
char *prog;				/* program name			*/
int32 retcode;				/* return code from logredo    */
int32 atransid;
int32 avolid;
int32 ainode;
int32 atype;
char *arectype;
char *aname;
int32 anamelen;

int32 fflag = 0;
int32 sflag = 0;
int32 headflag = 0;
int32 volflag = 0;
int32 trflag = 0;
int32 inoflag = 0;
int32 tyflag = 0;
int32 adflag = 0;
int32 ucomflag = 0;
int32 nmflag = 0;
int32 begflag = 0;
int32 pflag = 0;

int32 header = 0;

int32 adcflag = 0;
char fsdevname[256];
int32 openVol(char * );
int32 vol;


// extern	char *optarg;
// extern	int optind;
extern errno;

main(argc,argv)
int argc;
char **argv;
{
	char *inc;
	int bufsize;
	int rc;
	caddr_t  logname;

	prog = argv[0];

	if (argc < 2)
	{
		usage();
		return(-1);
	}

//++	while ((inc = getopt (argc, argv, "buapfc:i:t:v:hn:r:s")) != EOF)
	rc=1;
	while ( rc < argc)
	{
	inc=argv[rc];
	if(*inc != '-')break;
       rc++;
	inc++;
       while(*inc)
		switch(inc++)
		{
		case 'b':  /* read log from log start */
			begflag++;
			break;
		case 'u':  /* uncommitted transaction. no action ?? */
			ucomflag++;
			break;
		case 'a':  /* print "REDOPAGE" rec data */
			adflag++;
			break;
		case 'f':  /* ?? */
			fflag++;
			break;
		case 'p':   /* log  page */
			pflag++;
			break;
		case 'c':  /* verify "REDOPAGE" log record */
			adcflag++;
			strcpy(fsdevname, inc);
			break;
		case 's':  /* print superblock  */
			sflag++;
			break;
		case 'i':  /* verify an inode number  */
			inoflag++;
			ainode = strtol(inc,0,16);
			break;
		case 't':  /* verify a tid number  */
			trflag++;
			atransid = strtol(inc,0,16);
			break;
		case 'v':   /* verify an aggregate/lv number  */
			volflag++;
			avolid = strtol(inc,0,16);
			break;
		case 'r':  /*  verify a rec type */
			tyflag++;
			arectype = inc;
			rc++;
			if ((atype = gettype(inc)) < 0)
				return(-1);
			break;
		case 'n':  /* given a name string for searching in log rec */
			nmflag++;
			aname = inc;
			rc++;
			if ((anamelen = strlen(aname)) <= 0)
				return(-1);
			break;
		case 'h': /* do we need print header? */
			headflag++;
			break;
		default:
			inc--;
		       printf("Unknown options %c\n",*inc);
			usage();
			return(-1);
		}
	}

	/*
	 * reposition argc & argv 
	 *
	argc -= optind - 1;
	argv += optind - 1;
*/

	/* loop tell we enough memory to read vmount struct
	 */
	mntinfo = (char *)&bufsize;
	bufsize = sizeof(int);

	/*
	 * Check to see if device is currently in use as a log 
	 */
	logname = *argv[rc];
	rc = isLogging(logname, mntinfo, mntcnt);
	free(mntinfo);
        if (rc== MAJOR_ERROR)
        {
                fprintf (stderr, "Error occurred when open/read device\n");
                return (-1);
        }
        if ( rc == MINOR_ERROR)
        {
                fprintf (stderr, "Log is active: %d\n", rc);
                return -1;
        }

	/*
	 *	open log
	 */
	logfd = logopen_view(logname);
	if (logfd < 0)
	{
		fprintf(stderr, "couldn't open log device\n");
		return(-1);
	}

	/*
	 * validate log superblock
	 */
        if ( loglocation == INLINELOG )
                rc = ujfs_rw_diskblocks( logfd,
                        (uint64)(vopen[logminor].logxaddr+LOGPNTOB(LOGSUPER_B)),
                        (unsigned)sizeof(logsuper_t),
                        (char *)&logsup,
                        GET);
        else
                rc = ujfs_rw_diskblocks( logfd,
                        (uint32)LOGPNTOB(LOGSUPER_B),
                        (unsigned)sizeof(logsuper_t),
                        (char *)&logsup,
                        GET);
        if (rc != 0)
        {
                fprintf(stderr, "couldn't read log superblock  \n");
                if ( logfd >= 0 )
                        DosClose(logfd);
                printf("failure in %s\n",prog);
                return (-1);
        }

	if (logsup.magic != LOGMAGIC )
	{
		fprintf(stderr, "%s: not a log file %-32s\n",prog ,logname);
		return(-1);
	}

	if (logsup.version != LOGVERSION)
	{
		fprintf(stderr, "%s and log file %s version mismatch\n", prog, 
			logname);
		return(-1);
	}


	logsize = logsup.size;
	logserial = logsup.serial;

	/*
	 * logview it 
	 */
	if (rc = logview(logname))
	{	fprintf (stderr, "Failure viewing log: %d\n",rc);
		return(-1);
	}

	return 0;
}

/*
 * NAME:	isLogging()
 *
 * FUNCTION: 	Check to see if device is currently in use as a log
 *
 * PRE CONDITION: NONE
 *
 * POST CONDITION: NONE
 *
 * NOTES: mntctl and stat are AIX system calls.  major and minor number are
 *	  also AIX concept. They could be changed in OS/2 system.
 *
 * DATA STRUCTURES: global variables logmajor and logminor are altered.
 *
 * PARAMETERS:	logname	- device name
 *		vmt	- pointer to buffer that contains an array of 
 *			  vmount sturctures, which are returned by the
 *			  mntctl system call. 
 *		cnt	- number of vmount structures in buffer pointed by vmt
 *
 * RETURNS:	0	- device logname is not actively used as a log
 *		MAJOR_ERROR - returned from isFilesystem().
 *		MINOR_ERROR - devices already mounted that are using logname
 */
isLogging(
caddr_t logname,
struct vmount *vmt,
int cnt)
{
	int32 i, rc = 0; 
	struct stat lsb, sb;
	dev_t mountdev;

	/*
	 * determine if logname represents a file system device  or
	 * a log device and set its major number
	 */
	if ((rc = isFilesystem(logname)) == MAJOR_ERROR)
		return(MAJOR_ERROR);

	return rc;
}

/*
 * NAME:	isFilesystem()
 *
 * FUNCTION:	 open the device to see if it's a valid filesystem.
 *		 If open failed, then return MAJOR_ERROR.
 *		 If open ok, and it is a valid file system, 
 *		 return the minor device number of the log for this
 *		 filesystem. Otherwise, return MINOR_ERROR.
 *
 * PRE CONDITION: other process opened the device log should allow
 *		  a O_RDONLY re-open.
 *
 * POST CONDITION: this O_RDONLY open is closed.
 *
 * NOTES: NONE
 *
 * DATA STRUCTURES: NONE
 *
 * PARAMETERS:	dev_name	- device name. This is the name passed to
 *				  logredo.
 *
 * RETURNS:	
 *		MAJOR_ERROR(-2) -  open device  or stat device failure, or
 *				   s_logdev number wrong.
 *		0 		-  1)read fs superblock ok but dev_name does not
 *				     represent a file system device name
 *				   2)read fs fs superblock ok, dev_name 
 *				     represents a fs device name.
 *				     loglocation is set up. ( INLINELOG or
 *				     OUTLINELOG ).
 *				   3)read fs superblock failed, but successful
 *				     to read the device as log
 */
int32
isFilesystem(caddr_t dev_name)
{
	int32 fd, rc = 0;
	struct superblock sb;
	struct stat lsb;
	int32 devmajor = 0;
	int32 devminor = 0;
	uint32 Action, actual;

	/*
	 * open the device with dev_name and check it out
	 */

/***
	if ((fd = open(dev_name, O_RDONLY)) < 0)
***/
        if (DosOpen(dev_name, (unsigned long *)&fd, &Action, 0, 0,
                        OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                        OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR |
                        OPEN_SHARE_DENYWRITE |OPEN_ACCESS_READONLY, 0))
	{
		BUGPRT(("isFilesystem: open failed errno = %d\n", errno));

		printf("isFilesystem: open %s failed\n",dev_name);
		return(MAJOR_ERROR);
	}

	/* try to read the superblock and check if it has a valid
	 * file system magic number.
	 */
	if ((rc = rdSuper(fd, &sb, PB_READ)) == 0 )
	{
		if ( strncmp(sb.s_magic,JFS_MAGIC,(unsigned)strlen(JFS_MAGIC))
					 == 0 )  /* the device is a fs  */
		{
			if ( ( sb.s_flag & JFS_INLINELOG ) == JFS_INLINELOG )
			{
				
				if ( !( logmajor == devmajor &&
					logminor == devminor ) )
				{
					
					BUGPRT(("isFilesystem: s_logdev is not a fs dev number\n"));
					rc = MAJOR_ERROR;
					goto out;
				}
				loglocation = INLINELOG;
				vopen[devminor].fd = fd;
				vopen[devminor].log_pxd = sb.s_logpxd;
				vopen[devminor].l2bsize = sb.s_l2bsize;
				vopen[devminor].logxaddr = 
					addressPXD(&sb.s_logpxd) << sb.s_l2bsize;
			}
			else /* it has an outline log  */
			{
				/* for _JFS_OS2, it always uses the inlinelog.
                                 * If the superblock indicated something else,
                                 * it is an error
                                 */
				rc = MAJOR_ERROR;
 			}
		}
		else   /* the device name does not represent a fs, check to 
			* see if it is a log device
		  	*/
		{
			rc = MAJOR_ERROR;
		}
	}
	else  /* if failed to read the device as a fs, to see if
	       * we can read it as an outline log device
	       */
	{
		rc = MAJOR_ERROR;
	}
		

out:	

#ifdef _JFS_OS2
     if ( rc != 0 )
#endif
	DosClose(fd);
	return (rc);
}

/*
 * logopen_view(log)
 *
 * This routine is used for logview.
 * opens the log and returns its fd. 
 * sets logmajor to the major number of the device.
 */
int32
logopen_view(log)
caddr_t log;
{
	int k;
	struct stat st;
	int fd;

	fd = vopen[logminor].fd;

	return(fd);
}


/* 
 * logview processing.
 *
 */

logview(pathname)
caddr_t pathname;
{
	int rc;

	fprintf(stdout,"Log View processing for %-32s \n",pathname);

	if ((logend = findEndOfLog()) < 0)
	{
		fprintf(stderr, "couldn't find log end\n");
		return(-1);
	}
printf("logend : %d\n", logend);

	if ((logstart = findStartOfLog()) < 0)
	{
		fprintf(stderr, "couldn't find log start\n");
		return(-1);
	}

	if (rc = dolog())
		return(rc);

	if (sflag)
		printsup();

	return(0);

}


/* 
 * NAME:	findEndOfLog()
 * 
 * FUNCTION:	Returns the address of the end of the last record in the log.
 * 		(i.e. the address of the byte following its descriptor).
 *
 *        Note: At the first release, log page is not written in a ping pong
 *		manner, so the logend if the binary search result
 *
 * 		The end of the log is found by finding the page with the 
 *		highest page number and for which h.eor == t.eor and  
 *		h.eor > 8 (i.e. a record ends on the page). 
 *		Page numbers are compared by comparing their difference 
 *		with zero (necessary because page numbers are allowed to wrap.)
 *
 * RETURNS:	>0		- byte offset of last record
 *		REFORMAT_ERROR(-3)	- i/o error, reformat the log
 *		MAJOR_ERROR(-2)	- other major errors other than EIO.
 */
int32
findEndOfLog()
{
	 
	int32	left, right, pmax, pval, eormax, eorval, k, rc;

	/* binary search for logend
	 */
	left = 2; 		/* first page containing log records  
                                   since page 0 is never used, page 1 is
                                   log superblock */
	right = logsize - 1;	/* last page containing log records */

	if ((rc = pageval(left, &eormax, &pmax)) < 0)
	{
		BUGPRT(("findEndOfLog: pageval[1] failed\n"));
		return(rc);
	}

	while ((right - left) > 1)
	{	
		k = (left + right ) >> 1;
		if ((rc = pageval(k, &eorval, &pval)) < 0)
		{
		 	BUGPRT(("findEndOfLog: pageval[2] failed\n"));
			return(rc);
		}

		if (pval - pmax > 0 )
		{	left = k;
			pmax = pval;
			eormax = eorval;
		}
	        else
			right = k;
	}
	if ((rc = pageval(right, &eorval, &pval)) < 0)
	{
		BUGPRT(("findEndOfLog: pageval[3] failed\n"));
		return(rc);
	}
	
	/*
	 * the last thing to determine is whether it is the first page of 
	 * the last long log record and system was crashed when its second
	 * page is written. If the eor of the chosen page is LOGPHDRSIZE,
	 * then this page contains a partial log record, ( otherwise, the
	 * the long log record's second page should be chosen ). 
	 * This page should be thrown away. its previous page will be
	 * the real last log page.
	 */   

	if (( pval - pmax ) > 0 )
	{
		if (eorval == LOGPHDRSIZE )
		{
			if ((rc = pageval(right-1, &eorval, &pval)) < 0)
			{
                        	BUGPRT(("findEndOfLog: pageval[4] failed\n"));
                		return(rc);
			}
			return(LOGPNTOB(right-1) + eorval);
		} else
			return(LOGPNTOB(right) + eorval);	
	}
	else  /* pmax will be returned */
	{	
		if ( eormax == LOGPHDRSIZE )
		{
			left = (left == 2) ? logsize -1 : left - 1;
			if ((rc = pageval(left, &eormax, &pmax)) < 0)
			{
                        	BUGPRT(("findEndOfLog: pageval[4] failed\n"));
                		return(rc);
			}
		} 
		return(LOGPNTOB(left) + eormax);
	}

}

/* 
 * NAME:	pageval(pno, eor, pmax)
 *
 * FUNCTION:    Read the page into the log buffer pool and call setLogpage
 *		to form consistent log page.
 *
 * RETURNS:	0			- ok
 *		REFORMAT_ERROR(-3) 	- I/O error, reformat the log
 *		MAJOR_ERROR(-2)		- other major errors other than EIO.
 */
pageval(int pno, 	/* page number in log 		*/
	int *eor, 	/* corresponding eor value 	*/
	int *pmax) 	/* pointer to returned page number */ 
{
	int  	rc,	
		buf0;		/* logp[] buffer element number		*/
	
	/* Read the page into the log buffer pool. 
	 */

	if ((buf0 = getLogpage(pno)) < 0)
	{
		BUGPRT(("pageval: getpage failed\n"));
		return(buf0);
	}
	return(setLogpage(pno, eor, pmax, buf0));
}
	
/*
 * NAME:	getLogpage(pno)
 *
 * FUNCTION:	if the specified log page is in buffer pool, return its
 *		index. Otherwise read log page into buffer pool.
 *
 * PARAMETERS:	pno -	log page number to look for.
 *
 * RETURNS:	0 - 3	- index of the buffer pool the page located
 *		REFORMAT_ERROR(-3)	- I/O error, reformat the log
 *		MAJOR_ERROR(-2)		- other major errors other than EIO.
 */
int32 getLogpage(
int pno)  /* page of log */                
{
	int k, rc;

	/*
	 * is it in buffer pool ?
	 */
	for (k = 0; k <= 3; k++)
		if (logptr[k] == pno) return(k);

	/*
	 * read page into buffer pool into next slot
	 */
	nextrep = (nextrep + 1) % 4;
        if ( loglocation == INLINELOG )
                rc = ujfs_rw_diskblocks( logfd,
                        (uint64)(vopen[logminor].logxaddr+LOGPNTOB(pno)),
                        (unsigned)LOGPSIZE,
                        (char *)&logp[nextrep],
                        GET);
        else
                rc = ujfs_rw_diskblocks( logfd,
                        (uint32)LOGPNTOB(pno),
                        (unsigned)LOGPSIZE,
                        (char *)&logp[nextrep],
                        GET);

	if ( rc !=  0 )
	{
		BUGPRT(("getLogpage: read failed rc =%d\n", rc));
#ifdef _JFS_OS2
		errno = rc;
#endif
		return(errno == EIO ? REFORMAT_ERROR : MAJOR_ERROR);
	}

	logptr[nextrep] = pno;
	return(nextrep);
}


/* 
 * NAME:	setLogpage(pno, eor, pmax, buf)
 *
 * FUNCTION: 	Forms consistent log page and returns eor and pmax values.
 *
 *		During the first release the following conditions are
 *		assumed:
 *		1) No corrupted write during power failure
 *		2) No split write
 *		3) No out-of-order sector write
 *
 *		If the header and trailer in the page are not equal, a 
 *		system crash happened during this page write. It 
 *		is reconciled as follows: 
 *
 *		1) if h.page != t.page, the smaller value is taken and
 *		   the eor fields set to LOGPHDSIZE.
 *		   reason: This can happen when a old page is over-written
 *		   by a new page and the system crashed. So this page 
 *		   should be considered not written.
 *		2) if h.eor != t.eor, the smaller value is taken.
 *		   reason: The last log page was rewritten for each
 *		   commit record. A system crash happened during the
 *		   page rewriting. Since we assume that no corrupted write
 *		   no split write and out-of-order sector write, the
 *		   previous successfuly writing is still good
 *		3) if no record ends on the page (eor = 8), still return it.
 *		   Let the caller determine whether a) a good long log record 
 *		   ends on the next log page. or b) it is the first page of the
 *		   last long log record and system was crashed when its second
 *		   page is written.
 *
 *
 * RETURNS:	0			- ok
 *		REFORMAT_ERROR(-3)	- I/O error, reformat log
 *		MAJOR_ERROR(-2)		- other major error
 */
setLogpage(int32 pno, 	/* page number of log 		*/
	int32 *eor,	/* log header eor to return 	*/ 
	int32 *pmax,	/* log header page number to return */
	int32 buf) 	/* logp[] index number for page */
{
	int32 diff1, diff2, rc;
	uint32 actual;

	/* check that header and trailer are the same 
	 */
	if ((diff1 = (logp[buf].h.page - logp[buf].t.page)) != 0)
	{	if (diff1 > 0) 
			logp[buf].h.page = logp[buf].t.page;
		else
			logp[buf].t.page = logp[buf].h.page;

		logp[buf].h.eor = logp[buf].t.eor = LOGPHDRSIZE; 
                                                /* empty page */
	}

	if ((diff2 = (logp[buf].h.eor - logp[buf].t.eor)) != 0)
	{	if (diff2 > 0)
			logp[buf].h.eor = logp[buf].t.eor;
		else
			logp[buf].t.eor = logp[buf].h.eor;

	}
	
	/* if any difference write the page out
	 */
	if (diff1 || diff2)
	{	
                if ( loglocation == INLINELOG )
                        rc = DosSetFilePtr(logfd,
                        (uint64)(vopen[logminor].logxaddr+LOGPNTOB(pno)),
                                                FILE_BEGIN, &actual);
                else
                        rc = DosSetFilePtr(logfd,
                                (uint32)LOGPNTOB(pno), FILE_BEGIN, &actual);
                if ((rc = DosWrite(logfd, (char *)&logp[buf],
                                (unsigned)LOGPSIZE, &actual)) !=  0)
		{
			BUGPRT (("setLogpage: write failed rc = %d\n", rc));
#ifdef _JFS_OS2
			errno = rc;
#endif
			return(errno == EIO ? REFORMAT_ERROR : MAJOR_ERROR);
		}
	}

	/*
	 * At this point, it is still possible that logp[buf].h.eor
	 * is LOGPHDRSIZE, but we return it anyway. The caller will make
	 * decision.
	 */

	*eor = logp[buf].h.eor;
	*pmax = logp[buf].h.page;

	return 0;
}


/* 
 * findStartOfLog()
 */

findStartOfLog()
{
	 
	int	left, right, pmin, pval, eormin, eorval, k, rc;
	int	eol;

	/* binary search for logstart
	 */
	left = 2; 		/* first page containing log records */
	right = logsize - 1;	/* last page containing log records */

	if ((rc = pageval(left, &eormin, &pmin)) < 0)
	{
		fprintf(stderr,"pageval failed\n");
		return(rc);
	}

	while ((right - left) > 1)
	{
		k = (left + right )/2;

		if ((rc = pageval(k, &eorval, &pval)) < 0)
		{
			fprintf(stderr,"pageval failed\n");
			return(rc);
		}

		if (pmin - pval > 0 )
		{
			left = k;
			pmin = pval;
			eormin = eorval;
		}
	        else
		{
			right = k;
		}
	}

	if ((rc = pageval(right, &eorval, &pval)) < 0)
	{
		fprintf(stderr,"pageval failed\n");
		return(rc);
	}

	eol =  (pmin - pval > 0) ? (LOGPNTOB(right) + eorval) :
 				   (LOGPNTOB(left) + eormin);
	return(eol);
}

/* 
 * process each log record
 */
dolog()
{
	int  rc,k,nextaddr,lastaddr;
	struct lrd ld;

	/*
	 *  if adcflag (verify after data), open all fsdevs that
	 *  were using this log
	 */
	if (adcflag)
		openVol(fsdevname);

	
	/* init commit table. At the beginning everyone is in the
	 * free list. index 0 is not used.
	 */
	comfree = 1;
	for (k = 1; k < COMSIZE; k++)
	{
		com[k].next = k + 1;
	}

        /* 
         * init comhash chains
         */
        for (k = 0; k < 64; k++)
                comhash[k] = 0;


	/* read log backwards and process records as we go.
	 * reading stops at place specified by first SYNCPT we
	 * encounter or end of log if begflag set. 
	 */
	nlogrecords = 0;
	nextaddr = logend;
	rc = 0;
	lastaddr = (begflag) ? logstart : 0;

	do
	{
		logaddr = nextaddr;
		nextaddr = logRead(logaddr, &ld, (char *)afterdata);
		nlogrecords += 1;
		if (nextaddr < 0)
		{
			if (nextaddr == REFORMAT_ERROR)
			{
				rc = nextaddr;
				goto loopexit;
			}
			break;
		}


		switch(ld.type) 
		{
		case LOG_COMMIT:
			if (docommit(&ld) == 0)
				prtcommit(&ld);
			break;

		case LOG_MOUNT:
			if (domount(&ld) == 0)
				prtmount(&ld);
			break;

		case LOG_SYNCPT:
			if (lastaddr == 0)
			{
				syncrecord = logaddr;
				lastaddr = (ld.log.syncpt.sync == 0)
					? logaddr
					: ld.log.syncpt.sync;
			}
			if (dosyncpt(&ld) == 0)
				prtsyncpt(&ld);
			break; 

		case LOG_REDOPAGE:
			if (doafter(&ld) == 0)
				prtafter(&ld);
			break;

		case LOG_NOREDOPAGE:
			if (doafter(&ld) == 0)
				prtnoredopage(&ld);
			break;

		case LOG_UPDATEMAP:
			if (doafter(&ld) == 0)
				prtupdmap(&ld);
			break;

		default:
			fprintf(stderr,"Unknown record type at %08x\n",logaddr);
			rc = MAJOR_ERROR;
			break;
		}
	
		if (rc == REFORMAT_ERROR)
		{
			fprintf(stderr,"format error. exiting\n");
			goto loopexit;
		}

		if (rc == MAJOR_ERROR)
		{
			fprintf(stderr,"fatal error. Exiting\n");
			goto loopexit;
		}


	} while (logaddr != lastaddr);

	loopexit:
	return(rc);
}


/*
 * NAME:	logRead(logaddr , ld, dataptr)
 *
 * FUNCTION:	reads the log record addressed by logaddr and 
 * 		returns the address of the preceding log record.   
 *
 * PARAMETERS:	logaddr -  address of the end of log record to read
 *				   Note: log is read backward, so this is
 *				   the address starting to read
 *		ld	- pointer to a log record descriptor
 *		dataptr	- pointer to data buffer
 *
 * RETURNS:	< 0 	- there is an i/o error in reading
 *		> 0	- the address of the end of the preceding log record
 */
logRead(logaddr, ld, dataptr)
int    logaddr;       /* address of log record to read */
struct lrd *ld;  /* pointer to a log record descriptor */
char	* dataptr;    /* pointer to buffer.  LOGPSIZE long */ 
{
	int buf,off,rc,nwords, pno;

	/* get page containing logaddr into log buffer pool
	 */
	pno = BTOLOGPN(logaddr);
	if (pno != loglastp)
	{
		loglastp = pno;
		lognumread += 1;
		if (lognumread > logsize - 2)
		{
			BUGPRT(( "logRead: logwrap\n"));
			return(MAJOR_ERROR);
		}
	}

	buf = getLogpage(pno);
	if (buf < 0)
	{
		BUGPRT(( "logRead: getLogpage failed\n"));
		return(buf);
	}

	/* read the descriptor */
	off = logaddr & (LOGPSIZE - 1) ;  /* offset just past desc. */
	rc = moveWords(LOGRDSIZE/4, (int *)ld, &buf, &off);
	if (rc < 0)
	{
		BUGPRT(( "logRead: moveWords[1] failed\n"));
		return(rc);
	}

	/* read the data if there is any */
	if (ld->length > 0)
	{
		nwords = (ld->length + 3)/4; /* if length is partial word, still
						read it   */ 
		rc = moveWords(nwords,(int *)dataptr,&buf,&off);
		if (rc < 0)
		{
			BUGPRT(( "logRead: moveWords[2] failed\n"));
			return(rc);
		}
	}

	return(LOGPNTOB(logptr[buf]) + off);
}


/*
 * NAME:	moveWords()
 *
 * FUNCTION:	moves nwords from buffer pool to target. data 
 *		is moved in backwards direction starting at offset.
 *		If partial log record is on the previous page,
 *		or we have exhaust the current page (all bytes were read), 
 *		the previous page is read into the buffer pool. 
 *		On exit buf will point to this page in the buffer pool 
 *		and offset to where the move stopped. 
 *
 *		Note: the previous page is fetched whenever
 * 		the current page is exhausted (all bytes were read)
 * 		even if all the words required to satisfy this move
 * 		are on the current page. 
 *
 * PARAMETERS:  nwords	- number of 4-byte words to move
 *		target	- address of target (begin address)
 *		buf	- index in buffer pool of current page
 *		offset	- initial offset in buffer pool page, this offset
 *			  includes the page head size
 *
 * RETURNS:     = 0 		- ok
 *		< 0 		- error returned from getLogpage
 */
moveWords(nwords, target, buf, offset)
int  nwords;  /* number of 4-byte words to move */
int *target;  /* address of target (begin address) */
int *buf;     /* index in buffer pool of current page */
int *offset;  /* initial offset in buffer pool page */                   
{
	int n,j,words,pno;
	int * ptr;

	j = (*offset - LOGPHDRSIZE)/4 - 1; /* index in log page data area   
	  			               of first word to move      */  
	words  = min(nwords,j + 1);  /* words on this page to move */
	ptr = target + nwords - 1; /* last word of target */
	for (n = 0; n < words; n++) 
	{
		*ptr = logp[*buf].data[j];
		j = j - 1;
		ptr = ptr - 1;
	}	
	*offset = *offset - 4*words;

	/* 
	 * If partial log record is on the previous page,
	 * or we have read all the log records in the current page,
	 * get the previous page
	 */

	if ( words != nwords	/* we get less than nwords */
              || j < 0)		/* or exhaust the page, so offset is just */
				/* the page head, then j < 0              */ 
	{
		/* get previous page */
		pno = logptr[*buf];
		pno = pno - 1;
		/* if we hit beginning location of the log, go wrapped, 
                   read log record from the end location of the log   */
		if (pno == 1) pno = logsize - 1;  
		*buf  = getLogpage(pno);
		if (*buf < 0) {
			BUGPRT(( "moveWords: getLogpage failed\n"));
			return(*buf);
		}
		*offset = LOGPSIZE - LOGPTLRSIZE;
		j = LOGPSIZE/4 - 4 - 1; /* index last word of data area */
		/* move rest of nwords if any. this will never 
		exhaust the page.                          */ 
		for (n = 0; n < nwords - words ; n++) {
			*ptr = logp[*buf].data[j];
			j = j - 1;
			ptr = ptr - 1;
		}
		*offset = *offset - 4*(nwords - words);
	}
	return(0);
}


/*
 * dosyncpt(ld)
 */
dosyncpt(ld)
struct lrd * ld;  	/* pointer to record descriptor */
{


	/* check if uncommit or name flag set.
	 */
	if (ucomflag || nmflag)
		return(-1);

	return(0);

}

/*
 * domount(ld)
 */
domount(ld)
struct lrd * ld;  	/* pointer to record descriptor */
{

	/* check if uncommit or name flag set.
	 */
	if (ucomflag || nmflag)
		return(-1);

	return(0);

}

/*
 * doafter(ld)
 */
doafter(ld)
struct lrd * ld;  	/* pointer to record descriptor */
{
	 
	int n,k,vol, rc;
	struct doblk *doptr;
	struct nodo *nodoptr;

	/* check if name (string) search was specified. if so,
	 * check after data for string.
	 */
	if (nmflag)
	{
		if (ld->type == LOG_REDOPAGE && findname(ld) == 0)
			return(0);
		else
			return(-1);
	}

	/* check if uncommit flag set.
	 */
	if (!ucomflag)
		return(0);

	/* is it committed ? */
	if (!findcom(ld->logtid))
		return(0);

	/* delete entry from com if no more records for tid
	 */
	if (ld->backchain == 0)
		deletecom(ld->logtid);


	return(-1);
}

/*
 * docommit(ld)
 * insert a commit record in com for transaction
 */
docommit(ld)
struct lrd * ld;  /* pointer to record descriptor */
{
	int  k,hash;

	/* check if name search flag set.
	 */
	if (nmflag)
		return(-1);

	/* check if uncommit flag set.
	 */
	if (!ucomflag)
		return(0);

	if (comfree == 0)
	{
		printf("MAJOR_ERROR\n");
		return(MAJOR_ERROR);
	}
	k = comfree;
	comfree = com[k].next;
	hash = ld->logtid & comhmask;
	com[k].next = comhash[hash];
	com[k].tid = ld->logtid;
	comhash[hash] = k;

	return(-1);
}

/*
 * dofree(ld)
 * processing for a dfree log record.
 */

dofree(ld)
struct lrd * ld;  	/* pointer to record descriptor */
{
	 
	int n,k,vol, rc, nblocks;

	/* check if name search flag set.
	 */
	if (nmflag)
		return(-1);

	/* check if uncommit flag set.
	 */
	if (!ucomflag)
		return(0);

	/* is it committed ? */
	if (!findcom(ld->logtid))
		return(0);

	/* delete entry from com if no more records for tid
	 */
	if (ld->backchain == 0)
		deletecom(ld->logtid);

	return(-1);

}

/*
 * findcom(tid)
 * search for tid in com. return index if found
 * and 0 if not found. 
 */

findcom(tid) 
int tid;  /* transaction id */
{

	int k,hash;
	hash = tid & comhmask;  /* hash class */
	for ( k = comhash[hash]; k != 0; k = com[k].next)
	{
		if (com[k].tid == tid)
			return(k);
	}
	
	return(0);   /* not found */
}

/*
 * deletecom(tid)
 * delete the entry in com for tid.
 */
deletecom(tid)
int tid;  /* transaction id to be deleted */
{

	int k,n,hash;
	hash = tid & comhmask;  	/* hash class */
	n = 0;  			/* previous entry on hash chain  */

	for (k = comhash[hash]; com[k].tid != tid ; k = com[k].next)
	{
		n = k;
	}
	
	/* remove k from hash chain and put it on free list
	 * Special case when 1st on the hash list
	 */
	if ( n == 0)
		comhash[hash] = com[k].next;
	else 
		com[n].next = com[k].next;

	com[k].next = comfree;
	comfree = k;
	return(0);
}

/*
 * findname(ld)
 * search after data for aname string.
 */
findname(
struct lrd * ld) 
{
	int i, j, seglen,linesize;
	char *dataptr;
	int16 off, ln;

	if (ld->length <= 0)
		return(-1);

	linesize = 1 << ld->log.redopage.l2linesize;
	dataptr = (char *) afterdata;
        j = 0;
        while ( j < ld->length )
        {
                dataptr  = dataptr  + j;
                off = *dataptr ;
                ln = *(dataptr + 2);
                seglen  = ln << ld->log.redopage.l2linesize;
		for (i = 0; i <= seglen; i++)
		{
			if (*aname == *(dataptr+i))
			{
				if (strncmp(aname,dataptr+i,anamelen) == 0)
					return(0);
			}
		}
                j = j + seglen + 4 ;
	}
	  
	return(-1);
}

/*
 * printsup()
 * print log superblock information.
 */
printsup()
{
	int  k, word, bit;

	printf("\n");
	printf(" Version......................0x%08x\n",logsup.version);
	printf(" Serial.......................0x%08x\n",logsup.serial);
	printf(" Size.........................0x%08x\n",logsup.size);
	printf(" Logend (logredo).............0x%08x\n",logsup.end);
	printf(" Logend (computed)............0x%08x\n",logend);
	printf(" Logstart (computed)..........0x%08x\n",logstart);
	printf(" Number of log records read...0x%08x\n",nlogrecords);
	if (!begflag)
	{
		printf(" SYNCPT record address.........%08x\n",syncrecord);
		printf(" SYNCPT address................%08x\n",logaddr);
	}
	printf(" Active Filesystems (minor)...."); 
	if ( loglocation == INLINELOG )
		printf("0x%04x ", logminor);
	else
		for ( k = 0; k < NUMMINOR ; k++)
		{
			word = k/32;
			bit = k - 32*word;
			if (logsup.active[word] & (UZBIT_32 >> bit))
			{
				printf("0x%04x ",k);
			}
		}
	printf("\n");
}


/* All the log records here is ld->type == LOG_REDOPAGE. So
 * the log rec data area should apply to disk pages with address pxd.
 * in addition, there are some other work need to be done at logredo
 * time. These works are determined by ld->log.redopage.type.
 */
prtafter(ld)
struct lrd * ld;
{
	int ino, ninos, rc, linesize;

	if (chkprt(ld))
		return;
        linesize = 1 << ld->log.redopage.l2linesize;
	switch ( ld->log.redopage.type)
	{
	case LOG_XTREE:
		prtdesc(" REDOPAGE:XTREE  ",ld);
		printf(" pxd.len=0x%08x",lengthPXD(&ld->log.redopage.pxd));
		printf(" pxd.addr=0x%llx", addressPXD(&ld->log.redopage.pxd));
		break;
	case (LOG_XTREE | LOG_NEW):
		prtdesc(" REDOPAGE:XTREE_NEW  ",ld);
		printf(" pxd.len=0x%08x",lengthPXD(&ld->log.redopage.pxd));
		printf(" pxd.addr=0x%llx", addressPXD(&ld->log.redopage.pxd));
		break;
	case (LOG_BTROOT | LOG_XTREE):
		prtdesc(" REDOPAGE:BTROOT_XTREE  ",ld);
		printf(" pxd.len=0x%08x",lengthPXD(&ld->log.redopage.pxd));
		printf(" pxd.addr=0x%llx", addressPXD(&ld->log.redopage.pxd));
		break;
	case LOG_DTREE: 
		prtdesc(" REDOPAGE:DTREE  ",ld);
		printf(" pxd.len=0x%08x",lengthPXD(&ld->log.redopage.pxd));
		printf(" pxd.addr=0x%llx", addressPXD(&ld->log.redopage.pxd));
		break;
	case (LOG_DTREE | LOG_NEW): 
		prtdesc(" REDOPAGE:DTREE_NEW  ",ld);
		printf(" pxd.len=0x%08x",lengthPXD(&ld->log.redopage.pxd));
		printf(" pxd.addr=0x%llx", addressPXD(&ld->log.redopage.pxd));
		break;
	case (LOG_DTREE | LOG_EXTEND): 
		prtdesc(" REDOPAGE:DTREE_EXTEND  ",ld);
		printf(" pxd.len=0x%08x",lengthPXD(&ld->log.redopage.pxd));
		printf(" pxd.addr=0x%llx", addressPXD(&ld->log.redopage.pxd));
		break;
	case (LOG_BTROOT | LOG_DTREE):
		prtdesc(" REDOPAGE:BTROOT_DTREE  ",ld);
		printf(" pxd.len=0x%08x",lengthPXD(&ld->log.redopage.pxd));
		printf(" pxd.addr=0x%llx", addressPXD(&ld->log.redopage.pxd));
		break;
	case (LOG_BTROOT | LOG_DTREE | LOG_NEW):
		prtdesc(" REDOPAGE:BTROOT_DTREE.NEW  ",ld);
		printf(" pxd.len=0x%08x",lengthPXD(&ld->log.redopage.pxd));
		printf(" pxd.addr=0x%llx", addressPXD(&ld->log.redopage.pxd));
		break;

	case LOG_INODE :
		/*
                 * logredo() updates imap for alloc of inode.
		 */
		prtdesc(" REDOPAGE:INODE  ",ld);
		printf(" pxd.len=0x%08x",lengthPXD(&ld->log.redopage.pxd));
		printf(" pxd.addr=0x%llx", addressPXD(&ld->log.redopage.pxd));
		break;
	case LOG_EA:
		prtdesc(" REDOPAGE:EA  ",ld);
		break;
	case LOG_DATA:
		prtdesc(" REDOPAGE:DATA  ",ld);
		break;
	}

	printf("\n");
	if (adflag || adcflag)
		prtadata(afterdata, ld->length, ld->log.redopage.l2linesize);
/***
	if (adcflag)
		if ((rc = ver_adata(ld, afterdata)) < 0)
		   printf("Can't verify afterdata: rc = %d, i/o error\n", rc);
***/
}

/* NAME: prtnoredopage
*/

prtnoredopage(struct lrd * ld)
{

      if (chkprt(ld))
	return;

	prtdesc("NOREDOPAGE",ld);
        printf(" pxd.len=0x%08x",lengthPXD(&ld->log.noredopage.pxd));
        printf(" pxd.addr=0x%llx", addressPXD(&ld->log.noredopage.pxd));
	printf("\n");
}

	

/* NAME: prtupdmap
 
   FUNCTION: When dtree extend the first extent, the log rec has a lrd 
             only with the in-line pxd.
             when xtree free extents, the log data has a list of xads.
             when xtree free partial of an extent, ( e.g. file truncation ),
             the log rec has a lrd only with the in-line pxd.
*/
prtupdmap(ld)
struct lrd * ld;
{
	if (chkprt(ld))
		return;

	switch (ld->log.updatemap.type)
	{
	case  LOG_ALLOCPXD:
		prtdesc(" UPDMAP.ALLOCPXD", ld);
		break;
	case  LOG_FREEXAD:
		prtdesc(" UPDMAP.FREEXAD", ld);
		break;
	case  LOG_FREEPXD:
		prtdesc(" UPDMAP.FREEPXD", ld);
		break;
	}
	printf(" nxd=%02x\n",ld->log.updatemap.nxd);	
	if (adflag)
	{
                printf(" pxd.len=%08x",lengthPXD(&ld->log.updatemap.pxd));
                printf(" pxd.addr=%08x", addressPXD(&ld->log.updatemap.pxd));
        	if ( ld->log.updatemap.type == LOG_FREEXAD)
		{
			printf("list of xads\n");
			prtxads(ld->log.updatemap.nxd);
		}
	}
}

prtcommit(ld)
struct lrd * ld;
{
	if (chkprt(ld))
		return;

	prtdesc(" COMMIT ",ld);
	printf("\n");
}


prtmount(ld)
struct lrd * ld;
{
	if (chkprt(ld))
		return;
	
	prtdesc(" MOUNT  ",ld);
	printf("\n");

}

prtsyncpt(ld)
struct lrd * ld;
{
	if (chkprt(ld))
		return;

	prtdesc(" SYNCPT ",ld);
	printf(" syncpt record at %08x  sync=%08x\n",logaddr,ld->log.syncpt.sync);
}

chkprt(ld)
struct lrd * ld;
{
	if (sflag)
		return(-1);

	if (trflag && ld->logtid != atransid)
		return(-1);
	
	if (volflag && minor(ld->aggregate) != avolid)
		return(-1);

	if (inoflag && ld->log.redopage.inode != ainode)
		return(-1);

	if (tyflag && ld->type != atype)
		return(-1);

	if (!header)
	{
		prtheader();
		header = 1;
	}

	return(0);
}

prtdesc(type,ld)
char *type;
struct lrd * ld;
{

		if (headflag)
		{
			printf(" logaddr: 0x%08x",logaddr);
			printf(" ld->backchain: 0x%08x",ld->backchain);
			printf(" ld->length: 0x%04x",ld->length);
			printf(" 0x%08x",ld->aggregate);

		}

		if (!tyflag)
			printf("%s\n",type);

		if (ld->type == LOG_SYNCPT )
			return;

		if (!trflag)
			printf("logtid:0x%08x",ld->logtid);

		if (!volflag)
		   if ( ld->type == LOG_REDOPAGE || 
			ld->type == LOG_NOREDOPAGE )
			printf(" Fileset:0x%08x",ld->log.redopage.fileset);

		if (!inoflag)
			if ( ld->type == LOG_REDOPAGE ||
                       	 ld->type == LOG_NOREDOPAGE )
			printf(" inode:0x%08x\n",ld->log.redopage.inode);

		return;
}

prtheader()
{
	printf("\n");
	if (trflag)
		printf(" Transaction Id........0x%08x\n",atransid);
	if (volflag)
		printf(" Volume Id.............0x%08x\n",avolid);
	if (inoflag)
		printf(" Inode Number..........0x%08x\n",ainode);
	if (tyflag)
		printf(" Record Type...........%s\n",arectype);
	printf("\n");

	if (headflag)
	{
		printf(" Logaddr ");
		printf(" Bkchain ");
		printf(" Len ");
	}

	if (!tyflag)
		printf(" Type   ");

	if (!trflag)
		printf(" Transid ");

	if (!volflag)
		printf(" Vol ");

	if (!inoflag)
		printf(" Inode   ");

	printf("\n");
}

prtxads(int32 nxd)
{
	int32 i;
	caddr_t *dataptr;

	dataptr = (caddr_t *)afterdata;

        for ( i = 0; i < nxd; i++)
	{
                printf(" xad.offset=0x%08x",offsetXAD((xad_t*)dataptr));
                printf(" xad.addr=0x%08x",addressXAD((xad_t*)dataptr));
                printf(" xad.len=0x%08x",lengthXAD((xad_t*)dataptr));
		dataptr += sizeof(xad_t);
	}
	printf("\n");
}


prtblks(
int nblocks)
{
	int i, n;

	for (i = 0, n = 0; i < nblocks; i++)
	{
		if (n == 0)
		{
			if (i == 0)
				printf("  blocks:");	
			else
				printf("         ");	
		}
		printf("  %06x",afterdata[i]);

		n++;
		if (n == 8 || i == nblocks - 1)
		{
			n = 0;
			printf("\n");
		}
	}
}

#define BYTESPERLN	16
#define L2BYTESPERLN	4
/*
 *  Read the redopage data area. The data area is consists of 
 *  multiple data segments. each segment format is:
 *  <Segment> = <segmentData><segmentDescriptor>
 * <segmentDescriptor> = <segmentOffset> <segmentLength>;
 * <segmentOffset> = int16: start slot index of segment;
 * <segmentLength> = int16: number of contiguous slots of segment;
 */

prtadata(int32  *afterdata,  /* redopage data area */
	 int32  len,         /* total length of the data area */
	int32 l2linesize)      /* log2 of the slot size */
{
	int numlines, lines, i,j, seglen,segnum;
	int16  off,ln;
	char asciibuf[BYTESPERLN+3];
	int16 *segdata;
	int32 *data;

	if (len <= 0)
		return;

	/* make segdata points to the end of afterdata */
        segdata = (int16 *)((caddr_t)afterdata + len);
	j = 0;
	seglen = 0;
	segnum = 0;
        while ( j < len )
        {
		ln = *--segdata; 
		off = *--segdata;
		printf("segnum: %d, off: %d, length:%d\n", segnum, off, ln);
		segnum++;
		seglen  = ln << l2linesize;
		numlines = seglen  >> L2BYTESPERLN;
		segdata = (int16 *)((caddr_t)segdata - seglen);
		data = (int32 *)segdata;
		for (i = lines = 0; lines < numlines; lines++, i += 4)
		{
			makestr(asciibuf,&data[i]);
			printf("  %08x  %08x  %08x  %08x  %s\n", data[i],
			      data[i+1],data[i+2],data[i+3],asciibuf);
		}
		j +=  seglen + 4 ;
	}
		
}

makestr(s,c)
char *s;
char *c;
{
	int i;

	*s='|';  s++;
	for (i = 0; i < BYTESPERLN; s++, c++, i++)
	{
		if ((*c >= 0x20) && (*c <= 0x7e))
			*s = *c;
		else
			*s = '.';
	}
	*s='|'; s++; *s='\0';
}

/* 
 * gettype
 *
 */

gettype(type)
caddr_t type;
{
	if (strcmp(type,"redopage") == 0)
		return(LOG_REDOPAGE);
	else if (strcmp(type,"commit") == 0)
		return(LOG_COMMIT);
	else if (strcmp(type,"updatemap") == 0)
		return(LOG_UPDATEMAP);
	else if (strcmp(type,"noredopage") == 0)
		return(LOG_NOREDOPAGE);
	else if (strcmp(type,"mount") == 0)
		return(LOG_MOUNT);
	else if (strcmp(type,"syncpt") == 0)
		return(LOG_SYNCPT);
	else
		return(-1);
}
	

usage()
{
	printf("jlogview -abcfhinrstuv logfile\n");
	printf("	-a              	print detailed record data\n");
	printf("	-b              	terminate search at log start\n");
	printf("	-c                   check afterdata against disk\n");
	printf("	-p			dump page header and trailer\n");
	printf("	-f			force use of active log\n");
	printf("	-h              	print record header data\n");
	printf("	-i ino number		search on inode number\n");
	printf("	-n search string	search after data for string\n");
	printf("	-r record type	search on record type\n");
	printf("	-s			log header information\n");
	printf("	-t transid		search on transaction id\n");
	printf("	-u			uncommitted records\n");
	printf("	-v vol number		search on volume number\n");
}

/*
 *  all functions needed for afterdata verification after this point  (adcflag)
 *  most of this swiped/hacked from logredo.c
 */


#ifdef  JFS_LOGVIEW_EXTRA

/*
 *  verify afterdata on disk
 *  returns:
 *	 1: log != disk 
 *	 0: log == disk
 *	<0: some sort of i/o error
 */
int
ver_adata(struct lrd *ld,char *adata)
{
	fdaddr_t frag;
	int fbo;   /* offset of frag in 4k block containing frag       */
	int abo;   /* offset of aftdata in 4k block containing aftdata */
	int afo;   /* offset of aftdata  in frag		       */
	int len;   /* bytes to read starting at frag to get all of aftdata */
	char buf[8192];  /* contains aft.disk + (afo + len + 511) / 512  */
	offset_t rc, fsoff;
	int	fd; 	/* fd of fs that this logrecord applied to*/
	uint32 actual;

	if (minor(ld->log.aft.volid) != vol)
	{
		printf("fsminor == %d, lrminor == %d\n", vol, minor(ld->log.aft.volid));
		return 0;
	}
	if (!vopen[vol].fd)
		return -1;

	/*
	 * seek to start of frag where afterdata begins
	 */
	frag.d = ld->log.aft.disk;
	fsoff = (offset_t)frag.f.addr * vopen[vol].fragsize;
	rc = DosSetFilePtr(vopen[vol].fd, fsoff, FILE_BEGIN, &actual);
	if ( actual != fsoff)
		return -2;

	/*
	 * read bunch of 512-byte blocks that contain aftdata
	 */
	abo = ld->log.aft.psaddr & 4095;
	fbo = ((offset_t)frag.f.addr * vopen[vol].fragsize) & 4095LL;
	afo = abo - fbo;
	len = (afo + ld->length + 511) & ~511; 
	rc = DosRead(vopen[vol].fd , buf, (unsigned)len, &actual)
	if ( rc == NO_ERROR && actual != len)
		return -3;

	/*
	 * see if afterdata looks ok on disk, and print out if different
	 */
	if (rc = memcmp(buf + afo, adata, ld->length))
	{
		printf("***ERROR: afterdata in log != afterdata on disk(0x%llx,%d)\n",
			 fsoff + afo, ld->length);
		prtadata((int *)(buf + afo), ld->length);
		printf("\n");
	}
	return rc ? 1 : 0;
}
#endif
			
	
	

/* 
 * fsError(type,vol,bn)
 *
 * error handling code for single filesystem (volume).
 */
fsError(type, vol, bn)
int type;
int vol; 
int bn;
{
	int status;

	fprintf (stderr, "bad error in volume %d \n", vol);

	retcode = -1;
	vopen[vol].status = FM_LOGREDO;

	switch(type) {
	case OPENERR:
		fprintf (stderr, "Open failed \n");
		break;
	case MAPERR:
		fprintf (stderr, "can not initialize maps \n");
		break;
	case DBTYPE:
		fprintf (stderr, "bad disk block number %d\n", bn);
		break;
	case INOTYPE:
		fprintf (stderr, "bad inode number %d\n", bn);
		break;
	case READERR:
		fprintf (stderr, "can not read block number %d\n", bn);
		break;
	case SERIALNO:
		fprintf(stderr, "log serial number no good /n");
		break;
	case IOERROR:
		fprintf (stderr, "io error reading block number %d\n", bn);
		break;
	}

	return 0;
}




/*
 * read the superblock for the file system described
 * by vol.
 */

int
rdSuper(int fd, struct superblock *sb)
{
unsigned long Actual;	//++ PS
	union {
		struct superblock super;
		char block[PAGESIZE];
		} super;

	/* seek to the postion of the superblock.
	 */
//++PS	lseek(fd, (off_t)(SUPER1_OFF), SEEK_SET);
	DosSetFilePtr(fd, (off_t)(SUPER1_OFF), SEEK_SET,&Actual);

	/* if the request is read, read the block containing the superblock
	 * from disk and set the returned superblock structure.  otherwise,
	 * clear the local block buffer, copy the passed superblock, and
	 * write the buffer to disk.
	 */
// PS Begin
//	if (read(fd, super.block, (unsigned)SIZE_OF_SUPER) != SIZE_OF_SUPER)
	DosRead(fd, super.block, (unsigned)SIZE_OF_SUPER,&Actual);
	if ( Actual != SIZE_OF_SUPER)
// PS End
	{
		BUGPRT(("read failed errno=%d\n", errno));
		return(MINOR_ERROR);
	}
	*sb = super.super;
	return 0;
}


/*
 * makeopen_view()
 * 
 * This routine is used for logview.
 * opens the device as a character device.
 * returns the fd of the opened device.
 */
int
makeopen_view(char *devname)
{
	int fd;
	char *name;
	struct stat st;
	uint32 Action, actual;

#ifdef Drova
	/* stat the device and check its major number
	 */ 
	if (stat(devname, &st) < 0) 
	{
		fprintf(stderr,"stat of file system device failed\n");
		return (-1);
	}
	if (major(st.st_rdev) != logmajor)
	{
		fprintf(stderr, "fs dev not in same vg as log dev\n");
		return -1;
	}
#endif
/***
	if ((fd = open(devname, O_RDONLY)) < 0)
***/
        if (DosOpen(devname, (unsigned long *)&fd, &Action, 0, 0,
                        OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                        OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR |
                        OPEN_SHARE_DENYWRITE |OPEN_ACCESS_READONLY, 0))

	{
		fprintf (stderr,"Cannot open %s\n", devname);
		return(-1);
	}
	vol = minor(st.st_rdev);
	printf("devname=%s, minor=%d\n", devname, vol);
	return(fd);
	fflush(stderr);
	fflush(stdout);
}



/*
 * NAME:	openVol
 *
 * FUNCTION: 	open the aggregate/volume specified. 
 * 		check if it was cleanly unmounted. also check log
 * 		serial number. initialize disk and inode mpas.
 */
int32
openVol(
char *devname)	/* pathname to special file */
{
	int32 k, fd, rc, fssize, l2agsize;
	struct superblock sb;

	fd = makeopen_view(devname);
	if (fd < 0)
	{	
		BUGPRT(( "openVol: makeopen_view failed\n"));
		fsError(OPENERR,vol,0);
		vopen[vol].fd = 0;
		return(MINOR_ERROR);
	}

	printf("makeopen_view passed, vol=%d\n", vol);
	vopen[vol].fd = fd;

	/* read in superblock of the aggregate/volume and
	 * check it out.
	 */
	if (rc = rdSuper(fd, &sb))
	{
		BUGPRT(( "openVol: rdSuper failed\n"));
		fsError(READERR,vol, SUPER1_B);
		DosClose(fd);
		vopen[vol].fd = 0;
		return(MINOR_ERROR);
	}

	/* check magic number and initialize version specific
	 * values in the vopen struct for this vol.
	 */
	if (strncmp(sb.s_magic,JFS_MAGIC,(unsigned)strlen(JFS_MAGIC)) == 0)
	{
		if (sb.s_version != JFS_VERSION)
		{
			fprintf(stderr, "version number not right %d\n",vol);
			DosClose(fd);
			vopen[vol].fd = 0;
			return(MINOR_ERROR);
		}
                vopen[vol].lblksize = sb.s_bsize;
                vopen[vol].l2bsize = sb.s_l2bsize;
                vopen[vol].l2bfactor = sb.s_l2bfactor;
                fssize = sb.s_size >> sb.s_l2bfactor;
                vopen[vol].fssize = fssize;
                vopen[vol].agsize = sb.s_agsize;
                LOG2NUM(vopen[vol].agsize, l2agsize);
                vopen[vol].numag = fssize >> l2agsize;
                vopen[vol].l2agsize = l2agsize;

	}
	else
	{
		fprintf(stderr, "magic number not right %d\n",vol);
		DosClose(fd);
		vopen[vol].fd = 0;
		return(MINOR_ERROR);
	}

	/* set lbperpage in vopen.
	 */
	vopen[vol].lbperpage = PSIZE / vopen[vol].lblksize;

	/* was it cleanly umounted ?
	 */
	if (sb.s_state == FM_CLEAN)
	{
		vopen[vol].status  = FM_CLEAN;
		DosClose(fd);
		vopen[vol].fd = 0;
		return(0);
	}

	/* else get status of volume 
	 */
	vopen[vol].status = sb.s_state;
	vopen[vol].is_fsdirty = ( sb.s_state == FM_DIRTY );

	/* check log serial number
	 */
	if (sb.s_logserial != logserial)
	{
		BUGPRT(( "openVol: bad serial number\n"));
		DosClose(fd);
		vopen[vol].fd = 0;
		fsError(SERIALNO, vol, SUPER1_B);
		return(MINOR_ERROR);
	}

/********* initialize the disk and inode maps.
	 
	fssize = sb.s_size/(vopen[vol].lblksize/sb.s_pbsize);
	if (rc = initmaps(vol,fssize))
	{
		BUGPRT(( "openVol: initmaps failed\n"));
		fsError(MAPERR,vol,0);
	}
**********/
	return rc;
}
