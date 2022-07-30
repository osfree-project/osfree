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
#ifdef _WIN32
#include <windows.h>
#endif
#include <io.h>	
#include <stdio.h>
#include <stdlib.h>
#include <sm.h>
#include <som.h>
#ifdef __cplusplus
 extern "C" {
#endif

#include <optlink.h>

// Here old Optlink caling convention functions having SOMLINK equals
#ifndef SOM_SOMC_NO_BACKCOMPAT
SOMEXTERN SOMDLLIMPORT BOOL OPTLINK_DECL somtfexists(char *file);
SOMEXTERN SOMDLLIMPORT FILE * OPTLINK_DECL somtopenEmitFile(char *file, char *ext);
SOMEXTERN SOMDLLIMPORT int  OPTLINK_DECL somtfclose(FILE *fp);
SOMEXTERN SOMDLLIMPORT void OPTLINK_DECL somterror(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void OPTLINK_DECL somtfatal(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void OPTLINK_DECL somtinternal(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void OPTLINK_DECL somtmsg(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT void OPTLINK_DECL somtresetEmitSignals();
SOMEXTERN SOMDLLIMPORT void OPTLINK_DECL somtunsetEmitSignals(void);
SOMEXTERN SOMDLLIMPORT void OPTLINK_DECL somtwarn(char *file, long lineno, char *fmt, ...);
SOMEXTERN SOMDLLIMPORT char * OPTLINK_DECL somtsearchFile(char *file, char *fullpath, char *env);
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
SOMEXTERN SOMDLLIMPORT BOOL SOMLINK somtfexistsSL(char *file);
SOMEXTERN SOMDLLIMPORT char * SOMLINK somtsearchFileSL(char *file, char *fullpath, char *env);

#ifdef __cplusplus
 }
#endif

#endif
