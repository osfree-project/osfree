/**
 * \file   l4vfs/term_server/lib/con_term/open.c
 * \brief  open a terminal
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/l4con/l4con-client.h>
#include <l4/l4con/l4con.h>
#include <l4/term_server/con_term.h>
#include <l4/term_server/vt100.h>
#include <l4/env/errno.h>
#include <l4/log/l4log.h>
#include <l4/names/libnames.h>
#include <l4/semaphore/semaphore.h>

#include <stdlib.h>

#include "lib.h"

//****************************************************************************/
/**
 * \brief   Open a connection to a con and init data
 *
 * \param   max_sbuf_size  ... max IPC string buffer
 * \param   hist           ... number of additional screenbuffer lines
 *
 * This is the open-function of libcon_term. It opens a console, allocates
 * data and returns a handle.
 */
/*****************************************************************************/
termstate_t * con_term_open(long max_sbuf_size, int hist)
{
    l4_int8_t gmode;
    unsigned bits_per_pixel, bytes_per_pixel, bytes_per_line;
    int xres, yres;
    CORBA_Environment env = dice_default_environment;
    int prio, ret;
    termstate_t * term;
  
    // get new termstate
    term = malloc(sizeof(termstate_t));
    if (! term)
    {
        LOGl("malloc for new term failed");
        return NULL;
    }
    // explizitely init. these, as they may be freed in init_termstate
    term->text = NULL;
    term->attrib = NULL;

    // get new termstate_con
    term->spec = malloc(sizeof(termstate_spec_t));
    if (! term->spec)
    {
        free(term);
        LOGl("malloc for new term failed");
        return NULL;
    }

    // ask for 'con'
    if (! names_waitfor_name(CON_NAMES_STR, &(term->spec->con_l4id),
                             CONTXT_TIMEOUT))
    {
        LOGl("names failed");
        free(term->spec);
        free(term);
        return NULL;
    }
  
    if ((prio = l4thread_get_prio(l4thread_myself())) < 0)
        prio = L4THREAD_DEFAULT_PRIO;

    if (con_if_openqry_call(&(term->spec->con_l4id), max_sbuf_size, 0, 0, prio,
                            &(term->spec->vtc_l4id), CON_NOVFB, &env)
        || (DICE_HAS_EXCEPTION(&env)))
    {
        LOGl("openqry failed (exc=%d)", DICE_EXCEPTION_MAJOR(&env));
        free(term->spec);
        free(term);
        return NULL;
    }

    term->spec->key_pending = 0;
    
    // init event stuff
    // removed __shift, __alt, __altgr, __ctrl as they
    // are initialized by vt100 lib now

    term->termsem = L4SEMAPHORE_UNLOCKED;
    l4semaphore_down(&term->termsem);


    if (evh_init(term) < 0)
    {
        l4semaphore_up(&term->termsem);
        free(term->spec);
        free(term);
        return NULL;
    }

    if (con_vc_smode_call(&(term->spec->vtc_l4id), CON_INOUT,
                          &(term->spec->evh_l4id), &env)
        || (DICE_HAS_EXCEPTION(&env)))
    {
        LOGl("smode failed (exc=%d)", DICE_EXCEPTION_MAJOR(&env));
        l4semaphore_up(&term->termsem);
        free(term->spec);
        free(term);
        return NULL;
    }
 
    if(con_vc_graph_gmode_call(&(term->spec->vtc_l4id), &gmode,
                               &xres, &yres,
                               &bits_per_pixel, &bytes_per_pixel,
                               &bytes_per_line,
                               &(term->spec->accel_flags),
                               &(term->spec->font_w), &(term->spec->font_h),
                               &env)
       || (DICE_HAS_EXCEPTION(&env)))
    {
        LOGl("gmode failed (exc=%d)", DICE_EXCEPTION_MAJOR(&env));
        l4semaphore_up(&term->termsem);
        free(term->spec);
        free(term);
        return NULL;
    }
    LOG("res: %dx%d", xres, yres);

    // init termstate
    if ((ret = init_termstate(term, xres / term->spec->font_w,
                              yres / term->spec->font_h, hist)))
    {
        l4semaphore_up(&term->termsem);
        free(term->spec);
        free(term);
        return NULL;
    }

    l4semaphore_up(&term->termsem);
    return term;
}
