/**
 * \file   l4vfs/term_server/server/common/term.h
 * \brief  
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#ifndef __L4VFS_TERM_SERVER_SERVER_COMMON_TERM_H_
#define __L4VFS_TERM_SERVER_SERVER_COMMON_TERM_H_

#include <l4/sys/types.h>
#include <l4/l4vfs/types.h>

// terminal options
extern int __opt_history;

int term_write(local_object_id_t object, l4_int8_t *buf, int count);
void term_read(void *argp);

// external function to do private initializations.
// must at least set the global var myvolume to the 
// server`s volume id
int term_server_init(void);

// these are wrapper functions and must be implemented by the
// specific server to perfom the actions either on dope_term or
// con_term
int term_open(l4_threadid_t thread, const object_id_t *object_id);
int term_close(local_object_id_t object);

#endif
