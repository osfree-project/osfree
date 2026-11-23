
/* Program name: Entryfld.C    Title: A	List Box Replacement		*/
/*									*/
/* OS/2	Developer Magazine, Issue:  Jan	'94, page 66                    */
/* Author:  Mark Benge	   IBM Corp.					*/
/*	    Matt Smith	   Prominare Inc.				*/
/* Description:	 Replacement for OS/2 List Box,	first of a series.	*/
/*									*/
/* Program Requirements:  OS/2 2.x					*/
/*			  IBM C	Set/2					*/
/*			  WATCOM C 386/9.0				*/
/*			  Borland C++ for OS/2				*/
/*			  OS/2 Toolkit					*/

/* Copyright ¸ International Business Machines Corp. 1991-1994		*/
/* Copyright ¸ 1989-1994  Prominare Inc.  All Rights Reserved.		*/

/* Entryfld.h	   Created:    1993-10-08  Revised:    1994-02-28	*/

/* List	Box Control Constant Definition	Header				*/

/* Functions are defined with strong typing to insure correct usage	*/
/* within the final program and	correct	linkage				*/


/* --- Memory Types ---------------------------------------------------	*/

typedef	ULONG HHEAPMEM;
#define	HALLOC_DEFAULT	0xffffffffUL

/* --- Internal	Structures --------------------------------------------	*/

#define	USER_RESERVED	   8	   /* Control Reserved Memory Size	*/

#define	QUCWP_WNDP (QWL_USER + 4)  /* Pointer to Internal Control Data	*/

typedef	struct _ENTRYFWIN
   {
	     /***********************************************************/
	     /*	Window Management					*/
	     /***********************************************************/
   HHEAPMEM  hHeap;		   /* Heap Handle			*/
   HAB	     hAB;		   /* Anchor Block Handle		*/
   HWND	     hWnd;		   /* Control Window Handle		*/
   HWND	     hwndOwner;		   /* Owner Window Handle		*/
   HWND	     hwndParent;	   /* Parent Window Handle		*/
   HPOINTER  hptr;		   /* Selection	Mouse Pointer		*/
	     /***********************************************************/
	     /*	Styles/ID						*/
	     /***********************************************************/
   ULONG     flStyle;		   /* Style Flags			*/
   ULONG     id;		   /* Window ID				*/
	     /***********************************************************/
	     /*	Flags							*/
	     /***********************************************************/
   BOOL	     fFocus;		   /* Focus Flag			*/
   BOOL	     fBtnDown;		   /* Mouse Button Down	Flag		*/
   BOOL	     fDblClk;		   /* Mouse Double Click Flag		*/
   BOOL	     fCapture;		   /* Mouse Capture Flag		*/
   BOOL	     fTimer;		   /* Mouse Capture Flag		*/
   BOOL	     fVisible;		   /* List Box Visible Flag		*/
   BOOL	     fScrollStart;	   /* Scrolling	Start Flag		*/
   BOOL	     fOvertype;		   /* Overtype Flag			*/
   BOOL	     fChanged;		   /* Text Changed Flag			*/
   BOOL	     fReadOnly;		   /* Read Only	Flag			*/
   BOOL	     fWordSelect;	   /* Word Select Flag			*/
	     /***********************************************************/
	     /*	Scrolling Rates						*/
	     /***********************************************************/
   ULONG     ulScrollRateStart;	   /* Starting Scroll Rate		*/
   ULONG     ulScrollRate;	   /* Standard Scroll Rate		*/
   LONG	     cScroll;		   /* Character	Scroll Count		*/
	     /***********************************************************/
	     /*	Window Display						*/
	     /***********************************************************/
   LONG	     cx;		   /* List Box Width			*/
   LONG	     cy;		   /* List Box Height			*/
   LONG	     lClrBackground;	   /* Background Colour			*/
   LONG	     lClrBorder;	   /* Border Colour			*/
   LONG	     lClrHilite;	   /* Hilite Colour			*/
   LONG	     lClrHiliteBackground; /* Hilite Background	Colour		*/
   LONG	     lClrText;		   /* Text Colour			*/
   RECTL     rcl;		   /* Window Rectangle			*/
   POINTL    aptlOutside[5];	   /* Border Points Array		*/
   POINTL    aptlInside[5];	   /* Border Points Array		*/
   PPOINTL   aptl;		   /* Text Display Points Array		*/
   LONG	     xScrollLimit;	   /* Scrolling	Display	Limit		*/
	     /***********************************************************/
	     /*	Indices							*/
	     /***********************************************************/
   LONG	     iAnchor;		   /* Anchor Item Index			*/
   LONG	     iSelStart;		   /* Selection	Start			*/
   LONG	     iSelEnd;		   /* Selection	End			*/
   LONG	     iSel;		   /* Selection	Point			*/
   LONG	     iOffset;		   /* Offset Index			*/
	     /***********************************************************/
	     /*	Cursor Management					*/
	     /***********************************************************/
   LONG	     xCursor;		   /* Selection	Point			*/
   LONG	     yCursor;		   /* Selection	Point			*/
   LONG	     cyCursor;		   /* Cursor Height			*/
   BOOL	     fCursorShown;	   /* Cursor Shown Flag			*/
	     /***********************************************************/
	     /*	Text Holder						*/
	     /***********************************************************/
   CHAR	     szText[1024];	   /* Text Holder			*/
   CHAR	     szPassword[1024];	   /* Display Holder			*/
   CHAR	     szUndo[1024];	   /* Undo Buffer Text Holder		*/
   CHAR	     szInput[4];	   /* Simple Input Buffer		*/
   LONG	     yBaseline;		   /* Text Baseline Measurement		*/
   LONG	     cText;		   /* Text Length			*/
   LONG	     cLimit;		   /* Text Length Limit			*/
   LONG	     cUndo;		   /* Undo Buffer Text Length		*/
   RECTL     rclText;		   /* Text Rectangle			*/
   RECTL     rclDraw;		   /* Text Drawing Rectangle		*/
   POINTL    ptlDraw;		   /* Text Display Point		*/
   LONG	     yDesc;		   /* Descender	Height			*/
   LONG	     yChar;		   /* Character	Height			*/
   LONG	     cxChar;		   /* Character	Width			*/
   LONG	     cxMargin;		   /* Character	Margin Buffer Width	*/
   HPS	     hPS;		   /* Temporary	Presentation Space	*/
   } ENTRYFWIN ;

typedef	ENTRYFWIN *PENTRYFWIN;

typedef	struct _ENTRYFDATA2    /* efd */
   {
   ULONG   cb;
   ULONG   cchEditLimit;
   ULONG   ichMinSel;
   ULONG   ichMaxSel;
   } ENTRYFDATA2 ;

typedef	ENTRYFDATA2 *PENTRYFDATA2;

typedef	union _ENTRYFCDATA    /* efcd */
   {
   ENTRYFDATA	efd;
   ENTRYFDATA2	efd2;
   } ENTRYFCDATA ;

typedef	ENTRYFCDATA *PENTRYFCDATA;

/************************************************************************/
/************************************************************************/
/*									*/
/*     Class Definition							*/
/*									*/
/************************************************************************/
/************************************************************************/

#define	CLASS_ENTRYFIELD  "EntryFieldWindow"

/************************************************************************/
/************************************************************************/
/*									*/
/*     Margins								*/
/*									*/
/************************************************************************/
/************************************************************************/

#define	EFMARGIN_LEFT	 3L
#define	EFMARGIN_TOP	 2L
#define	EFMARGIN_BOTTOM	 3L
#define	EFMARGIN_RIGHT	 3L

/************************************************************************/
/************************************************************************/
/*									*/
/*     Move Direction							*/
/*									*/
/************************************************************************/
/************************************************************************/

#define	EFMOVE_LEFT	   0L
#define	EFMOVE_RIGHT	   1L

/************************************************************************/
/************************************************************************/
/*									*/
/*     Highlight Flags							*/
/*									*/
/************************************************************************/
/************************************************************************/

#define	EFDISPLAY_NORMAL	0L
#define	EFDISPLAY_HIGHLIGHT	1L

/************************************************************************/
/************************************************************************/
/*									*/
/*     Helper Macros							*/
/*									*/
/************************************************************************/
/************************************************************************/

#define	VALIDATERIGHTMARGIN(x) ((x) > pefw->ptlText.xRight ? pefw->ptlText.xRight : (x))
#define	FINDCHARACTER(xClick) (pefw->xCursor = pefw->aptl[pefw->iSel = lFindCharacterIndex(pefw, (xClick))].x +	EFMARGIN_LEFT)
#define	FALLOWSCROLLRIGHT(z) (((pefw->aptl[pefw->cText].x - pefw->aptl[pefw->iOffset].x) < (pefw->rclText.xRight / 3L))	?\
			      FALSE : TRUE)

/************************************************************************/
/************************************************************************/
/*									*/
/*     Prototypes							*/
/*									*/
/************************************************************************/
/************************************************************************/

/* --- EventMgr.C Prototype Definitions	-------------------------------	*/

MRESULT	mrNotifyOwner(PENTRYFWIN pefw, ULONG ulNotification);

/* --- HeapMgr.C Prototype Definitions --------------------------------	*/

HHEAPMEM HeapAlloc(ULONG cbInitial, ULONG cbNewBlks);
VOID	 HeapRelease(HHEAPMEM hHeap);
ULONG	 HeapSize(HHEAPMEM hHeap);

VOID	 HeapStatus(HHEAPMEM hHeap, PULONG pcBlocks, PULONG pulSize,
		    PULONG pulUsed, PULONG pulFree, PULONG pulUnused, PULONG pulOverhead);
VOID	 HeapDisplayStatus(HHEAPMEM hHeap);

PVOID	 HeapMalloc(HHEAPMEM hHeap, ULONG cbSize);
PVOID	 HeapCalloc(HHEAPMEM hHeap, ULONG cItems, ULONG	cbSize);
PVOID	 HeapRealloc(HHEAPMEM hHeap, PVOID pv, ULONG cbSize);
VOID	 HeapFree(HHEAPMEM hHeap, PVOID	pv);

/* --- ListBox.C Prototype Definitions --------------------------------	*/

BOOL EXPENTRY fRegisterEntryField(HAB hAB);
MRESULT	EXPENTRY EntryWndProc(HWND hWnd, ULONG msg, MPARAM mp1,	MPARAM mp2);
