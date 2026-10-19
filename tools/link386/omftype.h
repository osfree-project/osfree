/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  Structures, defines and enums for OMF files.
*
****************************************************************************/

/*!
 * @file omftype.h
 * @brief Structures, defines and enums for OMF files.
 */

#ifndef OMF_TYPE_INCLUDED
#define OMF_TYPE_INCLUDED

/*!
 * @def OMFENTRY
 * @brief Calling convention marker for OMF entry points.
 */
#define OMFENTRY

/* status flags
 */

/*!
 * @def OMF_STATUS_FILE_LOADED
 * @brief The OMF file has been fully parsed.
 */
#define OMF_STATUS_FILE_LOADED          0x00000001
/*!
 * @def OMF_STATUS_ARCH_SET
 * @brief The target architecture has been determined.
 */
#define OMF_STATUS_ARCH_SET             0x00000002
/*!
 * @def OMF_STATUS_ADD_LIDATA
 * @brief A deferred LIDATA record is pending.
 */
#define OMF_STATUS_ADD_LIDATA           0x00000008
/*!
 * @def OMF_STATUS_EASY_OMF
 * @brief The file uses the PharLap Easy OMF variant.
 */
#define OMF_STATUS_EASY_OMF             0x00000010
/*!
 * @def OMF_STATUS_ADD_BAKPAT
 * @brief A deferred BAKPAT/NBKPAT record is pending.
 */
#define OMF_STATUS_ADD_BAKPAT           0x00000020

/*!
 * @def OMF_STATUS_ADD_MASK
 * @brief Mask for deferred-record status bits.
 */
#define OMF_STATUS_ADD_MASK             (OMF_STATUS_ADD_LIDATA | OMF_STATUS_ADD_BAKPAT)

/* section flags
 */

/*!
 * @def OMF_SEG_IS_32
 * @brief The segment uses 32-bit addressing.
 */
#define OMF_SEG_IS_32                   0x00000001
/*!
 * @def OMF_SEG_IS_BIG
 * @brief The segment is "big" (exactly 64K or 4G).
 */
#define OMF_SEG_IS_BIG                  0x00000002
/*!
 * @def OMF_SEG_IS_ABS
 * @brief The segment has an absolute frame number.
 */
#define OMF_SEG_IS_ABS                  0x00000004

/* symbol flags
 */

/*!
 * @def OMF_SYM_FLAGS_LOCAL
 * @brief The symbol is local to this module.
 */
#define OMF_SYM_FLAGS_LOCAL             0x00000001
/*!
 * @def OMF_SYM_FLAGS_COMDEF
 * @brief The symbol is a communal definition.
 */
#define OMF_SYM_FLAGS_COMDEF            0x00000002
/*!
 * @def OMF_SYM_FLAGS_GRPDEF
 * @brief The symbol is a group definition.
 */
#define OMF_SYM_FLAGS_GRPDEF            0x00000004

/*!
 * @brief Pseudo section indices used internally by the OMF reader.
 */
enum{
    OMF_SEC_NEXT_AVAILABLE = -1,        /*!< Allocate the next free index. */
    OMF_SEC_LNAME_INDEX,                /*!< LNAMES string table index. */
    OMF_SEC_IMPORT_INDEX,               /*!< EXTDEF string table index. */
    OMF_SEC_RELOC_INDEX,                /*!< Reloc section index. */
    OMF_SEC_SYM_TABLE_INDEX,            /*!< Symbol table index. */
    OMF_SEC_COMMENT_INDEX,              /*!< Comment section index. */
    OMF_SEC_DATA_CODE_START             /*!< First user section index. */
};

/*!
 * @brief Debug information style recognized by the reader.
 */
/* Debug info styles we recognize */
enum {
    OMF_DBG_STYLE_UNKNOWN = -1,         /*!< Unrecognized style. */
    OMF_DBG_STYLE_CODEVIEW,             /*!< Microsoft CodeView style. */
    OMF_DBG_STYLE_HLL                    /*!< IBM HLL style. */
};

/*!
 * @brief OMF file flags type.
 */
// handle definitions

typedef uint_8                          omf_file_flags;
/*!
 * @brief OMF file index type.
 */
typedef uint_32                         omf_file_index;
/*!
 * @brief OMF header size type.
 */
typedef uint_32                         omf_headers_size;

/*!
 * @brief OMF section size type.
 */
typedef uint_32                         omf_sec_size;
/*!
 * @brief OMF section flags type.
 */
typedef uint_32                         omf_sec_flags;
/*!
 * @brief OMF section offset type.
 */
typedef uint_32                         omf_sec_offset;

/*!
 * @brief OMF symbol flags type.
 */
typedef uint_32                         omf_sym_flags;

/*!
 * @brief OMF relocation type type.
 */
typedef uint_16                         omf_reloc_type;
/*!
 * @brief OMF index type.
 */
typedef uint_16                         omf_idx;
/*!
 * @brief OMF frame type.
 */
typedef uint_16                         omf_frame;

/*!
 * @brief OMF quantity type.
 */
typedef int_32                          omf_quantity;

/*!
 * @brief OMF byte-buffer pointer type.
 */
typedef uint_8                          *omf_bytes;
/*!
 * @brief OMF record type type.
 */
typedef uint_8                          omf_rectyp;
/*!
 * @brief OMF debug-style type.
 */
typedef int_8                           omf_dbg_style;

/*!
 * @brief OMF handle.
 */
typedef struct omf_handle_struct        omf_handle_struct;
typedef omf_handle_struct               *omf_handle;

/*!
 * @brief OMF file handle.
 */
typedef struct omf_file_handle_struct   omf_file_handle_struct;
typedef omf_file_handle_struct          *omf_file_handle;

/*!
 * @brief OMF section handle.
 */
typedef struct omf_sec_handle_struct    omf_sec_handle_struct;
typedef omf_sec_handle_struct           *omf_sec_handle;

/*!
 * @brief OMF symbol handle.
 */
typedef struct omf_symbol_handle_struct omf_symbol_handle_struct;
typedef omf_symbol_handle_struct        *omf_symbol_handle;

/*!
 * @brief OMF group handle.
 */
typedef struct omf_grp_handle_struct    omf_grp_handle_struct;
typedef omf_grp_handle_struct           *omf_grp_handle;

/*!
 * @brief OMF relocation handle.
 */
typedef orl_reloc                       omf_reloc_handle_struct;
typedef omf_reloc_handle_struct         *omf_reloc_handle;

/*!
 * @brief Deferred LIDATA state.
 */
typedef struct omf_tmp_lidata_struct    omf_tmp_lidata_struct;
typedef omf_tmp_lidata_struct           *omf_tmp_lidata;

/*!
 * @brief Deferred fixup state.
 */
typedef struct omf_tmp_fixup_struct     omf_tmp_fixup_struct;
typedef omf_tmp_fixup_struct            *omf_tmp_fixup;

/*!
 * @brief Deferred backpatch queue.
 */
typedef struct omf_tmp_bakpat_struct    omf_tmp_bakpat_struct;
typedef omf_tmp_bakpat_struct           *omf_tmp_bakpat;

/*!
 * @brief Deferred backpatch entry.
 */
typedef struct omf_tmp_bkfix_struct    omf_tmp_bkfix_struct;
typedef omf_tmp_bkfix_struct           *omf_tmp_bkfix;

/*!
 * @brief THRED fixup state.
 */
typedef struct omf_thred_fixup_struct   omf_thred_fixup;

/*!
 * @struct omf_tmp_lidata_struct
 * @brief Deferred LIDATA record state.
 */
struct omf_tmp_lidata_struct {
    uint_32             size;           /*!< Remaining size in bytes. */
    uint_32             used;           /*!< Consumed size in bytes. */
    uint_32             offset;         /*!< Current expanded offset. */
    int                 is32;           /*!< Non-zero for a 32-bit record. */
    omf_tmp_fixup       first_fixup;    /*!< First pending fixup. */
    omf_tmp_fixup       last_fixup;     /*!< Last pending fixup. */
    omf_tmp_fixup       new_fixup;      /*!< First rewritten fixup. */
};


/*!
 * @struct omf_tmp_fixup_struct
 * @brief Deferred fixup record.
 */
struct omf_tmp_fixup_struct {
    omf_tmp_fixup       next;           /*!< Next fixup in the queue. */
    int                 is32;           /*!< Non-zero for a 32-bit record. */
    int                 mode;           /*!< Segment-relative (1) or self-relative (0). */
    int                 location;       /*!< OMF location specifier. */
    orl_sec_offset      offset;         /*!< Offset within the section. */
    int                 fmethod;        /*!< Frame method. */
    omf_idx             fidx;           /*!< Frame index. */
    int                 tmethod;        /*!< Target method. */
    omf_idx             tidx;           /*!< Target index. */
    orl_sec_offset      disp;           /*!< Displacement. */
};

/*!
 * @struct omf_tmp_bakpat_struct
 * @brief Deferred backpatch queue.
 */
struct omf_tmp_bakpat_struct {
    omf_tmp_bkfix       first_fixup;    /*!< First queued backpatch. */
    omf_tmp_bkfix       last_fixup;     /*!< Last queued backpatch. */
};

/*!
 * @struct omf_tmp_bkfix_struct
 * @brief Deferred backpatch entry.
 */
struct omf_tmp_bkfix_struct {
    omf_tmp_bkfix       next;           /*!< Next backpatch in the queue. */
    orl_reloc_type      reltype;        /*!< Word size to backpatch. */
    omf_idx             segidx;         /*!< Target segment index (BAKPAT). */
    omf_idx             symidx;         /*!< Target COMDAT name index (NBKPAT). */
    orl_sec_offset      offset;         /*!< Offset within the target section. */
    orl_sec_offset      disp;           /*!< Displacement to apply. */
};

/*!
 * @struct omf_handle_struct
 * @brief Top-level OMF handle state.
 */
struct omf_handle_struct {
    orl_funcs *         funcs;          /*!< Allocator/reader callback table. */
    omf_file_handle     first_file_hnd; /*!< Head of the file handle list. */
};

/*!
 * @struct omf_thred_fixup_struct
 * @brief Saved THRED fixup state.
 */
struct omf_thred_fixup_struct {
    unsigned int        idx;            /*!< Saved index. */
    unsigned char       method;         /*!< Saved method. */
};

/*!
 * @struct omf_file_handle_struct
 * @brief Per-file OMF parsing state.
 */
struct omf_file_handle_struct {
    omf_handle          omf_hnd;        /*!< Owning OMF handle. */
    omf_file_handle     next;           /*!< Next file in the list. */
    void                *file;          /*!< Caller's file object. */
    unsigned char       *parsebuf;      /*!< Current record buffer. */
    unsigned short      parselen;       /*!< Current record length. */
    long                status;         /*!< OMF_STATUS_* flags. */

    omf_sec_handle      lnames;         /*!< LNAMES string table. */
    omf_sec_handle      extdefs;        /*!< EXTDEF string table. */
    omf_sec_handle      relocs;         /*!< Reloc section. */
    omf_sec_handle      comments;       /*!< COMENT section. */

    omf_thred_fixup     frame_thred[4]; /*!< Saved frame THREDs. */
    omf_thred_fixup     target_thred[4];/*!< Saved target THREDs. */

    omf_sec_handle      first_sec;      /*!< First section in the list. */
    omf_sec_handle      last_sec;       /*!< Last section in the list. */
    omf_quantity        next_idx;       /*!< Next section index to allocate. */

    omf_sec_handle      *segs;          /*!< Segment table. */
    omf_quantity        num_segs;       /*!< Number of segments. */

    omf_sec_handle      *comdats;       /*!< COMDAT table. */
    omf_quantity        num_comdats;    /*!< Number of COMDATs. */

    omf_grp_handle      *groups;        /*!< Group table. */
    omf_quantity        num_groups;     /*!< Number of groups. */

    orl_machine_type    machine_type;   /*!< Target machine type. */
    orl_file_type       type;           /*!< OMF file type. */
    orl_file_size       size;           /*!< OMF file size. */
    orl_file_flags      flags;          /*!< OMF file flags. */
    omf_quantity        num_sections;   /*!< Number of sections. */
    omf_sec_handle      work_sec;       /*!< Current work section. */

    omf_tmp_lidata      lidata;         /*!< Deferred LIDATA state. */
    omf_tmp_bakpat      bakpat;         /*!< Deferred backpatch queue. */

    omf_dbg_style       debug_style;    /*!< Debug information style. */

    omf_sec_handle      symbol_table;   /*!< Symbol table section. */
    omf_rectyp          last_rec;       /*!< Last record type byte. */
};

/*!
 * @def SEC_NAME_HASH_TABLE_SIZE
 * @brief Size of the section-name hash table.
 */
#define SEC_NAME_HASH_TABLE_SIZE        53
/*!
 * @def STRING_HASH_TABLE_SIZE
 * @brief Size of the string hash table.
 */
#define STRING_HASH_TABLE_SIZE          53
/*!
 * @def STD_HASH_TABLE_SIZE
 * @brief Standard hash table size.
 */
#define STD_HASH_TABLE_SIZE             257

/*!
 * @def MAX_LNAMES
 * @brief Maximum number of LNAME entries.
 */
#define MAX_LNAMES                      256

/*!
 * @struct omf_comdat_struct
 * @brief COMDAT association data.
 */
typedef struct omf_comdat_struct {
    omf_sec_handle      assoc_seg;      /*!< Associated explicit segment. */
    omf_grp_handle      group;          /*!< Associated group. */
    uint_32             frame;          /*!< Associated frame. */
} omf_comdat_struct;

/*!
 * @struct omf_seg_assoc_struct
 * @brief Segment association data.
 */
struct omf_seg_assoc_struct {
    omf_idx             name;           /*!< LNAME index for the segment. */
    omf_idx             class;          /*!< LNAME index for the class. */
    orl_sec_alignment   alignment;      /*!< Alignment value. */
    orl_sec_combine     combine;        /*!< Combine attribute. */
    orl_sec_frame       frame;          /*!< Absolute frame number. */
    uint_32             seg_flags;      /*!< OMF_SEG_* flags. */
    uint_32             cur_size;       /*!< Current contents buffer size. */
    uint_32             cur_offset;     /*!< Current write offset. */
    omf_idx             seg_id;         /*!< Segment index. */
    omf_symbol_handle   sym;            /*!< Segment symbol. */
    omf_comdat_struct   comdat;         /*!< COMDAT association data. */
    omf_grp_handle      group;          /*!< Group membership. */
    orl_linnum          *lines;         /*!< Line-number array. */
    orl_table_index     num_lines;      /*!< Number of line numbers. */
};

/*!
 * @struct omf_reloc_assoc_struct
 * @brief Relocation association data.
 */
struct omf_reloc_assoc_struct {
    omf_quantity        num;            /*!< Number of relocations. */
    omf_reloc_handle    *relocs;        /*!< Relocation array. */
};

/*!
 * @struct omf_sym_assoc_struct
 * @brief Symbol table association data.
 */
struct omf_sym_assoc_struct {
    omf_quantity        num;            /*!< Number of symbols. */
    omf_symbol_handle   *syms;          /*!< Symbol array. */
    orl_hash_table      hash_tab;       /*!< Symbol hash table. */
};

/*!
 * @struct omf_string_struct
 * @brief One string table entry.
 */
typedef struct omf_string_struct {
    unsigned char       len;            /*!< String length in bytes. */
    char                string[1];      /*!< NUL-terminated string. */
} omf_string_struct;

/*!
 * @struct omf_string_assoc_struct
 * @brief String table association data.
 */
struct omf_string_assoc_struct {
    omf_quantity        num;            /*!< Number of strings. */
    omf_string_struct   **strings;      /*!< String array. */
};

/*!
 * @struct omf_comment_struct
 * @brief One comment record.
 */
typedef struct omf_comment_struct {
    uint_8              class;          /*!< Comment class. */
    uint_8              flags;          /*!< Comment flags byte. */
    omf_sec_offset      len;            /*!< Payload length. */
    unsigned char       data[1];        /*!< Payload bytes. */
} omf_comment_struct;

/*!
 * @struct omf_comment_assoc_struct
 * @brief Comment section association data.
 */
struct omf_comment_assoc_struct {
    omf_quantity        num;            /*!< Number of comments. */
    omf_comment_struct  **comments;     /*!< Comment array. */
};

/*!
 * @struct omf_sec_handle_struct
 * @brief OMF section state.
 */
struct omf_sec_handle_struct {
    orl_file_format     file_format;    /*!< File format (ORL_OMF). */
    omf_file_handle     omf_file_hnd;   /*!< Owning file handle. */
    omf_sec_handle      next;           /*!< Next section in the list. */
    orl_sec_size        size;           /*!< Declared size in bytes. */
    orl_sec_type        type;           /*!< Section type. */
    orl_sec_flags       flags;          /*!< Section flags. */
    omf_bytes           contents;       /*!< Contents buffer. */
    omf_quantity        index;          /*!< Section index. */
    // assoc - things associated with the section
    union {                             /*!< Type-specific association data. */
        struct omf_seg_assoc_struct     seg;
        struct omf_reloc_assoc_struct   reloc;
        struct omf_string_assoc_struct  string;
        struct omf_comment_assoc_struct comment;
        struct omf_sym_assoc_struct     sym;
    } assoc;
};

/*!
 * @struct omf_symbol_handle_struct
 * @brief OMF symbol state.
 */
struct omf_symbol_handle_struct {
    orl_file_format     file_format;    /*!< File format (ORL_OMF). */
    omf_file_handle     omf_file_hnd;   /*!< Owning file handle. */
    orl_symbol_type     typ;            /*!< Symbol type flags. */
    int                 rec_typ;        /*!< Record type that defined it. */
    omf_idx             idx;            /*!< Symbol index. */
    omf_idx             seg;            /*!< Segment index. */
    omf_frame           frame;          /*!< Absolute frame number. */
    omf_sec_offset      offset;         /*!< Symbol offset. */
    omf_sym_flags       flags;          /*!< OMF_SYM_FLAGS_* flags. */
    omf_sec_handle      section;        /*!< Containing section. */
    orl_symbol_binding  binding;        /*!< Symbol binding. */
    int                 namelen;        /*!< Name length in bytes. */
    char                name[1];        /*!< NUL-terminated name. */
};

/*!
 * @struct omf_grp_handle_struct
 * @brief OMF group state.
 */
struct omf_grp_handle_struct {
    orl_file_format     file_format;    /*!< File format (ORL_OMF). */
    omf_file_handle     omf_file_hnd;   /*!< Owning file handle. */
    omf_idx             name;           /*!< Group name index. */
    omf_idx             id;             /*!< Group identifier. */
    omf_quantity        size;           /*!< Number of member segments. */
    omf_idx             *segs;          /*!< Member segment indices. */
    omf_symbol_handle   sym;            /*!< Group symbol. */
};

/*!
 * @struct omf_reloc_handle_struct
 * @brief OMF relocation state.
 */
struct omf_reloc_handle_struct {
    omf_sec_handle      section;        /*!< Section being relocated. */
    orl_sec_offset      locale;         /*!< Locale value. */
    omf_symbol_handle   symbol;         /*!< Target symbol. */
    orl_reloc_type      type;           /*!< Relocation type. */
    omf_sec_offset      offset;         /*!< Offset within the section. */
};

/*!
 * @struct omf_scan_tab_struct
 * @brief Parsed scan-table disassembly directive.
 */
typedef struct omf_scan_tab_struct {
    omf_idx             seg;            /*!< Segment index. */
    omf_idx             lname;          /*!< COMDAT name index. */
    orl_sec_offset      start;          /*!< Start offset. */
    orl_sec_offset      end;            /*!< End offset (exclusive). */
} omf_scan_tab_struct;

#endif
