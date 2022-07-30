/**************************************************************************
 *
 *  Copyright 2022, Yuri Prokushev
 *
 *  This file is part of osFree project
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#ifndef smstd_h
#define smstd_h

#ifdef __cplusplus      /* Turn off inlining of strcmp, etc for C++ */
#undef __STR__
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>

#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif

/*
 *  Standard typedefs:
 */
//typedef int bool;

#define global /* Maps onto nothing, used as a prefix to global variables */

/*
 *  Standard NULL casts:
 */
#ifndef NULL
#define NULL   0
#endif
#define CNULL       ((char *)0)
#define FNULL       ((FILE *)0)
#define VNULL       ((void *)0)
#define INULL       ((int  *)0)

#endif /* smstd_h */
