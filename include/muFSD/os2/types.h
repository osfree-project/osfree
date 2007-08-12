//
// $Header: /cur/cvsroot/boot/include/muFSD/os2/types.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
//

// 32 bits OS/2 device driver and IFS support. Provides 32 bits kernel
// services (DevHelp) and utility functions to 32 bits OS/2 ring 0 code
// (device drivers and installable file system drivers).
// Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



#ifndef __os2_types_h
#define __os2_types_h

#if defined (__IBMC__)
#define INLINE       _Inline

#define DH32ENTRY    _System                            /* DevHlp32_* entry points     */
#define DH32ENTRY2   _Optlink                           /* DevHlp32_* entry points     */
#define FS32ENTRY    _Far32 _Pascal                     /* fs32_* entry points         */
#define FS32INIT     _System                            /* fsh32_* entry points        */
#define DEV32ENTRY   _System                            /* device_init entry point     */
#define FSH32ENTRY   _System                            /* fsh32_* entry points        */
#define FSH32ENTRY2  _Optlink                           /* fsh32_* entry points        */
#define DRV32ENTRY   _Far32 _Pascal _loadds             /* strategy or IDC entry point */
#define FS32CALLBACK _Far32 _Pascal

#pragma pack(1)
typedef struct {
    unsigned short ofs;
    unsigned short seg;
} PTR16;
#pragma pack()

// #define FAR    /**/
// #define NEAR   /**/
// #define near   /**/
#define far    /**/
#define pascal /**/
    typedef char           INT8;
    typedef char *pchar;
    typedef unsigned char  UINT8;
    typedef short          INT16;
    typedef unsigned short UINT16;
    typedef int            INT32;
    typedef unsigned int   UINT32;
    typedef UINT32        *PUINT32;
    typedef INT32         *PINT32;

#define __const__  const          // for os2/fnmatch.h
#define __inline__ INLINE         // for os2/ctype.h

#else
#if defined(__EMX__)
    typedef char           INT8;
    typedef char *pchar;
    typedef unsigned char  UINT8;
    typedef short          INT16;
    typedef unsigned short UINT16;
    typedef int            INT32;
    typedef unsigned int   UINT32;
    typedef UINT32        *PUINT32;
    typedef INT32         *PINT32;

#define __const__  const          // for os2/fnmatch.h
#define __inline__ inline         // for os2/ctype.h
#else

                                                 // 16 bits MS Visual C++ types
    #define INLINE _inline
    #define _FS_ENTRY far  pascal _loadds

    #define _FS_PTR   far *             // This is no longer necessary with MSVC : -Gx- assumes data is FAR, I'll cleanup this later !
    #define _FS_RET   int

    typedef char           INT8;      /* signed 8 bits integer    */
    typedef INT8           int8;

    typedef unsigned char  UINT8;     /* unsigned 8 bits integer  */
    typedef int            INT16;     /* signed 16 bits integer   */
    typedef unsigned int   UINT16;    /* unsigned 16 bits integer */
    typedef long           INT32;     /* signed 32 bits integer   */

    typedef unsigned long  UINT32;    /* unsigned 32 bits integer */
    typedef UINT32 _FS_PTR PUINT32;   /* pointer to unsigned 32 bits integer */
    typedef INT32 _FS_PTR PINT32;   /* pointer to unsigned 32 bits integer */
    typedef char _FS_PTR   pchar;

#if defined (__WATCOM__)

//#define FS32ENTRY  _Far32  _Pascal
#define FS32ENTRY    _Pascal                     /* fs32_* entry points         */
#define FS32INIT     _System                     /* fsh32_* entry points        */

#pragma pack(1)
typedef struct {
    unsigned short ofs;
    unsigned short seg;
} PTR16;
#pragma pack()

#endif
#endif
#endif

typedef UINT32  blk_t;
typedef PUINT32 pblk_t;

// typedef UINT32 fileptr_t;
typedef INT32 fileptr_t;
// typedef PUINT32 pfileptr_t;
typedef PINT32 pfileptr_t;


// #ifndef OS2

typedef UINT32 __u32;           /* Linux unsigned 32 bits integer */
typedef UINT32   u32;           /* Linux unsigned 32 bits integer */
typedef UINT16 __u16;           /* Linux unsigned 16 bits integer */
typedef UINT8  __u8;            /* Linux unsigned  8 bits integer */

typedef INT32  __s32;
typedef INT16  __s16;
typedef INT8   __s8;

typedef UINT32 sem_t;           // RAM semaphore
typedef PUINT32 psem_t;         // pointer to RAM semaphore

/*
 * Defnintions not used for user mode programs
 */
#if defined (__KERNEL__) || defined (__LIBEXT2FS__)
//
// From /usr/src/linux-1.2.1/include/linux/types.h
//
// typedef int pid_t;
// typedef unsigned short dev_t;
// typedef __u32 ino_t;
// typedef unsigned short mode_t;
typedef unsigned short umode_t;
typedef unsigned short nlink_t;
//typedef int daddr_t;
typedef long off_t;
typedef long loff_t;
typedef unsigned long mode_t;
#ifdef __KERNEL__
typedef long time_t; /* emx defines this as unsigned long */
#endif

// end from

typedef unsigned short pid_t;

#define asmlinkage

typedef unsigned short uid_t;
typedef unsigned short gid_t;
typedef uid_t __uid_t;
typedef gid_t __gid_t;

typedef UINT32 ino_t;
typedef PUINT32 pino_t;

typedef UINT16 dev_t;           /* for hVPB */

#endif /* __KERNEL__ */

#endif /* __os2_types_h */
