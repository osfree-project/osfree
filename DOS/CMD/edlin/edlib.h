/* edlib.h -- block manipulation routines for edlin

  AUTHOR: Gregory Pietsch <gpietsch@comcast.net>

  DESCRIPTION:

  This file contains block manipulation routines for edlin, an 
  edlin-style line editor.

  COPYRIGHT NOTICE AND DISCLAIMER:

  Copyright (C) 2003 Gregory Pietsch

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
*/

#ifndef EDLIB_H
#define EDLIB_H

/* includes */
#if defined(__STDC__) || defined(HAVE_STDDEF_H)
#include <stddef.h>
#elif defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

/* typedefs */

/* static variables */

/* functions */

/* commands */

/* initialize the buffer */
void create_buffer (void);

/* destroy the buffer */
void destroy_buffer (void);

/* transfer_file - merges the contents of a file on disk with a file in memory
 */
void transfer_file (unsigned long before_line, char *filename);

/* write X number of lines to a file */
void write_file (unsigned long lines, char *filename);

/* copy a block of lines elsewhere in the buffer */
void copy_block (unsigned long line1, unsigned long line2,
                 unsigned long line3, size_t count);

/* delete a block from line1 to line2 */
void delete_block (unsigned long line1, unsigned long line2);

/* move the block from line1 to line2 to immediately before line3 */
void move_block (unsigned long line1, unsigned long line2,
                 unsigned long line3);

/* read a line from stdin */
char *read_line (char *prompt);

/* display a block of text */
void display_block (unsigned long first_line, unsigned long last_line,
                    unsigned long current_line, size_t page_size);

/* modify_line - modify a line in the buffer */
void modify_line (unsigned long line);

/* search_buffer - search a buffer for a string */
unsigned long search_buffer (unsigned long current_line,
                             unsigned long line1, unsigned long line2,
                             int verify, char *s);

/* replace_buffer - search a buffer for a string, then replace */
unsigned long replace_buffer (unsigned long current_line,
                              unsigned long line1, unsigned long line2,
                              int verify, char *s);

/* insert_block - go into insert mode */
unsigned long insert_block (unsigned long line);

/* get the last line in the buffer */
unsigned long get_last_line (void);

#endif

/* END OF FILE */
