/*
	@(#)DrvPrinting.c	1.3
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

 */


#ifdef __MWERKS__
	#include <Printing.h>
	#include <Quickdraw.h>
	#include <Dialogs.h>
	#include <OSUtils.h>
	#include <ToolUtils.h>
	#include <Files.h>
	#include <MixedMode.h>
#endif

#include <string.h>
#include "Driver.h"

#define PostScriptBegin	190
#define PostScriptEnd		191
#define PostScriptHandle	192
#define PostScriptFile		193

/*****************************P R O T O T Y P E S ********************/

	short	ModalDLOGFilter(void *theDialog, void *theEvent,  short *itemHit);

	short PrintAFile( );

	void FlushGrafPortState();

	void * OpenPrinterPort();

	short SendData( char *, char *buff, long buffLen);

	short ClosePrinterPort();

	void	OpenPage();

	void	ClosePage();

DWORD DrvPrintingTab(void);
static DWORD DrvPrintingDoNothing(LPARAM,LPARAM,LPVOID);

/**********************F U N C T I O N S ***************************
				CONTAINS STUBS FOR ALL FUNCTIONS ONLY!!!!!!!!!!!!!
************************************************************/

/*********************OpenPrinterPort********************/


/********************** ************************/

static TWINDRVSUBPROC DrvPrintingEntryTab[] = { /* Nothing defined */ 
	DrvPrintingDoNothing,
	DrvPrintingDoNothing,
	DrvPrintingDoNothing,
	DrvPrintingDoNothing,
	DrvPrintingDoNothing,
	DrvPrintingDoNothing
};

DWORD
DrvPrintingTab(void)
{
#if defined(TWIN_RUNTIME_DRVTAB)
        DrvPrintingEntryTab[0] = DrvPrintingDoNothing;
        DrvPrintingEntryTab[1] = DrvPrintingDoNothing;
        DrvPrintingEntryTab[2] = DrvPrintingDoNothing;
        DrvPrintingEntryTab[3] = DrvPrintingDoNothing;
        DrvPrintingEntryTab[4] = DrvPrintingDoNothing;
        DrvPrintingEntryTab[5] = DrvPrintingDoNothing;
#endif
	return (DWORD)DrvPrintingEntryTab;
}

static DWORD
DrvPrintingDoNothing(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 0L;
}

/****************OpenPage ************************/
/*
*/
/********************** ************************/

void OpenPage( void){
} /*OpenPage*/

/****************ClosePage ************************/
/*
*/
/********************** ************************/

void ClosePage( ){

}/* ClosePage */

/****************SendData ************************/
/*
*/
/********************** ************************/
short SendData( char *empty, char *buff, long buffLen){
	
	return -1;
 
} /* SendData*/

/****************ClosePrinterPort ************************/
/*
*/
/********************** ************************/

short ClosePrinterPort(){
	return -1;

} /*  ClosePrinterPort */

/****************FlushGrafPortState ************************/
/*
*/
/********************** ************************/
void FlushGrafPortState()
{

} /*  FlushGrafPortState  */

/****************ModalDLOGFilter ************************/
/*
*/
/********************** ************************/
short ModalDLOGFilter(void *theDialog, void *theEvent,  short *itemHit){

	return -1;
}
