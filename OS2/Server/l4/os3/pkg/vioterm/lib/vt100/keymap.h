/**
 * \file   l4vfs/term_server/lib/vt100/keymap.h
 * \brief  
 *
 * \date   2004-08-10
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#ifndef __L4VFS_TERM_SERVER_LIB_VT100_KEYMAP_H_
#define __L4VFS_TERM_SERVER_LIB_VT100_KEYMAP_H_

typedef unsigned char * keymap_t[128][2];
extern keymap_t vt100_keymap_us;
extern keymap_t vt100_keymap_de;
extern keymap_t * vt100_keymap;     // keymap defining scancode ->
                                    // ascii translation

#endif   // __L4VFS_TERM_SERVER_LIB_VT100_KEYMAP_H__
