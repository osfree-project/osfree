/* $Id: fscklog.h,v 1.1.1.1 2003/05/21 13:39:36 pasha Exp $ */

/* static char *SCCSID = "@(#)1.4  7/8/97 11:07:24 src/jfs/utils/chkdsk/fscklog.h, jfschk, w45.fs32, 990417.1";*/
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
 *   COMPONENT_NAME: jfschk
 *
 *   FUNCTIONS: none
 *
 */
#ifndef H_FSCKLOG
#define	H_FSCKLOG


/*----------------------------------------------------------------------------
 *
 * The in-aggregate fsck service log 
 */

#define flog_eyecatcher_string "fscklog "

typedef struct fscklog_entry_hdr {
  int16  entry_length;
  int16  msg_num;
  int16  num_parms;
  }  fscklog_entry_hdr_t;

typedef struct fscklog_entry_hdr *logent_hdptr;

typedef int8  *logent_prmtype; /* array of parm types:
                                *  1 => text insert idx
                                *  2 => string literal
                                */
typedef int16 *logent_prmval;  /* array of parm values 
                                *  if prmtype[i] == 1 
                                *  then prmval[i] = index of
                                *                   text insert
                                *  if prmtype[i] == 2
                                *  then prmval[i] = length of 
                                *                   string literal 
                                */
typedef char  *logent_prmstr_ptr; /* will be used to step through
                                * the string literal parms (if any)
                                */

typedef struct fscklog_error {
   int64   err_offset;       /*  8 -- start of attempted write */
   int32   bytes_written;    /*  4 -- number actually written  */
   int32   io_retcode;       /*  4 -- return code from write   */
   } fscklog_error_t;        /*    total: 16 bytes */

/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 *
 *  The first log message number for the JFS/Warp chkdsk Service Log
 *
 *
 */

#define JFSCHKLOG_FIRSTMSGNUM 10000


/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 *
 *  The format of the log entry in the file created by
 *  extracting the contents of the chkdsk service log
 *  (stored in the fsck workspace) into a file.
 *
 *
 * N.B. The log entries are blocked such that no log entry crosses
 *      an XCHKLOG_BUFSIZE byte boundary in the file.
 */

#define XCHKLOG_BUFSIZE  8192

#define jfs_chklog_eyecatcher "JFS chkdskSvcLog"

typedef struct chklog_entry_hdr {
  int16  entry_length;
  int16  msg_num;
  int16  num_parms;
  }  chklog_entry_hdr_t;

typedef struct chklog_entry_hdr *chklogent_hdptr;

typedef int16 *chklogent_prmlen; /* array of parm lengths */

typedef char  *chklogent_prmstr_ptr; /* to step through the
                              * string literal parms (if any)
                              */

#endif
