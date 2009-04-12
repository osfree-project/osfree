/* $Id: message.h,v 1.2 2004/03/21 02:43:19 pasha Exp $ */

/* static char *SCCSID = "@(#)1.6  9/23/98 14:41:05 src/jfs/utils/libfs/message.h, jfslib, w45.fs32, 990417.1";*/
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
 *   MODULE_NAME:               message.h  - message header for JFS Utilities
 *
 *   COMPONENT_NAME:    jfsformat
 *
 *   FUNCTIONS:         none
 *
 */

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

/*
 *  message_user (mesg_no, param, param_cnt, device, response, msg_file)
 *
 *  ENTRY    mesg_no   - index into the array of messages
 *           param     - list of pointers to parameters in the message
 *           param_cnt - count of parameters in the param variable
 *           device    - where to write to (STDOUT_CODE, STDERR_CODE)
 *           response  - what kind of response is needed from user
 *           msg_file  - OSO_MSG/JFS_MSG which message file to use
 *
 */
extern char *message_user(unsigned,char **,unsigned, unsigned, unsigned, int);

#define OSO_MSG 0
#define JFS_MSG 1

#define STDOUT_CODE       0
#define STDERR_CODE       1

/*
 * Magic numbers for interfacing with the computmsg() function.
 * The first set of numbers are codes which determine what kind of response
 * to a message is desired.  The second set defines special return codes
 * for the "yes/no" and "abort/retry/ignore" options.  The third set are
 * codes for which handle to write messages to.  Note that they are not
 * file handle numbers.
 */

#define NO_RESPONSE             0       /* no response is necessary */
#define CR_RESPONSE             1       /* press Enter key, input is eaten */
#define YES_NO_RESPONSE         2       /* "y" or "n" only -- value returned */
#define STRING_RESPONSE         3       /*  string value returned */
#define ANY_KEY_RESPONSE        4       /* press Any key, input is eaten */
#define ARI_RESPONSE            5       /* Abort, Retry, Ignore */

#define YES_ANS         0
#define NO_ANS          1


/* The following section is used to define message numbers used by functions */


/* messages used by Format */

#define MSG_JFS_PART_SMALL      1        /* from JFS MSG file */
#define MSG_JFS_LOG_LARGE       2        /* from JFS MSG file */
#define MSG_JFS_LT16MEG_ADAPTER    3
#define MSG_JFS_VOL_NOT_LVM       4        /* from JFS MSG file */
#define MSG_JFS_NOT_JFS            450
#define MSG_JFS_BAD_SUPERBLOCK     451
#define MSG_JFS_DIRTY              452
#define MSG_JFS_EXTENDFS_COMPLETE  453
#define MSG_JFS_EXTENDFS_USAGE     454
#define MSG_JFS_EXTENDFS_FAILED    455
#define MSG_JFS_DEFRAGMENTING      456
#define MSG_JFS_DEFRAGFS_COMPLETE  457
#define MSG_JFS_NUM_DEFRAGED       458
#define MSG_JFS_SKIPPED_FREE       459
#define MSG_JFS_SKIPPED_FULL       460
#define MSG_JFS_SKIPPED_CONTIG     461
#define MSG_JFS_TOTAL_AGS          462
#define MSG_JFS_DEFRAGFS_USAGE     463
#define MSG_JFS_NUM_CANDIDATE      464
#define MSG_JFS_AVG_FREE_RUNS      465
#define MSG_JFS_UNEXPECTED_ERROR   466

#define MSG_OSO_NOT_ENOUGH_MEMORY    8
#define MSG_OSO_CANT_FIND_DRIVE     15
#define MSG_OSO_CANT_OPEN          110
#define MSG_OSO_ERR_ACCESSING_DISK 868
#define MSG_OSO_VALUE_REQUIRED     871
#define MSG_OSO_INVALID_PARAMETER  629
#define MSG_OSO_DISK_LOCKED        566
#define MSG_OSO_VALUE_NOT_ALLOWED  873
#define MSG_OSO_FORMAT_FAILED      528
#define MSG_OSO_INSUFF_MEMORY   529
#define MSG_OSO_CURR_LABEL         539
#define MSG_OSO_BAD_LABEL          636
#define MSG_OSO_DESTROY_DATA      1271
#define MSG_OSO_NEW_LABEL         1288
#define MSG_OSO_FORMAT_COMPLETE    635
#define MSG_OSO_DISK_SPACE         535
#define MSG_OSO_DISK_SPACE2        568         //198316 pmformat fix
#define MSG_OSO_FREE_SPACE         537
#define MSG_OSO_PERCENT_FORMAT     538
#define MSG_OSO_READ_ERROR        1566

#endif
