#include <io.h>
#include <fcntl.h>
#include <stdio.h>

/* Import: NFILE, NOSTDIO, NOBUFFER */

#ifndef NFILE
# define NFILE 6
#endif

/*----------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C"
#endif
unsigned _Cdecl _nfile = NFILE;

#ifdef __cplusplus
extern "C"
#endif
unsigned int _Cdecl _openfd [NFILE] =
{
	O_RDONLY | O_DEVICE | O_TEXT,
	O_WRONLY | O_DEVICE | O_TEXT,
	O_WRONLY | O_DEVICE | O_TEXT,
	O_RDWR   | O_DEVICE | O_BINARY,
	O_WRONLY | O_DEVICE | O_BINARY
};

/*----------------------------------------------------------------------*/

#ifndef NOSTDIO
# ifdef __cplusplus
extern "C"
# endif
FILE _Cdecl _streams [NFILE] =
{
	{ 0, _F_READ|_F_TERM|_F_LBUF,	0, 0, 0, NULL, NULL, 0, (short)stdin },
	{ 0, _F_WRIT|_F_TERM|_F_LBUF,	1, 0, 0, NULL, NULL, 0, (short)stdout },
	{ 0, _F_WRIT|_F_TERM,		2, 0, 0, NULL, NULL, 0, (short)stderr },
	{ 0, _F_RDWR|_F_TERM|_F_BIN,	3, 0, 0, NULL, NULL, 0, (short)stdaux },
	{ 0, _F_WRIT|_F_TERM|_F_BIN,	4, 0, 0, NULL, NULL, 0, (short)stdprn },
# if NFILE > 5
	{ 0, 0,			-1, 0, 0, NULL, NULL, 0, (short)&_streams[5] },
# endif
# if NFILE > 6
	{ 0, 0,			-1, 0, 0, NULL, NULL, 0, (short)&_streams[6] },
# endif
# if NFILE > 7
	{ 0, 0,			-1, 0, 0, NULL, NULL, 0, (short)&_streams[7] },
# endif
};
#endif

/*----------------------------------------------------------------------*/

/* "Check for stack overflow" off for BC++ 3.1				*/
/* "Suppress redundant load" on for BC++ 3.1				*/
#pragma option -N- -Z

#ifndef NOSTDIO
# ifdef NOBUFFER

static void near __fastcall _setbuf_ (FILE *fp) {
	if (!isatty (fp->fd))
		fp->flags &= ~_F_TERM;
}

# else

static void near __fastcall _setbuf_ (FILE *fp) {
	unsigned type;
	if (!isatty (fp->fd)){
		fp->flags &= ~_F_TERM,
		type = _IOFBF;
	} else	type = _IONBF;
	setvbuf (fp, NULL, type, BUFSIZ);
}

# endif
#endif

#ifdef __cplusplus
extern "C"
#endif
void near _Cdecl _setupio (void) {
#ifndef NOSTDIO
# if NFILE > 8
	FILE *fp = &_streams [8];
	do {	/*_openfd[i] = 0,*/
		fp->fd = -1, fp->token = (short)fp, fp++;
	} while (fp < &_streams [NFILE]);
# endif
	_setbuf_ (stdin);
	_setbuf_ (stdout);
#endif
}

#pragma option -N. -Z.

#ifndef NOSTDIO
# pragma startup _setupio 2
#endif
