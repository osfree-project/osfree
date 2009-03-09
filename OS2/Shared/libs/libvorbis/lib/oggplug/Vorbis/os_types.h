#ifndef _OS_TYPES_H
#define _OS_TYPES_H
/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE Ogg Vorbis SOFTWARE CODEC SOURCE CODE.  *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS SOURCE IS GOVERNED BY *
 * THE GNU PUBLIC LICENSE 2, WHICH IS INCLUDED WITH THIS SOURCE.    *
 * PLEASE READ THESE TERMS DISTRIBUTING.                            *
 *                                                                  *
 * THE OggSQUISH SOURCE CODE IS (C) COPYRIGHT 1994-2000             *
 * by Monty <monty@xiph.org> and The XIPHOPHORUS Company            *
 * http://www.xiph.org/                                             *
 *                                                                  *
 ********************************************************************

 function: #ifdef jail to whip a few platforms into the UNIX ideal.
 last mod: $Id: os_types.h 1596 2002-02-23 03:51:15Z bh $

 ********************************************************************/

#if defined (_WIN32) 
#if !defined(__GNUC__)

/* MSVC/Borland */
typedef __int64 ogg_int64_t;
typedef __int32 ogg_int32_t;
typedef unsigned __int32 ogg_uint32_t;
typedef __int16 ogg_int16_t;

#else

/* Cygwin */
#include <_G_config.h>
typedef _G_int64_t ogg_int64_t;
typedef _G_int32_t ogg_int32_t;
typedef unsigned _G_int32_t ogg_uint32_t;
typedef _G_int16_t ogg_int16_t;
#endif
#else

#ifdef __BEOS__
/* Be */
#include <inttypes.h>
#endif

#include <sys/types.h>

/* filled in by configure */
typedef signed short ogg_int16_t;
typedef signed long ogg_int32_t;
typedef unsigned long ogg_uint32_t;
typedef __int64 ogg_int64_t;

#endif

#endif 

