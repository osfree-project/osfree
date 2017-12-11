/**
 * \file   l4vfs/term_server/include/con_term.h
 * \brief  
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#ifndef __L4VFS_TERM_SERVER_INCLUDE_CON_TERM_H_
#define __L4VFS_TERM_SERVER_INCLUDE_CON_TERM_H_

#include <dice/dice.h>
#include <l4/sys/types.h>
#include <l4/semaphore/semaphore.h>

#include <l4/term_server/vt100.h>

/* This file describes data intended to be used by a terminal server
 * communicating with con.
 */

typedef struct termstate_spec_s
{
    unsigned      accel_flags;
    l4_threadid_t con_l4id;      // maybe we want to have several cons later
    l4_threadid_t vtc_l4id;      // we want several terminals
    unsigned int  font_w;        // font size
    unsigned int  font_h;

    // event stuff
    l4_threadid_t evh_l4id;	 // L4 thread id of key event handler
    l4_threadid_t btn_l4id;      // L4 thread id for the key repeat thread
    l4thread_t    evh_tid;       // for shutdown, l4env threadids
    l4thread_t    btn_tid;
    unsigned      key_code, key_repeat, key_pending;
} termstate_spec_t;

void con_term_close(termstate_t * term);
termstate_t * con_term_open(long max_sbuf_size, int hist);

#endif
