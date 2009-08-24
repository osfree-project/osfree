/* $Id: fsckcbbl.c,v 1.1.1.1 2003/05/21 13:43:08 pasha Exp $ */

static char *SCCSID = "@(#)1.1  2/9/99 09:11:26 src/jfs/utils/xpeek/fsckcbbl.c, jfsutil, w45.fs32, 990417.1";
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
*/

#include "xpeek.h"

/* 
 * JFS includes		
 */
#include <jfs_filsys.h>
#include <jfs_superblock.h>

/* 
 * libfs includes	
 */
#include <fsckcbbl.h>
#include <xfsck.h>
#include <fsckwsp.h>

/* 
 * global data 
 */
extern int64  fsckwsp_offset;	 	/* defined in xpeek.c    */
extern int64  jlog_super_offset; 	/* defined in xpeek.c    */

/* 
 * forward references
 */
int32 display_cbblfsck( struct fsck_blk_map_hdr * );
int32 display_fsck_wsphdr( struct fsck_blk_map_hdr * );
int32 display_logsuper( logsuper_t * );

int32 get_fsckwsphdr( struct fsck_blk_map_hdr * );
int32 get_logsuper( logsuper_t * );

int32 put_fsckwsphdr( struct fsck_blk_map_hdr * );
int32 put_logsuper( logsuper_t * );


/*************************************************************
 *  cbblfsck() 
 *
 */
void cbblfsck()
{
    struct fsck_blk_map_hdr	fsckwsp;

    if( get_fsckwsphdr( &fsckwsp ) ) {
	fputs("cbblfsck: error returned from get_fsckwsphdr()\n\n",stderr);
	return;
	}
    if (display_cbblfsck( &fsckwsp ) == XPEEK_CHANGED)
	if( put_fsckwsphdr( &fsckwsp ) )
	     fputs("cbblfsck: error returned from put_fsckwsphdr()\n\n",stderr);
    return;
}					/* end cbblfsck() */


/*************************************************************
 *  fsckwsphdr() 
 *
 */
void fsckwsphdr()
{
    struct fsck_blk_map_hdr	fsckwsp;

    if( get_fsckwsphdr( &fsckwsp ) ) {
	fputs("fsckwsphdr: error returned from get_fsckwsphdr()\n\n",stderr);
	return;
	}
    if (display_fsck_wsphdr( &fsckwsp ) == XPEEK_CHANGED)
	if( put_fsckwsphdr( &fsckwsp ) )
	     fputs("fsckwsphdr: error returned from put_fsckwsphdr()\n\n",stderr);
    return;
}					/* end fsckwsphdr() */


/*************************************************************
 *  logsuper() 
 *
 */
void logsuper()
{
    struct fsck_blk_map_hdr    fsckwsp;
    char			buffer[PSIZE];
    logsuper_t    *lsp;

    lsp = (logsuper_t *) &(buffer[0]);

    if( get_logsuper( lsp ) ) {
	fputs("logsuper: error returned from get_fsckwsphdr()\n\n",stderr);
	return;
	}
    if (display_logsuper( lsp ) == XPEEK_CHANGED)
	if( put_logsuper( lsp ) )
	     fputs("logsuper: error returned from put_fsckwsphdr()\n\n",stderr);
    return;
}					/* end logsuper() */

/*
 * ============================================================
 */

/*************************************************************
 *  display_cbblfsck() 
 *
 *****************  Sample output of display_cbblfsck() 

[1] eyecatcher:		'fsckcbbl'	[13] bufptr_eyecatch:	'cbblbufs'
[2] cbbl_retcode: 	0		[14] cbbl_agg_recptr:	0x00067f40
[3] fs_blksize: 		1024		[15] ImapInoPtr: 	0x00067cb0
[4] lv_blksize:		512		[16] ImapCtlPtr:	0x00061a70
[5] fs_lv_ratio: 		2		
[6] fs_last_metablk:	0x00000000000008a8     (0d02216)
[7] fs_first_wspblk:	0x00000000001fba2b     (0d02079275)
[8] total_bad_blocks:	20		     
[9] resolved_blocks:	18		[17] ImapLeafPtr:	0x00060a60
[10] reloc_extents:       14		[18] iagPtr:		0x00066ca0
[11] reloc_blocks:	1028		[19] InoExtPtr:		0x00060a70
[12] LVM_lists: 		3		

 */
int32 display_cbblfsck( struct fsck_blk_map_hdr *fsckwsp_ptr )
{
    int32  rc = XPEEK_OK;
    struct fsckcbbl_record *cbblrec;
    char  cmdline[512];
    int32  field;
    char  *token;
    char  s1[9];
    char  s2[9];

    cbblrec = &( fsckwsp_ptr->cbblrec );

changed:

    strncpy(s1, cbblrec->eyecatcher, 8);
    s1[8] = '\0';
    strncpy(s2, cbblrec->bufptr_eyecatcher, 8);  
    s2[8] = '\0';

    printf("[1] eyecatcher:	\t'%s'\t", s1);
    printf("[13] bufptr_eyecatch:\t'%s'\n", s2);
    printf("[2] cbbl_retcode:\t%d\t\t", cbblrec->cbbl_retcode);
    printf("[14] cbbl_agg_recptr:\t0x%08x\n", cbblrec->clrbblks_agg_recptr);
    printf("[3] fs_blksize:\t\t%d\t\t", cbblrec->fs_blksize);
    printf("[15] ImapInoPtr:\t0x%08x\n", cbblrec->ImapInoPtr);
    printf("[4] lv_blksize:\t\t%d\t\t", cbblrec->lv_blksize);
    printf("[16] ImapCtlPtr:\t0x%08x\n", cbblrec->ImapCtlPtr);
    printf("[5] fs_lv_ratio:\t%d\n", cbblrec->fs_lv_ratio);
    printf("[6] last_metablk:\t0x%016llx    (0d0%lld)\n", 
		cbblrec->fs_last_metablk, cbblrec->fs_last_metablk);
    printf("[7] first_wspblk:\t0x%016llx    (0d0%lld)\n", 
		cbblrec->fs_first_wspblk, cbblrec->fs_first_wspblk);
    printf("[8] total_bad_blocks:\t%0d\n", cbblrec->total_bad_blocks);
    printf("[9] resolved_blocks:\t%d\t\t", cbblrec->resolved_blocks);
    printf("[17] ImapLeafPtr:\t0x%08x\n", cbblrec->ImapLeafPtr);
    printf("[10] reloc_extents:\t%d\t\t", cbblrec->reloc_extents);
    printf("[18] iagPtr:\t\t0x%08x\n", cbblrec->iagPtr);
    printf("[11] reloc_blocks:\t%d\t\t", cbblrec->reloc_blocks);
    printf("[19] InoExtPtr:\t\t0x%08x\n", cbblrec->InoExtPtr);
    printf("[12] LVM_lists:\t\t%d\n", cbblrec->LVM_lists);

retry:
    fputs("display_cbblfsck: [m]odify or e[x]it: ", stdout);
    gets(cmdline);
    token = strtok(cmdline, " 	");
    if (token == 0 || token[0] != 'm')
	return rc;
	
    field = m_parse(cmdline, 19, &token);
    if (field == 0)
	goto retry;

    switch (field)  {
	case 1:
	    strncpy(cbblrec->eyecatcher, token, 8);
	    break;
	case 2:
	    cbblrec->cbbl_retcode = strtol(token, 0, 8);
	    break;
	case 3:
	    cbblrec->fs_blksize = strtol(token, 0, 0);
	    break;
	case 4:
	    cbblrec->lv_blksize = strtol(token, 0, 0);
	    break;
	case 5:
	    cbblrec->fs_lv_ratio = strtol(token, 0, 0);
	    break;
	case 6:
	    cbblrec->fs_last_metablk = strtoull(token, 0, 16);
	    break;
	case 7:
	    cbblrec->fs_first_wspblk = strtoull(token, 0, 16);
	    break;
	case 8:
	    cbblrec->total_bad_blocks = strtol(token, 0, 0);
	    break;
	case 9:
	    cbblrec->resolved_blocks = strtol(token, 0, 0);
	    break;
	case 10:
	    cbblrec->reloc_extents = strtol(token, 0, 0);
	    break;
	case 11:
	    cbblrec->reloc_blocks = strtol(token, 0, 0);
	    break;
	case 12:
	    cbblrec->LVM_lists = strtoul(token, 0, 0);
	    break;
	case 13:
	    strncpy(cbblrec->bufptr_eyecatcher, token, 8);
	    break;
	case 14:
	    cbblrec->clrbblks_agg_recptr = (void *) strtoul(token, 0, 16);
	    break;
	case 15:
	    cbblrec->ImapInoPtr = (void *) strtoul(token, 0, 16);
	    break;
	case 16:
	    cbblrec->ImapCtlPtr = (void *) strtoul(token, 0, 16);
	    break;
	case 17:
	    cbblrec->ImapLeafPtr = (void *) strtoul(token, 0, 16);
	    break;
	case 18:
	    cbblrec->iagPtr = (void *) strtoul(token, 0, 16);
	    break;
	case 19:
	    cbblrec->InoExtPtr = (void *) strtoul(token, 0, 16);
	    break;
	}
	rc = XPEEK_CHANGED;
	goto changed;
}				/* end display_cbblfsck() */


/*************************************************************
 *  display_fsck_wsphdr() 
 *
 *
 *****************  Sample output of display_fsck_wsphdr() 

[1] eyecatcher:			'wspblkmp'	
[2] last_entry_pos:		0		
[3] next_entry_pos: 		0		
[4] start_time: 			2/5/999.11.2
[5] end_time:	 		2/5/999.11.4	
[6] return_code:		0
[7] super_buff_addr:		0x164f0be0 
[8] agg_record_addr:		0x164f4cd0		     
[9] bmap_record_addr:		0x16463ce0		
[10] fscklog_agg_offset:	0x000000003f786600	(0d01064855040)
[11] fscklog_full:		0
[12] fscklog_buf_allocated:	-1	
[13] fscklog_buf_alloc_err:	0
[14] num_logwrite_errors:	0

 */

int32 display_fsck_wsphdr( struct fsck_blk_map_hdr *wp )
{
    int32  rc = XPEEK_OK;
    char  cmdline[512];
    int32  field;
    char  *token;
    char  s1[9];

changed:

    strncpy(s1, wp->hdr.eyecatcher, 8);
    s1[8] = '\0';

    printf("[1] eyecatcher:\t\t\t'%s'\n", s1 );
    printf("[2] last_entry_pos:\t\t%ld\n", wp->hdr.last_entry_pos );
    printf("[3] next_entry_pos:\t\t%ld\n", wp->hdr.next_entry_pos );
    printf("[4] start_time:\t\t\t%s\n", wp->hdr.start_time );
    printf("[5] end_time:\t\t\t%s\n", wp->hdr.end_time );
    printf("[6] return_code:\t\t%d\n", wp->hdr.return_code );
    printf("[7] super_buff_addr:\t\t0x%08x\n", wp->hdr.super_buff_addr );
    printf("[8] agg_record_addr:\t\t0x%08x\n", wp->hdr.agg_record_addr );
    printf("[9] bmap_record_addr:\t\t0x%08x\n", wp->hdr.bmap_record_addr );
    printf("[10] fscklog_agg_offset:\t0x%016llx    (0d0%lld)\n", 
		wp->hdr.fscklog_agg_offset, wp->hdr.fscklog_agg_offset);
    printf("[11] fscklog_full:\t\t%d\n", wp->hdr.fscklog_full );
    printf("[12] fscklog_buf_allocated:\t%d\n", wp->hdr.fscklog_buf_allocated );
    printf("[13] fscklog_buf_alloc_err:\t%d\n", wp->hdr.fscklog_buf_alloc_err );
    printf("[14] num_logwrite_errors:\t%d\n", wp->hdr.num_logwrite_errors );

retry:
    fputs("display_fsck_wsphdr: [m]odify or e[x]it: ", stdout);
    gets(cmdline);
    token = strtok(cmdline, " 	");
    if (token == 0 || token[0] != 'm')
	return rc;
	
    field = m_parse(cmdline, 14, &token);
    if (field == 0)
	goto retry;

    switch (field)  {
	case 1:
	    strncpy(wp->hdr.eyecatcher, token, 8);
	    break;
	case 2:
	   wp->hdr.last_entry_pos = strtol(token, 0, 0);
	    break;
	case 3:
	    wp->hdr.next_entry_pos = strtol(token, 0, 0);
	    break;
	case 4:
	    strncpy(wp->hdr.start_time, token, 16);
	    break;
	case 5:
	    strncpy(wp->hdr.end_time, token, 16);
	    break;
	case 6:
	    wp->hdr.return_code = strtol(token, 0, 0);
	    break;
	case 7:
	    wp->hdr.super_buff_addr = (char *) strtoul(token, 0, 16);
	    break;
	case 8:
	    wp->hdr.agg_record_addr = (char *) strtoul(token, 0, 16);
	    break;
	case 9:
	    wp->hdr.bmap_record_addr = (char *) strtoul(token, 0, 16);
	    break;
	case 10:
	    wp->hdr.fscklog_agg_offset = strtoull(token, 0, 16);
	    break;
	case 11:
	    wp->hdr.fscklog_full  = strtol(token, 0, 0);
	    break;
	case 12:
	    wp->hdr.fscklog_buf_allocated = strtol(token, 0, 0);
	    break;
	case 13:
	    wp->hdr.fscklog_buf_alloc_err = strtol(token, 0, 0);
	    break;
	case 14:
	    wp->hdr.num_logwrite_errors = strtol(token, 0, 8);
	    break;
	}
	rc = XPEEK_CHANGED;
	goto changed;
}				/* end display_fsck_wsphdr() */


/*************************************************************
 *  display_logsuper() 
 *
 *
 *****************  Sample output of display_logsuper() 

[1] eyecatcher:			'wspblkmp'	
[2] last_entry_pos:		0		
[3] next_entry_pos: 		0		
[4] start_time: 			2/5/999.11.2
[5] end_time:	 		2/5/999.11.4	
[6] return_code:		0
[7] super_buff_addr:		0x164f0be0 
[8] agg_record_addr:		0x164f4cd0		     
[9] bmap_record_addr:		0x16463ce0		
[10] fscklog_agg_offset:	0x000000003f786600	(0d01064855040)
[11] fscklog_full:		0
[12] fscklog_buf_allocated:	-1	
[13] fscklog_buf_alloc_err:	0
[14] num_logwrite_errors:	0

 */

int32 display_logsuper( logsuper_t *lsp )
{
    int32  rc = XPEEK_OK;
    char  cmdline[512];
    int32  field;
    char  *token;
    char  s1[9];

changed:

    printf("[1] magic:\t\t\t0x%lx\n", lsp->magic );
    printf("[2] version:\t\t\t%ld\n", lsp->version );
    printf("[3] serial:\t\t\t%ld\n", lsp->serial );
    printf("[4] log size:\t\t\t%ld\t(at 4096 bytes/log block)\n", lsp->size );
    printf("[5] agg block size:\t\t%ld\t(bytes)\n", lsp->bsize );
    printf("[6] log2(agg blk size):\t\t%d\n", lsp->l2bsize );
    printf("[7] flag:\t\t\t0x%08x\n", lsp->flag );
    printf("[8] state:\t\t\t0x%08x\n", lsp->state );
    printf("[9] end:\t\t\t0x%lx  (d %ld)\n", lsp->end, lsp->end);
    printf("[]  active[0]:\t0x%lx (%ld)\t active[4]:\t0x%lx (%ld)\n",
		lsp->active[0], lsp->active[0],
		lsp->active[4], lsp->active[4] );
    printf("    active[1]:\t0x%lx (%ld)\t active[5]:\t0x%lx (%ld)\n",
		lsp->active[1], lsp->active[1],
		lsp->active[5], lsp->active[5] );
    printf("    active[2]:\t0x%lx (%ld)\t active[6]:\t0x%lx (%ld)\n",
		lsp->active[2], lsp->active[2],
		lsp->active[6], lsp->active[6] );
    printf("    active[3]:\t0x%lx (%ld)\t active[7]:\t0x%lx (%ld)\n",
		lsp->active[3], lsp->active[3],
		lsp->active[7], lsp->active[7] );

retry:
    fputs("display_fsck_wsphdr: [m]odify or e[x]it: ", stdout);
    gets(cmdline);
    token = strtok(cmdline, " 	");
    if (token == 0 || token[0] != 'm')
	return rc;
	
    field = m_parse(cmdline, 9, &token);
    if (field == 0)
	goto retry;

    switch (field)  {
	case 1:
	    lsp->magic = strtoul(token, 0, 16);
	    break;
	case 2:
	   lsp->version = strtol(token, 0, 0);
	    break;
	case 3:
	    lsp->serial = strtol(token, 0, 0);
	    break;
	case 4:
	    lsp->size = strtol(token, 0, 0);
	    break;
	case 5:
	    lsp->bsize = strtol(token, 0, 0);
	    break;
	case 6:
	    lsp->l2bsize = strtol(token, 0, 0);
	    break;
	case 7:
	    lsp->flag = strtoul(token, 0, 16);
	    break;
	case 8:
	    lsp->state = strtoul(token, 0, 16);
	    break;
	case 9:
	    lsp->end = strtoul(token, 0, 16);
	    break;
	}
	rc = XPEEK_CHANGED;
	goto changed;
}				/* end display_logsuper() */


/*************************************************************
 *  get_fsckwsphdr() 
 *
 */
int32 get_fsckwsphdr( struct fsck_blk_map_hdr *wsphdr_ptr )
{
    if( xRead( fsckwsp_offset, PSIZE, (char *)wsphdr_ptr ) )  {
	fputs("get_fsckwsphdr: error returned from xRead(fsckwsp_offset)\n\n",stderr);
	return( -2 );
	}
	 
    return( 0 );
}				/* end get_fsckwsphdr() */


/*************************************************************
 *  get_logsuper() 
 *
 */
int32 get_logsuper( logsuper_t *lsp )
{
    if( xRead( jlog_super_offset, PSIZE, (char *)lsp ) )  {
	fputs("get_logsuper: error returned from xRead(jlog_super_offset)\n\n",stderr);
	return( -3 );
	}
	 
    return( 0 );
}				/* end get_logsuper() */
	 
	 
/*************************************************************
 *  put_fsckwsphdr() 
 *
 */
int32 put_fsckwsphdr( blk_cntrlptr wsphdr_ptr )
{
    if( xWrite( fsckwsp_offset, PSIZE, (char *)wsphdr_ptr ) )  {
	fputs("put_fsckwsphdr: error returned from xWrite(fsckwsp_offset)\n\n",stderr);
	return( -4 );
	}
	 
    return( 0 );
}				/* end put_fsckwsphdr() */
	 
	 
/*************************************************************
 *  put_logsuper() 
 *
 */
int32 put_logsuper( logsuper_t *lsp )
{
    if( xWrite( jlog_super_offset, PSIZE, (char *)lsp ) )  {
	fputs("put_logsuper: error returned from xWrite(jlog_super_offset)\n\n",stderr);
	return( -5 );
	}
	 
    return( 0 );
}				/* end put_logsuper() */

