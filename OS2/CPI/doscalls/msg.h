/*  OS/2 indexed message file structure definitions
 *  (c) osFree project, 2011 Sep 22
 *  author: valerius
 */

#ifndef __MSG_H__
#define __MSG_H__

/* message file header magic */
#define HDR_MAGIC {0xff, 0x4d, 0x4b, 0x4d, 0x53, 0x47, 0x46, 0x00}

/* are offsets 16/32 bits? */
#define OFS_32BITS 0
#define OFS_16BITS 1

/* message file format version */
#define FMT_VERSION0 0
#define FMT_VERSION2 2

#pragma pack(1)

/* MSG file header */
typedef struct
{
  char magic[8];
  char id[3];
  unsigned short msgs_no;
  unsigned short firstmsgno;
  char is_offs_16bits;
  unsigned short version;
  unsigned short idx_ofs;
  unsigned short ctry_info_ofs;
  unsigned short next_ctry_info;
} msghdr_t;

/* country info block list header */
typedef struct
{
  unsigned short block_size;
  unsigned short blocks_cnt;
} ctry_block_hdr_t;

/* contry info block */
typedef struct
{
  char bytes_per_char;
  char reserved1[2];
  unsigned short lang_family_id;
  unsigned short lang_dialect_id;
  unsigned short codepages_no;
  unsigned short codepages[16];
  char filename[260];
} ctry_block_t;

#pragma pack()

#endif /* __MSG_H__ */
