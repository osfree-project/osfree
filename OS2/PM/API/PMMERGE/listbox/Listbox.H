
/* Program name: Listbox.C    Title: A List Box	Replacement		*/
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

/* listbox.h	   Created:    1993-10-08  Revised:    1994-02-28	*/

/* List	Box Control Constant Definition	Header				*/

/* Functions are defined with strong typing to insure correct usage	*/
/* within the final program and	correct	linkage				*/

/************************************************************************/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/*									*/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/*									*/ /* 1.1 */
/*     Additions for Version 1.1 of the	control	are denoted with	*/ /* 1.1 */
/*     comments	starting in column 76 with the 1.1 version number	*/ /* 1.1 */
/*									*/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/*									*/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/************************************************************************/ /* 1.1 */

/************************************************************************/ /* 1.2 */
/************************************************************************/ /* 1.2 */
/*									*/ /* 1.2 */
/************************************************************************/ /* 1.2 */
/*									*/ /* 1.2 */
/*     Additions for Version 1.2 of the	control	are denoted with	*/ /* 1.2 */
/*     comments	starting in column 76 with the 1.2 version number	*/ /* 1.2 */
/*									*/ /* 1.2 */
/************************************************************************/ /* 1.2 */
/*									*/ /* 1.2 */
/************************************************************************/ /* 1.2 */
/************************************************************************/ /* 1.2 */


/* --- Memory Types ---------------------------------------------------	*/

typedef	ULONG HHEAPMEM;
#define	HALLOC_DEFAULT		  0xffffffffUL

/* --- Internal	Structures --------------------------------------------	*/

#define	USER_RESERVED	   8	   /* Control Reserved Memory Size	*/

#define	QUCWP_WNDP (QWL_USER + 4)  /* Pointer to Internal Control Data	*/

#define	CBLK_LIST	   1024	   /* List Array Block Size		*/

typedef	struct _LISTITEM
   {				   /* Structure	Size:  24 Bytes		*/
   PSZ	     pszText;		   /* Text Pointer			*/
   ULONG     cText;		   /* Text Length			*/
   ULONG     ulHandle;		   /* User Defined Handle		*/
   ULONG     fl;		   /* Flags (Upper 16 bits old Flags)	*/
   LONG	     cyItem;		   /* Item Height			*/
   LONG	     cxItem;		   /* Item Width			*/
   } LISTITEM ;

typedef	LISTITEM *PLISTITEM;

#define	LI_CLEAR	   0x00000000UL
#define	LI_SELECTED	   0x00000001UL
#define	LI_FOCUS	   0x00000002UL
#define	LI_CHECKED	   0x00000004UL					   /* 1.1 */
#define	LI_INSERTED	   0x00001000UL
#define	LI_SELECTEDPREV	   0x00000010UL
#define	LI_SELECTEDOLD	   0x00010000UL
#define	LI_FOCUSOLD	   0x00020000UL
#define	LI_CHECKEDOLD	   0x00040000UL					   /* 1.1 */
#define	LI_INSERTEDOLD	   0x10000000UL
#define	LI_SELECTEDPREVOLD 0x00100000UL

typedef	struct _LISTCOL
   {
   HHEAPMEM  hHeap;		   /* Heap Handle			*/
   PLISTITEM *apli;		   /* List Item	Array Pointer		*/
   } LISTCOL ;

typedef	LISTCOL	*PLISTCOL;

typedef	struct _LISTBOXWIN
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
   ULONG     flStyleExt;	   /* Extended Style Flags		*/ /* 1.1 */
   ULONG     id;		   /* Window ID				*/
	     /***********************************************************/
	     /*	Flags							*/
	     /***********************************************************/
   BOOL	     fFocus;		   /* Focus Flag			*/
   BOOL	     fDblClk;		   /* Mouse Double Click Flag		*/
   BOOL	     fCapture;		   /* Mouse Capture Flag		*/
   BOOL	     fTimer;		   /* Mouse Capture Flag		*/
   BOOL	     fVisible;		   /* List Box Visible Flag		*/
   BOOL	     fDelaySort;	   /* Delay Sort Flag			*/
   BOOL	     fFocusShown;	   /* FocusShown Flag			*/
	     /***********************************************************/
	     /*	Window Display						*/
	     /***********************************************************/
   LONG	     cx;		   /* List Box Width			*/
   LONG	     cy;		   /* List Box Height			*/
   LONG	     cxWindow;		   /* List Box Width  (Actual)		*/
   LONG	     cyWindow;		   /* List Box Height (Actual)		*/
   LONG	     lClrBackground;	   /* Background Colour			*/
   LONG	     lClrBorder;	   /* Border Colour			*/
   LONG	     lClrHilite;	   /* Hilite Colour			*/
   LONG	     lClrHiliteBackground; /* Hilite Background	Colour		*/
   LONG	     lClrList;		   /* List Colour			*/
   LONG	     lClrText;		   /* Text Colour			*/
   LONG	     lClrListIndex;	   /* List Colour			*/
   LONG	     lClrTextIndex;	   /* Text Colour			*/
   RECTL     rcl;		   /* Window Rectangle			*/
   RECTL     rclCorner;		   /* Scroll Corner Rectangle		*/
   RECTL     rclAdjusted;	   /* Adjusted Top Rectangle		*/
   POINTL    aptlOutside[5];	   /* Border Points Array		*/
   POINTL    aptlInside[5];	   /* Border Points Array		*/
   LONG	     cptl;		   /* Border Points Array Count		*/
	     /***********************************************************/
	     /*	Scroll Bars						*/
	     /***********************************************************/
   SWP	     aswp[4];		   /* Scroll Bar Position Array		*/
   HWND	     hwndScrollBottom;	   /* Bottom Scroll Bar	Window Handle	*/
   HWND	     hwndScrollLeft;	   /* Left Scroll Bar Window Handle	*/
   HWND	     hwndScrollRight;	   /* Right Scroll Bar Window Handle	*/
   HWND	     hwndScrollTop;	   /* Top Scroll Bar Window Handle	*/
   LONG	     cxScroll;		   /* Vertical Scroll Bar Width		*/
   LONG	     cyScroll;		   /* Horizontal Scroll	Bar Height	*/
	     /***********************************************************/
	     /*	Scroll Bar Indices					*/
	     /***********************************************************/
   LONG	     cHorzScroll;	   /* Horizontal Scroll	Count		*/
   LONG	     cVertScroll;	   /* Vertical Scroll Count		*/
   LONG	     iHorzScroll;	   /* Horizontal Scroll	Position	*/
   LONG	     iVertScroll;	   /* Vertical Scroll Position		*/
	     /***********************************************************/
	     /*	Indices							*/
	     /***********************************************************/
   LONG	     iAnchor;		   /* Anchor Item Index			*/
   LONG	     iAnchorLast;	   /* Last Anchor Item Index		*/
   LONG	     iFocus;		   /* Focus Item Index			*/
   LONG	     iFocusLast;	   /* Last Focus Item Index		*/
   LONG	     iSelected;		   /* Selected Item Index		*/
   LONG	     iSelectedLast;	   /* Last Selected Item Index		*/
	     /***********************************************************/
	     /*	List Box Limits						*/
	     /***********************************************************/
   LONG	     cCharsPage;	   /* Characters per Page		*/
   LONG	     cLinesPage;	   /* Lines per	Page			*/
   LONG	     cxItem;		   /* Longest Horizontal Item		*/
   LONG	     cyItem;		   /* Item Height			*/
   LONG	     xChar;		   /* Character	Width			*/
   LONG	     yAscender;		   /* Character	Ascender Height		*/
	     /***********************************************************/
	     /*	List Box List						*/
	     /***********************************************************/
   PLISTCOL  plc;		   /* List Column Array			*/
   LONG	     cColumns;		   /* Columns Count			*/
   LONG	     cItems;		   /* Items Count			*/
   BOOL	     fDirty;		   /* Dirty Update Flag			*/
	     /***********************************************************/ /* 1.1 */
	     /*	Sound Support						*/ /* 1.1 */
	     /***********************************************************/ /* 1.1 */
   HMODULE   hmodSnd;		   /* Sound Support DLL	Handle		*/ /* 1.1 */
   ULONG     (*	EXPENTRY pfnLoadWaveFile)(HWND hWnd, PSZ pszWaveFile);	   /* 1.1 */
   VOID	     (*	EXPENTRY pfnUnloadWave)(HWND hWnd, ULONG hSound);	   /* 1.1 */
   VOID	     (*	EXPENTRY pfnPlayWave)(ULONG hSound);			   /* 1.1 */
   CHAR	     szSClkWAV[CCHMAXPATH];/* Single Click Wave	File		*/ /* 1.1 */
   ULONG     ulSClkWAV;		   /* Single Click Wave	File Handle	*/ /* 1.1 */
   CHAR	     szDClkWAV[CCHMAXPATH];/* Double Click Wave	File		*/ /* 1.1 */
   ULONG     ulDClkWAV;		   /* Double Click Wave	File Handle	*/ /* 1.1 */
	     /***********************************************************/ /* 1.1 */
	     /*	Check Box Support					*/ /* 1.1 */
	     /***********************************************************/ /* 1.1 */
   HBITMAP   hbm;		   /* Check Mark Bitmap	Handle		*/ /* 1.1 */
   BOOL	     fEnableCheckboxes;	   /* Check Box	Enable Flag		*/ /* 1.1 */
   RECTL     rclChecked;	   /* Checked Bitmap Rectangle		*/ /* 1.1 */
   RECTL     rclUnChecked;	   /* Unchecked	Bitmap Rectangle	*/ /* 1.1 */
   LONG	     xTextOffset;	   /* Text Offset			*/ /* 1.1 */
   LONG	     cyHalfBitmap;	   /* Bitmap Half Height		*/ /* 1.1 */
	     /***********************************************************/ /* 1.2 */
	     /*	Direct Editing Support					*/ /* 1.2 */
	     /***********************************************************/ /* 1.2 */
   HWND	     hwndEdit;		   /* Entry Field Handle		*/ /* 1.2 */
   LONG	     iEditing;		   /* Entry Field Item Index		*/ /* 1.2 */
   } LISTBOXWIN	;

typedef	LISTBOXWIN *PLISTBOXWIN;

#define	SWP_VERT       0
#define	SWP_HORZ       1

#define	IDT_SCROLL    512

#if defined(VERSION_100)						   /* 1.1 */

typedef	struct _LISTBOXCDATA
   {
   ULONG    cb;			   /* Structure	Size			*/
   ULONG    ulVersion;		   /* Structure	Version	NUmber		*/
   ULONG    cItems;		   /* Items Count			*/
   BYTE	    abList[1];		   /* Array List Start			*/
   } LISTBOXCDATA ;

typedef	LISTBOXCDATA *PLISTBOXCDATA;

#define	LBV_100		10000UL	   /* Control Data Version Number	*/

#else									   /* 1.1 */
									   /* 1.1 */
typedef	struct _LISTBOXCDATA1						   /* 1.1 */
   {									   /* 1.1 */
   ULONG    cItems;		   /* Items Count			*/ /* 1.1 */
   BYTE	    abList[1];		   /* Array List Start			*/ /* 1.1 */
   } LISTBOXCDATA1 ;							   /* 1.1 */
									   /* 1.1 */
typedef	struct _LISTBOXCDATA2						   /* 1.1 */
   {									   /* 1.1 */
   ULONG    flExtStyles;	   /* Extended Styles			*/ /* 1.1 */
   ULONG    cItems;		   /* Items Count			*/ /* 1.1 */
   ULONG    cSounds;		   /* Sounds Count			*/ /* 1.1 */
   BYTE	    abList[1];		   /* Array List Start			*/ /* 1.1 */
   } LISTBOXCDATA2 ;							   /* 1.1 */
									   /* 1.1 */
typedef	union								   /* 1.1 */
   {									   /* 1.1 */
   LISTBOXCDATA1 lbcd1_0;	   /* List Box Control Data Version 1.0	*/ /* 1.1 */
   LISTBOXCDATA2 lbcd1_1;	   /* List Box Control Data Version 1.1	*/ /* 1.1 */
   } LISTBOXVDATA ;							   /* 1.1 */
									   /* 1.1 */
typedef	struct _LISTBOXCDATA						   /* 1.1 */
   {									   /* 1.1 */
   ULONG	cb;		   /* Structure	Size			*/ /* 1.1 */
   ULONG	ulVersion;	   /* Structure	Version	Number		*/ /* 1.1 */
   LISTBOXVDATA	vdata;		   /* Version Specific Data Union	*/ /* 1.1 */
   } LISTBOXCDATA ;							   /* 1.1 */
									   /* 1.1 */
typedef	LISTBOXCDATA *PLISTBOXCDATA;					   /* 1.1 */
									   /* 1.1 */
#define	LBV_100	   0x00010000UL	   /* Control Data Version Number	*/ /* 1.1 */
#define	LBV_110	   0x000100a0UL	   /* Control Data Version Number	*/ /* 1.1 */
									   /* 1.1 */
#endif									   /* 1.1 */

#define	LBV_120	   0x000100b0UL	   /* Control Data Version Number	*/ /* 1.2 */

/************************************************************************/
/************************************************************************/
/*									*/
/*     Class Definition							*/
/*									*/
/************************************************************************/
/************************************************************************/

#define	CLASS_LISTBOX  "ListBoxWindow"

/************************************************************************/
/************************************************************************/
/*									*/
/*     Inline Macros							*/
/*									*/
/************************************************************************/
/************************************************************************/

#define	SaveSelectState(iCol, iSelected)  { if ( plbw->flStyle & LS_OWNERDRAW )\
   if (	(plbw->plc[iCol].apli[iSelected]->fl & LI_SELECTED) && fItemVisible(plbw, iSelected) )\
       plbw->plc[iCol].apli[iSelected]->fl |= LI_SELECTEDPREV;\
   else\
       plbw->plc[iCol].apli[iSelected]->fl &= ~LI_SELECTEDPREV;	}

/************************************************************************/
/************************************************************************/
/*									*/
/*     Extended	Messages						*/
/*									*/
/************************************************************************/
/************************************************************************/

#define	LMX_CALCLINECOUNT      0x0180UL
#define	LMX_CALCSIZE	       0x0181UL
#define	LMX_COPY	       0x0182UL
#define	LMX_CUT		       0x0183UL
#define	LMX_PASTE	       0x0184UL
#define	LMX_QUERYITEMCOUNT     0x0185UL
#define	LMX_QUERYITEMRECT      0x0186UL
#define	LMX_QUERYSELECTLIST    0x0187UL
#define	LMX_RESETLIST	       0x0188UL
#define	LMX_SELECTALL	       0x0189UL
#define	LMX_SELECTITEMBITMAP   0x018aUL
#define	LMX_SETARRAY	       0x018bUL
#define	LMX_SETARRAYHANDLES    0x018cUL
#define	LMX_SETITEMBITMAPS     0x018dUL
#define	LMX_SETITEMCOUNT       0x018eUL
#define	LMX_SORT	       0x018fUL
#define	LMX_SETARRAYITEMS      0x0190UL
#define	LMX_ADDARRAY	       0x0191UL

#define	LMX_SETCHECK	       0x0192UL					   /* 1.1 */
#define	LMX_QUERYCHECK	       0x0193UL					   /* 1.1 */
#define	LMX_SETCHECKARRAY      0x0194UL					   /* 1.1 */
#define	LMX_QUERYCHECKARRAY    0x0195UL					   /* 1.1 */

#define	LMXM_SETSOUNDEVENT     0x0196UL					   /* 1.1 */
#define	LMXM_QUERYSOUNDEVENT   0x0197UL					   /* 1.1 */

/************************************************************************/ /* 1.2 */
/************************************************************************/ /* 1.2 */
/*									*/ /* 1.2 */
/*     Private Messages							*/ /* 1.2 */
/*									*/ /* 1.2 */
/************************************************************************/ /* 1.2 */
/************************************************************************/ /* 1.2 */

#define	WM_DESTROYEDIT	   (WM_USER + 1024UL)				   /* 1.2 */

/************************************************************************/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/*									*/ /* 1.1 */
/*     Sound Events							*/ /* 1.1 */
/*									*/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/************************************************************************/ /* 1.1 */

#define	LSND_SINGLECLICK	 1					   /* 1.1 */
#define	LSND_DOUBLECLICK	 2					   /* 1.1 */

/************************************************************************/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/*									*/ /* 1.1 */
/*     Check Boxes							*/ /* 1.1 */
/*									*/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/************************************************************************/ /* 1.1 */

#define	IDB_CHECKMARKS		 512					   /* 1.1 */

/************************************************************************/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/*									*/ /* 1.1 */
/*     Style Extensions							*/ /* 1.1 */
/*									*/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/************************************************************************/ /* 1.1 */

#define	LSXS_CHECKBOX		 0x00000001UL				   /* 1.1 */
#define	LSXS_EDITABLE		 0x00000002UL				   /* 1.2 */

/************************************************************************/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/*									*/ /* 1.1 */
/*     Notification Extensions						*/ /* 1.1 */
/*									*/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/************************************************************************/ /* 1.1 */

#define	LNX_CHECKED	     256UL					   /* 1.1 */
#define	LNX_EDITED	     257UL					   /* 1.2 */

/************************************************************************/
/************************************************************************/
/*									*/
/*     Prototypes							*/
/*									*/
/************************************************************************/
/************************************************************************/

/* --- EventMgr.C Prototype Definitions	-------------------------------	*/

MRESULT	mrNotifyOwner(PLISTBOXWIN plbw,	ULONG ulNotification);

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

BOOL EXPENTRY fRegisterListBox(HAB hAB);
VOID UpdateScrollBars(PLISTBOXWIN plbw);
VOID DrawSelection(PLISTBOXWIN plbw, LONG iItem);
VOID DrawItemSelection(PLISTBOXWIN plbw, LONG iItem);
BOOL fItemVisible(PLISTBOXWIN plbw, LONG iItem);
LONG lItemFromPoint(PLISTBOXWIN	plbw, LONG y);
MRESULT	EXPENTRY ScrollBarWndProc(HWND hWnd, ULONG msg,	MPARAM mp1, MPARAM mp2);
MRESULT	EXPENTRY ListBoxWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* --- ListMgr.C Prototype Definitions --------------------------------	*/

LONG lSetItemText(HHEAPMEM hHeap, HWND hWnd, PLISTITEM pli, PSZ	pszText);
VOID SaveExtendedState(PLISTBOXWIN plbw);
VOID RestoreExtendedState(PLISTBOXWIN plbw);
BOOL fSelectMultipleItem(PLISTBOXWIN plbw, LONG	iCol, LONG iSelected, BOOL fSelect);
BOOL fSelectItem(PLISTBOXWIN plbw, LONG	iCol, LONG iItem, BOOL fSelect);
VOID SelectExtendedItems(PLISTBOXWIN plbw, LONG	iCol, LONG iSelected);
VOID SetControlDataList(PLISTBOXWIN plbw, ULONG	cItems,	PBYTE pb);
MRESULT	EXPENTRY mrBaseListHandler(HWND	hWnd, ULONG msg, MPARAM	mp1, MPARAM mp2);
MRESULT	EXPENTRY mrExtendedListHandler(HWND hWnd, ULONG	msg, MPARAM mp1, MPARAM	mp2);
MRESULT	EXPENTRY mrChkBoxHandler(HWND hWnd, ULONG msg, MPARAM mp1,	   /* 1.1 */
				 MPARAM	mp2);				   /* 1.1 */

/* --- SndMgr.C	Prototype Definitions ---------------------------------	*/ /* 1.1 */

VOID LoadSoundSupport(PLISTBOXWIN plbw);				   /* 1.1 */
VOID PlaySound(PLISTBOXWIN plbw, ULONG ulSound);			   /* 1.1 */
VOID UnloadSoundSupport(PLISTBOXWIN plbw);				   /* 1.1 */
VOID SetControlDataSounds(PLISTBOXWIN plbw, ULONG cItems, ULONG	cSounds,   /* 1.1 */
			  PBYTE	pb);					   /* 1.1 */
MRESULT	EXPENTRY mrSoundHandler(HWND hWnd, ULONG msg, MPARAM mp1,	   /* 1.1 */
				MPARAM mp2);				   /* 1.1 */

/* --- TraffMgr.C Prototype Definitions	-------------------------------	*/

LONG lGetPresParam(HWND	hWnd, ULONG ulID1, ULONG ulID2,	LONG lDefault);
BOOL fAddHorzScroll(HWND hWnd, PLISTBOXWIN plbw);
VOID MeasureItem(PLISTBOXWIN plbw, LONG	lMaxBaselineExt);
VOID SizeListBox(PLISTBOXWIN plbw);
VOID FocusChange(PLISTBOXWIN plbw, LONG	iFocus,	BOOL fSet);
VOID RemoveFocus(PLISTBOXWIN plbw);
VOID SetFocus(PLISTBOXWIN plbw,	LONG iFocus);
VOID DrawItemSelection(PLISTBOXWIN plbw, LONG iItem);
BOOL fCreateEditArea(PLISTBOXWIN plbw, MPARAM mp1);			  /* 1.2 */
BOOL fDecodeChkSelect(PLISTBOXWIN plbw,	MPARAM mp1);			   /* 1.1 */
BOOL fDecodeMouse(PLISTBOXWIN plbw, MPARAM mp1);
BOOL fDecodeExtendedMouse(PLISTBOXWIN plbw, MPARAM mp1,	BOOL fBlock);
VOID LineUp(PLISTBOXWIN	plbw);
VOID LineDown(PLISTBOXWIN plbw);
VOID DragUp(PLISTBOXWIN	plbw);
VOID DragDown(PLISTBOXWIN plbw);

/************************************************************************/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/*									*/ /* 1.1 */
/*     Sound DLL Support						*/ /* 1.1 */
/*									*/ /* 1.1 */
/************************************************************************/ /* 1.1 */
/************************************************************************/ /* 1.1 */

ULONG EXPENTRY LoadWaveFile(HWND hWnd, PSZ pszWaveFile);		   /* 1.1 */
VOID  EXPENTRY UnloadWave(HWND hWnd, ULONG hSound);			   /* 1.1 */
VOID  EXPENTRY PlayWave(ULONG hSound);					   /* 1.1 */

#define	ORD_LOADWAVEFILE	 1					   /* 1.1 */
#define	ORD_PLAYWAVE		 2					   /* 1.1 */
#define	ORD_UNLOADWAVE		 3					   /* 1.1 */
