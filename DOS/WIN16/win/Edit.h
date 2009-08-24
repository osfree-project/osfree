/*
	@(#)Edit.h	2.10
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
 
#ifndef Edit__h
#define Edit__h


typedef struct tagFRAGMENT
{
	int nLength;
	int nOffset;
} FRAGMENT, FAR * LPFRAGMENT;

#define ES_FOCUS	0x0001	/* Has input focus */
#define ES_MODIFIED	0x0002	/* Text has been modified */
#define ES_SHIFTDOWN    0x0004	/* Shift key is currently pressed */
#define ES_CTRLDOWN     0x0008	/* Ctrl ley is currently pressed */
#define ES_CARET	0x0010	/* we have already created a cursor */
#define ES_FORMATRECT	0x0020	/* has format rect (EM_SETRECT msg) */
#define ES_FORMATLINE	0x0040	/* EM_FMTLINES message received */

/* Private window data fields */
#define	SWD_LPSTR	0x0000	/* pointer to LPEDIT structure */

typedef struct tagEDITUNDO
{
	int	nStartPos ;
	int	nEndPos ;
	char   *pDelChars ;
	int	nDelLen ;
} EDITUNDO ;

typedef struct tagEDIT
{
	/* control specific information */
	UINT	state;   		/* flags, FOCUS... */
	UINT	lpid;			/* the window control id */
	int 	lpidx;
	HANDLE	hData;			/* data handle		*/
	LPSTR	npdata;			/* data pointer */
	int	uSelDS;			/* DS selector for binary/0 for src */
	int	memlen;			/* size of memory buffer	*/
	int	strlen;  		/* size of current string  	   */
	unsigned long	nMaxLen;		/* longest possible string */

	/* displayed line */
	LPFRAGMENT lpFragments;
	int 	nCurFragment;
	int	anchor;    		/* anchor index, -1 => not dropped */
	int	linelen;		/* the length of a line */
	int	lineheight; 		/* the height of a single line */
	int	externalLeading;
	int	aveWidth;		/* average character width */
	int 	VisLines;		/* number of visible lines */
	int 	nFirstFragment;
	int 	nFragments;
	POINT	ptOffset;		/* offset for drawing */
	RECT	rcClient;   		/* the writable area */

	int	lastAnchor;
	int	lastidx;

	HBRUSH	hBr;	     		/* the brush to use for the bground */

	HFONT	hFont;			/* font handle to TextOut() with */	
	DWORD 	dwStyle;		/* same as dwStyle in CreateWindow() */

	UINT	PasswordChar;		/* password character */

	BOOL	bMouseAnchor;   	/* TRUE user drag/selecting w/ mouse */
	EDITWORDBREAKPROC	WordBreakProc ;

	int	cTabs;			/* Number of tabs stops */
	int    *pTabPos;		/* Pointer to tab array */

	HWND	hWnd;
	
	EDITUNDO *pUndoBuffer ;
} EDIT, FAR *LPEDIT ;

/* General window access */
LPEDIT GetLPEdit(HWND hWnd) ;
void NotifyParent(LPEDIT lp, UINT msg) ;

/* The caret and the cursor position */
enum PREFER { any, next, prev, here, other } ;
void DoCaretPos(LPEDIT lp, BOOL bScroll);
void SetEditPosition(LPEDIT lp, int idx, enum PREFER pref);
void MovePosTo(LPEDIT lp, int idx, enum PREFER pref);	/* Also updates */

/* Point translation */
void ClientToWindowPoint(LPEDIT, POINT*);
void ClientToWindowRect(LPEDIT, RECT*);
void WindowToClientPoint(LPEDIT, POINT*);
void WindowToClientRect(LPEDIT, RECT*);

/* Scrolling */
void VScroll(LPEDIT lp, int nLines);
void ScrollToLine(LPEDIT lp, int line);
void HScroll(LPEDIT lp, int nPixels);
void ScrollToHPos(LPEDIT lp, int pos);
int HPosAt(LPEDIT lp, int line, int pos) ;
int HPosAtIndex(LPEDIT lp, int idx) ;
int HPosHere(LPEDIT lp) ;
void ScrollToHere(LPEDIT lp) ;
void SetVScrollSize(LPEDIT lp);
void SetHScrollSize(LPEDIT lp);

/* Anchoring */
enum anchor_status { no_anchor = -1 } ;

void SetAnchor(LPEDIT lp, int idx);
void ClearAnchor(LPEDIT lp);
BOOL AnchorSet(LPEDIT lp) ;
void GetAnchor(LPEDIT lp, int *beg, int *end);

/* Status and style */
BOOL HasFocus(LPEDIT lp) ;
BOOL FormatLines(LPEDIT lp) ;
BOOL IsModified(LPEDIT lp) ;

BOOL HasVScroll(LPEDIT lp) ;
BOOL AutoVScroll(LPEDIT lp) ;
BOOL HasHScroll(LPEDIT lp) ;
BOOL AutoHScroll(LPEDIT lp) ;
BOOL Multiline(LPEDIT lp) ;
BOOL NoHideSel(LPEDIT lp) ;
BOOL IsReadOnly(LPEDIT lp) ;
BOOL IsPassword(LPEDIT lp) ;

void SetReadOnly(LPEDIT lp, BOOL b) ;
void SetModified(LPEDIT lp, BOOL b) ;

/* Drawing and updating */
void PaintEditControl(LPEDIT lp);	/* WM_PAINT handler */
void InvalidateLine(LPEDIT lp, int lineno);	/* Invalidate one specific line */
void UpdateMe(LPEDIT lp);		/* Redraw stuff */
void UpdateHighlight(LPEDIT lp);	/* Invalidate all highlighted lines */
void UpdateAnchor(LPEDIT lp);		/* Invalidate just the anchor */
void UpdateRange(LPEDIT lp, int start, int end);
void InvalidateClientRect(LPEDIT lp, RECT*);	/* Invalidates specific region in client area */

void SetLineHeight(LPEDIT lp);
void SetEditRect(LPEDIT lp);
int Width(LPEDIT lp) ;
int Height(LPEDIT lp) ;
int FindSpot(LPEDIT lp, int x, int y);
void DrawLineAt(LPEDIT lp, HDC, int lineno);

/* Index <-> line translation */
int FindLine(LPEDIT lp, int idx);
BOOL IndexBeforeLine(LPEDIT lp, int idx, int line);
BOOL IndexOnLine(LPEDIT lp, int idx, int line);
BOOL IndexAfterLine(LPEDIT lp, int idx, int line);

/* Horizontal position <-> x,y translation */
int xyToHPos(LPEDIT lp, int line, int pos);
int HPosToX(LPEDIT lp, int line, int HPos);

/* Actual edit control stuff (insertion/deletion) */
void InsertChars(LPEDIT lp, LPSTR str, int len, BOOL bTestReadOnlyStyle);
void DeleteChars(LPEDIT lp, int beg, int end, BOOL bTestReadOnlyStyle);

/* Fragment management */
FRAGMENT *CurFragment(LPEDIT lp) ;
char *StartOf(LPEDIT lp, int nFrag) ;
void ReformatFragments(LPEDIT lp);	/* Rebuilds fragment table, invalidates if necessary */
int BuildAFragment(LPEDIT lp, int *start);

/* Undo support */
BOOL CanUndo(LPEDIT lp) ;
void DoUndo(LPEDIT lp) ;
void ClearUndoBuffer(LPEDIT lp) ;
void SetUndoInsert(LPEDIT lp, int nPos, char *pIns, int nLen) ;
void SetUndoDelete(LPEDIT lp, int nPos, char *pDel, int nLen) ;

/* EM_FMTLINES message support */
void InsertRRN(LPEDIT lp) ;
void DeleteRRN(LPEDIT lp) ;

/* Keystroke handlers */
void	TextMsg(LPEDIT lp, UINT ch) ;
void	HandleDelete(LPEDIT lp) ;
void	HandleInsert(LPEDIT lp) ;
void	HandleHome(LPEDIT lp) ;
void	HandleEnd(LPEDIT lp) ;
void	HandleLeft(LPEDIT lp) ;
void	HandleRight(LPEDIT lp) ;
void	HandleUp(LPEDIT lp) ;
void	HandleDown(LPEDIT lp) ;
void	HandleNext(LPEDIT lp) ;
void	HandlePrior(LPEDIT lp) ;

LRESULT CALLBACK _export DefEDITProc(HWND,UINT,WPARAM,LPARAM);
int CALLBACK DefWordBreakProc(LPSTR lpszText, int iWordBegin, int nTextLen, int action);

#ifdef LATER
#define GetHandle32(ptr) GlobalPtrHandle(ptr)
#else
#define GetHandle32(ptr) GlobalHandle32(ptr)
#endif

#define INIT_DATA_SIZE  32	/* size to alloc when we are created */
#define INIT_DATA_INCR	8	/* increment line index array size */

#define INNER_OFFSET	2 	/* offset from each side of the client window */

#define EDITMEMORY	(GMEM_MOVEABLE | GMEM_ZEROINIT)

#ifdef TWIN32
#define MB_STANDARDBEEP   0xFFFFFFFF
#else
#define MB_STANDARDBEEP   0xFFFF
#endif

#define SetLPEDIT(hWnd,lp) 	((LPEDIT)SetWindowLong(hWnd,SWD_LPSTR,lp))

/* prototypes for EditClip.c */

void SelectionToClipboard(HWND, LPEDIT);
void ClipboardToSelection(HWND, LPEDIT);

/* prototypes for EditMem.c */

LRESULT EditMemoryAPI(HWND,UINT,WPARAM,LPARAM);

typedef LRESULT (*EDITMEMORYPROC)(HWND, UINT, WPARAM, LPARAM) ;

/* action flags for EditMemoryAPI */
#define EMA_ALLOC	1
#define EMA_REALLOC	2
#define EMA_LOCK	3
#define EMA_UNLOCK	4
#define EMA_FREE	5
#define EMA_SIZE	6

#define LF_EDIT		LF_CONTROL

#endif /* EDIT__H */
