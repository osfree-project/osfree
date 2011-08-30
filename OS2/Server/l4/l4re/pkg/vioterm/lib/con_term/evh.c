/**
 * \file   l4vfs/term_server/lib/con_term/evh.c
 * \brief  event handlers for key repeat, key press
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
/* L4 includes */
#include <stdio.h>

#include <l4/l4con/stream-server.h>
#include <l4/l4con/l4con_ev.h>
#include <l4/term_server/vt100.h>
#include <l4/term_server/con_term.h>
#include <l4/input/libinput.h>
#include <l4/log/l4log.h>
#include <l4/semaphore/semaphore.h>
#include <l4/thread/thread.h>
#include <l4/util/l4_macros.h>

#include "lib.h"

static void evh_loop(void *data);
static void btn_repeat(void *data);

/******************************************************************************
 * evh_init                                                                   *
 *                                                                            *
 * event handler initialization                                               *
 ******************************************************************************/
int evh_init(termstate_t * term)
{
    /* start thread */
    if ((term->spec->evh_tid =
         l4thread_create(evh_loop, term, L4THREAD_CREATE_SYNC)) < 0)
        return term->spec->evh_tid;
    term->spec->evh_l4id = l4thread_l4_id(term->spec->evh_tid);

    if ((term->spec->btn_tid =
         l4thread_create(btn_repeat, term, L4THREAD_CREATE_SYNC)) < 0)
        return term->spec->btn_tid;
    term->spec->btn_l4id = l4thread_l4_id(term->spec->btn_tid);

//    term->__key_init = 1;
    return 0;
}

static void
touch_repeat(termstate_t * term, unsigned code, unsigned repeat)
{
    l4_msgdope_t result;
    l4_umword_t dummy;
    int error;

    error = l4_ipc_call(term->spec->btn_l4id, 
                        L4_IPC_SHORT_MSG, code, repeat,
                        L4_IPC_SHORT_MSG, &dummy, &dummy,
                        L4_IPC_SEND_TIMEOUT_0, &result);
    if (error)
    {
        term->spec->key_pending = 1;
        term->spec->key_code = code;
        term->spec->key_repeat = repeat;
    }
}

/******************************************************************************
 * stream_io_server_push                                                      *
 *                                                                            *
 * in: request        ... Flick request structure                             *
 *     event          ... incoming event                                      *
 * out: _ev           ... Flick exception (unused)                            *
 * ret:   0           ... success                                             *
 *                                                                            *
 * handle incoming events                                                     *
 *****************************************************************************/
void 
stream_io_push_component(CORBA_Object _dice_corba_obj,
                         const stream_io_input_event_t *event,
                         CORBA_Server_Environment *_dice_corba_env)
{
    struct l4input *input_ev = (struct l4input*) event;
    // get thread local data
    termstate_t * term = (termstate_t *)_dice_corba_env->user_data;

    switch(input_ev->type)
    {
    case EV_KEY:
        switch(input_ev->code)
        {
        case KEY_RIGHTSHIFT:        // modifiers
        case KEY_LEFTSHIFT:
            if (input_ev->value)
                term->__shift = 1;
            else
                term->__shift = 0;
            touch_repeat(term, input_ev->code, 0);
            return;
        case KEY_LEFTCTRL:
        case KEY_RIGHTCTRL:
            if (input_ev->value)
                term->__ctrl  = 1;
            else
                term->__ctrl  = 0;
            touch_repeat(term, input_ev->code, 0);
            return;
        case KEY_LEFTALT:
            if (input_ev->value)
                term->__alt  = 1;
            else
                term->__alt  = 0;
            touch_repeat(term, input_ev->code, 0);
            return;
        case KEY_RIGHTALT:
            if (input_ev->value)
                term->__altgr  = 1;
            else
                term->__altgr  = 0;
            touch_repeat(term, input_ev->code, 0);
            return;
        case KEY_PAGEUP:            // special terminal movement chars
            if(input_ev->value && term->__shift)
            {
                vis_up(term, term->phys_h / 2); // scroll for half screen
                vt100_redraw(term);
                touch_repeat(term, input_ev->code, input_ev->value);
                return;
            }
            break;
        case KEY_PAGEDOWN:
            if(input_ev->value && term->__shift)
            {
                vis_down(term, term->phys_h / 2); // scroll for half screen
                vt100_redraw(term);
                touch_repeat(term, input_ev->code, input_ev->value);
                return;
            }
            break;
        }
        if(input_ev->value)         // regular chars
        {
            vt100_add_key(term, input_ev->code);
        }
        touch_repeat(term, input_ev->code, input_ev->value);
        break;
    
    case EV_CON:
        switch(input_ev->code) 
        {
        case EV_CON_REDRAW:
            LOG("vt100_redraw()");
            vt100_redraw(term);
            break;
        }
        break;
    default:
        //LOGl("Event = %d", input_ev->type);
        break;
    }
}

/******************************************************************************
 * evh_loop                                                                   *
 *                                                                            *
 * evh - IDL server loop                                                      *
 ******************************************************************************/
static void
evh_loop(void *data)
{
    CORBA_Server_Environment env = dice_default_server_environment;
    env.user_data = data;

    l4thread_started(NULL);
    stream_io_server_loop(&env);
}

static void
btn_repeat(void *data)
{
    termstate_t * term = (termstate_t *)data;

    l4thread_started(NULL);
  
    for (;;)
    {
        l4_umword_t code, new_code, repeat, new_repeat;
        l4_msgdope_t result;
        int error;

        // wait for touch_repeat()
        error = l4_ipc_receive(term->spec->evh_l4id, 
                               L4_IPC_SHORT_MSG, &code, &repeat,
                               L4_IPC_NEVER, &result);
        error = l4_ipc_send   (term->spec->evh_l4id,
                               L4_IPC_SHORT_MSG, 0, 0,
                               L4_IPC_NEVER, &result);
        if (! repeat)
            continue;

        for (;;)
        {
            /* wait for around 250ms */
            error = l4_ipc_receive(term->spec->evh_l4id,
                                   L4_IPC_SHORT_MSG, &new_code, &new_repeat,
                                   l4_timeout(L4_IPC_TIMEOUT_0, l4_timeout_rel(976,8)), &result);
            if (error == L4_IPC_RETIMEOUT && !term->spec->key_pending)
            {
                /* no new key in the meantime -- start repeat.
                 * wait for round 30ms */
                for (;;)
                {
                    if (term->autorepeat)
                    {
                        switch (code)
                        {
                        case KEY_PAGEUP:
                            if(term->__shift)
                            {
                                vis_up(term, term->phys_h / 2);
                                vt100_redraw(term);
                                return;
                            }
                            break;
                        case KEY_PAGEDOWN:
                            if(term->__shift)
                            {
                                vis_down(term, term->phys_h / 2);
                                vt100_redraw(term);
                                return;
                            }
                            break;
                        default:
                            // this one prevents autorepeat
                            //vt100_add_key_if_not_too_busy(term, code);
                            vt100_add_key(term, code);
                        }
                    }

                    /* wait for key up or other key down, 30ms */
                    error = l4_ipc_receive(term->spec->evh_l4id,
                                           L4_IPC_SHORT_MSG,
                                           &new_code, &repeat,
                                           l4_timeout(L4_IPC_TIMEOUT_0, l4_timeout_rel(515, 6)),
                                           &result);
                    if (error != L4_IPC_RETIMEOUT || term->spec->key_pending)
                    {
                        /* new key or key_up received -- break repeat.
                         * tricky: fall through until next send */
                        break;
                    }
                }
            }

            if (error == 0)
            {
                code   = new_code;
                repeat = new_repeat;

                /* new key or key_up received -- only reply, do not repeat */
                error = l4_ipc_send(term->spec->evh_l4id,
                                    L4_IPC_SHORT_MSG, 0, 0,
                                    L4_IPC_NEVER, &result);
                if (repeat)
                    continue;

                break;
            }
            else if (term->spec->key_pending)
            {
                code   = term->spec->key_code;
                repeat = term->spec->key_repeat;
                term->spec->key_pending = 0;

                if (repeat)
                    continue;

                break;
            }
            else
            {
                /* ??? */
                LOG("btn_repeat: ipc error %02x", error);
                break;
            }
        }
    }
}
