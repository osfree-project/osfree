/**
 * \file   l4vfs/term_server/server/vc_server/init.c
 * \brief  init vc_server and DOpE
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
// dope headers
#include <l4/dope/dopelib.h>
#include <l4/dope/vscreen.h>
#include <l4/log/l4log.h>

#include <l4/l4vfs/types.h>

#include "config.h"
#include "term.h"
#include "basic_io.h"

char LOG_tag[9] = "vcserver";

const int l4thread_max_threads = 100;

// this function is meant to provide private
// initializations for the terminal server
int term_server_init(void)
{
    int ret;

    myvolume = VC_SERVER_VOLUME_ID;

    // for we are going to handle dope terminals
    // we initialize DOpE here
    ret = dope_init();
    LOG("Initialized DOpE: %d", ret);

    return ret;
}
