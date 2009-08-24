/* $Id: fsckea.c,v 1.1.1.1 2003/05/21 13:39:18 pasha Exp $ */

static char *SCCSID = "@(#)1.5  7/30/97 10:53:36 src/jfs/utils/chkdsk/fsckea.c, jfschk, w45.fs32, 990417.1";
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
 *   COMPONENT_NAME: jfschk
 *
 *   FUNCTIONS:
 *              jfs_ValidateFEAList
 *
 *   NOTES: 
 *       This module was created by copying jfs_ValidateFEAList from
 *       jfs_ea.c 
 *
 *       Any changes to this code in jfs_ea.c require corresponding 
 *       changes here.
 *
 */

#define INCL_DOSERRORS
/*                            #define INCL_NOPMAPI           */
#define INCL_DOSPROCESS

#define _cdecl __cdecl

#include "xfsck.h"

/* some macros for dealing with variable length EA lists.
 */

#define FEA_SIZE(ea) ((ea)->cbValue + (ea)->cbName + 1 + sizeof (FEA))
#define	NEXT_FEA(ea) ((FEA*)(((char *) (ea)) + (FEA_SIZE (ea))))

#define GEA_SIZE(ea) ((ea)->cbName + sizeof (GEA))
#define	NEXT_GEA(ea) ((GEA*)(((char *) (ea)) + (GEA_SIZE (ea))))

/* an extended attribute consists of a <name,value> double with <name>
 * being restricted to a subset of the 8 bit ASCII character set.  this
 * table both defines valid characters for <name> and provides a lower-case
 * to upper-case mapping.
 */

#define CH_BAD_EA	'\0'

/* 
 * jfs_ValidateFEAList -- validate structure of an FEALIST
 */

int
jfs_ValidateFEAList (
	FEALIST		*pfeal,
	ULONG		*poError)
{
    USHORT	cbNeeded = 0;	/* count of storage bytes needed */
    USHORT	cbLeft;		/* count of bytes left in FEA list */
    PFEA	pfea = pfeal->list; /* pointer to current FEA */
    USHORT	cbFEA;		/* count of bytes in current FEA */

    if ((cbLeft = (USHORT) pfeal->cbList - sizeof (pfeal->cbList)) == 0)
	return 0;

    do {
	/* check for our reserved bits
	 */
	if (pfea->fEA & ~(FEA_NEEDEA) || cbLeft < sizeof *pfea)
	    return ERROR_EA_LIST_INCONSISTENT;

	cbFEA = FEA_SIZE (pfea);
	pfea = NEXT_FEA (pfea);

	if (cbLeft < cbFEA)
	{
	    *poError = (((char *) pfea) - ((char *) pfeal));
	    return ERROR_EA_LIST_INCONSISTENT;
	}

    } while ((cbLeft -= cbFEA) > 0);

    return 0;
}

