
/*
 *      Copyright (C) 2000 Christoph Schulte M”nting.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *      GNU General Public License for more details.
 */

/*
 *@@sourcefile resh.h:
 *      header file for resh.c. See remarks there.
 */

#ifndef RESH_H_INCLUDED
#define RESH_H_INCLUDED

#include <os2.h>

#ifdef __cplusplus
extern "C" {
#endif

APIRET reshWriteResourceToFile(HMODULE hmod,
                               ULONG  ulTypeID,
                               ULONG  ulNameID,
                               const char *pcszFilename,
                               PULONG pulBytesWritten);

#ifdef __cplusplus
}
#endif

#endif /* not RESH_H_INCLUDED */

