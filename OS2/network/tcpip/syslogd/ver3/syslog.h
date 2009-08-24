/*  
**  Futher modified by Mikael St†ldal <d96-mst@nada.kth.se>.
**  Modified by Jochen Friedrich <jochen@audio.ruessel.sub.org> for OS/2.
*/
/* $Revision: 1.3 $
 * Copyright (c) 1982, 1986, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution is only permitted until one year after the first shipment
 * of 4.4BSD by the Regents.  Otherwise, redistribution and use in source and
 * binary forms are permitted provided that: (1) source distributions retain
 * this entire copyright notice and comment, and (2) distributions including
 * binaries display the following acknowledgement:  This product includes
 * software developed by the University of California, Berkeley and its
 * contributors'' in the documentation or other materials provided with the
 * distribution and in all advertising materials mentioning features or use
 * of this software.  Neither the name of the University nor the names of
 * its contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)syslog.h	7.16 (Berkeley) 6/28/90
 */

#ifndef SYSLOG_H
#define SYSLOG_H

#include <sys/syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

void syslog(int pri, const char *fmt, ...);
void vsyslog(int pri, const char *fmt, va_list ap);
void openlog(const char *ident, int logstat, int logfac);
void closelog(void);
int setlogmask(int pmask);

#ifdef __cplusplus
}
#endif

#endif  /* SYSLOG */
