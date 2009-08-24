/* $Id: lvmbdblk.h,v 1.1.1.1 2003/05/21 13:41:36 pasha Exp $ */

/* static char *SCCSID = "@(#)1.2  9/12/98 11:52:19 src/jfs/utils/format/lvmbdblk.h, jfsformat, w45.fs32, 990417.1";*/
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
 *   MODULE_NAME:		lvmbdblk.h
 *
 *   COMPONENT_NAME: 	jfsformat
 *
 *   FUNCTIONS: 	none
 *
 */
#ifndef H_LVMBDBLK
#define H_LVMBDBLK

typedef	ULONG	LSNumber;
typedef LSNumber RelocationTable;	/* actually this is an array */

/*----------------------------------------------------------------------
 *
 * lbb_bufrec describes one buffer containing an LVM bad block table
 *
 */
typedef struct lbb_bufrec {
    struct lbb_bufrec 	*next;
    char        		*buf_addr;
    int32    		buflen;
    USHORT		tblnum;
    ULONG		tblsize;
   } lbb_bufrec;

/*--------------------------------------------------------------------
 *
 * lvm_bdblk_record contains global information for bad block processing. 
 *
 */
typedef struct lvm_bdblk_record   {	
    char		eyecatcher[8];
    int32		num_tables;		/* number of bad block tables the LVM 
					 * maintains for this partition
					 */
    struct lbb_bufrec 	*bufs;		/* ptr to 1st in list of buffer records.
					 * each buffer holds an LVM bad block
					 * table for this partition
					 */
    LSNumber	first_relocatable_lsn; 	/* 1st device block after the metadata
					 * written at the beginning of the 
					 * aggregate during format processing.
					 */
    LSNumber 	last_relocatable_lsn;	/* the last device block before the 
					 * areas reserved for fixed-position
					 * metadata at the end of the aggregate.
					 */
   } lvm_bdblk_record;



/*----------------------------------------------------------------------
 *
 * routines defined in lvmbdblk.c
 *
 */
int32 disable_LVM_BdBlkReloc( int32 );

int32 enable_LVM_BdBlkReloc(  int32 );

int32 get_LVM_BdBlkLst_count( int32 );

int32 get_LVM_BadBlockLists( int32 );

int32 tell_LVM_ClearLists( int32 );

#endif
