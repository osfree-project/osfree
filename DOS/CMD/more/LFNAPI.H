/* lfnapi.h */

/* Copyright (C)      2007, Imre Leber <imre.leber at telenet.be> */
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA
*/

#ifndef LFN_API_H_
#define LFN_API_H_

int LFNConvertToSFN(char* file);
int LFNFirstFile(char* wildcard, char* file, char* longfile);
int LFNNextFile(char* file, char* longfile);
void LFNFindStop(void);

/* In LFNCHK.ASM */
int CheckDriveOnLFN (char drive);

#endif
