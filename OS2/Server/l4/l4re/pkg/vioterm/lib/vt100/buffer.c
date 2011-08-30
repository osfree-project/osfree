/**
 * \file   l4vfs/term_server/lib/vt100/buffer.c
 * \brief  managment functions for the history buffer
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/semaphore/semaphore.h>
#include <l4/log/l4log.h>

#include "lib.h"

extern int _DEBUG;

// move vis up
void vis_up(termstate_t *term, int up)
{
    int i;

    vt100_hide_cursor(term);

    l4semaphore_down(&term->termsem);
    for (i = 0; i < up && term->vis_off < term->virt_h - term->phys_h ; i++)
    {
        term->vis_off++;
    }
    if (term->vis_off > term->hist_len)
        term->vis_off = term->hist_len;
    l4semaphore_up(&term->termsem);

    vt100_show_cursor(term);
    vt100_redraw(term);
}

// move vis down
void vis_down(termstate_t *term, int down)
{
    int i;

    vt100_hide_cursor(term);
    l4semaphore_down(&term->termsem);
    for (i = 0; i < down && term->vis_off > 0; i++)
    {
        term->vis_off--;
    }
    l4semaphore_up(&term->termsem);

    vt100_show_cursor(term);
    vt100_redraw(term);
}

// scroll virt buffer
void scroll(termstate_t *term)
{
    l4semaphore_down(&term->termsem);

    term->phys_off++;

    // increment history length if it is not already
    // as long as the buffer
    if ( term->hist_len < term->virt_h - term->phys_h )
        term->hist_len++;

    // if we reach the end, wrap around
    // ==  phys_off %= virt_h
    if (term->phys_off > term->virt_h - 1)
        term->phys_off -= term->virt_h;

    clean_line(term, term->phys_h - 1);
    l4semaphore_up(&term->termsem);

    vt100_redraw(term);
}

// scroll region one line up
void scroll_region(termstate_t *term, int start, int end)
{
    int x,y;

    l4semaphore_down(&term->termsem);

    for (y = start + 1; y < end; y++)
    {
        for (x = 0; x < term->w; x++)
        {
            term->text[xy2index(term, x, y - 1)] =
                term->text[xy2index(term, x, y)];
            term->attrib[xy2index(term, x, y - 1)] =
                term->attrib[xy2index(term, x, y)];
        }
    }

    clean_line(term, end - 1);
    l4semaphore_up(&term->termsem);

    vt100_redraw(term);
}

// reverse scroll a region
void rev_scroll_region(termstate_t *term, int start, int end)
{
    int x, y;

    l4semaphore_down(&term->termsem);

    // fixme: probably we don't need the following two checks
    //        (as in scroll_region)
    // start must be at least scroll_top
    if (start < term->scroll_top)
        start = term->scroll_top;
    // end must be less than scroll_bottom
    if (end > term->scroll_bottom)
        end = term->scroll_bottom;

    // yes, no need to scroll one line, but clean_line is required
//    // no need to scroll one line
//    if (start == end)
//    {
//        l4semaphore_up(&term->termsem);
//        return;
//    }

    // text
    for (y = end - 1; y > start; y--)
        for (x = 0; x < term->w; x++)
        {
            term->text[xy2index(term, x, y)] =
                term->text[xy2index(term, x, y - 1)];
        }

    // color
    for (y = end - 1; y > start; y--)
        for (x = 0; x < term->w; x++)
        {
            term->attrib[xy2index(term, x, y)] =
                term->attrib[xy2index(term, x, y - 1)];
        }

    // clean first line
    clean_line(term, start);

    l4semaphore_up(&term->termsem);

    vt100_redraw(term);
}

void rev_scroll(termstate_t *term)
{
    if (term->cur_y == term->scroll_top)
        rev_scroll_region(term, term->scroll_top, term->scroll_bottom);
    else
        term->cur_y--;
}

/* convert (x,y) on phys to index in virt
 */
int xy2index(termstate_t * term, int x, int y)
{
    int max = (term->virt_h) * term->w;
    // XXX should we check the range for 'x' here ?
    int ret = x + (y + term->phys_off) * term->w;

    // ==  ret %= max
    while (ret < 0)
        ret += max;

    while (ret >= max)
        ret -= max;

    return ret;
}

/* convert (x,y) on virt to index in virt
 */
int xy2index_virt(termstate_t * term, int x, int y)
{
    return x + y * term->w;
}

void set_char(termstate_t *term, l4_int8_t c)
{
    set_char_xy(term, c, term->cur_x, term->cur_y);
}

void set_char_xy(termstate_t *term, l4_int8_t c, int x, int y)
{
    int index, fg, bg, in;
    l4_int8_t attrib;

    attrib = term->attrib_mode;
    index = xy2index(term, x, y);
    term->text[index] = c;
    if (term->decscnm)  // switch fg and bg
    {
        unpack_attribs(attrib, &fg, &bg, &in);
        attrib = pack_attribs(bg, fg, in);
    }

    term->attrib[index] = attrib;
}
