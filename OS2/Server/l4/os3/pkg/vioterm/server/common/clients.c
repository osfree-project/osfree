/**
 * \file   l4vfs/term_server/server/common/clients.c
 * \brief  client management
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <l4/log/l4log.h>
#include <l4/env/errno.h>

#include "basic_io.h"
#include "term.h"

extern int _DEBUG;

int get_free_clientstate(void)
{
    int i;  
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].open == false)
        return i;
    }
    LOG("Could not find free client. VC Server busy.");
    return -1;
}

void free_clientstate(int handle) 
{
    // kill the worker thread
    if (!l4_is_invalid_id(clients[handle].worker))
        l4thread_shutdown(l4thread_id(clients[handle].worker));

    // invalidate clientstate entries
    clients[handle].open      = false;
    clients[handle].rw_mode   = 0;
    clients[handle].object_id = L4VFS_ILLEGAL_OBJECT_ID;
    clients[handle].seekpos   = 0;
    clients[handle].client    = L4_INVALID_ID;
    clients[handle].worker    = L4_INVALID_ID;
    clients[handle].client_sem= L4SEMAPHORE_UNLOCKED;
}


int clientstate_open(int flags, l4_threadid_t client,
                      local_object_id_t object_id)
{
    int ret;

    // get 1st free client
    ret = get_free_clientstate();
    LOGd(_DEBUG, "free client: %d", ret);
    
    // no free client found ?
    if (ret < 0)
        return -ENOMEM;
        
    // valid object id ?
    LOGd(_DEBUG, "object: %d", object_id);
    if (object_id < 0 || object_id > MAX_TERMS)
        return -ENOENT;
    
    // else it is a valid object id, fill in values
    clients[ret].open       = true;
    clients[ret].rw_mode    = flags;
    clients[ret].client     = client;
    clients[ret].object_id  = object_id;
    clients[ret].seekpos    = 0;
    clients[ret].client_sem = L4SEMAPHORE_UNLOCKED;

    // if the client wants to open with read permission,
    // we need to set up a worker thread
    if ( flags & O_WRONLY )
    {
        clients[ret].worker = L4_INVALID_ID;
    }
    else
    {
        l4thread_t temp = l4thread_create(term_read, NULL,
                                          L4THREAD_CREATE_SYNC);
        if (temp < 0)
        {
            LOG("FAILED CREATING WORKER THREAD: %s, %d!",
                l4env_strerror(-temp), -temp);
            return -ENOMEM;
        }
        clients[ret].worker = l4thread_l4_id(temp);
    }

    return ret;
}

int clientstate_close(int handle, l4_threadid_t client)
{
    // valid object handle ?
    if (handle < 0 || handle > MAX_CLIENTS)
        return -EBADF;

    // closing a non-open file ?
    if (!clients[handle].open)
        return -EBADF;
    // someone closing another task`s terminal?
    if (!l4_task_equal(clients[handle].client, client ))
        return -EBADF;

    // else closing is ok
    free_clientstate(handle);
    return 0;
}
