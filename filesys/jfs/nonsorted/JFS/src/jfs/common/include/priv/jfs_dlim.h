/* $Id: jfs_dlim.h,v 1.1.1.1 2003/05/21 13:36:06 pasha Exp $ */

/************************************************************************
* SCCSID: src/public/include/os2c/priv/jfs_dlim.h, sysjfs, c.pc, fixbld 99/07/28
*
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
* Header File Name: jfs_dlim.h
*
* Description:  This file contains the structures and definitions for the
*               JFS FSCTL interface used to support the storage and retrieval
*               of Directory Limits in the Journaled File System (JFS).
*
* Dependencies: This file is currently kept in the OS2 family and exported
*               to LAN which also depends on it.
*
* Revisions:
*
* Defect   Date        Initials   Description
* ======   =========== ========   ========================================
* 226941   28-Jul-1999 DJK        Initial Create
************************************************************************/

#ifndef JFS_DLIM_H_INCLUDED

#define JFS_DLIM_H_INCLUDED 1


/* If visual age compiler, use _Seg16 qualifier, otherwise use far keyword */

#ifdef __IBMC__
  #define FAR16_PTR * _Seg16
#else
  #define FAR16_PTR far *
#endif

#ifndef CCHMAXPATHCOMP
  #define CCHMAXPATHCOMP 256
#endif

#ifndef CCHMAXPATH
  #define CCHMAXPATH 260
#endif

#ifndef MAX_DRIVES
  #define MAX_DRIVES 26
#endif

#ifndef JFS_FSDNAME
  #define JFS_FSDNAME "JFS"
#endif


/*
*  JFS FSCtl DASD Limit function codes
*/

#define JFS_DLIM_INIT   0x8820         /* Initialization info for DASD Limits  */
#define JFS_DLIM_STOP   0x8821         /* Server is stopping; stop DASD Limits */
#define JFS_DLIM_ALERT  0x8822         /* Captive thread for sending alerts    */
#define JFS_DLIM_FF     0x8823         /* Find first                           */
#define JFS_DLIM_FN     0x8824         /* Find next                            */
#define JFS_DLIM_FC     0x8825         /* Find close                           */

/*
*  JFS FSCtl Miscellaneous function codes
*/

#define JFS_VOLENUM     0x8830         /* Enumerate volumes handled by JFS     */

/*
*  JFS FSCtl DASD Limits initialization parameter packet
*/

struct jfs_alert_delay_info {
   unsigned long ulDiskFullDelay;      /* delay for disk full alerts */
   unsigned long ulThreshDelay;        /* delay for threshold alerts */
};

struct jfs_dlim_init_info {
   char FAR16_PTR pSessionFlagsPtr;    /* 16:16 pointer to session flags table */
   unsigned long ulSessionFlagsLen;    /* length of the session flags table    */
   struct jfs_alert_delay_info aDelayInfo[MAX_DRIVES];
                                       /* alert delay info for each drive      */
};

#define R3SRV_SESFLG_DLIM  0x01        /* Enforce DASD limits on the session   */

/*
*  JFS FSCtl DASD Limits enumeration parameters
*/

struct jfs_DLIM_parms {
   unsigned long ulReturnCode;         /* return code for operation        */
   unsigned long ulLevel;              /* info level                       */
   unsigned long ulFlags;              /* flags (input/output)             */
   unsigned long cchData;              /* count of bytes returnable        */
   unsigned long hEnum;                /* handle for use by enum functions */
   unsigned long ulReserved;           /* reserved                         */
   char          szPath[CCHMAXPATH];   /* path name                        */
};


/*
*  JFS FSCtl DASD Limits enumeration data
*
*  The enumeration data returned from JFS starts with the following header.
*  The header is followed by a dasd_info_0 structure (from
*  src\lanman\common\include\dasd.h in the LAN source tree.)
*/

struct jfs_DLIM_enum_header {
   unsigned char cchName;                 /* length of name */
   char          achName[CCHMAXPATHCOMP]; /* directory name */
};


/*
*  JFS FSCtl parameter packet for the captive alert thread
*
*  For now the Server doesn't have to pass anything to JFS.  We pass a level in
*  case future versions will require a different parameter format for more
*  information.  Level 0 has no other parameters.
*/

struct JFS_alert_request {
   unsigned long ulLevel;              /* level of information requested */
};


/*
*  JFS FSCtl data packet for the captive alert thread
*/

struct JFS_alert_info {
   unsigned long  ulLevel;             /* level of information returned          */
   unsigned short usType;              /* type of alert (disk full or threshold) */
   unsigned short usSession;           /* session number (from TCB)              */
   unsigned long  ulFreespace;         /* free space (threshold alert only)      */
   char           szDirname[CCHMAXPATH];  /* directory name                      */
};

#define JFS_THRESH_ALERT	7	/* SrvThreshAlert	*/
#define JFS_DIRFULL_ALERT	8	/* SrvDirFullAlert	*/

/*
*  JFS FSCtl parameter packet for requesting an enueration of the volumes
*  currently being handled by JFS.
*
*  For now the caller doesn't have to pass anything to JFS.  We pass a level in
*  case future versions will require a different parameter format for more
*  information.
*
*  For level 0, JFS will return in the data buffer an array of characters which
*  are the drive letters of the volumes currently being managed by JFS.  For
*  example:  "dfgjlmnop"
*/

struct JFS_volenum_request {
   unsigned long ulLevel;              /* level of information requested */
};

#endif
