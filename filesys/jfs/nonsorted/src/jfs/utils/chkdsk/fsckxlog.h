/* $Id: fsckxlog.h,v 1.2 2004/03/21 02:43:28 pasha Exp $ */

/* static char *SCCSID = "@(#)1.1  7/8/97 11:06:47 src/jfs/utils/chkdsk/fsckxlog.h, jfschk, w45.fs32, 990417.1";*/
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
#ifndef H_FSCKXLOG
#define	H_FSCKXLOG


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
  short  entry_length;
  short  msg_num;
  short  num_parms;
  }  chklog_entry_hdr_t;

typedef struct chklog_entry_hdr *chklogent_hdptr;

typedef short *chklogent_prmlen; /* array of parm lengths */

typedef char  *chklogent_prmstr_ptr; /* to step through the
                              * string literal parms (if any)
                              */

#endif
