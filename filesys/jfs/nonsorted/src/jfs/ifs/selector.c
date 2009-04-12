/* $Id: selector.c,v 1.1 2000/04/21 10:58:19 ktk Exp $ */

static char *SCCSID = "@(#)1.6  7/30/98 14:22:29 src/jfs/ifs/selector.c, sysjfs, w45.fs32, 990417.1";
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
*/

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"

#include <jfs_debug.h>
void printf(char *, ...);

typedef unsigned char uchar_t;
typedef unsigned short ushort_t;
typedef unsigned long ulong_t;

#include <seldesc.h>

#define MAX_SELECTORS 50	/* Size of our selector table */

ulong_t num_selectors = 0;

ulong_t GDT;

struct {
	ushort_t	selector;
	ushort_t	pad;
	ulong_t		start_address;
	ulong_t		end_address;
	ulong_t		accessed;
//	ulong_t		length;		/* Not used, but makes table readable */
}	selector_table[MAX_SELECTORS];

void DefineSelector(
ushort_t	selector,
ulong_t		pgdt)		/* Only used on first call */
{
	long			base;
	struct desctab *	entry;
	long			gran;
	long			length;
/*
	printf("DefineSelector: selector = %x\n", (int)selector);
*/
	if (selector & 0x04)		/* LDT */
		brkpoint();

	if (num_selectors == 0)
		GDT = pgdt;
	else if (num_selectors == (MAX_SELECTORS - 1))
		brkpoint();

	entry = (struct desctab *) (GDT + (selector & 0xFFF8));

/* Sanity checks */
	if ((entry->d_attr & D_DBIG) == 1)	/* 32-bit addresses */
		panic("Define_Selector: 32-bit selector\n");
	if ((entry->d_access & D_PRES) == 0)	/* Not Present */
		panic("Define_Selector: Not Present\n");
	if ((entry->d_access & D_SEG) == 0)	/* Not Memory Segment */
		panic("Define_Selector: Not Memory Segment\n");

	selector_table[num_selectors].selector = selector;
	base = entry->d_extaddr << 24 | entry->d_hiaddr << 16 |
	       entry->d_loaddr;
	selector_table[num_selectors].start_address = base;
	gran = (entry->d_attr & D_GRAN4K) ? 4096 : 1;
	length = (((entry->d_attr && D_EXTLIMIT) << 16) | entry->d_limit) *
		 ((entry->d_attr & D_GRAN4K) ? 4096 : 1);
	if (length > 0x10000)
		panic("Define_Selector: Length > 64K\n");
//	selector_table[num_selectors].length = length;
	selector_table[num_selectors].end_address = base + length - 1;
	selector_table[num_selectors].accessed = 0;

	num_selectors++;
	return;
}

ulong_t FlatToSel(
ulong_t address)
{
	ulong_t	i;
	ulong_t offset;
	for (i = 0; i < num_selectors; i++) {
		if (address >= selector_table[i].start_address &&
		    address <= selector_table[i].end_address)
			goto found;
	}
	/* Not found */
	printf("DosFlatToSel: Could not map address 0x%8x to selector.\n",
		address);
	printf("Selector Table:\n");
	for (i = 0; i < num_selectors; i++)
		printf(" %04x  0x%8x  0x%8x  0x%4x\n",
			selector_table[i].selector,
			selector_table[i].start_address,
			selector_table[i].end_address,
//			selector_table[i].length);
			selector_table[i].accessed);
	brkpoint();
found:
	offset = address - selector_table[i].start_address;
	selector_table[i].accessed = 1;
	return (selector_table[i].selector << 16) | offset;
}

void	ForgetSelector(
ulong_t	address)
{
	ulong_t	i;
	for (i = 0; i < num_selectors; i++) {
		if (address == selector_table[i].start_address) {
			/* Keep table compact.  If not last entry, move last
			   entry to this one, so we have no holes.	*/
			num_selectors--;
			if (i < num_selectors) {
				selector_table[i].selector =
				 selector_table[num_selectors].selector;
				selector_table[i].start_address =
				 selector_table[num_selectors].start_address;
				selector_table[i].end_address =
				 selector_table[num_selectors].end_address;
			}
			break;
		}
	}
	return;
}
