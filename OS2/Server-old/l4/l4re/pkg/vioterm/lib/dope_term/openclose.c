/**
 * \file   l4vfs/term_server/lib/dope_term/openclose.c
 * \brief  open / close dope terminal
 *
 * \date   08/10/2004
 * \author Björn Döbel  <doebel@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/dope/dopelib.h>
#include <l4/dope/vscreen.h>
#include <l4/thread/thread.h>
#include <l4/util/l4_macros.h>
#include <l4/env/errno.h>

#include <l4/term_server/vt100.h>
#include <l4/term_server/dope_term.h>

#include <stdlib.h>

#include "internal.h"

extern int _DEBUG;


termstate_t * dope_term_open(long app, char *widgetname,
        int width, int height, int hist)
{
    // allocate memory for termstate and termstate_spec
    termstate_t *term = (termstate_t *)malloc( sizeof(termstate_t));
    termstate_spec_s *termspec =
        (termstate_spec_s *)malloc(sizeof(termstate_spec_s));

    // make sure the user does not use a name we are going to use
    // internally
    if (strcmp(widgetname, "v") == 0 ||
        strcmp(widgetname, "s") == 0)
        widgetname = "g";

    // create a new VTextScreen
//    dope_cmdf( app, "win = new Window()" );
    dope_cmd (app, "v = new VTextScreen(-grabfocus yes)");
    dope_cmdf(app, "v.setmode( %d, %d, C8A8PLN )", width, height);
    dope_cmd (app, "s = new Scrollbar()");
    dope_cmd (app, "s.set(-autoview off -orient vertical)");
    dope_cmdf(app, "s.set(-realsize %d -viewsize %d)", height, height);
    dope_cmdf(app, "%s = new Grid()", widgetname);
    dope_cmdf(app, "%s.place(v, -column 1 -row 1)", widgetname);
    dope_cmdf(app, "%s.place(s, -column 2 -row 1)", widgetname);

//    dope_cmdf( app, "win.set( -background off -content vts)");

    // init vt100 termstate, set up termsem before

    term->termsem = L4SEMAPHORE_UNLOCKED;
    l4semaphore_down( &term->termsem );

    term->text = NULL;
    term->attrib = NULL;
    init_termstate( term, width, height, hist);

    l4semaphore_up( &term->termsem );

    // init dope_term specific entries
    termspec->app_id = app;
    termspec->charbuf = vscr_get_fb( app, "v" );
    termspec->attrbuf = termspec->charbuf + width*height;
    termspec->widget_name = (char *)malloc(32);
    termspec->widget_name = strcpy(termspec->widget_name, widgetname);

    LOGd(_DEBUG,"charbuf mapped: %p", termspec->charbuf);

    term->spec = termspec;

    LOG("Term initialized");

    // bind keypress callback
    dope_bind( app, "v", "press", dope_term_press_callback, (void *)term );
    dope_bind( app, "v", "release", dope_term_press_callback, (void *)term );
    // bind key repeat callback
    dope_bind( app, "v", "keyrepeat", dope_term_repeat_callback, (void *)term );
    // bind scrollbar change callback
    dope_bind( app, "s", "change", dope_term_scroll_callback, (void *)term );

    LOG("callback registered");

    // create new thread running the dope event loop
    if ((term->spec->evh_tid = l4thread_create( dope_term_eventloop,
                                                (void *)&app,
                                                L4THREAD_CREATE_SYNC )) > 0 )
    {
        term->spec->evh_l4id = l4thread_l4_id(term->spec->evh_tid);
        LOG("event thread created: "l4util_idfmt,
            l4util_idstr(term->spec->evh_l4id));
//        term->__key_init = 1;
    }
    else
    {
        LOG("FAILED CREATING EVENT THREAD: %s, %d!",
            l4env_strerror(-(term->spec->evh_tid)), -(term->spec->evh_tid));
    }

    // return terminal state
    return term;
}

int dope_term_close( termstate_t *term )
{
    // free termstate memory
    l4thread_shutdown(term->spec->evh_tid);
    free(term->spec->widget_name);
    free(term);

    return 0;
}
