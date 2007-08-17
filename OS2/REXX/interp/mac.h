/*
 * This file contains functions that are missing in Apple's MPW compilers.
 */
#if defined(MAC)
# if !defined(_MAC_H_INCLUDED)
# define _MAC_H_INCLUDED
#include <fcntl.h>
#include <string.h>
#include	<ErrMgr.h>
#include	<CursorCtl.h>
#include	<Errors.h>
typedef unsigned long dev_t;

/*
 * For stat() function...
 */
struct stat
{
   int st_dev;
   long st_ino;
   dev_t st_rdev;
   off_t st_size;
   int st_mode;
   time_t st_mtime;
   int st_nlink;
};

#define S_IFMT  0
#define S_IFREG 1
#define S_IFDIR 2

#define F_OK 0
#define R_OK 1
#define W_OK 2
#define X_OK 4

int stat( char *, struct stat * );
int fstat( int fd, struct stat * );
int sleep( int ms ) ;
char * getcwd( char *buf, size_t size );
int chdir( char *buf );
int isatty( int fd );
# endif /* _MAC_H_INCLUDED */
#endif
