//
// $Header: /cur/cvsroot/boot/muFSD/ext2/buffer.c,v 1.2 2006/11/24 11:43:13 valerius Exp $
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

//#include <os21x/os2.h>

#include <os2/types.h>
#include <os2/magic.h>
#include <microfsd.h>
#include <mfs.h>
#include <ext2fs.h>

#define NR_BUFFERS 5


char b_data[NR_BUFFERS  * BLOCK_SIZE];
struct buffer_head *hash_table[NR_BUFFERS] = {0, 0, 0, 0, 0};

struct buffer_head  bhlist[NR_BUFFERS] = {
    {
        b_data,             // pointer to data block
        -1,                 // block number
        0,                  // contents of buffer is valid (1) or not (0)
        0,                  // use count
        0,                  // next in hash queue
        0,                  // prev in hash queue
        bhlist + 1,         // next in LRU circular list
        bhlist + 4,         // prev in LRU circular list
        BUFFER_HEAD_MAGIC   //  Magic signature always equal to  BUFFER_HEAD_MAGIC
    },
    {
        b_data + BLOCK_SIZE,             // pointer to data block
        -1,                  // block number
        0,                  // contents of buffer is valid (1) or not (0)
        0,                  // use count
        0,                  // next in hash queue
        0,                  // prev in hash queue
        bhlist + 2,         // next in LRU circular list
        bhlist,             // prev in LRU circular list
        BUFFER_HEAD_MAGIC   //  Magic signature always equal to  BUFFER_HEAD_MAGIC
    },
    {
        b_data + 2 * BLOCK_SIZE,             // pointer to data block
        -1,                 // block number
        0,                  // contents of buffer is valid (1) or not (0)
        0,                  // use count
        0,                  // next in hash queue
        0,                  // prev in hash queue
        bhlist + 3,         // next in LRU circular list
        bhlist + 1,         // prev in LRU circular list
        BUFFER_HEAD_MAGIC   //  Magic signature always equal to  BUFFER_HEAD_MAGIC
    },
    {
        b_data + 3 * BLOCK_SIZE,             // pointer to data block
        -1,                  // block number
        0,                  // contents of buffer is valid (1) or not (0)
        0,                  // use count
        0,                  // next in hash queue
        0,                  // prev in hash queue
        bhlist + 4,         // next in LRU circular list
        bhlist + 2,         // prev in LRU circular list
        BUFFER_HEAD_MAGIC   //  Magic signature always equal to  BUFFER_HEAD_MAGIC
    },
    {
        b_data + 4 * BLOCK_SIZE,             // pointer to data block
        -1,                  // block number
        0,                  // contents of buffer is valid (1) or not (0)
        0,                  // use count
        0,                  // next in hash queue
        0,                  // prev in hash queue
        bhlist,             // next in LRU circular list
        bhlist + 3,         // prev in LRU circular list
        BUFFER_HEAD_MAGIC   //  Magic signature always equal to  BUFFER_HEAD_MAGIC
    }
};
struct buffer_head *used_list  = 0;
struct buffer_head *free_list  = bhlist;

#define hash(block) hash_table[block % NR_BUFFERS]

void remove_from_hash_queue(struct buffer_head * bh) {
    if (!bh)
        microfsd_panic("bh is NULL");
    if (bh->b_magic != BUFFER_HEAD_MAGIC)
        microfsd_panic("remove_from_hash_queue - invalid magic number");
    if (bh->b_blocknr == -1)
        microfsd_panic("remove_from_hash_queue - not in hash queue");

    if (bh->b_next)
        bh->b_next->b_prev = bh->b_prev;
    if (bh->b_prev)
        bh->b_prev->b_next = bh->b_next;
    if (hash(bh->b_blocknr) == bh)
        hash(bh->b_blocknr) = bh->b_next;
    bh->b_next = 0;
    bh->b_prev = 0;

}

void add_to_hash_queue(struct buffer_head *bh) {

    if (!bh)
        microfsd_panic("bh is NULL");
    if (bh->b_magic != BUFFER_HEAD_MAGIC)
        microfsd_panic("add_to_hash_queue - invalid magic number");
    if (bh->b_next || bh->b_prev)
        microfsd_panic("add_to_hash_queue - already in hash queue");

    bh->b_next = hash(bh->b_blocknr);
    hash(bh->b_blocknr) = bh;
    if (bh->b_next)
        bh->b_next->b_prev = bh;

}

struct buffer_head *get_hash_table(blk_t block) {
        struct buffer_head * tmp;

        for (tmp = hash(block) ; tmp ; tmp = tmp->b_next) {
                if (tmp->b_blocknr == block) {
                    return tmp;
                }
        }
        return 0;
}

void remove_from_used_list(struct buffer_head *bh) {

    if (!bh)
        microfsd_panic("bh is NULL");
    if (bh->b_magic != BUFFER_HEAD_MAGIC)
        microfsd_panic("remove_from_used_list - invalid magic number");
    if (!(bh->b_prev_free) || !(bh->b_next_free))
        microfsd_panic("remove_from_used_list - used block list corrupted");
    if(!used_list)
        microfsd_panic("remove_from_used_list - used list empty");

    bh->b_prev_free->b_next_free = bh->b_next_free;
    bh->b_next_free->b_prev_free = bh->b_prev_free;

    if (used_list == bh)
        used_list = bh->b_next_free;
    if(used_list == bh)
         used_list = 0;
    bh->b_next_free = 0;
    bh->b_prev_free = 0;

}
void remove_from_free_list(struct buffer_head * bh) {
    if (!bh)
        microfsd_panic("remove_from_free_list - bh is NULL");
    if (bh->b_magic != BUFFER_HEAD_MAGIC)
        microfsd_panic("remove_from_free_list - invalid magic number");
    if (!bh->b_prev_free || !bh->b_next_free)
        microfsd_panic("remove_from_free_list - free block list corrupted");
    if(!free_list)
        microfsd_panic("remove_from_free_list - free list empty");

    bh->b_prev_free->b_next_free = bh->b_next_free;
    bh->b_next_free->b_prev_free = bh->b_prev_free;

    if (free_list == bh)
        free_list = bh->b_next_free;
    if(free_list == bh)
         free_list = 0;
    bh->b_next_free = 0;
    bh->b_prev_free = 0;
}

void put_last_used(struct buffer_head * bh) {
    if (!bh)
        microfsd_panic("bh is NULL");
    if (bh->b_magic != BUFFER_HEAD_MAGIC)
        microfsd_panic("put_last_used - invalid magic number");
    if (bh->b_prev_free || bh->b_next_free)
        microfsd_panic("put_last_used - used block list corrupted");

    if(!used_list) {
        used_list = bh;
        used_list->b_prev_free = bh;
    };

    bh->b_next_free = used_list;
    bh->b_prev_free = used_list->b_prev_free;
    used_list->b_prev_free->b_next_free = bh;
    used_list->b_prev_free = bh;
}



void put_last_free(struct buffer_head * bh) {
    if (!bh)
        microfsd_panic("put_last_free - bh is NULL");
    if (bh->b_magic != BUFFER_HEAD_MAGIC)
        microfsd_panic("put_last_free - invalid magic number");
    if (bh->b_prev_free || bh->b_next_free)
        microfsd_panic("put_last_free - free block list corrupted");

    if(!free_list) {
        free_list = bh;
        bh->b_prev_free = bh;
    };

    bh->b_next_free = free_list;
    bh->b_prev_free = free_list->b_prev_free;
    free_list->b_prev_free->b_next_free = bh;
    free_list->b_prev_free = bh;
}


struct buffer_head * getblk(blk_t block) {
    struct buffer_head *bh;

    bh = get_hash_table(block);

    if (bh) {
        if (bh->b_count) {
            /*
             * buffer was on LRU list
             */
            remove_from_used_list(bh);
        } else {
            /*
             * buffer was on free list
             */
            remove_from_free_list(bh);
        }
        put_last_used(bh);
        bh->b_count ++;
        return bh;
    }

    bh = free_list;
    if (!bh)
        microfsd_panic("No more buffers !");
    remove_from_free_list(bh);
    put_last_used(bh);
    if (bh->b_blocknr != -1) {
        /*
         * buffer was on hash queue
         */
        remove_from_hash_queue(bh);
    }
    bh->b_count    = 1;
    bh->b_blocknr  = block;
    bh->b_uptodate = 0;
    add_to_hash_queue(bh);
    return bh;
}


void brelse(struct buffer_head *buf) {
    if (buf) {
        if (buf->b_count) {
            buf->b_count --;
            if (!buf->b_count) {
                remove_from_used_list(buf);
                put_last_free(buf);
            }
        } else {
            microfsd_panic("brelse: Trying to free free buffer");
        }
    }
}

void bforget(struct buffer_head *buf) {
    if (buf) {
        if (buf->b_count) {
            buf->b_count --;
            if (!buf->b_count) {
                remove_from_used_list(buf);
                put_last_free(buf);
                remove_from_hash_queue(buf);
                buf->b_blocknr  = -1;
                buf->b_uptodate = 0;
            }
        } else {
            microfsd_panic("brelse: Trying to free free buffer");
        }
    }
}

void dumpblk() {
    int i;
    struct buffer_head *bh;
    for (i = 0; i < NR_BUFFERS; i++) {
        bh = bhlist + i;
        printk("[%d] data=%04X blocknr=%ld uptodate=%d count=%d magic=%04X", i,
               bh->b_data, bh->b_blocknr, bh->b_uptodate, bh->b_count, bh->b_magic);
    }
}




struct buffer_head *bread(blk_t block) {
    struct buffer_head *bh;

    bh = getblk(block);            /* failure in getblk      = panic */
    if (bh->b_uptodate)
        return bh;
    ll_rw_block(READ, 1, &bh);     /* failure in ll_rw_block = panic */
    bh->b_uptodate = 1;
    return bh;
}
