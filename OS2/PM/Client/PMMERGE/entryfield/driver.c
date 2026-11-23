#pragma	title("Prominare Designer  --  Version 2.10 -- (Colours.C)")
#pragma	subtitle("   Prominare Designer - Interface Definitions")

#define	INCL_DOS		   /* Include OS/2 DOS Kernal		*/
#define	INCL_GPI		   /* Include OS/2 PM GPI Interface	*/
#define	INCL_WIN		   /* Include OS/2 PM Windows Interface	*/
#define	INCL_WINSTDSPIN		   /* Include OS/2 PM Spin Buttons	*/
#define	INCL_NLS		   /* Include OS/2 PM NLS		*/
#define	INCL_SW			   /* Include OS/2 MultiMedia Support	*/

#pragma	info(nopar)

static char *MODID = "@(#)driver.c:1.07";

#include <os2.h>
#include <string.h>
#include <stdio.h>

#include "entryfld.h"

/* The application is created using the	following options in Prominare	*/
/* Builder:								*/
/*									*/
/* Option	       Value						*/
/* ------	       -----						*/
/* Memory Model	       Small						*/
/* Op Sys	       OS/2 2.x	PM Normal				*/
/* Optimization	       Maximize						*/
/* Compile Type	       Compile Only					*/
/*		       Error Trap					*/
/*		       ExePack						*/
/*		       Link						*/
/* CPU		       80386						*/
/* Warning Level       3						*/	
/*									*/
/* Equivalent command line invocation of each module using the		*/
/* IBM C Set/2 Compiler	Version	1.0 is:					*/
/*									*/
/*     Icc -G3s- -O+ -Sm -C -W3	-Fofilename filename.C			*/

/* Filename:   Driver.C							*/

/*  Version:   2.10							*/
/*  Created:   1990-12-29						*/
/*  Revised:   1994-01-05						*/


/* --------------------------------------------------------------------	*/

/* --- Module Definitions ---------------------------------------------	*/

#define	USER_RESERVED	   8	   /* Control Reserved Memory Size	*/

PSZ pszDriverClassName = "Entryfield.Desktop";

HPOINTER hptrWait;
HPOINTER hptrArrow;
HWND	 hwndDriver;		   /* Program Window Handle		*/
HWND	 hwndDriverFrame;	   /* Program Frame Handle		*/
HAB	 hAB;			   /* Program Anchor Block Handle	*/
HMQ	 hmqDriver;		   /* Program Message Queue Handle	*/

HWND	 hwndList1;

HWND	 hwndListA;

#define	RGBCLR_DIALOGBACKGROUND	0x00ccccccL

INT main(INT, CHAR *[ ]);
MRESULT	EXPENTRY EntryWndProc(HWND hWnd, ULONG msg, MPARAM mp1,	MPARAM mp2);
MRESULT	EXPENTRY DriverWndProc(HWND hWnd, ULONG	msg, MPARAM mp1, MPARAM	mp2);


#pragma	subtitle("   Client Window - Client Window Procedure")
#pragma	page( )

/* --- DriverWndProc --------------------------------------------------	*/
/*									*/
/*     This function is	used to	process	the messages for the main	*/
/*     Client Window.							*/
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
/*     DriverWndProc = Message Handling	Result				*/
/*									*/
/* --------------------------------------------------------------------	*/

MRESULT	EXPENTRY DriverWndProc(HWND hWnd, ULONG	msg, MPARAM mp1, MPARAM	mp2)

{
HPS	     hPS;		   /* Presentation Space Handle		*/
RECTL	     rcl;		   /* Client Window Rectangle		*/

switch ( msg )
   {
		       /* Perform window initialization			*/
   case	WM_CREATE :
       hptrArrow = WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW,	FALSE);
       WinSetPointer(HWND_DESKTOP, hptrWait = WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE));

       hwndList1 = WinCreateWindow(hWnd, "EntryFieldWindow", "Text Text Text",
				   ES_MARGIN | ES_AUTOSCROLL | WS_VISIBLE,
				   10L,	10L, 100L, 30L,	hWnd,
				   HWND_TOP, 1024UL, (PVOID)NULL, (PVOID)NULL);

       hwndListA = WinCreateWindow(hWnd, WC_ENTRYFIELD,	"Text Text Text Text Text Text",
				   ES_MARGIN | ES_AUTOSCROLL | WS_VISIBLE,
				   115L, 10L, 150L, 25L, hWnd,
				   HWND_TOP, 1026UL, (PVOID)NULL, (PVOID)NULL);
       break;
		       /* Erase	background				*/

   case	WM_ERASEBACKGROUND :
       WinQueryWindowRect(hWnd,	&rcl);
       WinFillRect((HPS)mp1, &rcl, SYSCLR_DIALOGBACKGROUND);
       break;
		       /* Paint	the main client	window			*/
   case	WM_PAINT :
       WinSetPointer(HWND_DESKTOP, hptrWait);

       WinFillRect(hPS = WinBeginPaint(hWnd, (HPS)NULL,	&rcl), &rcl, SYSCLR_DIALOGBACKGROUND);

       WinEndPaint(hPS);
       WinSetPointer(HWND_DESKTOP, hptrArrow);
       break;
		       /* Close	Down					*/
   case	WM_CLOSE :

       WinPostMsg(hWnd,	WM_QUIT, 0L, 0L);
       break;

   case	WM_DESTROY :
       WinDestroyPointer(hptrArrow);
       WinDestroyPointer(hptrWait);
       break;
		       /* Default message processing			*/
   default:
       return(WinDefWindowProc(hWnd, msg, mp1, mp2));
   }
return(0L);
}
#pragma	subtitle("   Program Controller")
#pragma	page( )

/* --- Main Program Controller ----------------------------------------	*/

INT main(INT argc, CHAR	*argv[ ])

{
QMSG	   qmsg;		   /* PM Message Queue Holder		*/
ULONG	   flCreateFlags;	   /* Window Creation Flags		*/

#ifdef INCL_VALAPI

HVAL hval;

hval = ValInitialize("Entry Field Driver", (PSZ)NULL, VL_VIEWPORT);

#endif
		       /* Initialize the program for PM	and create the	*/
		       /* message queue					*/

WinSetCp(hmqDriver = WinCreateMsgQueue(hAB = WinInitialize(0), 0), 850);

		       /* Register the main program window class	*/

if ( !WinRegisterClass(hAB, pszDriverClassName,	(PFNWP)DriverWndProc,
		       CS_CLIPCHILDREN | CS_SYNCPAINT |	CS_SIZEREDRAW, 0) )
   return(1);
		       /* Register the main program window class	*/

if ( !fRegisterEntryField(hAB) )
   return(1);
		       /* Create the main program window but do	not	*/
		       /* show it yet					*/

flCreateFlags =	FCF_TITLEBAR | FCF_NOBYTEALIGN | FCF_SYSMENU | FCF_SIZEBORDER |	FCF_SHELLPOSITION;
if ( !(hwndDriverFrame = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE,
					      &flCreateFlags,
					      pszDriverClassName, NULL,	0L,
					     (HMODULE)0L, 0L,
					      &hwndDriver)) )
   {
   return(1);
   }
		       /* Retrieve and then dispatch messages		*/

while (	WinGetMsg(hAB, &qmsg, (HWND)NULL, 0, 0)	)
   WinDispatchMsg(hAB, &qmsg);

WinDestroyWindow(hwndDriverFrame);

WinDestroyMsgQueue(hmqDriver);

		       /* Notify PM that main program thread not needed	*/
		       /* any longer					*/
WinTerminate(hAB);

#ifdef INCL_VALAPI

ValTerminate(hval);

#endif

return(0);
}
