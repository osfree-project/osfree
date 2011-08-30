/**
 * \file   l4vfs/term_server/server/term_con/term.c
 * \brief  open / close term_con console
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <stdlib.h>

#include <l4/log/l4log.h>
#include <l4/term_server/vt100.h>
#include <l4/term_server/con_term.h>

#include "basic_io.h"
#include "term.h"
#include "config.h"

extern int _DEBUG;
extern term_object_t terms[MAX_TERMS];

// open a terminal
//  -> if the specified vc is open already, just increment the
//  reference count
//  -> else open a new dope vscreen window
int term_open(l4_threadid_t thread, const object_id_t *object_id)
{
    int term = object_id->object_id - 1;

    terms[term].terminal    = con_term_open(MAXBUF, MAXHIST);
    terms[term].refcount    = 1;

    return 0;
}

// close an existing terminal
//  -> if the term is referenced more than once, just decrement
//  the reference count
//  -> else close the window and cleanup
int term_close(local_object_id_t object)
{
    int term = object - 1;
    con_term_close(terms[term].terminal);
    return 0;
}
