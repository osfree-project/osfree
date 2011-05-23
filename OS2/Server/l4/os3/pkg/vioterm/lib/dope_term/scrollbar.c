#include <stdlib.h>
#include "internal.h"

// found these in Christian's code ;)
void dope_term_scroll_callback( dope_event *e, void *arg )
{
    termstate_t *term = (termstate_t *)arg;

    int newpos, oldpos;
    char buf[16];

    dope_req(term->spec->app_id, buf, sizeof(buf), "s.offset");
    newpos = atoi(buf);
    oldpos = term->hist_len - term->vis_off;

    if (oldpos < newpos)
        vis_down(term, newpos - oldpos);
    else
        vis_up(term, oldpos - newpos);
}

void update_scrollbar( termstate_t *term )
{
    dope_cmdf(term->spec->app_id, "s.set(-realsize %d -offset %d)",
            term->hist_len + term->phys_h, term->hist_len - term->vis_off);
}
