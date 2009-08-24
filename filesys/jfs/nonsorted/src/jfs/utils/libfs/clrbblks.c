/* $Id: clrbblks.c,v 1.2 2004/07/24 00:49:05 pasha Exp $ */

static char *SCCSID = "@(#)1.3  3/31/99 16:29:26 src/jfs/utils/clrbblks/clrbblks.c, jfscbbl, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		clrbblks.c
 *
 *   COMPONENT_NAME: 	jfscbbl
 *
 *   FUNCTIONS:		clrbblks
 *		
 *			alloc_to_BBInode
 *			tell_LVM
 *		 
 *			get_LVM_BadBlockLists
 *			OpenFilSys
 *			parseParms 
 *			preamble
 *			scan_for_free_blocks
 *
 */

#include "clrbblks.h"

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *	device and file system under clrbblks
  */
char	LVName[4];	// device name
uint8  LVNumber;     //PS24072004
 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *	inode allocation map
  */
IMap_t		IMap;		// inode allocation map control page
IMap_t		*iMap = &IMap;

/* 
 * bitmap page control 
 */
iag_t	iagbuf;  

/* 
 * inode extent buffer 
 */
int32	IXNumber = 0;
dinode_t ixbuf[INOSPEREXT];

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *	communication area 
  */
char	cbblfsck_commun_area[PAGESIZE];	
int64    cbblfsck_area_byteoff = 0;
char    *cbblfsck_area_ptr = &(cbblfsck_commun_area[0]);
struct fsckcbbl_record *cbblfsck_recptr;
int	cbblfsck_area_size = PAGESIZE;
int       cbblfsck_commun_active = 0;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * clrbblks anchoring block and pointer
  *
  */
struct cbbl_agg_record  agg_record;
struct cbbl_agg_record *agg_recptr;

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */
int32 get_LVM_BadBlockLists_cb( void );
int32 OpenFilSys( void );
int32 parseParms( int32, char ** );
int32 preamble( HFILE );
int32 scan_for_free_blocks( void );

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */
#ifdef Drova
/******************************************************************
 * NAME: 	main	
 *
 */
main( int32  argc,  char  **argv )
{
    int32 main_rc = 0;

    main_rc = clrbblks( argc, argv );

    return( main_rc );
}
#endif
/******************************************************************
 * NAME: 	clrbblks
 *
 * FUNCTION: 	Main routine of clrbblks utility which has the goal of 
 *		isolating bad blocks on the device and so that the LVM
 *		can drop them from its Bad Block List.
 *
 * PARAMETERS:
 *		input: the file system on which to operate
 *
 *
 * INVOCATION: 
 * 		clrbblks  {device_name}
 *
 *  		where: 
 *		    device_name - device under clear bad blocks list.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 clrbblks( int32  argc,  char  **argv, HFILE DevIO, char *VolLabel )     //PS24072004
{
    int32		rc;
    int		finished = 0;
    int32		pList;
    ULONG 	pListLen = 0;
    clrbblks_t	pData;
    ULONG	pDataLen = 0;

	/*
	 * first things first
	 */
    agg_recptr = &(agg_record);
    init_agg_record_cb();		/* need this before calling preamble() */
	
	/* 
	 * for what logical volume? 
	 */
//PS24072004  Begin
    if( !DevIO )
      {
      rc = parseParms(argc, argv);
      if( rc ) goto out;
      }
    else
      {
      LVName[0] = VolLabel[0];
      if( LVName[0] >= 'a' && LVName[0] <= 'z')
         LVNumber = LVName[0] - 'a';
      else
        if( LVName[0] >= 'A' && LVName[0] <= 'Z')
           LVNumber = LVName[0] - 'A';
        else
           return -1;
      }
//PS24072004  End
mainidea:
	/* 
	 * init clrbblks 
	 */
    rc = preamble(DevIO);
    if( rc ) 	goto out;
    
    if(!DevIO) LVNumber=lvMount->LVNumber;	//PS24072004

    cbblfsck_recptr = (struct fsckcbbl_record *) cbblfsck_area_ptr;
    init_cbblfsck_record();
	
	/*
	 * get the LVM bad block lists for this logical volume
	 * 
	 */
    rc = get_LVM_BadBlockLists_cb(); 
	
    if( agg_recptr->bdblk_baltree.right == NULL ) { 
	finished = -1;
	}
	
	/*
	 * Process the current list 
	 */
    while( (!finished) && (rc == 0) ) {
		/*
		 * create a sequential list of the nodes in the
		 * balanced tree.
		 */
	rc = seqlist_create();
	if( rc == 0 ) {  /* we have a tree and a sequential list */
		/*
		 * sync fs meta-data 
		 */
	    pDataLen = sizeof(clrbblks_t);
	    pData.flag = CLRBBLKS_SYNC;
//PS24072004	    pData.dev = lvMount->LVNumber;
	    pData.dev = LVNumber;    

	    rc = fscntl(JFSCTL_CLRBBLKS,
			(void *)&pList, &pListLen,
			(void *)&pData, &pDataLen
			);
	    if( rc != 0 )  {
#ifdef _JFS_DEBUG
	printf("main: sync failure: rc=%d\n", rc);
#endif
		rc = CBBL_FSCNTL_1 - rc;
		}
	    }  /* end else we have a tree and a sequential list */

	if( (!finished) && (rc == 0) ) {  /* we have synched the file system */
		/*
		 * scan the JFS Bad Block inode to see if anything on
		 * the current list is already allocated to it.  
	 	 */
#ifdef _JFS_DEBUG
	printf("Are any already on the Bad Block Inode?\n");
#endif
	    rc = process_BadBlockInode();
	    }  /* end we have synched the file system */
	
	if( (agg_recptr->bdblk_baltree.seq_list != NULL) && (rc == 0) ) { /*
				* there are still bad blocks to process 
				*/
		/*
		 * scan the list for bad blocks which are currently
		 * not allocated to any inode.
		 */
#ifdef _JFS_DEBUG
	printf("Are any are currently free?\n");
#endif
	    rc = scan_for_free_blocks(); 
	    }  /* end there are still bad blocks to process */
	
	if( (agg_recptr->bdblk_baltree.seq_list != NULL) && (rc == 0) ) { /* 
				* there are still bad blocks to process 
				*/
		/*
		 * scan the JFS fileset inodes for owners of data
		 * residing in bad blocks.  If any are found, try to
		 * relocate the data and allocate the bad blocks to 
		 * the JFS Bad Block inode.
	 	 */
#ifdef _JFS_DEBUG
	printf("Are any owned by fileset inodes?\n");
#endif
	    rc = process_FilesetInodes();
	    }  /* end there are still bad blocks to process */
	
	if( rc == 0 ) { /* clear resolved list and anything still on seq list */
		/*
		 * Any blocks still on the sequential list must 
		 * contain data which we can't relocate.  Drop them
		 * from the list and forget them.
	 	 */
	    rc = baltree_purge( );
	    }  /* end clear resolved list and anything still on seq list */
	
		/*
		 * do we need to go around again?
		 */
	if( rc == 0 ) {  /* still in business */
		/*
		 * if the file system has been extended, get the LVM bad
		 * block list for each new device partition added, and
		 * add entries to the balanced tree as appropriate.
		 */
	    rc = get_LVM_BadBlockLists_cb(); 
	    if( rc == 0 ) {  /* nothing fatal */
		    /*
		     * if any blocks made it to the retry list, put them
		     * back into the balanced tree.
		     */
		rc = retrylist_purge();
		}  /* end else nothing fatal */
	    }  /* end still in business */
    	
	if( agg_recptr->bdblk_baltree.right == NULL ) { 
	    finished = -1;
	    }
	}  /* end while !finished */

	/* 
	 * finish the communication area then close FS 
	 */
    cbblfsck_recptr->cbbl_retcode = rc;
    complete_cbblfsck_record();
    closeFS();

	/* 
	 * release buffers
	 */
    workspace_release_cb();

out:
    return rc;
}					/* end clrbblks() */


/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */
	
	
/******************************************************************
 * NAME: 	alloc_to_BBInode
 *
 * FUNCTION: 	Call jfs_clrbblks() to allocate the file system block 
 *		(described by the given bad block record) to the JFS
 *		Bad Block inode.
 *
 * PARAMETERS:	
 *	bdblk_recptr	ptr to the bad block record describing the file
 *			system block to allocate to the Bad Block inode.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 alloc_to_BBInode( cbbl_bdblk_recptr bdblk_recptr ) 
{
    int32 atb_rc = 0;
    int32	pList;
    ULONG 	pListLen = 0;
    clrbblks_t	pData;
    ULONG 	pDataLen = 0;

    pDataLen = sizeof(clrbblks_t);
    pData.flag = CLRBBLKS_BADBLKALLOC;
//PS24072004    pData.dev = lvMount->LVNumber;
    pData.dev = LVNumber;
    pData.old_xaddr = bdblk_recptr->fs_blkno;
    pData.xlen = 1;
    pData.agg_blksize = agg_recptr->fs_blksize;
	
    atb_rc = fscntl(	JFSCTL_CLRBBLKS,
		    	(void *)&pList, &pListLen,
		    	(void *)&pData, &pDataLen
			);
	
    if( atb_rc == 0 ) {
	bdblk_recptr->allocated_to_BBino = -1;
	}
	
    return( atb_rc );
}				/* end alloc_to_BBInode() */
	
/******************************************************************
 * NAME: 	get_LVM_BadBlockLists
 *
 * FUNCTION: 	Query the LVM for the (set of tables composing) its bad 
 *		block list for the file system.  Call a routine to populate
 *		the balanced tree to represent the LVMs bad block list.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 get_LVM_BadBlockLists_cb( void ) 
{
    int32 glbbl_rc = 0;
    int32 new_LVM_lists = 0;
    int32 bbl_idx;
    int32 lsn_idx;
    DDI_OS2LVM_param LVMpp;
    DDI_OS2LVM_param *pLVMpp = &LVMpp;
    DDI_OS2LVM_data LVMdp;
    DDI_OS2LVM_data *pLVMdp = &LVMdp;
    ULONG ppLen = 0;
    ULONG dpLen = 0;
    int32 		min_bufsize;
    int32 		bufsize = 4096;
    char 		*bufptr = NULL;
    BadBlockInfo	*BB_bufptr = NULL;
    BadBlockTableInfo 	*BBT_bufptr = NULL;
    int32 		tablesize;
    RelocationTable	*RTable = NULL;
	
	/* 
	 * initialize the LVM DosDevIOCtl parm and data packets
	 */
    pLVMpp->Command = 0;
    pLVMpp->DriveUnit = 0;
    pLVMpp->TableNumber = 0;
    pLVMpp->LSN = 0;
    pLVMdp->ReturnData = 0;
    pLVMdp->UserBuffer = NULL; 
	
	/*
	 * allocate a buffer
	 */
    bufptr = malloc( bufsize );
    if( bufptr == NULL ) {
	return( CBBL_CANTALLOCBUFFR1 );
	}
	
	/* 
	 * ask the LVM how many bad block lists it has for the filesystem
	 */
    pLVMpp->Command = GET_BBR_INFO;
    pLVMdp->UserBuffer = bufptr; 
    glbbl_rc = DosDevIOCtl( LVHandle,		IOC_DC,	
			    IODC_LV,		(void *) pLVMpp,	
			    sizeof(DDI_OS2LVM_param),
			    &ppLen,		(void *) pLVMdp,	
			    sizeof(DDI_OS2LVM_data),		&dpLen
			    );	
    if( glbbl_rc != 0 ) {  
	return( CBBL_LVMRC_1 - glbbl_rc );
	}
    else {  /* DosDevIOCtl successful */
	if( pLVMdp->ReturnData != BBR_SUCCESS ) {	/* cat 8, funct 69 failed */
	    return( CBBL_LVMRC_2 - pLVMdp->ReturnData );
	    }
	else {  
	    BB_bufptr = (BadBlockInfo *) bufptr;
	    new_LVM_lists = BB_bufptr->TotalTables;
	    }
	}  /* end else DosDevIOCtl successful */
				
		/*
		 * process the LVM Bad Block Lists 
		 *
		 * N.B. agg_recptr->LVM_lists are the number of tables
		 *	already seen during this session of clrbblks.
		 * 	new_LVM_lists is the number of tables now.  Might
		 * 	be larger, can never be smaller.
		 */
    for( bbl_idx = agg_recptr->LVM_lists;
	 ( (bbl_idx < new_LVM_lists) && (glbbl_rc == 0) );
	 bbl_idx++ ) {
		/*
		 * ask the LVM how big this list is.  
		 * allocate a new buffer if the current one isn't big enough.
		 */
	pLVMpp->Command = GET_TBL_SIZE;
	pLVMpp->TableNumber = bbl_idx;
	pLVMdp->ReturnData = 0;
	pLVMdp->UserBuffer = bufptr; 
	glbbl_rc = 
		DosDevIOCtl( LVHandle,	IOC_DC,	
			     IODC_LV,		(void *) pLVMpp,	
			     sizeof(DDI_OS2LVM_param),
			     &ppLen,		(void *) pLVMdp,	
			     sizeof(DDI_OS2LVM_data),		&dpLen	
			    );	
	if( glbbl_rc != 0 ) {  
	    return( CBBL_LVMRC_3 - glbbl_rc );
	    }
	else {  /* DosDevIOCtl successful */
	    if( pLVMdp->ReturnData != BBR_SUCCESS ) { /* cat 8, funct 69 failed */
		return( CBBL_LVMRC_4 - pLVMdp->ReturnData );
		}
	    else {  
		BBT_bufptr = (BadBlockTableInfo *) bufptr;
		tablesize = BBT_bufptr->ActiveRelocations;
		agg_recptr->total_bad_blocks += tablesize;
		}
	    }  /* end else DosDevIOCtl successful */

		/*
		 * ask the LVM for the list
		 */
	if( (glbbl_rc == 0) && (tablesize != 0) ) {  /* 
				* successful so far and the 
				* current table isn't empty
				*/
	    min_bufsize = tablesize * sizeof( LSNumber );
	    if( min_bufsize > bufsize ) {  /* more buffer is needed */
		free( (void *) bufptr ); 
		bufsize = ((min_bufsize + 1023) / 1024) * 1024;
		bufptr = malloc( bufsize );
		if( bufptr == NULL ) {
		    return( CBBL_CANTALLOCBUFFR2 );
		    }
		}  /* end more buffer is needed */

	    pLVMpp->Command = GET_SECT_LIST;
	    pLVMpp->TableNumber = bbl_idx;
	    pLVMdp->ReturnData = 0;
	    pLVMdp->UserBuffer = bufptr; 
	    glbbl_rc = 
		   DosDevIOCtl( LVHandle,	IOC_DC,	 
				IODC_LV,	(void *) pLVMpp, 
				sizeof(DDI_OS2LVM_param),
				&ppLen,	(void *) pLVMdp, 
				sizeof(DDI_OS2LVM_data),	&dpLen	
				);
	    }  /* end successful so far */	 
	if( glbbl_rc != 0 ) {  
	    return( CBBL_LVMRC_5 - glbbl_rc );
	    }
	else  {  /* DosDevIOCtl successful */
	    if( pLVMdp->ReturnData != BBR_SUCCESS ) { 
		return( CBBL_LVMRC_6 - pLVMdp->ReturnData );
		}
	    else { 
		RTable = (RelocationTable *) bufptr;
		}
	    }  /* end DosDevIOCtl successful */
	
	if( glbbl_rc == 0 ) {  /* we got a list */
		/*
		 * load the list elements into the balanced tree
		 */
	    for( lsn_idx = 0; 
		 ((lsn_idx < tablesize) && (glbbl_rc == 0));
		 lsn_idx++ ) {
		glbbl_rc = baltree_load( RTable[lsn_idx], bbl_idx ); 
		}  /* end for lsn_idx */
	    }  /* end we got a list */
	}  /* end for bbl_idx */
	
    free( (void *) bufptr ); 
	
    if( glbbl_rc == 0 ) {
	agg_recptr->LVM_lists = new_LVM_lists;
	}

    return( glbbl_rc );
}				/* end get_LVM_BadBlockLists() */
	
/******************************************************************
 * NAME: 	OpenFilSys
 *
 * FUNCTION: 	Read the superblock into a buffer.  
 *		Extract various pieces of information.
 *		Release the buffer.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 OpenFilSys( void )
{
	int32			rc;
	struct superblock	*sb;
	buf_t			*bp;

	/*
	 *	validate and retrieve fs parameters from superblock
	 */
	/* try to read the primary superblock */
	rc = bRawRead(LVHandle, (int64)SUPER1_OFF, (int32)PAGESIZE, &bp);
	if (rc != 0)  {
		/* try to read the secondary superblock */
		rc = bRawRead(LVHandle, (int64)SUPER2_OFF, (int32)PAGESIZE, &bp);
		if (rc != 0)  {
#ifdef _JFS_DEBUG
	printf("OpenFilSys: i/o error: rc=%d\n", rc);
#endif
			return CBBL_CANTREADSBLKS;
			}
		}

	sb = (struct superblock *)bp->b_data;

	/* check magic/version number */
	if (strncmp(sb->s_magic,JFS_MAGIC,(unsigned)strlen(JFS_MAGIC))
	 || (sb->s_version != JFS_VERSION))  {
		return CBBL_INVALMAGORVERS;
		}

	if (sb->s_state & FM_DIRTY)  {
		return CBBL_FSDIRTY;
		}

	fsMount->bsize = sb->s_bsize;
	fsMount->l2bsize = sb->s_l2bsize;
	fsMount->l2bfactor = sb->s_l2bfactor;
	fsMount->nbperpage = PAGESIZE >> fsMount->l2bsize;
	fsMount->l2nbperpage = log2shift(fsMount->nbperpage);

	fsMount->FSSize = sb->s_size >> sb->s_l2bfactor;
	
	fsMount->AGSize = sb->s_agsize;
#ifdef _JFS_DEBUG
	printf("superblock: attribute:0x%08x state:0x%08x\n",
		 sb->s_flag, sb->s_state);
	printf("superblock: bsize:%d FSSize:%lld\n",
		fsMount->bsize, fsMount->FSSize);
#endif
	agg_recptr->fs_blksize = sb->s_bsize;	/* aggregate block size */
	agg_recptr->lv_blksize = sb->s_pbsize;	/* device block size	*/
	agg_recptr->fs_lv_ratio = sb->s_bsize / sb->s_pbsize;
	agg_recptr->fs_last_metablk = addressPXD( &sb->s_aim2 ) +
					lengthPXD( &sb->s_aim2);  
	agg_recptr->fs_first_wspblk = addressPXD( &sb->s_fsckpxd );

	bRelease(bp);

	return rc;
}						/* end OpenFilSys() */


/******************************************************************
 * NAME: 	parseParms
 *
 * FUNCTION: 	Extract the file system specification from the parms
 *		specified by the caller.  Verify that no invalid parms
 *		were included in the call.
 *
 * PARAMETERS:	the clrbblks invocation parms
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 parseParms( int32  argc,  char  **argv )
{
    int32		rc = 0;
    int32		i;
    char		*argp;
    int		devFound = FALSE;

    if (argc < 2)
	return CBBL_INTERNAL_ERROR_1;

	/* 
	 * parse each of the command line parameters 
	 */
    for (i = 1; i < argc && rc == 0; i++)  {
	argp = argv[i];
		/* 
		 * a switch ? 
		 */
	if (*argp == '/' || *argp == '-')  {
	    argp++;  /* increment past / or - */
	    while (rc == 0 && *argp)  {
		switch (toupper(*argp)) {
		    default:    /* unknown switch */
			rc = CBBL_INTERNAL_ERROR_2;
			goto out;
		    }  /* end switch */
		}  /* end while */
	    if (*argp) {
		rc = CBBL_INTERNAL_ERROR_3;
		goto out;
		}
	    continue;
	    }  /* endif */
		/* 
		 * a drive letter 
		 */
	if (isalpha(*argp) && argp[1] == ':' && !argp[2]) {  
		/* only allow one drive letter to be specified */
	    if (devFound)  {
		rc = CBBL_INTERNAL_ERROR_4;
		break;
		}
	    else  {
		strcpy(LVName, argp);
		devFound = TRUE;
		}
	    continue;
	    }  /* endif */
		/*
		 * It's not a switch and it's not a drive letter
		 */
	rc = CBBL_INTERNAL_ERROR_5;
	break;
	} /* end for */

    if( (rc==0) && (!devFound) )
	rc = CBBL_INTERNAL_ERROR_6;
	
out:
	
    return (rc);
}								/* end parseParms() */
	
	
/******************************************************************
 * NAME: 	preamble
 *
 * FUNCTION: 	Open the logical volume containing the file system.
 *		Read aggregate-level control structures for the file
 *		system inodes.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 preamble( HFILE DevIo )
{
    int32	rc = 0;
    buf_t	*bp;
    dinode_t	*dip;
    int32	i;
	
	/* 
	 * validate and open LV 
	 */
    if(!DevIo)               //PS24072004
      {
      rc = openLV(LVName);
      if ( rc != 0 ) 
          return CBBL_CANTOPENLV;
      }
    else
       LVHandle=DevIo;       //PS24072004
	/* 
	 * Open FS 
	 */
    rc = OpenFilSys();
    if ( rc != 0 ) 
	return rc;	/* OpenFilSys assigns a unique CBBL return code */

	/*
	 * read in the fileset inode allocation map inode (i_number = 16)
	 */
    i = FILESYSTEM_I / INOSPERPAGE;
    rc = bRawRead(LVHandle, (int64)(AITBL_OFF + PAGESIZE * i), PAGESIZE, &bp);
    if ( rc != 0 ) {
	return CBBL_CANTREADIMAPINO;
	}
	
        /* 
	 * locate the inode in the buffer page 
	 */
    dip = (dinode_t *)bp->b_data;
    dip += FILESYSTEM_I & (INOSPERPAGE - 1);
    memcpy(diIMap, dip, DISIZE);
	
    bRelease(bp);
	
	/*
	 * read imap global control page
	 */
    rc = readIMapGCPSequential(iMap, &iagbuf);
    if ( rc != 0 ) 
	return CBBL_CANTREADIMAPCTL;
	
    return rc;
}
	
	
/******************************************************************
 * NAME: 	scan_for_free_blocks
 *
 * FUNCTION: 	For each block on the sequential list of bad blocks,
 *		    Attempt to allocate the block to the JFS Bad Block inode.
 *
 *		    If this succeeds, the block was not already allocated to 
 *			and so no relocate was/is necessary.  Remove the
 *			block from the sequential list.  Notify the LVM
 *			that the bad LV blocks in this file system block
 *			can be removed from its list.
 *
 * 		    Otherwise, the block is allocated to some inode.  Leave
 *			it on the sequential list for relocation in a later
 *			step.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 scan_for_free_blocks() 
{
    int32 sffb_rc = 0;
    int32 atb_rc = 0;
    cbbl_bdblk_recptr bdblk_recptr, next_bdblk_recptr;

    bdblk_recptr = agg_recptr->bdblk_baltree.seq_list;
    while( bdblk_recptr != NULL ) {
	next_bdblk_recptr = bdblk_recptr->next;
	atb_rc = alloc_to_BBInode( bdblk_recptr );
	if( atb_rc == 0 ) {   /* the block is safely locked to the 
				* bad block inode 
				*/
	    sffb_rc = tell_LVM( bdblk_recptr );
	    if( sffb_rc == 0 ) {
		sffb_rc = seqlist_remove( bdblk_recptr );
		if( sffb_rc == 0 ) { 
		    sffb_rc = freelist_insert( bdblk_recptr );
		    }
		}
	    }  /* end the block is safely locked to the bad block inode */
	bdblk_recptr = next_bdblk_recptr;
	}
	
    return( sffb_rc );
}				/* end scan_for_free_blocks() */
	
	
/******************************************************************
 * NAME: 	tell_LVM
 *
 * FUNCTION: 	Notify the LVM that it can remove, from its bad block list,
 *		the entries describing the bad LV blocks contained in the
 *		given file system block.
 *
 * PARAMETERS:	
 *	bdblk_recptr	ptr to the bad block record describing the file
 *			system block which is now allocated to the Bad 
 *			Block inode.
 *
 * NOTES:	Each bad block record describes one file system block
 *		which contains one or more bad LV blocks.
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 tell_LVM( cbbl_bdblk_recptr thisrec ) 
{
    int32 tl_rc = 0;
    int32 lvidx;
    int32 bufsize = 0;
    DDI_OS2LVM_param LVMpp;
    DDI_OS2LVM_param *pLVMpp = &LVMpp;
    DDI_OS2LVM_data LVMdp;
    DDI_OS2LVM_data *pLVMdp = &LVMdp;
    ULONG ppLen = 0;
    ULONG dpLen = 0;

	/* 
	 * initialize the LVM DosDevIOCtl parm and data packets
	 */
    pLVMpp->Command = REMOVE_TBL_ENTRY;
    pLVMpp->DriveUnit = 0;
    pLVMpp->TableNumber = thisrec->LV_table;
    pLVMdp->ReturnData = 0;
    pLVMdp->UserBuffer = NULL; 

		/*
		 * process each LV block contained in the given
		 * FS block.
		 *
		 * N.B. We'll exit the loop if the DosDevIOCtl fails,
		 *      but not if the LVM fails to execute the request
		 *	successfully.
		 */
    for( lvidx = 0; 
	 ((lvidx < MAX_LVBLKS_PER_FSBLK) && (tl_rc == 0)); 
	 lvidx++ ) {
	if( thisrec->LV_blkno[lvidx] != 0 ) {  /*
				* 0 can never be an actual LV block number
				* in this context since the 1st block in
				* the file system is NEVER relocated.
				*/
		/*
		 * tell the LVM to forget this one 
		 */
	    pLVMpp->LSN = thisrec->LV_blkno[lvidx];
	    tl_rc = DosDevIOCtl( LVHandle,		IOC_DC,	 
				 IODC_LV,	 	(void *) pLVMpp, 
				 sizeof(DDI_OS2LVM_param),
				 &ppLen,		 (void *) pLVMdp, 
				 sizeof(DDI_OS2LVM_data), 	 &dpLen	
				);
	    }  /* end if */
	}  /* end for lvidx = ... */

    if( tl_rc != 0 ) {	
	return( CBBL_LVMRC_7 - tl_rc );
	}
    else {
	thisrec->LVM_notified = -1;
	agg_recptr->resolved_blocks += 1;
	}

    return( tl_rc );
}				/* end tell_LVM() */
