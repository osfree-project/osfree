/**
 * \file   l4vfs/term_server/lib/vt100/init.c
 * \brief  vt100 initialization
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/term_server/vt100.h>
#include <l4/log/l4log.h>
#include <l4/semaphore/semaphore.h>

#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "keymap.h"

keymap_t * vt100_keymap = &vt100_keymap_us;

/* init. a termstate, does not malloc term->spec,
 * but term->text and term->attrib.
 */
int init_termstate(termstate_t * term, int w, int h, int hist)
{
    term->w                 = w;
    term->echo              = 1;
    term->autorepeat        = 1;
    term->autowrap          = 1;
    term->term_mode         = VT100MODE_COOKED;
    term->origin_mode       = VT100_ORIGIN_GLOBAL;
    term->insert_mode       = 0;
    term->keylist_next_write= 0;
    term->keylist_next_read = 0;
//    term->key_pending       = 0;
//    term->__key_init        = 0;
    term->keysem            = L4SEMAPHORE_LOCKED;
    term->keybufsem         = L4SEMAPHORE_UNLOCKED;
    term->__shift           = 0;
    term->__alt             = 0;
    term->__altgr           = 0;
    term->__ctrl            = 0;
    term->__keydown         = 0;
    term->returns           = 0;
    term->hist_len          = 0;
    term->phys_h            = h;
    term->virt_h            = term->phys_h + hist;
    term->phys_off          = 0;
    term->vis_off           = 0;
    term->cur_x             = 0;
    term->cur_y             = 0;
    term->attrib_mode       = pack_attribs(DEFAULT_FG, DEFAULT_BG, 1);
    term->inverse           = 0;
    term->decscnm           = 0;
    term->esc_state         = ESnormal;
    term->used_par          = 0;
    term->cursor_vis        = 1;
    term->cur_stored_x      = 0;
    term->cur_stored_y      = 0;
    term->cur_stored_attr   = term->attrib_mode;
    term->newline           = 1;
    term->ques              = 0;
    term->scroll_top        = 0;
    term->scroll_bottom     = term->phys_h - 1;
    term->tab_last          = -1;
    term->select_list       = NULL;

    LOG("cols:%d, lines:%d, hist:%d", term->w, term->phys_h, hist);

    free(term->text);
    term->text = malloc(term->w * term->virt_h);
    if (! term->text)
    {
        LOGl("no mem for text (%d)", term->w * term->virt_h);
        return -1;
    }
    memset(term->text, ' ', term->w * term->virt_h);

    free(term->attrib);
    term->attrib = malloc(term->w * term->virt_h);
    if (! term->attrib)
    {
        LOGl("no mem for attrib (%d)", term->w * term->virt_h);
        free(term->text);
        return -2;
    }
    memset(term->attrib, term->attrib_mode, term->w * term->virt_h);

    return 0;
}

void vt100_set_keymap(char * keymap)
{
    if (keymap == NULL || strncmp(keymap, "us", 2) == 0)
    {
        vt100_keymap = &vt100_keymap_us;
    }
    else if (strncmp(keymap, "de", 2) == 0)
    {
        vt100_keymap = &vt100_keymap_de;
    }
    else  // provide a default mapping
    {
        vt100_keymap = &vt100_keymap_us;
    }
}
