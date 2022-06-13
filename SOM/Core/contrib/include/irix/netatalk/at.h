/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/* 
 * $Id$
 */

#ifndef __NETATALK_AT_H__
#define __NETATALK_AT_H__
#ifdef __cplusplus
extern "C" {
#endif

struct sockaddr_at
{
    sa_family_t sat_family;
    struct
    {
        unsigned short s_net;
        unsigned char s_node;
    } sat_addr;
    unsigned char sat_port,sat_type;
    char pad[6];
};

#define KTALK_APPLETALK     1

#ifdef __cplusplus
}
#endif
#endif
