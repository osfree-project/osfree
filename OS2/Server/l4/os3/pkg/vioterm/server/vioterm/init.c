/**
 * \file   l4vfs/term_server/server/term_con/init.c
 * \brief  initialization of term_con
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#undef getchar
#undef putchar
#include <l4/log/l4log.h>
#include <l4/util/util.h>
#include <l4/term_server/con_term.h>

#include <l4/l4vfs/types.h>

#include "config.h"
#include "term.h"
#include "basic_io.h"

char LOG_tag[9] = "term_con";

l4_ssize_t l4libc_heapsize = 256 * 1024;

int term_server_init()
{
    // set up myvolume
    myvolume = TERM_CON_VOLUME_ID;

    return 0;
}
