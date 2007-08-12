/*
 *  dos.h       Defines the structs and unions used to handle the input and
 *              output registers for the DOS and 386 DOS Extender interface
 *              routines.
 *
 *                          Open Watcom Project
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
 */
#ifndef _DOS_H_INCLUDED
#define _DOS_H_INCLUDED
#if !defined(_ENABLE_AUTODEPEND)
  #pragma read_only_file;
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _COMDEF_H_INCLUDED
  #include "_comdef.h"
#endif
#if defined(_M_IX86) && !defined(_I86_H_INCLUDED)
 #include "i86.h"
#endif

#if defined(__DOS__) && defined(__386__)
 #define _DOSFAR _WCFAR
#else
 #define _DOSFAR _WCI86FAR
#endif

#if defined(_M_IX86)
  #pragma pack(__push,1);
#else
  #pragma pack(__push,8);
#endif

#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED_
#ifdef __cplusplus
typedef long char wchar_t;
#else
typedef unsigned short wchar_t;
#endif
#endif

/* dosexterror struct */

struct _DOSERROR {
        int exterror;
        char errclass;
        char action;
        char locus;
};
#ifndef __cplusplus
struct DOSERROR {
        int exterror;
        char class;
        char action;
        char locus;
};
#endif

struct dosdate_t {
        unsigned char day;      /* 1-31 */
        unsigned char month;    /* 1-12 */
        unsigned short year;    /* 1980-2099 */
        unsigned char dayofweek;/* 0-6 (0=Sunday) */
};
#define _dosdate_t dosdate_t

struct dostime_t {
        unsigned char hour;     /* 0-23 */
        unsigned char minute;   /* 0-59 */
        unsigned char second;   /* 0-59 */
        unsigned char hsecond;  /* 1/100 second; 0-99 */
};
#define _dostime_t dostime_t

struct find_t {
        char reserved[21];      /* reserved for use by DOS    */
        char attrib;            /* attribute byte for file    */
        unsigned short wr_time; /* time of last write to file */
        unsigned short wr_date; /* date of last write to file */
        unsigned long  size;    /* length of file in bytes    */
#if defined(__OS2__) || defined(__NT__)
        char name[256];         /* null-terminated filename   */
#else
        char name[13];          /* null-terminated filename   */
#endif
};
struct _wfind_t {
        char reserved[21];      /* reserved for use by DOS    */
        char attrib;            /* attribute byte for file    */
        unsigned short wr_time; /* time of last write to file */
        unsigned short wr_date; /* date of last write to file */
        unsigned long  size;    /* length of file in bytes    */
#if defined(__OS2__) || defined(__NT__)
        wchar_t name[256];      /* null-terminated filename   */
#else
        wchar_t name[13];       /* null-terminated filename   */
#endif
};
#define _find_t find_t

/* Critical error handler equates for _hardresume result parameter */

#define _HARDERR_IGNORE 0       /* Ignore the error */
#define _HARDERR_RETRY  1       /* Retry the operation */
#define _HARDERR_ABORT  2       /* Abort the program */
#define _HARDERR_FAIL   3       /* Fail the system call in progress */

/* File attribute constants for attribute field */

#define _A_NORMAL       0x00    /* Normal file - read/write permitted */
#define _A_RDONLY       0x01    /* Read-only file */
#define _A_HIDDEN       0x02    /* Hidden file */
#define _A_SYSTEM       0x04    /* System file */
#define _A_VOLID        0x08    /* Volume-ID entry */
#define _A_SUBDIR       0x10    /* Subdirectory */
#define _A_ARCH         0x20    /* Archive file */

#ifndef _DISKFREE_T_DEFINED
#define _DISKFREE_T_DEFINED
#define _DISKFREE_T_DEFINED_
struct _diskfree_t {
        unsigned total_clusters;
        unsigned avail_clusters;
        unsigned sectors_per_cluster;
        unsigned bytes_per_sector;
};
#define diskfree_t _diskfree_t
#endif

_WCRTLINK extern int      bdos( int __dosfn, unsigned int __dx,
                                unsigned int __al);
_WCRTLINK extern void     _chain_intr( register void
                                      (_WCINTERRUPT _DOSFAR *__handler)() );
#if defined(__NT__) || ( defined(__OS2__) && (defined(__386__)||defined(__PPC__)) )
_WCRTLINK extern unsigned _dos_allocmem( unsigned __size, void **__storage );
#else
_WCRTLINK extern unsigned _dos_allocmem( unsigned __size,
                                         unsigned short *__seg );
#endif
_WCRTLINK extern unsigned _dos_close( int __handle );
_WCRTLINK extern unsigned _dos_commit( int __handle );
_WCRTLINK extern unsigned _dos_creat( const char *__path,
                                      unsigned __attr, int *__handle );
_WCRTLINK extern unsigned _dos_creatnew( const char *__path,
                                         unsigned __attr, int *__handle );
_WCRTLINK extern unsigned _dos_findfirst( const char *__path,
                                          unsigned __attr,
                                          struct find_t *__buf );
_WCRTLINK extern unsigned _dos_findnext( struct find_t *__buf );
_WCRTLINK extern unsigned _dos_findclose( struct find_t *__buf );
#if defined(__NT__) || ( defined(__OS2__) && (defined(__386__)||defined(__PPC__)) )
_WCRTLINK extern unsigned _dos_freemem( void *__storage );
#else
_WCRTLINK extern unsigned _dos_freemem( unsigned short __seg );
#endif
_WCRTLINK extern void     _dos_getdate( struct dosdate_t *__date );
_WCRTLINK extern unsigned _dos_getdiskfree( unsigned __drive,
                                            struct _diskfree_t *__diskspace );
_WCRTLINK extern unsigned _getdiskfree( unsigned __drive,
                                        struct _diskfree_t *__diskspace );
_WCRTLINK extern void     _dos_getdrive( unsigned *__drive );
_WCRTLINK extern unsigned _getdrive( void );
_WCRTLINK extern unsigned _dos_getfileattr( const char *__path,
                                            unsigned *__attr );
_WCRTLINK extern unsigned _dos_getftime( int __handle, unsigned short *__date,
                                         unsigned short *__time );
_WCRTLINK extern void     _dos_gettime( struct dostime_t *__time );
_WCRTLINK extern void     (_WCINTERRUPT _DOSFAR *_dos_getvect( int __intnum ))();
_WCRTLINK extern void     _dos_keep( unsigned __retcode, unsigned __memsize );

_WCRTLINK extern unsigned _dos_open( const char *__path, unsigned __mode,
                                     int *__handle );
_WCRTLINK extern unsigned _dos_read( int __handle, void _DOSFAR *__buf,
                                     unsigned __count, unsigned *__bytes );
_WCRTLINK extern unsigned _dos_setblock( unsigned __size, unsigned short __seg,
                                         unsigned  *__maxsize );
_WCRTLINK extern unsigned _dos_setdate( struct dosdate_t *__date );
_WCRTLINK extern void     _dos_setdrive( unsigned __drivenum,
                                         unsigned *__drives );
_WCRTLINK extern unsigned _dos_setfileattr( const char *__path,
                                            unsigned __attr );
_WCRTLINK extern unsigned _dos_setftime( int __handle, unsigned short __date,
                                         unsigned short __time );
_WCRTLINK extern unsigned _dos_settime( struct dostime_t *__time );
_WCRTLINK extern void     _dos_setvect( int __intnum, void
                                       (_WCINTERRUPT _DOSFAR *__handler)() );
_WCRTLINK extern unsigned _dos_write( int __handle, void const _DOSFAR *__buf,
                                      unsigned __count, unsigned *__bytes );
_WCRTLINK extern int      dosexterr( struct _DOSERROR * );
_WCRTLINK extern void     _harderr( register int (_DOSFAR *__func)(
                                    unsigned __deverr, unsigned __errcode,
                                    unsigned _DOSFAR *__devhdr));
_WCRTLINK extern void     _hardresume( int __result );
_WCRTLINK extern void     _hardretn( int __error );
#if defined(_M_IX86)
_WCRTLINK extern int      intdos( union REGS *, union REGS * );
_WCRTLINK extern int      intdosx( union REGS *, union REGS *, struct SREGS * );
#endif
_WCRTLINK extern void     sleep( unsigned __seconds );

_WCRTLINK extern unsigned _wdos_findfirst( const wchar_t *__path,
                                           unsigned __attr,
                                           struct _wfind_t *__buf );
_WCRTLINK extern unsigned _wdos_findnext( struct _wfind_t *__buf );
_WCRTLINK extern unsigned _wdos_findclose( struct _wfind_t *__buf );

#pragma pack(__pop);

#undef _DOSFAR

#ifdef __cplusplus
};
#endif /* __cplusplus */
#endif
