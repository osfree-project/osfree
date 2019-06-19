#include <stdio.h>
#include <alloc.h>

#ifdef __cplusplus
extern "C"{
#endif

extern void _Cdecl (*_Cdecl _exitbuf)();
extern void _Cdecl	    _xfflush ();

#ifdef __cplusplus
}
#endif

/* "Check for stack overflow" off for BC++ 3.1 */
/* "Suppress redundant load" on for BC++ 3.1 */
#pragma option -N- -Z

#ifdef __cplusplus
extern "C"
#endif
int _Cdecl setvbuf (FILE *fp, char *buf, int type, size_t sz) {
	if (type == _IONBF)
		sz = 0;

/*	if (fp->token != (short)fp || type > _IONBF || sz > MAX_INT)
 *		return EOF;
 */
	fflush (fp); /* Ensure in no loss of characters */

	if (fp->flags & _F_BUF)
		free (fp->buffer);

	fp->flags &= ~(_F_BUF | _F_LBUF | _F_IN | _F_OUT | _F_EOF),
/*	fp->curp = fp->buffer = &fp->hold,
 */	fp->level = fp->bsize = 0;

	if (sz) {
		if (buf == NULL) {
			if ((buf = (char*) malloc (sz)) == NULL)
				return EOF;
			fp->flags |= _F_BUF;
		}

		if (type == _IOLBF)
			fp->flags |= _F_LBUF;
		fp->curp = fp->buffer = (unsigned char*) buf,
		fp->bsize = sz,
		_exitbuf = _xfflush;
	}

	return 0;
}

#pragma option -N. -Z.
