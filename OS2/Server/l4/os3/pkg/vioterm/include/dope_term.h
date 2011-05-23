/**
 * \file   l4vfs/term_server/include/dope_term.h
 * \brief  
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#ifndef __L4VFS_TERM_SERVER_INCLUDE_DOPE_TERM_H_
#define __L4VFS_TERM_SERVER_INCLUDE_DOPE_TERM_H_

#include <l4/sys/types.h>
#include <l4/term_server/vt100.h>
#include <dice/dice.h>
#include <l4/semaphore/semaphore.h>

#define KEYBUF_SIZE 30

typedef struct termstate_spec_s
{
    long app_id;                  // dope application id
    char *widget_name;            // DOpE internal widget name
                                  // users of the widget may thus use
                                  // it within their own dope_cmd commands

    char *charbuf;                // vtextscreen character buffer
    char *attrbuf;                // vtextscreen attribute buffer

    l4_threadid_t evh_l4id;       // dope event handler thread id
    l4thread_t    evh_tid;

    l4semaphore_t keysem;
} termstate_spec_s;

//termstate_t * dope_term_open( int height, int width, int hist );
termstate_t * dope_term_open( long dope_appid, char *widget_name, 
        int width, int height, int hist);
int dope_term_close( termstate_t *term );

#endif
