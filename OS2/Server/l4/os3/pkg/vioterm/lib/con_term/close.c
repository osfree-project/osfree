/**
 * \file   l4vfs/term_server/lib/con_term/close.c
 * \brief  close con_term
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/term_server/con_term.h>
#include <l4/term_server/vt100.h>
#include <l4/l4con/l4con-client.h>
#include <l4/log/l4log.h>
#include <l4/thread/thread.h>

#include <stdlib.h>

void con_term_close(termstate_t * term)
{
    CORBA_Environment env = dice_default_environment;
  
    // close terminal
    con_vc_close_call(&(term->spec->vtc_l4id), &env);

    // shutdown helper threads
    if(l4thread_shutdown(term->spec->evh_tid))
        LOG("Problem shutting down Event Handler Thread!");
    if(l4thread_shutdown(term->spec->btn_tid))
        LOG("Problem shutting down Button Repeat Thread!");

    // free data
    free(term->text);
    free(term->attrib);
    free(term->spec);
    free(term);
}
