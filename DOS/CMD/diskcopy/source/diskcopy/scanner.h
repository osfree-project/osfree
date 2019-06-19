/*
   DISKCOPY.EXE, floppy diskette duplicator similar to MSDOS Diskcopy.
   Copyright (C) 1998, Matthew Stanford.
   Copyright (C) 1999, 2000, 2001 Imre Leber.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have recieved a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


   If you have any questions, comments, suggestions, or fixes please
   email me at: imre.leber@worldonline.be

 */

#ifndef SCANNER_H_
#define SCANNER_H_

/* Return codes. */
#define SCANFILENOTFOUND  0
#define SCANFILEERROR    -1
#define SCANSUCCESS       1

/* ini file name. */
#define INIFILENAME "diskcopy.ini"

int OpenScannerFile (char *filename);
void CloseScanner (void);
void RestartScanning (char *filename);
TOKEN GetNextToken (void);
char *ScannerString (void);
TOKEN PeekToken (void);
int GetScannerLine (void);

char *GetIniDir (char *filename, char *buffer);

#endif
