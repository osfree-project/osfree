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

#ifndef emitlib_h
#define emitlib_h

#include <stdio.h>
#include <stdlib.h>
#include <sm.h>
#include <som.h>
#ifdef __cplusplus
 extern "C" {
#endif
 FILE * SOMLINK somtopenEmitFile(char *file, char *ext);
 int SOMLINK somtfclose(FILE *fp);
 void SOMLINK somterror(char *file, long lineno, char *fmt, ...);
 void SOMLINK somtfatal(char *file, long lineno, char *fmt, ...);
 void SOMLINK somtinternal(char *file, long lineno, char *fmt, ...);
 void SOMLINK somtmsg(char *file, long lineno, char *fmt, ...);
 void SOMLINK somtresetEmitSignals();
 void SOMLINK somtunsetEmitSignals(void);
 void SOMLINK somtwarn(char *file, long lineno, char *fmt, ...);
 SOMTEntryC * SOMLINK somtGetObjectWrapper(Entry * ep);
#ifdef __cplusplus
 }
#endif

#endif
