/**
 * \file   l4vfs/term_server/server/common/dirs.h
 * \brief  directory struct
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#ifndef __L4VFS_TERM_SERVER_SERVER_COMMON_DIRS_H_
#define __L4VFS_TERM_SERVER_SERVER_COMMON_DIRS_H_

#include <l4/sys/types.h>

typedef struct dir_t
{
    char *name;
    local_object_id_t object_id;
} dir_t;

#endif
