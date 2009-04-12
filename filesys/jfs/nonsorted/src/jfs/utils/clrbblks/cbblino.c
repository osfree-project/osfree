/* $Id: cbblino.c,v 1.2 2004/03/21 02:43:32 pasha Exp $ */

static char *SCCSID = "@(#)1.2  9/12/98 11:51:30 src/jfs/utils/clrbblks/cbblino.c, jfscbbl, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		cbblino.c
 *
 *   COMPONENT_NAME: 	jfscbbl
 *
 *   FUNCTIONS:	process_BadBlockInode
 *		process_FilesetInodes
 *		 
 *              process_Inode
 *              process_Inode_Dtree
 *              process_Inode_EA
 *              process_Inode_Xtree
 *              we_Couldnt_Relocate
 *              we_Did_Relocate
 *              we_DidntTryTo_Relocate
 *
 */

#include "clrbblks.h"

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *	device and file system under clrbblks
  */
extern char	LVName[];	// device name

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *	inode allocation map
  */
extern IMap_t	IMap;		// inode allocation map control page
extern IMap_t	*iMap;


/* 
 * bitmap page control 
 */
extern iag_t	iagbuf;  

/* 
 * inode extent buffer 
 */
extern int32	IXNumber;
extern dinode_t ixbuf[];

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * fsck aggregate info structure pointer
  *
  *      defined in xchkdsk.c
  */
extern struct cbbl_agg_record *agg_recptr;


/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */
int32 process_Inode ( int8, dinode_t * ); 
int32 process_Inode_Dtree( dinode_t *, int8 );
int32 process_Inode_EA( dinode_t *, int8, int8 * );
int32 process_Inode_Xtree( dinode_t *, int8 );
int32 we_Couldnt_Relocate( int64, int64, cbbl_bdblk_recptr, int32 );
int32 we_Did_Relocate( int64, int64, cbbl_bdblk_recptr );
int32 we_DidntTryTo_Relocate( int64, int64, cbbl_bdblk_recptr );

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */


/****************************************************************************
 * NAME: 	process_BadBlockInode
 *
 * FUNCTION:	Reads in the inode extent containing the JFS Bad Block
 *		inode and calls process_Inode() to scan for blocks on the
 *		bad block list and handle any that are detected.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 process_BadBlockInode ( ) 
{
    int32	pbbi_rc = 0;
    dinode_t	*bb_inoptr = NULL;
    int32	i;
    buf_t	*bp;
    int8	isBadBlockInode = -1;
	
        /*
         * read in the aggregate bad block inode (i_number = BADBLOCK_I)
         */
    i = BADBLOCK_I / INOSPERPAGE;
    pbbi_rc = bRawRead( LVHandle, 
			  (int64)(AITBL_OFF + PAGESIZE * i), 
			  PAGESIZE, &bp
			  );
    if( pbbi_rc )  {
        return( CBBL_CANTREADBBINO );
	}
	
        /* 
	 * locate the inode in the buffer page 
	 */
    bb_inoptr = (dinode_t *)bp->b_data;
    bb_inoptr += BADBLOCK_I & (INOSPERPAGE - 1);
	
        /* 
	 * check for list entries already assigned
	 * to this inode
	 */
    pbbi_rc = process_Inode( isBadBlockInode, bb_inoptr );

    return( pbbi_rc );
}				/* end process_BadBlockInode() */


/****************************************************************************
 * NAME: 	process_FilesetInodes 
 *
 * FUNCTION:	Reads in the fileset inode extents, one by one, and for
 *		each in-use inode, calls process_Inode() to scan for 
 *		blocks on the bad block list and handle any that are 
 *		detected.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 process_FilesetInodes ( ) 
{
    int32	pfsi_rc = 0;
    dinode_t	*inoptr = NULL;
    dinomap_t	*icp;
    int64	xaddr;
    int32	xlen;
    int8	isBadBlockInode = 0;

	/*
	 * scan each IAG in the file set 
	 */
    icp = (dinomap_t *)&IMap.ctl; 
    for ( agg_recptr->iag_idx = 0; 
	  (	(agg_recptr->iag_idx < icp->in_nextiag) && 
		(pfsi_rc == 0) &&
		(agg_recptr->bdblk_baltree.seq_list != NULL)	); 
	  agg_recptr->iag_idx++) {  /* for each IAG */

	 	/* 
		 * read in the next IAG 
		 */
	pfsi_rc = readIMapSequential(&iagbuf);
		/*
		 * process the extents described by the IAG
		 */
	for ( agg_recptr->extent_idx = 0; 
	     (  	(agg_recptr->extent_idx < EXTSPERIAG) && 
		(pfsi_rc == 0) &&
		(agg_recptr->bdblk_baltree.seq_list != NULL)	); 
	    agg_recptr->extent_idx++) {  /* for each extent in the IAG */
		/*
		 * Get the address and length of the extent
		 */
	    xaddr = addressPXD(&iagbuf.inoext[agg_recptr->extent_idx]);
	    xlen = lengthPXD(&iagbuf.inoext[agg_recptr->extent_idx]);
		/* 
		 * If the extent isn't allocated, bump current inode number
		 */
	    if( xaddr == 0 ) {  /* not allocated */
		agg_recptr->this_inonum += INOSPEREXT;
		}
	    else {  /* extent is allocated */
		/*
		 * Otherwise, read in the inode extent
		 */
		pfsi_rc = pRead(fsMount, xaddr, xlen, (void *)ixbuf);
			/*
			 * process the inodes in the extent 
			 */
		for ( agg_recptr->inode_idx = 0; 
	 	      (	(agg_recptr->inode_idx < INOSPEREXT) && 
			(pfsi_rc == 0) &&
			(agg_recptr->bdblk_baltree.seq_list != NULL)	); 
		      agg_recptr->inode_idx++) {  /* 
				* for each inode in the extent
				*/
		    inoptr = &ixbuf[agg_recptr->inode_idx];
			/*
			 * if the inode isn't in use, just
			 * increment the inode number
			 */
		    if( (inoptr->di_nlink == 0) || 
			(inoptr->di_inostamp != DIIMap.di_inostamp) ) {
			agg_recptr->this_inonum += 1;
			}
			/*
			 * otherwise, scan the inode for bad blocks 
			 * allocated to it and, if possible, relocate
			 * their contents and get them transferred to 
			 * the JFS Bad Block Inode 
			 */
		    else {  /* the inode is in use */
			pfsi_rc = process_Inode( isBadBlockInode, inoptr );
				/*
				 * increment to the next inode
				 */
			agg_recptr->this_inonum += 1; 
			}  /* end else the inode is in use */
		    }  /* end for each inode in the extent */
		}  /* end else extent is allocated */
	    }  /* end for each extent in the IAG */ 
	}  /* end for each IAG */

    return( pfsi_rc );
}				/* end process_FilesetInodes() */


/****************************************************************************
 * NAME: 	process_Inode
 *
 * FUNCTION:	Scan the storage allocated to the given inode to see whether
 *		any block on the bad block list is allocated to it.  If so,
 *		handle it.
 *
 * PARAMETERS:	
 *	isBadBlockInode	input	0 => The given inode is NOT the JFS Bad Block
 *					inode for the current file system
 *				1 => The given inode IS the JFS Bad Block 
 *					inode for the current file system 
 *
 *	inoptr			pointer to the inode in the buffer
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 process_Inode( int8 isBadBlockInode, dinode_t *inoptr ) 
{
    int32	sbbi_rc = 0;
    int8	relocate_extent = -1;
    int8	extent_relocated = 0;
	
    if( isBadBlockInode ) {
	relocate_extent = 0;
	}
	
	/*
	 * if there's an EA associated with the inode, process it
	 */
    if( inoptr->di_ea.flag == DXD_EXTENT ) {
	sbbi_rc = process_Inode_EA( inoptr, 
				    relocate_extent, 
				    &extent_relocated 
				    );
	}
	
	/*
	 * process the tree for this inode
	 */
    if( sbbi_rc == 0 ) {  /* no errors so far */

	switch(inoptr->di_mode & IFMT)  {
	    case IFREG:
		sbbi_rc = process_Inode_Xtree( inoptr, relocate_extent );
		break;
	    case IFDIR:
		sbbi_rc = process_Inode_Dtree( inoptr, relocate_extent );
		break;
	    default:
		break;
	    }
	}  /* no errors so far */

    return( sbbi_rc );
}				/* end process_Inode() */


/****************************************************************************
 * NAME: 	process_Inode_Dtree 
 *		
 * FUNCTION:	Traverse the dtree of the given directory inode
 *		looking for allocated extents containing bad blocks.
 *
 * PARAMETERS:	
 *	dip		ptr to the owning disk inode in a buffer
 *
 *	doRelocate	 0 => this is not the JFS Bad Block inode
 *			!0 => this is the JFS Bad Block inode
 *
 * NOTES:	
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 process_Inode_Dtree( dinode_t *dip, int8 doRelocate )
{
    int32	pid_rc = 0;
    int32 relocate_rc = 0;
    dtpage_t	*p; 
    int8	*stbl;
    int32	i;	
    pxd_t	*pxd;
    int64	xaddr, lmxaddr, xlastblk;
    int32	xlen;
    int8	didRelocate = 0;
    cbbl_bdblk_recptr bdblk_recptr = NULL;
    int32	pList;

    ULONG 	pListLen = 0;
    clrbblks_t	pData;
    ULONG 	pDataLen = 0;
    int64		lmpg_addr;
    int32		lmxlen;
    dtpage_t 	next_dtpg;  	/* next dtpage to work on  */
    dtpage_t 	*pnext_dtpg;
    int32 	ndtpg_inx; 	/* the next index in next_dtpg to work on */

	/* 
	 * we start with the root 
	 */
    pnext_dtpg = p = (dtpage_t *)&dip->di_btroot;

	/* 
	 * is it leaf, i.e., inode inline data ? 
	 */
    if (p->header.flag & BT_LEAF)  {
	goto out;
	}
	
    p->header.next = 0;
	/* 
	 * save leftmost dtpage xaddr 
	 */
    lmpg_addr = 0;

    stbl = DT_GETSTBL(p);
    pxd = (pxd_t *)&p->slot[stbl[0]];
	/* 
	 * save leftmost child dtpage extent 
	 */
    lmxaddr = addressPXD(pxd); /* leftmost child xaddr */
    lmxlen = lengthPXD(pxd);
    ndtpg_inx = 0;

	/*
	 * scan each level of dtree
	 */
    while(1)  {
	/*
	 * scan each dtpage of current level of dtree
	 */
	while(1)  {
	    stbl = DT_GETSTBL(p);

		/*
		 * scan each idtentry in current dtpage
		 */
	    for (i = ndtpg_inx; i < p->header.nextindex; i++) {
		pxd = (pxd_t *)&p->slot[stbl[i]];

			/* 
			 * does the extent contain at least 1 
			 * bad block? 
			 */ 
		xaddr = addressPXD(pxd);
		xlen = lengthPXD(pxd);

		pid_rc = baltree_search( xaddr, &bdblk_recptr );
		if( pid_rc != 0 ) {  /* something fatal on search */
		    return( pid_rc );
		    }
		else if( bdblk_recptr == NULL ) {  /* hit end of tree w/o match */
		    continue;
		    }  /* end hit end of tree w/o match */

		xlastblk = xaddr + xlen - 1;
		if( bdblk_recptr->fs_blkno <= xlastblk ) { /*
				* the extent contains at least 1 bad block
				*/
#ifdef _JFS_DEBUG
	printf("bad block 0x0%llx found in dtree for inode %d\n", 
		bdblk_recptr->fs_blkno, dip->di_number );
#endif
		    if( !doRelocate ) { /* relocation not requested */
			pid_rc = we_DidntTryTo_Relocate( xaddr,
							     xlastblk,
							     bdblk_recptr
							    );
			}  /* end relocation not requested */
		    else {  /* relocation is requested */
			pDataLen = sizeof(clrbblks_t);
			pData.flag = CLRBBLKS_RELOCATE | IFDIR | DTPAGE;
			pData.dev = lvMount->LVNumber;
			pData.fileset = dip->di_fileset;
			pData.inostamp = dip->di_inostamp;
			pData.ino = dip->di_number;
			pData.gen = dip->di_gen;
			pData.xoff = lmpg_addr; /* leftmost page 
						 * describing this level 
						 */
			pData.old_xaddr = xaddr;
			pData.new_xaddr = 0;
			pData.xlen = xlen;
			pData.agg_blksize = agg_recptr->fs_blksize;
				/*
				 * attempt to relocate the extent
				 */
			didRelocate = 0;
			relocate_rc = fscntl(	JFSCTL_CLRBBLKS,
						(void *)&pList, &pListLen,
						(void *)&pData, &pDataLen
						);
			if( (relocate_rc == 0) && 
			    (pData.new_xaddr != 0) ) {  /*
				* extent has been relocated 
				*/
			    didRelocate = -1;
			    }  /* end extent has been relocated */	
				/*
				 * Now handle the individual bad block(s) 
				 * in the extent
				 */
			if( didRelocate ) {  /* 
				* actually did relocate 
				*/
			    pid_rc = we_Did_Relocate( xaddr, 
							  xlastblk,
							  bdblk_recptr 
							  );
			    }  /* end actually did relocate */
			else {  /* tried but failed to relocate */
			    pid_rc = we_Couldnt_Relocate( xaddr, 
							      xlastblk,
							      bdblk_recptr, 
							      relocate_rc 
							     );
			    }  /* end else tried but failed to relocate */
			}  /* end else relocation is requested */
		    }  /* end the extent contains at least 1 bad block */
		}  /* end for loop */

		/* 
		 * read in next/right sibling dtpage 
		 */
	    if (p->header.next != 0) {
		xaddr = p->header.next;
		pid_rc = pRead(fsMount, xaddr, fsMount->nbperpage, &next_dtpg);
		if (pid_rc != 0) {
		    return CBBL_CANTREADNEXTDTPG; /* i/o error */
		    }

		pnext_dtpg = p = &next_dtpg;
		ndtpg_inx = 0;
		}
	    else
		break;
	    } /* end while current level scan */

		/*
		 * descend: read leftmost dtpage of next lower level of dtree
		 */
		/* 
		 * the first child of the dtroot split may not have PSIZE 
		 */ 
	pid_rc = pRead(fsMount, lmxaddr, lmxlen, &next_dtpg);
	if ( pid_rc != 0 )  {
	    return CBBL_CANTREADLMDTCHILD; /* i/o error */
	    }

	pnext_dtpg = p = &next_dtpg;

		/* 
		 * for dir, the leaf contains data, its pxd info 
		 * has been reported by the parent page. so we stop here 
		 */
	if (p->header.flag & BT_LEAF) {
	    break;
	    }

		/* 
		 * save leftmost dtpage xaddr 
		 */
	lmpg_addr = lmxaddr;

	stbl = DT_GETSTBL(p);
	pxd = (pxd_t *)&p->slot[stbl[0]];
		/* 
		 * save leftmost child dtpage extent 
		 */
	lmxaddr = addressPXD(pxd); /* leftmost child xaddr */
	lmxlen = lengthPXD(pxd);
	ndtpg_inx = 0;
	}  /* end while scan each level of tree */

	/* reset global state variable for the inode */
out:
	pnext_dtpg = NULL;

	return pid_rc;
}					/* end process_Inode_Dtree() */


/****************************************************************************
 * NAME: 	process_Inode_EA 
 *		
 * FUNCTION:	If the extent containing the Extended Attributes for the
 *		given inode contains any bad blocks, handle it.
 *
 * PARAMETERS:	
 *	dip		ptr to the owning disk inode in a buffer
 *
 *	doRelocate	 0 => this is not the JFS Bad Block inode
 *			!0 => this is the JFS Bad Block inode
 *
 *	didRelocate	ptr to var in which to return:
 *				 0 if the extent was not relocated
 *				!0 if the extent was relocated
 *
 * NOTES:	
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 process_Inode_EA( dinode_t *dip, int8 doRelocate, int8 *didRelocate )
{
    int32 	sea_rc = 0;
    int32 	relocate_rc = 0;
    int64 	ext_firstblk, ext_lastblk;
    int32 	ext_len;
    cbbl_bdblk_recptr bdblk_recptr = NULL;
    cbbl_bdblk_recptr next_bdblk_recptr;
    int32	pList;
    ULONG 	pListLen = 0;
    clrbblks_t	pData;
    ULONG 	pDataLen = 0;

    ext_firstblk = addressDXD( &(dip->di_ea) );
    ext_len = lengthDXD( &(dip->di_ea) );
    ext_lastblk = ext_firstblk + ext_len - 1;

	/*
	 * find the first entry in bad block list with key greater 
	 * than or equal to the first block in this extent 
	 */
    sea_rc = baltree_search( ext_firstblk, &bdblk_recptr );
    if( sea_rc != 0 ) {  /* something fatal on search */
	return( sea_rc );
	}
    else if( bdblk_recptr == NULL ) {  /* hit end of tree w/o match */
	return( sea_rc );
	}  /* end hit end of tree w/o match */

    if( bdblk_recptr->fs_blkno <= ext_lastblk ) { /*
				* the extent contains at least 1 bad block
				*/
#ifdef _JFS_DEBUG
	printf("bad block 0x0%llx found in EA for inode %d\n", 
		bdblk_recptr->fs_blkno, dip->di_number );
#endif
	if( !doRelocate ) { /* relocation not requested */
	    sea_rc = we_DidntTryTo_Relocate(	ext_firstblk, 
						ext_lastblk,
						bdblk_recptr 
						);
	    }  /* end not asked to relocate */
	else {  /* relocation is requested */
	    pDataLen = sizeof(clrbblks_t);
	    pData.flag = CLRBBLKS_RELOCATE | CLRBBLKS_EA;
	    pData.dev = lvMount->LVNumber;
	    pData.fileset = dip->di_fileset;
	    pData.inostamp = dip->di_inostamp;
	    pData.ino = dip->di_number;
	    pData.gen = dip->di_gen;
	    pData.old_xaddr = ext_firstblk;
	    pData.new_xaddr = 0;
	    pData.xlen = ext_len;
	    pData.agg_blksize = agg_recptr->fs_blksize;
		/*
		 * attempt to relocate the extent
		 */
	    *didRelocate = 0;
	    relocate_rc = fscntl(	JFSCTL_CLRBBLKS,
					(void *)&pList, &pListLen,
					(void *)&pData, &pDataLen
					);
	    if( (relocate_rc == 0) && (pData.new_xaddr != 0) ) {  /*
				* extent has been relocated 
				*/
		*didRelocate = -1;
		}  /* end extent has been relocated */	
		/*
		 * Now handle the individual bad block(s) in the EA
		 */
	    if( !(*didRelocate) ) {  /* tried and failed to relocate */
		sea_rc = we_Couldnt_Relocate(	ext_firstblk, 
							ext_lastblk,
							bdblk_recptr, 
							relocate_rc 
							);
		}  /* end tried and failed to relocate */
	    else {  /* actually did relocate */
		sea_rc = we_Did_Relocate(	ext_firstblk, 
						ext_lastblk,
						bdblk_recptr 
						);
		}  /* else end actually did relocate */
	    }  /* end else relocation is requested */
	}  /* end the extent contains at least 1 bad block */

    return( sea_rc );
}				/* end process_Inode_EA()		*/


/****************************************************************************
 * NAME: 	 process_Inode_Xtree
 *		
 * FUNCTION:	Traverse the xtree of the given inode looking for
 *		allocated extents containing bad blocks.
 *
 * PARAMETERS:	
 *	dip		ptr to the owning disk inode in a buffer
 *
 *	doRelocate	 0 => this is not the JFS Bad Block inode
 *			!0 => this is the JFS Bad Block inode
 *
 * NOTES:	
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 process_Inode_Xtree( dinode_t *dip, int8 doRelocate )
{
    int32		pix_rc = 0;
    int32		relocate_rc;
    xtpage_t	*p; 
    int32		xlen, i;
    int64		xaddr, xoff, xlastblk;
    int8		didRelocate = 0;
    int32		xtype;
    cbbl_bdblk_recptr bdblk_recptr = NULL;
    int32		pList;
    ULONG 	pListLen = 0;
    clrbblks_t	pData;
    ULONG 	pDataLen = 0;

    int64	lmxaddr;	/* address of left-most child		*/
    xtpage_t next_xtpg;  	/* next xtpage to work on for the inode */
    xtpage_t *pnext_xtpg;
    int32 nxtpg_inx; 		/* the next index in next_xtpg to work on */

	/* 
	 * we start with the root 
	 */
    pnext_xtpg = p = (xtpage_t *)&dip->di_btroot;
    if (p->header.flag & BT_LEAF)  {
	xtype = DATAEXT;
	}
    else  {
	xtype = XTPAGE;
	p->header.next = 0;
		/* 
		 * save leftmost child xtpage xaddr 
		 */
	lmxaddr = addressXAD(&p->xad[XTENTRYSTART]);
	}

    nxtpg_inx = XTENTRYSTART;

	/*
	 * scan each level of xtree
	 */
    while(1)  {
		/*
		 * scan each xtpage of current level of xtree
		 */
	while(1)  {
		/*
		 * scan each xad in current xtpage
		 */
	    for (i = nxtpg_inx; i < p->header.nextindex; i++)  {
			/* 
			 * does the extent contain at least 1 
			 * bad block? 
			 */ 
		xoff = offsetXAD(&p->xad[i]);
		xaddr = addressXAD(&p->xad[i]) ;
		xlen = lengthXAD(&p->xad[i]) ;

		pix_rc = baltree_search( xaddr, &bdblk_recptr );
		if( pix_rc != 0 ) {  /* something fatal on search */
		    return( pix_rc );
		    }
		else if( bdblk_recptr == NULL ) {  /* hit end of tree w/o match */
		    continue;
		    }  /* end hit end of tree w/o match */

		xlastblk = xaddr + xlen - 1;
		if( bdblk_recptr->fs_blkno <= xlastblk ) { /*
				* the extent contains at least 1 bad block
				*/
#ifdef _JFS_DEBUG
	printf("bad block 0x0%llx found in xtree for inode %d\n", 
		bdblk_recptr->fs_blkno, dip->di_number );
#endif
		    if( !doRelocate ) { /* relocation not requested */
			pix_rc = we_DidntTryTo_Relocate( xaddr,
							     xlastblk,
							     bdblk_recptr
							    );
			}  /* end relocation not requested */
		    else {  /* relocation is requested */
			pDataLen = sizeof(clrbblks_t);
			pData.flag = CLRBBLKS_RELOCATE | IFREG | xtype;
			pData.dev = lvMount->LVNumber;
			pData.fileset = dip->di_fileset;
			pData.inostamp = dip->di_inostamp;
			pData.ino = dip->di_number;
			pData.gen = dip->di_gen;
			pData.xoff = xoff;	/* offset within the file */
			pData.old_xaddr = xaddr;
			pData.new_xaddr = 0;
			pData.xlen = xlen;
			pData.agg_blksize = agg_recptr->fs_blksize;
				/*
				 * attempt to relocate the extent
				 */
			didRelocate = 0;
			relocate_rc = fscntl(	JFSCTL_CLRBBLKS,
						(void *)&pList, &pListLen,
						(void *)&pData, &pDataLen
						);
			if( (relocate_rc == 0) && 
			    (pData.new_xaddr != 0) ) {  /*
				* extent has been relocated 
				*/
			    didRelocate = -1;
			    }  /* end extent has been relocated */	
				/*
				 * Now handle the individual bad block(s) 
				 * in the extent
				 */
			if( didRelocate ) {  /* 
				* actually did relocate 
				*/
			    pix_rc = we_Did_Relocate( xaddr, 
							  xlastblk,
							  bdblk_recptr 
							  );
			    }  /* end actually did relocate */
			else {  /* tried but failed to relocate */
			    pix_rc = we_Couldnt_Relocate( xaddr, 
							      xlastblk,
							      bdblk_recptr, 
							      relocate_rc 
							     );
			    }  /* end else tried but failed to relocate */
			}  /* end else relocation is requested */
		    }  /* end the extent contains at least 1 bad block */
		}  /* end for current xtpage scan */

		/*
		 * read in next/right sibling xtpage 
		 */
	    if (p->header.next != 0) {
		xaddr = p->header.next;
		pix_rc = pRead(fsMount, xaddr, fsMount->nbperpage, &next_xtpg);
		if ( pix_rc != 0 )  {
		    return CBBL_CANTREADNEXTXTPG; /* i/o error */
		    }

		pnext_xtpg = p = &next_xtpg;
		nxtpg_inx = XTENTRYSTART;
		}
	    else
		break;
	    } /* end while current level scan */

		/*
		 * descend: 	read leftmost xtpage of next 
		 * 		lower level of xtree
		 */
	if (xtype == XTPAGE)  {
		/* 
		 * get the leftmost child page  
		 */
	    pix_rc = pRead(fsMount, lmxaddr, fsMount->nbperpage, &next_xtpg);
	    if ( pix_rc != 0 )  {
		return CBBL_CANTREADLMXTCHILD; /* i/o error */
		}

	    pnext_xtpg = p = &next_xtpg;
	    nxtpg_inx = XTENTRYSTART;
	    if (p->header.flag & BT_LEAF)  {
		xtype = DATAEXT;
		}
	    else  {
		xtype = XTPAGE;
			/* 
			 * save leftmost child xtpage xaddr 
			 */
		lmxaddr = addressXAD(&p->xad[XTENTRYSTART]);
		}
	    }   /* end xtype == XTPAGE */
		else
			break;
	} /* end while level scan */

	/* 
	 * this inode is done: reset variables 
	 */
    pnext_xtpg = NULL;

    return 0;
}					/* end process_Inode_Xtree() */


/****************************************************************************
 * NAME: 	we_Couldnt_Relocate 
 *		
 * FUNCTION:	For each entry (on the sequential list of bad blocks)
 *		which describes a block in the specified extent, remove 
 *		the entry from the sequential list and add it to the 
 *		retry list.  Increment counters as appropriate.
 *
 * PARAMETERS:	
 *	ext_firstblk	the first block in the extent which the caller
 *			failed to relocate
 *
 *	ext_lastblk	the last block in the extent which the caller 
 *			failed to relocate
 *
 *	bdblk_recptr	ptr to the first record in the sequential list 
 *			which describes a block in the extent which the
 *			caller failed to relocate
 *
 *	reloc_rc	reason the attempt to relocate failed
 *
 * NOTES:	This routine is called when an attempt to relocate an
 *		extent (containing bad block(s)) fails, either because
 *		of insufficient contiguous space to accomodate the 
 *		extent or because the information was stale due to 
 *		concurrent file system activity.
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 we_Couldnt_Relocate( int64		ext_firstblk,
			   int64			ext_lastblk,
			   cbbl_bdblk_recptr	bdblk_recptr,
			   int32			reloc_rc
			   )
{
    int32 fcr_rc = 0;
    int64 idx, block_limit;
    int8  done;
    cbbl_bdblk_recptr thisrec, nextrec;
	
    done = 0;
    block_limit = ext_lastblk + 1;
    thisrec = bdblk_recptr;
    for( idx = ext_firstblk; 
	 ((idx < block_limit) && (fcr_rc == 0) && (!done)); 
	 idx++ ) {	/* process each fs block in the extent */
	
	if( thisrec == NULL ) {  /* 
				* no more in sequential list 
				*/
	    done = -1;
	    }  /* end no more in sequential list */
	
	else if( thisrec->fs_blkno > block_limit ) {  /* 
				* no more in this extent 
				*/
	    done = -1;
	    }  /* end no more in this extent */
	
	else {  /* there may be more bad blocks in this extent */
		/*
		 * locate next on sequential list of bad blocks
		 */
	    nextrec = thisrec->next;  
	    if( thisrec->fs_blkno == idx ) {  /* this is a bad block */
			/*
			 * take the record off the sequential list
			 * add the record to the retry list
			 * increment retry count and retry reason
			 */
		seqlist_remove( thisrec );
		retrylist_insert( thisrec );
		thisrec->retry_list_count += 1;
		if( reloc_rc == ENOSPC ) {
		    thisrec->insuffspace_count += 1;
		    }
		else {
		    thisrec->staledata_count += 1;
		    }
			/*
			 * move on to next in sequential list 
			 */
		thisrec = nextrec;
		}  /* end this is a bad block */
	    }  /* end else there may be more bad blocks in this extent */
	}  /* end for idx = ... */
	
    return( fcr_rc );
}				/* end we_Couldnt_Relocate()	*/


/****************************************************************************
 * NAME: 	we_Did_Relocate 
 *		
 * FUNCTION:	For each entry (on the sequential list of bad blocks)
 *		which describes a block in the specified extent, attempt
 *		to allocate the block to the JFS Bad Block inode. 
 *
 *		If able to allocate the block to the Bad Block inode,
 *		remove the entry from the sequential list and tell the 
 *		LVM to remove applicable entries from its bad block list
 *
 *		If unable to allocate the block to the Bad Block inode,
 *		remove the entry from the sequential list and add it to 
 *		the retry list.  Increment counters as appropriate.
 *
 * PARAMETERS:	
 *	ext_firstblk	the first block in the extent which the caller
 *			has relocated
 *
 *	ext_lastblk	the last block in the extent which the caller 
 *			has relocated
 *
 *	bdblk_recptr	ptr to the first record in the sequential list 
 *			which describes a block in the extent which the
 *			caller has relocated
 *
 * NOTES:	
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 we_Did_Relocate( int64		ext_firstblk,
		       int64		ext_lastblk,
		       cbbl_bdblk_recptr	bdblk_recptr
		       )
{
    int32 fdr_rc = 0;
    int64 idx, block_limit;
    int8  done;
    cbbl_bdblk_recptr thisrec, nextrec;

	/* 
	 * Increment # of relocated extents
	 */
    agg_recptr->reloc_extents += 1;
    agg_recptr->reloc_blocks += ext_lastblk - ext_firstblk + 1;

    done = 0;
    block_limit = ext_lastblk + 1;
    thisrec = bdblk_recptr;
    for( idx = ext_firstblk; 
	 ((idx < block_limit) && (fdr_rc == 0) && (!done)); 
	 idx++ ) {	/* process each fs block in the extent */
	
	if( thisrec == NULL ) {  /* 
				* no more in sequential list 
				*/
	    done = -1;
	    }  /* end no more in sequential list */
	
	else if( thisrec->fs_blkno > block_limit ) {  /* 
				* no more in this extent 
				*/
	    done = -1;
	    }  /* end no more in this extent */
	
	else {	/* there may be more bad blocks in this extent */
		/*
		 * locate next on sequential list of bad blocks
		 */
	    nextrec = thisrec->next;  
	    if( thisrec->fs_blkno == idx ) {  /* this is a bad block */
			/*
			 * increment # times data relocated from this block
			 */
		thisrec->relocated_count += 1;
			/*
			 * attempt to allocate the block to the 
			 * JFS Bad Block inode
			 */
		fdr_rc = alloc_to_BBInode( thisrec );
		if( fdr_rc != 0 ) {  	/* 
					 * couldn't allocate the block 
					 */
		    fdr_rc = seqlist_remove( thisrec );
		    fdr_rc = retrylist_insert( thisrec );
		    thisrec->retry_list_count += 1;
		    thisrec->tooslow_count += 1;
		    }  /* end couldn't allocate the block */
		else {	/* 
			 * the block is allocated to the bad block inode 
			 */
		    fdr_rc = tell_LVM( thisrec );
			/*
			 * take the record off the sequential list and
			 * put it into the resolved blocks list.
			 *
			 * N.B. this record is STILL in the tree
			 */
		    fdr_rc = seqlist_remove( thisrec );  
		    fdr_rc = rslvdlist_insert( thisrec );
		    }  /* end else the block is allocated to the bad ... */
			/*
			 * move on to next in sequential list 
			 */
		thisrec = nextrec;
		}  /* end this is a bad block */
	    }  /* end else there may be more bad blocks in this extent */
	}  /* end for idx = ... */
	
    return( fdr_rc );
}				/* end we_Did_Relocate()	*/


/****************************************************************************
 * NAME: 	we_DidntTryTo_Relocate 
 *		
 * FUNCTION:	For each entry (on the sequential list of bad blocks)
 *		which describes a block in the specified extent, remove 
 *		the entry from the sequential list and tell the LVM to 
 *		remove applicable entries from its bad block list
 *
 * PARAMETERS:	
 *	ext_firstblk	the first block in the extent 
 *
 *	ext_lastblk	the last block in the extent 
 *
 *	bdblk_recptr	ptr to the first record in the sequential list 
 *			which describes a block in the extent 
 *
 * NOTES:	Since we weren't asked to relocate this extent, it
 *		must be allocated to the Bad Block Inode already.  
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 we_DidntTryTo_Relocate( int64		ext_firstblk,
			      int64		ext_lastblk,
			      cbbl_bdblk_recptr	bdblk_recptr
			      )
{
    int32 fnr_rc = 0;
    int64 idx, block_limit;
    int8  done;
    cbbl_bdblk_recptr thisrec, nextrec;
	
    done = 0;
    block_limit = ext_lastblk + 1;
    thisrec = bdblk_recptr;
    for( idx = ext_firstblk; 
	 ((idx < block_limit) && (fnr_rc == 0) && (!done)); 
	 idx++ ) {	/* process each fs block in the extent */
	
	if( thisrec == NULL ) {  /* 
				* no more in sequential list 
				*/
	    done = -1;
	    }  /* end no more in sequential list */
	
	else if( thisrec->fs_blkno > block_limit ) {  /* 
				* no more in this extent 
				*/
	    done = -1;
	    }  /* end no more in this extent */
	
	else {  /* there may be more bad blocks in this extent */
		/*
		 * locate next on sequential list of bad blocks
		 */
	    nextrec = thisrec->next;  
	    if( thisrec->fs_blkno == idx ) {  /* this is a bad block */
	
		fnr_rc = tell_LVM( thisrec );
			/*
			 * take the record off the sequential list and
			 * put it into the resolved blocks list.
			 *
			 * N.B. this record is STILL in the tree
			 */
		fnr_rc = seqlist_remove( thisrec );  
		fnr_rc = rslvdlist_insert( thisrec );
			/*
			 * move on to next in sequential list 
			 */
		thisrec = nextrec;
		}  /* end this is a bad block */
	    }  /* end else there may be more bad blocks in this extent */
	}  /* end for idx = ... */

    return( fnr_rc );
}				/* end we_DidntTryTo_Relocate()	*/

