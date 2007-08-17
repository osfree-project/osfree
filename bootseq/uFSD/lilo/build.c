//
// $Header: /cur/cvsroot/boot/muFSD/lilo/build.c,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
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

/*
 *  linux/tools/build.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * This file builds a disk-image from three different files:
 *
 * - bootsect: exactly 512 bytes of 8086 machine code, loads the rest
 * - setup: 8086 machine code, sets up system parm
 * - system: 80386 code for actual system
 *
 * It does some checking that all files are of the correct type, and
 * just writes the result to stdout, removing headers and padding to
 * the right amount. It also writes some system data to stderr.
 */

/*
 * Changes by tytso to allow root device specification
 */

#include <stdio.h>      /* fprintf */
#include <string.h>
#include <stdlib.h>     /* contains exit */
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MINIX_HEADER 32



/* max nr of sectors of setup: don't change unless you also change
 * bootsect etc */
#define SETUP_SECTS 4

#define STRINGIFY(x) #x

typedef union {
        long l;
        short s[2];
        char b[4];
} conv;

long intel_long(long l)
{
        conv t;

        t.b[0] = l & 0xff; l >>= 8;
        t.b[1] = l & 0xff; l >>= 8;
        t.b[2] = l & 0xff; l >>= 8;
        t.b[3] = l & 0xff; l >>= 8;
        return t.l;
}

short intel_short(short l)
{
        conv t;

        t.b[0] = l & 0xff; l >>= 8;
        t.b[1] = l & 0xff; l >>= 8;
        return t.s[0];
}

void die(const char * str)
{
        perror("");
        fprintf(stderr,"%s\n",str);
        exit(1);
}

void usage(void)
{
        die("Usage: build bootsect microfsd image");
}

int main(int argc, char ** argv)
{
        int i,c,id, sz, fdout;
        unsigned long sys_size;
        char buf[1024];
        char major_root, minor_root;
        struct stat sb;
        unsigned char setup_sectors;

        if ((argc < 4) || (argc > 5))
                usage();


        if ((fdout=open(argv[3],O_WRONLY | O_BINARY | O_CREAT,S_IWRITE)) == -1)
                die("Unable to open 'image'");

        if ((id=open(argv[1],O_RDONLY | O_BINARY,0))<0)
                die("Unable to open 'boot'");
        i=read(id,buf,sizeof buf);
        fprintf(stderr,"Boot sector %d bytes.\n",i);
        if (i != 512)
                die("Boot block must be exactly 512 bytes");
        if ((*(unsigned short *)(buf+510)) != (unsigned short)intel_short(0xAA55))
                die("Boot block hasn't got boot flag (0xAA55)");
/*      buf[508] = (char) minor_root;*/
/*      buf[509] = (char) major_root;   */
        i=write(fdout,buf,512);
        if (i!=512)
                die("Write call failed");
        close (id);

        if ((id=open(argv[2],O_RDONLY | O_BINARY,0))<0)
                die("Unable to open 'setup'");
        for (i=0 ; (c=read(id,buf,sizeof buf))>0 ; i+=c ) {
                if (write(fdout,buf,c)!=c)
                        die("Write call failed");
                else
                        fprintf(stderr, "Setup : wrote %d bytes\n", c);
        }
        if (c != 0)
                die("read-error on 'setup'");
        close (id);

        setup_sectors = (unsigned char)((i + 511) / 512);
        /* for compatibility with LILO */
        if (setup_sectors < SETUP_SECTS)
                setup_sectors = SETUP_SECTS;
        fprintf(stderr,"Setup is %d bytes.\n",i);
        for (c=0 ; c<sizeof(buf) ; c++)
                buf[c] = '\0';
        while (i < setup_sectors * 512) {
                c = setup_sectors * 512 - i;
                if (c > sizeof(buf))
                        c = sizeof(buf);
                if (write(fdout,buf,c) != c)
                        die("Write call failed");
                i += c;
        }

        if (lseek(fdout, 497, 0) == 497) {
                if (write(fdout, &setup_sectors, 1) != 1)
                        die("Write of setup sectors failed");
        }
        close(fdout);
        return(0);
}
