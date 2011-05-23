/**
 * \file   l4vfs/term_server/server/vc_server/term.c
 * \brief  open/close dope terminal
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <stdlib.h>
#include <stdio.h>

#include <l4/dope/dopelib.h>
#include <l4/dope/vscreen.h>
#include <l4/names/libnames.h>
#include <l4/log/l4log.h>
#include <l4/term_server/dope_term.h>
#include <l4/term_server/vt100.h>

#include "basic_io.h"
#include "term.h"
#include "config.h"

extern term_object_t terms[MAX_TERMS];
extern int _DEBUG;

// open a terminal
// NOTE: This is only called, when a new terminal really
// has to be opened.
int term_open(l4_threadid_t thread, const object_id_t *object_id)
{
    int term = object_id->object_id - 1;
    char namebuf[25];
    char *winvar = "win";
    char *widgetvar = "term";
    long newapp;

    snprintf(namebuf, 25, "Virtual Console %d", term);
    newapp = dope_init_app(namebuf);

    LOGd(_DEBUG, "newapp: %ld", newapp);

    if (newapp >= 0)
    {
        dope_cmdf( newapp, "%s = new Window()", winvar );
        terms[term].terminal = dope_term_open(newapp, widgetvar,
                                              INIT_WIDTH, INIT_HEIGHT, __opt_history);
        terms[term].refcount = 1;
        terms[term].window   = winvar;

        // check if char buffer was mapped correctly
        if (!terms[term].terminal->spec->charbuf)
        {
            LOG("WARNING: could not map char buffer for vts.");
            return -1;
        }

        dope_cmdf(newapp, "%s.set( -background off -content %s)",
                  winvar, widgetvar);
        dope_cmdf(newapp, "%s.set( -x %d -y %d )", winvar, 150 + term * 20,
                  150 + term * 20);
        dope_cmdf(newapp, "%s.open()", terms[term].window);

        LOG("opened: %s", terms[term].window);

        return 0;
    }

    return newapp;
}

// close terminal
int term_close(local_object_id_t object)
{
    int t = object-1;
    termstate_t *term = terms[t].terminal;

    LOGd(_DEBUG, "term to close: %d", t);
    dope_cmdf(term->spec->app_id, "%s.close()", terms[t].window);
    dope_deinit_app(term->spec->app_id);
    return dope_term_close(term);
}
