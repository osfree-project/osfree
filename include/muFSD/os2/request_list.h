//
// $Header: /cur/cvsroot/boot/include/muFSD/os2/request_list.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
//

// 32 bits OS/2 device driver and IFS support. Provides 32 bits kernel 
// services (DevHelp) and utility functions to 32 bits OS/2 ring 0 code 
// (device drivers and installable file system drivers).
// Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef __reqlist_h
#define __reqlist_h

#pragma pack(1)
#include <strat2.h>
#pragma pack()



#pragma pack(1)

union _Private {

    /*
     * Page I/O private part of request list
     */
    struct _PG_Private {
        struct reqlist        *reqlist;      // reqlist ptr - to be able to free it when notified by OS2DASD of completion
        magic_t                magic;        // Magic signature always equal to PG_PRIVATE_MAGIC
        struct PageCmdHeader  *list;         // Corresponding PageCmdList structure
        struct PageCmd        *cmd[1];       // Corresponding PageCmd structure(s)
    } pages;

    /*
     * buffer_head I/O private part of request list
     */
    struct RW_private {
        struct reqlist        *reqlist;      // reqlist ptr - to be able to free it when notified by OS2DASD of completion
        magic_t                magic;        // Magic signature always equal to RW_PRIVATE_MAGIC
        int                    nb_bh;        // Number of buffer_head structure(s)
        struct buffer_head    *bh[1];        // Corresponding buffer_head structure(s)
    } buffers;
};

#define SZ_ONE_REQ (                                \
                    sizeof(struct _PB_Read_Write) + \
                    sizeof(struct _SG_Descriptor) + \
                    sizeof(union  _Private)         \
                   )
    struct reqlist_hdr {
        struct reqlist          *s_next;
        struct reqlist          *s_prev;
        struct reqlist         **s_list;
        struct _PB_Read_Write   *s_last_RLE;       // Offset of last RLE
        magic_t                  s_magic;          //  Magic signature always equal to REQLIST_MAGIC
	PTR16			 s_self_virt;	   // Virtual address of request list
//        char                    *s_self;           // Linear  address of request list
        struct _Req_List_Header *s_self;           // Linear  address of request list
    };

struct reqlist {
    struct reqlist_hdr s;
    struct _Req_List_Header  s_rwlist;
    char buf[32 * SZ_ONE_REQ];			   // Room for 32 requests of one block each
};
#pragma pack()

#endif /* __reqlist_h */
