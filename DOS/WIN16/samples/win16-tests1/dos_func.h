/*
 * Header for C DOS native interface
 *
 * Copyright (c) 2007 Google, Jennifer Lai
 * Copyright (c) 2004 Markus Amsler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <dos.h>

#ifndef __DOS_FUNCS_H
#define __DOS_FUNCS_H

#define LONG long

#define LOWORD(l)              ((WORD)(DWORD)(l))

#define MAKEWORD(low,high)     ((WORD)(((BYTE)(low)) | ((WORD)((BYTE)(high))) << 8))
#define MAKELONG(low,high)     ((LONG)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))

/* DOS File attributes */
#define AT_READONLY 0x01
#define AT_HIDDEN   0x02
#define AT_SYSTEM   0x04
#define AT_ARCHIVE  0x20

/* DOS File Sharing modes */
#define FS_DENYALL   0x10
#define FS_DENYWRITE 0x20
#define FS_DENYREAD  0x30
#define FS_DENYNONE  0x40

/* DOS seek modes */
#define FILE_BEGIN              0
#define FILE_CURRENT            1
#define FILE_END                2

/* DOS device info */
#define IOCTL_DEVICE   0x80
#define DEVICE_STDIN   0x01
#define DEVICE_STDOUT  0x02
#define DEVICE_NUL     0x04
#define DEVICE_CLOCK   0x08
#define DEVICE_SPECIAL 0x10
#define DEVICE_RAW     0x20
#define DEVICE_EOF     0x40

/*
14    device driver can process IOCTL requests (see AX=4402h"DOS 2+")
13    output until busy supported
11    driver supports OPEN/CLOSE calls
8    ??? (set by MS-DOS 6.2x KEYB)
7    set (indicates device)
6    EOF on input
5    raw (binary) mode
4    device is special (uses INT 29)
3    clock device
2    NUL device
1    standard output
0    standard input
*/
int dos_get_psp(unsigned *);
int dos_creat(char far *, unsigned, unsigned *);
int dos_open(char far *, short, short, unsigned *);
int dos_close(unsigned, unsigned *);
int dos_unlink(char far *, short, unsigned *);
int dos_rename(char far *, char far *, short attrib, unsigned *);
int dos_dup(unsigned, unsigned *);
int dos_dup2(unsigned, unsigned, unsigned *);
int dos_set_handle_count(unsigned, unsigned *);
int dos_read(unsigned, char far *, unsigned, unsigned *);
int dos_write(unsigned, char far *, unsigned, unsigned *);
int dos_lseek(unsigned, long, short, long *);
int dos_get_device_info(unsigned, unsigned *);


#endif
