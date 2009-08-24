/* $Id: utilsubs.c,v 1.1.1.1 2003/05/21 13:42:26 pasha Exp $ */

static char *SCCSID = "@(#)1.3  7/31/98 08:20:22 src/jfs/utils/libfs/utilsubs.c, jfslib, w45.fs32, 990417.1";
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
 *	utilsubs.c: general utility subroutines
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jfs_types.h"

#include "utilsubs.h"


/*
 *	countLZ32()
 */
void countLZ32(int32 word, int32 *lzero)
{
  register int32 i;
  register uint32 j;

  for (i = 0, j = 0x80000000; !(word & j) && i < 32; i++, j>>=1) ;

  *lzero = i;

  return;
}


/*
 *	countTZ32()
 */
void countTZ32(int32 word, int32 *tzero)
{
  register int32 i;
  register uint32 j;

  for (i = 0, j = 0x00000001; !(word & j) && i < 32; i++, j<<=1) ;

  *tzero = i;

  return;
}


/*
 *	log2shift()
 */
int32 log2shift(uint32	n)
{
	register uint32		shift = 0;

	while (n > 1)
	{
		/* n is not power of 2 */
		if (n & 1)
			return -1;

		shift++;
		n >>= 1;
	}

	return shift;
}


/*
 *	ui
 *	==
 */
/*
 *	prompt()
 */
char prompt(
	char	*str)
{
	char cmd[81];

	fputs(str, stdout);
	fflush(stdout);

	/* get NULL terminated input (exclude \n) */
	gets(cmd);

	return cmd[0]; /* return response letter */
}


/*
 *	more()
 */
int32 more(void)
{
	char cmd[81];

	fputs("- hit Enter to continue, e[x]it -", stdout);
	fflush(stdout);

	/* get NULL terminated input (exclude \n) */
	gets(cmd);

	if (!cmd[0])
		return 0; /* do NOT continue */	
	else
		return 1; /* continue */
}
