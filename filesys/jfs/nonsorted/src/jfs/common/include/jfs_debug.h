/* $Id: jfs_debug.h,v 1.1 2000/04/21 10:57:56 ktk Exp $ */

/* static char *SCCSID = "@(#)1.13  11/1/99 12:20:59 src/jfs/common/include/jfs_debug.h, sysjfs, w45.fs32, fixbld";
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#ifndef _H_JFS_DEBUG
#define _H_JFS_DEBUG

/*
 *	jfs_debug.h
 *
 * global debug message, data structure/macro definitions
 * under control of _JFS_DEBUG, _JFS_STATISTICS;
 * 
 * note: for individual modulei debug control, use _JFS_DEBUG_module; 
 */

/*
 * Change History :
 *
 */

/*
 *	brkpoint()/panic()
 */

#ifdef _JFS_OS2
#include <builtin.h>	/* _interrupt declaration */

#if defined(_JFS_UTILITY) || defined(_JFS_DEBUG)		// D233382
#define ALL_DIRTY()						// D233382
#else
#define ALL_DIRTY all_dirty					// D233382
void all_dirty(void);
#endif

#define brkpoint() _interrupt(3)
#define brkpoint1(foo) brkpoint()
#define brkpoint2(foo,bar) brkpoint()

#define panic(s)\
{\
	printf("[%s #%d]\n",__FILE__,__LINE__);\ 
	printf(s);\
	ALL_DIRTY();\
	brkpoint();\
}

/* XXX
 * OS/2 debugger doesn't know about static symbols, so we
 * define away the attribute -- just until bringup is done.
 */
#define static
#endif /* _JFS_OS2 */

/*
 *	assert with traditional printf/panic
 */
#define assert(p)\
{\
	if (!(p)) {\
		printf("[%s #%d]\n",__FILE__,__LINE__);\ 
		ALL_DIRTY();\
		brkpoint();\
	}\
}


/*
 *	chatterbox control
 */
/* temporary until cleanup */
#define	NOISE(button,prspec)

/*
 *	debug ON
 *	--------
 */
#ifdef _JFS_DEBUG
#define ASSERT(p) assert(p)

/* information message: e.g., configuration, major event */
extern	int32	jfsFYI;
#define jFYI(button,prspec)\
{ if (button && jfsFYI) printf prspec; }

/* 
 *	chatterbox control
 */
/* debug event message: */
#define jEVENT(button,prspec)\
{ if (button) { printf prspec; if (button > 1) brkpoint(); } }
	
/* alert warning message: e.g., critical event */
extern	int32	jfsALERT;
#define jALERT(button, prspec)\
{ if (button && jfsALERT) printf prspec; }

/* error event message: e.g., i/o error */
extern	int32	jfsERROR;
#define jERROR(button, prspec)\
{ if (button && jfsERROR) { printf prspec; if (button > 1) brkpoint(); } }

/* if dial is set above volume level of given message, print it */
#define jNOISESET(dial, level)	int dial = (level);
#define jNOISEGET(dial)		extern dial;
#define jNOISE(dial,level,prspec)\
{ if ((dial) >= (level)) printf prspec; }

/* invoke sanity check function */
#define jSANITY(funct, arg)\
{ funct(arg); }

/*
 *	debug OFF
 *	---------
 */
#else /* _JFS_DEBUG */
#define ASSERT(p)
#define jEVENT(button,prspec)
#define jERROR(button,prspec)
#define jALERT(button,prspec)
#define jFYI(button,prspec)
#define jNOISESET(dial, level)
#define jNOISEGET(dial)
#define jNOISE(dial,level,prspec)
#define jSANITY(funct, arg)

#endif /* _JFS_DEBUG */

#ifdef	_JFS_OS2
#define	jError
#define	jError0(type, event)
#define	jError2(type, event, data1, data2)
#define	jTrace
#define	jTrace0(type, event)
#define	jTrace2(type, event, data1, data2)
#define	jTrace4(type, event, data1, data2, data3, data4)
#endif	/* _JFS_OS2 */


/*
 *	statistics
 *	----------
 */
#ifdef	_JFS_STATISTICS
#define	INCREMENT(x)	((x)++)
#define	DECREMENT(x)	((x)--)
#define	HIGHWATERMARK(x,y)	x = MAX((x), (y))
#else
#define	INCREMENT(x)
#define	DECREMENT(x)
#define	HIGHWATERMARK(x,y)
#endif	/* _JFS_STATISTICS */

#endif /* _H_JFS_DEBUG */
