/**
 * \file   l4vfs/term_server/lib/dope_term/event.c
 * \brief  function to run the dope event loop
 *
 * \date   08/10/2004
 * \author Björn Döbel  <doebel@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/thread/thread.h>
#include <l4/util/l4_macros.h>
#include <l4/sys/ipc.h>

#include <l4/dope/dopelib.h>
#include <l4/dope/vscreen.h>

#include <l4/term_server/vt100.h>
#include <l4/term_server/dope_term.h>

#include "internal.h"

extern int _DEBUG;

void dope_term_eventloop( void *app )
{
    l4thread_started(NULL);

    dope_eventloop(*(long *)app);

    return; // probably never
}
