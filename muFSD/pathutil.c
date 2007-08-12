//
// $Header: /cur/cvsroot/boot/muFSD/pathutil.c,v 1.2 2006/11/24 11:43:13 valerius Exp $
//

// 32 bits OS/2 device driver and IFS support. Provides 32 bits kernel
// services (DevHelp) and utility functions to 32 bits OS/2 ring 0 code
// (device drivers and installable file system drivers).
// Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifdef __IBMC__
#pragma strings(readonly)
#endif

#ifndef MICROFSD
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2/os2.h>                // From the "Developer Connection Device Driver Kit" version 2.0

#include <os2/types.h>
//#include <os2/os2proto.h>
#endif /* #ifndef MICROFSD */


char * DecoupePath(char * path, char * component)
{
    char * tmp1;
    char * tmp2;
    char * tmp3;


    tmp1 = path;
    while ((*tmp1 != '\\') && (*tmp1 != '/') && (*tmp1 != '\0')) tmp1++;
    if (*tmp1 == '\0') return 0;
    tmp1++;
    if (*tmp1 == '\0') return 0;
    tmp3 = tmp1;
    tmp2 = component;
    while ((*tmp1 != '\\') && (*tmp1 != '/') && (*tmp1 != '\0')) {
        *component = *tmp1;
        tmp1++;
        component ++;
    }
    *component = 0;
    return tmp3;

}

#ifndef MICROFSD
void ExtractPath(char * Name, char * Path)
{
    int i, j;

    i = 0;
    while (Name[i] != 0) i++;

    while ((Name[i] != '\\') && (Name[i] != '/') && i > 0) {
        i--;
    }
    for (j = 0; j < i; j++) Path[j] = Name[j];
    Path[i] = 0;

}

void ExtractName(char * Name, char * Nom)
{
    int i, j, k;

    i = 0;
    while (Name[i] != 0) i++;
    j = i;

    while ((Name[j] != '\\') && (Name[j] != '/') && j > 0) {
        j--;
    }
    for (k = 0; k < i - j; k++) Nom[k] = Name[j + k + 1];

}
#endif /* #ifndef MICROFSD */
