/*!
 * @file cond.h
 * @brief Conditional expressions.
 *
 * Copyright (c) 1993-1995 Eberhard Mattes
 *
 * This file is part of emxdoc.
 *
 * emxdoc is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * emxdoc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with emxdoc; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


/*!
 * @brief Evaluate a conditional expression.
 *
 * @param[in] p Expression text. Not NULL.
 *
 * @return Non-zero if the expression is true, zero otherwise.
 */
int condition (const uchar *p);
/*!
 * @brief Assign a value to a conditional variable.
 *
 * @param[in] name  Variable name. Not NULL.
 * @param[in] value New value.
 */
void cond_set (const uchar *name, int value);
