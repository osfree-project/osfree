/* $Id: display.c,v 1.4 2004/03/21 02:43:33 pasha Exp $ */

static char *SCCSID = "@(#)1.5  7/31/98 08:22:02 src/jfs/utils/xpeek/display.c, jfsutil, w45.fs32, 990417.1";
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
/* display
	Displays data in a variety of formats
*/

#include "xpeek.h"

/* libfs includes */
#include <devices.h>

void display_hex(char*, uint32, uint32);
/* void display_ixad(ixad_t *, uint32, uint32); */

void display()
{
	int64	addr;
	int64	block;
	char	*buffer;
	char	cmdline[512];
	uint32	data_size;
	int	format = 'a';
	int64	len;
	uint32	length = 0;
	uint32	offset = 0;
	char	*token;

	token = strtok(0, " 	");
	if (token == 0) {
		fputs("display: Please enter: block [offset [format [count]]]\ndisplay> ",stdout);
		gets(cmdline);
		token = strtok(cmdline, " 	");
		if (token == 0)
			return;
	}
	errno = 0;
	block = strtoull(token, 0, 0);
	if (block == 0 && errno) {
		fputs("display: invalid block\n\n", stderr);
		return;
	}
	if ((token = strtok(0, " 	")) != 0) {
		offset = strtoul(token, 0, 16);
		if (offset == 0 && errno) {
			fputs("display: invalid offset\n\n", stderr);
			return;
		}
	}
	if ((token = strtok(0, " 	")) != 0)
		format = token[0];

	if ((token = strtok(0, " 	")) != 0) {
		length = strtoul(token, 0, 0);
		if (length == 0 && errno) {
			fputs("display: invalid length\n\n", stderr);
			return;
		}
	}

	if (strtok(0, " 	")) {
		fputs("display: Too many arguments\n\n", stderr);
		return;
	}

	switch (format)
	{
	case 'a':
		data_size=1;
		if (length == 0)
			length = bsize;
		break;
	case 'i':
		data_size=sizeof(struct dinode);
		if (length == 0)
			length = 1;
		break;
	case 'I':
		data_size=sizeof(iag_t);
		if (length == 0)
			length = 1;
		break;
	case 's':
		data_size=sizeof(struct superblock);
		if (length == 0)
			length = 1;
		break;
	case 'x':
		data_size=4;
		if (length == 0)
			length = bsize/4;
		break;
	case 'X':
		data_size=sizeof(xad_t);
		if (length == 0)
			length = 1;
		break;
	default:
		fputs("display:  invalid format\n\n", stderr);
		return;
	}

	addr = block << l2bsize;
	len = ((length*data_size)+offset+bsize-1)&(~(bsize-1));
	buffer = malloc(len);
	if (buffer == 0) {
		fputs("display: error calling malloc\n\n", stderr);
		return;
	}

	if (ujfs_rw_diskblocks(fd, addr, len, buffer, GET)) {
		fputs("display: ujfs_rw_diskblocks failed\n\n", stderr);
		free(buffer);
		return;
	}

	printf("Block: %lld     Real Address 0x%llx\n", block, addr);
	switch (format)
	{
	case 'a':
	case 'x':
		display_hex(&buffer[offset], length, offset);
		break;
	case 'i':
		{
			int i;
			struct dinode *inode = (struct dinode*)&buffer[offset];
			for (i=0; i < length; i++, inode++) {
				display_inode(inode);
				if (more())
					return;
			}
		}
		break;
	case 'I':
		{
			int i;
			iag_t *iag = (iag_t *)&buffer[offset];
			for (i=0; i < length; i++, iag++) {
				display_iag(iag);
				if (more())
					return;
			}
		}
		break;

	case 's':
		if (display_super((struct superblock *)&buffer[offset]) ==
		    XPEEK_CHANGED)
			if (ujfs_rw_diskblocks(fd, addr, len, buffer, PUT))
				fputs("Display:  Error writing superblock!\n",
					stderr);
		
		break;
	default:
		fputs("display:  specified format not yet supported\n\n", stderr);
		break;
	}

	free (buffer);
	return;
}

/*
 *	display_hex: display region in hex/ascii
 */
static void display_hex(
	char	*addr,
	uint32	length,
	uint32	offset)
{
	uint8	hextext[37];
  	uint8	asciitxt[17];
	uint8	*x = (uint8 *)addr, x1, x2;
	int32 	i, j, k, l;
	int32	c;

	hextext[36] = '\0';
    	asciitxt[16] = '\0';	/* null end of string */

	l = 0;

	for (i=1; i<=((length+15)/16); i++) {
		if (i>1 && ((i-1)%16) == 0)
			if (more())
				break;

		/* print address/offset */
		printf("%08x: ", offset+l);

		/* print 16 bytes per line */
	    	for (j=0, k = 0; j<16; j++, x++, l++) {
			if ((j%4) == 0)
				hextext[k++] = ' ';
			if (l < length) {
				hextext[k++] = ((x1 = ((*x & 0xf0) >> 4)) < 10)
						? ('0' + x1) : ('A' + x1 - 10);
				hextext[k++] = ((x2 = (*x & 0x0f)) < 10)
						? ('0' + x2) : ('A' + x2 - 10);
				asciitxt[j] = ((*x < 0x20) ||
					(*x >= 0x7f)) ? '.' : *x;
	      		} else  {  /* byte not in range */
				hextext[k++] = ' ';
				hextext[k++] = ' ';
				asciitxt[j] = '.';
			}
	    	}
	  	printf("%s   |%s|\n", hextext, asciitxt);
	}
}

#if 0
static void display_ixad(
ixad_t	*ptr,
uint32	count,
uint32	offset)
{
	int i;

	for (i = 0; i < count; i++, ptr++) {
		if (i && (i%2 == 0))
			if (more())
				return;
		printf("\nixad_t at offset 0x%04x\n", offset);
		printf("[1] next:\t0x%08x\t\t", ptr->next);
		printf("[8] wmap:\t0x%08x\n", ptr->wmap);
		printf("[2] prev:\t0x%08x\t\t", ptr->prev);
		printf("[9] pmap:\t0x%08x\n", ptr->pmap);
		printf("[3] ixd.flag:\t0x%02x\t\t\t", ptr->ixd.flag);
		printf("[10] rsrvd:\tNot Displayed\n");
		printf("[4] ixd.rsrvd:\tNot Displayed\n");
		printf("[5] ixd.len:\t%d\n", ptr->ixd.len);
		printf("[6] ixd.addr1:\t0x%02x\n", ptr->ixd.addr1);
		printf("[7] ixd.addr2:\t0x%08x\t\taddr = %lld\n", ptr->ixd.addr2,
			addressXD(&(ptr->ixd)));
		offset += sizeof(ixad_t);
	}
}
#endif
