/**
 * \file   l4vfs/term_server/lib/vt100/insert.c
 * \brief  insert and erase characters and whole lines
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include "lib.h"
#include <l4/log/l4log.h>

extern int _DEBUG;

void insert_line(termstate_t *term)
{
    // only insert if we are inside the scrolling region
    if (term->cur_y >= term->scroll_top)
    {
        rev_scroll_region( term, term->cur_y, term->scroll_bottom );
    }
}

// insert char, move all following chars one step further
void insert_char(termstate_t *term, char c)
{
    int x, index_cur, index_pred;

    for (x = term->w-1; x > term->cur_x; x--)
    {
        index_cur           = xy2index( term, x, term->cur_y );
        index_pred          = xy2index( term, x-1, term->cur_y ); 
        term->text[index_cur]   = term->text[index_pred];
        term->attrib[index_cur] = term->attrib[index_pred];
    }
    index_cur               = xy2index( term, term->cur_x, term->cur_y );
    term->text[index_cur]   = c;
    term->attrib[index_cur] = term->attrib_mode;
}

void erase_line(termstate_t *term)
{
    // only erase if we are inside the scrolling region
    if (term->cur_y >= term->scroll_top)
    {
        scroll_region( term, term->cur_y, term->scroll_bottom );
    }
}
