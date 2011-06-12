/**
 * \file   l4vfs/term_server/lib/con_term/lib.h
 * \brief  
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#ifndef __L4VFS_TERM_SERVER_LIB_CON_TERM_LIB_H_
#define __L4VFS_TERM_SERVER_LIB_CON_TERM_LIB_H_

#include <l4/term_server/con_term.h>
#include <l4/term_server/vt100.h>
#include <l4/l4con/l4con_pslim.h>
#include <l4/semaphore/semaphore.h>

#define FG_COLOR 0x00ffffff
#define BG_COLOR 0x00000000
#define CONTXT_TIMEOUT 5000

#define CONV_X(term, x) ((term->spec->font_w) * (x))
#define CONV_Y(term, y) ((term->spec->font_h) * (y))

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

int term_getchar(termstate_t * term);
int term_trygetchar(termstate_t * term);

int evh_init(termstate_t * term);

void clrscr(termstate_t *term);
void putstocon(termstate_t *term, int x, int y, l4_int8_t *s, int len);
void putstocon_color(termstate_t *term, int x, int y, l4_int8_t *s, int len,
                     l4con_pslim_color_t fg, l4con_pslim_color_t bg);

void attribs_to_colors(l4_int8_t a, l4con_pslim_color_t * fg,
                       l4con_pslim_color_t * bg);

#endif
