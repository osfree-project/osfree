


// #include <stdio.h>

// **** Internal Watcom values and functions ****

// __NFiles max number of file handles - internal rtl variable
extern unsigned int __NFiles;

// clib - stiomode.c
extern signed __SetIOMode( int handle, unsigned value );

// clib - _dos/c/dosret.c
_WCRTLINK extern int __set_errno_dos( unsigned int );
_WCRTLINK extern void __set_errno( unsigned int );

// clib - umaskval.c, rtdata.h -- default file permission mask
extern int __umaskval;
#define _RWD_umaskval  __umaskval

// stdlib.h
#define _RWD_fmode _fmode

// **** end of internal Open Watcom ****

//#define OS2LLTO64(a)    (((__int64)(a).ulHi) << 32) + ((__int64)(a).ulLo);
#define OS2LLTO64(a)    (((__int64)(a).ulHi) << 32) + ((__int64)(a).ulLo);
#define i64TOOS2LLhi(a) (unsigned long)((a) >> 32)
#define i64TOOS2LLlo(a) (unsigned long)(a)

// from fileacc.h
#define _ValidFile( fp, retval )        /* make sure fp is a valid pointer */
#define SetupTGCSandNCS( x )            /* initialization for Netware/386 */

extern void (*_AccessFileH)( int );
extern void (*_ReleaseFileH)( int );
extern void (*_AccessIOB)( void );
extern void (*_ReleaseIOB)( void );

#define _AccessFile( fp )       _AccessFileH( (fp)->_handle )
#define _ReleaseFile( fp )      _ReleaseFileH( (fp)->_handle )

struct __pipe_info {
    int                 isPipe;     /* non-zero if it's a pipe */
    int                 pid;        /* PID of spawned process */
};

typedef struct __stream_link {
    struct __stream_link *  next;
    struct __iobuf *        stream;
    unsigned char *         _base;          /* location of buffer */
    int                     _orientation;   /* wide/byte/not oriented */
    int                     _extflags;      /* extended flags */
    unsigned char           _tmpfchar;      /* tmpfile number */
    unsigned char           _filler[sizeof(int)-1];/* explicit padding */
    struct __pipe_info      pipeInfo;       /* pipe-related fields */
} __stream_link;

// lib_misc/h/rtdata.h
extern __stream_link  *__OpenStreams;
#define _RWD_ostream  __OpenStreams
extern __stream_link  *__ClosedStreams;
#define _RWD_cstream  __ClosedStreams


#define PMODE   (S_IREAD | S_IWRITE)

#define _FP_BASE(__fp)          ((__fp)->_link->_base)
#define _FP_ORIENTATION(__fp)   ((__fp)->_link->_orientation)
#define _FP_EXTFLAGS(__fp)      ((__fp)->_link->_extflags)
#define _FP_TMPFCHAR(__fp)      ((__fp)->_link->_tmpfchar)
#define _FP_PIPEDATA(__fp)      ((__fp)->_link->pipeInfo)


// clib - streamio/c/fdopen.c -- use rtl __open_flags
extern int __open_flags( const char *modestr, int *extflags );
extern void __chktty( FILE *fp );

// clib - streamio/h/streamio.h
extern FILE *__allocfp( int handle );
extern void __freefp( FILE * );

// clib - _dos/c/dosret.c
_WCRTLINK extern void __set_errno( unsigned int );

// clib - streamio/c/fclose.c
extern int __doclose( FILE *fp, int close_handle );
extern unsigned __GetIOMode( int __handle );
extern int __flush( FILE * );

