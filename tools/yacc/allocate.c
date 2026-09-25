/****************************************************************
 * Allocate and clear storage for bison,
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
 *  @file allocate.c
 *  @brief Memory allocation helpers for Bison.
 *
 *  Provides xmalloc() and xrealloc(): thin wrappers around calloc()
 *  and realloc() that terminate the program with a diagnostic when
 *  the allocation fails.
 *
 *  @copyright Copyright (C) 1984, 1989 Free Software Foundation, Inc.
 *             Licensed under the GNU General Public License v2 or later.
 */

#include <stdio.h>
#include "system.h"

#ifdef NEED_DECLARATION_CALLOC
#if defined (__STDC__) || defined (_MSC_VER)
/*!
 *  @brief Allocates zero-initialized storage.
 *
 *  @param[in] nmemb Number of elements.
 *  @param[in] size  Size of each element.
 *
 *  @return Pointer to the allocated block, or NULL on failure.
 */
extern void *calloc ();
#else
/*!
 *  @brief Allocates zero-initialized storage.
 *
 *  @param[in] nmemb Number of elements.
 *  @param[in] size  Size of each element.
 *
 *  @return Pointer to the allocated block, or NULL on failure.
 */
extern char *calloc ();
#endif
#endif  /* NEED_DECLARATION_CALLOC */

#ifdef NEED_DECLARATION_REALLOC
#if defined (__STDC__) || defined (_MSC_VER)
/*!
 *  @brief Resizes a previously allocated block.
 *
 *  @param[in] ptr  Block to resize.
 *  @param[in] size New size in bytes.
 *
 *  @return Pointer to the resized block, or NULL on failure.
 */
extern void *realloc ();
#else
/*!
 *  @brief Resizes a previously allocated block.
 *
 *  @param[in] ptr  Block to resize.
 *  @param[in] size New size in bytes.
 *
 *  @return Pointer to the resized block, or NULL on failure.
 */
extern char *realloc ();
#endif
#endif  /* NEED_DECLARATION_REALLOC */

/*!
 *  @brief Allocates and clears @p n bytes of storage.
 *
 *  @param[in] n Number of bytes to allocate. Zero is treated as 1.
 *
 *  @return Pointer to the allocated, zeroed block.
 */
char *xmalloc PARAMS((register unsigned));

/*!
 *  @brief Resizes a previously allocated block.
 *
 *  @param[in] block Pointer to the block to resize.
 *  @param[in] n     New size in bytes. Zero is treated as 1.
 *
 *  @return Pointer to the resized block.
 */
char *xrealloc PARAMS((register char *, register unsigned));

/*!
 *  @brief Terminates the program after closing the output files.
 *
 *  @param[in] k Exit status.
 */
extern void done PARAMS((int));

/*!
 *  @brief Name of the running program, used in diagnostics.
 */
extern char *program_name;

/*!
 *  @brief Allocates and clears @p n bytes of storage.
 *
 *  @param[in] n Number of bytes to allocate. Zero is treated as 1.
 *
 *  @return Pointer to the allocated, zeroed block.
 */
char *xmalloc (register unsigned n)
{
  register char *block;

  /* Avoid uncertainty about what an arg of 0 will do.  */
  if (n == 0)
    n = 1;
  block = calloc (n, 1);
  if (block == NULL)
    {
      fprintf (stderr, _("%s: memory exhausted\n"), program_name);
      done (1);
    }

  return (block);
}

/*!
 *  @brief Resizes a previously allocated block.
 *
 *  @param[in] block Pointer to the block to resize.
 *  @param[in] n     New size in bytes. Zero is treated as 1.
 *
 *  @return Pointer to the resized block.
 */
char *xrealloc (register char *block, register unsigned n)
{
  /* Avoid uncertainty about what an arg of 0 will do.  */
  if (n == 0)
    n = 1;
  block = realloc (block, n);
  if (block == NULL)
    {
      fprintf (stderr, _("%s: memory exhausted\n"), program_name);
      done (1);
    }

  return (block);
}
