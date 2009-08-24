// OS2TTY.C - OS/2 tty i/o routines for 4OS2
//   Copyright (c) 1992 - 1996  Rex C. Conn   All rights reserved

#include "product.h"

#include <conio.h>
#include <ctype.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "4all.h"


// get a keystroke from STDIN, with optional echoing & CR/LF
unsigned int GetKeystroke( int eflag )
{
	unsigned int c;
	KBDKEYINFO keyinfo;

	// if console redirected, we can't use KbdCharIn()
	if (( QueryIsConsole( STDIN ) == 0 ) && (( eflag & EDIT_BIOS_KEY ) == 0 ))
		c = getchar();

	else {
		// C Set/2 has a bug with ^C and getche()
		// wait for a key, and check for extended scan code
		do {
			(void)KbdCharIn( &keyinfo, (USHORT)0, (HKBD)0 );
			c = (((keyinfo.fbStatus & 2) && ((keyinfo.chChar == 0) || (keyinfo.chChar == 0xE0))) ? (keyinfo.chScan | 0x100) : keyinfo.chChar);
		} while ( c == 0 );
	}

	// check for upper case requested
	c = ((eflag & EDIT_UC_SHIFT) ? _ctoupper(c) : c);

	if ((eflag & EDIT_NO_ECHO) == 0)
		qputc( STDOUT, (char)c );

	// check for CRLF requested
	if (eflag & EDIT_ECHO_CRLF)
		crlf();

	return c;
}


// get the cursor location
void GetCurPos(int *row, int *col)
{
	*row = *col = 0;
	(void)VioGetCurPos( (PUSHORT)row, (PUSHORT)col, 0 );
}


// return the character attribute at the current cursor position
void GetAtt(unsigned int *normal, unsigned int *inverse)
{
	CHAR achCells[2];
	USHORT cb = sizeof(achCells);
	USHORT row = 0, column = 0;

	(void)VioGetCurPos( &row, &column, 0 );
	(void)VioReadCellStr( achCells, &cb, row, column, 0 );
	*normal = achCells[1];

	// remove blink & intensity bits, and check for black on black
	if ((*inverse = (achCells[1] & 0x77)) == 0)
		*inverse = 0x70;
	else {
		// ROR 4 to get inverse
		*inverse = (*inverse >> 4) + ((*inverse << 4) & 0xFF);
	}
}


// position the cursor
void SetCurPos(int row, int col)
{
	(void)VioSetCurPos( row, col, (HVIO)0 );
}


// Scroll the window up or down
void Scroll(int ulrow, int ulcol, int lrrow, int lrcol, int mode, int attrib)
{
	BYTE bBlank[2];

	bBlank[0] = ' ';
	bBlank[1] = (char)attrib;

	if (mode > 0)
		(void)VioScrollUp(ulrow,ulcol,lrrow,lrcol,1,bBlank,0);
	else
		(void)VioScrollDn(ulrow,ulcol,lrrow,lrcol,((mode == 0) ? (lrrow-ulrow) + 1 : 1),bBlank,(HVIO)0);
}


// write the specified character & attribute to the specified cursor position
void WriteChrAtt(int row, int col, int attrib, int c)
{
	CHAR achCells[2];

	achCells[0] = (char)c;
	achCells[1] = (char)attrib;
	(void)VioWrtNCell( achCells, 1, row, col, 0 );
}


// display the string with the specified attribute
void WriteStrAtt(int row, int col, int attrib, char *str)
{
	(void)VioWrtCharStrAtt( str, strlen( str ), row, col, (PBYTE)&attrib, 0 );
}


// display the string vertically with the specified attribute
void WriteVStrAtt(int row, int col, int attrib, char *str)
{
	char cell[2];

	for (cell[1] = (char)attrib; (*str != '\0'); str++, row++) {
		cell[0] = *str;
		(void)VioWrtNCell( (PCH)cell, 1, row, col, 0 );
	}
}


// display the string at the current cursor position
void WriteTTY(char *str)
{
	(void)VioWrtTTY( str, strlen(str), 0 );
}


// change the screen attributes on the specified line
void SetLineColor(int row, int column, int length, int attrib)
{
	(void)VioWrtNAttr( (PBYTE)&attrib, length, row, column, 0 );
}


// return the current number of screen rows (make it 0 based)
unsigned int GetScrRows(void)
{
	VIOMODEINFO viomiMode;

	if (gpIniptr->Rows != 0)
		return (gpIniptr->Rows - 1);

	viomiMode.cb = sizeof( viomiMode );
	(void)VioGetMode( &viomiMode, 0 );

	return (viomiMode.row - 1);
}


// return the current number of screen columns
unsigned int GetScrCols(void)
{
	VIOMODEINFO vioMode;

	vioMode.cb = sizeof( vioMode );
	(void)VioGetMode( &vioMode, 0 );

	return vioMode.col;
}


// set the cursor shape; the defaults are in gpIniptr (CursO & CursI)
// "c_type" defaults are:
//	0 - overstrike
//	1 - insert
void SetCurSize( int c_type )
{
	VIOCURSORINFO viociCursor;

	// if CursO or CursI == -1, don't attempt to modify cursor at all
	if ((gpIniptr->CursO != -1) && (gpIniptr->CursI != -1)) {

	    // if we're defaulting to insert mode, reverse the "c_type" meaning
	    if (gpIniptr->EditMode)
		c_type = !c_type;

	    // set the end & start lines for the cursor cell (as percentages)
	    viociCursor.cEnd = -100;
	    viociCursor.yStart = ((c_type == 0) ? gpIniptr->CursO : gpIniptr->CursI);

	    // set the attribute (-1 = hidden, anything else is normal)
	    viociCursor.attr = ((viociCursor.yStart == 0) ? -1 : 0);
	    viociCursor.yStart -= 100;
	    viociCursor.cx = 0;		// default character width
	    (void)VioSetCurType( &viociCursor, 0 );
	}

	SetBrightBG();
}


// Set bright background / blink foreground
void SetBrightBG( void )
{
	struct {
		USHORT cb;
		USHORT type;
		USHORT fs;
	} VioIntensity;

	// check for no background/blinking diddling requested
	// or if in a windowed session
	if (( gpIniptr->BrightBG == 2 ) || ( gpLIS->typeProcess == PT_WINDOWABLEVIO ))
		return;

	// set bright/blink background
	VioIntensity.fs = gpIniptr->BrightBG;
	VioIntensity.cb = 6;
	VioIntensity.type = 2;
	VioSetState( &VioIntensity, 0 );
}

