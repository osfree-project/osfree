/**
 * \file   l4vfs/term_server/lib/dope_term/internal.h
 * \brief  
 *
 * \date   08/10/2004
 * \author Björn Döbel  <doebel@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#ifndef __L4VFS_TERM_SERVER_LIB_DOPE_TERM_INTERNAL_H_
#define __L4VFS_TERM_SERVER_LIB_DOPE_TERM_INTERNAL_H_

#include <l4/term_server/vt100.h>
#include <l4/term_server/dope_term.h>
#include <l4/dope/dopelib.h>
#include <l4/log/l4log.h>

#include "keys.h"

// getchar
int term_getchar( termstate_t *term );

// for nonblocking io we will need: fixme
// int term_trygetchar( termstate_t *term )

// thread func for a new dope event loop
void dope_term_eventloop( void *app );

// add a key to key buffer
void add_key( termstate_t *t, int keycode );

// keypress callback function
extern void dope_term_press_callback( dope_event *, void * );
// key repeat callback function
extern void dope_term_repeat_callback( dope_event *, void * );
// scrollbar callback function
extern void dope_term_scroll_callback( dope_event *, void * );

// scrollbar update func
void update_scrollbar(termstate_t *t);

#endif
