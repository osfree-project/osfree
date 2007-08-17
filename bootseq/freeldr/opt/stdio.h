/*
 *  stdio.h     Standard I/O functions
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
#ifndef _STDIO_H_INCLUDED
#define _STDIO_H_INCLUDED
#if !defined(_ENABLE_AUTODEPEND)
  #pragma read_only_file;
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _COMDEF_H_INCLUDED
 #include "_comdef.h"
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

#ifndef _WCTYPE_T_DEFINED
#define _WCTYPE_T_DEFINED
#define _WCTYPE_T_DEFINED_
typedef wchar_t wint_t;
typedef wchar_t wctype_t;
#endif

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
#define _SIZE_T_DEFINED_
typedef unsigned size_t;
#endif

#ifndef NULL
 #if defined(__SMALL__) || defined(__MEDIUM__) || defined(__386__) || defined(__AXP__) || defined(__PPC__)
  #define NULL   0
 #else
  #define NULL   0L
 #endif
#endif

#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
#if defined(__PPC__)
 #if defined(__NT__)
    typedef char * __va_list;
 #else
    typedef struct {
      char  __gpr;
      char  __fpr;
      char  __reserved[2];
      char *__input_arg_area;
      char *__reg_save_area;
    } __va_list;
  #endif
#elif defined(__AXP__)
  typedef struct {
    char *__base;
    int   __offset;
  } __va_list;
#elif defined(__HUGE__) || defined(__SW_ZU)
  typedef char _WCFAR *__va_list[1];
#else
  typedef char *__va_list[1];
#endif
#endif

#if defined(__386__) || defined(__AXP__) || defined(__PPC__)
 #define BUFSIZ         4096
#else
 #define BUFSIZ         512
#endif
#define _NFILES         20      /* number of files that can be handled */
#if defined(__OS2__) || defined(__NT__)
 #define FILENAME_MAX   260
#else
 #define FILENAME_MAX   144
#endif

struct __stream_link;
typedef struct  __iobuf
#if !defined(__OBSCURE_STREAM_INTERNALS)
{
    unsigned char        *_ptr;         /* next character position */
    int                   _cnt;         /* number of characters left */
    struct __stream_link *_link;        /* location of associated struct */
    unsigned              _flag;        /* mode of file access */
    int                   _handle;      /* file handle */
    unsigned              _bufsize;     /* size of buffer */
    unsigned short        _ungotten;    /* used by ungetc and ungetwc */
}
#endif
FILE;

typedef long    fpos_t;

#if !defined(NO_EXT_KEYS) /* extensions enabled */
 #define FOPEN_MAX      _NFILES
 #define OPEN_MAX       FOPEN_MAX
 #if defined(__OS2__) || defined(__NT__)
  #define PATH_MAX      259 /* maximum length of full pathname excl. '\0' */
 #else
  #define PATH_MAX      143 /* maximum length of full pathname excl. '\0' */
 #endif
#else                   /* extensions not enabled */
 #define FOPEN_MAX      (_NFILES-2)
#endif

#if defined(__FUNCTION_DATA_ACCESS)
 #define __iob (*__get_iob_ptr())
#elif defined(__SW_BR) || defined(_RTDLL)
 #define __iob __iob_br
#endif

#if !defined(__OBSCURE_STREAM_INTERNALS)

_WCRTLINK extern FILE _WCNEAR __iob[];
/*
 *  Define macros to access the three default file pointer (and descriptors)
 *  provided to each process by default.  They will always occupy the
 *  first three file pointers in each processes' table.
 */
#define stdin   ((FILE *)&__iob[0])     /* standard input file  */
#define stdout  ((FILE *)&__iob[1])     /* standard output file */
#define stderr  ((FILE *)&__iob[2])     /* standard error file  */
#ifndef NO_EXT_KEYS     /* extensions enabled */
#if !defined(__NT__)
#define stdaux  ((FILE *)&__iob[3])     /* standard auxiliary file  */
#define stdprn  ((FILE *)&__iob[4])     /* standard printer file  */
#endif
#endif

#else

_WCRTLINK extern FILE *__get_std_file(unsigned handle);

#define stdin   (__get_std_file(0))     /* standard input file  */
#define stdout  (__get_std_file(1))     /* standard output file */
#define stderr  (__get_std_file(2))     /* standard error file  */
#ifndef NO_EXT_KEYS     /* extensions enabled */
#if !defined(__NT__)
#define stdaux  (__get_std_file(3))     /* standard auxiliary file  */
#define stdprn  (__get_std_file(4))     /* standard printer file  */
#endif
#endif

#endif

/* values for _flag field in FILE struct and _iomode array */

#define _READ   0x0001  /* file opened for reading */
#define _WRITE  0x0002  /* file opened for writing */
#define _UNGET  0x0004  /* ungetc has been done */
#define _BIGBUF 0x0008  /* big buffer allocated */
#define _EOF    0x0010  /* EOF has occurred */
#define _SFERR  0x0020  /* error has occurred on this file */
#define _APPEND 0x0080  /* file opened for append */
#define _BINARY 0x0040  /* file is binary, skip CRLF processing */
#define _IOFBF  0x0100  /* full buffering */
#define _IOLBF  0x0200  /* line buffering */
#define _IONBF  0x0400  /* no buffering */
#define _TMPFIL 0x0800  /* this is a temporary file */
#define _DIRTY  0x1000  /* buffer has been modified */
#define _ISTTY  0x2000  /* is console device */
#define _DYNAMIC 0x4000 /* FILE is dynamically allocated   */
#define _FILEEXT 0x8000 /* lseek with positive offset has been done */
#define _COMMIT 0x0001  /* extended flag: commit OS buffers on flush */

#define EOF         (-1)                /*  End of File/Error return code   */
#define WEOF        ((wint_t)(-1))      /*  EOF equivalent for wide chars   */

#define SEEK_SET    0                   /*  Seek relative to start of file  */
#define SEEK_CUR    1                   /*  Seek relative to current positn */
#define SEEK_END    2                   /*  Seek relative to end of file    */

#define _NOT_ORIENTED   0               /* stream not yet oriented */
#define _BYTE_ORIENTED  1               /* byte-oriented stream */
#define _WIDE_ORIENTED  2               /* wide-oriented stream */

#define L_tmpnam    13
#define _P_tmpdir   "\\"                /* used by _tempnam */
#define _wP_tmpdir  L"\\"               /* used by _wtempnam */
#define TMP_MAX     (26*26*26)          /*  Max times tmpnam can be called  */

_WCRTLINK extern void   clearerr( FILE *__fp );
_WCRTLINK extern int    fclose( FILE *__fp );
_WCRTLINK extern int    feof( FILE *__fp );
_WCRTLINK extern int    ferror( FILE *__fp );
_WCRTLINK extern int    fflush( FILE *__fp );
_WCRTLINK extern int    fgetc( FILE *__fp );
_WCRTLINK extern int    fgetpos( FILE *__fp, fpos_t *__pos );
_WCRTLINK extern char   *fgets( char *__s, int __n, FILE *__fp );
_WCRTLINK extern FILE   *fopen( const char *__filename, const char *__mode );
_WCRTLINK extern int    fprintf( FILE *__fp, const char *__format, ... );
_WCRTLINK extern int    fputc( int __c, FILE *__fp );
_WCRTLINK extern int    fputs( const char *__s, FILE *__fp );
_WCRTLINK extern size_t fread( void *__ptr, size_t __size, size_t __n,
                               FILE *__fp );
_WCRTLINK extern FILE   *freopen( const char *__filename, const char *__mode,
                                  FILE *__fp );
_WCRTLINK extern int    fscanf( FILE*__fp, const char *__format, ... );
_WCRTLINK extern int    fseek( FILE *__fp, long int __offset, int __whence );
_WCRTLINK extern int    fsetpos( FILE *__fp, const fpos_t *__pos );
_WCRTLINK extern long int ftell( FILE *__fp );
_WCRTLINK extern size_t fwrite( const void *__ptr, size_t __size, size_t __n,
                                FILE *__fp );
_WCRTLINK extern int    getc( FILE *__fp );
_WCRTLINK extern int    getchar( void );
_WCRTLINK extern int    _getw( FILE *__fp );
_WCRTLINK extern char   *gets( char *__s );
_WCRTLINK extern void   perror( const char *__s );
_WCRTLINK extern FILE * _popen( const char *__command, const char *__mode );
_WCRTLINK extern int    _pclose( FILE *__fp );
_WCRTLINK extern int   __cdecl printf( const char *__format, ... );
_WCRTLINK extern int    putc( int __c, FILE *__fp );
_WCRTLINK extern void __cdecl putchar(char c);

_WCRTLINK extern int    _putw( int __binint, FILE *__fp );
_WCRTLINK extern int    puts( const char *__s );
_WCRTLINK extern int    remove( const char *__filename );
_WCRTLINK extern int    rename( const char *__old, const char *__new );
_WCRTLINK extern void   rewind( FILE *__fp );
_WCRTLINK extern int    scanf( const char *__format, ... );
_WCRTLINK extern void   setbuf( FILE *__fp, char *__buf );
_WCRTLINK extern int    setvbuf( FILE *__fp, char *__buf, int __mode,
                                 size_t __size );
_WCRTLINK extern int    sprintf( char *__s, const char *__format, ... );
_WCRTLINK extern int    sscanf( const char *__s, const char *__format, ... );
_WCRTLINK extern char * _tempnam( char *__dir, char *__prefix );
_WCRTLINK extern FILE   *tmpfile( void );
_WCRTLINK extern char   *tmpnam( char *__s );
_WCRTLINK extern int    ungetc( int __c, FILE *__fp );
_WCRTLINK extern int    vfprintf( FILE *__fp, const char *__format,
                                  __va_list __arg );
_WCRTLINK extern int    vprintf( const char *__format, __va_list __arg );

#if !defined(NO_EXT_KEYS) /* extensions enabled */
_WCRTLINK extern int    fileno(FILE *);
_WCRTLINK extern int    _fileno(FILE *);
_WCRTLINK extern int    fcloseall( void );
_WCRTLINK extern FILE   *fdopen( int __handle, const char *__mode );
_WCRTLINK extern FILE   *_fdopen( int __handle, const char *__mode );
_WCRTLINK extern int    _grow_handles( int __new_count );
_WCRTLINK extern int    _fgetchar( void );
_WCRTLINK extern int    fgetchar( void );
_WCRTLINK extern int    _fputchar( int __c );
_WCRTLINK extern int    fputchar( int __c );
_WCRTLINK extern FILE   *_fsopen( const char *__filename, const char *__mode,
                                  int __shflag );
_WCRTLINK extern int    flushall( void );
_WCRTLINK extern int    vfscanf( FILE *__fp, const char *__format,
                                 __va_list __arg );
_WCRTLINK extern int    vscanf( const char *__format, __va_list __arg );
_WCRTLINK extern int    vsscanf( const char *__s, const char *__format,
                                 __va_list __arg );
_WCRTLINK extern int    snprintf( char *__buf, size_t __bufsize,
                                  const char *__fmt, ... );
_WCRTLINK extern int    _bprintf( char *__buf, size_t __bufsize,
                                  const char *__fmt, ... );
_WCRTLINK extern int    _snprintf( char *__buf, size_t __bufsize,
                                   const char *__fmt, ... );
_WCRTLINK extern int    vsnprintf( char *__s, size_t __bufsize,
                                   const char *__format, __va_list __arg );
_WCRTLINK extern int    _vbprintf( char *__s, size_t __bufsize,
                                   const char *__format, __va_list __arg );
_WCRTLINK extern int    _vsnprintf( char *__s, size_t __bufsize,
                                    const char *__format, __va_list __arg );

_WCRTLINK extern FILE   *_wfopen( const wchar_t *, const wchar_t * );
_WCRTLINK extern int    fwprintf( FILE *, const wchar_t *, ... );
_WCRTLINK extern int    fputws( const wchar_t *, FILE * );
_WCRTLINK extern FILE   *_wfsopen( const wchar_t *__filename,
                                   const wchar_t *__mode,  int __shflag );
_WCRTLINK extern FILE   *_wfdopen( int, const wchar_t * );
_WCRTLINK extern FILE   *_wfreopen( const wchar_t *, const wchar_t *, FILE * );
_WCRTLINK extern wint_t putwc( wint_t, FILE * );
_WCRTLINK extern wint_t fputwc( wint_t, FILE * );
_WCRTLINK extern wint_t getwc( FILE * );
_WCRTLINK extern wint_t fgetwc( FILE * );
_WCRTLINK extern wint_t ungetwc( wint_t, FILE * );
_WCRTLINK extern int    fwscanf( FILE *, const wchar_t *, ... );
_WCRTLINK extern wchar_t *fgetws( wchar_t *, int, FILE * );
_WCRTLINK extern int    vfwprintf( FILE *, const wchar_t *, __va_list );
_WCRTLINK extern int    vfwscanf( FILE *, const wchar_t *, __va_list );
_WCRTLINK extern int    vwscanf( const wchar_t *, __va_list );
_WCRTLINK extern int    vswscanf( const wchar_t *, const wchar_t *, __va_list );
_WCRTLINK extern int    vswprintf( wchar_t *, size_t, const wchar_t *,
                                   __va_list );
_WCRTLINK extern wint_t getwchar( void );
_WCRTLINK extern wint_t _fgetwchar( void );
_WCRTLINK extern wint_t fgetwchar( void );
_WCRTLINK extern wchar_t *_getws( wchar_t * );
_WCRTLINK extern wchar_t *getws( wchar_t * );
_WCRTLINK extern wint_t putwchar( wint_t );
_WCRTLINK extern int    _putws( const wchar_t * );
_WCRTLINK extern int    putws( const wchar_t * );
_WCRTLINK extern int    _wremove( const wchar_t * );
_WCRTLINK extern wchar_t *_wtempnam( wchar_t *__dir, wchar_t *__prefix );
_WCRTLINK extern wchar_t *_wtmpnam( wchar_t * );
_WCRTLINK extern int    wprintf( const wchar_t *, ... );
_WCRTLINK extern int    wscanf( const wchar_t *, ... );
#ifndef swprintf
_WCRTLINK extern int    swprintf( wchar_t *, size_t, const wchar_t *, ... );
#endif
_WCRTLINK extern int    swscanf( const wchar_t *, const wchar_t *, ... );
_WCRTLINK extern wint_t _fputwchar( wint_t );
_WCRTLINK extern wint_t fputwchar( wint_t );
_WCRTLINK extern void   _wperror( const wchar_t * );
_WCRTLINK extern int    vwprintf( const wchar_t *, __va_list );
_WCRTLINK extern int    _swprintf( wchar_t *, const wchar_t *, ... );
_WCRTLINK extern int    _vswprintf( wchar_t *, const wchar_t *, __va_list );
_WCRTLINK extern int    _bwprintf( wchar_t *, size_t, const wchar_t *, ... );
_WCRTLINK extern int    _snwprintf( wchar_t *, size_t, const wchar_t *, ... );
_WCRTLINK extern int    _vbwprintf( wchar_t *, size_t, const wchar_t *, __va_list );
_WCRTLINK extern int    _vsnwprintf( wchar_t *, size_t, const wchar_t *,
                                    __va_list );
_WCRTLINK extern int    _wrename( const wchar_t *, const wchar_t * );
_WCRTLINK extern FILE * _wpopen( const wchar_t *__command, const wchar_t *__mode );

#endif

#if !defined(__OBSCURE_STREAM_INTERNALS)
#define clearerr(fp)    ((fp)->_flag &= ~(_SFERR|_EOF))
#define feof(fp)        ((fp)->_flag & _EOF)
#define ferror(fp)      ((fp)->_flag & _SFERR)
#define fileno(fp)      ((fp)->_handle)
#define _fileno(fp)     ((fp)->_handle)
#endif
#if defined(__SW_BD) || defined(__SW_BM)
#define getc(fp)        fgetc(fp)
#define putc(c,fp)      fputc(c,fp)
#else
#if !defined(__OBSCURE_STREAM_INTERNALS)
#define getc(fp) \
        ((fp)->_cnt<=0 \
        || (unsigned)((*(fp)->_ptr)-'\x0d')<=('\x1a'-'\x0d') \
        ? fgetc(fp) \
        : ((fp)->_cnt--,*(fp)->_ptr++))
#define putc(c,fp) \
        ((fp)->_flag&_IONBF \
        || (fp)->_bufsize-(fp)->_cnt<=1 \
        ? fputc(c,fp) \
        : ((*(fp)->_ptr=(unsigned char)(c))=='\n') \
        ? fputc('\n',fp) \
        : ((fp)->_flag|=_DIRTY,(fp)->_cnt++,*(fp)->_ptr++))
#endif
#endif
#define getchar()       getc(stdin)
//#define putchar(c)      putc(c,stdout)

#pragma pack(__pop);
#ifdef __cplusplus
};
#endif /* __cplusplus */
#endif
