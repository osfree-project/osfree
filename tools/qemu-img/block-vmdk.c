/*
 * Block driver for the VMDK format
 *
 * Copyright (c) 2004 Fabrice Bellard
 * Copyright (c) 2005 Filip Navara
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
 * @file block-vmdk.c
 * @brief QEMU block driver for the VMDK format.
 *
 * Supports VMDK3 ("COWD") and VMDK4 ("KDMV") monolithic sparse images,
 * including cluster allocation and backup grain directory maintenance.
 */
#include "vl.h"
#include "block_int.h"

#ifdef __WATCOMC__
#include "porting.h"
#endif

/*!
 * @def VMDK3_MAGIC
 * @brief Magic number identifying a VMDK3 image ("COWD").
 */
#define VMDK3_MAGIC (('C' << 24) | ('O' << 16) | ('W' << 8) | 'D')

/*!
 * @def VMDK4_MAGIC
 * @brief Magic number identifying a VMDK4 image ("KDMV").
 */
#define VMDK4_MAGIC (('K' << 24) | ('D' << 16) | ('M' << 8) | 'V')

/*!
 * @brief On-disk VMDK3 header.
 */
typedef struct {
    uint32_t version;
    uint32_t flags;
    uint32_t disk_sectors;
    uint32_t granularity;
    uint32_t l1dir_offset;
    uint32_t l1dir_size;
    uint32_t file_sectors;
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectors_per_track;
} VMDK3Header;

#ifndef __WATCOMC__
/*!
 * @brief On-disk VMDK4 header.
 */
typedef struct {
    uint32_t version;
    uint32_t flags;
    int64_t capacity;
    int64_t granularity;
    int64_t desc_offset;
    int64_t desc_size;
    int32_t num_gtes_per_gte;
    int64_t rgd_offset;
    int64_t gd_offset;
    int64_t grain_offset;
    char filler[1];
    char check_bytes[4];
} __attribute__((packed)) VMDK4Header;
#else
/*!
 * @brief On-disk VMDK4 header (Watcom packed variant).
 */
typedef _Packed struct {
    uint32_t version;
    uint32_t flags;
    int64_t capacity;
    int64_t granularity;
    int64_t desc_offset;
    int64_t desc_size;
    int32_t num_gtes_per_gte;
    int64_t rgd_offset;
    int64_t gd_offset;
    int64_t grain_offset;
    char filler[1];
    char check_bytes[4];
} VMDK4Header;
#endif

/*!
 * @def L2_CACHE_SIZE
 * @brief Number of cached level-2 tables.
 */
#define L2_CACHE_SIZE 16

/*!
 * @brief VMDK block driver private state.
 */
typedef struct BDRVVmdkState {
    int fd;
    int64_t l1_table_offset;
    int64_t l1_backup_table_offset;
    uint32_t *l1_table;
    uint32_t *l1_backup_table;
    unsigned int l1_size;
    uint32_t l1_entry_sectors;

    unsigned int l2_size;
    uint32_t *l2_cache;
    uint32_t l2_cache_offsets[L2_CACHE_SIZE];
    uint32_t l2_cache_counts[L2_CACHE_SIZE];

    unsigned int cluster_sectors;
} BDRVVmdkState;

/*!
 * @brief Probe for the VMDK format.
 * @param[in] buf Image header buffer.
 * @param[in] buf_size Buffer size.
 * @param[in] filename Filename (unused).
 * @retval 100 VMDK3 or VMDK4 magic matched.
 * @retval 0 Not a VMDK image.
 * @return Match score.
 */
static int vmdk_probe(const uint8_t *buf, int buf_size, const char *filename)
{
    uint32_t magic;

    if (buf_size < 4)
        return 0;
    magic = be32_to_cpu(*(uint32_t *)buf);
    if (magic == VMDK3_MAGIC ||
        magic == VMDK4_MAGIC)
        return 100;
    else
        return 0;
}

/*!
 * @brief Open a VMDK image.
 * @param[in,out] bs Block driver state.
 * @param[in] filename Image filename.
 * @retval -1 Open or parse failed.
 * @retval 0 Success.
 * @return 0 on success, -1 on failure.
 */
static int vmdk_open(BlockDriverState *bs, const char *filename)
{
    BDRVVmdkState *s = bs->opaque;
    int fd, i;
    uint32_t magic;
    int l1_size;

    fd = open(filename, O_RDWR | O_BINARY | O_LARGEFILE);
    if (fd < 0) {
        fd = open(filename, O_RDONLY | O_BINARY | O_LARGEFILE);
        if (fd < 0)
            return -1;
        bs->read_only = 1;
    }
    if (read(fd, &magic, sizeof(magic)) != sizeof(magic))
        goto fail;
    magic = be32_to_cpu(magic);
    if (magic == VMDK3_MAGIC) {
        VMDK3Header header;
        if (read(fd, &header, sizeof(header)) !=
            sizeof(header))
            goto fail;
        s->cluster_sectors = le32_to_cpu(header.granularity);
        s->l2_size = 1 << 9;
        s->l1_size = 1 << 6;
        bs->total_sectors = le32_to_cpu(header.disk_sectors);
        s->l1_table_offset = le32_to_cpu(header.l1dir_offset) << 9;
        s->l1_backup_table_offset = 0;
        s->l1_entry_sectors = s->l2_size * s->cluster_sectors;
    } else if (magic == VMDK4_MAGIC) {
        VMDK4Header header;

        if (read(fd, &header, sizeof(header)) != sizeof(header))
            goto fail;
        bs->total_sectors = le32_to_cpu(header.capacity);
        s->cluster_sectors = le32_to_cpu(header.granularity);
        s->l2_size = le32_to_cpu(header.num_gtes_per_gte);
        s->l1_entry_sectors = s->l2_size * s->cluster_sectors;
        if (s->l1_entry_sectors == 0) // In original <= but how unsigned can be < 0?
            goto fail;
        s->l1_size = (bs->total_sectors + s->l1_entry_sectors - 1)
            / s->l1_entry_sectors;
        s->l1_table_offset = le64_to_cpu(header.rgd_offset) << 9;
        s->l1_backup_table_offset = le64_to_cpu(header.gd_offset) << 9;
    } else {
        goto fail;
    }
    /* read the L1 table */
    l1_size = s->l1_size * sizeof(uint32_t);
    s->l1_table = qemu_malloc(l1_size);
    if (!s->l1_table)
        goto fail;
    if (lseek(fd, s->l1_table_offset, SEEK_SET) == -1)
        goto fail;
    if (read(fd, s->l1_table, l1_size) != l1_size)
        goto fail;
    for(i = 0; i < s->l1_size; i++) {
        le32_to_cpus(&s->l1_table[i]);
    }

    if (s->l1_backup_table_offset) {
        s->l1_backup_table = qemu_malloc(l1_size);
        if (!s->l1_backup_table)
            goto fail;
        if (lseek(fd, s->l1_backup_table_offset, SEEK_SET) == -1)
            goto fail;
        if (read(fd, s->l1_backup_table, l1_size) != l1_size)
            goto fail;
        for(i = 0; i < s->l1_size; i++) {
            le32_to_cpus(&s->l1_backup_table[i]);
        }
    }

    s->l2_cache = qemu_malloc(s->l2_size * L2_CACHE_SIZE * sizeof(uint32_t));
    if (!s->l2_cache)
        goto fail;
    s->fd = fd;
    return 0;
 fail:
    qemu_free(s->l1_backup_table);
    qemu_free(s->l1_table);
    qemu_free(s->l2_cache);
    close(fd);
    return -1;
}

/*!
 * @brief Get the offset of a cluster, optionally allocating it.
 * @param[in,out] bs Block driver state.
 * @param[in] offset Byte offset within the image.
 * @param[in] allocate Non-zero to allocate a cluster when not present.
 * @retval 0 Cluster not allocated or allocation failed.
 * @return Cluster byte offset.
 */
static uint64_t get_cluster_offset(BlockDriverState *bs,
                                   uint64_t offset, int allocate)
{
    BDRVVmdkState *s = bs->opaque;
    unsigned int l1_index, l2_offset, l2_index;
    int min_index, i, j;
    uint32_t min_count, *l2_table, tmp;
    uint64_t cluster_offset;

    l1_index = (offset >> 9) / s->l1_entry_sectors;
    if (l1_index >= s->l1_size)
        return 0;
    l2_offset = s->l1_table[l1_index];
    if (!l2_offset)
        return 0;
    for(i = 0; i < L2_CACHE_SIZE; i++) {
        if (l2_offset == s->l2_cache_offsets[i]) {
            /* increment the hit count */
            if (++s->l2_cache_counts[i] == 0xffffffff) {
                for(j = 0; j < L2_CACHE_SIZE; j++) {
                    s->l2_cache_counts[j] >>= 1;
                }
            }
            l2_table = s->l2_cache + (i * s->l2_size);
            goto found;
        }
    }
    /* not found: load a new entry in the least used one */
    min_index = 0;
    min_count = 0xffffffff;
    for(i = 0; i < L2_CACHE_SIZE; i++) {
        if (s->l2_cache_counts[i] < min_count) {
            min_count = s->l2_cache_counts[i];
            min_index = i;
        }
    }
    l2_table = s->l2_cache + (min_index * s->l2_size);
    lseek(s->fd, (int64_t)l2_offset * 512, SEEK_SET);
    if (read(s->fd, l2_table, s->l2_size * sizeof(uint32_t)) !=
        s->l2_size * sizeof(uint32_t))
        return 0;
    s->l2_cache_offsets[min_index] = l2_offset;
    s->l2_cache_counts[min_index] = 1;
 found:
    l2_index = ((offset >> 9) / s->cluster_sectors) % s->l2_size;
    cluster_offset = le32_to_cpu(l2_table[l2_index]);
    if (!cluster_offset) {
        if (!allocate)
            return 0;
        cluster_offset = lseek(s->fd, 0, SEEK_END);
        ftruncate(s->fd, cluster_offset + (s->cluster_sectors << 9));
        cluster_offset >>= 9;
        /* update L2 table */
        tmp = cpu_to_le32(cluster_offset);
        l2_table[l2_index] = tmp;
        lseek(s->fd, ((int64_t)l2_offset * 512) + (l2_index * sizeof(tmp)), SEEK_SET);
        if (write(s->fd, &tmp, sizeof(tmp)) != sizeof(tmp))
            return 0;
        /* update backup L2 table */
        if (s->l1_backup_table_offset != 0) {
            l2_offset = s->l1_backup_table[l1_index];
            lseek(s->fd, ((int64_t)l2_offset * 512) + (l2_index * sizeof(tmp)), SEEK_SET);
            if (write(s->fd, &tmp, sizeof(tmp)) != sizeof(tmp))
                return 0;
        }
    }
    cluster_offset <<= 9;
    return cluster_offset;
}

/*!
 * @brief Check whether a range of sectors is allocated.
 * @param[in,out] bs Block driver state.
 * @param[in] sector_num First sector number.
 * @param[in] nb_sectors Number of sectors to check.
 * @param[out] pnum Number of consecutive sectors with the same state.
 * @return Non-zero if the first sector is allocated.
 */
static int vmdk_is_allocated(BlockDriverState *bs, int64_t sector_num,
                             int nb_sectors, int *pnum)
{
    BDRVVmdkState *s = bs->opaque;
    int index_in_cluster, n;
    uint64_t cluster_offset;

    cluster_offset = get_cluster_offset(bs, sector_num << 9, 0);
    index_in_cluster = sector_num % s->cluster_sectors;
    n = s->cluster_sectors - index_in_cluster;
    if (n > nb_sectors)
        n = nb_sectors;
    *pnum = n;
    return (cluster_offset != 0);
}

/*!
 * @brief Read sectors from a VMDK image.
 * @param[in,out] bs Block driver state.
 * @param[in] sector_num First sector number.
 * @param[out] buf Destination buffer.
 * @param[in] nb_sectors Number of sectors to read.
 * @retval -1 Read failed.
 * @retval 0 Success.
 * @return 0 on success, -1 on failure.
 */
static int vmdk_read(BlockDriverState *bs, int64_t sector_num,
                    uint8_t *buf, int nb_sectors)
{
    BDRVVmdkState *s = bs->opaque;
    int ret, index_in_cluster, n;
    uint64_t cluster_offset;

    while (nb_sectors > 0) {
        cluster_offset = get_cluster_offset(bs, sector_num << 9, 0);
        index_in_cluster = sector_num % s->cluster_sectors;
        n = s->cluster_sectors - index_in_cluster;
        if (n > nb_sectors)
            n = nb_sectors;
        if (!cluster_offset) {
            memset(buf, 0, 512 * n);
        } else {
            lseek(s->fd, cluster_offset + index_in_cluster * 512, SEEK_SET);
            ret = read(s->fd, buf, n * 512);
            if (ret != n * 512)
                return -1;
        }
        nb_sectors -= n;
        sector_num += n;
        buf += n * 512;
    }
    return 0;
}

/*!
 * @brief Write sectors to a VMDK image.
 * @param[in,out] bs Block driver state.
 * @param[in] sector_num First sector number.
 * @param[in] buf Source buffer.
 * @param[in] nb_sectors Number of sectors to write.
 * @retval -1 Allocation or write failed.
 * @retval 0 Success.
 * @return 0 on success, -1 on failure.
 */
static int vmdk_write(BlockDriverState *bs, int64_t sector_num,
                     const uint8_t *buf, int nb_sectors)
{
    BDRVVmdkState *s = bs->opaque;
    int ret, index_in_cluster, n;
    uint64_t cluster_offset;

    while (nb_sectors > 0) {
        index_in_cluster = sector_num & (s->cluster_sectors - 1);
        n = s->cluster_sectors - index_in_cluster;
        if (n > nb_sectors)
            n = nb_sectors;
        cluster_offset = get_cluster_offset(bs, sector_num << 9, 1);
        if (!cluster_offset)
            return -1;
        lseek(s->fd, cluster_offset + index_in_cluster * 512, SEEK_SET);
        ret = write(s->fd, buf, n * 512);
        if (ret != n * 512)
            return -1;
        nb_sectors -= n;
        sector_num += n;
        buf += n * 512;
    }
    return 0;
}

/*!
 * @brief Create a VMDK4 monolithic sparse image.
 * @param[in] filename Image filename.
 * @param[in] total_size Image size in 512-byte sectors.
 * @param[in] backing_file Backing file name (unsupported).
 * @param[in] flags Creation flags (unused).
 * @retval -1 Creation failed.
 * @retval 0 Success.
 * @return 0 on success, -1 on failure.
 */
static int vmdk_create(const char *filename, int64_t total_size,
                       const char *backing_file, int flags)
{
    int fd, i;
    VMDK4Header header;
    uint32_t tmp, magic, grains, gd_size, gt_size, gt_count;
    char *desc_template =
        "# Disk DescriptorFile\n"
        "version=1\n"
        "CID=%x\n"
        "parentCID=ffffffff\n"
        "createType=\"monolithicSparse\"\n"
        "\n"
        "# Extent description\n"
        "RW %lu SPARSE \"%s\"\n"
        "\n"
        "# The Disk Data Base \n"
        "#DDB\n"
        "\n"
        "ddb.virtualHWVersion = \"3\"\n"
        "ddb.geometry.cylinders = \"%lu\"\n"
        "ddb.geometry.heads = \"16\"\n"
        "ddb.geometry.sectors = \"63\"\n"
        "ddb.adapterType = \"ide\"\n";
    char desc[1024];
    const char *real_filename, *temp_str;

    /* XXX: add support for backing file */

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_LARGEFILE,
              0644);
    if (fd < 0)
        return -1;
    magic = cpu_to_be32(VMDK4_MAGIC);
    memset(&header, 0, sizeof(header));
    header.version = cpu_to_le32(1);
    header.flags = cpu_to_le32(3); /* ?? */
    header.capacity = cpu_to_le64(total_size);
    header.granularity = cpu_to_le64(128);
    header.num_gtes_per_gte = cpu_to_le32(512);

    grains = (total_size + header.granularity - 1) / header.granularity;
    gt_size = ((header.num_gtes_per_gte * sizeof(uint32_t)) + 511) >> 9;
    gt_count = (grains + header.num_gtes_per_gte - 1) / header.num_gtes_per_gte;
    gd_size = (gt_count * sizeof(uint32_t) + 511) >> 9;

    header.desc_offset = 1;
    header.desc_size = 20;
    header.rgd_offset = header.desc_offset + header.desc_size;
    header.gd_offset = header.rgd_offset + gd_size + (gt_size * gt_count);
    header.grain_offset =
       ((header.gd_offset + gd_size + (gt_size * gt_count) +
         header.granularity - 1) / header.granularity) *
        header.granularity;

    header.desc_offset = cpu_to_le64(header.desc_offset);
    header.desc_size = cpu_to_le64(header.desc_size);
    header.rgd_offset = cpu_to_le64(header.rgd_offset);
    header.gd_offset = cpu_to_le64(header.gd_offset);
    header.grain_offset = cpu_to_le64(header.grain_offset);

    header.check_bytes[0] = 0xa;
    header.check_bytes[1] = 0x20;
    header.check_bytes[2] = 0xd;
    header.check_bytes[3] = 0xa;

    /* write all the data */
    write(fd, &magic, sizeof(magic));
    write(fd, &header, sizeof(header));

    ftruncate(fd, header.grain_offset << 9);

    /* write grain directory */
    lseek(fd, le64_to_cpu(header.rgd_offset) << 9, SEEK_SET);
    for (i = 0, tmp = header.rgd_offset + gd_size;
         i < gt_count; i++, tmp += gt_size)
        write(fd, &tmp, sizeof(tmp));

    /* write backup grain directory */
    lseek(fd, le64_to_cpu(header.gd_offset) << 9, SEEK_SET);
    for (i = 0, tmp = header.gd_offset + gd_size;
         i < gt_count; i++, tmp += gt_size)
        write(fd, &tmp, sizeof(tmp));

    /* compose the descriptor */
    real_filename = filename;
    if ((temp_str = strrchr(real_filename, '\\')) != NULL)
        real_filename = temp_str + 1;
    if ((temp_str = strrchr(real_filename, '/')) != NULL)
        real_filename = temp_str + 1;
    if ((temp_str = strrchr(real_filename, ':')) != NULL)
        real_filename = temp_str + 1;
    sprintf(desc, desc_template, time(NULL), (unsigned long)total_size,
            real_filename, total_size / (63 * 16));

    /* write the descriptor */
    lseek(fd, le64_to_cpu(header.desc_offset) << 9, SEEK_SET);
    write(fd, desc, strlen(desc));

    close(fd);
    return 0;
}

/*!
 * @brief Close a VMDK image.
 * @param[in,out] bs Block driver state.
 */
static void vmdk_close(BlockDriverState *bs)
{
    BDRVVmdkState *s = bs->opaque;
    qemu_free(s->l1_table);
    qemu_free(s->l2_cache);
    close(s->fd);
}

/*!
 * @brief VMDK block driver descriptor.
 */
BlockDriver bdrv_vmdk = {
    "vmdk",
    sizeof(BDRVVmdkState),
    vmdk_probe,
    vmdk_open,
    vmdk_read,
    vmdk_write,
    vmdk_close,
    vmdk_create,
    vmdk_is_allocated,
};
