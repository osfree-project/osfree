/*

gbmspr.c - Archimedes Sprite from RiscOS Format support

In Archimedes terminology, a sprite is a bitmap with an optional mask plane.

Reads a sprite from file created by *ScreenSave or *SSave command.
Will also write such a file containing a single sprite.

Input options: index=# (default: 0)

History:
--------
(Heiko Nitzsche)

19-Feb-2006: Add function to query number of images
22-Feb-2006: Move format description strings to gbmdesc.h
28-Sep-2006: Attempt to protect the format detection better against wrong interpretations.
             Number of pages is now restricted to 10000 due to this.
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
#include "gbmmem.h"

/*...vgbm\46\h:0:*/
/*...vgbmhelp\46\h:0:*/

#ifndef min
#define	min(a,b)	(((a)<(b))?(a):(b))
#endif
/*...e*/
/*...suseful:0:*/
#define	low_byte(w)	((gbm_u8)  ((w)&0x00ff)    )
#define	high_byte(w)	((gbm_u8) (((w)&0xff00)>>8))
#define	make_word(a,b)	(((gbm_u16)a) + (((gbm_u16)b) << 8))

/*...sread_word:0:*/
static gbm_boolean read_word(int fd, gbm_u16 *w)
	{
	gbm_u8 low = 0, high = 0;

	gbm_file_read(fd, (char *) &low, 1);
	gbm_file_read(fd, (char *) &high, 1);
	*w = (gbm_u16) (low + ((gbm_u16) high << 8));
	return GBM_TRUE;
	}
/*...e*/
/*...sread_dword:0:*/
static gbm_boolean read_dword(int fd, gbm_u32 *d)
	{
	gbm_u16 low, high;

	read_word(fd, &low);
	read_word(fd, &high);
	*d = low + ((gbm_u32) high << 16);
	return GBM_TRUE;
	}
/*...e*/
/*...swrite_word:0:*/
static gbm_boolean write_word(int fd, gbm_u16 w)
	{
	gbm_u8 low  = (gbm_u8) w;
	gbm_u8 high = (gbm_u8) (w >> 8);

	return gbm_file_write(fd, &low, 1) == 1 && gbm_file_write(fd, &high, 1) == 1;
	}
/*...e*/
/*...swrite_dword:0:*/
static gbm_boolean write_dword(int fd, gbm_u32 d)
	{
	return write_word(fd, (gbm_u16) d) && write_word(fd, (gbm_u16) (d >> 16));
	}
/*...e*/
/*...e*/

static GBMFT spr_gbmft =
	{
        GBM_FMT_DESC_SHORT_SPR,
        GBM_FMT_DESC_LONG_SPR,
        GBM_FMT_DESC_EXT_SPR,
	GBM_FT_R1|GBM_FT_R4|GBM_FT_R8|
	GBM_FT_W1|GBM_FT_W4|GBM_FT_W8,
	};

#define	GBM_ERR_SPR_FIRST	((GBM_ERR) 1400)
#define	GBM_ERR_SPR_MODE	((GBM_ERR) 1401)
#define	GBM_ERR_SPR_OFFSET	((GBM_ERR) 1402)
#define	GBM_ERR_SPR_PAL8	((GBM_ERR) 1403)

typedef struct
	{
	long pos_palette, pos_image, pos_mask;
	gbm_u32 bytes_per_line, first_bit, last_bit, actual_bpp;
	} SPR_PRIV;

/*...sbpp_of_mode\44\ mode_of_bpp:0:*/
static int bpp_of_mode[] =
	{
	1,	/*  0: 640x256 */
	2,	/*  1: 320x256 */
	4,	/*  2: 160x256 */
	-1,	/*  3: Text only */
	1,	/*  4: 320x256 */
	2,	/*  5: 160x256 */
	2,	/*  6: 160x256 */
	-1,	/*  7: Teletext */
	2,	/*  8: 640x256 */
	4,	/*  9: 320x256 */
	8,	/* 10: 160x256 */
	2,	/* 11: 640x250 */
	4,	/* 12: 640x256 */
	8,	/* 13: 320x256 */
	4,	/* 14: 640x250 */
	8,	/* 15: 640x256 */
	4,	/* 16: 1056x250 */
	4,	/* 17: 1056x256 */
	1,	/* 18: 640x512 multisync-monitor */
	2,	/* 19: 640x512 multisync-monitor */
	4,	/* 20: 640x512 multisync-monitor */
	8,	/* 21: 640x512 multisync-monitor */
	-1,	/* 22: ? */
	1,	/* 23: 1152x896 61.2Hz-hires-montor */
	8,	/* 24: 1056x256 */
	1,	/* 25: 640x480 multisync-or-60Hz-VGA-monitor */
	2,	/* 26: 640x480 multisync-or-60Hz-VGA-monitor */
	4,	/* 27: 640x480 multisync-or-60Hz-VGA-monitor */
	8,	/* 28: 640x480 multisync-or-60Hz-VGA-monitor */
	};

#define	N_MODES	29

/* Return highest resolution mode for given bits per pixel. */

static int mode_of_bpp[] = { -1,23,-1,-1,17,-1,-1,-1,24 };
/*...e*/
/*...squick tables:0:*/
/* These are to account for the reverse ordering of pixels in a scan line. */

static gbm_u8 nibble_swap[0x100] =
	{
	0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,
	0x80,0x90,0xa0,0xb0,0xc0,0xd0,0xe0,0xf0,
	0x01,0x11,0x21,0x31,0x41,0x51,0x61,0x71,
	0x81,0x91,0xa1,0xb1,0xc1,0xd1,0xe1,0xf1,
	0x02,0x12,0x22,0x32,0x42,0x52,0x62,0x72,
	0x82,0x92,0xa2,0xb2,0xc2,0xd2,0xe2,0xf2,
	0x03,0x13,0x23,0x33,0x43,0x53,0x63,0x73,
	0x83,0x93,0xa3,0xb3,0xc3,0xd3,0xe3,0xf3,
	0x04,0x14,0x24,0x34,0x44,0x54,0x64,0x74,
	0x84,0x94,0xa4,0xb4,0xc4,0xd4,0xe4,0xf4,
	0x05,0x15,0x25,0x35,0x45,0x55,0x65,0x75,
	0x85,0x95,0xa5,0xb5,0xc5,0xd5,0xe5,0xf5,
	0x06,0x16,0x26,0x36,0x46,0x56,0x66,0x76,
	0x86,0x96,0xa6,0xb6,0xc6,0xd6,0xe6,0xf6,
	0x07,0x17,0x27,0x37,0x47,0x57,0x67,0x77,
	0x87,0x97,0xa7,0xb7,0xc7,0xd7,0xe7,0xf7,
	0x08,0x18,0x28,0x38,0x48,0x58,0x68,0x78,
	0x88,0x98,0xa8,0xb8,0xc8,0xd8,0xe8,0xf8,
	0x09,0x19,0x29,0x39,0x49,0x59,0x69,0x79,
	0x89,0x99,0xa9,0xb9,0xc9,0xd9,0xe9,0xf9,
	0x0a,0x1a,0x2a,0x3a,0x4a,0x5a,0x6a,0x7a,
	0x8a,0x9a,0xaa,0xba,0xca,0xda,0xea,0xfa,
	0x0b,0x1b,0x2b,0x3b,0x4b,0x5b,0x6b,0x7b,
	0x8b,0x9b,0xab,0xbb,0xcb,0xdb,0xeb,0xfb,
	0x0c,0x1c,0x2c,0x3c,0x4c,0x5c,0x6c,0x7c,
	0x8c,0x9c,0xac,0xbc,0xcc,0xdc,0xec,0xfc,
	0x0d,0x1d,0x2d,0x3d,0x4d,0x5d,0x6d,0x7d,
	0x8d,0x9d,0xad,0xbd,0xcd,0xdd,0xed,0xfd,
	0x0e,0x1e,0x2e,0x3e,0x4e,0x5e,0x6e,0x7e,
	0x8e,0x9e,0xae,0xbe,0xce,0xde,0xee,0xfe,
	0x0f,0x1f,0x2f,0x3f,0x4f,0x5f,0x6f,0x7f,
	0x8f,0x9f,0xaf,0xbf,0xcf,0xdf,0xef,0xff,
	};
static gbm_u8 bit_swap[0x100] =
	{
	0x00,0x80,0x40,0xc0,0x20,0xa0,0x60,0xe0,
	0x10,0x90,0x50,0xd0,0x30,0xb0,0x70,0xf0,
	0x08,0x88,0x48,0xc8,0x28,0xa8,0x68,0xe8,
	0x18,0x98,0x58,0xd8,0x38,0xb8,0x78,0xf8,
	0x04,0x84,0x44,0xc4,0x24,0xa4,0x64,0xe4,
	0x14,0x94,0x54,0xd4,0x34,0xb4,0x74,0xf4,
	0x0c,0x8c,0x4c,0xcc,0x2c,0xac,0x6c,0xec,
	0x1c,0x9c,0x5c,0xdc,0x3c,0xbc,0x7c,0xfc,
	0x02,0x82,0x42,0xc2,0x22,0xa2,0x62,0xe2,
	0x12,0x92,0x52,0xd2,0x32,0xb2,0x72,0xf2,
	0x0a,0x8a,0x4a,0xca,0x2a,0xaa,0x6a,0xea,
	0x1a,0x9a,0x5a,0xda,0x3a,0xba,0x7a,0xfa,
	0x06,0x86,0x46,0xc6,0x26,0xa6,0x66,0xe6,
	0x16,0x96,0x56,0xd6,0x36,0xb6,0x76,0xf6,
	0x0e,0x8e,0x4e,0xce,0x2e,0xae,0x6e,0xee,
	0x1e,0x9e,0x5e,0xde,0x3e,0xbe,0x7e,0xfe,
	0x01,0x81,0x41,0xc1,0x21,0xa1,0x61,0xe1,
	0x11,0x91,0x51,0xd1,0x31,0xb1,0x71,0xf1,
	0x09,0x89,0x49,0xc9,0x29,0xa9,0x69,0xe9,
	0x19,0x99,0x59,0xd9,0x39,0xb9,0x79,0xf9,
	0x05,0x85,0x45,0xc5,0x25,0xa5,0x65,0xe5,
	0x15,0x95,0x55,0xd5,0x35,0xb5,0x75,0xf5,
	0x0d,0x8d,0x4d,0xcd,0x2d,0xad,0x6d,0xed,
	0x1d,0x9d,0x5d,0xdd,0x3d,0xbd,0x7d,0xfd,
	0x03,0x83,0x43,0xc3,0x23,0xa3,0x63,0xe3,
	0x13,0x93,0x53,0xd3,0x33,0xb3,0x73,0xf3,
	0x0b,0x8b,0x4b,0xcb,0x2b,0xab,0x6b,0xeb,
	0x1b,0x9b,0x5b,0xdb,0x3b,0xbb,0x7b,0xfb,
	0x07,0x87,0x47,0xc7,0x27,0xa7,0x67,0xe7,
	0x17,0x97,0x57,0xd7,0x37,0xb7,0x77,0xf7,
	0x0f,0x8f,0x4f,0xcf,0x2f,0xaf,0x6f,0xef,
	0x1f,0x9f,0x5f,0xdf,0x3f,0xbf,0x7f,0xff,
	};

static gbm_u8 pair_swap[0x10] =
	{
	0x00,0x10,0x20,0x30,0x01,0x11,0x21,0x31,
	0x02,0x12,0x22,0x32,0x03,0x13,0x23,0x33,
	};
/*...e*/

/*...sspr_qft:0:*/
GBM_ERR spr_qft(GBMFT *gbmft)
	{
	*gbmft = spr_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/

/* ----------------------------------------------------------- */

/* Read number of directories in the TIFF file. */
GBM_ERR spr_rimgcnt(const char *fn, int fd, int *pimgcnt)
{
    gbm_u32 num_sprites;
    fn=fn; /* Suppress 'unref arg' compiler warnings */

    read_dword(fd, &num_sprites);

    /* check plausibility as far as possible: restrict to 10000 images */
    if (num_sprites > 10000)
    {
        return GBM_ERR_SPR_MODE;
    }

    *pimgcnt = (int) num_sprites;
    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/*...sspr_rhdr:0:*/
GBM_ERR spr_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	SPR_PRIV *priv = (SPR_PRIV *) gbm->priv;
	const char *index;
	int i;
	int num_sprites;
	long pos_sprite;
	gbm_u32 offset_sprite;
	gbm_u32 dword_w, scans_h, first_bit, last_bit, bits_per_line;
	gbm_u32 offset_image, offset_mask, mode;
	GBM_ERR rc;

	fn=fn; /* Suppress 'unref arg' compiler warnings */

	if ( (index = gbm_find_word_prefix(opt, "index=")) != NULL )
		sscanf(index + 6, "%d", &i);
	else
		i = 0;

	rc = spr_rimgcnt(fn, fd, &num_sprites);
	if (rc != GBM_ERR_OK)
		{
		return rc;
		}
	if ( i >= num_sprites )
		{
		return GBM_ERR_SPR_OFFSET;
		}
	read_dword(fd, &offset_sprite);
	pos_sprite = gbm_file_lseek(fd, offset_sprite - 4L, GBM_SEEK_SET);
	if (pos_sprite < 0)
		{
		return GBM_ERR_READ;
		}

	while ( i-- > 0 )
		{
		read_dword(fd, &offset_sprite);
		pos_sprite = gbm_file_lseek(fd, pos_sprite + offset_sprite, GBM_SEEK_SET);
		if (pos_sprite < 0)
			{
			return GBM_ERR_READ;
			}
		}

	gbm_file_lseek(fd, 16, GBM_SEEK_CUR);	/* Skip 4 byte next-ptr + 12 byte name */
	read_dword(fd, &dword_w);	/* Width in dwords - 1 */
	read_dword(fd, &scans_h);	/* Scan lines high - 1 */
	read_dword(fd, &first_bit);	/* First bit used (left end of row) */
	read_dword(fd, &last_bit);	/* Last bit used (right end of row) */
	read_dword(fd, &offset_image);	/* Offset of image data */
	read_dword(fd, &offset_mask);	/* Offset of mask plane or above if none */
	read_dword(fd, &mode);		/* Mode sprite defined in */

	if ( first_bit & 7 )
		return GBM_ERR_SPR_FIRST;
	if ( mode >= N_MODES )
		return GBM_ERR_SPR_MODE;
	if ( (gbm->bpp = priv->actual_bpp = bpp_of_mode[mode]) == -1 )
		return GBM_ERR_SPR_MODE;

	gbm->h = (int) (scans_h+1);
	if ( gbm->h < 0 || gbm->h > 10000 )
		return GBM_ERR_BAD_SIZE;

	if ( gbm->bpp == 2 )
		gbm->bpp = 4;

	bits_per_line = ((dword_w+1) * 32) - first_bit - (32 - (last_bit+1));
	gbm->w = (int) (bits_per_line / priv->actual_bpp);
	if ( gbm->w < 0 || gbm->w > 10000 )
		return GBM_ERR_BAD_SIZE;

	priv->pos_palette    = gbm_file_lseek(fd, 0L, GBM_SEEK_CUR);
	priv->pos_image      = (long) pos_sprite + offset_image;
	priv->pos_mask       = (long) pos_sprite + offset_mask;
	priv->bytes_per_line = (dword_w + 1) * 4;
	priv->first_bit      = first_bit;
	priv->last_bit       = last_bit;

	return GBM_ERR_OK;
	}
/*...e*/
/*...sspr_rpal:0:*/
/*...sread_pal:0:*/
/* Palette entry is 2 dwords, which are same if no flashing */
/* We will simply use first dword in each case */

static gbm_boolean read_pal(int fd, GBMRGB *gbmrgb)
	{
	gbm_u8 pal[8];
	if ( gbm_file_read(fd, pal, 8) != 8 )
		return GBM_FALSE;
	gbmrgb->r = pal[1];
	gbmrgb->g = pal[2];
	gbmrgb->b = pal[3];
	return GBM_TRUE;
	}
/*...e*/

/*...sgbmrgb_1bpp:0:*/
/*
I do not expect the palette to be missing from a file defined in a mode with 2
colours because the Wimp uses 16 colours, and I am led to beleive it is the
only thing that saves files without the palette. However, if I am wrong...
*/

static GBMRGB gbmrgb_1bpp[2] =
	{
	{0xff,0xff,0xff},	/* 0=White */
	{0x00,0x00,0x00},	/* 1=black */
	};
/*...e*/
/*...sgbmrgb_2bpp:0:*/
/*
I do not expect the palette to be missing from a file defined in a mode with 4
colours because the Wimp uses 16 colours, and I am led to beleive it is the
only thing that saves files without the palette. However, if I am wrong...
*/

static GBMRGB gbmrgb_2bpp[4] =
	{
	{0xff,0xff,0xff},
	{0xaa,0xaa,0xaa},
	{0x55,0x55,0x55},
	{0x00,0x00,0x00},
	};
/*...e*/
/*...sgbmrgb_4bpp:0:*/
/*
This is the default Wimp defined 16 colour palette.
The exact r,g,b values are not known so some nice bold examples of each,
according to their descriptions in the RiscOS books have been found manually.
*/

static GBMRGB gbmrgb_4bpp[16] =
	{
	{0xff,0xff,0xff},	/* 0=white */
	{0xdb,0xdb,0xdb},	/* 1=grey */
	{0xb6,0xb6,0xb6},	/* 2=grey */
	{0x92,0x92,0x92},	/* 3=grey */
	{0x6d,0x6d,0x6d},	/* 4=grey */
	{0x49,0x49,0x49},	/* 5=grey */
	{0x24,0x24,0x24},	/* 6=grey */
	{0x00,0x00,0x00},	/* 7=black */
	{0x00,0x00,0xff},	/* 8=dark blue */
	{0xff,0xff,0x00},	/* 9=yellow */
	{0x00,0xff,0x00},	/* a=green */
	{0xff,0x00,0x00},	/* b=red */
	{0xff,0xd8,0xd8},	/* c=cream */
	{0x40,0x80,0x40},	/* d=army green */
	{0xff,0x9c,0x00},	/* e=orange */
	{0x00,0xb9,0xff},	/* f=light blue */
	};
/*...e*/
/*...sexpand_0x10:0:*/
/*
The Archimedes does not have 0x100 palette registers, it has 0x10 VIDC registers.
Given an 8 bit pixel :-
	Bits 3-0 are bits 3-0 of palette index
	Bit 4 overrides red bit 7
	Bit 5 overrides green bit 6
	Bit 6 overrides green bit 7
	Bit 7 overrides blue bit 7
So we duplicate the 0x10 values we have read, and make the others from them.
*/

static void expand_0x10(GBMRGB *gbmrgb)
	{
	int i, bank;

	for ( bank = 0x10; bank < 0x100; bank += 0x10 )
		{
		gbm_u8 override_r = ((bank & 0x10) << 3);
		gbm_u8 override_g = ((bank & 0x60) << 1);
		gbm_u8 override_b =  (bank & 0x80)      ;
		for ( i = 0; i < 0x10; i++ )
			{
			gbmrgb[bank + i].r = ((gbmrgb[i].r & 0x7f) | override_r);
			gbmrgb[bank + i].g = ((gbmrgb[i].g & 0x3f) | override_g);
			gbmrgb[bank + i].b = ((gbmrgb[i].b & 0x7f) | override_b);
			}
		}
	}
/*...e*/
/*...sexpand_0x40:0:*/
/*
The Archimedes does not have 0x100 palette registers, it has 0x10.
Planning for the future, files can be written with 0x40 palette entries.
Given an 8 bit pixel :-
	Bits 5-0 are bits 5-0 of palette index
	Bit 6 overrides green bit 7
	Bit 7 overrides blue bit 7
So we duplicate the 0x40 values we have read, and make the others from them.
Now, although we have seen files with 0x40 entries, when you take the first
0x10, and perform expand_0x10 on them, the result is the same as this routine.
Clearly Acorn are looking forward to a day when the VIDC chip has 0x40
registers.
*/

static void expand_0x40(GBMRGB *gbmrgb)
	{
	int i, bank;

	for ( bank = 0; bank < 0x100; bank += 0x40 )
		{
		gbm_u8 override_g = ((bank & 0x40) << 1);
		gbm_u8 override_b =  (bank & 0x80)      ;
		for ( i = 0; i < 0x40; i++ )
			{
			gbmrgb[bank + i].r =   gbmrgb[i].r;
			gbmrgb[bank + i].g = ((gbmrgb[i].g & 0x7f) | override_g);
			gbmrgb[bank + i].b = ((gbmrgb[i].b & 0x7f) | override_b);
			}
		}
	}
/*...e*/

GBM_ERR spr_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	SPR_PRIV *priv = (SPR_PRIV *) gbm->priv;
	int npal = (priv->pos_image - priv->pos_palette) / 8;

	if ( npal == 0 )
/*...sno palette in file\44\ use the default Wimp one:16:*/
switch ( priv->actual_bpp )
	{
	case 1: memcpy(gbmrgb, gbmrgb_1bpp, sizeof(gbmrgb_1bpp)); break;
	case 2: memcpy(gbmrgb, gbmrgb_2bpp, sizeof(gbmrgb_2bpp)); break;
	case 4: memcpy(gbmrgb, gbmrgb_4bpp, sizeof(gbmrgb_4bpp)); break;
	case 8:
/*...sdefault palette:32:*/
{
int i;
for ( i = 0; i < 0x10; i++ )
	{
	gbm_u8 tint = ((i & 0x03) << 4);
	gbmrgb[i].r = tint + ((i & 0x04) << 4);
	gbmrgb[i].g = tint;
	gbmrgb[i].b = tint + ((i & 0x08) << 3);
	}
expand_0x10(gbmrgb);
}
/*...e*/
		break;
	}

/*...e*/
	else
/*...sread palette from disk:16:*/
{
int i;
gbm_file_lseek(fd, priv->pos_palette, GBM_SEEK_SET);
for ( i = 0; i < npal; i++ )
	if ( !read_pal(fd, gbmrgb + i) )
		return GBM_ERR_READ;

if ( gbm->bpp == 8 )
	/* Handle getting too few palette entries */
	{
	if ( npal == 0x40 )
		expand_0x40(gbmrgb);
	else if ( npal == 0x10 )
		expand_0x10(gbmrgb);
	else
		return GBM_ERR_SPR_PAL8;
	}
}
/*...e*/

	return GBM_ERR_OK;
	}
/*...e*/
/*...sspr_rdata:0:*/
GBM_ERR spr_rdata(int fd, GBM *gbm, gbm_u8 *data)
	{
	int stride = ((gbm->bpp * gbm->w + 31) / 32) * 4;
	SPR_PRIV *priv = (SPR_PRIV *) gbm->priv;
	int scan_stride = priv->bytes_per_line;
	int scan_first = ((priv->first_bit) >> 3);
	int scan_bytes, i, j;
	gbm_u8 *datal = data + (gbm->h - 1) * stride;

	gbm_file_lseek(fd, priv->pos_image, GBM_SEEK_SET);

	switch ( priv->actual_bpp )
		{
/*...s8 \45\ read mapped pixels:16:*/
case 8:
	scan_bytes = gbm->w;
	for ( j = 0; j < gbm->h; j++, datal -= stride )
		{
		gbm_file_lseek(fd, priv->pos_image + j * scan_stride + scan_first, GBM_SEEK_SET);
		if ( gbm_file_read(fd, datal, scan_bytes) != scan_bytes )
			return GBM_ERR_READ;
		}
	break;
/*...e*/
/*...s4 \45\ read pixels\44\ nibble swapping:16:*/
case 4:
	scan_bytes = ((unsigned)(gbm->w + 1) >> 1);
	for ( j = 0; j < gbm->h; j++, datal -= stride )
		{
		gbm_file_lseek(fd, priv->pos_image + j * scan_stride + scan_first, GBM_SEEK_SET);
		if ( gbm_file_read(fd, datal, scan_bytes) != scan_bytes )
			return GBM_ERR_READ;
		for ( i = 0; i < scan_bytes; i++ )
			datal[i] = nibble_swap[datal[i]];
		}
	break;
/*...e*/
/*...s2 \45\ read pixels\44\ bit\45\pair reversing and expanding:16:*/
/*
Data is coming in a 2bpp, but we don't actually support this.
So we will expand the data to 4bpp as we read it.
We will do this inline, by reading into the second half.
*/

case 2:
	scan_bytes = ((unsigned)(gbm->w + 3) >> 2);
	for ( j = 0; j < gbm->h; j++, datal -= stride )
		{
		gbm_file_lseek(fd, priv->pos_image + j * scan_stride + scan_first, GBM_SEEK_SET);
		if ( gbm_file_read(fd, datal + scan_bytes, scan_bytes) != scan_bytes )
			return GBM_ERR_READ;
		for ( i = 0; i < scan_bytes; i++ )
			{
			datal[i * 2    ] = pair_swap[datal[scan_bytes + i] & 0x0f];
			datal[i * 2 + 1] = pair_swap[datal[scan_bytes + i] >> 4  ];
			}
		}
	break;
/*...e*/
/*...s1 \45\ read pixels\44\ bit reversing:16:*/
case 1:
	scan_bytes = ((unsigned)(gbm->w + 7) >> 3);
	for ( j = 0; j < gbm->h; j++, datal -= stride )
		{
		gbm_file_lseek(fd, priv->pos_image + j * scan_stride + scan_first, GBM_SEEK_SET);
		if ( gbm_file_read(fd, datal, scan_bytes) != scan_bytes )
			return GBM_ERR_READ;
		for ( i = 0; i < scan_bytes; i++ )
			datal[i] = bit_swap[datal[i]];
		}
	break;
/*...e*/
		}

	return GBM_ERR_OK;
	}
/*...e*/
/*...sspr_w:0:*/
/*
We have a problem here for 256 colour modes.
The 256 colours we are asked to write can be any colours and unrelated.
There are only 16 VIDC palette registers, and the other 4 bits in an 8 bit
byte override specific palette entry bits.
We will ignore all but the top 4 bits of each colour component. ie: just 7-4.
Therefore we will write a specifically fixed palette :-
	Bits 1 to 0 will be bits 5 and 4 for all 3 guns.
	Bit 2 will be red bit 6
	Bit 3 will be blue bit 6
	Bit 4 will be red bit 7
	Bit 5 will be green bit 6
	Bit 6 will be green bit 7
	Bit 7 will be blue bit 7
This is the default palette used by RiscOS.
We will map all incoming palette entrys first, to give a quick lookup table.
*/

/*...swrite_pal:0:*/
static gbm_boolean write_pal(int fd, GBMRGB gbmrgb)
	{
	gbm_u8 pal[8];
	int j;
	pal[0] = pal[4] = 0x00;
	pal[1] = pal[5] = (gbm_u8) (gbmrgb.r & 0xf0);
	pal[2] = pal[6] = (gbm_u8) (gbmrgb.g & 0xf0);
	pal[3] = pal[7] = (gbm_u8) (gbmrgb.b & 0xf0);
	for ( j = 0; j < 8; j++ )
		pal[j] += (pal[j] >> 4);
	return gbm_file_write(fd, pal, 8) == 8;
	}
/*...e*/

GBM_ERR spr_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{
	int stride = ((gbm->bpp * gbm->w + 31) / 32) * 4;
	int i, j, npal = ( 1 << gbm->bpp );
	gbm_u32 dword_w, last_bit, offset_image, mode;
	char name[12];
	gbm_u8 qpal[0x100], *buf;
	gbm_boolean ok;

	opt=opt; /* Suppress 'unref arg' warning */

	if ( gbm->bpp == 24 )
		return GBM_ERR_NOT_SUPP;

	if ( npal == 0x100 )
		npal = 0x10;

	ok  = write_dword(fd, 1);		/* Number of sprites */
	ok &= write_dword(fd, 12 + 4);		/* Offset to sprite */
	ok &= write_dword(fd, (12 + 44 + npal * 8 + stride * gbm->h) + 4);
						/* Offset to free word (beyond sprites) */
	ok &= write_dword(fd, 44 + npal * 8 + stride * gbm->h);

/*...sbuild a name:8:*/
/*
We will use the supplied filename to build the name field.
We will ignore the path part (if any present).
Also we will chop of the file extension of .spr if present.
(This change was by customer request).
Also, map to lower case to keep RiscOS programs happy.
This isn't documented, but appears to be the case.
*/

{
const char *p = fn;
char *q;
if ( (q = strrchr(p, '\\')) != NULL )
	p = q + 1;
if ( (q = strrchr(p, '/')) != NULL )
	p = q + 1;
memset(name, 0, 12);
strncpy(name, p, 11);
if ( (q = strrchr(name, '.')) != NULL && gbm_same(q + 1, "spr", 4) )
	memset(q, '\0', 4);
for ( q = name; (q - name) < 12 && *q != '\0'; q++ )
	*q = tolower(*q);
}
/*...e*/
	ok &= ( gbm_file_write(fd, name, 12) == 12 );

	dword_w = ((gbm->w * gbm->bpp + 31) / 32) - 1;
	ok &= write_dword(fd, dword_w);		/* Width in dwords - 1 */
	ok &= write_dword(fd, gbm->h - 1);	/* Scan lines high - 1 */
	ok &= write_dword(fd, 0);		/* First bit used (left end of row) */
	last_bit = ((gbm->w * gbm->bpp - 1) & 0x1f);
	ok &= write_dword(fd, last_bit);	/* Last bit used (right end of row) */
	offset_image = ( 44 + npal * 8 );
	ok &= write_dword(fd, offset_image);	/* Offset of image data */
	ok &= write_dword(fd, offset_image);	/* Offset of mask plane or above if none */
	mode = mode_of_bpp[gbm->bpp];
	ok &= write_dword(fd, mode);		/* Mode sprite defined in */

	if ( !ok )
		return GBM_ERR_WRITE;

	/* Write palette */

	switch ( gbm->bpp )
		{
/*...s8 \45\ write RiscOS default palette\44\ and work out mapping:16:*/
case 8:

	/* Write the fixed RiscOS default palette */
	for ( i = 0; i < 0x10; i++ )
		{
		GBMRGB gbmrgb_def;
		gbm_u8 tint = ((i & 0x03) << 4);
		gbmrgb_def.r = tint + ((i & 0x04) << 4);
		gbmrgb_def.g = tint;
		gbmrgb_def.b = tint + ((i & 0x08) << 3);
		if ( !write_pal(fd, gbmrgb_def) )
			return GBM_ERR_WRITE;
		}

	/* Determine palette mapping */
	for ( i = 0; i < 0x100; i++ )
		{
		gbm_u8 r = gbmrgb[i].r;
		gbm_u8 g = gbmrgb[i].g;
		gbm_u8 b = gbmrgb[i].b;
		gbm_u8 k32 = ((((r & 0x30) + (g & 0x30) + (b & 0x30)) / 3) & 0x30);
		qpal[i] =  (b & 0x80U)       +
			  ((g & 0xc0U) >> 1) +
			  ((r & 0x80U) >> 3) +
			  ((b & 0x40U) >> 3) +
			  ((r & 0x40U) >> 4) +
			   (k32 >> 4);
		}
	break;
/*...e*/
/*...s4\44\1 \45\ write 16 or 2 entry palette:16:*/
case 4:
case 1:
	/* Write the palette */
	for ( i = 0; i < (1 << gbm->bpp); i++ )
		if ( !write_pal(fd, gbmrgb[i]) )
			return GBM_ERR_WRITE;
	break;
/*...e*/
		}

	/* Write data */

	if ( (buf = gbmmem_malloc((size_t) stride)) == NULL )
		return GBM_ERR_MEM;
	memset(buf, 0, stride);

	data += (gbm->h - 1) * stride; /* Start at the top */
	switch ( gbm->bpp )
		{
/*...s8 \45\ write mapped pixels\44\ funny order:16:*/
case 8:
	for ( j = 0; j < gbm->h; j++, data -= stride )
		{
		for ( i = 0; i < stride; i++ )
			buf[i] = qpal[data[i]];
		if ( gbm_file_write(fd, buf, stride) != stride )
			{
			gbmmem_free(buf);
			return GBM_ERR_WRITE;
			}
		}
	break;
/*...e*/
/*...s4 \45\ write pixels\44\ funny order:16:*/
case 4:
	for ( j = 0; j < gbm->h; j++, data -= stride )
		{
		for ( i = 0; i < stride; i++ )
			buf[i] = nibble_swap[data[i]];
		if ( gbm_file_write(fd, buf, stride) != stride )
			{
			gbmmem_free(buf);
			return GBM_ERR_WRITE;
			}
		}
	break;
/*...e*/
/*...s1 \45\ write pixels\44\ funny order:16:*/
case 1:
	for ( j = 0; j < gbm->h; j++, data -= stride )
		{
		for ( i = 0; i < stride; i++ )
			buf[i] = bit_swap[data[i]];
		if ( gbm_file_write(fd, buf, stride) != stride )
			{
			gbmmem_free(buf);
			return GBM_ERR_WRITE;
			}
		}
	break;
/*...e*/
		}

	gbmmem_free(buf);

	return GBM_ERR_OK;
	}
/*...e*/
/*...sspr_err:0:*/
const char *spr_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_SPR_FIRST:
			return "sprite has first bit that is not a multiple of 8";
		case GBM_ERR_SPR_MODE:
			return "sprite defined in unknown mode";
		case GBM_ERR_SPR_OFFSET:
			return "less sprites in file than index requested";
		case GBM_ERR_SPR_PAL8:
			return "8 bit file does not have 0, 16 or 64 palette entries in it";
		}
	return NULL;
	}
/*...e*/
