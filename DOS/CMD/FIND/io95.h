/* $Id: IO95.H 1.3 2000/01/11 09:39:41 ska Exp ska $
   $Locker: ska $	$Name:  $	$State: Exp $

	I/O functions supporting Win95's LFN as well as DOS 4+.
	This implementation uses no global variables to determine default values!

	Differences:
	- A findfirst/findnext cycle must be closed with a findclose95().
	- The mode parameter of open95() is long rather than intú
		You may ignore the difference if you always use one of:
			O95_RDONLY, O95_WRONLY, O95_RDWR, or O95_SNOOP.
	- The O95_* macros differ from the standard. So don't set the USE_IO95
		and the "#include <fcntl.h>" simultaneously!

   $Log: IO95.H $
   Revision 1.3  2000/01/11 09:39:41  ska
   add: STDIO95.H

   Revision 1.2  2000/01/11 09:34:48  ska
   add: support Turbo C v2.01

   Revision 1.1  2000/01/11 09:10:09  ska
   Auto Check-in

*/

#ifndef __IO95_H
#define __IO95_H

/******
	compiler - specific settings
		*******/

			/* if defined, getdta() and setdta() do exist */
#define	HAVE_GETSETDTA
	/* See also: stdio95.h */



#ifndef HI_TECH_C
#include <io.h>
#endif

/**********		Unchanged functions 	********/
#define close95 close
#define lseek95 lseek

#define unlink95 remove95

/**********		Open mode macros		*********/
#define O95_AUTOCOMMIT	(1<<14)	/* flush file after each write	*/
#define O95_NOCRIT		(1<<13)	/* no critical error handler (autofail) */
#define O95_NOCOMPRESS	(1<< 9)	/* don't compress on volumes normally compress (Win95 only, otherwise ignored) */
#define O95_UNBUFFERED	(1<< 8)	/* don't BUFFER; all IO must use physical sector size (Win95 only, otherwise ignored) */
#define O95_NOINHERIT		(1<< 7)	/* don't inherit file descriptor to child processes */
#define O95_RDONLY		(0x00L)	/* read access only */
#define O95_WRONLY		(0x01L)	/* write access only */
#define O95_RDWR			(0x02L)	/* read & write access */
#define O95_SNOOP			(0x04L)	/* read access only; don't update "last access time" (Win95 only, otherwise ignored) */
#define O95_DENYALL		(1<< 4)	/* deny read&write shared access */
#define O95_DENYWRITE		(2<< 4)	/* deny write shared access (default) */
#define O95_DENYREAD		(3<< 4)	/* deny read shared access */
#define O95_DENYNONE		(4<< 4)	/* deny no shared access */

#define O95_CREAT	0x010000l
#define O95_TRUNC	0x020000l
#define O95_EXCL	0x040000l
#define O95_APPEND	0x080000l
#define O95_TEXT	0x100000l
#define O95_BINARY	0

/* Creation flags for open95() */
#define S95_IREAD	0			/* DOS cannot deny read permission */
#define S95_IWRITE	1			/* Grant write permission */
#define S95_IHIDDEN	2
#define S95_ISYSTEM	4

#undef MAXPATH95
#define MAXPATH95 270
#undef MAXNAME95
#define MAXNAME95 256
#undef MAXDIR95
#define MAXDIR95 MAXPATH95
#undef MAXDRIVE95
#define MAXDRIVE95 MAXNAME95
#undef MAXFILE95
#define MAXFILE95 MAXNAME95
#undef MAXEXT95
#define MAXEXT95 MAXNAME95

#ifdef IO95_NO_OPEN_PROTOTYPE
int open95();
#else
int open95(const char * const fnam, long omode, ... /* [, int flags]*/ );
#endif
/*
	fnam: file to be opened
	omode: open mode, any combination of:
		O95_BINARY:	open in binary mode (default)
		O95_TEXT:		open in text mode
		O95_CREAT:	create if not exist
		O95_TRUNC:	truncate if exist
		O95_APPEND:	append if exist
		O95_EXCL:		fail if exist (preceed O95_TRUNC)
		Exactly one of the following must be given:
			O95_RDWR:		read/write access
			O95_RDONLY:	read access
			O95_WRONLY:	write access
			O95_NDELAY:	(ignored)
		Non-portable share flags:
			O95_DENYALL:	deny every shared access
			O95_DENYREAD:	deny only read access
			O95_DENYWRITE:	deny only write access (default)
			O95_DENYNONE:	deny no shared access
	flags: create attribute flags, if file is to be created
		any combination of:
			S95_IREAD:	Allow read (ignored)
			S95_IWRITE:	Allow write

	Return:
		-1: failure
		else: file descriptor
*/

int mkdir95(const char * const dnam);
int rmdir95(const char * const dnam);
int chdir95(const char * const dnam);
/*	Return:
		== 0: success
		else: OS error code
*/

int remove95(const char * const fnam);
/*	remove/unlink file

	Return:
		0: success
		else: failure
*/

int rename95(const char * const oldname, const char * const newname);
/*	Rename the oldname into newname.

	Return:
		0: success
		else: failure
*/

int truename95(const char * const fnam, char * const buf);
/* Fully-qualify fnam and convert all long filename portions into
	their short form.

	The output filename may contain lowercase character.
	buf should be greater or equal than 267 characters.

	Bug: If the coverted filename is >255, the Win95 API returns
		an error. If this error occurs, but the output buffer has
		been changed to a string longer than 255 characters, the
		function assumes that the bug has been triggered ans that the
		converted filename is correct.

	Return:
		0: success
		2: invalid component in path or drive letter
		3: malformed path or buffer too short
*/

int lfn2sfn95(const char * const fnam, char * const buf);
/* Convert all long filename portions into their short form.

	The output filename may contain lowercase character.
	buf[] should be greater or equal than 67 characters.

	Return:
		0: success
		2: invalid component in path or drive letter
		3: malformed path or buffer too short
*/

char *fullpath95(char * const Xbuffer, const char *path, int buflen);
/* Fully-qualify path.
   	If buffer != NULL, buflen tells its size.
   	If buffer == NULL, a buffer of buflen bytes is malloc'ed.

   	This code requires to define "maximum" values for the filesystem:
   		MAXPATH95		260
   		MAXNAME95		255
	Unfortunately, these values are assumptions for Win95 & DOS only!

	If the path starts with "\\\\" both backslashes remain; that
	gives some support for UNC path spec.

	Return:
		NULL: on failure
		buffer: on success && buffer != NULL
		!= NULL: on success && buffer == NULL; malloc'ed area
*/

int getcurdir95(int drive, char * const dir);
char * getdcwd95(int drive, char *buf, int buflen);
char * getcwd95(char *buf, int buflen);
/* Get current working directory without drive/colon/leading backslash

	Return:
		0: success
		-1: error
*/

/**********		Define USE_IO95 to always use io95-functions	********/
#ifdef USE_IO95
#define open open95
#define sopen(fnam,arg,sarg,mode) open95((fnam),(arg)|(sarg),mode)
#define mkdir mkdir95
#define chdir chdir95
#define rmdir rmdir95
#define getdcwd getdcwd95
#define getcwd getcwd95
#define getcurdir getcurdir95
#define remove remove95
#define unlink remove95
#define rename rename95
#define fullpath fullpath95
#define lfn2sfn lfn2sfn95
#undef O_AUTOCOMMIT
#define O_AUTOCOMMIT O95_AUTOCOMMIT
#undef O_NOCRIT
#define O_NOCRIT O95_NOCRIT
#undef O_NOCOMPRESS
#define O_NOCOMPRESS O95_NOCOMPRESS
#undef O_UNBUFFERED
#define O_UNBUFFERED O95_UNBUFFERED
#undef O_NOINHERIT
#define O_NOINHERIT O95_NOINHERIT
#undef O_RDONLY
#define O_RDONLY O95_RDONLY
#undef O_WRONLY
#define O_WRONLY O95_WRONLY
#undef O_RDWR
#define O_RDWR O95_RDWR
#undef O_SNOOP
#define O_SNOOP O95_SNOOP
#undef O_DENYALL
#define O_DENYALL O95_DENYALL
#undef O_DENYWRITE
#define O_DENYWRITE O95_DENYWRITE
#undef O_DENYREAD
#define O_DENYREAD O95_DENYREAD
#undef O_DENYNONE
#define O_DENYNONE O95_DENYNONE
#undef O_CREAT
#define O_CREAT O95_CREAT
#undef O_TRUNC
#define O_TRUNC O95_TRUNC
#undef O_EXCL
#define O_EXCL O95_EXCL
#undef O_APPEND
#define O_APPEND O95_APPEND
#undef O_TEXT
#define O_TEXT O95_TEXT
#undef O_BINARY
#define O_BINARY O95_BINARY
#undef S_IREAD
#define S_IREAD S95_IREAD
#undef S_IWRITE
#define S_IWRITE S95_IWRITE
#undef MAXPATH
#define MAXPATH MAXPATH95
#undef MAXNAME
#define MAXNAME MAXNAME95
#undef MAXDRIVE
#define MAXDRIVE MAXDRIVE95
#undef MAXDIR
#define MAXDIR MAXDIR95
#undef MAXEXT
#define MAXEXT MAXEXT95
#undef MAXFILE
#define MAXFILE MAXFILE95
#undef findfirst
#define findfirst findfirst95
#undef findnext
#define findnext findnext95
#undef findstop
#define findstop findstop95
#undef ffblk
#define ffblk ffblk95
#undef FF_Block
#define FF_Block FF_Block95
#endif

#endif
