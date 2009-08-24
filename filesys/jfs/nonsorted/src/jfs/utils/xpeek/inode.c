/* $Id: inode.c,v 1.2 2004/03/21 02:43:32 pasha Exp $ */

static char *SCCSID = "@(#)1.10  10/20/98 10:17:12 src/jfs/utils/xpeek/inode.c, jfsutil, w45.fs32, 990417.1";
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
 *   MODULE_NAME:	inode.c
 *
 *   FUNCTION: 		display/modify an inode 
 *
 */

#include "xpeek.h"

/* JFS includes */
#include <jfs_filsys.h>

/* libfs includes */
#include <inode.h>
#include <devices.h>

char *mode_string(mode_t);
int32 change_inode(struct dinode *);

void inode()
{
	int64	address;
	int64	block_num;
	char	cmd_line[80];
	struct dinode	inode;
	ino_t	inum;
	uint32	offset;
	char	*token;
	ino_t	which_table = FILESYSTEM_I;

	token = strtok(0, " 	");
	if (token == 0) {
		fputs("inode: Please enter: inum [ a | 0 ]\ninode> ",stdout);
		gets(cmd_line);
		token=strtok(cmd_line, " 	");
		if (token == 0)
			return;
	}
	errno = 0;
	inum = strtoul(token, 0, 0);
	if (inum == 0 && errno) {
		fputs("inode: invalid inum\n\n", stderr);
		return;
	}
	token = strtok(0, " 	");
	if (token) {
		if (token[0] == 'a')
			which_table = AGGREGATE_I;
		else if (token[0] == 's')
			which_table = AGGREGATE_2ND_I;
		else if (token[0] != '0') {
			fputs("inode: invalide fileset\n\n", stderr);
			return;
		}
	}
	if (strtok(0, " 	")) {
		fputs("inode: Too many arguments\n\n", stderr);
		return;
	}

/*	if (ujfs_rwinode(fd, &inode, inum, GET, bsize, which_table)) {
		fputs("inode: error reading inode\n\n", stderr);
		return;
	}
*/
	if (find_inode(inum, which_table, &address) ||
		xRead(address, sizeof(dinode_t), (char *)&inode))
	{
		fputs("inode: error reading inode\n\n", stderr);
		return;
	}

	block_num = address >> l2bsize;
	offset = address & (bsize -1);

changed:
	printf("Inode %d at block %lld, offset 0x%x:\n\n", inum, block_num,
		offset);
	display_inode(&inode);
	if (change_inode(&inode) == XPEEK_CHANGED) {
		if (xWrite(address, sizeof(dinode_t), (char *)&inode)) {
			fputs("inode: error writing inode\n\n", stderr);
			return;
		}
		goto changed;
	}
	return;
}

/****************************************************************************
 ******	Sample output of dislpay_inode():

Inode 2 at block 402, offset 0x0:

[1] di_inostamp:	0x00000000	[19] di_mtime.tv_nsec:	0x00000000
[2] di_fileset:		16		[20] di_otime.tv_sec:	0x322dea3f
[3] di_number:		2		[21] di_otime.tv_nsec:	0x00000000
[4] di_gen:		1		[22] di_acl.flag:	0x00
[5] di_ixpxd.len:	32		[23] di_acl.rsrvd:	Not Displayed
[6] di_ixpxd.addr1:	0x00		[24] di_acl.size:	0x00000000
[7] di_ixpxd.addr2:	0x00000190	[25] di_acl.len:	0
     di_ixpxd.address:	400		[26] di_acl.addr1:	0x00
[8] di_size:	0x0000000000000100	[27] di_acl.addr2:	0x00000000
[9] di_nblocks:	0x0000000000000000	     di_acl.address:	0
[10] di_nlink:		1		[28] di_ea.flag:	0x00
[11] di_uid:		0		[29] di_ea.rsrvd:	Not Displayed
[12] di_gid:		0		[30] di_ea.size:	0x00000000
[13] di_mode:		0x000141c0	[31] di_ea.len:		0
		0040700	drwx		[32] di_ea.addr1:	0x00
[14] di_atime.tv_sec:	0x322dea3f	[33] di_ea.addr2:	0x00000000
[15] di_atime.tv_nsec:	0x00000000	     di_ea.address:	0
[16] di_ctime.tv_sec:	0x322dea3f	[34] di_compress:	0
[17] di_ctime.tv_nsec:	0x00000000	[35] di_acltype:	0x00000000
[18] di_mtime.tv_sec:	0x322dea3f

*****************************************************************************/

void display_inode(
struct dinode *inode)
{
	printf("[1] di_inostamp:\t0x%08x\t", inode->di_inostamp);
	printf("[19] di_mtime.tv_nsec:\t0x%08x\n", inode->di_mtime.tv_nsec);
	printf("[2] di_fileset:\t\t%d\t\t", inode->di_fileset);
	printf("[20] di_otime.tv_sec:\t0x%08x\n", inode->di_otime.tv_sec);
	printf("[3] di_number:\t\t%d\t\t", inode->di_number);
	printf("[21] di_otime.tv_nsec:\t0x%08x\n", inode->di_otime.tv_nsec);
	printf("[4] di_gen:\t\t%d\t\t", inode->di_gen);
	printf("[22] di_acl.flag:\t0x%02x\n", inode->di_acl.flag);
	printf("[5] di_ixpxd.len:\t%d\t\t", inode->di_ixpxd.len);
	printf("[23] di_acl.rsrvd:\tNot Displayed\n");
	printf("[6] di_ixpxd.addr1:\t0x%02x\t\t", inode->di_ixpxd.addr1);
	printf("[24] di_acl.size:\t0x%08x\n", inode->di_acl.size);
	printf("[7] di_ixpxd.addr2:\t0x%08x\t", inode->di_ixpxd.addr2);
	printf("[25] di_acl.len:\t%d\n", inode->di_acl.len);
	printf("     di_ixpxd.address:\t%lld\t\t",
		addressPXD( &(inode->di_ixpxd)));
	printf("[26] di_acl.addr1:\t0x%02x\n", inode->di_acl.addr1);
	printf("[8] di_size:\t0x%016llx\t", inode->di_size);
	printf("[27] di_acl.addr2:\t0x%08x\n", inode->di_acl.addr2);
	printf("[9] di_nblocks:\t0x%016llx\t", inode->di_nblocks);
	printf("     di_acl.address:\t%lld\n", addressDXD(&inode->di_acl));
	printf("[10] di_nlink:\t\t%d\t\t", inode->di_nlink);
	printf("[28] di_ea.flag:\t0x%02x\n", inode->di_ea.flag);
	printf("[11] di_uid:\t\t%d\t\t", inode->di_uid);
	printf("[29] di_ea.rsrvd:\tNot Displayed\n");
	printf("[12] di_gid:\t\t%d\t\t", inode->di_gid);
	printf("[30] di_ea.size:\t0x%08x\n", inode->di_ea.size);
	printf("[13] di_mode:\t\t0x%08x\t", inode->di_mode);
	printf("[31] di_ea.len:\t\t%d\n", inode->di_ea.len);
#ifdef _JFS_OS2
	printf("\t\t%07o\t%4s\t\t",
#else
	printf("\t\t%07o\t%10s\t",
#endif
		inode->di_mode & 0177777, mode_string(inode->di_mode));
	printf("[32] di_ea.addr1:\t0x%02x\n", inode->di_ea.addr1);
	printf("[14] di_atime.tv_sec:\t0x%08x\t", inode->di_atime.tv_sec);
	printf("[33] di_ea.addr2:\t0x%08x\n", inode->di_ea.addr2);
	printf("[15] di_atime.tv_nsec:\t0x%08x\t", inode->di_atime.tv_nsec);
	printf("     di_ea.address:\t%lld\n", addressDXD(&inode->di_ea));
	printf("[16] di_ctime.tv_sec:\t0x%08x\t", inode->di_ctime.tv_sec);
	printf("[34] di_compress:\t%d\n", inode->di_compress);
	printf("[17] di_ctime.tv_nsec:\t0x%08x\t", inode->di_ctime.tv_nsec);
	printf("[35] di_acltype:\t0x%08x\n", inode->di_acltype);
	printf("[18] di_mtime.tv_sec:\t0x%08x\n", inode->di_mtime.tv_sec);
}

int32 change_inode(
struct dinode *inode)
{
	char	cmdline[512];
	int32	field;
	char	*token;

retry:
	fputs("change_inode: [m]odify or e[x]it > ", stdout);
	gets(cmdline);
	token = strtok(cmdline, " 	");
	if (token == 0 || token[0] != 'm')
		return XPEEK_OK;
	field = m_parse(cmdline, 35, &token);
	if (field == 0)
		goto retry;

	switch (field)
	{
	case 1:
		inode->di_inostamp = strtoul(token, 0, 16);
		break;
	case 2:
		inode->di_fileset = strtol(token, 0, 0);
		break;
	case 3:
		inode->di_number = strtol(token, 0, 0);
		break;
	case 4:
		inode->di_gen = strtol(token, 0, 0);
		break;
	case 5:
		inode->di_ixpxd.len = strtoul(token, 0, 0);
		break;
	case 6:
		inode->di_ixpxd.addr1 = strtoul(token, 0, 0);
		break;
	case 7:
		inode->di_ixpxd.addr2 = strtoul(token, 0, 0);
		break;
	case 8:
		inode->di_size = strtoull(token, 0, 16);
		break;
	case 9:
		inode->di_nblocks = strtoull(token, 0, 16);
		break;
	case 10:
		inode->di_nlink = strtol(token, 0, 0);
		break;
	case 11:
		inode->di_uid = strtol(token, 0, 0);
		break;
	case 12:
		inode->di_gid = strtol(token, 0, 0);
		break;
	case 13:
		inode->di_mode = strtoul(token, 0, 16);
		break;
	case 14:
		inode->di_atime.tv_sec = strtoul(token, 0, 16);
		break;
	case 15:
		inode->di_atime.tv_nsec = strtoul(token, 0, 16);
		break;
	case 16:
		inode->di_ctime.tv_sec = strtoul(token, 0, 16);
		break;
	case 17:
		inode->di_ctime.tv_nsec = strtoul(token, 0, 16);
		break;
	case 18:
		inode->di_mtime.tv_sec = strtoul(token, 0, 16);
		break;
	case 19:
		inode->di_mtime.tv_nsec = strtoul(token, 0, 16);
		break;
	case 20:
		inode->di_otime.tv_sec = strtoul(token, 0, 16);
		break;
	case 21:
		inode->di_otime.tv_nsec = strtoul(token, 0, 16);
		break;
	case 22:
		inode->di_acl.flag = strtol(token, 0, 16);
		break;
	case 23:
	case 29:
		fputs("change_inode: Can't change this field\n", stderr);
		goto retry;
	case 24:
		inode->di_acl.size = strtol(token, 0, 16);
		break;
	case 25:
		inode->di_acl.len = strtol(token, 0, 0);
		break;
	case 26:
		inode->di_acl.addr1 = strtol(token, 0, 16);
		break;
	case 27:
		inode->di_acl.addr2 = strtol(token, 0, 16);
		break;
	case 28:
		inode->di_ea.flag = strtol(token, 0, 16);
		break;
	case 30:
		inode->di_ea.size = strtol(token, 0, 16);
		break;
	case 31:
		inode->di_ea.len = strtol(token, 0, 0);
		break;
	case 32:
		inode->di_ea.addr1 = strtol(token, 0, 16);
		break;
	case 33:
		inode->di_ea.addr2 = strtol(token, 0, 16);
		break;
	case 34:
		inode->di_compress = strtol(token, 0, 0);
		break;
	case 35:
		inode->di_acltype = strtol(token, 0, 16);
		break;
	}
	return XPEEK_CHANGED;
}

#ifdef _JFS_OS2
char *mode_string(
mode_t mode)
{
	int	type;
static	char	mode_str[5];

	strcpy(mode_str, "----");

	type = mode & IFMT;
	switch (type)
	{
	case IFREG:
		break;
	case IFDIR:
		mode_str[0] = 'd';
		break;
	default:
		mode_str[0] = '?';
		break;
	}
	if (mode & IREAD)
		mode_str[1] = 'r';
	if (mode & IWRITE)
		mode_str[2] = 'w';
	if (mode & IEXEC) {
		if (mode & ISUID & ISGID)
			mode_str[3] = 's';
		else if (mode & ISUID)
			mode_str[3] = 'u';
		else if (mode & ISGID)
			mode_str[3] = 'g';
		else
			mode_str[3] = 'x';
	}
	else {
		if (mode & ISUID & ISGID)
			mode_str[3] = 'S';
		else if (mode & ISUID)
			mode_str[3] = 'U';
		else if (mode & ISGID)
			mode_str[3] = 'G';
	}
	return mode_str;
}

#else	/* not _JFS_OS2 */
char *mode_string(
mode_t mode)
{
	int	type;
static	char	mode_str[11];

	strcpy(mode_str, "----------");

	type = mode & S_IFMT;
	switch (type)
	{
	case S_IFREG:
		break;
	case S_IFDIR:
		mode_str[0] = 'd';
		break;
	case S_IFBLK:
		mode_str[0] = 'b';
		break;
	case S_IFCHR:
		mode_str[0] = 'c';
		break;
	case S_IFIFO:
		mode_str[0] = 'p';
		break;
	case S_IFSOCK:
		mode_str[0] = 's';
		break;
	case S_IFLNK:
		mode_str[0] = 'l';
		break;
	default:
		mode_str[0] = '?';
		break;
	}
	if (mode & S_IRUSR)
		mode_str[1] = 'r';
	if (mode & S_IWUSR)
		mode_str[2] = 'w';
	if (mode & S_IXUSR) {
		if (mode & S_ISUID)
			mode_str[3] = 's';
		else
			mode_str[3] = 'x';
	}
	else {
		if (mode & S_ISUID)
			mode_str[3] = 'S';
	}
	if (mode & S_IRGRP)
		mode_str[4] = 'r';
	if (mode & S_IWGRP)
		mode_str[5] = 'w';
	if (mode & S_IXGRP) {
		if (mode & S_ISGID)
			mode_str[6] = 's';
		else
			mode_str[6] = 'x';
	}
	else {
		if (mode & S_ISGID)
			mode_str[6] = 'S';
	}
	if (mode & S_IROTH)
		mode_str[7] = 'r';
	if (mode & S_IWOTH)
		mode_str[8] = 'w';
	if (mode & S_IXOTH)
		mode_str[9] = 'x';

	return mode_str;
}
#endif	/* _JFS_OS2 */

int32 find_inode(
ino_t inum,
ino_t which_table,
int64 *address)
{
	int32		extnum;
	iag_t		iag;
	int32		iagnum;
	int64		map_address;
	int32		rc;

	iagnum = INOTOIAG(inum);
	extnum = (inum & (INOSPERIAG -1)) >> L2INOSPEREXT;
	if (find_iag(iagnum, which_table, &map_address) == 1)
		return 1;

	rc = ujfs_rw_diskblocks(fd, map_address, sizeof(iag_t),
				&iag, GET);
	if (rc) {
		fputs("find_inode: Error reading iag\n", stderr);
		return 1;
	}
	if (iag.inoext[extnum].len == 0)
		return 1;

	*address = (addressXD(&iag.inoext[extnum]) << l2bsize) +
	           ((inum & (INOSPEREXT-1)) * sizeof(dinode_t));
	return 0;
}
