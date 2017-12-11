/**
 * \file   l4vfs/term_server/lib/con_term/helper.c
 * \brief  redraw con, write to con
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/l4con/l4con-client.h>
#include <l4/l4con/l4con_pslim.h>
#include <l4/term_server/vt100.h>
#include <l4/term_server/con_term.h>
#include <l4/log/l4log.h>
#include <l4/semaphore/semaphore.h>

#include "lib.h"

extern int _DEBUG;

void putstocon(termstate_t *term, int x, int y, l4_int8_t *s, int len)
{
    CORBA_Environment env = dice_default_environment;

    con_vc_puts_call(&(term->spec->vtc_l4id), s, len,
                     CONV_X(term, x), CONV_Y(term, y),
                     FG_COLOR, BG_COLOR, &env);
}

void putstocon_color(termstate_t *term, int x, int y, l4_int8_t *s, int len,
                     l4con_pslim_color_t fg, l4con_pslim_color_t bg)
{
    CORBA_Environment env = dice_default_environment;
    long ret;
    
//    LOGl("c = %c, len = %d, fg = %x, bg = %x", *s, len, fg, bg);
    ret = con_vc_puts_call(&(term->spec->vtc_l4id), s, len,
                           CONV_X(term, x), CONV_Y(term, y),
                           fg, bg, &env);
}

/*
 * Clear screen on our console
 */
void clrscr(termstate_t *term)
{
    l4con_pslim_rect_t rect;
    CORBA_Environment env = dice_default_environment;
  
    rect.x = 0; 
    rect.y = 0;
    rect.w = CONV_X(term, term->w - 1);
    rect.h = CONV_Y(term, term->phys_h - 1);
  
    con_vc_pslim_fill_call(&(term->spec->vtc_l4id), &rect, BG_COLOR, &env);
}

// redraw whole screen
void vt100_redraw(termstate_t *term)
{
    int x, y;
    int old_x = 0, old_y = 0;
    l4_int8_t *s = NULL;
    l4_int8_t old_attrib = 0;
    int old_index = 0;
    l4con_pslim_color_t fg, bg;

//    LOGl("term = %p, text = %p, color = %p", term, term->text, term->color);
    if (term == NULL)
        return;
    l4semaphore_down(&term->termsem);
    // correct y for vis offset
    for (y = 0 - term->vis_off; y < term->phys_h - term->vis_off; y++)
    {
        for (x = 0; x < term->w; x++)
        {
            //vt100_redraw_xy(term, x, y);

            // if we observe a change in attributes, send the
            // accumulated string
            if (s != NULL && old_attrib != term->attrib[xy2index(term, x, y)])
            {
                attribs_to_colors(old_attrib, &fg, &bg);
                // correct y for vis offset
                putstocon_color(term, old_x, old_y + term->vis_off, s,
                                xy2index(term, x, y) - old_index, fg, bg);
                s = NULL;
            }
            // start a new string
            if (s == NULL)
            {
                old_index = xy2index(term, x, y);
                s = term->text + old_index;
                old_attrib = term->attrib[old_index];
                old_x = x;
                old_y = y;
            }
        }
        // care for end of line remainder strings
        if (s != NULL)
        {
            attribs_to_colors(old_attrib, &fg, &bg);
            // correct y for vis offset
            putstocon_color(term, old_x, old_y + term->vis_off, s,
                            xy2index(term, x - 1, y) - old_index + 1, fg, bg);
            s = NULL;
        }
    }
    l4semaphore_up(&term->termsem);
}

void vt100_redraw_xy(termstate_t *term, int x, int y)
{
    l4con_pslim_color_t fg, bg;
    l4_int8_t a;
    l4_int8_t *c;

    l4semaphore_down(&term->termsem);
    // if out of bound, do nothing
    if (y + term->vis_off >= term->phys_h)
    {
        l4semaphore_up(&term->termsem);
        return;
    }

    c = term->text + xy2index(term, x, y);
    a = term->attrib[xy2index(term, x, y)];
    attribs_to_colors(a, &fg, &bg);
    // correct for moved vis
    putstocon_color(term, x, y + term->vis_off, c, 1, fg, bg);
    l4semaphore_up(&term->termsem);
}
