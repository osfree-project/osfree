/*
 * QEMU Block driver for DMG images
 * 
 * Copyright (c) 2004 Johannes E. Schindelin
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*!
 * @file block-dmg.c
 * @brief QEMU block driver for DMG (Apple disk image) files.
 *
 * Read-only support for compressed DMG images, including zlib-compressed,
 * raw-copy and zero-filled chunks.
 */
#include "vl.h"
#include "block_int.h"
#include "bswap.h"
#include <zlib.h>

/*!
 * @brief DMG block driver private state.
 *
 * Each chunk contains a certain number of sectors:
 * @c offsets[i] is the offset in the .dmg file, @c lengths[i] is the length
 * of the compressed chunk, @c sectors[i] is the sector beginning at
 * @c offsets[i], and @c sectorcounts[i] is the number of sectors in that
 * chunk. The sectors array is ordered, with 0 <= i < n_chunks.
 */
typedef struct BDRVDMGState {
    int fd;
    
    /* each chunk contains a certain number of sectors,
     * offsets[i] is the offset in the .dmg file,
     * lengths[i] is the length of the compressed chunk,
     * sectors[i] is the sector beginning at offsets[i],
     * sectorcounts[i] is the number of sectors in that chunk,
     * the sectors array is ordered
     * 0<=i<n_chunks */

    uint32_t n_chunks;
    uint32_t* types;
    uint64_t* offsets;
    uint64_t* lengths;
    uint64_t* sectors;
    uint64_t* sectorcounts;
    uint32_t current_chunk;
    char* compressed_chunk;
    char* uncompressed_chunk;
    z_stream zstream;
} BDRVDMGState;

/*!
 * @brief Probe for the DMG format.
 * @param[in] buf Image header buffer (unused).
 * @param[in] buf_size Buffer size (unused).
 * @param[in] filename Filename to inspect.
 * @retval 2 Filename ends in ".dmg".
 * @retval 0 Otherwise.
 * @return Match score.
 */
static int dmg_probe(const uint8_t *buf, int buf_size, const char *filename)
{
    int len=strlen(filename);
    if(len>4 && !strcmp(filename+len-4,".dmg"))
	return 2;
    return 0;
}

/*!
 * @brief Read a big-endian 64-bit value from a file.
 * @param[in] fd File descriptor.
 * @retval 0 Read failed.
 * @return Big-endian host-converted value.
 */
static off_t read_off(int fd)
{
	uint64_t buffer;
	if(read(fd,&buffer,8)<8)
		return 0;
	return be64_to_cpu(buffer);
}

/*!
 * @brief Read a big-endian 32-bit value from a file.
 * @param[in] fd File descriptor.
 * @retval 0 Read failed.
 * @return Big-endian host-converted value.
 */
static off_t read_uint32(int fd)
{
	uint32_t buffer;
	if(read(fd,&buffer,4)<4)
		return 0;
	return be32_to_cpu(buffer);
}

/*!
 * @brief Open a DMG image.
 * @param[in,out] bs Block driver state.
 * @param[in] filename Image filename.
 * @retval -1 Open or parse failed; caller may fall back to raw.
 * @retval 0 Success.
 * @return 0 on success, -1 on failure.
 */
static int dmg_open(BlockDriverState *bs, const char *filename)
{
    BDRVDMGState *s = bs->opaque;
    off_t info_begin,info_end,last_in_offset,last_out_offset;
    uint32_t count;
    uint32_t max_compressed_size=1,max_sectors_per_chunk=1,i;

    s->fd = open(filename, O_RDONLY | O_BINARY | O_LARGEFILE);
    if (s->fd < 0)
        return -1;
    bs->read_only = 1;
    s->n_chunks = 0;
    s->offsets = s->lengths = s->sectors = s->sectorcounts = 0;
    
    /* read offset of info blocks */
    if(lseek(s->fd,-0x1d8,SEEK_END)<0) {
dmg_close:
	close(s->fd);
	/* open raw instead */
	bs->drv=&bdrv_raw;
	return bs->drv->bdrv_open(bs,filename);
    }
    info_begin=read_off(s->fd);
    if(info_begin==0)
	goto dmg_close;
    if(lseek(s->fd,info_begin,SEEK_SET)<0)
	goto dmg_close;
    if(read_uint32(s->fd)!=0x100)
	goto dmg_close;
    if((count = read_uint32(s->fd))==0)
	goto dmg_close;
    info_end = info_begin+count;
    if(lseek(s->fd,0xf8,SEEK_CUR)<0)
	goto dmg_close;

    /* read offsets */
    last_in_offset = last_out_offset = 0;
    while(lseek(s->fd,0,SEEK_CUR)<info_end) {
        uint32_t type;

	count = read_uint32(s->fd);
	if(count==0)
	    goto dmg_close;
	type = read_uint32(s->fd);
	if(type!=0x6d697368 || count<244)
	    lseek(s->fd,count-4,SEEK_CUR);
	else {
	    int new_size, chunk_count;
	    if(lseek(s->fd,200,SEEK_CUR)<0)
	        goto dmg_close;
	    chunk_count = (count-204)/40;
	    new_size = sizeof(uint64_t) * (s->n_chunks + chunk_count);
	    s->types = realloc(s->types, new_size/2);
	    s->offsets = realloc(s->offsets, new_size);
	    s->lengths = realloc(s->lengths, new_size);
	    s->sectors = realloc(s->sectors, new_size);
	    s->sectorcounts = realloc(s->sectorcounts, new_size);

	    for(i=s->n_chunks;i<s->n_chunks+chunk_count;i++) {
		s->types[i] = read_uint32(s->fd);
		if(s->types[i]!=0x80000005 && s->types[i]!=1 && s->types[i]!=2) {
		    if(s->types[i]==0xffffffff) {
			last_in_offset = s->offsets[i-1]+s->lengths[i-1];
			last_out_offset = s->sectors[i-1]+s->sectorcounts[i-1];
		    }
		    chunk_count--;
		    i--;
		    if(lseek(s->fd,36,SEEK_CUR)<0)
			goto dmg_close;
		    continue;
		}
		read_uint32(s->fd);
		s->sectors[i] = last_out_offset+read_off(s->fd);
		s->sectorcounts[i] = read_off(s->fd);
		s->offsets[i] = last_in_offset+read_off(s->fd);
		s->lengths[i] = read_off(s->fd);
		if(s->lengths[i]>max_compressed_size)
		    max_compressed_size = s->lengths[i];
		if(s->sectorcounts[i]>max_sectors_per_chunk)
		    max_sectors_per_chunk = s->sectorcounts[i];
	    }
	    s->n_chunks+=chunk_count;
	}
    }

    /* initialize zlib engine */
    if(!(s->compressed_chunk=(char*)malloc(max_compressed_size+1)))
	goto dmg_close;
    if(!(s->uncompressed_chunk=(char*)malloc(512*max_sectors_per_chunk)))
	goto dmg_close;
    if(inflateInit(&s->zstream) != Z_OK)
	goto dmg_close;

    s->current_chunk = s->n_chunks;
    
    return 0;
}

/*!
 * @brief Check whether a sector lies within a given chunk.
 * @param[in] s DMG driver state.
 * @param[in] chunk_num Chunk index.
 * @param[in] sector_num Sector number.
 * @retval -1 Sector is inside the chunk.
 * @retval 0 Sector is outside the chunk.
 * @return Non-zero if inside.
 */
static inline int is_sector_in_chunk(BDRVDMGState* s,
		uint32_t chunk_num,int sector_num)
{
    if(chunk_num>=s->n_chunks || s->sectors[chunk_num]>sector_num ||
	    s->sectors[chunk_num]+s->sectorcounts[chunk_num]<=sector_num)
	return 0;
    else
	return -1;
}

/*!
 * @brief Binary-search the chunk containing a sector.
 * @param[in] s DMG driver state.
 * @param[in] sector_num Sector number.
 * @return Chunk index, or @c n_chunks on error.
 */
static inline uint32_t search_chunk(BDRVDMGState* s,int sector_num)
{
    /* binary search */
    uint32_t chunk1=0,chunk2=s->n_chunks,chunk3;
    while(chunk1!=chunk2) {
	chunk3 = (chunk1+chunk2)/2;
	if(s->sectors[chunk3]>sector_num)
	    chunk2 = chunk3;
	else if(s->sectors[chunk3]+s->sectorcounts[chunk3]>sector_num)
	    return chunk3;
	else
	    chunk1 = chunk3;
    }
    return s->n_chunks; /* error */
}

/*!
 * @brief Decompress a chunk into the buffer.
 * @param[in,out] s DMG driver state.
 * @param[in] sector_num Sector within the chunk to read.
 * @retval -1 Read, decompression or lookup failed.
 * @retval 0 Success.
 * @return 0 on success, -1 on failure.
 */
static inline int dmg_read_chunk(BDRVDMGState *s,int sector_num)
{
    if(!is_sector_in_chunk(s,s->current_chunk,sector_num)) {
	int ret;
	uint32_t chunk = search_chunk(s,sector_num);

	if(chunk>=s->n_chunks)
	    return -1;

	s->current_chunk = s->n_chunks;
	switch(s->types[chunk]) {
	case 0x80000005: { /* zlib compressed */
	    int i;

	    ret = lseek(s->fd, s->offsets[chunk], SEEK_SET);
	    if(ret<0)
		return -1;

	    /* we need to buffer, because only the chunk as whole can be
	     * inflated. */
	    i=0;
	    do {
		ret = read(s->fd, s->compressed_chunk+i, s->lengths[chunk]-i);
		if(ret<0 && errno==EINTR)
		    ret=0;
		i+=ret;
	    } while(ret>=0 && ret+i<s->lengths[chunk]);

	    if (ret != s->lengths[chunk])
		return -1;
	
	    s->zstream.next_in = s->compressed_chunk;
	    s->zstream.avail_in = s->lengths[chunk];
	    s->zstream.next_out = s->uncompressed_chunk;
	    s->zstream.avail_out = 512*s->sectorcounts[chunk];
	    ret = inflateReset(&s->zstream);
	    if(ret != Z_OK)
		return -1;
	    ret = inflate(&s->zstream, Z_FINISH);
	    if(ret != Z_STREAM_END || s->zstream.total_out != 512*s->sectorcounts[chunk])
		return -1;
	    break; }
	case 1: /* copy */
	    ret = read(s->fd, s->uncompressed_chunk, s->lengths[chunk]);
	    if (ret != s->lengths[chunk])
		return -1;
	    break;
	case 2: /* zero */
	    memset(s->uncompressed_chunk, 0, 512*s->sectorcounts[chunk]);
	    break;
	}
	s->current_chunk = chunk;
    }
    return 0;
}

/*!
 * @brief Read sectors from a DMG image.
 * @param[in,out] bs Block driver state.
 * @param[in] sector_num First sector number.
 * @param[out] buf Destination buffer.
 * @param[in] nb_sectors Number of sectors to read.
 * @retval -1 Read or decompression failed.
 * @retval 0 Success.
 * @return 0 on success, -1 on failure.
 */
static int dmg_read(BlockDriverState *bs, int64_t sector_num, 
                    uint8_t *buf, int nb_sectors)
{
    BDRVDMGState *s = bs->opaque;
    int i;

    for(i=0;i<nb_sectors;i++) {
	uint32_t sector_offset_in_chunk;
	if(dmg_read_chunk(s, sector_num+i) != 0)
	    return -1;
	sector_offset_in_chunk = sector_num+i-s->sectors[s->current_chunk];
	memcpy(buf+i*512,s->uncompressed_chunk+sector_offset_in_chunk*512,512);
    }
    return 0;
}

/*!
 * @brief Close a DMG image.
 * @param[in,out] bs Block driver state.
 */
static void dmg_close(BlockDriverState *bs)
{
    BDRVDMGState *s = bs->opaque;
    close(s->fd);
    if(s->n_chunks>0) {
	free(s->types);
	free(s->offsets);
	free(s->lengths);
	free(s->sectors);
	free(s->sectorcounts);
    }
    free(s->compressed_chunk);
    free(s->uncompressed_chunk);
    inflateEnd(&s->zstream);
}

/*!
 * @brief DMG block driver descriptor.
 */
BlockDriver bdrv_dmg = {
    "dmg",
    sizeof(BDRVDMGState),
    dmg_probe,
    dmg_open,
    dmg_read,
    NULL,
    dmg_close,
};
