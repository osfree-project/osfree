/* ccl - routines for character classes */

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Vern Paxson.
 * 
 * The United States Government has rights in this work pursuant
 * to contract no. DE-AC03-76SF00098 between the United States
 * Department of Energy and the University of California.
 *
 * Redistribution and use in source and binary forms with or without
 * modification are permitted provided that: (1) source distributions retain
 * this entire copyright notice and comment, and (2) distributions including
 * binaries display the following acknowledgement:  ``This product includes
 * software developed by the University of California, Berkeley and its
 * contributors'' in the documentation or other materials provided with the
 * distribution and in all advertising materials mentioning features or use
 * of this software.  Neither the name of the University nor the names of
 * its contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* $Header: /home/daffy/u0/vern/flex/RCS/ccl.c,v 2.9 93/09/16 20:32:14 vern Exp $ */

/*!
 *  @file ccl.c
 *  @brief Routines for character classes.
 *
 *  Provides ccladd(), cclinit(), cclnegate() and list_character_set().
 */

#include "flexdef.h"

/*!
 *  @brief Adds a single character to a character class.
 *
 *  @param[in] cclp Character class index.
 *  @param[in] ch   Character to add.
 */
void ccladd( cclp, ch )
int cclp;
int ch;
	{
	int ind, len, newpos, i;

	check_char( ch );

	len = ccllen[cclp];
	ind = cclmap[cclp];

	/* check to see if the character is already in the ccl */

	for ( i = 0; i < len; ++i )
		if ( ccltbl[ind + i] == ch )
			return;

	newpos = ind + len;

	if ( newpos >= current_max_ccl_tbl_size )
		{
		current_max_ccl_tbl_size += MAX_CCL_TBL_SIZE_INCREMENT;

		++num_reallocs;

		ccltbl = reallocate_Character_array( ccltbl,
						current_max_ccl_tbl_size );
		}

	ccllen[cclp] = len + 1;
	ccltbl[newpos] = ch;
	}


/*!
 *  @brief Returns an empty character class.
 *
 *  @return Index of the newly created character class.
 */
int cclinit()
	{
	if ( ++lastccl >= current_maxccls )
		{
		current_maxccls += MAX_CCLS_INCREMENT;

		++num_reallocs;

		cclmap = reallocate_integer_array( cclmap, current_maxccls );
		ccllen = reallocate_integer_array( ccllen, current_maxccls );
		cclng = reallocate_integer_array( cclng, current_maxccls );
		}

	if ( lastccl == 1 )
		/* we're making the first ccl */
		cclmap[lastccl] = 0;

	else
		/* The new pointer is just past the end of the last ccl.
		 * Since the cclmap points to the \first/ character of a
		 * ccl, adding the length of the ccl to the cclmap pointer
		 * will produce a cursor to the first free space.
		 */
		cclmap[lastccl] = cclmap[lastccl - 1] + ccllen[lastccl - 1];

	ccllen[lastccl] = 0;
	cclng[lastccl] = 0;	/* ccl's start out life un-negated */

	return lastccl;
	}


/*!
 *  @brief Negates the given character class.
 *
 *  @param[in] cclp Character class index.
 */
void cclnegate( cclp )
int cclp;
	{
	cclng[cclp] = 1;
	}


/*!
 *  @brief Lists the members of a set of characters in CCL form.
 *
 *  Writes to the given file a character-class representation of those
 *  characters present in the given CCL.  A character is present if it
 *  has a non-zero value in the cset array.
 *
 *  @param[in] file Output stream.
 *  @param[in] cset Character set array.
 */
void list_character_set( file, cset )
FILE *file;
int cset[];
	{
	register int i;

	putc( '[', file );

	for ( i = 0; i < csize; ++i )
		{
		if ( cset[i] )
			{
			register int start_char = i;

			putc( ' ', file );

			fputs( readable_form( i ), file );

			while ( ++i < csize && cset[i] )
				;

			if ( i - 1 > start_char )
				/* this was a run */
				fprintf( file, "-%s", readable_form( i - 1 ) );

			putc( ' ', file );
			}
		}

	putc( ']', file );
	}
