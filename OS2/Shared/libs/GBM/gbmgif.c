/*

gbmgif.c - Graphics Interchange Format support

Input options: index=# to get a given image in the file
Output options: xscreen=#,yscreen=#,background=#,xpos=#,ypos=#,transcol=#,ilace.

Fixed bugs in LZW compressor.
1. Don't need to write 'tail' at end, if no unprocessed input.
2. Writing 'tail' at end may increase code size.

History:
--------
(Heiko Nitzsche)

19-Feb-2006: Add function to query number of images
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

static GBMFT gif_gbmft =
	{
	GBM_FMT_DESC_SHORT_GIF,
	GBM_FMT_DESC_LONG_GIF,
	GBM_FMT_DESC_EXT_GIF,
	GBM_FT_R1|GBM_FT_R4|GBM_FT_R8|
	GBM_FT_W1|GBM_FT_W4|GBM_FT_W8,
	};

#define	GBM_ERR_GIF_BPP		((GBM_ERR) 1100)
#define	GBM_ERR_GIF_TERM	((GBM_ERR) 1101)
#define	GBM_ERR_GIF_CODE_SIZE	((GBM_ERR) 1102)
#define	GBM_ERR_GIF_CORRUPT	((GBM_ERR) 1103)
#define	GBM_ERR_GIF_HEADER	((GBM_ERR) 1104)

typedef struct
	{
	BOOLEAN ilace, errok;
	int bpp;
	byte pal[0x100*3];
	} GIF_PRIV;

typedef unsigned int cword;

/*...sstep_ilace:0:*/
/* Pass 0 is all lines where  y%8    == 0
   Pass 1 is all lines where (y-4)%8 == 0
   Pass 2 is all lines where (y-2)%4 == 0
   Pass 3 is all lines where (y-1)%2 == 0
   The complexity comes in when you realise there can be < 8 lines in total! */

static int step_ilace(int y, int h, int *pass)
	{
	switch ( *pass )
		{
		case 0:	y += 8;	break;
		case 1: y += 8; break;
		case 2: y += 4; break;
		case 3: y += 2; break;
		}
	if ( y < h ) return y;
	if ( *pass == 0 ) { *pass = 1; y = 4; if ( y < h ) return y; }
	if ( *pass == 1 ) { *pass = 2; y = 2; if ( y < h ) return y; }
	if ( *pass == 2 ) { *pass = 3; y = 1; }
	return y;
	}
/*...e*/

/*...sgif_qft:0:*/
GBM_ERR gif_qft(GBMFT *gbmft)
	{
	*gbmft = gif_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/

/* ---------------------------------------- */

/* Read number of images in the GIF file. */
GBM_ERR gif_rimgcnt(const char *fn, int fd, int *pimgcnt)
{
    GBM gbm;
    GIF_PRIV *gif_priv = (GIF_PRIV *) gbm.priv;
    byte signiture[6], scn_desc[7], image_desc[10];
    int bits_gct;

    fn=fn; /* Suppress 'unref arg' compiler warnings */

    *pimgcnt = 0;

    /* Read and validate signiture block */
    if ( gbm_file_read(fd, signiture, 6) != 6 )
        return GBM_ERR_READ;
    if ( memcmp(signiture, "GIF87a", 6) &&
         memcmp(signiture, "GIF89a", 6) )
        return GBM_ERR_BAD_MAGIC;

    /* Read screen descriptor */
    if ( gbm_file_read(fd, scn_desc, 7) != 7 )
        return GBM_ERR_READ;
    bits_gct = (scn_desc[4] & 7) + 1;

    if ( scn_desc[4] & 0x80 )
    /* Global colour table follows screen descriptor */
    {
        if ( gbm_file_read(fd, gif_priv->pal, 3 << bits_gct) != (3 << bits_gct) )
            return GBM_ERR_READ;
    }

    /* Expected image descriptors / extension blocks / terminator */
    while (gbm_file_read(fd, image_desc, 1) == 1)
    {
        switch ( image_desc[0] )
        {
            /*...s0x2c \45\ image descriptor:24:*/
            case 0x2c:
                if ( gbm_file_read(fd, image_desc + 1, 9) != 9 )
                    return GBM_ERR_READ;

                if ( image_desc[9] & 0x80 )
                /* Local colour table follows */
                {
                    gif_priv->bpp = (image_desc[9] & 7) + 1;
                    if ( gbm_file_read(fd, gif_priv->pal, 3 << gif_priv->bpp) != (3 << gif_priv->bpp) )
                        return GBM_ERR_READ;
                }
                /* Skip the image data */
                {
                    byte code_size, block_size;

                    if ( gbm_file_read(fd, &code_size, 1) != 1 )
                        return GBM_ERR_READ;
                    do
                    {
                       if ( gbm_file_read(fd, &block_size, 1) != 1 )
                           return GBM_ERR_READ;
                       gbm_file_lseek(fd, block_size, GBM_SEEK_CUR);
                    }
                    while ( block_size );
                }

                (*pimgcnt)++;
                break;
                /*...e*/

            /*...s0x21 \45\ extension block:24:*/
            /* Ignore all extension blocks */
            case 0x21:
            {
                byte func_code, byte_count;

                if ( gbm_file_read(fd, &func_code, 1) != 1 )
                    return GBM_ERR_READ;
                do
                {
                    if ( gbm_file_read(fd, &byte_count, 1) != 1 )
                        return GBM_ERR_READ;
                    gbm_file_lseek(fd, byte_count, GBM_SEEK_CUR);
                }
                while ( byte_count );
            }
            break;
            /*...e*/

            /*...s0x3b \45\ terminator:24:*/
            /* Oi, we were hoping to get an image descriptor! */

            case 0x3b:
                break;
                /*...e*/

            /*...sdefault:24:*/
            default:
                return GBM_ERR_GIF_HEADER;
                /*...e*/
        }
    }

    return GBM_ERR_OK;
}

/* ---------------------------------------- */

/*...sgif_rhdr:0:*/
GBM_ERR gif_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	GIF_PRIV *gif_priv = (GIF_PRIV *) gbm->priv;
	byte signiture[6], scn_desc[7], image_desc[10];
	const char *index;
	int img = -1, img_want = 0;
	int bits_gct;

	fn=fn; /* Suppress 'unref arg' compiler warnings */

	/* Discover which image in GIF file we want */

	if ( (index = gbm_find_word_prefix(opt, "index=")) != NULL )
		sscanf(index + 6, "%u", &img_want);

	gif_priv->errok = ( gbm_find_word(opt, "errok") != NULL );

	/* Read and validate signiture block */

	if ( gbm_file_read(fd, signiture, 6) != 6 )
		return GBM_ERR_READ;
	if ( memcmp(signiture, "GIF87a", 6) &&
	     memcmp(signiture, "GIF89a", 6) )
		return GBM_ERR_BAD_MAGIC;

	/* Read screen descriptor */

	if ( gbm_file_read(fd, scn_desc, 7) != 7 )
		return GBM_ERR_READ;
	gif_priv->bpp = bits_gct = (scn_desc[4] & 7) + 1;

	if ( scn_desc[4] & 0x80 )
		/* Global colour table follows screen descriptor */
		{
		if ( gbm_file_read(fd, gif_priv->pal, 3 << bits_gct) != (3 << bits_gct) )
			return GBM_ERR_READ;
		}
	else
		/* Blank out palette, but make entry 1 white */
		{
		memset(gif_priv->pal, 0, 3 << bits_gct);
		gif_priv->pal[3] =
		gif_priv->pal[4] =
		gif_priv->pal[5] = 0xff;
		}

	/* Expected image descriptors / extension blocks / terminator */

	while ( img < img_want )
		{
		if ( gbm_file_read(fd, image_desc, 1) != 1 )
			return GBM_ERR_READ;
		switch ( image_desc[0] )
			{
/*...s0x2c \45\ image descriptor:24:*/
case 0x2c:
	if ( gbm_file_read(fd, image_desc + 1, 9) != 9 )
		return GBM_ERR_READ;
	gbm->w = make_word(image_desc[5], image_desc[6]);
	gbm->h = make_word(image_desc[7], image_desc[8]);
	gif_priv->ilace = ( (image_desc[9] & 0x40) != 0 );

	if ( image_desc[9] & 0x80 )
		/* Local colour table follows */
		{
		gif_priv->bpp = (image_desc[9] & 7) + 1;
		if ( gbm_file_read(fd, gif_priv->pal, 3 << gif_priv->bpp) != (3 << gif_priv->bpp) )
			return GBM_ERR_READ;
		}

	if ( ++img != img_want )
		/* Skip the image data */
		{
		byte code_size, block_size;

		if ( gbm_file_read(fd, &code_size, 1) != 1 )
			return GBM_ERR_READ;
		do
			{
			if ( gbm_file_read(fd, &block_size, 1) != 1 )
				return GBM_ERR_READ;
			gbm_file_lseek(fd, block_size, GBM_SEEK_CUR);
			}
		while ( block_size );
		}

	break;
/*...e*/
/*...s0x21 \45\ extension block:24:*/
/* Ignore all extension blocks */

case 0x21:
	{
	byte func_code, byte_count;

	if ( gbm_file_read(fd, &func_code, 1) != 1 )
		return GBM_ERR_READ;
	do
		{
		if ( gbm_file_read(fd, &byte_count, 1) != 1 )
			return GBM_ERR_READ;
		gbm_file_lseek(fd, byte_count, GBM_SEEK_CUR);
		}
	while ( byte_count );
	}
	break;
/*...e*/
/*...s0x3b \45\ terminator:24:*/
/* Oi, we were hoping to get an image descriptor! */

case 0x3b:
	return GBM_ERR_GIF_TERM;
/*...e*/
/*...sdefault:24:*/
default:
	return GBM_ERR_GIF_HEADER;
/*...e*/
			}
		}

	switch ( gif_priv->bpp )
		{
		case 1:		gbm->bpp = 1;		break;
		case 2:
		case 3:
		case 4:		gbm->bpp = 4;		break;
		case 5:
		case 6:
		case 7:
		case 8:		gbm->bpp = 8;		break;
		default:	return GBM_ERR_GIF_BPP;
		}

	return GBM_ERR_OK;
	}
/*...e*/
/*...sgif_rpal:0:*/
GBM_ERR gif_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	GIF_PRIV *gif_priv = (GIF_PRIV *) gbm->priv;
	byte *pal = gif_priv->pal;
	int i;

	fd=fd; /* Suppress 'unref arg' compiler warning */

	memset(gbmrgb, 0x80, (sizeof(GBMRGB) << gbm->bpp));

	for ( i = 0; i < (1 << gif_priv->bpp); i++ )
		{
		gbmrgb[i].r = *pal++;
		gbmrgb[i].g = *pal++;
		gbmrgb[i].b = *pal++;
		}

	return GBM_ERR_OK;
	}
/*...e*/
/*...sgif_rdata:0:*/
/*...sread context:0:*/
typedef struct
	{
	int fd;				/* File descriptor to read */
	int inx, size;			/* Index and size in bits */
	byte buf[255+3];		/* Buffer holding bits */
	int code_size;			/* Number of bits to return at once */
	cword read_mask;		/* 2^code_size-1 */
	} READ_CONTEXT;

static cword read_code(READ_CONTEXT *c)
	{
	dword raw_code; int byte_inx;

	while ( c->inx + c->code_size > c->size )
/*...snot enough bits in buffer\44\ refill it:16:*/
/* Not very efficient, but infrequently called */

{
int bytes_to_lose = ((unsigned) c->inx >> 3);
byte bytes;

/* Note biggest code size is 12 bits */
/* And this can at worst span 3 bytes */
memcpy(c->buf, c->buf + bytes_to_lose, 3);
(c->inx) &= 7;
(c->size) -= (bytes_to_lose << 3);
if ( gbm_file_read(c->fd, &bytes, 1) != 1 )
	return 0xffff;
if ( gbm_file_read(c->fd, c->buf + ((unsigned) c->size >> 3), bytes) != bytes )
	return 0xffff;
(c->size) += (bytes << 3);
}
/*...e*/

	byte_inx = ((unsigned) c->inx >> 3);
	raw_code = c->buf[byte_inx] + ((c->buf[byte_inx + 1]) << 8);
	if ( c->code_size > 8 )
		raw_code += ((c->buf[byte_inx + 2]) << 16);
	raw_code >>= ((c->inx) & 7U);
	(c->inx) += (byte) (c->code_size);

	return (cword) raw_code & c->read_mask;
	}
/*...e*/
/*...soutput context:0:*/
typedef struct
	{
	int x, y, w, h, bpp, pass;
	BOOLEAN ilace;
	int stride;
	byte *data, *data_this_line;
	} OUTPUT_CONTEXT;

static void output(byte value, OUTPUT_CONTEXT *o)
	{
	if ( o->y >= o->h )
		return;

	switch ( o->bpp )
		{
		case 1:
			if ( (o->x) & 7U )
				o->data_this_line[(unsigned)(o->x) >> 3] |= (value << (7 - (((unsigned)o->x) & 7U)));
			else
				o->data_this_line[(unsigned)(o->x) >> 3] = (value << 7);
			break;
		case 4:
			if ( (o->x) & 1U )
				o->data_this_line[(unsigned)(o->x) >> 1] |= value;
			else
				o->data_this_line[(unsigned)(o->x) >> 1] = (value << 4);
			break;
		case 8:
			o->data_this_line[o->x] = value;
			break;
		}

	if ( ++(o->x) < o->w )
		return;

	o->x = 0;
	if ( o->ilace )
		{
		o->y = step_ilace(o->y, o->h, &(o->pass));
		o->data_this_line = o->data + (o->h - 1 - o->y) * o->stride;
		}
	else
		{
		(o->y)++;
		o->data_this_line -= (o->stride);
		}
	}
/*...e*/

GBM_ERR gif_rdata(int fd, GBM *gbm, byte *data)
	{
	GIF_PRIV *gif_priv = (GIF_PRIV *) gbm->priv;
	byte min_code_size;		/* As read from the file */
	int init_code_size;		/* Initial code size */
	cword max_code;			/* 1 << code_size */
	cword clear_code;		/* Code to clear table */
	cword eoi_code;			/* End of information code */
	cword first_free_code;		/* First free code */
	cword free_code;		/* Next available free code slot */
	word bit_mask;			/* Output pixel mask */ 
	int i, out_count = 0;
	cword code = 0, cur_code = 0, old_code = 0, in_code = 0, fin_char = 0;
	cword *prefix, *suffix, *outcode;
	READ_CONTEXT c;
	OUTPUT_CONTEXT o;
	BOOLEAN table_full = FALSE;	/* To help implement deferred clear */

	if ( (prefix = (cword *) malloc((size_t) (4096 * sizeof(cword)))) == NULL )
		return GBM_ERR_MEM;
	if ( (suffix = (cword *) malloc((size_t) (4096 * sizeof(cword)))) == NULL )
		{
		free(prefix);
		return GBM_ERR_MEM;
		}
	if ( (outcode = (cword *) malloc((size_t) (4097 * sizeof(cword)))) == NULL )
		{
		free(suffix);		
		free(prefix);
		return GBM_ERR_MEM;
		}

	if ( gbm_file_read(fd, &min_code_size, 1) != 1 )
		{	
		free(outcode);
		free(suffix);
		free(prefix);
		return GBM_ERR_READ;
		}

	if ( min_code_size < 2 || min_code_size > 9 )
		{	
		free(outcode);
		free(suffix);
		free(prefix);
		return GBM_ERR_GIF_CODE_SIZE;
		}

	/* Initial read context */

	c.inx            = 0;
	c.size           = 0;
	c.fd             = fd;
	c.code_size      = min_code_size + 1;
	c.read_mask      = (cword) (( 1 << c.code_size ) - 1);

	/* Initialise pixel-output context */

	o.x              = 0;
	o.y              = 0;
	o.pass           = 0;
	o.w              = gbm->w;
	o.h              = gbm->h;
	o.bpp            = gbm->bpp;
	o.ilace          = gif_priv->ilace;
	o.stride         = ( (gbm->w * gbm->bpp + 31) / 32 ) * 4;
	o.data           = data;
	o.data_this_line = data + (gbm->h - 1) * o.stride;

	bit_mask = (word) ((1 << gif_priv->bpp) - 1);

	/* 2^min_code size accounts for all colours in file */

	clear_code = (cword) ( 1 << min_code_size );
	eoi_code = (cword) (clear_code + 1);
	free_code = first_free_code = (cword) (clear_code + 2);

	/* 2^(min_code_size+1) includes clear and eoi code and space too */

	init_code_size = c.code_size;
	max_code = (cword) ( 1 << c.code_size );

	while ( o.y < o.h && (code = read_code(&c)) != eoi_code && code != 0xffff )
		{
		if ( code == clear_code )
			{
			c.code_size = init_code_size;
			max_code = (cword) ( 1 << c.code_size );
			c.read_mask = (cword) (max_code - 1);
			free_code = first_free_code;
			cur_code = old_code = code = read_code(&c);
			if ( code == 0xffff )
				break;
			if ( code & ~bit_mask )
				{
				free(outcode);
				free(suffix);
				free(prefix);
				return gif_priv->errok ? GBM_ERR_OK : GBM_ERR_GIF_CORRUPT;
				}
			fin_char = (cur_code & bit_mask);
			output((byte) fin_char, &o);
			table_full = FALSE;
			}
		else
			{
			cur_code = in_code = code;
			if ( cur_code >= free_code )
				{
				cur_code = old_code;
				outcode[out_count++] = fin_char;
				}
			while ( cur_code > bit_mask )
				{
				if ( out_count > 4096 )
					{
					free(outcode);
					free(suffix);
					free(prefix);
					return gif_priv->errok ? GBM_ERR_OK : GBM_ERR_GIF_CORRUPT;
					}
				outcode[out_count++] = suffix[cur_code];
				cur_code = prefix[cur_code];
				}
			fin_char = (cur_code & bit_mask);
			outcode[out_count++] = fin_char;
			for ( i = out_count - 1; i >= 0; i-- )
				output((byte) outcode[i], &o);
			out_count = 0;

			/* Update dictionary */

			if ( !table_full )
				{
				prefix[free_code] = old_code;
				suffix[free_code] = fin_char;

				/* Advance to next free slot */

				if ( ++free_code >= max_code )
					{
					if ( c.code_size < 12 )
						{
						c.code_size++;
						max_code <<= 1;
						c.read_mask = (cword) (( 1 << c.code_size ) - 1);
						}
					else
						table_full = TRUE;
					}
				}
			old_code = in_code;
			}
		}

	free(outcode);
	free(suffix);
	free(prefix);

	if ( o.y < o.h && code == 0xffff )
		/* If ran out of data and hadn't got to the end */
		return gif_priv->errok ? GBM_ERR_OK : GBM_ERR_READ;

	return GBM_ERR_OK;
	}
/*...e*/
/*...sgif_w:0:*/
/*
We won't write any GIF89a or higher extensions into file.
Write palette as global colour table, not local.
*/

/*...swrite context:0:*/
typedef struct
	{
	int fd;				/* Open file descriptor to write to */
	int inx;			/* Bit index into buf */
	int code_size;			/* Code size in bits */
	byte buf[255+2];		/* Biggest block + overflow space */
	} WRITE_CONTEXT;

static BOOLEAN write_code(cword code, WRITE_CONTEXT *w)
	{
	byte *buf = w->buf + ((unsigned)w->inx >> 3);

	code <<= ((w->inx) & 7);
	*buf++ |= (byte)  code       ;
	*buf++  = (byte) (code >>  8);
	*buf    = (byte) (code >> 16);

	(w->inx) += (w->code_size);
	if ( w->inx >= 255 * 8 )
		/* Flush out full buffer */
		{
		byte bytes = 255;

		if ( gbm_file_write(w->fd, &bytes, 1) != 1 )
			return FALSE;
		if ( gbm_file_write(w->fd, w->buf, 255) != 255 )
			return FALSE;

		memcpy(w->buf, w->buf + 255, 2);
		memset(w->buf + 2, 0, 255);
		(w->inx) -= (255 * 8);
		}

	return TRUE;
	}

static BOOLEAN flush_code(WRITE_CONTEXT *w)
	{
	byte bytes = ((unsigned)(w->inx + 7) >> 3);

	if ( bytes )
		{
		if ( gbm_file_write(w->fd, &bytes, 1) != 1 )
			return FALSE;
		if ( gbm_file_write(w->fd, w->buf, bytes) != bytes )
			return FALSE;
		}

	/* Data block terminator - a block of zero size */

	bytes = 0;
	return gbm_file_write(w->fd, &bytes, 1) == 1;
	}
/*...e*/

typedef struct { cword tail; byte col; } DICT;

GBM_ERR gif_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt)
	{
	int xpos = 0, ypos = 0;
	int xscreen = gbm->w, yscreen = gbm->h;
	int inx_background = 0, inx_transcol = -1;
	BOOLEAN ilace;

/*...shandle options:8:*/
{
const char *s;

fn=fn; /* Suppress 'unref arg' compiler warnings */

if ( gbm->bpp != 1 && gbm->bpp != 4 && gbm->bpp != 8 )
	return GBM_ERR_NOT_SUPP;

ilace = ( gbm_find_word(opt, "ilace") != NULL );

if ( (s = gbm_find_word_prefix(opt, "xscreen=")) != NULL )
	sscanf(s + 8, "%d", &xscreen);

if ( (s = gbm_find_word_prefix(opt, "yscreen=")) != NULL )
	sscanf(s + 8, "%d", &yscreen);

if ( (s = gbm_find_word_prefix(opt, "background=")) != NULL )
	sscanf(s + 11, "%d", &inx_background);

if ( (s = gbm_find_word_prefix(opt, "xpos=")) != NULL )
	sscanf(s + 5, "%d", &xpos);

if ( (s = gbm_find_word_prefix(opt, "ypos=")) != NULL )
	sscanf(s + 5, "%d", &ypos);

if ( (s = gbm_find_word_prefix(opt, "transcol=")) != NULL )
	{
	if ( gbm_same(s+9, "edge", 4) )
		switch ( gbm->bpp )
			{
			case 8:	inx_transcol =   *data       ;	break;
			case 4:	inx_transcol = ( *data >> 4 );	break;
			case 1:	inx_transcol = ( *data >> 7 );	break;
			}
	else
		sscanf(s + 9, "%d", &inx_transcol);
	}
}
/*...e*/
/*...swrite header etc\46\:8:*/
{
byte scn_desc[7], image_desc[10]; int p;
char *sig = ( inx_transcol != -1 ) ? "GIF89a" : "GIF87a";

/* Write signiture */

if ( gbm_file_write(fd, sig, 6) != 6 )
	return GBM_ERR_WRITE;

/* Write screen descriptor */

scn_desc[0] = low_byte(xscreen);
scn_desc[1] = high_byte(xscreen);
scn_desc[2] = low_byte(yscreen);
scn_desc[3] = high_byte(yscreen);
scn_desc[4] = (0x80 | ((gbm->bpp - 1) * 0x11));
			/* Global colour table follows */
			/* Quality bpp == gct bpp == gbm->bpp */
scn_desc[5] = (byte) inx_background;
scn_desc[6] = 0;
if ( gbm_file_write(fd, scn_desc, 7) != 7 )
	return GBM_ERR_WRITE;

/* Write global colour table */

for ( p = 0; p < (1 << gbm->bpp); p++ )
	{
	byte pal[3];

	pal[0] = gbmrgb[p].r;
	pal[1] = gbmrgb[p].g;
	pal[2] = gbmrgb[p].b;
	if ( gbm_file_write(fd, pal, 3) != 3 )
		return GBM_ERR_WRITE;
	}

if ( inx_transcol != -1 )
	/* Do GIF89a "Graphic Control Extension" application extension block */
	{
	char gce[8];
	gce[0] = 0x21;			/* Extension Introducer */
	gce[1] = (char) 0xf9;		/* Graphic Control Label */
	gce[2] = 4;			/* Block size */
	gce[3] = 0x01;			/* No 'disposal', no 'user input' */
					/* Just transparent index present */
	gce[4] = 0;			/* Delay time, 0 => not set */
	gce[5] = 0;			/* Delay time, 0 => not set */
	gce[6] = (char) inx_transcol;	/* Transparent colour index */
	gce[7] = 0;			/* Block size, 0 => end of extension */
	if ( gbm_file_write(fd, gce, 8) != 8 )
		return GBM_ERR_WRITE;
	}

/* Do image descriptor block */

image_desc[0] = (byte) 0x2c;
image_desc[1] = low_byte(xpos);
image_desc[2] = high_byte(xpos);
image_desc[3] = low_byte(ypos);
image_desc[4] = high_byte(ypos);
image_desc[5] = low_byte(gbm->w);
image_desc[6] = high_byte(gbm->w);
image_desc[7] = low_byte(gbm->h);
image_desc[8] = high_byte(gbm->h);
image_desc[9] = gbm->bpp - 1;
	/* Non-interlaced, no local colour map, no sorted palette */
if ( ilace )
	image_desc[9] |= 0x40; /* Interlaced */
if ( gbm_file_write(fd, image_desc, 10) != 10 )
	return GBM_ERR_WRITE;
}
/*...e*/
/*...sLZW encode data\44\ tail\43\col lookup version:8:*/
/*
hashvalue is calculated from a string of pixels cumulatively.
hashtable is searched starting at index hashvalue for to find the entry.
hashtable is big enough so that MAX_HASH > 4*MAX_DICT.
*/

#define	MAX_HASH	17777			/* Probably prime and > 4096 */
#define	MAX_DICT	4096			/* Dictionary size           */
#define	INIT_HASH(p)	(((p)+3)*301)		/* Initial hash value        */

{
int stride = ((gbm->w * gbm->bpp + 31) / 32) * 4;
byte min_code_size;
int init_code_size, x, y, pass;
cword clear_code = 0, eoi_code = 0, last_code = 0, max_code = 0, tail = 0;
unsigned int hashvalue = 0, lenstring = 0, j;
DICT *dict, **hashtable;
WRITE_CONTEXT w;

/* Now LZW encode data */

if ( (dict = (DICT *) malloc((size_t) (MAX_DICT * sizeof(DICT)))) == NULL )
	return GBM_ERR_MEM;

if ( (hashtable = (DICT **) malloc((size_t) (MAX_HASH * sizeof(DICT *)))) == NULL )
	{
	free(dict);
	return GBM_ERR_MEM;
	}

/* Initialise encoder variables */

init_code_size = gbm->bpp + 1;
if ( init_code_size == 2 )
	/* Room for col0, col1, cc, eoi, but no others! */
	init_code_size++;

min_code_size = init_code_size - 1;
if ( gbm_file_write(fd, &min_code_size, 1) != 1 )
	{
	free(hashtable);
	free(dict);
	return GBM_ERR_WRITE;
	}

clear_code = ( 1 << min_code_size );
eoi_code   = clear_code + 1;
last_code  = eoi_code;
max_code   = ( 1 << init_code_size );
lenstring  = 0;

/* Setup write context */

w.fd        = fd;
w.inx       = 0;
w.code_size = init_code_size;
memset(w.buf, 0, sizeof(w.buf));

if ( !write_code(clear_code, &w) )
	{
	free(hashtable);
	free(dict);
	return GBM_ERR_WRITE;
	}

for ( j = 0; j < MAX_HASH; j++ )
	hashtable[j] = NULL;

data += ( (gbm->h - 1) * stride );
for ( y = pass = 0; y < gbm->h; )
	{
	const byte *pdata = data - y * stride;
	for ( x = 0; x < gbm->w; x++ )
		{
		byte col;
/*...sget col:24:*/
switch ( gbm->bpp )
	{
	case 8:
		col = *pdata++;
		break;
	case 4:
		if ( x & 1 )
			col = (*pdata++ & 0x0f);
		else
			col = (*pdata >> 4);
		break;
	default: /* must be 1 */
		if ( (x & 7) == 7 )
			col = (*pdata++ & 0x01);
		else
			col = ((*pdata >> (7-(x&7))) & 0x01);
		break;
	}
/*...e*/
/*...sLZW encode:24:*/
if ( ++lenstring == 1 )
	{
	tail      = col;
	hashvalue = INIT_HASH(col);
	}
else
	{
	hashvalue *= ( col + lenstring + 4 );
	j = ( hashvalue %= MAX_HASH );
	while ( hashtable[j] != NULL &&
		( hashtable[j]->tail != tail ||
		  hashtable[j]->col  != col  ) )
		if ( ++j >= MAX_HASH )
			j = 0;
	if ( hashtable[j] != NULL )
		/* Found in the strings table */
		tail = (hashtable[j]-dict);
	else
		/* Not found */
		{
		if ( !write_code(tail, &w) )
			{
			free(hashtable);
			free(dict);
			return GBM_ERR_WRITE;
			}
		hashtable[j]       = dict + ++last_code;
		hashtable[j]->tail = tail;
		hashtable[j]->col  = col;
		tail               = col;
		hashvalue          = INIT_HASH(col);
		lenstring          = 1;

		if ( last_code >= max_code )
			/* Next code will be written longer */
			{
			max_code <<= 1;
			w.code_size++;
			}
		else if ( last_code >= MAX_DICT-2 )
			/* Reset tables */
			{
			if ( !write_code(tail      , &w) ||
			     !write_code(clear_code, &w) )
				{
				free(hashtable);
				free(dict);
				return GBM_ERR_WRITE;
				}
			lenstring   = 0;
			last_code   = eoi_code;
			w.code_size = init_code_size;
			max_code    = ( 1 << init_code_size );
			for ( j = 0; j < MAX_HASH; j++ )
				hashtable[j] = NULL;
			}
		}
	}
/*...e*/
		}
	if ( ilace )
		y = step_ilace(y, gbm->h, &pass);
	else
		y++;
	}

free(hashtable);
free(dict);

if ( lenstring != 0 )
	{
	if ( !write_code(tail, &w) )
		return GBM_ERR_WRITE;
	if ( ++last_code >= max_code )
		/* Next code will be written longer */
		w.code_size++;
	}

if ( !write_code(eoi_code, &w) ||
     !flush_code(          &w) )
	return GBM_ERR_WRITE;
}
/*...e*/
/*...swrite terminator:8:*/
{
byte term = (byte) 0x3b;
if ( gbm_file_write(fd, &term, 1) != 1 )
	return GBM_ERR_WRITE;
}
/*...e*/

	return GBM_ERR_OK;
	}
/*...e*/
/*...sgif_err:0:*/
const char *gif_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_GIF_BPP:
			return "bad bits per pixel";
		case GBM_ERR_GIF_TERM:
			return "terminator found before requested image descriptor";
		case GBM_ERR_GIF_CODE_SIZE:
			return "code size not in range 2 to 9";
		case GBM_ERR_GIF_CORRUPT:
			return "encoded data is corrupt";
		case GBM_ERR_GIF_HEADER:
			return "bad header";
		}
	return NULL;
	}
/*...e*/
