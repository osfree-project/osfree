/* $Id: clrbblks.h,v 1.3 2004/07/24 00:49:06 pasha Exp $ */

/* static char *SCCSID = "@(#)1.3  9/22/98 13:43:08 src/jfs/utils/clrbblks/clrbblks.h, jfscbbl, w45.fs32, 990417.1";*/
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
 *   MODULE_NAME:		clrbblks.h
 *
 *   COMPONENT_NAME: 	jfscbbl
 *
 */
#ifndef H_CLRBBL
#define	H_CLRBBL


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *   C L R B B L K S   S H A R E D   D E C L A R A T I O N S
 *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

#define INCL_DOS
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#define INCL_DOSMEMMGR
#define INCL_DOSDEVICES

#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sesrqpkt.h>
#include <ioctl.h>
#include <dskioctl.h>
#include <specdefs.h>

#include "jfs_types.h"
#include "jfs_aixisms.h"
#include "jfs_superblock.h"
#include "jfs_filsys.h"
#include "jfs_inode.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_xtree.h"
#include "jfs_dtree.h"
#include "jfs_cntl.h"
#include "jfs_clrbblks.h"
#include "jfs_debug.h"
#include "fsckcbbl.h"

#include "devices.h"
#include "debug.h"

#include "fssubs.h"
#include "utilsubs.h"

#include "cbblwsp.h"

typedef ULONG LSNumber;
typedef LSNumber RelocationTable;	/* actually this is an array */

/*
 * miscellaneous constants
 */
#define BYTESPERPAGE      4096
#define MEMSEGSIZE        (64*1024)

/*
 * policy control
 */
#define CLRBBLKS_RETRY_LIMIT	5
#define XMOVESIZE			64	/* max size of extent to move + 1 */

/*
 * return codes
 */
#define CBBL_CANTALLOCBUFFR1	-2000001
#define CBBL_CANTALLOCBUFFR2	-2000002
#define CBBL_CANTOPENLV   		-2000003
#define CBBL_CANTOPENFS   		-2000004
#define CBBL_CANTBBINO		-2000005
#define CBBL_CANTREADBBINO	-2000006
#define CBBL_CANTREADIMAPINO	-2000007
#define CBBL_CANTREADIMAPCTL	-2000008
#define CBBL_CANTREADLMDTCHILD	-2000009
#define CBBL_CANTREADLMXTCHILD	-2000010
#define CBBL_CANTREADNEXTDTPG	-2000011
#define CBBL_CANTREADNEXTXTPG	-2000012
#define CBBL_CANTREADSBLKS	-2000013
#define CBBL_ENOMEM1		-2000014
#define CBBL_FSDIRTY			-2000015
#define CBBL_INVALMAGORVERS	-2000016


#define CBBL_INTERNAL_ERROR_1   	-2100001
#define CBBL_INTERNAL_ERROR_2   	-2100002
#define CBBL_INTERNAL_ERROR_3   	-2100003
#define CBBL_INTERNAL_ERROR_4   	-2100004
#define CBBL_INTERNAL_ERROR_5   	-2100005
#define CBBL_INTERNAL_ERROR_6   	-2100006
#define CBBL_INTERNAL_ERROR_7   	-2100007
#define CBBL_INTERNAL_ERROR_8   	-2100008
#define CBBL_INTERNAL_ERROR_9   	-2100009
#define CBBL_INTERNAL_ERROR_10  	-2100010
#define CBBL_INTERNAL_ERROR_11  	-2100011
#define CBBL_INTERNAL_ERROR_12  	-2100012
	/*
	 * fatal error return codes passed from the LVM will be reported
	 * as 	-22%%###
	 * where %% 	is a number unique to a line of code in clrbblks.
	 * and    ### 	is the rc passed back by the LVM
	 *
	 * The caller can recognize them because they are always
	 * 		-2300000 < rc < -2200000
	 */
#define CBBL_BEF_LVMRC		-2200000
#define CBBL_AFT_LVMRC		-2300000
#define CBBL_LVMRC_1		-2201000
#define CBBL_LVMRC_2		-2202000
#define CBBL_LVMRC_3		-2203000
#define CBBL_LVMRC_4		-2204000
#define CBBL_LVMRC_5		-2205000
#define CBBL_LVMRC_6		-2206000
#define CBBL_LVMRC_7		-2207000
#define CBBL_LVMRC_8		-2208000
#define CBBL_LVMRC_9		-2209000
#define CBBL_LVMRC_10		-2210000
	/*
	 * fatal error return codes passed from fscntl() will be reported
	 * as 	-23%%###
	 * where %% 	is a number unique to a line of code in clrbblks.
	 * and    ### 	is the rc passed back by fscntl()
	 *
	 * The caller can recognize them because they are always
	 * 		-2400000 < rc < -2300000
	 */
#define CBBL_BEF_FSCNTL		-2300000
#define CBBL_AFT_FSCNTL		-2400000
#define CBBL_FSCNTL_1		-2301000
#define CBBL_FSCNTL_2		-2302000

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *   C L R B B L K S   I N T E R N A L   R O U T I N E S
 *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


/*------------------------------------------------------------------
 * 			defined in cbblfsck.c
 */
int32 init_cbblfsck_record( void );
int32 complete_cbblfsck_record( void );


/*------------------------------------------------------------------
 * 			defined in cbblino.c
 */
int32 process_BadBlockInode( void );
int32 process_FilesetInodes( void );


/*------------------------------------------------------------------
 * 			defined in cbblwsp.c
 */
int32 baltree_load( LSNumber, int32 );
int32 baltree_purge ( void );
int32 baltree_search ( int64, cbbl_bdblk_recptr * );
int32 init_agg_record_cb( void );
int32 freelist_insert ( cbbl_bdblk_recptr );
int32 retrylist_insert ( cbbl_bdblk_recptr );
int32 retrylist_purge ( void );
int32 rslvdlist_insert ( cbbl_bdblk_recptr );
int32 seqlist_create ( void );
int32 seqlist_remove ( cbbl_bdblk_recptr );
int32 workspace_release_cb ( void );

/*------------------------------------------------------------------
 * 			defined in clrbblks.c
 */

int32 clrbblks( int32  ,  char  **, HFILE , char * );     //PS24072004
int32 alloc_to_BBInode( cbbl_bdblk_recptr );
int32 tell_LVM( cbbl_bdblk_recptr );

#endif








