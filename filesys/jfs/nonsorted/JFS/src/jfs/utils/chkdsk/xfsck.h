/* $Id: xfsck.h,v 1.1.1.1 2003/05/21 13:40:52 pasha Exp $ */

/* static char *SCCSID = "@(#)1.27  6/28/99 13:50:25 src/jfs/utils/chkdsk/xfsck.h, jfschk, w45.fs32, fixbld";*/
/*
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *  
 *   MODULE_NAME:		xfsck.h
 *
 *   COMPONENT_NAME: 	jfschk
 *
 *	common data & function prototypes
 *
 * 
 */

#ifndef H_XFSCK
#define H_XFSCK

typedef int    retcode_t;

/*

   ------------------------------ defines ------------------------------

*/

#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_DOS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSDATETIME
#define INCL_DOSEXCEPTIONS

/*

   -------------------------- system includes  --------------------------

*/
#ifndef _JFS_FSCKDIRE
#include <os2.h>  
#endif 

#include <builtin.h>
#include <extboot.h>
#include <sysbloks.h>

#include <stdarg.h>
#include <stddef.h>

#ifndef _JFS_FSCKDIRE
#include <stdio.h>
#endif

#include <string.h>
#include <stdlib.h>


/*

   ---------------------------- JFS includes  ----------------------------

*/

/* JFS includes */

#include "jfs_types.h"

/* Prevent conflict between unikern.h & uconv.h */
#undef ULS_INVALID
#undef ULS_ILLEGALSEQUENCE
#undef ULS_BADOBJECT
#undef ULS_BUFFERFULL

#ifndef _JFS_FSCKDIRE
#include "jfs_aixisms.h"
#endif

#include "jfs_btree.h"
#include "jfs_dinode.h"
#include "jfs_dmap.h"
#include "jfs_btree.h"
#include "jfs_dtree.h"
#include "jfs_filsys.h"
#include "jfs_imap.h"
#include "jfs_superblock.h"
#include "jfs_xtree.h"
#include "fsckcbbl.h"


/*
 * unicode stuff
 */
#include <uconv.h>

/*
 * jfs library routine includes
 */
#include "libjufs.h"
#include "bootsec.h"
#include "devices.h"
#include "diskmap.h"
#include "inode.h"
#include "logform.h"
#include "logredo.h"
#include "message.h"
#include "super.h"

extern int jfs_ValidateFEAList( FEALIST *, ULONG * );

/*

   --------- Defines and typedefs shared among the fsck modules ---------

*/

#define ISDIR(m)         (((m)&(IFMT)) == (IFDIR))
#define ISREG(m)         (((m)&(IFMT)) == (IFREG))
#define ISLNK(m)         (((m)&(IFMT)) == (IFLNK))


/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 *
 * miscellaneous types used by fsck
 */
typedef int64    agg_byteidx_t;
typedef int64    fsblkidx_t;
typedef uint32   inoidx_t;
typedef uint64   long_idx_t;
typedef int64    mappgidx_t;
typedef uint32   reg_idx_t;
typedef int32    iagno_t;


/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 * some useful constants
 */
#define BYTESPERPAGE      4096
#define log2BYTESPERPAGE  12
#define BITSPERPAGE       (4096*8)
#define log2BITSPERPAGE   15
#define BITSPERDWORD      32
#define log2BITSPERDWORD  5
#define BYTESPERDWORD     4
#define log2BYTESPERDWORD 2
#define BITSPERBYTE       8
#define log2BITSPERBYTE   3
#define MEMSEGSIZE        (64*1024)
#define log2BYTESPERKBYTE 10


/* ***** IMPORTANT ***** IMPORTANT ***** IMPORTANT ***** IMPORTANT *****
 *
 * fsck_first_msgid
 * 	MUST be set to the first message id for fsck, according
 *	to jfs.txt.  Will be used to locate the text for any
 *	message which is displayed in the local language.
 *
 * fsck_highest_msgid_defined 
 *	MUST be maintained in synch with the
 * 	message id constants (defined in fsckmsgc.h) since the
 * 	message text array and the message attributes array (declared
 * 	in fsckmsgp.h) are both dimensioned using it.
 *
 * ***** IMPORTANT ***** IMPORTANT ***** IMPORTANT ***** IMPORTANT ***** */

#define fsck_msgid_offset           50
#define fsck_highest_msgid_defined 599

#define MAXPARMLEN 64

#define fscklog_var_text 1
#define fscklog_literal  2


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * The following are used to access the columns of
  * MsgProtocol[][], which is defined in fsckmsgp.h
  *
  */
#define MP_STREAM     0
#define MP_MSGLVL     1
#define MP_MSGFILE    2
#define MP_HPFSMSGNUM 3

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * The following are the possible values effective fsck
  * messaging level and for implied response level.
  *
  */

/*
 * special cases (must not match any other constants in the table)
 */
#define fsck_suffix            128
#define fsck_notqry            129
#define fsck_hrtbt             130
#define fsck_txtins            131

/*
 * The lowest messaging level (dictated by input parms) at which
 * the message is displayed.
 */
#define fsck_quiet		2
#define fsck_terse		4
#define fsck_verbose		8
#define fsck_autochk		16
#define fsck_debug		32

/*
 * The message file to use for local language lookup
 *
 *   These are determined by the array message_file_name[] 
 *   in messages.c
 */
#define no_msgfile              -1
#define jfs_msgfile              1
#define hpfs_msgfile             0

/*
 * The (hpfs) message number if hpfs_msgfile 
 */
#define not_an_hpfs_msg          0

/*
 * Whether the message is English-only (intended for debugging purposes) or
 * needs to be displayed in the local language.
 */
#define fsck_engl_only      0
#define fsck_local_lang     1

/*
 * constants to select standard I/O stream
 */
#define fsck_out        0
#define fsck_err        1
#define fsck_in         2

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 * The following are used for reporting storage allocation
 * failures.  
 */

#ifndef _JFS_FSCKDIRE

void report_dynstg_error ( void );

#define  dynstg_unknown                 0

/* the actions */

#define dynstg_allocation                1
#define dynstg_initialization            2

/* the objects */

#define dynstg_unknown_object    0
#define dynstg_blkmap            1
#define dynstg_blkmap_buf        2
#define dynstg_blkmap_hdr        3
#define dynstg_inomap            4
#define dynstg_fer               5
#define dynstg_wspext            6
#define dynstg_pathbuffer        7
#define dynstg_inoextrec         8
#define dynstg_inorec            9
#define dynstg_dtreeQ_elem      10
#define dynstg_treeQ_elem       11
#define dynstg_ait_map          12
#define dynstg_fsit_map         13
#define dynstg_dupall_blkrec    14
#define dynstg_dupall_inorec    15
#define dynstg_agg_agtbl        16
#define dynstg_agg_iagtbl       17
#define dynstg_fs_agtbl         18
#define dynstg_fs_iagtbl        19
#define dynstg_iobufs           20
#define dynstg_eaiobuf          21
#define dynstg_tmpinoiobuf      22
#define dynstg_recondnodebuf    23
#define dynstg_xtreebuf         24
#define dynstg_xtreepagebuf     25
#define dynstg_treeStack_elem   26
#define dynstg_fsckcbblbuf1      27
#define dynstg_fsckcbblbuf2      28
#define dynstg_inotbl                29                  /* @F1*/
#define dynstg_inoexttbl            30                 /* @F1*/
#define dynstg_fsit_iagtbl           31                 /* @F1*/
#define dynstg_fsit_inoexttbl       32                 /* @F1*/
#define dynstg_fsit_inotbl            33                 /* @F1*/
#define dynstg_ait_inotbl             34                 /* @F1*/

#endif /* ! _JFS_FSCKDIRE */


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  * The following are the possible values for current_fsck_phase
  * which is declared in xchkdsk.c
  *
  */
#define fsck_entry         16         /* x10 => entry linkage  */
#define fsck_phase_initial 17         /* x11 => initialization */
#define fsck_phase0         0
#define fsck_phase1         1
#define fsck_phase2         2
#define fsck_phase3         3
#define fsck_phase4         4
#define fsck_phase5         5
#define fsck_phase6         6
#define fsck_phase7         7
#define fsck_phase8         8
#define fsck_phase9         9
#define fsck_phase_final   32       /* x20 => cleanup        */
#define fsck_exit          33       /* x21 => exit linkage   */


/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 *
 * used during blockmap verify/rebuild
 *
 */
typedef struct fsck_bmap_record {
     char           eyecatcher[8];
     fsblkidx_t     total_blocks;
     fsblkidx_t     free_blocks;
     int8           ctl_fctl_error;
     int8           ctl_other_error;
     char           rsvd0[2];

     char           bmpctlinf_eyecatcher[8];
     dbmap_t       *bmpctl_bufptr;
     agg_byteidx_t  bmpctl_agg_fsblk_offset;
     char           rsvd1[4];

     char           AGinf_eyecatcher[8];
     fsblkidx_t    *AGFree_tbl;
     int8           AGActive[MAXAG];

     char           dmapinf_eyecatcher[8];
     reg_idx_t      dmappg_count;
     reg_idx_t      dmappg_ordno;
     reg_idx_t      dmappg_idx;
     int8          *dmap_wsp_stree;
     int8          *dmap_wsp_sleafs;
     dmap_t        *dmap_bufptr;
     agg_byteidx_t  dmap_agg_offset;
     fsblkidx_t     dmap_1stblk;
     int8           dmap_pmap_error;
     int8           dmap_slfv_error;
     int8           dmap_slnv_error;
     int8           dmap_other_error;
     char           rsvd2[4];

     char           L0inf_eyecatcher[8];
     reg_idx_t      L0pg_count;
     reg_idx_t      L0pg_ordno;
     reg_idx_t      L0pg_idx;
     int8          *L0_wsp_stree;
     int8          *L0_wsp_sleafs;
     dmapctl_t     *L0_bufptr;
     agg_byteidx_t  L0pg_agg_offset;
     fsblkidx_t     L0pg_1stblk;
     int8           L0_rsvd;
     int8           L0pg_slfv_error;
     int8           L0pg_slnv_error;
     int8           L0pg_other_error;
     char           rsvd3[4];

     char           L1inf_eyecatcher[8];
     reg_idx_t      L1pg_count;
     reg_idx_t      L1pg_ordno;
     reg_idx_t      L1pg_idx;
     int8          *L1_wsp_stree;
     int8          *L1_wsp_sleafs;
     dmapctl_t     *L1_bufptr;
     agg_byteidx_t  L1pg_agg_offset;
     fsblkidx_t     L1pg_1stblk;
     int8           L1_rsvd;
     int8           L1pg_slfv_error;
     int8           L1pg_slnv_error;
     int8           L1pg_other_error;
     char           rsvd4[4];

     char           L2inf_eyecatcher[8];
     reg_idx_t      L2pg_count;
     int8          *L2_wsp_stree;
     int8          *L2_wsp_sleafs;
     dmapctl_t     *L2_bufptr;
     agg_byteidx_t  L2pg_agg_offset;
     fsblkidx_t     L2pg_1stblk;
     int8           L2pg_slfv_error;
     int8           L2pg_slnv_error;
     int8           L2pg_other_error;
     char           rsvd5[5];
     }  fsck_bmap_record;


/*
 * a doubleword bitmap
 */
typedef uint32 fsck_dword_bitmap_t;
/*
 * a mask for referencing the bits in a doubleword
 */
typedef uint32 fsck_dword_bitmask_t;

typedef fsck_dword_bitmap_t  *fsck_bitmap_ptr;
typedef fsck_dword_bitmask_t *fsck_bitmask_ptr;



/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 *
 * unicharacter name structure
 */
struct uniname {
           uint32  len_in_UniChars;
           UniChar name_in_UniChars[256];
};

typedef struct uniname *UniCharname;


/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 *
 * used to pass message inserts from one routine to another economically
 */
struct fsck_ino_msg_info {
   uint32 msg_inonum;
   int    msg_inopfx;
   int    msg_inotyp; 
   int    msg_dxdtyp;
   };

typedef struct fsck_ino_msg_info *fsck_msg_info_ptr;

struct fsck_imap_msg_info {
   iagno_t msg_iagnum;
   int32   msg_agnum;
   int     msg_mapowner;
   };

typedef struct fsck_imap_msg_info *fsck_imap_msg_info_ptr;


/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 *
 * used to specify what action should be performed by routing
 * process_extent
 */
#define FSCK_RECORD                           1      
#define FSCK_RECORD_DUPCHECK           2       
#define FSCK_UNRECORD                        3    
#define FSCK_QUERY                            4        
#define FSCK_GET_OBJNAME                  5         
#define FSCK_FSIM_RECORD_DUPCHECK   6       
#define FSCK_FSIM_UNRECORD               7         
#define FSCK_FSIM_QUERY                    8      


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * The following are the values which may be displayed in
  * message fsck_INOINLINECONFLICT
  *
  */
#define fsck_longdata_and_otherinline  1


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * The following define return codes passed back by fsck when an
  * is detected in or by fsck.
  *
  */

/* informational return codes */

#define FSCK_OK                   00000
#define FSCK_FAILED              -00001

#define FSCK_AGFS_INOBAD          10001
#define FSCK_AGGAITINOBAD         10002
#define FSCK_AGGFSINOBAD          10003
#define FSCK_BADAGFSSIZ           10004
#define FSCK_BADBLKCTTTL          10005
#define FSCK_BADEADESCRIPTOR      10006
#define FSCK_BADMDDATA            10007
#define FSCK_BADMDDATAEXT         10008
#define FSCK_BADMDDATAIDX         10009
#define FSCK_BADMDDATAINLN        10010
#define FSCK_BADREAD_FBLKMP       10011
#define FSCK_BADREAD_FSCKLOG      10012
#define FSCK_BADSBAGSIZ           10013
#define FSCK_BADSBFJLA            10014
#define FSCK_BADSBFJLL            10015
#define FSCK_BADSBFSSIZ           10016
#define FSCK_BADSBFWSA            10017
#define FSCK_BADSBFWSL            10018
#define FSCK_BADSBFWSL1           10019
#define FSCK_BADSBMGC             10020
#define FSCK_BADSBVRSN            10021
#define FSCK_BADSBOTHR1           10022
#define FSCK_BADSBOTHR2           10023
#define FSCK_BADSBOTHR3           10024
#define FSCK_BADSBOTHR4           10025
#define FSCK_BADSBOTHR5           10026
#define FSCK_BADSBOTHR6           10027
#define FSCK_BADSBOTHR7           10028
#define FSCK_BADSBOTHR8           10029
#define FSCK_BADSBOTHR9           10030
#define FSCK_BADSBOTHR10          10031
#define FSCK_BADSBOTHR11          10032
#define FSCK_BADSBOTHR12          10033
#define FSCK_BADSBOTHR13          10034
#define FSCK_BADWRITE_FSCKLOG     10035
#define FSCK_BADWRITE_FBLKMP       10036
#define FSCK_BBINOBAD             10037
#define FSCK_BLSIZLTLVBLSIZ       10038
#define FSCK_BMINOBAD             10039
#define FSCK_CANT_ALLOC_INOREC    10040
#define FSCK_CANT_ALLOC_LSFN      10041
#define FSCK_CANT_EXTEND_ROOTDIR  10042
#define FSCK_CANTREADEAITEXT1     10043
#define FSCK_CANTREADAITEXT1      10044
#define FSCK_CANTREADAITEXT2      10045
#define FSCK_CANTREADAITEXT3      10046
#define FSCK_CANTREADAITEXT4      10047
#define FSCK_CANTREADAGGFSINO     10048
#define FSCK_CANTREADBBINO        10049
#define FSCK_CANTREADBMINO        10050
#define FSCK_CANTREADEA           10051
#define FSCK_CANTREADFSEXT        10052
#define FSCK_CANTREADFSRTDR       10053
#define FSCK_CANTREADLOGINO       10054
#define FSCK_CANTREADRECONDNODE   10055
#define FSCK_CANTREADRECONDNODE1  10056
#define FSCK_CANTREADSELFINO      10057
#define FSCK_CANTWRITRECONDNODE   10058
#define FSCK_CANTWRITRECONDNODE1  10059
#define FSCK_DUPMDBLKREF          10060
#define FSCK_FSETEXTBAD           10061
#define FSCK_FSRTDRBAD            10062
#define FSCK_IAGNOOOAGGBOUNDS     10063
#define FSCK_IAGNOOOFSETBOUNDS    10064
#define FSCK_INOEXTNOTALLOC       10065
#define FSCK_INOINLINECONFLICT1   10066
#define FSCK_INOINLINECONFLICT2   10067
#define FSCK_INOINLINECONFLICT3   10068
#define FSCK_INOINLINECONFLICT4   10069
#define FSCK_INOINLINECONFLICT5   10070
#define FSCK_LOGINOBAD            10071
#define FSCK_RIBADTREE            10072
#define FSCK_RIDATAERROR          10073
#define FSCK_RINOTDIR             10074
#define FSCK_RIUNALLOC            10075
#define FSCK_SELFINOBAD           10076
#define FSCK_INSUFDSTG4RECON      10077
#define FSCK_INSUFDSTG4RECON1     10078
#define FSCK_BLKSNOTAVAILABLE     10079
#define FSCK_BADREADTARGET        10080
#define FSCK_BADREADTARGET1       10081
#define FSCK_BADREADTARGET2       10082
#define FSCK_ENOMEMBDBLK1          10083
#define FSCK_ENOMEMBDBLK2          10084
#define FSCK_ENOMEMBDBLK3          10085
#define FSCK_ENOMEMBDBLK4          10086
#define FSCK_PARENTNULLIFIED       10087
#define FSCK_IOTARGETINJRNLLOG     10088

/* fatal condition return codes */

#define FSCK_FAILED_SEEK             -10001
#define FSCK_FAILED_BADSEEK          -10002

#define FSCK_FAILED_NODE_BADFLUSH    -10003
#define FSCK_FAILED_NODE_FLUSH       -10004
#define FSCK_FAILED_BADREAD_NODE     -10005
#define FSCK_FAILED_BADREAD_NODE1     -10006
#define FSCK_FAILED_READ_NODE        -10007
#define FSCK_FAILED_READ_NODE2       -10008
#define FSCK_FAILED_READ_NODE3       -10009
#define FSCK_FAILED_READ_NODE4       -10010

#define FSCK_FAILED_BADREAD_DNODE    -10011
#define FSCK_FAILED_READ_DNODE       -10012

#define FSCK_FAILED_INODE_BADFLUSH   -10013
#define FSCK_FAILED_INODE_FLUSH      -10014
#define FSCK_FAILED_BADREAD_INODE    -10015
#define FSCK_FAILED_BADREAD_INODE1   -10016
#define FSCK_FAILED_READ_INODE       -10017

#define FSCK_FAILED_IAG_BADFLUSH     -10018
#define FSCK_FAILED_IAG_FLUSH        -10019
#define FSCK_FAILED_BADREAD_IAG      -10020
#define FSCK_FAILED_BADREAD1_IAG     -10021
#define FSCK_FAILED_READ_IAG         -10022
#define FSCK_FAILED_IAG_CORRUPT_PXD  -10023

#define FSCK_FAILED_FBMAP_FLUSH      -10024
#define FSCK_FAILED_FBMAP_BADFLUSH   -10025
#define FSCK_FAILED_BADREAD_FBLKMP   -10026
#define FSCK_FAILED_READ_FBLKMP      -10027
#define FSCK_FAILED_WRITE_FBLKMP     -10028
#define FSCK_FAILED_BADWRITE_FBLKMP  -10029

#define FSCK_FAILED_PSBLK_WRITE      -10030
#define FSCK_FAILED_SSBLK_WRITE      -10031
#define FSCK_FAILED_BTHSBLK_WRITE    -10032
#define FSCK_FAILED_BTHSBLK_BAD      -10033

#define FSCK_FAILED_FSSIEXT_READ2    -10034
#define FSCK_FAILED_FSRTDIR_READ2    -10035
#define FSCK_FAILED_BADBLK_READ2     -10036
#define FSCK_FAILED_BMAP_READ2       -10037
#define FSCK_FAILED_LOG_READ2        -10038
#define FSCK_FAILED_SELF_READ2       -10039
#define FSCK_FAILED_SELF_READ3       -10040
#define FSCK_FAILED_SELF_READ4       -10041
#define FSCK_FAILED_SELF_READ5       -10042
#define FSCK_FAILED_SELF_NOWBAD      -10043
#define FSCK_FAILED_AGFS_READ2       -10044
#define FSCK_FAILED_AGFS_READ3       -10045
#define FSCK_FAILED_AGFS_READ4       -10046
#define FSCK_FAILED_AGFS_READ5       -10047
#define FSCK_FAILED_AGFS_NOWBAD      -10048

#define FSCK_FAILED_BOTHAITBAD       -10049
#define FSCK_FAILED_CANTREADAITEXT1  -10050
#define FSCK_FAILED_CANTREADAITEXT2  -10051
#define FSCK_FAILED_CANTREADAITEXT3  -10052
#define FSCK_FAILED_CANTREADAITEXT4  -10053
#define FSCK_FAILED_CANTREADAITEXT5  -10054
#define FSCK_FAILED_CANTREADAITEXT6  -10055
#define FSCK_FAILED_CANTREADAITEXT7  -10056
#define FSCK_FAILED_CANTREADAITEXT8  -10057
#define FSCK_FAILED_CANTREADAITEXT9  -10058
#define FSCK_FAILED_CANTREADAITEXTA  -10059
#define FSCK_FAILED_CANTREADAITEXTB  -10060
#define FSCK_FAILED_CANTREADAITEXTC  -10061
#define FSCK_FAILED_CANTREADAITEXTD  -10062
#define FSCK_FAILED_CANTREADAITEXTE  -10063
#define FSCK_FAILED_CANTREADAITEXTF  -10064
#define FSCK_FAILED_CANTREADAITEXTG  -10065
#define FSCK_FAILED_CANTREADAITEXTH  -10066
#define FSCK_FAILED_CANTREADAITEXTJ  -10067
#define FSCK_FAILED_CANTREADAITEXTK  -10068
#define FSCK_FAILED_CANTREADAITCTL   -10069
#define FSCK_FAILED_CANTREADAITS     -10070
#define FSCK_FAILED_CANTREADAIMNOW   -10071

#define FSCK_FAILED_IMPLF_BADFLUSH   -10072
#define FSCK_FAILED_IMPLF_FLUSH      -10073
#define FSCK_FAILED_BADREAD_IMPLF    -10074
#define FSCK_FAILED_READ_IMPLF       -10075

#define FSCK_FAILED_CANTREAD_DIRNOW  -10076
#define FSCK_FAILED_DIRGONEBAD       -10077
#define FSCK_FAILED_DIRGONEBAD2      -10078
#define FSCK_FAILED_DIRENTRYGONE     -10079
#define FSCK_FAILED_DIRENTRYBAD      -10080

#define FSCK_FAILED_MAPCTL_BADFLUSH  -10081
#define FSCK_FAILED_MAPCTL_FLUSH     -10082
#define FSCK_FAILED_BADREAD_MAPCTL   -10083
#define FSCK_FAILED_READ_MAPCTL      -10084

#define FSCK_FAILED_CANTREADBMPCTL   -10085

#define FSCK_FAILED_BMPLV_BADFLUSH   -10086
#define FSCK_FAILED_BMPLV_FLUSH      -10087
#define FSCK_FAILED_BADREAD_BMPLV    -10088
#define FSCK_FAILED_READ_BMPLV       -10089

#define FSCK_FAILED_BMPDM_BADFLUSH   -10090
#define FSCK_FAILED_BMPDM_FLUSH      -10091
#define FSCK_FAILED_BADREAD_BMPDM    -10092
#define FSCK_FAILED_READ_BMPDM       -10093
#define FSCK_FAILED_DYNSTG_EXHAUST1  -10094
#define FSCK_FAILED_DYNSTG_EXHAUST2  -10095
#define FSCK_FAILED_DYNSTG_EXHAUST3  -10096
#define FSCK_FAILED_DYNSTG_EXHAUST4  -10097
#define FSCK_FAILED_DYNSTG_EXHAUST5  -10098
#define FSCK_FAILED_DYNSTG_EXHAUST6  -10099
#define FSCK_FAILED_DYNSTG_EXHAUST7  -10100
#define FSCK_FAILED_DYNSTG_EXHAUST8  -10101
#define FSCK_FAILED_DYNSTG_EXHAUST9  -10102
#define FSCK_FAILED_DYNSTG_EXHAUSTA  -10103

#define FSCK_FAILED_REREAD_AGGINO    -10104

/* catastrophic error return codes */

#define FSCK_INTERNAL_ERROR_1   -11001
#define FSCK_INTERNAL_ERROR_2   -11002
#define FSCK_INTERNAL_ERROR_3   -11003
#define FSCK_INTERNAL_ERROR_4   -11004
#define FSCK_INTERNAL_ERROR_5   -11005
#define FSCK_INTERNAL_ERROR_6   -11006
#define FSCK_INTERNAL_ERROR_7   -11007
#define FSCK_INTERNAL_ERROR_8   -11008
#define FSCK_INTERNAL_ERROR_9   -11009
#define FSCK_INTERNAL_ERROR_10  -11010
#define FSCK_INTERNAL_ERROR_11  -11011
#define FSCK_INTERNAL_ERROR_12  -11012
#define FSCK_INTERNAL_ERROR_13  -11013
#define FSCK_INTERNAL_ERROR_14  -11014
#define FSCK_INTERNAL_ERROR_15  -11015
#define FSCK_INTERNAL_ERROR_16  -11016
#define FSCK_INTERNAL_ERROR_17  -11017
#define FSCK_INTERNAL_ERROR_18  -11018
#define FSCK_INTERNAL_ERROR_19  -11019
#define FSCK_INTERNAL_ERROR_20  -11020
#define FSCK_INTERNAL_ERROR_21  -11021
#define FSCK_INTERNAL_ERROR_22  -11022
#define FSCK_INTERNAL_ERROR_23  -11023
#define FSCK_INTERNAL_ERROR_24  -11024
#define FSCK_INTERNAL_ERROR_25  -11025
#define FSCK_INTERNAL_ERROR_26  -11026
#define FSCK_INTERNAL_ERROR_27  -11027
#define FSCK_INTERNAL_ERROR_28  -11028
#define FSCK_INTERNAL_ERROR_29  -11029
#define FSCK_INTERNAL_ERROR_30  -11030
#define FSCK_INTERNAL_ERROR_31  -11031
#define FSCK_INTERNAL_ERROR_32  -11032
#define FSCK_INTERNAL_ERROR_33  -11033
#define FSCK_INTERNAL_ERROR_34  -11034
#define FSCK_INTERNAL_ERROR_35  -11035
#define FSCK_INTERNAL_ERROR_36  -11036
#define FSCK_INTERNAL_ERROR_37  -11037
#define FSCK_INTERNAL_ERROR_38  -11038
#define FSCK_INTERNAL_ERROR_39  -11039
#define FSCK_INTERNAL_ERROR_40  -11040
#define FSCK_INTERNAL_ERROR_41  -11041
#define FSCK_INTERNAL_ERROR_42  -11042
#define FSCK_INTERNAL_ERROR_43  -11043
#define FSCK_INTERNAL_ERROR_44  -11044
#define FSCK_INTERNAL_ERROR_45  -11045
#define FSCK_INTERNAL_ERROR_46  -11046
#define FSCK_INTERNAL_ERROR_47  -11047
#define FSCK_INTERNAL_ERROR_48  -11048
#define FSCK_INTERNAL_ERROR_49  -11049
#define FSCK_INTERNAL_ERROR_50  -11050
#define FSCK_INTERNAL_ERROR_51  -11051
#define FSCK_INTERNAL_ERROR_52  -11052
#define FSCK_INTERNAL_ERROR_53  -11053
#define FSCK_INTERNAL_ERROR_54  -11054
#define FSCK_INTERNAL_ERROR_55  -11055
#define FSCK_INTERNAL_ERROR_56  -11056
#define FSCK_INTERNAL_ERROR_57  -11057
#define FSCK_INTERNAL_ERROR_58  -11058
#define FSCK_INTERNAL_ERROR_59  -11059
#define FSCK_INTERNAL_ERROR_60  -11060
#define FSCK_INTERNAL_ERROR_61  -11061
#define FSCK_INTERNAL_ERROR_62  -11062
#define FSCK_INTERNAL_ERROR_63  -11063
#define FSCK_INTERNAL_ERROR_64  -11064
#define FSCK_INTERNAL_ERROR_65  -11065
#define FSCK_INTERNAL_ERROR_66  -11066
#define FSCK_INTERNAL_ERROR_67  -11067
#define FSCK_INTERNAL_ERROR_68  -11068
#define FSCK_INTERNAL_ERROR_69  -11069
#define FSCK_INTERNAL_ERROR_70  -11070                /* @F1*/
#define FSCK_INTERNAL_ERROR_71  -11071                /* @F1*/
#define FSCK_INTERNAL_ERROR_72  -11072                /* @F1*/
#define FSCK_INTERNAL_ERROR_73  -11073                /* @F1*/
#define FSCK_INTERNAL_ERROR_74  -11074                /* @F1*/
#define FSCK_INTERNAL_ERROR_75  -11075                /* @F1*/
#define FSCK_INTERNAL_ERROR_76  -11076                /* @F1*/
#define FSCK_INTERNAL_ERROR_77  -11077                /* @F1*/
#define FSCK_INTERNAL_ERROR_78  -11078                /* @F1*/
#define FSCK_INTERNAL_ERROR_79  -11079                /* @F1*/
#define FSCK_INTERNAL_ERROR_80  -11080                /* @F1*/
#define FSCK_INTERNAL_ERROR_81  -11081                /* @F1*/
#define FSCK_INTERNAL_ERROR_82  -11082                /* @F1*/
#define FSCK_INTERNAL_ERROR_83  -11083                /* @F1*/
#define FSCK_INTERNAL_ERROR_84  -11084                /* @F1*/
#define FSCK_INTERNAL_ERROR_85  -11085                /* @F1*/
#define FSCK_INTERNAL_ERROR_86  -11086                /* @F1*/
#define FSCK_INTERNAL_ERROR_87  -11087                /* @F1*/
#define FSCK_INTERNAL_ERROR_88  -11088                /* @F1*/
#define FSCK_INTERNAL_ERROR_89  -11089                /* @F1*/

#endif

