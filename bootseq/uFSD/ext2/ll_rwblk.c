//
// $Header: /cur/cvsroot/boot/muFSD/ext2/ll_rwblk.c,v 1.2 2006/11/24 11:43:13 valerius Exp $
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
#include <os21x/os2.h>

#include <i86.h>
#include <add.h>

#include <os2/types.h>
#include <os2/magic.h>
#include <mfs.h>
#include <microfsd.h>

extern BIOSPARAMETERBLOCK far *pbpb;

extern dev_t bios_device;
blk_t b_size        = BLOCK_SIZE;      // hardcoded for the moment : only this block size supported

int stub_bios_read_hd(
                        unsigned short dev,
                        unsigned short head,
                        unsigned short cyl,
                        unsigned short sect,
                        unsigned short nbsect,
                        unsigned long buffer
                       ) {

    if ((!(dev >= 0 && dev <= 26)) &&
        (!(dev >= 0x80 && dev <= 0x96)))
        microfsd_panic("invalid BIOS device 0x%0X", dev);

    if (nbsect > 128)
        microfsd_panic("nbsect (%u) out of range", nbsect);
    if (cyl > 1023)
        microfsd_panic("cyl (%u) out of range", cyl);
    if (sect > 64)
        microfsd_panic("sect (%u) out of range", sect);
    if (head > 255)
        microfsd_panic("head (%u) out of range", head);
    if (!buffer)
        microfsd_panic("bios_read_hd - buffer is NULL");

#if 0
    printk(
           "bios_read_hd dev=%0X, h=%u c=%u s=%u nr=%u buf=%04X:%04X",
           dev,
           head,
           cyl,
           sect,
           nbsect,
           (unsigned short)(buffer >> 16),
           (unsigned short)(buffer &  0xFFFF)
          );
#endif

    return bios_read_hd(dev, head, cyl, sect, nbsect, buffer);
}

void ll_rw_block(int rw, int nr, struct buffer_head **bh) {
    int rc;
    int i, j;
    blk_t LBA;
    blk_t tracks;
    unsigned short track;
    unsigned short head;
    unsigned short sector;
    unsigned short nsectors;
    unsigned long  tmpbuf;


    if (!nr)
        microfsd_panic("ll_rwblk - nr is 0");

    switch (rw) {
        case READ  :
        case READA :
            for (i = 0; i < nr; i++) {
                if (bh[i] == 0)
                        microfsd_panic("ll_rw_block : bh[%d] = 0", i);
                if (bh[i]->b_magic != BUFFER_HEAD_MAGIC)
                        microfsd_panic("ll_rw_block : bh[%d] with invalid magic nr", i);

                LBA = (bh[i]->b_blocknr) * (b_size / (blk_t)(pbpb->usBytesPerSector)) + pbpb->cHiddenSectors;

                for (j = 0 ; j < BLOCK_SIZE / 512 ; j++) {
                    LBA     += j;

                    if (((pbpb->cSectors)       && (LBA > pbpb->cSectors + pbpb->cHiddenSectors))      ||
                        ((pbpb->cLargeSectors)  && (LBA > pbpb->cLargeSectors + pbpb->cHiddenSectors)) ||
                        ((pbpb->cHiddenSectors) && (LBA < pbpb->cHiddenSectors))) {
                        microfsd_panic("ll_rw_block : block %ld out of partition,\n", bh[i]->b_blocknr);
                    }

                    tracks   = LBA / pbpb->usSectorsPerTrack;
                    head     = (unsigned short)(tracks % pbpb->cHeads);
                    track    = (unsigned short)(tracks / pbpb->cHeads);
                    sector   = (unsigned short)(LBA % pbpb->usSectorsPerTrack) + 1;
//                    nsectors = (unsigned short)(b_size / (blk_t)pbpb->usBytesPerSector);

                    //tmpbuf   = (unsigned long)MAKEP(current_seg, bh[i]->b_data + j * 512);
                    tmpbuf   = (unsigned long)MK_FP(current_seg, bh[i]->b_data + j * 512);

                    rc = stub_bios_read_hd(
                                           bios_device,
                                           head,
                                           track,
                                           sector,
                                           1,
                                           tmpbuf
                                          );
                    if (rc) {
                        microfsd_panic("ll_rwblk - bios_read_hd returned %04X", rc);
                    }
                }

                bh[i]->b_uptodate = 1;
            }
            break;
        default :
            microfsd_panic("ll_rwblk"" - invalid op.");
            break;
    }
}

