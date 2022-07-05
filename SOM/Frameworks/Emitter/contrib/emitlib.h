/*
 *   COMPONENT_NAME: somc
 *
 *   ORIGINS: 27
 *
 *
 *   10H9767, 10H9769  (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* @(#) somc/emitlib.pub 2.7 6/7/96 14:29:25 [12/22/96 21:11:33] */


/*
 *         File:    emitlib.h.
 *     Contents:    General routines used by Emitter Framework.
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
