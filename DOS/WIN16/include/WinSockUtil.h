/*  WinSockUtil.h	1.18
    Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/

#include <sys/types.h>
#include "platform.h"

void TWIN_GetWSAError  ( void);
extern unsigned int  TWIN_WSAError;

#define TWIN_CHECKDATAPENDING    0x0020

/* This struct is used by the GetDB Routines */
struct get_db_info
{
       const char *db_char_arg1;
       const char *db_char_arg2;
       int 	db_arg2;
       int 	db_arg3;
};
struct SelectInfo
  {
    fd_set *read_fds_info;
    fd_set *write_fds_info;
    fd_set *except_fds_info;
    struct timeval *Select_time_out;
    unsigned int         sock;
  };
struct sock_data
   {
     unsigned int  sock;
     char   *SockData;
     int    DataLen;
     int    DataFlags;
     struct sockaddr *from_addr;
     int    *from_len;
     int    to_len;
   }; 

