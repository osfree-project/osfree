/*

gbmkps.c - IBM KIPS support

Reads array as 8 bit palettised colour.
Writes 8 bit palettised colour.
Input options: pal,kpl (default: pal)
Output options: pal,kpl (default: pal)

History:
--------
(Heiko Nitzsche)

22-Feb-2006: Move format description strings to gbmdesc.h

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

/*...suseful:0:*/
#define	low_byte(w)	((byte)  (          (w)&0x00ffU)    )
#define	high_byte(w)	((byte) (((unsigned)(w)&0xff00U)>>8))
#define	make_word(a,b)	(((word)a) + (((word)b) << 8))
/*...e*/
/*...sextension:0:*/
static char *extension(char *fn)
	{
	char	*dot, *slash;

	if ( (dot = strrchr(fn, '.')) == NULL )
		return NULL;

	if ( (slash = strpbrk(fn, "/\\")) == NULL )
		return dot + 1;

	return ( slash < dot ) ? dot + 1 : NULL;
	}
/*...e*/

static GBMFT kps_gbmft =
	{
        GBM_FMT_DESC_SHORT_KPS,
        GBM_FMT_DESC_LONG_KPS,
        GBM_FMT_DESC_EXT_KPS,
	GBM_FT_R8|
	GBM_FT_W8,
	};

#define	GBM_ERR_KPS_OPEN	((GBM_ERR) 500)
#define	GBM_ERR_KPS_CREATE	((GBM_ERR) 501)

/*...skps file header definition:0:*/
/*
This defines the 32 byte header found on .KPS and .KPL files.
*/

#define	KPS_SIGNITURE	"DFIMAG00"

typedef struct
	{
	byte	signiture[8];		/* Usually "DFIMAG00"                */
	byte	height_low;	
	byte	height_high;		/* Image height in pixels            */
	byte	width_low;	
	byte	width_high;		/* Image width in pixels             */
	byte	unknown[20];		/* 20 unknown bytes                  */
	} KPS_HEADER;
/*...e*/

typedef struct
	{
	char fn[600+1];
	BOOLEAN kpl;
	} KPS_PRIV;

/*...skps_qft:0:*/
GBM_ERR kps_qft(GBMFT *gbmft)
	{
	*gbmft = kps_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...skps_rhdr:0:*/
GBM_ERR kps_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	BOOLEAN	pal = ( gbm_find_word(opt, "pal") != NULL );
	BOOLEAN	kpl = ( gbm_find_word(opt, "kpl") != NULL );
	KPS_HEADER kps_header;
	KPS_PRIV *priv = (KPS_PRIV *) gbm->priv;
	int	w, h;

	if ( kpl && pal )
		return GBM_ERR_BAD_OPTION;

	gbm_file_read(fd, (char *) &kps_header, sizeof(KPS_HEADER));

	if ( memcmp(kps_header.signiture, KPS_SIGNITURE, strlen(KPS_SIGNITURE)) )
		return GBM_ERR_BAD_MAGIC;

	w = make_word(kps_header.width_low , kps_header.width_high );
	h = make_word(kps_header.height_low, kps_header.height_high);

	if ( w <= 0 || h <= 0 )
		return GBM_ERR_BAD_SIZE;

	gbm->w   = w;
	gbm->h   = h;
	gbm->bpp = 8;

	/* Keep these for a later kps_rpal() call */

	strcpy(priv->fn, fn);
	priv->kpl = kpl;

	return GBM_ERR_OK;
	}
/*...e*/
/*...skps_rpal:0:*/
GBM_ERR kps_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	KPS_PRIV *priv = (KPS_PRIV *) gbm->priv;
	char	fn2[600+1], *ext;

	fd=fd; /* Suppress 'unref arg' compiler warning */

	strcpy(fn2, priv->fn);
	ext = extension(fn2);
	if ( priv->kpl )
/*...sread a \46\kpl palette file:16:*/
{
int	fd2, i, w, h;
byte	p[3][0x100];
KPS_HEADER kps_header;

if ( ext != NULL )
	strcpy(ext, "kpl");
else
	strcat(fn2, ".kpl");

if ( (fd2 = gbm_file_open_from_codec(fn2, GBM_O_RDONLY, fd)) == -1 )
	return GBM_ERR_KPS_OPEN;

gbm_file_read(fd2, (char *) &kps_header, sizeof(KPS_HEADER));
if ( memcmp(kps_header.signiture, KPS_SIGNITURE, strlen(KPS_SIGNITURE)) )
	return GBM_ERR_BAD_MAGIC;

w = make_word(kps_header.width_low , kps_header.width_high );
h = make_word(kps_header.height_low, kps_header.height_high);

if ( w != 0x100 || h != 3 )
	return GBM_ERR_BAD_SIZE;

gbm_file_read(fd2, &(p[0][0]), 0x300);
gbm_file_close(fd2);

for ( i = 0; i < 0x100; i++ )
	{
	gbmrgb[i].r = p[0][i];
	gbmrgb[i].b = p[1][i];
	gbmrgb[i].g = p[2][i];
	}
}
/*...e*/
	else
/*...sread a \46\pal palette file:16:*/
{
int	fd2, i;
byte	b[4];

if ( ext != NULL )
	strcpy(ext, "pal");
else
	strcat(fn2, ".pal");

if ( (fd2 = gbm_file_open_from_codec(fn2, GBM_O_RDONLY, fd)) == -1 )
	return GBM_ERR_KPS_OPEN;

for ( i = 0; i < 0x100; i++ )
	{
	gbm_file_read(fd2, (char *) b, 4);
	gbmrgb[i].r = b[0];
	gbmrgb[i].b = b[1];
	gbmrgb[i].g = b[2];
	}
gbm_file_close(fd2);
}
/*...e*/

	return GBM_ERR_OK;
	}
/*...e*/
/*...skps_rdata:0:*/
GBM_ERR kps_rdata(int fd, GBM *gbm, byte *data)
	{
	int	i, stride;
	byte	*p;

	stride = ((gbm->w + 3) & ~3);
	p = data + ((gbm->h - 1) * stride);
	for ( i = gbm->h - 1; i >= 0; i-- )
		{
		gbm_file_read(fd, p, gbm->w);
		p -= stride;
		}

	return GBM_ERR_OK;
	}
/*...e*/
/*...skps_w:0:*/
GBM_ERR kps_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt)
	{
	KPS_HEADER kps_header;
	int i, stride;
	const byte *p;
	char fn2[600+1], *ext;
	BOOLEAN	pal = ( gbm_find_word(opt, "pal") != NULL );
	BOOLEAN	kpl = ( gbm_find_word(opt, "kpl") != NULL );

	if ( gbm->bpp != 8 )
		return GBM_ERR_NOT_SUPP;

	if ( pal && kpl )
		return GBM_ERR_BAD_OPTION;

	memcpy(kps_header.signiture, KPS_SIGNITURE, strlen(KPS_SIGNITURE));
	kps_header.width_low   = low_byte(gbm->w);
	kps_header.width_high  = high_byte(gbm->w);
	kps_header.height_low  = low_byte(gbm->h);
	kps_header.height_high = high_byte(gbm->h);
	kps_header.unknown[0] = 1;
	memset(&kps_header.unknown[1], 0, 19);
	gbm_file_write(fd, (char *) &kps_header, sizeof(KPS_HEADER));

	stride = ((gbm->w + 3) & ~3);
	p = data + ((gbm->h - 1) * stride);
	for ( i = gbm->h - 1; i >= 0; i-- )
		{
		gbm_file_write(fd, p, gbm->w);
		p -= stride;
		}

	strcpy(fn2, fn);
	ext = extension(fn2);
	if ( kpl )
/*...swrite a \46\kpl palette file:16:*/
{
int fd2, j;
byte palette[3][0x100];

if ( ext != NULL )
	strcpy(ext, "kpl");
else
	strcat(fn2, ".kpl");

if ( (fd2 = gbm_file_create_from_codec(fn2, GBM_O_WRONLY, fd)) == -1 )
	return GBM_ERR_KPS_CREATE;

kps_header.width_low   = low_byte(0x100);
kps_header.width_high  = high_byte(0x100);
kps_header.height_low  = low_byte(3);
kps_header.height_high = high_byte(3);
gbm_file_write(fd2, (char *) &kps_header, sizeof(KPS_HEADER));

for ( j = 0; j < 0x100; j++ )
	{
	palette[0][j] = gbmrgb[j].r;
	palette[1][j] = gbmrgb[j].b;
	palette[2][j] = gbmrgb[j].g;
	}

gbm_file_write(fd2, &(palette[0][0]), 0x300);
gbm_file_close(fd2);
}
/*...e*/
	else
/*...swrite a \46\pal palette file:16:*/
{
int	fd2;
byte	b[4];

if ( ext != NULL )
	strcpy(ext, "pal");
else
	strcat(fn2, ".pal");

if ( (fd2 = gbm_file_create_from_codec(fn2, GBM_O_WRONLY, fd)) == -1 )
	return GBM_ERR_KPS_CREATE;

b[3] = 0;
for ( i = 0; i < 0x100; i++ )
	{
	b[0] = gbmrgb[i].r;
	b[1] = gbmrgb[i].b;
	b[2] = gbmrgb[i].g;
	gbm_file_write(fd2, (char *) b, 4);
	}
gbm_file_close(fd2);
}
/*...e*/

	return GBM_ERR_OK;
	}
/*...e*/
/*...skps_err:0:*/
const char *kps_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_KPS_OPEN:
			return "can't open complementary palette file";
		case GBM_ERR_KPS_CREATE:
			return "can't create complementary palette file";
		}
	return NULL;
	}
/*...e*/
