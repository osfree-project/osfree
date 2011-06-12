/**
 * \file   l4vfs/term_server/server/common/basic_io.h
 * \brief  
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#ifndef __L4VFS_TERM_SERVER_SERVER_COMMON_BASIC_IO_H_
#define __L4VFS_TERM_SERVER_SERVER_COMMON_BASIC_IO_H_

#include <unistd.h>

#include <l4/sys/types.h>
#include <l4/thread/thread.h>
#include <l4/semaphore/semaphore.h>

#include <l4/l4vfs/types.h>
#include <l4/term_server/vt100.h>

#define MAX_TERMS   4
#define MAX_CLIENTS 100

#define MAX(x, y) ((x) > (y) ? (x) : (y))

typedef struct
{
    int                 open;           // is it open?
    int                 rw_mode;        // rw flags
    l4_threadid_t       client;         // owning client
    l4_threadid_t       worker;         // worker thread for this clientstate
    local_object_id_t   object_id;      // object id
    int                 seekpos;        // current seek position
    l4semaphore_t       client_sem;     // semaphore to protect this
                                        // client struct
} clientstate_t;

typedef struct
{
    termstate_t *terminal;              // the termstate
    int         refcount;               // reference counter
    char        *window;                // DOpE internal window name
} term_object_t;

extern clientstate_t clients[MAX_CLIENTS];   // list of our clients
extern term_object_t terms[MAX_TERMS];       // list of our terminals

extern volume_id_t myvolume;            // the servers volume id
                                        // (needed for checks)

// get first free entry in clients table
int  get_free_clientstate(void);
// free a client table entry
void free_clientstate(int handle);
// init a client state
int  clientstate_open(int flags, l4_threadid_t client,
                       local_object_id_t object_id);
// deinit a client state
int  clientstate_close(int handle, l4_threadid_t client);

#endif
