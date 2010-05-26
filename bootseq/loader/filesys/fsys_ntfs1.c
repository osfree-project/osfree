/* vim: set sw=4 :*/
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999  Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Samuel Leo <samuel@_.remove.me._szonline.net>
 * Limitations:
 * 1. Only 32 bit size support
 * 2. don't support >1k MFT record size, >16k INDEX record size
 * 3. don't support recursive at_attribute_list
 * 4. don't support compressed attribute other than Datastream
 * 5. all MFT's at_attribute_list must resident at first run list
 * 6. don't support journaling
 * 7. don't support EFS encryption
 * 8. don't support mount point and junction
 */
#if defined(fsys_ntfs) || defined(FSYS_NTFS)

//#define DEBUG_NTFS 1

/*
#define NO_ATTRIBUTE_LIST 1
   totally disable at_attribute_list support,
   if no compressed/fragment file and MFT,
   not recommended
#define NO_NON_RESIDENT_ATTRIBUTE_LIST 1
   disable non-resident at_attribute_list support,
   if no huge compressed/fragment file and MFT
#define NO_NTFS_DECOMPRESSION 1
   disable ntfs compressed file support
#define NO_ALTERNATE_DATASTREAM 1
   disable ntfs alternate datastream support
*/

#include "shared.h"
#include "filesys.h"

#include "fsys.h"
#include "misc.h"
#include "fsd.h"

#ifdef STAGE1_5
/* safe turn off non-resident attribute list if MFT fragments < 4000 */
//#define NO_NON_RESIDENT_ATTRIBUTE_LIST 1
#define NO_NTFS_DECOMPRESSION 1
#endif

#define MAX_MFT_RECORD_SIZE 1024
#define MAX_INDEX_RECORD_SIZE 16384
#define MAX_INDEX_BITMAP_SIZE 4096
#define DECOMP_DEST_BUFFER_SIZE 16384
#define DECOMP_SOURCE_BUFFER_SIZE (8192+2)
#define MAX_DIR_DEPTH 64

/* sizes are always in bytes, BLOCK values are always in DEV_BSIZE (sectors) */
#define DEV_BSIZE 512

/* include/linux/fs.h */
#define BLOCK_SIZE      512

#define WHICH_SUPER 1
#define SBLOCK (WHICH_SUPER * BLOCK_SIZE / DEV_BSIZE)   /* = 2 */

/* include/asm-i386/type.h */
typedef signed char __s8;
typedef unsigned char __u8;
typedef signed short __s16;
typedef unsigned short __u16;
typedef signed int __s32;
typedef unsigned int __u32;
typedef signed long long __s64;
typedef unsigned long long __u64;

#define FILE_MFT      0
#define FILE_MFTMIRR  1
#define FILE_LOGFILE  2
#define FILE_VOLUME   3
#define FILE_ATTRDEF  4
#define FILE_ROOT     5
#define FILE_BITMAP   6
#define FILE_BOOT     7
#define FILE_BADCLUS  8
#define FILE_QUOTA    9
#define FILE_UPCASE  10

#define at_standard_information 0x10
#define at_attribute_list       0x20
#define at_filename             0x30
#define at_security_descriptor  0x50
#define at_data                 0x80
#define at_index_root           0x90
#define at_index_allocation     0xa0
#define at_bitmap               0xb0
#define at_symlink              0xc0

#define NONAME  ""
#define ATTR_NORMAL     0
#define ATTR_COMPRESSED 1
#define ATTR_RESIDENT   2
#define ATTR_ENCRYPTED  16384
#define ATTR_SPARSE     32768

typedef struct run_list {
        char *start;
        char *ptr;
        int svcn;
        int evcn;
        int vcn;
        int cnum0;
        int cnum;
        int clen;
} RUNL;

typedef struct ntfs_mft_record {
        char mft[MAX_MFT_RECORD_SIZE];
        char mft2[MAX_MFT_RECORD_SIZE];
        int attr_type;
        char *attr_name;
        int attr_flag;
        int attr_size;
        char *attr;
        int attr_len;
        RUNL runl;
        char *attr_list;
        int attr_list_len;
        int attr_list_size;
        int attr_list_off;
        char attr_list_buf[2*BLOCK_SIZE];
        RUNL attr_list_runl;
} MFTR;


#define index_data      ((char *)FSYS_BUF)
#define bitmap_data     ((__u8 *)(FSYS_BUF+MAX_INDEX_RECORD_SIZE))
#define dcdbuf  ((__u8 *)index_data)
#define dcsbuf  (bitmap_data)
#define dcend   (dcsbuf+DECOMP_SOURCE_BUFFER_SIZE)
#define fnbuf ((char *)(bitmap_data+MAX_INDEX_BITMAP_SIZE))
#define mmft    ((MFTR *)dcend)
#define cmft    ((MFTR *)(dcend+sizeof(MFTR)))
#define mft_run ((RUNL *)(dcend+2*sizeof(MFTR)))
#define path_ino ((int *)(dcend+2*sizeof(MFTR)+sizeof(RUNL)))
#define cluster16 (path_ino+MAX_DIR_DEPTH)
#define index16 cluster16[16]
#define blocksize cluster16[17]
#define clustersize cluster16[18]
#define mft_record_size cluster16[19]
#define index_record_size cluster16[20]
#define dcvcn cluster16[21]
#define dcoff cluster16[22]
#define dclen cluster16[23]
#define dcrem cluster16[24]
#define dcslen cluster16[25]
#define dcsptr ((__u8 *)cluster16[26])
#define is_ads_completion cluster16[27]

static int read_mft_record(int mftno, char *mft, int self);
static int read_attribute(MFTR *mftr, int offset, char *buf, int len, RUNL *from_rl);
static int get_next_run(RUNL *runl);

static inline int
nsubstring (char *s1, char *s2)
{
    while (tolower(*s1) == tolower(*s2))
    {
        /* The strings match exactly. */
        if (! *(s1++))
            return 0;
        s2 ++;
    }

    /* S1 is a substring of S2. */
    if (*s1 == 0)
        return -1;

    /* S1 isn't a substring. */
    return 1;
}

static int fixup_record(char *record, char *magic, int size)
{
    int start, count, offset;
    __u16 fixup;

    if(*(int *)record != *(int *)magic)
        return 0;
    start=*(__u16 *)(record+4);
    count=*(__u16 *)(record+6);
    count--;
    if(size && blocksize*count != size)
        return 0;
    fixup = *(__u16 *)(record+start);
    start+=2;
    offset=blocksize-2;
    while(count--){
        if(*(__u16 *)(record+offset)!=fixup)
            return 0;
        *(__u16 *)(record+offset) = *(__u16 *)(record+start);
        start+=2;
        offset+=blocksize;
    }
    return 1;
}

static void rewind_run_list( RUNL *runl) {
    runl->vcn = runl->svcn;
    runl->ptr = runl->start;
    runl->cnum0 = 0;
    runl->cnum = 0;
    runl->clen = 0;
}

static int get_next_run(RUNL *runl){
    int t, n, v;

#ifdef DEBUG_NTFS
    printf("get_next_run: s=%d e=%d c=%d start=%x ptr=%x\n",
           runl->svcn, runl->evcn, runl->vcn, runl->start, runl->ptr);
#endif

    runl->vcn += runl->clen;
    if(runl->vcn > runl->evcn) {
        return 0;
    }

    t = *(runl->ptr)++;
    n = t&0xf;
    runl->clen = 0; v = 1;
    while(n--) {
        runl->clen += v * *((__u8 *)runl->ptr)++;
        v <<= 8;
    }
    n = (t>>4)&0xf;
    if(n==0)
        runl->cnum = 0;
    else {
        int c = 0;
        v = 1;
        while(n--) {
            c += v * *((__u8 *)runl->ptr)++;
            v <<= 8;
        }
        if(c & (v>>1)) c -= v;
        runl->cnum0 += c;
        runl->cnum = runl->cnum0;
    }
#ifdef DEBUG_NTFS
    printf("got_next_run: t=%x cluster %x len %x vcn=%x ecn=%x\n",
        t, runl->cnum, runl->clen, runl->vcn, runl->evcn);
#endif
    return 1;
}

#ifndef NO_ATTRIBUTE_LIST
static void init_run_list(char *attr, int len, RUNL *runl) {
    int allocated;

    runl->svcn = *(__u32 *)(attr+0x10); /* only support 32 bit */
    runl->evcn = *(__u32 *)(attr+0x18); /* only support 32 bit */
    runl->start = attr + *(__u16 *)(attr+0x20);
    allocated = *(__u32 *)(attr+0x28);
    if(!runl->evcn) runl->evcn = (allocated - 1) / clustersize;
#ifdef DEBUG_NTFS
    printf("size %d allocated=%d inited=%d cegin=%x csize=%d vcn=%d-%d\n",
            /*attr_size*/ *(__u32 *)(attr+0x30),
            /*allocated*/ *(__u32 *)(attr+0x28),
            /*attr_inited*/ *(__u32 *)(attr+0x38),
            /*cengin*/ *(__u16 *)(attr+0x22),
            /*csize*/ *(__u16 *)(attr+0x40),
            runl->svcn, runl->evcn);
#endif
    rewind_run_list(runl);
}
#endif


static int find_attribute(char *mft, int type, char *name, char **attr, int *size, int *len, int *flag) {
    int t, l, r, n, i, namelen;
    unsigned short *attr_name;

    n = strlen(name);
    r = mft_record_size - *(__u16 *)(mft+0x14);
    mft += *(__u16 *)(mft+0x14);
    while( (t = *(__s32 *)mft) != -1 ) {
        l = *(__u32 *)(mft+4);
        if(l>r) break;
#ifdef DEBUG_NTFS
        printf("type = %x len = %d namelen=%d resident=%d compresed=%d attrno=%d\n",
                t, l,
                /*namelen*/ *(mft+9),
                //name = (__u16 *)(mft + *(__u16 *)(mft+10)),
                /*resident */ (*(mft+8) == 0),
                /*compressed*/ *(__u16 *)(mft+12),
                /*attrno*/ *(__u16 *)(mft+14));
#endif
        namelen = *(mft+9);
        if(t == type) {
#ifndef STAGE1_5
#ifndef NO_ALTERNATE_DATASTREAM
            if(is_ads_completion && type == at_data) {
                if(namelen && namelen >= n &&
                   (!*(mft+8)/*resident*/ || !*(__u32 *)(attr+0x10)/*svcn==0*/))
                {
                    for(i=0, attr_name=(__u16 *)(mft + *(__u16 *)(mft+10)); i < n; i++)
                        if(tolower(name[i]) != tolower(attr_name[i]))
                            break;
                    if(i >= n) {
                        for(; i < namelen; i++)
                            name[i] = attr_name[i];
                        name[i] = '\0';
                        if(*pprint_possibilities > 0)
                            *pprint_possibilities = - *pprint_possibilities;
                        (*pprint_a_completion) (fnbuf);
                        name[n] = '\0';
                    }
                }
            } else
#endif
#endif
                if(namelen == n) {

                for(i=0, attr_name=(__u16 *)(mft + *(__u16 *)(mft+10)); i<n; i++)
                    if(tolower(name[i]) != tolower(attr_name[i]))
                        break;
                if(i>=n) {
                    if(flag) *flag = *(__u16 *)(mft+12);
                    if(*(mft+8) == 0) {
                        if(flag) *flag |= ATTR_RESIDENT;
#ifdef DEBUG_NTFS
                        printf("resident data at %x size %x indexed=%d\n",
                               /*data*/ *(__u16 *)(mft+0x14),
                               /*attr_size*/ *(__u16 *)(mft+0x10),
                               /*indexed*/ *(__u16 *)(mft+0x16));
#endif
                        if(attr) *attr = mft + *(__u16 *)(mft+0x14);
                        if(size) *size = *(__u16 *)(mft+0x10);
                        if(len) *len = *(__u16 *)(mft+0x10);
                    } else {
                        if(attr) *attr = mft;
                        if(size) *size = *(__u32 *)(mft+0x30);
                        if(len) *len = l;
                    }
                    return 1;
                }
            }
        }
        mft += l;
        r -= l;
    }
    return 0;
}

#ifndef NO_ATTRIBUTE_LIST
static __u32 get_next_attribute_list(MFTR *mftr, int *size) {
    int l, t, mftno;
#ifdef DEBUG_NTFS
    printf("get_next_attribute_list: type=%x\n",mftr->attr_type);
#endif
again:
    while(mftr->attr_list_len>0x14) {
        t = *(__u32 *)(mftr->attr_list + 0);
        l = *(__u16 *)(mftr->attr_list + 4);
#ifdef DEBUG_NTFS
        printf("attr_list type=%x len=%x remain=%x\n", t, l, mftr->attr_list_len);
#endif
        if(l==0 || l>mftr->attr_list_len) return 0;
        mftno = *(__u32 *)(mftr->attr_list + 0x10);
        mftr->attr_list_len -= l;
        mftr->attr_list += l;
        if(t==mftr->attr_type)
        {
#ifdef DEBUG_NTFS
        printf("attr_list mftno=%x\n", mftno);
#endif
            if(read_mft_record(mftno, mftr->mft2, (mftr==mmft))==0)
                break;
            if(find_attribute(mftr->mft2, mftr->attr_type, mftr->attr_name,
                        &mftr->attr, size, &mftr->attr_len, &mftr->attr_flag))
                return 1;
        }
    }
#ifndef NO_NON_RESIDENT_ATTRIBUTE_LIST
    if(mftr->attr_list_off < mftr->attr_list_size) {
        int len = mftr->attr_list_size - mftr->attr_list_off;
        if(len > BLOCK_SIZE) len = BLOCK_SIZE;

        if(mftr->attr_list_len)
            (*grub_memmove)(mftr->attr_list_buf, mftr->attr_list, mftr->attr_list_len);
        mftr->attr_list = mftr->attr_list_buf;

        if(read_attribute( NULL, mftr->attr_list_off,
                        mftr->attr_list_buf + mftr->attr_list_len,
                        len, &mftr->attr_list_runl) != len)
        {
#ifdef DEBUG_NTFS
            printf("CORRUPT NON-RESIDENT ATTRIBUTE_LIST\n");
#endif
            /* corrupt */
            *perrnum = ERR_FSYS_CORRUPT;
            mftr->attr_list_size = 0;
            mftr->attr_len = 0;
            mftr->attr_list = NULL;
            return 0;
        }

        mftr->attr_list_len += len;
        mftr->attr_list_off += len;
        goto again;
    }
#endif
    mftr->attr_list = NULL;
    return 0;
}
#endif

static int search_attribute( MFTR *mftr, int type, char *name)
{
#ifdef DEBUG_NTFS
    printf("searching attribute %x <%s>\n", type, name);
#endif

    mftr->attr_type = type;
    mftr->attr_name = name;
    mftr->attr_list = NULL;
    mftr->attr_list_len = 0;
    mftr->attr_list_size = 0;
    mftr->attr_list_off = 0;
    dcrem = dclen = 0;

#ifndef NO_ATTRIBUTE_LIST
    if(find_attribute(mftr->mft, at_attribute_list, NONAME,
                      &mftr->attr_list, &mftr->attr_list_size,
                      &mftr->attr_list_len, &mftr->attr_list_off)) {
        if(mftr->attr_list_off&ATTR_RESIDENT) {
            /* resident at_attribute_list */
            mftr->attr_list_size = 0;
#ifdef DEBUG_NTFS
            printf("resident attribute_list len=%x\n", mftr->attr_list_len);
#endif
        } else {
#ifdef DEBUG_NTFS
            printf("non-resident attribute_list len=%x size=%x\n",
                   mftr->attr_list_len, mftr->attr_list_size);
#endif
#ifndef NO_NON_RESIDENT_ATTRIBUTE_LIST
            init_run_list(mftr->attr_list, mftr->attr_list_len, &mftr->attr_list_runl);
            if(get_next_run(&mftr->attr_list_runl)==0 ||
               mftr->attr_list_runl.cnum==0)
                mftr->attr_list_size = 0;
#endif
            mftr->attr_list = NULL;
            mftr->attr_list_len = 0;
        }
    }
#endif

    if(find_attribute(mftr->mft, type, name,
                      &mftr->attr, &mftr->attr_size, &mftr->attr_len,
                      &mftr->attr_flag)
#ifndef NO_ATTRIBUTE_LIST
       || get_next_attribute_list(mftr, &mftr->attr_size)
#endif
       )
    {
#ifndef NO_ATTRIBUTE_LIST
        if(!(mftr->attr_flag&ATTR_RESIDENT)){
            init_run_list(mftr->attr, mftr->attr_len, &mftr->runl);
            if(get_next_run(&mftr->runl)==0) {
                mftr->attr_flag |= ATTR_RESIDENT;
                mftr->attr_len = 0;
            }
        }
#endif

        return 1;
    }

    mftr->attr_type = 0;
    return 0;
}

static int get_run( RUNL *rl, int vcn, int *clp, int *lenp) {
    if(rl->evcn < vcn)
        return 0;

    if(rl->vcn > vcn) {
        rewind_run_list(rl);
        get_next_run(rl);
    }

    while(rl->vcn+rl->clen <= vcn)
    {
        if(get_next_run(rl)==0)
            return 0;
    }

    if(clp) *clp = rl->cnum == 0 ? 0 : rl->cnum + vcn - rl->vcn;
    if(lenp) *lenp = rl->clen - vcn + rl->vcn;
    return 1;
}

static int search_run(MFTR *mftr, int vcn) {

    if( mftr->attr==NULL && !search_attribute(mftr, mftr->attr_type, mftr->attr_name))
        return 0;

    if(mftr->runl.svcn > vcn)
        search_attribute(mftr, mftr->attr_type, mftr->attr_name);

#ifdef NO_ATTRIBUTE_LIST
    if(mftr->runl.evcn < vcn)
        return 0;
#else
    while(mftr->runl.evcn < vcn) {
        if(get_next_attribute_list(mftr, NULL)==0) {
            mftr->attr = NULL;
            return 0;
        }
        init_run_list(mftr->attr, mftr->attr_len, &mftr->runl);
        if(get_next_run(&mftr->runl)==0) {
            mftr->attr = NULL;
            return 0;
        }
    }
#endif

    return 1;
}

static int read_attribute(MFTR *mftr, int offset, char *buf, int len, RUNL *from_rl) {
    int vcn;
    int cnum, clen;
    int done = 0;
    int n;
    RUNL *rl;

    if(!from_rl && (mftr->attr_flag & ATTR_RESIDENT)) {
        /* resident attribute */
        if(offset > mftr->attr_len)
            return 0;
        if(offset+len > mftr->attr_len)
            len = mftr->attr_len - offset;
        (*pgrub_memmove)( buf, mftr->attr + offset, len);
        return len;
    }

    vcn = offset / clustersize;
    offset %= clustersize;

    while(len>0) {
        if(from_rl)
            rl = from_rl;
        else if(search_run(mftr, vcn) == 0)
            break;
        else
            rl = &mftr->runl;
        if(get_run(rl, vcn, &cnum, &clen) == 0)
            break;
        if(cnum==0 && from_rl)
            break;
        n = clen * clustersize - offset;
        if(n > len) n = len;
        if(cnum==0) {
            (*pgrub_memset)( buf, 0, n);
        } else if(!(*pdevread)(cnum*(clustersize>>9)+(offset>>9), offset&0x1ff, n, buf))
            break;

        buf += n;
        vcn += (offset+n)/clustersize;
        done += n;
        offset = 0;
        len -= n;
    }
    return done;
}

static int read_mft_record(int mftno, char *mft, int self){
#ifdef DEBUG_NTFS
    printf("Reading MFT record: mftno=%d\n", mftno);
#endif
    if( read_attribute( mmft, mftno * mft_record_size,
            mft, mft_record_size, self?mft_run:NULL) != mft_record_size)
        return 0;
    if(!fixup_record( mft, "FILE", mft_record_size))
        return 0;
    return 1;
}

#ifndef NO_NTFS_DECOMPRESSION
static int get_16_cluster(MFTR *mftr, int vcn) {
    int n = 0, cnum, clen;
    while(n < 16 && search_run(mftr, vcn) && get_run(&mftr->runl, vcn, &cnum, &clen) && cnum) {
        if(clen > 16 - n)
            clen = 16 - n;
        vcn += clen;
        while(clen--)
            cluster16[n++] = cnum++;
    }
    cluster16[n] = 0;
    return n;
}

static inline int compressed_block_size( unsigned char *src ) {
    return 3 + (*(__u16 *)src & 0xfff);
}

static int decompress_block(unsigned char *dest, unsigned char *src) {
    int head;
    int copied=0;
    unsigned char *last;
    int bits;
    int tag=0;

    /* high bit indicates that compression was performed */
    if(!(*(__u16 *)src & 0x8000)) {
        (*pgrub_memmove)(dest,src+2,0x1000);
        return 0x1000;
    }

    if((head = *(__u16 *)src & 0xFFF)==0)
        /* block is not used */
        return 0;

    src += 2;
    last = src+head;
    bits = 0;

    while(src<=last)
    {
        if(copied>4096)
        {
#ifdef DEBUG_NTFS
            printf("decompress error 1\n");
#endif
            *perrnum = ERR_FSYS_CORRUPT;
            return 0;
        }
        if(!bits){
            tag=*(__u8 *)src;
            bits=8;
            src++;
            if(src>last)
                break;
        }
        if(tag & 1){
            int i,len,delta,code,lmask,dshift;
            code = *(__u16 *)src;
            src+=2;
            if(!copied)
            {
#ifdef DEBUG_NTFS
                printf("decompress error 2\n");
#endif
                *perrnum = ERR_FSYS_CORRUPT;
                return 0;
            }
            for(i=copied-1,lmask=0xFFF,dshift=12;i>=0x10;i>>=1)
            {
                lmask >>= 1;
                dshift--;
            }
            delta = code >> dshift;
            len = (code & lmask) + 3;
            for(i=0; i<len; i++)
            {
                dest[copied]=dest[copied-delta-1];
                copied++;
            }
        } else
            dest[copied++]=*(__u8 *)src++;
        tag>>=1;
        bits--;
    }

    return copied;
}
#endif

int ntfs_read(char *buf, int len){
    int ret;
#ifdef STAGE1_5
/* stage2 can't be resident/compressed/encrypted files,
 * but does sparse flag, cause stage2 never sparsed
 */
    if((cmft->attr_flag&~ATTR_SPARSE) != ATTR_NORMAL)
        return 0;
    *pdisk_read_func = *pdisk_read_hook;
    ret = read_attribute(cmft, *pfilepos, buf, len, 0);
    *pdisk_read_func = NULL;
    *pfilepos += ret;
#else

#ifndef NO_NTFS_DECOMPRESSION
    int off;
    int vcn;
    int size;
#endif

    if(len<=0 || *pfilepos > cmft->attr_size || (cmft->attr_flag&ATTR_ENCRYPTED))
        return 0;

    if(*pfilepos+len > cmft->attr_size)
        len = cmft->attr_size - *pfilepos;

    if((cmft->attr_flag&(ATTR_COMPRESSED|ATTR_RESIDENT)) != ATTR_COMPRESSED) {
        if(cmft->attr_flag==ATTR_NORMAL)
            *pdisk_read_func = *pdisk_read_hook;
        ret = read_attribute(cmft, *pfilepos, buf, len, 0);
        if(cmft->attr_flag==ATTR_NORMAL)
            *pdisk_read_func = NULL;
        *pfilepos += ret;
        return ret;
    }

    ret = 0;

#ifndef NO_NTFS_DECOMPRESSION
    /* NTFS don't support compression if cluster size > 4k */
    if(clustersize > 4096) {
        *perrnum = ERR_FSYS_CORRUPT;
        return 0;
    }

    while(len > 0){
        if(*pfilepos >= dcoff && *pfilepos < (dcoff+dclen)) {
            size = dcoff + dclen - *pfilepos;
            if(size > len) size = len;
            (*pgrub_memmove)( buf, dcdbuf + *pfilepos - dcoff, size);
            *pfilepos += size;
            len -= size;
            ret += size;
            if(len==0)
                return ret;
            buf += size;
        }

        vcn = *pfilepos / clustersize / 16;
        vcn *= 16;
        off = *pfilepos % (16 * clustersize);
        if( dcvcn != vcn || *pfilepos < dcoff)
            dcrem = 0;

        if(dcrem) {
            int head;

            /* reading source */
            if(dcslen < 2 || compressed_block_size(dcsptr) > dcslen) {
                if(cluster16[index16]==0) {
                    *perrnum = ERR_FSYS_CORRUPT;
                    return ret;
                }
                if(dcslen)
                    (*pgrub_memmove)(dcsbuf, dcsptr, dcslen);
                dcsptr = dcsbuf;
                while((dcslen+clustersize) < DECOMP_SOURCE_BUFFER_SIZE) {
                    if(cluster16[index16]==0)
                        break;
                    if(!(*pdevread)(cluster16[index16]*(clustersize>>9), 0, clustersize, dcsbuf+dcslen))
                        return ret;
                    dcslen += clustersize;
                    index16++;
                }
            }
            /* flush destination */
            dcoff += dclen;
            dclen = 0;

            while(dcrem && dclen < DECOMP_DEST_BUFFER_SIZE &&
                  dcslen >= 2 && (head=compressed_block_size(dcsptr)) <= dcslen) {
                size = decompress_block(dcdbuf+dclen, dcsptr);
                if(dcrem>=0x1000 && size!=0x1000) {
                    *perrnum = ERR_FSYS_CORRUPT;
                    return ret;
                }
                dcrem -= size;
                dclen += size;
                dcsptr += head;
                dcslen -= head;
            }
            continue;
        }
        dclen = dcrem = 0;
        switch(get_16_cluster(cmft, vcn)) {
        case 0:
            /* sparse */
            size = 16 * clustersize - off;
            if( len < size )
                size = len;
#ifndef STAGE1_5
            (*pgrub_memset)( buf, 0, size);
#endif
            *pfilepos += size;
            len -= size;
            ret += size;
            buf += size;
            break;

        case 16:
            /* uncompressed */
            index16 = off / clustersize;
            off %= clustersize;
            while(index16 < 16) {
                size = clustersize - off;
                if( len < size )
                    size = len;
                if(!(*pdevread)(cluster16[index16]*(clustersize>>9)+(off>>9), off&0x1ff, size, buf))
                    return ret;
                *pfilepos += size;
                len -= size;
                ret += size;
                if(len==0)
                    return ret;
                off = 0;
                buf += size;
                index16++;
            }
            break;

        default:
            index16 = 0;
            dcvcn = vcn;
            dcoff = vcn * clustersize;
            dcrem = *pfilemax - dcoff;
            if(dcrem > 16 * clustersize)
                dcrem = 16 * clustersize;
            dcsptr = dcsbuf;
            dcslen = 0;
        }
    }
#endif
#endif
    return ret;
}

int ntfs_mount (void)
{
    char *sb = (char *)FSYS_BUF;
    int mft_record;
    int spc;

  if (((*pcurrent_drive & 0x80) || (*pcurrent_slice != 0))
       && (*pcurrent_slice != /*PC_SLICE_TYPE_NTFS*/7)
       && (*pcurrent_slice != /*PC_SLICE_TYPE_NTFS*/0x17))
      return 0;

    if (!(*pdevread) (0, 0, 512, (char *) FSYS_BUF))
        return 0;                       /* Cannot read superblock */

    if(sb[3]!='N' || sb[4]!='T' || sb[5]!='F' || sb[6]!='S')
        return 0;
    blocksize = *(__u16 *)(sb+0xb);
    spc = *(unsigned char *)(sb+0xd);
    clustersize = spc * blocksize;
    mft_record_size = *(char *)(sb+0x40);
    index_record_size = *(char *)(sb+0x44);
    if(mft_record_size>0)
        mft_record_size *= clustersize;
    else
        mft_record_size = 1 << (-mft_record_size);

    index_record_size *= clustersize;
    mft_record = *(__u32 *)(sb+0x30); /* only support 32 bit */
    spc = clustersize / 512;

    if(mft_record_size > MAX_MFT_RECORD_SIZE || index_record_size > MAX_INDEX_RECORD_SIZE) {
        /* only support 1k MFT record, 4k INDEX record */
        return 0;
    }

#ifdef DEBUG_NTFS
    printf("spc=%x mft_record=%x:%x\n", spc, *(__s64 *)(sb+0x30));
#endif

    if (!(*pdevread) (mft_record*spc, 0, mft_record_size, (char *)(mmft->mft)))
        return 0;                       /* Cannot read superblock */

    if(!fixup_record((char *)mmft->mft, "FILE", mft_record_size))
        return 0;

#ifndef NO_ALTERNATE_DATASTREAM
    is_ads_completion = 0;
#endif
    if(!search_attribute(mmft, at_data, NONAME)) return 0;

    *mft_run = mmft->runl;

    *path_ino = FILE_ROOT;

    return 1;
}

int
ntfs_dir (char *dirname)
{
    char *rest, ch;
    int namelen;
    int depth = 0;
    int chk_sfn = 1;
    int flag = 0;
    int record_offset;
    int my_index_record_size;
    unsigned char *index_entry = 0, *entry, *index_end;
    int i;

    /* main loop to find desired directory entry */
loop:

#ifdef DEBUG_NTFS
    printf("dirname=%s\n", dirname);
#endif
    if(!read_mft_record(path_ino[depth], (char *)cmft->mft, 0))
    {
#ifdef DEBUG_NTFS
        printf("MFT error 1\n");
#endif
        *perrnum = ERR_FSYS_CORRUPT;
        return 0;
    }

    /* if we have a real file (and we're not just printing possibilities),
       then this is where we want to exit */

    if (!*dirname || (*pgrub_isspace) (*dirname) || *dirname==':')
    {
#ifndef STAGE1_5
#ifndef NO_ALTERNATE_DATASTREAM
        if (*dirname==':' && *pprint_possibilities) {
            char *tmp;

            /* preparing ADS name completion */
            for(tmp = dirname; *tmp != '/'; tmp--);
            for(tmp++, rest=fnbuf; *tmp && !isspace(*tmp); *rest++ = *tmp++)
                if(*tmp==':') dirname = rest;
            *rest++ = '\0';

            is_ads_completion = 1;
            search_attribute(cmft, at_data, dirname+1);
            is_ads_completion = 0;

            if(*perrnum==0) {
                if(*pprint_possibilities < 0)
                    return 1;
                *perrnum = ERR_FILE_NOT_FOUND;
            }
            return 0;
        }
#endif
#endif

        if (*dirname==':') dirname++;
        for (rest = dirname; (ch = *rest) && !(*pgrub_isspace) (ch); rest++);
        *rest = 0;

        if (!search_attribute(cmft, at_data, dirname)) {
            *perrnum = *(dirname-1)==':'?ERR_FILE_NOT_FOUND:ERR_BAD_FILETYPE;
            *rest = ch;
            return 0;
        }
        *rest = ch;

        *pfilemax = cmft->attr_size;
        return 1;
    }

    if(depth >= (MAX_DIR_DEPTH-1)) {
        *perrnum = ERR_FSYS_CORRUPT;
        return 0;
    }

    /* continue with the file/directory name interpretation */

    while (*dirname == '/')
        dirname++;

    for (rest = dirname; (ch = *rest) && !(*pgrub_isspace) (ch) && ch != '/' && ch != ':'; rest++);

    *rest = 0;

    if (!search_attribute(cmft, at_index_root, "$I30"))
    {
        *perrnum = ERR_BAD_FILETYPE;
        return 0;
    }

    read_attribute(cmft, 0, fnbuf, 16, 0);
    my_index_record_size = *(__u32 *)(fnbuf+8);

    if(my_index_record_size > MAX_INDEX_RECORD_SIZE) {
        *perrnum = ERR_FSYS_CORRUPT;
        return 0;
    }

#ifdef DEBUG_NTFS
    printf("index_record_size=%x\n", my_index_record_size);
#endif

    if(cmft->attr_size > MAX_INDEX_RECORD_SIZE) {
        *perrnum = ERR_FSYS_CORRUPT;
        return 0;
    }
    read_attribute(cmft, 0, index_data, cmft->attr_size, 0);
    index_end = (unsigned char *)index_data + cmft->attr_size;
    index_entry = (unsigned char *)index_data + 0x20;
    record_offset = -1;

#ifndef STAGE1_5
    if (*pprint_possibilities && ch != '/' && ch != ':' && !*dirname)
    {
        *pprint_possibilities = - *pprint_possibilities;
        /* fake '.' for empty directory */
        (*pprint_a_completion) (".");
    }
#endif

    if (search_attribute(cmft, at_bitmap, "$I30")) {
        if(cmft->attr_size > MAX_INDEX_BITMAP_SIZE) {
            *perrnum = ERR_FSYS_CORRUPT;
            return 0;
        }

        read_attribute(cmft, 0, (char *)bitmap_data, cmft->attr_size, 0);

        if (search_attribute(cmft, at_index_allocation, "$I30")==0) {
            *perrnum = ERR_FSYS_CORRUPT;
            return 0;
        }

        for(record_offset = 0; record_offset*my_index_record_size<cmft->attr_size; record_offset++){
            int bit = 1 << (record_offset&3);
            int byte = record_offset>>3;
#ifdef DEBUG_NTFS
            printf("record_offset=%x\n", record_offset);
#endif
            if((bitmap_data[byte]&bit))
                break;
        }

        if(record_offset*my_index_record_size>=cmft->attr_size) record_offset = -1;
    }

    do
    {
        entry = index_entry; index_entry += *(__u16 *)(entry+8);
        if(entry+0x50>=index_entry||entry>=index_end||
           index_entry>=index_end||(entry[0x12]&2)){
            if(record_offset < 0 ||
               !read_attribute(cmft, record_offset*my_index_record_size, index_data, my_index_record_size, 0)){
                if (!*perrnum)
                {
                    if (*pprint_possibilities < 0)
                    {
#if 0
                        putchar ('\n');
#endif
                        return 1;
                    }

                    *perrnum = ERR_FILE_NOT_FOUND;
                    *rest = ch;
                }

                return 0;
            }
            if(!fixup_record( index_data, "INDX", my_index_record_size))
            {
#ifdef DEBUG_NTFS
                printf("index error\n");
#endif
                *perrnum = ERR_FSYS_CORRUPT;
                return 0;
            }
            entry = (unsigned char *)index_data + 0x18 + *(__u16 *)(index_data+0x18);
            index_entry = entry + *(__u16 *)(entry+8);
            index_end = (unsigned char *)index_data + my_index_record_size - 0x52;
            for(record_offset++; record_offset*my_index_record_size<cmft->attr_size; record_offset++){
                int bit = 1 << (record_offset&3);
                int byte = record_offset>>3;
                if((bitmap_data[byte]&bit)) break;
            }
            if(record_offset*my_index_record_size>=cmft->attr_size) record_offset = -1;
#ifdef DEBUG_NTFS
            printf("record_offset=%x\n", record_offset);
#endif
        }
        flag = entry[0x51];
        path_ino[depth+1] = *(__u32 *)entry;
        if(path_ino[depth+1] < 16)
            continue;
        namelen = entry[0x50];
        //if(index_data[0x48]&2) printf("hidden file\n");
#ifndef STAGE1_5
        /* skip short file name */
        if( flag == 2 && *pprint_possibilities && ch != '/' && ch != ':' )
            continue;
#endif

        for( i = 0, entry+=0x52; i < namelen; i++, entry+=2 )
        {
            int c = *(__u16 *)entry;
            if(c==' '||c>=0x100)
                fnbuf[i] = '_';
            else
                fnbuf[i] = c;
        }
        fnbuf[namelen] = 0;
#ifdef DEBUG_NTFS
        printf("FLAG: %d  NAME: %s  inum=%d\n", flag,fnbuf,path_ino[depth+1]);
#endif

        //uncntrl(fnbuf);

        chk_sfn = nsubstring(dirname,fnbuf);
#ifndef STAGE1_5
        if (*pprint_possibilities && ch != '/' && ch != ':'
            && (!*dirname || chk_sfn <= 0))
        {
            if (*pprint_possibilities > 0)
                *pprint_possibilities = - *pprint_possibilities;
            (*pprint_a_completion) (fnbuf);
        }
#endif /* STAGE1_5 */
    }
    while (chk_sfn != 0 ||
           (*pprint_possibilities && ch != '/' && ch != ':'));

    *(dirname = rest) = ch;

    depth++;

    /* go back to main loop at top of function */
    goto loop;
}

#endif /* FSYS_NTFS */
