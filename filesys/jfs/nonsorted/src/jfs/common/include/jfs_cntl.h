/* $Id: jfs_cntl.h,v 1.1 2000/04/21 10:57:56 ktk Exp $ */

/* static char *SCCSID = "@(#)1.19  11/1/99 11:35:31 src/jfs/common/include/jfs_cntl.h, sysjfs, w45.fs32, fixbld";
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
 * MODULE_NAME:     jfs_cntl.h
 *
 * COMPONENT_NAME:  sysjfs
 *
 *  common data & function prototypes
 *
*/

/*
 * Change History :
 *
 */

#ifndef _H_JFS_CNTL
#define _H_JFS_CNTL

/*
 *      command
 */
#ifdef  _JFS_OS2
#define JFSCTL_LW_GET       99  /* Get lazywrite parameters */
#define JFSCTL_LW_SET       98  /* Set lazywrite parameters */
#define JFSCTL_SYNC     97  /* sync each jfs vfs */
#define JFSCTL_DEFRAGFS     96  /* defragfs() */
#define JFSCTL_EXTENDFS     95  /* extendfs() */
#define JFSCTL_CLRBBLKS     94  /* clrbblks() */
#define JFSCTL_CHKDSK       93  /* chkdsk() */
#define JFSCTL_RESUME       92  /* called by LVM during extend */
#define JFSCTL_QUIESCE      91  /* called by LVM during extend */
#define JFSCTL_LAZYCOMMIT   90  /* Lazy Commit Thread   D230860 */
#define JFSCTL_CACHESTATS   89  /* Get cache statistics   D230860 */


/*
 * Cache stats
 * KSO: these were not defined.
 */
#define FSCNTL_NCACHE        1
#define FSCNTL_ICACHE        2
#define FSCNTL_JCACHE        3
#define FSCNTL_DCACHE        4
#define FSCNTL_LCACHE        5



// BEGIN F226941

/*
 * DASD limits operations.
 * Also defined in LAN server private header, lsfsctl.h
 */
#define PBDASD_ADD  0x8035  /* Add a DASD record        */
#define PBDASD_DEL  0x8036  /* Delete a DASD record     */
#define PBDASD_GETINFO  0x8037  /* Retrieve a DASD record   */
#define PBDASD_SETINFO  0x8038  /* Modify a DASD record     */
#define PBDASD_ENUM 0x8039  /* Enumerate DASD records   */
#define PBDASD_CHECK    0x803A  /* Retrieve DASD avail space    */
#define PBDASD_CTLVOL   0x803B  /* Prime volume for DASD    */
#define PBDASD_INIT 0x803C  /* Init alerting limits     */

// END F226941

/* Defined in jfs_acl.h
#define JFS_GETACC0 0x8811  * retrieve ACL type and size *
#define JFS_GETACC1 0x8812  * retrieve all ACL level 1 info *
#define JFS_SETAUD  0x8813  * set first word of ACL (acc1_attr) *
#define JFS_SETACC  0x8814  * set all ACL level 1 info *
#define JFS_ADDACC  0x8815  * add an ACL *
#define JFS_DELACC  0x8816  * delete an ACL *
#define JFS_TESTACC 0x8817  * test if ACL interface exists *
#define JFS_FFACC   0x8818  * enum findfirst ACL *
#define JFS_FNACC   0x8819  * enum findnext ACL *
#define JFS_FCACC   0x881A  * enum findclose ACL *
#define JFS_APPACC  0x881B  * append ACL (currently unsupported) */

// BEGIN F226941

/* Defined in jfs_dlim.h
#define JFS_DLIM_INIT   0x8820         * Initialization info for DASD Limits  *
#define JFS_DLIM_STOP   0x8821         * Server is stopping; stop DASD Limits *
#define JFS_DLIM_ALERT  0x8822         * Captive thread for sending alerts    *
#define JFS_DLIM_FF     0x8823         * Find first                           *
#define JFS_DLIM_FN     0x8824         * Find next                            *
#define JFS_DLIM_FC     0x8825         * Find close                           *
#define JFS_VOLENUM     0x8830         * Enumerate volumes handled by JFS     */
// END F226941

/* LazyWrite parameters */

struct LWPARMS  {
    int32   LazyOff;    /* TRUE if lazywrite disabled       */
    int32   SyncTime;   /* Sleep interval for sync thread   */
    int32   MaxAge;     /* Maximum age of dirty buffer      */
    int32   BufferIdle; /* Time to wait after change before writing */
    uint32  CacheSize;  /* Size of buffer cache         */
    int32   MinFree;    /* Minimum number of free cbufs  D230860 */
    int32   MaxFree;    /* Maximum number of free cbufs  D230860 */
};

#define OLD_LWPARMS_LEN 20                  // D230860

#endif  /* _JFS_OS2 */

#endif /* _H_JFS_CNTL */
