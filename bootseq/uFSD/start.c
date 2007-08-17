//
//  muFSD logic routines
//  (filesystem-independent).
//

//
// $Header: /cur/cvsroot/boot/muFSD/start.c,v 1.2 2006/11/24 11:43:13 valerius Exp $
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

#define INCL_DOSDEVIOCTL
#define INCL_NOPMAPI
#include <os2/os2.h>

#include <add.h>
#include <i86.h>
#include <string.h>

#include <os2/types.h>
#include <os2/magic.h>
#include <microfsd.h>

#ifndef __NO_LILO__
#include <linux/ext2_fs.h>
#endif
//#include <mfs.h>

#define OS2LDRSEG    0x1000 /* OS2LDR seems to be loaded at 1000:0000 and relocate at 9400:0000 */
#define MINIFSDSEG   0x007C /* HPFS seems to load its mini FSD at 007C:0000                     */
#define INITSEG      0x9000 /* LILO loads us here                                               */
#define MICROFSDSEG  0x8C20 /* we relocate ourself here                                         */
#define LILODATASEG  0x8C00 /* we relocate LILO data here                                       */
#define BOOTSEC_DATASEG LILODATASEG

dev_t bios_device;

extern unsigned short image_length;

/*
 * Standard data structure to be passed to OS2LDR (defined in the IFS document)
 */
struct FileTable os2ldr_data = {
    3,
    OS2LDRSEG, 0,
    MICROFSDSEG, 0,
    MINIFSDSEG, 0,
    0, 0,
    stub_mu_Open, MICROFSDSEG,
    stub_mu_Read, MICROFSDSEG,
    stub_mu_Close, MICROFSDSEG,
    stub_mu_Terminate, MICROFSDSEG
};
void far *os2ldr_data_ptr;

/*
 * BIOS parameter block to be passed to OS2LDR
 */
#ifndef __NO_LILO__
BIOSPARAMETERBLOCK bpb = {0, 1, 1, 2, 224, 0, 0, 9, 0, };
//#else
//BIOSPARAMETERBLOCK bpb;
#endif

//void far *pbpb;
BIOSPARAMETERBLOCK far *pbpb;
unsigned long os2ldr;

/*
 * Current segment register
 */
extern unsigned short current_seg;

#ifndef __NO_LILO__

unsigned short far *lilo_cmd_line_magic = (unsigned short far *)0x8C000020;     /* CL_MAGIC at 8C00:0020 in lilo.h */
unsigned short far *lilo_cmd_line       = (unsigned short far *)0x8C000022;

#define CL_MAGIC 0xa33f

#define MAJOR(dev)      ((dev)>>8)
#define MINOR(dev)      ((dev) & 0xff)
#define MKDEV(ma,mi)    ((ma)<<8 | (mi))

/*
 * major number definitions in linux/major.h
 */
#define IDE0_MAJOR 3
#define SCSI_DISK_MAJOR 8
#define IDE1_MAJOR 22
#define IDE2_MAJOR 33
#define IDE3_MAJOR 34
#define FLOPPY_MAJOR 2

#endif

/*
 * Registers passed to us by LILO
 */
unsigned short parm_cs;
unsigned short parm_ds;
unsigned short parm_es;
unsigned short parm_ss;

char banner[]   = "Linux ext2fs IFS for OS/2 - MICROFSD - Copyright (C) 1996 Matthieu WILLM";

char name_microfsd[] = "MICROFSD.FSD";
char name_minifsd[]  = "MINIFSD.FSD";
char name_os2ldr[]   = "OS2LDR";

extern char disk_table[];       /* BIOS floppy drive parameters */


#define BOOTFLAG_NOVOLIO   0x0100
#define BOOTFLAG_RIPL      0x0200
#define BOOTFLAG_MINIFSD   0x0400
#define BOOTFLAG_RESERVED3 0x0800
#define BOOTFLAG_MICROFSD  0x1000
#define BOOTFLAG_RESERVED5 0x2000
#define BOOTFLAG_RESERVED6 0x4000
#define BOOTFLAG_RESERVED7 0x8000

/*
 * Main micro FSD entry point
 */
void __cdecl mu_Start(void) {
    int bios_device_ok = 0;
    int hidden_ok      = 0;
    int bsize_ok       = 0;
    int sect_ok        = 0;
    int head_ok        = 0;
    int csect_ok       = 0;
    int bootdrv_ok     = 0;
    ULONG  csect;
    USHORT bootdrive;
    dev_t dev = 0;
    unsigned long  s;
    long sz;
    unsigned short mem;

//#ifndef __NO_LILO__
    struct ext2_super_block *es;
    struct buffer_head      *bh;
//#endif

    void (far *x)();
    unsigned short flags;
    char msg[1024];

    unsigned short _seg, _ofs;

    /*
     * prints the banner
     */
    printk("%s", banner);


    /*
     * prints the segment registers passed to microfsd.fsd
     */
    printk("CS = 0x%04X, DS = 0x%04X, ES = 0x%04X, SS = 0x%04X", parm_cs, parm_ds, parm_es, parm_ss);

//---> begin
#ifndef __NO_LILO__

    pbpb = &bpb;

    /*
     * Tests LILO command line parameter
     */
    if (*lilo_cmd_line_magic == CL_MAGIC) {
        USHORT ofs = *lilo_cmd_line;
        char *tmp;
        //char far *cmd = (char far *)MAKEP(INITSEG, ofs);
        char far *cmd = (char far *)MK_FP(INITSEG, ofs);
        printk("LILO command line found - Offset = 0x%04X", *lilo_cmd_line);
        _fstrcpy(msg, cmd);
        printk("cmd line : %s", msg);

       for (tmp = strtok(msg, " ") ; tmp != NULL ; tmp = strtok(NULL, " ")) {
           if (tmp) {
                char *line = tmp;
#define OS2_BIOS_DRV_STR "os2_bios_device="
#define OS2_BIOS_DRV_LEN (sizeof(OS2_BIOS_DRV_STR) - 1)
                if (!strncmp(line, OS2_BIOS_DRV_STR, OS2_BIOS_DRV_LEN)) {
                    line += OS2_BIOS_DRV_LEN;
                    bios_device    = (dev_t)simple_strtoul(line,0,0);
                    bios_device_ok = 1;
                    continue;
                }
#define OS2_HIDDEN_STR "os2_hidden_sectors="
#define OS2_HIDDEN_LEN (sizeof(OS2_HIDDEN_STR) - 1)
                if (!strncmp(line, OS2_HIDDEN_STR, OS2_HIDDEN_LEN)) {
                    line += OS2_HIDDEN_LEN;
                    pbpb->cHiddenSectors    = (blk_t)simple_strtoul(line,0,0);
                    hidden_ok             = 1;
                    continue;
                }
#define OS2_BSIZE_STR "os2_bytes_per_sector="
#define OS2_BSIZE_LEN (sizeof(OS2_BSIZE_STR) - 1)
                if (!strncmp(line, OS2_BSIZE_STR, OS2_BSIZE_LEN)) {
                    line += OS2_BSIZE_LEN;
                    pbpb->usBytesPerSector  = (USHORT)simple_strtoul(line,0,0);
                    bsize_ok              = 1;
                    continue;
                }
#define OS2_SECT_STR "os2_sect="
#define OS2_SECT_LEN (sizeof(OS2_SECT_STR) - 1)
                if (!strncmp(line, OS2_SECT_STR, OS2_SECT_LEN)) {
                    line += OS2_SECT_LEN;
                    pbpb->usSectorsPerTrack = (USHORT)simple_strtoul(line,0,0);
                    sect_ok               = 1;
                    continue;
                }
#define OS2_HEAD_STR "os2_head="
#define OS2_HEAD_LEN (sizeof(OS2_HEAD_STR) - 1)
                if (!strncmp(line, OS2_HEAD_STR, OS2_HEAD_LEN)) {
                    line += OS2_HEAD_LEN;
                    pbpb->cHeads            = (USHORT)simple_strtoul(line,0,0);
                    head_ok               = 1;
                    continue;
                }
#define OS2_BOOTDRV_STR "os2_bootdrive="
#define OS2_BOOTDRV_LEN (sizeof(OS2_BOOTDRV_STR) - 1)
                if (!strncmp(line, OS2_BOOTDRV_STR, OS2_BOOTDRV_LEN)) {
                    line += OS2_BOOTDRV_LEN;
                    bootdrive             = (USHORT)simple_strtoul(line,0,0);
                    if (bootdrive > 25)
                        microfsd_panic("os2_bootdrive %u out of range", bootdrive);
                    bootdrv_ok            = 1;
                    continue;
                }

#define OS2_CSECT_STR "os2_total_sectors="
#define OS2_CSECT_LEN (sizeof(OS2_CSECT_STR) - 1)
                if (!strncmp(line, OS2_CSECT_STR, OS2_CSECT_LEN)) {
                    line += OS2_CSECT_LEN;
                    csect = (ULONG)simple_strtoul(line,0,0);
                    if (csect < 65536UL) {
                        pbpb->cSectors      = (USHORT)csect;
                        pbpb->cLargeSectors = 0;
                    } else {
                        pbpb->cSectors      = 0;
                        pbpb->cLargeSectors = csect;
                    }
                    csect_ok   = 1;
                    continue;
                }
           }
       }
       if (!bios_device_ok)
            microfsd_panic("os2_bios_device not set in lilo.conf");
       if (!hidden_ok)
            microfsd_panic("os2_hidden_sectors not set in lilo.conf");
       if (!bsize_ok)
            microfsd_panic("os2_bytes_per_sector not set in lilo.conf");
       if (!sect_ok)
            microfsd_panic("os2_sect not set in lilo.conf");
       if (!head_ok)
            microfsd_panic("os2_head not set in lilo.conf");
       if (!csect_ok)
            microfsd_panic("os2_total_sectors not set in lilo.conf");
       if (!bootdrv_ok)
            microfsd_panic("os2_bootdrive not set in lilo.conf");

    } else {
        microfsd_panic("Could not find LILO command line parameters");
    }

    /*
     * Updates the sector size in BIOS floppy drive parameter table
     */
    switch (pbpb->usBytesPerSector) {
        case 512:
            disk_table[3] = 2;
            break;
        default:
            microfsd_panic("sector size %d invalid", pbpb->usBytesPerSector);
    }

    /*
     * Updates the sector per track count in floppy drive parameter table
     */
    disk_table[4] = (unsigned char)(pbpb->usSectorsPerTrack);

#else
    /*
     * Set a pointer to the BPB
     */
    //pbpb = (BIOSPARAMETERBLOCK far *) MAKEP(BOOTSEC_DATASEG, 11);
    pbpb = (BIOSPARAMETERBLOCK far *) MK_FP(BOOTSEC_DATASEG, 11);
    bios_device = pbpb->abReserved[0]; //???
    bootdrive   = pbpb->abReserved[1];
    printk("hiddensectors = 0x%lX", pbpb->cHiddenSectors);
    printk("bios_device = 0x%02X", bios_device);
    printk("current_seg = 0x%04X", current_seg);
#endif
//<--- end

    /*
     * Mounts the partition
     */
    mu_Mount(); /* failure in read_super = panic */
    printk("ext2fs partition mounted successfuly");


    /*
     * Loads MINIFSD into memory
     */
    mu_Open((char far *)name_minifsd, (unsigned long far *) &sz);
    //mu_Read(0, (char far *) MAKEP(MINIFSDSEG, 0), (unsigned long) sz);
    mu_Read(0, (char far *) MK_FP(MINIFSDSEG, 0), (unsigned long) sz);
    mu_Close();
    os2ldr_data.ft_mfslen = sz;

    /*
     * Loads OS2LDR into memory
     */
    mu_Open((char far *)name_os2ldr, (unsigned long far *) &sz);
    //mu_Read(0, (char far *) MAKEP(OS2LDRSEG, 0), (unsigned long) sz);
    mu_Read(0, (char far *) MK_FP(OS2LDRSEG, 0), (unsigned long) sz);
    mu_Close();
    os2ldr_data.ft_ldrlen = sz;

    /*
     * base/length of micro-fsd
     */
    os2ldr_data.ft_mulen  = image_length;

    /*
     * base/length of boot data to pass to mini FSD
     */
    os2ldr_data.ft_ripseg  = 0;
    os2ldr_data.ft_riplen  = 0;

#ifndef __NO_LILO__
    /*
     * HACK !!!!
     */
//    pbpb->usReservedSectors  = BOOTDATA_MAGIC;
//    pbpb->usSectorsPerFAT    = bootdrive;

    pbpb->abReserved[0] = bootdrive;
    pbpb->abReserved[1] = BOOTDATA_MAGIC_LO;
    pbpb->abReserved[2] = BOOTDATA_MAGIC_HI;

    /*
     * Media descriptor
     */
    if ((bios_device >=0) && (bios_device <= 26))
        pbpb->bMedia = 0xF0;
    else if ((bios_device >=0x80) && (bios_device <= 0x96))
        pbpb->bMedia = 0xF8;
    else
        microfsd_panic("Media type");

#endif

    /*
     * prints the file table
     */
    printk("MICROFSD address %04X:0000 length %u", os2ldr_data.ft_museg, os2ldr_data.ft_mulen);
    printk("MINIFSD  address %04X:0000 length %u", os2ldr_data.ft_mfsseg, os2ldr_data.ft_mfslen);
    printk("OS2LDR   address %04X:0000 length %u", os2ldr_data.ft_ldrseg, os2ldr_data.ft_ldrlen);
    printk("RIPLDATA address %04X:0000 length %u", os2ldr_data.ft_ripseg, os2ldr_data.ft_riplen);

    /*
     * Boot flags
     */
    flags = bios_device | BOOTFLAG_MICROFSD | BOOTFLAG_MINIFSD;

    /*
     * Calls OS2LDR
     */
    os2ldr_data_ptr = MK_FP(current_seg, &os2ldr_data);
    //pbpb         = &bpb;
    printk("bpb             = %04X:%04X", FP_SEG(pbpb), FP_OFF(pbpb));
    printk("os2ldr_data_ptr = %04X:%04X", FP_SEG(os2ldr_data_ptr), FP_OFF(os2ldr_data_ptr));

    _seg = os2ldr_data.ft_ldrseg;
    _ofs = 0;

    __asm {
        mov dx, flags
        lds si, cs:pbpb
        les di, cs:os2ldr_data_ptr
        push cs:_seg
        push cs:_ofs
        retf
    };

}

