/**
 * \file   l4vfs/term_server/lib/dope_term/cursor.c
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

#include "internal.h"

void vt100_show_cursor(termstate_t *term)
{
    if (term->cursor_vis && term->cur_y + term->vis_off <= term->phys_h)
    {
        dope_cmdf(term->spec->app_id, "v.set(-cursorx %d -cursory %d)",
            term->cur_x, term->cur_y);
    }
}

extern int _DEBUG;
void vt100_hide_cursor(termstate_t *term)
{
    dope_cmd(term->spec->app_id, "v.set(-cursorx -1 -cursory 0)");
}

