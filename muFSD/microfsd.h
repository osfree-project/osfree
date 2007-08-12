//
// $Header: /cur/cvsroot/boot/muFSD/microfsd.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
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

#ifndef __microfsd_h
#define __microfsd_h

#include <os2/types.h>

/*
 * Data structure to be passed to OS2LDR
 */
struct FileTable {
    unsigned short ft_cfiles;
    unsigned short ft_ldrseg;
    unsigned long  ft_ldrlen;
    unsigned short ft_museg;
    unsigned long  ft_mulen;
    unsigned short ft_mfsseg;
    unsigned long  ft_mfslen;
    unsigned short ft_ripseg;
    unsigned long  ft_riplen;
    unsigned short __cdecl (*ft_muOpen)(char far *pName, unsigned long far *pulFileSize);
    unsigned short seg1;
    unsigned long  __cdecl (*ft_muRead)(long loffseek, char far *pBuf, unsigned long cbBuf);
    unsigned short seg2;
    void           __cdecl (*ft_muClose)(void);
    unsigned short seg3;
    void           __cdecl (*ft_muTerminate)(void);
    unsigned short seg4;
};

extern void _pascal video_output(char *msg, unsigned short msglen);
extern void _pascal video_crlf(void);
extern void _pascal video_init(void);

int sprintf(char * buf, const char *fmt, ...);

/*
 * Standard micro-fsd entry points
 */
unsigned short __cdecl mu_Open(char far *pName, unsigned long far *pulFileSize);
unsigned short __cdecl stub_mu_Open(char far *pName, unsigned long far *pulFileSize);
unsigned long  __cdecl mu_Read(long loffseek, char far *pBuf, unsigned long cbBuf);
unsigned long  __cdecl stub_mu_Read(long loffseek, char far *pBuf, unsigned long cbBuf);
void __cdecl mu_Close(void);
void __cdecl stub_mu_Close(void);
void __cdecl mu_Terminate(void);
void __cdecl stub_mu_Terminate(void);

void read_super(void);

/*
 * Current microfsd segment register
 */
extern unsigned short current_seg;

extern void microfsd_panic(const char *format, ...);

/*
 * pathutil.c
 */
char *DecoupePath(char *path, char *component);

//void buffer_init(int blocksize);

/*
 * From vsprintf.c
 */
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
extern int printk(const char *fmt, ...);

/*
 * From entry.asm
 */
extern int _pascal bios_read_hd(
                        unsigned short dev,
                        unsigned short head,
                        unsigned short cyl,
                        unsigned short sect,
                        unsigned short nbsect,
                        unsigned long buffer
                       );


extern dev_t bios_device;

#endif /* __microfsd_h */
