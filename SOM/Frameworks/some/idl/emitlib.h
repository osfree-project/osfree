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

#ifndef SOM_SOMC_NO_BACKCOMPAT
 FILE *  somtopenEmitFile(char *file, char *ext);
 int  somtfclose(FILE *fp);
 void somterror(char *file, long lineno, char *fmt, ...);
 void somtfatal(char *file, long lineno, char *fmt, ...);
 void somtinternal(char *file, long lineno, char *fmt, ...);
 void somtmsg(char *file, long lineno, char *fmt, ...);
 void somtresetEmitSignals();
 void somtunsetEmitSignals(void);
 void somtwarn(char *file, long lineno, char *fmt, ...);
 SOMTEntryC * somtGetObjectWrapper(Entry * ep);
#endif

FILE * SOMLINK somtopenEmitFileSL(char *file, char *ext);
int SOMLINK somtfcloseSL(FILE *fp);
void SOMLINK somterrorSL(char *file, long lineno, char *fmt, ...);
void SOMLINK somtfatalSL(char *file, long lineno, char *fmt, ...);
void SOMLINK somtinternalSL(char *file, long lineno, char *fmt, ...);
void SOMLINK somtmsgSL(char *file, long lineno, char *fmt, ...);
void SOMLINK somtresetEmitSignalsSL();
void SOMLINK somtunsetEmitSignalsSL(void);
void SOMLINK somtwarnSL(char *file, long lineno, char *fmt, ...);
SOMTEntryC * SOMLINK somtGetObjectWrapperSL(Entry * ep);

#ifdef __cplusplus
 }
#endif

#endif
