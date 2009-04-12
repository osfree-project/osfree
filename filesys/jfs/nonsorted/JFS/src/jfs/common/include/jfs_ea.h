/* $Id: jfs_ea.h,v 1.1.1.1 2003/05/21 13:35:48 pasha Exp $ */

/* static char *SCCSID = "@(#)1.5  12/4/98 12:56:42 src/jfs/common/include/jfs_ea.h, sysjfs, w45.fs32, 990417.1";
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
 *
 * MODULE NAME:	jfs_ea.h
 *
 */

#ifndef H_JFS_EA
#define H_JFS_EA

/* bsedos.h defines EAOP to be regular (flat) pointers.  We need to redefine
 * it with _Seg16 pointers
 */
typedef struct _EAOP16 {
	PGEALIST _Seg16 fpGEAList; /* general EA list */
	PFEALIST _Seg16 fpFEAList; /* full EA list */
	ULONG oError;
} EAOP16;
#define EAOP EAOP16

/* Macros for defining maxiumum number of bytes supported for EAs */
#define MAXEASIZE	65535
#define MAXEALISTSIZE	MAXEASIZE

int32 jfs_ValidateFEAList(FEALIST *pfeal,
			  ULONG *poError);
int32 jfs_ConvertFEAListNames(FEALIST *pfeal,
			ULONG *poError);
int32 jfs_ConvertGEAListNames(GEALIST *pgeal,
			ULONG *poError);
void jfs_MergeFEALists(FEALIST *out,
		       FEALIST *orig,
		       FEALIST *chg);
int32 jfs_GEAListToFEAList(FEALIST *old,
			   FEALIST *new,
			   GEALIST *pgeal,
			   ULONG *poError);
void jfs_EABufFree(char *cp,
		   int size);
int32 jfs_ReadEA(inode_t *ip,
		 FEALIST *pfeal);
int32 jfs_WriteEA(inode_t * ip,
		  FEALIST *pfeal,
		  dxd_t *ea);
FEALIST *jfs_EABufAlloc(int32 size);
int jfs_ValidateUserFEAList(EAOP	*pEABuf,
			    FEALIST	**out,
			    ULONG	*oError);
int jfs_InitializeFEAList(inode_t	*ip,
			  int32		tid,
			  FEALIST	*pfeal);

int32 eaRelocate (inode_t  *ip);


#endif	/* H_JFS_EA */
