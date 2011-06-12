/**
 * \file   l4vfs/term_server/lib/dope_term/callback.c
 * \brief  key repeat and key press event handling
 *
 * \date   08/10/2004
 * \author Björn Döbel  <doebel@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/dope/dopelib.h>
#include <l4/log/l4log.h>
#include <l4/sys/types.h>
#include <l4/sys/syscalls.h>
#include <l4/sys/ipc.h>
#include "internal.h"

extern int _DEBUG;

void dope_term_repeat_callback( dope_event *e, void *arg )
{
    int c;
    // extract the term data
    termstate_t *term = (termstate_t *)arg;

    // return immediately if autorepeat is off
    if (!term->autorepeat)
        return;

    if (e->type == EVENT_TYPE_KEYREPEAT)
    {
        // get the key code
        c = e->keyrepeat.code;

        switch( c )
        {
            case KEY_PGUP:
                if (term->__shift)
                {
                    vis_up(term, term->phys_h/2);
                    //vt100_redraw(term);
                }
                break;
            case KEY_PGDOWN:
                if (term->__shift)
                {
                    vis_down(term, term->phys_h/2);
                    //vt100_redraw(term);
                }
                break;
            default:
                vt100_add_key( term, c );
                break;
        }
        update_scrollbar(term);
    }
}

void dope_term_press_callback( dope_event *e, void *arg )
{
    int c;

    // unpack termstate argument
    termstate_t *term = (termstate_t *)arg;

    if (e->type == EVENT_TYPE_PRESS)
    {
        term->__keydown = 1;

        // get keycode
        c = e->press.code;
        LOGd(_DEBUG, "key pressed: %d", c);

        // check if it is a special key
        switch(c)
        {
            // shift pressed
            case KEY_SHIFT_R:
            case KEY_SHIFT_L:
                term->__shift = 1; break;
            // ctrl pressed
            case KEY_CTRL_R:
            case KEY_CTRL_L:
                term->__ctrl = 1; break;
            // alt pressed
            case KEY_ALT:
                term->__alt = 1; break;
            // altgr pressed
            case KEY_ALT_GR:
                term->__altgr = 1; break;

            // shift + pgdown = scroll down
            case KEY_PGDOWN:
                if (term->__shift)
                {
                    vis_down( term, term->phys_h/2 );
                    //vt100_redraw(term);
                }
                break;

            // shift + pgup = scroll up
            case KEY_PGUP:
                if (term->__shift)
                {
                    vis_up( term, term->phys_h/2 );
                    //vt100_redraw(term);
                }
                break;

            // ignore mouse events (for now)
            case KEY_MOUSE_LEFT:
            case KEY_MOUSE_RIGHT:
                break;

            // any other key
            default:
                vt100_add_key( term, c );
                break;
        } //switch
        update_scrollbar(term);
    }

    // handle key release
    if (e->type == EVENT_TYPE_RELEASE)
    {
        c = e->release.code;

        term->__keydown = 0;

        switch(c)
        {
            case KEY_SHIFT_R:
            case KEY_SHIFT_L:
                term->__shift = 0; break;
            case KEY_CTRL_R:
            case KEY_CTRL_L:
                term->__ctrl = 0; break;
            case KEY_ALT:
                term->__alt = 0; break;
            case KEY_ALT_GR:
                term->__altgr = 0; break;
            default: 
                break;
        }
    }
}

