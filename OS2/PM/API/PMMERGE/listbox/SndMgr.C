#pragma	title("List Box Replacement  --  Version 1.1 -- (SndMgr.C)")
#pragma	subtitle("   Sound Manager - Interface Definitions")

/* Program name: Listbox.Dll  Title: A List Box	Replacement		*/
/*									*/
/* OS/2	Developer Magazine, Issue:  May	'94, page 12                    */
/* Author:  Mark Benge	   IBM Corp.					*/
/*	    Matt Smith	   Prominare Inc.				*/
/* Description:	 Replacement for OS/2 List Box,	first of a series.	*/
/*									*/
/* Program Requirements:  OS/2 2.x					*/
/*			  IBM C	Set++					*/
/*			  WATCOM C 386/9.0				*/
/*			  Borland C++ for OS/2				*/
/*			  OS/2 Toolkit					*/

/* Copyright ¸ International Business Machines Corp. 1991-1994		*/
/* Copyright ¸ 1989-1994  Prominare Inc.  All Rights Reserved.		*/

/************************************************************************/
/************************************************************************/
/*		       DISCLAIMER OF WARRANTIES.			*/
/************************************************************************/
/************************************************************************/
/*     The following [enclosed]	code is	source code created by the	*/
/*     authors.	 This source code is  provided to you solely		*/
/*     for the purpose of assisting you	in the development of your	*/
/*     applications.  The code is provided "AS IS", without		*/
/*     warranty	of any kind.  The authors shall	not be liable		*/
/*     for any damages arising out of your use of the source code,	*/
/*     even if they have been advised of the possibility of such	*/
/*     damages.	 It is provided	purely for instructional and		*/
/*     illustrative purposes.						*/
/************************************************************************/
/************************************************************************/

#pragma	info(noext)
#pragma	strings(readonly)

#define	INCL_DOS		   /* Include OS/2 DOS Kernal		*/
#define	INCL_GPI		   /* Include OS/2 PM GPI Interface	*/
#define	INCL_WIN		   /* Include OS/2 PM Windows Interface	*/

#include <os2.h>
#include <setjmp.h>
#include <string.h>

#include "listbox.h"

/* This	module contains	the routines that handle the sound management	*/
/* for the list	box.							*/
/*									*/
/* Equivalent command line invocation of each module using the		*/
/* IBM C Set++ Compiler	Version	2.0 is:					*/
/*									*/
/*     Icc -G3e- -O+ -Rn -C -W3	-FoSndMgr SndMgr.C			*/

/* Filename:   SndMgr.C							*/

/*  Version:   1.1							*/
/*  Created:   1994-02-27						*/
/*  Revised:   1994-04-23						*/

/* Routines:   VOID LoadSoundSupport(PLISTBOXWIN plbw);			*/
/*	       VOID PlaySound(PLISTBOXWIN plbw,	ULONG ulSound);		*/
/*	       VOID UnloadSoundSupport(PLISTBOXWIN plbw);		*/
/*	       VOID SetControlDataSounds(PLISTBOXWIN plbw, ULONG cItems,*/
/*					 ULONG cSounds,	PBYTE pb);	*/
/*	       VOID SetControlDataSounds(PLISTBOXWIN plbw,		*/
/*					 ULONG cItems, ULONG cSounds	*/
/*					 PBYTE pb);			*/
/*	       MRESULT EXPENTRY	mrSoundHandler(HWND hWnd, ULONG	msg,	*/
/*					       MPARAM mp1, MPARAM mp2);	*/


/* --------------------------------------------------------------------	*/

/* Sound List Box messages						*/
/*									*/
/*     LMXM_SETSOUNDEVENT	  0x0396UL				*/
/*     LMXM_QUERYSOUNDEVENT	  0x0397UL				*/

extern jmp_buf jBuf;		   /* Jump Buffer			*/

/************************************************************************/
/*									*/
/* Module Prototype Definitions						*/
/*									*/
/************************************************************************/

APIRET APIENTRY	ListBoxExceptionHandler(PEXCEPTIONREPORTRECORD pxcptrepr,
					PEXCEPTIONREGISTRATIONRECORD pxcptregr,
					PCONTEXTRECORD pcr, PVOID sysinfo);

#pragma	subtitle("   Sound Manager - Sound Support Loading Function")
#pragma	page( )

/* --- LoadSoundSupport	-------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	dynamically load the sound support DLL	*/
/*     that will allow the associating of sounds with the listbox.	*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Internal Data Pointer		*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID LoadSoundSupport(PLISTBOXWIN plbw)

{
CHAR szFile[CCHMAXPATH];	   /* Failing Component	Filename	*/

		       /* Try loading the sound	support	DLL.  If MMPM/2	*/
		       /* has been loaded, the DLL will	be successfully	*/
		       /* loaded.  The routines	within depend on the	*/
		       /* handle being either zero (0) which indicates	*/
		       /* sound	support	is not found or	a non-zero	*/
		       /* indicating that the soupport DLL was found	*/
		       /* and loaded and that sounds can be associated	*/
		       /* with list box	events and played.		*/

if ( DosLoadModule(szFile, CCHMAXPATHCOMP, "LBSnd", &plbw->hmodSnd) )

		       /* Sound	support	DLL could not be loaded.  The	*/
		       /* value	within the szFile variable will	contain	*/
		       /* the failing component.  Set the module handle	*/
		       /* to zero to indicate that sound events	are	*/
		       /* not to use MMPM/2 wave files.			*/
   plbw->hmodSnd = 0UL;
else
		       /* Try loading the LoadWaveFile function	entry	*/
		       /* point						*/

   if (	DosQueryProcAddr(plbw->hmodSnd,	ORD_LOADWAVEFILE, NULL,
			 (PFN *)(PVOID)&plbw->pfnLoadWaveFile) )
       {
		       /* Ordinal not found within the DLL, release the	*/
		       /* sound	support	DLL and	clear the module handle	*/
		       /* to indicate sound events cannot be used	*/

       DosFreeModule(plbw->hmodSnd);
       plbw->hmodSnd = 0UL;
       }
   else
		       /* Try loading the UnloadWave function entry	*/
		       /* point						*/

       if ( DosQueryProcAddr(plbw->hmodSnd, ORD_UNLOADWAVE, NULL,
			     (PFN *)(PVOID)&plbw->pfnUnloadWave) )
	   {
		       /* Ordinal not found within the DLL, release the	*/
		       /* sound	support	DLL and	clear the module handle	*/
		       /* to indicate sound events cannot be used	*/

	   DosFreeModule(plbw->hmodSnd);
	   plbw->hmodSnd = 0UL;
	   }
       else
		       /* Try loading the PlayWav function entry point	*/

	   if (	DosQueryProcAddr(plbw->hmodSnd,	ORD_PLAYWAVE, NULL,
				 (PFN *)(PVOID)&plbw->pfnPlayWave) )
	       {
		       /* Ordinal not found within the DLL, release the	*/
		       /* sound	support	DLL and	clear the module handle	*/
		       /* to indicate sound events cannot be used	*/

	       DosFreeModule(plbw->hmodSnd);
	       plbw->hmodSnd = 0UL;
	       }
}
#pragma	subtitle("   Sound Manager - Sound Player Function")
#pragma	page( )

/* --- PlaySound --------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	play the associated sound if one is	*/
/*     defined for an event and	the sound support is present.		*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;    = List Box Internal	Data Pointer		*/
/*     ULONG	   ulSound; = Sound Event				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID PlaySound(PLISTBOXWIN plbw, ULONG ulSound)

{
		       /* Check	to see if the sound file was loaded	*/
		       /* before trying	to unload it.  When the	handle	*/
		       /* for the sound	module is 0, then the sound	*/
		       /* support DLL was not loaded.			*/
if ( plbw->hmodSnd )
   switch ( ulSound )
       {
		       /* Defining the single click wave file		*/

       case LSND_SINGLECLICK :
	   if (	plbw->ulSClkWAV	)
	       plbw->pfnPlayWave(plbw->ulSClkWAV);
	   break;
		       /* Defining the double click wave file		*/

       case LSND_DOUBLECLICK :
	   if (	plbw->ulDClkWAV	)
	       plbw->pfnPlayWave(plbw->ulDClkWAV);
	   break;
       }
}
#pragma	subtitle("   Sound Manager - Sound Support Unload Procedure")
#pragma	page( )

/* --- UnloadSoundSupport ------------------------------ [ Public ] ---	*/
/*									*/
/*     This function is	used to	release	the loaded sound support DLL.	*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Internal Data Pointer		*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID UnloadSoundSupport(PLISTBOXWIN plbw)

{
		       /* Check	to see if the sound file was loaded	*/
		       /* before trying	to unload it.  When the	handle	*/
		       /* for the sound	module is 0, then the sound	*/
		       /* support DLL was not loaded.			*/
if ( plbw->hmodSnd )
   {
		       /* See if the single click wave file was	loaded	*/
		       /* and if so, close the .WAV file		*/
   if (	plbw->ulSClkWAV	)
       plbw->pfnUnloadWave(plbw->hWnd, plbw->ulSClkWAV);

		       /* See if the double click wave file was	loaded	*/
		       /* and if so, close the .WAV file		*/

   if (	plbw->ulDClkWAV	)
       plbw->pfnUnloadWave(plbw->hWnd, plbw->ulDClkWAV);

		       /* Release the sound support DLL			*/

   DosFreeModule(plbw->hmodSnd);
   }
}
#pragma	subtitle("   List Manager - List Box Control Data Procedure")
#pragma	page( )

/* --- SetControlDataSounds ---------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	decode the control data	that may form	*/
/*     part the	of the WM_CREATE message.  The control data is		*/
/*     designed	to allow the user to specify a list of wave files that	*/
/*     are to be used with the single and double click events.	The	*/
/*     first holder is for the single click wave file and the second	*/
/*     holder is for the double	click wave file.  If a preceeding wave	*/
/*     is not specified, the NULL byte is still	defined	for the	holder.	*/
/*     Therefore if the	first holder is	empty and the second holder	*/
/*     defines a wave file, the	following would	be the format of the	*/
/*     resultant data:							*/
/*									*/
/*     \0Filename.Wav\0							*/
/*									*/
/*     The design also allows for only the necessary items to be	*/
/*     specified.  For example,	if only	the single click event is	*/
/*     to be associated	with a wave file, then only it needs to	be	*/
/*     defined within the list and the list count will be 1.  If no	*/
/*     items are to be defined within the list,	then a count of	0 is	*/
/*     also valid.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;    = List Box Internal	Data Pointer		*/
/*     ULONG	   cItems;  = Items in Control Data List		*/
/*     ULONG	   cSounds; = Items in Sounds Control Data List		*/
/*     PBYTE	   pb;	    = Control Data Wave	List Pointer		*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID SetControlDataSounds(PLISTBOXWIN plbw, ULONG cItems, ULONG	cSounds,
			  PBYTE	pb)

{
HHEAPMEM hHeap;			   /* Heap Handle			*/
register INT i;			   /* Loop Counter			*/
register UINT n;		   /* Length Holder			*/
EXCEPTIONREGISTRATIONRECORD xcptregr;	   /* Exception	Record		*/

if ( !cItems &&	!cSounds )
   return;
		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

xcptregr.prev_structure	  = NULL;
xcptregr.ExceptionHandler = &ListBoxExceptionHandler;

DosSetExceptionHandler(&xcptregr);

		       /* Set the jump buffer and check	to see if an	*/
		       /* exception has	occurred in which case ignore	*/
		       /* the decoding of the control data and return	*/
if ( setjmp(jBuf) )
   {
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

   DosUnsetExceptionHandler(&xcptregr);
   return;
   }

if ( cItems )
   {
		       /* Starting off the list	box from scratch,	*/
		       /* create the new heap that is to be used for	*/
		       /* list box.  Allocate the memory needed	for the	*/
		       /* control information and save the heap	handle	*/
		       /* within it.					*/

   plbw->plc = (PLISTCOL)HeapMalloc(hHeap = HeapAlloc(8192UL, 4096UL), sizeof(LISTCOL));

		       /* Allocate memory for the initial start	of the	*/
		       /* list.	 The list is allocated in groups of 8,	*/
		       /* hence	whenever the list total	is a multiple	*/
		       /* of 8,	the current list is full and needs to	*/
		       /* be expanded.					*/

   plbw->plc[0].apli = (PLISTITEM *)HeapMalloc(plbw->plc[0].hHeap = hHeap,
					       ((cItems	/ CBLK_LIST) + 1UL) * CBLK_LIST	* sizeof(PLISTITEM));

		       /* Set the text of the item being inserted at	*/
		       /* the end of the list box list and check to see	*/
		       /* if the item is wider than the	rest in	which	*/
		       /* case the maximum item	width should be	updated	*/

   for ( i = 0;	i < cItems; i++	)
       {
       if ( (lSetItemText(plbw->plc[0].hHeap, plbw->hWnd, plbw->plc[0].apli[i],
			  pb) >	plbw->cxItem) && (plbw->flStyle	& LS_HORZSCROLL) )
	   plbw->cxItem	= plbw->plc[0].apli[i]->cxItem;
       pb += strlen(pb)	+ 1;
       }
   }

if ( cSounds )
   {
		       /* Check	to see if the single click holder a	*/
		       /* NULL byte which indicates that the single	*/
		       /* click	event is to be non-associated with a	*/
		       /* wave file.  When a value is present, get the	*/
		       /* name of the wave file	and save it internally	*/
		       /* while	also opening the wave file for usage by	*/
		       /* the list box control.				*/
   if (	pb[0] )
       {
       plbw->ulSClkWAV = plbw->pfnLoadWaveFile(plbw->hWnd,
					       memcpy(plbw->szSClkWAV,
					       pb, n = strlen(pb)));
       pb += n + 1;
       }
   else
       ++pb;
		       /* Check	to see if the double click holder a	*/
		       /* NULL byte which indicates that the double	*/
		       /* click	event is to be non-associated with a	*/
		       /* wave file.  When a value is present, get the	*/
		       /* name of the wave file	and save it internally	*/
		       /* while	also opening the wave file for usage by	*/
		       /* the list box control.				*/

   if (	(cSounds == 2) && pb[0]	)
       plbw->ulDClkWAV = plbw->pfnLoadWaveFile(plbw->hWnd,
					       strcpy(plbw->szDClkWAV,
						      pb));
   }
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

DosUnsetExceptionHandler(&xcptregr);
}
#pragma	subtitle("   Sound Manager - Extended Sound Messages Handler")
#pragma	page( )

/* --- mrSoundHandler ---------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	process	the extended sound messages	*/
/*     for the list box	control	window.					*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HWND   hWnd; = Window Handle					*/
/*     ULONG  msg;  = PM Message					*/
/*     MPARAM mp1;  = Message Parameter	1				*/
/*     MPARAM mp2;  = Message Parameter	2				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     mrSoundHandler =	Message	Handling Result				*/
/*									*/
/* --------------------------------------------------------------------	*/

MRESULT	EXPENTRY mrSoundHandler(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{
PLISTBOXWIN  plbw;		   /* List Box Internal	Data Pointer	*/
PSZ	     psz;		   /* String Pointer			*/
ULONG	     ul;		   /* String Length			*/
EXCEPTIONREGISTRATIONRECORD xcptregr;	   /* Exception	Record		*/

plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

switch ( msg )
   {

   /*********************************************************************/
   /*  Extended	message:   LMXM_SETSOUNDEVENT				*/
   /*			   mp1 = MPFROMLONG(ulSoundEvent);		*/
   /*			   mp2 = MPFROMP(pszWaveFileName);		*/
   /*********************************************************************/

   case	LMXM_SETSOUNDEVENT :
       if ( (LONGFROMMP(mp1) > 0L) && PVOIDFROMMP(mp2) && plbw->hmodSnd	)
	   {
		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

	   xcptregr.prev_structure   = NULL;
	   xcptregr.ExceptionHandler = &ListBoxExceptionHandler;

	   DosSetExceptionHandler(&xcptregr);

	   if (	setjmp(jBuf) )
	       {
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	       DosUnsetExceptionHandler(&xcptregr);
	       return(MRFROMLONG(TRUE));
	       }
		       /* Determine which event	is having the wave file	*/
		       /* defined					*/

	   switch ( LONGFROMMP(mp1) )
	       {
		       /* Defining the single click wave file		*/

	       case LSND_SINGLECLICK :
		   if (	stricmp(plbw->szSClkWAV, (PSZ)PVOIDFROMMP(mp2))	)
		       {
		       if ( plbw->ulSClkWAV )
			   plbw->pfnUnloadWave(plbw->hWnd, plbw->ulSClkWAV);
		       plbw->ulSClkWAV = plbw->pfnLoadWaveFile(plbw->hWnd,
							       strcpy(plbw->szSClkWAV,
								      (PSZ)PVOIDFROMMP(mp2)));
		       }
		   break;

		       /* Defining the double click wave file		*/

	       case LSND_DOUBLECLICK :
		   if (	stricmp(plbw->szDClkWAV, (PSZ)PVOIDFROMMP(mp2))	)
		       {
		       if ( plbw->ulDClkWAV )
			   plbw->pfnUnloadWave(plbw->hWnd, plbw->ulDClkWAV);
		       plbw->ulDClkWAV = plbw->pfnLoadWaveFile(plbw->hWnd,
							       strcpy(plbw->szDClkWAV,
								      (PSZ)PVOIDFROMMP(mp2)));
		       }
		   break;

	       default :
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

		   DosUnsetExceptionHandler(&xcptregr);
		   return(MRFROMLONG(TRUE));
	       }
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	   DosUnsetExceptionHandler(&xcptregr);
	   }
       else
	   return(MRFROMLONG(TRUE));
       break;

   /*********************************************************************/
   /*  Extended	message:   LMXM_QUERYSOUNDEVENT				*/
   /*			   mp1 = MPFROMLONG(ulSoundEvent);		*/
   /*			   mp2 = MPFROMP(pszWaveFileName);		*/
   /*********************************************************************/

   case	LMXM_QUERYSOUNDEVENT :
       if ( (LONGFROMMP(mp1) > 0L) && PVOIDFROMMP(mp2) && plbw->hmodSnd	)
	   {
		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

	   xcptregr.prev_structure   = NULL;
	   xcptregr.ExceptionHandler = &ListBoxExceptionHandler;

	   DosSetExceptionHandler(&xcptregr);

	   if (	setjmp(jBuf) )
	       {
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	       DosUnsetExceptionHandler(&xcptregr);
	       return(0L);
	       }

	   switch ( LONGFROMMP(mp1) )
	       {
		       /* Querying the single click wave file		*/

	       case LSND_SINGLECLICK :
		   if (	plbw->ulSClkWAV	)
		       ul = strlen(strcpy((PSZ)PVOIDFROMMP(mp2), plbw->szSClkWAV));
		   else
		       {
		       psz = (PSZ)PVOIDFROMMP(mp2);
		       psz[0] =	0;
		       ul = 0UL;
		       }
		   break;

		       /* Querying the double click wave file		*/

	       case LSND_DOUBLECLICK :
		   if (	plbw->ulSClkWAV	)
		       ul = strlen(strcpy((PSZ)PVOIDFROMMP(mp2), plbw->szDClkWAV));
		   else
		       {
		       psz = (PSZ)PVOIDFROMMP(mp2);
		       psz[0] =	0;
		       ul = 0UL;
		       }
		   break;

	       default :
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

		   DosUnsetExceptionHandler(&xcptregr);
		   return(0L);
	       }
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	   DosUnsetExceptionHandler(&xcptregr);
	   return(MRFROMLONG(ul));
	   }
       else
	   return(0L);
   }
return(0L);
}
