/**
 * \file   l4vfs/term_server/lib/dope_term/draw.c
 * \brief  redraw dope terminal
 *
 * \date   08/10/2004
 * \author Björn Döbel  <doebel@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/dope/dopelib.h>
#include <l4/dope/vscreen.h>

#include <l4/term_server/vt100.h>
#include <l4/term_server/dope_term.h>

#include <l4/log/l4log.h>
#include <l4/semaphore/semaphore.h>

#include <string.h>
#include "internal.h"

extern int _DEBUG;

// completely redraw the terinal window
void vt100_redraw( termstate_t *term )
{
    int x,y;

    l4semaphore_down(&term->termsem);

    for (y=0; y<term->phys_h; y++)
    {
        for (x=0; x<term->w; x++)
        {
            int index = xy2index(term, x, y-term->vis_off);
            // tricky: xy2index_virt() is abused to index into a
            //         dope-provided buffer
            // FIXME: Here we should use a dope-provided client lib
            //        or write libdope_term specific index function, to cope
            //        with future changes in the dope buffer layout.
            int index_virt = xy2index_virt(term, x, y);

            term->spec->charbuf[index_virt] =
                term->text[index];
            term->spec->attrbuf[index_virt] =
                term->attrib[index];
        }
    }

    // refresh the terminal
    dope_cmdf( term->spec->app_id,
            "v.refresh( -x 0 -y 0 -w %d -h %d )",
            term->w+1,
            term->phys_h+1);

    l4semaphore_up(&term->termsem);
    update_scrollbar(term);
}

// redraw 1 character
void vt100_redraw_xy( termstate_t *term, int x, int y )
{

    int charpos=0, charpos_virt=0;
    
    // out of physical drawing area?
    if (y+term->vis_off >= term->phys_h)
        return;
    
    l4semaphore_down(&term->termsem);

    // the char`s position in text and attrib buffer
    // --> only calculate it once
    charpos = xy2index_virt( term, x, y );
    charpos_virt = xy2index( term, x, y );

//    LOGd(_DEBUG, "charpos = (%d,%d)", x, y);
//    LOGd(_DEBUG, "virtual = %d, physical = %d", charpos,
//                        charpos_virt );

    // remap only the one character to redraw
    term->spec->charbuf[ charpos ]
        = term->text[ charpos_virt ];
    term->spec->attrbuf[ charpos ]
        = term->attrib[ charpos_virt ];
    
    // refresh 1 character
    dope_cmdf( term->spec->app_id,
            "v.refresh( -x 0 -y 0 -w %d -h %d )",
            term->w+1,
            term->phys_h+1);

    l4semaphore_up(&term->termsem);
}
