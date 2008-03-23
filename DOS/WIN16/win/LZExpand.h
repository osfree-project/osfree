/***************************************************************************

	LZExpand.h
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

*************************************************************************/

#define LZAPI
int     LZAPI LZStart(void);
void    LZAPI LZDone(void);
LONG    LZAPI CopyLZFile(HFILE, HFILE);
LONG    LZAPI LZCopy(HFILE, HFILE);
HFILE   LZAPI LZInit(HFILE);
int     LZAPI GetExpandedName(LPCSTR, LPSTR);
HFILE   LZAPI LZOpenFile(LPCSTR, OFSTRUCT FAR*, UINT);
LONG    LZAPI LZSeek(HFILE, LONG, int);
int     LZAPI LZRead(HFILE, void FAR*, int);
void    LZAPI LZClose(HFILE);

#define LZERROR_BADINHANDLE   (-1)  /* invalid input handle */
#define LZERROR_BADOUTHANDLE  (-2)  /* invalid output handle */
#define LZERROR_READ          (-3)  /* corrupt compressed file format */
#define LZERROR_WRITE         (-4)  /* out of space for output file */
#define LZERROR_GLOBALLOC     (-5)  /* insufficient memory for LZFile struct */
#define LZERROR_GLOBLOCK      (-6)  /* bad global handle */
#define LZERROR_BADVALUE      (-7)  /* input parameter out of range */
#define LZERROR_UNKNOWNALG    (-8)  /* compression algorithm not recognized */

typedef struct tagLZFILE {
	BOOL	bOpen;
	HFILE	hFile;			/* uncompressed image of LZ file */
	char	szDeleteFile[255];
} LZFILE;

typedef LZFILE		*PLZFILE;
typedef LZFILE NEAR	*NPLZFILE;
typedef LZFILE FAR	*LPLZFILE;




