/* $Id: iag.c,v 1.1.1.1 2003/05/21 13:43:10 pasha Exp $ */

static char *SCCSID = "@(#)1.6  7/31/98 08:22:25 src/jfs/utils/xpeek/iag.c, jfsutil, w45.fs32, 990417.1";
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
 *   MODULE_NAME:	iag.c
 *
 *   FUNCTION: 		display/modify iag
 *
 */

#include "xpeek.h"
#include <ctype.h>

/* JFS includes */
#include <jfs_filsys.h>

extern int64	AIT_2nd_offset;	/* Defined and assigned in xpeek.c	*/

int32 change_iag(iag_t *);
int32 display_ext(pxd_t *, char *);
int32 display_map(uint32 *, int32);

void iag()
{
	int64	address;
	int64	block_num;
	char	cmd_line[80];
	iag_t	iag;
	int32	iagnum;
	uint32	offset;
	int32	rc;
	char	*token;
	ino_t	which_table = FILESYSTEM_I;

	token = strtok(0, " 	");
	if (token == 0) {
		fputs("iag: Please enter: iagnum [ a | 0 ]\niag> ",stdout);
		gets(cmd_line);
		token=strtok(cmd_line, " 	");
		if (token == 0)
			return;
	}
	errno = 0;
	iagnum = strtoul(token, 0, 0);
	token = strtok(0, " 	");
	if (token) {
		if (token[0] == 'a')
			which_table = AGGREGATE_I;
		else if (token[0] == 's')
			which_table = AGGREGATE_2ND_I;
		else if (token[0] != '0') {
			fputs("iag: invalide fileset\n\n", stderr);
			return;
		}
	}
	if (strtok(0, " 	")) {
		fputs("iag: Too many arguments\n\n", stderr);
		return;
	}

	if (find_iag(iagnum, which_table, &address) ||
		xRead(address, sizeof(iag_t), (char *)&iag))
	{
		fputs("iag: error reading iag\n\n", stderr);
		return;
	}

	block_num = address >> l2bsize;

changed:
	printf("IAG %d at block %lld\n\n", iagnum, block_num);
	display_iag(&iag);
	if ((rc = change_iag(&iag)) == XPEEK_ERROR)
		return;
	if (rc & XPEEK_CHANGED)
		if (xWrite(address, sizeof(iag_t), (char *)&iag)) {
			fputs("iag: error writing iag\n\n", stderr);
			return;
		}
	if (rc & XPEEK_REDISPLAY)
		goto changed;
	return;
}

/****************************************************************************
 ************************  Sample output of display_iag()

IAG 0 at block 440

[1] agstart:		25		[12] extsmap[0]:	ffffffff
[2] iagnum:		0		[13] extsmap[1]:	ffffffff
[3] inofreefwd:		-1		[14] extsmap[2]:	ffffe000
[4] inofreeback:	-1		[15] extsmap[3]:	00000000
[5] extfreefwd:		-1		[16] nfreeinos:		284
[6] extfreeback:	-1		[17] nfreeexts:		45
[7] iagfree:		-1		[18] pad:		Not Displayed
[8] inosmap[0]:		ffffffff	[19] wmap:		Type 'w'
[9] inosmap[1]:		ffffffff	[20] pmap:		Type 'p'
[10] inosmap[2]:	ff801fff	[21] inoext:		Type 'i'
[11] inosmap[3]:	ffffffff

 ****************************************************************************/

void display_iag(
iag_t *iag)
{
	printf("[1] agstart:\t\t%lld\t\t", iag->agstart);
	printf("[12] extsmap[0]:\t%08x\n", iag->extsmap[0]);
	printf("[2] iagnum:\t\t%d\t\t", iag->iagnum);
	printf("[13] extsmap[1]:\t%08x\n", iag->extsmap[1]);
	printf("[3] inofreefwd:\t\t%d\t\t", iag->inofreefwd);
	printf("[14] extsmap[2]:\t%08x\n", iag->extsmap[2]);
	printf("[4] inofreeback:\t%d\t\t", iag->inofreeback);
	printf("[15] extsmap[3]:\t%08x\n", iag->extsmap[3]);
	printf("[5] extfreefwd:\t\t%d\t\t", iag->extfreefwd);
	printf("[16] nfreeinos:\t\t%d\n", iag->nfreeinos);
	printf("[6] extfreeback:\t%d\t\t", iag->extfreeback);
	printf("[17] nfreeexts:\t\t%d\n", iag->nfreeexts);
	printf("[7] iagfree:\t\t%d\t\t", iag->iagfree);
	printf("[18] pad:\t\tNot Displayed\n");
	printf("[8] inosmap[0]:\t\t%08x\t", iag->inosmap[0]);
	printf("[19] wmap:\t\tType 'w'\n");
	printf("[9] inosmap[1]:\t\t%08x\t", iag->inosmap[1]);
	printf("[20] pmap:\t\tType 'p'\n");
	printf("[10] inosmap[2]:\t%08x\t", iag->inosmap[2]);
	printf("[21] inoext:\t\tType 'i'\n");
	printf("[11] inosmap[3]:\t%08x\n", iag->inosmap[3]);
}

int32 change_iag(
iag_t *iag)
{
	char	cmdline[80];
	int32	field;
	char	*token;

retry:
	fputs("change_iag: [m]odify, [w]map, [p]map, [i]noext or e[x]it > ",
		stdout);
	gets(cmdline);
	token = strtok(cmdline, " 	");
	if (token == 0 || token[0] == 'x')
		return XPEEK_OK;
	if (token[0] == 'w')
		return display_map(iag->wmap, EXTSPERIAG);
	if (token[0] == 'p')
		return display_map(iag->pmap, EXTSPERIAG);
	if (token[0] == 'i') {
		return display_ext(iag->inoext, cmdline);
	}
	field = m_parse(cmdline, 17, &token);
	if (field == 0)
		goto retry;

	switch (field)
	{
	case 1:
		iag->agstart = strtoll(token, 0, 0);
		break;
	case 2:
		iag->iagnum = strtol(token, 0, 0);
		break;
	case 3:
		iag->inofreefwd = strtol(token, 0, 0);
		break;
	case 4:
		iag->inofreeback = strtol(token, 0, 0);
		break;
	case 5:
		iag->extfreefwd = strtol(token, 0, 0);
		break;
	case 6:
		iag->extfreeback = strtol(token, 0, 0);
		break;
	case 7:
		iag->iagfree = strtol(token, 0, 0);
		break;
	case 8:
		iag->inosmap[0] = strtoul(token, 0, 16);
		break;
	case 9:
		iag->inosmap[1] = strtoul(token, 0, 16);
		break;
	case 10:
		iag->inosmap[2] = strtoul(token, 0, 16);
		break;
	case 11:
		iag->inosmap[3] = strtoul(token, 0, 16);
		break;
	case 12:
		iag->extsmap[0] = strtoul(token, 0, 16);
		break;
	case 13:
		iag->extsmap[1] = strtoul(token, 0, 16);
		break;
	case 14:
		iag->extsmap[2] = strtoul(token, 0, 16);
		break;
	case 15:
		iag->extsmap[3] = strtoul(token, 0, 16);
		break;
	case 16:
		iag->nfreeinos = strtol(token, 0, 0);
		break;
	case 17:
		iag->nfreeexts = strtol(token, 0, 0);
		break;
	}
	return XPEEK_CHANGED | XPEEK_REDISPLAY;
}

#define XT_CMP(CMP, K, X) \
{ \
	int64 offset64 = offsetXAD(X); \
	(CMP) = ((K) >= offset64 + lengthXAD(X)) ? 1 : \
		((K) < offset64) ? -1 : 0 ; \
}

int32 find_iag(
ino_t iagnum,
ino_t which_table,
int64 *address)
{
	int32		base;
	char		buffer[PSIZE];
	int32		cmp;
	dinode_t	fileset_inode;
	int64		fileset_inode_address;
	iag_t		iag;
	int64		iagblock;
	int32		index;
	int32		lim;
	xtpage_t	*page;
	int32		rc;

	if (which_table != FILESYSTEM_I && 
	    which_table != AGGREGATE_I && 
	     which_table != AGGREGATE_2ND_I) {
		fprintf(stderr, "find_iag: Invalid fileset, %d\n",
			which_table);
		return 1;
	}
	iagblock = IAGTOLBLK(iagnum, L2PSIZE-l2bsize);

	if( which_table == AGGREGATE_2ND_I ) {
		fileset_inode_address = AIT_2nd_offset + sizeof(dinode_t);
		}
	else {
		fileset_inode_address = AGGR_INODE_TABLE_START +
			    (which_table * sizeof(dinode_t));
		}
	rc = xRead(fileset_inode_address, sizeof(dinode_t),
		   (char *)&fileset_inode);
	if (rc) {
		fputs("find_inode: Error reading fileset inode\n", stderr);
		return 1;
	}
	page = (xtpage_t *)&(fileset_inode.di_btroot);

descend:
	/* Binary search */
	for (base = XTENTRYSTART, lim = page->header.nextindex - XTENTRYSTART;
		lim; lim >>=1) {
		index = base + (lim >> 1);
		XT_CMP(cmp, iagblock, &(page->xad[index]));
		if (cmp == 0) {
			/* HIT! */
			if (page->header.flag & BT_LEAF) {
				*address = (addressXAD(&(page->xad[index]))
					    + (iagblock -
					       offsetXAD(&(page->xad[index]))))
					    << l2bsize;
				return 0;
			}
			else {
				rc = xRead(
				    addressXAD(&(page->xad[index])) << l2bsize,
				    PSIZE, buffer);
				if (rc) {
					fputs("find_iag: Error reading btree node\n", stderr);
					return 1;
				}
				page = (xtpage_t *)buffer;
				goto descend;
			}
		}
		else if (cmp > 0) {
			base = index + 1;
			--lim;
		}
	}

	if (page->header.flag & BT_INTERNAL ) 
	{
		/* Traverse internal page, it might hit down there
		 * If base is non-zero, decrement base by one to get the parent
		 * entry of the child page to search.
		 */
		index = base ? base - 1 : base;

		rc = xRead(addressXAD(&(page->xad[index])) << l2bsize, PSIZE,
			   buffer);
		if (rc) {
			fputs("find_iag: Error reading btree node\n", stderr);
			return 1;
		}
		page = (xtpage_t *)buffer;
		goto descend;
	}

	/* Not found! */
	fprintf(stderr, "find_iag:  IAG %d not found!\n", iagnum);
	return 1;
}

/* display_map is also called from display_page in dmap.c */

int32	display_map(
uint32	*map,
int32	size)
{
	char	cmdline[80];
	int32	end;
	int32	i;
	int32	index;
	int32	rc = XPEEK_OK;
	int32	start = 0;
	char	*token;

map_display:
	end = MIN(start+128, size);
	for (i = start; i < end; i+=8) {
		if ((i+7) < 100)	/* [i-(i+7)] fits in first field */
			printf("[%d-%d]\t", i, i+7);
		else
			printf("[%d- ]\t", i);

		printf("%08x %08x %08x %08x %08x %08x %08x %08x\n", map[i],
			map[i+1],map[i+2],map[i+3],map[i+4],map[i+5],map[i+6],
			map[i+7]);
	}
map_retry:
	fputs("display_map: [m]odify, [b]ack, e[x]it\n", stdout);
	gets(cmdline);
	token = strtok(cmdline, " 	");
	if (token == 0) {
		start = (size > end)? end : 0;
		goto map_display;
	}
	if (token[0] == 'x')
		return rc;
	if (token[0] != 'm') {			/* assuming 'b' */
		return (rc | XPEEK_REDISPLAY);
	}

	index = m_parse(cmdline, size-1, &token);
	if (index == 0)
		goto map_retry;

	map[index] = strtoul(token, 0, 16);
	rc = XPEEK_CHANGED;
	goto map_display;
}

int32 display_ext(
pxd_t *ext,
char *cmdline)
{
	int32	field;
	int32	index;
	int32	rc = XPEEK_OK;
	char	*token;

	token = strtok(0, " 	");
	if (token == 0) {
ext_retry:
		fputs("Please enter: index [0-127] > ",stdout);
		gets(cmdline);
		token = strtok(cmdline, " 	");
		if (token == 0)
			return rc | XPEEK_REDISPLAY;
	}
newext:
	index = strtol(token, 0, 0);
	if (index < 0 || index >= EXTSPERIAG) {
		fputs("Invalid index\n", stderr);
		goto ext_retry;
	}
ext_changed:
	printf("[1] inoext[%d].len:\t%d\n", index, ext[index].len);
	printf("[2] inoext[%d].addr1:\t0x%02x\n", index, ext[index].addr1);
	printf("[3] inoext[%d].addr2:\t0x%08x\n", index, ext[index].addr2);
	printf("    addressPXD:\t\t%lld\n", addressPXD(&ext[index]));
ext_again:
	fputs("display_ext: [m]odify, ext [#], [b]ack to iag, e[x]it > ",
		stdout);
	gets(cmdline);
	token = strtok(cmdline, " 	");
	if (token == 0 || token[0] == 'x')
		return rc;
	if (isdigit(token[0]))
		goto newext;
	if (token[0] != 'm')	/* assuming 'b' */
		return rc | XPEEK_REDISPLAY;

	field = m_parse(cmdline, 3, &token);
	if (field == 0)
		goto ext_again;

	switch (field)
	{
	case 1:
		ext[index].len = strtol(token,0,0);
		break;
	case 2:
		ext[index].addr1 = strtol(token,0,16);
		break;
	case 3:
		ext[index].addr2 = strtol(token,0,16);
		break;
	}
	rc = XPEEK_CHANGED;
	goto ext_changed;
}
