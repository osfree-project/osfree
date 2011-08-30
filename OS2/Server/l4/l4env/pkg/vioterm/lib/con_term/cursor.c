/**
 * \file   l4vfs/term_server/lib/con_term/cursor.c
 * \brief  cursor movement and display
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/term_server/vt100.h>
#include <l4/log/l4log.h>

#include "lib.h"

void vt100_hide_cursor(termstate_t *term)
{
    if (term->cursor_vis)
    {
        // if the cursor is in visible area...
        if ( (term->cur_y + term->vis_off) <= term->phys_h )
        {
            int bg, fg, intensity;
            int index = xy2index( term, term->cur_x, term->cur_y );

            unpack_attribs( term->attrib[index], &fg, &bg, &intensity );
            term->attrib[index] = pack_attribs( bg, fg, intensity );

            // only redraw if cursor is visible
            vt100_redraw_xy( term, term->cur_x, term->cur_y );
        }
    }
}

void vt100_show_cursor(termstate_t *term)
{
    if (term->cursor_vis)
    {
        if ( (term->cur_y + term->vis_off) <= term->phys_h )
        {
            int bg, fg, intensity;
            int index = xy2index( term, term->cur_x, term->cur_y );

            unpack_attribs( term->attrib[index], &fg, &bg, &intensity );
            term->attrib[index] = pack_attribs( bg, fg, intensity );

            // only redraw if cursor is visible
            vt100_redraw_xy( term, term->cur_x, term->cur_y );
        }
    }
}

