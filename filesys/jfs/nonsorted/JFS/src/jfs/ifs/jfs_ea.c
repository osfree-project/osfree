/* $Id: jfs_ea.c,v 1.1.1.1 2003/05/21 13:37:02 pasha Exp $ */

static char *SCCSID = "@(#)1.20  9/13/99 15:02:25 src/jfs/ifs/jfs_ea.c, sysjfs, w45.fs32, fixbld";
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
 *
*/

/*
 * Change History :
 *
 */

/*
 *	jfs_ea.c: extended attribute service
 *
 * Overall design --
 *
 * Format:
 *
 *   Extended attribute lists (FEALIST) consist of an overall size (32 bit
 *   value) and a variable (0 or more) number of extended attribute
 *   entries.  Each extended attribute entry (FEA) is a <name,value> double
 *   where <name> is constructed from a case-shifted
 *   subset of the 8 bit character set (1 ... 255 bytes in the name) and
 *   <value> is arbitrary 8 bit data (1 ... 65535 bytes).  The in-memory
 *   format is (per the FEA structure)
 *
 *   0       1        2        4                4 + cbName + 1
 *   +-------+--------+--------+----------------+-------------------+
 *   | Flags | Name   | Value  | Name String \0 | Data . . . .      |
 *   |       | Length | Length |                |                   |
 *   +-------+--------+--------+----------------+-------------------+
 *
 *   The end of the FEA structure is defined by the following expression
 *
 *	FEA_SIZE(fea) = sizeof (FEA) + fea->cbName + fea->cbValue + 1
 *
 *   where "fea" is a pointer to a FEA structure.
 *
 *   A FEALIST then is structured as
 *
 *   0            4                   4 + FEA_SIZE(fea1)
 *   +------------+-------------------+--------------------+-----
 *   | Overall EA | First FEA Element | Second FEA Element | ..... 
 *   | List Size  |                   |                    |
 *   +------------+-------------------+--------------------+-----
 *
 *   out to a total of cbList bytes total.
 *
 *   The address of each FEA, given the preceding FEA, is defined by
 *
 *	NEXT_FEA(fea) = (fea + FEA_SIZE(fea))
 *
 *   using byte addressing [ NB: casts are required!!! ]
 *
 *   The on-disk (and that includes those in-lined EAs) format of an EA list
 *   is identical to the format passed in by applications programs and
 *   returned back to applications programs.
 *
 *   A second structure, known as a GEALIST, is used to query FEALIST's
 *   and has a slightly different structure.  As with the FEALIST, a GEALIST
 *   consists of zero or more GEA elements.  Here is the in-memory layout of
 *   a GEA element.
 *
 *   0        1             2 + cbName
 *   +--------+-------------+
 *   | Name   | Name ... \0 |
 *   | Length |             |
 *   +--------+-------------+
 *
 *   The end of the GEA structure is defined by the following expression
 *
 *	GEA_SIZE(gea) = sizeof (GEA) + gea->cbName
 *
 *   where "gea" is a pointer to a GEA structure.
 *
 *   A GEALIST then is structured as
 *
 *   0            4                   4 + GEA_SIZE(fea1)
 *   +------------+-------------------+--------------------+-----
 *   | Overall EA | First GEA Element | Second GEA Element | ..... 
 *   | List Size  |                   |                    |
 *   +------------+-------------------+--------------------+-----
 *
 *   out to a total of cbList bytes total.
 *
 *   The address of each GEA, given the preceding GEA, is defined by
 *
 *	NEXT_GEA(gea) = (gea + GEA_SIZE(gea))
 *
 *   using byte addressing [ NB: casts are required!!! ]
 *
 * Operations:
 *
 *   EA lists may only be fetched and modified selectively.  There is no
 *   operation for deleting an EA list without first finding the name of
 *   every EA in the list and then requesting that each of those EAs be
 *   set to a 0 value size, thereby deleting the EA.
 *
 *   Retrieval
 *
 *     EA lists may be queried by explicit lists of elements or in whole.
 *     The result of the query is a FEALIST.
 *
 *     When queried selectively, a GEALIST is supplied (see the function
 *     jfs_GEAListToFEAList) along with the file's FEALIST and the value of
 *     each requested EA returned (or a cbValue value of 0 if the EA was
 *     not found).
 *
 *     When queried in whole, no GEALIST is supplied and the entire FEALIST
 *     for the file is returned.
 *
 *   Modification
 *
 *     EA lists are modified by the addition of new elements, the deletion
 *     of existing elements, or by changing the data associated with an
 *     existing element.  There is no function for deleting all existing
 *     elements.
 *
 *     Delete
 *       To delete an EA, the caller passes in an FEA element with a zero
 *       value field size.
 *
 *     Add
 *       To add an EA, the caller passes in an FEA element with the name of
 *       a non-existent EA and a non-zero value field size.  The data in the
 *       value field will be copied to the FEALIST from the FEA.
 *
 *     Modify
 *       To modify an EA, the caller passes in an FEA element with a
 *       non-zero value field size.  The data in the value field will be
 *       used to replace the existing data.
 *     
 * Storage:
 *
 *   On-disk:
 *
 *     FEALISTs are stored on disk using blocks allocated by dbAlloc() and
 *     written directly thru the buffer pool without going thru the cache
 *     manager.  An EA list may be in-lined in the inode if there is
 *     sufficient room available.
 *
 *     The functions jfs_ReadEA() and jfs_WriteEA() are responsible for
 *     moving FEALISTs between disk and memory.  jfs_WriteEA() will perform
 *     the required disk space allocation.  Freeing allocated on-disk
 *     storage is the responsibility of the functions which invoke
 *     jfs_WriteEA().
 *
 *   In-memory:
 *
 *     FEALISTs are supplied by users, read from disk, or constructed from
 *     GEALISTs and existing FEALISTs.  Memory is allocated to store
 *     non-user supplied FEALISTs from a memory pool which is managed by
 *     jfs_EABufAlloc() and jfs_EABufFree().
 *
 * Validation:
 *
 *   The EA functions assume that all FEALISTs and GEALISTs have been
 *   properly validated prior to manipulation or storage to disk.  No validity
 *   checking is performed by lower level functions as to proper format,
 *   correct output buffer size, or composition of EA names.  Specific
 *   functions exist either to perform this validation or to allow higher
 *   level functions to perform the validation.
 *
 *   Format:
 *
 *     The format of an FEALIST or GEALIST is defined elsewhere.  The
 *     function jfs_ValidateFEAList() is available for insuring that the FEA
 *     list elements have the correct size values and that the lists
 *     themselves have the correct overall format.
 *
 *   Buffer size requirements:
 *
 *     Memory requirements for storing FEALISTs must be satisfied in a
 *     single request for storage.  When two FEALISTs are being merged (such
 *     as when entries are being added to an existing FEALIST), a buffer
 *     must be allocated using jfs_EABufAlloc() using the output buffer size
 *     determined by jfs_MergedFEAListSize().  ( NOTE:  You =must= allocate
 *     the storage for the input FEALIST =and= the output FEALIST at the
 *     same time to prevent deadlock.)
 *
 *   EA name validation:
 *
 *     The name fields in FEA elements should be validated prior to writing
 *     an FEALIST to disk.  This may be performed with the function
 *     jfs_ConvertFEAListNames().
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#define INCL_DOSPROCESS

#define _cdecl __cdecl

#include <os2.h>
#include <fsd.h>
#include <sysbloks.h>
#include <extboot.h>

#include "jfs_os2.h"
#include "jfs_inode.h"
#include "jfs_dirent.h"
#include "jfs_cntl.h"
#include "jfs_dmap.h"
#include "jfs_debug.h"
#include "jfs_bufmgr.h"
#include "jfs_cachemgr.h"
#include "jfs_lock.h"
#include "jfs_ea.h"
#include "jfs_proto.h"

/* External declarations
 */
int32 txFileSystemDirty(inode_t *);

/* some macros for dealing with variable length EA lists.
 */

#define FEA_SIZE(ea) ((ea)->cbValue + (ea)->cbName + 1 + sizeof (FEA))
#define	NEXT_FEA(ea) ((FEA*)(((char *) (ea)) + (FEA_SIZE (ea))))

/* Since the GEA structure has one character in it along with the cbList field,
 * we don't have to add one for NULL when determining the size of a particular
 * GEA, since the sizeof(GEA) will already account for the NULL for us.
 */
#define GEA_SIZE(ea) ((ea)->cbName + sizeof (GEA))
#define	NEXT_GEA(ea) ((GEA*)(((char *) (ea)) + (GEA_SIZE (ea))))

/* an extended attribute consists of a <name,value> double with <name>
 * being restricted to a subset of the 8 bit ASCII character set.  this
 * table both defines valid characters for <name> and provides a lower-case
 * to upper-case mapping.
 */

#define CH_BAD_EA	'\0'

UCHAR abUCaseEA []= {
	/* 000 */	CH_BAD_EA,
	/* 001 */	CH_BAD_EA,
	/* 002 */	CH_BAD_EA,
	/* 003 */	CH_BAD_EA,
	/* 004 */	CH_BAD_EA,
	/* 005 */	CH_BAD_EA,
	/* 006 */	CH_BAD_EA,
	/* 007 */	CH_BAD_EA,
	/* 008 */	CH_BAD_EA,
	/* 009 */	CH_BAD_EA,
	/* 010 */	CH_BAD_EA,
	/* 011 */	CH_BAD_EA,
	/* 012 */	CH_BAD_EA,
	/* 013 */	CH_BAD_EA,
	/* 014 */	CH_BAD_EA,
	/* 015 */	CH_BAD_EA,
	/* 016 */	CH_BAD_EA,
	/* 017 */	CH_BAD_EA,
	/* 018 */	CH_BAD_EA,
	/* 019 */	CH_BAD_EA,
	/* 020 */	CH_BAD_EA,
	/* 021 */	CH_BAD_EA,
	/* 022 */	CH_BAD_EA,
	/* 023 */	CH_BAD_EA,
	/* 024 */	CH_BAD_EA,
	/* 025 */	CH_BAD_EA,
	/* 026 */	CH_BAD_EA,
	/* 027 */	CH_BAD_EA,
	/* 028 */	CH_BAD_EA,
	/* 029 */	CH_BAD_EA,
	/* 030 */	CH_BAD_EA,
	/* 031 */	CH_BAD_EA,
	/* 032 */	' ',
	/* 033 */	'!',
	/* 034 */	CH_BAD_EA,
	/* 035 */	'#',
	/* 036 */	'$',
	/* 037 */	'%',
	/* 038 */	'&',
	/* 039 */	'\047',
	/* 040 */	'(',
	/* 041 */	')',
	/* 042 */	'*',
	/* 043 */	CH_BAD_EA,
	/* 044 */	CH_BAD_EA,
	/* 045 */	'-',
	/* 046 */	'.',
	/* 047 */	CH_BAD_EA,
	/* 048 */	'0',
	/* 049 */	'1',
	/* 050 */	'2',
	/* 051 */	'3',
	/* 052 */	'4',
	/* 053 */	'5',
	/* 054 */	'6',
	/* 055 */	'7',
	/* 056 */	'8',
	/* 057 */	'9',
	/* 058 */	CH_BAD_EA,
	/* 059 */	CH_BAD_EA,
	/* 060 */	CH_BAD_EA,
	/* 061 */	CH_BAD_EA,
	/* 062 */	CH_BAD_EA,
	/* 063 */	'?',
	/* 064 */	'@',
	/* 065 */	'A',
	/* 066 */	'B',
	/* 067 */	'C',
	/* 068 */	'D',
	/* 069 */	'E',
	/* 070 */	'F',
	/* 071 */	'G',
	/* 072 */	'H',
	/* 073 */	'I',
	/* 074 */	'J',
	/* 075 */	'K',
	/* 076 */	'L',
	/* 077 */	'M',
	/* 078 */	'N',
	/* 079 */	'O',
	/* 080 */	'P',
	/* 081 */	'Q',
	/* 082 */	'R',
	/* 083 */	'S',
	/* 084 */	'T',
	/* 085 */	'U',
	/* 086 */	'V',
	/* 087 */	'W',
	/* 088 */	'X',
	/* 089 */	'Y',
	/* 090 */	'Z',
	/* 091 */	CH_BAD_EA,
	/* 092 */	CH_BAD_EA,
	/* 093 */	CH_BAD_EA,
	/* 094 */	'^',
	/* 095 */	'_',
	/* 096 */	'`',
	/* 097 */	'A',
	/* 098 */	'B',
	/* 099 */	'C',
	/* 100 */	'D',
	/* 101 */	'E',
	/* 102 */	'F',
	/* 103 */	'G',
	/* 104 */	'H',
	/* 105 */	'I',
	/* 106 */	'J',
	/* 107 */	'K',
	/* 108 */	'L',
	/* 109 */	'M',
	/* 110 */	'N',
	/* 111 */	'O',
	/* 112 */	'P',
	/* 113 */	'Q',
	/* 114 */	'R',
	/* 115 */	'S',
	/* 116 */	'T',
	/* 117 */	'U',
	/* 118 */	'V',
	/* 119 */	'W',
	/* 120 */	'X',
	/* 121 */	'Y',
	/* 122 */	'Z',
	/* 123 */	'{',
	/* 124 */	CH_BAD_EA,
	/* 125 */	'}',
	/* 126 */	'~',
	/* 127 */	127,
	/* 128 */	128,
	/* 129 */	129,
	/* 130 */	130,
	/* 131 */	131,
	/* 132 */	132,
	/* 133 */	133,
	/* 134 */	134,
	/* 135 */	135,
	/* 136 */	136,
	/* 137 */	137,
	/* 138 */	138,
	/* 139 */	139,
	/* 140 */	140,
	/* 141 */	141,
	/* 142 */	142,
	/* 143 */	143,
	/* 144 */	144,
	/* 145 */	145,
	/* 146 */	146,
	/* 147 */	147,
	/* 148 */	148,
	/* 149 */	149,
	/* 150 */	150,
	/* 151 */	151,
	/* 152 */	152,
	/* 153 */	153,
	/* 154 */	154,
	/* 155 */	155,
	/* 156 */	156,
	/* 157 */	157,
	/* 158 */	158,
	/* 159 */	159,
	/* 160 */	160,
	/* 161 */	161,
	/* 162 */	162,
	/* 163 */	163,
	/* 164 */	164,
	/* 165 */	165,
	/* 166 */	166,
	/* 167 */	167,
	/* 168 */	168,
	/* 169 */	169,
	/* 170 */	170,
	/* 171 */	171,
	/* 172 */	172,
	/* 173 */	173,
	/* 174 */	174,
	/* 175 */	175,
	/* 176 */	176,
	/* 177 */	177,
	/* 178 */	178,
	/* 179 */	179,
	/* 180 */	180,
	/* 181 */	181,
	/* 182 */	182,
	/* 183 */	183,
	/* 184 */	184,
	/* 185 */	185,
	/* 186 */	186,
	/* 187 */	187,
	/* 188 */	188,
	/* 189 */	189,
	/* 190 */	190,
	/* 191 */	191,
	/* 192 */	192,
	/* 193 */	193,
	/* 194 */	194,
	/* 195 */	195,
	/* 196 */	196,
	/* 197 */	197,
	/* 198 */	198,
	/* 199 */	199,
	/* 200 */	200,
	/* 201 */	201,
	/* 202 */	202,
	/* 203 */	203,
	/* 204 */	204,
	/* 205 */	205,
	/* 206 */	206,
	/* 207 */	207,
	/* 208 */	208,
	/* 209 */	209,
	/* 210 */	210,
	/* 211 */	211,
	/* 212 */	212,
	/* 213 */	213,
	/* 214 */	214,
	/* 215 */	215,
	/* 216 */	216,
	/* 217 */	217,
	/* 218 */	218,
	/* 219 */	219,
	/* 220 */	220,
	/* 221 */	221,
	/* 222 */	222,
	/* 223 */	223,
	/* 224 */	224,
	/* 225 */	225,
	/* 226 */	226,
	/* 227 */	227,
	/* 228 */	228,
	/* 229 */	229,
	/* 230 */	230,
	/* 231 */	231,
	/* 232 */	232,
	/* 233 */	233,
	/* 234 */	234,
	/* 235 */	235,
	/* 236 */	236,
	/* 237 */	237,
	/* 238 */	238,
	/* 239 */	239,
	/* 240 */	240,
	/* 241 */	241,
	/* 242 */	242,
	/* 243 */	243,
	/* 244 */	244,
	/* 245 */	245,
	/* 246 */	246,
	/* 247 */	247,
	/* 248 */	248,
	/* 249 */	249,
	/* 250 */	250,
	/* 251 */	251,
	/* 252 */	252,
	/* 253 */	253,
	/* 254 */	254,
	/* 255 */	255,
};

/* the extended attribute buffer pool is a fixed-length piece of pinned
 * memory.  the allocator uses a very simple find-first-fit algorithm to
 * manage the pool.  allocations are rounded to the nearest 1K boundary
 * and the number of slots in the allocation list is set to ensure that
 * no amount of fragmentation will result in running out of slots.
 *
 * when an allocation request cannot be satisfied, the thread sleeps on
 * EABufFree waiting for buffers to be freed
 */

#define	EABUFSIZ	(256*1024)
#define	NEABUFS		(EABUFSIZ / 1024 / 2)

struct {
	char	*addr;
	int	size;
} eamap[NEABUFS];
static	int	eamap_entries;
static	char	*eapoolbase;
/* eabufsiz should not be changed without modifying NEABUFS as well */
static	int	eabufsiz = EABUFSIZ;

/* Lock for EA Buffer pool */
MUTEXLOCK_T	jfsEALock;

#define EA_LOCK()		MUTEXLOCK_LOCK(&jfsEALock)
#define EA_UNLOCK()		MUTEXLOCK_UNLOCK(&jfsEALock)

/* Event for EA Buffer pool */
event_t	EABufFree = EVENT_NULL;
#define	EA_SLEEP()	EVENT_SLEEP(&EABufFree, &jfsEALock, T_MUTEXLOCK)
#define	EA_WAKEUP()	EVENT_WAKEUP(&EABufFree)

/*
 * NAME: jfs_EABufAlloc
 *                                                                    
 * FUNCTION: find-first-fit allocator for the EA buffer pool
 *                                                                    
 * PARAMETERS:
 *	size	- Number of bytes requested, will be rounded up to 1K boundary
 *
 * NOTES:
 *	This allocator is loosely based on the 7th Edition UNIX allocator as
 *	described by Dennis Ritchie.
 *
 *	To prevent deadlock the caller should only call this once since this
 *	allocator never gets more memory if it can't handle a request.  It will
 *	sleep waiting for someone else to free up memory being used.
 *
 * RETURNS: Pointer to pinned memory
 */
FEALIST *
jfs_EABufAlloc (int size)
{
	char	*cp;
	int	i;

	/* zero sized requests are impossible, so return NULL.
	 */

	if (! size)
		return 0;

	/* see if the buffer pool has been allocated from the pinned
	 * heap.
	 */
	EA_LOCK();

	if (eapoolbase == 0) {

		/* allocate the pool from the pinned heap and initialize
		 * the first entry in the allocation list to contain the
		 * entire pool.
		 */
		eapoolbase = xmalloc(eabufsiz, 0, pinned_heap);
		assert(eapoolbase != NULL);

		eamap[0].addr = eapoolbase;
		eamap[0].size = eabufsiz;

		eamap_entries = 1;
	}

	/* round this request to the nearest 1K boundary.
	 */
	size = (size + 1023) & ~1023;

again:
	/* scan the allocation list searching for the first free region
	 * large enough to satisfy the request.
	 */
	for (i = 0;i < eamap_entries;i++)
	{
		if (eamap[i].size >= size)
		{
			/* this region is large enough -- slice off the
			 * piece we need and see if anything is left over.
			 * if it isn't we must move all of the remaining
			 * list entries towards the front of the table.
			 */
			cp = eamap[i].addr;
			eamap[i].size -= size;
			if (eamap[i].size == 0)
			{
				for (;i < (eamap_entries-1);i++)
					eamap[i] = eamap[i+1];

				eamap_entries--;
			}
			else
				eamap[i].addr += size;

			EA_UNLOCK();
			assert (! ((unsigned int) cp & 1023));
			return (FEALIST *) cp;
		}
	}

	EA_SLEEP();

	goto again;
}


/*
 * NAME: jfs_EABufFree
 *                                                                    
 * FUNCTION: Free up an allocation request
 *                                                                    
 * PARAMETERS:
 *	cp	- Pointer to start of memory region
 *	size	- Size of memory region; must match size from call to
 *		  jfs_EABufAlloc()
 *
 * NOTES:
 *	Returns a previously allocated memory region to the EA buffer pool and
 *	wakes up any threads which are sleeping on a memory request.
 *
 * RETURNS: NONE
 */
void
jfs_EABufFree (char *cp, int size)
{
	int	i, j;

	/* The NULL buffer does not go back in the map and it had better
	 * have a zero size associated with it!
	 */
	if (! cp)
	{
		assert (size == 0);
		return;
	}
	assert (! ((unsigned int) cp & 1023));
	assert (cp >= eapoolbase && (cp + size) <= (eapoolbase + eabufsiz));

	size = ((size + 1023) & ~1023);

	/* scan for the location of the returned buffer chunk in the
	 * allocation list.
	 */
	EA_LOCK();
	for (i = 0;i < eamap_entries;i++)
	{
		if (eamap[i].addr == cp + size)
		{
			/* end of returned space is where this map
			 * entry begins.
			 */

			eamap[i].addr = cp;
			eamap[i].size += size;

			if (i > 0 && eamap[i-1].addr + eamap[i-1].size == cp)
			{
coalesce:
				/* we filled in a hole behind us
				 */
				eamap[i-1].size += eamap[i].size;
				for (;i < (eamap_entries-1);i++)
					eamap[i] = eamap[i+1];
				eamap_entries --;
			}
			goto ea_added;
		}
		else if (eamap[i].addr + eamap[i].size == cp)
		{
			/* end of this map entry is where this returned
			 * space begins.
			 */
			eamap[i].size += size;

			if (i < (eamap_entries-1) && eamap[i].addr +
					eamap[i].size == eamap[i+1].addr)
			{
				/* we filled in a hole in front of us.  quick,
				 * jump =there= and then smush the map.
				 */
				i++;
				goto coalesce;
			}
			goto ea_added;
		}
		else if (cp < eamap[i].addr)
		{
			/* this needs a new map entry stuck in here
			 */

			for (j = (eamap_entries-1);j >= i;j--)
				eamap[j+1] = eamap[j];
			eamap_entries++;

			eamap[i].addr = cp;
			eamap[i].size = size;
			goto ea_added;
		}
	}

	/*
	 * Didn't find an entry larger, so add to end of list
	 */
	eamap[i].addr = cp;
	eamap[i].size = size;
	eamap_entries++;

ea_added:
	if (EABufFree != EVENT_NULL)
	{
		EA_WAKEUP();
	}

	EA_UNLOCK();
}


/*
 * NAME: jfs_WriteEAInLine
 *                                                                    
 * FUNCTION: Attempt to write an EA inline if area is available
 *                                                                    
 * PRE CONDITIONS:
 *	Already verified that the specified EA is small enough to fit inline
 *
 * PARAMETERS:
 *	ip	- Inode pointer
 *	pfeal	- EA pointer
 *	ea	- dxd_t structure to be filled in with necessary EA information
 *		  if we successfully copy the EA inline
 *
 * NOTES:
 *	Checks if the inode's inline area is available.  If so, copies EA inline
 *	and sets <ea> fields appropriately.  Otherwise, returns failure, EA will
 *	have to be put into an extent.
 *
 * RETURNS: 0 for successful copy to inline area; -1 if area not available
 */
int
jfs_WriteEAInLine (inode_t * ip, FEALIST *pfeal, dxd_t *ea)
{
	/* make sure we have an EA -- the NULL EA list is valid, but you
	 * can't copy it!
	 */
	if (pfeal && pfeal->cbList > sizeof pfeal->cbList)
	{
		assert (pfeal->cbList <= sizeof ip->i_inlineea);

		/* see if the space is available or if it is already being
		 * used for an inline EA.
		 */
		if (! (ip->i_mode & INLINEEA) &&
				! (ip->i_ea.flag & DXD_INLINE))
			return -1;

		ea->size = pfeal->cbList;
		DXDlength(ea, 0);
		DXDaddress (ea, 0);
		memcpy((char *) ip->i_inlineea, (char *) pfeal, pfeal->cbList);
		ea->flag = DXD_INLINE;
		ip->i_mode &= ~INLINEEA;
	}
	else
	{
		ea->flag = 0;
		ea->size = 0;
		DXDlength(ea, 0);
		DXDaddress(ea, 0);

		if (ip->i_ea.flag & DXD_INLINE)
		{
			/* Free up INLINE area */
			ip->i_mode |= INLINEEA;
		}
	}
	imark (ip, ICHG);

	return 0;
}


/*
 * NAME: jfs_WriteEA
 *                                                                    
 * FUNCTION: Write an EA for an inode
 *                                                                    
 * PRE CONDITIONS: EA has been verified 
 *
 * PARAMETERS:
 *	ip	- Inode pointer
 *	pfeal	- EA pointer
 *	ea	- dxd_t structure to be filled in appropriately with where the
 *		  EA was copied
 *
 * NOTES: Will write EA inline if able to, otherwise allocates blocks for an
 *	extent and synchronously writes it to those blocks.
 *
 * RETURNS: 0 for success; Anything else indicates failure
 */
int
jfs_WriteEA (inode_t * ip, FEALIST *pfeal, dxd_t *ea)
{
	int	nblocks;
	int64	blkno;
	int64	hint;
	int	rc = 0;
	char	*cp;
	int	i;
	jbuf_t	*bp;
	int32	nbytes, nb;
	int32	bytes_to_write;

	/* quick check to see if this is an in-linable EA.  Short EAs
	 * and empty EAs are all in-linable, provided the space exists.
	 */
	if (! pfeal || pfeal->cbList <= sizeof ip->i_inlineea)
		if (! jfs_WriteEAInLine (ip, pfeal, ea))
			return 0;

	/* figure out how many blocks we need */
	nblocks = (pfeal->cbList + (ip->i_ipmnt->i_bsize - 1))
			>> ip->i_ipmnt->i_l2bsize;

	hint = INOHINT (ip);

	rc = dbAlloc (ip, hint, nblocks, &blkno);
	if (rc != 0)
		return rc;

	/* Now have nblocks worth of storage to stuff into the FEALIST.
	 * loop over the FEALIST copying data into the buffer one page at
	 * a time.
	 */
	for (i = 0, cp = (char *)pfeal, nbytes = pfeal->cbList;
		i < nblocks;
		i += ip->i_ipmnt->i_nbperpage, cp += PSIZE, nbytes -= nb)
	{
		/* Determine how many bytes for this request, and round up to
		 * the nearest aggregate block size for bmAssign request
		 */
		nb = MIN(PSIZE, nbytes);
		bytes_to_write = (((nb + (1 << ip->i_ipmnt->i_l2bsize) - 1)
			>> ip->i_ipmnt->i_l2bsize)) << ip->i_ipmnt->i_l2bsize;
		bp = bmAssign (ip, blkno + i, blkno + i, bytes_to_write,
			bmREAD_BLOCK);

		memcpy(bp->b_bdata, cp, nb);

		if (rc = bmWrite (bp))
		{
			/* the write failed -- this means that the buffer
			 * is still assigned and the blocks are not being
			 * used.  this seems like the best error recovery
			 * we can get ...
			 */
			bmRelease(bp);
			(void) dbFree (ip, blkno, nblocks);
			break;
		}
	}
	if (! rc) {
		ea->flag = DXD_EXTENT;
		ea->size = pfeal->cbList;
		DXDlength(ea, nblocks);
		DXDaddress(ea, blkno);
		if (ip->i_ea.flag & DXD_INLINE)
		{
			/* Free up INLINE area */
			ip->i_mode |= INLINEEA;
		}

		return 0;
	}
	return rc;
}


/*
 * NAME: jfs_ReadEAInLine
 *                                                                    
 * FUNCTION: Read an inlined EA into user's buffer
 *                                                                    
 * PARAMETERS:
 *	ip	- Inode pointer
 *	pfeal	- Pointer to buffer to fill in with EA
 *
 * RETURNS: 0
 */
int
jfs_ReadEAInLine (inode_t * ip, FEALIST *pfeal)
{
	if (ip->i_ea.size == 0)
	{
		pfeal->cbList = 0;
		return 0;
	}

	/* Sanity Check */
	if ((ip->i_ea.size > sizeof ip->i_inlineea) ||
	    (((FEALIST *)&ip->i_inlineea)->cbList != ip->i_ea.size))
	{
		txFileSystemDirty(ip->i_ipmnt);
		return EIO;
	}

	memcpy((char *) pfeal, (char *) ip->i_inlineea, ip->i_ea.size);

	return 0;
}


/*
 * NAME: jfs_ReadEA
 *                                                                    
 * FUNCTION: copy EA data into user's buffer
 *                                                                    
 * PARAMETERS:
 *	ip	- Inode pointer
 *	pfeal	- Pointer to buffer to fill in with EA
 *
 * NOTES:  If EA is inline calls jfs_ReadEAInLine() to copy EA.
 *
 * RETURNS: 0 for success; other indicates failure
 */
int
jfs_ReadEA (inode_t * ip, FEALIST *pfeal)
{
	int	nblocks;
	int64	blkno;
	int	rc;
	int	cbList;
	char	*cp;
	int	i;
	jbuf_t	*bp;
	int32	nbytes, nb;

	/* quick check for in-line EA
	 */
	if (ip->i_ea.flag & DXD_INLINE)
		return jfs_ReadEAInLine (ip, pfeal);

	cbList = ip->i_ea.size;
	assert (cbList);

	/* figure out how many blocks were allocated when this EA list was
	 * originally written to disk.
	 */
	nblocks = lengthDXD(&ip->i_ea);
	blkno = addressDXD(&ip->i_ea);

	/* i have found the disk blocks which were originally used to store
	 * the FEALIST.  now i loop over each contiguous block copying the
	 * data into the buffer.
	 */
	for (i = 0, cp = (char *)pfeal, nbytes = cbList; i < nblocks;
		i += ip->i_ipmnt->i_nbperpage, cp += PSIZE, nbytes -= nb) {
		if (rc = bmRead (ip, blkno + i, PSIZE, bmREAD_BLOCK, &bp))
			return rc;

		/* Sanity Check */
		if ((i == 0) && (((FEALIST *)bp->b_bdata)->cbList != cbList))
		{
			txFileSystemDirty(ip->i_ipmnt);
			bmRelease(bp);
			return EIO;
		}

		nb = MIN(PSIZE, nbytes);
		memcpy(cp, bp->b_bdata, nb);

		bmRelease(bp);
	}
	return 0;
}


/*
 * NAME: jfs_CompareEANames
 *                                                                    
 * FUNCTION: Perform comparisons on two EA names.
 *                                                                    
 * PRE CONDITIONS: Both <a> and <b> should have already been validated and
 *	converted to uppercase.
 *
 * PARAMETERS:
 *	a	- EA name
 *	b	- EA name
 *
 * RETURNS: 1 for match; 0 don't match
 */
int
jfs_CompareEANames (register char * a, register char * b)
{
	/* as long as there is a character in the first string, compare
	 * the characters for an exact match.
	 */
	for (; *a; a++, b++)
	{
		if (*a == *b)
			continue;

		return 0;
	}

	/* if the comparison ended with the first string, the second string
	 * should have been completely scanned as well.
	 */
	return *b ? 0:1;
}


/*
 * NAME: jfs_ValidateFEAList
 *                                                                    
 * FUNCTION: Validate structure of an FEALIST
 *                                                                    
 * PARAMETERS:
 *	pfeal	- FEALIST to validate
 *	oError	- Filled in with error code if not validated
 *
 * RETURNS: 0 for success; Others indicate failure
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
    ULONG	error;		/* Value to copy to poError */

    cbLeft = (USHORT) pfeal->cbList - sizeof (pfeal->cbList);
    if (cbLeft == 0)
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
	    /* poError points to users buffer */
	    error = (((char *) pfea) - ((char *) pfeal));
	    (void) KernCopyOut(poError, &error, sizeof(ULONG));
	    return ERROR_EA_LIST_INCONSISTENT;
	}

	cbLeft -= cbFEA;
    } while (cbLeft > 0);

    return 0;
}


/*
 * NAME: jfs_ConvertFEAListNames
 *                                                                    
 * FUNCTION: Convert EA names in an FEALIST to uppercase and verify names are
 *	valid
 *                                                                    
 * PARAMETERS:
 *	pfeal	- Should be modifiable storage, i.e. already copied in.
 *	oError	- On failure filled in with place of failure
 *
 * RETURNS: 0 for success; Others indicate failure
 */
int32
jfs_ConvertFEAListNames(FEALIST *pfeal,
			ULONG *poError)
{
	PFEA	lastEA;
	PFEA	pfea;
	int	index;
	char	*cp;
	ULONG	error;

	/* find the end of the FEALIST
	 */
	lastEA = ((PFEA) (((char *) pfeal) + pfeal->cbList));

	/* Loop over the entire FEALIST validating each individual FEA
	 * element.  Convert to uppercase.
	 */
	for (pfea = pfeal->list; pfea < lastEA ; pfea = NEXT_FEA (pfea))
	{
		/* check every character in the name to make sure that it
		 * is valid according to abUCaseEA[]
		 */
		for (index = 0, cp = ((char *)pfea) + sizeof(*pfea);
			index < pfea->cbName; index++, cp++)
		{
			if (! abUCaseEA[*cp])
			{
				error = (char *) pfea - (char *) pfeal;
				KernCopyOut(poError, &error, sizeof(ULONG));
				return ERROR_INVALID_EA_NAME;
			}
			if (*cp >= 'a' && *cp <= 'z')
			{
				*cp = abUCaseEA[*cp];
			}
		}

		/* make sure that the name is NULL terminated.
		 */
		if (*cp)
		{
			error = (char *) pfea - (char *) pfeal;
			(void) KernCopyOut(poError, &error, sizeof(ULONG));
			return ERROR_EA_LIST_INCONSISTENT;
		}
	}
	return 0;
}


/*
 * NAME: jfs_ConvertGEAListNames
 *                                                                    
 * FUNCTION: Convert EA names in an GEALIST to uppercase and verify names are
 *	valid
 *                                                                    
 * PARAMETERS:
 *	pgeal	- Should be modifiable storage, i.e. already copied in.
 *	oError	- On failure filled in with place of failure
 *
 * RETURNS: 0 for success; Others indicate failure
 */
int32
jfs_ConvertGEAListNames(GEALIST *pgeal,
			ULONG *poError)
{
	PGEA	lastEA;
	PGEA	pgea;
	int	index;
	char	*cp;
	ULONG	error;

	/* find the end of the GEALIST */
	lastEA = ((PGEA) (((char *) pgeal) + pgeal->cbList));

	/* Loop over the entire GEALIST validating each individual GEA
	 * element.  Convert to uppercase.
	 */
	for (pgea = pgeal->list; pgea < lastEA ; pgea = NEXT_GEA(pgea))
	{
		/* check every character in the name to make sure that it
		 * is valid according to abUCaseEA[]
		 */
		for (index = 0, cp = ((char *)pgea) + sizeof(pgea->cbName);
			index < pgea->cbName; index++, cp++)
		{
			if (! abUCaseEA[*cp])
			{
				error = (char *) pgea - (char *) pgeal;
				KernCopyOut(poError, &error, sizeof(ULONG));
				return ERROR_INVALID_EA_NAME;
			}
			if (*cp >= 'a' && *cp <= 'z')
			{
				*cp = abUCaseEA[*cp];
			}
		}

		/* make sure that the name is NULL terminated. */
		if (*cp)
		{
			error = (char *) pgea - (char *) pgeal;
			(void) KernCopyOut(poError, &error, sizeof(ULONG));
			return ERROR_EA_LIST_INCONSISTENT;
		}
	}
	return 0;
}

/*
 * NAME: jfs_ValidateUserFEAList
 *                                                                    
 * FUNCTION: Verify user's FEALIST, convert names to uppercase
 *                                                                    
 * PARAMETERS:
 *	pEABuf	- User's EA buffer
 *	out	- Buffer to copy FEALIST into
 *	oError	- Filled in on failure
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int
jfs_ValidateUserFEAList(EAOP	*pEABuf,
			FEALIST	**out,
			ULONG	*oError)
{
	ULONG	cbList;
	PFEALIST pfeal;
	EAOP	eaop;
	int	rc;

	if (rc = KernCopyIn(&eaop, pEABuf, sizeof(EAOP)))
		return rc;

	*out = NULL;

	/* validate FEALIST storage size and data locations */
	pfeal = eaop.fpFEAList;
	if (rc = KernCopyIn(&cbList, &pfeal->cbList, sizeof(cbList)))
		return rc;

	/* copy the FEALIST from the user's address space into pinned memory. */
	if (cbList <= sizeof (FEALIST))
		return 0;

	/* allocate a buffer twice the size of the FEAList.  It will be
	 * used later in jfs_InitializeFEAList() to eliminate duplicate
	 * names.
	 */
	*out = (PFEALIST) jfs_EABufAlloc (cbList << 1);

	if (rc = KernCopyIn ((unsigned char *) *out, (unsigned char *) pfeal,
			     cbList))
	{
		jfs_EABufFree ((unsigned char *) *out, cbList);
		*out = 0;
		return rc;
	}

	/* validate the FEALIST proper
	 */
	if (rc = jfs_ValidateFEAList (*out, oError))
	{
		jfs_EABufFree ((unsigned char *) *out, cbList);
		*out = 0;
		return rc;
	}

	/* validate the FEALIST names, and convert to uppercase as necessary
	 */
	if (rc = jfs_ConvertFEAListNames(*out, oError))
	{
		jfs_EABufFree((unsigned char *) *out, cbList);
		*out = NULL;
		return rc;
	}
	return 0;
}


/*
 * NAME: jfs_FindFEAByName
 *                                                                    
 * FUNCTION: Locate a FEA entry in a FEALIST given the name
 *                                                                    
 * PRE CONDITIONS: Both eaName and names in pfeal must already be converted to
 *	uppercase and validated
 *
 * PARAMETERS:
 *	pfeal	- FEALIST to search
 *	eaName	- name to search for
 *
 * RETURNS: 0 if not found; Pointer to matching entry
 */
PFEA
jfs_FindFEAByName (FEALIST *pfeal, char *eaName)
{
	PFEA	lastfea;
	PFEA	pfea;
	PFEA	next;
	int	i;
	int	cbName = strlen (eaName);

	/* the NULL FEALIST is the empty FEALIST ...
	 */
	if (pfeal == 0)
		return 0;

	lastfea = ((PFEA) (((char *) pfeal) + pfeal->cbList));

	for (pfea = pfeal->list; pfea < lastfea; pfea = next)
	{
		next = NEXT_FEA(pfea);

		/* Sanity Check */
		if ((next <= pfea) || (next > lastfea))
		{
			jEVENT(1,("Invalid FEA in jfs_FindFEAByName!\n"));
			return 0;
		}

		/* check name lengths for potential match
		 */
		if (cbName != pfea->cbName) {
			continue;
		}
		/* see if the names match when compared monocased
		 */
		if (jfs_CompareEANames ((char *) pfea + sizeof *pfea, eaName))
			return pfea;
	}
	return 0;
}


/*
 * NAME: jfs_FindLastFEAByName
 *                                                                    
 * FUNCTION: Locate the last FEA entry in a FEALIST with the given name
 *                                                                    
 * PRE CONDITIONS: Both eaName and names in pfeal must already be converted to
 *	uppercase and validated
 *
 * PARAMETERS:
 *	pfeal	- FEALIST to search
 *	eaName	- name to search for
 *
 * RETURNS: 0 if not found; Pointer to last matching entry
 */
PFEA
jfs_FindLastFEAByName (FEALIST *pfeal, char *eaName)
{
	PFEA	lastfea;
	PFEA	pfea;
	PFEA	latest_match = 0;
	PFEA	next;
	int	i;
	int	cbName = strlen (eaName);

	/* the NULL FEALIST is the empty FEALIST ...
	 */
	if (pfeal == 0)
		return 0;

	lastfea = ((PFEA) (((char *) pfeal) + pfeal->cbList));

	for (pfea = pfeal->list; pfea < lastfea; pfea = next)
	{
		next = NEXT_FEA(pfea);

		/* Sanity Check */
		if ((next <= pfea) || (next > lastfea))	
		{
			jEVENT(1,("Invalid FEA in jfs_FindFEAByName!\n"));
			return 0;
		}

		/* check name lengths for potential match
		 */
		if (cbName != pfea->cbName) {
			continue;
		}
		/* see if the names match when compared monocased
		 */
		if (jfs_CompareEANames ((char *) pfea + sizeof *pfea, eaName))
		{
			latest_match = pfea;
		}
	}
	return latest_match;
}


/*
 * NAME: jfs_DeleteFEAFromList
 *                                                                    
 * FUNCTION: Remove an FEA entry from a FEALIST
 *                                                                    
 * PARAMETERS:
 *	pfeal	- FEALIST to modify
 *	oldEA	- Entry to remove
 *
 * RETURNS: NONE
 */
void
jfs_DeleteFEAFromList (FEALIST *pfeal, FEA *oldEA)
{
	int	eaSize;
	int	tailSize;

	eaSize = FEA_SIZE (oldEA);

	assert ((char *) pfeal < (char *) oldEA) ;
	assert ((char *) pfeal + pfeal->cbList >= (char *) oldEA + eaSize);

	tailSize = pfeal->cbList -
		   (((char *) oldEA) - ((char *) pfeal->list)) -
		   eaSize;

	if (tailSize)
		memmove ((char *) oldEA, (char *) oldEA + eaSize, tailSize);

	pfeal->cbList -= eaSize;
}


/*
 * NAME: jfs_AddFEAToList
 *                                                                    
 * FUNCTION: Append a FEA entry to a FEALIST
 *                                                                    
 * PRE CONDITIONS: Doesn't check if there is space to add it; caller needs to
 *	handle
 *
 * PARAMETERS:
 *	pfeal	- FEALIST to modify (User buffer - beware!)
 *	newEA	- FEA entry to add (Pinned)
 *
 * RETURNS: 0 for success; other indicates failure
 */
int32
jfs_AddFEAToList (FEALIST *pfeal, FEA *newEA)
{
	ULONG	cbList;
	FEA	*pfea;
	uint32	newsize;
	int32	rc;

	/* find the first free spot after the FEALIST and copy the new
	 * FEA entry to that spot.
	 */
	rc = KernCopyIn(&cbList, &pfeal->cbList, sizeof(cbList));
	if (rc != NO_ERROR)
		return rc;
	ASSERT(cbList >= sizeof(cbList));
	pfea = (FEA *) ((char *) pfeal + cbList);
	newsize = FEA_SIZE(newEA);
	rc = KernCopyOut(pfea, newEA, newsize);
	if (rc != NO_ERROR)
		return rc;
	cbList += newsize;
	rc = KernCopyOut(&pfeal->cbList, &cbList, sizeof(cbList));
	return rc;
}


/*
 * NAME: jfs_AddGEAToList
 *                                                                    
 * FUNCTION: Convert a GEA entry to an FEA entry and append to the end of a
 *	FEALIST
 *                                                                    
 * PRE CONDITIONS: Doesn't check if there is space to add it; caller needs to
 *	handle
 *
 * PARAMETERS:
 *	pfeal	- FEALIST to modify (User buffer - beware!)
 *	pgea	- GEA entry to add (pinned)
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int32
jfs_AddGEAToList (FEALIST *pfeal, GEA *pgea)
{
	ULONG	cbList;
	FEA	fea;
	PFEA	pfea;
	int32	rc;

	/* construct the FEA using what GEA parts we have and default
	 * the rest of the fields.
	 */
	rc = KernCopyIn(&cbList, &pfeal->cbList, sizeof(cbList));
	if (rc != NO_ERROR)
		return rc;
	ASSERT(cbList >= sizeof(cbList));
	pfea = ((FEA *) ((char *) pfeal + cbList));
	fea.fEA = 0;
	fea.cbName = pgea->cbName;
	fea.cbValue = 0;
	rc = KernCopyOut((char *)pfea, &fea, sizeof(FEA));
	if (rc != NO_ERROR)
		return rc;
	rc = KernCopyOut((char *)pfea+sizeof(FEA), pgea->szName, fea.cbName+1);
	if (rc != NO_ERROR)
		return rc;
	cbList += FEA_SIZE (&fea);
	rc = KernCopyOut(&pfeal->cbList, &cbList, sizeof(cbList));
	return rc;
}


/*
 * NAME: jfs_GEAListToFEAList
 *                                                                    
 * FUNCTION: Construct a new FEALIST containing the attributes requested in a
 *	GEALIST.
 *                                                                    
 * PARAMETERS:
 *	old	- FEALIST to grab entries from	(pinned)
 *	new	- FEALIST to be constructed	(User buffer)
 *	pgeal	- GEALIST describing entries to find (pinned)
 *	poError	- Filled in on failure		(User buffer)
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int
jfs_GEAListToFEAList (
	FEALIST *old,
	FEALIST *new,
	GEALIST *pgeal,
	ULONG *poError)
{
	ULONG	error;
	PFEA	pfea;
	PGEA	pgea, lastgea;
	ULONG	cbList, new_cbList;
	int32	rc;

	rc = KernCopyIn(&new_cbList, &new->cbList, sizeof(new_cbList));
	if (rc != NO_ERROR)
	{
		error = 0;
		(void)KernCopyOut(poError, &error, sizeof(error));
		return rc;
	}

	/* see if all FEAs have been requested by passing in a NULL GEALIST
	 * pointer.  this is an idiotic special case behavior ...
	 */

	if (pgeal == 0)
	{
		/* Make sure there is room enough to return the entire list
		 * (or at least the =size= of the entire list).
		 */
		if (new_cbList >= old->cbList)
		{
			rc = KernCopyOut(new, old, old->cbList);
			if (rc != NO_ERROR)
			{
				error = 0;
				(void) KernCopyOut(poError, &error,
						  sizeof(error));
			}
			return rc;
		}

		/* nope, the entire FEALIST didn't fit.  the return cbList
		 * may be set if there was sufficient room for the buffer
		 * size to be stored.
		 */
		(void) KernCopyOut(&new->cbList, &old->cbList, sizeof(cbList));
		error = 0;
		(void) KernCopyOut(poError, &error, sizeof(error));
		return ERROR_BUFFER_OVERFLOW;
	}

	/*
	 * this is the not-so-special case -- the caller has passed in a
	 * GEALIST and wants the appropriate FEALIST members (or empty
	 * FEA entries if there was no corresponding FEA in the FEALIST).
	 *
	 * start by finding all of the FEAs that have been requested and
	 * tallying up the sizes.  the required size must be less than
	 * the size of the space in the new list's buffer.  this value is
	 * stored in cbList and will be compared against the supplied
	 * buffer size.
	 */

	lastgea = ((PGEA) (((char *) pgeal) + pgeal->cbList));
	cbList = sizeof new->cbList;

	for (pgea = pgeal->list ; pgea < lastgea ; pgea = NEXT_GEA (pgea))
	{
		/* the size is that of the found EA or a dummy EA with
		 * a data size of 0.
		 */
		if (pfea = jfs_FindFEAByName (old, pgea->szName))
			cbList += FEA_SIZE (pfea);
		else
			cbList += pgea->cbName + 1 + sizeof (FEA);

		if (cbList > new_cbList)
		{
			error = ((char *) pgea) - ((char *) pgeal);
			(void) KernCopyOut(poError, &error, sizeof(error));
			(void) KernCopyOut(&new->cbList, &old->cbList,
					   sizeof(cbList));
			
			return ERROR_BUFFER_OVERFLOW;
		}
	}

	/* now walk back thru the list and copy all of the requested FEAs
	 * to the new buffer.
	 */

	new_cbList = sizeof(new_cbList);
	rc = KernCopyOut(&new->cbList, &new_cbList, sizeof(new_cbList));
	if (rc != NO_ERROR)
	{
		error = 0;
		(void) KernCopyOut(poError, &error, sizeof(error));
		return rc;
	}
	for (pgea = pgeal->list ; pgea < lastgea ; pgea = NEXT_GEA (pgea))
	{
		if (pfea = jfs_FindFEAByName (old, pgea->szName))
			rc = jfs_AddFEAToList (new, pfea);
		else
			rc = jfs_AddGEAToList (new, pgea);
		if (rc != NO_ERROR)
		{
			error = ((char *) pgea) - ((char *) pgeal);
			(void) KernCopyOut(poError, &error, sizeof(error));
			return rc;
		}
	}

	return 0;
}


/*
 * NAME: jfs_MergeFEALists
 *                                                                    
 * FUNCTION: Merge a new FEALIST into the space occupied by an existing FEALIST.
 *                                                                    
 * PRE CONDITIONS: It is assumed that both lists are valid and that there is
 *	sufficient room in the existing list to add the new entries.
 *
 * PARAMETERS:
 *	out	- FEALIST to be constructed
 *	orig	- Original FEALIST, keep any entries that don't appear in <chg>
 *	chg	- New FEALIST
 *
 * RETURNS: NONE
 */
void
jfs_MergeFEALists (
	FEALIST *out,
	FEALIST *orig,
	FEALIST *chg)
{
	PFEA	pfea;
	PFEA	lastfea;
	PFEA	found, found_another;

	/* search the original list for every entry in the list of entries
	 * to be updated.  
	 *
	 * if the entry is not found, the old EA is copied to the new list 
	 * without change.  
	 *
	 * if the entry is found, check to see if there are any by the same
	 * name later in the list of entries to update. 
	 * If there are, and the last new entry by that name has non-zero 
	 * value size, replace the old one with the last  new one (with 
	 * matching name).
	 */

	out->cbList = sizeof(out->cbList);

	if (orig)
	{
		lastfea = ((PFEA) (((char *) orig) + orig->cbList));

		for (pfea = orig->list ; pfea < lastfea ;
						pfea = NEXT_FEA (pfea))
		{
			if (found = jfs_FindLastFEAByName (chg,
					(char *) pfea + sizeof *pfea))
			{
				if (found->cbValue)
					jfs_AddFEAToList (out, found);
			}
			else
				jfs_AddFEAToList (out, pfea);
		}
	}

	/* go back thru the list of changes and find the completely new
	 * entries.
	 */
	if (chg)
	{
		lastfea = ((PFEA) (((char *) chg) + chg->cbList));

		for (pfea = chg->list ; pfea < lastfea ; pfea = NEXT_FEA (pfea))
		{
			if (! jfs_FindFEAByName (orig,
					(char *) pfea + sizeof *pfea))
			{
				found_another = jfs_FindLastFEAByName (chg,
						(char *) pfea + sizeof *pfea); 
				if( pfea == found_another ) 
				{
					if (pfea->cbValue)
						jfs_AddFEAToList (out, pfea);
				}
			}
		}
	}

}


/*
 * NAME: jfs_InitializeFEAList
 *                                                                    
 * FUNCTION: Set the initial value for a file EA list
 *                                                                    
 * PARAMETERS:
 *	ip	- Inode pointer
 *	tid	- Transaction ID
 *	pfeal	- FEALIST to initialize
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int
jfs_InitializeFEAList (
	inode_t		*ip,
	int32		tid,
	FEALIST		*pfeal)
{
	int	rc;
	dxd_t	dxd;
           FEALIST *purged_pfeal, *old;
	
	/* If there is no FEALIST to apply initialize an empty dxd.  Then let
	 * the txEA() function handle releasing old EA for us.  This case
	 * happens on an OPEN_REPLACE_IF_EXISTS call
	 */
	if (! pfeal)
	{
		dxd.size = 0;
		DXDlength(&dxd, 0);
		DXDaddress(&dxd, 0);
		dxd.flag = 0;
		/* If old EA was inline, the space is available again */
		if (ip->i_ea.flag & DXD_INLINE)
			ip->i_mode |= INLINEEA;
		rc = 0;
	}
	else
	{
			/* 
			 * The buffer passed into this routine was
			 * allocated by jfs_ValidateUserFEAList() at
			 * twice the size needed for the actual FEAList.
			 *
			 * This was done so we'd have the 2nd half
			 * for use here.  
			 *
			 * (We can't allocate a separate buffer because
			 * that would risk deadlock on the buffer pool.)
			 * 
			 * Find the address of the 2nd half of the buffer,
			 * then call the merge routine to drop all but the
			 * last occurrence of each named EA in the list.
			 */
                     purged_pfeal = (FEALIST *)((char *)pfeal + pfeal->cbList);
		purged_pfeal->cbList = sizeof purged_pfeal->cbList;
		old = 0;		
		jfs_MergeFEALists (purged_pfeal, old, pfeal);

		/* allocate storage for and copy the user's FEALIST to disk */
		rc = jfs_WriteEA(ip, purged_pfeal, &dxd);
	}

	if (! rc)
	{
		if (ip->i_ea.flag & DXD_EXTENT)
			/* free old EA pages from cache */
			bmExtentInvalidate(ip, addressDXD(&ip->i_ea),
					   lengthDXD(&ip->i_ea));

		txEA(tid, ip, &ip->i_ea, &dxd);

		imark(ip, ICHG);
	
		ip->i_nblocks = ip->i_nblocks + lengthDXD(&dxd)
						- lengthDXD(&ip->i_ea);
		ip->i_ea = dxd;
	}

	return rc;
}


/*-----------------------------------------------------------------------
 * NAME: eaRelocate 
 *                                                                    
 * FUNCTION: 	Relocate the Extended Attributes from their current 
 *		address to a new location.
 *                                                                    
 * PARAMETERS:
 *	ip	pointer to owning inode in memory
 *
 * NOTES:	none
 *
 * RETURNS: success:	0 
 *	    failure: 	something else
 */
int32 eaRelocate ( inode_t  *ip )	
{
    int32		ear_rc = 0;
    FEALIST 	*ea_bufptr;
    dxd_t	new_dxd;
    
    int		nblocks;
    int64		blkno;

jFYI(0,("jfs_ea: entering eaRelocate  ino:%d  ip:0x%08x\n",ip->i_number,ip));
	
    blkno = addressDXD(&ip->i_ea);
    nblocks = lengthDXD( &(ip->i_ea) );	

    ea_bufptr = jfs_EABufAlloc ( (nblocks << ip->i_ipmnt->i_l2bsize) );

jFYI(0,("eaRelocate:  jfs_EABufAlloc( nblocks:%d ), rc:%d\n",nblocks,ear_rc));
	
    ear_rc = jfs_ReadEA ( ip, ea_bufptr );

jFYI(0,("eaRelocate:  jfs_ReadEA( ), rc:%d\n",ear_rc));

    if( ear_rc == 0 ) {	/* the EA is in the buffer */
 	ear_rc = jfs_WriteEA ( ip, ea_bufptr, &new_dxd );

jFYI(0,("eaRelocate:  jfs_WriteEA( ), rc:%d\n",ear_rc));
	
	if( ear_rc == 0 ) {	/* the ea has been relocated */
		/*
		 * copy the new descriptor into the inode
		 */
	    memcpy( (void *) &(ip->i_ea), 
	 	       (void *) &(new_dxd), 
		       sizeof(dxd_t) 
		      );
		/*
		 * free the storage where the EA used to be
		 */
	    dbFree( ip, blkno, nblocks );
	    }  /* end the ea has been relocated */
	}  /* end the EA is in the buffer */
	
    jfs_EABufFree( (unsigned char *)ea_bufptr,
			(nblocks<< ip->i_ipmnt->i_l2bsize) );
	
jFYI(0,("jfs_ea: leavingeaRelocate  ear_rc:%d\n",ear_rc));

    return( ear_rc );
}
