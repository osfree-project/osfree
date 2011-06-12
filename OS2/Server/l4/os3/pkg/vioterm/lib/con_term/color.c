/**
 * \file   l4vfs/term_server/lib/con_term/color.c
 * \brief  color map
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/l4con/l4con_pslim.h>
#include <l4/log/l4log.h>

#include "lib.h"

// copied colors from dope
l4con_pslim_color_t std_colors[24] = {
    // dark
    0x00000000,
    0x007f0000,
    0x00007f00,
    0x007f7f00,
    0x0000007f,
    0x007f007f,
    0x00007f7f,
    0x007f7f7f,
    // normal
    0x00000000,
    0x00cf0000,
    0x0000cf00,
    0x00cfcf00,
    0x000000cf,
    0x00cf00cf,
    0x0000cfcf,
    0x00cfcfcf,
    // bright
    0x00000000,
    0x00ff0000,
    0x0000ff00,
    0x00ffff00,
    0x000000ff,
    0x00ff00ff,
    0x0000ffff,
    0x00ffffff
};

// convert attributes to corresponding l4con_pslim_color_t
void attribs_to_colors(l4_int8_t a, l4con_pslim_color_t * fg,
                       l4con_pslim_color_t * bg)
{
    int fg_i, bg_i, in;

    unpack_attribs(a, &fg_i, &bg_i, &in);
    *fg = std_colors[fg_i + 8 * in];
    *bg = std_colors[bg_i + 8 * in];
}
