/*
 * C DOS native interface
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

#include <stdio.h>
#include <dos.h>
#include "dos_func.h"
#include <string.h>
#include <win16.h>
/*
 * DOS 2+ internal - GET CURRENT PROCESS ID (GET PSP ADDRESS)
 * AH = 51h
 *
 * RETURNS:
 *   BX = segment of PSP for current process
 */
int dos_get_psp(unsigned *seg)
{
    union REGS r;
    memset(&r, 0, sizeof(union REGS));  //initialize fields to 0 
    r.h.ah = 0x51;
    intdos(&r, &r);
    *seg = r.x.bx;
    return !r.x.cflag;
}


/* DOS 2+ - CREAT - CREATE OR TRUNCATE FILE
 * 
 * AH = 3Ch
 * CX = file attributes (see #01401)
 * DS:DX -> ASCIZ filename
 * 
 * Return:
 *   CF clear if successful
 *     AX = file handle 
 *   CF set on error
 *     AX = error code (03h,04h,05h) (see #01680 at AH=59h/BX=0000h)
 */
int dos_creat(char far * fn, unsigned attrib, unsigned *ret)
{
    union REGS r;
    struct SREGS sr;
    memset(&r, 0, sizeof(union REGS));  //initialize fields to 0 
    memset(&sr, 0, sizeof(struct SREGS));
    r.h.ah = 0x3C;
    r.x.cx = attrib;
    r.x.dx = FP_OFF(fn);
    sr.ds = FP_SEG(fn);
    intdosx(&r, &r, &sr);
    *ret = r.x.ax;
    return !r.x.cflag;
}


/* DOS 2+ - OPEN - OPEN EXISTING FILE
 *
 * AH = 3Dh
 * AL = access and sharing modes (see #01402)
 * DS:DX -> ASCIZ filename
 * CL = attribute mask of files to look for (server call only)
 * 
 * Return:
 * CF clear if successful 
 * AX = file handle 
 * CF set on error 
 * AX = error code (01h,02h,03h,04h,05h,0Ch,56h) (see #01680 at AH=59h)
 */
int dos_open(char far * fn, short access, short attrib, unsigned *ret)
{
    union REGS r;
    struct SREGS sr;
    memset(&r, 0, sizeof(union REGS));  //initialize fields to 0 
    memset(&sr, 0, sizeof(struct SREGS));
    r.h.ah = 0x3D;
    r.h.al = access;
    r.h.cl = attrib;
    r.x.dx = FP_OFF(fn);
    sr.ds = FP_SEG(fn);
    intdosx(&r, &r, &sr);
    *ret = r.x.ax;
    return !r.x.cflag;
}


/* DOS 2+ - CLOSE - CLOSE FILE
 * 
 * AH = 3Eh
 * BX = file handle
 * 
 * Return:
 * CF clear if successful 
 * AX destroyed 
 * CF set on error 
 * AX = error code (06h) (see #01680 at AH=59h/BX=0000h)
 */
int dos_close(unsigned fh, unsigned *ret)
{
    union REGS r;
    memset(&r, 0, sizeof(union REGS));  //initialize fields to 0 
    r.h.ah = 0x3E;
    r.x.bx = fh;
    intdos(&r, &r);
    *ret = r.x.ax;
    return !r.x.cflag;
}


/* DOS 2+ - UNLINK - DELETE FILE
 * 
 * AH = 41h
 * DS:DX -> ASCIZ filename (no wildcards, but see notes)
 * CL = attribute mask for deletion (server call only, see notes)
 * 
 * Return:
 * CF clear if successful 
 * AX destroyed (DOS 3.3) 
 * AL seems to be drive of deleted file 
 * CF set on error 
 * AX = error code (02h,03h,05h) (see #01680 at AH=59h/BX=0000h)
 */
int dos_unlink(char far * fn, short attrib, unsigned *ret)
{
    union REGS r;
    struct SREGS sr;
    memset(&r, 0, sizeof(union REGS));
    memset(&sr, 0, sizeof(struct SREGS));
    r.h.ah = 0x41;
    r.h.cl = attrib;
    r.x.dx = FP_OFF(fn);
    sr.ds = FP_SEG(fn);
    intdosx(&r, &r, &sr);
    *ret = r.x.ax;
    return !r.x.cflag;
}


/* DOS 2+ - RENAME - RENAME FILE
 * 
 * AH = 56h
 * DS:DX -> ASCIZ filename of existing file (no wildcards, but see below)
 * ES:DI -> ASCIZ new filename (no wildcards)
 * CL = attribute mask (server call only, see below)
 * 
 * Return:
 * CF clear if successful 
 * CF set on error 
 * AX = error code (02h,03h,05h,11h) (see #01680)
 */
int dos_rename(char far * from, char far * to, short attrib, unsigned *ret)
{
    union REGS r;
    struct SREGS sr;
    memset(&r, 0, sizeof(union REGS));
    memset(&sr, 0, sizeof(struct SREGS));
    r.h.ah = 0x56;
    r.h.cl = attrib;
    sr.ds = FP_SEG(from);
    r.x.dx = FP_OFF(from);
    sr.es = FP_SEG(to);
    r.x.di = FP_OFF(to);
    intdosx(&r, &r, &sr);
    *ret = r.x.ax;
    return !r.x.cflag;
}


/* DOS 2+ - DUP - DUPLICATE FILE HANDLE
 * 
 * AH = 45h
 * BX = file handle
 * 
 * Return:
 * CF clear if successful 
 * AX = new handle 
 * CF set on error 
 * AX = error code (04h,06h) (see #01680 at AH=59h/BX=0000h)
 */
int dos_dup(unsigned fh, unsigned *ret)
{
    union REGS r;
    memset(&r, 0, sizeof(union REGS));
    r.h.ah = 0x45;
    r.x.bx = fh;
    intdos(&r, &r);
    *ret = r.x.ax;
    return !r.x.cflag;
}


/* DOS 2+ - DUP2, FORCEDUP - FORCE DUPLICATE FILE HANDLE
 * 
 * AH = 46h
 * BX = file handle
 * CX = file handle to become duplicate of first handle
 * 
 * Return:
 * CF clear if successful 
 * CF set on error 
 * AX = error code (04h,06h) (see #01680 at AH=59h/BX=0000h)
 */
int dos_dup2(unsigned fh, unsigned fh2, unsigned *ret)
{
    union REGS r;
    memset(&r, 0, sizeof(union REGS));
    r.h.ah = 0x46;
    r.x.bx = fh;
    r.x.cx = fh2;
    intdos(&r, &r);
    *ret = r.x.ax;
    return !r.x.cflag;
}


/* DOS 3.3+ - SET HANDLE COUNT
 * 
 * AH = 67h
 * BX = size of new file handle table for process
 * 
 * Return:
 * CF clear if successful 
 * CF set on error 
 * AX = error code (see #01680 at AH=59h/BX=0000h)
 */
int dos_set_handle_count(unsigned size, unsigned *ret)
{
    union REGS r;
    memset(&r, 0, sizeof(union REGS));
    r.h.ah = 0x67;
    r.x.bx = size;
    intdos(&r, &r);
    *ret = r.x.ax;
    return !r.x.cflag;
}


/* DOS 2+ - READ - READ FROM FILE OR DEVICE
 * 
 * AH = 3Fh
 * BX = file handle
 * CX = number of bytes to read
 * DS:DX -> buffer for data
 * 
 * Return:
 * CF clear if successful 
 * AX = number of bytes actually read (0 if at EOF before call) 
 * CF set on error 
 * AX = error code (05h,06h) (see #01680 at AH=59h/BX=0000h)
 */
int dos_read(unsigned handle, char far * buff, unsigned bytes, unsigned *ret)
{
    union REGS r;
    struct SREGS sr;
    memset(&r, 0, sizeof(union REGS));
    memset(&sr, 0, sizeof(struct SREGS));
    r.h.ah = 0x3f;
    r.x.bx = handle;
    r.x.cx = bytes;
    sr.ds = FP_SEG(buff);
    r.x.dx = FP_OFF(buff);
    intdosx(&r, &r, &sr);
    *ret = r.x.ax;
    return !r.x.cflag;
}

/* DOS 2+ - WRITE - WRITE TO FILE OR DEVICE
 * 
 * AH = 40h
 * BX = file handle
 * CX = number of bytes to write
 * DS:DX -> data to write
 * 
 * Return:
 * CF clear if successful 
 * AX = number of bytes actually written 
 * CF set on error 
 * AX = error code (05h,06h) (see #01680 at AH=59h/BX=0000h)
 */
int dos_write(unsigned handle, char far * buff, unsigned bytes, unsigned *ret)
{
    union REGS r;
    struct SREGS sr;
    memset(&r, 0, sizeof(union REGS));
    memset(&sr, 0, sizeof(struct SREGS));
    r.h.ah = 0x40;
    r.x.bx = handle;
    r.x.cx = bytes;
    sr.ds = FP_SEG(buff);
    r.x.dx = FP_OFF(buff);
    intdosx(&r, &r, &sr);
    *ret = r.x.ax;
    return !r.x.cflag;
}

/* DOS 2+ - LSEEK - SET CURRENT FILE POSITION
 * 
 * AH = 42h
 * AL = origin of move 00h start of file 01h current file position 02h end of file
 * BX = file handle
 * CX:DX = (signed) offset from origin of new file position
 * 
 * Return:
 * CF clear if successful 
 * DX:AX = new file position in bytes from start of file 
 * CF set on error 
 * AX = error code (01h,06h) (see #01680 at AH=59h/BX=0000h)
 */
int dos_lseek(unsigned handle, long offset, short origin, long *ret)
{
    union REGS r;
    memset(&r, 0, sizeof(union REGS));
    r.h.ah = 0x42;
    r.h.al = origin;
    r.x.bx = handle;

    r.x.cx = HIWORD(offset);
    r.x.dx = LOWORD(offset);
    intdos(&r, &r);
    if (r.x.cflag)
        *ret = MAKELONG(r.x.ax, 0);
    else
        *ret = MAKELONG(r.x.ax, r.x.dx);

    return !r.x.cflag;
}


/* INT 21 - DOS 2+ - IOCTL - GET DEVICE INFORMATION
 * AX = 4400h
 * BX = handle
 * 
 * Return:
 * CF clear if successful 
 * DX = device information word (see #01423) 
 * AX destroyed 
 * CF set on error 
 * AX = error code (01h,05h,06h) (see #01680 at AH=59h/BX=0000h)
 */
int dos_get_device_info(unsigned handle, unsigned *ret)
{
    union REGS r;
    memset(&r, 0, sizeof(union REGS));
    r.x.ax = 0x4400;
    r.x.bx = handle;
    intdos(&r, &r);

    if (r.x.cflag)
        *ret = r.x.ax;
    else
        *ret = r.x.dx;

    return !r.x.cflag;
}
