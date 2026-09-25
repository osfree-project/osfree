/****************************************************************
 * Storage allocation interface for bison,
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
 *  @file alloc.h
 *  @brief Storage allocation interface for Bison.
 *
 *  Declares the xmalloc()/xrealloc() wrappers and provides the NEW(),
 *  NEW2() and FREE() convenience macros used throughout Bison to
 *  allocate and release heap objects.
 *
 *  @copyright Copyright (C) 1984, 1989 Free Software Foundation, Inc.
 *             Licensed under the GNU General Public License v2 or later.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*!
 *  @brief Allocates one object of type @p t.
 *
 *  @param[in] t Type of the object to allocate.
 *  @def NEW
 */
#define	NEW(t)		((t *) xmalloc((unsigned) sizeof(t)))

/*!
 *  @brief Allocates an array of @p n objects of type @p t.
 *
 *  @param[in] n Number of objects.
 *  @param[in] t Element type.
 *  @def NEW2
 */
#define	NEW2(n, t)	((t *) xmalloc((unsigned) ((n) * sizeof(t))))

#ifdef __STDC__
/*!
 *  @brief Releases the object pointed to by @p x, if any.
 *
 *  @param[in] x Pointer to release.
 *  @def FREE
 */
#define	FREE(x)		(x ? (void) free((char *) (x)) : (void)0)
#else
#define FREE(x) 	((x) != 0 && (free ((char *) (x)), 0))
#endif

/*!
 *  @brief Allocates and clears @p n bytes of storage.
 *
 *  @param[in] n Number of bytes to allocate.
 *
 *  @return Pointer to the allocated, zeroed block.
 */
extern char *xmalloc  PARAMS((register unsigned));

/*!
 *  @brief Resizes a previously allocated block.
 *
 *  @param[in] block Pointer to the block to resize.
 *  @param[in] n     New size in bytes.
 *
 *  @return Pointer to the resized block.
 */
extern char *xrealloc PARAMS((register char *, register unsigned));
