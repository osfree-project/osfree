/********************************************************************
	@(#)WinConfig.h	1.12 Twin configuration file management.
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

********************************************************************/
 
 
char *GetTwinFilename(void);
char *GetTwinString(int, char *,int );
unsigned long GetTwinInt(int);
char *GetEnv(const char *env);


/* WinConfig Sections definitions */
#define WCS_BOOT        0
#define WCS_WINDOWS     1
#define WCS_XDOS        2
#define WCS_COMMDLG     3
#define WCS_ENV      	4

/* WinConfig Parameter definitions */
#define	WCP_DISPLAY	1
#define	WCP_WINDOWS	4
#define	WCP_TEMP	5
#define	WCP_OPENPATH	6
#define	WCP_TASKING	7
#define WCP_FATAL	8

#define	WCP_DBLCLICK	9
#define	WCP_FONTFACE	10
#define	WCP_FONTSIZE	11
#define	WCP_FONTBOLD	12

#define	WCP_DOSMODE	13
#define WCP_MEMORY	14
#define WCP_EXTENDED	15

#define WCP_ICONFONTFACE 16
#define WCP_ICONFONTSIZE 17

#define WCP_DOSDRIVES		18
#define WCP_DRIVELETTERS	19
#define WCP_PATHSASDRIVES	20
#define WCP_CONTROL	21

#define WCP_MAXIMUM     21

typedef struct twinrc {
	int     parameter;
	int	opcode;
	int     section;
	char   *lpszkeyname;
	char   *lpszdefault;
	char   *lpszenviron;
	long    lparam;
	char   *lpszstring;
} TWINRC, *LPTWINRC;

/*
**   MiD 03-JAN-1996   The first 3 fontmapper flags have been
**                     moved from DrvText.h, plus added Scalable 
**                     Only flag. 
*/
#define FM_HIRESFONT    0x10   /* use highest resolution fonts only        */
#define FM_NOSCALABLE   0x20   /* do not allow scalable font mapping       */
#define FM_SYSTEMFONT   0x40   /* use system font in dialog boxes w/ fonts */
#define FM_SCALABLEONLY 0x80   /* skip bitmap fonts: overwrites FM_NOSCALABLE */

