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
SOMEXTERN SOMDLLIMPORT FILE *  somtopenEmitFile(char *file, char *ext);
SOMEXTERN SOMDLLIMPORT int  somtfclose(FILE *fp);
SOMEXTERN SOMDLLIMPORT void somterror(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void somtfatal(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void somtinternal(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void somtmsg(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void somtresetEmitSignals();
SOMEXTERN SOMDLLIMPORT void somtunsetEmitSignals(void);
SOMEXTERN SOMDLLIMPORT void somtwarn(char *file, long lineno, char *fmt, ...);
#endif

SOMEXTERN SOMDLLIMPORT FILE * SOMLINK somtopenEmitFileSL(char *file, char *ext);
SOMEXTERN SOMDLLIMPORT int SOMLINK somtfcloseSL(FILE *fp);
SOMEXTERN SOMDLLIMPORT void SOMLINK somterrorSL(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtfatalSL(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtinternalSL(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtmsgSL(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtresetEmitSignalsSL();
SOMEXTERN SOMDLLIMPORT void SOMLINK somtunsetEmitSignalsSL(void);
SOMEXTERN SOMDLLIMPORT void SOMLINK somtwarnSL(char *file, long lineno, char *fmt, ...);

#ifdef __cplusplus
 }
#endif

#endif
