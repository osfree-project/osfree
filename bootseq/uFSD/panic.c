//
// $Header: /cur/cvsroot/boot/muFSD/panic.c,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
//

// 32 bits Linux ext2 file system driver for OS/2 WARP - Allows OS/2 to
// access your Linux ext2fs partitions as normal drive letters.
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

#include <stdarg.h>

#include <string.h>

#include <add.h>

#include <os2/types.h>
#include <microfsd.h>

static char panicmsg[] =
"                >>> EXT2-OS2 MICROFSD FATAL ERROR <<<                      \r\n"
"\r\n"
"%s\r\n"
"\r\n"
"Please write down the error message above and e-mail it to willm@ibm.net "
"along with your partition scheme (FDISK /QUERY) and a description "
"of the problem, then reboot your system and FIRST RUN E2FSCK FROM LINUX "
"ON ALL YOUR EXT2FS PARTITIONS.\r\n";

char scratch_buffer[1024];

void microfsd_panic(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vsprintf(scratch_buffer + 512, fmt, args);
    va_end(args);

    sprintf(scratch_buffer, panicmsg, scratch_buffer + 512);

    /*
     * Outputs the message to the screen using BIOS services
     */
    video_output(scratch_buffer, strlen(scratch_buffer));
    video_crlf();

    /*
     * Halts the system
     */
#ifndef MICROFSD_TEST
    __asm {
sleep:  hlt
        jmp sleep
    }
#else
    __asm {
        mov ax, 0x4C00
        int 21h
    }
#endif
}
