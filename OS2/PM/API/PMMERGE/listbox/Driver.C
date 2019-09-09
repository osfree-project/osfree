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

#include "listbox.h"

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

PSZ pszDriverClassName = "ListBox.Desktop";

HPOINTER hptrWait;
HPOINTER hptrArrow;
HWND	 hwndDriver;		   /* Program Window Handle		*/
HWND	 hwndDriverFrame;	   /* Program Frame Handle		*/
HAB	 hAB;			   /* Program Anchor Block Handle	*/
HMQ	 hmqDriver;		   /* Program Message Queue Handle	*/

HWND	 hwndListx;
HWND	 hwndList1;
HWND	 hwndList2;

HWND	 hwndListA;
HWND	 hwndListB;

#define	RGBCLR_DIALOGBACKGROUND	0x00ccccccL

INT main(INT, CHAR *[ ]);
MRESULT	EXPENTRY ListBoxWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2);
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
CHAR	     szFile[CCHMAXPATH];   /* File HOlder			*/
FILEFINDBUF3 findbuf;		   /* File Find	Buffer			*/
FONTMETRICS  fm;		   /* Font Metrics Structure		*/
HDIR	     hDir = (HDIR)HDIR_CREATE; /* Directory Handle		*/
HPS	     hPS;		   /* Presentation Space Handle		*/
HWND	     hwndListBox;	   /* List Box Window Handle		*/
LISTBOXCDATA lbwcd;		   /* List Box Window Control Data	*/
POWNERITEM   poi;		   /* Owner Item			*/
RECTL	     rcl;		   /* Client Window Rectangle		*/
ULONG	     ulFileCnt = 1UL;	   /* File Counter Holder		*/
register INT i,	k, n;		   /* Loop Counter			*/

switch ( msg )
   {
		       /* Perform window initialization			*/
   case	WM_CREATE :
       hptrArrow = WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW,	FALSE);
       WinSetPointer(HWND_DESKTOP, hptrWait = WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE));
       memset(&lbwcd, 0, sizeof(LISTBOXCDATA));
       lbwcd.cb	= sizeof(LISTBOXCDATA);
       lbwcd.ulVersion = LBV_110;
       lbwcd.vdata.lbcd1_1.flExtStyles = LSXS_CHECKBOX | LSXS_EDITABLE;

       hwndList1 = WinCreateWindow(hWnd, "ListBoxWindow", "", LS_HORZSCROLL | LS_EXTENDEDSEL | WS_VISIBLE,
				   10L,	10L, 100L, 175L, hWnd,
				   HWND_TOP, 1024UL, (PVOID)&lbwcd, (PVOID)NULL);

       hwndListx = WinCreateWindow(hWnd, "ListBoxWindow", "", LS_HORZSCROLL | LS_EXTENDEDSEL | WS_VISIBLE,
				   125L, 10L, 100L, 175L, hWnd,
				   HWND_TOP, 1024UL, (PVOID)&lbwcd, (PVOID)NULL);

       hwndList2 = WinCreateWindow(hWnd, "ListBoxWindow", "", WS_VISIBLE, 10L, 190L, 250L, 200L, hWnd,
				   HWND_TOP, 1025UL, (PVOID)&lbwcd, (PVOID)NULL);

       WinSendMsg(hwndList1, LMXM_SETSOUNDEVENT, MPFROMLONG(LSND_SINGLECLICK),
		  MPFROMP("ahooga.wav"));
       WinSendMsg(hwndListx, LMXM_SETSOUNDEVENT, MPFROMLONG(LSND_DOUBLECLICK),
		  MPFROMP("flush.wav"));

       hwndListA = WinCreateWindow(hWnd, WC_LISTBOX, "", LS_OWNERDRAW |	LS_HORZSCROLL |	LS_MULTIPLESEL | WS_VISIBLE,
				   275L, 10L, 100L, 175L, hWnd,
				   HWND_TOP, 1026UL, (PVOID)NULL, (PVOID)NULL);

		       /* Sub-class the	default	window procedure with	*/
		       /* new one which	will handle specific messages	*/
		       /* allowing better control over the sizing frame	*/

       hwndListB = WinCreateWindow(hWnd, WC_LISTBOX, "", WS_VISIBLE | LS_EXTENDEDSEL, 275L, 190L, 250L,	200L, hWnd,
				   HWND_TOP, 1027UL, (PVOID)NULL, (PVOID)NULL);

       for ( i = 0; i <	10; i++	)
	   {
	   sprintf(szFile, "Item %04d Item %04d Item %04d", i, i, i);
	       WinSendMsg(hwndListx, LM_INSERTITEM,
			  MPFROMLONG(LIT_END),
			  MPFROMP(szFile));
	   }

       n = (INT)WinSendMsg(hwndList2, LMX_CALCLINECOUNT, MPFROMLONG(175L), 0L);

       k = 0;

       WinSendMsg(hwndList2, LM_SETITEMHEIGHT, MPFROMLONG(36L),	0L);

       if ( !DosFindFirst("*.*", &hDir,	FILE_NORMAL,
			  (PVOID)&findbuf, sizeof(FILEFINDBUF3),
			  &ulFileCnt, FIL_STANDARD) )
	   {
	   do
	       {
	       WinSendMsg(hwndList2, LM_INSERTITEM,
			  MPFROMLONG(LIT_SORTDESCENDING),
			  MPFROMP(findbuf.achName));
	       WinSendMsg(hwndList1, LM_INSERTITEM,
			  MPFROMLONG(LIT_SORTASCENDING),
			  MPFROMP(findbuf.achName));
	       if ( ++k	> n )
		   WinSendMsg(hwndList2, LM_SETTOPINDEX, MPFROMLONG(k -	n), 0L);
	       WinSendMsg(hwndListB, LM_INSERTITEM,
			  MPFROMLONG(LIT_SORTDESCENDING),
			  MPFROMP(findbuf.achName));
	       WinSendMsg(hwndListA, LM_INSERTITEM,
			  MPFROMLONG(LIT_SORTASCENDING),
			  MPFROMP(findbuf.achName));
	       }
	       while ( !DosFindNext(hDir, &findbuf, sizeof(FILEFINDBUF3), &ulFileCnt) );
	   DosFindClose(hDir);
	   }
       break;

   case	WM_BUTTON1DOWN :
       WinEnableWindowUpdate(hwndListA,	FALSE);
       WinEnableWindowUpdate(hwndList1,	FALSE);

       WinSendMsg(hwndListA, LM_DELETEALL, 0L, 0L);
       WinSendMsg(hwndList1, LM_DELETEALL, 0L, 0L);
       i = 0;
       if ( !DosFindFirst("*.*", &hDir,	FILE_NORMAL,
			  (PVOID)&findbuf, sizeof(FILEFINDBUF3),
			  &ulFileCnt, FIL_STANDARD) )
	   {
	   do
	       {
	       WinSendMsg(hwndListA, LM_INSERTITEM,
			  MPFROMLONG(LIT_SORTASCENDING),
			  MPFROMP(findbuf.achName));
	       WinSendMsg(hwndList1, LM_INSERTITEM,
			  MPFROMLONG(LIT_SORTASCENDING),
			  MPFROMP(findbuf.achName));
	       ++i;
	       }
	       while ( !DosFindNext(hDir, &findbuf, sizeof(FILEFINDBUF3), &ulFileCnt) );
	   DosFindClose(hDir);
	   }
       WinShowWindow(hwndListA,	TRUE);
       WinShowWindow(hwndList1,	TRUE);

       while ( i )
	   {
	   if (	i % 2 )
	       {
	       WinSendMsg(hwndListA, LM_SELECTITEM, MPFROMLONG(i - 1), MPFROMLONG(TRUE));
	       WinSendMsg(hwndList1, LM_SELECTITEM, MPFROMLONG(i - 1), MPFROMLONG(TRUE));
	       }
	   --i;
	   }
       break;

   case	WM_CONTROL :
       switch (	SHORT2FROMMP(mp1) )
	   {
		       /* Process double click on a List Box		*/

	   case	LN_SELECT :
	       switch (	SHORT1FROMMP(mp1) )
		   {
		   case	1024UL :
		   /*  WinAlarm(HWND_DESKTOP, WA_NOTE);	  */
		       break;

		   case	1027UL :
		   /*  WinAlarm(HWND_DESKTOP, WA_ERROR);  */
		       break;
		   }
	       break;
	   }
       break;


   case	WM_MEASUREITEM :
       GpiQueryFontMetrics(hPS = WinGetPS(hWnd), sizeof(FONTMETRICS), &fm);
       WinReleasePS(hPS);
       return(MRFROMLONG(fm.lMaxBaselineExt));

		       /* Draw a list box item				*/
   case	WM_DRAWITEM :

       if ( SHORT1FROMMP(mp1) == 1024 )
	   hwndListBox = hwndList1;
       else
	   if (	SHORT1FROMMP(mp1) == 1026 )
	       hwndListBox = hwndListA;

		       /* Point	to Owner Item information		*/

       poi = (POWNERITEM)mp2;
       GpiCreateLogColorTable(poi->hps,	0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);

       i = (INT)LONGFROMMR(WinSendMsg(hwndListBox, LM_QUERYITEMTEXTLENGTH,
				      MPFROMLONG(poi->idItem), 0L));

       i = (INT)LONGFROMMR(WinSendMsg(hwndListBox, LM_QUERYITEMTEXT,
				      MPFROM2SHORT(poi->idItem,	CCHMAXPATH),
				      MPFROMP(szFile)));

		       /* Check	to see if item to be drawn or		*/
		       /* highlighted					*/

       if ( poi->fsState != poi->fsStateOld )
	   {
	   WinFillRect(poi->hps, &poi->rclItem,	RGBCLR_DIALOGBACKGROUND);

	   if (	poi->fsState )
	       WinDrawText(poi->hps, -1L, szFile, &poi->rclItem,
			   RGB_BLACK, RGBCLR_DIALOGBACKGROUND, DT_LEFT | DT_VCENTER |
			   DT_ERASERECT);

		       /* Set item highlighting	by inverting the items	*/
		       /* rectangle colour				*/

	   WinInvertRect(poi->hps, &poi->rclItem);

	   if (	!poi->fsState )
	       WinDrawText(poi->hps, -1L, szFile, &poi->rclItem,
			   RGB_BLACK, RGBCLR_DIALOGBACKGROUND, DT_LEFT | DT_VCENTER |
			   DT_ERASERECT);
	   poi->fsState	= poi->fsStateOld = 0;
	   }
       else
	   {
		       /* Item needs to	be drawn, get item rectangle	*/
		       /* item position					*/

	   WinFillRect(poi->hps, &poi->rclItem,	RGBCLR_DIALOGBACKGROUND);

	   WinDrawText(poi->hps, -1L, szFile, &poi->rclItem,
		       RGB_BLACK, RGBCLR_DIALOGBACKGROUND, DT_LEFT | DT_VCENTER	|
		       DT_ERASERECT);
	   }
       return(MRFROMLONG(TRUE));

   case	WM_BUTTON1DBLCLK :
       i = 0;
       if ( !DosFindFirst("*.*", &hDir,	FILE_NORMAL,
			  (PVOID)&findbuf, sizeof(FILEFINDBUF3),
			  &ulFileCnt, FIL_STANDARD) )
	   {
	   do
	       {
	       ++i;
	       }
	       while ( !DosFindNext(hDir, &findbuf, sizeof(FILEFINDBUF3), &ulFileCnt) );
	   DosFindClose(hDir);
	   }

       while ( i )
	   {
	   if (	i % 2 )
	       {
	       WinSendMsg(hwndListA, LM_SELECTITEM, MPFROMLONG(i - 1), MPFROMLONG(FALSE));
	       WinSendMsg(hwndList1, LM_SELECTITEM, MPFROMLONG(i - 1), MPFROMLONG(FALSE));
	       }
	   --i;
	   }
       break;

   case	WM_BUTTON2DOWN :

       for ( i = 0; i <	5; i++ )
	   WinSendMsg(hwndListx, LM_DELETEITEM,	MPFROMSHORT(2),	0L);

#if 0
       WinSendMsg(hwndListA, LM_SELECTITEM, MPFROMLONG(LIT_NONE), MPFROMLONG(TRUE));

       i = LIT_FIRST;
       while ( (i = (INT)LONGFROMMR(WinSendMsg(hwndList1, LM_QUERYSELECTION, MPFROMLONG(i), 0L))) != LIT_NONE )
	   WinSendMsg(hwndList1, LM_SELECTITEM,	MPFROMLONG(i), MPFROMLONG(FALSE));
#endif
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

		       /* Initialize the program for PM	and create the	*/
		       /* message queue					*/

WinSetCp(hmqDriver = WinCreateMsgQueue(hAB = WinInitialize(0), 0), 850);

		       /* Register the main program window class	*/

if ( !WinRegisterClass(hAB, pszDriverClassName,	(PFNWP)DriverWndProc,
		       CS_CLIPCHILDREN | CS_SYNCPAINT |	CS_SIZEREDRAW, 0) )
   return(1);
		       /* Register the main program window class	*/

if ( !fRegisterListBox(hAB) )
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

return(0);
}
