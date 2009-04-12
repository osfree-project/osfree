/* $Id: super2.c,v 1.4 2004/03/21 02:43:34 pasha Exp $ */

static char *SCCSID = "@(#)1.2  6/16/99 08:33:28 src/jfs/utils/xpeek/super2.c, jfsutil, w45.fs32, fixbld";
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
 *
 *   MODULE_NAME:		super2.c
 *
 *   COMPONENT_NAME: 	jfsutil
 *
 *   FUNCTION:	Display and Alter primary or secondary superblock
 *
 */

#include "xpeek.h"

/* JFS includes		*/
#include <jfs_filsys.h>

/* libfs includes	*/
#include <super.h>

int32 display_super2( struct superblock *sb);

void superblock2()
{
	char			*arg;
	int			is_primary = 1;
	struct superblock	super;

	if ((arg = strtok(0, " 	")) != 0) {	/* space & tab	*/
		if (strcmp(arg, "p") == 0)
			is_primary = 1;
		else if (strcmp(arg, "s") == 0)
			is_primary = 0;
		else {
			fprintf(stderr, "superblock: invalid parameter '%s'\n\n", arg);
			return;
		}
	}
	else if (strtok(0, " 	")) {
		fputs("superblock: too many arguments.\n\n", stderr);
		return;
	}

	if (ujfs_get_superblk(fd, &super, is_primary)) {
		fputs("superblock: error returned from ujfs_get_superblk\n\n",stderr);
		return;
	}
	if (display_super2(&super) == XPEEK_CHANGED)
		if (ujfs_put_superblk(fd, &super, (short)is_primary))
			fputs("superblock: error returned from ujfs_put_superblk\n\n",stderr);
	return;
}

/*****************************************************************************
 ****************  Sample output of display_super2():

[1] s_magic:		'JFS1'		[13] s_aim2.len:		8			
[2] s_version:		1		[14] s_aim2.addr1:	0x00
[3] s_size:	0x0000000000031fa0	[15] s_aim2.addr2:	0x00000250
[4] s_bsize:		1024		     s_aim2.address:	592
[5] s_l2bsize:		10		[19] s_logdev:		0x00000000
[6] s_l2bfactor:		1		[20] s_logserial:	0x00000000
[7] s_pbsize:		512		[21] s_logpxd.len:	4096
[8] s_l2pbsize:		9		[22] s_logpxd.addr1:	0x00
[9] s_agsize:		0x00002000	[23] s_logpxd.addr2:	0x000fdee1
[10] s_flag:		0x80000100	     s_logpxd.address:	1040097
	OS2		       		[24] s_fsckpxd.len:	332
	GROUPCOMMIT	       		[25] s_fsckpxd.addr1:	0x00
	       		           	[26] s_fsckpxd.addr2:	0x000fdd95
	INLINELOG			      s_fsckpxd.address:	1039765
[11] s_state:		0x00000000	[27] fsckloglen		200
	CLEAN				[28] fscklog		1
[12] s_compress:	0		[29] s_fpack:		'        '
[13] s_ait2.len:		16		[30] s_attach:		2
[14] s_ait2.addr1:	0x00		[31] totalUsable: 0x0000000000000000
[15] s_ait2.addr2:	0x00000258	[32] minFree:	  0x00000000fe000000
     s_ait2.address:	600		[33] realFree:	  0x19a9000000000000

******************************************************************************/

int32 display_super2( struct superblock *sb)
{
	char	*aix_platform;
	char	*os2_platform;
	char	cmdline[512];
	int32	field;
	char	*dir_posix;
	char	*dir_unicode;
	int32	rc = XPEEK_OK;
	char	*state;
	char	*token;
	char	*commit;
	char	*groupcommit;
	char	*lazycommit;
	char	*tmpfs;
	char	*inlinelog;
	char	*inlinemove;
	char	*badsait;
	char	*sparse;                                              /* @F1 */
	char	*dasdenabled;                                      /* @F1 */
	char	*dasdprime;                                          /* @F1 */

changed:
	switch (sb->s_state)
	{
	case FM_CLEAN:
		state="CLEAN";
		break;
	case FM_MOUNT:
		state="MOUNT";
		break;
	case FM_DIRTY:
		state="DIRTY";
		break;
	case FM_LOGREDO:
		state="LOGREDO";
		break;
	default:
		state="Unknown State";
		break;
	}
	if  (sb->s_flag & JFS_AIX)
		aix_platform="AIX";
	else
		aix_platform="   ";
	if  (sb->s_flag & JFS_OS2)
		os2_platform="OS2";
	else
		os2_platform="   ";
	if  (sb->s_flag & JFS_DFS)
		dir_posix="DFS";
	else
		dir_posix="   ";
	if  (sb->s_flag & JFS_UNICODE)
		dir_unicode="UNICODE";
	else
		dir_unicode="       ";
	if  (sb->s_flag & JFS_GROUPCOMMIT)
		groupcommit="GROUPCOMMIT";
	else
		groupcommit="           ";
	if  (sb->s_flag & JFS_LAZYCOMMIT)
		lazycommit="LAZYCOMMIT";
	else
		lazycommit="          ";
	if  (sb->s_flag & JFS_TMPFS)
		tmpfs="TMPFS";
	else
		tmpfs="     ";
	if  (sb->s_flag & JFS_INLINELOG)
		inlinelog="INLINELOG";
	else
		inlinelog="         ";
	if  (sb->s_flag & JFS_INLINEMOVE)
		inlinemove="INLINEMOVE";
	else
		inlinemove="          ";
	if  (sb->s_flag & JFS_BAD_SAIT)
		badsait="BAD_SAIT";
	else
		badsait="        ";
	if  (sb->s_flag & JFS_SPARSE)                                   /* @F1 */
		sparse="SPARSE";                                         /* @F1 */
	else                                                                       /* @F1 */
		sparse="         ";                                           /* @F1 */
	if  (sb->s_flag & JFS_DASD_ENABLED)                         /* @F1 */
		dasdenabled="DASD_ENABLED";                       /* @F1 */
	else                                                                        /* @F1 */
		dasdenabled="         ";                                    /* @F1 */
	if  (sb->s_flag & JFS_DASD_PRIME)                              /* @F1 */
		dasdprime="DASD_PRIME";                                /* @F1 */
	else                                                                          /* @F1 */
		dasdprime="         ";                                         /* @F1 */

	printf("[1] s_magic:\t\t'%4.4s'\t\t", &(sb->s_magic));
	printf("[16] s_aim2.len:\t%d\n", sb->s_aim2.len);
	printf("[2] s_version:\t\t%d\t\t", sb->s_version);
	printf("[17] s_aim2.addr1:\t0x%02x\n", sb->s_aim2.addr1);
	printf("[3] s_size:\t0x%016llx\t", sb->s_size);
	printf("[18] s_aim2.addr2:\t0x%08x\n", sb->s_aim2.addr2);
	printf("[4] s_bsize:\t\t%d\t\t", sb->s_bsize);
	printf("     s_aim2.address:\t%lld\n", addressPXD(&sb->s_aim2));
	printf("[5] s_l2bsize:\t\t%d\t\t", sb->s_l2bsize);
	printf("[19] s_logdev:\t\t0x%08x\n", sb->s_logdev);
	printf("[6] s_l2bfactor:\t%d\t\t", sb->s_l2bfactor);
	printf("[20] s_logserial:\t0x%08x\n", sb->s_logserial);
	printf("[7] s_pbsize:\t\t%d\t\t", sb->s_pbsize);
	printf("[21] s_logpxd.len:\t%d\n", sb->s_logpxd.len);
	printf("[8] s_l2pbsize:\t\t%d\t\t", sb->s_l2pbsize);
	printf("[22] s_logpxd.addr1:\t0x%02x\n", sb->s_logpxd.addr1);
	printf("[9]  s_agsize:\t\t0x%08x\t", sb->s_agsize);
	printf("[23] s_logpxd.addr2:\t0x%08x\n", sb->s_logpxd.addr2);
	printf("[10] s_flag:\t\t0x%08x\t", sb->s_flag);
	printf("     s_logpxd.address:\t%lld\n", addressPXD(&sb->s_logpxd));
	printf("    %s   %s   %s   %s   %s\t",
		aix_platform, os2_platform, dir_posix, dir_unicode, tmpfs );    /* @F1 */
	printf("[24] s_fsckpxd.len:\t%d\n", sb->s_fsckpxd.len);
	printf("    %s   %s   %s\t", groupcommit, lazycommit, badsait );         /* @F1 */
	printf("[25] s_fsckpxd.addr1:\t0x%02x\n", sb->s_fsckpxd.addr1);
	printf("    %s   %s   %s\t", sparse, inlinelog, inlinemove );                  /* @F1 */
	printf("[26] s_fsckpxd.addr2:\t0x%08x\n", sb->s_fsckpxd.addr2);
	printf("    %s   %s\t\t", dasdenabled, dasdprime);                             /* @F1 */
	printf("     s_fsckpxd.address:\t%lld\n", addressPXD(&sb->s_fsckpxd));
	printf("[11] s_state:\t\t0x%08x\t", sb->s_state);
	printf("[27] s_fsckloglen:\t%ld\t\n", sb->s_fsckloglen);
	printf("\t%13s\t\t\t", state);
	printf("[28] s_fscklog:\t\t%d\t\n", sb->s_fscklog);
	printf("[12] s_compress:\t%d\t\t",sb->s_compress);
	printf("[29] s_fpack:\t\t'%8s'\n", &(sb->s_fpack));
	printf("[13] s_ait2.len:\t%d\t\t", sb->s_ait2.len);
	printf("[30] s_attach:\t\t%1d\n", sb->s_attach);
	printf("[14] s_ait2.addr1:\t0x%02x\t\t", sb->s_ait2.addr1);
	printf("[31] totalUsable: 0x%016llx\n", sb->totalUsable);
	printf("[15] s_ait2.addr2:\t0x%08x\t", sb->s_ait2.addr2);
	printf("[32] minFree:\t  0x%016llx\n", sb->minFree);
	printf("     s_ait2.address:\t%lld\t\t", addressPXD(&sb->s_ait2));
	printf("[33] realFree:\t  0x%016llx\n", sb->realFree);

retry:
	fputs("display_super: [m]odify or e[x]it: ", stdout);
	gets(cmdline);
	token = strtok(cmdline, " 	");
	if (token == 0 || token[0] != 'm')
		return rc;
	
	field = m_parse(cmdline, 33, &token);
	if (field == 0)
		goto retry;

	switch (field)
	{
	case 1:
		strncpy(sb->s_magic, token, 4);
		break;
	case 2:
		sb->s_version = strtoul(token, 0, 0);
		break;
	case 3:
		sb->s_size = strtoll(token, 0, 16);
		break;
	case 4:
		sb->s_bsize = strtol(token, 0, 0);
		break;
	case 5:
		sb->s_l2bsize = strtol(token, 0, 0);
		break;
	case 6:
		sb->s_l2bfactor = strtol(token, 0, 0);
		break;
	case 7:
		sb->s_pbsize = strtol(token, 0, 0);
		break;
	case 8:
		sb->s_l2pbsize = strtol(token, 0, 0);
		break;
	case 9:
		sb->s_agsize = strtoul(token, 0, 16);
		break;
	case 10:
		sb->s_flag = strtoul(token, 0, 16);
		break;
	case 11:
		sb->s_state = strtoul(token, 0, 16);
		break;
	case 12:
		sb->s_compress = strtoul(token, 0, 0);
		break;
	case 13:
		sb->s_ait2.len = strtoul(token, 0, 0);
		break;
	case 14:
		sb->s_ait2.addr1 = strtoul(token, 0, 16);
		break;
	case 15:
		sb->s_ait2.addr2 = strtoul(token, 0, 16);
		break;
	case 16:
		sb->s_aim2.len = strtoul(token, 0, 0);
		break;
	case 17:
		sb->s_aim2.addr1 = strtoul(token, 0, 16);
		break;
	case 18:
		sb->s_aim2.addr2 = strtoul(token, 0, 16);
		break;
	case 19:
		sb->s_logdev = strtoul(token, 0, 16);
		break;
	case 20:
		sb->s_logserial = strtol(token, 0, 16);
		break;
	case 21:
		sb->s_logpxd.len = strtoul(token, 0, 0);
		break;
	case 22:
		sb->s_logpxd.addr1 = strtoul(token, 0, 16);
		break;
	case 23:
		sb->s_logpxd.addr2 = strtoul(token, 0, 16);
		break;
	case 24:
		sb->s_fsckpxd.len = strtoul(token, 0, 0);
		break;
	case 25:
		sb->s_fsckpxd.addr1 = strtoul(token, 0, 16);
		break;
	case 26:
		sb->s_fsckpxd.addr2 = strtoul(token, 0, 16);
		break;
	case 27:
		sb->s_fsckloglen = strtol(token, 0, 0);
		break;
	case 28:
		sb->s_fscklog = strtol(token, 0, 0);
		break;
	case 29:
		strncpy(sb->s_fpack, token, 8);
		break;
	case 30:
		sb->s_attach = strtol(token, 0, 0);
		break;
	case 31:
		sb->totalUsable = strtoull(token, 0, 16);
		break;
	case 32:
		sb->minFree = strtoull(token, 0, 16);
		break;
	case 33:
		sb->realFree = strtoull(token, 0, 16);
		break;
	default:
		fputs("display_super: Field number out of range\n", stderr);
		goto retry;
	}
	rc = XPEEK_CHANGED;
	goto changed;
}
