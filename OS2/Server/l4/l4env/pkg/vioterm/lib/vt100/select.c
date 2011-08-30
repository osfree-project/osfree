/**
 * \file   l4vfs/term_server/lib/vt100/select.c
 * \brief  select() implementation
 *
 * \date   09/03/2007
 * \author Bjoern Doebel <doebel@tudos.org>
 */
/* (c) 2007 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

#include <l4/log/l4log.h>
#include <l4/util/macros.h>

#include "lib.h"
#include "keymap.h"

extern int _DEBUG;

static void select_list_enqueue(termstate_t *term, struct term_select_info *t)
{
    l4semaphore_down(&term->termsem);
    t->prev       = NULL;
    t->next       = term->select_list;
    term->select_list = t;
    l4semaphore_up(&term->termsem);
}


static void select_list_dequeue(termstate_t *term, struct term_select_info *t)
{
    if (t)
    {
        l4semaphore_down(&term->termsem);
        if (t == term->select_list)
            term->select_list = t->next;
        else
            t->prev->next = t->next;
        l4semaphore_up(&term->termsem);
    }
}


void vt100_set_select_info(termstate_t *term, object_handle_t handle,
                          int mode, const l4_threadid_t *notify_handler)
{
    struct term_select_info *t = malloc(sizeof(struct term_select_info));
    t->handler    = *notify_handler;
    t->mode       = mode;
    t->fd         = handle;

    select_list_enqueue(term, t);
}


void vt100_unset_select_info(termstate_t *term, object_handle_t handle,
                             int mode, const l4_threadid_t *handler)
{
    struct term_select_info *n = term->select_list;

    while (n)
    {
        if (l4_thread_equal(n->handler, *handler) &&
            (n->mode == mode) && (n->fd == handle))
            break;
        n = n->next;
    }

    select_list_dequeue(term, n);
    free(n);
}


void vt100_select_notify(termstate_t *term)
{
    struct term_select_info *n = term->select_list;

    select_list_dequeue(term, n);

    if (n)
    {
        l4vfs_select_listener_send_notification(n->handler,
                                                n->fd, n->mode);
        free(n);
    }
}


int vt100_data_avail(termstate_t *term)
{
    int ret;

    return (term->keylist_next_write != term->keylist_next_read);
}
