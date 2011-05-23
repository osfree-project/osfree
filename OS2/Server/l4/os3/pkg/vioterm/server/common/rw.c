/**
 * \file   l4vfs/term_server/server/common/rw.c
 * \brief  read and write functions
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/l4vfs/types.h>
#include <l4/log/l4log.h>
#include <l4/term_server/vt100.h>
#include <l4/thread/thread.h>
#include <l4/util/l4_macros.h>

#include <stdlib.h>

#include <dice/dice.h>
#include <l4/sys/types.h>
#include <l4/sys/syscalls.h>
#include <l4/l4vfs/common_io_notify-client.h>
#include <l4/term_server/internal-server.h>
#include <l4/term_server/main-client.h>

#include "term.h"
#include "basic_io.h"

extern int _DEBUG;

int term_write(local_object_id_t object, l4_int8_t *buf, int count)
{
    int term = object - 1;
    return vt100_write(terms[term].terminal, buf, count);
}

void term_read(void *argp)
{
    CORBA_Server_Environment env = dice_default_server_environment;

    l4thread_started(0);
    LOGd(_DEBUG, "read thread created: "l4util_idfmt, 
             l4util_idstr(l4_myself()));

    env.malloc  = (dice_malloc_func)malloc;
    env.free    = (dice_free_func)free;

    l4vfs_term_server_internal_server_loop(&env);
}

void l4vfs_term_server_internal_start_read_component(
                        CORBA_Object _dice_corba_obj,
                        object_handle_t fd,
                        l4vfs_size_t *count,
                        const l4_threadid_t *notifier,
                        const l4_threadid_t *source,
                        CORBA_Server_Environment *_dice_corba_env)
{
    // return value, read mode
    int ret, mode;
    // buffer for read
    char *mybuf;
    // terminal id
    local_object_id_t object;
    // terminal state
    termstate_t *term;
    CORBA_Environment env = dice_default_environment;

    l4semaphore_down(&(clients[fd].client_sem));

    // read object id, ...
    object = clients[fd].object_id;
    // ... get term state, ...
    term   = terms[object-1].terminal;
    // ... read rw mode ...
    mode   = clients[fd].rw_mode;
    // ... allocate mem for the buffer
    // fixme: how about alloca?  should be faster and is freed
    //        automatically on return
    mybuf  = malloc(*count + 1);

    l4semaphore_up(&(clients[fd].client_sem));

    // call read
    ret    = vt100_read(term, (l4_int8_t*)mybuf, *count, mode);
    LOGd(_DEBUG, "vt100_read: ret %d, buf %p, count %d", ret, mybuf, *count);
    // count will now give the size of the buffer for sending data
    // back to the main server loop. If ret is negative, read() failed and
    // we then return count=0 bytes. Otherwise we return count=ret bytes
    // as read returns the bytes read.
    *count = MAX(ret, 0);

    env.malloc = (dice_malloc_func)malloc;
    env.free   = (dice_free_func)free;

    // return data to the main thread. This is a drawback of L4V2 as the
    // client expects the read()-answer to come from the thread he called.
    // We therefore cannot return the buffer directly to the caller.
    l4vfs_common_io_notify_read_notify_send(notifier, fd, ret,
                                            mybuf, count, source, &env);

    // free the allocated buffer
    free(mybuf);
}

