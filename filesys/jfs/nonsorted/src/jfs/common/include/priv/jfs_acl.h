/* $Id: jfs_acl.h,v 1.1 2000/04/21 10:58:02 ktk Exp $ */

/************************************************************************
* SCCSID: src/public/include/os2c/priv/jfs_acl.h, sysjfs, c.pc, 991103.1 99/02/22
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
* Header File Name: jfs_acl.h
*
* Description:  This file contains the structures and definitions for the
*               JFS FSCTL interface used to support the storage and retrieval
*               of ACLs in the Journaled File System (JFS).
*
* Dependencies: This file is currently kept in the OS2 family and exported
*               to LAN which also depends on it.
*
* Revisions:
*
* Defect   Date        Initials   Description
* ======   =========== ========   ========================================
* 208695   20-Oct-1998 shaggy     Initial Create
************************************************************************/

#ifndef JFS_ACL_H_INCLUDED

#define JFS_ACL_H_INCLUDED 1


/* If visual age compiler use _Seg16 qualifier otherwise use far keyword */

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

/*
*  main JFS access header for all ACLs
*
*  Initial object creation ACLs and initial container creation ACLs are only
*  valid if object is a directory. The offsets to the different ACLs are
*  relative from the start of this header.
*
*  A value of -1 for an initial creation ACL denotes that the ACL does exist
*  but is the same as the object ACL. A value of 0 (zero) for an initial
*  creation ACL indicates none exists (should always be true for a file).
*  If not 0 or -1 then it is the offset to an initial creation ACL different
*  from the object ACL.
*/

struct jfs_access_header {
  unsigned long ulFlags;                   /* ACL flags */
  unsigned long offulObjectACL;            /* offset to object ACL */
  unsigned long offulIObjectCreationACL;   /* offset to initial object creation ACL */
  unsigned long offulIContainerCreationACL;/* offset to initial container ""     "" */
};

typedef struct jfs_access_header FAR16_PTR PJFSACCHDR_FAR;

/* possible values for offulIObjectCreationACL and offulIContainerCreationACL */

#define JFS_DUP_OBJECTACL   0xFFFFFFFF     /* duplicate of object ACL */
#define JFS_NO_ACL          0x00000000     /* no ACL exists */

/*
*  JFS access control list header
*
*  Every ACL that physically exists is preceded by the JFS ACL header. It
*  currently tracks the size of the ACL blob stored by JFS.
*/

struct jfs_access_control_list_header {
  unsigned long cchEntries;                /* total length of ACL and ACEs */
};

typedef struct jfs_access_control_list_header FAR16_PTR PJFSACLHDR_FAR;

/*
*  JFS enum header
*
*  The following header is used for records retrieved with the access
*  enumeration functions JFS_FFACC (FindFirst) and JFS_FNACC (FindNext).
*
*  At this time the parm packet ulFlags returned are used to indicate whether
*  a directory pathname was returned with no matching target ACL type in order
*  to be able to still traverse down into it for recursive searches. Files
*  without the matching target ACL type are not returned by the enum functions.
*
*/

struct jfs_access_enum_header {
   unsigned char cchName;                  /* length of name */
   char          achName[CCHMAXPATHCOMP];  /* file/dir name */
};

typedef struct jfs_access_enum_header FAR16_PTR PJFSENUMHDR_FAR;

/*
*  JFS FSCTL parameter packet
*
*  Below is the generic parameter packet supplied to the JFS access functions.
*  It contains the return code for the operation if the DosFSCtl() does reach
*  the function. The flags can be input or output or both. The cchData is
*  output for get operations that identify the size required to retrieve the
*  ACL data if ERROR_MORE_DATA is returned as the return code.
*
*/

struct jfs_access_parms {
  unsigned long ulReturnCode;              /* return code for operation */
  unsigned long ulFlags;                   /* flags (input/output) - see below */
  unsigned long cchData;                   /* count of bytes returnable (for gets) */
  unsigned long hEnum;                     /* handle for use by enum functions */
  unsigned long ulReserved;                /* reserved */
  char          achPath[CCHMAXPATH];       /* pathname */
};

/* four bits in flag to indicate target ACL to work on (input) */
#define JFS_ACCESS_FLAGS_TARGETACL_OBJ  0x00000000  /* target is ObjectACL */
#define JFS_ACCESS_FLAGS_TARGETACL_IC   0x00000400  /* target is IContainerCreationACL */
#define JFS_ACCESS_FLAGS_TARGETACL_IO   0x00000800  /* target is IObjectCreationACL */
#define JFS_ACCESS_FLAGS_TARGETACL_ALL  0x00000C00  /* target is all three */

/* mask for getting target ACL bits */
#define JFS_ACCESS_FLAGS_TARGETACL_MASK 0x00000F00

/* eight bits to identify ACL type stored (input/output) */
#define JFS_ACCESS_FLAGS_ACLTYPE_LAN    0x00000001  /* LAN Server ACL type */
#define JFS_ACCESS_FLAGS_ACLTYPE_DCE    0x00000002  /* DSS/DCE ACL type (unused) */

/* mask for getting ACL type bits */
#define JFS_ACCESS_FLAGS_ACLTYPE_MASK   0x000000FF

/* mask for determing if info returned is for a directory (output) */
#define JFS_ACCESS_FLAGS_DIRECTORY      0x80000000

/* Bit defining scope of search (input) */
#define JFS_ACCESS_FLAGS_INHERIT	0x08000000 /* Return parent's ACL */

/* special value for hEnum */
#define JFS_ACCESS_HANDLE_UNALLOCATED   0xFFFFFFFF

/* JFS FS_FSCTL access control function codes */
/* We chose obscure codes because the LAN server may not know what type of */
/* filesystem to which it sends the FSCTL, and we don't want another       */
/* filesystem to interpret the FSCTL as its own.                           */

#define JFS_GETACC0 0x8811  /* retrieve ACL type and size */
#define JFS_GETACC1 0x8812  /* retrieve all ACL level 1 info */
#define JFS_SETAUD  0x8813  /* set audit field in ACL (acc1_attr) (currently unsupported) */
#define JFS_SETACC  0x8814  /* set all ACL level 1 info */
#define JFS_ADDACC  0x8815  /* add an ACL */
#define JFS_DELACC  0x8816  /* delete an ACL */
#define JFS_TESTACC 0x8817  /* test if ACL interface exists (currently unsupported) */
#define JFS_FFACC   0x8818  /* enum findfirst ACL */
#define JFS_FNACC   0x8819  /* enum findnext ACL */
#define JFS_FCACC   0x881A  /* enum findclose ACL */
#define JFS_APPACC  0x881B  /* append ACL (currently unsupported) */

/* JFS FSD name string */
#define JFS_FSDNAME "JFS"

/* macros for accessing pre-recorded portions of an ACL */

#define JFSACLHDR_PTR(pjfs_acc_hdr,offACL) \
        ((PJFSACLHDR_FAR)(((char FAR16_PTR)pjfs_acc_hdr) + ((PJFSACCHDR_FAR)pjfs_acc_hdr)->offACL))

#define JFSLANACL_PTR(pjfs_acc_hdr,offACL) \
        ((struct access_info_1 FAR16_PTR)(JFSACLHDR_PTR(pjfs_acc_hdr,offACL)+1))

#define MAXJFSACL (sizeof(struct jfs_access_header) +             \
                  sizeof(struct jfs_access_control_list_header) + \
                  sizeof(struct access_info_1) +                  \
                  (sizeof(struct access_list) * MAXPERMENTRIES))

#endif
