/*

gbmcvp.c - Portrait

History:
--------
(Heiko Nitzsche)

22-Feb-2006: Move format description strings to gbmdesc.h
15-Aug-2008  Integrate new GBM types

*/

#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"
#include "gbmmem.h"

static GBMFT cvp_gbmft =
	{
	GBM_FMT_DESC_SHORT_CVP,
	GBM_FMT_DESC_LONG_CVP,
	GBM_FMT_DESC_EXT_CVP,
	GBM_FT_R24|
	GBM_FT_W24,
	};

#define	GBM_ERR_CVP_FSIZE	((GBM_ERR) 1800)
#define	GBM_ERR_CVP_SIZE	((GBM_ERR) 1801)

GBM_ERR cvp_qft(GBMFT *gbmft)
	{
	*gbmft = cvp_gbmft;
	return GBM_ERR_OK;
	}

GBM_ERR cvp_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	long length;

	fn=fn; fd=fd; opt=opt; /* Suppress 'unref arg' compiler warnings */

	length = gbm_file_lseek(fd, 0L, GBM_SEEK_END);
	gbm_file_lseek(fd, 0L, GBM_SEEK_SET);

	if ( length != 512*512*3 )
		return GBM_ERR_CVP_FSIZE;

	gbm->w   = 512;
	gbm->h   = 512;
	gbm->bpp = 24;

	return GBM_ERR_OK;
	}

GBM_ERR cvp_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	fd=fd; gbm=gbm; gbmrgb=gbmrgb; /* Suppress 'unref arg' compiler warnings */

	return GBM_ERR_OK;
	}

GBM_ERR cvp_rdata(int fd, GBM *gbm, gbm_u8 *data)
	{
	int p, stride = ((gbm->w*3+3)&~3);
	gbm_u8 *line;
	if ( (line = gbmmem_malloc((size_t) gbm->w)) == NULL )
		return GBM_ERR_MEM;
	for ( p = 2; p >= 0; p-- )
		{
		int y;
		gbm_u8 *ptr = data + ( (stride * (gbm->h-1)) + p );
		for ( y = 0; y < gbm->h; y++, ptr-=stride )
			{
			int x;
			if ( gbm_file_read(fd, line, gbm->w) != gbm->w )
				{
				gbmmem_free(line);
				return GBM_ERR_READ;
				}
			for ( x = 0; x < gbm->w; x++ )
				ptr[x*3] = line[gbm->w-1-x];
			}
		}
	gbmmem_free(line);
	return GBM_ERR_OK;
	}

GBM_ERR cvp_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{
	int p, stride = ((gbm->w*3+3)&~3);
	gbm_u8 *line;

	fn=fn; gbmrgb=gbmrgb; opt=opt; /* Suppress 'unref arg' compiler warning */

	if ( gbm->bpp != 24 )
		return GBM_ERR_NOT_SUPP;

	if ( gbm->w != 512 || gbm->h != 512 )
		return GBM_ERR_CVP_SIZE;

	if ( (line = gbmmem_malloc((size_t) gbm->w)) == NULL )
		return GBM_ERR_MEM;

	for ( p = 2; p >= 0; p-- )
		{
		int y;
		const gbm_u8 *ptr = data + ( (stride * (gbm->h-1)) + p );
		for ( y = 0; y < gbm->h; y++, ptr-=stride )
			{
			int x;
			for ( x = 0; x < gbm->w; x++ )
				line[gbm->w-1-x] = ptr[x*3];
			if ( gbm_file_write(fd, line, gbm->w) != gbm->w )
				{
				gbmmem_free(line);
				return GBM_ERR_WRITE;
				}
			}
		}
	gbmmem_free(line);
	return GBM_ERR_OK;
	}

const char *cvp_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_CVP_FSIZE:
			return "file is not correct size";
		case GBM_ERR_CVP_SIZE:
			return "portrait files can only hold 512x512 images";
		}
	return NULL;
	}

