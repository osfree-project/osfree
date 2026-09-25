/****************************************************************
 * Generate transitive closure of a matrix,
 * Copyright (C) 1984, 1989 Free Software Foundation, Inc.
 *
 * This file is part of Bison, the GNU Compiler Compiler.
 *
 * Bison is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * Bison is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bison; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 ****************************************************************/

/*!
 *  @file warshall.c
 *  @brief Transitive closure of a bit matrix.
 *
 *  Implements TC() and RTC(): the plain and the reflexive transitive
 *  closure of an n-by-n bit matrix, used by Bison to compute FIRSTS,
 *  FDERIVES, FOLLOWS and other relations.
 *
 *  @copyright Copyright (C) 1984, 1989 Free Software Foundation, Inc.
 *             Licensed under the GNU General Public License v2 or later.
 */


#include <stdio.h>
#include "system.h"
#include "machine.h"

/*!
 *  @brief Computes the reflexive transitive closure of @p R in place.
 *
 *  @param[in,out] R Bit matrix to close; modified in place.
 *  @param[in]     n Dimension of the matrix.
 */
void RTC PARAMS((unsigned *, int));


/*!
 *  @brief Computes the transitive closure of the bit matrix @p R.
 *
 *  @param[in,out] R Bit matrix; modified in place.
 *  @param[in]     n Dimension of the matrix.
 */
static void
TC (unsigned *R, int n)
{
  register int rowsize;
  register unsigned mask;
  register unsigned *rowj;
  register unsigned *rp;
  register unsigned *rend;
  register unsigned *ccol;

  unsigned *relend;
  unsigned *cword;
  unsigned *rowi;

  rowsize = WORDSIZE(n) * sizeof(unsigned);
  relend = (unsigned *) ((char *) R + (n * rowsize));

  cword = R;
  mask = 1;
  rowi = R;
  while (rowi < relend)
    {
      ccol = cword;
      rowj = R;

      while (rowj < relend)
	{
	  if (*ccol & mask)
	    {
	      rp = rowi;
	      rend = (unsigned *) ((char *) rowj + rowsize);

	      while (rowj < rend)
		*rowj++ |= *rp++;
	    }
	  else
	    {
	      rowj = (unsigned *) ((char *) rowj + rowsize);
	    }

	  ccol = (unsigned *) ((char *) ccol + rowsize);
	}

      mask <<= 1;
      if (mask == 0)
	{
	  mask = 1;
	  cword++;
	}

      rowi = (unsigned *) ((char *) rowi + rowsize);
    }
}


/*!
 *  @brief Computes the reflexive transitive closure of @p R in place.
 *
 *  @param[in,out] R Bit matrix; modified in place.
 *  @param[in]     n Dimension of the matrix.
 */
void
RTC (unsigned *R, int n)
{
  register int rowsize;
  register unsigned mask;
  register unsigned *rp;
  register unsigned *relend;

  TC(R, n);

  rowsize = WORDSIZE(n) * sizeof(unsigned);
  relend = (unsigned *) ((char *) R + n*rowsize);

  mask = 1;
  rp = R;
  while (rp < relend)
    {
      *rp |= mask;

      mask <<= 1;
      if (mask == 0)
	{
	  mask = 1;
	  rp++;
	}

      rp = (unsigned *) ((char *) rp + rowsize);
    }
}
