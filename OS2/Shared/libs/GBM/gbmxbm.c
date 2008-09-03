/*

gbmxbm.c - X Windows Bitmap support

Reads and writes most X bitmaps

History:
--------
(Heiko Nitzsche)

22-Feb-2006: Move format description strings to gbmdesc.h
15-Aug-2008: Integrate new GBM types

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"

/*...vgbm\46\h:0:*/
/*...vgbmhelp\46\h:0:*/
/*...e*/
/*...sisalnum fix:0:*/
#ifdef LINUX
/* On Slackware 3.4 /lib/libc.so.5 -> libc.5.4.33
   isalnum(c) is implemented as (__ctype_b(c)&_ISalnum)
   Programs compiled on Slackware therefore look for this bit
   On RedHat 6.0, /usr/i486-linux-libc5/lib/libc.so.5 -> libc.so.5.3.12
   isalnum(c) is probably done as (__ctype_b(c)&(_ISalnum|_ISdigit))
   So the __ctype_b array doesn't have this bit.
   So programs compiled, using isalnum on Slackware 3.4, don't work
   when run on RedHat 6.0. Best to avoid it. */
#undef isalnum
#define isalnum(c) (isalpha(c)||isdigit(c))
#endif
/*...e*/

static GBMFT xbm_gbmft =
	{
        GBM_FMT_DESC_SHORT_XBM,
        GBM_FMT_DESC_LONG_XBM,
        GBM_FMT_DESC_EXT_XBM,
	GBM_FT_R1|
	GBM_FT_W1,
	};

#define	GBM_ERR_XBM_EXP_ID	((GBM_ERR) 1300)
#define	GBM_ERR_XBM_UNEXP_EOF	((GBM_ERR) 1301)
#define	GBM_ERR_XBM_EXP_CHAR	((GBM_ERR) 1302)
#define	GBM_ERR_XBM_EXP_LSQR	((GBM_ERR) 1303)
#define	GBM_ERR_XBM_EXP_RSQR	((GBM_ERR) 1304)
#define	GBM_ERR_XBM_EXP_EQUALS	((GBM_ERR) 1305)
#define	GBM_ERR_XBM_EXP_LCUR	((GBM_ERR) 1306)
#define	GBM_ERR_XBM_EXP_RCUR	((GBM_ERR) 1307)
#define	GBM_ERR_XBM_EXP_COMMA	((GBM_ERR) 1308)
#define	GBM_ERR_XBM_EXP_NUMBER	((GBM_ERR) 1309)
#define	GBM_ERR_XBM_EXP_SEMI	((GBM_ERR) 1310)

#define	MAX_BUF	1024
#define	MAX_ID	100

typedef struct
	{
	int fd;
	int inx, cnt;
	gbm_u8 buf[MAX_BUF];
	char id[MAX_ID+1];
	int number;
	int size;
	} XBM_PRIV;

/*...srev:0:*/
static gbm_u8 rev[0x100] =
	{
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
	};
/*...e*/
/*...sbuffered text read:0:*/
static int nextbuf(XBM_PRIV *xbm_priv)
	{
	int cnt;
	if ( (cnt = gbm_file_read(xbm_priv->fd, xbm_priv->buf, MAX_BUF)) <= 0 )
		return -1;
	xbm_priv->cnt = cnt;
	xbm_priv->inx = 1;
	return xbm_priv->buf[0];
	}

#define	nextchar(x) ( ((x)->inx<(x)->cnt) ? \
	(int) (unsigned int) (unsigned char) ((x)->buf[((x)->inx)++]) : nextbuf(x) )
		/* This is worded to char (char)255 does not become (int)-1 */

static void pushchar(XBM_PRIV *xbm_priv, int c)
	{
	if ( c != -1 )
		xbm_priv->buf[--(xbm_priv->inx)] = (gbm_u8) c;
	}

typedef gbm_u8 SYM;
#define	S_DEFINE	((SYM)  1)
#define	S_ID		((SYM)  2)
#define	S_NUMBER	((SYM)  3)
#define	S_STATIC	((SYM)  4)
#define	S_CHAR		((SYM)  5)
#define	S_SHORT		((SYM)  6)
#define	S_LSQR		((SYM)  7)
#define	S_RSQR		((SYM)  8)
#define	S_LCUR		((SYM)  9)
#define	S_RCUR		((SYM) 10)
#define	S_COMMA		((SYM) 11)
#define	S_SEMI		((SYM) 12)
#define	S_EQUALS	((SYM) 13)
#define	S_EOF		((SYM) 14)
#define	S_ERROR_ID	((SYM) 15)
#define	S_ERROR_MINUS	((SYM) 16)
#define	S_ERROR_PLUS	((SYM) 17)
#define	S_ERROR_NUMBER	((SYM) 18)
#define	S_UNSIGNED	((SYM) 19)
#define	S_SIGNED	((SYM) 20)

/*...svalof:0:*/
static int valof(char c)
	{
	if ( c >= 'a' && c <= 'f' )
		return c - 'a' + 10;
	if ( c >= 'A' && c <= 'F' )
		return c - 'A' + 10;
	return c - '0';
	}
/*...e*/
/*...sskipit:0:*/
/* Skip whitespace and comments,
   returning -1 or the first interesting character. */

static int skipit(XBM_PRIV *xbm_priv)
	{
	int c;
	for ( ;; )
		{
		while ( (c = nextchar(xbm_priv)) != -1 && isspace(c) )
			;
		if ( c == -1 )
			return -1;

		if ( c != '/' )
			break;

		if ( (c = nextchar(xbm_priv)) != '*' )
			return -1;

		if ( (c = nextchar(xbm_priv)) == -1 )
			return -1;

		do
			while ( c != '*' )
				if ( (c = nextchar(xbm_priv)) == -1 )
					return S_EOF;
		while ( (c = nextchar(xbm_priv)) != -1 && c != '/' );
		if ( c == -1 )
			return -1;
		}
	return c;
	}
/*...e*/

static SYM nextsym(XBM_PRIV *xbm_priv)
	{
	int c, i, sign = 1;

	if ( (c = skipit(xbm_priv)) == -1 )
		return S_EOF;

	switch ( c )
		{
		case '[':	return S_LSQR  ;
		case ']':	return S_RSQR  ;
		case '{':	return S_LCUR  ;
		case '}':	return S_RCUR  ;
		case ',':	return S_COMMA ;
		case ';':	return S_SEMI  ;
		case '=':	return S_EQUALS;
		}

	if ( isdigit(c) || c == '-' || c == '+' )
		{
		if ( c == '-' )
			{
			sign = -1;
			if ( (c = nextchar(xbm_priv)) == -1 )
				return S_ERROR_MINUS;
			if ( !isdigit(c) )
				return S_ERROR_NUMBER;
			}
		else if ( c == '+' )
			{
			if ( (c = nextchar(xbm_priv)) == -1 )
				return S_ERROR_PLUS;
			if ( !isdigit(c) )
				return S_ERROR_NUMBER;
			}
		if ( c == '0' )
			{
			xbm_priv->number = 0;
			if ( (c = nextchar(xbm_priv)) == 'x' || c == 'X' )
				/* Hex number */
				{
				while ( (c = nextchar(xbm_priv)) != -1 && isxdigit(c) )
					{
					xbm_priv->number <<= 4;
					xbm_priv->number += valof((char) c);
					}
				}
			else
				/* Octal number */
				while ( c != -1 && c >= '0' && c <= '7' )
					{
					xbm_priv->number <<= 3;
					xbm_priv->number += ( c - '0' );
					c = nextchar(xbm_priv);
					}
			}
		else
			{
			xbm_priv->number = ( c - '0' );
			while ( (c = nextchar(xbm_priv)) != -1 && isdigit(c) )
				{
				xbm_priv->number *= 10;
				xbm_priv->number += ( c - '0' );
				}
			}

		xbm_priv->number *= sign;

		pushchar(xbm_priv, c);
		return S_NUMBER;
		}

	i = 0;
	do
		xbm_priv->id[i++] = c;
	while ( (c = nextchar(xbm_priv)) != -1 && (isalnum(c) || c == '_') && i < MAX_ID );
	xbm_priv->id[i] = '\0';

	pushchar(xbm_priv, c);

	if ( !strcmp(xbm_priv->id, "#define" ) ) return S_DEFINE  ;
	if ( !strcmp(xbm_priv->id, "static"  ) ) return S_STATIC  ;
	if ( !strcmp(xbm_priv->id, "char"    ) ) return S_CHAR    ;
	if ( !strcmp(xbm_priv->id, "short"   ) ) return S_SHORT   ;
	if ( !strcmp(xbm_priv->id, "unsigned") ) return S_UNSIGNED;
	if ( !strcmp(xbm_priv->id, "signed"  ) ) return S_SIGNED  ;

	if ( !isalnum(xbm_priv->id[0]) )
		return S_ERROR_ID;

	return S_ID;
	}

static SYM nextid(XBM_PRIV *xbm_priv)
	{
	int c, i;

	if ( (c = skipit(xbm_priv)) == -1 )
		return S_EOF;

	i = 0;
	do
		xbm_priv->id[i++] = c;
	while ( (c = nextchar(xbm_priv)) != -1 && !isspace(c) && c != '[' && i < MAX_ID );

	xbm_priv->id[i] = '\0';

	pushchar(xbm_priv, c);

	return S_ID;
	}
/*...e*/

/*...sxbm_qft:0:*/
GBM_ERR xbm_qft(GBMFT *gbmft)
	{
	*gbmft = xbm_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...sxbm_rhdr:0:*/
GBM_ERR xbm_rhdr(char *fn, int fd, GBM *gbm, char *opt)
	{
	XBM_PRIV *xbm_priv = (XBM_PRIV *) gbm->priv;
	SYM sym;

	fn=fn; opt=opt; /* Suppress 'unref arg' compiler warnings */

	xbm_priv->fd  = fd;
	xbm_priv->inx = 0;
	xbm_priv->cnt = 0;

	gbm->w   = -1;
	gbm->h   = -1;
	gbm->bpp = 1;

	while ( (sym = nextsym(xbm_priv)) == S_DEFINE )
		{
		char *p;

		if ( (sym = nextid(xbm_priv)) != S_ID )
			return GBM_ERR_XBM_EXP_ID;
		if ( (p = strrchr(xbm_priv->id, '_')) != NULL )
			p++;
		else
			p = (char *) (xbm_priv->id);

		if ( !strcmp(p, "width") )
			{
			if ( (sym = nextsym(xbm_priv)) != S_NUMBER )
				return GBM_ERR_XBM_EXP_NUMBER;
			gbm->w = xbm_priv->number;
			}
		else if ( !strcmp(p, "height") )
			{
			if ( (sym = nextsym(xbm_priv)) != S_NUMBER )
				return GBM_ERR_XBM_EXP_NUMBER;
			gbm->h = xbm_priv->number;
			}
		else
			{
			if ( (sym = nextsym(xbm_priv)) != S_NUMBER )
				return GBM_ERR_XBM_EXP_NUMBER;
			}

		}

	if ( gbm->w == -1 || gbm->h == -1 )
		return GBM_ERR_BAD_SIZE;

	if ( sym == S_STATIC )
		sym = nextsym(xbm_priv);

	if ( sym == S_EOF )
		return GBM_ERR_XBM_UNEXP_EOF;

	if ( sym == S_UNSIGNED )
		sym = nextsym(xbm_priv);

	if ( sym == S_SIGNED )
		sym = nextsym(xbm_priv);

	if ( sym == S_EOF )
		return GBM_ERR_XBM_UNEXP_EOF;

	switch ( sym )
		{
		case S_CHAR:
			xbm_priv->size = 8;
			break;
		case S_SHORT:
			xbm_priv->size = 16;
			break;
		default:
			return GBM_ERR_XBM_EXP_CHAR;
		}

	if ( nextid(xbm_priv) != S_ID )
		return GBM_ERR_XBM_EXP_ID;

	if ( nextsym(xbm_priv) != S_LSQR )
		return GBM_ERR_XBM_EXP_LSQR;

	if ( nextsym(xbm_priv) != S_RSQR )
		return GBM_ERR_XBM_EXP_RSQR;

	if ( nextsym(xbm_priv) != S_EQUALS )
		return GBM_ERR_XBM_EXP_EQUALS;

	if ( nextsym(xbm_priv) != S_LCUR )
		return GBM_ERR_XBM_EXP_LCUR;

	return GBM_ERR_OK;
	}
/*...e*/
/*...sxbm_rpal:0:*/
GBM_ERR xbm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	fd=fd; gbm=gbm; /* Suppress 'unref arg' compiler warnings */

	gbmrgb[0].r = 0xff;
	gbmrgb[0].g = 0xff;
	gbmrgb[0].b = 0xff; /* White background */
	gbmrgb[1].r = 0x00;
	gbmrgb[1].g = 0x00;
	gbmrgb[1].b = 0x00; /* Black background */
	return GBM_ERR_OK;
	}
/*...e*/
/*...sxbm_rdata:0:*/
GBM_ERR xbm_rdata(int fd, GBM *gbm, gbm_u8 *data)
	{
	XBM_PRIV *xbm_priv = (XBM_PRIV *) gbm->priv;
	int stride = ( ( gbm->w * gbm->bpp + 31 ) / 32 ) * 4;
	int x, y;
	SYM sym;
	gbm_boolean keep_going = GBM_TRUE;

	fd=fd; /* Suppres 'unref arg' compiler warning */

	memset(data, 0, gbm->h * stride);
	data += ( (gbm->h - 1) * stride );

	switch ( xbm_priv->size )
		{
/*...s8:16:*/
case 8:
	for ( y = gbm->h - 1; keep_going && y >= 0; y--, data -= stride )
		for ( x = 0; keep_going && x < (int) ((unsigned)(gbm->w + 7) >> 3); x++ )
			if ( (sym = nextsym(xbm_priv)) == S_RCUR )
				keep_going = GBM_FALSE;
			else if ( sym != S_NUMBER )
				return GBM_ERR_XBM_EXP_NUMBER;
			else
				{
				data[x] = rev[xbm_priv->number];

				sym = nextsym(xbm_priv);
				if ( sym == S_RCUR )
					keep_going = GBM_FALSE;
				else if ( sym != S_COMMA )
					return GBM_ERR_XBM_EXP_COMMA;
				}
	break;
/*...e*/
/*...s16:16:*/
case 16:
	for ( y = gbm->h - 1; keep_going && y >= 0; y--, data -= stride )
		for ( x = 0; keep_going && x < (int) ((unsigned)(gbm->w + 15) >> 4); x++ )
			if ( (sym = nextsym(xbm_priv)) == S_RCUR )
				keep_going = GBM_FALSE;
			else if ( sym != S_NUMBER )
				return GBM_ERR_XBM_EXP_NUMBER;
			else
				{
				data[x * 2    ] = rev[          xbm_priv->number & 0xffU];
				data[x * 2 + 1] = rev[(unsigned)xbm_priv->number >> 8   ];

				sym = nextsym(xbm_priv);
				if ( sym == S_RCUR )
					keep_going = GBM_FALSE;
				else if ( sym != S_COMMA )
					return GBM_ERR_XBM_EXP_COMMA;
				}
	break;
/*...e*/
		}

	if ( keep_going )
		if ( nextsym(xbm_priv) != S_RCUR )
			return GBM_ERR_XBM_EXP_RCUR;

	if ( nextsym(xbm_priv) != S_SEMI )
		return GBM_ERR_XBM_EXP_SEMI;

	return GBM_ERR_OK;
	}
/*...e*/
/*...sxbm_w:0:*/
/*
Write darkest colour as 1s, lightest colour with 0s.
*/

GBM_ERR xbm_w(char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, char *opt)
	{
	int stride = ((gbm->w * gbm->bpp + 31) / 32) * 4;
	int x, y, col = 0;
	char s[100+1], name[100+1], *dot, *p;
	int k0, k1, xor;

	opt=opt; /* Suppress 'unref arg' compiler warning */

	/* Normally palette entry 0 brightest is usual */
	/* If not reverse all the bits */

	k0 = gbmrgb[0].r * 77 + gbmrgb[0].g * 150 + gbmrgb[0].b * 29;
	k1 = gbmrgb[1].r * 77 + gbmrgb[1].g * 150 + gbmrgb[1].b * 29;

	xor = ( k0 > k1 ) ? 0 : 0xff;

	/* Make the name from the filename, without the extension */

	strncpy(name, fn, 100);
	if ( (dot = strrchr(name, '.')) != NULL )
		*dot = '\0';

	/* Nobble any potentially nasty characters */

	if ( !isalpha(*name) )
		*name = '_';
	for ( p = name + 1; *p; p++ )
		if ( !isalnum(*p) )
			*p = '_';

	sprintf(s, "#define %s_width %d\r\n" , name, gbm->w);
	if ( (size_t) gbm_file_write(fd, s, strlen(s)) != strlen(s) )
		return GBM_ERR_WRITE;
	sprintf(s, "#define %s_height %d\r\n", name, gbm->h);
	if ( (size_t) gbm_file_write(fd, s, strlen(s)) != strlen(s) )
		return GBM_ERR_WRITE;
	sprintf(s, "static char %s_bits[] = {\r\n", name);
	if ( (size_t) gbm_file_write(fd, s, strlen(s)) != strlen(s) )
		return GBM_ERR_WRITE;

	data += (gbm->h - 1) * stride;
	for ( y = gbm->h - 1; y >= 0; y--, data -= stride )
		for ( x = 0; x < (int) ((unsigned)(gbm->w + 7) >> 3); x++ )
			{
			sprintf(s, "0x%02x,", rev[data[x]] ^ xor);
			col += 5;
			if ( col > 70 )
				{
				strcat(s, "\r\n");
				col = 0;
				}
			if ( (size_t) gbm_file_write(fd, s, strlen(s)) != strlen(s) )
				return GBM_ERR_WRITE;
			}

	if ( col )
		{
		if ( gbm_file_write(fd, "\r\n", 2) != 2 )
			return GBM_ERR_WRITE;
		}		

	if ( gbm_file_write(fd, "};\r\n", 4) != 4 )
		return GBM_ERR_WRITE;

	return GBM_ERR_OK;
	}
/*...e*/
/*...sxbm_err:0:*/
const char *xbm_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_XBM_EXP_ID:
			return "expected identifier";
		case GBM_ERR_XBM_UNEXP_EOF:
			return "unexpected end of file";
		case GBM_ERR_XBM_EXP_CHAR:
			return "expected char or short";
		case GBM_ERR_XBM_EXP_LSQR:
			return "expected[";
		case GBM_ERR_XBM_EXP_RSQR:
			return "expected ]";
		case GBM_ERR_XBM_EXP_EQUALS:
			return "expected =";
		case GBM_ERR_XBM_EXP_LCUR:
			return "expected {";
		case GBM_ERR_XBM_EXP_RCUR:
			return "expected }";
		case GBM_ERR_XBM_EXP_COMMA:
			return "expected ,";
		case GBM_ERR_XBM_EXP_NUMBER:
			return "expected number";
		case GBM_ERR_XBM_EXP_SEMI:
			return "expected ;";
		}
	return NULL;
	}
/*...e*/
