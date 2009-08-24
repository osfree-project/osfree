/* $Id: fsckcbbl.c,v 1.1.1.1 2003/05/21 13:38:58 pasha Exp $ */

static char *SCCSID = "@(#)1.9  3/31/99 16:29:31 src/jfs/utils/chkdsk/fsckcbbl.c, jfschk, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		fsckcbbl.c
 *
 *   COMPONENT_NAME: 	jfschk
 *
 *   FUNCTIONS:
 *              check_BdBlkLst_FillLevel
 *              ClrBdBlkLst_processing
 *
 *              exec_clrbdblks
 *              examine_LVM_BadBlockLists
 *              get_LVM_BadBlockList_count
 *
 *
 */

#define INCL_TYPES

/*
 * defines and includes common among the xfsck modules
 */
#include "xfsckint.h"

#include <sesrqpkt.h>
#include <ioctl.h>
#include <dskioctl.h>
#include <specdefs.h>

typedef ULONG LSNumber;
typedef LSNumber RelocationTable;	/* actually this is an array */

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * superblock buffer pointer
  *
  *      defined in xchkdsk.c
  */
extern struct superblock *sb_ptr;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * fsck aggregate info structure pointer
  *
  *      defined in xchkdsk.c
  */
extern struct fsck_agg_record *agg_recptr;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For message processing
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

extern char *Vol_Label;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * Device information.
  *
  *      defined in xchkdsk.c
  */
extern HFILE  Dev_IOPort;
extern uint32 Dev_blksize;


/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */

retcode_t exec_clrbdblks( void );

retcode_t examine_LVM_BadBlockLists( int32, int32 * );

retcode_t get_LVM_BadBlockList_count( int32 * );

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */


/*****************************************************************************
 * NAME: check_BdBlkLst_FillLevel
 *
 * FUNCTION:	If any of the LVM Bad Block Tables composing the LVM Bad
 *		Block List for this partition is more than 50% full, issue a message.
 *
 * PARAMETERS:  none
 *
 * NOTES:	This routine is called only during autocheck processing.  chkdsk
 *		cannot perform /B processing at that time since the file system
 *		is not yet fully initialized.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t check_BdBlkLst_FillLevel ( )
{
  retcode_t ccbblfl_rc = FSCK_OK;
  int32 intermed_rc = 0;
  int32 num_tables = 0;
  int32 highest_percent_full = 0;

  intermed_rc = get_LVM_BadBlockList_count( &num_tables );
	
  if( intermed_rc == FSCK_OK ) {
    intermed_rc = examine_LVM_BadBlockLists( num_tables, &highest_percent_full );
    if( intermed_rc == FSCK_OK ) {
      if( highest_percent_full > 50 ) {
        fsck_send_msg( fsck_LVMFOUNDBDBLKS, 0, 0 );
        }
      }
    }
	
  return( ccbblfl_rc );
}                            /* end of check_BdBlkLst_FillLevel()  */


/*****************************************************************************
 * NAME: ClrBdBlkLst_processing
 *
 * FUNCTION:  Invoke the JFS processing to clear the LVM's bad block list.
 *
 * PARAMETERS:  none
 *
 * NOTES:	Starts a child process for the utility.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t ClrBdBlkLst_processing ( )
{
  retcode_t kcbbl_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  struct fsckcbbl_record  *cbblrecptr;
  int I_am_logredo = 0;
	
  cbblrecptr = &(agg_recptr->blkmp_ctlptr->cbblrec);
  if( cbblrecptr == NULL ) {	
    if( agg_recptr->blkmp_ctlptr == NULL ) {
      intermed_rc = alloc_wrksp( sizeof(fsck_blk_map_hdr_t),
                                            dynstg_blkmap_hdr, I_am_logredo,	
                                            (void **) &(agg_recptr->blkmp_ctlptr) );
      if( intermed_rc == FSCK_OK ) {
        strncpy( agg_recptr->blkmp_ctlptr->hdr.eyecatcher,
		fbmh_eyecatcher_string,
		strlen(fbmh_eyecatcher_string));  /* fill eyecatcher */
        }
      }
    intermed_rc = blkmap_get_ctl_page ( agg_recptr->blkmp_ctlptr );
    if( intermed_rc == FSCK_OK ) {
      cbblrecptr = &(agg_recptr->blkmp_ctlptr->cbblrec);
      }
    }
  if( cbblrecptr != NULL ) {	
    memcpy( (void *) &(cbblrecptr->eyecatcher), (void *) "*unset**", 8 );
    intermed_rc = blkmap_put_ctl_page ( agg_recptr->blkmp_ctlptr );
    }
	
  fsck_send_msg( fsck_LVMFSNOWAVAIL, 0, 0 );
  fsck_send_msg( fsck_LVMTRNSBBLKSTOJFS, 0, 0 );

	/*
	 * close the file system so clrbblks can get it
	 */
  close_volume();
	
  kcbbl_rc = exec_clrbdblks( );
	
	/*
	 * open the file system and get the clrbblks
	 * communication area
	 */
  open_volume( Vol_Label );
  intermed_rc = blkmap_get_ctl_page ( agg_recptr->blkmp_ctlptr );
  if( intermed_rc == FSCK_OK ) {
    if( memcmp((void *)&(cbblrecptr->eyecatcher),(void *)"*unset**", 8) != 0 ) {
	/*
	 * The eyecatcher field was reset.  there is good reason to
	 * believe that clrbblks processing did actually write to the
	 * record.
	 */
      sprintf( message_parm_0, "%ld", cbblrecptr->cbbl_retcode );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      fsck_send_msg( fsck_CLRBBLKSRC, 0, 1 );
	
      sprintf( message_parm_0, "%ld", cbblrecptr->LVM_lists );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      fsck_send_msg( fsck_CLRBBLVMNUMLISTS, 0, 1 );
	
      sprintf( message_parm_0, "%lld", (cbblrecptr->fs_last_metablk+1) );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%lld", (cbblrecptr->fs_first_wspblk-1) );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      fsck_send_msg( fsck_CLRBBRANGE, 0, 2 );
	
      sprintf( message_parm_0, "%ld", cbblrecptr->reloc_extents );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%ld", cbblrecptr->reloc_blocks );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      fsck_send_msg( fsck_CLRBBACTIVITY, 0, 2 );
	
      sprintf( message_parm_0, "%ld", cbblrecptr->total_bad_blocks );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%ld", cbblrecptr->resolved_blocks );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      fsck_send_msg( fsck_CLRBBRESULTS, 0, 2 );
      }
    }
	
  return( kcbbl_rc );
}                            /* end of ClrBdBlkLst_processing()  */
	
	
/*****************************************************************************
 * NAME: exec_clrbdblks
 *
 * FUNCTION:	Start clrbblks in a child process and wait for it to complete.
 *
 * PARAMETERS:  none
 *
 * NOTES:	
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t exec_clrbdblks( )
{
  retcode_t ec_rc = FSCK_OK;
  CHAR szFailName[CCHMAXPATH];
  RESULTCODES resc;
  CHAR szCommandLine[] = "clrbblks\0\0\0\0\0\0\0\0\0\0\0\0";
	
  memcpy((void *) &(szCommandLine[9]), (void *) Vol_Label, sizeof(Vol_Label));
	
  ec_rc = DosExecPgm( szFailName,   sizeof(szFailName),
                                 EXEC_SYNC,
                                 szCommandLine, (PSZ) NULL,
                                 &resc, "clrbblks.exe" );
	/*
	 * log the result of the DosExecPgm
	 */
  sprintf( message_parm_0, "%ld", ec_rc );
  msgprms[0] = message_parm_0;
  msgprmidx[0] = 0;
  fsck_send_msg( fsck_DOSEXECPGMRC, 0, 1 );
	/*
	 * if the DosExecPgm was successful, log and
	 * return the result of the clrbblks process
	 */
  if( ec_rc == 0 ) {
    sprintf( message_parm_0, "%ld,%ld", resc.codeTerminate, resc.codeResult );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    fsck_send_msg( fsck_CLRBBLKSRC, 0, 1 );
    ec_rc = resc.codeResult + resc.codeTerminate;
    }
	
  return( ec_rc );
}                            /* end of exec_clrbdblks()  */
	
	
/******************************************************************
 * NAME: 	examine_LVM_BadBlockLists
 *
 * FUNCTION: 	Query the LVM for (the set of tables composing) its bad
 *		block list for the file system.  Save the buffers containing
 *		the tables for use populating the JFS Bad Block Inode later.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 examine_LVM_BadBlockLists( int32 list_count, int32 *max_fill_level )
{
    int32 elbbl_rc = 0;
    int8  buffer_allocated = 0;
    int32 bbl_idx;
    DDI_OS2LVM_param LVMpp;
    DDI_OS2LVM_param *pLVMpp = &LVMpp;
    DDI_OS2LVM_data LVMdp;
    DDI_OS2LVM_data *pLVMdp = &LVMdp;
    ULONG ppLen = 0;
    ULONG dpLen = 0;
    int32 		bufsize = 1024;
    char 		*bufptr = NULL;
    BadBlockTableInfo 	*BBT_bufptr = NULL;
    int32 		num_entries, max_entries;
    int32			this_percent, highest_percent = 0;
    int32       alloc_rc = 0;
    int          I_am_logredo = 0;
	
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
    alloc_rc = alloc_wrksp( bufsize, dynstg_fsckcbblbuf1,
                                     I_am_logredo, (void **) &bufptr );
    if( (bufptr == NULL) || (alloc_rc != FSCK_OK) ) {
      elbbl_rc = FSCK_ENOMEMBDBLK2;
    }
    else {
      buffer_allocated = -1;
      }
	/*
	 *
	 * get info about each of the LVM Bad Block Tables
	 *
	 */
    for( bbl_idx = 0;
	 ( (bbl_idx < list_count) && (elbbl_rc == 0) );
	 bbl_idx++ ) {
		/*
		 * ask the LVM how big this table is.
		 */
	pLVMpp->Command = GET_TBL_SIZE;
	pLVMpp->TableNumber = bbl_idx;
	pLVMdp->ReturnData = 0;
	pLVMdp->UserBuffer = bufptr;
	elbbl_rc =
		DosDevIOCtl( Dev_IOPort,	IOC_DC,	
			     IODC_LV,		(void *) pLVMpp,	
			     sizeof(DDI_OS2LVM_param),
			     &ppLen,		(void *) pLVMdp,	
			     sizeof(DDI_OS2LVM_data),		&dpLen	
			    );	
          sprintf( message_parm_0, "%ld", elbbl_rc );
          msgprms[0] = message_parm_0;
          msgprmidx[0] = 0;
          fsck_send_msg( fsck_LVMGETTBLSIZERC, 0, 1 );
	
	if( elbbl_rc == 0 ) {  /* DosDevIOCtl successful */
	    if( pLVMdp->ReturnData != BBR_SUCCESS ) { /* cat 8, funct 69 failed */
		elbbl_rc = pLVMdp->ReturnData;
		break;
		}
	    else {
		BBT_bufptr = (BadBlockTableInfo *) bufptr;
		num_entries = BBT_bufptr->ActiveRelocations;
		max_entries = BBT_bufptr->MaxRelocationsAllowed;
	
		sprintf( message_parm_0, "%ld", bbl_idx );
		msgprms[0] = message_parm_0;
		msgprmidx[0] = 0;
		sprintf( message_parm_1, "%ld", max_entries );
		msgprms[1] = message_parm_1;
		msgprmidx[1] = 0;
		sprintf( message_parm_2, "%ld", num_entries );
		msgprms[2] = message_parm_2;
		msgprmidx[2] = 0;
		fsck_send_msg( fsck_CLRBBLVMLISTDATA, 0, 3 );
		}
	    }  /* end else DosDevIOCtl successful */
	
		/*
		 *
		 * If the table isn't empty
		 *
		 */
	if( (elbbl_rc == 0) && (num_entries > 0) && (max_entries > 0)) {  /*
				* successful so far and the
				* current table isn't empty
				*/
               this_percent = (num_entries * 100) / max_entries;
               if( this_percent > highest_percent ) {
                 highest_percent = this_percent;
                 }
	    }  /* end successful so far and table not empty */	
	}  /* end for bbl_idx */
			
	/*
	 * free the buffer
	 */
	
    if( elbbl_rc == 0 ) {
      *max_fill_level = highest_percent;
      }
    else {
      *max_fill_level = 0;
      }
	
    return( elbbl_rc );
}				/* end examine_LVM_BadBlockLists() */
	
	
/******************************************************************
 * NAME: 	get_LVM_BadBlockList_count
 *
 * FUNCTION: 	Query the LVM for the number of tables composing) its bad
 *		block list for the file system.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 get_LVM_BadBlockList_count( int32 *list_count )
{
    int32 glbblc_rc = 0;
	
    DDI_OS2LVM_param LVMpp;
    DDI_OS2LVM_param *pLVMpp = &LVMpp;
    DDI_OS2LVM_data LVMdp;
    DDI_OS2LVM_data *pLVMdp = &LVMdp;
    ULONG ppLen = 0;
    ULONG dpLen = 0;
    int32 		min_bufsize;
    int32 		bufsize = 1024;
    char 		*bufptr = NULL;
    BadBlockInfo	*BB_bufptr = NULL;
    BadBlockTableInfo 	*BBT_bufptr = NULL;
    int32 		tablesize;
    RelocationTable	*RTable = NULL;
    int32       alloc_rc = 0;
    int          I_am_logredo = 0;
	
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
    alloc_rc = alloc_wrksp( bufsize, dynstg_fsckcbblbuf2,
                                     I_am_logredo, (void **) &bufptr );
    if( (bufptr == NULL) || (alloc_rc != FSCK_OK) ) {
	return( FSCK_ENOMEMBDBLK1 );
	}
	
	/*
	 * ask the LVM how many bad block lists it has for the filesystem
	 */
    pLVMpp->Command = GET_BBR_INFO;
    pLVMdp->UserBuffer = bufptr;
    glbblc_rc = DosDevIOCtl( Dev_IOPort,	IOC_DC,	
			    IODC_LV,		(void *) pLVMpp,	
			    sizeof(DDI_OS2LVM_param),
			    &ppLen,		(void *) pLVMdp,	
			    sizeof(DDI_OS2LVM_data),		&dpLen
			    );	
    sprintf( message_parm_0, "%ld", glbblc_rc );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    fsck_send_msg( fsck_LVMGETBBLKINFORC, 0, 1 );
	
    if( glbblc_rc != 0 ) {
	return( glbblc_rc );
	}
    else {  /* DosDevIOCtl successful */
	if( pLVMdp->ReturnData != BBR_SUCCESS ) {	/* cat 8, funct 69 failed */
	    glbblc_rc = pLVMdp->ReturnData;
	    }
	else {
	    BB_bufptr = (BadBlockInfo *) bufptr;
	    *list_count = BB_bufptr->TotalTables;
	
	    sprintf( message_parm_0, "%ld", BB_bufptr->TotalTables );
	    msgprms[0] = message_parm_0;
	    msgprmidx[0] = 0;
	    fsck_send_msg( fsck_CLRBBLVMNUMLISTS, 0, 1 );
	    }
	}  /* end else DosDevIOCtl successful */
				
    return( glbblc_rc );
}				/* end get_LVM_BadBlockList_count() */

