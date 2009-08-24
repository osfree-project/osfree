/* $Id: dumpfs.c,v 1.1.1.1 2003/05/21 13:42:48 pasha Exp $ */

static char *SCCSID = "@(#)1.4  9/5/97 12:23:34 src/jfs/utils/validfs/dumpfs.c, jfsutil, w45.fs32, 990417.1";
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
 *   COMPONENT_NAME: jfsutil
 *
 *   FUNCTIONS: SZARRAY
 *		build_index
 *		display_cpage
 *		display_dmap
 *		display_level
 *		print_bytes_as_word
 *		print_char
 *		print_char_array
 *		print_int64
 *		print_int64_array
 *		print_uint_array
 *		print_word
 *
 */
#define INCL_DOSERRORS
#include <os2.h>
#include "jfs_types.h"
#include <jfs_aixisms.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "jfs_filsys.h"
#include "jfs_superblock.h"
#include "jfs_dinode.h"
#include "inode.h"
#include "jfs_dmap.h"
#include "super.h"
#include "devices.h"
#include "jfs_xtree.h"
#include "jfs_dtree.h"
#include "jfs_imap.h"

#define BUF	   64	/* tmp buffer length 				*/
#define	MAP_INDEX  12	/* number of spaces for printing map indicies	*/
#define NUM_COLS    3	/* number of columns 			*/
#define SZARRAY(map)	(sizeof(map) / sizeof(map[0]))

/*
 * build_index
 *
 *
 */
void build_index( char  *index,
		  int  new,
		  int  old)
{
	
	if (new  == old + 1)
		sprintf (index, "[%d]:", old);
	else
		sprintf (index, "[%d-%d]:",  old, new - 1);
}


void print_word( int  new,
		 int  old,
		 int	 space,
		 uint32 value)
{
	char  index[BUF];

	build_index (index, new, old);
	printf ("%*s  0x%08x",	space, index, value);
}


void print_bytes_as_word( int		new,
			  int		old,
			  int		space,
			  unsigned char	*value )
{
	char  index[BUF];

	build_index( index, new, old );
	printf( "%*s  0x%02x%02x%02x%02x", space, index, value[0], value[1],
		value[2], value[3] );
}


/*
 * print_uint_array
 *
 */
void print_uint_array( char  *name,
		       uint32  *map,
		       uint32  len)
{
	uint32	old_value = *map;
	uint32	old_index = 0;
	int	cols = 1;
	int	index_len = strlen (name);
	int	i, index;
	unsigned char	four_bytes[4], *cp;

	
	printf ("%s", name);

	index_len = MAP_INDEX - index_len;
	
	for (i = 0; i < len; i++)
	{
		if (map[i] != old_value)
		{
			print_word (i, old_index, index_len, old_value);
			old_index = i;
			old_value = map[i];
			index_len = MAP_INDEX;

			if  (cols++ % NUM_COLS == 0)
			{
				printf ("\n");
				cols = 1;
			}
		}
	}
	cp = (unsigned char *)&old_value;
	for(index = 0; index < 4; index++) {
		four_bytes[index] = *(cp + index);
	}
	print_bytes_as_word( i, old_index, index_len, four_bytes );
	printf ("\n");
}


void print_int64( int	new,
		  int	old,
		  int	space,
		  int64	value)
{
	char  index[BUF];

	build_index (index, new, old);
	printf ("%*s  %lld",	space, index, value);
}


/*
 * print_int64_array
 *
 */
void print_int64_array( char  *name,
			int64  *map,
			uint32  len)
{
	int64	old_value = *map;
	uint32	old_index = 0;
	int	cols = 1;
	int	index_len = strlen (name);
	int	i, index;
	unsigned char	four_bytes[4], *cp;

	
	printf ("%s", name);

	index_len = MAP_INDEX - index_len;
	
	for (i = 0; i < len; i++)
	{
		if (map[i] != old_value)
		{
			print_int64 (i, old_index, index_len, old_value);
			old_index = i;
			old_value = map[i];
			index_len = MAP_INDEX;

			if  (cols++ % NUM_COLS == 0)
			{
				printf ("\n");
				cols = 1;
			}
		}
	}
	print_int64( i, old_index, index_len, old_value );
	printf ("\n");
}


/*
 * print_char
 *
 */
void print_char( int  new,
		 int  old,
		 int	 space,
		 signed char value)
{
	char  index[BUF];

	build_index (index, new, old);
	printf ("%*s  %-10d", space, index, value);
}


/*
 * print_char_array
 *
 */
void print_char_array(  char  *name,
			signed char  *map,
			uint32  len)
{
	signed char	old_value = *map;
	uint32	old_index = 0;
	int	cols = 1;
	int	index_len = strlen (name);
	int	i;

	printf ( "%s", name);

	index_len = MAP_INDEX - index_len;
	
	for (i = 0; i < len; i++)
	{
		if (map[i] != old_value)
		{
			print_char(i, old_index, index_len, old_value);
			old_index = i;
			old_value = map[i];
			index_len = MAP_INDEX;
			if  (cols++ % NUM_COLS == 0)
			{
				printf ("\n");
				cols = 1;
			}
		}
	}
	print_char(i, old_index, index_len, old_value);
	printf ("\n");
}


int32 display_cpage( dbmap_t	*control_page )
{
	printf("\nOverall Control Page\n");
	printf("MapSize\t%lld\t\tNumber Free Blocks\t%lld\n",
		control_page->dn_mapsize, control_page->dn_nfree );
	printf("L2 Num Blocks Per Page\t%d\n", control_page->dn_l2nbperpage );
	printf("Num Active AG\t%d\t\tMax Level\t%d\n", control_page->dn_numag,
		control_page->dn_maxlevel );
	printf("AG Size\t%lld\t\tl2 of AG size\t%d\n", control_page->dn_agsize,
		control_page->dn_agl2size );
	printf("Max AG\t%d\t\tAG Pref\t%d\n", control_page->dn_maxag,
		control_page->dn_agpref );
	printf("AG Width\t%d\t\tAG Start\t%d\n", control_page->dn_agwidth,
		control_page->dn_agstart );
	printf("AG Heigth\t%d\t\tAG Level %d\n", control_page->dn_agheigth,
		control_page->dn_aglevel );
	printf("Max Free Buddy\t%d\n", control_page->dn_maxfreebud );
	print_int64_array( "dn_agfree", control_page->dn_agfree,
			   SZARRAY(control_page->dn_agfree) );
	printf("\n\n");
}


int32 display_dmap(int32	page_number,
		   dmap_t	*dmap_page )
{
	printf("dmap PAGE %d: nblocks %d, nfree %d\n", page_number,
		dmap_page->nblocks, dmap_page->nfree);
	printf("start blockno\t%lld\t\tnbudmin\t%d\n", dmap_page->start,
		dmap_page->tree.budmin);
	printf("Num Leaves\t%d\t\tLeaf index\t%d\n", dmap_page->tree.nleafs,
		dmap_page->tree.leafidx);
	printf("L2 Num Leaves\t%d\n", dmap_page->tree.l2nleafs );
	printf("Height\t%d\n", dmap_page->tree.height);

	print_char_array("stree:", dmap_page->tree.stree,
			 SZARRAY(dmap_page->tree.stree));

	if (memcmp((void *) dmap_page->wmap,
		    (void *)dmap_page->pmap,
		    sizeof(dmap_page->wmap[0]) * LPERDMAP) == 0)
	{
		print_uint_array ("maps:", dmap_page->wmap, LPERDMAP);
	}
	else
	{
		print_uint_array ("wmap:", dmap_page->wmap, LPERDMAP);
		print_uint_array ("pmap", dmap_page->pmap, LPERDMAP);
	}
	printf("\n\n");
}


int32 display_level(int32	level,
		    int32	page_number,
		    dmapctl_t	*level_page )
{
	/*
	 * Display control page
	 */
	printf("Level %d Control PAGE %d\n", level, page_number );
	printf("Num Leaves\t%d\t\tLeaf Index\t%d\n", level_page->nleafs,
		level_page->leafidx );
	printf("L2 Num Leaves\t%d\n", level_page->l2nleafs);
	printf("Height\t%d\t\tBuddy Min\t%d\n", level_page->height,
		level_page->budmin );
	print_char_array("stree:", level_page->stree,
		SZARRAY(level_page->stree));
	printf("\n\n");
}
