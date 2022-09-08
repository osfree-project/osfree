/*  OS/2 indexed message file structure definitions
 *  (c) osFree project, 2011 Sep 22
 *  author: valerius
 */

#ifndef __MSG_H__
#define __MSG_H__

/* message file header magic */
#define HDR_MAGIC "\0xFFMKMSGF"

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
  char magic[8];                  /* file header magic                       */
  char id[3];                     /* message file ID (SYS, NET, REX or etc.) */
  unsigned short msgs_no;         /* total number of messages in file        */
  unsigned short firstmsgno;      /* first message number                    */
  char is_offs_16bits;            /* are offsets 32/16 bits?                 */
  unsigned short version;         /* file format version                     */
  unsigned short idx_ofs;         /* file index offset                       */
  unsigned short ctry_info_ofs;   /* country info offset                     */
  unsigned short next_ctry_info;  /* next country info (if the file is a multi-language) */
  char pad[7];                    /* padding/reserved                        */
} msghdr_t;

/* country info block list header */
typedef struct
{
  unsigned short block_size;      /* country info block size                  */
  unsigned short blocks_cnt;      /* total number of blocks                   */
} ctry_block_hdr_t;

/* contry info block */
typedef struct
{
  char bytes_per_char;            /* char size (SBCS/DBCS)                    */
  char reserved1[2];              /* reserved                                 */
  unsigned short lang_family_id;  /* language family ID                       */
  unsigned short lang_dialect_id; /* language dialect ID                      */
  unsigned short codepages_no;    /* number of codepages                      */
  unsigned short codepages[16];   /* codepage list                            */
  char filename[260];             /* filename                                 */
} ctry_block_t;

#pragma pack()

#endif /* __MSG_H__ */
