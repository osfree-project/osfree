/**
 * \file   l4vfs/term_server/lib/vt100/cursor.c
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

void cursor_down( termstate_t *term  )
{
    // in global origin mode, move around the whole term
    if (term->origin_mode == VT100_ORIGIN_GLOBAL)
    {
        if (term->cur_y < term->phys_h - term->par[0] )
            term->cur_y += term->par[0];
        else
            term->cur_y = term->phys_h - 1;
    }
    // in scroll origin mode move only inside the defined
    // scrolling region
    else // VT100_ORIGIN_SCROLL
    {
        if (term->cur_y < term->scroll_bottom - term->par[0])
            term->cur_y += term->par[0];
        else
            term->cur_y = term->scroll_bottom - 1;
    }
}

void cursor_up( termstate_t *term  )
{
    // in global origin mode, move around the whole term
    if (term->origin_mode == VT100_ORIGIN_GLOBAL)
    {
        if (term->cur_y >= term->par[0] )
            term->cur_y -= term->par[0];
        else
            term->cur_y = 0;
    }
    // in scroll origin mode move only inside the defined
    // scrolling region
    else // VT100_ORIGIN_SCROLL
    {
        if (term->cur_y >= term->scroll_top+term->par[0] )
            term->cur_y -= term->par[0];
        else
            term->cur_y = term->scroll_top;
    }
}

void cursor_left( termstate_t *term  )
{
    // currently only handle global mode. this function
    // exists to make sure, that we can handle right
    // and left margins in the same manner as scrolling
    // regions
    if (term->cur_x - term->par[0] > 0 )
        term->cur_x -= term->par[0];
    else
        term->cur_x = 0;
}

void cursor_right( termstate_t *term  )
{
    // currently only handle global mode. this function
    // exists to make sure, that we can handle right
    // and left margins in the same manner as scrolling
    // regions
    if (term->cur_x < term->w - term->par[0] - 1)
        term->cur_x += term->par[0];
    else
        term->cur_x = term->w - 1;
}
