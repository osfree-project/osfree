/* $Id: log_map.c,v 1.2 2004/03/21 02:43:15 pasha Exp $ */

static char *SCCSID = "@(#)1.10  5/14/99 11:06:00 src/jfs/utils/libfs/log_map.c, jfslib, w45.fs32, currbld";
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
 *   MODULE_NAME:		log_map.c
 *
 *   COMPONENT_NAME: 	jfslib
 *
 *   FUNCTIONS:  log_map.c: recovery manager
 *
 *              initMaps
 *              bMapInit
 *              bMapRead
 *              bMapWrite
 *              dMapGet
 *              iMapInit
 *              iMapRead
 *              iMapWrite
 *              iagGet
 *              updateMaps
 *              writeImap
 *              updateImapPage
 *              writeBmap
 *              updDmapPage
 *              rXtree()
 *              adjTree
 *              maxBud
 *              bread
 *
 *
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


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *   L O C A L   M A C R O    D E F I N I T I O N S
  *
  */
#define UZBIT_32        ((uint32) (1 << 31 ))

/* The following define is for aggregate block allocation map */
#define SIZEOFDMAPCTL   sizeof(dmapctl_t)

/*
 * At the logredo time, the dmap read into memory to form an array
 * of file pages. The first page is always the aggregate disk allocation
 * map descriptor ( i.e. the bmap control page), the remaining pages are
 * either dmap control pages or dmap pages.
 * given zero origin dmapctl level of the top dmapctl, tlvl,
 * if tlvl == 2, L2 page exists;
 * if tlvl == 1, L2 does not exist,
 *               but L1.n and L0.n pages exist (0 <= n <= 1023);
 * if tlvl == 0, L2 and L1 pages do not exist,
 *               only L0.n pages exist (0 <= n <= 1023);
 */

/* convert disk block number to bmap file page number */
#define BLKTODMAPN(b)\
        (((b) >> 13) + ((b) >> 23) + ((b) >> 33) + 3 + 1)

/* convert dmap block number to bmap file page number */
#define DMAPTOBMAPN(d)\
        ((d) + ((d) >> 10) + ((d) >> 20) + 3 + 1)

/* convert disk block number to allocation group number */
#define BLKNOTOAG(b,l2agsize)   ((b) >> l2agsize)

/* things for the block allocation map */
int16 top_dmapctl_lvl; /* zero origin dmapctl level of the top dmapctl */


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *    S T U F F    F O R    T H E    L O G 
  *
  *       externals defined in logredo.c
  */
extern struct vopen vopen[];              /* (88) */

#define  fs_next        fsimap_lst.next
#define  fs_fileset     fsimap_lst.fileset
#define  fsimap_ctrl    fsimap_lst.fsimapctrl

/*
 *      file system page buffer cache
 *
 * for k > 0, bufhdr[k] describes contents of buffer[k-1].
 * bufhdr[0] is reserved as anchor for free/lru list:
 * bufhdr[0].next points to the MRU buffer (head),
 * bufhdr[0].prev points to the LRU buffer (tail);
 */
int32 bhmask = (NBUFPOOL - 1);  /* hash mask for bhash */
int16 bhash[NBUFPOOL];          /* hashlist anchor table */

/* buffer header table */
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

/* buffer table */
extern struct bufpool
{
        char    bytes[PSIZE];
} buffer[];


/*
 *      maptab[]
 *
 * maptab is used for imap. It determines number of zeroes within
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
        4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0
};


/*
 *      budtab[]
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
        2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1
};

/*
 * external references
 */
extern errno;
extern int fsError( int , int , int64 );
extern int markBmap( dmap_t *, pxd_t ,   int  ,  int );
extern int bflush( int32  ,   struct bufpool * );
extern int32 alloc_storage( int32, void **, int32 * );
extern int32 alloc_dmap_bitrec( dmap_bitmaps_t ** );

extern void fsck_send_msg( int, int, int );		/* defined in fsckmsg.c */
extern int alloc_wrksp( uint32, int, int, void ** );	/* defined in fsckwsp.c */


/*
 * forward references
 */
int initMaps( int32 );
int bMapInit( int, struct dinode * );
int bMapRead( int, int32, void  * );
int bMapWrite( int, int32, void  * );
int dMapGet( int, int32 );
int iMapInit( int, struct dinode * );
int iMapRead( int, int32, void  * );
int iMapWrite( int, int32, void  * );
int iagGet( int, int32 );
int updateMaps(int);
int writeImap( int, struct fsimap_lst,  struct dinode * );
int updateImapPage( int32  , iag_t *, int16 *, int16 *);
int writeBmap( int  , dbmap_t *, struct dinode * );
int updDmapPage( dmap_t *, int32 , signed char *);
int rXtree( int32 , struct dinode *,  xtpage_t **);
signed char adjTree( dmapctl_t  *, int32  , int32 );
static int32 maxBud( char *);
int bread( int32  ,  pxd_t , void  **, int32  );

/*
 * NAME:        initMaps()
 *
 * FUNCTION:    Logredo() needs to reconstruct fileset imap and Blk alloc map.
 *              In the first release, the aggregate imap is regarding as
 *              static.
 *              In XJFS, the imaps and bmap are dynamically allocated.
 *              At the beginning of logredo, the xtrees for these maps are
 *              the only trustable things. The xtree is rooted
 *              at the inode. So read the inodes first. then call
 *              readMap() to allocate storage for inode and disk maps and
 *              read them into memory. initialize workmaps to zeros.
 */
initMaps(
        int32 vol)  /* index in vopen array = minor(volid) */
{
        int32   rc;
        struct dinode   *dip;
        pxd_t   pxd1;

        /*
         *      initialize in-memory block allocation map
         */
        /* read in the bmap inode (i_number = 2) in a buffer: */
        PXDaddress(&pxd1, AITBL_OFF >> vopen[vol].l2bsize);
        PXDlength(&pxd1, vopen[vol].lbperpage);
        if ((rc = bread(vol, pxd1, (void **)&dip , PB_READ))!=0)
        {
                sprintf( message_parm_0, "(d) %d", errno );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_READBMAPINOFAIL, 0, 1 );
                return(BREAD_BMAPINIT);
        }

        /* locate the inode in the buffer page */
        dip += BMAP_I;

        /* read block map into memory and init workmap to zeros */
        if ((rc = bMapInit(vol, dip)) !=0)
        {
                fsck_send_msg( lrdo_READBMAPFAIL, 0, 0 );
                return(rc);
        }

        /*
         *      initialize in-memory fileset inode allocation map
         */
        /* read in the fileset imap inode (i_number = 16) in a buffer: */
        PXDaddress(&pxd1,
                 (AITBL_OFF + (SIZE_OF_MAP_PAGE << 1)) >> vopen[vol].l2bsize);
        if ((rc = bread(vol, pxd1, (void **)&dip , PB_READ)) !=0)
        {
                sprintf( message_parm_0, "(d) %d", errno );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_READIMAPINOFAIL, 0, 1 );
                return(BREAD_IMAPINIT);
        }

        /* locate the inode in the buffer page */
        dip += FILESYSTEM_I & 0x07;

        /* read inode map into memory and init workmap to zeros */
        if ((rc = iMapInit(vol, dip)) !=0)
        {
                fsck_send_msg( lrdo_READIMAPFAIL, 0, 0 );
                return (rc);
        }
        /* Return 0 */
        return (rc);
}


/***************************************************************
 * NAME: 	bMapInit()
 *
 * FUNCTION:	Calculate the number of pages in the block map.  Allocate
 *		an array of records so there is one for each page in the
 *		Aggregate Block Map.  Initialize each array element with the
 *		aggregate offset of the page it describes. 
 *
 *		Allocate page buffers for the control page, one dmap page, 
 *		and one control page at each Ln level used in this BMap.  
 *
 *		Read in the map control page.
 *
 *		Get the bitmaps for the last dMap page and set the
 *		'excess bits' to ones.
 *
 * NOTES:	In order to minimize logredo storage usage (because we 
 *		must be able to run during autocheck when there is no
 *		paging space available), we won't actually read in a dmap 
 *		page unless/until we need to touch it.  At that point we'll 
 *		allocate enough storage to accomodate the dmap's wmap 
 *		and pmap only.  
 *
 * MORE NOTES: 
 *              There are two fields are trustable at the beginning of
 *              the logredo. One is fssize, which is the s_size field in
 *              aggregate superblock converting to number of aggregate
 *              blocks. This size only tells how many dmap_t pages are
 *              need for the bmap. Another is di_size field in dinode_t.
 *              In XJFS, the aggre. bmap xtree  is rooted at aggregate
 *              inode #2. The xtree for map is journaled.
 *              Since a COMMIT_FORCE is used for the transaction of
 *              xtree update, index pages are synced written out at
 *              commit time, we can assume that the xtree as well as
 *              the di_size for map is ok for reading the map pages
 *              at the logredo time.
 *
 *              Allocate storage according to di_size for bmap file.
 *
 *              In XJFS, the bmap is dynamically allocated. Its xtree
 *              is rooted at aggregate inode #2.  The xtree for map is
 *              journaled. Since a COMMIT_FORCE is used for the
 *              transaction of xtree update, index pages are synced 
 *              (i.e., written out at commit time), we can assume that 
 *              the xtree for map is ok for reading the map pages at 
 *              the logredo
 */
int bMapInit( int     vol,            	/* index in vopen array */
                  dinode_t   *dip)   	/* disk inode of bmap */
{
    int bmi_rc = 0;
    int32	ndmaps, nl0pages = 0, nl1pages = 0, nl2pages = 0;
    int I_am_logredo = -1;
    uint32 bytes_needed = 0;
    caddr_t p = NULL;
    caddr_t p0 = NULL;
    xtpage_t        *xp;  
    int i, j, k, w, pgidx;
    int32  nbytes, npages, this_page;
    uint32  *pmap, mask;
    pxd_t  pxd;
    int64  xaddr;

  /*
   * compute the number pages in the Aggregate Block Map, then 
   * allocate an array of records to describe them.
   *
   * Note that we also allocate 
   *      a page so we can start on a page boundary.
   *      a page for the BMap control page
   *      a page buffer for reading and writing dmap pages
   *      a page buffer for reading and writing L0 pages
   *      a page buffer for reading and writing L1 pages
   *      a page buffer for reading and writing the L2 page
   */
    vopen[vol].bmap_page_count = dip->di_size / PSIZE;

    bytes_needed = (6 * PSIZE) + 
                           ( vopen[vol].bmap_page_count * sizeof(bmap_wsp_t));

    ndmaps = ((vopen[vol].fssize + BPERDMAP - 1) >> L2BPERDMAP);
    nl0pages = (ndmaps + LPERCTL - 1)/LPERCTL;
    if( nl0pages > 1 ) {
        nl1pages = (nl0pages + LPERCTL - 1)/LPERCTL;
        }
        
    sprintf( message_parm_0, "(d) %lld", bytes_needed );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    fsck_send_msg( lrdo_ALLOC4BMAP, 0, 1 );

    bmi_rc = alloc_wrksp( (uint32) bytes_needed, 
                                 0,		/* not meaningful from logredo */
                                 I_am_logredo,
                                 (void **) &p0
                                );
	/*
	 * note that failure to allocate the bmap is a special case.
	 * 
	 * We can replay the log without updating the bmap and
	 * then tell chkdsk to run a full check/repair which will 
	 * rebuild the bmap.  This would not work with the imap.
	 */
    if( (bmi_rc == 0) && (p0 != NULL) ) {  /* we got the storage */
        bmap_stg_addr = p0;
        bmap_stg_bytes = bytes_needed;
        }
    else  {  /* (bmi_rc != 0) || (p0 == NULL) */
        sprintf( message_parm_0, "(d) %lld", dip->di_size );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_ALLOC4BMAPFAIL, 0, 1 );
        Insuff_memory_for_maps = -1;
        return(0);		
        }  /* end else (bmi_rc != 0) || (p0 == NULL) */

      /*
       * we got the storage.  
       * find the first page boundary and parcel it out.
       */
    p0 = (char *) ( (( ((uint32)p0) + PSIZE - 1) / PSIZE ) * PSIZE ); 
    vopen[vol].bmap_ctl = (dbmap_t *) p0;
    p0 = (char *) ( ((uint32) p0) + PSIZE);
    vopen[vol].L2_pbuf = (dmapctl_t *) p0;
    p0 = (char *) ( ((uint32) p0) + PSIZE);
    vopen[vol].L1_pbuf = (dmapctl_t *) p0;
    p0 = (char *) ( ((uint32) p0) + PSIZE);
    vopen[vol].L0_pbuf = (dmapctl_t *) p0;
    p0 = (char *) ( ((uint32) p0) + PSIZE);
    vopen[vol].dmap_pbuf = (dmap_t *) p0;
    p0 = (char *) ( ((uint32) p0) + PSIZE);
    vopen[vol].bmap_wsp = (bmap_wsp_t *) p0;

      /*
       * set the record to say they are currently empty.
       */
    vopen[vol].L2_pagenum = -1;
    vopen[vol].L1_pagenum = -1;
    vopen[vol].L0_pagenum = -1;
    vopen[vol].dmap_pagenum = -1;

      /*
       * Initialize the BMap workspace array with aggregate offsets
       */
    pgidx = 0;
            /* 
             * read in the leftmost leaf page of the 
             * block allocation map xtree 
             */
    if (rXtree(vol, dip, &xp))  {
        fsck_send_msg( lrdo_RBMPREADXTFAIL, 0, 0 );
        return(BMAP_READERROR1);
        }

            /* 
             * in case of leaf root, init next sibling pointer so it will
             * appear as last non-root leaf page termination
             */
    if (xp->header.flag & BT_ROOT)
            xp->header.next = 0;

            /* 
             * the leaf pages contain the aggregate offsets we need
             */
    PXDlength(&pxd, vopen[vol].lbperpage);
    do  {
            /* 
             * get extent descriptors from the current leaf
             */
        for (i = XTENTRYSTART; i < xp->header.nextindex; i++) {
            xaddr = addressXAD(&xp->xad[i]) << vopen[vol].l2bsize;
            nbytes = lengthXAD(&xp->xad[i]) << vopen[vol].l2bsize;
            npages = nbytes / PSIZE;
                /* 
                 * get page offsets from the current extent descriptor
                 */
            for( j = 0; j < npages; j++ ) {
                vopen[vol].bmap_wsp[pgidx].page_offset = xaddr + (j * PSIZE);
                pgidx++;
                }
            }
            /* 
             * read in the next leaf (if any)
             */
        xaddr = xp->header.next;
        if (xaddr)  {
            PXDaddress(&pxd, xaddr);
            if (bread(vol, pxd, (void **)&xp, PB_READ)) {
                fsck_send_msg( lrdo_RBMPREADNXTLFFAIL, 0, 0 );
                return(BMAP_READERROR3);
                }
            }
        } while (xaddr);

      /*
       * Now read in the map control page
       */
    bmi_rc = bMapRead( vol, 0, (void  *)vopen[vol].bmap_ctl );
    if( bmi_rc != 0 ) {
       return( bmi_rc );
       }

      /*
       * And the last dmap page
       */
    bmi_rc = dMapGet( vol, (ndmaps-1) );
    if ( (bmi_rc == 0) && !Insuff_memory_for_maps ) {  /* we have bit maps */
        /*
         * init persistent bit map of last/partial dmap page
         *
         * if the last dmap may have bits that are beyond mapsize,
         * the pmap[] bits for these non-existing blocks have to be
         * inited as allocated.
         */
initLastPage:
        k = vopen[vol].fssize & (BPERDMAP - 1);
        if (k > 0)  {
                this_page = DMAPTOBMAPN(ndmaps-1);
                pmap = &(vopen[vol].bmap_wsp[this_page].dmap_bitmaps->pmap);

                i = k & (DBWORD-1); /* valid bits in partial word */
                w = k >> L2DBWORD; /* number of valid full words */

                /* init last valid/first invalid partial word */
                if (i)  {
                        mask = ((uint32)(ONES << i)) >> i;
                        pmap[w] |= mask;
                        w++;
                        }

                /* init full invalid words */
                for ( ; w < LPERDMAP; w++)
                        pmap[w] = ONES;
                }
        }  /* end we have the bit maps */

    return( bmi_rc );
}


/***************************************************************
 * NAME: 	bMapRead()
 *
 * FUNCTION:	Read the requested block map page from the device
 *                   into the specified buffer.
 *
 * NOTES:	  
 *
 */
int bMapRead( int     vol,            	/* index in vopen array */
                     int32 page_number,      /* block map page offset to read */
                     void  *page_buffer )    /* where to put the page read */
{
    int bmr_rc = 0;

        /* 
         * read in the map page
         */
    bmr_rc = ujfs_rw_diskblocks(vopen[vol].fd, 
                                            vopen[vol].bmap_wsp[page_number].page_offset, 
                                            PSIZE, 
                                            page_buffer, 
                                            GET);
    if ( bmr_rc != 0 )  {
        fsck_send_msg( lrdo_RBMPREADDATFAIL, 0, 0 );
        return(BMAP_READERROR7);
        }

    return( bmr_rc );
}


/***************************************************************
 * NAME: 	bMapWrite()
 *
 * FUNCTION:	Write the contents of the given buffer to the specified
 *                   block map page on the device
 *
 * NOTES:	  
 *
 */
int bMapWrite( int     vol,            	/* index in vopen array */
                     int32 page_number,      /* where to write the page */
                     void  *page_buffer )    /* where get the data to write */
{
    int bmw_rc = 0;

        /* 
         * write out the map page
         */
    bmw_rc = ujfs_rw_diskblocks(vopen[vol].fd, 
                                            vopen[vol].bmap_wsp[page_number].page_offset, 
                                            PSIZE, 
                                            page_buffer, 
                                            PUT);
    if ( bmw_rc != 0 )  {
        sprintf( message_parm_0, "0x0%llx", 
                    vopen[vol].bmap_wsp[page_number].page_offset );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_WRBMPBLKWRITEFAIL, 0, 1 );
        return(BMAP_WRITEERROR2);
        }

    return( bmw_rc );
}


/***************************************************************
 * NAME: 	dMapGet()
 *
 * FUNCTION:	Calculate the page position (in the block map) of the
 *                   requested dmap page.  Read it into the dmap buffer,
 *		allocate a bit map record for it, copy the pmap from the
 *                   page read into the bit map record and then initialize the
 *                   wmap in the bit map record.
 *
 * NOTES:	  If we are unable to allocate storage, we'll reset some
 *                    stuff so that the storage already allocated to the block
 *                    map is released (for other use by logredo) and then 
 *                    continue processing without further updates to the block
 *                    map.  (In this case, when we return to chkdsk we'll pass
 *                    a return code which will ensure that full chkdsk will be
 *                    done.  The full chkdsk includes rebuilding the block map,
 *                    so this provides the data protection of logredo even if not
 *                    the performance of logredo alone.)
 *
 */
int dMapGet( int     vol,            	/* index in vopen array */
                   int32 dmap_num )          /* the ordinal dmap page wanted */
{
    int dmg_rc = 0, intermed_rc = 0;
    int64  page_needed;
    dmap_bitmaps_t *dmap_bitrec;
    dmap_t *dmap_pg;
    int32  maplen;

        /* 
         * figure out the map page needed
         */
    page_needed = DMAPTOBMAPN(dmap_num);

        /* 
         * read in the map page
         */
    dmg_rc = bMapRead( vol, 
                                  page_needed, 
                                  (void  *)vopen[vol].dmap_pbuf 
                                 );
    if ( dmg_rc != 0 )  {
        return(dmg_rc);
        }
    vopen[vol].dmap_pagenum = page_needed;

        /* 
         * allocate and set up a bitmap workspace for this page
         */
    dmg_rc = alloc_dmap_bitrec( &dmap_bitrec );
    if( (dmg_rc == 0) && !Insuff_memory_for_maps ) {
        dmap_pg = vopen[vol].dmap_pbuf;
        maplen = sizeof(uint32) * LPERDMAP;
        memset( (void *) &(dmap_bitrec->wmap), 
                     0, 
                     maplen
                    );
        memcpy( (void *) &(dmap_bitrec->pmap), 
                      (void *) &(dmap_pg->pmap), 
                      maplen
                    );
        vopen[vol].bmap_wsp[page_needed].dmap_bitmaps = dmap_bitrec;
        }

    return( dmg_rc );
}


/*
 * NAME:        iMapInit()
 *
 * FUNCTION:    Calculate the number of pages in the fileset Inode Map.
 *              Allocate an array of records so there is one for each
 *              page in the map.  Initialize each array element with the
 *              aggregate offset of the page it describes.
 *              
 *              Allocate a page buffer for IAGs.
 *              
 *              Read in the map control page.
 *              
 *
 * NOTES: 
 *              Regarding extendfs,
 *                  allocate storage for a map, read it into memory, and 
 *                  initialize workmap to zeros. recompute on the basis of 
 *                  the file system size and allocation group sizes in the filesystem 
 *                  superblock the number of allocation groups and the size of map. 
 *                  (ie the size info in the map is not trusted). this is done to 
 *                  recover from a crash in the middle of extending a filesystem. 
 *                  depending on where the crash occurred, the extendfs will be 
 *                  completed here, or backed-out. in backing out, the map 
 *                  considered as files may be bigger than neceassry, but logically
 *                  this causes no problem: extendfs can be re-run, and the extra 
 *                  data not accessed before then.
 */
iMapInit(	int     vol,              	/* index in vopen array */
		struct dinode * dp)	/* disk inode of map */
{
        int32  imi_rc;
        int32 bytes_needed, map_pages;
        int32 allocated_from_bmap = 0;
        int64 nbytes, xaddr;
        char *ptr0 = NULL;
        int32 i, j, pgidx;
        xtpage_t *p;   /* xtree page  */
        pxd_t pxd;

        /* 
         * figure out how many pages there are in the map, and 
         * from that the size array we need, and from that the
         * number of bytes to allocate now.
         *
         * Note that we also allocate 
         *     a page so we fcan start on a page boundary
         *     a page for the IMap control page
         *     a page buffer for reading and writing IAGs
         *     a page buffer for reading and writing IAGs 
         *         (when rebuilding the by-AG lists we need a 2nd buffer)
         */
        vopen[vol].fsimap_lst.fileset = dp->di_number;
        vopen[vol].fsimap_lst.next = NULL;  /* in rel 1, only one fileset */

        vopen[vol].fsimap_lst.imap_page_count = dp->di_size / PSIZE;

        bytes_needed = (4 * PSIZE) +
                   ( vopen[vol].fsimap_lst.imap_page_count * sizeof(imap_wsp_t) );

        /* allocate storage for imap. the size is always a multiple
         * of PSIZE so we don't have to round it up.
         * di_size is number of bytes
         */
       sprintf( message_parm_0, "(d) %lld", bytes_needed );
       msgprms[0] = message_parm_0;
       msgprmidx[0] = 0;
       fsck_send_msg( lrdo_ALLOC4IMAP, 0, 1 );

       imi_rc = alloc_storage( 	(uint32) bytes_needed, 
				(void **) &ptr0,
				&allocated_from_bmap 
				);

       if( (imi_rc != 0) || (ptr0 == NULL) ) {  /* imap allocation failed */
                sprintf( message_parm_0, "(d) %lld", bytes_needed );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_ALLOC4IMAPFAIL, 0, 1 );
	     return(ENOMEM2);
              }  /* end imap allocation failed */
       else if( ptr0 != NULL ) {  /* imap allocation successful */
              if( allocated_from_bmap ) {
                     fsck_send_msg( lrdo_USINGBMAPALLOC4IMAP, 0, 0 );
                     }
              }  /* end imap allocation successful */

	/*
	 * take it to a page boundary and parcel it out
	 */
      ptr0 = (char *) ( (( ((uint32)ptr0) + PSIZE - 1) / PSIZE ) * PSIZE ); 
      vopen[vol].fsimap_lst.fsimapctrl = (dinomap_t *) ptr0;
      ptr0 = (char *) ( ((uint32) ptr0) + PSIZE );
      vopen[vol].fsimap_lst.iag_pbuf = (iag_t *) ptr0;
      ptr0 = (char *) ( ((uint32) ptr0) + PSIZE );
      vopen[vol].fsimap_lst.iag_pbuf2 = (iag_t *) ptr0;
      ptr0 = (char *) ( ((uint32) ptr0) + PSIZE );
      vopen[vol].fsimap_lst.imap_wsp = (imap_wsp_t *) ptr0;

        /* 
         * set the record to say the buffer is currently empty
         */
      vopen[vol].fsimap_lst.imap_pagenum = -1;

    /*
     * Initialize the IMap workspace array with aggregate offsets
     */

        /*
         * from the xtroot, go down to the xtree leaves
         * which have the xad's for the map data
         */
        if (rXtree(vol, dp, &p ))
        {
                fsck_send_msg( lrdo_RIMPREADXTFAIL, 0, 0 );
                return(IMAP_READERROR1);
        }

      /*
       * Initialize the IMap workspace array with aggregate offsets
       */
    pgidx = 0;
            /* 
             * read in the leftmost leaf page of the 
             * inode allocation map xtree 
             */
    if (rXtree(vol, dp, &p))  {
        fsck_send_msg( lrdo_RIMPREADXTFAIL, 0, 0 );
        return(IMAP_READERROR2);
        }

            /* 
             * in case of leaf root, init next sibling pointer so it will
             * appear as last non-root leaf page termination
             */
    if (p->header.flag & BT_ROOT)
            p->header.next = 0;

            /* 
             * the leaf pages contain the aggregate offsets we need
             */
    PXDlength(&pxd, vopen[vol].lbperpage);
    do  {
            /* 
             * get extent descriptors from the current leaf
             */
        for (i = XTENTRYSTART; i < p->header.nextindex; i++) {
            xaddr = addressXAD(&p->xad[i]) << vopen[vol].l2bsize;
            nbytes = lengthXAD(&p->xad[i]) << vopen[vol].l2bsize;
            map_pages = nbytes / PSIZE;
                /* 
                 * get page offsets from the current extent descriptor
                 */
            for( j = 0; j < map_pages; j++ ) {
                vopen[vol].fsimap_lst.imap_wsp[pgidx].page_offset = 
                                                                  xaddr + (j * PSIZE);
                pgidx++;
                }
            }
            /* 
             * read in the next leaf (if any)
             */
        xaddr = p->header.next;
        if (xaddr)  {
            PXDaddress(&pxd, xaddr);
            if (bread(vol, pxd, (void **)&p, PB_READ)) {
                fsck_send_msg( lrdo_RIMPREADNXTLFFAIL, 0, 0 );
                return(IMAP_READERROR3);
                }
            }
        } while (xaddr);

      /*
       * Now read in the map control page
       */
    imi_rc = iMapRead( vol, 0, (void  *)vopen[vol].fsimap_lst.fsimapctrl );
    if( imi_rc != 0 ) {
       return( imi_rc );
       }

        return (0);
}


/***************************************************************
 * NAME: 	iMapRead()
 *
 * FUNCTION:	Read the requested inode map page from the device
 *                   into the specified buffer.
 *
 * NOTES:	  
 *
 */
int iMapRead( int     vol,            	/* index in vopen array */
                     int32 page_number,      /* map page offset to read */
                     void  *page_buffer )    /* where to put the page read */
{
    int imr_rc = 0;

        /* 
         * read in the map page
         */
    imr_rc = ujfs_rw_diskblocks(vopen[vol].fd, 
                             vopen[vol].fsimap_lst.imap_wsp[page_number].page_offset, 
                             PSIZE, 
                             page_buffer, 
                             GET
                             );
    if ( imr_rc != 0 )  {
        fsck_send_msg( lrdo_RIMPREADDATFAIL, 0, 0 );
        return(IMAP_READERROR4);
        }

    return( imr_rc );
}


/***************************************************************
 * NAME: 	iMapWrite()
 *
 * FUNCTION:	Write the contents of the given buffer to the specified
 *                   inode map page on the device
 *
 * NOTES:	  
 *
 */
int iMapWrite( int     vol,            	/* index in vopen array */
                     int32 page_number,      /* where to write the page */
                     void  *page_buffer )    /* where get the data to write */
{
    int imw_rc = 0;

        /* 
         * write out the map page
         */
    imw_rc = ujfs_rw_diskblocks(vopen[vol].fd, 
                             vopen[vol].fsimap_lst.imap_wsp[page_number].page_offset, 
                             PSIZE, 
                             page_buffer, 
                             PUT
                             );
    if ( imw_rc != 0 )  {
        sprintf( message_parm_0, "0x0%llx", 
                    vopen[vol].bmap_wsp[page_number].page_offset );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_WRIMPBLKWRITEFAIL, 0, 1 );
        return(IMAP_WRITEERROR2);
        }

    return( imw_rc );
}


/***************************************************************
 * NAME: 	iagGet()
 *
 * FUNCTION:	Read the requested iag into the iag page buffer, allocate
 *		a data record for it, copy the pmap and inoext array from 
 *                   the page read into the data and then initialize the wmap 
 *                   in the data record.
 *
 * NOTES:	 
 *
 */
int iagGet( int     vol,            	        /* index in vopen array */
               int32  iag_num )             /* the iag number of the iag wanted */
{
    int ig_rc = 0;
    int32 bytes_needed;
    int32 allocated_from_bmap = 0;
    iag_data_t *iag_datarec;
    iag_t *iag_pg;
    int32  maplen, inoext_arrlen;

        /* 
         * read in the map page
         */
    ig_rc = iMapRead( vol, 
                             (iag_num + 1), 
                             (void  *)vopen[vol].fsimap_lst.iag_pbuf 
                            );
    if ( ig_rc != 0 )  {
        return(ig_rc);
        }
    vopen[vol].fsimap_lst.imap_pagenum = iag_num + 1;

        /* 
         * allocate and set up a data workspace for this page
         */
    bytes_needed = sizeof(iag_data_t);
    sprintf( message_parm_0, "(d) %lld", bytes_needed );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    fsck_send_msg( lrdo_ALLOC4IMAP, 0, 1 );
    ig_rc = alloc_storage( 	(uint32) bytes_needed, 
				(void **) &iag_datarec,
				&allocated_from_bmap 
				);

    if( (ig_rc != 0) || (iag_datarec == NULL) ) {  /* imap allocation failed */
        sprintf( message_parm_0, "(d) %lld", bytes_needed );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( lrdo_ALLOC4IMAPFAIL, 0, 1 );
        return(ENOMEM2);
        }  /* end imap allocation failed */
    else if( iag_datarec != NULL ) {  /* imap allocation successful */
        if( allocated_from_bmap ) {
            fsck_send_msg( lrdo_USINGBMAPALLOC4IMAP, 0, 0 );
            }
        }  /* end imap allocation successful */

    if( ig_rc == 0 ) {
        iag_pg = vopen[vol].fsimap_lst.iag_pbuf;
        maplen = sizeof(uint32) * LPERDMAP;
        inoext_arrlen = sizeof(pxd_t) * EXTSPERIAG;
        memset( (void *) &(iag_datarec->wmap), 
                     0, 
                     maplen
                    );
        memcpy( (void *) &(iag_datarec->pmap), 
                      (void *) &(iag_pg->pmap), 
                      maplen
                    );
        memcpy( (void *) &(iag_datarec->inoext), 
                      (void *) &(iag_pg->inoext), 
                      inoext_arrlen 
                    );
        vopen[vol].fsimap_lst.imap_wsp[(iag_num + 1)].imap_data = iag_datarec;
        }

    return( ig_rc );
}


/*
 * NAME:        updateMaps(vol)
 *
 * FUNCTION:    finalize and write out both imap and bmap.
 *
 * NOTE:        Imaps update must be before the bmap update
 *              because we need to change bmap while rebuild the imaps.
 */
updateMaps(int32 vol)   /* index in vopen array */
{
        int     rc;
        struct  dinode *dip;
        pxd_t pxd1;

        /*
         *      update fileset inode allocation map
         */
        /* read in the fileset imap inode (i_number = 16) in a buffer: */
        PXDaddress(&pxd1,
                (AITBL_OFF + (SIZE_OF_MAP_PAGE << 1)) >> vopen[vol].l2bsize);
        PXDlength(&pxd1, vopen[vol].lbperpage);
        if ((rc = bread(vol, pxd1, (void **)&dip , PB_READ)) !=0)
        {
                fsck_send_msg( lrdo_UMPREADIMAPINOFAIL, 0, 0 );
                return(IMAP_READERROR5);
        }

        /* locate the inode in the buffer page */
        dip += FILESYSTEM_I & 0x07;

        /* finalize the imap and write it out */
        if ((rc = writeImap(vol, vopen[vol].fsimap_lst, dip)) !=0)
        {
                fsck_send_msg( lrdo_UMPWRITEIMAPCTLFAIL, 0, 0 );
                return (rc);
        }

        /*
         *      if we were able to allocate enough storage for the BMap,
         *              update file system block allocation map
         *
         *      (Otherwise we'll let chkdsk be responsible for the BMap)
         */
        if( ! Insuff_memory_for_maps ) {  /* we do have a local BMap image */

            /* read in the bmap inode (i_number = 2) in a buffer: */
            PXDaddress(&pxd1, AITBL_OFF>>vopen[vol].l2bsize);
            if ((rc = bread(vol, pxd1, (void **)&dip , PB_READ)) !=0)  {
                    fsck_send_msg( lrdo_UMPREADBMAPINOFAIL, 0, 0 );
                    return(BMAP_READERROR4);
                    }

            /* locate the inode in the buffer page */
            dip += BMAP_I;

            /* finalize the bmap and write it out */
            if ((rc = writeBmap(vol, vopen[vol].bmap_ctl, dip)) !=0)  {
                    fsck_send_msg( lrdo_UMPWRITEBMAPCTLFAIL, 0, 0 );
                    }
            }  /* end we do have a local BMap image */

        return(rc);
}


/*
 * NAME:        writeImap()
 *
 * FUNCTION:    copy permanent map to work map. Rebuild control information
 *              for each iag_t page. Then rebuild the imap control page.
 *              We assume the iagnum is correct at the beginning of logredo.
 *
 */
int writeImap(   int    vol,                                 /* index in vopen array */
                      struct fsimap_lst fsimap,            /* fileset imap workspace */ 
                      struct dinode * dp)                   /* disk inode of imap */
{
        int wi_rc = 0;
        dinomap_t *imap_ctl;
        int32 i,k, iagpages, npages,fd, next_iag,npgs, rc;
        int64 nbytes, xaddr;
        uint32 actual;
        int16 iagfree, numinos, agno, xad_start;
        iag_t  *iagp, *iag_pg;
        xtpage_t *xtpg;
        char *p1, *buf_ptr;
        pxd_t pxd;
        int32 next_imap_page = 1;
        iag_data_t *iag_datarec;
        int32  maplen, inoext_arrlen;
    
        if (vopen[vol].status == FM_LOGREDO)  {
                fsck_send_msg( lrdo_WRIMPNOTRBLDGIMAP, 0, 0 );
                return(NOTREBUILDING_IMAP);
                }

        fsck_send_msg( lrdo_WRIMPSTART, 0, 0 );

        imap_ctl = fsimap.fsimapctrl;
        iagp = fsimap.iag_pbuf;
        iag_pg = fsimap.iag_pbuf2;
        maplen = sizeof(uint32) * LPERDMAP;
        inoext_arrlen = sizeof(pxd_t) * EXTSPERIAG;

        npages = dp->di_size >> L2PSIZE;

        /* the first page is imap control page, so the number of
         * iag_t pages is one less of npages.
         */
        iagpages = npages - 1;

        /* initialize the dinomap_t page
         */
        imap_ctl->in_freeiag = -1;

        /* iag_t.iagnum is zero origin. They are in order in the
         * imap file. So in_nextiag should be the last iag_t page
         * plus one. The last iag_t.iagnum is (npages - 2).
         */
        imap_ctl->in_nextiag = iagpages ;

        imap_ctl->in_numinos = 0;
        imap_ctl->in_numfree = 0;

        /* init imap_ctl->in_agctl[]. Although the aggregate
         * has only vopen[vol].numag, since the structure
         * has defined MAXAG, the initializarion will do
         * against MAXAG
         */
        for (k = 0; k < MAXAG; k++)
        {
                imap_ctl->in_agctl[k].inofree = -1;
                imap_ctl->in_agctl[k].extfree = -1;
                imap_ctl->in_agctl[k].numinos = 0;
                imap_ctl->in_agctl[k].numfree = 0;
        }

        /* process each iag_t page of the map.
         * rebuild AG Free Inode List, AG Free Inode Extent List,
         * and IAG Free List from scratch
         */

        for (k = 0; k < iagpages; k++)
        {
                     /*
                      * read in the IAG
                      */
                if( fsimap.imap_pagenum != next_imap_page ) {
                        rc = iMapRead( vol, next_imap_page, iagp );
                        if( rc != 0 ) {
                                return( rc );
                        }
                }
                fsimap.imap_pagenum = next_imap_page;
                     /*
                      * if the bit maps and inoext arrary for this iag are
                      * in memory, copy the pmap and inoext into the page
                      */
                iag_datarec = fsimap.imap_wsp[next_imap_page].imap_data;
                if( iag_datarec != NULL ) {
                        memcpy( (void *) &(iagp->pmap), 
                                      (void *) &(iag_datarec->pmap), 
                                      maplen
                                    );
                        memcpy( (void *) &(iagp->inoext), 
                                      (void *) &(iag_datarec->inoext), 
                                      inoext_arrlen 
                                    );
                        }
                next_imap_page++;

                iagfree = 0;
                agno = BLKNOTOAG(iagp->agstart, vopen[vol].l2agsize);
                updateImapPage(vol, iagp, &numinos, &iagfree);

                if ( iagfree ) /* all inodes are free, then this iag should
                                * be inserted into iag free list.
                                */
                {
                        iagp->inofreefwd = iagp->inofreeback = -1;
                        iagp->extfreefwd = iagp->extfreeback = -1;

                        iagp->iagfree = imap_ctl->in_freeiag;
                        imap_ctl->in_freeiag = iagp->iagnum;
                }
                else if (iagp->nfreeinos > 0 )
                {
                        if ( (next_iag=imap_ctl->in_agctl[agno].inofree) == -1 )
                           iagp->inofreefwd = iagp->inofreeback = -1;
                        else
                        {
                                /*
                                 * read in the IAG
                                 */
                           if( fsimap.imap_pagenum2 != (next_iag + 1) ) {
                                   rc = iMapRead( vol, (next_iag + 1), iag_pg );
                                   if( rc != 0 ) {
                                           return( rc );
                                           }
                                   fsimap.imap_pagenum2 = next_iag + 1;
                                   }

                           iagp->inofreefwd = next_iag;
                           iag_pg->inofreeback = iagp->iagnum;
                           iagp->inofreeback = -1;

                                /*
                                 * write out the IAG 
                                 */
                           rc = iMapWrite( vol, fsimap.imap_pagenum2, iag_pg );
                           if( rc != 0 ) {
                                    return( rc );
                                    }
                        }

                        imap_ctl->in_agctl[agno].inofree = iagp->iagnum;
                        imap_ctl->in_agctl[agno].numfree += iagp->nfreeinos;
                        imap_ctl->in_numfree += iagp->nfreeinos;
                }

                if ( numinos )
                {
                        imap_ctl->in_agctl[agno].numinos += numinos;
                        imap_ctl->in_numinos += numinos;
                }

                if ( iagp->nfreeexts > 0 && !iagfree )

                /* When an IAG is on the IAG free list, its nfreeexts
                 * is EXTSPERIAG which is > 0. But here we only consider
                 * those IAGs that are not on the IAG free list
                 */

                {
                        if ( (next_iag=imap_ctl->in_agctl[agno].extfree) == -1 )
                           iagp->extfreefwd = iagp->extfreeback = -1;
                        else
                        {
                                /*
                                 * read in the IAG
                                 */
                           if( fsimap.imap_pagenum2 != (next_iag + 1) ) {
                                   rc = iMapRead( vol, (next_iag + 1), iag_pg );
                                   if( rc != 0 ) {
                                           return( rc );
                                           }
                                   fsimap.imap_pagenum2 = next_iag + 1;
                                   }

                           iagp->extfreefwd = next_iag;
                           iag_pg->extfreeback = iagp->iagnum;
                           iagp->extfreeback = -1;

                                /*
                                 * write out the IAG 
                                 */
                           rc = iMapWrite( vol, fsimap.imap_pagenum2, iag_pg );
                           if( rc != 0 ) {
                                    return( rc );
                                    }
                        }
                        imap_ctl->in_agctl[agno].extfree = iagp->iagnum;
                }
                /*
                 * write out the IAG 
                 */
                rc = iMapWrite( vol, fsimap.imap_pagenum, iagp );
                if( rc != 0 ) {
                         return( rc );
                         }
        }

        /* 
         * And now, write the control page to the device
         */
        rc = iMapWrite( vol, 0, fsimap.fsimapctrl );
        if( rc != 0 ) {
                return( rc );
                }

        fsck_send_msg( lrdo_WRIMPDONE, 0, 0 );

        return (0);
}


/*
 * NAME:        updateImapPage()
 *
 * FUNCTION:    copies the pmap to the wmap in each iag_t since pmap is
 *              updated at the logredo process. Now we need to
 *              reconstruct the nfreeinos and nfreeexts fields in iag_t.
 */
int
updateImapPage(
int32   vol,    /* index in vopen array  */
iag_t *p,       /* pointer to the current iag_t page */
int16 *numinos, /* number of backed inodes for this iag */
int16 *iagfree) /* to tell if all inodes in this iag are free
                 * so that this iag need to put on iag free list */
{
        int rc = 0;
        uint i, sword, mask;
        uint16 allfree;
        uint8  *cp;
        uint64 pxdaddr;

        /* copy the perm map to the work map.
         */
        p->nfreeinos = 0;
        p->nfreeexts = 0;
        allfree = 0;
        *numinos = 0;
        for ( i = 0; i < EXTSPERIAG; i++)
        {
                p->wmap[i] = p->pmap[i];
                sword = i >> L2EXTSPERSUM;
                mask = UZBIT_32 >> ( i & (EXTSPERSUM - 1));
                if ( p->pmap[i] == 0 )
                {
                /* There can be the cases that p->pmap[i == 0 but
                 * addressPXD(&p->inoext[i]) != 0.
                 * This could happen that the log sync point has passed
                 * the lastinode free log rec for this ino extent, but
                 * we have not reach the hwm so that no NOREDOPAGE
                 * log rec is written out yet before the system crash.
                 * At the logredo time, we have to null out the
                 * address of p->inoext[i] if p->pmap[i] is zero.
                 */
                        if ((pxdaddr= addressPXD(&p->inoext[i])) != 0 )
                        {
                                rc = markBmap((dmap_t *)vopen[vol].bmap_ctl,
                                                 p->inoext[i], 0, vol);
                                if( rc != 0 ) {
                                        return( rc );
                                        }
                                PXDaddress(&p->inoext[i], 0);
                        }
                        p->extsmap[sword] &= ~mask;
                        p->inosmap[sword] |= mask;
                        p->nfreeexts++;
                        allfree++;

                }
                else
                if ( p->pmap[i] == ONES )
                {
                        if ( addressPXD(&p->inoext[i]) != 0 )
                        {
                                p->inosmap[sword] |= mask;
                                p->extsmap[sword] |= mask;
                                *numinos += INOSPEREXT;
                        }
                        else
                                fsck_send_msg( lrdo_RBLDGIMAPERROR2, 0, 0 );
                }
                else
                if ( ~p->pmap[i] && (addressPXD(&p->inoext[i]) != 0) )
                   /* there is some bits are zeroes */
                {
                        p->extsmap[sword] |= mask;
                        p->inosmap[sword] &= ~mask;
                        *numinos += INOSPEREXT;
                        cp = (uint8 *)&p->pmap[i];
                        p->nfreeinos += (maptab[*cp] + maptab[*(cp+1)]
                                        + maptab[*(cp+2)] + maptab[*(cp+3)]);
                }
                else
                        fsck_send_msg( lrdo_RBLDGIMAPERROR1, 0, 0 );
        }

        if (allfree ==  EXTSPERIAG)
                *iagfree = 1;

        return (0);
}


/*
 * NAME:        writeBmap()
 *
 * FUNCTION:    copy pmap to wmap in dmap pages,
 *              rebuild summary tree of dmap and dmap control pages, and
 *              rebuild bmap control page.
 */
int writeBmap(
        int32   vol,            /* index in vopen array */
        dbmap_t *bmap,          /* pointer to the bmap control page */
        struct dinode   *dip)   /* disk inode of map */
{
        int32   rc, fd, i, j, k, n;
        int64   fssize, h_fssize, nblocks, nbytes, xaddr;
        int32   npages;
        char    *p;
        dmapctl_t       *l2ptr, *l1ptr, *l0ptr;
        dmap_t  *dmap;
        int8    *l0leaf, *l1leaf, *l2leaf;
        int32   agno, l2agsize;
        int32   actags, inactags, l2nl;
        int64   ag_rem, actfree, inactfree, avgfree;
        xtpage_t *xp;   /* xtree page */
        pxd_t   pxd;
        int32    next_bmap_page = 1;
        dmap_bitmaps_t *dmap_bitrec;
        int32  bitmaplen;

        if (vopen[vol].status == FM_LOGREDO)  {
               fsck_send_msg( lrdo_WRBMPNOTRBLDGBMAP, 0, 0 );
               return(NOTREBUILDING_BMAP);
               }

        fsck_send_msg( lrdo_WRBMPSTART, 0, 0 );

	/*
	 * set the pointers to the corresponding page buffers
	 */
        l2ptr =  vopen[vol].L2_pbuf;
        l1ptr =  vopen[vol].L1_pbuf;
        l0ptr =  vopen[vol].L0_pbuf;
        dmap = vopen[vol].dmap_pbuf;
        bitmaplen = sizeof(uint32) * LPERDMAP;

        /*
         *      validate file system size and bmap file size
         *
         * Since the di_size includes the mkfs hidden dmap page
         * and its related control pages, when calculate the
         * l_totalpages we pretend fs size is fssize plus BPERDMAP.
         * The macro give the page index # (zero origin )
         * so the (+ 1) gives the total pages.
         */
        h_fssize = vopen[vol].fssize + BPERDMAP ;
        npages = BLKTODMAPN(h_fssize-1) + 1;

        if (npages > (dip->di_size >> L2PSIZE))
        {
               fsck_send_msg( lrdo_WRBMPBADMAPSIZE, 0, 0 );
                return(BMAP_WRITEERROR1);
        }

        /*
         *      reconstruct bmap extended information from bit map
         */
        fssize = vopen[vol].fssize;

        /*
         * initialize bmap control page.
         *
         * all the data in bmap control page should exclude
         * the mkfs hidden dmap page.
         */
        bmap->dn_mapsize = fssize;
        bmap->dn_maxlevel = BMAPSZTOLEV(bmap->dn_mapsize);
        bmap->dn_nfree = 0;
        bmap->dn_agl2size = vopen[vol].l2agsize;
        l2agsize = bmap->dn_agl2size;
        bmap->dn_agsize = vopen[vol].agsize;
        bmap->dn_numag = vopen[vol].numag;
        for (agno = 0; agno < bmap->dn_numag; agno++)
                bmap->dn_agfree[agno] = 0;

        /*
         * reconstruct summary tree and control information
         * in dmap_t pages and dmapctl_t pages
         */
        nblocks = fssize;
        p = (char *)bmap + sizeof(dbmap_t);

        if( vopen[vol].L2_pagenum != next_bmap_page ) {
                rc = bMapRead( vol, next_bmap_page, l2ptr );
                if( rc != 0 ) {
                        return( rc );
                        }
                }
        vopen[vol].L2_pagenum = next_bmap_page;
        next_bmap_page++;
        l2leaf = l2ptr->stree + CTLLEAFIND;

        /* reconstruct each L1 in L2 */
        p += SIZEOFDMAPCTL  ; /* the L1.0 */
        for (k = 0; k < LPERCTL; k++)
        {
                if( vopen[vol].L1_pagenum != next_bmap_page ) {
                        rc = bMapRead( vol, next_bmap_page, l1ptr );
                        if( rc != 0 ) {
                                return( rc );
                                }
                        }
                vopen[vol].L1_pagenum = next_bmap_page;
                next_bmap_page++;
                l1leaf = l1ptr->stree + CTLLEAFIND;

                /* reconstruct each L0 in L1 */
                p += SIZEOFDMAPCTL; /* 1st L0 of L1.k  */
                for (j = 0; j < LPERCTL; j++)
                {
                        if( vopen[vol].L0_pagenum != next_bmap_page ) {
                                rc = bMapRead( vol, next_bmap_page, l0ptr );
                                if( rc != 0 ) {
                                        return( rc );
                                        }
                                }
                        vopen[vol].L0_pagenum = next_bmap_page;
                        next_bmap_page++;
                        if( l0ptr->leafidx != CTLLEAFIND )  {
                                sprintf( message_parm_0, "(d) %d", k  );
                                msgprms[0] = message_parm_0;
                                msgprmidx[0] = 0;
                                sprintf( message_parm_1, "(d) %d", j  );
                                msgprms[1] = message_parm_1;
                                msgprmidx[1] = 0;
                                sprintf( message_parm_2, "(d) %d", l0ptr->leafidx  );
                                msgprms[2] = message_parm_2;
                                msgprmidx[2] = 0;
                                fsck_send_msg( lrdo_WRBMPBADLFIDX0, 0, 3 );
                        	return ILLEGAL_LEAF_IND0;
                                }
                        l0leaf = l0ptr->stree + l0ptr->leafidx;

                        /*
                         * reconstruct each dmap in L0
                         */
                        for (i = 0; i < LPERCTL; i++)
                        {
			/*
			 * read in the dmap page
			 */
                                if( vopen[vol].dmap_pagenum != next_bmap_page ) {
                                        rc = bMapRead( vol, next_bmap_page, dmap );
                                        if( rc != 0 ) {
                                                return( rc );
                                                }
                                        }
                                vopen[vol].dmap_pagenum = next_bmap_page;
			/*
			 * if the bit maps for this dmap page are
			 * in memory, copy the pmap into the page
			 */
                                dmap_bitrec = vopen[vol].bmap_wsp[ next_bmap_page
                                                                                   ].dmap_bitmaps;
                                if( dmap_bitrec != NULL ) {
                                        memcpy( (void *) &(dmap->pmap), 
                                                      (void *) &(dmap_bitrec->pmap), 
                                                      bitmaplen
                                                      );
                                        }
                                next_bmap_page++;
                                /*
                                 * reconstruct the dmap page, and
                                 * initialize corresponding parent L0 leaf
                                 */
                                n = MIN(nblocks, BPERDMAP);
                                rc = updDmapPage(dmap, n, l0leaf);
                                if( rc != 0 )  { 
                                        sprintf( message_parm_0, "(d) %d", k  );
                                        msgprms[0] = message_parm_0;
                                        msgprmidx[0] = 0;
                                        sprintf( message_parm_1, "(d) %d", j  );
                                        msgprms[1] = message_parm_1;
                                        msgprmidx[1] = 0;
                                        sprintf( message_parm_2, "(d) %d", i );
                                        msgprms[2] = message_parm_2;
                                        msgprmidx[2] = 0;
                                        fsck_send_msg( lrdo_RBLDGDMAPERROR, 0, 3 );
                                        return(DMAP_UPDATEFAIL);
                                        }

                                bmap->dn_nfree += dmap->nfree;
                                agno = dmap->start >> l2agsize;
                                bmap->dn_agfree[agno] += dmap->nfree;

                                l0leaf++;
				/*
				 * write out the dmap page
				 */
                                rc = bMapWrite( vol, vopen[vol].dmap_pagenum, dmap );
                                if( rc != 0 ) {
                                        return( rc );
                                        }
                                vopen[vol].dmap_pagenum = -1;

                                nblocks -= n;
                                if (nblocks == 0)
                                        break;
                        } /* for each dmap in a L0 */

                        /*
                         * build current L0 page from its leaves, and
                         * initialize corresponding parent L1 leaf
                         */
                        *l1leaf = adjTree(l0ptr, L2LPERCTL, L2BPERDMAP);

                        /*
                         * write out the L0 page
                         */
                        rc = bMapWrite( vol, vopen[vol].L0_pagenum, l0ptr );
                        if( rc != 0 ) {
                                return( rc );
                                }
                        vopen[vol].L0_pagenum = -1;

                        if (nblocks)
                                l1leaf++; /* continue for next L0 */
                        else
                        {
                                /* more than 1 L0 ? */
                                if (j > 0)
                                        break; /* build L1 page */
                                else
                                {
                                        /* initialize global bmap page */
                                        bmap->dn_maxfreebud = *l1leaf;
                                        goto finalize;
                                }
                        }
                } /* for each L0 in a L1 */

                /*
                 * build current L1 page from its leaves, and
                 * initialize corresponding parent L2 leaf
                 */
                *l2leaf = adjTree(l1ptr, L2LPERCTL, L2MAXL0SIZE);

                /*
                 * write out the L1 page to disk
                 */
                rc = bMapWrite( vol, vopen[vol].L1_pagenum, l1ptr );
                if( rc != 0 ) {
                        return( rc );
                        }
                vopen[vol].L1_pagenum = -1;

                if (nblocks)
                        l2leaf++; /* continue for next L1 */
                else
                {
                        /* more than 1 L1 ? */
                        if (k > 0)
                                break; /* build L2 page */
                        else
                        {
                                /* initialize global bmap page */
                                bmap->dn_maxfreebud = *l2leaf;
                                goto finalize;
                        }
                }
        } /* for each L1 in a L2 */

        /* initialize global bmap page */
        bmap->dn_maxfreebud = adjTree(l2ptr, L2LPERCTL, L2MAXL1SIZE);

        /*
         * write out the L2 page to disk
         */
        rc = bMapWrite( vol, vopen[vol].L2_pagenum, l2ptr );
        if( rc != 0 ) {
                return( rc );
                }
        vopen[vol].L2_pagenum = -1;

        /*
         * finalize bmap control page
         */
finalize:
        /*
         * compute dn_maxag: highest active ag number
         * (the rightmost allocation group with blocks allocated in it);
         */
        /* get last ag number: assert(bmap->dn_numag >= 1); */
        i = bmap->dn_numag - 1;

        /* is last ag partial ag ? */
        ag_rem = bmap->dn_mapsize & (bmap->dn_agsize - 1);
        if (ag_rem)
        {
                /* is last ag active ? */
                if (bmap->dn_agfree[i] < ag_rem)
                {
                        bmap->dn_maxag = i;
                        goto agpref;
                }
                else
                        i--;
        }

        /* scan backward for first ag with blocks allocated:
         * (ag0 must be active from allocation of map itself)
         */
        for ( ; i >= 0; i--)
        {
                if (bmap->dn_agfree[i] < bmap->dn_agsize)
                        break;
        }
        bmap->dn_maxag = i;

        /*
         * compute db_agpref: preferred ag to allocate from
         * (the leftmost ag with average free space in it);
         */
agpref:
        /* get the number of active ags and inacitve ags */
        actags = bmap->dn_maxag+1;
        inactags = bmap->dn_numag - actags;

        /* determine how many blocks are in the inactive allocation
         * groups. in doing this, we must account for the fact that
         * the rightmost group might be a partial group (i.e. file
         * system size is not a multiple of the group size).
         */
        inactfree = (inactags && ag_rem) ? ((inactags-1) << l2agsize) + ag_rem
                                         : inactags << l2agsize;

        /* determine how many free blocks are in the active
         * allocation groups plus the average number of free blocks
         * within the active ags.
         */
        actfree = bmap->dn_nfree - inactfree;
        avgfree = actfree / actags;

        /* if the preferred allocation group has not average free space.
         * re-establish the preferred group as the leftmost
         * group with average free space.
         */
        if (bmap->dn_agfree[bmap->dn_agpref] < avgfree)
        {
                for (bmap->dn_agpref = 0; bmap->dn_agpref < actags;
                     bmap->dn_agpref++)
                {
                        if (bmap->dn_agfree[bmap->dn_agpref] >= avgfree)
                                break;
                }
                assert( bmap->dn_agpref < bmap->dn_numag );
        }

        /*
         * compute db_aglevel, db_agheigth, db_width, db_agstart:
         * an ag is covered in aglevel dmapctl summary tree,
         * at agheight level height (from leaf) with agwidth number of nodes
         * each, which starts at agstart index node of the smmary tree node
         * array;
         */
        bmap->dn_aglevel = BMAPSZTOLEV(bmap->dn_agsize);
        l2nl = bmap->dn_agl2size - (L2BPERDMAP + bmap->dn_aglevel * L2LPERCTL);
        bmap->dn_agheigth = l2nl >> 1;
        bmap->dn_agwidth = 1 << (l2nl - (bmap->dn_agheigth << 1));
        for (i = 5 - bmap->dn_agheigth, bmap->dn_agstart=0, n = 1; i > 0; i--)
        {
                bmap->dn_agstart += n;
                n <<= 2;
        }

        /*
         *      write out the control page to disk
         */
        rc = bMapWrite( vol, 0, (void  *)bmap );
        if(rc != 0 ) {
                return( rc );
                }

        fsck_send_msg( lrdo_WRBMPDONE, 0, 0 );

        return (0);
}


/*
 * NAME:        updDmapPage()
 *
 * FUNCTION:    copies the pmap to the wmap in the dmap.
 *              Rebuild the summary tree for this dmap.
 *              initializes the other fields for dmap_t.
 */
int updDmapPage(
        dmap_t  *p0,            /* pointer to this dmap_t page */
        int32   nblk,           /* num blks covered by this dmap */
        int8    *treemax)       /* filled in with max free for this dmap */
{
        dmaptree_t  *tp;  /* pointer to dmap tree */
        int8       *cp;
        uint8       *ucp;
        int16       w;

        /* update nblock field according to nblk.
         * Note that all the dmap page have the same nblock (BPERDMAP)
         * except the last dmap. Without the extendfs the
         * last dmap nblock is set properly by mkfs.
         * With the extendfs, logredo need to take care of
         * nblock field in case the system crashed after
         * the transaction for extendfs is committed, but
         * before the fs superblock was sync-written out.
         * In that case, we need to reset the last dmap
         * nblock field according to the fssize in the
         * unchanged fs superblock.
         */
        p0->nblocks = nblk;
        tp = &p0->tree;

        /* copy the perm map to the work map.
         * count the num of free blks in this dmap
         * Set the initial state for the leaves of the dmap tree
         * according to the current state of pmap/wmap words.
         */
        p0->nfree = 0;  /* init nfree field */

        if( (int32)(tp->leafidx) != (int32)(LEAFIND) ) {
            fsck_send_msg( lrdo_UPDMPBADLFIDX, 0, 0 );
            return ILLEGAL_LEAF_IND1;
            }
        cp = tp->stree + tp->leafidx;  /*cp points to first leaf of dmap tree*/
        for (w = 0; w < LPERDMAP; w++)
        {
                p0->wmap[w] = p0->pmap[w];
                *(cp+w) = maxBud((char *)&p0->wmap[w]);
                if (~p0->wmap[w])
                {
                        ucp = (uint8 *)&p0->wmap[w];
                        p0->nfree += (maptab[*ucp] + maptab[*(ucp+1)]
                                     + maptab[*(ucp+2)] + maptab[*(ucp+3)]);
                }
        }

        /*
         * With the leaves of the dmap initialized
         * rebuild the dmap's tree.
         */
        *treemax = adjTree( (dmapctl_t *)tp, L2LPERDMAP, BUDMIN );
        return (0);
}

/*
 * NAME:        rXtree()
 *
 * FUNCTION:    return buffer page of leftmost leaf page of the xtree
 *              by traversing down leftmost path of xtree;
 *
 * RETURN:      buffer pointer to the first leaf of the xtree.
 */
rXtree(
        int32   vol,            /* index in vopen array */
        struct dinode *dp,      /* disk inode of map    */
        xtpage_t **first_leaf)  /* pointer to first leaf page of map xtree */
{
        int             rc;
        xtpage_t        *p;
        caddr_t         buf_ptr;
        pxd_t           pxd;

        /* start from root in dinode */
        p = (xtpage_t *)&dp->di_btroot;
        /* is this page leaf ? */
        if (p->header.flag & BT_LEAF)
                goto out;

        /* get the pxd of leftmost child page */
        PXDlength(&pxd, vopen[vol].lbperpage);
        PXDaddress(&pxd, addressXAD(&p->xad[XTENTRYSTART]));

        /*
         * traverse down leftmost child node to the leftmost leaf of xtree
         */
        do
        {
                /* read in the leftmost child page */
                if ((rc = bread(vol, pxd, (void **)&buf_ptr, PB_READ)) != 0)
                {
                        fsck_send_msg( lrdo_RXTREADLFFAIL, 0, 0 );
                        return(MINOR_ERROR);
                }

                p = (xtpage_t *)buf_ptr;
                /* is this page leaf ? */
                if (p->header.flag & BT_LEAF)
                        break;
                else /* get the pxd of leftmost child page */
                {
                        PXDlength(&pxd, vopen[vol].lbperpage);
                        PXDaddress(&pxd, addressXAD(&p->xad[XTENTRYSTART]));
                }

        } while (!(p->header.flag & BT_LEAF));

out:
        *first_leaf = p;

        return (0);
}



/*
 * NAME:        adjTree
 *
 * FUNCTION:    rebuild the tree of a dmap or dmapctl. the tree is fixed size.
 *
 *              for dmap tree, the number of leaf nodes are always LPERDMAP
 *              for a fixed aggregate size, the last dmap page may not
 *              used fully. The the partial unused pmap/wmap bits are marked
 *              as allocated.
 *              For dmapctl_t tree, the number of leaf nodes are always
 *              LPERCTL. If not 1024 dmaps exist, then the unused leaf
 *              nodes are marked as "-1".
 *
 * PARAMETERS:
 *      tp      - pointer to the current dmap page or dmapctl page
 *      l2leaves- Number of leaf nodes as a power of 2:
 *                for dmap, this is always L2LPERDMAP,
 *                for dmapctl, this is always L2LPERCTL;
 *      l2min   - Number of blocks actually covered by a leaf of the tree
 *                as a power of 2
 *
 * NOTES: This routine first works against the leaves of the tree to calculate
 *      the maximum free string for leaf buddys.  Once this is accomplished the
 *      values of the leaf nodes are bubbled up the tree.
 *
 * RETURNS:
 */
signed char adjTree(
dmapctl_t       *tp,
int32             l2leaves,
int32            l2min )
{
        int nleaves, l2max, nextbud, budsz, index;
        int l2free, leaf, nparent, firstp;
        int16 nparents;
        signed char *cp0, *pp;
        signed char *cp = tp->stree; /* Pointer to the top of the stree */

    /*
     * Determine the number of leaves of the tree
     */
    nleaves = (1 << l2leaves);

    /*
     * Determine the maximum free string possible for the leaves.
     */
    l2max = l2min + l2leaves;

    /*
     * Combine buddies starting with a buddy size of 1 (i.e. two leaves).
     *
     * At a buddy size of 1 two buddy leaves can be combined if both buddies
     * have a maximum free of l2min; the combination will result in the
     * left buddy leaf having a maximum free of l2min+1 and the right
     * buddy leaf changing value to '-1'.
     *
     * After processing  all buddies for a current size, process buddies
     * at the next higher buddy size (i.e. current size * 2) against
     * the next maximum free level (i.e. current free + 1 ).
     *
     * This continues until the maximum possible buddy
     * combination yields maximum free.
     *
     * since the stree is fixed size, the index of the first leaf
     * is in fixed position, tp->leafidx has this value.
     */
        for(l2free=l2min, budsz=1; l2free < l2max; l2free++, budsz = nextbud )
        {
                nextbud = budsz << 1;
                for( cp0 = cp + tp->leafidx, index = 0; index < nleaves;
                        index += nextbud, cp0 += nextbud )
                {
                        if( *cp0 == l2free && *(cp0 + budsz) == l2free )
                        {
                                *cp0 = l2free + 1;
                                *(cp0 + budsz) = -1;
                        }
                }
        }

    /*
     * With the leaves having maximum free values,
     * bubble this information up the stree.
     * Starting at the leaf node level, each four nodes form a group and
     * have a parent in the higher level. The parent holds the maximum
     * free value among its four children.
     * All lower level nodes are processed in this fashion until we
     * reach the root.
     */
        for( leaf = tp->leafidx, nparents = nleaves >> 2;
                nparents > 0; nparents >>= 2, leaf = firstp )
        {
                /* get the index of the first parent of the current
                 * leaf level
                 */
                firstp = (leaf - 1) >> 2;

                /*
                 * Process all nodes at the current leaf level.
                 * Parent node pp has the maximum value of its
                 * four children nodes.
                 */
                for( cp0 = cp + leaf, pp = cp + firstp, index = 0;
                        index < nparents; index++, cp0 += 4, pp++ )
                {
                        *pp = TREEMAX(cp0);
                }
        }

    return(*cp);
}


/*
 * NAME:        maxBud
 *
 * FUNCTION: Determines the maximum binary buddy string of free
 *              bits within a 32-bits word of the pmap/wmap.
 *
 * PARAMETERS:
 *      cp      - Pointer to wmap or pmap word.
 *
 * RETURNS: Maximum binary buddy of free bits within a dmap word.
 */
static int32
maxBud( char *cp )
{
        /* check if the dmap word is all free. if so, the
         * free buddy size is BUDMIN.
         */
        if (*((uint32 *)cp) == 0)
                return(BUDMIN);

        /* check if the dmap word is half free. if so, the
         * free buddy size is BUDMIN-1.
         */
        if (*((unsigned short *)cp) == 0 || *((unsigned short *)cp+1) == 0)
                return(BUDMIN-1);

        /* not all free or half free. determine the free buddy
         * size thru table lookup using quarters of the dmap word.
         */
        return(MAX(MAX(budtab[*cp],budtab[*(cp+1)]),
                        MAX(budtab[*(cp+2)],budtab[*(cp+3)])));
}


/*
 * NAME:        bread ()
 *
 * FUNCTION:    return with buf set to pointer of page in buffer pool
 *              containing disk page specified by pxd.
 *              the parameter update specifies the caller's intentions.
 *
 * NOTE:        offset_t is "long long" type.
 */
int bread(
        int32   vol,            /* index in vopen (minor of aggregate)  */
        pxd_t   pxd,            /* on-disk page pxd                     */
        void    **buf,          /* set to point to buffer pool page     */
        int32   update)         /* true if buffer will be modified      */
{
        int32   rc, k, hash, oldhash, nxt, prv, found, head, fd;
        int32   nblocks, nbytes;
        int64   blkno; /* number of agge. blks in pxd*/

        /* verify that pxd is within aggregate range */
        nblocks = lengthPXD(&pxd);
        blkno = addressPXD(&pxd);
        if (vopen[vol].bmap_ctl != NULL &&
            (blkno + nblocks ) > vopen[vol].fssize)
        {
                fsError(DBTYPE,vol,blkno);
                sprintf( message_parm_0, "0x0%llx", blkno );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_BRDBADBLOCK, 0, 1 );
                return(MINOR_ERROR);
        }

        /* search buffer pool for specified page */
        hash = (blkno ^ vol) & bhmask;
        for (k = bhash[hash]; k != 0; k = bufhdr[k].hnext)
        {
                if (addressPXD(&bufhdr[k].pxd) == blkno &&
                    lengthPXD(&bufhdr[k].pxd) >= nblocks &&
                    bufhdr[k].vol == vol)
                        break;
        }

        /* was it in buffer pool ? */
        found = (k != 0);
        k = (found) ? k : bufhdr[0].prev;

        /* remove k from current position in lru list */
        nxt = bufhdr[k].next;
        prv = bufhdr[k].prev;
        bufhdr[nxt].prev = prv;
        bufhdr[prv].next = nxt;

        /* move k to head of lru list */
        head = bufhdr[0].next;
        bufhdr[k].next = head;
        bufhdr[head].prev = k;
        bufhdr[k].prev = 0;
        bufhdr[0].next = k;

        /* bufhdr[k] describes buffer[k-1] */
        *buf = &buffer[k-1];

        /*
         * buffer has been reclaimed: update modify bit and return
         */
        if (found)
        {
                bufhdr[k].modify |= update;
                return (0);
        }

        /*
         * buffer is to be recycled:
         */
        /* write it out if it was modified */
        if (bufhdr[k].inuse && bufhdr[k].modify)
        {
                if ((rc = bflush(k, &buffer[k - 1])) !=0)
                        return (rc);
        }

        /* remove it from hash chain if necessary.
         * hprev is 0 if it is at head of hash chain.
         */
        if (bufhdr[k].inuse)
        {
                nxt = bufhdr[k].hnext;
                prv = bufhdr[k].hprev;
                if (prv == 0)
                {
                    oldhash = (bufhdr[k].vol ^ addressPXD(&bufhdr[k].pxd)) & bhmask;
                    bhash[oldhash] = nxt;
                }
                else
                {
                        bufhdr[prv].hnext = nxt;
                }

                /* next assign is ok even if nxt is 0 */
                bufhdr[nxt].hprev  = prv;

        }

        /* insert k at head of new hash chain */
        head = bhash[hash];
        bufhdr[k].hnext = head;
        bufhdr[k].hprev = 0;
        bufhdr[head].hprev = k;   /* ok even if head = 0 */
        bhash[hash] = k;

        /* fill in bufhdr with new data and read the page in */
        bufhdr[k].vol = vol;
        bufhdr[k].pxd  = pxd;
        bufhdr[k].inuse = 1;
        bufhdr[k].modify = update;

        fd = vopen[vol].fd;
        nbytes = nblocks << vopen[vol].l2bsize;
        rc = ujfs_rw_diskblocks(fd, (uint64)(blkno<< vopen[vol].l2bsize),
                        (unsigned)nbytes, (char *)&buffer[k-1], GET);
        if (rc != 0)
        {
                fsError(IOERROR, vol, blkno);
                sprintf( message_parm_0, "0x0%llx", blkno );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_BRDREADBLKFAIL, 0, 1 );
                return(MINOR_ERROR);
        }

        return (0);
}
